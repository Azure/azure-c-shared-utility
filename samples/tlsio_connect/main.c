// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdio.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/platform.h"

static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context, (void)open_result;
    (void)printf("Open complete called\r\n");

    if (open_result == IO_OPEN_OK)
    {
        (void)printf("Sending bytes ...\r\n");
        XIO_HANDLE tlsio = (XIO_HANDLE)context;
        const char to_send[] = "GET / HTTP/1.1\r\n"
            "Host: www.google.com\r\n"
            "\r\n";
        if (xio_send(tlsio, to_send, sizeof(to_send), NULL, NULL) != 0)
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
    (void)context, (void)buffer;
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

    (void)argc, (void)argv;

    if (platform_init() != 0)
    {
        (void)printf("Cannot initialize platform.");
        result = __LINE__;
    }
    else
    {
        const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();
        if (tlsio_interface == NULL)
        {
            (void)printf("Error getting tlsio interface description.");
            result = __LINE__;
        }
        else
        {
            TLSIO_CONFIG tlsio_config;
            XIO_HANDLE tlsio;

            tlsio_config.hostname = "www.google.com";
            tlsio_config.port = 443;
            tlsio = xio_create(tlsio_interface, &tlsio_config);
            if (tlsio == NULL)
            {
                (void)printf("Error creating TLS IO.");
                result = __LINE__;
            }
            else
            {
                if (xio_open(tlsio, on_io_open_complete, tlsio, on_io_bytes_received, tlsio, on_io_error, tlsio) != 0)
                {
                    (void)printf("Error opening TLS IO.");
                    result = __LINE__;
                }
                else
                {
                    while (1)
                    {
                        xio_dowork(tlsio);
                    }

                    result = 0;
                }

                xio_destroy(tlsio);
            }
        }

        platform_deinit();
    }

    return result;
}
