#include "../base/cnode.h"  /* cnode base */
#include "config.h"         /* config base */
#include "sysctl.h"         /* system control */
#include "crypto/md5.h"     /* md5 hashing */
#include "crypto/uuid.h"    /* UUID gen */
#include "../data/data.h"   /* rtonm() */

#include <string.h>
#include <stdio.h>
#include <ctype.h>          /* isdigit() */

#include <sys/stat.h>       /* mkdir */
#include <sys/types.h>


int config_create_uuid(char *optout) {
    FILE *fp = fopen(CNODE_UUID_PATH, "wb");

    if (fp == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to create new UUID file:", errno);
        log_write_trace("config_create_uuid()", "fp");
        return EXIT_FAILURE;
    }

    int ret = EXIT_SUCCESS;
    char uuid_str[UUID_STR_LEN];
    
    ret = crypto_gen_safe_uuid(uuid_str);

    if (ret != EXIT_SUCCESS) { goto END_ROUTINE; }

    /* save uuid to CNODE_UUID_PATH file */
    fprintf(fp, "%s", uuid_str);

    if (optout != NULL) {
        snprintf(optout, UUID_STR_LEN, uuid_str);
    }

END_ROUTINE:
    fclose(fp);
    return ret;
}

/* UUID is sent to server as md5 hash */
int config_read_uuid(char *outbuf) {
    int anti_deadlock = 0;
TRY_OPEN_FILE:
    ; FILE *fp = fopen(CNODE_UUID_PATH, "rb");

    if (fp == NULL) {
        if (config_create_uuid(NULL) != EXIT_SUCCESS) {
            NODE_SETLASTERR(CURRENT_ADDR, "Unable to create UUID file:", errno);
            log_write_trace("config_read_uuid()", "confog_create_uuid()");

            return EXIT_FAILURE;
        }

        anti_deadlock++;
        if (anti_deadlock >= 2) { return EXIT_FAILURE; }
        goto TRY_OPEN_FILE;
    }

    int ret = EXIT_SUCCESS;
    char uuid_str[UUID_STR_LEN] = { 0 };

    if (!fgets(uuid_str, UUID_STR_LEN, fp)) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to read UUID file:", EFPREAD);
        log_write_trace("config_read_uuid()", "fgets() == NULL");
        
        ret = EXIT_FAILURE;
        goto END_ROUTINE;
    }

    if (NULL != outbuf) {
        /* shit goes down if outbuf isnt big enough, hope you are aware.. */
        strncpy(outbuf, uuid_str, UUID_STR_LEN);
    }    

END_ROUTINE:
    fclose(fp);
    return ret;
}

/**
 * @brief Create new Central Node config file.
 * Spawns config entries and sets their default values
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE 
 */
int config_createcfg(void) {
    /* Default cfg entries to check against */
    const char *config_entries[CFG_NENTRIES] = {
        "AUTOFIX=\"0\"",                /* autofix system */
        "GODMODE=\"0\"",                /* nodeWD godmode support */
        "NWDMODE=\"0\"",                /* nodeWD enforcing mode */
        "NETLOGO=\"1\"",                /* show/hide network logos */
        "ALLOWSU=\"0\"",                /* allow running central node as non-setuid superuser */
        "NOHTTPS=\"0\"",                /* allows HTTP unencrypted communication */
        "NLOGLVL=\"1\"",                /* central node logging level */
        "TIMEOUT=\"15\"",               /* network timeout in seconds */
        "NTUNDEV=\"tun0\"",             /* default tun interface */
        "S5PROXY=\"NULL\"",             /* default socks5 proxy (none) */
        "NETREPO=\"centralnode.xyz\""   /* network repository */
    };

    if (!dir_exists(CNODE_CFG_FOLDER)) {
        log_write(LOG_NOTIF, "Creating Central Node configuration folder - '%s'", CNODE_CFG_FOLDER);
        int mkdir_ret = mkdir(CNODE_CFG_FOLDER, 0644);
        if (mkdir_ret != EXIT_SUCCESS) {
            NODE_SETLASTERR(CURRENT_ADDR, "Unable to create Central Node config directory:", errno);
            log_write_trace("config_createcfg()", "mkdir()");
            return EXIT_FAILURE;
        }
        log_write(LOG_NOTIF, "Successfully created new config directory");
    }

    FILE *fp = fopen(CNODE_CFG_PATH, "wb");
    if (fp == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, "Failed to create new config file:", errno);
        log_write_trace("config_createcfg()", "fp");
        return EXIT_FAILURE;
    }

    /* write default config entries */
    for (int i = 0; i < CFG_NENTRIES; i++) {
        log_write(LOG_DEBUG, "Setting default config value: [%s]", config_entries[i]);
        fprintf(fp, "%s\n", config_entries[i]);
    }

    fclose(fp);

    return EXIT_SUCCESS;
}

