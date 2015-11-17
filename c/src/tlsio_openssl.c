// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/crypto.h"
#include <stdio.h>
#include <stdbool.h>
#include "tlsio_openssl.h"
#include "socketio.h"

typedef enum HANDSHAKE_STATE_ENUM_TAG
{
    HANDSHAKE_STATE_NOT_STARTED,
    HANDSHAKE_STATE_IN_HANDSHAKE,
    HANDSHAKE_STATE_DONE
} HANDSHAKE_STATE_ENUM;

typedef struct TLS_IO_INSTANCE_TAG
{
    IO_HANDLE socket_io;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_STATE_CHANGED on_io_state_changed;
    void* callback_context;
    LOGGER_LOG logger_log;
    IO_STATE io_state;
    SSL* ssl;
    SSL_CTX* ssl_context;
    BIO* in_bio;
    BIO* out_bio;
    HANDSHAKE_STATE_ENUM handshake_state;
} TLS_IO_INSTANCE;

static const IO_INTERFACE_DESCRIPTION tlsio_openssl_interface_description =
{
    tlsio_openssl_create,
    tlsio_openssl_destroy,
    tlsio_openssl_open,
    tlsio_openssl_close,
    tlsio_openssl_send,
    tlsio_openssl_dowork
};

static void set_io_state(TLS_IO_INSTANCE* tls_io_instance, IO_STATE io_state)
{
    IO_STATE previous_state = tls_io_instance->io_state;
    tls_io_instance->io_state = io_state;
    if (tls_io_instance->on_io_state_changed != NULL)
    {
        tls_io_instance->on_io_state_changed(tls_io_instance->callback_context, io_state, previous_state);
    }
}

/*#define SSL_WHERE_INFO(ssl, w, flag, msg) {                \
    if(w & flag) {                                         \
      printf("%20.20s", msg);                              \
      printf(" - %30.30s ", SSL_state_string_long(ssl));   \
      printf(" - %5.10s ", SSL_state_string(ssl));         \
      printf("\n");                                        \
            }                                                      \
  } 

static void ssl_client_info_callback(const SSL* ssl, int where, int ret)
{
    SSL_WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
    SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_START, "HANDSHAKE START");
    SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
}*/

static int write_outgoing_bytes(TLS_IO_INSTANCE* tls_io_instance, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    int pending = BIO_ctrl_pending(tls_io_instance->out_bio);
    if (pending <= 0)
    {
        result = 0;
    }
    else
    {
        unsigned char* bytes_to_send = malloc(pending);
        if (bytes_to_send == NULL)
        {
            result = __LINE__;
        }
        else
        {
            if (BIO_read(tls_io_instance->out_bio, bytes_to_send, pending) != pending)
            {
                result = __LINE__;
            }
            else
            {
                if (io_send(tls_io_instance->socket_io, bytes_to_send, pending, on_send_complete, callback_context) != 0)
                {
                    result = __LINE__;
                }
                else
                {

                    result = 0;
                }
            }

            free(bytes_to_send);
        }
    }

    return result;
}

static int send_handshake_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result;
    int r = 0;
    int pending = 0;

    if (SSL_is_init_finished(tls_io_instance->ssl))
    {
        tls_io_instance->handshake_state = HANDSHAKE_STATE_DONE;
        set_io_state(tls_io_instance, IO_STATE_OPEN);

        result = 0;
    }
    else
    {
        SSL_do_handshake(tls_io_instance->ssl);
        if (SSL_is_init_finished(tls_io_instance->ssl))
        {
            tls_io_instance->handshake_state = HANDSHAKE_STATE_DONE;
            set_io_state(tls_io_instance, IO_STATE_OPEN);

            result = 0;
        }
        else
        {
            if (write_outgoing_bytes(tls_io_instance, NULL, NULL) != 0)
            {
                result = __LINE__;
            }
            else
            {
                if (SSL_is_init_finished(tls_io_instance->ssl))
                {
                    tls_io_instance->handshake_state = HANDSHAKE_STATE_DONE;
                    set_io_state(tls_io_instance, IO_STATE_OPEN);
                }

                result = 0;
            }
        }
    }

    return result;
}

static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result = 0;
    unsigned char buffer[64];
    
    int rcv_bytes = 1;
    while (rcv_bytes > 0)
    {
        rcv_bytes = SSL_read(tls_io_instance->ssl, buffer, sizeof(buffer));
        if (rcv_bytes > 0)
        {
            if (tls_io_instance->on_bytes_received != NULL)
            {
                tls_io_instance->on_bytes_received(tls_io_instance->callback_context, buffer, rcv_bytes);
            }
        }
    }

    return result;
}

static void tlsio_on_bytes_received(void* context, const void* buffer, size_t size)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    int written = BIO_write(tls_io_instance->in_bio, buffer, size);
    if (written != size)
    {
        /* error */
    }
    else
    {
        switch (tls_io_instance->handshake_state)
        {
        default:
        case HANDSHAKE_STATE_NOT_STARTED:
            break;

        case HANDSHAKE_STATE_IN_HANDSHAKE:
            if (send_handshake_bytes(tls_io_instance) != 0)
            {
                /* error */
            }
            break;

        case HANDSHAKE_STATE_DONE:
            if (decode_ssl_received_bytes(tls_io_instance) != 0)
            {
                /* error */
            }
            break;
        }
    }
}

static void tlsio_on_io_state_changed(void* context, IO_STATE new_io_state, IO_STATE previous_io_state)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
}

