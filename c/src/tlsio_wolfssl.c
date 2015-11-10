// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdbool.h>
#include "tlsio_wolfssl.h"
#include "socketio.h"
#include "amqpalloc.h"
#include "logger.h"

typedef struct TLS_IO_INSTANCE_TAG
{
	IO_HANDLE socket_io;
	ON_BYTES_RECEIVED on_bytes_received;
	ON_IO_STATE_CHANGED on_io_state_changed;
	void* callback_context;
	LOGGER_LOG logger_log;
	IO_STATE io_state;
} TLS_IO_INSTANCE;

static void set_io_state(TLS_IO_INSTANCE* tls_io_instance, IO_STATE io_state)
{
	IO_STATE previous_state = tls_io_instance->io_state;
	tls_io_instance->io_state = io_state;
	if (tls_io_instance->on_io_state_changed != NULL)
	{
		tls_io_instance->on_io_state_changed(tls_io_instance->callback_context, io_state, previous_state);
	}
}

static void tlsio_on_bytes_received(void* context, const void* buffer, size_t size)
{
	TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
}

static void tlsio_on_io_state_changed(void* context, IO_STATE new_io_state, IO_STATE previous_io_state)
{
	TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;
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
		result = amqpalloc_malloc(sizeof(TLS_IO_INSTANCE));
		if (result != NULL)
		{
			SOCKETIO_CONFIG socketio_config;

			socketio_config.hostname = tls_io_config->hostname;
			socketio_config.port = tls_io_config->port;

			result->on_bytes_received = NULL;
			result->on_io_state_changed = NULL;
			result->logger_log = logger_log;
			result->callback_context = NULL;

			const IO_INTERFACE_DESCRIPTION* socket_io_interface = socketio_get_interface_description();
			if (socket_io_interface == NULL)
			{
				amqpalloc_free(result);
				result = NULL;
			}
			else
			{
				result->socket_io = io_create(socket_io_interface, &socketio_config, logger_log);
				if (result->socket_io == NULL)
				{
					amqpalloc_free(result);
					result = NULL;
				}
				else
				{
					set_io_state(result, IO_STATE_NOT_OPEN);
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
		io_destroy(tls_io_instance->socket_io);
		amqpalloc_free(tls_io);
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
				result = 0;
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

int tlsio_wolfssl_send(IO_HANDLE tls_io, const void* buffer, size_t size)
{
	int result;

	result = 0;

	return result;
}

void tlsio_wolfssl_dowork(IO_HANDLE tls_io)
{
	if (tls_io != NULL)
	{
		TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
		io_dowork(tls_io_instance->socket_io);
	}
}

static const IO_INTERFACE_DESCRIPTION tlsio_wolfssl_interface_description =
{
	tlsio_wolfssl_create,
	tlsio_wolfssl_destroy,
	tlsio_wolfssl_open,
	tlsio_wolfssl_close,
	tlsio_wolfssl_send,
	tlsio_wolfssl_dowork
};

const IO_INTERFACE_DESCRIPTION* tlsio_wolfssl_get_interface_description(void)
{
	return &tlsio_wolfssl_interface_description;
}
