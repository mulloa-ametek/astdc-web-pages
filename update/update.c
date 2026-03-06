#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>


#define FIELD_NAME       "fw_file"
#define CHUNK_SIZE       8192

/* Default upload directory (change if desired) */
#define DEFAULT_SAVE_DIR "/home/ametek/matt/web_pages/transferred_files"

/* ---------------- Utility: always valid CGI responses ---------------- */

static void cgi_text(const char *msg)
{
    /* Must print headers first for CGI */
    printf("Content-Type: text/plain\r\n\r\n%s", msg ? msg : "");
    fflush(stdout);
}

/* Read and discard remaining stdin (important if we error out early) */
static void drain_stdin(long remaining)
{
    char buf[CHUNK_SIZE];
    while (remaining > 0) {
        size_t to_read = (remaining > (long)sizeof(buf)) ? sizeof(buf) : (size_t)remaining;
        size_t n = fread(buf, 1, to_read, stdin);
        if (n == 0) break;
        remaining -= (long)n;
    }
}

/* mkdir -p for a single directory (no recursion needed for your case) */
static int ensure_dir_exists(const char *dir)
{
    struct stat st;
    if (stat(dir, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return 0;
        return -1;
    }
    if (mkdir(dir, 0755) == 0) return 0;
    if (errno == EEXIST) return 0;
    return -1;
}

/* Extract boundary=... from Content-Type */
static int parse_boundary(const char *content_type, char *boundary_out, size_t out_sz)
{
    const char *p = strstr(content_type, "boundary=");
    if (!p) return -1;
    p += 9;

    /* boundary may be quoted */
    if (*p == '"') {
        p++;
        const char *q = strchr(p, '"');
        if (!q) return -1;
        size_t len = (size_t)(q - p);
        if (len + 1 > out_sz) return -1;
        memcpy(boundary_out, p, len);
        boundary_out[len] = '\0';
        return 0;
    } else {
        /* up to ; or end */
        const char *q = strchr(p, ';');
        size_t len = q ? (size_t)(q - p) : strlen(p);
        while (len > 0 && isspace((unsigned char)p[len - 1])) len--;
        if (len + 1 > out_sz) return -1;
        memcpy(boundary_out, p, len);
        boundary_out[len] = '\0';
        return 0;
    }
}

/* Make filename safe: keep only basename, strip path, replace weird chars */
static void sanitize_filename(const char *in, char *out, size_t out_sz)
{
    const char *base = in;
    const char *s;

    /* handle both / and \ */
    s = strrchr(in, '/');
    if (s && s[1]) base = s + 1;
    s = strrchr(base, '\\');
    if (s && s[1]) base = s + 1;

    /* copy and replace unsafe chars */
    size_t j = 0;
    for (size_t i = 0; base[i] && j + 1 < out_sz; i++) {
        unsigned char c = (unsigned char)base[i];
        if (isalnum(c) || c == '.' || c == '_' || c == '-' ) {
            out[j++] = (char)c;
        } else {
            out[j++] = '_';
        }
    }
    out[j] = '\0';

    if (out[0] == '\0') {
        strncpy(out, "upload.bin", out_sz - 1);
        out[out_sz - 1] = '\0';
    }
}

/* Read a CRLF-terminated line from stdin (for multipart headers). Returns:
 *  1 if line read, 0 on EOF. The line includes trailing \n if present.
 */
static int read_line(FILE *fp, char *buf, size_t bufsz)
{
    if (!fgets(buf, (int)bufsz, fp)) return 0;
    return 1;
}

/* Trim CRLF from line end */
static void rstrip_crlf(char *line)
{
    size_t n = strlen(line);
    while (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) {
        line[n-1] = '\0';
        n--;
    }
}

/* Parse Content-Disposition line for name= and filename=
 * Example:
 * Content-Disposition: form-data; name="fw_file"; filename="abc.uImage"
 */
static void parse_content_disposition(const char *line, char *name_out, size_t name_sz,
                                      char *filename_out, size_t fn_sz)
{
    name_out[0] = '\0';
    filename_out[0] = '\0';

    const char *p;

    p = strstr(line, "name=");
    if (p) {
        p += 5;
        if (*p == '"') {
            p++;
            const char *q = strchr(p, '"');
            if (q) {
                size_t len = (size_t)(q - p);
                if (len >= name_sz) len = name_sz - 1;
                memcpy(name_out, p, len);
                name_out[len] = '\0';
            }
        } else {
            const char *q = strpbrk(p, ";\r\n");
            size_t len = q ? (size_t)(q - p) : strlen(p);
            if (len >= name_sz) len = name_sz - 1;
            memcpy(name_out, p, len);
            name_out[len] = '\0';
        }
    }

    p = strstr(line, "filename=");
    if (p) {
        p += 9;
        if (*p == '"') {
            p++;
            const char *q = strchr(p, '"');
            if (q) {
                size_t len = (size_t)(q - p);
                if (len >= fn_sz) len = fn_sz - 1;
                memcpy(filename_out, p, len);
                filename_out[len] = '\0';
            }
        } else {
            const char *q = strpbrk(p, ";\r\n");
            size_t len = q ? (size_t)(q - p) : strlen(p);
            if (len >= fn_sz) len = fn_sz - 1;
            memcpy(filename_out, p, len);
            filename_out[len] = '\0';
        }
    }
}

/* Stream part body to file until we hit the next boundary.
 * Looks for delimiter: "\r\n--<boundary>"
 * Returns bytes written, or -1 on error.
 */
static long stream_part_to_file(FILE *out, const char *boundary, long remaining)
{
    /* delimiter includes CRLF then boundary */
    char delim[256];
    snprintf(delim, sizeof(delim), "\r\n--%s", boundary);
    const size_t delim_len = strlen(delim);

    unsigned char buf[CHUNK_SIZE];
    unsigned char carry[512]; /* must be >= delim_len + some slack; boundary usually small */
    size_t carry_len = 0;

    long written = 0;

    while (remaining > 0) {
        size_t to_read = (remaining > (long)sizeof(buf)) ? sizeof(buf) : (size_t)remaining;
        size_t n = fread(buf, 1, to_read, stdin);
        if (n == 0) break;
        remaining -= (long)n;

        /* build a work buffer = carry + buf */
        size_t work_len = carry_len + n;
        unsigned char *work = (unsigned char*)malloc(work_len);
        if (!work) return -1;
        memcpy(work, carry, carry_len);
        memcpy(work + carry_len, buf, n);

        /* search for delimiter in work */
        size_t i;
        int found = 0;
        size_t pos = 0;
        for (i = 0; i + delim_len <= work_len; i++) {
            if (memcmp(work + i, delim, delim_len) == 0) {
                found = 1;
                pos = i; /* start of delimiter */
                break;
            }
        }

        if (!found) {
            /* write everything except a tail we keep for boundary detection */
            size_t keep = delim_len + 8;
            if (keep > sizeof(carry)) keep = sizeof(carry);

            if (work_len > keep) {
                size_t wlen = work_len - keep;
                if (fwrite(work, 1, wlen, out) != wlen) {
                    free(work);
                    return -1;
                }
                written += (long)wlen;

                /* save tail into carry */
                carry_len = keep;
                memcpy(carry, work + wlen, carry_len);
            } else {
                /* keep all */
                carry_len = work_len;
                if (carry_len > sizeof(carry)) carry_len = sizeof(carry);
                memcpy(carry, work + (work_len - carry_len), carry_len);
            }
            free(work);
            continue;
        }

        /* found delimiter: write bytes before delimiter (file data) */
        if (pos > 0) {
            if (fwrite(work, 1, pos, out) != pos) {
                free(work);
                return -1;
            }
            written += (long)pos;
        }

        /* We found boundary start inside this buffer.
         * We have already consumed part data, but we also consumed boundary bytes
         * from stdin into work. We must "unread" the bytes after delimiter start
         * for the next header parsing phase.
         *
         * Easiest approach in CGI: we can't push back arbitrary bytes reliably,
         * so we stop here and discard the remainder by tracking what we already read.
         *
         * Practically: we don't need to parse subsequent parts; we just saved the file.
         * So we can ignore rest of request body (already in work after pos).
         */
        free(work);

        /* Now drain the rest of stdin (remaining bytes) quickly so browser doesn't reset */
        drain_stdin(remaining);
        return written;
    }

    /* If we get here without finding boundary, write whatever is in carry */
    if (carry_len > 0) {
        if (fwrite(carry, 1, carry_len, out) != carry_len) return -1;
        written += (long)carry_len;
    }

    return written;
}

static void serve_upload_page(void)
{
    /* Minimal HTML so GET /cgi-bin/update.cgi is usable */
    // printf("Content-Type: text/html\r\n\r\n");
    // printf("<!doctype html><html><head><meta charset='utf-8'>"
    //        "<title>Firmware Upload</title>"
    //        "<style>"
    //        "body{font-family:sans-serif;margin:2rem}"
    //        "#progress{width:400px;height:14px;background:#eee;border-radius:6px;overflow:hidden}"
    //        "#progress_bar{height:100%%;width:0%%;background:#3b82f6}"
    //        "</style>"
    //        "</head><body>"
    //        "<h2>Upload firmware</h2>"
    //        "<input id='fw_file' type='file' />"
    //        "<button id='upload_btn'>Upload</button>"
    //        "<div id='progress' style='margin-top:1rem'><div id='progress_bar'></div></div>"
    //        "<div id='status' style='margin-top:1rem;white-space:pre-wrap'></div>"
    //        "<pre id='debug_log' style='margin-top:1rem;height:180px;overflow:auto;background:#111;color:#0f0;padding:8px;'></pre>"
    //        "<script src='/update.js?v=1'></script>"
    //        "</body></html>");
    printf("Content-Type: text/plain\r\n\r\n");
	printf("Done");
}

/* ---------------- main ---------------- */

int main(void)
{
	system("echo debug log >> weblog.txt");

    const char *method = getenv("REQUEST_METHOD");
    if (!method) method = "";

    if (strcmp(method, "POST") != 0) {
        serve_upload_page();
        return 0;
    }

    const char *cl = getenv("CONTENT_LENGTH");
    const char *ct = getenv("CONTENT_TYPE");

    long content_length = 0;
    if (cl && *cl) {
        content_length = strtol(cl, NULL, 10);
        if (content_length < 0) content_length = 0;
    }

    if (!ct || !*ct) {
        drain_stdin(content_length);
        cgi_text("ERR missing_content_type\n");
        return 0;
    }

    /* Determine save directory */
    const char *save_dir = getenv("TRANSFER_DIR");
    if (!save_dir || !*save_dir) save_dir = DEFAULT_SAVE_DIR;

    if (ensure_dir_exists(save_dir) != 0) {
        drain_stdin(content_length);
        cgi_text("ERR cannot_create_transfer_dir\n");
        return 0;
    }

    /* Only multipart/form-data supported (matches your update.js FormData) */
    if (strstr(ct, "multipart/form-data") == NULL) {
        drain_stdin(content_length);
        cgi_text("ERR expected_multipart_form_data\n");
        return 0;
    }

    char boundary[200];
    if (parse_boundary(ct, boundary, sizeof(boundary)) != 0) {
        drain_stdin(content_length);
        cgi_text("ERR missing_boundary\n");
        return 0;
    }

    /* Read initial boundary line */
    char line[1024];
    if (!read_line(stdin, line, sizeof(line))) {
        cgi_text("ERR empty_request\n");
        return 0;
    }
    rstrip_crlf(line);

    char expected0[256];
    snprintf(expected0, sizeof(expected0), "--%s", boundary);

    if (strcmp(line, expected0) != 0) {
        /* Some clients might send a leading CRLF, tolerate by searching */
        /* Drain and error */
        drain_stdin(content_length);
        cgi_text("ERR bad_boundary_start\n");
        return 0;
    }

    /* Read part headers */
    char part_name[128];
    char raw_filename[512];
    part_name[0] = '\0';
    raw_filename[0] = '\0';

    while (read_line(stdin, line, sizeof(line))) {
        /* subtracting from content_length is hard with stdio buffering;
           we rely on boundary detection and final drain for robustness */
        if (strcmp(line, "\r\n") == 0 || strcmp(line, "\n") == 0) break;

        if (strncasecmp(line, "Content-Disposition:", 20) == 0) {
            parse_content_disposition(line, part_name, sizeof(part_name),
                                      raw_filename, sizeof(raw_filename));
        }
        /* You could parse Content-Type of part here if desired */
    }

    if (strcmp(part_name, FIELD_NAME) != 0) {
        /* Not the expected field; drain remaining body and return error */
        drain_stdin(content_length);
        cgi_text("ERR missing_fw_file_field\n");
        return 0;
    }

	system("echo \"debug log\" > /tmp/weblog");

    char safe_name[256];
    sanitize_filename(raw_filename[0] ? raw_filename : "upload.uImage",
                      safe_name, sizeof(safe_name));

    char out_path[1024];
    snprintf(out_path, sizeof(out_path), "%s/%s", save_dir, safe_name);

    FILE *out = fopen(out_path, "wb");
    if (!out) {
        drain_stdin(content_length);
        cgi_text("ERR cannot_open_output_file\n");
        return 0;
    }

    long bytes_written = stream_part_to_file(out, boundary, content_length);
    fclose(out);

    if (bytes_written < 0) {
        unlink(out_path);
        cgi_text("ERR write_failed\n");
        return 0;
    }

    char ok[1200];
    snprintf(ok, sizeof(ok), "OK saved %s (%ld bytes)\n", out_path, bytes_written);
    cgi_text(ok);
    return 0;
}
