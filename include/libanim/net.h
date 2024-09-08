#ifndef LIBANIM_NET_H
#define LIBANIM_NET_H

#include <stddef.h>

/**
 * Initializes networking(libcurl).
 *
 * @return 0 if success
 */
int net_init();

/**
 * Cleans up networking(libcurl).
 */
void net_cleanup();

/**
 * Sends a HTTP GET request.
 *
 * @param url URL to request
 * @param headers Request headers
 * @param headers_size Header count
 * @param response String pointer to get response
 * @return 0 if success
 */
int get(const char *url, const char **headers, int headers_size,
        char **response);

/**
 * Sends a HTTP POST request.
 *
 * @param url URL to request
 * @param headers Request headers
 * @param headers_size Header count
 * @param request Request body
 * @param request_len Request body length
 * @param response String pointer to get response
 * @return 0 if success
 */
int post(const char *url, const char **headers, int headers_size,
         const char *request, size_t request_len, char **response);

/**
 * Writes response of a request to a file.
 *
 * @param url URL to download
 * @param headers Request headers
 * @param headers_size Header count
 * @param path File path to download
 * @return 0 if success
 */
int downloadfile(const char *url, const char **headers, int headers_size,
                 const char *path);

typedef struct parallel_download_info {
    char *url;
    const char **headers;
    int headers_size;
    char *path;
} parallel_download_info;

/**
 * Concurrent download function.
 *
 * @param informations Download informations
 * @param size Informations size
 * @return 0 if success
 */
int downloadfile_concurrent(parallel_download_info *informations, size_t size);

/**
 * URL encodes given data.
 *
 * @param data the Data
 * @param encoded String pointer to get encoded data
 * @return 0 if success
 */
int urlenc(const char *data, char **encoded);

#endif
