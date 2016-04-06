// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdlib>
#include <cstring>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "testrunnerswitcher.h"
#include "micromock.h"
#include "micromockcharstararenullterminatedstrings.h"
#include "azure_c_shared_utility/constmap.h"
#include "azure_c_shared_utility/lock.h"

static MICROMOCK_MUTEX_HANDLE g_testByTest;
static MICROMOCK_GLOBAL_SEMAPHORE_HANDLE g_dllByDll;

#define GBALLOC_H

extern "C" int gballoc_init(void);
extern "C" void gballoc_deinit(void);
extern "C" void* gballoc_malloc(size_t size);
extern "C" void* gballoc_calloc(size_t nmemb, size_t size);
extern "C" void* gballoc_realloc(void* ptr, size_t size);
extern "C" void gballoc_free(void* ptr);

namespace BASEIMPLEMENTATION
{
    /*if malloc is defined as gballoc_malloc at this moment, there'd be serious trouble*/
#define Lock(x) (LOCK_OK + gballocState - gballocState) /*compiler warning about constant in if condition*/
#define Unlock(x) (LOCK_OK + gballocState - gballocState)
#define Lock_Init() (LOCK_HANDLE)0x42
#define Lock_Deinit(x) (LOCK_OK + gballocState - gballocState)
#include "gballoc.c"
#undef Lock
#undef Unlock
#undef Lock_Init
#undef Lock_Deinit

};

static size_t currentmalloc_call;
static size_t whenShallmalloc_fail;

DEFINE_MICROMOCK_ENUM_TO_STRING(CONSTMAP_RESULT, CONSTMAP_RESULT_VALUES);

#define VALID_MAP_HANDLE    (MAP_HANDLE)0xDEAF
#define VALID_CONST_CHAR_POINTER (const char*const*)0xDADA
#define VALID_MAP_CLONE1     (MAP_HANDLE)0xDEDE
#define VALID_MAP_CLONE2     (MAP_HANDLE)0xDEDD
#define INVALID_MAP_HANDLE  (MAP_HANDLE)0xDEAD
#define INVALID_CLONE_HANDLE  (MAP_HANDLE)0xDEAE
#define VALID_KV_COUNT		(size_t)100
#define VALID_VALUE			"value"

static MAP_RESULT currentMapResult;

DEFINE_MICROMOCK_ENUM_TO_STRING(MAP_RESULT, MAP_RESULT_VALUES);

