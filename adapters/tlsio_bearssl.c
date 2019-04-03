// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

#include "bearssl.h"

#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/tlsio_bearssl.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/vector.h"
#include "azure_c_shared_utility/buffer_.h"

static const char *const OPTION_UNDERLYING_IO_OPTIONS = "underlying_io_options";

typedef enum TLSIO_STATE_ENUM_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING_UNDERLYING_IO,
    TLSIO_STATE_IN_HANDSHAKE,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE_ENUM;

typedef struct PENDING_TLS_IO_TAG
{
    unsigned char* bytes;
    size_t size;
    ON_SEND_COMPLETE on_send_complete;
    void* callback_context;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} PENDING_TLS_IO;

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
    SINGLYLINKEDLIST_HANDLE pending_toencrypt_list;
    SINGLYLINKEDLIST_HANDLE pending_todecrypt_list;

    br_ssl_client_context sc;
    br_x509_minimal_context xc;
    br_sslio_context ioc;
    br_x509_trust_anchor *tas;
    size_t ta_count;
    char *trusted_certificates;
	char *x509_certificate;
    char *x509_private_key;
    unsigned char iobuf[BR_SSL_BUFSIZE_BIDI];
    char *hostname;
} TLS_IO_INSTANCE;

typedef struct {
	char *name;
	unsigned char *data;
	size_t data_len;
} pem_object;

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

