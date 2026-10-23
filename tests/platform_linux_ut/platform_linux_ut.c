// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <string.h>

#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"
#include "macro_utils/macro_utils.h"

#define ENABLE_MOCKS
#include "umock_c/umock_c_prod.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/httpapiex.h"
#include "azure_c_shared_utility/tlsio_openssl.h"

MOCKABLE_FUNCTION(, int, mock_logger_init);
MOCKABLE_FUNCTION(, void, mock_logger_deinit);
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/platform.h"

/* Variadic, so not mockable; only referenced by platform_get_platform_info, which these tests do not cover. */
STRING_HANDLE STRING_construct_sprintf(const char* format, ...)
{
    (void)format;
    return NULL;
}

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%" PRI_MU_ENUM "", MU_ENUM_VALUE(UMOCK_C_ERROR_CODE, error_code));
}

static TEST_MUTEX_HANDLE g_testByTest;

BEGIN_TEST_SUITE(platform_linux_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
    ASSERT_ARE_EQUAL(int, 0, umocktypes_charptr_register_types());
    ASSERT_ARE_EQUAL(int, 0, umocktypes_stdint_register_types());

    REGISTER_UMOCK_ALIAS_TYPE(HTTPAPIEX_RESULT, int);
    REGISTER_GLOBAL_MOCK_RETURNS(mock_logger_init, 0, MU_FAILURE);
    REGISTER_GLOBAL_MOCK_RETURNS(HTTPAPIEX_Init, HTTPAPIEX_OK, HTTPAPIEX_ERROR);
    REGISTER_GLOBAL_MOCK_RETURNS(tlsio_openssl_init, 0, MU_FAILURE);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(g_testByTest);
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("Could not acquire test serialization mutex.");
    }
    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(method_cleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

TEST_FUNCTION(platform_init_calls_logger_init_first)
{
    //arrange
    STRICT_EXPECTED_CALL(mock_logger_init());
#if defined(USE_HTTP) && !defined(DONT_USE_UPLOADTOBLOB)
    STRICT_EXPECTED_CALL(HTTPAPIEX_Init());
#endif
    STRICT_EXPECTED_CALL(tlsio_openssl_init());

    //act
    int result = platform_init();

    //assert
    ASSERT_ARE_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(platform_init_logger_init_fails)
{
    //arrange
    STRICT_EXPECTED_CALL(mock_logger_init()).SetReturn(MU_FAILURE);

    //act
    int result = platform_init();

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

#if defined(USE_HTTP) && !defined(DONT_USE_UPLOADTOBLOB)
TEST_FUNCTION(platform_init_HTTPAPIEX_Init_fails_releases_logger)
{
    //arrange
    STRICT_EXPECTED_CALL(mock_logger_init());
    STRICT_EXPECTED_CALL(HTTPAPIEX_Init()).SetReturn(HTTPAPIEX_ERROR);
    STRICT_EXPECTED_CALL(mock_logger_deinit());

    //act
    int result = platform_init();

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}
#endif

TEST_FUNCTION(platform_init_tlsio_openssl_init_fails_releases_logger)
{
    //arrange
    STRICT_EXPECTED_CALL(mock_logger_init());
#if defined(USE_HTTP) && !defined(DONT_USE_UPLOADTOBLOB)
    STRICT_EXPECTED_CALL(HTTPAPIEX_Init());
#endif
    STRICT_EXPECTED_CALL(tlsio_openssl_init()).SetReturn(MU_FAILURE);
    STRICT_EXPECTED_CALL(mock_logger_deinit());

    //act
    int result = platform_init();

    //assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

TEST_FUNCTION(platform_deinit_calls_logger_deinit_last)
{
    //arrange
#if defined(USE_HTTP) && !defined(DONT_USE_UPLOADTOBLOB)
    STRICT_EXPECTED_CALL(HTTPAPIEX_Deinit());
#endif
    STRICT_EXPECTED_CALL(tlsio_openssl_deinit());
    STRICT_EXPECTED_CALL(mock_logger_deinit());

    //act
    platform_deinit();

    //assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

END_TEST_SUITE(platform_linux_ut)
