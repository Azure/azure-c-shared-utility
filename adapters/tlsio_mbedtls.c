// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "mbed_wait_api.h"
#include "mbedtls/config.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy_poll.h"

#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"

static const char *const OPTION_UNDERLYING_IO_OPTIONS = "underlying_io_options";

#define HANDSHAKE_TIMEOUT_MS 5000
#define HANDSHAKE_WAIT_INTERVAL_MS 10

typedef enum TLSIO_STATE_ENUM_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING_UNDERLYING_IO,
    TLSIO_STATE_IN_HANDSHAKE,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE_ENUM;

typedef struct TLS_IO_INSTANCE_TAG
{
    XIO_HANDLE socket_io;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    ON_IO_ERROR on_io_error;
    void *on_bytes_received_context;
    void *on_io_open_complete_context;
    void *on_io_close_complete_context;
    void *on_io_error_context;
    TLSIO_STATE_ENUM tlsio_state;
    unsigned char *socket_io_read_bytes;
    size_t socket_io_read_byte_count;
    ON_SEND_COMPLETE on_send_complete;
    void *on_send_complete_callback_context;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config config;
    mbedtls_x509_crt trusted_certificates_parsed;
    mbedtls_ssl_session ssn;
    char *trusted_certificates;

    char *hostname;
    mbedtls_x509_crt owncert;
    mbedtls_pk_context pKey;
    int tls_status;
} TLS_IO_INSTANCE;

typedef enum TLS_STATE_TAG
{
    TLS_STATE_NOT_INITIALIZED,
    TLS_STATE_INITIALIZED,
    TLS_STATE_CLOSING,
} TLS_STATE;

static void indicate_error(TLS_IO_INSTANCE *tls_io_instance)
{
    if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) || (tls_io_instance->tlsio_state == TLSIO_STATE_ERROR))
    {
        return;
    }
    tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
    if (tls_io_instance->on_io_error != NULL)
    {
        tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
    }
}

static void indicate_open_complete(TLS_IO_INSTANCE *tls_io_instance, IO_OPEN_RESULT open_result)
{
    if (tls_io_instance->on_io_open_complete != NULL)
    {
        tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, open_result);
    }
}

static int decode_ssl_received_bytes(TLS_IO_INSTANCE *tls_io_instance)
{
    int result = 0;
    unsigned char buffer[64];
    int rcv_bytes = 1;

    while (rcv_bytes > 0)
    {
        rcv_bytes = mbedtls_ssl_read(&tls_io_instance->ssl, buffer, sizeof(buffer));
        if (rcv_bytes > 0)
        {
            if (tls_io_instance->on_bytes_received != NULL)
            {
                tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, rcv_bytes);
            }
        }
    }

    return result;
}

static void on_underlying_io_open_complete(void *context, IO_OPEN_RESULT open_result)
{
    if (context == NULL)
    {
        LogError("Invalid context NULL value passed");
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)context;
        int result = 0;

        if (open_result != IO_OPEN_OK)
        {
            xio_close(tls_io_instance->socket_io, NULL, NULL);
            tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
            indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
        }
        else
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_IN_HANDSHAKE;

            do
            {
                result = mbedtls_ssl_handshake(&tls_io_instance->ssl);
            } while (result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE);

            if (result == 0)
            {
                tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
                indicate_open_complete(tls_io_instance, IO_OPEN_OK);
            }
            else
            {
                xio_close(tls_io_instance->socket_io, NULL, NULL);
                tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
                indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
            }
        }
    }
}

static void on_underlying_io_bytes_received(void *context, const unsigned char *buffer, size_t size)
{
    if (context != NULL)
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)context;

        unsigned char *new_socket_io_read_bytes = (unsigned char *)realloc(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_byte_count + size);

        if (new_socket_io_read_bytes == NULL)
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
            indicate_error(tls_io_instance);
        }
        else
        {
            tls_io_instance->socket_io_read_bytes = new_socket_io_read_bytes;
            (void)memcpy(tls_io_instance->socket_io_read_bytes + tls_io_instance->socket_io_read_byte_count, buffer, size);
            tls_io_instance->socket_io_read_byte_count += size;
        }
    }
    else
    {
        LogError("NULL value passed in context");
    }
}

