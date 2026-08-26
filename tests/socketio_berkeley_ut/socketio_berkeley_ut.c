// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef __APPLE__
#include <net/if.h>
#include <sys/ioctl.h>
#endif

#include "testrunnerswitcher.h"

static void* real_malloc(size_t size)
{
    return malloc(size);
}

static void* real_calloc(size_t count, size_t size)
{
    return calloc(count, size);
}

static void real_free(void* pointer)
{
    free(pointer);
}

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/optionhandler.h"
#include "azure_c_shared_utility/dns_resolver.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/shared_util_options.h"

#define TEST_HOSTNAME "hostname"
#define TEST_PORT 23456
#define TEST_SOCKET_FIRST 42
#define TEST_MAC_ADDRESS "AA:BB:CC:DD:EE:FF"

static TEST_MUTEX_HANDLE g_testByTest;

static int g_resolver_call_count;
static int g_resolver_failures;
static int g_resolver_failure_code;
static int g_resolver_pending;
static int g_socket_call_count;
static int g_socket_failures;
static int g_close_call_count;
static int g_shutdown_call_count;
static int g_fcntl_call_count;
static int g_fcntl_failures;
static int g_connect_call_count;
static int g_connect_failures;
static int g_connect_einprogress;
static int g_select_call_count;
static int g_select_failures;
static int g_getsockopt_call_count;
static int g_getsockopt_failures;
static int g_ioctl_call_count;
static int g_ioctl_failures;
static int g_next_socket;
static int g_open_callback_count;
static IO_OPEN_RESULT g_last_open_result;
static int g_error_callback_count;
static bool g_error_callback_saw_invalid_socket;

static struct sockaddr_in g_test_sockaddr;
static struct addrinfo g_test_addrinfo;

static void reset_fake_network(void)
{
    (void)memset(&g_test_sockaddr, 0, sizeof(g_test_sockaddr));
    g_test_sockaddr.sin_family = AF_INET;
    g_test_sockaddr.sin_port = htons(TEST_PORT);
    g_test_sockaddr.sin_addr.s_addr = 0x0100007FU;

    (void)memset(&g_test_addrinfo, 0, sizeof(g_test_addrinfo));
    g_test_addrinfo.ai_family = AF_INET;
    g_test_addrinfo.ai_socktype = SOCK_STREAM;
    g_test_addrinfo.ai_protocol = IPPROTO_TCP;
    g_test_addrinfo.ai_addrlen = sizeof(g_test_sockaddr);
    g_test_addrinfo.ai_addr = (struct sockaddr*)&g_test_sockaddr;

    g_resolver_call_count = 0;
    g_resolver_failures = 0;
    g_resolver_failure_code = EAI_AGAIN;
    g_resolver_pending = 0;
    g_socket_call_count = 0;
    g_socket_failures = 0;
    g_close_call_count = 0;
    g_shutdown_call_count = 0;
    g_fcntl_call_count = 0;
    g_fcntl_failures = 0;
    g_connect_call_count = 0;
    g_connect_failures = 0;
    g_connect_einprogress = 0;
    g_select_call_count = 0;
    g_select_failures = 0;
    g_getsockopt_call_count = 0;
    g_getsockopt_failures = 0;
    g_ioctl_call_count = 0;
    g_ioctl_failures = 0;
    g_next_socket = TEST_SOCKET_FIRST;
    g_open_callback_count = 0;
    g_last_open_result = IO_OPEN_ERROR;
    g_error_callback_count = 0;
    g_error_callback_saw_invalid_socket = false;
}

static DNSRESOLVER_HANDLE fake_dns_resolver_create(const char* hostname, int port, const DNSRESOLVER_OPTIONS* options)
{
    static int resolver_state;

    (void)hostname;
    (void)port;
    (void)options;
    g_resolver_call_count++;
    return &resolver_state;
}