static int add_pending_operation(SINGLYLINKEDLIST_HANDLE list, const unsigned char *buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void *context)
{
    int result;
    PENDING_TLS_IO* pending_tls_io = (PENDING_TLS_IO*)malloc(sizeof(PENDING_TLS_IO));
    
    if (pending_tls_io == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        pending_tls_io->bytes = (unsigned char*)malloc(size);

        if (pending_tls_io->bytes == NULL)
        {
            LogError("Allocation Failure: Unable to allocate pending list.");
            free(pending_tls_io);
            result = MU_FAILURE;
        }
        else
        {
            pending_tls_io->size = size;
            pending_tls_io->on_send_complete = on_send_complete;
            pending_tls_io->callback_context = context;
            (void)memcpy(pending_tls_io->bytes, buffer, size);

            if (singlylinkedlist_add(list, pending_tls_io) == NULL)
            {
                LogError("Failure: Unable to add tls io to pending list.");
                free(pending_tls_io->bytes);
                free(pending_tls_io);
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
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
        }
    }
}

static void on_underlying_io_bytes_received(void *context, const unsigned char *buffer, size_t size)
{
    if (context != NULL)
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)context;

        // Dump this on the queue and let dowork deal with it
        add_pending_operation(tls_io_instance->pending_todecrypt_list, buffer, size, NULL, NULL);
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
        LogError("NULL value passed in context");
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

static void vblob_append(void *cc, const void *data, size_t len)
{
    int result = BUFFER_append_build((BUFFER_HANDLE)cc, (const unsigned char *)data, len);

    if (result != 0)
    {
        // Unfortunately the design does not allow for this error to be passed back
        LogError("Failed to reallocate pem decode buffer");
    }
}

VECTOR_HANDLE decode_pem(const void *src, size_t len)
{
    VECTOR_HANDLE pem_list;
	br_pem_decoder_context pc;
	pem_object po;
    //pem_object *pos;
	const unsigned char *buf;
    BUFFER_HANDLE bv;
	int inobj;
	int extra_nl;
    size_t i;

    pem_list = VECTOR_create(sizeof(pem_object));

	br_pem_decoder_init(&pc);
	buf = src;
	inobj = 0;
	po.name = NULL;
	po.data = NULL;
	po.data_len = 0;
	extra_nl = 1;

	while (len > 0) {
		size_t tlen;

		tlen = br_pem_decoder_push(&pc, buf, len);
		buf += tlen;
		len -= tlen;
		switch (br_pem_decoder_event(&pc)) {

		case BR_PEM_BEGIN_OBJ:
			inobj = 1;

			if (0 != mallocAndStrcpy_s(&po.name, br_pem_decoder_name(&pc)))
            {
                LogError("Unable to allocate memory for certificate name");
                break;
            }

            bv = BUFFER_new();
			br_pem_decoder_setdest(&pc, vblob_append, bv);
			break;

		case BR_PEM_END_OBJ:
			if (inobj) 
            {
                po.data = BUFFER_u_char(bv);
				po.data_len = BUFFER_length(bv);
                free(bv);
                VECTOR_push_back(pem_list, &po, 1);
				po.name = NULL;
				po.data = NULL;
				po.data_len = 0;
				inobj = 0;
			}
			break;

		case BR_PEM_ERROR:
			LogError("ERROR: invalid PEM encoding");
            inobj = 1;
            break;
		}

		/*
		 * We add an extra newline at the end, in order to
		 * support PEM files that lack the newline on their last
		 * line (this is somewhat invalid, but PEM format is not
		 * standardised and such files do exist in the wild, so
		 * we'd better accept them).
		 */
		if (len == 0 && extra_nl) {
			extra_nl = 0;
			buf = (const unsigned char *)"\n";
			len = 1;
		}
	}

	if (inobj) 
    {
		fprintf(stderr, "ERROR: unfinished PEM object\n");


        for (i = 0; i < VECTOR_size(pem_list); i++)
        {
            free(((pem_object *)VECTOR_element(pem_list, i))->name);
            free(((pem_object *)VECTOR_element(pem_list, i))->data);
        }

        VECTOR_clear(pem_list);
		free(po.name);
        BUFFER_delete(bv);
        pem_list = NULL;
	}
    
	return pem_list;
}

static VECTOR_HANDLE read_certificates_string(const char *buf, size_t len)
{
	VECTOR_HANDLE cert_list; //(br_x509_certificate) cert_list = VEC_INIT;
    VECTOR_HANDLE pem_list;
	size_t u;
    size_t num_pos;
	br_x509_certificate dummy;
    int result;
    static const char CERTIFICATE[] = "CERTIFICATE";
    static const char X509_CERTIFICATE[] = "X509 CERTIFICATE";
    static const int CERTIFICATE_LEN = sizeof(CERTIFICATE) - 1;
    static const int X509_CERTIFICATE_LEN = sizeof(X509_CERTIFICATE) - 1;

    cert_list = VECTOR_create(sizeof(br_x509_certificate));

    if (cert_list == NULL)
    {
        LogError("Unable to allocate memory to decode pem strings");
        VECTOR_destroy(cert_list);
        cert_list = NULL;
    }
    else
    {
        pem_list = decode_pem(buf, len);
        
        if (pem_list == NULL) 
        {
            LogError("Failed to decode pem");
            VECTOR_destroy(cert_list);
            cert_list = NULL;
        }
        else
        {
            for (u = 0; u < VECTOR_size(pem_list); u++) 
            {
                if (0 == memcmp(CERTIFICATE, ((pem_object *)VECTOR_element(pem_list, u))->name, CERTIFICATE_LEN) ||
                    0 == memcmp(X509_CERTIFICATE, ((pem_object *)VECTOR_element(pem_list, u))->name, X509_CERTIFICATE_LEN))
                {
                    br_x509_certificate xc;

                    xc.data = ((pem_object *)VECTOR_element(pem_list, u))->data;
                    xc.data_len = ((pem_object *)VECTOR_element(pem_list, u))->data_len;
                    ((pem_object *)VECTOR_element(pem_list, u))->data = NULL;
                    ((pem_object *)VECTOR_element(pem_list, u))->data_len = 0;
                    free(((pem_object *)VECTOR_element(pem_list, u))->name);
                    ((pem_object *)VECTOR_element(pem_list, u))->name = NULL;

                    result = VECTOR_push_back(cert_list, &xc, 1);

                    if (result != 0)
                    {
                        LogError("Failed to add certificate to vector");
                        break;
                    }
                }
                else
                {
                    LogError("Unable to determine the certificate type");
                }
            }

            // If we enter this loop something failed
            for (; u < VECTOR_size(pem_list); u++)
            {
                free(((pem_object *)VECTOR_element(pem_list, u))->name);
                free(((pem_object *)VECTOR_element(pem_list, u))->data);
            }

            VECTOR_destroy(pem_list);

            if (0 == VECTOR_size(cert_list))
            {
                fprintf(stderr, "No certificate in string");
                result = MU_FAILURE;
            }

            if (result != 0)
            {
                for (u = 0; u < VECTOR_size(cert_list); u++)
                {
                    free(((br_x509_certificate*)VECTOR_element(cert_list, u))->data);
                }

                VECTOR_destroy(cert_list);
                cert_list = NULL;
            }
        }
    }

	return cert_list;
}

static void dn_append(void *ctx, const void *buf, size_t len)
{
	if (0 != (BUFFER_append_build((BUFFER_HANDLE)ctx, buf, len)))
    {
        // Design does not allow this error to be reported to caller
        LogError("Failed to append data");
    }
}

void free_ta_contents(br_x509_trust_anchor *ta)
{
	free(ta->dn.data);
	switch (ta->pkey.key_type) 
    {
	case BR_KEYTYPE_RSA:
		free(ta->pkey.key.rsa.n);
		free(ta->pkey.key.rsa.e);
		break;
	case BR_KEYTYPE_EC:
		free(ta->pkey.key.ec.q);
		break;
	}
}

void free_certificates(br_x509_certificate *certs, size_t num)
{
	size_t u;

	for (u = 0; u < num; u ++) {
		free(certs[u].data);
	}
}

static int certificate_to_trust_anchor(br_x509_certificate *xc, br_x509_trust_anchor *ta)
{
	br_x509_decoder_context dc;
	br_x509_pkey *pk;
    BUFFER_HANDLE vdn;
    //br_x509_trust_anchor *ta;
    int result;

    vdn = BUFFER_new();

    if (NULL == vdn)
    {
        LogError("Failed to allocate memory to decode x509 certificate");
        result = MU_FAILURE;
    }
    else
    {
        memset(ta, 0, sizeof(br_x509_trust_anchor));
        br_x509_decoder_init(&dc, dn_append, vdn);
        br_x509_decoder_push(&dc, xc->data, xc->data_len);
        pk = br_x509_decoder_get_pkey(&dc);

        if (pk == NULL) 
        {
            LogError("ERROR: CA decoding failed with error %d", br_x509_decoder_last_error(&dc));
            BUFFER_delete(vdn);
        }
        else
        {
            ta->dn.data = BUFFER_u_char(vdn);
            ta->dn.len = BUFFER_length(vdn);
            free(vdn);
            ta->flags = 0;

            if (br_x509_decoder_isCA(&dc)) 
            {
                ta->flags |= BR_X509_TA_CA;
            }

            switch (pk->key_type) 
            {
            case BR_KEYTYPE_RSA:
                ta->pkey.key_type = BR_KEYTYPE_RSA;
        		ta->pkey.key.rsa.nlen = pk->key.rsa.nlen;
                ta->pkey.key.rsa.elen = pk->key.rsa.elen;

                if (NULL == (ta->pkey.key.rsa.n = (unsigned char *)malloc(ta->pkey.key.rsa.nlen)) ||
                    NULL == ( ta->pkey.key.rsa.e = (unsigned char *)malloc(ta->pkey.key.rsa.elen)))
                {
                    LogError("Unable to allocate memory");
                    free_ta_contents(ta);
                    result = MU_FAILURE;
                }
                else
                {
                    memcpy(ta->pkey.key.rsa.n, pk->key.rsa.n, ta->pkey.key.rsa.nlen);
                    memcpy(ta->pkey.key.rsa.e, pk->key.rsa.e, ta->pkey.key.rsa.elen);
                    result = 0;
                }
                break;
            case BR_KEYTYPE_EC:
                ta->pkey.key_type = BR_KEYTYPE_EC;
                ta->pkey.key.ec.curve = pk->key.ec.curve;
                ta->pkey.key.ec.qlen = pk->key.ec.qlen;

                if (NULL == (ta->pkey.key.ec.q = (unsigned char *)malloc(ta->pkey.key.ec.qlen)))
                {
                    LogError("Unable to allocate memory");
                    free_ta_contents(ta);
                    result = MU_FAILURE;
                }
                else
                {
                    memcpy(ta->pkey.key.ec.q, pk->key.ec.q, ta->pkey.key.ec.qlen);
                    result = 0;
                }
                break;
            default:
                fprintf(stderr, "ERROR: unsupported public key type in CA\n");
                free_ta_contents(ta);
                result = MU_FAILURE;
            }
        }
    }

	return result;
}

static size_t get_trusted_anchors(const char *certificates, size_t len, br_x509_trust_anchor *anchOut[])
{
    // Converts a PEM certificate in a string into the format required by BearSSL
    VECTOR_HANDLE xcs;
    br_x509_trust_anchor work;
	br_x509_trust_anchor *anchArray;
    size_t u;
    size_t v;
    size_t num;
    int result;

    xcs = read_certificates_string(certificates, len);
    num = VECTOR_size(xcs);

    if (VECTOR_size(xcs) == 0)
    {
        LogError("No certificates found in string");
    }
    else
    {
        anchArray = (br_x509_trust_anchor *)malloc(sizeof(br_x509_trust_anchor) * num);

        if (anchArray == NULL)
        {
            LogError("Memory allocation for trust anchors failed");
        }
        else
        {
            *anchOut = anchArray;

            for (u = 0; u < num; u++)
            {
                result = certificate_to_trust_anchor((br_x509_certificate *)VECTOR_element(xcs, u), &work);

                if (result != 0)
                {
                    for (v = 0; v < u; v++)
                    {
                        free_ta_contents((anchOut[u]));
                    }

                    free(anchArray);
                    *anchOut = NULL;
                    num = 0;
                    break;
                }

                anchArray[u] = work;
            }
        }
    }

    free_certificates((br_x509_certificate *)VECTOR_front(xcs), VECTOR_size(xcs));
    VECTOR_destroy(xcs);
    
	return num;
}

CONCRETE_IO_HANDLE tlsio_bearssl_create(void *io_create_parameters)
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
        /* Codes_SRS_TLSIO_MBED_OS5_TLS_99_006: [ The tlsio_bearssl_create shall return NULL if allocating memory for TLS_IO_INSTANCE failed. ]*/
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
                mallocAndStrcpy_s(&result->hostname, tls_io_config->hostname);
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
                        result->pending_toencrypt_list = singlylinkedlist_create();
                        result->pending_todecrypt_list = singlylinkedlist_create();
                        result->trusted_certificates = NULL;
                        result->x509_certificate = NULL;
                        result->x509_private_key = NULL;
                        result->tlsio_state = TLSIO_STATE_NOT_OPEN;
                    }
                }
            }
        }
    }

    return result;
}

