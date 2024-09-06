#ifndef LIBANIM_EXTRACTOR_H
#define LIBANIM_EXTRACTOR_H

#include "libanim/anim.h"

typedef int (*_animSourceExtract)(animPart *, const char *, const char *);
typedef int (*_animSourceDownload)(const animSource *, const char *,
                                   const char *);
typedef int (*_animSourceStream)(const animSource *, char **, const char *);

typedef struct animExtractor {
    _animSourceExtract extract;
    _animSourceDownload download;
    _animSourceStream stream;
} animExtractor;

#endif
