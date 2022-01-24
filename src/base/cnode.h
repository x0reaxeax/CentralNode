#ifndef _CNODE71_BASE_H_
#define _CNODE71_BASE_H_

#define CNODE_DEBUG                 1

#ifndef __intptr_t_defined
typedef long int intptr_t;
typedef unsigned long int uintptr_t;
#   define __intptr_t_defined
#endif

#include "../sys/logging.h"                                 /* logging */

#ifdef __amd64__
#   include <bits/types/error_t.h>                          /* error_t */
#else
    typedef int error_t;
#endif

#include "../net/networks.h"
#include "../sys/crypto/md5.h"                              /* md5 openssl crypto */
#include "../sys/crypto/uuid.h"                             /* libuuid */
#include "../net/ifs.h"                                     /* ifnet struct */

#include <errno.h>                                          /* errno numbers */

#include <stdlib.h>                                         /* EXIT_FAILURE & EXIT_SUCCESS */
#include <unistd.h>                                         /* stdfd fileno */
#include <stdbool.h>
#include <linux/limits.h>                                   /* PATH_MAX */


#define CNODE_VERSION_MAJOR         7
#define CNODE_VERSION_MINOR         1
#define CNODE_BUILD_VERSION         7111

#define BASE_DECIMAL                10
#define BASE_HEXADECIMAL            16

//#define MD5_LEN_MAX                 32                      /* Maximum MD5sum length */


/* cnode startup arguments/parameters */
#define CNODE_SARGN                 6                       /* Number of all supported cnode startup parameters */   
#define CNODE_SARG_CMPLEN           16                      /* max strlen of startup arguments */

#define CNODE_SARG_CLEANUP          0                       /* calls cnode_cleanup() and exits */
#define CNODE_SARG_PANIC            1                       /* calls cnode_panic() which deletes the presence of cnode from the machine */
#define CNODE_SARG_RESETCFG         2                       /* resets cnode config to defaults */
#define CNODE_SARG_DEBUG            3                       /* force debug loglevel */
#define CNODE_SARG_TESTCLR          4                       /* test ncurses color support */
#define CNODE_SARG_GENUUID          5                       /* Generate new crypto-secure UUID */
#define CNODE_SARG_UNKNOWN          0xf                     /* unknown/invalid argument */

#define CNODE_EXECUTION_CONTINUE    EXIT_SUCCESS            /* Signalizes continuing execution in functions where returning an error leads to conditional shutdowns  */
#define CNODE_EXECUTION_EXIT        0xbeef                  /* Signalizes strict shutdown in functions where returning an error leads to conditional shutdowns */

/*
 * Central Node instance config struct. 
 * The struct is a global variable, used to store various configuration fields.
 * 
 * Comments starting with '[!]' indicates that the variable is deprecated and should be removed/replaced ASAP
 *
 * 
 * TODO_NOTE: properly pad this giant mess.. 
*/
struct cnode_settings {
    /* startup swithces */
    bool check_update;                                  /* UpdateCheck switch */
    bool force_refresh;                                 /* Force Network Refresh switch */
    bool nc_color_test;                                 /* is true when SARG for testing ncurses colors is present. Central Node doesn't start when this is true */
    bool launch_nverify;                                /* NVerify Only switch */
    bool tinc_genpair;                                  /* only generate new keypair and exit after */

    /* log settings */
    bool         log_written;                           /* indicates if log file has been altered */
    bool         logfile_available;                     /* LogFile availability flag */
    bool         log_level_force_set;                   /* Indicates if changing the loglevel according to config is possible */
    char        *log_path;                              /* logfile system path */
    unsigned int log_level;                             /* logging level */
    
    /* runtime switches */
    bool status_critical;                               /* [!] if true, shutdown immediately */
    bool panic;                                         /* Panic switch */
    
    /* update */
    bool update_available;                              /* Update Available Flag */

    /* nodewd */
    int nodewd_status;                                  /* nodeWD status */
    
    /* ncurses gui */
    void    *pmainwin;                                  /* pointer to main window */
    void    *pcncolor;                                  /* [!] pointer to color struct */
    size_t   ifnet_table_offset;                        /* COLS offset for displaying ifnet table, serves to avoid clearing the screen at the location of the table */
    bool     winrunning;

    /* debug */
    uintptr_t       rsp64;                              /* For exit debugging purposes */
    unsigned int    version;                            /* CNODE version */
    
    /* Config options */
    bool  autofix;                                      /* AutoFix switch */
    bool  godmode;                                      /* nodeWD force-godmode status */
    bool  nodewd_required;                              /* nodeWD requirement flag */
    bool  netlogos;                                     /* enable/disable network logos */
    bool  allow_root_execution;                         /* allows non-setuid pure root execution. false by default. config or startup parameters can change this */
    bool  net_nohttps;                                  /* allows unencrypted HTTP comm */
    long  net_timeout;                                  /* cURL timeout */
    char *tincifnet;                                    /* tinc ifnet name */
    char *proxyaddr;                                    /* socks5 proxy address */
    char *netrepo;                                      /* network repository */
    bool  first_time_launch;                            /* first time launch calibration (no cfg) */

