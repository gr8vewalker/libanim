#include "libanim/util.h"

unsigned char hex_char(char c) {
    if ('0' <= c && c <= '9')
        return c - '0';
    if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
    return 0xFF;
}

int hex_to_bin(const char *hex, unsigned char *buff, int length) {
    int result;
    if (!hex || !buff || length <= 0)
        return -1;

    for (result = 0; *hex; ++result) {
        unsigned char msn = hex_char(*hex++);
        if (msn == 0xFF)
            return -1;
        unsigned char lsn = hex_char(*hex++);
        if (lsn == 0xFF)
            return -1;
        unsigned char bin = (msn << 4) + lsn;

        if (length-- <= 0)
            return -1;
        *buff++ = bin;
    }
    return result;
}
