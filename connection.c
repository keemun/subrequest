//
//  connection.c
//  fastcgi
//
//  Created by keemun on 14-5-30.
//  Copyright (c) 2014年 keemun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include "php.h"
#include "connection.h"

int
connect_to_fpm(const char *host, __uint16_t port, int *sock)
{
    struct addrinfo hints;
	struct addrinfo *ainfo, *rp;
    int s;
    int rc=-1;
    
    memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = PF_UNSPEC;
	hints.ai_flags = AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;
    s = getaddrinfo(host, NULL, &hints, &ainfo);
    if(s)
    {
        return s;
    }
    
    for(rp = ainfo; rp != NULL; rp = rp->ai_next)
    {
        *sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(*sock == INVALID_SOCKET)
        {
            continue;
        }
        
        if(rp->ai_family == PF_INET)
        {
			((struct sockaddr_in *)rp->ai_addr)->sin_port = htons(port);
	}
        else if(rp->ai_family == PF_INET6)
        {
			((struct sockaddr_in6 *)rp->ai_addr)->sin6_port = htons(port);
	}
        else
        {
			continue;
	}
        
        rc = connect(*sock, rp->ai_addr, rp->ai_addrlen);
        if(rc == 0 || errno == EINPROGRESS || errno == COMPAT_EWOULDBLOCK)
        {
            break;
        }
    }
    
    
    freeaddrinfo(ainfo);
    return rc;
}


ssize_t
begin_send(int *sock, unsigned short request_id)
{
    FCGI_BeginRequestBody *begin_body = (FCGI_BeginRequestBody *)emalloc(sizeof(FCGI_BeginRequestBody));
    memset(begin_body, '0', sizeof(FCGI_BeginRequestBody));
    fcgi_header_twobyte_set(begin_body, role, RESPONDER);
    begin_body->flags = 0;
    
    unsigned char padding_length;
    unsigned short content_lenth;
    content_lenth = sizeof(FCGI_BeginRequestBody);
    padding_length = content_lenth % 8;
    padding_length = padding_length == 0 ? 0 : 8 - padding_length;
   
    size_t record_len = sizeof(FCGI_Record)+content_lenth+padding_length;
    FCGI_Record *begin_record = (FCGI_Record *)emalloc(record_len);
    memset(begin_record, '0', record_len);
    
    begin_record->version = VERSION_1;
    begin_record->type = BEGIN_REQUEST;
    fcgi_header_twobyte_set(begin_record, requestId, request_id);
    fcgi_header_twobyte_set(begin_record, contentLength, content_lenth);
    begin_record->paddingLength = padding_length;
    begin_record->reserved = 0;
    memcpy(begin_record->contentAndPaddingData, begin_body, content_lenth);
    
    efree(begin_body);
    begin_body = NULL;
    
    ssize_t send_ret = send(*sock, begin_record, sizeof(FCGI_Record)+content_lenth+padding_length, 0);
    
    efree(begin_record);
    begin_record = NULL;
    
    return send_ret;
}


