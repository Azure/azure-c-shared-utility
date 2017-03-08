// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/crypto.h"
#include "openssl/opensslv.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_openssl.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/x509_openssl.h"

typedef enum TLSIO_STATE_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING_UNDERLYING_IO,
    TLSIO_STATE_IN_HANDSHAKE,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE;

typedef int(*TLS_CERTIFICATE_VALIDATION_CALLBACK)(X509_STORE_CTX*, void*);

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
    BIO* in_bio;
    BIO* out_bio;
    TLSIO_STATE tlsio_state;
    char* certificate;
    const char* x509certificate;
    const char* x509privatekey;
    int tls_version;
    TLS_CERTIFICATE_VALIDATION_CALLBACK tls_validation_callback;
    void* tls_validation_callback_data;
} TLS_IO_INSTANCE;

struct CRYPTO_dynlock_value 
{
    LOCK_HANDLE lock; 
};

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
        else if (
            (strcmp(name, "tls_version") == 0) ||
            (strcmp(name, "tls_validation_callback") == 0) ||
            (strcmp(name, "tls_validation_callback_data") == 0)
            )
        {
            result = (void*)value;
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
        else if (
            (strcmp(name, "tls_version") == 0) ||
            (strcmp(name, "tls_validation_callback") == 0) ||
            (strcmp(name, "tls_validation_callback_data") == 0)
            )
        {
            // nothing to free.
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
            else if (tls_io_instance->tls_version != 0)
            {
                if (OptionHandler_AddOption(result, "tls_version", (void*)(intptr_t)tls_io_instance->tls_version) != 0)
                {
                    LogError("unable to save tls_version option");
                    OptionHandler_Destroy(result);
                    result = NULL;
                }
            }
            else if (tls_io_instance->tls_validation_callback != NULL)
            {
                #pragma warning(push)
                #pragma warning(disable:4152)
                void* ptr = tls_io_instance->tls_validation_callback;
                #pragma warning(pop)

                if (OptionHandler_AddOption(result, "tls_validation_callback", (const char*)ptr) != 0)
                {
                    LogError("unable to save tls_validation_callback option");
                    OptionHandler_Destroy(result);
                    result = NULL;
                }

                if (OptionHandler_AddOption(result, "tls_validation_callback_data", (const char*)tls_io_instance->tls_validation_callback_data) != 0)
                {
                    LogError("unable to save tls_validation_callback_data option");
                    OptionHandler_Destroy(result);
                    result = NULL;
                }
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


static void openssl_lock_unlock_helper(LOCK_HANDLE lock, int lock_mode, const char* file, int line)
{
    if (lock_mode & CRYPTO_LOCK)
    {
        if (Lock(lock) != 0)
        {
            LogError("Failed to lock openssl lock (%s:%d)", file, line);
        }
    }
    else
    {
        if (Unlock(lock) != 0)
        {
            LogError("Failed to unlock openssl lock (%s:%d)", file, line);
        }
    }
}

static void log_ERR_get_error(const char* message)
{
    char buf[128];
    unsigned long error;
    
    if (message != NULL)
    {
        LogError("%s", message);
    }
    
    error = ERR_get_error();
    
    for(int i = 0; 0 != error; i++)
    {
        LogError("  [%d] %s", i, ERR_error_string(error, buf));
        error = ERR_get_error();
    }
}

static struct CRYPTO_dynlock_value* openssl_dynamic_locks_create_cb(const char* file, int line)
{
    struct CRYPTO_dynlock_value* result;
    
    result = malloc(sizeof(struct CRYPTO_dynlock_value));
    
    if (result == NULL)
    {
        LogError("Failed to allocate lock!  Out of memory (%s:%d).", file, line);
    }
    else
    {
        result->lock = Lock_Init();
        if (result->lock == NULL)
        {
            LogError("Failed to create lock for dynamic lock (%s:%d).", file, line);
            
            free(result);
            result = NULL;
        }
    }
    
    return result;
}

static void openssl_dynamic_locks_lock_unlock_cb(int lock_mode, struct CRYPTO_dynlock_value* dynlock_value, const char* file, int line)
{
    openssl_lock_unlock_helper(dynlock_value->lock, lock_mode, file, line);
}

static void openssl_dynamic_locks_destroy_cb(struct CRYPTO_dynlock_value* dynlock_value, const char* file, int line)
{
    (void)file;
    (void)line;
    Lock_Deinit(dynlock_value->lock);
    free(dynlock_value);
}

static void openssl_dynamic_locks_uninstall(void)
{
#if (OPENSSL_VERSION_NUMBER >= 0x00906000)
    CRYPTO_set_dynlock_create_callback(NULL);
    CRYPTO_set_dynlock_lock_callback(NULL);
    CRYPTO_set_dynlock_destroy_callback(NULL);
#endif
}

static void openssl_dynamic_locks_install(void)
{
#if (OPENSSL_VERSION_NUMBER >= 0x00906000)
    CRYPTO_set_dynlock_destroy_callback(openssl_dynamic_locks_destroy_cb);
    CRYPTO_set_dynlock_lock_callback(openssl_dynamic_locks_lock_unlock_cb);
    CRYPTO_set_dynlock_create_callback(openssl_dynamic_locks_create_cb);
#endif
}

static void openssl_static_locks_lock_unlock_cb(int lock_mode, int lock_index, const char * file, int line)
{
    if (lock_index < 0 || lock_index >= CRYPTO_num_locks())
    {
        LogError("Bad lock index %d passed (%s:%d)", lock_index, file, line);
    }
    else
    {
        openssl_lock_unlock_helper(openssl_locks[lock_index], lock_mode, file, line);
    }
}

static void openssl_static_locks_uninstall(void)
{
    if (openssl_locks != NULL)
    {
        CRYPTO_set_locking_callback(NULL);
        
        for(int i = 0; i < CRYPTO_num_locks(); i++)
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

static int openssl_static_locks_install(void)
{
    int result;
    
    if (openssl_locks != NULL)
    {
        LogError("Locks already initialized");
        result = __FAILURE__;
    }
    else
    {
        openssl_locks = malloc(CRYPTO_num_locks() * sizeof(LOCK_HANDLE));
        if(openssl_locks == NULL)
        {
            LogError("Failed to allocate locks");
            result = __FAILURE__;
        }
        else
        {
            int i;
            for(i = 0; i < CRYPTO_num_locks(); i++)
            {
                openssl_locks[i] = Lock_Init();
                if (openssl_locks[i] == NULL)
                {
                    LogError("Failed to allocate lock %d", i);
                    break;
                }
            }
            
            if (i != CRYPTO_num_locks())
            {
                for (int j = 0; j < i; j++)
                {
                    Lock_Deinit(openssl_locks[j]);
                }
                result = __FAILURE__;
            }
            else
            {
                CRYPTO_set_locking_callback(openssl_static_locks_lock_unlock_cb);
                
                result = 0;
            }
        }
    }
    return result;
}

static void indicate_error(TLS_IO_INSTANCE* tls_io_instance)
{
    if (tls_io_instance->on_io_error == NULL)
    {
        LogError("NULL on_io_error.");
    }
    else
    {
        tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
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
            LogError("NULL bytes_to_send.");
            result = __FAILURE__;
        }
        else
        {
            if (BIO_read(tls_io_instance->out_bio, bytes_to_send, pending) != pending)
            {
                log_ERR_get_error("BIO_read not in pending state.");
                result = __FAILURE__;
            }
            else
            {
                if (xio_send(tls_io_instance->underlying_io, bytes_to_send, pending, on_send_complete, callback_context) != 0)
                {
                    LogError("Error in xio_send.");
                    result = __FAILURE__;
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

    if (tls_io_instance->ssl == NULL)
    {
        LogError("SSL channel closed in send_handshake_bytes.");
        result = __FAILURE__;
        return result;
    }

    if (SSL_is_init_finished(tls_io_instance->ssl))
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
        indicate_open_complete(tls_io_instance, IO_OPEN_OK);
        result = 0;
    }
    else
    {
        SSL_do_handshake(tls_io_instance->ssl);
        if (SSL_is_init_finished(tls_io_instance->ssl))
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
            indicate_open_complete(tls_io_instance, IO_OPEN_OK);
            result = 0;
        }
        else
        {
            if (write_outgoing_bytes(tls_io_instance, NULL, NULL) != 0)
            {
                LogError("Error in write_outgoing_bytes.");
                result = __FAILURE__;
            }
            else
            {
                if (SSL_is_init_finished(tls_io_instance->ssl))
                {
                    tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
                    indicate_open_complete(tls_io_instance, IO_OPEN_OK);
                }

                result = 0;
            }
        }
    }

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
                    tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
                    indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
                    LogError("Error in xio_close.");
                }
            }
        }
        else
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
            indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPENING_UNDERLYING_IO.");
        }
    }
}

static void on_underlying_io_error(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    switch (tls_io_instance->tlsio_state)
    {
        default:
            break;

        case TLSIO_STATE_OPENING_UNDERLYING_IO:
        case TLSIO_STATE_IN_HANDSHAKE:
            tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
            indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
            break;

        case TLSIO_STATE_OPEN:
            indicate_error(tls_io_instance);
            break;
    }
}

static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result = 0;
    unsigned char buffer[64];

    int rcv_bytes = 1;

    while (rcv_bytes > 0)
    {
        if (tls_io_instance->ssl == NULL)
        {
            LogError("SSL channel closed in decode_ssl_received_bytes.");
            result = __FAILURE__;
            return result;
        }

        rcv_bytes = SSL_read(tls_io_instance->ssl, buffer, sizeof(buffer));
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
    }

    return result;
}

