#ifndef LIBANIM_PROVIDER_H
#define LIBANIM_PROVIDER_H

#include <stdlib.h>

#include "libanim/anim.h"

typedef int (*_animSearch)(const char *, size_t *, animEntry **);
typedef int (*_animDetails)(animEntry *);
typedef int (*_animSources)(animPart *);

typedef struct _animProviderData {
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
