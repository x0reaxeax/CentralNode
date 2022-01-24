#include "cnode.h"                  /* cnode base */
#include "help.h"                   /* help dialog */

#include "../gui/cursesctl.h"       /* curses control */
#include "../gui/termcolor.h"       /* terminal colors */
#include "../sys/sysctl.h"          /* sysctl */
#include "../sys/config.h"          /* config */
#include "../net/netops.h"          /* network operations */
#include "../net/curlcomm.h"        /* cURL backend comm */

#include <string.h>                 /* strncmp */
#include <stdlib.h>

/* Central Node instance settings struct */
struct cnode_settings nodecfg;

void print_message_format_color(int fdno, int msg_level) {
    char *fmt_color = NULL;

    switch (msg_level) {
        case LOG_DEBUG:
            fmt_color = TC_CLR_NEON_PINK;
            break;

        case LOG_NOTIF:
            fmt_color = TC_CLR_GREEN;
            break;

        case LOG_WARNING:
            fmt_color = TC_CLR_ORANGE;
            break;

        case LOG_ERROR:
            fmt_color = TC_CLR_RED;
            break;

        case LOG_CRITICAL:
            fmt_color = TC_BOLDCLR_RED;
            break;

        /* below are non-gui printf fmt information messages */
        case CNODE_NOTIF_OK:
            fmt_color = TC_CLR_GREEN;
            break;

        case CNODE_NOTIF_INFO:
            fmt_color = TC_CLR_YELLOW;
            break;

        case CNODE_NOTIF_WARN:
            fmt_color = TC_CLR_ORANGE;
            break;

        case CNODE_NOTIF_ERR:
            fmt_color = TC_CLR_RED;
            break;

        default:
            fmt_color = TC_CLR_LIME;
            break;
    }

    char *strloglvl = log_lvl_to_str(msg_level);
    if (strloglvl == NULL) {
        strloglvl = "UNKNOWN";
    }

    FILE *__restrict__ stream = (fdno == STDOUT_FILENO) ? stdout : stderr;
    fprintf(stream, "[%s%s%s] ", fmt_color, strloglvl, TC_CLR_RESET);
}

char *node_strerror(error_t errid) {
    char *node_list_str_error[__ELASTERR - 0x700] = {
    "Attempted to pass NULL pointer",
    "Integrity check failure",
    "UUID authenctication failure",
    "UUID generation failure",
    "Invalid config entry",
    "Invalid length",
    "Terminal window size too small",
    "Generic read failure",
    "Generic write failure",
    "Unable to initialize ncurses color",
    "Unable to initialize ncurses color pair",
    "Unable to initialize ncurses window",
    "No color capabilities",
    "Unable to start ncurses color mode",
    "Unable to initialize ncurses menu",
    "Unable to save default terminal color values",
    "Operation refused for current user",
    "Unable to read environment variable",
    "Out of bounds error",
    "Unable to initialize cURL",
    "Generic crypto failure",
    "Unmet dependencies",
    "Auto Event Logger failure"
};
    
    /* Node errors start at 0x700, so we can subtract that from last error number to get number of errors */
    size_t n_errors = __ELASTERR - 0x700;
    
    if (n_errors != (sizeof(node_list_str_error) / sizeof(node_list_str_error[0]))) {
        NODE_SETLASTERR(CURRENT_ADDR,  "Failed to initialize error logging:", EINVAL);
        log_write_trace("node_strerror()", "n_errors");
        return NULL;
    }

    char *error_str = NULL;

    if (errid < 0x700) {
        /* cerrno */
        error_str = strerror(errid);
    } else {
        /* node errno */
        for (error_t i = 0x700; i < __ELASTERR; i++) {
            if (i == errid) {
                error_str = node_list_str_error[i - 0x700];
                break;
            }
        }
    }

    return error_str;
}

