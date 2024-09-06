#include "libanim/util.h"

#include <string.h>

#include <openssl/crypto.h>
#include <openssl/evp.h>

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    if (!(ctx = EVP_CIPHER_CTX_new()))
        return -1;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        return -1;

    if (1 !=
        EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        return -1;
    plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        return -1;
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

int key_and_iv(char *salttext, unsigned char *key, unsigned char *iv,
               char *password) {
    unsigned char hexsalt[8];
    hex_to_bin(salttext, hexsalt, strlen(salttext));

    return EVP_BytesToKey(EVP_aes_256_cbc(), EVP_md5(), hexsalt,
                          (unsigned char *)password, strlen(password), 1, key,
                          iv);
}

char *decrypt_aes(char *ciphertext, char *salttext, char *password) {
    unsigned char *ciphertext_decoded;
    size_t ciphertext_len;
    if (base64_decode(ciphertext, &ciphertext_decoded, &ciphertext_len) != 0)
        return NULL;

    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];
    if (!key_and_iv(salttext, key, iv, password)) {
        return NULL;
    }

    unsigned char *plain = malloc(sizeof(unsigned char) * 512);
    int len = decrypt(ciphertext_decoded, ciphertext_len, key, iv, plain);
    plain[len] = 0;

    free(ciphertext_decoded);
    return (char *)plain;
}
