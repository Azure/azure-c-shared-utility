// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#ifdef FREERTOS_ARCH_ESP8266
#include "openssl/ssl_compat-1.0.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "espressif/esp8266/ets_sys.h"
#include "espressif/espconn.h"

#else
//mock header
#include "esp8266_mock.h"
#include "azure_c_shared_utility/gballoc.h"
#endif

#include <stdio.h>
#include <stdbool.h>
#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_openssl.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"

#define OPENSSL_FRAGMENT_SIZE 5120
#define OPENSSL_LOCAL_TCP_PORT 1000
#define MAX_RETRY 10
#define MAX_RETRY_WRITE 500
#define RETRY_DELAY 1000

typedef enum TLSIO_STATE_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE;

typedef struct TLS_IO_INSTANCE_TAG
{
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_open_complete_context;
    void* on_io_close_complete_context;
    void* on_io_error_context;
    SSL* ssl;
    SSL_CTX* ssl_context;
    TLSIO_STATE tlsio_state;
    char* hostname;
    int port;
    char* certificate;
    const char* x509certificate;
    const char* x509privatekey;
} TLS_IO_INSTANCE;


int ICACHE_FLASH_ATTR ERR_get_error(void)
{
    return 0;
}

void ICACHE_FLASH_ATTR ERR_error_string_n(uint32 error, char* out, uint32 olen)
{
    return;
}

void ICACHE_FLASH_ATTR ERR_error_string(uint32 error, char* out)
{
    return;
}

/*this function destroys an option previously created*/
static void tlsio_openssl_DestroyOption(const char* name, const void* value)
{
    /*since all options for this layer are actually string copies, disposing of one is just calling free*/
    if (
        (name == NULL) || (value == NULL)
        )
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
    }
    else
    {
        if (
            (strcmp(name, "TrustedCerts") == 0) ||
            (strcmp(name, "x509certificate") == 0) ||
            (strcmp(name, "x509privatekey") == 0)
        )
        {
            free((void*)value);
        }
        else
        {
            LogError("not handled option : %s", name);
        }
    }
}

/* Codes_SRS_TLSIO_SSL_ESP8266_99_001: [ The tlsio_openssl_retrieveoptions shall not do anything, and return NULL. ]*/
static OPTIONHANDLER_HANDLE tlsio_openssl_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result = NULL;
    return result;
}

static const IO_INTERFACE_DESCRIPTION tlsio_openssl_interface_description =
{
    tlsio_openssl_retrieveoptions,
    tlsio_openssl_create,
    tlsio_openssl_destroy,
    tlsio_openssl_open,
    tlsio_openssl_close,
    tlsio_openssl_send,
    tlsio_openssl_dowork,
    tlsio_openssl_setoption
};

static LOCK_HANDLE * openssl_locks = NULL;

static void openssl_dynamic_locks_uninstall(void)
{
}

static void openssl_dynamic_locks_install(void)
{
}

static void openssl_static_locks_lock_unlock_cb(int lock_mode, int lock_index, const char * file, int line)
{
}

static void indicate_open_complete(TLS_IO_INSTANCE* tls_io_instance, IO_OPEN_RESULT open_result)
{
    if (tls_io_instance->on_io_open_complete == NULL)
    {
        LogError("NULL on_io_open_complete.");
    }
    else
    {
        tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, open_result);
    }
}


static int lwip_net_errno(int fd)
{
    int sock_errno = 0;
    u32_t optlen = sizeof(sock_errno);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &sock_errno, &optlen);
    return sock_errno;
}

static void lwip_set_non_block(int fd) 
{
  int flags = -1;
  int error = 0;

  while(1){
      flags = fcntl(fd, F_GETFL, 0);
      if (flags == -1){
          error = lwip_net_errno(fd);
          if (error != EINTR){
              break;
          }
      } else{
          break;
      }
  }

  while(1){
      flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
      if (flags == -1) {
          error = lwip_net_errno(fd);
          if (error != EINTR){
              break;
          }
      } else{
          break;
      }
  }

}


