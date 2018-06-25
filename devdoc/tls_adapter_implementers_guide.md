# tls_adapter implementer's guide

This document describes how to implement a `tls_adapter`.

A `tls_adapter` is a component designed to work with the Azure IoT C SDK to implement
secure communication via TLS to an Azure IoT Hub. The `tls_adapter` is the lowest level
in a stack of components that compose what the Azure IoT C SDK documentation
refers to as a "tlsio".
For more information, see the [tlsio_adapter overview](tlsio_adapter_overview.md).

### Can your TLS library work with a socket?

##### Libraries that work with sockets
TLS libraries are often written to eliminate any dependency on the specific mechanism
of TCP/IP communication. The usual technique is for the user of the library to 
pass in a set of "bio" functions, which are very simple functions, written by the
library user, that the library uses when it needs to read or write over TCP.

Examples of libraries that can work with sockets via "bio" functions include 
Windows Schannel, WolfSSL, Mbed TLS, OpenSSL, and CycloneSSL. At least one
TLS library, the OpenSSL implementation used in Espressif's 
ESP32, can even use a socket directly.

The Azure IoT SDK helps users of libraries that work with sockets by performing the 
work of socket maintenance, and it provides an abstracted socket called
`socket_async` to the `tls_adapter` during initialization. This socket is already
open and ready for use. Libraries that use "bio" functions should implement
these functions in terms of the read and write functions provided in 
`socket_async.h`. Libraries that want to use a socket directly can simply
cast the provided `socket_async` object to a socket handle; nothing else
is required.

If your TLS library can work with sockets, you will:
* write a `socket_async_os.h` that #includes your system's socket-related headers 
as needed to allow `socket_async.c` to compile.
* include `xio_state.c` in your project
* include `socket_async.c` in your project
* include `tlsio_adapter_with_sockets.c` in your project
* make sure you exclude `tlsio_adapter_basic.c` from your project 
* create a `tls_adapter_my_lib_name.c` file that implements the functions in
  * `tls_adapter_common.h`, and
  * `tls_adapter_with_sockets.h`

That's it! You have completed writing your tlsio adapter. (See 
below for more details.)

##### Libraries that don't work with sockets

Other TLS libraries have no use for sockets because they already have TCP/IP
functionality integrated. Examples include Arduino, Apple OSX, and Apple iOS.

For these libraries the SDK will not create an unneeded `socket_async`.

If you are using a library that doesn't use sockets, you will:
* include `xio_state.c` in your project
* include `tlsio_adapter_basic.c` in your project
* make sure you exclude `tlsio_adapter_with_sockets.c` from your project 
* create a `tls_adapter_my_lib_name.c` file that implements the functions in
  * `tls_adapter_common.h`, and
  * `tls_adapter_basic.h`
  
That's it! You have completed writing your tlsio adapter.

Now let's dive into the details.

## Required headers

All `tls_adapter` components need a few standard required headers 
in order to integrate properly with the Azure IoT C SDK:
```c
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/tlsio_options.h"
#include "tls_adapter_common.h"
// and either
#include "tls_adapter_basic.h"
// or
#include "tls_adapter_with_sockets.h"
```

## Object lifetime

The Azure IoT SDK attempts to maintain an open connection to its IoT Hub at
all times. If the connection fails for any reason, the SDK will keep attempting
to reconnect (at reasonable intervals) until it succeeds. 

The call sequence looks like this:
1. `tls_adapter_common_init` (called only once)
2. `tls_adapter_with_sockets_create` or `tls_adapter_basic_create`
3. Repeated calls to `tls_adapter_common_open` until it either fails or succeeds.
4. Repeated calls to `tls_adapter_common_read` and `tls_adapter_common_write` 
(assuming `tls_adapter_common_open` succeeded)
5. When the connection eventually fails or the app is closed, 
`tls_adapter_common_close_and_destroy` is called.
6. For a failed connection, the SDK returns to Step 2 to reconnect.
7. For an app shutdown, the SDK calls `tls_adapter_common_deinit` 
and is finished.

