#ifndef _CNODE_NETWORK_INFO_H_
#define _CNODE_NETWORK_INFO_H_

#include <stdint.h>

#define NETWORKS_MAX            UINT16_MAX
#define NETWORK_NAME_MAX        64
#define NETREPO_NAME_MAX        64
#define NEWORK_LOGO_BYTES_MAX   64

#define NETWORK_ID_INVALID      0

typedef unsigned short  network_id_t;
typedef uint64_t        network_uuid_t;

typedef enum _NODE_NETWORK_STATUS {
    NETWORK_STATUS_ERROR            = -1,
    NETWORK_STATUS_UNINITIALIZED    =  0,
    NETWORK_STATUS_OFFLINE          =  0,
    NETWORK_STATUS_ONLINE           =  1
} NODE_NETWORK_STATUS;

struct node_network_info {
    char                       *network_name;   /* network name */
    unsigned char              *network_logo;   /* network ascii banner */
    network_id_t                net_local_id;   /* network ID */
    network_uuid_t              net_uuid;       /* network UUID */
    NODE_NETWORK_STATUS         network_status; /* network status */ 
    struct node_network_info   *next;           /* linked list */
};

typedef struct node_network_info network_info_st;

#endif