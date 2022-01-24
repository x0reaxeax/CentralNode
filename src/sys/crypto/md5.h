#ifndef _CNODE_CRYPTO_MD5_H_
#define _CNODE_CRYPTO_MD5_H_

/* Maximum MD5sum length */
#define MD5_LEN 32

/**
 * @brief Calculates MD5 sum of file
 * 
 * @param filepath  path to file to hash
 * @param hashout   output buffer ( minimum length of MD5_LEN )
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE 
 */
int crypto_md5hash_file(const char *filepath, unsigned char *hashout);

#endif