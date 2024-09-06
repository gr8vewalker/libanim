#ifndef LIBANIM_ANIM_H
#define LIBANIM_ANIM_H

#include <stddef.h>

typedef enum animFilterType { SELECTION, TEXT, CHECK } animFilterType;

typedef struct animSelectionFilter {
    size_t size;
    char **elements;
    char **data; // if elements are same with data this should be NULL.
    int value;
} animSelectionFilter;

typedef struct animTextFilter {
    size_t max_size;
    char *value;
} animTextFilter;

typedef struct animCheckFilter {
    int value;
} animCheckFilter;

typedef struct animFilter {
    animFilterType type;
    char *name;
    union {
        animSelectionFilter *selection;
        animTextFilter *text;
        animCheckFilter *check;
    } data;
} animFilter;

struct animExtractor;
struct animSource;
struct animEntry;
struct animPart;
struct animProvider;

typedef struct animSource {
    struct animPart *part;
    struct animExtractor *extractor;
    char *name;
    char *link;
} animSource;

typedef struct animPart {
    struct animEntry *entry;
    char *name;
    char *link;
    size_t sources_size;
    animSource *sources;
} animPart;

typedef struct animEntry {
    struct animProvider *provider;
    char *name;
    char *link;
    size_t parts_size;
    animPart *parts;
} animEntry;

typedef struct animProvider {
    int id;
    char *name;
    void *data; // internal
} animProvider;

/**
 * Initialize library's functionality. Providers and networking is initialized
 * with this.
 * Must called at start of program.
 *
 * @return 0 if success
 */
int anim_initialize();

/**
 * Cleans up networking and frees providers.
 */
void anim_cleanup();

/**
 * List available providers.
 *
 * @param size Pointer to get size of providers
 * @return Providers array.
 */
animProvider *anim_list_providers(size_t *size);

/**
 * Get a specific provider.
 *
 * @param name Name of provider
 * @param exact Exact match flag
 * @return Provider found or NULL
 */
animProvider *anim_get_provider(char *name, int exact);

/**
 * Get filters for a provider.
 *
 * @param provider the Provider
 * @param size Pointer to get size of filters
 * @return newly allocated filters array
 */
animFilter *anim_get_filters(animProvider *provider, size_t *size);

/**
 * Search for entries in a provider.
 *
 * @param provider the Provider
 * @param input Search input
 * @param size Pointer to get size of entries
 * @param entries Array pointer to get entries
 * @return 0 if success
 */
int anim_search(animProvider *provider, const char *input, size_t *size,
                animEntry **entries);

/**
 * Fill details(parts etc.) in an entry.
 *
 * @param provider the Provider
 * @param entry the Entry
 * @return 0 if success
 */
int anim_details(animProvider *provider, animEntry *entry);

/**
 * Fill sources in a part.
 *
 * @param provider the Provider
 * @param part the Part
 * @return 0 if success
 */
int anim_sources(animProvider *provider, animPart *part);

/**
 * Download a source to a file.
 *
 * @param source the Source
 * @param path Path of download file
 * @param tmp Temporary folder
 * @return 0 if success
 */
int anim_download(animSource *source, const char *path, const char *tmp);

/**
 * Stream a source. Used for playback without downloading.
 *
 * @param source the Source
 * @param result String pointer for streaming url/file
 * @param tmp Temporary folder
 * @return 0 if success
 */
int anim_stream(animSource *source, char **result, const char *tmp);

/**
 * Frees entries.
 * Also calls anim_free_parts for each entry.
 *
 * @param entries the Entries
 * @param size Entry count
 */
void anim_free_entries(animEntry *entries, size_t size);

/**
 * Frees parts.
 * Also calls anim_free_sources for each source.
 *
 * @param parts the Parts
 * @param size Part count
 */
void anim_free_parts(animPart *parts, size_t size);

/**
 * Frees sources.
 *
 * @param sources the Sources
 * @param size Source count
 */
void anim_free_sources(animSource *sources, size_t size);

#endif
