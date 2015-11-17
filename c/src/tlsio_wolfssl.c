// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "wolfssl/ssl.h"
#include "wolfssl/error-ssl.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tlsio_wolfssl.h"
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
	WOLFSSL* ssl;
	WOLFSSL_CTX* ssl_context;
	HANDSHAKE_STATE_ENUM handshake_state;
	unsigned char* socket_io_read_bytes;
	size_t socket_io_read_byte_count;
	ON_SEND_COMPLETE on_send_complete;
	void* on_send_complete_callback_context;
} TLS_IO_INSTANCE;

static const IO_INTERFACE_DESCRIPTION tlsio_wolfssl_interface_description =
{
	tlsio_wolfssl_create,
	tlsio_wolfssl_destroy,
	tlsio_wolfssl_open,
	tlsio_wolfssl_close,
	tlsio_wolfssl_send,
	tlsio_wolfssl_dowork
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

static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
	int result = 0;
	unsigned char buffer[64];

	int rcv_bytes = 1;
	while (rcv_bytes > 0)
	{
		rcv_bytes = wolfSSL_read(tls_io_instance->ssl, buffer, sizeof(buffer));
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

	unsigned char* new_socket_io_read_bytes = (unsigned char*)realloc(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_byte_count + size);
	if (new_socket_io_read_bytes == NULL)
	{
	}
	else
	{
		tls_io_instance->socket_io_read_bytes = new_socket_io_read_bytes;
		(void)memcpy(tls_io_instance->socket_io_read_bytes + tls_io_instance->socket_io_read_byte_count, buffer, size);
		tls_io_instance->socket_io_read_byte_count += size;
	}
}

static int on_io_recv(WOLFSSL *ssl, char *buf, int sz, void *context)
{
	int result;
	TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
	unsigned char* new_socket_io_read_bytes;

	while (tls_io_instance->socket_io_read_byte_count == 0)
	{
		io_dowork(tls_io_instance->socket_io);
		if (tls_io_instance->handshake_state == HANDSHAKE_STATE_DONE)
		{
			break;
		}
	}

	result = tls_io_instance->socket_io_read_byte_count;
	if (result > sz)
	{
		result = sz;
	}

	if (result > 0)
	{
		(void)memcpy(buf, tls_io_instance->socket_io_read_bytes, result);
		(void)memmove(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_bytes + result, tls_io_instance->socket_io_read_byte_count - result);
		tls_io_instance->socket_io_read_byte_count -= result;
		if (tls_io_instance->socket_io_read_byte_count > 0)
		{
			new_socket_io_read_bytes = (unsigned char*)realloc(tls_io_instance->socket_io_read_bytes, tls_io_instance->socket_io_read_byte_count);
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

	if ((result == 0) && (tls_io_instance->handshake_state == HANDSHAKE_STATE_DONE))
	{
		result = WOLFSSL_CBIO_ERR_WANT_READ;
	}

	return result;
}

static int on_io_send(WOLFSSL *ssl, char *buf, int sz, void *context)
{
	int result;
	TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

	if (io_send(tls_io_instance->socket_io, buf, sz, tls_io_instance->on_send_complete, tls_io_instance->on_send_complete_callback_context) != 0)
	{
		result = 0;
	}
	else
	{
		result = sz;
	}

	return result;
}

static int on_handshake_done(WOLFSSL* ssl, void* context)
{
	TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
	if ((tls_io_instance->io_state == IO_STATE_OPENING) &&
		(tls_io_instance->handshake_state == HANDSHAKE_STATE_IN_HANDSHAKE))
	{
		tls_io_instance->handshake_state = HANDSHAKE_STATE_DONE;
		set_io_state(tls_io_instance, IO_STATE_OPEN);
	}

	return 0;
}

static void tlsio_on_io_state_changed(void* context, IO_STATE new_io_state, IO_STATE previous_io_state)
{
}

int tlsio_wolfssl_init(void)
{
	(void)wolfSSL_library_init();
	wolfSSL_load_error_strings();

	return 0;
}

void tlsio_wolfssl_deinit(void)
{
}

IO_HANDLE tlsio_wolfssl_create(void* io_create_parameters, LOGGER_LOG logger_log)
{
	TLSIO_WOLFSSL_CONFIG* tls_io_config = io_create_parameters;
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

			result->ssl_context = wolfSSL_CTX_new(wolfTLSv1_client_method());
			if (result->ssl_context == NULL)
			{
				free(result);
				result = NULL;
			}
			else
			{
				const IO_INTERFACE_DESCRIPTION* socket_io_interface = socketio_get_interface_description();
				if (socket_io_interface == NULL)
				{
					wolfSSL_CTX_free(result->ssl_context);
					free(result);
					result = NULL;
				}
				else
				{
					result->socket_io = io_create(socket_io_interface, &socketio_config, logger_log);
					if (result->socket_io == NULL)
					{
						wolfSSL_CTX_free(result->ssl_context);
						free(result);
						result = NULL;
					}
					else
					{
						result->ssl = wolfSSL_new(result->ssl_context);
						if (result->ssl == NULL)
						{
							wolfSSL_CTX_free(result->ssl_context);
							free(result);
							result = NULL;
						}
						else
						{
							result->socket_io_read_bytes = NULL;
							result->socket_io_read_byte_count = 0;
							result->on_send_complete = NULL;
							result->on_send_complete_callback_context = NULL;

							wolfSSL_set_verify(result->ssl, SSL_VERIFY_NONE, NULL);
							wolfSSL_set_using_nonblock(result->ssl, 1);
							wolfSSL_SetIOSend(result->ssl_context, on_io_send);
							wolfSSL_SetIORecv(result->ssl_context, on_io_recv);
							wolfSSL_SetHsDoneCb(result->ssl, on_handshake_done, result);
							wolfSSL_SetIOWriteCtx(result->ssl, result);
							wolfSSL_SetIOReadCtx(result->ssl, result);

							set_io_state(result, IO_STATE_NOT_OPEN);
						}
					}
				}
			}
		}
	}

	return result;
}

void tlsio_wolfssl_destroy(IO_HANDLE tls_io)
{
	if (tls_io != NULL)
	{
		TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
		wolfSSL_free(tls_io_instance->ssl);
		wolfSSL_CTX_free(tls_io_instance->ssl_context);

		if (tls_io_instance->socket_io_read_bytes != NULL)
		{
			free(tls_io_instance->socket_io_read_bytes);
		}

		io_destroy(tls_io_instance->socket_io);
		free(tls_io);
	}
}

int tlsio_wolfssl_open(IO_HANDLE tls_io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context)
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
				int res;
				tls_io_instance->handshake_state = HANDSHAKE_STATE_IN_HANDSHAKE;

				res = wolfSSL_connect(tls_io_instance->ssl);
				if (res != SSL_SUCCESS)
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

	return result;
}

int tlsio_wolfssl_close(IO_HANDLE tls_io)
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

int tlsio_wolfssl_send(IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
	int result;

	if (tls_io == NULL)
	{
		result = __LINE__;
	}
	else
	{
		TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
		tls_io_instance->on_send_complete = on_send_complete;
		tls_io_instance->on_send_complete_callback_context = callback_context;
		int res = wolfSSL_write(tls_io_instance->ssl, buffer, size);
		if (res != size)
		{
			result = __LINE__;
		}
		else
		{
			result = 0;
		}
	}

	return result;
}

void tlsio_wolfssl_dowork(IO_HANDLE tls_io)
{
	if (tls_io != NULL)
	{
		TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

		decode_ssl_received_bytes(tls_io_instance);
		io_dowork(tls_io_instance->socket_io);
	}
}

const IO_INTERFACE_DESCRIPTION* tlsio_wolfssl_get_interface_description(void)
{
	return &tlsio_wolfssl_interface_description;
}
