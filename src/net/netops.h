#ifndef _CNODE_NETOPS_H_
#define _CNODE_NETOPS_H_

#include "networks.h"
#include "../base/cnode.h"


#define NET_URL_MAX         128      /* max length of network url */

/* network op ids */
#define NETOP_INVALID       0
#define NETOP_GET_NETWORKS  1
#define NETOP_GET_UUID      2

/**
 * @brief Format network url based on operation ID
 * 
 * @param netop         - net operation ID
 * @param output_url    - preallocated output url string
 * @return 
 * EXIT_SUCCESS 
 * EXIT_FAILURE         - output_url is also set to NULL
 */
int net_format_url(int netop, char *output_url);

/**
 * @brief orts and indexes available networks from unformatted server-returned network list.
 * Also determines status/availability of remote network repository.
 * 
 * @return Total number of formatted networks 
 */
int net_get_networks(void);

/**
 * @brief Checks for available Central Node updates 
 * 
 * @return true     - Update available
 * @return false    - No update available or an error has occured
 */
bool net_checkupdate(void);

/**
 * @brief Finalizes nodecfg.network_info struct, frees memory
 */
void network_info_final(network_info_st **netinfo);

#endif