#ifndef _CNODE_SYSCTL_H_
#define _CNODE_SYSCTL_H_

#define UID_ROOT        0

#include "../base/cnode.h"

/* WRITE DOC ON THIS */
long finit_module_wrapper(int fd, const char *uargs, int flags);
long delete_module_wrapper(const char *name_user, unsigned int flags);

/**
 * @brief Detects if system is Windows Subsystem Linux
 * 
 * @return true 
 * @return false 
 */
bool is_wsl(void);

/**
 * @brief Checks for root access
 * 
 * @return EXIT_SUCCESS     - root
 * @return EXIT_FAILURE     - non-root 
 */
int rootchecker(void);

/**
 * @brief Drops superuser privileges gained with sudo.
 * 
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE             - failed to drop root, shut cnode down..
 * @return CNODE_EXECITON_CONTINUE  - If root execution is allowed, dont shut down
 */
int drop_superuser(void);

/**
 * @brief Checks if file exists on system
 * 
 * @param filename  - path to file to check
 * @return true     - file exists
 * @return false    - file doesn't exist or an error has occured. check errno or nodecfg.last_errno
 */
bool file_exists(char *filename);

/**
 * @brief Checks if directory exists on system
 * 
 * @param dirpath   - path to directory to check
 * @return true     - directory exists
 * @return false    - folder doesn't exist or an error has occured
 */
bool dir_exists(char *dirpath);

#endif