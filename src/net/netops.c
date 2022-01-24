#define _XOPEN_SOURCE 700           /* fmemopen() */
#include "netops.h"
#include "curlcomm.h"
#include "../data/data.h"

#include <stdio.h>
#include <string.h>

static int network_info_entry_add(network_info_st **netinfo, network_id_t net_local_id, const char *network_name) {
    if (NULL == netinfo) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("network_info_entry_add", "netinfo");
        return EXIT_FAILURE;
    }

    network_info_st *last = NULL;
    
    if (NULL != *netinfo) {
        /* already init'd, go to the end of the list */
        while (*netinfo) {
            last = *netinfo;
            netinfo = &(*netinfo)->next;
        }
    }

    *netinfo = malloc(sizeof(network_info_st));
    if (NULL == *netinfo) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to allocate memory:", errno);
        log_write_trace("network_info_entry_add", "*netinfo");
        return EXIT_FAILURE;
    }

    (*netinfo)->next = NULL;
    (*netinfo)->net_uuid = 0;                   /* to be implemented */
    (*netinfo)->network_logo = NULL;            /* to be implemented */
    (*netinfo)->net_local_id = net_local_id;
    (*netinfo)->network_name = strdup(network_name);
    (*netinfo)->network_status = (NULL == network_name) ? NETWORK_STATUS_ERROR : NETWORK_STATUS_ONLINE;

    if (NULL != last) {
        last->next = *netinfo;
    }

    return EXIT_SUCCESS;
}

void network_info_final(network_info_st **netinfo) {
    if (netinfo == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("network_info_final()", "network_info_st **ninfo");
        return;
    }

    network_info_st *tmp_swap = NULL;

    while (*netinfo) {
        log_write(LOG_DEBUG, "Freeing memory for network ID: %hu ['%s']", (*netinfo)->net_local_id, (*netinfo)->network_name);
        free((*netinfo)->network_name);

        tmp_swap = *netinfo;

        *netinfo = (*netinfo)->next;
        free(tmp_swap);
    }

    *netinfo = NULL;
}

int net_format_url(int netop, char *output_url) {
    if (output_url == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("net_format_url()", "output_url");
        return EXIT_FAILURE;
    }

    switch (netop) {
        case NETOP_GET_NETWORKS:
            ; char *http_sec = (nodecfg.net_nohttps == false) ? "https://" : "http://";
            snprintf(output_url, NET_URL_MAX, "%s%s/router/getnetworks", http_sec, nodecfg.netrepo);
            break;

        default:
            NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, EINVAL);
            log_write_trace("net_format_url()", "netop");
            output_url = NULL;
            return EXIT_FAILURE;
            break;
    }

    return EXIT_SUCCESS;
}

int net_get_networks(void) {
    /* Fetch network list */
    size_t buffer_len;                              /* curl output str len */
    char curl_netlist[BUFSIZ];                      /* unformatted output */
    char net_cmd_getnet_url[NET_URL_MAX] = { 0 };   /* net destination url */
    
    if (net_format_url(NETOP_GET_NETWORKS, net_cmd_getnet_url) != EXIT_SUCCESS) {
        return 0;
    }

    int ret = curl_sendrequest(net_cmd_getnet_url, curl_netlist, 0, &buffer_len);

    if (ret != EXIT_SUCCESS) {
        /*if (ret == CNODE_EXECUTION_CONTINUE) {}*/
        /* network repo is unavailable */
        nodecfg.netrepo_status = false;
        return 0;
    } else {
        /* Repo(man) Online */
        nodecfg.netrepo_status = true;
    }
    

    size_t num_networks = 0;

    if (curl_netlist == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("net_get_networks()", "net_get_networks");
        return 0;
    }

    FILE *fbp = fmemopen(curl_netlist, buffer_len + 1, "r");

    if (fbp == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to map memory for reading:", errno);
        log_write_trace("net_get_networks()", "fmemopen() => fbp");
        return 0;
    }

    char tmp_network_name[NETWORK_NAME_MAX] = { 0 };
    while (fgets(tmp_network_name, NETWORK_NAME_MAX, fbp)) {
        size_t slen = strlen(tmp_network_name);

        if (slen > NETWORK_NAME_MAX - 1 || slen < 1) {
            /* This should never happen, but whatever */
            continue;
        }
        tmp_network_name[strcspn(tmp_network_name, "\n")] = 0;
            
        if (slen > nodecfg.longest_net_entry) {
        size_t next_mul = rtonm(nodecfg.longest_net_entry + slen, 8);
            if (next_mul > nodecfg.longest_net_entry) {
                nodecfg.longest_net_entry = next_mul;
            }
        }

        log_write(LOG_DEBUG, "Evaluating network: %hu", num_networks);
        network_info_entry_add(&nodecfg.network_info, num_networks, tmp_network_name);
        num_networks++;

        memset(tmp_network_name, 0, sizeof(tmp_network_name));
    }


    fclose(fbp);
    return num_networks;
}

bool net_checkupdate(void) {
    char version_out[6];
    if (curl_sendrequest("https://raw.githubusercontent.com/x0reaxeax/CentralNode-Status/main/version", version_out, 6, NULL) != EXIT_SUCCESS) {
        log_write(LOG_ERROR, "Unable to retrieve update status");
        return false;
    }

    if (version_out[0] != 'X') {
        log_write(LOG_ERROR, "Invalid remote version data");
        return false;
    }

    if (atoi(&version_out[1]) > CNODE_BUILD_VERSION) {
        log_write(LOG_NOTIF, "Update available: v%d", atoi(&version_out[1]));
        return true;
    }

    log_write(LOG_DEBUG, "No updates available (%d == %u)", atoi(&version_out[1]), CNODE_BUILD_VERSION);
    return false;
}