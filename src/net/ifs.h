#ifndef _CNODE_NETIF_CTL_H_
#define _CNODE_NETIF_CTL_H_

#include <net/if.h>                                 /* netdevice */

#ifdef IFNAMESIZ                                    /* old kernels */
    #define IF_NAMESIZE IFNAMSIZ
#endif

#define NET_IFNET_NMAX          32                  /* maximum number of interface names storable in sys_ifnets struct */
#define NET_IPV4_MAX            (IF_NAMESIZE - 1)   /* max IPv4 strlen */

#define IF_REQUEST_IFADDR       SIOCGIFADDR         /* get_ifnet_ipaddr() if_flag for retrieving interface ip address */
#define IF_REQUEST_NETMASK      SIOCGIFNETMASK      /* get_ifnet_ipaddr() if_flag for retrieving interface netmask */

#define IF_DEV_MAXLEN           (IF_NAMESIZE - 1)

#define IF_DEV_TUN_DEFAULT      "tun0"              /* default tun interface */
#define IF_DEV_TUN_DEFAULT_PATH "/dev/net/tun"      /* default tun interface character device path */

#define PROXY_MAXLEN            96                  /* socks 5 proxy maxlen */

/* rewrite this as linked list please, thank you */
struct sys_ifnets {                                 /* struct stores information and names of available net interfaces on the system */
    int  if_nifs;                                   /* Number of interfaces found */
    char if_name[NET_IFNET_NMAX][IF_NAMESIZE];      /* Names of interfaces found (max NET_IFNET_NMAX interfaces) */
    char if_addr[NET_IFNET_NMAX][IF_NAMESIZE];      /* IPv4 address of interfaces */
    char if_mask[NET_IFNET_NMAX][IF_NAMESIZE];      /* netmask of interfaces */
};

/**
 * @brief Retrieves all available network interface names and saves them in sys_ifnets struct
 * 
 * @param ifnst         - pointer to system network interfaces list struct
 * @return EXIT_SUCCESS || EXIT_FAILURE 
 */
int if_get_ifnet_names(struct sys_ifnets *ifnst);

#endif