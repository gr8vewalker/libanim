#include "libanim/libanimv.h"

const char *libanim_version() {
#ifdef LIBANIM_VERSION
    return LIBANIM_VERSION;
#else
    return "source";
#endif
}

const char *libanim_build_date() { return __DATE__ " " __TIME__; }
