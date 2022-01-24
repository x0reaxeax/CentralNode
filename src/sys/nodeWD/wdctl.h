#ifndef _CNODE_WATCHDOG_CTL_H_
#define _CNODE_WATCHDOG_CTL_H_

enum NODEWD_STATUS {
    NODEWD_STATUS_INACTIVE  = 0,
    NODEWD_STATUS_RUNNING   = 1,
    NODEWD_STATUS_ERROR     = 2
};

/**
 * @brief Checks nodeWD status and returns it in human readable, menu-entry string
 * Used for nodeWD CTL menu
 * @return nodeWD status string
 */
char *nodewd_get_strstatus(void);

#endif