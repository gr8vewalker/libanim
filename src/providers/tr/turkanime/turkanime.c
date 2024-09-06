#include "turkanime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "libanim/anim.h"
#include "libanim/html.h"
#include "libanim/net.h"
#include "libanim/util.h"
#include "src/cjson/cJSON.h"
#include "src/extractors/alucard/alucard.h"
#include "src/extractors/extractor.h"
#include "src/providers/provider.h"

#define TURKANIME_BASE "https://www.turkanime.co"
#define TURKANIME_ANIME_ENDPOINT "https://www.turkanime.co/anime"
#define TURKANIME_ANIME_ENDPOINT_LEN strlen(TURKANIME_ANIME_ENDPOINT)
#define TURKANIME_SEARCH_ENDPOINT "https://www.turkanime.co/arama"
#define TURKANIME_EPISODES_ENDPOINT_FORMAT                                     \
    "https://www.turkanime.co/ajax/bolumler?animeId=%s"

#define TURKANIME_AES                                                          \
    "710^8A@3@>T2}#zN5xK?kR7KNKb@-A!LzYL5~M1qU0UfdWsZoBm4UUat%}ueUv6E--*"      \
    "hDPPbH7K2bp9^3o41hw,khL:}Kx8080@M"

#define MURMUR_ALUCARD 16231995801545578170ull

static int tr_turkanime_filters(size_t *size, animFilter **filters) {
    *filters = NULL;
    *size = 0;

    // TODO: implement.
    return 0;
}

static int tr_turkanime_search(const char *input, size_t *size,
                               animEntry **entries) {
    if (strncmp(TURKANIME_ANIME_ENDPOINT, input,
                TURKANIME_ANIME_ENDPOINT_LEN) == 0) {
        *entries = malloc(sizeof(animEntry));

        if (!(*entries))
            return -1;

        *size = 1;
        (*entries)->name = format_string("Link: %s", input);
        (*entries)->link = strdup(input);
        return 0;
    }

    int retval = 0;
    char *encoded = NULL, *search_data = NULL, *body = NULL;
    xmlXPathObjectPtr result = NULL;
    animDocument document;

    if (urlenc(input, &encoded) != 0) {
        retval = -1;
        goto end;
    }

    search_data = format_string("arama=%s", encoded);

    if (post(TURKANIME_SEARCH_ENDPOINT, NULL, 0, search_data,
             strlen(search_data), &body) != 0 ||
        load_document(body, &document) != 0) {
        retval = -1;
        goto end;
    }

    char *redirect_location = xpath_s(
        "concat('" TURKANIME_BASE "/', "
        "substring-before(substring-after("
        "//div[@class='panel-body']//script[contains(.,'window.location')]/."
        ", 'window.location = \"'), '\"'))",
        &document);

    if (redirect_location &&
        strncmp(TURKANIME_ANIME_ENDPOINT, redirect_location,
                strlen(TURKANIME_ANIME_ENDPOINT)) == 0) {
        *entries = malloc(sizeof(animEntry));

        if (!(*entries)) {
            retval = -1;
            goto end;
        }

        *size = 1;
        (*entries)->name = format_string("Redirected to %s", redirect_location);
        (*entries)->link = redirect_location;
        goto end;
    }

    result = xpath("//div[@class='col-md-6 col-sm-6 "
                   "col-xs-12']//a[@class='baloon']",
                   &document);
    xmlNodeSetPtr buttons = result->nodesetval;
    size_t count = buttons->nodeNr;

    *entries = calloc(count, sizeof(animEntry));
    if (!(*entries)) {
        retval = -1;
        goto end;
    }

    *size = count;
    for (size_t i = 0; i < count; ++i) {
        xmlNodePtr node = buttons->nodeTab[i];
        (*entries)[i].name =
            xpath_ns("substring-before(@title, ' izle')", node, &document);
        (*entries)[i].link =
            xpath_ns("concat('https:', @href)", node, &document);
    }

end:
    free(encoded);
    free(search_data);
    free(body);
    xmlXPathFreeObject(result);
    unload_document(&document);
    return retval;
}