void tlsio_bearssl_destroy(CONCRETE_IO_HANDLE tls_io)
{
    size_t i;
	LIST_ITEM_HANDLE first_pending_io;

    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;

        //xio_close(tls_io_instance->socket_io, NULL, NULL);

        if (tls_io_instance->socket_io_read_bytes != NULL)
        {
            free(tls_io_instance->socket_io_read_bytes);
            tls_io_instance->socket_io_read_bytes = NULL;
        }
        xio_destroy(tls_io_instance->socket_io);

        first_pending_io = singlylinkedlist_get_head_item(tls_io_instance->pending_toencrypt_list);
        while (first_pending_io != NULL)
        {
            PENDING_TLS_IO *pending_tls_io = (PENDING_TLS_IO *)singlylinkedlist_item_get_value(first_pending_io);
            if (pending_tls_io != NULL)
            {
                free(pending_tls_io->bytes);
                free(pending_tls_io);
            }

            (void)singlylinkedlist_remove(tls_io_instance->pending_toencrypt_list, first_pending_io);
            first_pending_io = singlylinkedlist_get_head_item(tls_io_instance->pending_toencrypt_list);
        }

        singlylinkedlist_destroy(tls_io_instance->pending_toencrypt_list);
        tls_io_instance->pending_toencrypt_list = NULL;

        first_pending_io = singlylinkedlist_get_head_item(tls_io_instance->pending_todecrypt_list);
        while (first_pending_io != NULL)
        {
            PENDING_TLS_IO *pending_tls_io = (PENDING_TLS_IO *)singlylinkedlist_item_get_value(first_pending_io);
            if (pending_tls_io != NULL)
            {
                free(pending_tls_io->bytes);
                free(pending_tls_io);
            }

            (void)singlylinkedlist_remove(tls_io_instance->pending_todecrypt_list, first_pending_io);
            first_pending_io = singlylinkedlist_get_head_item(tls_io_instance->pending_todecrypt_list);
        }

        singlylinkedlist_destroy(tls_io_instance->pending_todecrypt_list);
        tls_io_instance->pending_todecrypt_list = NULL;

        if (tls_io_instance->hostname != NULL)
        {
            free(tls_io_instance->hostname);
            tls_io_instance->hostname = NULL;
        }
        if (tls_io_instance->trusted_certificates != NULL)
        {
            free(tls_io_instance->trusted_certificates);
            tls_io_instance->trusted_certificates = NULL;

            if (tls_io_instance->tas != NULL)
            {
                // Free the memory if it has been previously allocated
                for (i = 0; i < tls_io_instance->ta_count; i++)
                {
		            free_ta_contents(&(tls_io_instance->tas[i]));
                }

                free(tls_io_instance->tas);
                tls_io_instance->tas = NULL;
                tls_io_instance->ta_count = 0;
            }

        }
        if (tls_io_instance->x509_certificate != NULL)
        {
            free(tls_io_instance->x509_certificate);
            tls_io_instance->x509_certificate = NULL;
        }
        if (tls_io_instance->x509_private_key != NULL)
        {
            free(tls_io_instance->x509_private_key);
            tls_io_instance->x509_private_key = NULL;
        }
        free(tls_io);
    }
}