/**
 * @brief Matches integer config entry to it's name containing string
 * 
 * @param cfg_entry     entry to evaluate
 * @param hread         human readable, 0 = no, 1 = yes
 * 
 * @return Config entry char pointer
 * @return `NULL` on error 
 */
char *config_entry_to_str(int cfg_entry, int hread) {
    /* if hread == 1, we return nice human readable string. if not, we assume it's for Central Node to check against known config entries. */
    switch (cfg_entry) {
        case CFG_AUTOFIX:
            return (hread == 1) ? "AutoFix" : "AUTOFIX";
            break;

        case CFG_GODMODE:
            return (hread == 1) ? "GodMode" : "GODMODE";
            break;

        case CFG_NWDMODE:
            return (hread == 1) ? "nodeWD strict mode" : "NWDMODE";
            break;

        case CFG_NETLOGO:
            return (hread == 1) ? "NetLogos" : "NETLOGO";
            break;

        case CFG_NLOGLVL:
            return (hread == 1) ? "LogLevel" : "NLOGLVL";
            break;

        case CFG_NOHTTPS:
            return (hread == 1) ? "No-HTTPS" : "NOHTTPS";
            break;

        case CFG_TIMEOUT:
            return (hread == 1) ? "Timeout" : "TIMEOUT";
            break;

        case CFG_ALLOWSU:
            return (hread == 1) ? "non-setuid root execution" : "ALLOWSU";
            break;

        case CFG_TUNDEV:
            return (hread == 1) ? "TUN interface" : "NTUNDEV";
            break;

        case CFG_PROXY:
            return (hread == 1) ? "Socks5 Proxy" : "S5PROXY";
            break;

        case CFG_NETREPO:
            return (hread == 1) ? "Network Repository" : "NETREPO";
            break;

        default:
            break;
    }

    return NULL;
}

/**
 * @brief Converts "machine" config string entry to it's numerical constant equivalent 
 * 
 * @param cfg_entry     entry string to convert
 * 
 * @return Converted config entry value
 * @return `CFG_NULL` on error
 *  
 */
int config_str_to_entry(char *cfg_entry) {
    for (int i = 0; i < CFG_NENTRIES; i++) {
        if (strncmp(cfg_entry, config_entry_to_str(i, CFG_FMT_MACHINE), CFG_ARG_MAX) == 0) {
            return i;
        }
    }

    return CFG_NULL;
}

/**
 * @brief Evaluates config entry values
 * 
 * @param cfg_entry         config entry ID
 * @param entry_value       pointer to config entry value
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE 
 */
