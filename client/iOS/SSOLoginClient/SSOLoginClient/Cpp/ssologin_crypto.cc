#include "ssologin_crypto.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>

EVP_PKEY* readRSAKey(const char *keyfile, bool publicKey) {

    FILE *fp = fopen(keyfile, "r");
    if (!fp) {
        return NULL;
    }

    EVP_PKEY *pkey;
    if (publicKey) {
        pkey = PEM_read_PUBKEY(fp, NULL, 0, NULL);
    } else {
        pkey = PEM_read_PrivateKey(fp, NULL, 0, NULL);
    }

    fclose (fp);

    if (pkey == NULL) {
        ERR_print_errors_fp(stderr);
    }

    return pkey;
}

int rsa_public_encrypt(unsigned char *plaintext,
                        size_t plaintext_len,
                        const char *public_key_file,
                        unsigned char *ciphertext,
                        size_t *ciphertext_len) {

    if (ciphertext == NULL) {
        return -1;
    }

    EVP_PKEY* key = readRSAKey(public_key_file, true);

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) {
        return -1;
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        return -1;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        return -1;
    }

    if (EVP_PKEY_encrypt(ctx, NULL, ciphertext_len, plaintext, plaintext_len) <= 0) {
        return -1;
    }

    if (EVP_PKEY_encrypt(ctx, ciphertext, ciphertext_len, plaintext, plaintext_len) <= 0) {
        return -1;
    }

    return 0;
}

int rsa_private_decrypt(unsigned char *ciphertext,
                        size_t ciphertext_len,
                        const char *private_key_file,
                        unsigned char *plaintext,
                        size_t *plaintext_len) {

    if (plaintext == NULL) {
        return -1;
    }

    EVP_PKEY *key = readRSAKey(private_key_file, false);
    
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) {
        return -1;
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        return -1;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        return -1;
    }

    if (EVP_PKEY_decrypt(ctx, NULL, plaintext_len, ciphertext, ciphertext_len) <= 0) {
        return -1;
    }

    if (EVP_PKEY_decrypt(ctx, plaintext, plaintext_len, ciphertext, ciphertext_len) <= 0) {
        return -1;
    }

    return 0;
}

void sha256(const char *string, char output[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = 0;
}