int cnode_cleanup(void) {
    // Kill GUI
    if (nodecfg.pmainwin != NULL) {
        log_write(LOG_DEBUG, "Restoring console window");
        gui_killgui(nodecfg.pmainwin);
    }

    /* free CFG_TUNDEV config string */
    if (nodecfg.tincifnet != NULL) {
        log_write(LOG_DEBUG, "Freeing malloc'd memory");
        free(nodecfg.tincifnet);
    }

    if (nodecfg.netrepo != NULL) {
        free(nodecfg.netrepo);
    }

    /* free network list */
    log_write(LOG_DEBUG, "Freeing network list");
    network_info_final(&nodecfg.network_info);

    /* free curl global instance */
    curl_node_final();

    if (nodecfg.log_written) {
        print_message_format_color(STDOUT_FILENO, LOG_NOTIF);
        puts("Logfile has been updated");
    }

    if (nodecfg.exit_status != EXIT_SUCCESS) {
        print_message_format_color(STDERR_FILENO, LOG_ERROR);
        printf("Central node exited with error code: %d - E%d [%#02x] - '%s'\n", nodecfg.last_errno, nodecfg.last_errno, errno, node_strerror(nodecfg.last_errno));
    }

    if (nodecfg.rsp64 != NO_ADDR) {
        log_write(LOG_DEBUG, "FINAL_TRACE_INFO:\n * 0x%llx\n * E%d\n * S%d\n", nodecfg.rsp64, nodecfg.last_errno, nodecfg.exit_status);
    }

    // Cleanup success
    log_write(LOG_NOTIF, "Cleanup successful");
    
    free(nodecfg.log_path);
    nodecfg.log_path = NULL;

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates string startup parameter and returns it's defined ID constant 
 * 
 * @param *str_sarg  startup argument string to evaluate 
 * @return CNODE_SARG ID constant 
 */
int cnode_eval_sarg(const char *str_sarg) {
    if (strncmp(str_sarg, "--cleanup", CNODE_SARG_CMPLEN) == 0) {
        return CNODE_SARG_CLEANUP; 
    } else if (strncmp(str_sarg, "--panic", CNODE_SARG_CMPLEN) == 0) {
        return CNODE_SARG_PANIC;
    } else if (strncmp(str_sarg, "--resetcfg", CNODE_SARG_CMPLEN) == 0) {
        return CNODE_SARG_RESETCFG;
    } else if (strncmp(str_sarg, "--debug", CNODE_SARG_CMPLEN) == 0) {
        return CNODE_SARG_DEBUG;
    } else if (strncmp(str_sarg, "--testcolor", CNODE_SARG_CMPLEN) == 0) {
        return CNODE_SARG_TESTCLR;
    } else if (strncmp(str_sarg, "--genuuid", CNODE_SARG_CMPLEN) == 0) {
        return CNODE_SARG_GENUUID;
    }
    
    return CNODE_SARG_UNKNOWN;
}

int cnode_setloglvl(int loglvl, bool force) {
    if (force) {
        nodecfg.log_level_force_set = true;
        nodecfg.log_level = loglvl;
    }
    if (!nodecfg.log_level_force_set) {
        nodecfg.log_level = loglvl;
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluates and executes startup arguments and their respective functions
 * 
 * @param argc      main's argc
 * @param argv      main's argv
 * @return CNODE_EXECUTION_CONTINUE     continue central node execution
 * @return CNODE_EXECUTION_EXIT         exits central node
 */
int cnode_parse_sargs(int argc, const char *argv[]) {
    int ret_code = CNODE_EXECUTION_CONTINUE;

    for (int i = 1; i < argc; i++) {
        int sarg_id = cnode_eval_sarg(argv[i]);
        switch (sarg_id) {
            case CNODE_SARG_CLEANUP:
                log_write(LOG_NOTIF, "Performing cleanup");
                
                /* int cret = cnode_cleanup();
                const char *cret_msg = (cret == EXIT_SUCCESS) ? "Cleanup successful" : "Failed to perform cleanup";        
                
                log_write(LOG_NOTIF, "R%d - %s", cret, cret_msg);
                
                int msglvl = (cret == EXIT_SUCCESS) ? CNODE_NOTIF_OK : CNODE_NOTIF_ERR;
                
                print_message_format_color(msglvl);
                puts(cret_msg);*/
                ret_code = CNODE_EXECUTION_EXIT;
                break;

            case CNODE_SARG_PANIC:
                log_write(LOG_NOTIF, "PANIC");
                break;

            case CNODE_SARG_RESETCFG:
                log_write(LOG_NOTIF, "RESETCFG");
                break;

            case CNODE_SARG_DEBUG:
                log_write(LOG_NOTIF, "Setting LogLevel to LOG_DEBUG");
                cnode_setloglvl(LOG_DEBUG, true);
                break;

            case CNODE_SARG_TESTCLR:
                log_write(LOG_NOTIF, "Preparing to test ncurses color compatibility");
                nodecfg.nc_color_test = true;
                break;

            case CNODE_SARG_GENUUID:
                log_write(LOG_DEBUG, "Checking for existing UUID..");
                if (config_read_uuid(NULL) == EXIT_SUCCESS) {
                    log_write(LOG_WARNING, "Attempted to overwrite existing UUID! Aborting..");
                    printf( "[!] An existing UUID file was found!\n"
                            "[!] Attempting to overwrite it may result in irreversible loss!\n"
                            "[!] Central Node will NOT allow this.\n"
                            TC_CLR_RED
                            "[***] If you are absolutely sure about this, manually delete existing UUID file at '%s' and restart Central Node [***]\n"
                            TC_CLR_RESET, CNODE_UUID_PATH
                    );
                } else {
                    log_write(LOG_NOTIF, "Generating new UUID..");
                    char uuid_printout[UUID_STR_LEN] = { 0 };
                    config_create_uuid(uuid_printout);

                    print_uuid_help(uuid_printout);
                }

                
                ret_code = CNODE_EXECUTION_EXIT;
                break;

            default:
                log_write(LOG_NOTIF, "Unknown startup parameter: '%s'", argv[i]);
                break;
        }
    }

    log_write(LOG_DEBUG, "Evaluated all known startup parameters [%d]", argc);

    return ret_code;
}

int cnode_init_instance(int argc, const char *argv[]) {    
    nodecfg.last_errno = EXIT_SUCCESS;
    nodecfg.rsp64 = NO_ADDR;

    /* Set default log level */
    nodecfg.log_level = LOG_NOTIF;

    nodecfg.log_path = NULL;            /* log file path */

    /* Set default values */
    nodecfg.version             = CNODE_BUILD_VERSION;
    nodecfg.first_time_launch   = false;                /* first time launch */
    nodecfg.autofix             = false;                /* autofix system */
    nodecfg.godmode             = false;                /* nodeWD's godmode */
    nodecfg.netlogos            = true;                 /* Display net logos */
    nodecfg.nodewd_required     = false;                /* nodeWD required dependency */
    nodecfg.execname            = argv[0];              /* executable name */
    nodecfg.tinc_install_path   = NULL;                 /* tinc installation path - NOT YET IMPLEMENTED */

    nodecfg.netrepo_status  = false;    /* no repo status yet */
    nodecfg.network_info    = NULL;     /* remote network info */
    nodecfg.proxyaddr       = NULL;     /* socks5 proxy addr */

    nodecfg.longest_net_entry = 0;      /* longest network name */
    nodecfg.longest_cfg_entry = 0;      /* longest config value */

    /* GUI isn't running yet */
    nodecfg.winrunning = false;

    /* The only function that doesn't require root access is printing the help dialog, so an explicit check for "--help" parameter is performed first */
    if (argc > 1 && strncmp(argv[1], "--help", CNODE_SARG_CMPLEN) == 0) {
        print_help();
        /* we don't continue the execution */
        return CNODE_EXECUTION_EXIT;
    } 


    /* First of all, check for r00t */
    if (rootchecker() != EXIT_SUCCESS) {
        // insufficient permission
        NODE_SETLASTERR(NO_ADDR, "Unable to start Central Node:", EPERM);

        /*~ set_log_path("cnode7.log"); */
        print_message_format_color(STDERR_FILENO, LOG_ERROR);
        fprintf(stderr, "Central Node requires root privileges [%d]\n", nodecfg.last_errno);
        return EXIT_FAILURE;
    }

    if (log_set_path(CNODE_LOG_PATH) != EXIT_SUCCESS) {
        print_message_format_color(STDERR_FILENO, LOG_ERROR);
        fprintf(stderr, "Error setting default log path: [E%d : %d]\n", nodecfg.last_errno, errno);
        return EXIT_FAILURE;
    }

    /* if this shows up in the log, this is either a dev-build, or something got really fucked up.. */
    log_write(LOG_DEBUG, "{1337-IMPOSSIBLE-NOTIFICATION} Setting LogFile path to: '%s'", CNODE_LOG_PATH);

    /* Set LogFile availability status (pointless, but historical reasons.. yea right) */
    nodecfg.logfile_available = true;

    log_write(LOG_NOTIF, "Starting Central Node 7 [build %d]", nodecfg.version);

    /* Check and evaluate startup parameters */
    int arg_retcode = cnode_parse_sargs(argc, argv);

    /* If cnode was supposed to perform an action at startup and exit afterwards, CNODE_EXECUTION_EXIT is returned from SARG evaluator */
    if (arg_retcode == CNODE_EXECUTION_EXIT) {
        return CNODE_EXECUTION_EXIT;
    }

    /* Evaluation of config entries begins here.
     * ~~Evaulate config logging level first..~~
    */
    if (config_readcfg() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    /* initialize cURL */
    log_write(LOG_DEBUG, "Initializing cURL instance..");
    if (curl_node_init() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    /* Fetch network list */
    int network_fetch_retval = net_get_networks();
    if (network_fetch_retval < 0) {
        /* fatal error, memory was not allocated or was free'd automatically */
        return EXIT_FAILURE;
    }
    /* otherwise, number of networks is returned */
    nodecfg.n_networks = (unsigned int) network_fetch_retval;
    if (!nodecfg.netrepo_status) {
        log_write(LOG_WARNING, "Remote network repository is unavailable");
    }
    if (nodecfg.netrepo_status && nodecfg.n_networks < 1) {
        log_write(LOG_WARNING, "Remote network repository is empty");
    }

    if (nodecfg.log_level == LOG_DEBUG) {
        log_write(LOG_DEBUG, "Number of networks: %zu", nodecfg.n_networks);

        network_info_st *loop = nodecfg.network_info;
        network_id_t itr = 0;
        while (loop) {
            log_write(LOG_DEBUG, "Network %hu: '%s'", itr, loop->network_name);
            if (loop->network_status == NETWORK_STATUS_ONLINE) {
                itr++;
            }
            loop = loop->next;
        }
    }

    nodecfg.is_sys_wsl = is_wsl();
    if (nodecfg.is_sys_wsl) {
        log_write(LOG_DEBUG, "Windows Subsystem Linux detected");
    }

    return EXIT_SUCCESS;
}