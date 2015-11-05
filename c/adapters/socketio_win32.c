// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include "socketio.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include "windows.h"

typedef struct SOCKET_IO_INSTANCE_TAG
{
	SOCKET socket;
	ON_BYTES_RECEIVED on_bytes_received;
	ON_IO_STATE_CHANGED on_io_state_changed;
	LOGGER_LOG logger_log;
	void* callback_context;
	char* hostname;
	int port;
	IO_STATE io_state;
	unsigned char* pending_send_bytes;
	size_t pending_send_byte_count;
} SOCKET_IO_INSTANCE;

static const IO_INTERFACE_DESCRIPTION socket_io_interface_description = 
{
	socketio_create,
	socketio_destroy,
	socketio_open,
	socketio_close,
	socketio_send,
	socketio_dowork
};

static void set_io_state(SOCKET_IO_INSTANCE* socket_io_instance, IO_STATE io_state)
{
	IO_STATE previous_state = socket_io_instance->io_state;
	socket_io_instance->io_state = io_state;
	if (socket_io_instance->on_io_state_changed != NULL)
	{
		socket_io_instance->on_io_state_changed(socket_io_instance->callback_context, io_state, previous_state);
	}
}

IO_HANDLE socketio_create(void* io_create_parameters, LOGGER_LOG logger_log)
{
	SOCKETIO_CONFIG* socket_io_config = io_create_parameters;
	SOCKET_IO_INSTANCE* result;

	if (socket_io_config == NULL)
	{
		result = NULL;
	}
	else
	{
		result = malloc(sizeof(SOCKET_IO_INSTANCE));
		if (result != NULL)
		{
			result->hostname = (char*)malloc(strlen(socket_io_config->hostname) + 1);
			if (result->hostname == NULL)
			{
				free(result);
				result = NULL;
			}
			else
			{
				strcpy(result->hostname, socket_io_config->hostname);
				result->port = socket_io_config->port;
				result->on_bytes_received = NULL;
				result->on_io_state_changed = NULL;
				result->logger_log = logger_log;
				result->socket = INVALID_SOCKET;
				result->callback_context = NULL;
				result->pending_send_byte_count = 0;
				result->pending_send_bytes = NULL;
				result->io_state = IO_STATE_NOT_OPEN;
			}
		}
	}

	return result;
}

void socketio_destroy(IO_HANDLE socket_io)
{
	if (socket_io != NULL)
	{
		SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
		/* we cannot do much if the close fails, so just ignore the result */
		(void)closesocket(socket_io_instance->socket);
		free(socket_io_instance->hostname);
		free(socket_io);
	}
}

int socketio_open(IO_HANDLE socket_io, ON_BYTES_RECEIVED on_bytes_received, ON_IO_STATE_CHANGED on_io_state_changed, void* callback_context)
{
	int result;

	SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
	if (socket_io == NULL)
	{
		result = __LINE__;
	}
	else
	{
		ADDRINFO* addrInfo;
		char portString[16];

		socket_io_instance->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socket_io_instance->socket == INVALID_SOCKET)
		{
			set_io_state(socket_io_instance, IO_STATE_ERROR);
			result = __LINE__;
		}
		else
		{
			sprintf(portString, "%u", socket_io_instance->port);
			if (getaddrinfo(socket_io_instance->hostname, portString, NULL, &addrInfo) != 0)
			{
				closesocket(socket_io_instance->socket);
				set_io_state(socket_io_instance, IO_STATE_ERROR);
				socket_io_instance->socket = INVALID_SOCKET;
				result = __LINE__;
			}
			else
			{
				u_long iMode = 1;

				if (connect(socket_io_instance->socket, addrInfo->ai_addr, sizeof(*addrInfo->ai_addr)) != 0)
				{
					closesocket(socket_io_instance->socket);
					set_io_state(socket_io_instance, IO_STATE_ERROR);
					socket_io_instance->socket = INVALID_SOCKET;
					result = __LINE__;
				}
				else if (ioctlsocket(socket_io_instance->socket, FIONBIO, &iMode))
				{
					closesocket(socket_io_instance->socket);
					set_io_state(socket_io_instance, IO_STATE_ERROR);
					socket_io_instance->socket = INVALID_SOCKET;
					result = __LINE__;
				}
				else
				{
					socket_io_instance->on_bytes_received = on_bytes_received;
					socket_io_instance->on_io_state_changed = on_io_state_changed;
					socket_io_instance->callback_context = callback_context;

					set_io_state(socket_io_instance, IO_STATE_OPEN);
					result = 0;
				}
			}
		}
	}

	return result;
}

