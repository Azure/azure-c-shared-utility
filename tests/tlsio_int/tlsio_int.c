// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Integration tests for the teardown contract of whichever tlsio adapter the platform
// provides. They drive the real adapter against a loopback port that the test itself owns
// and that nothing is listening on, so the connection never becomes a usable TLS session.
// No external service, no name server and no certificates are required.
//
// The subject is the teardown path, not the handshake. An adapter has to release every
// resource when it is destroyed, including when it is destroyed while still open, still
// connecting, or sitting in an error state after the connection failed. SRS_TLSIO_30_022
// makes destroying a tlsio that is not closed a supported call, so a caller doing exactly
// that must not leak the connection.
//
// The adapters disagree about when they notice an unusable connection - some report the
// open as successful and only fail later - so these tests deliberately assert nothing about
// which open result arrives, only about what teardown leaves behind.

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
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
typedef int TEST_SOCKET;
#define TEST_INVALID_SOCKET     (-1)
#define test_close_socket(s)    (void)close(s)
typedef socklen_t test_socklen_t;
// The POSIX platforms can count their own descriptors cheaply and portably, which is what
// turns "the adapter leaked the connection" into an assertable fact.
#define TEST_CAN_COUNT_DESCRIPTORS
#endif

#include "testrunnerswitcher.h"

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xio.h"

// Nothing accepts on the target port, so every adapter settles quickly. This cap only
// exists so that an adapter that never settles fails the test instead of hanging the run.
#define OPEN_POLL_INTERVAL_MS       10
#define OPEN_POLL_MAX_ITERATIONS    300

// Enough passes for an adapter to get a socket up and start connecting, but short of
// letting the open settle, so the io is torn down while it is still in flight.
#define PARTIAL_OPEN_DOWORK_PASSES  3

// A leak of one descriptor per cycle shows up well inside this count, while the whole test
// still runs quickly.
#define TEARDOWN_CYCLES             20

#define TEST_HOSTNAME               "127.0.0.1"

static XIO_HANDLE g_io;
static TEST_SOCKET g_reserved = TEST_INVALID_SOCKET;

static bool g_open_completed;
static IO_OPEN_RESULT g_open_result;
static bool g_send_completed;
static IO_SEND_RESULT g_send_result;

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

static void on_send_complete(void* context, IO_SEND_RESULT send_result)
{
    (void)context;
    g_send_completed = true;
    g_send_result = send_result;
}

// Binds an ephemeral loopback port and keeps the socket bound without listening on it, so
// the port stays owned by this test for its whole lifetime while connections to it are
// refused promptly. That keeps every cycle below fast and free of timeouts.
static TEST_SOCKET reserve_port(int* port)
{
    struct sockaddr_in address;
    test_socklen_t address_length = sizeof(address);
    TEST_SOCKET reserved = socket(AF_INET, SOCK_STREAM, 0);

    ASSERT_IS_TRUE(reserved != TEST_INVALID_SOCKET, "could not create the reservation socket");

    (void)memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;

    ASSERT_ARE_EQUAL(int, 0, bind(reserved, (struct sockaddr*)&address, sizeof(address)), "could not bind the reservation socket");
    ASSERT_ARE_EQUAL(int, 0, getsockname(reserved, (struct sockaddr*)&address, &address_length), "could not read the reserved port");

    *port = (int)ntohs(address.sin_port);

    return reserved;
}

static XIO_HANDLE create_tlsio(int port)
{
    TLSIO_CONFIG config;
    const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();

    ASSERT_IS_NOT_NULL(tlsio_interface, "the platform provides no tlsio");

    (void)memset(&config, 0, sizeof(config));
    config.hostname = TEST_HOSTNAME;
    config.port = port;

    return xio_create(tlsio_interface, &config);
}

