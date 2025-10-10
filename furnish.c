#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>

/* Dynamic buffer for code blocks */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} CodeBuffer;

int ensure_capacity(CodeBuffer *buf, size_t needed) {
    if (buf->cap >= needed) return 0;
    size_t newcap = (buf->cap == 0) ? 512 : buf->cap * 2;
    while (newcap < needed) newcap *= 2;
    char *p = realloc(buf->data, newcap);
    if (!p) return -1;
    buf->data = p;
    buf->cap = newcap;
    return 0;
}

int append_str(CodeBuffer *buf, const char *s) {
    size_t sl = strlen(s);
    if (ensure_capacity(buf, buf->len + sl + 1)) return -1;
    memcpy(buf->data + buf->len, s, sl);
    buf->len += sl;
    buf->data[buf->len] = '\0';
    return 0;
}

int append_char_escaped(CodeBuffer *buf, int ch) {
    if (ch == '<') return append_str(buf, "&lt;");
    if (ch == '>') return append_str(buf, "&gt;");
    if (ch == '&') return append_str(buf, "&amp;");
    if (ensure_capacity(buf, buf->len + 2)) return -1;
    buf->data[buf->len++] = (char)ch;
    buf->data[buf->len] = '\0';
    return 0;
}

bool has_non_whitespace(const CodeBuffer *buf) {
    for (size_t i = 0; i < buf->len; ++i) {
        char c = buf->data[i];
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') return true;
    }
    return false;
}

void start_code(CodeBuffer *buf) {
    buf->len = 0;
    if (buf->data) buf->data[0] = '\0';
}

void flush_code(FILE *out, CodeBuffer *buf) {
    if (buf->len > 0 && has_non_whitespace(buf)) {
        fprintf(out, "<pre><code>");
        fwrite(buf->data, 1, buf->len, out);
        fprintf(out, "</code></pre>\n");
    }
    buf->len = 0;
    if (buf->data) buf->data[0] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s input.c output.html\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    const char *html_file = argv[2];

    char html_dir[256];
    strncpy(html_dir, html_file, sizeof(html_dir));
    char *slash = strrchr(html_dir, '/');
    if (slash) *slash = '\0';
    else strcpy(html_dir, ".");

    char html_base[256];
    strncpy(html_base, basename((char *)html_file), sizeof(html_base));

    char cfg_name[256];
    strncpy(cfg_name, html_base, sizeof(cfg_name));
    char *dot = strrchr(cfg_name, '.');
    if (dot) *dot = '\0';

    char cfg_file[256];
    snprintf(cfg_file, sizeof(cfg_file), "%s/%s.cfg", html_dir, cfg_name);

    struct stat st;
    bool cfg_exists = (stat(cfg_file, &st) == 0);

    FILE *in = fopen(input_file, "r");
    FILE *out = fopen(html_file, "w");
    FILE *cfg = cfg_exists ? NULL : fopen(cfg_file, "w");
    if (!in || !out || (!cfg_exists && !cfg)) { perror("File open"); return 1; }

    int c;
    bool in_comment = false, block_comment = false, in_code = false;
    CodeBuffer codebuf = {NULL, 0, 0};

    while ((c = fgetc(in)) != EOF) {
        if (!in_comment) {
            if (c == '/') {
                int next = fgetc(in);
                if (next == '/') {
                    flush_code(out, &codebuf);
                    in_comment = true; block_comment = false;
                    fprintf(out, "<p>");
                    continue;
                } else if (next == '*') {
                    flush_code(out, &codebuf);
                    in_comment = true; block_comment = true;
                    fprintf(out, "<p>");
                    continue;
                } else {
                    if (!in_code) { start_code(&codebuf); in_code = true; }
                    append_char_escaped(&codebuf, c);
                    ungetc(next, in);
                }
            } else {
                if (!in_code) { start_code(&codebuf); in_code = true; }
                append_char_escaped(&codebuf, c);
            }
        } else { /* in_comment */
            if (block_comment) {
                if (c == '*') {
                    int next = fgetc(in);
                    if (next == '/') { fprintf(out, "</p>\n"); in_comment = false; continue; }
                    else { fputc(c, out); ungetc(next, in); }
                } else fputc(c, out);
            } else {
                if (c == '\n') { fprintf(out, "</p>\n"); in_comment = false; }
                else fputc(c, out);
            }
        }
    }

    flush_code(out, &codebuf);
    if (in_comment) fprintf(out, "</p>\n");

    if (!cfg_exists) {
        fprintf(cfg, "filename = %s\n", html_base);
        fprintf(cfg, "title = Latest blog post\n");
        fprintf(cfg, "description = Blog post for %s\n", html_base);
        fprintf(cfg, "keywords = yujie\n");
        fprintf(cfg, "created = 2025-10-10\n");
        fprintf(cfg, "updated = 2025-10-10\n");
        fclose(cfg);
    }

    fclose(in);
    fclose(out);
    free(codebuf.data);

    printf("Generated %s%s\n", html_file, cfg_exists ? "" : " and new .cfg");
    return 0;
}
