#ifndef _ESP8266_MOCK_H_
#define _ESP8266_MOCK_H_

#include <stdint.h>
#include "azure_c_shared_utility/umock_c_prod.h"

typedef void SSL;
typedef void SSL_CTX;
typedef void SSL_METHOD;

/*
 * SSL_CTX_new - create a SSL context
 *
 * @param method - the SSL context configuration file
 *
 * @return the context point, if create failed return NULL
 */
//SSL_CTX* SSL_CTX_new(SSL_METHOD *method);
MOCKABLE_FUNCTION(, SSL_CTX*, SSL_CTX_new, SSL_METHOD*, method);


/*
 * SSL_CTX_free - free a SSL context
 *
 * @param method - the SSL context point
 *
 * @return none
 */
//void SSL_CTX_free(SSL_CTX *ctx);
MOCKABLE_FUNCTION(, void, SSL_CTX_free, SSL_CTX*, ctx);


/*
 * SSL_CTX_set_option - set the option of the SSL context
 *
 * @param ctx - the SSL context
 *
 * @return the result of verifying
 *     result = 0 : successful
 *     result < 0 : error, you can see the mbedtls error code
 */
int SSL_CTX_set_option(SSL_CTX *ctx, int opt);

/*
 * SSL_set_fragment - set the global SSL fragment size
 *
 * @param ssl_ctx - the SSL context point
 * @param frag_size - fragment size
 *
 * @return the result of verifying
 *     result = 0 : successful
 *     result < 0 : error, you can see the mbedtls error code
 */
//int SSL_set_fragment(SSL_CTX *ssl_ctx, unsigned int frag_size);
MOCKABLE_FUNCTION(, int, SSL_set_fragment, SSL_CTX*, ssl_ctx, unsigned int, frag_size);

/*
 * SSL_new - create a SSL
 *
 * @param ssl_ctx - the SSL context which includes the SSL parameter
 *
 * @return the result
 *     result = 0 : successfully
 *     result < 0 : error, you may see the mbedtls error code
 */
//SSL* SSL_new(SSL_CTX *ssl_ctx);
MOCKABLE_FUNCTION(, SSL*, SSL_new, SSL_CTX*, ssl_ctx);

/*
 * SSL_free - free the SSL
 *
 * @param ssl - the SSL point which has been "SSL_new"
 *
 * @return none
 */
//void SSL_free(SSL *ssl);
MOCKABLE_FUNCTION(, void, SSL_free, SSL*, ssl);

/*
 * SSL_connect - connect to the remote SSL server
 *
 * @param ssl - the SSL point which has been "SSL_new"
 *
 * @return the result
 *     result = 0 : successfully
 *     result < 0 : error, you can see the mbedtls error code
 */
//int SSL_connect(SSL *ssl);
MOCKABLE_FUNCTION(, int, SSL_connect, SSL*, ssl);

/*
 * SSL_accept - accept the remote connection
 *
 * @param ssl - the SSL point which has been "SSL_new"
 *
 * @return the result
 *     result = 0 : successfully
 *     result < 0 : error, you can see the mbedtls error code
 */
int SSL_accept(SSL *ssl);

/*
 * SSL_read - read data from remote
 *
 * @param ssl - the SSL point which has been connected
 * @param buffer - the received data point
 * @param len - the received data length
 *
 * @return the result
 *     result > 0 : the length of the received data
 *     result = 0 : the connect is closed
 *     result < 0 : error, you can see the mbedtls error code
 */
// int SSL_read(SSL *ssl, void *buffer, int len);
MOCKABLE_FUNCTION(, int, SSL_read, SSL*, ssl, void*, buffer, int, len);

/*
 * SSL_write - send the data to remote
 *
 * @param ssl - the SSL point which has been connected
 * @param buffer - the send data point
 * @param len - the send data length
 *
 * @return the result of verifying
 *     result > 0 : the length of the written data
 *     result = 0 : the connect is closed
 *     result < 0 : error, you can see the mbedtls error code
 */
//int SSL_write(SSL *ssl, const void *buffer, int len);
MOCKABLE_FUNCTION(, int, SSL_write, SSL*, ssl, const void*, buffer, int, len);


/*
 * SSL_get_verify_result - get the verifying result of the SSL certification
 *
 * @param ssl - the SSL point
 *
 * @return the result of verifying
 *     result = 0 : successful
 *     result < 0 : error, you can see the mbedtls error code
 */
int SSL_get_verify_result(SSL *ssl);

/*
 * SSL_shutdown - shutdown the connection to the remote
 *
 * @param ssl - the SSL point which has been connected or accepted
 *
 * @return the result
 *     result = 0 : successfully
 *     result < 0 : error, you may see the mbedtls error code
 */