static void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    int written = BIO_write(tls_io_instance->in_bio, buffer, size);
    if (written != (int)size)
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
        indicate_error(tls_io_instance);
        log_ERR_get_error("Error in BIO_write.");
    }
    else
    {
        switch (tls_io_instance->tlsio_state)
        {
            default:
                break;

            case TLSIO_STATE_IN_HANDSHAKE:
                if (send_handshake_bytes(tls_io_instance) != 0)
                {
                    if (xio_close(tls_io_instance->underlying_io, on_underlying_io_close_complete, tls_io_instance) != 0)
                    {
                        tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
                        indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
                        LogError("Error in xio_close.");
                    }
                }
                break;

            case TLSIO_STATE_OPEN:
                if (decode_ssl_received_bytes(tls_io_instance) != 0)
                {
                    tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                    indicate_error(tls_io_instance);
                    LogError("Error in decode_ssl_received_bytes.");
                }
                break;
        }
    }
}

static void close_openssl_instance(TLS_IO_INSTANCE* tls_io_instance)
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
    int result = 0;

    if (certValue != NULL)
    {
        X509_STORE* cert_store = SSL_CTX_get_cert_store(tls_io_instance->ssl_context);
        if (cert_store == NULL)
        {
            log_ERR_get_error("failure in SSL_CTX_get_cert_store.");
            result = __FAILURE__;
        }
        else
        {
            BIO_METHOD* bio_method = BIO_s_mem();
            if (bio_method == NULL)
            {
                log_ERR_get_error("failure in BIO_s_mem");
                result = __FAILURE__;
            }
            else
            {
                BIO* cert_memory_bio = BIO_new(bio_method);

                if (cert_memory_bio == NULL)
                {
                    log_ERR_get_error("failure in BIO_new");
                    result = __FAILURE__;
                }
                else
                {
                    int puts_result = BIO_puts(cert_memory_bio, certValue);
                    if (puts_result < 0)
                    {
                        log_ERR_get_error("failure in BIO_puts");
                        result = __FAILURE__;
                    }
                    else
                    {
                        if ((size_t)puts_result != strlen(certValue))
                        {
                            log_ERR_get_error("mismatching legths");
                            result = __FAILURE__;
                        }
                        else
                        {
                            X509* certificate;
                            while ((certificate = PEM_read_bio_X509(cert_memory_bio, NULL, NULL, NULL)) != NULL)
                            {
                                if (!X509_STORE_add_cert(cert_store, certificate))
                                {
                                    X509_free(certificate);
                                    log_ERR_get_error("failure in X509_STORE_add_cert");
                                    break;
                                }
                                X509_free(certificate);
                            }
                            if (certificate == NULL)
                            {
                                result = 0;/*all is fine*/
                            }
                            else
                            {
                                /*previous while loop terminated unfortunately*/
                                result = __FAILURE__;
                            }
                        }
                    }
                    BIO_free(cert_memory_bio);
                }
            }
        }
    }
    return result;
}

