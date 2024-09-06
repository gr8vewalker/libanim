#ifndef LIBANIM_PROVIDER_H
#define LIBANIM_PROVIDER_H

#include <stdlib.h>

#include "libanim/anim.h"

// Useful macros for filter definition
#define ANIM_FILTER_SELECTION(PTR, INDEX, NAME, ELEMENT_COUNT,                 \
                              IS_DATA_DIFFERENT)                               \
    {                                                                          \
        PTR[INDEX]->type = SELECTION;                                          \
        PTR[INDEX]->name = NAME;                                               \
        PTR[INDEX]->data.selection = malloc(sizeof(animSelectionFilter));      \
        PTR[INDEX]->data.selection->size = 0;                                  \
        PTR[INDEX]->data.selection->elements =                                 \
            calloc(ELEMENT_COUNT, sizeof(char *));                             \
        PTR[INDEX]->data.selection->data =                                     \
            IS_DATA_DIFFERENT ? calloc(ELEMENT_COUNT, sizeof(char *)) : NULL;  \
    }

#define ANIM_FILTER_SELECTION_ENTRY(PTR, INDEX, ELEMENT, DATA)                 \
    {                                                                          \
        if (DATA)                                                              \
            PTR[INDEX]                                                         \
                ->data.selection->data[PTR[INDEX]->data.selection->size] =     \
                DATA;                                                          \
        PTR[INDEX]                                                             \
            ->data.selection->elements[PTR[INDEX]->data.selection->size] =     \
            ELEMENT;                                                           \
        PTR[INDEX]->data.selection->size++;                                    \
    }

#define ANIM_FILTER_SELECTION_ELEMENT(PTR, INDEX, ELEMENT)                     \
    ANIM_FILTER_SELECTION_ENTRY(PTR, INDEX, ELEMENT, NULL)

#define ANIM_FILTER_TEXT(PTR, INDEX, NAME, MAX_SIZE)                           \
    {                                                                          \
        PTR[INDEX]->type = TEXT;                                               \
        PTR[INDEX]->name = NAME;                                               \
        PTR[INDEX]->data.text = malloc(sizeof(animTextFilter));                \
        PTR[INDEX]->data.text->max_size = MAX_SIZE;                            \
    }

#define ANIM_FILTER_CHECK(PTR, INDEX, NAME)                                    \
    {                                                                          \
        PTR[INDEX]->type = CHECK;                                              \
        PTR[INDEX]->name = NAME;                                               \
        PTR[INDEX]->data.text = malloc(sizeof(animCheckFilter));               \
    }

// Internal part
typedef int (*_animCreateFilters)(size_t *, animFilter **);
typedef int (*_animSearch)(const char *, size_t *, animEntry **);
typedef int (*_animDetails)(animEntry *);
typedef int (*_animSources)(animPart *);

typedef struct _animProviderData {
    _animCreateFilters create_filters;
    _animSearch search;
    _animDetails details;
    _animSources sources;
} _animProviderData;

typedef struct _animProvider {
    int id;
    char *name;
    _animProviderData *data;
} _animProvider;

void provider_free(_animProvider *ptr);

#endif