int tr_turkanime_details(animEntry *entry) {
    if (strncmp(TURKANIME_ANIME_ENDPOINT, entry->link,
                TURKANIME_ANIME_ENDPOINT_LEN) != 0)
        return -1;

    int retval = 0;
    char *body = NULL, *anime_id = NULL, *episodes_url = NULL;
    xmlXPathObjectPtr result = NULL;
    animDocument document;

    if (get(entry->link, NULL, 0, &body) != 0 ||
        load_document(body, &document) != 0) {
        retval = -1;
        goto end;
    }
    free(body);

    free(entry->name); // Remove old allocated name from search
    entry->name =
        xpath_s("//div[@id='detayPaylas']//div[@class='panel']//"
                "div[@class='panel-ust']//div[@class='panel-title']/.",
                &document);

    anime_id = xpath_s("//div[@id='animedetay']//div[@class='oylama']/@data-id",
                       &document);
    episodes_url = format_string(TURKANIME_EPISODES_ENDPOINT_FORMAT, anime_id);

    const char *header = "X-Requested-With: XMLHttpRequest";

    unload_document(&document);

    if (get(episodes_url, &header, 1, &body) != 0 ||
        load_document(body, &document) != 0) {
        retval = -1;
        goto end;
    }

    result = xpath("//div[@id='bolumler']//li", &document);

    xmlNodeSetPtr episodes = result->nodesetval;
    size_t count = episodes->nodeNr;

    entry->parts = calloc(count, sizeof(animPart));

    if (!entry->parts) {
        retval = -1;
        goto end;
    }

    entry->parts_size = count;

    for (size_t i = 0; i < count; ++i) {
        entry->parts[i].entry = entry;

        xmlNodePtr node = episodes->nodeTab[i];
        entry->parts[i].name = xpath_ns("./a[2]/.", node, &document);
        entry->parts[i].link =
            xpath_ns("concat('https:', ./a[2]/@href)", node, &document);
    }

end:
    free(body);
    free(anime_id);
    free(episodes_url);
    xmlXPathFreeObject(result);
    unload_document(&document);
    return retval;
}

int tr_turkanime_host_sources(animPart *part, const char *encrypted,
                              const char *host, const char *fansub) {
    animExtractor *extractor = NULL;

    // Comparison Time!
    switch (murmur64(host)) {
    case MURMUR_ALUCARD:
        extractor = ALUCARD_EXTRACTOR();
        break;
    }

    if (!extractor)
        return 0; // No impl but no error so 0

    unsigned char *crypt_info;
    size_t crypt_info_len;
    base64_decode(encrypted, &crypt_info, &crypt_info_len);

    cJSON *json = cJSON_ParseWithLength((char *)crypt_info, crypt_info_len);
    char *ciphertext = cJSON_GetObjectItem(json, "ct")->valuestring;
    char *salt = cJSON_GetObjectItem(json, "s")->valuestring;

    char *decrypted = decrypt_aes(ciphertext, salt, TURKANIME_AES);
    decrypted[strlen(decrypted) - 1] = 0;
    char *url = format_string("https:%s", decrypted + 1);

    free(decrypted);

    char *name = format_string("%s: %s", fansub, host);
    extractor->extract(part, url, name);

    free(name);
    free(url);
    free(crypt_info);
    cJSON_Delete(json);
    return 0;
}

