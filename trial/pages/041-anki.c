#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#define OUTFILE "main.tsv"
#define DECK_NAME "main"

/* The buffer must be large enough for paragraphs. */
#define BUF_SIZE 65536 

// Check if filename ends with .html
int is_html(const char *filename) {
    size_t len = strlen(filename);
    return len > 5 && strcmp(filename + len - 5, ".html") == 0;
}

// Trim leading/trailing whitespace
char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

// Process a single HTML file
void process_html_file(const char *filepath, FILE *out) {
    FILE *f = fopen(filepath, "r");
    if (!f) { perror(filepath); return; }

    char buffer[BUF_SIZE];
    size_t buf_len = 0;

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        size_t l = strlen(line);
        /* Reset the buffer if it is too large. */
        if (buf_len + l >= BUF_SIZE) buf_len = 0; 
        memcpy(buffer + buf_len, line, l);
        buf_len += l;
        buffer[buf_len] = '\0';

        char *p_start;
        while ((p_start = strstr(buffer, "<p>")) != NULL) {
            char *p_end = strstr(p_start, "</p>");
            /* The paragraph is not complete yet in this case.*/
            if (!p_end) break; // paragraph not complete yet

            *p_end = '\0';
            /* Skip the ending characters <p> */
            char *text = p_start + 3;

            /* Find the first colon, this is how we will generate the Anki deck.*/
            char *sep = strchr(text, ':');
            if (sep) {
                *sep = '\0';
                char *front = trim(text);
                char *back = trim(sep + 1);
                fprintf(out, "%s\t%s\n", front, back);
            }

            /* Remove the processed paragraph from the buffer.*/
            size_t remaining = buf_len - (p_end + 4 - buffer);
            memmove(buffer, p_end + 4, remaining);
            buf_len = remaining;
            buffer[buf_len] = '\0';
        }
    }

    fclose(f);
}

int main(int argc, char *argv[]) {
    const char *dirpath = ".";
    if (argc >= 2) dirpath = argv[1];

    DIR *d = opendir(dirpath);
    if (!d) { perror("opendir"); return 1; }

    FILE *out = fopen(OUTFILE, "w");
    if (!out) { perror(OUTFILE); closedir(d); return 1; }

    /* Write the Anki header.*/
    fprintf(out, "#separator:tab\n#html:true\n#notetype:Basic\n#deck:%s\n", DECK_NAME);

    struct dirent *entry;
    char filepath[1024];

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type != DT_REG) continue;
        if (!is_html(entry->d_name)) continue;

        snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);
        process_html_file(filepath, out);
    }

    fclose(out);
    closedir(d);

    printf("Created %s ready for Anki import.\n", OUTFILE);
    return 0;
}
