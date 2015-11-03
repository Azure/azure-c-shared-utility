#include <stdbool.h>
#include "testrunnerswitcher.h"
#include "micromock.h"
#include "list.h"

bool fail_alloc_calls;

TYPED_MOCK_CLASS(list_mocks, CGlobalMock)
{
public:
	/* amqpalloc mocks */
	MOCK_STATIC_METHOD_1(, void*, amqpalloc_malloc, size_t, size)
	MOCK_METHOD_END(void*, malloc(size));
	MOCK_STATIC_METHOD_1(, void, amqpalloc_free, void*, ptr)
		free(ptr);
	MOCK_VOID_METHOD_END();

	/* test match function mock */
	MOCK_STATIC_METHOD_2(, bool, test_match_function, LIST_ITEM_HANDLE, list_item, const void*, match_context)
	MOCK_METHOD_END(bool, true);
};

extern "C"
{
	DECLARE_GLOBAL_MOCK_METHOD_1(list_mocks, , void*, amqpalloc_malloc, size_t, size);
	DECLARE_GLOBAL_MOCK_METHOD_1(list_mocks, , void, amqpalloc_free, void*, ptr);

	DECLARE_GLOBAL_MOCK_METHOD_2(list_mocks, , bool, test_match_function, LIST_ITEM_HANDLE, list_item, const void*, match_context);
}

MICROMOCK_MUTEX_HANDLE test_serialize_mutex;

#define TEST_CONTEXT ((const void*)0x4242)