int socketio_close(IO_HANDLE socket_io)
{
	int result = 0;

	if (socket_io == NULL)
	{
		result = __LINE__;
	}
	else
	{
		SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;

		closesocket(socket_io_instance->socket);
		socket_io_instance->socket = INVALID_SOCKET;
		set_io_state(socket_io_instance, IO_STATE_NOT_OPEN);
		result = 0;
	}

	return result;
}

int socketio_send(IO_HANDLE socket_io, const void* buffer, size_t size)
{
	int result;

	if ((socket_io == NULL) ||
		(buffer == NULL) ||
		(size == 0))
	{
		/* Invalid arguments */
		result = __LINE__;
	}
	else
	{
		SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
		if (socket_io_instance->io_state != IO_STATE_OPEN)
		{
			result = __LINE__;
		}
		else
		{
			int send_result = send(socket_io_instance->socket, buffer, size, 0);
			if (send_result != size)
			{
				int last_error = WSAGetLastError();

				if (last_error != WSAEWOULDBLOCK)
				{
					result = __LINE__;
				}
				else
				{
					/* queue data */
					unsigned char* new_pending_send_bytes = realloc(socket_io_instance->pending_send_bytes, socket_io_instance->pending_send_byte_count + size);
					if (new_pending_send_bytes == NULL)
					{
						result = __LINE__;
					}
					else
					{
						socket_io_instance->pending_send_bytes = new_pending_send_bytes;
						(void)memcpy(socket_io_instance->pending_send_bytes + socket_io_instance->pending_send_byte_count, buffer, size);
						socket_io_instance->pending_send_byte_count += size;

						result = 0;
					}
				}
			}
			else
			{
				size_t i;
				for (i = 0; i < size; i++)
				{
					LOG(socket_io_instance->logger_log, 0, "%02x-> ", ((unsigned char*)buffer)[i]);
				}

				result = 0;
			}
		}
	}

	return result;
}

void socketio_dowork(IO_HANDLE socket_io)
{
	if (socket_io != NULL)
	{
		SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
		if (socket_io_instance->io_state == IO_STATE_OPEN)
		{
			int received = 1;

			if (socket_io_instance->pending_send_byte_count > 0)
			{
				int send_result = send(socket_io_instance->socket, socket_io_instance->pending_send_bytes, socket_io_instance->pending_send_byte_count, 0);
				if (send_result != socket_io_instance->pending_send_byte_count)
				{
					int last_error = WSAGetLastError();
					if (last_error != WSAEWOULDBLOCK)
					{
						/* error */
					}
					else
					{
						/* simply wait */
					}
				}
				else
				{
					free(socket_io_instance->pending_send_bytes);
					socket_io_instance->pending_send_bytes = NULL;
					socket_io_instance->pending_send_byte_count = 0;
				}
			}

			while (received > 0)
			{
				unsigned char recv_bytes[1];
				received = recv(socket_io_instance->socket, recv_bytes, sizeof(recv_bytes), 0);
				if (received > 0)
				{
					int i;
					for (i = 0; i < received; i++)
					{
						LOG(socket_io_instance->logger_log, 0, "<-%02x ", (unsigned char)recv_bytes[i]);
					}

					if (socket_io_instance->on_bytes_received != NULL)
					{
						/* explictly ignoring here the result of the callback */
						(void)socket_io_instance->on_bytes_received(socket_io_instance->callback_context, recv_bytes, received);
					}
				}
			}
		}
	}
}

const IO_INTERFACE_DESCRIPTION* socketio_get_interface_description(void)
{
	return &socket_io_interface_description;
}
