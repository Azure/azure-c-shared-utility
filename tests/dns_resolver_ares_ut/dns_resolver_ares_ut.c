// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#endif

#include "macro_utils/macro_utils.h"
#include "c_logging/logger.h"

#include "dns_resolver.h"

// Allocation counters prove that a failed create releases everything it took,
// which no expected-call sequence can show on its own.
static int g_live_allocations;

void* my_gballoc_malloc(size_t size)
{
    void* result = malloc(size);
    if (result != NULL)
    {
        g_live_allocations++;
    }
    return result;
}

void* my_gballoc_calloc(size_t nmemb, size_t size)
{
    void* result = calloc(nmemb, size);
    if (result != NULL)
    {
        g_live_allocations++;
    }
    return result;
}

void* my_gballoc_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void my_gballoc_free(void* ptr)
{
    if (ptr != NULL)
    {
        g_live_allocations--;
    }
    free(ptr);
}

#define ENABLE_MOCKS

#include "socket_async_os.h"
#include "azure_c_shared_utility/gballoc.h"
#include "ares.h"

#ifdef __cplusplus
extern "C" {
#endif

MOCKABLE_FUNCTION(, int, ares_library_init, int, flags);
MOCKABLE_FUNCTION(, int, ares_init, ares_channel*, channelptr);
MOCKABLE_FUNCTION(, void, ares_gethostbyname, ares_channel, channel, const char*, name, int, family, ares_host_callback, callback, void*, arg);
MOCKABLE_FUNCTION(, int, ares_getsock, ares_channel, channel, ares_socket_t*, socks, int, numsocks);
MOCKABLE_FUNCTION(, void, ares_process_fd, ares_channel, channel, ares_socket_t, read_fd, ares_socket_t, write_fd);

#ifdef __cplusplus
}
#endif

#undef ENABLE_MOCKS

// Not mocked: nothing is asserted on them and they must not consume expected calls.
void ares_destroy(ares_channel channel)
{
    (void)channel;
}

void ares_library_cleanup(void)
{
}

const char* ares_strerror(int code)
{
    (void)code;
    return "test error";
}

#define TEST_HOSTNAME               "fake.com"
#define TEST_PORT                   443
#define TEST_IPV4_ADDRESS           0x0102030AU
#define TEST_SOCKET_0               ((ares_socket_t)11)
#define TEST_SOCKET_2               ((ares_socket_t)13)
// What ares_getsock leaves in the caller's buffer for entries its bitmask does not report.
#define TEST_STALE_SOCKET           ((ares_socket_t)42)
#define MAX_RECORDED_PROCESS_FD     8

static ares_host_callback g_query_callback;
static void* g_query_arg;

typedef struct PROCESS_FD_CALL_TAG
{
    ares_socket_t read_fd;
    ares_socket_t write_fd;
} PROCESS_FD_CALL;

static PROCESS_FD_CALL g_process_fd_calls[MAX_RECORDED_PROCESS_FD];
static int g_process_fd_call_count;

// What the mocked ares_getsock reports on the next call.
static int g_getsock_bitmask;

static void my_ares_gethostbyname(ares_channel channel, const char* name, int family, ares_host_callback callback, void* arg)
{
    (void)channel;
    (void)name;
    (void)family;
    g_query_callback = callback;
    g_query_arg = arg;
}

static int my_ares_getsock(ares_channel channel, ares_socket_t* socks, int numsocks)
{
    int i;
    (void)channel;

    // c-ares only writes the entries its return value reports, so the rest of the
    // caller's buffer keeps whatever it already held.
    for (i = 0; i < numsocks; i++)
    {
        socks[i] = TEST_STALE_SOCKET;
    }

    if (numsocks > 0)
    {
        socks[0] = TEST_SOCKET_0;
    }
    if (numsocks > 2)
    {
        socks[2] = TEST_SOCKET_2;
    }

    return g_getsock_bitmask;
}

