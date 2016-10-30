// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "openssl/ssl_compat-1.0.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "espressif/esp8266/ets_sys.h"
#include "espressif/espconn.h"

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

typedef enum TLSIO_STATE_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING_UNDERLYING_IO,
    TLSIO_STATE_IN_HANDSHAKE,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE;

typedef struct TLS_IO_INSTANCE_TAG
{
    XIO_HANDLE underlying_io;
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

struct CRYPTO_dynlock_value 
{
    LOCK_HANDLE lock; 
};

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

/*this function will clone an option given by name and value*/
static void* tlsio_openssl_CloneOption(const char* name, const void* value)
{
    void* result;
    if(
        (name == NULL) || (value == NULL)
    )
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
        result = NULL;
    }
    else
    {
        if (strcmp(name, "TrustedCerts") == 0)
        {
            if(mallocAndStrcpy_s((char**)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s TrustedCerts value");
                result = NULL;
            }
            else
            {
                /*return as is*/
            }
        }
        else if (strcmp(name, "x509certificate") == 0)
        {
            if (mallocAndStrcpy_s((char**)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s x509certificate value");
                result = NULL;
            }
            else
            {
                /*return as is*/
            }
        }
        else if (strcmp(name, "x509privatekey") == 0)
        {
            if (mallocAndStrcpy_s((char**)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s x509privatekey value");
                result = NULL;
            }
            else
            {
                /*return as is*/
            }
        }
        else
        {
            LogError("not handled option : %s", name);
            result = NULL;
        }
    }
    return result;
}

/*this function destroys an option previously created*/
static void tlsio_openssl_DestroyOption(const char* name, const void* value)
{
    /*since all options for this layer are actually string copies., disposing of one is just calling free*/
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

static OPTIONHANDLER_HANDLE tlsio_openssl_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result;
    if(handle == NULL)
    {
        LogError("invalid parameter detected: CONCRETE_IO_HANDLE handle=%p", handle);
        result = NULL;
    }
    else
    {
        result = OptionHandler_Create(tlsio_openssl_CloneOption, tlsio_openssl_DestroyOption, tlsio_openssl_setoption);
        if (result == NULL)
        {
            LogError("unable to OptionHandler_Create");
            /*return as is*/
        }
        else
        {
            /*this layer cares about the certificates and the x509 credentials*/
            TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)handle;
            if(
                (tls_io_instance->certificate != NULL) && 
                (OptionHandler_AddOption(result, "TrustedCerts", tls_io_instance->certificate) != 0)
            )
            {
                LogError("unable to save TrustedCerts option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else if (
                (tls_io_instance->x509certificate != NULL) &&
                (OptionHandler_AddOption(result, "x509certificate", tls_io_instance->x509certificate) != 0)
                )
            {
                LogError("unable to save x509certificate option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else if (
                (tls_io_instance->x509privatekey != NULL) && 
                (OptionHandler_AddOption(result, "x509privatekey", tls_io_instance->x509privatekey) != 0)
            )
            {
                LogError("unable to save x509privatekey option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else
            {
                /*all is fine, all interesting options have been saved*/
                /*return as is*/
            }
        }
    }
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

static struct CRYPTO_dynlock_value* openssl_dynamic_locks_create_cb(const char* file, int line)
{
    struct CRYPTO_dynlock_value* result;
    return result;
}

static void openssl_dynamic_locks_destroy_cb(struct CRYPTO_dynlock_value* dynlock_value, const char* file, int line)
{
    Lock_Deinit(dynlock_value->lock);
    free(dynlock_value);
}

static void openssl_dynamic_locks_uninstall(void)
{
}

static void openssl_dynamic_locks_install(void)
{
}

static void openssl_static_locks_lock_unlock_cb(int lock_mode, int lock_index, const char * file, int line)
{
}

static void openssl_static_locks_uninstall(void)
{
    if (openssl_locks != NULL)
    {
        CRYPTO_set_locking_callback(NULL);
        int i = 0;
        for(i = 0; i < CRYPTO_num_locks(); i++)
        {
            if (openssl_locks[i] != NULL)
            {
                Lock_Deinit(openssl_locks[i]);
            }
        }
        
        free(openssl_locks);
        openssl_locks = NULL;
    }
    else
    {
        LogError("Locks already uninstalled");
    }
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

LOCAL void openssl_thread_LWIP_CONNECTION(void *p)
{
    int ret = -1;

    int socket = -1;
    struct sockaddr_in sock_addr;
    fd_set readset;
    fd_set writeset;
    fd_set errset;

    ip_addr_t target_ip;
    SSL_CTX *ctx;
    SSL *ssl;

    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)p;

    LogInfo("OpenSSL thread start...\n");

    do {
        ret = netconn_gethostbyname(tls_io_instance->hostname, &target_ip);
    } while(ret);
    LogInfo("get target IP is %d.%d.%d.%d\n", (unsigned char)((target_ip.addr & 0x000000ff) >> 0),
                                                (unsigned char)((target_ip.addr & 0x0000ff00) >> 8),
                                                (unsigned char)((target_ip.addr & 0x00ff0000) >> 16),
                                                (unsigned char)((target_ip.addr & 0xff000000) >> 24));

    LogInfo("create socket ......");
    socket = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        LogError("create socket failed\n");
        return;
    }
    LogInfo("create socket OK\n");

    LogInfo("bind socket ......");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = 0;
    sock_addr.sin_port = htons(OPENSSL_LOCAL_TCP_PORT);
    ret = bind(socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if (ret) {
        LogError("bind socket failed\n");
        return;
    }
    LogInfo("bind socket OK\n");

    LogInfo("socket connect to remote ......");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = target_ip.addr;
    sock_addr.sin_port = htons(tls_io_instance->port);

    lwip_set_non_block(socket);

    ret = connect(socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    if (ret == -1) {
        ret = lwip_net_errno(socket);
        if (ret != EINPROGRESS){
            LogError("socket connect failed %s\n", tls_io_instance->hostname);
            return;
        } else{
            LogInfo("socket connect in progress\n");
        }
    }
    LogInfo("socket connect OK\n");

    char recv_buf[128];
    int recv_bytes = (int)sizeof(recv_buf);
    int retry = 0;

    while (retry < 10){
        LogInfo("lwip select retry #: %d\n", retry);

        FD_ZERO(&readset);
        FD_SET(socket, &readset);
    
        FD_ZERO(&writeset);
        FD_SET(socket, &writeset);
    
        FD_ZERO(&errset);
        FD_SET(socket, &errset);
    
        ret = lwip_select(socket + 1, &readset, &writeset, &errset, NULL);
        if (ret > 0){
            if (FD_ISSET(socket, &writeset)){
              break;
            }
    
            if (FD_ISSET(socket, &readset)){
                memset(recv_buf, 0, recv_bytes);
                break;
            }
        }else{
            LogInfo("lwip_select returned: %d\n", ret);
        }

        retry++;
    }

    int s = socket;
    ctx = SSL_CTX_new(TLSv1_client_method());
    if (!ctx) {
        LogError("create new SSL CTX failed\n");
        return;
    }
    LogInfo("create new SSL CTX OK\n");
    
    LogInfo("SSL set fragment\n");
    ret = SSL_set_fragment(ctx, OPENSSL_FRAGMENT_SIZE);

    LogInfo("SSL new... \n");
    ssl = SSL_new(ctx);

    LogInfo("SSL set fd\n");
    SSL_set_fd(ssl, s);

    LogInfo("SSL connect... \n");

    while (-1 == SSL_connect(ssl))
    {  
        FD_ZERO(&readset);
        FD_SET(s, &readset);
        FD_ZERO(&writeset);
        FD_SET(s, &writeset);
        FD_ZERO(&errset);
        FD_SET(s, &errset);

        ret = lwip_select(s + 1, &readset, &writeset, &errset, NULL);
    }

    LogInfo("SSL connect ok\n");

    tls_io_instance->ssl = ssl;
    tls_io_instance->ssl_context = ctx;
}

static int send_handshake_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    //system_print_meminfo(); // This is useful for debugging purpose.
    //LogInfo("free heap size %d", system_get_free_heap_size()); // This is useful for debugging purpose.
    openssl_thread_LWIP_CONNECTION(tls_io_instance);
    tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
    indicate_open_complete(tls_io_instance, IO_OPEN_OK);    
    int result = 0;

    return result;
}



static void on_underlying_io_close_complete(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;   
    switch (tls_io_instance->tlsio_state)
    {
        default:
        case TLSIO_STATE_NOT_OPEN:
        case TLSIO_STATE_OPEN:
            break;

        case TLSIO_STATE_OPENING_UNDERLYING_IO:
        case TLSIO_STATE_IN_HANDSHAKE:
            tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
            indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
            break;

        case TLSIO_STATE_CLOSING:
            tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;

            if (tls_io_instance->on_io_close_complete != NULL)
            {
                tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
            }
            break;
    }
}

static void on_underlying_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    if (tls_io_instance->tlsio_state == TLSIO_STATE_OPENING_UNDERLYING_IO)
    {
        if (open_result == IO_OPEN_OK)
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_IN_HANDSHAKE;

            if (send_handshake_bytes(tls_io_instance) != 0)
            {
                if (xio_close(tls_io_instance->underlying_io, on_underlying_io_close_complete, tls_io_instance) != 0)
                {
                    indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
                    LogError("Error in xio_close.");
                }
            }
        }
        else
        {
            indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPENING_UNDERLYING_IO.");
        }
    }
}

static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result = 0;
    unsigned char buffer[64];

    int rcv_bytes = 1;
    rcv_bytes = SSL_read(tls_io_instance->ssl, buffer, sizeof(buffer));
    LogInfo("decode ssl recv bytes: %d\n", rcv_bytes);
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

    return result;
}

static void destroy_openssl_instance(TLS_IO_INSTANCE* tls_io_instance)
{
    if (tls_io_instance != NULL)
    {
        SSL_free(tls_io_instance->ssl);
        SSL_CTX_free(tls_io_instance->ssl_context);
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

int tlsio_openssl_init(void)
{
    return 0;
}

void tlsio_openssl_deinit(void)
{
    openssl_dynamic_locks_uninstall();
    openssl_static_locks_uninstall();

    ERR_free_strings();
}

CONCRETE_IO_HANDLE tlsio_openssl_create(void* io_create_parameters)
{
    TLSIO_CONFIG* tls_io_config = io_create_parameters;
    TLS_IO_INSTANCE* result;

    if (tls_io_config == NULL)
    {
        result = NULL;
        LogError("NULL tls_io_config.");
    }
    else
    {
        result = (TLS_IO_INSTANCE*) malloc(sizeof(TLS_IO_INSTANCE));
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

void tlsio_openssl_destroy(CONCRETE_IO_HANDLE tls_io)
{
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
            tls_io_instance->certificate = NULL;
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
        xio_destroy(tls_io_instance->underlying_io);
        free(tls_io);
    }
}

int tlsio_openssl_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;

    if (tls_io == NULL)
    {

        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {

        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
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

            tls_io_instance->tlsio_state = TLSIO_STATE_OPENING_UNDERLYING_IO;

            if (create_openssl_instance(tls_io_instance) != 0)
            {

                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                result = __LINE__;
            }
            else
            {
                send_handshake_bytes(tls_io_instance);
                result = 0;
            }
        }
    }

    return result;
}

int tlsio_openssl_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN or TLSIO_STATE_CLOSING.");
        }
        else
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
            tls_io_instance->on_io_close_complete = on_io_close_complete;
            tls_io_instance->on_io_close_complete_context = callback_context;

            if (xio_close(tls_io_instance->underlying_io, on_underlying_io_close_complete, tls_io_instance) != 0)
            {
                result = __LINE__;
                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                LogError("Error in xio_close.");
            }
            else
            {
                destroy_openssl_instance(tls_io_instance);
                result = 0;
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
        result = __LINE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPEN.");
        }
        else
        {
            int total_write = 0;
            int res = 0;
            int retry = 0;

            while(size > 0 && retry < 500){
                res = SSL_write(tls_io_instance->ssl, ((uint8*)buffer)+total_write, size);
                LogInfo("SSL_write res: %d, size: %d, retry: %d", res, size, retry);
                if(res > 0){
                    total_write += res;
                    size = size - res;
                }
                else
                {
                    retry++;
                }
            }

            if (retry >= 500)
            {
                result = 1;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

void tlsio_openssl_dowork(CONCRETE_IO_HANDLE tls_io)
{
     if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        decode_ssl_received_bytes(tls_io_instance);
    }

}

int tlsio_openssl_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    int result;

    if (tls_io == NULL || optionName == NULL)
    {
        result = __LINE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (strcmp("TrustedCerts", optionName) == 0)
        {
            const char* cert = (const char*)value;

            if (tls_io_instance->certificate != NULL)
            {
                // Free the memory if it has been previously allocated
                free(tls_io_instance->certificate);
            }

            // Store the certificate
            size_t len = strlen(cert);
            tls_io_instance->certificate = malloc(len+1);
            if (tls_io_instance->certificate == NULL)
            {
                result = __LINE__;
            }
            else
            {
                strcpy(tls_io_instance->certificate, cert);
                result = 0;
            }

            // If we're previously connected then add the cert to the context
            if (tls_io_instance->ssl_context != NULL)
            {
                result = add_certificate_to_store(tls_io_instance, cert);
            }
        }
        else if (strcmp("x509certificate", optionName) == 0)
        {
            if (tls_io_instance->x509certificate != NULL)
            {
                LogError("unable to set x509 options more than once");
                result = __LINE__;
            }
            else
            {   
                /*let's make a copy of this option*/
                if (mallocAndStrcpy_s((char**)&tls_io_instance->x509certificate, value) != 0)
                {
                    LogError("unable to mallocAndStrcpy_s");
                    result = __LINE__;
                }
                else
                {
                    result = 0;
                }
            }
        }
        else if (strcmp("x509privatekey", optionName) == 0)
        {
            if (tls_io_instance->x509privatekey != NULL)
            {
                LogError("unable to set more than once x509 options");
                result = __LINE__;
            }
            else
            {
                /*let's make a copy of this option*/
                if (mallocAndStrcpy_s((char**)&tls_io_instance->x509privatekey, value) != 0)
                {
                    LogError("unable to mallocAndStrcpy_s");
                    result = __LINE__;
                }
                else
                {
                    result = 0;
                }
            }
        }

        else
        {
            if (tls_io_instance->underlying_io == NULL)
            {
                result = __LINE__;
            }
            else
            {
                result = xio_setoption(tls_io_instance->underlying_io, optionName, value);
            }
        }
    }

    return result;
}

const IO_INTERFACE_DESCRIPTION* tlsio_openssl_get_interface_description(void)
{
    return &tlsio_openssl_interface_description;
}
