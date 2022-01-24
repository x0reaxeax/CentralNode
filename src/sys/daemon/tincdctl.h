#ifndef _CNODE_TINCCTL_H_
#define _CNODE_TINCCTL_H_

/**
 * @brief Determines if tinc daemon is present on system
 * 
 * @param tincdpath         custom tincd executable location. default location is used if `NULL`
 *
 * @return EXIT_SUCCESS     tincd is present on system
 * @return EXIT_FAILURE     tincd is either not present, or an error has occured while checking
 */
int tinc_check_installation(const char *tincdpath);

#endif