static int create_openssl_instance(TLS_IO_INSTANCE* tlsInstance)
{
    int result;

    const SSL_METHOD* method = TLSv1_method();

    if (tlsInstance->tls_version == 12)
    {
        method = TLSv1_2_method();
    }
    else if (tlsInstance->tls_version == 11)
    {
        method = TLSv1_1_method();
    }

    tlsInstance->ssl_context = SSL_CTX_new(method);
    if (tlsInstance->ssl_context == NULL)
    {
        log_ERR_get_error("Failed allocating OpenSSL context.");
        result = __FAILURE__;
    }
    else if (add_certificate_to_store(tlsInstance, tlsInstance->certificate) != 0)
    {
        SSL_CTX_free(tlsInstance->ssl_context);
        tlsInstance->ssl_context = NULL;
        log_ERR_get_error("unable to add_certificate_to_store.");
        result = __FAILURE__;
    }
    /*x509 authentication can only be build before underlying connection is realized*/
    else if(
            (tlsInstance->x509certificate != NULL) &&
            (tlsInstance->x509privatekey != NULL) &&
            (x509_openssl_add_credentials(tlsInstance->ssl_context, tlsInstance->x509certificate, tlsInstance->x509privatekey) != 0)
        )
        {
            SSL_CTX_free(tlsInstance->ssl_context);
            tlsInstance->ssl_context = NULL;
            log_ERR_get_error("unable to use x509 authentication");
            result = __FAILURE__;
        }
    else
    {
        SSL_CTX_set_cert_verify_callback(tlsInstance->ssl_context, tlsInstance->tls_validation_callback, tlsInstance->tls_validation_callback_data);

        tlsInstance->in_bio = BIO_new(BIO_s_mem());
        if (tlsInstance->in_bio == NULL)
        {
            SSL_CTX_free(tlsInstance->ssl_context);
            tlsInstance->ssl_context = NULL;
            log_ERR_get_error("Failed BIO_new for in BIO.");
            result = __FAILURE__;
        }
        else
        {
            tlsInstance->out_bio = BIO_new(BIO_s_mem());
            if (tlsInstance->out_bio == NULL)
            {
                (void)BIO_free(tlsInstance->in_bio);
                SSL_CTX_free(tlsInstance->ssl_context);
                tlsInstance->ssl_context = NULL;
                log_ERR_get_error("Failed BIO_new for out BIO.");
                result = __FAILURE__;
            }
            else
            {
                if ((BIO_set_mem_eof_return(tlsInstance->in_bio, -1) <= 0) ||
                    (BIO_set_mem_eof_return(tlsInstance->out_bio, -1) <= 0))
                {
                    (void)BIO_free(tlsInstance->in_bio);
                    (void)BIO_free(tlsInstance->out_bio);
                    SSL_CTX_free(tlsInstance->ssl_context);
                    tlsInstance->ssl_context = NULL;
                    LogError("Failed BIO_set_mem_eof_return.");
                    result = __FAILURE__;
                }
                else
                {
                    SSL_CTX_set_verify(tlsInstance->ssl_context, SSL_VERIFY_PEER, NULL);

                    // Specifies that the default locations for which CA certificates are loaded should be used.
                    if (SSL_CTX_set_default_verify_paths(tlsInstance->ssl_context) != 1)
                    {
                        // This is only a warning to the user. They can still specify the certificate via SetOption.
                        LogInfo("WARNING: Unable to specify the default location for CA certificates on this platform.");
                    }

                    tlsInstance->ssl = SSL_new(tlsInstance->ssl_context);
                    if (tlsInstance->ssl == NULL)
                    {
                        (void)BIO_free(tlsInstance->in_bio);
                        (void)BIO_free(tlsInstance->out_bio);
                        SSL_CTX_free(tlsInstance->ssl_context);
                        tlsInstance->ssl_context = NULL;
                        log_ERR_get_error("Failed creating OpenSSL instance.");
                        result = __FAILURE__;
                    }
                    else
                    {
                        SSL_set_bio(tlsInstance->ssl, tlsInstance->in_bio, tlsInstance->out_bio);
                        SSL_set_connect_state(tlsInstance->ssl);
                        result = 0;
                    }
                }
            }
        }
    }
    return result;
}

