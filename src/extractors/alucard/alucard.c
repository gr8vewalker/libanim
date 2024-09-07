#include "alucard.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libanim/anim.h"
#include "libanim/net.h"
#include "libanim/util.h"
#include "src/cjson/cJSON.h"
#include "src/extractors/extractor.h"

static animExtractor *ALUCARD;

int alucard_extract(animPart *part, const char *link, const char *name) {
    int retval = 0;

    cJSON *json = NULL, *response = NULL, *sources = NULL, *source = NULL;
    char *body = NULL, *converted_link = NULL, *file = NULL, *playlist = NULL;

    const char *source_id =
        link + strlen("https://www.turkanime.co/player/");
    const char *source_headers[4] = {
        "X-Requested-With: XMLHttpRequest",
        "Csrf-Token: EqdGHqwZJvydjfbmuYsZeGvBxDxnQXeARRqUNbhRYnPEWqdDnYFEKVBaUPCAGTZA",
        "Cookie: __", "Referer: www.turkanime.co"};

    converted_link =
        format_string("https://www.turkanime.co/sources/%s/true", source_id);

    if (get(converted_link, source_headers, 4, &body) != 0) {
        retval = 1;
        goto end;
    }

    json = cJSON_Parse(body);
    response = cJSON_GetObjectItem(json, "response");
    sources = cJSON_GetObjectItem(response, "sources");
    source = cJSON_GetArrayItem(sources, 0);
    file = cJSON_GetObjectItem(source, "file")->valuestring;

    if (get(file, NULL, 0, &playlist) != 0) {
        retval = 1;
        goto end;
    }

    const char *lf = "\n";
    const char *ext_x_stream_inf = "#EXT-X-STREAM-INF";
    const char *resolution = "RESOLUTION=";
    char *saveptr;
    char *line = strtok_r(playlist, lf, &saveptr);

    animSource *src = NULL;

    while (line) {
        if (strncmp(ext_x_stream_inf, line, strlen(ext_x_stream_inf)) == 0) {
            part->sources = realloc(part->sources, (part->sources_size + 1) *
                                                       sizeof(animSource));
            src = &part->sources[part->sources_size];
            src->part = part;
            src->extractor = ALUCARD;
            char *res =
                strtok(strchr(strstr(line, resolution), 'x') + 1, ",\r\n");
            src->name = format_string("%s %sp", name, res);
        } else if (src) {
            src->link = strdup(line);
            src = NULL;
            part->sources_size++;
        }
        line = strtok_r(0, lf, &saveptr);
    }

end:
    cJSON_Delete(json);
    free(playlist);
    free(converted_link);
    free(body);
    return retval;
}

int alucard_download(const animSource *source, const char *path,
                     const char *tmp) {
    int retval = 0;

    char *playlist = NULL, *download_directory = NULL, *file = NULL,
         *concat_file = NULL, *command = NULL;
    FILE *ffmpeg_concat = NULL;

    if (get(source->link, NULL, 0, &playlist) != 0) {
        retval = -1;
        goto end;
    }

    download_directory = format_string("%s/alucard", tmp);
    if (mkdir(download_directory, 0755) != 0 && errno != EEXIST) {
        retval = -1;
        goto end;
    }

    const char *lf = "\n";
    const char *https = "https://";
    char *line = strtok(playlist, lf);

    size_t index = 0, threaded = 0;

    // Threading... Fucking playlists.
    // TODO: Convert this to curl multi handle
    pthread_t threads[30];
    threaded_download_info informations[30];

    concat_file = format_string("%s/concat.txt", download_directory);
    ffmpeg_concat = fopen(concat_file, "wb");

    while (line) {
        if (strncmp(https, line, strlen(https)) == 0) {
            file = format_string("%s/%zu.mp4", download_directory, index);

            informations[threaded].url = strdup(line);
            informations[threaded].headers = NULL;
            informations[threaded].headers_size = 0;
            informations[threaded].path = strdup(file);
            fprintf(ffmpeg_concat, "file '%zu.mp4'\n", index);
            free(file);

            if (pthread_create(&threads[threaded], NULL, downloadfile_t,
                               &informations[threaded]) != 0) {
                retval = -1;
                goto end;
            }

            threaded++;
            index++;
        }

        line = strtok(0, lf);

        if (threaded == 30 || !line) {
            for (size_t i = 0; i < threaded; i++) {
                pthread_join(threads[i], NULL);
                free(informations[i].url);
                free(informations[i].path);
            }
            threaded = 0;
        }
    }

end:
    if (ffmpeg_concat)
        fclose(ffmpeg_concat);

    if (retval == 0) {
        command =
            format_string("ffmpeg -safe 0 -f concat -i \"%s\" -c copy \"%s\"",
                          concat_file, path);
        system(command);
    }

    free(command);
    free(concat_file);
    free(download_directory);
    free(playlist);
    return retval;
}

int alucard_stream(const animSource *source, char **result, const char *tmp) {
    *result = format_string("%s/%s.m3u8", tmp, source->name);
    return downloadfile(source->link, NULL, 0, *result);
}

animExtractor *ALUCARD_EXTRACTOR() {
    if (!ALUCARD) {
        ALUCARD = malloc(sizeof(animExtractor));
        ALUCARD->extract = alucard_extract;
        ALUCARD->download = alucard_download;
        ALUCARD->stream = alucard_stream;
    }
    return ALUCARD;
}
