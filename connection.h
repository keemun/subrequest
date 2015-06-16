//
//  connection.h
//  fastcgi
//
//  Created by keemun on 14-5-30.
//  Copyright (c) 2014年 keemun. All rights reserved.
//

#ifndef fastcgi_connection_h
#define fastcgi_connection_h

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#define COMPAT_CLOSE(a) close(a)
#define COMPAT_ECONNRESET ECONNRESET
#define COMPAT_EWOULDBLOCK EWOULDBLOCK

#define VERSION_1            1

#define BEGIN_REQUEST        1
#define ABORT_REQUEST        2
#define END_REQUEST          3
#define PARAMS               4
#define STDIN                5
#define STDOUT               6
#define STDERR               7
#define DATA                 8
#define GET_VALUES           9
#define GET_VALUES_RESULT    10
#define UNKNOWN_TYPE         11
#define MAXTYPE              UNKNOWN_TYPE

#define RESPONDER            1
#define AUTHORIZER           2
#define FILTER               3

#define REQUEST_COMPLETE     0
#define CANT_MPX_CONN        1
#define OVERLOADED           2
#define UNKNOWN_ROLE         3

#define MAX_CONNS            "MAX_CONNS"
#define MAX_REQS             "MAX_REQS"
#define MPXS_CONNS           "MPXS_CONNS"

#define HEADER_LEN           8

#define REQ_STATE_WRITTEN    1
#define REQ_STATE_OK         2
#define REQ_STATE_ERR        3
#define REQ_STATE_TIMED_OUT  4



#define fcgi_header_twobyte_set(header, field, value) do { \
(header)->field ## B0 = value & 0xff; \
(header)->field ## B1 = (value >> 8) & 0xff; \
} while (0)

#define fcgi_header_twobyte_get(header, field) \
(((header)->field ## B0 & 0xff) | \
(((header)->field ## B1 & 0xff) << 8))

#define fcgi_header_fourbyte_set(header, field, value) do { \
(header)->field ## B0 = value & 0xff; \
(header)->field ## B1 = (value >> 8) & 0xff; \
(header)->field ## B2 = (value >> 16) & 0xff; \
(header)->field ## B3 = (value >> 24) & 0xff; \
} while (0)

#define fcgi_header_fourbyte_get(header, field) \
(((header)->field ## B0 & 0xff) | \
(((header)->field ## B1 & 0xff) << 8) | \
(((header)->field ## B1 & 0xff) << 16) | \
(((header)->field ## B1 & 0xff) << 24))


typedef struct {
    unsigned char roleB1;
    unsigned char roleB0;
    unsigned char flags;
    unsigned char reserved[5];
} FCGI_BeginRequestBody;

typedef struct {
    unsigned char appStatusB3;
    unsigned char appStatusB2;
    unsigned char appStatusB1;
    unsigned char appStatusB0;
    unsigned char protocolStatus;
    unsigned char reserved[3];
} FCGI_EndRequestBody;

typedef struct {
    unsigned char version;
    unsigned char type;
    unsigned char requestIdB1;
    unsigned char requestIdB0;
    unsigned char contentLengthB1;
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
    unsigned char contentAndPaddingData[0];
} FCGI_Record;

typedef struct {
    unsigned char nameLengthB0;  /* nameLengthB0  >> 7 == 0 */
    unsigned char valueLengthB0; /* valueLengthB0 >> 7 == 0 */
    unsigned char nameAndValueData[0];
} FCGI_NameValuePair11;

typedef struct {
    unsigned char nameLengthB0;  /* nameLengthB0  >> 7 == 0 */
    unsigned char valueLengthB3; /* valueLengthB3 >> 7 == 1 */
    unsigned char valueLengthB2;
    unsigned char valueLengthB1;
    unsigned char valueLengthB0;
    unsigned char nameAndValueData[0];
} FCGI_NameValuePair14;

typedef struct {
    unsigned char nameLengthB3;  /* nameLengthB3  >> 7 == 1 */
    unsigned char nameLengthB2;
    unsigned char nameLengthB1;
    unsigned char nameLengthB0;
    unsigned char valueLengthB0; /* valueLengthB0 >> 7 == 0 */
    unsigned char nameAndValueData[0];
} FCGI_NameValuePair41;

typedef struct {
    unsigned char nameLengthB3;  /* nameLengthB3  >> 7 == 1 */
    unsigned char nameLengthB2;
    unsigned char nameLengthB1;
    unsigned char nameLengthB0;
    unsigned char valueLengthB3; /* valueLengthB3 >> 7 == 1 */
    unsigned char valueLengthB2;
    unsigned char valueLengthB1;
    unsigned char valueLengthB0;
    unsigned char nameAndValueData[0];
} FCGI_NameValuePair44;


int connect_to_fpm(const char *host, __uint16_t port, int *sock);
ssize_t begin_send(int *sock, unsigned short request_id);
ssize_t send_env(int *sock, char **keys, char **vals, int len, unsigned short request_id);
ssize_t send_content(int *sock, char *content, unsigned short request_id);
ssize_t recv_header(int *sock, unsigned short request_id, unsigned short *header_long);
ssize_t recv_content(int *sock, unsigned short request_id, unsigned short content_len, char *content);
ssize_t send_end(int *sock, unsigned short request_id);

#endif