static bool fake_dns_resolver_is_lookup_complete(DNSRESOLVER_HANDLE resolver)
{
    (void)resolver;
    if (g_resolver_pending > 0)
    {
        g_resolver_pending--;
        return false;
    }

    return true;
}

static uint32_t fake_dns_resolver_get_ipv4(DNSRESOLVER_HANDLE resolver)
{
    (void)resolver;
    return g_test_sockaddr.sin_addr.s_addr;
}

static struct addrinfo* fake_dns_resolver_get_addr_info(DNSRESOLVER_HANDLE resolver)
{
    (void)resolver;
    if (g_resolver_failures > 0)
    {
        g_resolver_failures--;
        (void)g_resolver_failure_code;
        return NULL;
    }
    return &g_test_addrinfo;
}

static void fake_dns_resolver_destroy(DNSRESOLVER_HANDLE resolver)
{
    (void)resolver;
}

int socket(int domain, int type, int protocol)
{
    (void)domain;
    (void)type;
    (void)protocol;
    g_socket_call_count++;
    if (g_socket_failures > 0)
    {
        g_socket_failures--;
        return -1;
    }
    return g_next_socket++;
}

int close(int descriptor)
{
    (void)descriptor;
    g_close_call_count++;
    return 0;
}

int shutdown(int descriptor, int how)
{
    (void)descriptor;
    (void)how;
    g_shutdown_call_count++;
    return 0;
}

int fcntl(int descriptor, int command, ...)
{
    (void)descriptor;
    (void)command;
    g_fcntl_call_count++;
    if (g_fcntl_failures > 0)
    {
        g_fcntl_failures--;
        return -1;
    }
    return 0;
}

int connect(int descriptor, const struct sockaddr* address, socklen_t address_length)
{
    (void)descriptor;
    (void)address;
    (void)address_length;
    g_connect_call_count++;
    if (g_connect_einprogress > 0)
    {
        g_connect_einprogress--;
        errno = EINPROGRESS;
        return -1;
    }
    if (g_connect_failures > 0)
    {
        g_connect_failures--;
        errno = ECONNREFUSED;
        return -1;
    }
    return 0;
}

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
{
    (void)nfds;
    (void)readfds;
    (void)writefds;
    (void)exceptfds;
    (void)timeout;
    g_select_call_count++;
    if (g_select_failures > 0)
    {
        g_select_failures--;
        errno = EIO;
        return -1;
    }
    return 1;
}

int getsockopt(int descriptor, int level, int option, void* value, socklen_t* value_length)
{
    (void)descriptor;
    (void)level;
    (void)option;
    g_getsockopt_call_count++;
    if (g_getsockopt_failures > 0)
    {
        g_getsockopt_failures--;
        errno = EIO;
        return -1;
    }
    *(int*)value = 0;
    *value_length = sizeof(int);
    return 0;
}

int setsockopt(int descriptor, int level, int option, const void* value, socklen_t value_length)
{
    (void)descriptor;
    (void)level;
    (void)option;
    (void)value;
    (void)value_length;
    return 0;
}

ssize_t send(int descriptor, const void* buffer, size_t length, int flags)
{
    (void)descriptor;
    (void)buffer;
    (void)flags;
    return (ssize_t)length;
}

ssize_t recv(int descriptor, void* buffer, size_t length, int flags)
{
    (void)descriptor;
    (void)buffer;
    (void)length;
    (void)flags;
    errno = EAGAIN;
    return -1;
}