int config_evalentry(unsigned int cfg_entry, void *entry_value) {
    if (entry_value == NULL) {
        log_write(LOG_ERROR, "Attempted to pass NULL pointer");
        log_write(LOG_DEBUG, "\nTRACE: E%d - config_evalentry() => entry_value", ENULLPTR);
        return EXIT_FAILURE;
    }

    size_t slen = 0;
    uint64_t next_mul = 0;
    int retcode = EXIT_SUCCESS;
    char *debug_msg = (*((bool *) entry_value) == true) ? "Enabled" : "Disabled";;

    /* Check for bad entry_value. Only CFG_TIMEOUT and CFG_TUNDEV are not bool, therefore all other entries can only by bool (0 or 1) */
    if (*((uint64_t *) entry_value) > 1) {
        if (cfg_entry < CFG_TIMEOUT) {
            NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
            log_write(LOG_ERROR, "Invalid value specified for config entry %d - %lu", cfg_entry, *((unsigned long *) entry_value));
            log_write_trace("config_evalentry()", "cfg_entry");
            return EXIT_FAILURE;
        }
    }

    if (cfg_entry >= CFG_TUNDEV) {
        slen = strlen((char *) entry_value);
    }

    switch (cfg_entry) {
        case CFG_AUTOFIX:
            nodecfg.autofix = *((bool *) entry_value);
            log_write(LOG_DEBUG, "%s AutoFix", debug_msg);
            break;

        case CFG_GODMODE:
            nodecfg.godmode = *((bool *) entry_value);
            log_write(LOG_DEBUG, "%s GodMode", debug_msg);
            break;

        case CFG_NWDMODE:
            nodecfg.nodewd_required = *((bool *) entry_value);
            log_write(LOG_DEBUG, "%s nodeWD strict/enforcing mode", debug_msg);
            break;

        case CFG_NETLOGO:
            nodecfg.netlogos = *((bool *) entry_value);
            log_write(LOG_DEBUG, "%s network logos", debug_msg);
            break;

        case CFG_TIMEOUT:
            nodecfg.net_timeout = *((unsigned long *) entry_value);
            log_write(LOG_DEBUG, "Set network timeout to '%lu'", *((unsigned long *) entry_value));
            break;

        case CFG_ALLOWSU:
            nodecfg.allow_root_execution = *((bool *) entry_value);
            if (nodecfg.allow_root_execution) {
                /* warn user in log if this is enabled */
                log_write(LOG_WARNING, "Enabled non-setuid root execution");
            } else {
                log_write(LOG_DEBUG, "%s non-setuid root execution", debug_msg);
            }
            break;

        case CFG_NOHTTPS:
            nodecfg.net_nohttps = *((bool *) entry_value);
            log_write(LOG_DEBUG, "%s No-HTTPS", debug_msg);
            break;

        case CFG_NLOGLVL:
            if (*((unsigned int *) entry_value) > (unsigned int) LOG_CRITICAL) {
                NODE_SETLASTERR(NO_ADDR, NODE_NOINFO, EINVCFG);
                log_write(LOG_ERROR, "Ignoring invalid LogLevel value '%lu'", *((unsigned long *) entry_value));
                log_write_trace("config_evalentry()", "entry_value");
                retcode = EXIT_FAILURE;
                break;
            }
            
            /* Call setloglvl function */
            int setlogret = cnode_setloglvl(*((int *) entry_value), false);

            if (setlogret == EXIT_SUCCESS) {
                /* log level isn't locked - changed successfully */
                const char *loglvl_str = log_lvl_to_str(*((int *) entry_value));
                if (loglvl_str == NULL) {
                    NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
                    log_write(LOG_ERROR, "Invalid logging level specified: %d", *((unsigned long *) entry_value));
                    log_write_trace("config_evalentry()", "loglvl_str");
                    retcode = EXIT_FAILURE;
                    break;
                }

                log_write(LOG_DEBUG, "Set LogLevel to LOG_%s", loglvl_str);
            } else {
                /* log level was locked from startup */
                log_write(LOG_WARNING, "LogLevel was forcibly set at startup. Ignoring config value.");
                retcode = EXIT_FAILURE;;
            }
            break;
    
        case CFG_TUNDEV:
            /* custom tun device */
            if (slen > IF_DEV_MAXLEN) {
                NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
                log_write(LOG_ERROR, "Invalid tun device '%s'", (char *) entry_value);
                log_write_trace("config_evalentry()", "slen");

                strncpy((char *) entry_value, "ERROR", 6);
                retcode = EXIT_FAILURE;
            }
            
            nodecfg.tincifnet = (char *) entry_value;
            
            if (slen > nodecfg.longest_cfg_entry) {
                next_mul = rtonm(slen + nodecfg.longest_cfg_entry, 8);
                if (next_mul > nodecfg.longest_cfg_entry) {
                    nodecfg.longest_cfg_entry = next_mul;
                }
            }

            log_write(LOG_DEBUG, "Set tun interface to '%s'", (char *) entry_value);
            break;

        case CFG_PROXY:
            /* socks5 proxy */
            if(slen > PROXY_MAXLEN) {
                NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
                log_write(LOG_ERROR, "Invalid SOCKS5 Proxy '%s'", (char *) entry_value);
                log_write_trace("config_evalentry()", "slen");

                strncpy((char *) entry_value, "ERROR", 6);
                retcode = EXIT_FAILURE;
            }

            nodecfg.proxyaddr = (char *) entry_value;
            
            if (slen > nodecfg.longest_cfg_entry) {
                next_mul = rtonm(slen + nodecfg.longest_cfg_entry, 8);
                if (next_mul > nodecfg.longest_cfg_entry) {
                    nodecfg.longest_cfg_entry = next_mul;
                }
            }

            if (strncmp(nodecfg.proxyaddr, "NULL", 4) == 0) {
                log_write(LOG_DEBUG, "No SOCKS5 Proxy set");
            } else {
                log_write(LOG_DEBUG, "Set SOCKS5 Proxy to '%s'", (char *) entry_value);
            }
            break;

        case CFG_NETREPO:
            /* network repo */
            if (slen > NETREPO_NAME_MAX) {
                NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
                log_write(LOG_ERROR, "Invalid repository name '%s'", (char *) entry_value);
                log_write_trace("config_evalentry()", "slen");

                strncpy((char *) entry_value, "ERROR", 6);
                retcode = EXIT_FAILURE;
            }
            nodecfg.netrepo = (char *) entry_value;
            
            if (slen > nodecfg.longest_cfg_entry) {
                next_mul = rtonm(slen + nodecfg.longest_cfg_entry, 8);
                if (next_mul > nodecfg.longest_cfg_entry) {
                    nodecfg.longest_cfg_entry = next_mul;
                }
            }

            log_write(LOG_DEBUG, "Set network repository to '%s'", (char *) entry_value);
            break;

        default:
            NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVCFG);
            log_write(LOG_ERROR, "Invalid config entry: %d", cfg_entry);
            log_write_trace("config_evalentry()", "cfg_entry");
            retcode = EXIT_FAILURE;
            break;
    }

    return retcode;
}

