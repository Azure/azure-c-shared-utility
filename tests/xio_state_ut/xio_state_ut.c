// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>

#ifdef __cplusplus
#else
#endif

/**
 * Include the C standards here.
 */
#ifdef __cplusplus
#include <cstddef>
#include <cstdlib>
#include <ctime>
#else
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#endif

/**
 * The gballoc.h will replace the malloc, free, and realloc by the my_gballoc functions, in this case,
 *    if you define these mock functions after include the gballoc.h, you will create an infinity recursion,
 *    so, places the my_gballoc functions before the #include "azure_c_shared_utility/gballoc.h"
 */
#include "gballoc_ut_impl_1.h"

 /**
 * Include the mockable headers here.
 * These are the headers that contains the functions that you will replace to execute the test.
 */
#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xio_adapter.h"
#undef ENABLE_MOCKS

/**
 * Include the test tools.
 */
#include "azure_c_shared_utility/xio_state.h"
#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/crt_abstractions.h"

// These "headers" are actually source files that are broken out of this file for readability
#include "message_processing.h"
#include "fake_adapter.h"
#include "callbacks.h"

#define SETOPTION_PV_COUNT 3
#define OPEN_PV_COUNT 4
#define SEND_PV_COUNT 4
#define CLOSE_PV_COUNT 2

#include "gballoc_ut_impl_2.h"

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

typedef struct create_params_tag
{
    void* dummy;
} create_params_t;

static create_params_t create_params = { 0 };

OPTIONHANDLER_HANDLE fake_option_handler = (OPTIONHANDLER_HANDLE)0x0004;

// Enum for the create_in_state helper function
// The order here is used in the create_in_state function; don't change it
typedef enum XIO_STATE_ENUM_TAG
{
    XIO_STATE_INITIAL,
    XIO_STATE_OPENING,
    XIO_STATE_OPEN,
    XIO_STATE_IO_ERROR,
    XIO_STATE_CLOSING,
    XIO_STATE_CLOSED,
    XIO_STATE_OPEN_ERROR
} XIO_STATE_ENUM;


/**
 * This is necessary for the test suite, just keep as is.
 */
static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;
static bool negative_mocks_used = false;