LOCAL void openssl_thread_LWIP_CONNECTION(TLS_IO_INSTANCE* p)
{
    int ret;
    int sock;

    struct sockaddr_in sock_addr;
    fd_set readset;
    fd_set writeset;
    fd_set errset;

    ip_addr_t target_ip;
    SSL_CTX *ctx;
    SSL *ssl;

    TLS_IO_INSTANCE* tls_io_instance = p;

    //LogInfo("OpenSSL thread start...");

    int netconn_retry = 0;
    do {
        ret = netconn_gethostbyname(tls_io_instance->hostname, &target_ip);
    } while(ret && netconn_retry++ < MAX_RETRY);
    // This is useful for debugging purpose.
    // LogInfo("get target IP is %d.%d.%d.%d", (unsigned char)((target_ip.addr & 0x000000ff) >> 0),
    //                                             (unsigned char)((target_ip.addr & 0x0000ff00) >> 8),
    //                                             (unsigned char)((target_ip.addr & 0x00ff0000) >> 16),
    //                                             (unsigned char)((target_ip.addr & 0xff000000) >> 24));

    //LogInfo("create socket ......");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LogError("create socket failed");
        return;
    }
    // LogInfo("create socket OK");

    // LogInfo("bind socket ......");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = 0;
    sock_addr.sin_port = htons(OPENSSL_LOCAL_TCP_PORT);
    ret = bind(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if (ret) {
        LogError("bind socket failed");
        return;
    }
    // LogInfo("bind socket OK");

    // LogInfo("socket connect to remote ......");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = target_ip.addr;
    sock_addr.sin_port = htons(tls_io_instance->port);

    lwip_set_non_block(sock);

    ret = connect(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if (ret == -1) {
        ret = lwip_net_errno(sock);
        if (ret != EINPROGRESS){
            LogError("socket connect failed %s", tls_io_instance->hostname);
            return;
        } else{
            LogInfo("socket connect in progress");
        }
    }
    // LogInfo("socket connect OK");

    char recv_buf[128];
    size_t recv_bytes = (size_t)sizeof(recv_buf);
    int retry = 0;

    while (retry < MAX_RETRY){
        // LogInfo("lwip select retry #: %d", retry);

        FD_ZERO(&readset);
        FD_SET(sock, &readset);
    
        FD_ZERO(&writeset);
        FD_SET(sock, &writeset);
    
        FD_ZERO(&errset);
        FD_SET(sock, &errset);
    
        ret = lwip_select(sock + 1, &readset, &writeset, &errset, NULL);
        if (ret > 0){
            if (FD_ISSET(sock, &writeset)){
              break;
            }
    
            if (FD_ISSET(sock, &readset)){
                memset(recv_buf, 0, recv_bytes);
                break;
            }
        }else{
            LogInfo("lwip_select returned: %d", ret);
        }

        retry++;
        os_delay_us(RETRY_DELAY);
    }

    ctx = SSL_CTX_new(TLSv1_client_method());
    if (!ctx) {
        LogError("create new SSL CTX failed");
        return;
    }
    // LogInfo("create new SSL CTX OK");
    
    // LogInfo("SSL set fragment");
    SSL_set_fragment(ctx, OPENSSL_FRAGMENT_SIZE);

    // LogInfo("SSL new... ");
    ssl = SSL_new(ctx);
    if (!ssl) {
        LogError("create ssl failed");
        return;
    }

    // LogInfo("SSL set fd");
    SSL_set_fd(ssl, sock);

    // LogInfo("SSL connect... ");

    while (-1 == SSL_connect(ssl))
    {  
        FD_ZERO(&readset);
        FD_SET(sock, &readset);
        FD_ZERO(&writeset);
        FD_SET(sock, &writeset);
        FD_ZERO(&errset);
        FD_SET(sock, &errset);

        lwip_select(sock + 1, &readset, &writeset, &errset, NULL);
    }

    // LogInfo("SSL connect ok");

    tls_io_instance->ssl = ssl;
    tls_io_instance->ssl_context = ctx;
}

static int send_handshake_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    //system_print_meminfo(); // This is useful for debugging purpose.
    //LogInfo("free heap size %d", system_get_free_heap_size()); // This is useful for debugging purpose.
    int result;
    openssl_thread_LWIP_CONNECTION(tls_io_instance);
    tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
    indicate_open_complete(tls_io_instance, IO_OPEN_OK);    
    result = 0;

    return result;
}


