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

int base64_encode(const unsigned char* buffer, size_t length, char** b64text) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    *b64text=(*bufferPtr).data;

    return 0;
}

size_t calc_decode_length(const char* b64input) {
    size_t len = strlen(b64input),
        padding = 0;

    if (b64input[len-1] == '=' && b64input[len-2] == '=')
        padding = 2;
    else if (b64input[len-1] == '=')
        padding = 1;

    return (len*3)/4 - padding;
}

int base64_decode(const char* b64message, unsigned char** buffer, size_t* length) {
    BIO *bio, *b64;

    int decodeLen = calc_decode_length(b64message);
    *buffer = (unsigned char*)malloc(decodeLen + 1);
    (*buffer)[decodeLen] = '\0';

    bio = BIO_new_mem_buf(b64message, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    *length = BIO_read(bio, *buffer, strlen(b64message));
    assert(*length == decodeLen);
    BIO_free_all(bio);

    return 0;
}


