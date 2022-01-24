#include "../../base/cnode.h"
#include "../sysctl.h"          /* finit_module_wrapper() */
#include "wdctl.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int nodewd_load(const char *kopath) {
    if (NULL == kopath) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("nodewd_load()", "kopath");
        return EXIT_FAILURE;
    }

    int kofd = open(kopath, O_RDONLY);
    if (kofd < 0) {
        NODE_SETLASTERR(CURRENT_ADDR, "open() failed:", errno);
        log_write_trace("nodewd_load()", "kofd");
        return EXIT_FAILURE;
    }

    int load_ret = finit_module_wrapper(kofd, NULL, 0);
    close(kofd);
    
    if (EXIT_SUCCESS != load_ret) {
        log_write(LOG_ERROR, "Failed to load nodeWD");
        return EXIT_FAILURE;
    }

    log_write(LOG_NOTIF, "Successfully loaded nodeWD");

    return EXIT_SUCCESS;
}

char *nodewd_get_strstatus(void) {
    switch (nodecfg.nodewd_status) {
        case NODEWD_STATUS_INACTIVE:
            return "Connect";
            break;

        case NODEWD_STATUS_RUNNING:
            return "Disconnect";
            break;

        case NODEWD_STATUS_ERROR:
            return "Retry Connection";
            break;

        default:
            return "NODEWD - ERROR";
            break;
    }

    return "NODEWD - ERROR";
}