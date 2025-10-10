/* The original source code is written <a href="www.maplant.com/2020-04-25-Writing-a-Simple-Garbage-Collector-in-C.html">here. </a>
The author said the code is not necessarily correct.
Nevertheless, we will work with what we have. This is a bit out of my depth, but this is a start to roughly know what is important.*/
/* The simplest malloc uses a linked list of free blocks of memory that can be split up and given out.*/
/* Since we use a linked list, to get out the header, the following is natural,
a pointer to next free block, and the unsigned int indicating the size.*/
typedef struct header {
    unsigned int    size;
    struct header   *next;
} header_t;
/* There is the heap between the stack and uninitialised data or BSS. The heap starts at low address bordering the BSS and ends at program break. Violation is when you access between the stack and the break.
*/
/* This is the base, or a first zero sized block to get us started. */
static header_t base;
/* This is an address pointing to first free block of memory. */
static header_t *freep = &base; 
/* This is an address pointing to first used block of memory. */
static header_t *usedp;         
/*
 * Scan the free list and look for a place to put the block. Basically, we're 
 * looking for any block that the to-be-freed block might have been partitioned from.
 */
static void
add_to_free_list(header_t *bp)
{
    /* This is the pointer that keeps track of where we are looking in the free list.*/
    header_t *p;
    /* The for loop gives a list for the corect place to insert bp.
    Denote bp as the freed memory block. 
    The list remains ordered by the memory addresses.*/
    for (p = freep; !(bp > p && bp < p->next); p = p->next)
        /* This is a like a ring, it wraps around the end of memory,
        p is at the highest address, and p->next is lowest. */
        if (p >= p->next && (bp > p || bp < p->next))
            break;
    /* Read, if bp is adjacent to p->next (bp->size is the size of the block), merge them into a single free block.*/
    if (bp + bp->size == p->next) {
        /* The merge is done here.*/
        bp->size += p->next->size;
        /* Shift this to the next pointer.*/
        bp->next = p->next->next;
        /* Otherwise, just link bp to p->next if there is no adjacency.*/
    } else
        bp->next = p->next;

    /* This is the case where if bp is adjacent to p, merge bp into p*/
    if (p + p->size == bp) {
        p->size += bp->size;
        p->next = bp->next;
    } else
        /* Otherwise, just link bp to p->next. */
        p->next = bp;

    /* Set the free block to the current block p. Updat the free list pointer. This imporves the efficiency of subsequent allocations and frees.*/
    freep = p;
}

/* We allocate blocks in page sized chunks. */
#define MIN_ALLOC_SIZE 4096 

/*
 Request more memory from the kernel.
 The num_units is the number of header sized units. Returns a pointer to the free list after adding the memory.
 */
static header_t *
morecore(size_t num_units)
{
    void *vp;
    header_t *up;
    /* The intention with this is to make sure an unreasonably large chunk is not requested. This is done by dividing the size of the header, you will see in the next if statement that */
    if (num_units > MIN_ALLOC_SIZE)
        num_units = MIN_ALLOC_SIZE / sizeof(header_t);
    /* Request the memory from the OS using sbrk. If there is a failure, return null.
    Note that modern memory can be allocated in non-continguous pages. mmap is called instead*/
    if ((vp = sbrk(num_units * sizeof(header_t))) == (void *) -1)
        return NULL;
    /* This converts raw memory into a header_t block called up, the size is set to num_units, then it is added to free list.*/
    up = (header_t *) vp;
    up->size = num_units;
    add_to_free_list (up);
    return freep;
}
/* Naive mark and sweep is used here. (1) Scan all blocks of memory that could point to heap data. For each word size chunk we examine, look at each block in the used list. If the word sized chunk value is in range of the used block, mark it; (2) after searching all possible memory locations, go through the used list and add to the free list all blocks that are not marked. These must be the free blocks.*/
/* Some comments, in C, attempts to access any virtual memory is possible. There is no chunk of memory the compiler can access but has an address that cannot be expressed as an integer and then casted to a pointer. This is unusual compared to other languages. Also, all variables are stored somewhere in memory, memory access is typically word aligned, so just need to check every word (be careful about computer architecture).*/
/* Clears the lowest 2 bits of a pointer to be used as mark flags. */
#define UNTAG(p) (((uintptr_t) (p)) & 0xfffffffc)
/* The function scan a region of memory and mark any items in the used list appropriately. Both arguments should be word aligned. The scan starts from sp and end. Each word is a potential pointer to the heap block. Then it marks blocks in the used list that are referenced from this region. */
static void
scan_region(uintptr_t *sp, uintptr_t *end)
{
    header_t *bp;
    /* This code loops over the region and reads each word v. */
    for (; sp < end; sp++) {
        uintptr_t v = *sp;
        /* Check. if v points to a used block.*/
        bp = usedp;
        do {
            /* This if condition checks if v is inside the memory range of this block, note it skips the header bp+1. If yes, set the lowest bit of bp->next to 1 to mark it as reachable, recall that we have cleared the lowest 2 bits of the pointer for this.*/
            if (bp + 1 <= v &&
                bp + 1 + bp->size > v) {
                    bp->next = ((uintptr_t) bp->next) | 1;
                    break;
            }
        /* Use the UNTAG to continue iteration while ignoring the marked bit. */
        } while ((bp = UNTAG(bp->next)) != usedp);
    }
}
/*
 * Scan the marked blocks for references to other unmarked blocks.
 */
