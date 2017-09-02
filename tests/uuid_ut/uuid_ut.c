// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"
#include "umock_c_negative_tests.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/uniqueid.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/uuid.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}


#define UUID_OCTET_COUNT    16
#define UUID_STRING_LENGTH  36
#define UUID_STRING_SIZE    (UUID_STRING_LENGTH + 1)

static UUID TEST_UUID = { 222, 193, 74, 152, 197, 252, 67, 14, 180, 227, 51, 193, 196, 52, 220, 175 };
static char* TEST_UUID_STRING = "dec14a98-c5fc-430e-b4e3-33c1c434dcaf";

static UNIQUEID_RESULT mock_UniqueId_Generate_result;
static UNIQUEID_RESULT mock_UniqueId_Generate(char* uid, size_t bufferSize)
{
    (void)memcpy(uid, TEST_UUID_STRING, bufferSize);
    return mock_UniqueId_Generate_result;
}

static void initialize_variables()
{
    mock_UniqueId_Generate_result = UNIQUEID_OK;
}

static void register_global_mock_returns()
{
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(malloc, NULL);
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(UniqueId_Generate, UNIQUEID_ERROR);
}

static void register_global_function_hooks()
{
    REGISTER_GLOBAL_MOCK_HOOK(UniqueId_Generate, mock_UniqueId_Generate);
}

static void register_mock_aliases()
{
    REGISTER_UMOCK_ALIAS_TYPE(UNIQUEID_RESULT, int);
}


BEGIN_TEST_SUITE(uuid_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);

    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    register_mock_aliases();
    register_global_mock_returns();
    register_global_function_hooks();
    initialize_variables();
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest) != 0)
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

// Tests_SRS_UUID_09_001: [ If `uuid` is NULL, uuid_generate shall return a non-zero value ]
TEST_FUNCTION(uuid_generate_NULL_uuid)
{
    //Arrange
    int result;

    umock_c_reset_all_calls();

    //Act
    result = uuid_generate(NULL);

    //Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
}

// Tests_SRS_UUID_09_002: [ uuid_generate shall obtain an UUID string from UniqueId_Generate ]
// Tests_SRS_UUID_09_004: [ The UUID string shall be parsed into an UUID type (16 unsigned char array) and filled in `uuid` ]  
// Tests_SRS_UUID_09_006: [ If no failures occur, uuid_generate shall return zero ]
TEST_FUNCTION(uuid_generate_succeed)
{
    //Arrange
    UUID uuid;
    int result;
    char uuid_string[UUID_STRING_SIZE];

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(malloc(UUID_STRING_SIZE))
        .SetReturn(uuid_string);
    STRICT_EXPECTED_CALL(UniqueId_Generate(uuid_string, UUID_STRING_SIZE));
    STRICT_EXPECTED_CALL(free(uuid_string));

    //Act
    result = uuid_generate(&uuid);

    //Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);

    {
        int i;
        for (i = 0; i < UUID_OCTET_COUNT; i++)
        {
            ASSERT_ARE_EQUAL(int, TEST_UUID[i], uuid[i]);
        }
    }
}

// Tests_SRS_UUID_09_003: [ If the UUID string fails to be obtained, uuid_generate shall fail and return a non-zero value ]
// Tests_SRS_UUID_09_005: [ If `uuid` fails to be set, uuid_generate shall fail and return a non-zero value ]
TEST_FUNCTION(uuid_generate_failure_checks)
{
    //Arrange
    UUID uuid;
    int result;
    size_t i;
    char uuid_string[UUID_STRING_SIZE];

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(malloc(UUID_STRING_SIZE))
        .SetReturn(uuid_string);
    STRICT_EXPECTED_CALL(UniqueId_Generate(uuid_string, UUID_STRING_SIZE));
    STRICT_EXPECTED_CALL(free(uuid_string));
    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        char temp_str[64];

        if (i == 2) continue;

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        (void)sprintf(temp_str, "On failed call %zu", i);

        // act
        result = uuid_generate(&uuid);

        // assert
        ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, 0, result, temp_str);
    }

    umock_c_negative_tests_reset();
    umock_c_negative_tests_deinit();
}

// Tests_SRS_UUID_09_011: [ If `uuid` is NULL, uuid_to_string shall return a non-zero value ]  
TEST_FUNCTION(uuid_to_string_NULL_uuid)
{
    //Arrange

    //Act
    char* result = uuid_to_string(NULL);

    //Assert
    ASSERT_IS_NULL(result);
}

// Tests_SRS_UUID_09_012: [ uuid_to_string shall allocate a valid UUID string (`uuid_string`) as per RFC 4122 ]  
// Tests_SRS_UUID_09_014: [ Each character in `uuid` shall be written in the respective positions of `uuid_string` as a 2-digit HEX value ]  
// Tests_SRS_UUID_09_016: [ If no failures occur, uuid_to_string shall return `uuid_string` ] 
TEST_FUNCTION(uuid_to_string_succeed)
{
    //Arrange
    char* result;

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(malloc(UUID_STRING_SIZE * sizeof(char)))
        .SetReturn(TEST_UUID_STRING);

    //Act
    result = uuid_to_string(&TEST_UUID);

    //Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(char_ptr, TEST_UUID_STRING, result);
}

// Tests_SRS_UUID_09_013: [ If `uuid_string` fails to be allocated, uuid_to_string shall return NULL ]  
// Tests_SRS_UUID_09_015: [ If `uuid_string` fails to be set, uuid_to_string shall return NULL ]  
TEST_FUNCTION(uuid_to_string_failure_checks)
{
    //Arrange
    char* result;
    size_t i;

    ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(malloc(UUID_STRING_SIZE * sizeof(char)))
        .SetReturn(TEST_UUID_STRING);

    umock_c_negative_tests_snapshot();

    for (i = 0; i < umock_c_negative_tests_call_count(); i++)
    {
        char temp_str[64];

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(i);

        (void)sprintf(temp_str, "On failed call %zu", i);

        // act
        result = uuid_to_string(&TEST_UUID);

        // assert
        ASSERT_IS_NULL_WITH_MSG(result, temp_str);
    }

    umock_c_negative_tests_reset();
    umock_c_negative_tests_deinit();
}

// Tests_SRS_UUID_09_007: [ If `uuid_string` or `uuid` are NULL, uuid_from_string shall return a non-zero value ]


// Tests_SRS_UUID_09_008: [ Each pair of digits in `uuid_string`, excluding dashes, shall be read as a single HEX value and saved on the respective position in `uuid` ]  
// Tests_SRS_UUID_09_010: [ If no failures occur, uuid_from_string shall return zero ]
TEST_FUNCTION(uuid_from_string_succeed)
{
    //Arrange
    int result;
    UUID uuid;

    umock_c_reset_all_calls();

    //Act
    result = uuid_from_string(TEST_UUID_STRING, &uuid);

    //Assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, 0, result);
    
    {
        int i;
        for (i = 0; i < UUID_OCTET_COUNT; i++)
        {
            ASSERT_ARE_EQUAL(int, TEST_UUID[i], uuid[i]);
        }
    }
}

// Tests_SRS_UUID_09_009: [ If `uuid_string` fails to be generated, uuid_from_string shall return a non-zero value ]
// To be implemented once sscanf mock is implemented.

END_TEST_SUITE(uuid_unittests)
