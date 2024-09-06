#include "libanim/util.h"

uint64_t murmur64(const char *key) {
    uint64_t h = 525201411107845655ull;
    for (; *key; ++key) {
        h ^= *key;
        h *= 0x5bd1e9955bd1e995;
        h ^= h >> 47;
    }
    return h;
}
