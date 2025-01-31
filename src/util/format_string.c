#include "libanim/util.h"
#include <stdarg.h>

#define __STDC_WANT_LIB_EXT2__ 1
#define _GNU_SOURCE
#include <stdio.h>

char *format_string(const char *format, ...) {
    char *str;
    va_list ap;
    va_start(ap, format);
    vasprintf(&str, format, ap);
    va_end(ap);
    return str;
}
