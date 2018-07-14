#ifndef SSOLOGIN_CRYPTO_H
#define SSOLOGIN_CRYPTO_H

#include <string>

int rsa_public_encrypt(unsigned char *plaintext,
                        size_t plaintext_len,
                        const char *public_key_file,
                        unsigned char *ciphertext,
                        size_t *ciphertext_len);

int rsa_private_decrypt(unsigned char *ciphertext,
                        size_t ciphertext_len,
                        const char *private_key_file,
                        unsigned char *plaintext,
                        size_t *plaintext_len);

void sha256(const char *string, char output[65]);

int base64_encode(const unsigned char* buffer, size_t length, char** b64text);

int base64_decode(const char* b64message, unsigned char** buffer, size_t* length);

#endif // SSOLOGIN_CRYPTO_H