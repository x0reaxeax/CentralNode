#ifndef _CNODE_LOGGING_H_
#define _CNODE_LOGGING_H_

#include "../base/cnode.h"

#include <sys/types.h>          /* ssize_t & time_t */

/* Logging levels */
#define LOG_CRITICAL    4
#define LOG_ERROR       3
#define LOG_WARNING     2
#define LOG_NOTIF       1
#define LOG_DEBUG       0

extern volatile uintptr_t   __spaddr64(void);

/* return current call stack function address */
#define CURRENT_ADDR        __spaddr64();

/**
 * @brief Set file path & name of log file on disk
 * 
 * @param log_path_arg      - path to logfile
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE 
 */
int log_set_path(const char *log_path_arg);

/**
 * @brief Return LOG_LEVEL as string prefix
 * 
 * @param loglevel      - loglevel to convert
 * 
 * @return log level prefix
 * @return `NULL` on error
 */
char *log_lvl_to_str(int loglevel);

/**
 * @brief Write or append to log file
 * 
 * @param log_level     - message log level
 * @param message       - formatted log message
 * @param ...           - fmt args
 * 
 * @return positive value   - number of characters written
 * @return negative value   - error code
 */
ssize_t log_write(unsigned int log_level, const char* fmt, ...);

/**
 * @brief Resets logfile
 * 
 * @return EXIT_SUCCESS
 * @return EXIT_FAILURE 
 */
int log_reset(void);

/**
 * @brief Logs unformatted trace debug information. This function is only to be called after NODE_SETLASTERR()
 * 
 * @param function_name     name of function in which the event/error occurred
 * @param error_object      name of "object" that caused the event/error
 */
void log_write_trace(const char *function_name, const char *error_object);

#endif