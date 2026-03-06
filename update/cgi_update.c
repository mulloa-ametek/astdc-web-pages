/*
 * cgi_upload.c — C equivalent of the provided Python3 CGI upload script
 *
 * Behavior:
 *  - Only accepts POST
 *  - Expects multipart/form-data with file field named "fw_file"
 *  - Saves uploaded file to UPLOAD_DIR (env) or /tmp/web_uploads
 *  - Destination filename: YYYYMMDD_HHMMSS_<basename(original)>
 *  - Responds with identical status lines/messages as the Python script
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#define FIELD_NAME "fw_file"
#define DEFAULT_UPLOAD_DIR "/tmp/web_uploads"
#define IO_CHUNK 65536

/* -------------------- CGI response helpers -------------------- */

static void respond(const char *status, const char *message) {
    printf("Status: %s\n", status);
    printf("Content-Type: text/plain\n");
    printf("Cache-Control: no-store\n");
    printf("\n");
    printf("%s\n", message);
}

/* -------------------- small utilities -------------------- */

static int mkdir_if_needed(const char *path) {
    if (mkdir(path, 0777) == 0) return 0;
    if (errno == EEXIST) return 0;
    return -1;
}

/* Similar to Python:
 * base = os.path.basename(name or "")
 * if not base: return None
 * return base.replace("\x00","")
 *
 * In C strings, embedded NUL terminates; still, we mimic by copying bytes and skipping '\0'.
 * Also strips directory components by taking last '/' (and '\\' for completeness).
 *
 * Returns malloc'd string or NULL.
 */
static char *safe_name(const char *name) {
    if (!name) return NULL;

    const char *base = name;
    const char *p1 = strrchr(name, '/');
    const char *p2 = strrchr(name, '\\');
    if (p1 && p1 + 1 > base) base = p1 + 1;
    if (p2 && p2 + 1 > base) base = p2 + 1;

    if (*base == '\0') return NULL;

    /* Copy while skipping NUL (mostly theoretical for C strings) */
    size_t len = strlen(base);
    char *out = (char *)malloc(len + 1);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (base[i] != '\0') out[j++] = base[i];
    }
    out[j] = '\0';

    if (out[0] == '\0') {
        free(out);
        return NULL;
    }
    return out;
}

/* Case-insensitive prefix match */
static int istartswith(const char *s, const char *prefix) {
    while (*prefix && *s) {
        if (tolower((unsigned char)*s) != tolower((unsigned char)*prefix))
            return 0;
        s++; prefix++;
    }
    return *prefix == '\0';
}

