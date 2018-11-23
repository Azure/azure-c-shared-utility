// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "mbed.h"

#include <stddef.h>
#include "TCPSocket.h"
#include "azure_c_shared_utility/tcpsocketconnection_c.h"

// The NetworkInterface instance of network device
extern NetworkInterface *_defaultSystemNetwork;

bool tcpsocketconnection_isConnected = false;

TCPSOCKETCONNECTION_HANDLE tcpsocketconnection_create(void)
{
	TCPSocket* tcpSocket = new TCPSocket();
	tcpSocket->open(_defaultSystemNetwork);
    return tcpSocket;
}

void tcpsocketconnection_set_blocking(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, bool blocking, unsigned int timeout)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	tsc->set_blocking(blocking);
	tsc->set_timeout(timeout);
}

void tcpsocketconnection_destroy(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
	delete (TCPSocket*)tcpSocketConnectionHandle;
}

int tcpsocketconnection_connect(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* host, const int port)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	int ret = tsc->connect(host, port);
	tcpsocketconnection_isConnected = true;
	return ret;
}

bool tcpsocketconnection_is_connected(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	return tcpsocketconnection_isConnected;
}

void tcpsocketconnection_close(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	tcpsocketconnection_isConnected = false;
	tsc->close();
}

int tcpsocketconnection_send(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* data, int length)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	int ret = tsc->send((char*)data, length);
	return ret;
}

int tcpsocketconnection_send_all(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* data, int length)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	return tsc->send((char*)data, length);
}

int tcpsocketconnection_receive(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, char* data, int length)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	return tsc->recv(data, length);
}

int tcpsocketconnection_receive_all(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, char* data, int length)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	return tsc->recv(data, length);
}