static void on_underlying_io_error(void *context)
{
    if (context == NULL)
    {
        LogError("Invalid context NULL value passed");
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)context;

        switch (tls_io_instance->tlsio_state)
        {
        default:
        case TLSIO_STATE_NOT_OPEN:
        case TLSIO_STATE_ERROR:
            break;

        case TLSIO_STATE_OPENING_UNDERLYING_IO:
        case TLSIO_STATE_IN_HANDSHAKE:
            // Existing socket impls are all synchronous close, and this
            // adapter does not yet support async close.
            xio_close(tls_io_instance->socket_io, NULL, NULL);
            tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
            indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
            break;

        case TLSIO_STATE_OPEN:
            indicate_error(tls_io_instance);
            break;
        }
    }
}

static void on_underlying_io_close_complete_during_close(void *context)
{
    if (context == NULL)
    {
        LogError("Invalid context NULL value passed");
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)context;

        tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;

        if (tls_io_instance->on_io_close_complete != NULL)
        {
            tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
        }
    }
}

static int on_io_recv(void *context, unsigned char *buf, size_t sz)
{
    int result;
    if (context == NULL)
    {
        LogError("Invalid context NULL value passed");
        result = MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)context;
        unsigned char *new_socket_io_read_bytes;
        int pending = 0;

        while (tls_io_instance->socket_io_read_byte_count == 0)
        {
            xio_dowork(tls_io_instance->socket_io);

            if (tls_io_instance->tlsio_state == TLSIO_STATE_OPEN)
            {
                break;
            }
            else if (tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN ||
                     tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING ||
                     tls_io_instance->tlsio_state == TLSIO_STATE_ERROR)
            {
                // Underlying io error, exit.
                return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
            }
            else
            {
                // Handkshake
                if (tls_io_instance->socket_io_read_byte_count == 0)
                {
                    if (pending++ >= HANDSHAKE_TIMEOUT_MS / HANDSHAKE_WAIT_INTERVAL_MS)
                    {
                        // The connection is close from server side and no response.
                        LogError("Tlsio_Failure: encountered unknow connection issue, the connection will be restarted.");
                        indicate_error(tls_io_instance);
                        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
                    }
                    wait_ms(HANDSHAKE_WAIT_INTERVAL_MS);
                }
            }
        }

        result = tls_io_instance->socket_io_read_byte_count;
        if (result > (int)sz)
        {
            result = sz;
        }

        if (result > 0)
        {
            (void)memcpy((void *)buf, tls_io_instance->socket_io_read_bytes, result);
            (void)memmove(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_bytes + result, tls_io_instance->socket_io_read_byte_count - result);
            tls_io_instance->socket_io_read_byte_count -= result;
            if (tls_io_instance->socket_io_read_byte_count > 0)
            {
                new_socket_io_read_bytes = (unsigned char *)realloc(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_byte_count);
                if (new_socket_io_read_bytes != NULL)
                {
                    tls_io_instance->socket_io_read_bytes = new_socket_io_read_bytes;
                }
            }
            else
            {
                free(tls_io_instance->socket_io_read_bytes);
                tls_io_instance->socket_io_read_bytes = NULL;
            }
        }

        if ((result == 0) && (tls_io_instance->tlsio_state == TLSIO_STATE_OPEN))
        {
            result = MBEDTLS_ERR_SSL_WANT_READ;
        }
    }

    return result;
}

static int on_io_send(void *context, const unsigned char *buf, size_t sz)
{
    int result;
    if (context == NULL)
    {
        LogError("Invalid context NULL value passed");
        result = 0;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)context;

        if (xio_send(tls_io_instance->socket_io, buf, sz, tls_io_instance->on_send_complete, tls_io_instance->on_send_complete_callback_context) != 0)
        {
            indicate_error(tls_io_instance);
            result = 0;
        }
        else
        {
            result = sz;
        }
    }
    return result;
}

static int tlsio_entropy_poll(void *v, unsigned char *output, size_t len, size_t *olen)
{
    (void)v;
    int result = 0;
    srand((unsigned int)time(NULL));
    for (uint16_t i = 0; i < len; i++)
    {
        output[i] = rand() % 256;
    }
    *olen = len;
    return result;
}

// Un-initialize mbedTLS
static void mbedtls_uninit(TLS_IO_INSTANCE *tls_io_instance)
{
    if (tls_io_instance->tls_status != TLS_STATE_NOT_INITIALIZED)
    {
        // mbedTLS cleanup...
        mbedtls_ssl_free(&tls_io_instance->ssl);
        mbedtls_ssl_config_free(&tls_io_instance->config);
        mbedtls_x509_crt_free(&tls_io_instance->trusted_certificates_parsed);
        mbedtls_ctr_drbg_free(&tls_io_instance->ctr_drbg);
        mbedtls_entropy_free(&tls_io_instance->entropy);

        tls_io_instance->tls_status = TLS_STATE_NOT_INITIALIZED;
    }
}