//int SSL_shutdown(SSL *ssl);
MOCKABLE_FUNCTION(, int, SSL_shutdown, SSL*, ssl);

/*
 * SSL_set_fd - set the socket file description to the SSL
 *
 * @param ssl - the SSL point which has been "SSL_new"
 * @param fd  - socket file description
 *
 * @return the result
 *     result = 1  : successfully
 *     result <= 0 : error, SSL is NULL or socket file description is NULL
 */
//int SSL_set_fd(SSL *ssl, int fd);
MOCKABLE_FUNCTION(, int, SSL_set_fd, SSL*, ssl, int, fd);

/*
 * SSL_set_rfd - set the read only socket file description to the SSL
 *
 * @param mbed_ssl - the SSL point which has been "SSL_new"
 * @param fd  - socket file description
 *
 * @return the result
 *     result = 1  : successfully
 *     result <= 0 : error, SSL is NULL or socket file description is NULL
 */
int SSL_set_rfd(SSL *ssl, int fd);

/*
 * SSL_set_wfd - set the write only socket file description to the SSL
 *
 * @param mbed_ssl - the SSL point which has been "SSL_new"
 * @param fd  - socket file description
 *
 * @return the result
 *     result = 1  : successfully
 *     result <= 0 : error, SSL is NULL or socket file description is NULL
 */
int SSL_set_wfd(SSL *ssl, int fd);

/*
 * SSL_CTX_use_PrivateKey - set the private key
 *
 * @param ctx  - the SSL context
 * @param buf  - the data point
 * @param len  - the data length
 * @param type - the data type
 *     type is always 0;
 *
 * @return the result of verifying
 *     result = 0 : successful
 *     result < 0 : error, you can see the mbedtls error code
 */
int SSL_CTX_use_PrivateKey(SSL_CTX *ctx, const char *buf, int len, int type);

/*
 * SSL_CTX_use_certificate - set the client own key
 *
 * @param ctx  - the SSL context
 * @param buf  - the data point
 * @param len  - the data length
 * @param type - the data type
 *     type is always 0;
 *
 * @return the result of verifying
 *     result = 0 : successful
 *     result < 0 : error, you can see the mbedtls error code
 */
int SSL_CTX_use_certificate(SSL_CTX *ctx, const char *buf, int len, int type);

/*
 * SSL_CTX_use_verify_certificate - set the CA certificate
 *
 * @param ctx  - the SSL context
 * @param buf  - the data point
 * @param len  - the data length
 * @param type - the data type
 *     type is always 0;
 *
 * @return the result of verifying
 *     result = 0 : successful
 *     result < 0 : error, you can see the mbedtls error code
 */
int SSL_CTX_use_verify_certificate(SSL_CTX *ctx, const char *buf, int len, int type);

/*
 * SSLv23_client_method - create the target SSL context client method
 *
 * @return the SSLV2.3 version SSL context client method
 */
SSL_METHOD* SSLv23_client_method(void);

/*
 * TLSv1_client_method - create the target SSL context client method
 *
 * @return the TLSV1.0 version SSL context client method
 */
//SSL_METHOD* TLSv1_client_method(void);
MOCKABLE_FUNCTION(, SSL_METHOD*, TLSv1_client_method);

/*
 * SSLv3_client_method - create the target SSL context client method
 *
 * @return the SSLV1.0 version SSL context client method
 */
SSL_METHOD* SSLv3_client_method(void);

/*
 * TLSv1_1_client_method - create the target SSL context client method
 *
 * @return the TLSV1.1 version SSL context client method
 */
SSL_METHOD* TLSv1_1_client_method(void);

/*
 * TLSv1_2_client_method- create the target SSL context client method
 *
 * @return the TLSV1.2 version SSL context client method
 */
SSL_METHOD* TLSv1_2_client_method(void);

/*
 * SSLv23_server_method - create the target SSL context server method
 *
 * @return the SSLV2.3 version SSL context server method
 */
SSL_METHOD* SSLv23_server_method(void);

/*
 * TLSv1_1_server_method - create the target SSL context server method
 *
 * @return the TLSV1.1 version SSL context server method
 */
SSL_METHOD* TLSv1_1_server_method(void);

/*
 * TLSv1_2_server_method - create the target SSL context server method
 *
 * @return the TLSV1.2 version SSL context server method
 */
SSL_METHOD* TLSv1_2_server_method(void);

/*
 * TLSv1_server_method - create the target SSL context server method
 *
 * @return the TLSV1.0 version SSL context server method
 */
SSL_METHOD* TLSv1_server_method(void);

/*
 * SSLv3_server_method - create the target SSL context server method
 *
 * @return the SSLV3.0 version SSL context server method
 */
SSL_METHOD* SSLv3_server_method(void);

