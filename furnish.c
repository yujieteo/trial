#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>  // for stat()

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

    // --- Check if .cfg already exists ---
    struct stat st;
    bool cfg_exists = (stat(cfg_file, &st) == 0);

    FILE *in = fopen(input_file, "r");
    FILE *out = fopen(html_file, "w");
    FILE *cfg = cfg_exists ? NULL : fopen(cfg_file, "w");
    if (!in || !out || (!cfg_exists && !cfg)) { perror("File open"); return 1; }

    // fprintf(out, "<html><body>\n");

    int c;
    bool in_comment = false, block_comment = false, in_code = false;
    while ((c = fgetc(in)) != EOF) {
        if (!in_comment) {
            if (c == '/') {
                int next = fgetc(in);
                if (next == '/') {
                    if (in_code) { fprintf(out, "</code></pre>\n"); in_code = false; }
                    in_comment = true; block_comment = false;
                    fprintf(out, "<p>");
                    continue;
                } else if (next == '*') {
                    if (in_code) { fprintf(out, "</code></pre>\n"); in_code = false; }
                    in_comment = true; block_comment = true;
                    fprintf(out, "<p>");
                    continue;
                } else {
                    if (!in_code) { fprintf(out, "<pre><code>"); in_code = true; }
                    fputc(c, out);
                    ungetc(next, in);
                }
            } else {
                if (!in_code) { fprintf(out, "<pre><code>"); in_code = true; }
                if (c == '<') fprintf(out, "&lt;");
                else if (c == '>') fprintf(out, "&gt;");
                else if (c == '&') fprintf(out, "&amp;");
                else fputc(c, out);
            }
        } else {
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

    if (in_code) fprintf(out, "</code></pre>\n");
    if (in_comment) fprintf(out, "</p>\n");
    // fprintf(out, "</body></html>\n");

    if (!cfg_exists) {
        fprintf(cfg, "filename = %s\n", html_base);
        fprintf(cfg, "title = Latest blog post\n");
        fprintf(cfg, "description = Blog post %s\n", html_base);
        fprintf(cfg, "keywords = yujie\n");
        fprintf(cfg, "created = 2025-10-10\n");
        fprintf(cfg, "updated = 2025-10-10\n");
        fclose(cfg);
    }

    fclose(in);
    fclose(out);

    printf("Generated %s%s\n", html_file, cfg_exists ? "" : " and new .cfg");
    return 0;
}
