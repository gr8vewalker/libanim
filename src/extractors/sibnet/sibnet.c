#include "sibnet.h"
#include "libanim/anim.h"
#include "libanim/html.h"
#include "libanim/net.h"
#include "libanim/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static animExtractor *SIBNET;

int sibnet_extract(animPart *part, const char *link, const char *name) {
    int retval = 0;
    animDocument doc = {NULL, NULL};
    char *body = NULL, *vidsrc = NULL;

    const char *USER_AGENT =
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; rv:128.0) Gecko/20100101 Firefox/128.0";

    if (get(link, &USER_AGENT, 1, &body) != 0 ||
        load_document(body, &doc) != 0) {
        retval = 1;
        goto end;
    }

    vidsrc = xpath_s("substring-before("
                     "substring-after("
                     "substring-after("
                     "substring-after("
                     "//script[contains(., 'player.src')]/."
                     ", 'player.src')"
                     ", 'src:')"
                     ", '\"')"
                     ", '\"')",
                     &doc);
    if (vidsrc[0] == 0) {
        retval = 1;
        goto end;
    }

    part->sources = malloc(sizeof(animSource));
    if (!part->sources) {
        retval = 1;
        goto end;
    }

    part->sources_size = 1;

    part->sources[0].name = strdup(name);
    part->sources[0].extractor = SIBNET;
    part->sources[0].part = part;

    char *format = strncmp("http", vidsrc, 4) == 0 ? "%s" : "https://video.sibnet.ru%s";

    part->sources[0].link = format_string(format, vidsrc);

end:
    free(vidsrc);
    free(body);
    unload_document(&doc);
    return retval;
}

int sibnet_download(const animSource *source, const char *path,
                    const char *tmp) {
    const char *REFERER = "Referer: https://video.sibnet.ru";

    return downloadfile(source->link, &REFERER, 1, path);
}

int sibnet_stream(const animSource *source, char **result, const char *tmp) {
    *result = strdup(source->link);
    return 0;
}

animExtractor *SIBNET_EXTRACTOR() {
    if (!SIBNET) {
        SIBNET = malloc(sizeof(animExtractor));
        SIBNET->extract = sibnet_extract;
        SIBNET->download = sibnet_download;
        SIBNET->stream = sibnet_stream;
    }
    return SIBNET;
}