#define ICACHE_FLASH_ATTR
#define MEMP_NUM_NETCONN                10


typedef uint32_t uint32;
typedef uint32_t u32_t;
//which types are registered if de?
//Could not copy type, type socklen_t not registered.
// typedef uint32_t socklen_t;
#define socklen_t unsigned int
typedef uint8_t u8_t;
typedef uint8_t uint8;
typedef uint16_t u16_t;
typedef uint32_t err_t;


struct ip_addr {
  u32_t addr;
};
typedef struct ip_addr ip_addr_t;

#define  SOL_SOCKET  0xfff    /* options for socket level */
#define SO_SNDBUF    0x1001    /* Unimplemented: send buffer size */
#define SO_RCVBUF    0x1002    /* receive buffer size */
#define SO_SNDLOWAT  0x1003    /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT  0x1004    /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO  0x1005    /* Unimplemented: send timeout */
#define SO_RCVTIMEO  0x1006    /* receive timeout */
#define SO_ERROR     0x1007    /* get error status and clear */
#define SO_TYPE      0x1008    /* get socket type */
#define SO_CONTIMEO  0x1009    /* Unimplemented: connect timeout */
#define SO_NO_CHECK  0x100a    /* don't create UDP checksum */
#define SO_REUSEADDR 1         /* Enable address reuse */

#define AF_INET         2
/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

int ioctl(int s, long cmd, void *argp);
int fcntl(int s, int cmd, int val);
#define F_GETFL 3
#define F_SETFL 4
#define O_NONBLOCK  1 /* nonblocking I/O */
#define O_NDELAY    1 /* same as O_NONBLOCK, for compatibility */
#define LOCAL



typedef u8_t sa_family_t;
typedef u16_t in_port_t;

typedef u32_t in_addr_t;

struct in_addr {
  in_addr_t s_addr;
};


struct sockaddr_in {
  u8_t            sin_len;
  sa_family_t     sin_family;
  in_port_t       sin_port;
  struct in_addr  sin_addr;
#define SIN_ZERO_LEN 8
  char            sin_zero[SIN_ZERO_LEN];
};


struct sockaddr {
  u8_t        sa_len;
  sa_family_t sa_family;
#if LWIP_IPV6
  char        sa_data[22];
#else /* LWIP_IPV6 */
  char        sa_data[14];
#endif /* LWIP_IPV6 */
};


/* FD_SET used for lwip_select */
#ifndef FD_SET
  #undef  FD_SETSIZE
  /* Make FD_SETSIZE match NUM_SOCKETS in socket.c */
  #define FD_SETSIZE    MEMP_NUM_NETCONN
  #define FD_SET(n, p)  ((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
  #define FD_CLR(n, p)  ((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
  #define FD_ISSET(n,p) ((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
  #define FD_ZERO(p)    memset((void*)(p),0,sizeof(*(p)))

  typedef struct fd_set {
          unsigned char fd_bits [(FD_SETSIZE+7)/8];
        } fd_set;

#endif /* FD_SET */

struct timeval {
  long    tv_sec;         /* seconds */
  long    tv_usec;        /* and microseconds */
};

//err_t netconn_gethostbyname(const char *name, ip_addr_t *addr);
MOCKABLE_FUNCTION(, err_t, netconn_gethostbyname, const char*, name, ip_addr_t*, addr);

//int socket(int domain, int type, int protocol);
MOCKABLE_FUNCTION(, int, socket, int, domain, int, type, int, protocol);

//int bind(int s, const struct sockaddr* name, socklen_t namelen);
MOCKABLE_FUNCTION(, int, bind, int, s, const struct sockaddr*, name, socklen_t, namelen);

//int connect(int s, const struct sockaddr *name, socklen_t namelen);
MOCKABLE_FUNCTION(, int, connect, int, s, const struct sockaddr*, name, socklen_t, namelen);

//int lwip_getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
MOCKABLE_FUNCTION(, int, getsockopt, int, s, int, level, int, optname, void*, optval, socklen_t*, optlen);

//int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
//                struct timeval *timeout);
MOCKABLE_FUNCTION(, int, lwip_select, int, maxfdp1, fd_set*, readset, fd_set*, writeset, fd_set*, exceptset, struct timeval*, timeout);

//os_delay_us(int us);
MOCKABLE_FUNCTION(, void, os_delay_us, int, us);

//int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
MOCKABLE_FUNCTION(, int, setsockopt, int, s, int, level, int, optname, const void*, optval, socklen_t, optlen);

//int close(int s)
MOCKABLE_FUNCTION(, int, close, int, s);


#define htons(x) (x)
#define ntohs(x) (x)
#define htonl(x) (x)
#define ntohl(x) (x)


#endif
