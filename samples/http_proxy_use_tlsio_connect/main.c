// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/http_proxy_io.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"


#define PROXY_HOSTNAME "127.0.0.1"
#define PROXY_PORT 443

#define FILE_PATH_OF_CERTIFICATE_OF_PROXY_SERVER "<proxy server cert file path in .pem format>"
#define FILE_PATH_OF_CERTIFICATE_OF_PROXY_CLIENT_AUTH "<proxy client cert file path in .pem format>"
#define FILE_PATH_OF_PRIVATE_KEY_OF_PROXY_CLIENT_AUTH "<proxy client private key file path in .pem format>"

#define FILE_PATH_OF_CERTIFICATE_OF_HOST_SERVER "<host cert file path in .pem format>"
#define FILE_PATH_OF_CERTIFICATE_OF_HOSR_CLIENT_AUTH "<host client cert file path in .pem format>"
#define FILE_PATH_OF_PRIVATE_KEY_OF_HOSR_CLIENT_AUTH "<host client private key file path in .pem format>"


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
        XIO_HANDLE tlsio = (XIO_HANDLE)context;
        const char to_send[] = "GET / HTTP/1.1\r\n"
            "Host: www.microsoft.com\r\n"
            "\r\n";
        (void)printf("Sending bytes ...\r\n");
        if (xio_send(tlsio, to_send, sizeof(to_send), on_send_complete, NULL) != 0)
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
    printf("%s\n", buffer);
    (void)printf("Received %lu bytes\r\n", (unsigned long)size);
}

static void on_io_error(void* context)
{
    (void)context;
    (void)printf("IO reported an error\r\n");
}

static char* read_data_from_file(const char* file_path)
{
    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen(file_path, "rb");
    if (!fp) perror("blah.txt"), exit(1);

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    /* allocate memory for entire content */
    buffer = calloc(1, lSize + 1);
    if (!buffer) fclose(fp), fputs("memory alloc fails", stderr), exit(1);

    /* copy the file into the buffer */
    if (1 != fread(buffer, lSize, 1, fp))
        fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

    fclose(fp);
    return buffer;
}

int main(int argc, char** argv)
{
    int result;

    (void)argc, (void)argv;
    char* proxy_host_cert_data = NULL;
    char* host_cert_data = NULL;
    proxy_host_cert_data = read_data_from_file(FILE_PATH_OF_CERTIFICATE_OF_PROXY_SERVER);
    host_cert_data = read_data_from_file(FILE_PATH_OF_CERTIFICATE_OF_HOST_SERVER);
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
            HTTP_PROXY_IO_CONFIG proxy_io_config = { "https://www.microsoft.com/", 443, PROXY_HOSTNAME, PROXY_PORT, NULL, NULL };

            XIO_HANDLE tlsio;

            TLSIO_CONFIG tlsio_config = { "https://www.microsoft.com/", 443, NULL, NULL };
            tlsio_config.underlying_io_parameters = &proxy_io_config;
            tlsio_config.underlying_io_interface = proxyio_interface;
            tlsio = xio_create(tlsio_interface, &tlsio_config);
            xio_setoption(tlsio, "use_tls_http_proxy", proxy_host_cert_data);
            xio_setoption(tlsio, "tls_proxy_host_TrustedCerts", proxy_host_cert_data);
            xio_setoption(tlsio, "TrustedCerts", host_cert_data);

            if (tlsio == NULL)
            {
                (void)printf("Error creatingTLS IO.");
                result = MU_FAILURE;
            }
            else
            {
                if (xio_open(tlsio, on_io_open_complete, tlsio, on_io_bytes_received, tlsio, on_io_error, tlsio) != 0)
                {
                    (void)printf("Error opening tlsio.");
                    result = MU_FAILURE;
                }
                else
                {
                    unsigned char done = 0;
                    while (!done)
                    {
                        xio_dowork(tlsio);
                    }

                    result = 0;
                }

                xio_destroy(tlsio);
            }
        }

        if (proxy_host_cert_data != NULL)
        {
            free(proxy_host_cert_data);
        }

        if (host_cert_data != NULL)
        {
            free(host_cert_data);
        }
        platform_deinit();
    }

    return result;
}
