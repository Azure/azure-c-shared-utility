// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#else
#include <stdlib.h>
#include <stddef.h>
#endif

void* real_malloc(size_t size)
{
    return malloc(size);
}

void real_free(void* ptr)
{
    free(ptr);
}

#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umock_c_negative_tests.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umocktypes_bool.h"
#include "umocktypes.h"
#include "umocktypes_c.h"
#include "azure_c_shared_utility/macro_utils.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#undef ENABLE_MOCKS

#include "azure_c_shared_utility/string_token.h"

static TEST_MUTEX_HANDLE g_dllByDll;
static TEST_MUTEX_HANDLE g_testByTest;

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

// Helpers
static int saved_malloc_returns_count = 0;
static void* saved_malloc_returns[20];

static void* TEST_malloc(size_t size)
{
    saved_malloc_returns[saved_malloc_returns_count] = real_malloc(size);

    return saved_malloc_returns[saved_malloc_returns_count++];
}

static void TEST_free(void* ptr)
{
    int i, j;
    for (i = 0, j = 0; j < saved_malloc_returns_count; i++, j++)
    {
        if (saved_malloc_returns[i] == ptr) j++;

        saved_malloc_returns[i] = saved_malloc_returns[j];
    }

    if (i != j) saved_malloc_returns_count--;

    real_free(ptr);
}

static void register_umock_alias_types()
{
    //REGISTER_UMOCK_ALIAS_TYPE(OPTIONHANDLER_HANDLE, void*);
}

static void register_global_mock_hooks()
{
    REGISTER_GLOBAL_MOCK_HOOK(malloc, TEST_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(free, TEST_free);
}

static void register_global_mock_returns()
{
    //REGISTER_GLOBAL_MOCK_RETURN(OptionHandler_Create, TEST_OPTIONHANDLER_HANDLE);
    //REGISTER_GLOBAL_MOCK_FAIL_RETURN(OptionHandler_Create, NULL);
}

// Set Expected Call Helpers
static void set_expected_calls_for_get_delimiters_lengths()
{
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
}

static void set_expected_calls_for_StringToken_GetFirst()
{
    STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
    set_expected_calls_for_get_delimiters_lengths();
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));
}

static void set_expected_calls_for_StringToken_GetNext()
{
    set_expected_calls_for_get_delimiters_lengths();
    STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // delimiters lengths
}

