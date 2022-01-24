#ifndef _CNODE_CONFIG_H_
#define _CNODE_CONFIG_H_

#include <stdint.h>                                 /* uint64_t */

#define CFG_ARG_MAX         127                     /* max cfg entry strlen */

/* Config entries */
#define CFG_AUTOFIX         0                       /* autofix system */
#define CFG_GODMODE         1                       /* nodeWD godmode assist */
#define CFG_NWDMODE         2                       /* nodeWD enforcing mode */
#define CFG_NETLOGO         3                       /* network logos */
#define CFG_ALLOWSU         4                       /* allows raw, non-setuid (sudo, gksudo,..) root execution */
#define CFG_NOHTTPS         5                       /* allows HTTP unencrypted comm */
#define CFG_NLOGLVL         6                       /* logging level */
#define CFG_TIMEOUT         7                       /* net timeout */
#define CFG_TUNDEV          8                       /* tun interface string */
#define CFG_PROXY           9                       /* socks5 proxy */
#define CFG_NETREPO         10                      /* network repo string */
#define CFG_VERSION         11                      /* unused */
#define CFG_NULL            0xff

#define CFG_NENTRIES        11                      /* number of cnode cfg entries */

#define CFG_ENTRYFMT_MAXLEN 12                      /* Maximum length of formatted config GUI entry. Includes 2 spaces, equal sign and 2 double colons */

/* config settings */
#define CNODE_CFG_PATH      "/etc/cnode/node.cfg"   /* Central Node default config file path */
#define CNODE_CFG_FOLDER    "/etc/cnode"            /* central node configuration files folder */
#define CNODE_UUID_PATH     "/etc/cnode/.uuidauth"  /* UUID file location */         

#define CFG_FMT_READABLE    1                       /* Human readable config entry string */
#define CFG_FMT_MACHINE     0                       /* opposite */

/* function declarations */

/**
 * @brief Creates new crypto-secure UUID and store it in `CNODE_UUID_PATH` file
 * 
 * @param optout    [OPTIONAL] additional output for the new UUID. Can be NULL
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE 
 */
int config_create_uuid(char *optout);

/**
 * @brief Reads existing UUID file (`CNODE_UUID_PATH`) 
 * 
 * @param outbuf    [OPTIONAL] output buffer for UUID string (can be `NULL`)
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE
 */
int config_read_uuid(char *outbuf);

/**
 * @brief Matches integer config entry to it's name containing string
 * 
 * @param cfg_entry entry to evaluate
 * @param hread     human readable, 0 = no, 1 = yes
 *  
 * @return config entry char pointer
 * @return `NULL` on error 
 */
char *config_entry_to_str(int cfg_entry, int hread);

/**
 * @brief Reads all config entries and evaluates them, so Central Node can be configured as desired by the user
 * 
 * @return EXIT_SUCCESS 
 * @return EXIT_FAILURE 
 */
int config_readcfg(void);

/**
 * @brief Retrieve config entry value that's already stored in nodecfg
 * 
 * @param cfg_entry Config entry id
 * 
 * @return pointer to nodecfg::config entry
 * @return `NULL` on Error
 */
void *config_get_value(int cfg_entry);

#endif