static void my_ares_process_fd(ares_channel channel, ares_socket_t read_fd, ares_socket_t write_fd)
{
    (void)channel;
    if (g_process_fd_call_count < MAX_RECORDED_PROCESS_FD)
    {
        g_process_fd_calls[g_process_fd_call_count].read_fd = read_fd;
        g_process_fd_calls[g_process_fd_call_count].write_fd = write_fd;
    }
    g_process_fd_call_count++;
}

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_stdint.h"

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static TEST_MUTEX_HANDLE g_testByTest;

static char g_ipv4_bytes[4];
static char* g_ipv4_addr_list[2];
#ifdef IPV6_ENABLED
static char g_ipv6_bytes[16];
static char* g_ipv6_addr_list[2];
#endif // IPV6_ENABLED

static struct hostent g_hostent;

static struct hostent* make_ipv4_hostent(uint32_t address)
{
    memcpy(g_ipv4_bytes, &address, sizeof(g_ipv4_bytes));
    g_ipv4_addr_list[0] = g_ipv4_bytes;
    g_ipv4_addr_list[1] = NULL;
    memset(&g_hostent, 0, sizeof(g_hostent));
    g_hostent.h_addrtype = AF_INET;
    g_hostent.h_length = (int)sizeof(struct in_addr);
    g_hostent.h_addr_list = g_ipv4_addr_list;
    return &g_hostent;
}

#ifdef IPV6_ENABLED
static struct hostent* make_ipv6_hostent(const uint8_t* address)
{
    memcpy(g_ipv6_bytes, address, sizeof(g_ipv6_bytes));
    g_ipv6_addr_list[0] = g_ipv6_bytes;
    g_ipv6_addr_list[1] = NULL;
    memset(&g_hostent, 0, sizeof(g_hostent));
    g_hostent.h_addrtype = AF_INET6;
    g_hostent.h_length = (int)sizeof(struct in6_addr);
    g_hostent.h_addr_list = g_ipv6_addr_list;
    return &g_hostent;
}
#endif // IPV6_ENABLED

// Starts a lookup and leaves it outstanding, which is the state every answer test needs.
static DNSRESOLVER_HANDLE start_lookup(void)
{
    DNSRESOLVER_HANDLE dns = dns_resolver_create(TEST_HOSTNAME, TEST_PORT, NULL);
    ASSERT_IS_NOT_NULL(dns);
    ASSERT_IS_FALSE(dns_resolver_is_lookup_complete(dns), "lookup completed before any answer arrived");
    ASSERT_IS_TRUE(g_query_callback != NULL, "no query was started");
    return dns;
}

static void deliver_answer(int status, struct hostent* he)
{
    ASSERT_IS_TRUE(g_query_callback != NULL, "no query callback was captured");
    g_query_callback(g_query_arg, status, 0, he);
}