static void
scan_heap(void)
{
    uintptr_t *vp;
    header_t *bp, *up;
    /* This looped through the circular used list. UNTAG is used here to ignore the two bits used as mark bits.*/
    for (bp = UNTAG(usedp->next); bp != usedp; bp = UNTAG(bp->next)) {
        /*If the mark bit is not set, skip with continue.*/
        if (!((uintptr_t)bp->next & 1))
            continue;
        /* Can all words in the heap block, reads each word as a potential pointer to another block.*/
        for (vp = (uintptr_t *)(bp + 1);
             vp < (bp + bp->size + 1);
             vp++) {
            uintptr_t v = *vp;
            /* Now check if v points to another block by iterating over the used list up. */
            up = UNTAG(bp->next);
            do {
                /* If v points inside some other block i.e. up is not equal to bp. */
                if (up != bp &&
                    up + 1 <= v &&
                    up + 1 + up->size > v) {
                    /* If yes, set that block marks bit.*/
                    up->next = ((uintptr_t) up->next) | 1;
                    break;
                }
                /* UNTAG is used for safe traversal, ignoring marked bits.*/
            } while ((up = UNTAG(up->next)) != bp);
        }
    }
}
/* Recap on some key points already marked blocks bp can mark blocks they reference up. All reachables blocks will eventually be marked. Recall that bp + 1 skipps the pointer, bp->size counts units, bp + bp->size + 1. This garbage collector is conservative.*/
/*
 * Now we can initialise the garbage collector. The initialisation really is all set up.
 */
void
GC_init(void)
{
    /* First thing is to ensure that the GC_init() runs only once, this is program idempotence. */
    static int initted;
    FILE *statfp;
    if (initted)
        return;
    initted = 1;
    /* This is specific to Linux, opens this to read process info.*/
    statfp = fopen("/proc/self/stat", "r");
    /* Check that there are no problems with the process info.*/
    assert(statfp != NULL);
    /* Now fscan checks that the bottom of the stack is found, this stored globally to know the range of memory. This is essential for root detection in mark and sweep garbage collection. */
    fscanf(statfp,
           "%*d %*s %*c %*d %*d %*d %*d %*d %*u "
           "%*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld "
           "%*ld %*ld %*ld %*ld %*llu %*lu %*ld "
           "%*lu %*lu %*lu %lu", &stack_bottom);
    fclose(statfp);
    /* Initialise the free/used list since no blocks are allocated.*/
    usedp = NULL;
    /* freep points to the dummy block, forming an initial circular free list. Dummy base prevents null pointer checks.*/
    base.next = freep = &base;
    base.size = 0;
}
/* This finally marks blocks of memory in use and free the ones not in use. */
void
GC_collect(void)
{
    header_t *p, *prevp, *tp;
    uintptr_t stack_top;
    /* This data is provided by the linker. */
    extern char end, etext; 
    /* If there is nothing allocated yet, there is nothing to do. Return.*/
    if (usedp == NULL)
        return;
    /* Scan the BSS and initialized data segments. */
    scan_region(&etext, &end);
    /* Scan the stack. BSS and data, global variables may reference heap blocks, stacks are local variables and function frames. Stores the stack top using ebp (x86) in line assembly. This mark roots.*/
    asm volatile ("movl %%ebp, %0" : "=r" (stack_top));
    scan_region(stack_top, stack_bottom);
    /* Mark from the heap. This propagate marks in the heap, see above. This marks reachable heap blocks.*/
    scan_heap();
    /* The main collection loop is here, recall what you learn from untag for marked bits. Walks the circular used list, using the least significant bit of the next pointer as a mark flag, untag ignores the mark for safe traversal.*/
    for (prevp = usedp, p = UNTAG(usedp->next);; prevp = p, p = UNTAG(p->next)) {
    next_chunk:
        if (!((unsigned int)p->next & 1)) {
            /*
             * The chunk hasn't been marked. Thus, it must be set free. That is the point of the garbage collection. p->next & 1 == 0 means it is unreachable and the block is not marked. This is freed via the add_to_free_list().
             */
            tp = p;
            p = UNTAG(p->next);
            add_to_free_list(tp);
            /* If the head of the used list is freed, set the usedp to NULL. Break if all blocks are freed. And then go to the next chunk.*/
            if (usedp == tp) { 
                usedp = NULL;
                break;
            }
            prevp->next = (uintptr_t)p | ((uintptr_t) prevp->next & 1);
            goto next_chunk;
        }
        /* After sweeping clear the marked bits for the remaining live block. If p == used p, this means the circular list is fully traversed. Break out here.*/
        p->next = ((uintptr_t) p->next) & ~1;
        if (p == usedp)
            break;
    }
}