int tr_turkanime_fansub_sources(animPart *part, const animDocument *document,
                                const char *fansub) {

    {
        char *selected_host = xpath_s(
            "normalize-space(//div[@id='videodetay']//div[@class='btn-group']//"
            "button[contains(@class, 'btn-danger')])",
            document);

        char *encrypted =
            xpath_s("substring-before(substring-after(//iframe[1]/"
                    "@src, '/embed/#/url/'), '?status=0')",
                    document);

        tr_turkanime_host_sources(part, encrypted, selected_host, fansub);

        free(selected_host);
        free(encrypted);
    }

    xmlXPathObjectPtr result = xpath(
        "//div[@id='videodetay']//div[@class='btn-group']//button[contains(@class, 'btn-default')]/.",
        document);

    xmlNodeSetPtr hosts = result->nodesetval;
    size_t hosts_size = hosts->nodeNr;

    animDocument host_document;
    const char *header = "X-Requested-With: XMLHttpRequest";

    char *url;
    char *body;
    char *host_name;
    char *encrypted;

    for (size_t i = 0; i < hosts_size; ++i) {
        xmlNodePtr node = hosts->nodeTab[i];
        url = xpath_ns("concat('" TURKANIME_BASE
                       "/', substring-before(substring-after("
                       "@onclick, "
                       "concat('IndexIcerik(', \"'\")), \"'\"))",
                       node, document);

        if (get(url, &header, 1, &body) != 0 ||
            load_document(body, &host_document) != 0) {
            free(url);
            continue;
        }

        host_name = xpath_s(
            "normalize-space(//div[@id='videodetay']//div[@class='btn-group']//"
            "button[contains(@class, 'btn-danger')])",
            &host_document);

        encrypted = xpath_s("substring-before(substring-after(//iframe[1]/"
                            "@src, '/embed/#/url/'), '?status=')",
                            &host_document);

        tr_turkanime_host_sources(part, encrypted, host_name, fansub);

        free(encrypted);
        free(host_name);
        free(body);
        free(url);
        unload_document(&host_document);
    }

    xmlXPathFreeObject(result);
    return 0;
}

int tr_turkanime_sources(animPart *part) {
    char *body;
    animDocument document;

    if (get(part->link, NULL, 0, &body) != 0 ||
        load_document(body, &document) != 0)
        return -1;

    free(body);

    xmlXPathObjectPtr result = xpath(
        "//div[@id='videodetay']//div[@class='pull-right']//button", &document);

    xmlNodeSetPtr fansubbers = result->nodesetval;
    size_t fansubbers_size = fansubbers->nodeNr;

    part->sources_size = 0;
    part->sources = malloc(sizeof(animSource));

    int retval;

    if (fansubbers_size == 1) {
        xmlNodePtr node = fansubbers->nodeTab[0];
        char *fansub = xpath_ns("normalize-space(.)", node, &document);
        retval = tr_turkanime_fansub_sources(part, &document, fansub);
        free(fansub);
    } else {
        const char *header = "X-Requested-With: XMLHttpRequest";
        animDocument fansub_document;
        for (size_t i = 0; i < fansubbers_size; ++i) {
            xmlNodePtr node = fansubbers->nodeTab[i];

            char *onclick = xpath_ns("concat('" TURKANIME_BASE
                                     "/', substring-before(substring-after("
                                     "@onclick, "
                                     "concat('IndexIcerik(', \"'\")), \"'\"))",
                                     node, &document);

            if (get(onclick, &header, 1, &body) != 0 ||
                load_document(body, &fansub_document) != 0) {
                free(onclick);
                continue;
            }

            char *fansub = xpath_ns("normalize-space(.)", node, &document);

            tr_turkanime_fansub_sources(part, &fansub_document, fansub);

            free(body);
            free(onclick);
            free(fansub);
            unload_document(&fansub_document);
        }

        xmlXPathFreeObject(result);
        retval = 0;
    }

    unload_document(&document);
    return retval;
}

int tr_turkanime_provider(_animProvider *ptr) {
    if (!ptr)
        return 1;

    ptr->id = 0;
    ptr->name = "TurkAnime";
    ptr->data = malloc(sizeof(_animProviderData));
    ptr->data->create_filters = tr_turkanime_filters;
    ptr->data->search = tr_turkanime_search;
    ptr->data->details = tr_turkanime_details;
    ptr->data->sources = tr_turkanime_sources;
    return 0;
}