/**
 * @brief Counts number of entries in existing config file
 * 
 * @param output    if true, existing config entries are written to the log file, with current set loglevel
 * @return          number of entries in config file
 */
unsigned int config_count_entries(bool output) {
    if (!file_exists(CNODE_CFG_PATH)) {
        return 0;
    }

    FILE *fp = fopen(CNODE_CFG_PATH, "rb");
    if (NULL == fp) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to open config file for reading:", errno);
        log_write_trace("config_count_entries()", "fp");
        return 0;
    }

    unsigned int entry_count = 0;

    char linebuf[256] = { 0 };
    while (fgets(linebuf, sizeof(linebuf), fp)) {
        if (linebuf[0] != '\n') {
            if (output) {
                linebuf[strcspn(linebuf, "\n")] = 0;
                log_write(LOG_DEBUG, "Outputting existing config value: '[%s]'", linebuf);
            }
            entry_count++;
        }
        memset(linebuf, 0, sizeof(linebuf));
    }

    fclose(fp);
    log_write(LOG_DEBUG, "Found total of %u valid config entries", entry_count);
    return entry_count;
}

int config_readcfg(void) {
    unsigned int cfg_nentries = config_count_entries(false);
    if (!file_exists(CNODE_CFG_PATH) || cfg_nentries != CFG_NENTRIES) {
        /* Config doesn't exist, or outdated.. create new one. also signal first time launch */
        nodecfg.first_time_launch = true;
        
        if (cfg_nentries != 0) {
            /* "backup" current config entries, dirty solution for now */
            config_count_entries(true);
        }

        int ret = config_createcfg();
        if (ret != EXIT_SUCCESS) {
            /* failed to create new config */
            return EXIT_FAILURE;
        }
    }

    FILE *fp = fopen(CNODE_CFG_PATH, "rb");
    if (fp == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to open config file for reading:", errno);
        log_write_trace("config_readcfg()", "fp");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < CFG_NENTRIES; i++) {
        unsigned long cvalue = CFG_NULL;    /* converted value */
        int reserve_id = CFG_NULL;          /* this is used if a config entry doesn't follow the actual entry order, and needs to be passed to cfg_evalentry(), so we don't modify i iterator */


        /* First and last double quotes pointers */
        char *fdq = NULL;
        char *ldq = NULL;
        
        /* strtol endptr (illegal character) should always equal to ldq */
        char *endptr = NULL;

        char read_buffer[CFG_ARG_MAX] = { 0 };
        

        fgets(read_buffer, sizeof(read_buffer), fp);

        fdq = strchr(read_buffer, '"');
        ldq = strrchr(read_buffer, '"');

        /* check if the entry label string matches a known config entry and current iterator value */
        {
            /* config entries are 7 characters long */
            size_t cfg_entry_len = 7;
            
            const char *cfgentry_str = config_entry_to_str(i, CFG_FMT_MACHINE);
            
            /* Checking if config entry exists. This error should NEVER happen, but extra safety never hurts */
            if (cfgentry_str == NULL) {
                NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
                log_write(LOG_ERROR, "Unknown config entry: ID: %d", i);
                log_write_trace("config_readcfg()", "cfgentry_str");
                continue;
            }

            if (strncmp(read_buffer, cfgentry_str, cfg_entry_len) != 0) {
                /* entry string mismatch, check if it's a known misplaced entry */
                for (int j = 0; j < (int) CFG_NENTRIES; j++) {
                    const char *reserve_cfgentry_str = config_entry_to_str(j, CFG_FMT_MACHINE);
                    
                    /* Checking if reserve entry exists. This error should NEVER happen, but extra safety never hurts */
                    if (reserve_cfgentry_str == NULL) {
                        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
                        log_write(LOG_ERROR, "Unknown config reserve entry: %d", j);
                        log_write_trace("config_readcfg()", "reserve_cfgentry_str");
                        continue;
                    }
                    if (strncmp(read_buffer, config_entry_to_str(j, CFG_FMT_MACHINE), cfg_entry_len) == EXIT_SUCCESS) {
                        /* known entry found, change reserve_id to it, so it can be passed to cfg_evalentry(), instead of the i iterator */
                        reserve_id = j;
                        break;
                    }
                }

                /* if no entry is found, report an error */
                if (reserve_id == CFG_NULL) {
                    log_write(LOG_WARNING, "Ignoring invalid config entry %d - '%s'", i, read_buffer);
                    
                    /* continue to next entry */
                    continue;
                }
            }
        }

        /* check if reserve_id is not CFG_NULL. if so, evalue it instead of the i iterator */
        int entry_to_pass = (reserve_id == CFG_NULL) ? i : reserve_id;


        /* Check if config entry is CFG_TUNDEV or larger (string options start at CFG_TUNDEV), because it's value is (char *) instead of ulong */
        if (entry_to_pass >= CFG_TUNDEV) {
            /* treat entry as string */
            size_t slen_diff = ldq - fdq;
            size_t mallocsz = (slen_diff >= 8) ? slen_diff : 8;
            char *strvalue = malloc(sizeof(char) * (mallocsz + 1));

            if (strvalue == NULL) {
                NODE_SETLASTERR(CURRENT_ADDR, "Unable to allocate memory:", errno);
                log_write_trace("config_readcfg()", "strvalue");
                return EXIT_FAILURE;
            }

            int scpy_ret = snprintf(strvalue, slen_diff, (fdq + 1));

            if (scpy_ret < 0) {
                NODE_SETLASTERR(CURRENT_ADDR, "Output error:", EIO);
                log_write_trace("config_readcfg()", "snprintf()");
                
                free(strvalue);
                return EXIT_FAILURE;
            }

            log_write(LOG_DEBUG, "Found config entry: '%s' with value \"%s\"", config_entry_to_str(i, CFG_FMT_READABLE), strvalue);

            config_evalentry(entry_to_pass, strvalue);

        } else {
            /* treat normally - as ulong */
            cvalue = strtoul(fdq + 1, &endptr, BASE_DECIMAL);

            if (endptr != ldq) {
                log_write(LOG_WARNING, "Invalid Config Entry: %d", i);
                log_write(LOG_DEBUG, "\nTRACE: EINVAL : ['%s' - ID - %d - CVAL: %ld - RSP64: %llx]", read_buffer, i, cvalue, __spaddr64());
                continue;
            }

            log_write(LOG_DEBUG, "Found config entry: '%s' with value '%d'", config_entry_to_str(i, CFG_FMT_READABLE), cvalue);


            /* last error check, this should never happen.. looking at this now, i dont even know why this is here, literally impossible and there's no scenario where this couldve happened */
            if (cvalue == CFG_NULL) {
                NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVCFG);
                log_write(LOG_ERROR, "Config Entry %d is CFG_NULL", i);
                return EXIT_FAILURE;
            }


            /* eval config entry now. we don't need to check for return value, since failure is handled in config_evalentry()
             * plus, we continue the loop on either success and failure.
            */
            config_evalentry(entry_to_pass, &cvalue);
        }
    }

    log_write(LOG_DEBUG, "Evaluated all known config entries, closing the config file");
    fclose(fp);
    return EXIT_SUCCESS;
}