int tlsio_openssl_init(void)
{
    (void)SSL_library_init();

    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    if (openssl_static_locks_install() != 0)
    {
        LogError("Failed to install static locks in OpenSSL!");
        return __FAILURE__;
    }

    openssl_dynamic_locks_install();
    return 0;
}

void tlsio_openssl_deinit(void)
{
    openssl_dynamic_locks_uninstall();
    openssl_static_locks_uninstall();
#if (OPENSSL_VERSION_NUMBER >= 0x00907000L) && (OPENSSL_VERSION_NUMBER < 0x20000000L)
    FIPS_mode_set(0);
#endif
    CRYPTO_set_locking_callback(NULL);
    CRYPTO_set_id_callback(NULL);
    ERR_free_strings();
    EVP_cleanup();
#ifndef OPENSSL_NO_DEPRECATED
    ERR_remove_state(0);
#if (OPENSSL_VERSION_NUMBER >= 0x10002000L) && (OPENSSL_VERSION_NUMBER < 0x10100000L)
    SSL_COMP_free_compression_methods();
#endif
#endif
    CRYPTO_cleanup_all_ex_data();
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
        result = malloc(sizeof(TLS_IO_INSTANCE));
        if (result == NULL)
        {
            LogError("Failed allocating TLSIO instance.");
        }
        else
        {
            const IO_INTERFACE_DESCRIPTION* underlying_io_interface;
            void* io_interface_parameters;

            if (tls_io_config->underlying_io_interface != NULL)
            {
                underlying_io_interface = tls_io_config->underlying_io_interface;
                io_interface_parameters = tls_io_config->underlying_io_parameters;
            }
            else
            {
                SOCKETIO_CONFIG socketio_config;

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
                result->certificate = NULL;
                result->in_bio = NULL;
                result->out_bio = NULL;
                result->on_bytes_received = NULL;
                result->on_bytes_received_context = NULL;
                result->on_io_open_complete = NULL;
                result->on_io_open_complete_context = NULL;
                result->on_io_close_complete = NULL;
                result->on_io_close_complete_context = NULL;
                result->on_io_error = NULL;
                result->on_io_error_context = NULL;
                result->ssl = NULL;
                result->ssl_context = NULL;
                result->tls_validation_callback = NULL;
                result->tls_validation_callback_data = NULL;
                result->x509certificate = NULL;
                result->x509privatekey = NULL;
                result->tls_version = 0;

                result->underlying_io = xio_create(underlying_io_interface, io_interface_parameters);
                if (result->underlying_io == NULL)
                {
                    free(result);
                    result = NULL;
                    LogError("Failed xio_create.");
                }
                else
                {
                    result->tlsio_state = TLSIO_STATE_NOT_OPEN;
                }
            }
        }
    }

    return result;
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
        free((void*)tls_io_instance->x509certificate);
        free((void*)tls_io_instance->x509privatekey);
        close_openssl_instance(tls_io_instance);
        if (tls_io_instance->underlying_io != NULL)
        {
            xio_destroy(tls_io_instance->underlying_io);
            tls_io_instance->underlying_io = NULL;
        }
        free(tls_io);
    }
}