int tlsio_bearssl_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void *on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void *on_bytes_received_context, ON_IO_ERROR on_io_error, void *on_io_error_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        LogError("NULL tls_io");
        result = MU_FAILURE;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
            LogError("IO should not be open: %d\n", tls_io_instance->tlsio_state);
            result = MU_FAILURE;
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

            if (tls_io_instance->ta_count == 0)
            {
                LogError("Trusted certificates are required but missing");
                result = MU_FAILURE;
            }
            else
            {
                br_ssl_client_init_full(&tls_io_instance->sc, &tls_io_instance->xc, tls_io_instance->tas, tls_io_instance->ta_count);
                br_ssl_engine_set_buffer(&tls_io_instance->sc.eng, tls_io_instance->iobuf, sizeof(tls_io_instance->iobuf), 1);
            	br_ssl_client_reset(&tls_io_instance->sc, tls_io_instance->hostname, 0);

                if (xio_open(tls_io_instance->socket_io, on_underlying_io_open_complete, tls_io_instance, on_underlying_io_bytes_received, tls_io_instance, on_underlying_io_error, tls_io_instance) != 0)
                {

                    LogError("Underlying IO open failed");
                    tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
                    result = MU_FAILURE;
                }
            }
        }
    }

    return result;
}

int tlsio_bearssl_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void *callback_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
        {
            result = MU_FAILURE;
        }
        else
        {
			tls_io_instance->on_io_close_complete = on_io_close_complete;
			tls_io_instance->on_io_close_complete_context = callback_context;
			tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
			br_ssl_engine_close(&tls_io_instance->sc.eng);
            result = 0;
        }
    }
    return result;
}