// Initialize mbedTLS
static void mbedtls_init(TLS_IO_INSTANCE *tls_io_instance)
{
    if (tls_io_instance->tls_status == TLS_STATE_INITIALIZED)
    {
        // Already initialized
        return;
    }
    else if (tls_io_instance->tls_status == TLS_STATE_CLOSING)
    {
        // The underlying connection has been closed, so here un-initialize first
        mbedtls_uninit(tls_io_instance);
    }

    const char *pers = "azure_iot_client";

    // mbedTLS initialize...
    mbedtls_x509_crt_init(&tls_io_instance->trusted_certificates_parsed);

    mbedtls_entropy_init(&tls_io_instance->entropy);
    // Add a weak entropy source here,avoid some platform doesn't have strong / hardware entropy
    mbedtls_entropy_add_source(&tls_io_instance->entropy, tlsio_entropy_poll, NULL, MBEDTLS_ENTROPY_MAX_GATHER, MBEDTLS_ENTROPY_SOURCE_WEAK);

    mbedtls_ctr_drbg_init(&tls_io_instance->ctr_drbg);
    mbedtls_ctr_drbg_seed(&tls_io_instance->ctr_drbg, mbedtls_entropy_func, &tls_io_instance->entropy, (const unsigned char *)pers, strlen(pers));

    mbedtls_ssl_config_init(&tls_io_instance->config);
    mbedtls_ssl_config_defaults(&tls_io_instance->config, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_rng(&tls_io_instance->config, mbedtls_ctr_drbg_random, &tls_io_instance->ctr_drbg);
    mbedtls_ssl_conf_authmode(&tls_io_instance->config, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_min_version(&tls_io_instance->config, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3); // v1.2

    mbedtls_ssl_init(&tls_io_instance->ssl);
    mbedtls_ssl_set_bio(&tls_io_instance->ssl, tls_io_instance, on_io_send, on_io_recv, NULL);
    mbedtls_ssl_set_hostname(&tls_io_instance->ssl, tls_io_instance->hostname);

    mbedtls_ssl_session_init(&tls_io_instance->ssn);

    mbedtls_ssl_set_session(&tls_io_instance->ssl, &tls_io_instance->ssn);
    mbedtls_ssl_setup(&tls_io_instance->ssl, &tls_io_instance->config);

    tls_io_instance->tls_status = TLS_STATE_INITIALIZED;
}

CONCRETE_IO_HANDLE tlsio_mbedtls_create(void *io_create_parameters)
{
    TLSIO_CONFIG *tls_io_config = (TLSIO_CONFIG *)io_create_parameters;
    TLS_IO_INSTANCE *result;

    if (tls_io_config == NULL)
    {
        LogError("NULL tls_io_config");
        result = NULL;
    }
    else
    {
        /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_006: [ The tlsio_mbedtls_create shall return NULL if allocating memory for TLS_IO_INSTANCE failed. ]*/
        result = calloc(1, sizeof(TLS_IO_INSTANCE));
        if (result != NULL)
        {
            SOCKETIO_CONFIG socketio_config;
            const IO_INTERFACE_DESCRIPTION *underlying_io_interface;
            void *io_interface_parameters;

            if (tls_io_config->underlying_io_interface != NULL)
            {
                underlying_io_interface = tls_io_config->underlying_io_interface;
                io_interface_parameters = tls_io_config->underlying_io_parameters;
            }
            else
            {
                socketio_config.hostname = tls_io_config->hostname;
                socketio_config.port = tls_io_config->port;
                socketio_config.accepted_socket = NULL;
                underlying_io_interface = socketio_get_interface_description();
                io_interface_parameters = &socketio_config;
            }

            if (underlying_io_interface == NULL)
            {
                free(result);
                result = NULL;
                LogError("Failed getting socket IO interface description.");
            }
            else
            {

                result->hostname = strdup(tls_io_config->hostname);
                if (result->hostname == NULL)
                {
                    free(result);
                    result = NULL;
                }
                else
                {
                    result->socket_io = xio_create(underlying_io_interface, io_interface_parameters);
                    if (result->socket_io == NULL)
                    {
                        LogError("socket xio create failed");
                        free(result->hostname);
                        free(result);
                        result = NULL;
                    }
                    else
                    {
                        result->tls_status = TLS_STATE_NOT_INITIALIZED;
                        mbedtls_init(result);

                        result->tlsio_state = TLSIO_STATE_NOT_OPEN;
                    }
                }
            }
        }
    }

    return result;
}

void tlsio_mbedtls_destroy(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;

        mbedtls_uninit(tls_io_instance);

        if (tls_io_instance->socket_io_read_bytes != NULL)
        {
            free(tls_io_instance->socket_io_read_bytes);
            tls_io_instance->socket_io_read_bytes = NULL;
        }
        if (tls_io_instance->hostname != NULL)
        {
            free(tls_io_instance->hostname);
            tls_io_instance->hostname = NULL;
        }
        if (tls_io_instance->trusted_certificates != NULL)
        {
            free(tls_io_instance->trusted_certificates);
            tls_io_instance->trusted_certificates = NULL;
        }

        xio_destroy(tls_io_instance->socket_io);

        free(tls_io);
    }
}

int tlsio_mbedtls_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void *on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void *on_bytes_received_context, ON_IO_ERROR on_io_error, void *on_io_error_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        LogError("NULL tls_io");
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
            LogError("IO should not be open: %d\n", tls_io_instance->tlsio_state);
            result = __FAILURE__;
        }
        else
        {

            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_bytes_received_context = on_bytes_received_context;

            tls_io_instance->on_io_open_complete = on_io_open_complete;
            tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            tls_io_instance->tlsio_state = TLSIO_STATE_OPENING_UNDERLYING_IO;

            mbedtls_ssl_session_reset(&tls_io_instance->ssl);

            if (xio_open(tls_io_instance->socket_io, on_underlying_io_open_complete, tls_io_instance, on_underlying_io_bytes_received, tls_io_instance, on_underlying_io_error, tls_io_instance) != 0)
            {

                LogError("Underlying IO open failed");
                tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
                result = __FAILURE__;
            }
        }
    }
    return result;
}