TYPED_MOCK_CLASS(CConstMapMocks, CGlobalMock)
{
public:

    // memory related
    MOCK_STATIC_METHOD_1(, void*, gballoc_malloc, size_t, size)
        void* result2;
    currentmalloc_call++;
    if (whenShallmalloc_fail>0)
    {
        if (currentmalloc_call == whenShallmalloc_fail)
        {
            result2 = NULL;
        }
        else
        {
            result2 = BASEIMPLEMENTATION::gballoc_malloc(size);
        }
    }
    else
    {
        result2 = BASEIMPLEMENTATION::gballoc_malloc(size);
    }
    MOCK_METHOD_END(void*, result2);

    MOCK_STATIC_METHOD_1(, void, gballoc_free, void*, ptr)
        BASEIMPLEMENTATION::gballoc_free(ptr);
	MOCK_VOID_METHOD_END()

	// Map related

	// Map_Clone
	MOCK_STATIC_METHOD_1(, MAP_HANDLE, Map_Clone, MAP_HANDLE, sourceMap)
		MAP_HANDLE result2;
		if (sourceMap == VALID_MAP_HANDLE)
		{
			result2 = VALID_MAP_CLONE1;
		}
		else if (sourceMap == VALID_MAP_CLONE1)
		{
			result2 = VALID_MAP_CLONE2;
		}
		else if (sourceMap == INVALID_CLONE_HANDLE)
		{
			result2 = INVALID_MAP_HANDLE;
		}
		else if (sourceMap == INVALID_MAP_HANDLE)
		{
			result2 = NULL;
		}
	MOCK_METHOD_END(MAP_HANDLE, result2);

	// Map_Destroy
	MOCK_STATIC_METHOD_1(, void, Map_Destroy, MAP_HANDLE, ptr)
	MOCK_VOID_METHOD_END();

	// Map_ContainsKey
	MOCK_STATIC_METHOD_3(, MAP_RESULT, Map_ContainsKey, MAP_HANDLE, handle, const char*, key, bool*, keyExists)
		MAP_RESULT result3 = currentMapResult;
		if (result3 == MAP_OK)
		{
			*keyExists = true;
		}
	MOCK_METHOD_END(MAP_RESULT, result3);

	// Map_ContainsValue 
	// MAP_RESULT Map_ContainsValue(MAP_HANDLE handle, const char* value, bool* valueExists);
	MOCK_STATIC_METHOD_3(, MAP_RESULT, Map_ContainsValue, MAP_HANDLE, handle, const char*, value, bool*, valueExists)
		MAP_RESULT result4 = currentMapResult;
		if (result4 == MAP_OK)
		{
			*valueExists = true;
		}
	MOCK_METHOD_END(MAP_RESULT, result4);

	// Map_GetValueFromKey
	MOCK_STATIC_METHOD_2(, const char*, Map_GetValueFromKey, MAP_HANDLE, sourceMap, const char*, key)
		const char* result5;
		if (currentMapResult == MAP_OK)
		{
			result5 = VALID_VALUE;
		}
		else
		{
			result5 = NULL;
		}
	MOCK_METHOD_END(const char*, result5);

	// Map_GetInternals
	MOCK_STATIC_METHOD_4(, MAP_RESULT, Map_GetInternals, MAP_HANDLE, handle, const char*const**, keys, const char*const**, values, size_t*, count)
		MAP_RESULT result6 = currentMapResult;
		*keys = VALID_CONST_CHAR_POINTER;
		*values = VALID_CONST_CHAR_POINTER;
		*count = VALID_KV_COUNT;
	MOCK_METHOD_END(MAP_RESULT, result6);
};

DECLARE_GLOBAL_MOCK_METHOD_1(CConstMapMocks, , void*, gballoc_malloc, size_t, size);
DECLARE_GLOBAL_MOCK_METHOD_1(CConstMapMocks, , void, gballoc_free, void*, ptr);

DECLARE_GLOBAL_MOCK_METHOD_1(CConstMapMocks, , MAP_HANDLE, Map_Clone, MAP_HANDLE, sourceMap);
DECLARE_GLOBAL_MOCK_METHOD_1(CConstMapMocks, , void, Map_Destroy, MAP_HANDLE, ptr);
DECLARE_GLOBAL_MOCK_METHOD_3(CConstMapMocks, , MAP_RESULT, Map_ContainsKey, MAP_HANDLE, handle, const char*, key, bool*, keyExists);
DECLARE_GLOBAL_MOCK_METHOD_3(CConstMapMocks, , MAP_RESULT, Map_ContainsValue, MAP_HANDLE, handle, const char*, key, bool*, keyExists);
DECLARE_GLOBAL_MOCK_METHOD_2(CConstMapMocks, , const char*, Map_GetValueFromKey, MAP_HANDLE, ptr, const char*, key);
DECLARE_GLOBAL_MOCK_METHOD_4(CConstMapMocks, , MAP_RESULT, Map_GetInternals, MAP_HANDLE, handle, const char*const**, keys, const char*const**, values, size_t*, count);
/* capacity */

