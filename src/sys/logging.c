#include "logging.h"            // logging base

#include <time.h>               // time
#include <stdio.h>              // io
#include <stdarg.h>             // vaargs
#include <string.h>             // strlen
#include <stdlib.h>
#include <linux/limits.h>       // PATH_MAX

int log_set_path(const char *log_path_arg) {
    if (log_path_arg == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        return EXIT_FAILURE;
    }

    size_t lpath_slen = strlen(log_path_arg);

    // compare slen against PATH_MAX
    if (lpath_slen >= (PATH_MAX - 1)) {
        NODE_SETLASTERR(NO_ADDR, "Unable to set logfile path:", EINVAL);
        return EXIT_FAILURE;
    }

    nodecfg.log_path = malloc(sizeof(char) * (lpath_slen + 1));
    if (NULL == nodecfg.log_path) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to set logfile path:", errno);
        log_write_trace("log_set_path()", "nodecfg.log_path");
        return EXIT_FAILURE;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-contains-nul"
    snprintf(nodecfg.log_path, lpath_slen + 1, "%s\0", log_path_arg);
#pragma GCC diagnostic pop

    return EXIT_SUCCESS;
}

char *log_lvl_to_str(int loglevel) {
    switch (loglevel) {
    case LOG_DEBUG:
        return "DBG";
        break;
    
    case LOG_NOTIF:
        return "INFO";
        break;

    case LOG_WARNING:
        return "WARN";
        break;
    case LOG_ERROR:
        return "ERR";
        break;

    case LOG_CRITICAL:
        return "CRIT";
        break;
    
    /* below are non-gui printf fmt information messages */
    case CNODE_NOTIF_OK:
        return "OK";
        break;

    case CNODE_NOTIF_INFO:
        return "INFO";
        break;

    case CNODE_NOTIF_WARN:
        return "WARN";
        break;

    case CNODE_NOTIF_ERR:
        return "ERR";
        break;

    default:
        break;
    }

    return NULL;
}

#ifdef FUTURE_IS_NOW
static char *log_auto_event_message(error_t node_errno) {
    return NULL;
}
#endif

ssize_t log_write(unsigned int log_level, const char* fmt, ...) {
    /* Check minimum log level */
    if (log_level < nodecfg.log_level) { return 0; }
    
    /* Check if logfile is available */
    if (!nodecfg.logfile_available) { return -EXIT_FAILURE; }
    
    va_list args;
    
    time_t now;
    size_t retsum;
    char* newline = "\n";

    va_start(args, fmt);
    FILE *errfp = fopen(nodecfg.log_path, "ab");

    if (!errfp) { return -errno; }

    /* Get current time */
    time(&now);

    char tt_time[256+64];

    snprintf(tt_time, 64, ctime(&now));
    int time_len = strlen(tt_time);
    
    // no idea what is this, but it's ugly, so check it out later and get rid of it..
    if (tt_time[time_len-1] == '\n') {
        tt_time[time_len-3] = 0;
    }

    const char *log_lvl_str = log_lvl_to_str(log_level);
    if (log_lvl_str == NULL) { log_lvl_str = "UNKNOWN"; }

    size_t _prefix = fprintf(errfp, "[%s]-[%s]: ", tt_time, log_lvl_str);
    size_t _suffix = vfprintf(errfp, fmt, args);
    fprintf(errfp, newline);

    va_end(args);
    fclose(errfp);

    retsum = _prefix+_suffix+1;

    /* Update flag indicator */
    nodecfg.log_written = true;

    return retsum;
}

int log_reset(void) {
    FILE *fp = fopen(nodecfg.log_path, "w");
    if (NULL == fp) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to reset log file:", errno)
        log_write_trace("log_reset()", "fp");
        return EXIT_FAILURE;
    }

    fclose(fp);
    log_write(LOG_DEBUG, "Logfile has been reset");
    return EXIT_SUCCESS;
}

void log_write_trace(const char *function_name, const char *error_object) {
    if (NO_ADDR == nodecfg.rsp64) {
        log_write(LOG_DEBUG, "TRACE: E%d - %s => %s", nodecfg.last_errno, function_name, error_object);
    } else {
        log_write(LOG_DEBUG, "TRACE: [%#02llx] E%d - %s => %s", nodecfg.rsp64, nodecfg.last_errno, function_name, error_object);
    }
}