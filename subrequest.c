/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_subrequest.h"
#include "connection.h"
#include <sys/epoll.h>



static int le_subrequest;

zend_function_entry subrequest_functions[] = {
	PHP_FE(subreq_set_domain, NULL)
    PHP_FE(subreq_set_param, NULL)
    PHP_FE(subrequest, NULL)
    PHP_FE(subreq_close, NULL)
	{NULL, NULL, NULL}
};


zend_module_entry subrequest_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"subrequest",
	subrequest_functions,
	PHP_MINIT(subrequest),
	PHP_MSHUTDOWN(subrequest),
	PHP_RINIT(subrequest),	
	PHP_RSHUTDOWN(subrequest),
	PHP_MINFO(subrequest),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_SUBREQUEST
ZEND_GET_MODULE(subrequest)
#endif





PHP_MINIT_FUNCTION(subrequest)
{
	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(subrequest)
{
	return SUCCESS;
}



PHP_RINIT_FUNCTION(subrequest)
{
	return SUCCESS;
}



PHP_RSHUTDOWN_FUNCTION(subrequest)
{
	return SUCCESS;
}


PHP_MINFO_FUNCTION(subrequest)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "subrequest support", "enabled");
	php_info_print_table_end();

}

#define MAX_EVENTS 10
#define TIME_OUT 10000
char *host;
__uint16_t port;

char ***keys_dict;
char ***vals_dict;
char **content_dict;
int len_dict[MAX_EVENTS];
int count_request = 0;

int g_epoll_fd;
int g_req_count = 0;

struct myevent_s
{
	int fd;
	unsigned short request_id;
};

PHP_FUNCTION(subreq_set_domain)
{
    char *host_param;
    int host_len_param;
    long port_param;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &host_param, &host_len_param, &port_param)) {
        return;
    }
    host = (char *)emalloc(sizeof(char)*strlen(host_param));
    strcpy(host, host_param);
    port = port_param;

    keys_dict = (char ***)emalloc(sizeof(char **) * MAX_EVENTS);
    vals_dict = (char ***)emalloc(sizeof(char **) * MAX_EVENTS);
    content_dict = (char **)emalloc(sizeof(char *) * MAX_EVENTS);
    
    g_epoll_fd = epoll_create(MAX_EVENTS);

    RETURN_BOOL(1);
}


int
subreq_run(int id)
{
    unsigned short request_id = id;
    int sock = INVALID_SOCKET;
    int connect_ret = connect_to_fpm(host, port, &sock);
    if (connect_ret) {
        return 1;
    }
    ssize_t begin_ret = begin_send(&sock, request_id);
    if (!begin_ret) {
        close(sock);
        return 2;
    }
    ssize_t send_env_ret = send_env(&sock, *(keys_dict+id), *(vals_dict+id), len_dict[id], request_id);
    if (!send_env_ret) {
        close(sock);
        return 3;
    }
    ssize_t send_content_ret = send_content(&sock, *(content_dict+id), request_id);
    if (!send_content_ret) {
        return 4;
    }

	struct myevent_s *ev;
	struct epoll_event epv = {0, {0}};
	epv.events = EPOLLIN;
	ev = emalloc(sizeof(struct epoll_event));
	ev->fd = sock;
	ev->request_id = request_id;
	epv.data.ptr = ev;
	epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, sock, &epv);

	return 0;
}

PHP_FUNCTION(subreq_set_param)
{
    zval *z_array;
    char *content_param;
    int content_len_param;
    int count, i;
    zval **z_item;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "as", &z_array, &content_param, &content_len_param)) {
        return;
    }
    count = zend_hash_num_elements(Z_ARRVAL_P(z_array));

    *(keys_dict+count_request) = (char **)emalloc(sizeof(char *)*count);
    *(vals_dict+count_request) = (char **)emalloc(sizeof(char *)*count);

    zend_hash_internal_pointer_reset(Z_ARRVAL_P(z_array));
    for (i = 0; i < count; i ++) {
        char *key;
        ulong idx;
        zend_hash_get_current_data(Z_ARRVAL_P(z_array), (void**) &z_item);
        convert_to_string_ex(z_item);
        char *val_tmp = Z_STRVAL_PP(z_item);
        
        if (zend_hash_get_current_key(Z_ARRVAL_P(z_array), &key, &idx, 0) == HASH_KEY_IS_STRING) 
        {
            *(*(keys_dict+count_request)+i) = (char *)emalloc(sizeof(char)*strlen(key));
            memset(*(*(keys_dict+count_request)+i), '\0', sizeof(char)*strlen(key));
            strcpy(*(*(keys_dict+count_request)+i), key);
        }
        else
        {
            *(*(keys_dict+count_request)+i) = (char *)emalloc(sizeof(char)*strlen("unkown"));
            memset(*(*(keys_dict+count_request)+i), '\0', sizeof(char)*strlen("unkown"));
            strcpy(*(*(keys_dict+count_request)+i), "unkown");
        }
        *(*(vals_dict+count_request)+i) = (char *)emalloc(sizeof(char)*strlen(val_tmp)+1);
        memset(*(*(vals_dict+count_request)+i), '\0', sizeof(char)*strlen(val_tmp)+1);
        strcpy(*(*(vals_dict+count_request)+i), val_tmp);
        zend_hash_move_forward(Z_ARRVAL_P(z_array));
    }

    if (count_request >= 10) {
        RETURN_BOOL(0);
    }
    *(content_dict+count_request) = (char *)emalloc(sizeof(char)*content_len_param);
    strcpy(*(content_dict+count_request), content_param);
    len_dict[count_request] = count;
	int request_ret = subreq_run(count_request);
	count_request++;
	if(request_ret == 0){
		g_req_count++;
		RETURN_BOOL(1);
	}
	else{
		RETURN_BOOL(0);
	}
    
}

char *
recv_subrequest(int sock, unsigned short request_id)
{
    unsigned short content_len;
    ssize_t recv_header_ret = recv_header(&sock, request_id, &content_len);
    if (!recv_header_ret) {
        pthread_exit((void *)5);
    }
    char *content = (char *)emalloc(sizeof(char)*content_len);
    memset(content, '\0', sizeof(char)*content_len);
    ssize_t recv_ret = recv_content(&sock, request_id, content_len, content);
    struct epoll_event epv = {0, {0}};
    epoll_ctl(g_epoll_fd, EPOLL_CTL_DEL, sock, &epv);
    close(sock);
    g_req_count--;
    return content;
}

PHP_FUNCTION(subrequest)
{
	struct epoll_event events[MAX_EVENTS];
	int mfds = 0;
	int i;
	if(g_req_count <= 0)
	{
		RETURN_BOOL(0);
	}
	mfds = epoll_wait(g_epoll_fd, events, MAX_EVENTS, TIME_OUT);
	for(i = 0; i < mfds; i++){
		struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;
		char *content;
		content = recv_subrequest(ev->fd, ev->request_id);
		RETVAL_STRING(content, 1);
		efree(content);
		return;
	}
}


PHP_FUNCTION(subreq_close)
{
    efree(host);

    int loop;
    for(loop=0; loop<count_request; loop++)
    {
        int array_count;
        for(array_count=0;array_count<len_dict[loop];array_count++)
        {
            efree(*(*(keys_dict+loop)+array_count));
            efree(*(*(vals_dict+loop)+array_count));
        }
        efree(*(keys_dict+loop));
        efree(*(vals_dict+loop));
        efree(*(content_dict+loop));
    }

    efree(keys_dict);
    efree(vals_dict);
    efree(content_dict);
    count_request=0;
}