int tlsio_bearssl_send(CONCRETE_IO_HANDLE tls_io, const void *buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void *callback_context)
{
    int result;

    if ((tls_io == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        /* Invalid arguments */
        LogError("Invalid argument: send given invalid parameter");
        result = MU_FAILURE;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;
        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            LogError("TLS is not open");
            result = MU_FAILURE;
        }
        else
        {
            if (add_pending_operation(tls_io_instance->pending_toencrypt_list, buffer, size, on_send_complete, callback_context) != 0)
            {
                LogError("Failure: add_pending_io failed.");
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

void tlsio_bearssl_dowork(CONCRETE_IO_HANDLE tls_io)
{
    unsigned char *buffer;
    size_t bufferLen;
    unsigned long bearResult;

    if (tls_io != NULL)
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;

        if (tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN)
        {
            // Not open do nothing
        }
        else 
        {
            if (tls_io_instance->tlsio_state != TLSIO_STATE_OPENING_UNDERLYING_IO)
            {
                bearResult = br_ssl_engine_current_state(&tls_io_instance->sc.eng);

                if (bearResult & BR_SSL_SENDREC)
                {
                    // The engine has data to send
                    buffer = br_ssl_engine_sendrec_buf(&tls_io_instance->sc.eng, &bufferLen);

                    if (xio_send(tls_io_instance->socket_io, buffer, bufferLen, NULL, NULL) != 0)
                    {
                        LogError("Error in xio_send.");
                        indicate_error(tls_io_instance);
                    }
                    else
                    {
                        br_ssl_engine_sendrec_ack(&tls_io_instance->sc.eng, bufferLen);
                    }
                }

                bearResult = br_ssl_engine_current_state(&tls_io_instance->sc.eng);

                if (bearResult & BR_SSL_RECVREC)
                {
                    // The engine can accept data from the peer if there is any
                    LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(tls_io_instance->pending_todecrypt_list);
                    
                    if (first_pending_io != NULL)
                    {
                        PENDING_TLS_IO *pending_tls_io = (PENDING_TLS_IO*)singlylinkedlist_item_get_value(first_pending_io);

                        if (pending_tls_io != NULL)
                        {
                            buffer = br_ssl_engine_recvrec_buf(&tls_io_instance->sc.eng, &bufferLen);

                            if (bufferLen == 0)
                            {
                                LogError("Zero length buffer returned by BearSSL");
                                indicate_error(tls_io_instance);
                            }
                            else
                            {
                                if (pending_tls_io->size < bufferLen)
                                {
                                    bufferLen = pending_tls_io->size;
                                }
                                memcpy(buffer, pending_tls_io->bytes, bufferLen);
                                br_ssl_engine_recvrec_ack(&tls_io_instance->sc.eng, bufferLen);

                                if (bufferLen < pending_tls_io->size)
                                {
                                    pending_tls_io->size -= bufferLen;
                                    memcpy(pending_tls_io->bytes, pending_tls_io->bytes + bufferLen, pending_tls_io->size);
                                }
                                else
                                {
                                    free(pending_tls_io->bytes);
                                    free(pending_tls_io);
                                    (void)singlylinkedlist_remove(tls_io_instance->pending_todecrypt_list, first_pending_io);
                                }
                            }
                        }
                    }
                }

                bearResult = br_ssl_engine_current_state(&tls_io_instance->sc.eng);

                if (bearResult & BR_SSL_SENDAPP)
                {
                    if (tls_io_instance->tlsio_state == TLSIO_STATE_IN_HANDSHAKE)
                    {
                        tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
                        indicate_open_complete(tls_io_instance, IO_OPEN_OK);
                    }

                    // Engine is ready for application data - send it if we have any
                    LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(tls_io_instance->pending_toencrypt_list);
                    
                    if (first_pending_io != NULL)
                    {
                        PENDING_TLS_IO *pending_tls_io = (PENDING_TLS_IO*)singlylinkedlist_item_get_value(first_pending_io);

                        if (pending_tls_io != NULL)
                        {
                            buffer = br_ssl_engine_sendapp_buf(&tls_io_instance->sc.eng, &bufferLen);

                            if (pending_tls_io->size < bufferLen)
                            {
                                bufferLen = pending_tls_io->size;
                            }
                            memcpy(buffer, pending_tls_io->bytes, bufferLen);
                            br_ssl_engine_sendapp_ack(&tls_io_instance->sc.eng, bufferLen);

                            if (bufferLen < pending_tls_io->size)
                            {
                                pending_tls_io->size -= bufferLen;
                                memcpy(pending_tls_io->bytes, pending_tls_io->bytes + bufferLen, pending_tls_io->size);
                            }
                            else
                            {
                                pending_tls_io->on_send_complete(pending_tls_io->callback_context, IO_SEND_OK);
                                free(pending_tls_io->bytes);
                                free(pending_tls_io);
                                (void)singlylinkedlist_remove(tls_io_instance->pending_toencrypt_list, first_pending_io);
                                br_ssl_engine_flush(&tls_io_instance->sc.eng, 0);
                            }
                        }
                    }
                    
                }

                bearResult = br_ssl_engine_current_state(&tls_io_instance->sc.eng);

                if (bearResult & BR_SSL_RECVAPP)
                {
                    // Application data is waiting to be forwarded
                    buffer = br_ssl_engine_recvapp_buf(&tls_io_instance->sc.eng, &bufferLen);
                    tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, bufferLen);
                    br_ssl_engine_recvapp_ack(&tls_io_instance->sc.eng, bufferLen);
                }

                bearResult = br_ssl_engine_current_state(&tls_io_instance->sc.eng);

                if (bearResult & BR_SSL_CLOSED)
                {
                    if (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING)
                    {
                        xio_close(tls_io_instance->socket_io, on_underlying_io_close_complete_during_close, tls_io_instance);
                    }
                    else
                    {
                        indicate_error(tls_io_instance);
                    }
                    
                }
            }

            xio_dowork(tls_io_instance->socket_io);
        }
    }
}

/*this function will clone an option given by name and value*/
static void *tlsio_bearssl_CloneOption(const char *name, const void *value)
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
                // return as is
            }
        }
