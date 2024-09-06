#ifndef LIBANIM_UTIL_H
#define LIBANIM_UTIL_H

// Some utility headers
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Calculates murmur hash value for a string.
 *
 * @param key the String
 * @return Murmur hash value
 */
uint64_t murmur64(const char *key);

/**
 * Creates a string with formatting like printf.
 *
 * @param fmt Format string
 * @return Formatted string
 */
char *format_string(const char *fmt, ...);

/**
 * Decodes a base64 string.
 *
 * @param encoded Encoded base64 string
 * @param buffer String pointer to get base64 decoded data
 * @param length Pointer to get length of decoded data
 * @return 0 if success
 */
int base64_decode(const char *encoded, unsigned char **buffer, size_t *length);

/**
 * Converts hex string to binary.
 *
 * @param hex Hex string
 * @param buff Byte array to store binary
 * @param length Length of buff
 * @return 0 if success
 */
int hex_to_bin(const char *hex, unsigned char *buff, int length);

/**
 * Decrypts AES string.
 *
 * @param ct Ciphertext
 * @param salt Used salt while encryption
 * @param pass Passphrase
 * @return Decrypted result
 */
char *decrypt_aes(char *ct, char *salt, char *pass);

#endif