    /* network stuff */
    bool                        netrepo_status;         /* True - Network repo available; False - not available */
    char                       *tinc_install_path;      /* custom tincd installation path */
    unsigned int                n_networks;             /* number of remote networks */
    struct sys_ifnets          *pifnets;                /* pointer to net interfaces struct */
    struct node_network_info   *network_info;           /* will replace `char *network_list[NETWORKS_MAX]` */
    //char *network_list[NETWORKS_MAX];                 /* network list */
    
    /* misc */
    bool        is_sys_wsl;                             /* is system Windows Subsystem Linux */
    int         exit_status;                            /* Exit flag for CALL_EXIT() (ERROR/SUCCESS) */
    int         uuid_status;                            /* UUID auth status, AUTH_OK = auth ok, AUTH_FAIL = invalid UUID, AUTH_NULL = no UUID */
    error_t     last_errno;                             /* Last Error Code */
    size_t      longest_cfg_entry;                      /* longest config entry size for dynamically creating menu windows with appropriate sizes */
    size_t      longest_net_entry;                      /* longest network entry size for dynamically creating menu windows with appropriate sizes */
    const char *execname;                               /* pointer to argv[0] */
    char        md5hash[MD5_LEN];                       /* CNODE executable md5sum */
};

extern struct cnode_settings nodecfg;


#define NODE_READLASTERR                                nodecfg.last_errno
#define NODE_NOINFO             ""

                                                        /* Update lasterrno */
#define NODE_SETLASTERR(addr_info, info, node_errno)    { do {\
                                                            nodecfg.rsp64 = addr_info; \
                                                            nodecfg.last_errno = node_errno; \
                                                            nodecfg.exit_status = EXIT_FAILURE; \
                                                            log_write(LOG_ERROR, "%s %s", info, node_strerror(NODE_READLASTERR)); \
                                                        } while(0); }


#define NO_ADDR                 0xc0d3                  /* NODE_SETLASTERR() with NO_ADDR signals no need for debug address info */

/* Central Node notification codes for non-gui informatonal messages */
#define CNODE_NOTIF_OK          0x100
#define CNODE_NOTIF_INFO        0x101
#define CNODE_NOTIF_WARN        0x102
#define CNODE_NOTIF_ERR         0x103


/* 
 * Central Node error numbers.
 * These custom error codes start at 0x700#U
*/

#define ENULLPTR                0x700                   /* Operation on nullpointer attempted */
#define EINTEGRITY              0x701                   /* Integrity check failure */
#define EAUTHFAIL               0x702                   /* UUID auth failure */
#define EUUIDGEN                0x703                   /* libuuid gen failure */
#define EINVCFG                 0x704                   /* Invalid cfg entry */
#define EINVLEN                 0x705                   /* Invalid length ? */
#define EWINSZ                  0x706                   /* Incompatible ncurses window size */
#define EFPREAD                 0x707                   /* generic fp read failure */
#define EFPWRITE                0x708                   /* generic fp write failure */
#define EINITCLR                0x709                   /* Unable to initialize ncurses color */
#define EINITPAIR               0x70A                   /* Unable to initialize ncurses pair */
#define EINITWIN                0x70B                   /* unable to initialize main ncurses window */
#define ENOCOLOR                0x70C                   /* Target doesn't support ncurses colors */
#define ESTARTCLR               0x70D                   /* unable to start_color() */
#define EMENUGEN                0x70E                   /* Generic menu ncurses error */
#define ECLRSAVE                0x70F                   /* Unable to save default color values via color_content() */
#define EINVUSR                 0x710                   /* invalid user for specific operation */
#define EGETENV                 0x711                   /* Unable to retrieve environment variable */
#define EOUTBND                 0x712                   /* Out of bounds */
#define ECURLINIT               0x713                   /* Unable to initialize cURL instance */
#define ECRYPTO                 0x714                   /* Crypto general failure */
#define EUDEPEND                0x715                   /* Generic Unmet dependency */
#define EAUTOLOG                0x716                   /* Auto Even Logger Error */
#define __ELASTERR              0x717                   /* For looping through errors, not an actual error */
//#define ECLRSWAP                0x710                   /* unable to swap default predefined color with custom values */

/* Central Node default LogFile path */
#define CNODE_LOG_PATH          "/var/log/cnode.log"



/* Functions */

/**
 * @brief Prints colored message with severity level formatting.
 * 
 * @param fdno          file descriptor number used for output
 * @param msg_level     severity level
 */
void print_message_format_color(int fdno, int msg_level);

/**
 * @brief Converts error id to a string with information on particular error
 * 
 * @param errid         error id to convert
 * @return *sterr       Str info (NULL on error)
 */
char *node_strerror(error_t errid);

/**
 * @brief Sets Central Node log level.
 * Can be used from startup arguments to set LOGLVL to debug before Central Node evaluates the config.
 * 
 * @param loglvl        log level to set
 * @param force         makes Central Node ignore config value for loglevel
 * @return EXIT_SUCCESS 
 * @return EXIT_FAILURE LogLevel was forcibly set, change request denied
 */
int cnode_setloglvl(int loglvl, bool force);

/**
 * @brief Cleans up Central Node's mess on exit
 * 
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE
 */
int cnode_cleanup(void);

/**
 * @brief Initialize all necessary CNODE settings and evaluate config files.
 * 
 * @param argc                  main's argc
 * @param *argv[]               main's argv
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE
 * @return CNODE_EXECUTION_EXIT - Central Node finished requested startup function and is going to exit normally.
 */
int cnode_init_instance(int argc, const char *argv[]);

#endif