/*        
        else if (strcmp(name, SU_OPTION_X509_CERT) == 0)
        {
            if (mallocAndStrcpy_s((char**)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s x509certificate value");
                result = NULL;
            }
            else
            {
                // return as is
            }
        }
        else if (strcmp(name, SU_OPTION_X509_PRIVATE_KEY) == 0)
        {
            if (mallocAndStrcpy_s((char**)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s x509privatekey value");
                result = NULL;
            }
            else
            {
                // return as is
            }
        }
        else if (strcmp(name, OPTION_X509_ECC_CERT) == 0)
        {
            if (mallocAndStrcpy_s((char**)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s x509EccCertificate value");
                result = NULL;
            }
            else
            {
                // return as is
            }
        }
        else if (strcmp(name, OPTION_X509_ECC_KEY) == 0)
        {
            if (mallocAndStrcpy_s((char**)&result, value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s x509EccKey value");
                result = NULL;
            }
            else
            {
                // return as is
            }
        }
*/        
        else
        {
            LogError("not handled option : %s", name);
            result = NULL;
        }
    }
    return result;
}

// This function destroys an option previously created
static void tlsio_bearssl_DestroyOption(const char *name, const void *value)
{
    /*since all options for this layer are actually string copies., disposing of one is just calling free*/
    if (name == NULL || value == NULL)
    {
        LogError("invalid parameter detected: const char* name=%p, const void* value=%p", name, value);
    }
    else
    {
        if (
            (strcmp(name, OPTION_TRUSTED_CERT) == 0) ||
            (strcmp(name, SU_OPTION_X509_CERT) == 0) ||
            (strcmp(name, SU_OPTION_X509_PRIVATE_KEY) == 0) ||
            (strcmp(name, OPTION_X509_ECC_CERT) == 0) ||
            (strcmp(name, OPTION_X509_ECC_KEY) == 0)
            )
        {
            free((void*)value);
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

int tlsio_bearssl_setoption(CONCRETE_IO_HANDLE tls_io, const char *optionName, const void *value)
{
    int result = 0;
    size_t i;

    if (tls_io == NULL || optionName == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        TLS_IO_INSTANCE *tls_io_instance = (TLS_IO_INSTANCE *)tls_io;

        if (strcmp(OPTION_TRUSTED_CERT, optionName) == 0)
        {
            if (tls_io_instance->tas != NULL)
            {
                // Free the memory if it has been previously allocated
                for (i = 0; i < tls_io_instance->ta_count; i++)
                {
		            free_ta_contents(&(tls_io_instance->tas[i]));
                }

                free(tls_io_instance->tas);
                tls_io_instance->tas = NULL;
                tls_io_instance->ta_count = 0;
                free(tls_io_instance->trusted_certificates);
                tls_io_instance->trusted_certificates = NULL;
            }

            if (value != NULL)
            {
                mallocAndStrcpy_s(&tls_io_instance->trusted_certificates, (const char *)value);

                if (tls_io_instance->trusted_certificates == NULL)
                {
                    LogError("Failed to allocate memory for certificate string");
                    result = MU_FAILURE;
                }
                else
                {
                    tls_io_instance->ta_count = get_trusted_anchors((const char*)value, strlen((const char *)value) + 1, &tls_io_instance->tas);

                    if (tls_io_instance->ta_count == 0)
                    {
                        LogError("Failed to extract certificate from option value");
                        result = MU_FAILURE;
                    }
                    else
                    {
                        result = 0;
                    }
                }
            }
        }
        else if (strcmp(optionName, OPTION_UNDERLYING_IO_OPTIONS) == 0)
        {
            if (OptionHandler_FeedOptions((OPTIONHANDLER_HANDLE)value, (void*)tls_io_instance->socket_io) != OPTIONHANDLER_OK)
            {
                LogError("failed feeding options to underlying I/O instance");
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }

/*        
        else if (strcmp(SU_OPTION_X509_CERT, optionName) == 0 || strcmp(OPTION_X509_ECC_CERT, optionName) == 0)
        {
            if (tls_io_instance->x509_certificate != NULL)
            {
                // Free the memory if it has been previously allocated
                free(tls_io_instance->x509_certificate);
            }

            if (mallocAndStrcpy_s(&tls_io_instance->x509_certificate, (const char *)value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s on certificate");
                result = MU_FAILURE;
            }
            else if (mbedtls_x509_crt_parse(&tls_io_instance->owncert, (const unsigned char *)value, (int)(strlen(value) + 1)) != 0)
            {
                result = MU_FAILURE;
            }
            else if (tls_io_instance->pKey.pk_info != NULL && mbedtls_ssl_conf_own_cert(&tls_io_instance->config, &tls_io_instance->owncert, &tls_io_instance->pKey) != 0)
            {
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }
        else if (strcmp(SU_OPTION_X509_PRIVATE_KEY, optionName) == 0 || strcmp(OPTION_X509_ECC_KEY, optionName) == 0)
        {
            if (tls_io_instance->x509_private_key != NULL)
            {
                // Free the memory if it has been previously allocated
                free(tls_io_instance->x509_private_key);
            }

            if (mallocAndStrcpy_s(&tls_io_instance->x509_private_key, (const char *)value) != 0)
            {
                LogError("unable to mallocAndStrcpy_s on private key");
                result = MU_FAILURE;
            }
            else if (mbedtls_pk_parse_key(&tls_io_instance->pKey, (const unsigned char *)value, (int)(strlen(value) + 1), NULL, 0) != 0)
            {
                result = MU_FAILURE;
            }
            else if (tls_io_instance->owncert.version > 0 && mbedtls_ssl_conf_own_cert(&tls_io_instance->config, &tls_io_instance->owncert, &tls_io_instance->pKey))
            {
                result = MU_FAILURE;
            }
            else
            {
                result = 0;
            }
        }
*/        
        else
        {
            // tls_io_instance->socket_io is never NULL
            result = xio_setoption(tls_io_instance->socket_io, optionName, value);
        }
    }

    return result;
}

OPTIONHANDLER_HANDLE tlsio_bearssl_retrieveoptions(CONCRETE_IO_HANDLE handle)
{
    OPTIONHANDLER_HANDLE result = NULL;
    if (handle == NULL)
    {
        LogError("invalid parameter detected: CONCRETE_IO_HANDLE handle=%p", handle);
        result = NULL;
    }
    else
    {
        result = OptionHandler_Create(tlsio_bearssl_CloneOption, tlsio_bearssl_DestroyOption, tlsio_bearssl_setoption);
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
/*
            else if (&tls_io_instance->owncert != NULL && tls_io_instance->x509_certificate != NULL && 
                    OptionHandler_AddOption(result, SU_OPTION_X509_CERT, tls_io_instance->x509_certificate) != OPTIONHANDLER_OK)
            {
                LogError("unable to save x509certificate option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
            else if (
                (&tls_io_instance->pKey != NULL) && tls_io_instance->x509_private_key != NULL &&
                (OptionHandler_AddOption(result, SU_OPTION_X509_PRIVATE_KEY, tls_io_instance->x509_private_key) != OPTIONHANDLER_OK)
                )
            {
                LogError("unable to save x509privatekey option");
                OptionHandler_Destroy(result);
                result = NULL;
            }
*/
            else
            {
                /*all is fine, all interesting options have been saved*/
                /*return as is*/
            }
        }
    }
    return result;
}

static const IO_INTERFACE_DESCRIPTION tlsio_bearssl_interface_description =
    {
        tlsio_bearssl_retrieveoptions,
        tlsio_bearssl_create,
        tlsio_bearssl_destroy,
        tlsio_bearssl_open,
        tlsio_bearssl_close,
        tlsio_bearssl_send,
        tlsio_bearssl_dowork,
        tlsio_bearssl_setoption};

const IO_INTERFACE_DESCRIPTION *tlsio_bearssl_get_interface_description(void)
{
    return &tlsio_bearssl_interface_description;
}