/* Trim trailing CRLF */
static void rstrip_crlf(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

/* -------------------- Buffered reader (stdin) -------------------- */

typedef struct {
    unsigned char buf[IO_CHUNK];
    size_t pos;
    size_t len;
} Reader;

static size_t reader_fill(Reader *r) {
    r->pos = 0;
    r->len = fread(r->buf, 1, sizeof(r->buf), stdin);
    return r->len;
}

static int reader_getc(Reader *r, unsigned char *out) {
    if (r->pos >= r->len) {
        if (reader_fill(r) == 0) return 0; /* EOF */
    }
    *out = r->buf[r->pos++];
    return 1;
}

/* Read a line ending in '\n' (keeps '\n' in buffer). Returns length, 0 on EOF, -1 on overflow. */
static int reader_readline(Reader *r, char *dst, size_t max) {
    size_t i = 0;
    unsigned char c;
    while (1) {
        if (!reader_getc(r, &c)) {
            if (i == 0) return 0;
            dst[i] = '\0';
            return (int)i;
        }
        if (i + 1 >= max) return -1;
        dst[i++] = (char)c;
        dst[i] = '\0';
        if (c == '\n') return (int)i;
    }
}

/* Read exactly n bytes; returns 1 on success, 0 on EOF/short read */
static int reader_read_exact(Reader *r, unsigned char *dst, size_t n) {
    size_t got = 0;
    while (got < n) {
        if (r->pos >= r->len) {
            if (reader_fill(r) == 0) return 0;
        }
        size_t avail = r->len - r->pos;
        size_t take = (n - got < avail) ? (n - got) : avail;
        memcpy(dst + got, r->buf + r->pos, take);
        r->pos += take;
        got += take;
    }
    return 1;
}

/* -------------------- multipart parsing helpers -------------------- */

/* Find substring (needle) in memory (haystack). Returns index or -1. */
static long memfind(const unsigned char *hay, size_t haylen,
                    const unsigned char *needle, size_t needlelen) {
    if (needlelen == 0 || haylen < needlelen) return -1;
    for (size_t i = 0; i + needlelen <= haylen; i++) {
        if (hay[i] == needle[0] && memcmp(hay + i, needle, needlelen) == 0) {
            return (long)i;
        }
    }
    return -1;
}

/* Extract name="..." and filename="..." from a Content-Disposition header line.
 * Sets *out_name and *out_filename to malloc'd strings (or NULL if absent).
 */
static void parse_content_disposition(const char *line, char **out_name, char **out_filename) {
    *out_name = NULL;
    *out_filename = NULL;

    /* Find name=" */
    const char *p = line;
    while ((p = strstr(p, "name=\"")) != NULL) {
        p += 6;
        const char *q = strchr(p, '"');
        if (!q) break;
        size_t n = (size_t)(q - p);
        char *val = (char *)malloc(n + 1);
        if (!val) break;
        memcpy(val, p, n);
        val[n] = '\0';
        *out_name = val;
        break;
    }

    /* Find filename=" */
    p = line;
    while ((p = strstr(p, "filename=\"")) != NULL) {
        p += 10;
        const char *q = strchr(p, '"');
        if (!q) break;
        size_t n = (size_t)(q - p);
        char *val = (char *)malloc(n + 1);
        if (!val) break;
        memcpy(val, p, n);
        val[n] = '\0';
        *out_filename = val;
        break;
    }
}

/* Stream the current part body until the next boundary delimiter.
 *
 * delimiter pattern is: "\r\n" + boundary
 * where boundary is "--<token>" (including the leading "--")
 *
 * If out_fp is non-NULL, writes body bytes to out_fp and increments *total.
 * If out_fp is NULL, discards body bytes.
 *
 * Leaves the reader positioned immediately AFTER the boundary string (not including the
 * trailing "--" or CRLF that follows boundary line).
 *
 * Returns 0 on success, -1 on error.
 */
static int stream_until_boundary(Reader *r,
                                 const unsigned char *pattern, size_t pattern_len,
                                 FILE *out_fp, unsigned long long *total) {
    /* Keep last (pattern_len - 1) bytes as tail to detect boundary across chunks */
    unsigned char *tail = (unsigned char *)malloc(pattern_len ? (pattern_len - 1) : 0);
    size_t tail_len = 0;

    if (pattern_len < 3) { /* must at least contain "\r\n--" */
        free(tail);
        return -1;
    }

    while (1) {
        /* Ensure buffer has data */
        if (r->pos >= r->len) {
            if (reader_fill(r) == 0) {
                free(tail);
                return -1; /* unexpected EOF */
            }
        }

        const unsigned char *cur = r->buf + r->pos;
        size_t cur_len = r->len - r->pos;

        /* Create a small concat view: tail + current chunk */
        size_t concat_len = tail_len + cur_len;
        unsigned char *concat = (unsigned char *)malloc(concat_len);
        if (!concat) {
            free(tail);
            return -1;
        }
        if (tail_len) memcpy(concat, tail, tail_len);
        memcpy(concat + tail_len, cur, cur_len);

        long idx = memfind(concat, concat_len, pattern, pattern_len);
        if (idx >= 0) {
            /* Write bytes before pattern */
            size_t before = (size_t)idx;

            size_t from_tail = (before < tail_len) ? before : tail_len;
            size_t from_cur  = before > tail_len ? (before - tail_len) : 0;

            if (from_tail && out_fp) {
                if (fwrite(tail, 1, from_tail, out_fp) != from_tail) { free(concat); free(tail); return -1; }
                *total += from_tail;
            }
            if (from_cur && out_fp) {
                if (fwrite(cur, 1, from_cur, out_fp) != from_cur) { free(concat); free(tail); return -1; }
                *total += from_cur;
            }

            /* Consume from reader: bytes up to end of pattern within current buffer */
            /* Bytes of pattern that lie in tail = (tail_len - idx) if idx < tail_len else 0 */
            size_t pattern_in_tail = (before < tail_len) ? (tail_len - before) : 0;
            if (pattern_in_tail > pattern_len) pattern_in_tail = pattern_len;
            size_t pattern_in_cur = pattern_len - pattern_in_tail;

            /* Advance reader.pos by bytes consumed in current:
               - from_cur bytes before pattern + pattern_in_cur bytes of the pattern */
            r->pos += from_cur + pattern_in_cur;

            free(concat);
            free(tail);
            return 0; /* boundary found */
        }

        /* No boundary found. Write/discard safe bytes, keep a tail of pattern_len-1 */
        size_t keep = pattern_len - 1;

        if (concat_len <= keep) {
            /* Keep everything in tail, consume all current */
            if (concat_len > 0) {
                memcpy(tail, concat, concat_len);
                tail_len = concat_len;
            }
            r->pos += cur_len;
            free(concat);
            continue;
        }

        size_t safe = concat_len - keep;

        /* safe bytes split between tail and current */
        size_t safe_from_tail = (safe < tail_len) ? safe : tail_len;
        size_t safe_from_cur  = safe > tail_len ? (safe - tail_len) : 0;

        if (safe_from_tail && out_fp) {
            if (fwrite(tail, 1, safe_from_tail, out_fp) != safe_from_tail) { free(concat); free(tail); return -1; }
            *total += safe_from_tail;
        }
        if (safe_from_cur && out_fp) {
            if (fwrite(cur, 1, safe_from_cur, out_fp) != safe_from_cur) { free(concat); free(tail); return -1; }
            *total += safe_from_cur;
        }

        /* New tail = last keep bytes of concat */
        memcpy(tail, concat + (concat_len - keep), keep);
        tail_len = keep;

        /* Consume all current bytes */
        r->pos += cur_len;

        free(concat);
    }
}

/* -------------------- main CGI logic -------------------- */

int main(void) {
    const char *method = getenv("REQUEST_METHOD");
    if (!method || strcmp(method, "POST") != 0) {
        respond("405 Method Not Allowed", "Use POST.");
        return 0;
    }

    const char *upload_dir = getenv("UPLOAD_DIR");
    if (!upload_dir || upload_dir[0] == '\0') upload_dir = DEFAULT_UPLOAD_DIR;

    const char *ctype = getenv("CONTENT_TYPE");
    if (!ctype) {
        respond("500 Internal Server Error", "Upload failed: Missing CONTENT_TYPE");
        return 0;
    }

    /* Must be multipart/form-data; boundary=... */
    const char *bpos = strstr(ctype, "boundary=");
    if (!bpos) {
        respond("500 Internal Server Error", "Upload failed: Missing multipart boundary");
        return 0;
    }
    bpos += 9;

    /* Boundary token may be quoted */
    char boundary_token[512];
    size_t bi = 0;
    if (*bpos == '"') {
        bpos++;
        while (*bpos && *bpos != '"' && bi + 1 < sizeof(boundary_token)) boundary_token[bi++] = *bpos++;
    } else {
        while (*bpos && *bpos != ';' && !isspace((unsigned char)*bpos) && bi + 1 < sizeof(boundary_token))
            boundary_token[bi++] = *bpos++;
    }
    boundary_token[bi] = '\0';

    if (boundary_token[0] == '\0') {
        respond("500 Internal Server Error", "Upload failed: Empty multipart boundary");
        return 0;
    }

    /* boundary line in body is: --<token> */
    char boundary_line[600];
    snprintf(boundary_line, sizeof(boundary_line), "--%s", boundary_token);

    /* pattern that ends a part body is: \r\n--<token> */
    unsigned char pattern[610];
    size_t pattern_len = 0;
    pattern[pattern_len++] = '\r';
    pattern[pattern_len++] = '\n';
    size_t blen = strlen(boundary_line);
    memcpy(pattern + pattern_len, boundary_line, blen);
    pattern_len += blen;

    Reader r = {0};
    r.pos = r.len = 0;

    /* First line should be boundary_line + CRLF */
    char line[4096];
    int lr = reader_readline(&r, line, sizeof(line));
    if (lr <= 0) {
        respond("500 Internal Server Error", "Upload failed: Empty request body");
        return 0;
    }
    rstrip_crlf(line);

    if (strcmp(line, boundary_line) != 0) {
        respond("500 Internal Server Error", "Upload failed: Malformed multipart body");
        return 0;
    }

    int saw_fw_field = 0;
    int saved_fw_file = 0;
    unsigned long long total = 0ULL;
    char *dest_path_alloc = NULL;

    while (1) {
        /* Read headers until blank line */
        char *part_name = NULL;
        char *part_filename = NULL;

        while (1) {
            lr = reader_readline(&r, line, sizeof(line));
            if (lr == 0) {
                /* EOF */
                break;
            }
            if (lr < 0) {
                respond("500 Internal Server Error", "Upload failed: Header line too long");
                free(part_name); free(part_filename);
                free(dest_path_alloc);
                return 0;
            }

            /* Blank line ends headers */
            if (strcmp(line, "\r\n") == 0 || strcmp(line, "\n") == 0) break;

            /* Parse Content-Disposition only */
            if (istartswith(line, "Content-Disposition:")) {
                char *n = NULL, *f = NULL;
                parse_content_disposition(line, &n, &f);
                if (n) { free(part_name); part_name = n; }
                if (f) { free(part_filename); part_filename = f; }
            }
        }

        if (lr == 0) {
            /* Unexpected EOF */
            respond("500 Internal Server Error", "Upload failed: Unexpected EOF in headers");
            free(part_name); free(part_filename);
            free(dest_path_alloc);
            return 0;
        }

        int is_fw = (part_name && strcmp(part_name, FIELD_NAME) == 0);

        if (is_fw) {
            saw_fw_field = 1;

            if (!part_filename || part_filename[0] == '\0') {
                respond("400 Bad Request", "No file selected.");
                free(part_name); free(part_filename);
                free(dest_path_alloc);
                return 0;
            }

            char *san = safe_name(part_filename);
            if (!san) {
                respond("400 Bad Request", "Invalid filename.");
                free(part_name); free(part_filename);
                free(dest_path_alloc);
                return 0;
            }

            if (mkdir_if_needed(upload_dir) != 0) {
                char msg[512];
                snprintf(msg, sizeof(msg), "Upload failed: %s", strerror(errno));
                respond("500 Internal Server Error", msg);
                free(san);
                free(part_name); free(part_filename);
                free(dest_path_alloc);
                return 0;
            }

            /* timestamp = datetime.now().strftime("%Y%m%d_%H%M%S") */
            time_t now = time(NULL);
            struct tm lt;
#if defined(_WIN32)
            localtime_s(&lt, &now);
#else
            localtime_r(&now, &lt);
#endif
            char ts[32];
            strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", &lt);

            /* dest_path = UPLOAD_DIR + "/" + timestamp + "_" + filename */
            size_t need = strlen(upload_dir) + 1 + strlen(ts) + 1 + strlen(san) + 1;
            dest_path_alloc = (char *)malloc(need);
            if (!dest_path_alloc) {
                respond("500 Internal Server Error", "Upload failed: Out of memory");
                free(san);
                free(part_name); free(part_filename);
                return 0;
            }
            snprintf(dest_path_alloc, need, "%s/%s_%s", upload_dir, ts, san);

            FILE *out = fopen(dest_path_alloc, "wb");
            if (!out) {
                char msg[512];
                snprintf(msg, sizeof(msg), "Upload failed: %s", strerror(errno));
                respond("500 Internal Server Error", msg);
                free(san);
                free(part_name); free(part_filename);
                free(dest_path_alloc);
                return 0;
            }

            /* Stream body to file until boundary */
            if (stream_until_boundary(&r, pattern, pattern_len, out, &total) != 0) {
                fclose(out);
                char msg[512];
                snprintf(msg, sizeof(msg), "Upload failed: %s", "Malformed multipart body");
                respond("500 Internal Server Error", msg);
                free(san);
                free(part_name); free(part_filename);
                free(dest_path_alloc);
                return 0;
            }

            fclose(out);
            saved_fw_file = 1;

            free(san);
        } else {
            /* Discard this part body until boundary */
            unsigned long long dummy_total = 0;
            if (stream_until_boundary(&r, pattern, pattern_len, NULL, &dummy_total) != 0) {
                respond("500 Internal Server Error", "Upload failed: Malformed multipart body");
                free(part_name); free(part_filename);
                free(dest_path_alloc);
                return 0;
            }
        }

        free(part_name);
        free(part_filename);

        /* After boundary string, expect either "--" (final) or "\r\n" (next) */
        unsigned char next2[2];
        if (!reader_read_exact(&r, next2, 2)) {
            respond("500 Internal Server Error", "Upload failed: Unexpected EOF after boundary");
            free(dest_path_alloc);
            return 0;
        }

        if (next2[0] == '-' && next2[1] == '-') {
            /* Final boundary. There may be a trailing CRLF; ignore if present. */
            /* Try to read next line end, but not required for our logic. */
            break;
        } else if (next2[0] == '\r' && next2[1] == '\n') {
            /* Next part starts */
            continue;
        } else if (next2[0] == '\n') {
            /* Some clients might send LF only; treat as next part (best-effort) */
            continue;
        } else {
            respond("500 Internal Server Error", "Upload failed: Malformed boundary terminator");
            free(dest_path_alloc);
            return 0;
        }
    }

    if (!saw_fw_field) {
        respond("400 Bad Request", "Missing file field 'fw_file'.");
        free(dest_path_alloc);
        return 0;
    }

    if (!saved_fw_file) {
        /* If field was present but not saved (should have already returned 400/500) */
        respond("400 Bad Request", "No file selected.");
        free(dest_path_alloc);
        return 0;
    }

    /* respond("200 OK", "Saved {} bytes to {}".format(total, dest_path)) */
    char okmsg[1024];
    snprintf(okmsg, sizeof(okmsg), "Saved %llu bytes to %s",
             (unsigned long long)total, dest_path_alloc ? dest_path_alloc : "");
    respond("200 OK", okmsg);

    free(dest_path_alloc);
    return 0;
}
