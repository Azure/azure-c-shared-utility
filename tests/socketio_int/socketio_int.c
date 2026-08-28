// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Integration tests for the socketio adapter. They drive the real socket adapter against a
// TCP listener that the test itself creates on the loopback interface, so no external
// service, no name server and no special privileges are required.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstring>
#else
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#endif

#if defined(WIN32) || defined(_WIN32)
#include "winsock2.h"
#include "ws2tcpip.h"
typedef SOCKET TEST_SOCKET;
#define TEST_INVALID_SOCKET     INVALID_SOCKET
#define test_close_socket(s)    (void)closesocket(s)
typedef int test_socklen_t;
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
typedef int TEST_SOCKET;
#define TEST_INVALID_SOCKET     (-1)
#define test_close_socket(s)    (void)close(s)
typedef socklen_t test_socklen_t;
#endif

#include "testrunnerswitcher.h"

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xio.h"

// The adapter allows up to 10 seconds for a connection to be decided, so the pump below has
// to outlast that before it declares the open stuck.
#define OPEN_POLL_INTERVAL_MS       10
#define OPEN_POLL_MAX_ITERATIONS    3000

#define TEST_HOSTNAME               "127.0.0.1"

static XIO_HANDLE g_io;
static TEST_SOCKET g_listener = TEST_INVALID_SOCKET;

static bool g_open_completed;
static IO_OPEN_RESULT g_open_result;

static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    g_open_completed = true;
    g_open_result = open_result;
}

static void on_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    (void)context;
    (void)buffer;
    (void)size;
}

static void on_io_error(void* context)
{
    (void)context;
}

// Binds an ephemeral loopback port and immediately releases it. Connecting to the returned
// port is then refused until the test brings a listener up on it.
static int reserve_unused_port(void)
{
    struct sockaddr_in address;
    test_socklen_t address_length = sizeof(address);
    int result;
    TEST_SOCKET probe = socket(AF_INET, SOCK_STREAM, 0);

    ASSERT_IS_TRUE(probe != TEST_INVALID_SOCKET, "could not create the probe socket");

    (void)memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;

    ASSERT_ARE_EQUAL(int, 0, bind(probe, (struct sockaddr*)&address, sizeof(address)), "could not bind the probe socket");
    ASSERT_ARE_EQUAL(int, 0, getsockname(probe, (struct sockaddr*)&address, &address_length), "could not read the probe socket port");

    result = (int)ntohs(address.sin_port);
    test_close_socket(probe);

    return result;
}

static TEST_SOCKET start_listener(int port)
{
    struct sockaddr_in address;
    int reuse = 1;
    TEST_SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);

    ASSERT_IS_TRUE(listener != TEST_INVALID_SOCKET, "could not create the listener socket");

    (void)setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    (void)memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons((unsigned short)port);

    ASSERT_ARE_EQUAL(int, 0, bind(listener, (struct sockaddr*)&address, sizeof(address)), "could not bind the listener socket");
    ASSERT_ARE_EQUAL(int, 0, listen(listener, 4), "could not listen on the listener socket");

    return listener;
}

// Opens the io and drives it to a decision. Returns true only when the open completed
// successfully, which covers both the synchronous adapters and the ones that finish the
// name resolution in xio_dowork.
static bool open_completes_successfully(XIO_HANDLE io)
{
    bool result;

    g_open_completed = false;
    g_open_result = IO_OPEN_ERROR;

    if (xio_open(io, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL) != 0)
    {
        result = false;
    }
    else
    {
        size_t i;

        for (i = 0; (i < OPEN_POLL_MAX_ITERATIONS) && !g_open_completed; i++)
        {
            xio_dowork(io);

            if (!g_open_completed)
            {
                ThreadAPI_Sleep(OPEN_POLL_INTERVAL_MS);
            }
        }

        ASSERT_IS_TRUE(g_open_completed, "the open never completed");

        result = (g_open_result == IO_OPEN_OK);
    }

    return result;
}

BEGIN_TEST_SUITE(socketio_int_tests)

TEST_SUITE_INITIALIZE(suite_init)
{
    ASSERT_ARE_EQUAL(int, 0, platform_init(), "platform_init failed");
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    platform_deinit();
}

TEST_FUNCTION_INITIALIZE(init)
{
    g_io = NULL;
    g_listener = TEST_INVALID_SOCKET;
    g_open_completed = false;
    g_open_result = IO_OPEN_ERROR;
}

TEST_FUNCTION_CLEANUP(cleanup)
{
    if (g_io != NULL)
    {
        (void)xio_close(g_io, NULL, NULL);
        xio_destroy(g_io);
        g_io = NULL;
    }

    if (g_listener != TEST_INVALID_SOCKET)
    {
        test_close_socket(g_listener);
        g_listener = TEST_INVALID_SOCKET;
    }
}

// A connect failure must leave the io reusable: opening the same handle again once the peer
// is reachable has to succeed, without the caller having to close or recreate the io first.
TEST_FUNCTION(socketio_open_succeeds_when_retried_after_a_connect_failure)
{
    ///arrange
    SOCKETIO_CONFIG config;
    int port = reserve_unused_port();

    config.hostname = TEST_HOSTNAME;
    config.port = port;
    config.accepted_socket = NULL;

    g_io = xio_create(socketio_get_interface_description(), &config);
    ASSERT_IS_NOT_NULL(g_io);

    ASSERT_IS_FALSE(open_completes_successfully(g_io), "the open must fail while nothing is listening");

    g_listener = start_listener(port);

    ///act
    ///assert
    ASSERT_IS_TRUE(open_completes_successfully(g_io), "the open must succeed once the peer is reachable");
}

// Same scenario, but with an explicit close between the failed open and the retry.
TEST_FUNCTION(socketio_open_succeeds_when_retried_after_a_connect_failure_and_a_close)
{
    ///arrange
    SOCKETIO_CONFIG config;
    int port = reserve_unused_port();

    config.hostname = TEST_HOSTNAME;
    config.port = port;
    config.accepted_socket = NULL;

    g_io = xio_create(socketio_get_interface_description(), &config);
    ASSERT_IS_NOT_NULL(g_io);

    ASSERT_IS_FALSE(open_completes_successfully(g_io), "the open must fail while nothing is listening");
    ASSERT_ARE_EQUAL(int, 0, xio_close(g_io, NULL, NULL), "xio_close failed after the failed open");

    g_listener = start_listener(port);

    ///act
    ///assert
    ASSERT_IS_TRUE(open_completes_successfully(g_io), "the open must succeed once the peer is reachable");
}

// An open/close/open cycle on a reachable peer must keep working, i.e. closing must not
// leave behind state that prevents the next open.
TEST_FUNCTION(socketio_open_succeeds_after_a_successful_open_and_close)
{
    ///arrange
    SOCKETIO_CONFIG config;
    int port = reserve_unused_port();

    g_listener = start_listener(port);

    config.hostname = TEST_HOSTNAME;
    config.port = port;
    config.accepted_socket = NULL;

    g_io = xio_create(socketio_get_interface_description(), &config);
    ASSERT_IS_NOT_NULL(g_io);

    ASSERT_IS_TRUE(open_completes_successfully(g_io), "the first open must succeed");
    ASSERT_ARE_EQUAL(int, 0, xio_close(g_io, NULL, NULL), "xio_close failed");

    ///act
    ///assert
    ASSERT_IS_TRUE(open_completes_successfully(g_io), "the open must succeed after a close");
}

END_TEST_SUITE(socketio_int_tests)
