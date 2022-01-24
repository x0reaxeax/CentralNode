#ifndef _CNODE_SECURE_UUIDGEN_H_
#define _CNODE_SECURE_UUIDGEN_H_

#include <uuid/uuid.h>

/**
 * @brief Generates cryptographically secure UUID
 * 
 * @param uuidstr   Allocated output buffer with size of UUID_STR_LEN (37)
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE 
 */
int crypto_gen_safe_uuid(char *uuidstr);

#endif