static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result;
    unsigned char buffer[64];

    int rcv_bytes;
    rcv_bytes = SSL_read(tls_io_instance->ssl, buffer, sizeof(buffer));
    // LogInfo("decode ssl recv bytes: %d", rcv_bytes);
    if (rcv_bytes > 0)
    {
        if (tls_io_instance->on_bytes_received == NULL)
        {
            LogError("NULL on_bytes_received.");
        }
        else
        {
            tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, rcv_bytes);
        }
    }
    result = 0;
    return result;
}

static void destroy_openssl_instance(TLS_IO_INSTANCE* tls_io_instance)
{
    if (tls_io_instance != NULL)
    {
        if (tls_io_instance->ssl != NULL)
        {
            SSL_free(tls_io_instance->ssl);
            tls_io_instance->ssl = NULL;
        }
        if (tls_io_instance->ssl_context != NULL)
        {
            SSL_CTX_free(tls_io_instance->ssl_context);
            tls_io_instance->ssl_context = NULL;
        }
    }
}

static int add_certificate_to_store(TLS_IO_INSTANCE* tls_io_instance, const char* certValue)
{
    return 0;
}


static int create_openssl_instance(TLS_IO_INSTANCE* tlsInstance)
{
    return 0;
}


/* Codes_SRS_TLSIO_SSL_ESP8266_99_005: [ The tlsio_openssl_create succeed. ]*/
CONCRETE_IO_HANDLE tlsio_openssl_create(void* io_create_parameters)
{
    TLSIO_CONFIG* tls_io_config = (TLSIO_CONFIG*)io_create_parameters;
    TLS_IO_INSTANCE* result;

    /* Codes_SRS_TLSIO_SSL_ESP8266_99_003: [ The tlsio_openssl_create shall return NULL when io_create_parameters is NULL. ]*/
    if (tls_io_config == NULL)
    {
        result = NULL;
        LogError("NULL tls_io_config.");
    }
    else
    {
        /* Codes_SRS_TEMPLATE_99_004: [ The tlsio_openssl_create shall return NULL when malloc fails. ]*/
        result = (TLS_IO_INSTANCE*) malloc(sizeof(TLS_IO_INSTANCE));
        // LogInfo("result is 0x%x", result);

        if (result == NULL)
        {
            LogError("Failed allocating TLSIO instance.");
        }
        else
        {
            memset(result, 0, sizeof(TLS_IO_INSTANCE));
            mallocAndStrcpy_s(&result->hostname, tls_io_config->hostname);
            result->port = tls_io_config->port;
            result->ssl_context = NULL;
            result->ssl = NULL;
            result->certificate = NULL;

            result->on_bytes_received = NULL;
            result->on_bytes_received_context = NULL;

            result->on_io_open_complete = NULL;
            result->on_io_open_complete_context = NULL;

            result->on_io_close_complete = NULL;
            result->on_io_close_complete_context = NULL;

            result->on_io_error = NULL;
            result->on_io_error_context = NULL;

            result->tlsio_state = TLSIO_STATE_NOT_OPEN;

            result->x509certificate = NULL;
            result->x509privatekey = NULL;
        }
    }

    return (CONCRETE_IO_HANDLE)result;
}

/* Codes_SRS_TLSIO_SSL_ESP8266_99_010: [ The tlsio_openssl_destroy succeed ]*/
void tlsio_openssl_destroy(CONCRETE_IO_HANDLE tls_io)
{
    /* Codes_SRS_TLSIO_SSL_ESP8266_99_009: [ The tlsio_openssl_destroy NULL parameter. make sure there is no crash ]*/
    if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        if (tls_io_instance->certificate != NULL)
        {
            free(tls_io_instance->certificate);
        }
        if (tls_io_instance->hostname != NULL)
        {
            free(tls_io_instance->hostname);
        }
        if (tls_io_instance->x509certificate != NULL)
        {
            free((void*)tls_io_instance->x509certificate);
        }
        if (tls_io_instance->x509privatekey != NULL)
        {
            free((void*)tls_io_instance->x509privatekey);
        }
        free(tls_io);
    }
}