int tlsio_openssl_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;

    if (tls_io == NULL)
    {
        result = __FAILURE__;
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN.");
            result = __FAILURE__;
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
                tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
                result = __FAILURE__;
            }
            else if (xio_open(tls_io_instance->underlying_io, on_underlying_io_open_complete, tls_io_instance, on_underlying_io_bytes_received, tls_io_instance, on_underlying_io_error, tls_io_instance) != 0)
            {
                tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
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

int tlsio_openssl_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
        {
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN or TLSIO_STATE_CLOSING.");
            result = __FAILURE__;
        }
        else
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
            tls_io_instance->on_io_close_complete = on_io_close_complete;
            tls_io_instance->on_io_close_complete_context = callback_context;

            if (xio_close(tls_io_instance->underlying_io, on_underlying_io_close_complete, tls_io_instance) != 0)
            {
                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                LogError("Error in xio_close.");
                result = __FAILURE__;
            }
            else
            {
                close_openssl_instance(tls_io_instance);
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
        LogError("NULL tls_io.");
        result = __FAILURE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPEN.");
            result = __FAILURE__;
        }
        else
        {
            if (tls_io_instance->ssl == NULL)
            {
                LogError("SSL channel closed in tlsio_openssl_send.");
                result = __FAILURE__;
                return result;
            }

            int res = SSL_write(tls_io_instance->ssl, buffer, size);
            if (res != (int)size)
            {
                log_ERR_get_error("SSL_write error.");
                result = __FAILURE__;
            }
            else
            {
                if (write_outgoing_bytes(tls_io_instance, on_send_complete, callback_context) != 0)
                {
                    LogError("Error in write_outgoing_bytes.");
                    result = __FAILURE__;
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

void tlsio_openssl_dowork(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io == NULL)
    {
        LogError("NULL tls_io.");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state != TLSIO_STATE_CLOSING) &&
            (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN) &&
            (tls_io_instance->tlsio_state != TLSIO_STATE_ERROR))
        {
            /* this is needed in order to pump out bytes produces by OpenSSL for things like renegotiation */
            write_outgoing_bytes(tls_io_instance, NULL, NULL);
        }

        /* Same behavior as schannel */
        xio_dowork(tls_io_instance->underlying_io);
    }
}

int tlsio_openssl_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    int result;

    if (tls_io == NULL || optionName == NULL)
    {
        result = __FAILURE__;
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
                result = __FAILURE__;
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
                result = __FAILURE__;
            }
            else
            {   
                /*let's make a copy of this option*/
                if (mallocAndStrcpy_s((char**)&tls_io_instance->x509certificate, value) != 0)
                {
                    LogError("unable to mallocAndStrcpy_s");
                    result = __FAILURE__;
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
                result = __FAILURE__;
            }
            else
            {
                /*let's make a copy of this option*/
                if (mallocAndStrcpy_s((char**)&tls_io_instance->x509privatekey, value) != 0)
                {
                    LogError("unable to mallocAndStrcpy_s");
                    result = __FAILURE__;
                }
                else
                {
                    result = 0;
                }
            }
        }
        else if (strcmp("tls_validation_callback", optionName) == 0)
        {
            #pragma warning(push)
            #pragma warning(disable:4055)
            tls_io_instance->tls_validation_callback = (TLS_CERTIFICATE_VALIDATION_CALLBACK)value;
            #pragma warning(pop)

            if (tls_io_instance->ssl_context != NULL)
            {
                SSL_CTX_set_cert_verify_callback(tls_io_instance->ssl_context, tls_io_instance->tls_validation_callback, tls_io_instance->tls_validation_callback_data);
            }

            result = 0;
        }
        else if (strcmp("tls_validation_callback_data", optionName) == 0)
        {
            tls_io_instance->tls_validation_callback_data = (void*)value;

            if (tls_io_instance->ssl_context != NULL)
            {
                SSL_CTX_set_cert_verify_callback(tls_io_instance->ssl_context, tls_io_instance->tls_validation_callback, tls_io_instance->tls_validation_callback_data);
            }

            result = 0;
        }
        else if (strcmp("tls_version", optionName) == 0)
        {
            tls_io_instance->tls_version = (int)(intptr_t)value;
            result = 0;
        }
        else
        {
            if (tls_io_instance->underlying_io == NULL)
            {
                result = __FAILURE__;
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