BEGIN_TEST_SUITE(list_unittests)

		TEST_SUITE_INITIALIZE(suite_init)
		{
			test_serialize_mutex = MicroMockCreateMutex();
			ASSERT_IS_NOT_NULL(test_serialize_mutex);
		}

		TEST_SUITE_CLEANUP(suite_cleanup)
		{
			MicroMockDestroyMutex(test_serialize_mutex);
		}

		TEST_FUNCTION_INITIALIZE(method_init)
		{
			if (!MicroMockAcquireMutex(test_serialize_mutex))
			{
				ASSERT_FAIL("Could not acquire test serialization mutex.");
			}
			fail_alloc_calls = false;
		}

		TEST_FUNCTION_CLEANUP(method_cleanup)
		{
			if (!MicroMockReleaseMutex(test_serialize_mutex))
			{
				ASSERT_FAIL("Could not release test serialization mutex.");
			}
		}

		/* list_create */

		/* Tests_SRS_LIST_01_001: [list_create shall create a new list and return a non-NULL handle on success.] */
		TEST_FUNCTION(when_underlying_calls_suceed_list_create_succeeds)
		{
			// arrange
			list_mocks mocks;

			EXPECTED_CALL(mocks, amqpalloc_malloc(IGNORED_NUM_ARG));

			// act
			LIST_HANDLE result = list_create();

			// assert
			ASSERT_IS_NOT_NULL(result);
		}

		/* Tests_SRS_LIST_01_002: [If any error occurs during the list creation, list_create shall return NULL.] */
		TEST_FUNCTION(when_underlying_malloc_fails_list_create_fails)
		{
			// arrange
			list_mocks mocks;

			EXPECTED_CALL(mocks, amqpalloc_malloc(IGNORED_NUM_ARG))
				.SetReturn((void*)NULL);

			// act
			LIST_HANDLE result = list_create();

			// assert
			ASSERT_IS_NULL(result);
		}

		/* list_destroy */

		/* Tests_SRS_LIST_01_003: [list_destroy shall free all resources associated with the list identified by the handle argument.] */
		TEST_FUNCTION(list_destroy_on_a_non_null_handle_frees_resources)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			mocks.ResetAllCalls();

			EXPECTED_CALL(mocks, amqpalloc_free(IGNORED_PTR_ARG));

			// act
			list_destroy(handle);

			// assert
			// uMock checks the calls
		}

		/* Tests_SRS_LIST_01_004: [If the handle argument is NULL, no freeing of resources shall occur.] */
		TEST_FUNCTION(list_destroy_on_a_null_handle_frees_nothing)
		{
			// arrange
			list_mocks mocks;

			// act
			list_destroy(NULL);

			// assert
			// uMock checks the calls
		}

		/* list_add */

		/* Tests_SRS_LIST_01_006: [If any of the arguments is NULL, list_add shall not add the item to the list and return a non-zero value.] */
		TEST_FUNCTION(list_add_with_NULL_handle_fails)
		{
			// arrange
			list_mocks mocks;
			int x = 42;

			// act
			int result = list_add(NULL, &x);

			// assert
			ASSERT_ARE_NOT_EQUAL(int, 0, result);
		}

		/* Tests_SRS_LIST_01_006: [If any of the arguments is NULL, list_add shall not add the item to the list and return a non-zero value.] */
		TEST_FUNCTION(list_add_with_NULL_item_fails)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			mocks.ResetAllCalls();

			// act
			int result = list_add(handle, NULL);

			// assert
			ASSERT_ARE_NOT_EQUAL(int, 0, result);
		}

		/* Tests_SRS_LIST_01_005: [list_add shall add one item to the tail of the list and on success it shall return 0.] */
		/* Tests_SRS_LIST_01_008: [list_get_head_item shall return the head of the list.] */
		TEST_FUNCTION(list_add_adds_the_item_and_returns_zero)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			mocks.ResetAllCalls();
			int x = 42;

			EXPECTED_CALL(mocks, amqpalloc_malloc(IGNORED_NUM_ARG));

			// act
			int result = list_add(handle, &x);

			// assert
			ASSERT_ARE_EQUAL(int, 0, result);
			mocks.AssertActualAndExpectedCalls();
			LIST_ITEM_HANDLE head = list_get_head_item(handle);
			ASSERT_IS_NOT_NULL(head);
			ASSERT_ARE_EQUAL(int, x, *(const int*)list_item_get_value(head));
		}

		/* Tests_SRS_LIST_01_005: [list_add shall add one item to the tail of the list and on success it shall return 0.] */
		/* Tests_SRS_LIST_01_008: [list_get_head_item shall return the head of the list.] */
		TEST_FUNCTION(list_add_when_an_item_is_in_the_list_adds_at_the_end)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x1 = 42;
			int x2 = 43;

			list_add(handle, &x1);
			mocks.ResetAllCalls();

			EXPECTED_CALL(mocks, amqpalloc_malloc(IGNORED_NUM_ARG));

			// act
			int result = list_add(handle, &x2);

			// assert
			ASSERT_ARE_EQUAL(int, 0, result);
			mocks.AssertActualAndExpectedCalls();
			LIST_ITEM_HANDLE list_item = list_get_head_item(handle);
			ASSERT_IS_NOT_NULL(list_item);
			ASSERT_ARE_EQUAL(int, x1, *(const int*)list_item_get_value(list_item));
			list_item = list_get_next_item(list_item);
			ASSERT_IS_NOT_NULL(list_item);
			ASSERT_ARE_EQUAL(int, x2, *(const int*)list_item_get_value(list_item));
		}

		/* Tests_SRS_LIST_01_007: [If allocating the new list node fails, list_add shall return a non-zero value.] */
		TEST_FUNCTION(when_the_underlying_malloc_fails_list_add_fails)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x = 42;
			mocks.ResetAllCalls();

			EXPECTED_CALL(mocks, amqpalloc_malloc(IGNORED_NUM_ARG))
				.SetReturn((void*)NULL);

			// act
			int result = list_add(handle, &x);

			// assert
			ASSERT_ARE_NOT_EQUAL(int, 0, result);
		}

		/* list_get_head_item */

		/* Tests_SRS_LIST_01_010: [If the list is empty, list_get_head_item_shall_return NULL.] */
		TEST_FUNCTION(when_the_list_is_empty_list_get_head_item_yields_NULL)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			mocks.ResetAllCalls();

			// act
			LIST_ITEM_HANDLE result = list_get_head_item(handle);

			// assert
			ASSERT_IS_NULL(result);
		}

		/* Tests_SRS_LIST_01_009: [If the handle argument is NULL, list_get_head_item shall return NULL.] */
		TEST_FUNCTION(list_get_head_item_with_NULL_handle_yields_NULL)
		{
			// arrange
			list_mocks mocks;

			// act
			LIST_ITEM_HANDLE result = list_get_head_item(NULL);

			// assert
			ASSERT_IS_NULL(result);
		}

		/* Tests_SRS_LIST_01_008: [list_get_head_item shall return the head of the list.] */
		TEST_FUNCTION(list_get_head_item_removes_the_item)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x = 42;
			(void)list_add(handle, &x);
			mocks.ResetAllCalls();

			// act
			LIST_ITEM_HANDLE head = list_get_head_item(handle);

			// assert
			ASSERT_IS_NOT_NULL(head);
			ASSERT_ARE_EQUAL(int, x, *(const int*)list_item_get_value(head));
		}

		/* list_get_next_item */

		/* Tests_SRS_LIST_01_018: [list_get_next_item shall return the next item in the list following the item item_handle.] */
		TEST_FUNCTION(list_get_next_item_gets_the_next_item)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x1 = 42;
			int x2 = 43;
			(void)list_add(handle, &x1);
			(void)list_add(handle, &x2);
			mocks.ResetAllCalls();
			LIST_ITEM_HANDLE item = list_get_head_item(handle);

			// act
			item = list_get_next_item(item);

			// assert
			ASSERT_IS_NOT_NULL(item);
			ASSERT_ARE_EQUAL(int, x2, *(const int*)list_item_get_value(item));
		}

		/* Tests_SRS_LIST_01_019: [If item_handle is NULL then list_get_next_item shall return NULL.] */
		TEST_FUNCTION(list_get_next_item_with_NULL_item_handle_returns_NULL)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			mocks.ResetAllCalls();

			// act
			LIST_ITEM_HANDLE item = list_get_next_item(NULL);

			// assert
			ASSERT_IS_NULL(item);
		}

		/* Tests_SRS_LIST_01_022: [If no more items exist in the list after the item_handle item, list_get_next_item shall return NULL.] */
		TEST_FUNCTION(list_get_next_item_when_no_more_items_in_list_returns_NULL)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x1 = 42;
			int x2 = 43;
			(void)list_add(handle, &x1);
			(void)list_add(handle, &x2);
			mocks.ResetAllCalls();
			LIST_ITEM_HANDLE item = list_get_head_item(handle);
			item = list_get_next_item(item);

			// act
			item = list_get_next_item(item);

			// assert
			ASSERT_IS_NULL(item);
		}

		/* list_item_get_value */

		/* Tests_SRS_LIST_01_020: [list_item_get_value shall return the value associated with the list item identified by the item_handle argument.] */
		TEST_FUNCTION(list_item_get_value_returns_the_item_value)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x = 42;
			(void)list_add(handle, &x);
			mocks.ResetAllCalls();
			LIST_ITEM_HANDLE item = list_get_head_item(handle);

			// act
			int result = *(const int*)list_item_get_value(item);

			// assert
			ASSERT_ARE_EQUAL(int, x, result);
		}

		/* Tests_SRS_LIST_01_021: [If item_handle is NULL, list_item_get_value shall return NULL.] */
		TEST_FUNCTION(list_item_get_value_with_NULL_item_returns_NULL)
		{
			// arrange
			list_mocks mocks;

			// act
			const void* result = list_item_get_value(NULL);

			// assert
			ASSERT_IS_NULL(result);
		}

		/* list_find */

		/* Tets_SRS_LIST_01_012: [If the handle or the match_function argument is NULL, list_find shall return NULL.] */
		TEST_FUNCTION(list_find_with_NULL_handle_fails_with_NULL)
		{
			// arrange
			list_mocks mocks;

			// act
			LIST_ITEM_HANDLE result = list_find(NULL, test_match_function, TEST_CONTEXT);

			// assert
			ASSERT_IS_NULL(result);
		}

		/* Tets_SRS_LIST_01_012: [If the handle or the match_function argument is NULL, list_find shall return NULL.] */
		TEST_FUNCTION(list_find_with_NULL__fails_with_NULL)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x = 42;
			(void)list_add(handle, &x);
			mocks.ResetAllCalls();

			// act
			LIST_ITEM_HANDLE result = list_find(handle, NULL, TEST_CONTEXT);

			// assert
			ASSERT_IS_NULL(result);
		}

		/* Tests_SRS_LIST_01_011: [list_find shall iterate through all items in a list and return the one that satisfies a certain match function.] */
		/* Tests_SRS_LIST_01_014: [list find shall determine whether an item satisfies the match criteria by invoking the match function for each item in the list until a matching item is found.] */
		/* Tests_SRS_LIST_01_013: [The match_function shall get as arguments the list item being attempted to be matched and the match_context as is.] */
		/* Tests_SRS_LIST_01_017: [If the match function returns true, list_find shall consider that item as matching.] */
		TEST_FUNCTION(list_find_on_a_list_with_1_matching_item_yields_that_item)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x = 42;
			(void)list_add(handle, &x);
			mocks.ResetAllCalls();

			STRICT_EXPECTED_CALL(mocks, test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
				.IgnoreArgument(1);

			// act
			LIST_ITEM_HANDLE result = list_find(handle, test_match_function, TEST_CONTEXT);

			// assert
			ASSERT_IS_NOT_NULL(result);
			ASSERT_ARE_EQUAL(int, x, *(const int*)list_item_get_value(result));
		}

		/* Tests_SRS_LIST_01_016: [If the match function returns false, list_find shall consider that item as not matching.] */
		TEST_FUNCTION(list_find_on_a_list_with_1_items_that_does_not_match_returns_NULL)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x = 42;
			(void)list_add(handle, &x);
			mocks.ResetAllCalls();

			STRICT_EXPECTED_CALL(mocks, test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
				.IgnoreArgument(1).SetReturn(false);

			// act
			LIST_ITEM_HANDLE result = list_find(handle, test_match_function, TEST_CONTEXT);

			// assert
			ASSERT_IS_NULL(result);
		}

		/* Tests_SRS_LIST_01_011: [list_find shall iterate through all items in a list and return the one that satisfies a certain match function.] */
		/* Tests_SRS_LIST_01_014: [list find shall determine whether an item satisfies the match criteria by invoking the match function for each item in the list until a matching item is found.] */
		/* Tests_SRS_LIST_01_013: [The match_function shall get as arguments the list item being attempted to be matched and the match_context as is.] */
		/* Tests_SRS_LIST_01_017: [If the match function returns true, list_find shall consider that item as matching.] */
		TEST_FUNCTION(list_find_on_a_list_with_2_items_where_the_first_matches_yields_the_first_item)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x1 = 42;
			int x2 = 43;
			(void)list_add(handle, &x1);
			(void)list_add(handle, &x2);
			mocks.ResetAllCalls();

			STRICT_EXPECTED_CALL(mocks, test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
				.IgnoreArgument(1);

			// act
			LIST_ITEM_HANDLE result = list_find(handle, test_match_function, TEST_CONTEXT);

			// assert
			ASSERT_IS_NOT_NULL(result);
			ASSERT_ARE_EQUAL(int, x1, *(int*)list_item_get_value(result));
		}

		/* Tests_SRS_LIST_01_011: [list_find shall iterate through all items in a list and return the one that satisfies a certain match function.] */
		/* Tests_SRS_LIST_01_014: [list find shall determine whether an item satisfies the match criteria by invoking the match function for each item in the list until a matching item is found.] */
		/* Tests_SRS_LIST_01_013: [The match_function shall get as arguments the list item being attempted to be matched and the match_context as is.] */
		/* Tests_SRS_LIST_01_017: [If the match function returns true, list_find shall consider that item as matching.] */
		/* Tests_SRS_LIST_01_016: [If the match function returns false, list_find shall consider that item as not matching.] */
		TEST_FUNCTION(list_find_on_a_list_with_2_items_where_the_second_matches_yields_the_second_item)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x1 = 42;
			int x2 = 43;
			(void)list_add(handle, &x1);
			(void)list_add(handle, &x2);
			mocks.ResetAllCalls();

			STRICT_EXPECTED_CALL(mocks, test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
				.IgnoreArgument(1).SetReturn(false);
			STRICT_EXPECTED_CALL(mocks, test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
				.IgnoreArgument(1);

			// act
			LIST_ITEM_HANDLE result = list_find(handle, test_match_function, TEST_CONTEXT);

			// assert
			ASSERT_IS_NOT_NULL(result);
			ASSERT_ARE_EQUAL(int, x2, *(int*)list_item_get_value(result));
		}

		/* Tests_SRS_LIST_01_016: [If the match function returns false, list_find shall consider that item as not matching.] */
		TEST_FUNCTION(list_find_on_a_list_with_2_items_where_none_matches_returns_NULL)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			int x1 = 42;
			int x2 = 43;
			(void)list_add(handle, &x1);
			(void)list_add(handle, &x2);
			mocks.ResetAllCalls();

			STRICT_EXPECTED_CALL(mocks, test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
				.IgnoreArgument(1).SetReturn(false);
			STRICT_EXPECTED_CALL(mocks, test_match_function(IGNORED_PTR_ARG, TEST_CONTEXT))
				.IgnoreArgument(1).SetReturn(false);

			// act
			LIST_ITEM_HANDLE result = list_find(handle, test_match_function, TEST_CONTEXT);

			// assert
			ASSERT_IS_NULL(result);
		}

		/* Tests_SRS_LIST_01_015: [If the list is empty, list_find shall return NULL.] */
		TEST_FUNCTION(list_find_on_a_list_with_no_items_yields_NULL)
		{
			// arrange
			list_mocks mocks;
			LIST_HANDLE handle = list_create();
			mocks.ResetAllCalls();

			// act
			LIST_ITEM_HANDLE result = list_find(handle, test_match_function, TEST_CONTEXT);

			// assert
			ASSERT_IS_NULL(result);
		}

END_TEST_SUITE(list_unittests)
