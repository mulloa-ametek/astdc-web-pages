#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <sys/stat.h>

#include <errno.h>



#define FIELD_NAME "fw_file"

#define UPLOAD_DIR "/tmp/web_uploads"

#define OUT_FILE   "/tmp/web_uploads/upload.bin"

#define BUF_SIZE   4096



static void respond(const char *status, const char *message)

{

    printf("Status: %s\r\n", status);

    printf("Content-Type: text/plain\r\n");

    printf("Cache-Control: no-store\r\n");

    printf("\r\n");

    printf("%s\n", message);

}



static int ensure_dir(const char *path)

{

    struct stat st;

    if (stat(path, &st) == 0) {

        return S_ISDIR(st.st_mode) ? 0 : -1;

    }

    if (mkdir(path, 0777) == 0) {

        return 0;

    }

    return -1;

}



static char *get_boundary(const char *content_type)

{

    const char *p;

    char *boundary;

    size_t len;



    if (!content_type) {

        return NULL;

    }



    p = strstr(content_type, "boundary=");

    if (!p) {

        return NULL;

    }



    p += 9;

    len = strlen(p);



    boundary = (char *)malloc(len + 1);

    if (!boundary) {

        return NULL;

    }



    strcpy(boundary, p);

    return boundary;

}



int main(void)

{

    const char *method = getenv("REQUEST_METHOD");

    const char *content_type = getenv("CONTENT_TYPE");

    const char *content_length_str = getenv("CONTENT_LENGTH");

    long content_length;

    char *boundary = NULL;

    char *body = NULL;

    char *part = NULL;

    char *data_start = NULL;

    char *data_end = NULL;

    FILE *out = NULL;

    size_t written;



    if (!method || strcmp(method, "POST") != 0) {

        respond("405 Method Not Allowed", "Use POST.");

        return 0;

    }



    if (!content_length_str) {

        respond("400 Bad Request", "Missing CONTENT_LENGTH.");

        return 0;

    }



    content_length = strtol(content_length_str, NULL, 10);

    if (content_length <= 0) {

        respond("400 Bad Request", "Invalid CONTENT_LENGTH.");

        return 0;

    }



    boundary = get_boundary(content_type);

    if (!boundary) {

        respond("400 Bad Request", "Missing multipart boundary.");

        return 0;

    }



    body = (char *)malloc((size_t)content_length + 1);

    if (!body) {

        free(boundary);

        respond("500 Internal Server Error", "Out of memory.");

        return 0;

    }



    if (fread(body, 1, (size_t)content_length, stdin) != (size_t)content_length) {

        free(boundary);

        free(body);

        respond("400 Bad Request", "Failed to read request body.");

        return 0;

    }

    body[content_length] = '\0';



    part = strstr(body, "name=\"fw_file\"");

    if (!part) {

        free(boundary);

        free(body);

        respond("400 Bad Request", "Missing file field fw_file.");

        return 0;

    }



    data_start = strstr(part, "\r\n\r\n");

    if (!data_start) {

        free(boundary);

        free(body);

        respond("400 Bad Request", "Malformed multipart body.");

        return 0;

    }

    data_start += 4;



    {

        char boundary_marker[512];

        snprintf(boundary_marker, sizeof(boundary_marker), "\r\n--%s", boundary);

        data_end = strstr(data_start, boundary_marker);

    }



    if (!data_end) {

        free(boundary);

        free(body);

        respond("400 Bad Request", "Could not find end of uploaded file.");

        return 0;

    }



    if (ensure_dir(UPLOAD_DIR) != 0) {

        free(boundary);

        free(body);

        respond("500 Internal Server Error", "Could not create upload directory.");

        return 0;

    }



    out = fopen(OUT_FILE, "wb");

    if (!out) {

        free(boundary);

        free(body);

        respond("500 Internal Server Error", "Could not open output file.");

        return 0;

    }



    written = fwrite(data_start, 1, (size_t)(data_end - data_start), out);

    fclose(out);



    free(boundary);

    free(body);



    {

        char msg[256];

        snprintf(msg, sizeof(msg), "Saved %lu bytes to %s",

                 (unsigned long)written, OUT_FILE);

        respond("200 OK", msg);

    }



    return 0;

}
