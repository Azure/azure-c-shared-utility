/*
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*/

#include "mbed.h"

#include <stddef.h>
#include "TCPSocketConnection.h"
#include "tcpsocketconnection_c.h"

extern "C"
{
	TCPSOCKETCONNECTION_HANDLE connHandle;
}

TCPSOCKETCONNECTION_HANDLE tcpsocketconnection_create(void)
{
    TCPSocketConnection* sock = new TCPSocketConnection();
    connHandle = (TCPSOCKETCONNECTION_HANDLE)sock;
    return connHandle;
}

void tcpsocketconnection_set_blocking(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, bool blocking, unsigned int timeout)
{
	TCPSocketConnection* tsc = (TCPSocketConnection*)tcpSocketConnectionHandle;
	tsc->set_blocking(blocking, timeout);
}

void tcpsocketconnection_destroy(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
	delete (TCPSocketConnection*)tcpSocketConnectionHandle;
}

int tcpsocketconnection_connect(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* host, const int port)
{
	TCPSocketConnection* tsc = (TCPSocketConnection*)tcpSocketConnectionHandle;
	return tsc->connect(host, port);
}

bool tcpsocketconnection_is_connected(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
	TCPSocketConnection* tsc = (TCPSocketConnection*)tcpSocketConnectionHandle;
	return tsc->is_connected();
}

void tcpsocketconnection_close(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
	TCPSocketConnection* tsc = (TCPSocketConnection*)tcpSocketConnectionHandle;
	tsc->close();
}

int tcpsocketconnection_send(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* data, int length)
{
	TCPSocketConnection* tsc = (TCPSocketConnection*)tcpSocketConnectionHandle;
	return tsc->send((char*)data, length);
}

int tcpsocketconnection_send_all(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* data, int length)
{
	TCPSocketConnection* tsc = (TCPSocketConnection*)tcpSocketConnectionHandle;
	return tsc->send_all((char*)data, length);
}

int tcpsocketconnection_receive(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, char* data, int length)
{
	TCPSocketConnection* tsc = (TCPSocketConnection*)tcpSocketConnectionHandle;
	return tsc->receive(data, length);
}

int tcpsocketconnection_receive_all(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, char* data, int length)
{
	TCPSocketConnection* tsc = (TCPSocketConnection*)tcpSocketConnectionHandle;
	return tsc->receive_all(data, length);
}