## One-time init and de-init functions

```c
int tls_adapter_common_init(void);
```
This function is called before any other `tls_adapter` function calls
are made, and can be used for any required one-time setup. This call only
occurs once during startup. Most implementations will leave this function
empty.

```c
void tls_adapter_common_deinit(void);
```
This function is called only once, during normal shutdown of the containing
application. Notice that on microcontrollers there is usually no such thing as
an orderly application shutdown, meaning that this function would never be
called. Most implementations will leave this function empty.

## The option capabilities function

The `tlsio_adapter` framework currently is capable of supporting two kinds
of settable options for how the TLS layer should verify the identity of the
IoT Hub server and how the TLS layer should present the device's identity
to the IoT Hub server. These options are set at high levels in the SDK and 
passed into the `tls_adapter_with_sockets_create` and 
`tls_adapter_basic_create` calls in the `tlsio_options` parameter.

Before calling `tls_adapter_with_sockets_create` or 
`tls_adapter_basic_create`, the middle layer will call this `tls_adapter` function:
```c
int tls_adapter_common_get_option_caps(void);
```
The `tlsio_adapter` must return a bit-or combination of this enum:
```c
    typedef enum TLSIO_OPTION_BIT_TAG
    {
        TLSIO_OPTION_BIT_NONE =          0x00,
        TLSIO_OPTION_BIT_TRUSTED_CERTS = 0x01,
        TLSIO_OPTION_BIT_x509_RSA_CERT = 0x02,
        TLSIO_OPTION_BIT_x509_ECC_CERT = 0x04,
    } TLSIO_OPTION_BIT;
```
This will tell the SDK what options the `tls_adapter` is able to understand. If a
higher-level SDK call tries to set an option that the `tls_adapter` has not claimed
to support in the `tls_adapter_common_get_option_caps` call, the middle layer will
return an error and refuse to set the option.

#### Trusted Certs option