BEGIN_TEST_SUITE(xio_state_unittests)

    TEST_SUITE_INITIALIZE(a)
    {
        int result;
        size_t type_size;
        TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
        g_testByTest = TEST_MUTEX_CREATE();
        ASSERT_IS_NOT_NULL(g_testByTest);

        (void)umock_c_init(on_umock_c_error);

        result = umocktypes_charptr_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);
        result = umocktypes_bool_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);
        umocktypes_stdint_register_types();
        ASSERT_ARE_EQUAL(int, 0, result);

        REGISTER_UMOCK_ALIAS_TYPE(XIO_ADAPTER_INSTANCE_HANDLE, void*);
        REGISTER_UMOCK_ALIAS_TYPE(ON_BYTES_RECEIVED, void*);
        REGISTER_UMOCK_ALIAS_TYPE(XIO_ASYNC_RESULT, int);

        type_size = sizeof(time_t);
        if (type_size == sizeof(uint64_t))
        {
            REGISTER_UMOCK_ALIAS_TYPE(time_t, uint64_t);
        }
        else if (type_size == sizeof(uint32_t))
        {
            REGISTER_UMOCK_ALIAS_TYPE(time_t, uint32_t);
        }
        else
        {
            ASSERT_FAIL("Bad size_t size");
        }

        REGISTER_GLOBAL_MOCK_RETURNS(xio_adapter_create, &adapter_instance, NULL);
        REGISTER_GLOBAL_MOCK_RETURNS(xio_adapter_close, XIO_ASYNC_RESULT_SUCCESS, XIO_ASYNC_RESULT_FAILURE);
        REGISTER_GLOBAL_MOCK_RETURNS(xio_adapter_open, XIO_ASYNC_RESULT_SUCCESS, XIO_ASYNC_RESULT_FAILURE);
        REGISTER_GLOBAL_MOCK_RETURNS(xio_adapter_read, XIO_ASYNC_RESULT_SUCCESS, XIO_ASYNC_RESULT_FAILURE);
        REGISTER_GLOBAL_MOCK_RETURNS(xio_adapter_retrieveoptions, fake_option_handler, NULL);
        REGISTER_GLOBAL_MOCK_RETURNS(xio_adapter_setoption, 0, 1);
        
        REGISTER_GLOBAL_MOCK_HOOK(xio_adapter_open, my_xio_adapter_open);
        REGISTER_GLOBAL_MOCK_HOOK(xio_adapter_read, my_xio_adapter_read);
        REGISTER_GLOBAL_MOCK_HOOK(xio_adapter_write, my_xio_adapter_write);

        /**
         * Or you can combine, for example, in the success case malloc will call my_gballoc_malloc, and for
         *    the failed cases, it will return NULL.
         */
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
        REGISTER_GLOBAL_MOCK_FAIL_RETURN(gballoc_malloc, NULL);
        REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
    }

    static void use_negative_mocks()
    {
        int negativeTestsInitResult = umock_c_negative_tests_init();
        negative_mocks_used = true;
        ASSERT_ARE_EQUAL(int, 0, negativeTestsInitResult);
    }

    /**
     * The test suite will call this function to cleanup your machine.
     * It is called only once, after all tests is done.
     */
    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        umock_c_deinit();

        TEST_MUTEX_DESTROY(g_testByTest);
        TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
    }

    /**
     * The test suite will call this function to prepare the machine for the new test.
     * It is called before execute each test.
     */
    TEST_FUNCTION_INITIALIZE(initialize)
    {
        if (TEST_MUTEX_ACQUIRE(g_testByTest))
        {
            ASSERT_FAIL("Could not acquire test serialization mutex.");
        }

        umock_c_reset_all_calls();
        reset_test_context_records();
        init_gballoc_checks();
    }

    /**
     * The test suite will call this function to cleanup your machine for the next test.
     * It is called after execute each test.
     */
    TEST_FUNCTION_CLEANUP(cleans)
    {
        if (negative_mocks_used)
        {
            negative_mocks_used = false;
            umock_c_negative_tests_deinit();
        }
        TEST_MUTEX_RELEASE(g_testByTest);
    }

    // Create an xio_state in the desired state
    static CONCRETE_IO_HANDLE create_in_state(XIO_STATE_ENUM state_required, bool with_unsent_messages)
    {
        // Creates in INITIAL
        CONCRETE_IO_HANDLE result;
        result = xio_state_create(&adapter_interface, &create_params);
        reset_test_context_records();
        ASSERT_IS_NOT_NULL(result);

        if (state_required >= XIO_STATE_OPENING)
        {
            // Enter XIO_STATE_OPENING
            int open_result;
            open_result = xio_state_open_async(result, on_io_open_complete, IO_OPEN_COMPLETE_CONTEXT, on_bytes_received,
                IO_BYTES_RECEIVED_CONTEXT, on_io_error, IO_ERROR_CONTEXT);
            ASSERT_ARE_EQUAL(int, open_result, 0);
            ASSERT_IO_OPEN_CALLBACK(false, IO_OPEN_OK);

            if (state_required == XIO_STATE_OPEN_ERROR)
            {
                umock_c_reset_all_calls();
                STRICT_EXPECTED_CALL(xio_adapter_open(IGNORED_PTR_ARG, on_bytes_received, IO_BYTES_RECEIVED_CONTEXT)).SetReturn(XIO_ASYNC_RESULT_FAILURE);
                xio_state_dowork(result);
                ASSERT_IO_OPEN_CALLBACK(true, IO_OPEN_ERROR);
                reset_test_context_records();
            }
            else if (state_required >= XIO_STATE_OPEN)
            {
                // Enter XIO_STATE_OPEN
                // Pump dowork until it opens
                xio_state_dowork(result);
                ASSERT_IO_OPEN_CALLBACK(true, IO_OPEN_OK);

                if (state_required == XIO_STATE_IO_ERROR)
                {
                    // Enqueue the doomed message
                    int send_result;
                    umock_c_reset_all_calls();
                    EXPECT_MESSAGE_CREATION();
                    STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, g_message_doomed_size)).SetReturn(0);
                    send_result = xio_state_send_async(result, get_message_doomed(), g_message_doomed_size,
                        on_io_send_complete, IO_SEND_COMPLETE_CONTEXT);
                    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
                    ASSERT_ARE_EQUAL(int, send_result, XIO_ASYNC_RESULT_SUCCESS);
                    ASSERT_IO_SEND_CALLBACK(false, IO_OPEN_OK);
                }

                if (with_unsent_messages)
                {
                    // Enqueue the unsent messages
                    int send_result;
                    size_t unsent_message_size = state_required == XIO_STATE_IO_ERROR ? g_message_doomed_size : g_message_1_size;
                    umock_c_reset_all_calls();
                    ASSERT_IO_SEND_CALLBACK(false, IO_OPEN_OK);
                    EXPECT_MESSAGE_CREATION();
                    STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, unsent_message_size)).SetReturn(0);
                    send_result = xio_state_send_async(result, get_message_1(), g_message_1_size,
                        on_io_send_complete, IO_SEND_COMPLETE_CONTEXT);
                    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
                    ASSERT_ARE_EQUAL(int, send_result, XIO_ASYNC_RESULT_SUCCESS);
                    EXPECT_MESSAGE_CREATION();
                    STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, unsent_message_size)).SetReturn(0);
                    send_result = xio_state_send_async(result, get_message_2(), g_message_2_size,
                        on_io_send_complete, IO_SEND_COMPLETE_CONTEXT);
                    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
                    ASSERT_ARE_EQUAL(int, send_result, XIO_ASYNC_RESULT_SUCCESS);
                    ASSERT_IO_SEND_CALLBACK(false, IO_OPEN_OK);
                }
                
                if (state_required == XIO_STATE_IO_ERROR)
                {
                    // Fail the doomed message
                    umock_c_reset_all_calls();
                    STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance)).SetReturn(XIO_ASYNC_RESULT_WAITING);
                    STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, g_message_doomed_size)).SetReturn(XIO_ASYNC_RESULT_FAILURE);
                    xio_state_dowork(result);
                    ASSERT_IO_SEND_CALLBACK(true, IO_OPEN_ERROR);
                }

                if (state_required >= XIO_STATE_CLOSING)
                {
                    int close_result;
                    umock_c_reset_all_calls();
                    STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG)).SetReturn(XIO_ASYNC_RESULT_WAITING);

                    close_result = xio_state_close_async(result, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);
                    ASSERT_ARE_EQUAL(int, close_result, 0);
                    ASSERT_IO_CLOSE_CALLBACK(false);

                    if (state_required >= XIO_STATE_CLOSED)
                    {
                        xio_state_dowork(result);
                        ASSERT_IO_CLOSE_CALLBACK(true);
                    }
                }
            }
        }
        reset_test_context_records();
        umock_c_reset_all_calls();

        return result;
    }


    /* Tests_SRS_XIO_STATE_30_050: [ If the xio_state_handle or on_close_complete parameter is NULL, 
    xio_state_close_async shall log an error and return _FAILURE_. ]*/
    /* Tests_SRS_XIO_STATE_30_054: [ On failure, the adapter shall not call on_io_close_complete. ]*/
    TEST_FUNCTION(xio_state__close_parameter_validation__fails)
    {
        int k;
        bool p0[CLOSE_PV_COUNT];
        ON_IO_CLOSE_COMPLETE p1[CLOSE_PV_COUNT];
        const char* fm[CLOSE_PV_COUNT];
        int i;

        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);


        umock_c_reset_all_calls();

        k = 0;
        p0[k] = false; p1[k] = on_io_close_complete; fm[k] = "Unexpected close success when tlsio_handle is NULL"; /* */  k++;
        p0[k] = true; p1[k] = NULL; /*           */ fm[k] = "Unexpected close success when on_io_close_complete is NULL"; k++;

        // Cycle through each failing combo of parameters
        for (i = 0; i < CLOSE_PV_COUNT; i++)
        {
            int close_result;
            reset_test_context_records();
            ///arrange

            ///act
            close_result = xio_state_close_async(p0[i] ? xio_state : NULL, p1[i], IO_CLOSE_COMPLETE_CONTEXT);

            ///assert
            ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, 0, close_result, fm[i]);
            ASSERT_IO_CLOSE_CALLBACK(false);
        }

        ///cleanup
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_009: [ The phrase "enter XIO_STATE_CLOSED" means xio_state shall
    deuque any unsent messages per Message Processing Requirements, then call the
    on_close_complete function and pass the on_close_complete_context that was supplied in xio_state_close_async. ]*/
    /* Tests_SRS_XIO_STATE_30_047: [ When xio_state dequeues a message it shall free the message's data. ]*/
    TEST_FUNCTION(xio_state__close_from_XIO_STATE_IO_ERROR_with_unsent_messages__succeeds)
    {
        int close_result;
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_IO_ERROR, true);

        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG));
        EXPECT_2_MESSAGE_DELETIONS(); // SRS_XIO_STATE_30_047: Verify message deletion per specified queue behavior
        reset_test_context_records();
        // End of arrange

        ///act
        close_result = xio_state_close_async(xio_state, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, close_result);
        ASSERT_IO_CLOSE_CALLBACK(true); // Verify the on_close_complete callback and context
        ASSERT_IO_SEND_ABANDONED(2); // 2 messages in this test
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());


        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_009: [ The phrase "enter XIO_STATE_CLOSED" means xio_state shall
    deuque any unsent messages per Message Processing Requirements, then call the
    on_close_complete function and pass the on_close_complete_context that was supplied in xio_state_close_async. ]*/
    /* Tests_SRS_XIO_STATE_30_047: [ When xio_state dequeues a message it shall free the message's data. ]*/
    TEST_FUNCTION(xio_state__close_from_XIO_STATE_OPEN_with_unsent_messages__succeeds)
    {
        int close_result;
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, true);

        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG));
        EXPECT_2_MESSAGE_DELETIONS(); // SRS_XIO_STATE_30_047: Verify message deletion per specified queue behavior
        reset_test_context_records();

        ///act
        close_result = xio_state_close_async(xio_state, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, close_result);
        ASSERT_IO_CLOSE_CALLBACK(true); // Verify the on_close_complete callback and context
        ASSERT_IO_SEND_ABANDONED(2); // 2 messages in this test
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());


        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_006: [ The phrase "enter XIO_STATE_CLOSING" means xio_state shall
    call xio_adapter_close during subsequent calls to xio_state_dowork. ]*/
    /* Tests_SRS_XIO_STATE_30_105: [ xio_state_dowork shall call xio_adapter_close on the
    xio_adapter provided during xio_state_open_async. ]*/
    /* Tests_SRS_XIO_STATE_30_106: [ If xio_adapter_close returns XIO_ASYNC_RESULT_FAILURE,
    xio_state_dowork shall log an error and enter XIO_STATE_CLOSED . ]*/
    TEST_FUNCTION(xio_state__dowork_close_from_XIO_STATE_CLOSING_with_failed_xio_adapter_close__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_CLOSING, false);
        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG)).SetReturn(XIO_ASYNC_RESULT_FAILURE);

        ///act
        xio_state_dowork(xio_state);

        ///assert
        ASSERT_IO_CLOSE_CALLBACK(true);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_006: [ The phrase "enter XIO_STATE_CLOSING" means xio_state shall
    call xio_adapter_close during subsequent calls to xio_state_dowork. ]*/
    /* Tests_SRS_XIO_STATE_30_105: [ xio_state_dowork shall call xio_adapter_close on the
    xio_adapter provided during xio_state_open_async. ]*/
    /* Tests_SRS_XIO_STATE_30_108: [ If the xio_adapter_close returns XIO_ASYNC_RESULT_WAITING, 
    xio_state_dowork shall remain in the XIO_STATE_CLOSING state. ]*/
    TEST_FUNCTION(xio_state__dowork_close_from_XIO_STATE_CLOSING_with_waiting_xio_adapter_close__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_CLOSING, false);
        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG)).SetReturn(XIO_ASYNC_RESULT_WAITING);

        ///act
        xio_state_dowork(xio_state);

        ///assert
        ASSERT_IO_CLOSE_CALLBACK(false);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_006: [ The phrase "enter XIO_STATE_CLOSING" means xio_state shall
    call xio_adapter_close during subsequent calls to xio_state_dowork. ]*/
    /* Tests_SRS_XIO_STATE_30_105: [ xio_state_dowork shall call xio_adapter_close on the
    xio_adapter provided during xio_state_open_async. ]*/
    /* Tests_SRS_XIO_STATE_30_107: [ If xio_adapter_close returns XIO_ASYNC_RESULT_SUCCESS,
    xio_state_dowork shall enter XIO_STATE_CLOSED . ]*/
    /* Tests_SRS_XIO_STATE_30_078: [ If xio_state is in XIO_STATE_CLOSING then
    xio_state_dowork shall perform only the XIO_STATE_CLOSING behaviors. ]*/
    TEST_FUNCTION(xio_state__dowork_close_from_XIO_STATE_CLOSING__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_CLOSING, false);
        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG));

        ///act
        xio_state_dowork(xio_state);

        ///assert
        ASSERT_IO_CLOSE_CALLBACK(true);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_055: [ If xio_state is in XIO_STATE_CLOSING
    then xio_state_close_async shall do nothing and return 0. ]*/
    TEST_FUNCTION(xio_state__close_from_XIO_STATE_CLOSING__succeeds)
    {
        int close_result;
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_CLOSING, false);

        ///act
        close_result = xio_state_close_async(xio_state, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, close_result);
        ASSERT_IO_CLOSE_CALLBACK(false);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_059: [ If xio_state is in XIO_STATE_CLOSED 
    then xio_state_close_async shall do nothing and return 0. ]*/
    TEST_FUNCTION(xio_state__close_from_XIO_STATE_CLOSED__succeeds)
    {
        int close_result;
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_CLOSED, false);

        ///act
        close_result = xio_state_close_async(xio_state, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, close_result);
        ASSERT_IO_CLOSE_CALLBACK(false);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_053: [ If xio_state is in XIO_STATE_INITIAL 
    xio_state_close_async shall log an error and return _FAILURE_. ]*/
    TEST_FUNCTION(xio_state__close_from_XIO_STATE_INITIAL__fails)
    {
        int close_result;
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_INITIAL, false);

        ///act
        close_result = xio_state_close_async(xio_state, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_NOT_EQUAL(int, 0, close_result);
        ASSERT_IO_CLOSE_CALLBACK(false);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_063: [ On success, xio_state_send_async shall enqueue for
    transmission the on_send_complete, the callback_context, the size, and the buffer per
    Message Processing Requirements and then return XIO_ASYNC_RESULT_SUCCESS. ]*/
    /* Tests_SRS_XIO_STATE_30_040: [ When xio_state enqueues a message it shall make
    a copy of the data supplied in xio_state_send_async. ]*/
    /* Tests_SRS_XIO_STATE_30_047: [ When xio_state dequeues a message it shall free 
    the message's data. ]*/
    /* Tests_SRS_XIO_STATE_30_044: [ If a message was sent successfully, then
    after it is dequeued xio_state shall call the message's on_send_complete
    along with its associated callback_context and IO_SEND_OK. ]*/
    /* Tests_SRS_XIO_STATE_30_061: [ On success, after enqueuing the message, 
    xio_state_send_async shall invoke the Data Transmission behavior of xio_state_dowork. ]*/
    TEST_FUNCTION(xio_state__send__succeeds)
    {
        int send_result;
        CONCRETE_IO_HANDLE xio_state;
        ///arrange
        xio_state = create_in_state(XIO_STATE_OPEN, false);

        const char* message = get_message_1();
        reset_test_context_records();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // PENDING_SOCKET_IO
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // message bytes per SRS_XIO_STATE_30_040
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // singlylinkedlist_add
        STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, g_message_1_size));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));  // PENDING_SOCKET_IO
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));  // message bytes per SRS_XIO_STATE_30_047
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));  // singlylinkedlist_add

        ///act
        send_result = xio_state_send_async(xio_state, message,
            g_message_1_size, on_io_send_complete, IO_SEND_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL_WITH_MSG(int, send_result, XIO_ASYNC_RESULT_SUCCESS, "Unexpected send failure");
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_MESSAGE_1_SENT_BY_COPY();
        ASSERT_IO_SEND_CALLBACK(true, IO_SEND_OK);

        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_064: [ If the supplied message cannot be enqueued for transmission,
    xio_state_send_async shall log an error and return XIO_ASYNC_RESULT_FAILURE. ]*/
    /* Tests_SRS_XIO_STATE_30_066: [ On failure, on_send_complete shall not be called. ]*/
    TEST_FUNCTION(xio_state__send_unhappy_paths__fails)
    {
        size_t i;
        use_negative_mocks();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // PENDING_TRANSMISSION
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // message bytes
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  // singlylinkedlist_add
        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            int send_result;

            ///arrange
            CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);
            reset_test_context_records();

            umock_c_reset_all_calls();

            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            send_result = xio_state_send_async(xio_state, get_message_1(),
                g_message_1_size, on_io_send_complete, IO_SEND_COMPLETE_CONTEXT);

            ///assert
            ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, send_result, 0, "Unexpected send success on unhappy path");
            ASSERT_IO_SEND_CALLBACK(false, IO_SEND_ERROR);

            ///cleanup
            xio_state_close_async(xio_state, on_io_close_complete, NULL);
            xio_state_destroy(xio_state);
        }

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_060: [ If any of the xio_state_handle, buffer, or on_send_complete 
    parameters is NULL, xio_state_send_async shall log an error and return XIO_ASYNC_RESULT_FAILURE. ]*/
    /* Tests_SRS_XIO_STATE_30_067: [ If the size is 0, xio_state_send_async shall log an error and return XIO_ASYNC_RESULT_FAILURE. ]*/
    /* Tests_SRS_XIO_STATE_30_066: [ On failure, on_send_complete shall not be called. ]*/
    TEST_FUNCTION(xio_state__send_parameter_validation__fails)
    {
        // Parameters arrays
        bool p0[SEND_PV_COUNT];
        const void* p1[SEND_PV_COUNT];
        size_t p2[SEND_PV_COUNT];
        ON_SEND_COMPLETE p3[SEND_PV_COUNT];
        const char* fm[SEND_PV_COUNT];
        int k;
        int i;

        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);

        k = 0;
        p0[k] = false; p1[k] = get_message_1(); p2[k] = g_message_1_size; p3[k] = on_io_send_complete; fm[k] = "Unexpected send success when tlsio_handle is NULL"; k++;
        p0[k] = true; p1[k] = NULL; /*       */ p2[k] = g_message_1_size; p3[k] = on_io_send_complete; fm[k] = "Unexpected send success when send buffer is NULL"; k++;
        p0[k] = true; p1[k] = get_message_1(); p2[k] = 0; /*           */ p3[k] = on_io_send_complete; fm[k] = "Unexpected send success when size is 0"; k++;
        p0[k] = true; p1[k] = get_message_1(); p2[k] = g_message_1_size; p3[k] = NULL; /*           */ fm[k] = "Unexpected send success when on_send_complete is NULL"; k++;

        // Cycle through each failing combo of parameters
        for (i = 0; i < SEND_PV_COUNT; i++)
        {
            int send_result;
            ///arrange
            reset_test_context_records();

            ///act
            send_result = xio_state_send_async(p0[i] ? xio_state : NULL, p1[i], p2[i], p3[i], IO_SEND_COMPLETE_CONTEXT);

            ///assert
            ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, send_result, 0, fm[i]);
            ASSERT_IO_SEND_CALLBACK(false, IO_SEND_ERROR);

            ///cleanup
        }

        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_065: [ If the adapter state is not XIO_STATE_STATE_EXT_OPEN, 
    xio_state_send_async shall log an error and return _FAILURE_. ]*/
    /* Tests_SRS_XIO_STATE_30_066: [ On failure, on_send_complete shall not be called. ]*/
    TEST_FUNCTION(xio_state__send_wrong_state__fails)
    {
        size_t i;
        XIO_STATE_ENUM states[] =
        {
            XIO_STATE_INITIAL,
            XIO_STATE_OPENING,
            XIO_STATE_IO_ERROR,
            XIO_STATE_CLOSING,
            XIO_STATE_CLOSED,
            XIO_STATE_OPEN_ERROR
        };

        for (i = 0; i < sizeof(states) / sizeof(XIO_STATE_ENUM); i++)
        {
            int send_result;
            ///arrange
            CONCRETE_IO_HANDLE result = create_in_state(states[i], false);

            ///act
            send_result = xio_state_send_async(result, get_message_1(),
                g_message_1_size, on_io_send_complete, IO_SEND_COMPLETE_CONTEXT);

            ///assert
            ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, send_result, 0, "Unexpected success in sending from wrong state");
            ASSERT_IO_SEND_CALLBACK(false, IO_SEND_ERROR);

            ///cleanup
            xio_state_destroy(result);
            assert_gballoc_checks();
        }
    }

    /* Tests_SRS_XIO_STATE_30_047: [ When xio_state dequeues a message it shall free the message's data. ]*/
    /* Tests_SRS_XIO_STATE_30_005: [ The phrase "enter XIO_STATE_OPEN_ERROR" means xio_state shall 
    call the on_io_error function and pass the on_io_error_context that was supplied in 
    xio_state_open_async. ]*/
    /* Tests_SRS_XIO_STATE_30_095: [ If the send process fails before sending all of the bytes
    in an enqueued message, xio_state_dowork shall deque the message per
    Message Processing Requirements and enter XIO_STATE_OPEN_ERROR. ]*/
    /* Tests_SRS_XIO_STATE_30_045: [ If a message was not sent successfully, then after 
    it is dequeued xio_state shall call the message's on_send_complete along with 
    its associated callback_context and IO_SEND_ERROR. ]*/
    TEST_FUNCTION(xio_state__dowork_send_unhappy_path__fails)
    {
        int send_result;
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);

        STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, SSL_SHORT_SENT_MESSAGE_SIZE)).SetReturn(0);
        send_result = xio_state_send_async(xio_state, SSL_send_buffer,
            SSL_SHORT_SENT_MESSAGE_SIZE, on_io_send_complete, IO_SEND_COMPLETE_CONTEXT);
        ASSERT_ARE_EQUAL(int, send_result, XIO_ASYNC_RESULT_SUCCESS);

        reset_test_context_records();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance)).SetReturn(XIO_ASYNC_RESULT_WAITING);
        STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, SSL_SHORT_SENT_MESSAGE_SIZE)).SetReturn(XIO_ASYNC_RESULT_FAILURE);
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

        ///act
        xio_state_dowork(xio_state);

        ///assert
        ASSERT_IO_SEND_CALLBACK(true, IO_SEND_ERROR);
        ASSERT_IO_ERROR_CALLBACK(true);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_093: [ If xio_adapter_write was not able to send 
    an entire enqueued message at once, subsequent calls to xio_state_dowork 
    shall continue to send the remaining bytes. ]*/
    TEST_FUNCTION(xio_state__dowork_send_big_message__succeeds)
    {
        int send_result;
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);

        // Don't let the async_write's call to dowork_send consume the message
        STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, SSL_TEST_MESSAGE_SIZE)).SetReturn(0);

        send_result = xio_state_send_async(xio_state, SSL_send_buffer,
            SSL_TEST_MESSAGE_SIZE, on_io_send_complete, IO_SEND_COMPLETE_CONTEXT);
        ASSERT_ARE_EQUAL(int, send_result, XIO_ASYNC_RESULT_SUCCESS);
        ASSERT_IO_ERROR_CALLBACK(false);

        reset_test_context_records();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance)).SetReturn(XIO_ASYNC_RESULT_WAITING);
        STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, SSL_TEST_MESSAGE_SIZE));

        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance)).SetReturn(XIO_ASYNC_RESULT_WAITING);
        STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, SSL_TEST_MESSAGE_SIZE - SSL_WRITE_MAX_TEST_SIZE));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        xio_state_dowork(xio_state);
        ASSERT_IO_ERROR_CALLBACK(false);

        ///act
        xio_state_dowork(xio_state);

        ///assert
        ASSERT_IO_SEND_CALLBACK(true, IO_SEND_OK);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IO_ERROR_CALLBACK(false);

        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_091: [ If xio_adapter_write is able to send all the bytes in the 
    enqueued message, xio_state_dowork shall deque the message per Message Processing Requirements. ]*/
    /* Tests_SRS_XIO_STATE_30_047: [ When xio_state dequeues a message it shall free the message's data. ]*/
    /* Tests_SRS_XIO_STATE_30_077: [ If xio_state is in XIO_STATE_OPEN then xio_state_dowork shall
    perform only the Data transmission behaviors and the Data reception behaviors. ]*/
    /* Tests_SRS_XIO_STATE_30_090: [ If there are any unsent messages in the queue, xio_state_dowork 
    shall call xio_adapter_write on the xio_adapter and pass in the first message in the queue. ]*/
    TEST_FUNCTION(xio_state__dowork_send__succeeds)
    {
        int send_result;
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);

        // Don't let the async_write's call to dowork_send consume the message
        STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, SSL_SHORT_SENT_MESSAGE_SIZE)).SetReturn(0);

        send_result = xio_state_send_async(xio_state, SSL_send_buffer,
            SSL_SHORT_SENT_MESSAGE_SIZE, on_io_send_complete, IO_SEND_COMPLETE_CONTEXT);
        ASSERT_ARE_EQUAL(int, send_result, XIO_ASYNC_RESULT_SUCCESS);
        ASSERT_IO_ERROR_CALLBACK(false);

        reset_test_context_records();
        umock_c_reset_all_calls();

        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance)).SetReturn(XIO_ASYNC_RESULT_WAITING);
        STRICT_EXPECTED_CALL(xio_adapter_write(&adapter_instance, IGNORED_PTR_ARG, SSL_SHORT_SENT_MESSAGE_SIZE));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

        ///act
        xio_state_dowork(xio_state);

        ///assert
        ASSERT_IO_SEND_CALLBACK(true, IO_SEND_OK);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IO_ERROR_CALLBACK(false);

        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_096: [ If there are no enqueued messages available, xio_state_dowork shall do nothing. ]*/
    TEST_FUNCTION(xio_state__dowork_send_empty_queue__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);

        // We do expect an empty read when we call dowork
        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance)).SetReturn(XIO_ASYNC_RESULT_WAITING);

        ///act
        xio_state_dowork(xio_state);

        ///assert
        // Verify we got no callback for 0 messages
        ASSERT_IO_SEND_CALLBACK(false, IO_SEND_OK);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IO_ERROR_CALLBACK(false);

        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_102: [ If the xio_adapter returns XIO_ASYNC_RESULT_FAILURE 
    then xio_state_dowork shall enter XIO_STATE_OPEN_ERROR. ] */
    TEST_FUNCTION(xio_state__dowork_receive_unhappy_path__fails)
    {
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);

        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance)).SetReturn(XIO_ASYNC_RESULT_FAILURE);

        ///act
        xio_state_dowork(xio_state);

        ///assert
        // Verify we got no callback for 0 bytes
        ASSERT_BYTES_RECEIVED_CALLBACK(false, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IO_ERROR_CALLBACK(true);

        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_101: [ If the xio_adapter returns XIO_ASYNC_RESULT_WAITING, xio_state_dowork shall do nothing. ]*/
    TEST_FUNCTION(xio_state__dowork_receive_no_data__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);

        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance)).SetReturn(XIO_ASYNC_RESULT_WAITING);

        ///act
        xio_state_dowork(xio_state);

        ///assert
        // Verify we got no callback for 0 bytes
        ASSERT_BYTES_RECEIVED_CALLBACK(false, 0);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IO_ERROR_CALLBACK(false);

        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_100: [ The xio_state_dowork shall repeatedly call xio_adapter_read on the 
    underlying xio_adapter until the return value is not XIO_ASYNC_RESULT_SUCCESS. ]*/
    /* Tests_SRS_XIO_STATE_30_103: [ If the xio_adapter_read returns returns data, it shall do so with the
    on_received callback and on_received_context provided in xio_state_open. ]*/
    /* Tests_SRS_XIO_STATE_30_077: [ If xio_state is in XIO_STATE_OPEN then xio_state_dowork shall 
    perform only the Data transmission behaviors and the Data reception behaviors. ]*/
    TEST_FUNCTION(xio_state__dowork_receive__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE xio_state = create_in_state(XIO_STATE_OPEN, false);

        // Two successes plus an XIO_ASYNC_RESULT_WAITING
        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance));
        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance));
        STRICT_EXPECTED_CALL(xio_adapter_read(&adapter_instance)).SetReturn(XIO_ASYNC_RESULT_WAITING);

        ///act
        xio_state_dowork(xio_state);

        ///assert
        // Verify we got the bytes and their callback context
        ASSERT_BYTES_RECEIVED_CALLBACK(true, 2);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        ASSERT_IO_ERROR_CALLBACK(false);

        ///cleanup
        xio_state_close_async(xio_state, on_io_close_complete, NULL);
        xio_state_destroy(xio_state);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_009: [ The phrase "enter XIO_STATE_CLOSED" means xio_state shall
    deuque any unsent messages per Message Processing Requirements, then call the
    on_close_complete function and pass the on_close_complete_context that was supplied in xio_state_close_async. ]*/
    /* Tests_SRS_XIO_STATE_30_058: [ xio_state_close_async shall call xio_adapter_close on its adapter. ]*/
    /* Tests_SRS_XIO_STATE_30_052: [ If xio_adapter_close returns XIO_ASYNC_RESULT_FAILURE, 
    xio_state_close_async shall enter XIO_STATE_CLOSED and return 0. ]*/
    TEST_FUNCTION(xio_state__close_from_XIO_STATE_OPEN_with_failed_xio_adapter_close__succeeds)
    {
        CONCRETE_IO_HANDLE create_result;
        int close_result;
        ///arrange
        create_result = create_in_state(XIO_STATE_OPEN, false);

        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG)).SetReturn(XIO_ASYNC_RESULT_FAILURE);
        // End of arrange

        ///act
        close_result = xio_state_close_async(create_result, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, close_result);
        ASSERT_IO_CLOSE_CALLBACK(true);

        ///cleanup
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_009: [ The phrase "enter XIO_STATE_CLOSED" means xio_state shall
    deuque any unsent messages per Message Processing Requirements, then call the
    on_close_complete function and pass the on_close_complete_context that was supplied in xio_state_close_async. ]*/
    /* Tests_SRS_XIO_STATE_30_058: [ xio_state_close_async shall call xio_adapter_close on its adapter. ]*/
    /* Tests_SRS_XIO_STATE_30_056: [ If xio_adapter_close returns XIO_ASYNC_RESULT_WAITING, 
    xio_state_close_async shall enter XIO_STATE_CLOSING and return 0. ]*/
    TEST_FUNCTION(xio_state__close_from_XIO_STATE_OPEN_with_xio_adapter_waiting__succeeds)
    {
        CONCRETE_IO_HANDLE create_result;
        int close_result;
        ///arrange
        create_result = create_in_state(XIO_STATE_OPEN, false);
        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG)).SetReturn(XIO_ASYNC_RESULT_WAITING);

        ///act
        close_result = xio_state_close_async(create_result, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, close_result);
        ASSERT_IO_CLOSE_CALLBACK(false);

        ///cleanup
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_009: [ The phrase "enter XIO_STATE_CLOSED" means xio_state shall
    deuque any unsent messages per Message Processing Requirements, then call the
    on_close_complete function and pass the on_close_complete_context that was supplied in xio_state_close_async. ]*/
    /* Tests_SRS_XIO_STATE_30_058: [ xio_state_close_async shall call xio_adapter_close on its adapter. ]*/
    /* Tests_SRS_XIO_STATE_30_051: [ If xio_adapter_close returns XIO_ASYNC_RESULT_SUCCESS or
    XIO_ASYNC_RESULT_FAILURE, xio_state_close_async shall enter XIO_STATE_CLOSED and return 0. ]*/
    TEST_FUNCTION(xio_state__close_from_XIO_STATE_OPEN__succeeds)
    {
        CONCRETE_IO_HANDLE create_result;
        int close_result;
        ///arrange
        create_result = create_in_state(XIO_STATE_OPEN, false);

        ///act
        close_result = xio_state_close_async(create_result, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, close_result);
        ASSERT_IO_CLOSE_CALLBACK(true);

        ///cleanup
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_057: [ If xio_state is in XIO_STATE_OPENING then 
    xio_state_close_async shall shall call on_open_complete with the 
    on_open_complete_context supplied in xio_state_open_async and IO_OPEN_CANCELLED. ]*/
    TEST_FUNCTION(xio_state__close_from_XIO_STATE_OPENING__succeeds)
    {
        CONCRETE_IO_HANDLE create_result;
        int close_result;
        ///arrange
        create_result = create_in_state(XIO_STATE_OPENING, false);

        ///act
        close_result = xio_state_close_async(create_result, on_io_close_complete, IO_CLOSE_COMPLETE_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, close_result);
        ASSERT_IO_OPEN_CALLBACK(true, IO_OPEN_CANCELLED);
        ASSERT_IO_CLOSE_CALLBACK(true);

        ///cleanup
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_082: [ If the xio_adapter returns XIO_ASYNC_RESULT_FAILURE, xio_state_dowork
    shall log an error, call on_open_complete with the on_open_complete_context parameter provided in
    xio_state_open_async and IO_OPEN_ERROR, and enter XIO_STATE_CLOSED. ]*/
    TEST_FUNCTION(xio_state__dowork_open__fails)
    {
        CONCRETE_IO_HANDLE create_result;
        ///arrange
        create_result = create_in_state(XIO_STATE_OPENING, false);

        STRICT_EXPECTED_CALL(xio_adapter_open(IGNORED_PTR_ARG, on_bytes_received, 
            IO_BYTES_RECEIVED_CONTEXT)).SetReturn(XIO_ASYNC_RESULT_FAILURE);

        ///act
        xio_state_dowork(create_result);

        ///assert
        // Check that we got the on_open callback
        ASSERT_IO_OPEN_CALLBACK(true, IO_OPEN_ERROR);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_close_async(create_result, on_io_close_complete, NULL);
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_081: [ If the xio_endpoint returns XIO_ASYNC_RESULT_WAITING,
    xio_state_dowork shall remain in the XIO_STATE_STATE_EXT_OPENING state. ]*/
    TEST_FUNCTION(xio_state__dowork_open__waiting)
    {
        CONCRETE_IO_HANDLE create_result;
        ///arrange
        create_result = create_in_state(XIO_STATE_OPENING, false);

        STRICT_EXPECTED_CALL(xio_adapter_open(IGNORED_PTR_ARG, on_bytes_received, 
            IO_BYTES_RECEIVED_CONTEXT)).SetReturn(XIO_ASYNC_RESULT_WAITING);

        ///act
        xio_state_dowork(create_result);

        ///assert
        // Check that we got the on_open callback
        ASSERT_IO_OPEN_CALLBACK(false, IO_OPEN_OK);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_close_async(create_result, on_io_close_complete, NULL);
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_080: [ The xio_state_dowork shall call xio_adapter_open on the underlying xio_adapter. ]*/
    /* Tests_SRS_XIO_STATE_30_083: [ If xio_adapter returns XIO_ASYNC_RESULT_SUCCESS, xio_state_dowork shall enter XIO_STATE_OPEN. ]*/
    /* Tests_SRS_XIO_STATE_30_007: [ The phrase "enter XIO_STATE_OPEN" means xio_state shall call the 
    on_open_complete function and pass IO_OPEN_OK and the on_open_complete_context that was supplied in xio_state_open_async. ]*/
    /* Tests_SRS_XIO_STATE_30_076: [ If xio_state is in XIO_STATE_OPENING then xio_state_dowork 
    shall perform only the XIO_STATE_OPENING behaviors. ]*/
    TEST_FUNCTION(xio_state__dowork_open__succeeds)
    {
        CONCRETE_IO_HANDLE create_result;
        ///arrange
        create_result = create_in_state(XIO_STATE_OPENING, false);

        STRICT_EXPECTED_CALL(xio_adapter_open(IGNORED_PTR_ARG, on_bytes_received, IO_BYTES_RECEIVED_CONTEXT));

        ///act
        xio_state_dowork(create_result);

        ///assert
        // Check that we got the on_open callback
        ASSERT_IO_OPEN_CALLBACK(true, IO_OPEN_OK);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_close_async(create_result, on_io_close_complete, NULL);
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_075: [ If the adapter is in XIO_STATE_STATE_EXT_CLOSED then xio_state_dowork shall do nothing. ]*/
    TEST_FUNCTION(xio_state__dowork_XIO_STATE_CLOSED__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE create_result = create_in_state(XIO_STATE_CLOSED, false);

        ///act
        xio_state_dowork(create_result);

        ///assert
        ASSERT_NO_CALLBACKS();
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_071: [ If xio_state is in XIO_STATE_ERROR then xio_state_dowork shall do nothing. ]*/
    TEST_FUNCTION(xio_state__dowork_XIO_STATE_ERROR__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE create_result = create_in_state(XIO_STATE_OPEN_ERROR, false);

        ///act
        xio_state_dowork(create_result);

        ///assert
        ASSERT_NO_CALLBACKS();
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_072: [ If xio_state is in XIO_STATE_INITIAL then xio_state_dowork shall do nothing. ]*/
    TEST_FUNCTION(xio_state__dowork_XIO_STATE_INITIAL__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE create_result = create_in_state(XIO_STATE_INITIAL, false);

        ///act
        xio_state_dowork(create_result);

        ///assert
        ASSERT_NO_CALLBACKS();
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_070: [ If the xio_state_handle parameter is NULL, 
    xio_state_dowork shall do nothing except log an error. ]*/
    TEST_FUNCTION(xio_state__dowork_parameter_validation__fails)
    {
        ///arrange
        reset_test_context_records();

        ///act
        xio_state_dowork(NULL);

        ///assert
        ASSERT_NO_CALLBACKS();

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_120: [ If any of the the xio_state_handle, optionName, or value parameters is NULL, 
    xio_state_setoption shall do nothing except log an error and return _FAILURE_. */
    TEST_FUNCTION(xio_state__setoption_parameter_validation__fails)
    {
        int k;
        // Parameters arrays
        bool p0[SETOPTION_PV_COUNT];
        const char* p1[SETOPTION_PV_COUNT];
        const char*  p2[SETOPTION_PV_COUNT];
        const char* fm[SETOPTION_PV_COUNT]; 
        int i;

        umock_c_reset_all_calls();

        k = 0;
        p0[k] = false; p1[k] = "fake name"; p2[k] = "fake value"; fm[k] = "Unexpected setoption success when tlsio_handle is NULL"; /* */  k++;
        p0[k] = true; p1[k] = NULL; /*   */ p2[k] = "fake value"; fm[k] = "Unexpected setoption success when option_name is NULL"; /*  */  k++;
        p0[k] = true; p1[k] = "fake name"; p2[k] = NULL; /*    */ fm[k] = "Unexpected setoption success when option_value is NULL"; /* */  k++;


        // Cycle through each failing combo of parameters
        for (i = 0; i < SETOPTION_PV_COUNT; i++)
        {
            int result;
            ///arrange
            CONCRETE_IO_HANDLE create_result = create_in_state(XIO_STATE_INITIAL, false);

            ///act
            result = xio_state_setoption(p0[i] ? create_result : NULL, p1[i], p2[i]);

            ///assert
            ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, 0, result, fm[i]);

            ///cleanup
            xio_state_destroy(create_result);
        }
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_121 [ xio_state shall delegate the behavior of 
    xio_state_setoption to the xio_adapter supplied in xio_state_create. ]*/
    TEST_FUNCTION(xio_state__setoption__succeeds)
    {
        int result;
        const char* fake_name = "fake name";
        const char* fake_value = "fake_value";
        ///arrange
        CONCRETE_IO_HANDLE create_result = create_in_state(XIO_STATE_INITIAL, false);

        STRICT_EXPECTED_CALL(xio_adapter_setoption(&adapter_instance, fake_name, fake_value));

        ///act
        result = xio_state_setoption(create_result, fake_name, fake_value);

        ///assert
        ASSERT_ARE_EQUAL(int, 0, result);

        ///cleanup
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_160: [ If the xio_state_handle parameter is NULL, xio_state_retrieveoptions 
    shall do nothing except log an error and return _FAILURE_. ]*/
    TEST_FUNCTION(xio_state__retrieveoptions_parameter_validation__fails)
    {
        ///arrange

        ///act
        OPTIONHANDLER_HANDLE result = xio_state_retrieveoptions(NULL);

        ///assert
        ASSERT_IS_NULL((void*)result);

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_161: [ xio_state shall delegate the behavior of 
    xio_state_retrieveoptions to the xio_adapter_config supplied in xio_state_create. ]*/
    TEST_FUNCTION(xio_state__retrieveoptions__succeeds)
    {
        OPTIONHANDLER_HANDLE result;
        ///arrange
        CONCRETE_IO_HANDLE create_result = create_in_state(XIO_STATE_INITIAL, false);

        STRICT_EXPECTED_CALL(xio_adapter_retrieveoptions(&adapter_instance));

        ///act
        result = xio_state_retrieveoptions(create_result);

        ///assert
        ASSERT_ARE_EQUAL(int, ((int)result), ((int)fake_option_handler));
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(create_result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_037: [ If xio_state is in any state other than XIO_STATE_INITIAL when 
    xio_state_open_async is called, it shall log an error, and return _FAILURE_. ]*/
    /* Tests_SRS_XIO_STATE_30_039: [ On failure, xio_state_open_async shall not call on_open_complete. ]*/
    TEST_FUNCTION(xio_state__open_wrong_state__fails)
    {
        size_t i;
        XIO_STATE_ENUM states[] = 
        { 
            XIO_STATE_OPENING,
            XIO_STATE_OPEN,
            XIO_STATE_IO_ERROR,
            XIO_STATE_CLOSING,
            XIO_STATE_CLOSED,
            XIO_STATE_OPEN_ERROR
        };

        for (i = 0; i < sizeof(states) / sizeof(XIO_STATE_ENUM); i++)
        {
            int open_result_2;
            ///arrange
            CONCRETE_IO_HANDLE result = create_in_state(states[i], false);

            ///act
            open_result_2 = xio_state_open_async(result, on_io_open_complete, IO_OPEN_COMPLETE_CONTEXT, on_bytes_received,
                IO_BYTES_RECEIVED_CONTEXT, on_io_error, IO_ERROR_CONTEXT);

            ///assert
            ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, open_result_2, 0, "Unexpected open success");
            ASSERT_IO_OPEN_CALLBACK(false, IO_OPEN_ERROR);

            ///cleanup
            xio_state_destroy(result);
            assert_gballoc_checks();
        }
    }

    /* Tests_SRS_XIO_STATE_30_030: [ If any of the xio_state_handle, on_open_complete, on_bytes_received, 
    or on_io_error parameters is NULL, xio_state_open_async shall log an error and return _FAILURE_. ]*/
    TEST_FUNCTION(xio_state__open_parameter_validation_fails__fails)
    {
        // Parameters arrays
        bool p0[OPEN_PV_COUNT];
        ON_IO_OPEN_COMPLETE p1[OPEN_PV_COUNT];
        ON_BYTES_RECEIVED p2[OPEN_PV_COUNT];
        ON_IO_ERROR p3[OPEN_PV_COUNT];
        const char* fm[OPEN_PV_COUNT];
        int i;

        int k = 0;
        p0[k] = false; p1[k] = on_io_open_complete; p2[k] = on_bytes_received; p3[k] = on_io_error; fm[k] = "Unexpected open success when tlsio_handle is NULL"; /* */  k++;
        p0[k] = true; p1[k] = NULL; /*           */ p2[k] = on_bytes_received; p3[k] = on_io_error; fm[k] = "Unexpected open success when on_io_open_complete is NULL"; k++;
        p0[k] = true; p1[k] = on_io_open_complete; p2[k] = NULL; /*         */ p3[k] = on_io_error; fm[k] = "Unexpected open success when on_bytes_received is NULL"; k++;
        p0[k] = true; p1[k] = on_io_open_complete; p2[k] = on_bytes_received;  p3[k] = NULL; /*  */ fm[k] = "Unexpected open success when on_io_error is NULL"; /*   */ k++;

        // Cycle through each failing combo of parameters
        for (i = 0; i < OPEN_PV_COUNT; i++)
        {
            CONCRETE_IO_HANDLE result;
            int open_result;
            ///arrange
            result = create_in_state(XIO_STATE_INITIAL, false);

            ///act
            open_result = xio_state_open_async(p0[i] ? result : NULL, p1[i], IO_OPEN_COMPLETE_CONTEXT, p2[i],
                IO_BYTES_RECEIVED_CONTEXT, p3[i], IO_ERROR_CONTEXT);

            ///assert
            ASSERT_ARE_NOT_EQUAL_WITH_MSG(int, open_result, 0, fm[i]);
            ASSERT_IO_OPEN_CALLBACK(false, IO_OPEN_ERROR);

            ///cleanup
            xio_state_destroy(result);
        }
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_035: [ On success, xio_state_open_async shall 
    cause the adapter to enter XIO_STATE_STATE_EXT_OPENING and return 0. ]*/
    TEST_FUNCTION(xio_state__open_async__succeeds)
    {
        int open_result;
        ///arrange
        CONCRETE_IO_HANDLE result = create_in_state(XIO_STATE_INITIAL, false);

        ///act
        open_result = xio_state_open_async(result, on_io_open_complete, IO_OPEN_COMPLETE_CONTEXT, on_bytes_received,
            IO_BYTES_RECEIVED_CONTEXT, on_io_error, IO_ERROR_CONTEXT);

        ///assert
        ASSERT_ARE_EQUAL(int, open_result, 0);
        // Should not have made any callbacks yet
        ASSERT_IO_OPEN_CALLBACK(false, IO_OPEN_ERROR);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        xio_state_destroy(result);
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_022: [ If xio_state is in any state other than XIO_STATE_CLOSED
    when xio_state_destroy is called, it shall call xio_adapter_close then enter XIO_STATE_CLOSED
    before completing the destroy process. ]*/
    /* Tests_SRS_XIO_STATE_30_009: [ The phrase "enter XIO_STATE_CLOSED" means xio_state 
    shall deuque any unsent messages per Message Processing Requirements, then call the 
    on_close_complete function and pass the on_close_complete_context that was supplied 
    in xio_state_close_async. ]*/
    TEST_FUNCTION(xio_state__destroy_with_unsent_messages__succeeds)
    {
        CONCRETE_IO_HANDLE xio_state;
        ///arrange
        xio_state = create_in_state(XIO_STATE_OPEN, true);
        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG));
        EXPECT_2_MESSAGE_DELETIONS();

        STRICT_EXPECTED_CALL(xio_adapter_destroy(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG));
        // End of arrange

        ///act
        xio_state_destroy(xio_state);

        ///assert
        ASSERT_IO_SEND_ABANDONED(2); // 2 messages in this test
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_021: [ The xio_state_destroy call shall release all allocated resources,
    call xio_adapter_destroy, and then release xio_state_handle. ]*/
    TEST_FUNCTION(xio_state__destroy_XIO_STATE_CLOSED__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE result = create_in_state(XIO_STATE_CLOSED, false);

        STRICT_EXPECTED_CALL(xio_adapter_destroy(&adapter_instance));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

        ///act
        xio_state_destroy(result);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_021: [ The xio_state_destroy call shall release all allocated resources,
    call xio_adapter_destroy, and then release xio_state_handle. ]*/
    /* Tests_SRS_XIO_STATE_30_025: [ If xio_state is in XIO_STATE_OPEN_ERROR, xio_state_destroy shall
    call xio_adapter_close then enter XIO_STATE_CLOSED before releasing resources. ]*/
    TEST_FUNCTION(xio_state__destroy_XIO_STATE_OPEN_ERROR__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE result = create_in_state(XIO_STATE_CLOSING, false);

        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(xio_adapter_destroy(&adapter_instance));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

        ///act
        xio_state_destroy(result);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_021: [ The xio_state_destroy call shall release all allocated resources,
    call xio_adapter_destroy, and then release xio_state_handle. ]*/
    /* Tests_SRS_XIO_STATE_30_024: [ If xio_state is in XIO_STATE_CLOSING, xio_state_destroy shall
    call xio_adapter_close then enter XIO_STATE_CLOSED before releasing resources. ]*/
    TEST_FUNCTION(xio_state__destroy_XIO_STATE_CLOSING__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE result = create_in_state(XIO_STATE_CLOSING, false);

        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(xio_adapter_destroy(&adapter_instance));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

        ///act
        xio_state_destroy(result);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_021: [ The xio_state_destroy call shall release all allocated resources,
    call xio_adapter_destroy, and then release xio_state_handle. ]*/
    /* Tests_SRS_XIO_STATE_30_023: [ If xio_state is in XIO_STATE_OPEN, xio_state_destroy shall
    call xio_adapter_close then enter XIO_STATE_CLOSED before releasing resources. ]*/
    TEST_FUNCTION(xio_state__destroy_XIO_STATE_OPEN__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE result = create_in_state(XIO_STATE_OPEN, false);

        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(xio_adapter_destroy(&adapter_instance));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

        ///act
        xio_state_destroy(result);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_021: [ The xio_state_destroy call shall release all allocated resources,
    call xio_adapter_destroy, and then release xio_state_handle. ]*/
    /* Tests_SRS_XIO_STATE_30_022: [ If xio_state is in XIO_STATE_OPENING, xio_state_destroy shall
    call xio_adapter_close then enter XIO_STATE_CLOSED before releasing resources. ]*/
    TEST_FUNCTION(xio_state__destroy_XIO_STATE_OPENING__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE result = create_in_state(XIO_STATE_OPENING, false);

        STRICT_EXPECTED_CALL(xio_adapter_close(IGNORED_PTR_ARG));
        STRICT_EXPECTED_CALL(xio_adapter_destroy(&adapter_instance));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

        ///act
        xio_state_destroy(result);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_021: [ The xio_state_destroy call shall release all allocated resources,
    call xio_adapter_destroy, and then release xio_state_handle. ]*/
    TEST_FUNCTION(xio_state__destroy_XIO_STATE_INITIAL__succeeds)
    {
        ///arrange
        CONCRETE_IO_HANDLE result = create_in_state(XIO_STATE_INITIAL, false);

        STRICT_EXPECTED_CALL(xio_adapter_destroy(&adapter_instance));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_free(IGNORED_NUM_ARG));

        ///act
        xio_state_destroy(result);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_020: [ If tlsio_handle is NULL, tlsio_destroy shall do nothing. ]*/
    TEST_FUNCTION(xio_state__destroy_NULL__succeeds)
    {
        ///arrange

        ///act
        xio_state_destroy(NULL);

        ///assert
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_011: [ If any resource allocation fails, xio_state_create shall return NULL. ]*/
    TEST_FUNCTION(xio_state__create_unhappy_paths__fails)
    {
        size_t i;
        use_negative_mocks();

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));
        STRICT_EXPECTED_CALL(xio_adapter_create(&create_params));
        umock_c_negative_tests_snapshot();

        for (i = 0; i < umock_c_negative_tests_call_count(); i++)
        {
            ///arrange
            CONCRETE_IO_HANDLE result;
            umock_c_negative_tests_reset();
            umock_c_negative_tests_fail_call(i);

            ///act
            result = xio_state_create(&adapter_interface, &create_params);

            ///assert
            ASSERT_IS_NULL(result);
        }

        ///cleanup
        assert_gballoc_checks();
    }

    /* Tests_SRS_XIO_STATE_30_013: [ If the  adapter_interface  parameter is NULL,  xio_state_create  shall log an error and return NULL. ]*/
    TEST_FUNCTION(xio_state__create_parameter_validation_fails__fails)
    {
        ///arrange

        ///act
        CONCRETE_IO_HANDLE result = xio_state_create(NULL, NULL);

        ///assert
        ASSERT_IS_NULL_WITH_MSG(result, "Unexpected success in xio_state_create");

        assert_gballoc_checks();
    }


    /* Tests_SRS_XIO_STATE_30_010: [ The xio_state_create shall allocate and initialize all necessary resources, 
    call xio_adapter_create, and return an instance of the xio_state in XIO_STATE_INITIAL. ]*/
    /* Tests_SRS_XIO_STATE_30_012: [ The xio_state_create shall pass io_create_parameters to xio_adapter_create. ]*/
    TEST_FUNCTION(xio_state__create__succeeds)
    {
        CONCRETE_IO_HANDLE result;
        ///arrange

        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG));  
        STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG)); 
        STRICT_EXPECTED_CALL(xio_adapter_create(&create_params));

        ///act
        result = xio_state_create(&adapter_interface, &create_params);

        ///assert
        ASSERT_IS_NOT_NULL(result);
        ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
        // Note: the STRICT_EXPECTED_CALL(xio_adapter_create(&create_params)) verifies Tests_SRS_XIO_STATE_30_012

        ///cleanup
        xio_state_destroy(result);
        assert_gballoc_checks();
    }

END_TEST_SUITE(xio_state_unittests)
