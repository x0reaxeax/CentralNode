#include "../../base/cnode.h"   /* cnode */
#include "../sysctl.h"          /* file exists, dir_exists */
#include "tincdctl.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct tinc_conf {
    char *cfg_addrfamily;       /* tinc.conf address family */
    char *cfg_destination;      /* tinc.conf destination node name */

    char *f_tincroot;           /* tinc root dir */
    char *f_nodedir;            /* tinc CNODE dir */
    char *f_hostsdir;           /* tinc CNODE hosts dir */

    char *key_private;          /* tinc private key */
    char *key_public;           /* tinc public key */

    char *cmd_genkeys;          /* tinc daemon command - generate 4096bit keypair */

    char *cnode_pubkey_chunk;   /* central node tinc pubkey chunk */
};

int tinc_check_installation(const char *tincdpath) {
    const char *execpath = NULL;
    if (tincdpath != NULL) {
        execpath = tincdpath;
    } else {
        execpath = "/usr/sbin/tincd";
    }

    if (!file_exists((char *) execpath)) {
        if (errno == ENOENT) {
            NODE_SETLASTERR(NO_ADDR, NODE_NOINFO, EUDEPEND);
            log_write(LOG_ERROR, "No tinc installation found");
            return EXIT_FAILURE;  
        }

        NODE_SETLASTERR(CURRENT_ADDR, "An error has occures while checking for tincd installation:", errno);
        log_write_trace("tinc_check_installation()", "access()");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}