// Counts the descriptors this process holds. Comparing this across identical create/destroy
// cycles is what catches a connection that was released without being closed.
#ifdef TEST_CAN_COUNT_DESCRIPTORS
static size_t count_open_descriptors(void)
{
    size_t result = 0;
    int fd;

    for (fd = 0; fd < 1024; fd++)
    {
        if (fcntl(fd, F_GETFD) != -1)
        {
            result++;
        }
    }

    return result;
}
#endif

// Opens the io and drives it until it reports a result, whichever result that is.
static void drive_open_to_completion(XIO_HANDLE io)
{
    g_open_completed = false;
    g_open_result = IO_OPEN_ERROR;

    if (xio_open(io, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL) == 0)
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
    }
}

BEGIN_TEST_SUITE(tlsio_int_tests)

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
    g_reserved = TEST_INVALID_SOCKET;
    g_open_completed = false;
    g_open_result = IO_OPEN_ERROR;
    g_send_completed = false;
    g_send_result = IO_SEND_ERROR;
}

TEST_FUNCTION_CLEANUP(cleanup)
{
    if (g_io != NULL)
    {
        xio_destroy(g_io);
        g_io = NULL;
    }

    if (g_reserved != TEST_INVALID_SOCKET)
    {
        test_close_socket(g_reserved);
        g_reserved = TEST_INVALID_SOCKET;
    }
}

// Destroying an io that was never opened must be clean, and is the baseline the leak checks
// below are measured against.
TEST_FUNCTION(tlsio_destroy_without_open_releases_everything)
{
    ///arrange
    int port;

    g_reserved = reserve_port(&port);

    ///act
    g_io = create_tlsio(port);
    ASSERT_IS_NOT_NULL(g_io);

    xio_destroy(g_io);
    g_io = NULL;

    ///assert
    // Reaching here without a crash is the assertion.
}

// The reported case: the caller destroys the io straight after the connection turned out to
// be unusable, without closing it first. The adapter may still be holding the connection at
// that point and has to close it rather than just letting go of it.
TEST_FUNCTION(tlsio_destroy_after_a_settled_open_does_not_leak_the_connection)
{
    ///arrange
    int port;
    size_t i;
#ifdef TEST_CAN_COUNT_DESCRIPTORS
    size_t baseline;
    size_t after;
#endif

    g_reserved = reserve_port(&port);

    // One warm-up cycle first, so that any one-off allocation the adapter makes on its very
    // first use is already accounted for in the baseline.
    g_io = create_tlsio(port);
    ASSERT_IS_NOT_NULL(g_io);
    drive_open_to_completion(g_io);
    xio_destroy(g_io);
    g_io = NULL;

#ifdef TEST_CAN_COUNT_DESCRIPTORS
    baseline = count_open_descriptors();
#endif

    ///act
    for (i = 0; i < TEARDOWN_CYCLES; i++)
    {
        g_io = create_tlsio(port);
        ASSERT_IS_NOT_NULL(g_io);

        drive_open_to_completion(g_io);

        // No xio_close on purpose. This is the call pattern the adapter has to survive.
        xio_destroy(g_io);
        g_io = NULL;
    }

    ///assert
#ifdef TEST_CAN_COUNT_DESCRIPTORS
    after = count_open_descriptors();
    ASSERT_IS_TRUE(after <= baseline, "destroying the io without closing it leaked a descriptor per cycle");
#endif
}