#ifndef __APPLE__
int ioctl(int descriptor, unsigned long request, ...)
{
    va_list arguments;
    void* argument;

    (void)descriptor;
    g_ioctl_call_count++;
    va_start(arguments, request);
    argument = va_arg(arguments, void*);
    va_end(arguments);

    if (g_ioctl_failures > 0)
    {
        g_ioctl_failures--;
        errno = EIO;
        return -1;
    }

    if (request == SIOCGIFCONF)
    {
        struct ifconf* interface_configuration = (struct ifconf*)argument;
        struct ifreq* interface_request = (struct ifreq*)interface_configuration->ifc_buf;
        (void)memset(interface_request, 0, sizeof(*interface_request));
        (void)strcpy(interface_request->ifr_name, "eth0");
        interface_configuration->ifc_len = sizeof(*interface_request);
    }
    else
    {
        struct ifreq* interface_request = (struct ifreq*)argument;
        (void)strcpy(interface_request->ifr_name, "eth0");
        if (request == SIOCGIFHWADDR)
        {
            const unsigned char mac_address[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
            (void)memcpy(interface_request->ifr_hwaddr.sa_data, mac_address, sizeof(mac_address));
        }
        else if (request == SIOCGIFADDR)
        {
            ((struct sockaddr_in*)&interface_request->ifr_addr)->sin_addr.s_addr = 0x0100007FU;
        }
    }
    return 0;
}
#endif

static void on_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context;
    g_open_callback_count++;
    g_last_open_result = open_result;
}

static void on_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    (void)context;
    (void)buffer;
    (void)size;
}

static void on_io_error(void* context)
{
    g_error_callback_count++;
    if (context != NULL)
    {
        g_error_callback_saw_invalid_socket = (*(int*)context == -1);
    }
}

static CONCRETE_IO_HANDLE create_socketio(void)
{
    SOCKETIO_CONFIG config = { TEST_HOSTNAME, TEST_PORT, NULL };
    CONCRETE_IO_HANDLE result = socketio_create(&config);
    ASSERT_IS_NOT_NULL(result);
    return result;
}

static int open_socketio(CONCRETE_IO_HANDLE socket_io, void* error_context)
{
    return socketio_open(socket_io, on_open_complete, NULL, on_bytes_received, NULL, on_io_error, error_context);
}

BEGIN_TEST_SUITE(socketio_berkeley_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    (void)umock_c_init(NULL);
    REGISTER_UMOCK_ALIAS_TYPE(SINGLYLINKEDLIST_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(LIST_ITEM_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(DNSRESOLVER_HANDLE, void*);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, real_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, real_calloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, real_free);
    REGISTER_GLOBAL_MOCK_HOOK(dns_resolver_create, fake_dns_resolver_create);
    REGISTER_GLOBAL_MOCK_HOOK(dns_resolver_is_lookup_complete, fake_dns_resolver_is_lookup_complete);
    REGISTER_GLOBAL_MOCK_HOOK(dns_resolver_get_ipv4, fake_dns_resolver_get_ipv4);
    REGISTER_GLOBAL_MOCK_HOOK(dns_resolver_get_addrInfo, fake_dns_resolver_get_addr_info);
    REGISTER_GLOBAL_MOCK_HOOK(dns_resolver_destroy, fake_dns_resolver_destroy);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_create, (SINGLYLINKEDLIST_HANDLE)0x4242);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_get_head_item, NULL);
    REGISTER_GLOBAL_MOCK_RETURN(singlylinkedlist_remove, 0);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_Create, (OPTIONHANDLER_HANDLE)0x4243);
    REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_AddOption, OPTIONHANDLER_OK);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    TEST_MUTEX_DESTROY(g_testByTest);
    umock_c_deinit();
}

TEST_FUNCTION_INITIALIZE(test_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }
    reset_fake_network();
}

TEST_FUNCTION_CLEANUP(test_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

TEST_FUNCTION(socketio_open_dns_failure_is_retryable)
{
    CONCRETE_IO_HANDLE socket_io = create_socketio();
    int result;

    g_resolver_failures = 1;
    g_resolver_failure_code = EAI_AGAIN;

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, (int)IO_OPEN_ERROR, (int)g_last_open_result);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_NOT_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 2, g_resolver_call_count);
    ASSERT_ARE_EQUAL(int, 1, g_socket_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_fcntl_call_count);
    ASSERT_ARE_EQUAL(int, 1, g_connect_call_count);
    ASSERT_ARE_EQUAL(int, 1, g_select_call_count);
    ASSERT_ARE_EQUAL(int, 1, g_getsockopt_call_count);

    socketio_destroy(socket_io);
}

