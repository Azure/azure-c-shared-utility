// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

static void my_gballoc_free(void* ptr)
{
    free(ptr);
}

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"
#include "azure_c_shared_utility/macro_utils.h"

#include "wolfssl/ssl.h"
#include "wolfssl/error-ssl.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/tlsio_wolfssl.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

static WOLFSSL_METHOD* TEST_WOLFSSL_CLIENT_METHOD = (WOLFSSL_METHOD*)0x0011;
static WOLFSSL_CTX* TEST_WOLFSSL_CTX = (WOLFSSL_CTX*)0x0012;
static WOLFSSL* TEST_WOLFSSL = (WOLFSSL*)0x0013;
static const IO_INTERFACE_DESCRIPTION* TEST_SOCKETIO_INTERFACE_DESCRIPTION = (const IO_INTERFACE_DESCRIPTION*)0x0014;
static XIO_HANDLE TEST_IO_HANDLE = (XIO_HANDLE)0x0015;

MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_SetIORecv, WOLFSSL_CTX*, ctx, CallbackIORecv, cb_rcv)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_SetIOSend, WOLFSSL_CTX*, ctx, CallbackIORecv, cb_rcv)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_SetIOReadCtx, WOLFSSL*, ssl, void*, ctx)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_SetIOWriteCtx, WOLFSSL*, ssl, void*, ctx)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, WOLFSSL_METHOD*, wolfTLSv1_2_client_method)
MOCK_FUNCTION_END(TEST_WOLFSSL_CLIENT_METHOD)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, WOLFSSL_CTX*, wolfSSL_CTX_new, WOLFSSL_METHOD*, method)
MOCK_FUNCTION_END(TEST_WOLFSSL_CTX)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, WOLFSSL*, wolfSSL_new, WOLFSSL_CTX*, ctx)
MOCK_FUNCTION_END(TEST_WOLFSSL)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_set_using_nonblock, WOLFSSL*, ssl, int, opt);
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_connect, WOLFSSL*, ssl)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_write, WOLFSSL*, ssl, const void*, data, int, len)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_read, WOLFSSL*, ssl, void*, buff, int, len)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_CTX_free, WOLFSSL_CTX*, ctx)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_free, WOLFSSL*, ssl)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, void, wolfSSL_load_error_strings)
MOCK_FUNCTION_END()
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_library_init)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_CTX_load_verify_buffer, WOLFSSL_CTX*, ctx, const unsigned char*, buff, long, len, int, opt)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_use_PrivateKey_buffer, WOLFSSL*, ssl, const unsigned char*, buff, long, len, int, opt)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_use_certificate_chain_buffer, WOLFSSL*, ssl, const unsigned char*, chain_buff, long, len)
MOCK_FUNCTION_END(0)
MOCK_FUNCTION_WITH_CODE(WOLFSSL_API, int, wolfSSL_SetHsDoneCb, WOLFSSL*, ssl, HandShakeDoneCb, hs_cb, void*, ctx)
MOCK_FUNCTION_END(0)

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(tlsio_wolfssl_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(CONCRETE_IO_HANDLE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);

    REGISTER_GLOBAL_MOCK_RETURN(socketio_get_interface_description, TEST_SOCKETIO_INTERFACE_DESCRIPTION);
    REGISTER_GLOBAL_MOCK_RETURN(xio_create, TEST_IO_HANDLE);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

TEST_FUNCTION(tlsio_wolfssl_create_succeeds)
{
    //arrange
    TLSIO_CONFIG tls_io_config;
    memset(&tls_io_config, 0, sizeof(tls_io_config));

    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(wolfTLSv1_2_client_method());
    STRICT_EXPECTED_CALL(wolfSSL_CTX_new(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(socketio_get_interface_description());
    STRICT_EXPECTED_CALL(xio_create(IGNORED_NUM_ARG, IGNORED_NUM_ARG));

    //act
    CONCRETE_IO_HANDLE io_handle = tlsio_wolfssl_create(&tls_io_config);

    //assert
    ASSERT_IS_NOT_NULL(io_handle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    //clean
    tlsio_wolfssl_destroy(io_handle);
}

END_TEST_SUITE(tlsio_wolfssl_ut)
