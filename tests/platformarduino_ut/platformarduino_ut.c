// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

static void* my_gballoc_malloc(size_t size)
{
	return malloc(size);
}

static void my_gballoc_free(void* s)
{
	free(s);
}

#include <stddef.h>
#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "umocktypes_charptr.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/platform.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "../adapters/tlsio_arduino.h"

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

#define TESTS_VALID_POINTER	0xCAFECAFE

static IO_INTERFACE_DESCRIPTION* my_tlsio_arduino_get_interface_description_return = NULL;
const IO_INTERFACE_DESCRIPTION* my_tlsio_arduino_get_interface_description(void)
{
    return my_tlsio_arduino_get_interface_description_return;
}

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

BEGIN_TEST_SUITE(platformarduino_ut)

    TEST_SUITE_INITIALIZE(a)
    {
        TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        int result = umocktypes_charptr_register_types();
		ASSERT_ARE_EQUAL(int, 0, result);

		REGISTER_UMOCK_ALIAS_TYPE(IO_INTERFACE_DESCRIPTION, void*);

        REGISTER_GLOBAL_MOCK_HOOK(tlsio_arduino_get_interface_description, my_tlsio_arduino_get_interface_description);
		REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
		REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
}

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
        TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
    }

    TEST_FUNCTION_INITIALIZE(initialize)
    {
        umock_c_reset_all_calls();
    }

    TEST_FUNCTION_CLEANUP(cleans)
    {

    }

	/*Tests_SRS_PLATFORM_ARDUINO_21_001: [ The platform_arduino shall implement the interface provided in the `platfom.h`. ]*/
	/*Tests_SRS_PLATFORM_ARDUINO_21_003: [ The platform_init shall initialize the platform. ]*/
	/*Tests_SRS_PLATFORM_ARDUINO_21_004: [ The platform_init shall allocate any memory needed to control the platform. ]*/
    TEST_FUNCTION(platform_init__succeed)
    {
        ///arrange

        ///act
        int result = platform_init();

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);

        ///cleanup
		platform_deinit();
    }

    /*Tests_SRS_PLATFORM_ARDUINO_21_005: [ The platform_deinit shall deinitialize the platform. ]*/
	/*Tests_SRS_PLATFORM_ARDUINO_21_006: [ The platform_deinit shall free all allocate memory needed to control the platform. ]*/
    TEST_FUNCTION(platform_deinit__succeed)
    {
        ///arrange
        (void)platform_init();

        ///act
        platform_deinit();

        ///assert
		ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
    }

	/*Tests_SRS_PLATFORM_ARDUINO_21_002: [ The platform_arduino shall use the tlsio functions defined by the 'xio.h'.*/
	/*Tests_SRS_PLATFORM_ARDUINO_21_007: [ The platform_get_default_tlsio shall return a set of tlsio functions provided by the Arduino tlsio implementation. ]*/
	TEST_FUNCTION(platform_get_default_tlsio__valid_ptr_succeed)
	{
		///arrange
		my_tlsio_arduino_get_interface_description_return = (IO_INTERFACE_DESCRIPTION*)TESTS_VALID_POINTER;
		const IO_INTERFACE_DESCRIPTION* result;

		STRICT_EXPECTED_CALL(tlsio_arduino_get_interface_description());

        (void)platform_init();

		///act
		result = platform_get_default_tlsio();

		///assert
		ASSERT_ARE_EQUAL(void_ptr, (void*)TESTS_VALID_POINTER, result);
		ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

		///cleanup
        platform_deinit();
    }

	/*Tests_SRS_PLATFORM_ARDUINO_21_007: [ The platform_get_default_tlsio shall return a set of tlsio functions provided by the Arduino tlsio implementation. ]*/
	TEST_FUNCTION(platform_get_default_tlsio__NULL_failed)
	{
		///arrange
		my_tlsio_arduino_get_interface_description_return = NULL;
		const IO_INTERFACE_DESCRIPTION* result;

		STRICT_EXPECTED_CALL(tlsio_arduino_get_interface_description());

        (void)platform_init();

		///act
		result = platform_get_default_tlsio();

		///assert
		ASSERT_IS_NULL(result);
		ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

		///cleanup
        platform_deinit();
    }

END_TEST_SUITE(platformarduino_ut)