BEGIN_TEST_SUITE(dns_resolver_ares_ut)

    TEST_SUITE_INITIALIZE(suite_init)
    {
        int result;
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);
        result = umocktypes_bool_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);
        result = umocktypes_stdint_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);

        REGISTER_UMOCK_ALIAS_TYPE(ares_channel, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ares_socket_t, int);
        REGISTER_UMOCK_ALIAS_TYPE(ares_host_callback, void*);

        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_calloc, my_gballoc_calloc);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_calloc, NULL);

        REGISTER_GLOBAL_MOCK_RETURNS(ares_library_init, ARES_SUCCESS, ARES_ENOMEM);
        REGISTER_GLOBAL_MOCK_RETURNS(ares_init, ARES_SUCCESS, ARES_ENOMEM);
        REGISTER_GLOBAL_MOCK_HOOK(ares_gethostbyname, my_ares_gethostbyname);
        REGISTER_GLOBAL_MOCK_HOOK(ares_getsock, my_ares_getsock);
        REGISTER_GLOBAL_MOCK_HOOK(ares_process_fd, my_ares_process_fd);
    }

    TEST_SUITE_CLEANUP(suite_cleanup)
    {
        umock_c_deinit();
        TEST_MUTEX_DESTROY(g_testByTest);
    }

    TEST_FUNCTION_INITIALIZE(test_init)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("Could not acquire test serialization mutex.");
        }

        umock_c_reset_all_calls();
        g_query_callback = NULL;
        g_query_arg = NULL;
        g_process_fd_call_count = 0;
        g_getsock_bitmask = 0;
        g_live_allocations = 0;
        memset(g_process_fd_calls, 0, sizeof(g_process_fd_calls));
    }

    TEST_FUNCTION_CLEANUP(test_cleanup)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    /* Tests_SRS_dns_resolver_30_011: [ If the hostname parameter is NULL, dns_resolver_create shall log an error and return NULL. ]*/
    TEST_FUNCTION(dns_resolver_ares__create_null_hostname__fails)
    {
        ///act
        DNSRESOLVER_HANDLE dns = dns_resolver_create(NULL, TEST_PORT, NULL);

        ///assert
        ASSERT_IS_NULL(dns);
    }

    /* Tests_SRS_dns_resolver_30_014: [ On any failure, dns_resolver_create shall log an error and return NULL. ]*/
    TEST_FUNCTION(dns_resolver_ares__create_when_ares_library_init_fails__releases_everything)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns;
        STRICT_EXPECTED_CALL(ares_library_init(IGNORED_ARG)).SetReturn(ARES_ENOMEM);

        ///act
        dns = dns_resolver_create(TEST_HOSTNAME, TEST_PORT, NULL);

        ///assert
        ASSERT_IS_NULL(dns);
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations, "create leaked memory when ares_library_init failed");
    }

    /* Tests_SRS_dns_resolver_30_014: [ On any failure, dns_resolver_create shall log an error and return NULL. ]*/
    TEST_FUNCTION(dns_resolver_ares__create_when_ares_init_fails__releases_everything)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns;
        STRICT_EXPECTED_CALL(ares_init(IGNORED_ARG)).SetReturn(ARES_ENOMEM);

        ///act
        dns = dns_resolver_create(TEST_HOSTNAME, TEST_PORT, NULL);

        ///assert
        ASSERT_IS_NULL(dns);
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations, "create leaked memory when ares_init failed");
    }

    /* Tests_SRS_dns_resolver_30_023: [ If the DNS lookup process is not yet complete, dns_resolver_is_create_complete shall return false. ]*/
    TEST_FUNCTION(dns_resolver_ares__first_poll__starts_the_query_and_reports_incomplete)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = dns_resolver_create(TEST_HOSTNAME, TEST_PORT, NULL);
        bool result;
        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(ares_gethostbyname(IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG, IGNORED_ARG));

        ///act
        result = dns_resolver_is_lookup_complete(dns);

        ///assert
        ASSERT_IS_FALSE(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    /* Tests_SRS_dns_resolver_30_032: [ If dns_resolver_is_create_complete has returned true and the lookup process has succeeded, dns_resolver_get_ipv4 shall return the discovered IPv4 address. ]*/
    TEST_FUNCTION(dns_resolver_ares__ipv4_answer__completes_with_the_address)
    {
        ///arrange
        struct addrinfo* addr;
        DNSRESOLVER_HANDLE dns = start_lookup();

        ///act
        deliver_answer(ARES_SUCCESS, make_ipv4_hostent(TEST_IPV4_ADDRESS));

        ///assert
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        ASSERT_ARE_EQUAL(uint32_t, (uint32_t)TEST_IPV4_ADDRESS, dns_resolver_get_ipv4(dns));
        addr = dns_resolver_get_addrInfo(dns);
        ASSERT_IS_NOT_NULL(addr);
        ASSERT_ARE_EQUAL(int, AF_INET, addr->ai_family);
        ASSERT_ARE_EQUAL(int, SOCK_STREAM, addr->ai_socktype);
        ASSERT_ARE_EQUAL(int, IPPROTO_TCP, addr->ai_protocol);
        ASSERT_ARE_EQUAL(int, (int)sizeof(struct sockaddr_in), (int)addr->ai_addrlen);
        ASSERT_ARE_EQUAL(int, (int)htons(TEST_PORT), (int)((struct sockaddr_in*)addr->ai_addr)->sin_port);

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    /* Tests_SRS_dns_resolver_30_033: [ If dns_resolver_is_create_complete has returned true and the lookup process has failed, dns_resolver_get_ipv4 shall return 0. ]*/
    TEST_FUNCTION(dns_resolver_ares__failed_answer__completes_as_failed)
    {
        // A lookup that never completes leaves the socket IO polling forever, so every
        // non-success status c-ares can report has to end the lookup.
        static const int failure_statuses[] = { ARES_ENOTFOUND, ARES_ENODATA, ARES_ESERVFAIL, ARES_ECONNREFUSED, ARES_ETIMEOUT, ARES_EBADNAME, ARES_ENOMEM, ARES_EREFUSED };
        size_t i;

        for (i = 0; i < sizeof(failure_statuses) / sizeof(failure_statuses[0]); i++)
        {
            ///arrange
            DNSRESOLVER_HANDLE dns = start_lookup();

            ///act
            deliver_answer(failure_statuses[i], NULL);

            ///assert
            ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns), "lookup never completed for status %d", failure_statuses[i]);
            ASSERT_ARE_EQUAL(uint32_t, 0, dns_resolver_get_ipv4(dns));
            ASSERT_IS_NULL(dns_resolver_get_addrInfo(dns));

            ///cleanup
            dns_resolver_destroy(dns);
            ASSERT_ARE_EQUAL(int, 0, g_live_allocations, "memory leaked for status %d", failure_statuses[i]);
        }
    }

    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    TEST_FUNCTION(dns_resolver_ares__answer_with_unexpected_address_family__completes_as_failed)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();
        struct hostent* he = make_ipv4_hostent(TEST_IPV4_ADDRESS);
        he->h_addrtype = AF_UNIX;

        ///act
        deliver_answer(ARES_SUCCESS, he);

        ///assert
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        ASSERT_ARE_EQUAL(uint32_t, 0, dns_resolver_get_ipv4(dns));
        ASSERT_IS_NULL(dns_resolver_get_addrInfo(dns));

        ///cleanup
        dns_resolver_destroy(dns);
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations);
    }

    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    TEST_FUNCTION(dns_resolver_ares__answer_with_unexpected_address_length__completes_as_failed)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();
        struct hostent* he = make_ipv4_hostent(TEST_IPV4_ADDRESS);
        he->h_length = 1;

        ///act
        deliver_answer(ARES_SUCCESS, he);

        ///assert
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        ASSERT_ARE_EQUAL(uint32_t, 0, dns_resolver_get_ipv4(dns));
        ASSERT_IS_NULL(dns_resolver_get_addrInfo(dns));

        ///cleanup
        dns_resolver_destroy(dns);
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations);
    }

    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    TEST_FUNCTION(dns_resolver_ares__success_without_an_address__completes_as_failed)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();
        struct hostent* he = make_ipv4_hostent(TEST_IPV4_ADDRESS);
        he->h_addr_list[0] = NULL;

        ///act
        deliver_answer(ARES_SUCCESS, he);

        ///assert
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        ASSERT_IS_NULL(dns_resolver_get_addrInfo(dns));

        ///cleanup
        dns_resolver_destroy(dns);
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations);
    }

    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    TEST_FUNCTION(dns_resolver_ares__success_with_null_hostent__completes_as_failed)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();

        ///act
        deliver_answer(ARES_SUCCESS, NULL);

        ///assert
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        ASSERT_IS_NULL(dns_resolver_get_addrInfo(dns));

        ///cleanup
        dns_resolver_destroy(dns);
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations);
    }

    /* Tests_SRS_dns_resolver_30_033: [ If dns_resolver_is_create_complete has returned true and the lookup process has failed, dns_resolver_get_ipv4 shall return 0. ]*/
    TEST_FUNCTION(dns_resolver_ares__answer_with_zero_ipv4__completes_as_failed)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();

        ///act
        deliver_answer(ARES_SUCCESS, make_ipv4_hostent(0));

        ///assert
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        ASSERT_ARE_EQUAL(uint32_t, 0, dns_resolver_get_ipv4(dns));
        ASSERT_IS_NULL(dns_resolver_get_addrInfo(dns));

        ///cleanup
        dns_resolver_destroy(dns);
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations);
    }