void *config_get_value(int cfg_entry) {
    switch (cfg_entry) {
        case CFG_AUTOFIX:
            return &nodecfg.autofix;
            break;

        case CFG_GODMODE:
            return &nodecfg.godmode;
            break;

        case CFG_NWDMODE:
            return &nodecfg.nodewd_required;
            break;

        case CFG_NETLOGO:
            return &nodecfg.netlogos;
            break;

        case CFG_ALLOWSU:
            return &nodecfg.allow_root_execution;
            break;

        case CFG_NOHTTPS:
            return &nodecfg.net_nohttps;
            break;

        case CFG_NLOGLVL:
            return &nodecfg.log_level;
            break;

        case CFG_TIMEOUT:
            return &nodecfg.net_timeout;
            break;

        case CFG_TUNDEV:
            return &nodecfg.tincifnet;
            break;

        case CFG_PROXY:
            return &nodecfg.proxyaddr;
            break;

        case CFG_NETREPO:
            return &nodecfg.netrepo;
            break;

        default:
            break;
    }

    return NULL;
}

/**
 * @brief Writes a config value to the config file
 * 
 * @param entry_id  entry to write
 * @param wvalue    pointer to value to write
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE 
 */
int config_writevalue(int entry_id, uint64_t *wvalue) {
    if (wvalue == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("config_writevalue()", "wvalue");
        return EXIT_FAILURE;
    }
    
    int ret_status = EXIT_SUCCESS;
    
    /* first, check entry_id credibility */
    if (entry_id > CFG_NENTRIES || entry_id < 0) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVCFG);
        log_write(LOG_ERROR, "Unable to write config entry %d (val: %lu [%p]): Invalid Entry", entry_id, *wvalue, wvalue);
        log_write_trace("config_writevalue()", "entry_id");
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(CNODE_CFG_PATH, "rb+");
    if (fp == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to open config file for appending:", errno);
        log_write_trace("config_writevalue()", "fp");
        return EXIT_FAILURE;
    }

    /* config entry we're gonna be checking fpread against */
    char *entry_str = config_entry_to_str(entry_id, CFG_FMT_MACHINE);

    /* sanity check */
    if (entry_str == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
        log_write(LOG_ERROR, "Unknown config entry specified: %d", entry_id);
        log_write_trace("config_writevalue()", "entry_str");
        ret_status = EXIT_FAILURE;
        goto FP_EXIT;
    }

    /* Read all config lines to find our entry */
    for (int i = 0; i < CFG_NENTRIES; i++) {
        char read_buffer[CFG_ARG_MAX] = { 0 };      /* fp output */
        char format_buffer[CFG_ARG_MAX] = { 0 };    /* fp input */
        fgets(read_buffer, sizeof(read_buffer), fp);

        /* check for known config entry */
        if (strncmp(read_buffer, entry_str, 7) == EXIT_SUCCESS) {
            /* match found, cook new input ( `TIMEOUT="13"\n"` )*/
            /* if the entry is CFG_TUNDEV, we need to cook the input buffer with (char *), otherwise ulong */
            if (config_str_to_entry(entry_str) >= CFG_TUNDEV) {
                snprintf(format_buffer, sizeof(format_buffer) - 1, "%s=\"%s\"\n", entry_str, (char *) wvalue);
            } else {
                snprintf(format_buffer, sizeof(format_buffer) - 1, "%s=\"%lu\"\n", entry_str, (unsigned long) *wvalue);
            }
            
            /* this should be the number of bytes written by fwrite(), we'll use it to check exactly that */
            size_t fmt_len = strlen(format_buffer);

            /* now we need to rewind to the beginning of the line.
             * that's done by getting the strlen of read_buffer (fp output) and subtracting it from the current fpos, which should be the end of the line
            */
            size_t line_len = strlen(read_buffer);  /* slen of the line that was just read */
            unsigned long cur_pos = ftell(fp);      /* current fpos */

            /* the difference is the beginning of the current line */
            size_t line_dif = cur_pos - line_len;

            /* rewind line */
            int retc = fseek(fp, line_dif, SEEK_SET);

            if (retc < 0) {
                /* error in fseek() */
                NODE_SETLASTERR(CURRENT_ADDR, "An error has occured while reading the config file:", errno);
                log_write(LOG_DEBUG, "RET: %d [LINE_DIF: %zu LINE_LEN: %lu]", retc, line_dif, line_len);
                log_write_trace("config_writevalue()", "fseek()");
                ret_status = EXIT_FAILURE;
                break;
            }

            size_t writesz = fwrite(format_buffer, 1, fmt_len, fp);

            if (writesz != fmt_len) {
                /* error - number of written bytes doesn't match the format_buffer length */
                error_t err = (errno == EXIT_SUCCESS) ? EFPWRITE : errno;
                NODE_SETLASTERR(CURRENT_ADDR, "An error has occured while writting to config file:", err);
                log_write(LOG_DEBUG, "WSZ: %zu != FMTSZ: %zu", writesz, fmt_len);
                log_write_trace("config_writevalue()", "writesz");
                ret_status = EXIT_FAILURE;
                break;
            }

            log_write(LOG_NOTIF, "Updating config entry %d: [%s]", entry_id, format_buffer);
            break;
        }
    }

    FP_EXIT:
    fclose(fp);
    return ret_status;
}
