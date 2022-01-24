#ifndef _CNODE_HELP_DIALOG_H_
#define _CNODE_HELP_DIALOG_H_

#include "../sys/crypto/uuid.h"

/**
 * @brief Prints Central Node's help dialog. This function doesn't require root privileges.
 * 
 */
void print_help(void);

/**
 * @brief Prints freshly generate UUID along with dialog info
 * 
 * @param uuid_str  UUID str buffer
 */
void print_uuid_help(char uuid_str[UUID_STR_LEN]);

#endif