ssize_t
send_env(int *sock, char **keys, char **vals, int len, unsigned short request_id)
{
    unsigned long key_len;
    unsigned long val_len;
    unsigned char padding_length;
    unsigned short content_lenth;
    char *tmp_name_value;
    ssize_t send_ret = 0;
    FCGI_NameValuePair11 *env_body11;
    FCGI_NameValuePair14 *env_body14;
    FCGI_NameValuePair41 *env_body41;
    FCGI_NameValuePair44 *env_body44;
    FCGI_Record *env_record;
    int i;
    for (i=0; i<len; i++) {
        key_len = strlen(*(keys+i));
        val_len = strlen(*(vals+i));
        if (key_len < 127) {
            if (val_len < 127) {
                tmp_name_value = emalloc(sizeof(char)*(key_len+val_len));
                memset(tmp_name_value, 0, sizeof(char)*(key_len+val_len));
                strncpy(tmp_name_value, *(keys+i), key_len);
                strncpy(tmp_name_value+key_len, *(vals+i), val_len);
                env_body11 = (FCGI_NameValuePair11 *)emalloc(sizeof(FCGI_NameValuePair11)+key_len+val_len);
                memset(env_body11, 0, sizeof(FCGI_NameValuePair11)+key_len+val_len);
                env_body11->nameLengthB0 = (unsigned char)key_len;
                env_body11->valueLengthB0 = (unsigned char)val_len;
                memcpy(env_body11->nameAndValueData, tmp_name_value, key_len+val_len);
                efree(tmp_name_value);
                tmp_name_value = NULL;
                content_lenth = sizeof(FCGI_NameValuePair11)+key_len+val_len;
                padding_length = content_lenth % 8;
                padding_length = padding_length == 0 ? 0 : 8 - padding_length;
                
                env_record = (FCGI_Record *)emalloc(sizeof(FCGI_Record)+content_lenth+padding_length);
                memset(env_record, 0, sizeof(FCGI_Record)+content_lenth+padding_length);
                env_record->version = VERSION_1;
                env_record->type = PARAMS;
                fcgi_header_twobyte_set(env_record, requestId, request_id);
                fcgi_header_twobyte_set(env_record, contentLength, content_lenth);
                env_record->paddingLength = padding_length;
                env_record->reserved = 0;
                memcpy(env_record->contentAndPaddingData, env_body11, content_lenth);
                if (env_body11) {
                    efree(env_body11);
                    env_body11 = NULL;
                }
                
                send_ret = send(*sock, env_record, sizeof(FCGI_Record)+content_lenth+padding_length, 0);
                if (env_record) {
                    efree(env_record);
                    env_record = NULL;
                }
            }
            else {
                tmp_name_value = emalloc(sizeof(char)*(key_len+val_len));
                memset(tmp_name_value, 0, sizeof(char)*(key_len+val_len));
                strncpy(tmp_name_value, *(keys+i), key_len);
                strncpy(tmp_name_value+key_len, *(vals+i), val_len);
                
                env_body14 = (FCGI_NameValuePair14 *)emalloc(sizeof(FCGI_NameValuePair14)+key_len+val_len);
                memset(env_body14, 0, sizeof(FCGI_NameValuePair14)+key_len+val_len);
                env_body14->nameLengthB0 = (unsigned char)key_len;
                env_body14->valueLengthB0 = (unsigned char)val_len;
                memcpy(env_body14->nameAndValueData, tmp_name_value, key_len+val_len);
                efree(tmp_name_value);
                tmp_name_value = NULL;
                
                content_lenth = sizeof(FCGI_NameValuePair14)+key_len+val_len;
                padding_length = content_lenth % 8;
                padding_length = padding_length == 0 ? 0 : 8 - padding_length;
                
                env_record = (FCGI_Record *)emalloc(sizeof(FCGI_Record)+content_lenth+padding_length);
                memset(env_record, 0, sizeof(FCGI_Record)+content_lenth+padding_length);
                env_record->version = VERSION_1;
                env_record->type = PARAMS;
                fcgi_header_twobyte_set(env_record, requestId, request_id);
                fcgi_header_twobyte_set(env_record, contentLength, content_lenth);
                env_record->paddingLength = padding_length;
                env_record->reserved = 0;
                memcpy(env_record->contentAndPaddingData, env_body14, content_lenth);
                if (env_body14) {
                    efree(env_body14);
                    env_body14 = NULL;
                }
                
                send_ret = send(*sock, env_record, sizeof(FCGI_Record)+content_lenth+padding_length, 0);
                if (env_record) {
                    efree(env_record);
                    env_record = NULL;
                }
            }
        }
        else {
            if (val_len < 127) {
                tmp_name_value = emalloc(sizeof(char)*(key_len+val_len));
                memset(tmp_name_value, 0, sizeof(char)*(key_len+val_len));
                strncpy(tmp_name_value, *(keys+i), key_len);
                strncpy(tmp_name_value+key_len, *(vals+i), val_len);
                
                env_body41 = (FCGI_NameValuePair41 *)emalloc(sizeof(FCGI_NameValuePair41)+key_len+val_len);
                memset(env_body41, 0, sizeof(FCGI_NameValuePair41)+key_len+val_len);
                env_body41->nameLengthB0 = (unsigned char)key_len;
                env_body41->valueLengthB0 = (unsigned char)val_len;
                memcpy(env_body41->nameAndValueData, tmp_name_value, key_len+val_len);
                efree(tmp_name_value);
                tmp_name_value = NULL;
                
                content_lenth = sizeof(FCGI_NameValuePair11)+key_len+val_len;
                padding_length = content_lenth % 8;
                padding_length = padding_length == 0 ? 0 : 8 - padding_length;
                
                env_record = (FCGI_Record *)emalloc(sizeof(FCGI_Record)+content_lenth+padding_length);
                memset(env_record, 0, sizeof(FCGI_Record)+content_lenth+padding_length);
                env_record->version = VERSION_1;
                env_record->type = PARAMS;
                fcgi_header_twobyte_set(env_record, requestId, request_id);
                fcgi_header_twobyte_set(env_record, contentLength, content_lenth);
                env_record->paddingLength = padding_length;
                env_record->reserved = 0;
                memcpy(env_record->contentAndPaddingData, env_body41, content_lenth);
                if (env_body41) {
                    efree(env_body41);
                    env_body41 = NULL;
                }
                
                send_ret = send(*sock, env_record, sizeof(FCGI_Record)+content_lenth+padding_length, 0);
                if (env_record) {
                    efree(env_record);
                    env_record = NULL;
                }
            }
            else {
                tmp_name_value = emalloc(sizeof(char)*(key_len+val_len));
                memset(tmp_name_value, 0, sizeof(char)*(key_len+val_len));
                strncpy(tmp_name_value, *(keys+i), key_len);
                strncpy(tmp_name_value+key_len, *(vals+i), val_len);
                
                env_body44 = (FCGI_NameValuePair44 *)emalloc(sizeof(FCGI_NameValuePair44)+key_len+val_len);
                memset(env_body44, 0, sizeof(FCGI_NameValuePair44)+key_len+val_len);
                env_body44->nameLengthB0 = (unsigned char)key_len;
                env_body44->valueLengthB0 = (unsigned char)val_len;
                memcpy(env_body44->nameAndValueData, tmp_name_value, key_len+val_len);
                efree(tmp_name_value);
                tmp_name_value = NULL;
                
                content_lenth = sizeof(FCGI_NameValuePair11)+key_len+val_len;
                padding_length = content_lenth % 8;
                padding_length = padding_length == 0 ? 0 : 8 - padding_length;
                
                env_record = (FCGI_Record *)emalloc(sizeof(FCGI_Record)+content_lenth+padding_length);
                memset(env_record, 0, sizeof(FCGI_Record)+content_lenth+padding_length);
                env_record->version = VERSION_1;
                env_record->type = PARAMS;
                fcgi_header_twobyte_set(env_record, requestId, request_id);
                fcgi_header_twobyte_set(env_record, contentLength, content_lenth);
                env_record->paddingLength = padding_length;
                env_record->reserved = 0;
                memcpy(env_record->contentAndPaddingData, env_body44, content_lenth);
                if (env_body44) {
                    efree(env_body44);
                    env_body44 = NULL;
                }
                
                send_ret = send(*sock, env_record, sizeof(FCGI_Record)+content_lenth+padding_length, 0);
                if (env_record) {
                    efree(env_record);
                    env_record = NULL;
                }
            }
        }
    }
    return send_ret;
}

