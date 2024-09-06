#include "libanim/util.h"

#include <string.h>

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>

size_t _decode_len(const char *encoded) {
    size_t len = strlen(encoded), padding = 0;

    if (encoded[len - 1] == '=' && encoded[len - 2] == '=')
        padding = 2;
    else if (encoded[len - 1] == '=')
        padding = 1;

    return (len * 3) / 4 - padding;
}

int base64_decode(const char *encoded, unsigned char **buffer, size_t *length) {
    BIO *bio, *b64;

    size_t decode_len = _decode_len(encoded);
    *buffer = (unsigned char *)malloc(decode_len + 1);
    (*buffer)[decode_len] = '\0';

    bio = BIO_new_mem_buf(encoded, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    *length = BIO_read(bio, *buffer, strlen(encoded));
    if (*length != decode_len)
        return -1;
    BIO_free_all(bio);

    return 0;
}
