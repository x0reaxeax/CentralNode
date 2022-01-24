#include "../../base/cnode.h"       /* cnode base */

#include "uuid.h"

int crypto_gen_safe_uuid(char *uuidstr) {
    if (uuidstr == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("sys_gen_safe_uuid()", "uuidstr");
        return EXIT_FAILURE;
    }
    
    uuid_t uuid;
    /* unsigned char uuidstr[UUID_STR_LEN] = { 0 }; */

    /* generate uuid */
    int ret = uuid_generate_time_safe(uuid);

    if (ret != EXIT_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "Could not generate cryptographically secure synchronization-free UUID:", EINTEGRITY);
        log_write(LOG_WARNING, "Falling back to unix high-quality randomness generator");

        uuid_generate(uuid);
    }


    /* Unpack UUID */
    uuid_unparse_lower(uuid, uuidstr);

    return EXIT_SUCCESS;
}