BEGIN_TEST_SUITE(constmap_unittests)

    TEST_SUITE_INITIALIZE(TestClassInitialize)
    {
        INITIALIZE_MEMORY_DEBUG(g_dllByDll);
        g_testByTest = MicroMockCreateMutex();
        ASSERT_IS_NOT_NULL(g_testByTest);
    }

    TEST_SUITE_CLEANUP(TestClassCleanup)
    {
        MicroMockDestroyMutex(g_testByTest);
        DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
    }

    TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
    {
        if (!MicroMockAcquireMutex(g_testByTest))
        {
            ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
        }
        currentmalloc_call = 0;
        whenShallmalloc_fail = 0;
		currentMapResult = MAP_OK;
    }

    TEST_FUNCTION_CLEANUP(TestMethodCleanup)
    {
        if (!MicroMockReleaseMutex(g_testByTest))
        {
            ASSERT_FAIL("failure in test framework at ReleaseMutex");
        }
    }

	/*Tests_SRS_CONSTMAP_17_001: [ConstMap_Create shall create an immutable map, populated by the key, value pairs in the source map.]*/
	/*Tests_SRS_CONSTMAP_17_048: [ConstMap_Create shall accept any non-NULL MAP_HANDLE as input.]*/
	/*Tests_SRS_CONSTMAP_17_003: [Otherwise, it shall return a non-NULL handle that can be used in subsequent calls.]*/
	/*Tests_SRS_CONSTMAP_17_004: [If the reference count is zero, ConstMap_Destroy shall release all resources associated with the immutable map.]*/
    TEST_FUNCTION(ConstMap_Create_Destroy_Success)
    {
		// Arrange
		CConstMapMocks mocks;

		STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_Clone(VALID_MAP_HANDLE));

		STRICT_EXPECTED_CALL(mocks, Map_Destroy(VALID_MAP_CLONE1));
		STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
			.IgnoreArgument(1);

        MAP_HANDLE sourceMap = VALID_MAP_HANDLE;

		///Act
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);

		ASSERT_IS_NOT_NULL(aHandle);

		ConstMap_Destroy(aHandle);

		///Assert
		mocks.AssertActualAndExpectedCalls();

		//Ablution                

    }

	/* Tests_SRS_CONSTMAP_17_002: [If during creation there are any errors, then ConstMap_Create shall return NULL.]*/
    TEST_FUNCTION(ConstMap_Create_Malloc_Failed)
    {
		// Arrange
		CConstMapMocks mocks;

		STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
			.IgnoreArgument(1);
		whenShallmalloc_fail = 1;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;

		///Act
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);

		///Assert
		ASSERT_IS_NULL(aHandle);

		mocks.AssertActualAndExpectedCalls();

		//Ablution                

    }

	/*Tests_SRS_CONSTMAP_17_002: [If during creation there are any errors, then ConstMap_Create shall return NULL.] */
    TEST_FUNCTION(ConstMap_Clone_Map_Failed)
    {
		// Arrange
		CConstMapMocks mocks;

		STRICT_EXPECTED_CALL(mocks, gballoc_malloc(IGNORED_NUM_ARG))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_Clone(INVALID_MAP_HANDLE));
		STRICT_EXPECTED_CALL(mocks, gballoc_free(IGNORED_PTR_ARG))
			.IgnoreArgument(1);

		MAP_HANDLE sourceMap = INVALID_MAP_HANDLE;

		///Act
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);

		///Assert
		ASSERT_IS_NULL(aHandle);

		mocks.AssertActualAndExpectedCalls();

		//Ablution                

    }

	/*Tests_SRS_CONSTMAP_17_039: [ConstMap_Clone shall increase the internal reference count of the immutable map indicated by parameter handle] */
	TEST_FUNCTION(ConstMap_Clone_Destroy_Success)
	{
		// Arrange
		CConstMapMocks mocks;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);
		CONSTMAP_HANDLE aClone = ConstMap_Clone(aHandle);

		mocks.ResetAllCalls();

		///Act
		ConstMap_Destroy(aClone);

		///Assert
		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle);

	}
	/*Tests_SRS_CONSTMAP_17_005: [If parameter handle is NULL then ConstMap_Destroy shall take no action.]*/
	TEST_FUNCTION(ConstMap_Destroy_Null)
	{
		///Arrange
		CConstMapMocks mocks;
		CONSTMAP_HANDLE handle = NULL;

		///Act
		ConstMap_Destroy(handle);

		///Assert
		mocks.AssertActualAndExpectedCalls();

		///Ablution
	}

	/*Tests_SRS_CONSTMAP_17_039: [ConstMap_Clone shall increase the internal reference count of the immutable map indicated by parameter handle]*/
	/*Tests_SRS_CONSTMAP_17_050: [ConstMap_Clone shall return the non-NULL handle. ]*/
	TEST_FUNCTION(ConstMap_Clone_Success)
	{
		// Arrange
		CConstMapMocks mocks;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);

		mocks.ResetAllCalls();

		///Act
		CONSTMAP_HANDLE aClone = ConstMap_Clone(aHandle);

		///Assert
		ASSERT_ARE_EQUAL(void_ptr, aHandle, aClone);

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aClone);
		ConstMap_Destroy(aHandle);

	}

	/*Tests_SRS_CONSTMAP_17_038: [ConstMap_Clone returns NULL if parameter handle is NULL.] */
	TEST_FUNCTION(ConstMap_Clone_Null)
	{
		// Arrange
		CConstMapMocks mocks;

		CONSTMAP_HANDLE aHandle = NULL;

		///Act
		CONSTMAP_HANDLE aClone = ConstMap_Clone(aHandle);

		///Assert
		ASSERT_IS_NULL(aClone);
		mocks.AssertActualAndExpectedCalls();

		//Ablution       
	}

	/*Tests_SRS_CONSTMAP_17_052: [ConstMap_CloneWriteable shall create a new, writeable map, populated by the key, value pairs in the parameter defined by handle.]*/
	/*Tests_SRS_CONSTMAP_17_054: [Otherwise, ConstMap_CloneWriteable shall return a non-NULL handle that can be used in subsequent calls.]*/
	TEST_FUNCTION(ConstMap_CloneWritable_Success)
	{
		// Arrange
		CConstMapMocks mocks;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);
		MAP_HANDLE newMap = NULL;

		mocks.ResetAllCalls();

		STRICT_EXPECTED_CALL(mocks, Map_Clone(VALID_MAP_CLONE1)).IgnoreArgument(1);

		//Act 
		newMap = ConstMap_CloneWriteable(aHandle);

		//Assert
		ASSERT_ARE_EQUAL(void_ptr, VALID_MAP_CLONE2, newMap);

		mocks.AssertActualAndExpectedCalls();

		//Ablution
		ConstMap_Destroy(aHandle);
		Map_Destroy(sourceMap);
		Map_Destroy(newMap);
	}

	/*Tests_SRS_CONSTMAP_17_053: [If during cloning, any operation fails, then ConstMap_CloneWriteableap_Clone shall return NULL.]*/
	TEST_FUNCTION(ConstMap_CloneWritable_Fail)
	{
		// Arrange
		CConstMapMocks mocks;

		MAP_HANDLE sourceMap = INVALID_CLONE_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);
		MAP_HANDLE newMap = NULL;

		mocks.ResetAllCalls();

		STRICT_EXPECTED_CALL(mocks, Map_Clone(INVALID_MAP_HANDLE)).IgnoreArgument(1);

		//Act 
		newMap = ConstMap_CloneWriteable(aHandle);

		//Assert
		ASSERT_IS_NULL(newMap);

		mocks.AssertActualAndExpectedCalls();

		//Ablution
		ConstMap_Destroy(aHandle);
		Map_Destroy(sourceMap);
	}

	/*Tests_SRS_CONSTMAP_17_051: [ConstMap_CloneWriteable returns NULL if parameter handle is NULL. ]*/
	TEST_FUNCTION(ConstMap_CloneWritable_NULL)
	{
		// Arrange
		CConstMapMocks mocks;

		MAP_HANDLE newMap = NULL;

		//Act 
		newMap = ConstMap_CloneWriteable(NULL);

		//Assert
		ASSERT_IS_NULL(newMap);

		mocks.AssertActualAndExpectedCalls();

		//Ablution
	}


	/*Tests_SRS_CONSTMAP_17_025: [Otherwise if a key exists then ConstMap_ContainsKey shall return true.]*/
	TEST_FUNCTION(ConstMap_ContainsKey_Success)
	{
		// Arrange
		CConstMapMocks mocks;
		const char * key = "aKey";
		bool keyExists;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);

		mocks.ResetAllCalls();

		// Call to Map
		STRICT_EXPECTED_CALL(mocks, Map_ContainsKey(IGNORED_PTR_ARG, key, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);

		///Act
		keyExists = ConstMap_ContainsKey(aHandle, key);


		///Assert
		ASSERT_IS_TRUE(keyExists);

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle);
	}

	/*Tests_SRS_CONSTMAP_17_024: [If parameter handle or key are NULL then ConstMap_ContainsKey shall return false.]*/
	TEST_FUNCTION(ConstMap_ContainsKey_Null)
	{
		// Arrange
		CConstMapMocks mocks;
		const char * key1 = "aKey";
		const char * key2 = NULL;
		bool keyExists1;
		bool keyExists2;
		CONSTMAP_HANDLE aHandle1 = NULL;
		CONSTMAP_HANDLE aHandle2;
		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;

		aHandle2 = ConstMap_Create(sourceMap);
		mocks.ResetAllCalls();

		///Act
		// NULL Handle
		keyExists1 = ConstMap_ContainsKey(aHandle1, key1);

		// NULL key
		keyExists2 = ConstMap_ContainsKey(aHandle2, key2);

		///Assert
		ASSERT_IS_FALSE(keyExists1);
		ASSERT_IS_FALSE(keyExists2);
		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle2);

	}

	/*Tests_SRS_CONSTMAP_17_026: [If a key doesn't exist, then ConstMap_ContainsKey shall return false.]*/
	TEST_FUNCTION(ConstMap_ContainsKey_Failures)
	{
		// Arrange
		CConstMapMocks mocks;
		const char * key = "aKey";
		bool keyExists;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);
		mocks.ResetAllCalls();

		// Call to Map_ContainsKey (match with mapErrorList size)
		STRICT_EXPECTED_CALL(mocks, Map_ContainsKey(IGNORED_PTR_ARG, key, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);
		STRICT_EXPECTED_CALL(mocks, Map_ContainsKey(IGNORED_PTR_ARG, key, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);
		STRICT_EXPECTED_CALL(mocks, Map_ContainsKey(IGNORED_PTR_ARG, key, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);
		STRICT_EXPECTED_CALL(mocks, Map_ContainsKey(IGNORED_PTR_ARG, key, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);
		STRICT_EXPECTED_CALL(mocks, Map_ContainsKey(IGNORED_PTR_ARG, key, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);


		///Act
		MAP_RESULT mapErrorList[] = {
			MAP_ERROR,
			MAP_INVALIDARG, 
			MAP_KEYEXISTS, 
			MAP_KEYNOTFOUND, 
			MAP_FILTER_REJECT
		};
		size_t errors = sizeof(mapErrorList) / sizeof(MAP_RESULT);
		CONSTMAP_RESULT constErrorList[] = {
			CONSTMAP_ERROR,
			CONSTMAP_INVALIDARG,
			CONSTMAP_ERROR,
			CONSTMAP_KEYNOTFOUND,
			CONSTMAP_ERROR
		};

		for (size_t e = 0; e < errors; e++)
		{
			currentMapResult = mapErrorList[e];
			keyExists = ConstMap_ContainsKey(aHandle, key);
			ASSERT_IS_FALSE(keyExists);
		}

		///Assert

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle);
	}

	/*Tests_SRS_CONSTMAP_17_028: [Otherwise, if a pair has its value equal to the parameter value, the ConstMap_ContainsValue shall return true.]*/
	TEST_FUNCTION(ConstMap_ContainsValue_Success)
	{
		// Arrange
		CConstMapMocks mocks;
		const char * value = "aValue";
		bool valueExists;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);

		mocks.ResetAllCalls();

		// Call to Map
		STRICT_EXPECTED_CALL(mocks, Map_ContainsValue(IGNORED_PTR_ARG, value, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);

		///Act
		valueExists = ConstMap_ContainsValue(aHandle, value);

		///Assert
		ASSERT_IS_TRUE(valueExists);

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle);
	}

	/*Tests_SRS_CONSTMAP_17_027: [If parameter handle or value is NULL then ConstMap_ContainsValue shall return false.]*/
	TEST_FUNCTION(ConstMap_ContainsValue_Null)
	{
		// Arrange
		CConstMapMocks mocks;
		const char * value1 = "aValue";
		bool valueExists1;
		const char * value2 = NULL;
		bool valueExists2;
		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle1 = NULL;
		CONSTMAP_HANDLE aHandle2 = NULL;

		aHandle2 = ConstMap_Create(sourceMap);

		mocks.ResetAllCalls();

		///Act
		valueExists1 = ConstMap_ContainsValue(aHandle1, value1);
		valueExists2 = ConstMap_ContainsValue(aHandle2, value2);


		///Assert
		ASSERT_IS_FALSE(valueExists1);
		ASSERT_IS_FALSE(valueExists2);

		mocks.AssertActualAndExpectedCalls();

		//Ablution   
		ConstMap_Destroy(aHandle2);
	}

	/* Tests_SRS_CONSTMAP_17_029: [Otherwise, if such a does not exist, then ConstMap_ContainsValue shall return false.]*/
	TEST_FUNCTION(ConstMap_ContainsValue_Failures)
	{
		// Arrange
		CConstMapMocks mocks;
		const char * value = "aValue";
		bool valueExists;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);
		mocks.ResetAllCalls();

		// Call to Map_ContainsValue (match with mapErrorList size)
		STRICT_EXPECTED_CALL(mocks, Map_ContainsValue(IGNORED_PTR_ARG, value, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);
		STRICT_EXPECTED_CALL(mocks, Map_ContainsValue(IGNORED_PTR_ARG, value, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);
		STRICT_EXPECTED_CALL(mocks, Map_ContainsValue(IGNORED_PTR_ARG, value, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);
		STRICT_EXPECTED_CALL(mocks, Map_ContainsValue(IGNORED_PTR_ARG, value, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);
		STRICT_EXPECTED_CALL(mocks, Map_ContainsValue(IGNORED_PTR_ARG, value, IGNORED_PTR_ARG))
			.IgnoreArgument(1).IgnoreArgument(3);

		///Act
		MAP_RESULT mapErrorList[] = {
			MAP_ERROR,
			MAP_INVALIDARG,
			MAP_KEYEXISTS,
			MAP_KEYNOTFOUND,
			MAP_FILTER_REJECT
		};
		size_t errors = sizeof(mapErrorList) / sizeof(MAP_RESULT);
		CONSTMAP_RESULT constErrorList[] = {
			CONSTMAP_ERROR,
			CONSTMAP_INVALIDARG,
			CONSTMAP_ERROR,
			CONSTMAP_KEYNOTFOUND,
			CONSTMAP_ERROR
		};

		for (size_t e = 0; e < errors; e++)
		{
			currentMapResult = mapErrorList[e];
			valueExists = ConstMap_ContainsValue(aHandle, value);
			ASSERT_IS_FALSE(valueExists);
		}

		///Assert

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle);
	}

	/* Tests_SRS_CONSTMAP_17_042: [Otherwise, ConstMap_GetValue returns the key's value.]*/
	TEST_FUNCTION(ConstMap_GetValue_Success)
	{
		// Arrange
		CConstMapMocks mocks;
		const char * key = "aKey";
		const char * value;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);
		mocks.ResetAllCalls();

		// Call to Map
		STRICT_EXPECTED_CALL(mocks, Map_GetValueFromKey(IGNORED_PTR_ARG, key))
			.IgnoreArgument(1);

		///Act
		value = ConstMap_GetValue(aHandle, key);


		///Assert
		ASSERT_ARE_EQUAL(char_ptr, VALID_VALUE, value);

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle);

	}

	TEST_FUNCTION(ConstMap_GetValue_Null)
	{
		// Arrange
		CConstMapMocks mocks;
		const char * key1 = "aKey";
		const char * value1;
		CONSTMAP_HANDLE aHandle1 = NULL;
		const char * key2 = NULL;
		const char * value2;
		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle2 = ConstMap_Create(sourceMap);

		mocks.ResetAllCalls();

		///Act
		value1 = ConstMap_GetValue(aHandle1, key1);
		value2 = ConstMap_GetValue(aHandle2, key2);

		///Assert
		ASSERT_IS_NULL(value1);
		ASSERT_IS_NULL(value2);

		mocks.AssertActualAndExpectedCalls();

		//Ablution   
		ConstMap_Destroy(aHandle2);

	}

	/*Tests_SRS_CONSTMAP_17_040: [If parameter handle or key is NULL then ConstMap_GetValue returns NULL.] */
	/*Tests_SRS_CONSTMAP_17_041: [If the key is not found, then ConstMap_GetValue returns NULL.]*/
	TEST_FUNCTION(ConstMap_GetValue_Failures)
	{
		// Arrange
		CConstMapMocks mocks;
		const char * key = "aKey";
		const char * value;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);
		mocks.ResetAllCalls();

		// Call to Map_ContainsKey (match with mapErrorList size)
		STRICT_EXPECTED_CALL(mocks, Map_GetValueFromKey(IGNORED_PTR_ARG, key))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_GetValueFromKey(IGNORED_PTR_ARG, key))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_GetValueFromKey(IGNORED_PTR_ARG, key))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_GetValueFromKey(IGNORED_PTR_ARG, key))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_GetValueFromKey(IGNORED_PTR_ARG, key))
			.IgnoreArgument(1);


		///Act
		MAP_RESULT mapErrorList[] = {
			MAP_ERROR,
			MAP_INVALIDARG,
			MAP_KEYEXISTS,
			MAP_KEYNOTFOUND,
			MAP_FILTER_REJECT
		};
		size_t errors = sizeof(mapErrorList) / sizeof(MAP_RESULT);
		CONSTMAP_RESULT constErrorList[] = {
			CONSTMAP_ERROR,
			CONSTMAP_INVALIDARG,
			CONSTMAP_ERROR,
			CONSTMAP_KEYNOTFOUND,
			CONSTMAP_ERROR
		};

		// Errors from Map_GetValueFromKey
		for (size_t e = 0; e < errors; e++)
		{
			currentMapResult = mapErrorList[e];
			value = ConstMap_GetValue(aHandle, key);
			ASSERT_IS_NULL(value);
		}

		///Assert

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle);
	}

	/*Tests_SRS_CONSTMAP_17_043: [ConstMap_GetInternals shall produce in *keys a pointer to an array of const char* having all the keys stored so far by the map.] */
	/*Tests_SRS_CONSTMAP_17_044: [ConstMap_GetInternals shall produce in *values a pointer to an array of const char* having all the values stored so far by the map.] */
	/*Tests_SRS_CONSTMAP_17_045: [ ConstMap_GetInternals shall produce in *count the number of stored keys and values.]*/
	TEST_FUNCTION(ConstMap_GetInternals_Success)
	{
		// Arrange
		CConstMapMocks mocks;
		const char*const* keys;
		const char*const* values;
		size_t count;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);
		mocks.ResetAllCalls();

		// Call to Map
		STRICT_EXPECTED_CALL(mocks, Map_GetInternals(IGNORED_PTR_ARG, &keys, &values, &count))
			.IgnoreArgument(1);

		///Act
		auto result = ConstMap_GetInternals(aHandle, &keys, &values, &count);

		///Assert
		ASSERT_ARE_EQUAL(CONSTMAP_RESULT, CONSTMAP_OK, result);
		ASSERT_ARE_EQUAL(void_ptr, VALID_CONST_CHAR_POINTER, keys);
		ASSERT_ARE_EQUAL(void_ptr, VALID_CONST_CHAR_POINTER, values);
		ASSERT_ARE_EQUAL(int, VALID_KV_COUNT, count);

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle);
	}

	/*Tests_SRS_CONSTMAP_17_046: [If parameter handle, keys, values or count is NULL then ConstMap_GetInternals shall return CONSTMAP_INVALIDARG.]*/
	TEST_FUNCTION(ConstMap_GetInternals_Null)
	{
		// Arrange
		CConstMapMocks mocks;
		const char*const* keys;
		const char*const* values;
		size_t count;

		CONSTMAP_HANDLE aHandle = NULL;

		///Act
		auto result = ConstMap_GetInternals(aHandle, &keys, &values, &count);

		///Assert
		ASSERT_ARE_EQUAL(CONSTMAP_RESULT, CONSTMAP_INVALIDARG, result);

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
	}

	TEST_FUNCTION(ConstMap_GetInternals_Failures)
	{
		// Arrange
		CConstMapMocks mocks;
		const char*const* keys;
		const char*const* values;
		size_t count;

		MAP_HANDLE sourceMap = VALID_MAP_HANDLE;
		CONSTMAP_HANDLE aHandle = ConstMap_Create(sourceMap);

		mocks.ResetAllCalls();

		// Call to Map_GetInternals (match with mapErrorList size)
		STRICT_EXPECTED_CALL(mocks, Map_GetInternals(IGNORED_PTR_ARG, &keys, &values, &count))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_GetInternals(IGNORED_PTR_ARG, &keys, &values, &count))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_GetInternals(IGNORED_PTR_ARG, &keys, &values, &count))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_GetInternals(IGNORED_PTR_ARG, &keys, &values, &count))
			.IgnoreArgument(1);
		STRICT_EXPECTED_CALL(mocks, Map_GetInternals(IGNORED_PTR_ARG, &keys, &values, &count))
			.IgnoreArgument(1);

		///Act
		MAP_RESULT mapErrorList[] = {
			MAP_ERROR,
			MAP_INVALIDARG,
			MAP_KEYEXISTS,
			MAP_KEYNOTFOUND,
			MAP_FILTER_REJECT
		};
		size_t errors = sizeof(mapErrorList) / sizeof(MAP_RESULT);
		CONSTMAP_RESULT constErrorList[] = {
			CONSTMAP_ERROR,
			CONSTMAP_INVALIDARG,
			CONSTMAP_ERROR,
			CONSTMAP_KEYNOTFOUND,
			CONSTMAP_ERROR
		};

		for (size_t e = 0; e < errors; e++)
		{
			currentMapResult = mapErrorList[e];
			auto result = ConstMap_GetInternals(aHandle, &keys, &values, &count);
			ASSERT_ARE_EQUAL(CONSTMAP_RESULT, constErrorList[e], result);
		}

		///Assert

		mocks.AssertActualAndExpectedCalls();

		//Ablution    
		ConstMap_Destroy(aHandle);
	}


END_TEST_SUITE(constmap_unittests)
