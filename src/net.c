#include "libanim/net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

typedef struct {
    int type;
    void *buffer;
    size_t size;
} curl_writer;

static size_t curl_write(void *contents, size_t size, size_t nmemb,
                         void *userp) {
    size_t realsize = size * nmemb;
    curl_writer *writer = (curl_writer *)userp;

    if (writer->type == 0) {
        char *ptr = realloc(writer->buffer, writer->size + realsize + 1);
        if (!ptr)
            return CURL_WRITEFUNC_ERROR;

        writer->buffer = ptr;
        memcpy(&(((char *)writer->buffer)[writer->size]), contents, realsize);
        writer->size += realsize;
        ((char *)writer->buffer)[writer->size] = 0;

        return realsize;
    } else if (writer->type == 1) {
        size_t written = fwrite(contents, size, nmemb, (FILE *)writer->buffer);
        return written;
    }

    return CURL_WRITEFUNC_ERROR;
}

int handle_curl(const char *url, const char **headers, int headers_size,
                const char *request, size_t request_len, void *response,
                int type) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl)
        return -1;

    struct curl_slist *header_slist = NULL;
    for (int i = 0; i < headers_size; i++) {
        header_slist = curl_slist_append(header_slist, headers[i]);
    }

    curl_writer resp_writer;
    resp_writer.type = type;
    if (type == 0) {
        resp_writer.buffer = malloc(1);
        if (resp_writer.buffer == NULL)
            return -1;
    } else if (type == 1) {
        resp_writer.buffer = response; // FILE *
    }
    resp_writer.size = 0;

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_slist);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    if (request_len > 0) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request_len);
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp_writer);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    res = curl_easy_perform(curl);

    curl_slist_free_all(header_slist);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK) {
        if (type == 0)
            *((char **)response) = resp_writer.buffer;
    }

    return res;
}

int net_init() { return curl_global_init(CURL_GLOBAL_ALL); }

void net_cleanup() { curl_global_cleanup(); }

int get(const char *url, const char **headers, int headers_size,
        char **response) {
    return handle_curl(url, headers, headers_size, NULL, 0, response, 0);
}

int post(const char *url, const char **headers, int headers_size,
         const char *request, size_t request_len, char **response) {
    return handle_curl(url, headers, headers_size, request, request_len,
                       response, 0);
}

int downloadfile(const char *url, const char **headers, int headers_size,
                 const char *path) {
    FILE *file = fopen(path, "wb");
    int res = handle_curl(url, headers, headers_size, NULL, 0, file, 1);
    fclose(file);
    return res;
}

int downloadfile_concurrent(parallel_download_info *informations, size_t size) {
    CURL *handles[size];
    FILE *files[size];
    struct curl_slist *header_slists[size];
    curl_writer writers[size];
    CURLM *multi_handle;

    int running = 1;

    multi_handle = curl_multi_init();
    if (!multi_handle)
        return 1;

    for (size_t i = 0; i < size; i++) {
        handles[i] = curl_easy_init();
        parallel_download_info information = informations[i];

        header_slists[i] = NULL;
        for (size_t j = 0; j < information.headers_size; j++) {
            header_slists[i] =
                curl_slist_append(header_slists[i], information.headers[j]);
        }

        files[i] = fopen(information.path, "wb");

        writers[i].type = 1;
        writers[i].buffer = files[i];
        writers[i].size = 0;

        CURL *curl = handles[i];
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_slists[i]);
        curl_easy_setopt(curl, CURLOPT_URL, information.url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writers[i]);
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

        curl_multi_add_handle(multi_handle, handles[i]);
    }

    while (running) {
        CURLMcode mc = curl_multi_perform(multi_handle, &running);

        if (running)
            mc = curl_multi_poll(multi_handle, NULL, 0, 1000, NULL);

        if (mc)
            break;
    }

    // TODO: add failed download handling by curl msgs

    for (size_t i = 0; i < size; i++) {
        fclose(files[i]);
        curl_slist_free_all(header_slists[i]);
        curl_multi_remove_handle(multi_handle, handles[i]);
        curl_easy_cleanup(handles[i]);
    }

    curl_multi_cleanup(multi_handle);
    return 0;
}

int urlenc(const char *data, char **encoded) {
    CURL *curl;

    curl = curl_easy_init();

    if (!curl)
        return -1;

    *encoded = curl_easy_escape(curl, data, strlen(data));
    if (*encoded == NULL)
        return -1;

    curl_easy_cleanup(curl);

    return 0;
}
