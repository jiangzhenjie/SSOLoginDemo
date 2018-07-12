#ifndef SSOLOGIN_CRYPTO_H
#define SSOLOGIN_CRYPTO_H

#include <string>

namespace ssologin_crypto {

    const int AES_BLOCK_SIZE = 16;  // 128 bits
    const int AES_KEY_SIZE = 32;    // 256 bits
    const int AES_IV_SIZE = AES_BLOCK_SIZE;

    int aes_encrypt(unsigned char *plaintext,
                    int plaintext_len,
                    unsigned char *key,
                    unsigned char *iv,
                    unsigned char *ciphertext,
                    int *ciphertext_len);

    int aes_decrypt(unsigned char *ciphertext,
                    int ciphertext_len,
                    unsigned char *key,
                    unsigned char *iv,
                    unsigned char *plaintext,
                    int *plaintext_len);

}

#endif // SSOLOGIN_CRYPTO_H