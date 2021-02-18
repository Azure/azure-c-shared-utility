// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdio.h"
#include "stdbool.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/http_proxy_io.h"
#include "azure_c_shared_utility/platform.h"


#define PROXY_HOSTNAME "<<<proxy server hostname>>>"
#define PROXY_PORT 443
#define PROXY_SERVER_CERTIFICATE_DATA "<<<server ssl certificate data which is used by the client to authentcate the identity of the proxy server>>>"
#define PROXY_CLIENT_CERTIFICATE_DATA "<<<client side certificate data which is used by the proxy server to authenticate the identity of the client>>>"
#define PROXY_CLIENT_PRIVATE_KEY_DATA "<<<client side private key which is used in the process of proxy server authenticating the client>>>"


// A simple sample callback.
static void on_send_complete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    (void)send_result;
}

static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context, (void)open_result;
    (void)printf("Open complete called\r\n");

    if (open_result == IO_OPEN_OK)
    {
        XIO_HANDLE httpproxyio = (XIO_HANDLE)context;
        const char to_send[] = "GET / HTTP/1.1\r\n"
            "Host: www.google.com\r\n"
            "\r\n";
        (void)printf("Sending bytes ...\r\n");
        if (xio_send(httpproxyio, to_send, sizeof(to_send), on_send_complete, NULL) != 0)
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
    (void)printf("Received %lu bytes\r\n", (unsigned long)size);
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
        result = MU_FAILURE;
    }
    else
    {
        const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();
        const IO_INTERFACE_DESCRIPTION* proxyio_interface = http_proxy_io_get_interface_description();
        if (tlsio_interface == NULL)
        {
            (void)printf("Error getting tlsio interface description.");
            result = MU_FAILURE;
        }
        else
        {
            HTTP_PROXY_IO_CONFIG proxy_io_config = { "www.google.com", 443, PROXY_HOSTNAME, PROXY_PORT, NULL, NULL };
            TLSIO_CONFIG tlsio_config = { "www.google.com", 443, NULL, NULL};
            XIO_HANDLE tlsio;
            XIO_HANDLE http_proxy_io;
    
            tlsio = xio_create(tlsio_interface, &tlsio_config);
            http_proxy_io = xio_create(proxyio_interface, &proxy_io_config);

            // configure underlying tlsio, set certificates
            bool use_tls_http_proxy = true;
            xio_setoption(http_proxy_io, "use_tls_http_proxy", &use_tls_http_proxy);
            const char* sever_ssl_cert = PROXY_SERVER_CERTIFICATE_DATA;
            const char* client_side_cert = PROXY_CLIENT_CERTIFICATE_DATA;
            const char* client_side_private_key = PROXY_CLIENT_PRIVATE_KEY_DATA;
            xio_setoption(http_proxy_io, "TrustedCerts", sever_ssl_cert);
            xio_setoption(http_proxy_io, "x509certificate", client_side_cert);
            xio_setoption(http_proxy_io, "x509privatekey", client_side_private_key);

            if (http_proxy_io == NULL)
            {
                (void)printf("Error creating HTTP PROXY IO.");
                result = MU_FAILURE;
            }
            else
            {
                if (xio_open(http_proxy_io, on_io_open_complete, http_proxy_io, on_io_bytes_received, http_proxy_io, on_io_error, http_proxy_io) != 0)
                {
                    (void)printf("Error opening HTTP PROXY IO.");
                    result = MU_FAILURE;
                }
                else
                {
                    unsigned char done = 0;
                    while (!done)
                    {
                        xio_dowork(http_proxy_io);
                    }

                    result = 0;
                }

                xio_destroy(http_proxy_io);
            }
        }

        platform_deinit();
    }

    return result;
}
