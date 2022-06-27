#include "../../base/cnode.h"       /* cnode base */

#include <openssl/md5.h>

#include <stdio.h>
#include <string.h>

/* MD2_Init(), MD2_Update(), MD2_Final(), MD4_Init(), MD4_Update(), MD4_Final(), MD5_Init(), MD5_Update(),
 * and MD5_Final() return 1 for success, 0 otherwise. 
*/
#define OPENSSL_SUCCESS 1
#define OPENSSL_FAILURE 0

int crypto_md5hash_file(const char *filepath, unsigned char *hashout) {
    if (filepath == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("crypto_md5hash_file()", "filepath");
        return EXIT_FAILURE;
    }

    int return_code = EXIT_SUCCESS;
    size_t crypto_bytes = 0;
    unsigned char stream_buffer[BUFSIZ] = { 0 };            /* FP read output */
    unsigned char md5_digest[MD5_DIGEST_LENGTH] = { 0 };    /* digest output bytes */

    FILE *fp = fopen(filepath, "rb");
    
    if (fp == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, errno);
        log_write(LOG_ERROR, "Unable to open file '%s' for binary reading: E%d", filepath, nodecfg.last_errno);
        log_write_trace("crypto_md5hash_file()", "filepath");
        return EXIT_FAILURE;
    }

    log_write(LOG_DEBUG, "Hashing file input - '%s'", filepath);

    MD5_CTX ctx;

    if (MD5_Init(&ctx) != OPENSSL_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to initialize MD5 CTX:", ECRYPTO);
        log_write_trace("crypto_md5hash_file()", "MD5_Init() => ctx");
        
        return_code = EXIT_FAILURE;
        goto CRYPTO_END_ROUTINE;
    }

    /* read BUFSIZ bytes, byte by byte from fp stream, until bytes read == 0 */
    do {
        crypto_bytes = fread(stream_buffer, 1, BUFSIZ, fp);
        if (MD5_Update(&ctx, stream_buffer, crypto_bytes) != OPENSSL_SUCCESS) {
            NODE_SETLASTERR(CURRENT_ADDR, "Unable to hash stream input:", ECRYPTO);
            log_write_trace("crypto_md5hash_file()", "MD5_Update() => ctx");
            
            return_code = EXIT_FAILURE;
            
            /* This will probably leak memory, since it won't free Init'd ctx */
            goto CRYPTO_END_ROUTINE;
        }
    } while (crypto_bytes > 0);


    if (MD5_Final(md5_digest, &ctx) != OPENSSL_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to finalize MD5 hash:", ECRYPTO);
        log_write_trace("crypto_md5hash_file()", "MD5_Final() => ctx");

        return_code = EXIT_FAILURE;
        goto CRYPTO_END_ROUTINE;
    }

    /* Convert bytes to hex str output */
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        if (hashout + ((i * 2) + 1) < hashout + MD5_DIGEST_LENGTH) {
            snprintf((char *) hashout + (i * 2), 4, "%02x", md5_digest[i]);
        } else {
            NODE_SETLASTERR(CURRENT_ADDR, "Attempted to write out of bounds:", EOUTBND);
            log_write_trace("crypto_md5hash_file()", "hashout");

            return_code = EXIT_FAILURE;
            goto CRYPTO_END_ROUTINE;
            break;
        }
    }

    /* last sanity check */
    size_t hashout_len = strlen((char *) hashout);
    if (hashout_len != MD5_LEN) {
        NODE_SETLASTERR(CURRENT_ADDR, "MD5 hash integrity check failed:", EINTEGRITY);
        log_write(LOG_DEBUG, "Crypto mismatch: hashout_len [%zu] != MD5_LEN", hashout_len);
        log_write_trace("crypto_md5hash_file()", "hashout_len");

        return_code = EXIT_FAILURE;
    }


CRYPTO_END_ROUTINE:
    fclose(fp);
    return return_code;
}

int crypto_md5hash_strinput(const char *strinput, unsigned char *hashout) {
    if (strinput == NULL || hashout == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("crypto_md5hash_strinput()", "strinput || hashout");
        return EXIT_FAILURE;
    }
    
    log_write(LOG_DEBUG, "Hashing string input - '%s'", strinput);

    MD5_CTX ctx;
    unsigned char md5_digest[MD5_DIGEST_LENGTH];

    if (MD5_Init(&ctx) != OPENSSL_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to initialize MD5 CTX:", ECRYPTO);
        log_write_trace("crypto_md5hash_strinput()", "MD5_Init() => ctx");
        
        return EXIT_FAILURE;
    }

    if (MD5_Update(&ctx, strinput, strlen(strinput)) != OPENSSL_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to hash stream input:", ECRYPTO);
        log_write_trace("crypto_md5hash_strinput()", "MD5_Update() => ctx");

        /* This will probably leak memory, since it won't free Init'd ctx */
        return EXIT_FAILURE;
    }

    if (MD5_Final(md5_digest, &ctx) != OPENSSL_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to finalize MD5 hash:", ECRYPTO);
        log_write_trace("crypto_md5hash_file()", "MD5_Final() => ctx");

        return EXIT_FAILURE;
    }

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        snprintf((char *) hashout + (i * 2), 4, "%02x", md5_digest[i]);
    }

    return EXIT_SUCCESS;
}