ssize_t
send_content(int *sock, char *content, unsigned short request_id)
{
    unsigned char padding_length;
    unsigned short content_lenth;
    content_lenth = strlen(content);
    padding_length = content_lenth % 8;
    padding_length = padding_length == 0 ? 0 : 8 - padding_length;
    
    FCGI_Record *content_record = (FCGI_Record *)emalloc(sizeof(FCGI_Record)+content_lenth+padding_length);
    memset(content_record, 0, sizeof(FCGI_Record)+content_lenth+padding_length);
    content_record->version = VERSION_1;
    content_record->type = STDIN;
    fcgi_header_twobyte_set(content_record, requestId, request_id);
    fcgi_header_twobyte_set(content_record, contentLength, content_lenth);
    content_record->paddingLength = padding_length;
    content_record->reserved = 0;
    memcpy(content_record->contentAndPaddingData, content, content_lenth);
    
    ssize_t send_ret = send(*sock, content_record, sizeof(FCGI_Record)+content_lenth+padding_length, 0);
    if (content_record) {
        efree(content_record);
        content_record = NULL;
    }
    return send_ret;
}

ssize_t
recv_header(int *sock, unsigned short request_id, unsigned short *header_long)
{
    FCGI_Record *recv_record = (FCGI_Record *)emalloc(sizeof(FCGI_Record));
    memset(recv_record, 0, sizeof(FCGI_Record));
    ssize_t recv_ret = recv(*sock, recv_record, sizeof(FCGI_Record), 0);
    *header_long = fcgi_header_twobyte_get(recv_record, contentLength);
    efree(recv_record);
    recv_record = NULL;
    return recv_ret;
}

ssize_t
recv_content(int *sock, unsigned short request_id, unsigned short content_len, char *content)
{
    ssize_t recv_ret = recv(*sock, content, content_len, 0);
    return recv_ret;
}

ssize_t
send_end(int *sock, unsigned short request_id)
{
    FCGI_EndRequestBody *end_body = (FCGI_EndRequestBody *)emalloc(sizeof(FCGI_EndRequestBody));
    memset(end_body, '0', sizeof(FCGI_EndRequestBody));
    fcgi_header_fourbyte_set(end_body, appStatus, REQ_STATE_OK);
    end_body->protocolStatus = REQUEST_COMPLETE;
    
    unsigned char padding_length;
    unsigned short content_lenth;
    content_lenth = sizeof(FCGI_EndRequestBody);
    padding_length = content_lenth % 8;
    padding_length = padding_length == 0 ? 0 : 8 - padding_length;
    
    size_t record_len = sizeof(FCGI_Record)+content_lenth+padding_length;
    FCGI_Record *end_record = (FCGI_Record *)emalloc(record_len);
    memset(end_record, '0', record_len);
    
    end_record->version = VERSION_1;
    end_record->type = END_REQUEST;
    fcgi_header_twobyte_set(end_record, requestId, request_id);
    fcgi_header_twobyte_set(end_record, contentLength, content_lenth);
    end_record->paddingLength = padding_length;
    end_record->reserved = 0;
    memcpy(end_record->contentAndPaddingData, end_body, content_lenth);
    
    efree(end_body);
    end_body = NULL;
    
    ssize_t send_ret = send(*sock, end_record, sizeof(FCGI_Record)+content_lenth+padding_length, 0);
    
    efree(end_record);
    end_record = NULL;
    
    return send_ret;
}