int tlsio_openssl_init(void)
{
    (void)SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    return 0;
}

void tlsio_openssl_deinit(void)
{
}

CONCRETE_IO_HANDLE tlsio_openssl_create(void* io_create_parameters, LOGGER_LOG logger_log)
{
    TLSIO_OPENSSL_CONFIG* tls_io_config = io_create_parameters;
    TLS_IO_INSTANCE* result;

    if (tls_io_config == NULL)
    {
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(TLS_IO_INSTANCE));
        if (result != NULL)
        {
            SOCKETIO_CONFIG socketio_config;

            socketio_config.hostname = tls_io_config->hostname;
            socketio_config.port = tls_io_config->port;

            result->on_bytes_received = NULL;
            result->on_io_state_changed = NULL;
            result->logger_log = logger_log;
            result->callback_context = NULL;
            result->handshake_state = HANDSHAKE_STATE_NOT_STARTED;

            result->ssl_context = SSL_CTX_new(TLSv1_method());
            if (result->ssl_context == NULL)
            {
                free(result);
                result = NULL;
            }
            else
            {
                result->in_bio = BIO_new(BIO_s_mem());
                if (result->in_bio == NULL)
                {
                    SSL_CTX_free(result->ssl_context);
                    free(result);
                    result = NULL;
                }
                else
                {
                    result->out_bio = BIO_new(BIO_s_mem());
                    if (result->out_bio == NULL)
                    {
                        (void)BIO_free(result->out_bio);
                        SSL_CTX_free(result->ssl_context);
                        free(result);
                        result = NULL;
                    }
                    else
                    {
                        const IO_INTERFACE_DESCRIPTION* socket_io_interface = socketio_get_interface_description();
                        if (socket_io_interface == NULL)
                        {
                            (void)BIO_free(result->in_bio);
                            (void)BIO_free(result->out_bio);
                            SSL_CTX_free(result->ssl_context);
                            free(result);
                            result = NULL;
                        }
                        else
                        {
                            result->socket_io = io_create(socket_io_interface, &socketio_config, logger_log);
                            if ((result->socket_io == NULL) ||
                                (BIO_set_mem_eof_return(result->in_bio, -1) <= 0) ||
                                (BIO_set_mem_eof_return(result->out_bio, -1) <= 0))
                            {
                                (void)BIO_free(result->in_bio);
                                (void)BIO_free(result->out_bio);
                                SSL_CTX_free(result->ssl_context);
                                free(result);
                                result = NULL;
                            }
                            else
                            {
                                result->ssl = SSL_new(result->ssl_context);
                                if (result->ssl == NULL)
                                {
                                    (void)BIO_free(result->in_bio);
                                    (void)BIO_free(result->out_bio);
                                    SSL_CTX_free(result->ssl_context);
                                    free(result);
                                    result = NULL;
                                }
                                else
                                {
                                    //SSL_set_info_callback(result->ssl, ssl_client_info_callback);
                                    SSL_set_verify(result->ssl, SSL_VERIFY_NONE, NULL);
                                    SSL_set_bio(result->ssl, result->in_bio, result->out_bio);
                                    SSL_set_connect_state(result->ssl);

                                    set_io_state(result, IO_STATE_NOT_OPEN);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

void tlsio_openssl_destroy(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        SSL_free(tls_io_instance->ssl);
        SSL_CTX_free(tls_io_instance->ssl_context);

        io_destroy(tls_io_instance->socket_io);
        free(tls_io);
    }
}

int tlsio_openssl_open(CONCRETE_IO_HANDLE tls_io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context)
{
    int result;

    if (tls_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->io_state != IO_STATE_NOT_OPEN)
        {
            result = __LINE__;
        }
        else
        {
            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_io_state_changed = on_io_state_changed;
            tls_io_instance->callback_context = callback_context;

            set_io_state(tls_io_instance, IO_STATE_OPENING);

            if (io_open(tls_io_instance->socket_io, tlsio_on_bytes_received, tlsio_on_io_state_changed, tls_io_instance) != 0)
            {
                set_io_state(tls_io_instance, IO_STATE_ERROR);
                result = __LINE__;
            }
            else
            {
                if (SSL_is_init_finished(tls_io_instance->ssl))
                {
                    result = __LINE__;
                }
                else
                {
                    tls_io_instance->handshake_state = HANDSHAKE_STATE_IN_HANDSHAKE;

                    if (send_handshake_bytes(tls_io_instance) != 0)
                    {
                        result = __LINE__;
                    }
                    else
                    {
                        result = 0;
                    }
                }
            }
        }
    }

    return result;
}

int tlsio_openssl_close(CONCRETE_IO_HANDLE tls_io)
{
    int result = 0;

    if (tls_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        (void)io_close(tls_io_instance->socket_io);
        set_io_state(tls_io_instance, IO_STATE_NOT_OPEN);
    }

    return result;
}

int tlsio_openssl_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if (tls_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        int res = SSL_write(tls_io_instance->ssl, buffer, size);
        if (res != size)
        {
            result = __LINE__;
        }
        else
        {
            if (write_outgoing_bytes(tls_io_instance, on_send_complete, callback_context) != 0)
            {
                result = __LINE__;
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
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        io_dowork(tls_io_instance->socket_io);
    }
}

const IO_INTERFACE_DESCRIPTION* tlsio_openssl_get_interface_description(void)
{
    return &tlsio_openssl_interface_description;
}
