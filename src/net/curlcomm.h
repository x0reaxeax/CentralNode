#ifndef _CNODE_CURL_CALLBACKS_H_
#define _CNODE_CURL_CALLBACKS_H_

/**
 * @brief Initializes global cURL instance
 * 
 * @return EXIT_SUCCESS || EXIT_FAILURE 
 */
int curl_node_init(void);


/**
 * @brief Finalizes and frees global cURL instance
 * 
 */
void curl_node_final(void);

/**
 * @brief Sends cURL request to set URL and saves str output
 * 
 * @param curl_http_desturl     - destination URL
 * @param strout                - output buffer
 * @param max_read_sz           - max number of bytes to write into `char *strout`
 * @param strout_sz             - output buffer for read length, can be `NULL`
 * @return 
 * EXIT_SUCCESS
 * EXIT_FAILURE
 * CNODE_EXECUTION_CONTINUE     - Unable to retrieve info from remote repo, continue execution
 */
int curl_sendrequest(char *curl_http_desturl, char *strout, size_t max_read_sz, size_t *strout_sz);

#endif