#ifdef IPV6_ENABLED
    /* Tests_SRS_dns_resolver_30_022: [ If the DNS lookup process has completed, dns_resolver_is_create_complete shall return true. ]*/
    TEST_FUNCTION(dns_resolver_ares__ipv6_answer__completes_with_the_address)
    {
        ///arrange
        const uint8_t address[16] = { 0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
        struct addrinfo* addr;
        DNSRESOLVER_HANDLE dns = start_lookup();

        ///act
        deliver_answer(ARES_SUCCESS, make_ipv6_hostent(address));

        ///assert
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        addr = dns_resolver_get_addrInfo(dns);
        ASSERT_IS_NOT_NULL(addr);
        ASSERT_ARE_EQUAL(int, AF_INET6, addr->ai_family);
        ASSERT_ARE_EQUAL(int, (int)sizeof(struct sockaddr_in6), (int)addr->ai_addrlen);
        ASSERT_ARE_EQUAL(int, 0, memcmp(&((struct sockaddr_in6*)addr->ai_addr)->sin6_addr, address, sizeof(address)));

        ///cleanup
        dns_resolver_destroy(dns);
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations);
    }

    /* Tests_SRS_dns_resolver_30_033: [ If dns_resolver_is_create_complete has returned true and the lookup process has failed, dns_resolver_get_ipv4 shall return 0. ]*/
    TEST_FUNCTION(dns_resolver_ares__all_zero_ipv6_answer__completes_as_failed)
    {
        ///arrange
        const uint8_t address[16] = { 0 };
        DNSRESOLVER_HANDLE dns = start_lookup();

        ///act
        deliver_answer(ARES_SUCCESS, make_ipv6_hostent(address));

        ///assert
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        ASSERT_IS_NULL(dns_resolver_get_addrInfo(dns));

        ///cleanup
        dns_resolver_destroy(dns);
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations);
    }
