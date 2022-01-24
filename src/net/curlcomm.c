#include "../base/cnode.h"

#include <curl/curl.h>
#include <string.h>

struct curl_memchunk {
    char *response;
    size_t size;
};

int curl_node_init(void) {
    int retcode; 
    if ((retcode = curl_global_init(CURL_GLOBAL_DEFAULT)) != CURLE_OK) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to initialize cURL instance:", retcode);
        log_write_trace("curl_node_init()", "curl_global_init()");
        return EXIT_FAILURE;
    }

    log_write(LOG_DEBUG, "Initialized global cURL instance");

    return EXIT_SUCCESS;
}

void curl_node_final(void) {
    curl_global_cleanup();
}

static size_t curl_writefunc(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct curl_memchunk *mem = (struct curl_memchunk *) userp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(ptr == NULL) { 
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to reallocate memory:", errno);
        log_write_trace("curl_write_callback()", "realloc()");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

int curl_sendrequest(char *curl_http_desturl, char *strout, size_t max_read_sz, size_t *strout_sz) {
    if (curl_http_desturl == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, NODE_NOINFO, ENULLPTR);
        log_write_trace("curl_sendrequest()", "curl_http_desturl");
        return EXIT_FAILURE;
    }

    CURL                   *curl_instance;                  /* cURL instance */
    CURLcode                curl_res_code;                  /* cURL result code */
    long                    error_code = EXIT_SUCCESS;      /* http error code */
    int                     return_code = EXIT_SUCCESS;     /* function return value */
    struct curl_memchunk    curlstr = { 0 };

    /* initialize cURL instance */
    curl_instance = curl_easy_init();

    if (curl_instance == NULL) {
        NODE_SETLASTERR(CURRENT_ADDR, "Unable to initialize new cURL instance:", ECURLINIT);
        log_write_trace("curl_sendrequest()", "curl_instance");
        return EXIT_FAILURE;
    }

    curl_easy_setopt(curl_instance, CURLOPT_SSL_VERIFYPEER, (long) !nodecfg.net_nohttps);
    curl_easy_setopt(curl_instance, CURLOPT_SSL_VERIFYHOST, (long) !nodecfg.net_nohttps);

    if (strncmp(nodecfg.proxyaddr, "NULL", 4) != 0) {
        /* socks5 proxy detected */
        curl_easy_setopt(curl_instance, CURLOPT_PROXY, nodecfg.proxyaddr);
    }

    /* set destination url */
    curl_easy_setopt(curl_instance, CURLOPT_URL, curl_http_desturl);

    /* fail on server error */
    curl_easy_setopt(curl_instance, CURLOPT_FAILONERROR, 1L);

    /* set timeout in seconds */
    curl_easy_setopt(curl_instance, CURLOPT_TIMEOUT, nodecfg.net_timeout);

    /* send all data to callback function */
    curl_easy_setopt(curl_instance, CURLOPT_WRITEFUNCTION, curl_writefunc);
    
    /* pass memchunk struct to the callback function */
    curl_easy_setopt(curl_instance, CURLOPT_WRITEDATA, (void *) &curlstr);


    /* perform request */
    curl_res_code = curl_easy_perform(curl_instance);
    curl_easy_getinfo(curl_instance, CURLINFO_RESPONSE_CODE, &error_code);

    log_write(LOG_DEBUG, "cURL result code: %#02x", curl_res_code);
    
    switch (curl_res_code) {
        case CURLE_OK:
            /* 200 - okay */
            log_write(LOG_DEBUG, "CURL_STRDATA: \n---\n%s---", curlstr.response);
            size_t cpysz = max_read_sz == 0 ? curlstr.size : max_read_sz;
            memcpy(strout, curlstr.response, cpysz);
            if (strout_sz != NULL) {
                *strout_sz = curlstr.size;
            }
            break;
        
        default:
            /* evaluate specific network error code */
            if (error_code >= 400 && error_code <= 599) {
                /* Client error & server error - continue execution */
                log_write(LOG_ERROR, "Remote repository returned HTTP error %ld", error_code);
                return_code = EXIT_FAILURE;
            } /* else if (error_code >= 500 ) {
                / * Server error - continue execution * /
                log_write(LOG_ERROR, "Remote repository returned HTTP error %ld", error_code);
                return_code = CNODE_EXECUTION_CONTINUE;
            }*/
            else {
                /* Unhandled exception */
                log_write(LOG_ERROR, "An unknown error has occured trying to communicate with remote repository");
                log_write(LOG_DEBUG, "\nTRACE: HTTP_RES='%ld'; cURL_RES='%ld'", error_code, curl_res_code);
                return_code = EXIT_FAILURE;
            }
            break;
    }


    /* cleanup */
    if (curlstr.response != NULL) {
        free(curlstr.response);
    }

    curl_easy_cleanup(curl_instance);

    return return_code;
}