int tlsio_mbedtls_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void *callback_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
        {
            result = __FAILURE__;
        }
        else
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
            tls_io_instance->on_io_close_complete = on_io_close_complete;
            tls_io_instance->on_io_close_complete_context = callback_context;
            if (xio_close(tls_io_instance->socket_io,
                          on_underlying_io_close_complete_during_close, tls_io_instance) != 0)
            {
                result = __FAILURE__;
            }
            else
            {
                if (tls_io_instance->tls_status == TLS_STATE_INITIALIZED)
                {
                    mbedtls_ssl_close_notify(&tls_io_instance->ssl);
                    tls_io_instance->tls_status = TLS_STATE_CLOSING;
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

int tlsio_mbedtls_send(CONCRETE_IO_HANDLE tls_io, const void *buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void *callback_context)
{
    int result = 0;

    if (tls_io == NULL || (buffer == NULL) || (size == 0))
    {
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;
        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            result = __FAILURE__;
        }
        else
        {
            tls_io_instance->on_send_complete = on_send_complete;
            tls_io_instance->on_send_complete_callback_context = callback_context;
            int res = mbedtls_ssl_write(&tls_io_instance->ssl, buffer, size);
            if (res != (int)size)
            {
                result = __FAILURE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

void tlsio_mbedtls_dowork(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;
        if (tls_io_instance->tlsio_state == TLSIO_STATE_OPENING_UNDERLYING_IO || tls_io_instance->tlsio_state == TLSIO_STATE_IN_HANDSHAKE || tls_io_instance->tlsio_state == TLSIO_STATE_OPEN)
        {
            decode_ssl_received_bytes(tls_io_instance);
            // Note: no need to call xio_dowork here because it's called in on_io_recv which is the callback function of decode_ssl_received_bytes
        }
    }
}

/*this function will clone an option given by name and value*/
static void *tlsio_mbedtls_CloneOption(const char *name, const void *value)
{
    void *result = NULL;
    if (name == NULL || value == NULL)
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
        result = NULL;
    }
    else
    {
        if (strcmp(name, OPTION_UNDERLYING_IO_OPTIONS) == 0)
        {
            result = (void *)value;
        }
        else if (strcmp(name, OPTION_TRUSTED_CERT) == 0)
        {
            if (mallocAndStrcpy_s((char **)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s TrustedCerts value");
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
static void tlsio_mbedtls_DestroyOption(const char *name, const void *value)
{
    /*since all options for this layer are actually string copies., disposing of one is just calling free*/
    if (name == NULL || value == NULL)
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
    }
    else
    {
        if (strcmp(name, OPTION_TRUSTED_CERT) == 0)
        {
            free((void *)value);
        }
        else if (strcmp(name, OPTION_UNDERLYING_IO_OPTIONS) == 0)
        {
            OptionHandler_Destroy((OPTIONHANDLER_HANDLE)value);
        }
        else
        {
            LogError("not handled option : %s", name);
        }
    }
}

int tlsio_mbedtls_setoption(CONCRETE_IO_HANDLE tls_io, const char *optionName, const void *value)
{
    int result = 0;

    if (tls_io == NULL || optionName == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;

        if (strcmp(OPTION_TRUSTED_CERT, optionName) == 0)
        {
            if (tls_io_instance->trusted_certificates != NULL)
            {
                // Free the memory if it has been previously allocated
                free(tls_io_instance->trusted_certificates);
                tls_io_instance->trusted_certificates = NULL;
            }
            if (mallocAndStrcpy_s(&tls_io_instance->trusted_certificates, (const char *)value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s");
                result = __FAILURE__;
            }
            else
            {
                int parse_result = mbedtls_x509_crt_parse(&tls_io_instance->trusted_certificates_parsed, (const unsigned char *)value, (int)(strlen(value) + 1));
                if (parse_result != 0)
                {
                    LogInfo("Malformed pem certificate");
                    result = __FAILURE__;
                }
                else
                {
                    mbedtls_ssl_conf_ca_chain(&tls_io_instance->config, &tls_io_instance->trusted_certificates_parsed, NULL);
                }
            }
        }
        else if (strcmp(SU_OPTION_X509_CERT, optionName) == 0 || strcmp(OPTION_X509_ECC_CERT, optionName) == 0)
        {
            if (mbedtls_x509_crt_parse(&tls_io_instance->owncert, (const unsigned char *)value, (int)(strlen(value) + 1)) != 0)
            {
                result = __FAILURE__;
            }
            else if (tls_io_instance->pKey.pk_info != NULL)
            {
                if (mbedtls_ssl_conf_own_cert(&tls_io_instance->config, &tls_io_instance->owncert, &tls_io_instance->pKey) != 0)
                {
                    result = __FAILURE__;
                }
            }
        }
        else if (strcmp(SU_OPTION_X509_PRIVATE_KEY, optionName) == 0 || strcmp(OPTION_X509_ECC_KEY, optionName) == 0)
        {
            if (mbedtls_pk_parse_key(&tls_io_instance->pKey, (const unsigned char *)value, (int)(strlen(value) + 1), NULL, 0) != 0)
            {
                result = __FAILURE__;
            }
            else if (tls_io_instance->owncert.version > 0)
            {
                if (mbedtls_ssl_conf_own_cert(&tls_io_instance->config, &tls_io_instance->owncert, &tls_io_instance->pKey))
                {
                    result = __FAILURE__;
                }
            }
        }
        else
        {
            // tls_io_instance->socket_io is never NULL
            result = xio_setoption(tls_io_instance->socket_io, optionName, value);
        }
    }

    return result;
}

OPTIONHANDLER_HANDLE tlsio_mbedtls_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result = NULL;
    if (handle == NULL)
    {
        LogError("invalid parameter detected: CONCRETE_IO_HANDLE handle=%p", handle);
        result = NULL;
    }
    else
    {
        result = OptionHandler_Create(tlsio_mbedtls_CloneOption, tlsio_mbedtls_DestroyOption, tlsio_mbedtls_setoption);
        if (result == NULL)
        {
            LogError("unable to OptionHandler_Create");
            /*return as is*/
        }
        else
        {
            /*this layer cares about the certificates*/
            TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)handle;
            OPTIONHANDLER_HANDLE underlying_io_options;

            if ((underlying_io_options = xio_retrieveoptions(tls_io_instance->socket_io)) == NULL ||
                OptionHandler_AddOption(result, OPTION_UNDERLYING_IO_OPTIONS, underlying_io_options) != OPTIONHANDLER_OK)
            {
                LogError("unable to save underlying_io options");
                OptionHandler_Destroy(underlying_io_options);
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else if (tls_io_instance->trusted_certificates != NULL &&
                     OptionHandler_AddOption(result, OPTION_TRUSTED_CERT, tls_io_instance->trusted_certificates) != OPTIONHANDLER_OK)
            {
                LogError("unable to save TrustedCerts option");
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

static const IO_INTERFACE_DESCRIPTION tlsio_mbedtls_interface_description =
    {
        tlsio_mbedtls_retrieveoptions,
        tlsio_mbedtls_create,
        tlsio_mbedtls_destroy,
        tlsio_mbedtls_open,
        tlsio_mbedtls_close,
        tlsio_mbedtls_send,
        tlsio_mbedtls_dowork,
        tlsio_mbedtls_setoption};

const IO_INTERFACE_DESCRIPTION *tlsio_mbedtls_get_interface_description(void)
{
    return &tlsio_mbedtls_interface_description;
}