/* Codes_SRS_TLSIO_SSL_ESP8266_99_008: [ The tlsio_openssl_open shall return 0 when succeed ]*/
int tlsio_openssl_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;

    if (tls_io == NULL)
    {
        /* Codes_SRS_TLSIO_SSL_ESP8266_99_006: [ The tlsio_openssl_open failed because tls_io is NULL. ]*/
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {

        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        /* Codes_SRS_TLSIO_SSL_ESP8266_99_007: [ The tlsio_openssl_open invalid state. ]*/
        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN.");
        }
        else
        {
            tls_io_instance->on_io_open_complete = on_io_open_complete;
            tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_bytes_received_context = on_bytes_received_context;

            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            tls_io_instance->tlsio_state = TLSIO_STATE_OPENING;

            if (create_openssl_instance(tls_io_instance) != 0)
            {

                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                result = __LINE__;
                tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
            }
            else
            {
                if(send_handshake_bytes(tls_io_instance) != 0)
                {
                    tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                    result = __LINE__;
                    tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
                }
                else
                {
                    result = 0;
                }
            }
        }
    }

    return result;
}


/* Codes_SRS_TLSIO_SSL_ESP8266_99_013: [ The tlsio_openssl_close succeed.]*/
int tlsio_openssl_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result;

    /* Codes_SRS_TLSIO_SSL_ESP8266_99_011: [ The tlsio_openssl_close NULL parameter.]*/
    if (tls_io == NULL)
    {
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_OPENING) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_ERROR))
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPEN.");
        }
        else
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
            tls_io_instance->on_io_close_complete = on_io_close_complete;
            tls_io_instance->on_io_close_complete_context = callback_context;

            destroy_openssl_instance(tls_io_instance);
            tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
            result = 0;
            if (tls_io_instance->on_io_close_complete != NULL)
            {
                tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
            }
        }
    }

    return result;
}

int tlsio_openssl_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if (tls_io == NULL)
    {
        /* Codes_SRS_TLSIO_SSL_ESP8266_99_014: [ The tlsio_openssl_send NULL instance.]*/
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else if (buffer == NULL)
    {
        /* Codes_SRS_TLSIO_SSL_ESP8266_99_014: [ The tlsio_openssl_send NULL instance.]*/
        result = __LINE__;
        LogError("NULL buffer.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            /* Codes_SRS_TLSIO_SSL_ESP8266_99_015: [ The tlsio_openssl_send wrog state.]*/
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPEN.");
        }
        else
        {
            int total_write = 0;
            int res = 0;
            int retry = 0;

            while(size > 0 && retry < MAX_RETRY_WRITE){
                /* Codes_SRS_TLSIO_SSL_ESP8266_99_016: [ The tlsio_openssl_send SSL_write success]*/
                /* Codes_SRS_TLSIO_SSL_ESP8266_99_017: [ The tlsio_openssl_send SSL_write failure]*/
                res = SSL_write(tls_io_instance->ssl, ((uint8*)buffer)+total_write, size);
                //LogInfo("SSL_write res: %d, size: %d, retry: %d", res, size, retry);
                if(res > 0){
                    total_write += res;
                    size = size - res;
                }
                else
                {
                    retry++;
                }
            }

            if (retry >= MAX_RETRY_WRITE)
            {
                result = __LINE__;
                if (on_send_complete != NULL)
                {
                    on_send_complete(callback_context, IO_SEND_ERROR);
                }
            }
            else
            {
                result = 0;
                if (on_send_complete != NULL)
                {
                    on_send_complete(callback_context, IO_SEND_OK);
                }
            }
        }
    }

    return result;
}

/* Codes_SRS_TLSIO_SSL_ESP8266_99_019: [ The tlsio_openssl_dowrok succeed]*/
void tlsio_openssl_dowork(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io == NULL)
    {
        /* Codes_SRS_TLSIO_SSL_ESP8266_99_018: [ The tlsio_openssl_dowork NULL parameter. No crash when passing NULL]*/
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state == TLSIO_STATE_OPEN)
        {
            decode_ssl_received_bytes(tls_io_instance);
        } 
        else
        {
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPEN.");
        }
    }

}

/* Codes_SRS_TLSIO_SSL_ESP8266_99_002: [ The tlsio_arduino_setoption shall not do anything, and return 0. ]*/
int tlsio_openssl_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    return 0;
}

const IO_INTERFACE_DESCRIPTION* tlsio_openssl_get_interface_description(void)
{
    return &tlsio_openssl_interface_description;
}
