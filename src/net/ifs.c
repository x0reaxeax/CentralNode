#include "ifs.h"
#include "../base/cnode.h"          /* cnode */
#include "../sys/sysctl.h"          /* file exists, dir_exists */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>             /* socket */
#include <sys/ioctl.h>              /* ioctl */
#include <sys/types.h>

#include <string.h>                 /* strncpy, strlen */
#include <stdlib.h>                 /* realloc */
#include <unistd.h>                 /* close */

char *if_get_ifnet_ipaddr(char *if_name, unsigned long if_flag);

/**
 * check_ifnet()
 * --------------
 * 
 * Checks if network interface exists
 * 
 * parameters:
 *      `char *ifnet`   - path to interface device 
 * 
 * returns:
 *      `true`          - interface exists
 *      `false`         - interface doesn't exist
 * 
 */ 
bool if_check_ifnet(char *ifnet) {
    if (!file_exists(ifnet)) { return false; }
    return true;
}

/*
 * get_ifnet_names()
 * ------------------ 
 * 
 * Retrieves all available network interface names and saves them in sys_ifnets struct
 * 
 * parameters:
 *      `struct sys_ifnets *ifnst`  - pointer to system network interfaces list struct
 * 
 * returns:
 *      `EXIT_SUCCESS`
 *      `EXIT_FAILURE`              - Sets `NODE_ERR`
 * 
*/ 
int if_get_ifnet_names(struct sys_ifnets *ifnst) {
    if (ifnst == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("get_ifnet_names()", "struct sys_ifnets *ifnst");
        return EXIT_FAILURE;
    }
    
    struct if_nameindex *if_nidxs, *intf;

    if_nidxs = if_nameindex();
    
    if ( if_nidxs != NULL ) {
        intf = if_nidxs;
        ifnst->if_nifs = 0;
        for (int i = 0; i < NET_IFNET_NMAX; i++) {
        //for (intf = if_nidxs; intf->if_index != 0 || intf->if_name != NULL; intf++) {
            if (intf->if_index != 0 && intf->if_name != NULL) {
                strncpy(ifnst->if_name[ifnst->if_nifs], intf->if_name, IF_NAMESIZE);

                /* Get interface IP and netmask */
                char *if_net_addr = if_get_ifnet_ipaddr(ifnst->if_name[ifnst->if_nifs], SIOCGIFADDR);
                char *if_net_mask = if_get_ifnet_ipaddr(ifnst->if_name[ifnst->if_nifs], SIOCGIFNETMASK);

                if (if_net_addr == NULL || if_net_mask == NULL) {
                    continue;
                }

                strncpy(ifnst->if_addr[ifnst->if_nifs], if_get_ifnet_ipaddr(ifnst->if_name[ifnst->if_nifs], SIOCGIFADDR), IF_NAMESIZE);
                strncpy(ifnst->if_mask[ifnst->if_nifs], if_get_ifnet_ipaddr(ifnst->if_name[ifnst->if_nifs], SIOCGIFNETMASK), IF_NAMESIZE);
                ifnst->if_nifs++;
                intf++;
            } else {
                break;
            }
        }

        if_freenameindex(if_nidxs);
    } else {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to retrieve network interfaces:", errno);
        log_write_trace("get_ifnet_names()", "if_nameindex()");
        return EXIT_FAILURE;
    }

    nodecfg.pifnets = ifnst;

    return EXIT_SUCCESS;
}

/**
 * @brief Retrieves IP address of specified interface's property 
 * 
 * @param if_name       - interface to retrieve address from
 * @param if_flag       - type of property to get address of 
 * [`SIOCGIFNETMASK` = netmask] || [`SIOCGIFADDR` = interface IPv4]
 * @return 
 * IP address string on success
 * `NULL` on error
 */
char *if_get_ifnet_ipaddr(char *if_name, unsigned long if_flag) {
    if (if_name == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("get_ifnet_ipaddr()", "if_name");
        return NULL;
    }
    
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
        /* error */
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to open socket:", errno);
        log_write_trace("get_ifnet_ipaddr()", "socket()");
        return NULL;
    }

    /* Only target IPv4 interfaces */
    ifr.ifr_addr.sa_family = AF_INET;

    /* load desired interface name into ifreq request struct */
    strncpy(ifr.ifr_name, if_name, NET_IPV4_MAX);

    int ioctl_ret =  ioctl(fd, if_flag, &ifr);
    if (ioctl_ret != EXIT_SUCCESS) {
        NODE_SETLASTERR(CURRENT_ADDR, "I/O control operation failed:", errno);
        log_write_trace("get_ifnet_ipaddr()", "ioctl()");
        return NULL;
    }

    close(fd);

    /* return ipaddr result */
    return (inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}