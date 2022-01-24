#include "sysctl.h"

#include <stdio.h>          /* FILE */
#include <string.h>         /* strstr() */

#include <sys/stat.h>
#include <sys/types.h>      /* uid_t */
#include <unistd.h>         /* getuid() */
#include <dirent.h>         /* opendir */

extern long sys_finit_module(int fd, const char *uargs, int flags);
extern long sys_delete_module(const char *name_user, unsigned int flags);

long finit_module_wrapper(int fd, const char *uargs, int flags) {
    /* can be NULL ?
    if (NULL == uargs) {
        NODE_SETLASTERR(CURRENT_ADDR, ENULLPTR);
        log_write_trace("finit_module_wrapper()", "uargs");
        return EXIT_FAILURE;
    }
    */

    if (fd < 1) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EBADFD);
        log_write_trace("finit_module_wrapper()", "fd");
        return EXIT_FAILURE;
    }

    if (sys_finit_module(fd, uargs, flags) != EXIT_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to load kernel module:", errno);
        log_write_trace("fint_module_wrapper()", "sys_finit_module()");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

long delete_module_wrapper(const char *name_user, unsigned int flags) {
    if (NULL == name_user) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("delete_module_warpper()", "name_user");
        return EXIT_FAILURE;
    }

    if (sys_delete_module(name_user, flags) != EXIT_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to remove kernel module:", errno);
        log_write_trace("delete_module_wrapper()", "sys_delete_module()");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int rootchecker(void) {
    uid_t uid = getuid();
    if (uid != UID_ROOT) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}

bool is_wsl(void) {
    /* ugly, but i think it will do.. */
    FILE *fp = fopen("/proc/sys/kernel/osrelease", "r");
    char readbuffer[256] = { 0 };
    
    if (NULL == fp) {
        NODE_SETLASTERR(CURRENT_ADDR, "fopen() error:", errno);
        log_write_trace("is_wsl()", "fp");
        return false;
    }

    fgets(readbuffer, (256 - 1), fp);
    fclose(fp);

    if (strstr(readbuffer, "Microsoft") != NULL || strstr(readbuffer, "microsoft") != NULL) {
        return true;
    }

    return false;
}

int drop_superuser(void) {
    if (!rootchecker()) {
        NODE_SETLASTERR(NO_ADDR, "Unable to drop superuser privileges:", EINVUSR);
        log_write(LOG_DEBUG, "Attempted to drop superuser privileges, but current user is not superuser");
        return EXIT_FAILURE;
    }

    char *str_nonsu_name = getenv("SUDO_USER");
    char *str_nonsu_uid = getenv("SUDO_UID");
    char *str_nonsu_gid = getenv("SUDO_GID");
    char **str_cptr;                            /* pointer to pointer to environment variable to convert, used in for loop down below */ 

    uid_t *entry_id = NULL;                     /* pointer to either nonsu_uid or nonsu_gid, used in for loop down below */
    uid_t nonsu_uid = 1;                        /* Converted uid */
    uid_t nonsu_gid = 1;                        /* converted gid */
    char *endptr = NULL;                        /* strtol error checking */

    if (str_nonsu_name == NULL || str_nonsu_uid == NULL || str_nonsu_gid == NULL) {
        NODE_SETLASTERR(NO_ADDR, "Unable to read environment variable:", EINVUSR);
        log_write_trace("drop_superuser()", "SUDO_UID | SUDO_GID | SUDO_USER");

        /* if nodecfg.allow_root_execution is true, don't treat this as an error */
        return ((nodecfg.allow_root_execution) ? CNODE_EXECUTION_CONTINUE : EXIT_FAILURE);
    }


    for (int i = 0; i < 2; i++) {       
        /* the for loop is going to run 2 times, I dodn't want to use a goto.. a solution for the big amount of error checks */
        switch (i) {
            case 0:
                entry_id = &nonsu_uid;
                str_cptr = &str_nonsu_uid;
                break;

            case 1:
                entry_id = &nonsu_gid;
                str_cptr = &str_nonsu_gid;
                break;

            default:
                /* nonsense */
                break;
        }

        /* if nonsense happens.. */
        if (entry_id == NULL || str_cptr == NULL) {
            log_write(LOG_ERROR, "Attempted to pass NULL pointer");
            log_write(LOG_DEBUG, "\nTRACE: drop_superuser() => entry_id | str_cptr");
            return EXIT_FAILURE;
        }
        
        /* Convert string uid to uid_t */
        *entry_id = strtol(*str_cptr, &endptr, BASE_DECIMAL);
        if (*entry_id == 0) {
            /* invalid conversion input */
            switch (errno) {
            case ERANGE:
                /* resulting value out of range */
                NODE_SETLASTERR(CURRENT_ADDR, "Datatype conversion failure:", ERANGE);
                log_write(LOG_DEBUG, "str_cptr: '%s'; base: %d", *str_cptr, BASE_DECIMAL);
                log_write_trace("drop_superuser()", "strtol(__str_cptr, PTR_ENDPTR_MASK, __base)");
                return EXIT_FAILURE;
                break;

            case EINVAL:
                /* invalid conversion base */
                NODE_SETLASTERR(CURRENT_ADDR, "Invalid conversion base specified:", EINVAL);
                log_write_trace("drop_superuser()", "strtol() => EINVAL(BASE_DECIMAL)");
                return EXIT_FAILURE;
                break;

            default:
                /* unknown error */
                NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, errno);
                log_write(LOG_CRITICAL, "An unknown error has occured. Central Node will exit due to security reasons.");
                log_write(LOG_CRITICAL, "TRACE_CRITICAL: E%d - drop_superuser() => strtol(\"%s\", '%#02x', '%d')", nodecfg.last_errno, *str_cptr, *endptr, BASE_DECIMAL);
                return EXIT_FAILURE;
                break;
            }
        }

        if (*endptr != '\0') {
            /* SUDO_UID should return a uid number ending with nullterm, if not, error error */
            NODE_SETLASTERR(CURRENT_ADDR, "Invalid conversion results:", EINVAL);
            log_write(LOG_DEBUG, "endptr [%#02x]", *endptr);
            log_write_trace("drop_superuser()", "endptr");
            return EXIT_FAILURE;
        }
    }

    log_write(LOG_DEBUG, "Dropping superuser privileges: Target user='%s' UID='%u' GID='%u'", str_nonsu_name, nonsu_uid, nonsu_gid);

    if (nonsu_gid == 1 || nonsu_uid == 1) {
        /* in case error handling somehow slipped.. */
        NODE_SETLASTERR(CURRENT_ADDR, "Invalid conversion results:", EINVAL);
        log_write_trace("drop_superuser()", "ERROR_HANDLER_FAILURE");
    }

    if (setgid(nonsu_gid) != EXIT_SUCCESS) {
        /* error */
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to drop superuser privileges. Central Node is shutting down:", errno);
        log_write(LOG_CRITICAL, "TRACE_CRITICAL: E%d - drop_superuser() => setgid('%u')", nodecfg.last_errno, nonsu_gid);
        return EXIT_FAILURE;
    }

    if (setuid(nonsu_uid) != EXIT_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to drop superuser privileges. Central Node is shutting down:", errno);
        log_write(LOG_CRITICAL, "TRACE_CRITICAL: E%d - drop_superuser() => setuid('%u')", nodecfg.last_errno, nonsu_uid);
        return EXIT_FAILURE;
    }

    log_write(LOG_DEBUG, "Successfully dropped superuser privileges");
    
    return EXIT_SUCCESS;
}

bool file_exists (char *filename) {
    if (filename == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("file_exists()", "filename");
        return false;
    }
    struct stat buffer;   
    return (stat (filename, &buffer) == 0);
}

bool dir_exists(char *dirpath) {
    if (dirpath == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("dir_exists()", "dirpath");
        return false;
    }
    DIR *cdir = opendir(dirpath);
    if (cdir) {
        /* directory exists */
        closedir(cdir);
        return true;
    } else if (errno == ENOENT) {
        /* directory doesn't exist */
        return false;
    }

    /* opendir() failed for some other reason */
    NODE_SETLASTERR(CURRENT_ADDR, "Unable to retrieve directory information:", errno);
    log_write_trace("dir_exists()", "UNKNOWN");
    return false;
}