TEST_FUNCTION(socketio_open_socket_failure_is_retryable)
{
    CONCRETE_IO_HANDLE socket_io = create_socketio();
    int result;

    g_socket_failures = 1;

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_NOT_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 2, g_socket_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_fcntl_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_connect_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_select_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_getsockopt_call_count);

    socketio_destroy(socket_io);
}

#ifndef __APPLE__
TEST_FUNCTION(socketio_open_interface_setup_failure_is_retryable)
{
    CONCRETE_IO_HANDLE socket_io = create_socketio();
    int result;

    result = socketio_setoption(socket_io, OPTION_NET_INT_MAC_ADDRESS, TEST_MAC_ADDRESS);
    ASSERT_ARE_EQUAL(int, 0, result);
    g_ioctl_failures = 1;

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 1, g_close_call_count);

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_NOT_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 2, g_socket_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_connect_call_count);

    socketio_destroy(socket_io);
}
#endif

TEST_FUNCTION(socketio_open_fcntl_failure_is_retryable)
{
    CONCRETE_IO_HANDLE socket_io = create_socketio();
    int result;

    g_fcntl_failures = 1;

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 1, g_close_call_count);

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_NOT_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 2, g_socket_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_connect_call_count);

    socketio_destroy(socket_io);
}

TEST_FUNCTION(socketio_open_connect_failure_is_retryable_and_refreshes_resolver)
{
    CONCRETE_IO_HANDLE socket_io = create_socketio();
    int result;

    g_connect_failures = 1;

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 1, g_close_call_count);

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_NOT_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 2, g_resolver_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_socket_call_count);

    socketio_destroy(socket_io);
}

TEST_FUNCTION(socketio_open_select_failure_is_retryable)
{
    CONCRETE_IO_HANDLE socket_io = create_socketio();
    int result;

    g_connect_einprogress = 1;
    g_select_failures = 1;

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 1, g_close_call_count);

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_NOT_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 2, g_socket_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_select_call_count);
    ASSERT_ARE_EQUAL(int, 1, g_getsockopt_call_count);

    socketio_destroy(socket_io);
}

TEST_FUNCTION(socketio_open_getsockopt_failure_is_retryable)
{
    CONCRETE_IO_HANDLE socket_io = create_socketio();
    int result;

    g_connect_einprogress = 1;
    g_getsockopt_failures = 1;

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 1, g_close_call_count);

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_NOT_EQUAL(int, -1, *(int*)socket_io);
    ASSERT_ARE_EQUAL(int, 2, g_socket_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_getsockopt_call_count);

    socketio_destroy(socket_io);
}

TEST_FUNCTION(socketio_dowork_opening_failure_is_retryable)
{
    CONCRETE_IO_HANDLE socket_io = create_socketio();
    int result;

    g_resolver_pending = 1;
    g_socket_failures = 1;

    result = open_socketio(socket_io, socket_io);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);

    socketio_dowork(socket_io);
    ASSERT_ARE_EQUAL(int, 1, g_error_callback_count);
    ASSERT_IS_TRUE(g_error_callback_saw_invalid_socket);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_NOT_EQUAL(int, -1, *(int*)socket_io);

    socketio_destroy(socket_io);
}

TEST_FUNCTION(socketio_close_recreates_resolver)
{
    CONCRETE_IO_HANDLE socket_io = create_socketio();
    int result;

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 1, g_resolver_call_count);
    ASSERT_ARE_EQUAL(int, 1, g_socket_call_count);

    result = socketio_close(socket_io, NULL, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, -1, *(int*)socket_io);

    result = open_socketio(socket_io, NULL);
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(int, 2, g_resolver_call_count);
    ASSERT_ARE_EQUAL(int, 2, g_socket_call_count);

    socketio_destroy(socket_io);
}

END_TEST_SUITE(socketio_berkeley_unittests)