// Same requirement, but torn down while the connection is still being established, which
// reaches a different branch of the adapter's teardown than a settled open does.
TEST_FUNCTION(tlsio_destroy_while_opening_does_not_leak_the_connection)
{
    ///arrange
    int port;
    size_t i;
    size_t pass;
#ifdef TEST_CAN_COUNT_DESCRIPTORS
    size_t baseline;
    size_t after;
#endif

    g_reserved = reserve_port(&port);

    g_io = create_tlsio(port);
    ASSERT_IS_NOT_NULL(g_io);
    (void)xio_open(g_io, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);
    for (pass = 0; pass < PARTIAL_OPEN_DOWORK_PASSES; pass++)
    {
        xio_dowork(g_io);
    }
    xio_destroy(g_io);
    g_io = NULL;

#ifdef TEST_CAN_COUNT_DESCRIPTORS
    baseline = count_open_descriptors();
#endif

    ///act
    for (i = 0; i < TEARDOWN_CYCLES; i++)
    {
        g_io = create_tlsio(port);
        ASSERT_IS_NOT_NULL(g_io);

        g_open_completed = false;
        (void)xio_open(g_io, on_io_open_complete, NULL, on_bytes_received, NULL, on_io_error, NULL);

        for (pass = 0; (pass < PARTIAL_OPEN_DOWORK_PASSES) && !g_open_completed; pass++)
        {
            xio_dowork(g_io);
        }

        // Again no xio_close, and this time the open has not been given time to settle.
        xio_destroy(g_io);
        g_io = NULL;
    }

    ///assert
#ifdef TEST_CAN_COUNT_DESCRIPTORS
    after = count_open_descriptors();
    ASSERT_IS_TRUE(after <= baseline, "destroying the io while it was opening leaked a descriptor per cycle");
#endif
}

// Closing explicitly and then destroying is the well-behaved caller's sequence. It has to
// stay clean too, and in particular the close must not leave anything for the destroy to
// double-close.
TEST_FUNCTION(tlsio_close_then_destroy_does_not_leak_the_connection)
{
    ///arrange
    int port;
    size_t i;
#ifdef TEST_CAN_COUNT_DESCRIPTORS
    size_t baseline;
    size_t after;
#endif

    g_reserved = reserve_port(&port);

    g_io = create_tlsio(port);
    ASSERT_IS_NOT_NULL(g_io);
    drive_open_to_completion(g_io);
    (void)xio_close(g_io, NULL, NULL);
    xio_destroy(g_io);
    g_io = NULL;

#ifdef TEST_CAN_COUNT_DESCRIPTORS
    baseline = count_open_descriptors();
#endif

    ///act
    for (i = 0; i < TEARDOWN_CYCLES; i++)
    {
        g_io = create_tlsio(port);
        ASSERT_IS_NOT_NULL(g_io);

        drive_open_to_completion(g_io);

        (void)xio_close(g_io, NULL, NULL);
        xio_destroy(g_io);
        g_io = NULL;
    }

    ///assert
#ifdef TEST_CAN_COUNT_DESCRIPTORS
    after = count_open_descriptors();
    ASSERT_IS_TRUE(after <= baseline, "closing and destroying the io leaked a descriptor per cycle");
#endif
}

// A message queued on an io that is then destroyed must not be abandoned silently: the
// adapter owns the caller's completion callback and has to invoke it rather than free the
// message underneath it. Only the adapters that report the open as successful get this far;
// the ones that reject the connection up front have nothing to queue and are skipped.
TEST_FUNCTION(tlsio_destroy_completes_a_queued_message)
{
    ///arrange
    int port;
    static const unsigned char message[] = "GET / HTTP/1.1\r\n\r\n";

    g_reserved = reserve_port(&port);

    g_io = create_tlsio(port);
    ASSERT_IS_NOT_NULL(g_io);

    drive_open_to_completion(g_io);

    if (g_open_result != IO_OPEN_OK)
    {
        xio_destroy(g_io);
        g_io = NULL;
    }
    else
    {
        g_send_completed = false;

        ///act
        if (xio_send(g_io, message, sizeof(message) - 1, on_send_complete, NULL) != 0)
        {
            // The adapter refused the message outright, so it never took ownership of the
            // callback and owes nothing.
            xio_destroy(g_io);
            g_io = NULL;
        }
        else
        {
            xio_destroy(g_io);
            g_io = NULL;

            ///assert
            ASSERT_IS_TRUE(g_send_completed, "destroy abandoned a queued message without completing it");
        }
    }
}

END_TEST_SUITE(tlsio_int_tests)