The "TrustedCerts" option is almost always necessary for microcontrollers, and most
of the time it will be the only option supported at this level. So for most 
implementations, the `tls_adapter_common_get_option_caps` function is 
implemented like this:
```c
int tls_adapter_common_get_option_caps()
{
    return TLSIO_OPTION_BIT_TRUSTED_CERTS;
}
```
If the "TrustedCerts" option is enabled and set, a PEM format CA cert chain will
be passed into the create functions as part of the create call's `tlsio_options`
parameter. An example of using a trusted cert can be seen in the
[tls_adapter_with_sockets example](#A-simple-tls_adapter_with_sockets-example).

If the `tls_adapter_common_get_option_caps` reports that it is capable 
of accepting the "TrustedCerts" 
option, then the SDK will only call the creation functions 
(`tls_adapter_with_sockets_create` or `tls_adapter_basic_create`) if the 
"Trusted Certs" option has been set.

##### x509 RSA option

The x509 RSA option replaces the
usual IoT Hub connection string. The IoT Hub connection string is the preferred 
method of presenting credentials to the IoT Hub, however, and the x509 RSA option 
is not discussed further in this document.

##### x509 ECC option

This is for future expansion only.

## Creation functions

Depending on whether your TLS library uses sockets or not, you will create
your `tls_adapter` with either this call:
```c
// All of the parameters of this call have permanent existence, and need not be deep-copied
TLS_ADAPTER_INSTANCE_HANDLE tls_adapter_with_sockets_create(TLSIO_OPTIONS* tlsio_options,
        const char* hostname, SOCKET_ASYNC_HANDLE socket_async);
```
or this one:
```c
// All of the parameters of this call have permanent existence, and need not be deep-copied
TLS_ADAPTER_INSTANCE_HANDLE tls_adapter_basic_create(TLSIO_OPTIONS* tlsio_options,
        const char* hostname, uint16_t port);
```
Very little work takes place in the creation functions. The typical usage is to `malloc` a
struct where you can store the supplied parameters plus any other data you may need
during the lifetime of your object, fill in that struct with the 
`tls_adapter_with_sockets_create` or `tls_adapter_basic_create` parameters, and
return a pointer to the struct. This struct then gets passed back to you in all the 
remaining function calls.

## The open function

After the SDK has successfully created a `tls_adapter` with `tls_adapter_with_sockets_create` or 
`tls_adapter_basic_create`, it will call start making repeated calls to 
`tls_adapter_common_open`:
```c
XIO_ASYNC_RESULT tls_adapter_common_open(TLS_ADAPTER_INSTANCE_HANDLE adapter);
```
This call returns a value from the `XIO_ASYNC_RESULT` enum:
```c
typedef enum
{
    XIO_ASYNC_RESULT_SUCCESS = 0,
    XIO_ASYNC_RESULT_WAITING = 1,
    XIO_ASYNC_RESULT_FAILURE = -1
} XIO_ASYNC_RESULT;
```
The return values cause the SDk to behave as follows:
* If **`XIO_ASYNC_RESULT_SUCCESS`** is returned the SDK will start communicating with 
the IoT Hub using the `tls_adapter_common_read` and `tls_adapter_common_write`
calls, and will continue to do so until there is a communiction failure or an app shutdown.
* If **`XIO_ASYNC_RESULT_FAILURE`** is returned, the SDK will call 
`tls_adapter_common_close_and_destroy` then try to reconnect later.
* If **`XIO_ASYNC_RESULT_WAITING`** is returned, the SDK will understand that the 
`tls_adapter_common_open` call is waiting for something (typically a response
from the IoT Hub server), and will retry the `tls_adapter_common_open` call in
a few milliseconds.

The kind of work done within the `tls_adapter_common_open` call varies depending
on the nature of the TLS library. See 
[tls_adapter_basic example](#A-simple-tls_adapter_basic-example) and
[tls_adapter_with_sockets example](#A-simple-tls_adapter_with_sockets-example).

## The read function

When the `tls_adapter` receives the read call,
```c
int tls_adapter_common_read(TLS_ADAPTER_INSTANCE_HANDLE adapter,
        uint8_t* buffer, size_t size);
```
it should try to read `size` bytes from the remote host and copy them into
`buffer'. If it succeeds in reading one or more bytes, it returns the 
number of bytes successfully read.

The `size` parameter is guaranteed to be smaller (in practice far smaller) 
than MAXINT, so the type mismatch between the supplied 
`size` and the integer return value is not an issue.

**Note:** It is important for the read function to ensure that the connection
is still good when it is called. If sockets or something similar are used,
the "end of file" condition in particular must be checked because "end of file"
is not usually considered an error, but it means the connection has been
closed by the host, and the SDK must be notified of this as an error.

The return values cause the SDK to behave as follows:
* If **`XIO_ASYNC_RESULT_FAILURE`** is returned, the SDK will call 
`tls_adapter_common_close_and_destroy` then try to reconnect later.
* If **`XIO_ASYNC_RESULT_WAITING`** is returned, the SDK will understand 
that the connection is still good
but that no data is available, and it will try again later.
* If a positive value is returned, the SDK will digest the data returned in
'buffer' then repeat the call to `tls_adapter_common_read` until all the
available data is consumed.

Examples of the read function can be found in
[tls_adapter_basic example](#A-simple-tls_adapter_basic-example) and
[tls_adapter_with_sockets example](#A-simple-tls_adapter_with_sockets-example).

## The write function

When the `tls_adapter` receives the write call,
```c
int tls_adapter_common_write(TLS_ADAPTER_INSTANCE_HANDLE adapter,
        const uint8_t* buffer, size_t size);
```
it should try to write `size` bytes from `buffer` to the remote host.
If it succeeds in writing one or more bytes, it returns the number of bytes
successfully written. It is considered normal for the return value to be
smaller than the `size` parameter; the SDK knows how to handle this,
and will repeat the call later with the unsent portion of the message. The
low-level `tls_adapter` can safely ignore the issue.

The `size` parameter is guaranteed to be smaller (in practice far smaller) 
than MAXINT, so the type mismatch between the supplied 
`size` and the integer return value is not an issue.

The return values cause the SDK to behave as follows:
* If **`XIO_ASYNC_RESULT_FAILURE`** is returned, the SDK will call 
`tls_adapter_common_close_and_destroy` then try to reconnect later.
* If **`XIO_ASYNC_RESULT_WAITING`** is returned, the SDK will understand 
that the connection is still good
but currently unable to accept more data, and it will try again later.
* If a positive value is returned, the SDK will understand that this number
of bytes have been successfully sent.

Examples of the write function can be found in
[tls_adapter_basic example](#A-simple-tls_adapter_basic-example) and
[tls_adapter_with_sockets example](#A-simple-tls_adapter_with_sockets-example).

## The close and destroy function

When the `tls_adapter` receives the close and destroy call,
```c
void tls_adapter_common_close_and_destroy(TLS_ADAPTER_INSTANCE_HANDLE adapter);
```
it will behave a little bit differently depending on whether the `tls_adapter` was
created with `tls_adapter_with_sockets_create` or 
`tls_adapter_basic_create`.

* If it was created with `tls_adapter_basic_create`, then the 
`tls_adapter_common_close_and_destroy` function must close the connection
to the IoT Hub and release any resources it has acquired, and free the supplied
`TLS_ADAPTER_INSTANCE_HANDLE`.

* If it was created with `tls_adapter_with_sockets_create`, the `socket_async`
connection to the IoT Hub will be closed by the SDK, so the `tls_adapter`
need do nothing with the supplied `socket_async`. All it needs to do is 
release any acquired resources and free the supplied
`TLS_ADAPTER_INSTANCE_HANDLE`.

Examples of the close and destroy function can be found in
[tls_adapter_basic example](#A-simple-tls_adapter_basic-example) and
[tls_adapter_with_sockets example](#A-simple-tls_adapter_with_sockets-example).

## A simple tls_adapter_basic example

Here is a non-working example of a `tls_adapter_basic` from Contoso Corporation:
```c
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/tlsio_options.h"
#include "tls_adapter_common.h"
#include "tls_adapter_basic.h"
#include "contosoTls_contoso.h"

typedef struct TLS_ADAPTER_INSTANCE_TAG
{
    const char*     hostname;
    uint16_t        port;
    uint32_t        ipAddress;
} TLS_ADAPTER_INSTANCE;

int tls_adapter_common_init()
{
    return 0;
}

void tls_adapter_common_deinit()
{
}

int tls_adapter_common_get_option_caps()
{
    return TLSIO_OPTION_BIT_NONE;
}

TLS_ADAPTER_INSTANCE_HANDLE tls_adapter_basic_create(TLSIO_OPTIONS* tlsio_options, 
    const char* hostname, uint16_t port)
{
    TLS_ADAPTER_INSTANCE* tls_adapter = malloc(sizeof(TLS_ADAPTER_INSTANCE));
    (void)tlsio_options;    // contosoTls_contoso doesn't support any options
    if (tls_adapter != NULL)
    {
        // No hostname string copy is needed here
        tls_adapter->hostname = hostname;
        tls_adapter->port = port;
    }

    return tls_adapter;
}

void tls_adapter_common_close_and_destroy(TLS_ADAPTER_INSTANCE_HANDLE tls_adapter)
{
    contosoTls_stop();
    free(tls_adapter);
}

// Called repeatedly until it returns either XIO_ASYNC_RESULT_SUCCESS 
// or XIO_ASYNC_RESULT_FAILURE
XIO_ASYNC_RESULT tls_adapter_basic_open(TLS_ADAPTER_INSTANCE_HANDLE tls_adapter)
{
    XIO_ASYNC_RESULT result;
    // A very simple open sequence. Non-zero ipAddress tells us that an earlier 
    // contosoTls_connect call succeeded
    if (tls_adapter->ipAddress == 0)
    {
        if (!contosoTls_hostByName(tls_adapter->hostname, &(tls_adapter->ipAddress)))
        {
            LogInfo("could not get DNS");
            tls_adapter->ipAddress = 0;
            result = XIO_ASYNC_RESULT_FAILURE;
        }
        else if (!contosoTls_connect(tlsio_instance->remote_addr, tlsio_instance->port))
        {
            LogInfo("contosoTls_connect failed");
            tls_adapter->ipAddress = 0;
            result = XIO_ASYNC_RESULT_FAILURE;
        }
        else
        {
            result = XIO_ASYNC_RESULT_WAITING;
        }
    }
    else
    {
        // Timeouts are handled by upper levels of the Azure IoT SDK,
        // so we don't need a timeout here
        if (contosoTls_connected() != 0)
        {
            result = XIO_ASYNC_RESULT_SUCCESS;
        }
        else
        {
            result = XIO_ASYNC_RESULT_WAITING;
        }
    }

    return result;
}

int tls_adapter_common_read(TLS_ADAPTER_INSTANCE_HANDLE tls_adapter, 
    uint8_t* buffer, size_t size)
{
    int result;
    (void)tls_adapter;

    // Here we check for "end of file"
    if (!contosoTls_connected())
    {
        LogInfo("lost connection");
        result = XIO_ASYNC_RESULT_FAILURE;
    }
    else
    {
        result = contosoTls_read((uint8_t*)buffer, size);
    }

    return result;
}

int tls_adapter_common_write(TLS_ADAPTER_INSTANCE_HANDLE tls_adapter, 
    const uint8_t* buffer, size_t count)
{
    int result;
    (void)tls_adapter;

    if (!contosoTls_connected())
    {
        LogInfo("lost connection");
        result = XIO_ASYNC_RESULT_FAILURE;
    }
    else
    {
        result = (int)contosoTls_write((uint8_t*)buffer, size);
    }

    return result;
}
```
## A simple tls_adapter_with_sockets example

Here is a non-working example of a `tls_adapter_with_sockets` from Contoso Corporation:
```c
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/tlsio_options.h"
#include "tls_adapter_with_sockets.h"
#include "socket_async.h"

#include "contosoTls.h"

typedef struct TLS_ADAPTER_INSTANCE_TAG
{
    // contosoTls stuff
    contosoTls_entropy_context     entropy;
    contosoTls_ctr_drbg_context    ctr_drbg;
    contosoTls_ssl_context         ssl;
    contosoTls_ssl_config          ssl_config;
    contosoTls_x509_crt            cacert;
    contosoTls_ssl_session         ssn;
    // socket stuff
    SOCKET_ASYNC_HANDLE         socket_async;
    // options
    TLSIO_OPTIONS*              options;
} TLS_ADAPTER_INSTANCE;

// Here is the "bio" send function that allows the Contoso library to
// send data using the supplied socket_async. We'll pass it to the library during
// the tls_adapter_with_sockets_create call.
static int contosoTls_ssl_send( void *ctx, const unsigned char *buffer, size_t len )
{
    size_t sent_count = 0;
    int result;
    TLS_ADAPTER_INSTANCE* tls_adapter = (TLS_ADAPTER_INSTANCE*)ctx;
    int socket_async_send_result = socket_async_send(tls_adapter->socket_async, 
        buffer, len, &sent_count);
    if (socket_async_send_result != 0)
    {
        LogInfo("Failed to receive from socket_async");
        result = CONTOSOTLS_ERR_NET_SEND_FAILED;
    }
    else if (sent_count == 0)
    {
        result = CONTOSOTLS_ERR_SSL_WANT_WRITE;
    }
    else
    {
        result = (int)sent_count;
    }
    return result;
}

// Here is the "bio" receive function that allows the Contoso library to
// receive data using the supplied socket_async. We'll pass it to the library during
// the tls_adapter_with_sockets_create call.
static int contosoTls_ssl_recv( void *ctx, unsigned char *buffer, size_t len )
{
    size_t received_count = 0;
    int result;
    TLS_ADAPTER_INSTANCE* tls_adapter = (TLS_ADAPTER_INSTANCE*)ctx;
    int socket_async_receive_result = socket_async_receive(
        tls_adapter->socket_async, (void*)buffer, len, &received_count);
    if (socket_async_receive_result != 0)
    {
        LogInfo("Failed to receive from socket_async");
        result = CONTOSOTLS_ERR_NET_RECV_FAILED;
    }
    else if (received_count == 0)
    {
        result = CONTOSOTLS_ERR_SSL_WANT_READ;
    }
    else
    {
        result = (int)received_count;
    }
    return result;
}

// TLS libraries often need to be given a way to generate good random numbers
static int tlsio_entropy_poll(void *v, unsigned char *output, 
    size_t len, size_t *olen)
{
    for (uint16_t i = 0; i < len; i++)
    {
        // Never use the C rand function because it would make your
        // "secure" communication with the IoT Hub as easy to crack
        // as a pistachio.
        output[i] = (unsigned char)(contosoHardwareGetRandomByte());
    }
    *olen = len;
    return(0);
}

// Nothing need be done for tls_adapter_common_init, as is usually the case
int tls_adapter_common_init()
{
    return 0;
}

void tls_adapter_common_deinit()
{
}

int tls_adapter_common_get_option_caps()
{
    return TLSIO_OPTION_BIT_TRUSTED_CERTS;
}

TLS_ADAPTER_INSTANCE_HANDLE tls_adapter_with_sockets_create(
    TLSIO_OPTIONS* tlsio_options, const char* hostname,
    SOCKET_ASYNC_HANDLE socket_async)
{
    int result;
    int ret;
    TLS_ADAPTER_INSTANCE* tls_adapter = malloc(sizeof(TLS_ADAPTER_INSTANCE));
    if (tls_adapter != NULL)
    {
        memset(tls_adapter, 0, sizeof(TLS_ADAPTER_INSTANCE));

        tls_adapter->options = tlsio_options;

        contosoTls_entropy_init(&tls_adapter->entropy);
        contosoTls_ctr_drbg_init(&tls_adapter->ctr_drbg);
        contosoTls_ssl_init(&tls_adapter->ssl);
        contosoTls_ssl_session_init(&tls_adapter->ssn);
        contosoTls_ssl_config_init(&tls_adapter->ssl_config);
        contosoTls_x509_crt_init(&tls_adapter->cacert);
        contosoTls_entropy_add_source(&tls_adapter->entropy, tlsio_entropy_poll, 
            NULL, 128, CONTOSOTLS_ENTROPY_SOURCE_STRONG);

        if ((ret = contosoTls_x509_crt_parse(&tls_adapter->cacert, 
            (const unsigned char*)tls_adapter->options->trusted_certs,
                (size_t)(strlen(tls_adapter->options->trusted_certs) + 1))) != 0)
        {
            LogError("contosoTls_x509_crt_parse returned %d", ret);
            result = XIO_ASYNC_RESULT_FAILURE;
        }
        else
        {
            // Require verification of the server (using trusted cert)
            contosoTls_ssl_conf_authmode(&tls_adapter->ssl_config, 
                CONTOSOTLS_SSL_VERIFY_REQUIRED);

            // Tell the Contoso TLS library which functions to use for network i/o
            contosoTls_ssl_set_bio(&tls_adapter->ssl, tls_adapter, 
                contosoTls_ssl_send, contosoTls_ssl_recv, NULL);

            if ((ret = contosoTls_ssl_set_hostname(&tls_adapter->ssl, hostname)) != 0)
            {
                LogError("contosoTls_ssl_set_hostname returned %d", ret);
                result = XIO_ASYNC_RESULT_FAILURE;
            }
            else
            {
                contosoTls_ssl_set_session(&tls_adapter->ssl, &tls_adapter->ssn);
                contosoTls_ssl_setup(&tls_adapter->ssl, &tls_adapter->ssl_config);
                contosoTls_ssl_conf_ca_chain(&tls_adapter->ssl_config, 
                    &tls_adapter->cacert, NULL);
                result = 0;
            }
        }
    }
    else
    {
        result = XIO_ASYNC_RESULT_FAILURE;
    }
    if (result != 0)
    {
        tls_adapter_common_close_and_destroy(tls_adapter);
        tls_adapter = NULL;
    }

    return tls_adapter;
}

void tls_adapter_common_close_and_destroy(TLS_ADAPTER_INSTANCE_HANDLE tls_adapter)
{
    contosoTls_ssl_close_notify(&tls_adapter->ssl);
    contosoTls_ssl_free(&tls_adapter->ssl);
    contosoTls_ssl_config_free(&tls_adapter->ssl_config);
    contosoTls_ctr_drbg_free(&tls_adapter->ctr_drbg);
    contosoTls_entropy_free(&tls_adapter->entropy);
    contosoTls_x509_crt_free(&tls_adapter->cacert);

    free(tls_adapter);
}

// Called repeatedly until it returns either XIO_ASYNC_RESULT_SUCCESS 
// or XIO_ASYNC_RESULT_FAILURE
XIO_ASYNC_RESULT tls_adapter_common_open(TLS_ADAPTER_INSTANCE_HANDLE tls_adapter)
{
    XIO_ASYNC_RESULT result;
    int ret = contosoTls_ssl_handshake(&tls_adapter->ssl);
    if (ret == CONTOSOTLS_ERR_SSL_WANT_READ || ret == CONTOSOTLS_ERR_SSL_WANT_WRITE)
    {
        result = XIO_ASYNC_RESULT_WAITING;
    }
    else if (ret == 0)
    {
        result = XIO_ASYNC_RESULT_SUCCESS;
    }
    else
    {
        LogError("contosoTls_ssl_handshake failure");
        result = XIO_ASYNC_RESULT_FAILURE;
    }
    return result;
}

int tls_adapter_common_read(TLS_ADAPTER_INSTANCE_HANDLE tls_adapter, 
    uint8_t* buffer, size_t size)
{
    int result;
    int rcv_bytes;

    if ((rcv_bytes = contosoTls_ssl_read(&tls_adapter->ssl, buffer, size)) > 0)
    {
        result = rcv_bytes;
    }
    else if (rcv_bytes == CONTOSOTLS_ERR_SSL_WANT_READ || 
        rcv_bytes == CONTOSOTLS_ERR_SSL_WANT_WRITE)
    {
        result = 0;
    }
    else
    {
        LogError("contosoTls_ssl_read failure");
        result = XIO_ASYNC_RESULT_FAILURE;
    }
    return result;
}

int tls_adapter_common_write(TLS_ADAPTER_INSTANCE_HANDLE tls_adapter, 
    const uint8_t* buffer, size_t count)
{
    int result;
    if ((result = contosoTls_ssl_write(&tls_adapter->ssl, buffer, count)) > 0)
    {
        // Success, nothing to do
    }
    else if (result == CONTOSOTLS_ERR_SSL_WANT_READ || 
        result == CONTOSOTLS_ERR_SSL_WANT_WRITE)
    {
        result = 0;
    }
    else
    {
        result = XIO_ASYNC_RESULT_FAILURE;
    }

    return result;
}
```
