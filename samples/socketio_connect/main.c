// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdio.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/platform.h"

static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context, open_result;
    (void)printf("Open complete called\r\n");

    if (open_result == IO_OPEN_OK)
    {
        (void)printf("Sending bytes ...\r\n");
        XIO_HANDLE socketio = (XIO_HANDLE)context;
        const char to_send[] = "GET / HTTP/1.1\r\n"
            "Host: www.google.com\r\n"
            "\r\n";
        if (xio_send(socketio, to_send, sizeof(to_send), NULL, NULL) != 0)
        {
            (void)printf("Send failed\r\n");
        }
    }
    else
    {
        (void)printf("Open error\r\n");
    }
}

static void on_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    (void)context, buffer;
    (void)printf("Received %zu bytes\r\n", size);
}

static void on_io_error(void* context)
{
    (void)context;
    (void)printf("IO reported an error\r\n");
}

int main(int argc, char** argv)
{
    int result;

    (void)argc, argv;

    if (platform_init() != 0)
    {
        (void)printf("Cannot initialize platform.");
        result = __LINE__;
    }
    else
    {
        const IO_INTERFACE_DESCRIPTION* socketio_interface = socketio_get_interface_description();
        if (socketio_interface == NULL)
        {
            (void)printf("Error getting socketio interface description.");
            result = __LINE__;
        }
        else
        {
            SOCKETIO_CONFIG socketio_config;
            XIO_HANDLE socketio;

            socketio_config.hostname = "www.google.com";
            socketio_config.port = 80;
            socketio = xio_create(socketio_interface, &socketio_config);
            if (socketio == NULL)
            {
                (void)printf("Error creating socket IO.");
                result = __LINE__;
            }
            else
            {
                if (xio_open(socketio, on_io_open_complete, socketio, on_io_bytes_received, socketio, on_io_error, socketio) != 0)
                {
                    (void)printf("Error opening socket IO.");
                    result = __LINE__;
                }
                else
                {
                    while (1)
                    {
                        xio_dowork(socketio);
                    }

                    result = 0;
                }

                xio_destroy(socketio);
            }
        }

        platform_deinit();
    }

    return result;
}