#endif // IPV6_ENABLED

    /* Tests_SRS_dns_resolver_30_021: [ dns_resolver_is_create_complete shall perform the asynchronous work of DNS lookup and log any errors. ]*/
    TEST_FUNCTION(dns_resolver_ares__poll_with_no_ready_socket__never_derives_a_socket)
    {
        // ares_getsock only writes the entries its bitmask reports, so an empty bitmask
        // must not produce a socket; c-ares is still driven so it can time the query out.
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();
        g_getsock_bitmask = 0;

        ///act
        ASSERT_IS_FALSE(dns_resolver_is_lookup_complete(dns));

        ///assert
        ASSERT_ARE_EQUAL(int, 1, g_process_fd_call_count);
        ASSERT_ARE_EQUAL(int, (int)ARES_SOCKET_BAD, (int)g_process_fd_calls[0].read_fd);
        ASSERT_ARE_EQUAL(int, (int)ARES_SOCKET_BAD, (int)g_process_fd_calls[0].write_fd);

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_021: [ dns_resolver_is_create_complete shall perform the asynchronous work of DNS lookup and log any errors. ]*/
    TEST_FUNCTION(dns_resolver_ares__poll_with_a_readable_socket__processes_it_for_read_only)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();
        g_getsock_bitmask = ARES_GETSOCK_READABLE(0xFFFFFFFF, 0);

        ///act
        ASSERT_IS_FALSE(dns_resolver_is_lookup_complete(dns));

        ///assert
        ASSERT_ARE_EQUAL(int, 1, g_process_fd_call_count);
        ASSERT_ARE_EQUAL(int, (int)TEST_SOCKET_0, (int)g_process_fd_calls[0].read_fd);
        ASSERT_ARE_EQUAL(int, (int)ARES_SOCKET_BAD, (int)g_process_fd_calls[0].write_fd);

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_021: [ dns_resolver_is_create_complete shall perform the asynchronous work of DNS lookup and log any errors. ]*/
    TEST_FUNCTION(dns_resolver_ares__poll_with_several_ready_sockets__processes_each_one)
    {
        // More than one socket is in play whenever c-ares has several servers configured,
        // parallel queries outstanding, or falls back to TCP.
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();
        g_getsock_bitmask = ARES_GETSOCK_READABLE(0xFFFFFFFF, 0) | ARES_GETSOCK_WRITABLE(0xFFFFFFFF, 2);

        ///act
        ASSERT_IS_FALSE(dns_resolver_is_lookup_complete(dns));

        ///assert
        ASSERT_ARE_EQUAL(int, 2, g_process_fd_call_count);
        ASSERT_ARE_EQUAL(int, (int)TEST_SOCKET_0, (int)g_process_fd_calls[0].read_fd);
        ASSERT_ARE_EQUAL(int, (int)ARES_SOCKET_BAD, (int)g_process_fd_calls[0].write_fd);
        ASSERT_ARE_EQUAL(int, (int)ARES_SOCKET_BAD, (int)g_process_fd_calls[1].read_fd);
        ASSERT_ARE_EQUAL(int, (int)TEST_SOCKET_2, (int)g_process_fd_calls[1].write_fd);

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_024: [ If dns_resolver_is_create_complete has previously returned true, dns_resolver_is_create_complete shall do nothing and return true. ]*/
    TEST_FUNCTION(dns_resolver_ares__poll_after_completion__does_nothing)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();
        deliver_answer(ARES_SUCCESS, make_ipv4_hostent(TEST_IPV4_ADDRESS));
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        umock_c_reset_all_calls();
        g_process_fd_call_count = 0;

        ///act
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));

        ///assert
        ASSERT_ARE_EQUAL(int, 0, g_process_fd_call_count);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_024: [ If dns_resolver_is_create_complete has previously returned true, dns_resolver_is_create_complete shall do nothing and return true. ]*/
    TEST_FUNCTION(dns_resolver_ares__poll_after_a_failed_lookup__stays_complete)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();
        deliver_answer(ARES_ENOTFOUND, NULL);
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));
        umock_c_reset_all_calls();
        g_process_fd_call_count = 0;

        ///act
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));

        ///assert
        ASSERT_ARE_EQUAL(int, 0, g_process_fd_call_count, "a completed lookup started work again");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_051: [ dns_resolver_destroy shall delete all acquired resources and delete the DNSRESOLVER_HANDLE. ]*/
    TEST_FUNCTION(dns_resolver_ares__destroy_after_a_successful_lookup__releases_everything)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();
        deliver_answer(ARES_SUCCESS, make_ipv4_hostent(TEST_IPV4_ADDRESS));
        ASSERT_IS_TRUE(dns_resolver_is_lookup_complete(dns));

        ///act
        dns_resolver_destroy(dns);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, g_live_allocations);
    }

    /* Tests_SRS_dns_resolver_30_031: [ If dns_resolver_is_create_complete has not yet returned true, dns_resolver_get_ipv4 shall log an error and return 0. ]*/
    TEST_FUNCTION(dns_resolver_ares__get_ipv4_before_completion__fails)
    {
        ///arrange
        DNSRESOLVER_HANDLE dns = start_lookup();

        ///act
        ///assert
        ASSERT_ARE_EQUAL(uint32_t, 0, dns_resolver_get_ipv4(dns));
        ASSERT_IS_NULL(dns_resolver_get_addrInfo(dns));

        ///cleanup
        dns_resolver_destroy(dns);
    }

    /* Tests_SRS_dns_resolver_30_020: [ If the dns parameter is NULL, dns_resolver_is_create_complete shall log an error and return false. ]*/
    /* Tests_SRS_dns_resolver_30_030: [ If the dns parameter is NULL, dns_resolver_get_ipv4 shall log an error and return 0. ]*/
    /* Tests_SRS_dns_resolver_30_050: [ If the dns parameter is NULL, dns_resolver_destroy shall log an error and do nothing. ]*/
    TEST_FUNCTION(dns_resolver_ares__null_handle__is_rejected)
    {
        ///act
        ///assert
        ASSERT_IS_FALSE(dns_resolver_is_lookup_complete(NULL));
        ASSERT_ARE_EQUAL(uint32_t, 0, dns_resolver_get_ipv4(NULL));
        ASSERT_IS_NULL(dns_resolver_get_addrInfo(NULL));
        dns_resolver_destroy(NULL);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    }

END_TEST_SUITE(dns_resolver_ares_ut)