BEGIN_TEST_SUITE(string_token_ut)

    TEST_SUITE_INITIALIZE(string_token_ut_initialize)
    {
        int result;

        TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);

        register_umock_alias_types();
        register_global_mock_returns();
        register_global_mock_hooks();
    }

    TEST_SUITE_CLEANUP(string_token_ut_cleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
        TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
    }

    TEST_FUNCTION_INITIALIZE(string_token_ut_test_function_init)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
        }

        umock_c_reset_all_calls();
    }

    TEST_FUNCTION_CLEANUP(string_token_ut_test_function_cleanup)
    {
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    // Tests_SRS_STRING_TOKENIZER_09_001: [ If `source` or `delimiters` are NULL, or `length` or `n_delims` are zero, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetFirst_NULL_source)
    {
        ///arrange
        size_t length = 10;

        char* delimiters[1];
        delimiters[0] = "?";

        umock_c_reset_all_calls();

        // act
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(NULL, length, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_001: [ If `source` or `delimiters` are NULL, or `length` or `n_delims` are zero, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetFirst_NULL_delimiters)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        umock_c_reset_all_calls();

        // act
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, NULL, 4);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_001: [ If `source` or `delimiters` are NULL, or `length` or `n_delims` are zero, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetFirst_ZERO_delimiters)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[1];
        delimiters[0] = "?";

        umock_c_reset_all_calls();

        // act
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 0);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_002: [ If any of the strings in `delimiters` are NULL, the function shall return NULL ]
    // Tests_SRS_STRING_TOKENIZER_09_007: [ If any failure occurs, all memory allocated by this function shall be released ]
    TEST_FUNCTION(StringToken_GetFirst_NULL_delimiter)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[4];
        delimiters[0] = "http://";
        delimiters[1] = NULL;
        delimiters[2] = "/";
        delimiters[3] = "?";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

        // act
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 4);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_004: [ If the STRING_TOKEN structure fails to be allocated, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetFirst_negative_tests)
    {
        ///arrange
        ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[1];
        delimiters[0] = "?";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        set_expected_calls_for_get_delimiters_lengths();
        umock_c_negative_tests_snapshot();

        // act
        size_t i;
        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            // arrange
            char error_msg[64];

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            // act
            STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 1);

            sprintf(error_msg, "On failed call %zu", i);
            ASSERT_IS_NULL_WITH_MSG(handle, error_msg);
        }

        // cleanup
        umock_c_negative_tests_deinit();
    }

    // Tests_SRS_STRING_TOKENIZER_09_003: [ A STRING_TOKEN structure shall be allocated to hold the token parameters ]
    // Tests_SRS_STRING_TOKENIZER_09_005: [ The source string shall be split in a token starting from the beginning of `source` up to occurrence of any one of the `demiliters`, whichever occurs first in the order provided ]
    TEST_FUNCTION(StringToken_GetFirst_Success)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);
        
        char* delimiters[1];
        delimiters[0] = "?";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG));

        // act
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(handle);

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_006: [ If the source string does not have any of the `demiliters`, the resulting token shall be the entire `source` string ]
    TEST_FUNCTION(StringToken_GetFirst_delimiter_not_found)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[1];
        delimiters[0] = "#";

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG));
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // delimiters lengths

        // act
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(void_ptr, (void_ptr)string, (void_ptr)StringToken_GetValue(handle));
        ASSERT_ARE_EQUAL(size_t, length, StringToken_GetLength(handle));

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_008: [ If `token` or `delimiters` are NULL, or `n_delims` is zero, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetNext_NULL_token)
    {
        ///arrange
        char* delimiters[2];
        delimiters[0] = "https://";
        delimiters[1] = "/path";

        umock_c_reset_all_calls();

        // act
        STRING_TOKEN_HANDLE handle = StringToken_GetNext(NULL, delimiters, 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_008: [ If `token` or `delimiters` are NULL, or `n_delims` is zero, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetNext_NULL_delimiters)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[2];
        delimiters[0] = "https://";
        delimiters[1] = "/path";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 2);

        umock_c_reset_all_calls();

        // act
        STRING_TOKEN_HANDLE handle2 = StringToken_GetNext(handle, NULL, 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle2);

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_008: [ If `token` or `delimiters` are NULL, or `n_delims` is zero, the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetNext_zero_delimiters)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[2];
        delimiters[0] = "https://";
        delimiters[1] = "/path";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 2);

        umock_c_reset_all_calls();

        // act
        STRING_TOKEN_HANDLE handle2 = StringToken_GetNext(handle, delimiters, 0);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle2);

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_010: [ The next token shall be selected starting from the position in `source` right after the previous delimiter up to occurrence of any one of `demiliters`, whichever occurs first in the order provided ]
    TEST_FUNCTION(StringToken_GetNext_Success)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[2];
        delimiters[0] = "https://";
        delimiters[1] = "/path";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 2);
        
        umock_c_reset_all_calls();
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // delimiters lengths

        // act
        handle = StringToken_GetNext(handle, delimiters, 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(int, 0, strncmp(StringToken_GetValue(handle), "some.site.com", StringToken_GetLength(handle)));

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_012: [ If any failure occurs, the memory allocated for STRING_TOKEN shall be released ]
    TEST_FUNCTION(StringToken_GetNext_NULL_delimiter)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[2];
        delimiters[0] = "https://";
        delimiters[1] = "/path"; // this causes the failure

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 2);

        umock_c_reset_all_calls();
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // delimiters lengths
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // STRING_TOKEN

        delimiters[1] = NULL; // this causes the failure

        // act
        handle = StringToken_GetNext(handle, delimiters, 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_012: [ If any failure occurs, the memory allocated for STRING_TOKEN shall be released ]
    TEST_FUNCTION(StringToken_GetNext_malloc_fails)
    {
        ///arrange
        ASSERT_ARE_EQUAL(int, 0, umock_c_negative_tests_init());

        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[2];
        delimiters[0] = "https://";
        delimiters[1] = "/path"; // this causes the failure

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 2);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(malloc(IGNORED_NUM_ARG)); // delimiters lengths
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // STRING_TOKEN
        umock_c_negative_tests_snapshot();

        umock_c_negative_tests_reset();
        umock_c_negative_tests_fail_call(0);

        // act
        handle = StringToken_GetNext(handle, delimiters, 2);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
        umock_c_negative_tests_deinit();
    }

    // Tests_SRS_STRING_TOKENIZER_09_009: [ If the previous token already extended to the end of `source`, the function shall delete `token` and return NULL ]
    TEST_FUNCTION(StringToken_GetNext_no_more_tokens)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[1];
        delimiters[0] = "?";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 1);
        ASSERT_IS_NOT_NULL(handle);

        set_expected_calls_for_StringToken_GetNext();
        handle = StringToken_GetNext(handle, delimiters, 1);
        ASSERT_IS_NOT_NULL(handle);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // STRING_TOKEN

        // act
        handle = StringToken_GetNext(handle, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(handle);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_011: [ If the source string, starting right after the position of the last delimiter found, does not have any of the `demiliters`, the resulting token shall be the entire remaining of the `source` string ]
    TEST_FUNCTION(StringToken_GetNext_delimiter_not_found)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[1];
        delimiters[0] = "?";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 1);

        umock_c_reset_all_calls();
        set_expected_calls_for_get_delimiters_lengths();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // delimiters lengths

        // act
        handle = StringToken_GetNext(handle, delimiters, 1);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(int, 0, strncmp(StringToken_GetValue(handle), "prop1=site.com&prop2=/prop2/abc", StringToken_GetLength(handle)));

        // cleanup
        StringToken_Destroy(handle);
    }

    // Tests_SRS_STRING_TOKENIZER_09_013: [ If `token` is NULL the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetValue_NULL_handle)
    {
        ///arrange
        umock_c_reset_all_calls();

        // act
        const char* value = StringToken_GetValue(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(value);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_015: [ If `token` is NULL the function shall return zero ]
    TEST_FUNCTION(StringToken_GetLength_NULL_handle)
    {
        ///arrange
        umock_c_reset_all_calls();

        // act
        size_t length = StringToken_GetLength(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_ARE_EQUAL(int, 0, length);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_017: [ If `token` is NULL the function shall return NULL ]
    TEST_FUNCTION(StringToken_GetDelimiter_NULL_handle)
    {
        ///arrange
        umock_c_reset_all_calls();

        // act
        const char* value = StringToken_GetDelimiter(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IS_NULL(value);

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_020: [ If `token` is NULL the function shall return ]
    TEST_FUNCTION(StringToken_Destroy_NULL_handle)
    {
        ///arrange
        umock_c_reset_all_calls();

        // act
        StringToken_Destroy(NULL);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_021: [ Otherwise the memory allocated for STRING_TOKEN shall be released ]
    TEST_FUNCTION(StringToken_Destroy_success)
    {
        ///arrange
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";
        size_t length = strlen(string);

        char* delimiters[1];
        delimiters[0] = "?";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, length, delimiters, 1);

        umock_c_reset_all_calls();
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // STRING_TOKEN

        // act
        StringToken_Destroy(handle);

        // assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
    }

    // Tests_SRS_STRING_TOKENIZER_09_014: [ The function shall return the pointer to the position in `source` where the current token starts. ]
    // Tests_SRS_STRING_TOKENIZER_09_016: [ The function shall return the length of the current token ]
    // Tests_SRS_STRING_TOKENIZER_09_018: [ The function shall return a pointer to the delimiter that defined the current token, as passed to the previous call to `StringToken_GetNext()` or `StringToken_GetFirst()` ]
    // Tests_SRS_STRING_TOKENIZER_09_019: [ If the current token extends to the end of `source`, the function shall return NULL ]
    TEST_FUNCTION(StringToken_tokenize_HTTP_URL)
    {
        ///arrange
        char* host = "some.site.com";
        char* relative_path = "path/morepath/";
        char* property1 = "prop1=site.com";
        char* property2 = "prop2=/prop2/abc";
        char* string = "https://some.site.com/path/morepath/?prop1=site.com&prop2=/prop2/abc";

        char* delimiters1[4];
        delimiters1[0] = "?";
        delimiters1[1] = "http://";
        delimiters1[2] = "https://";
        delimiters1[3] = "/";

        char* delimiters2[1];
        delimiters2[0] = "&";

        // act
        // assert
        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, strlen(string), delimiters1, 4);
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters1[2], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_NULL(StringToken_GetValue(handle));
        ASSERT_ARE_EQUAL(int, 0, StringToken_GetLength(handle));

        set_expected_calls_for_StringToken_GetNext();
        handle = StringToken_GetNext(handle, delimiters1, 4);
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters1[3], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(host, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);

        set_expected_calls_for_StringToken_GetNext();
        handle = StringToken_GetNext(handle, delimiters1, 1); // intentionally restricting to "?" only
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters1[0], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(relative_path, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);

        set_expected_calls_for_StringToken_GetNext();
        handle = StringToken_GetNext(handle, delimiters2, 1);
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters2[0], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(property1, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);

        set_expected_calls_for_StringToken_GetNext();
        handle = StringToken_GetNext(handle, delimiters2, 1);
        ASSERT_IS_NOT_NULL(handle);
        // SRS_STRING_TOKENIZER_09_019:
        ASSERT_IS_NULL(StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(property2, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);

        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // STRING_TOKEN
        handle = StringToken_GetNext(handle, delimiters2, 1);
        ASSERT_IS_NULL(handle);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
    }

    TEST_FUNCTION(StringToken_string_ends_with_delimiter)
    {
        ///arrange
        char* string = "abcde";

        char* delimiters[1];
        delimiters[0] = "de";

        umock_c_reset_all_calls();
        set_expected_calls_for_StringToken_GetFirst();

        // act
        STRING_TOKEN_HANDLE handle = StringToken_GetFirst(string, strlen(string), delimiters, 1);

        // assert
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_ARE_EQUAL(void_ptr, (void*)delimiters[0], (void*)StringToken_GetDelimiter(handle));
        ASSERT_IS_TRUE(strncmp(string, StringToken_GetValue(handle), StringToken_GetLength(handle)) == 0);
        ASSERT_ARE_EQUAL(int, 3, StringToken_GetLength(handle));

        ///arrange
        set_expected_calls_for_StringToken_GetNext();

        // act
        handle = StringToken_GetNext(handle, delimiters, 1);

        // assert
        ASSERT_IS_NOT_NULL(handle);
        ASSERT_IS_NULL(StringToken_GetDelimiter(handle));
        ASSERT_IS_NULL(StringToken_GetValue(handle));
        ASSERT_ARE_EQUAL(int, 0, StringToken_GetLength(handle));

        ///arrange
        STRICT_EXPECTED_CALL(free(IGNORED_PTR_ARG)); // STRING_TOKEN

        // act
        handle = StringToken_GetNext(handle, delimiters, 1);

        // assert
        ASSERT_IS_NULL(handle);

        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        // cleanup
    }

END_TEST_SUITE(string_token_ut)
