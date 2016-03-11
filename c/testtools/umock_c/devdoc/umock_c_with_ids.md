#umock_c
 
#Overview

umock_c is a C mocking library that exposes APIs to allow:
-	defining mock functions, 
-	recording expected calls 
-	comparing expected calls with actual calls. 
On top of the basic functionality, additional convenience features like modifiers on expected calls are provided.

#Simple example

A test written with umock_c looks like below:

Let’s assume unit A depends on unit B. unit B has a function called test_dependency_1_arg.

In unit B’s header one would write:

```c
MOCKABLE_FUNCTION(int, test_dependency_1_arg, int, a);
```

Let’s assume unit A has a function called function_under_test.

A test that checks that function_under_test calls its dependency and injects a return value, while ignoring all arguments on the call looks like this:

```c
TEST_FUNCTION(my_first_test)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(44)
        .IgnoreAllArguments();

    // act
    function_under_test();

    // assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}
```

#Exposed API (umock_c.h)

```c
DEFINE_ENUM(UMOCK_C_ERROR_CODE,
    UMOCK_C_ARG_INDEX_OUT_OF_RANGE);

typedef void(*ON_UMOCK_C_ERROR)(UMOCK_C_ERROR_CODE error_code);

#define IGNORED_PTR_ARG (NULL)
#define IGNORED_NUM_ARG (0)

#define MOCKABLE_FUNCTION(result, function, ...) \
	...

#define REGISTER_GLOBAL_MOCK_RETURN_HOOK(mock_function, mock_hook_function) \
    ...

#define REGISTER_GLOBAL_MOCK_RETURN(mock_function, return_value) \
    ...

#define REGISTER_GLOBAL_MOCK_FAIL_RETURN(mock_function, fail_return_value) \
    ...

#define REGISTER_GLOBAL_MOCK_RETURNS(mock_function, return_value, fail_return_value) \
    ...
    ...

#define STRICT_EXPECTED_CALL(call) \
	...

#define EXPECTED_CALL(call) \
	...

    extern int umock_c_init(ON_UMOCK_C_ERROR on_umock_c_error);
    extern void umock_c_deinit(void);
    extern int umock_c_reset_all_calls(void);
    extern const char* umock_c_get_actual_calls(void);
    extern const char* umock_c_get_expected_calls(void);
```

##Mock definitions API

###MOCKABLE_FUNCTION

```c
MOCKABLE_FUNCTION(result, function, ...)
```

XX**SRS_UMOCK_C_01_001: [**MOCKABLE_FUNCTION shall be used to wrap function definition allowing the user to declare a function that can be mocked.**]**

XX**SRS_UMOCK_C_01_002: [**The macro shall generate a function signature in case ENABLE_MOCKS is not defined.**]**

XX**SRS_UMOCK_C_01_003: [**If ENABLE_MOCKS is defined, MOCKABLE_FUNCTION shall generate all the boilerplate code needed by the macros in umock API to function to record the calls. Note: a lot of code (including function definitions and bodies, global variables (both static and extern).**]**

Example:

```c
MOCKABLE_FUNCTION(int, test_function, int, arg1)
```

should generate for production code:

```c
int test_function(int arg1);
```

###ENABLE_MOCKS

XX**SRS_UMOCK_C_01_004: [**If ENABLE_MOCKS is defined, MOCKABLE_FUNCTION shall generate the declaration of the function and code for the mocked function, thus allowing setting up of expectations in test functions.**]**
XX**SRS_UMOCK_C_01_005: [**If ENABLE_MOCKS is not defined, MOCKABLE_FUNCTION shall only generate a declaration for the function.**]**

##umock init/deinit

###umock_c_init

```c
int umock_c_init(ON_UMOCK_C_ERROR on_umock_c_error);
```

umock_c_init is needed before performing any action related to umock_c calls (or registering any types).

**SRS_UMOCK_C_01_006: [**umock_c_init shall initialize umock_c.**]**

**SRS_UMOCK_C_01_007: [**umock_c_init called if already initialized shall fail and return a non-zero value.**]**
 
**SRS_UMOCK_C_01_008: [**umock_c_init shall initialize the umock supported types.**]**

**SRS_UMOCK_C_01_009: [**on_umock_c_error can be NULL.**]**

**SRS_UMOCK_C_01_010: [**If on_umock_c_error is non-NULL it shall be saved for later use (to be invoked whenever an umock_c error needs to be signaled to the user).**]**

###umock_c_deinit

```c
void umock_c_deinit(void);
```

**SRS_UMOCK_C_01_011: [**umock_c_deinit shall free all umock_c used resources.**]**
**SRS_UMOCK_C_01_012: [**If umock_c was not initialized, umock_c_deinit shall do nothing.**]**

##Expected calls recording API

###STRICT_EXPECTED_CALL

```c
STRICT_EXPECTED_CALL(call)
```

XX**SRS_UMOCK_C_01_013: [**STRICT_EXPECTED_CALL shall record that a certain call is expected.**]**
XX**SRS_UMOCK_C_01_014: [**For each argument the argument value shall be stored for later comparison with actual calls.**]**

XX**SRS_UMOCK_C_01_015: [**The call argument shall be the complete function invocation.**]**

Example:

```c
STRICT_EXPECTED_CALL(test_dependency_1_arg(42));
```

###EXPECTED_CALL

```c
EXPECTED_CALL(call)
```

XX**SRS_UMOCK_C_01_016: [**EXPECTED_CALL shall record that a certain call is expected.**]**
XX**SRS_UMOCK_C_01_017: [**No arguments shall be saved by default, unless other modifiers state it.**]**

XX**SRS_UMOCK_C_01_018: [**The call argument shall be the complete function invocation.**]**

Example:

```c
EXPECTED_CALL(test_dependency_1_arg(42));
```

##Call comparison API

###umock_c_reset_all_calls

```c
int umock_c_reset_all_calls(void);
```

**SRS_UMOCK_C_01_019: [**umock_c_reset_all_calls shall reset all calls (actual and expected).**]**

**SRS_UMOCK_C_01_020: [**On success, umock_c_reset_all_calls shall return 0.**]**
**SRS_UMOCK_C_01_021: [**In case of any error, umock_c_reset_all_calls shall return a non-zero value.**]**

###umock_c_get_expected_calls

```c
const char* umock_c_get_expected_calls(void);
```

**SRS_UMOCK_C_01_022: [**umock_c_get_expected_calls shall return all the calls that were expected, but were not fulfilled.**]**

**SRS_UMOCK_C_01_023: [**For each call, the format shall be "functionName(argument 1 value, ...)".**]**
**SRS_UMOCK_C_01_024: [**Each call shall be enclosed in "[]".**]**

Example:

For a call with the signature:

```c
int test_dependency_2_args(int a, int b)
```

if an expected call was recorded:

```c
STRICT_EXPECTED_CALL(test_dependency_2_args(42, 1));
```

umock_c_get_expected_calls would return:

```c
"[test_dependency_2_args(42,1)]"
```

###umock_c_get_actual_calls

```c
const char* umock_c_get_actual_calls(void);
```

**SRS_UMOCK_C_01_025: [**umock_c_get_actual_calls shall return all the actual calls that were not matched to expected calls.**]**

**SRS_UMOCK_C_01_026: [**For each call, the format shall be "functionName(argument 1 value, ...)".**]**
**SRS_UMOCK_C_01_027: [**Each call shall be enclosed in "[]". A call to umock_c_get_actual_calls shall not modify the actual calls that were recorded.**]** 

Example:

For a call with the signature:

```c
int test_dependency_2_args(int a, int b)
```

if an actual call was recorded:

```c
test_dependency_2_args(42, 2);
```

umock_c_get_actual_calls would return:

```c
"[test_dependency_2_args(42,2)]"
```

### Call comparison rules

XX**SRS_UMOCK_C_01_115: [** umock_c shall compare calls in order. **]** That means that "[A()][B()]" is different than "[B()][A()]". 

##Supported types

###Out of the box

Out of the box umock_c shall support the following types:
-	**SRS_UMOCK_C_01_028: [**char**]**
-	**SRS_UMOCK_C_01_029: [**unsigned char**]**
-	**SRS_UMOCK_C_01_030: [**short**]**
-	**SRS_UMOCK_C_01_031: [**unsigned short**]**
-	**SRS_UMOCK_C_01_032: [**int**]**
-	**SRS_UMOCK_C_01_033: [**unsigned int**]**
-	**SRS_UMOCK_C_01_034: [**long**]**
-	**SRS_UMOCK_C_01_035: [**unsigned long**]**
-	**SRS_UMOCK_C_01_036: [**long long**]**
-	**SRS_UMOCK_C_01_037: [**unsigned long long**]**
-	**SRS_UMOCK_C_01_038: [**float**]**
-	**SRS_UMOCK_C_01_039: [**double**]**
-	**SRS_UMOCK_C_01_040: [**long double**]**
-	**SRS_UMOCK_C_01_041: [**size_t**]**
-	**SRS_UMOCK_C_01_042: [**clock_t**]**
-	**SRS_UMOCK_C_01_043: [**time_t**]**
-	**SRS_UMOCK_C_01_044: [**struct tm**]**

###Custom types

**SRS_UMOCK_C_01_045: [**Custom types, like structures shall be supported by allowing the user to define a set of functions that can be used by umock_c to operate with these types.**]**

Five functions shall be provided to umock_c:
-	**SRS_UMOCK_C_01_046: [**A stringify function.**]**

**SRS_UMOCK_C_01_047: [**This function shall return the string representation of a value of the given type.**]**

-	**SRS_UMOCK_C_01_048: [**An are_equal function.**]**

**SRS_UMOCK_C_01_049: [**This function shall compare 2 values of the given type and return an int indicating whether they are equal (1 means equal, 0 means different).**]**

-	**SRS_UMOCK_C_01_050: [**A copy function.**]**

**SRS_UMOCK_C_01_051: [**This function shall make a copy of a value for the given type.**]**

-	**SRS_UMOCK_C_01_052: [**A free function.**]**

**SRS_UMOCK_C_01_053: [**This function shall free a copied value.**]**

####umockvalue_stringify_type

```c
char* umockvalue_stringify_{type}(const {type}* value)
```

**SRS_UMOCK_C_01_054: [**A stringify function shall allocate using malloc a char* and fill it with a string representation of value.**]**

**SRS_UMOCK_C_01_055: [**If any error is encountered during building the string representation, umockvalue_stringify_type shall return NULL.**]**

Example:

```c
char* umockvalue_stringify_int(const int* value)
{
    char* result;

    if (value == NULL)
    {
        result = NULL;
    }
    else
    {
        char temp_buffer[32];
        int length = sprintf(temp_buffer, "%d", *value);
        if (length < 0)
        {
            result = NULL;
        }
        else
        {
            result = (char*)malloc(length + 1);
            if (result != NULL)
            {
                memcpy(result, temp_buffer, length + 1);
            }
        }
    }

    return result;
}
```

####umockvalue_are_equal_type

```c
int umockvalue_are_equal_{type}(const {type}* left, const {type}* right)
```

**SRS_UMOCK_C_01_056: [**The umockvalue_are_equal_type function shall return 1 if the 2 values are equal and 0 if they are not.**]**

**SRS_UMOCK_C_01_057: [**If both left and right are NULL, umockvalue_are_equal_type shall return 1.**]**

**SRS_UMOCK_C_01_058: [**If only one of left and right is NULL, umockvalue_are_equal_type shall return 0.**]**

Example:

```c
int umockvalue_are_equal_int(const int* left, const int* right)
{
    int result;

    if (left == right)
    {
        result = 1;
    }
    else if ((left == NULL) || (right == NULL))
    {
        result = 0;
    }
    else
    {
        result = ((*left) == (*right)) ? 1 : 0;
    }

    return result;
}
```

####umockvalue_copy_type

```c
int umockvalue_copy_{type}({type}* destination, const {type}* source)
```

**SRS_UMOCK_C_01_059: [**The umockvalue_copy_type function shall copy the value from source to destination.**]**

**SRS_UMOCK_C_01_060: [**On success umockvalue_copy_type shall return 0.**]**

**SRS_UMOCK_C_01_061: [**If any of the arguments is NULL, umockvalue_copy_type shall return a non-zero value.**]**

**SRS_UMOCK_C_01_062: [**If any error occurs during copying the value, umockvalue_copy_type shall return a non-zero value.**]**

Example:

```c
int umockvalue_copy_int(int* destination, const int* source)
{
    int result;

    if ((destination == NULL) ||
        (source == NULL))
    {
        result = __LINE__;
    }
    else
    {
        *destination = *source;
        result = 0;
    }

    return result;
}
```

####umockvalue_free_type

```c
void umockvalue_free_{type}({type}* value)
```

**SRS_UMOCK_C_01_063: [**The umockvalue_free_type function shall free a value previously copied using umockvalue_copy_type.**]**
**SRS_UMOCK_C_01_064: [**If value is NULL, no free shall be performed.**]**

Example:

```c
void umockvalue_free_int(int* value)
{
    /* no free required for int */
}
```

####REGISTER_UMOCK_VALUE_TYPE

```c
REGISTER_UMOCK_VALUE_TYPE(value_type, stringify_func, are_equal_func, copy_func, free_func)
```

**SRS_UMOCK_C_01_065: [**REGISTER_UMOCK_VALUE_TYPE shall register the type identified by value_type to be usable by umock_c for argument and return types and instruct umock_c which functions to use for getting the stringify, are_equal, copy and free.**]**

Example:

```c
REGISTER_UMOCK_VALUE_TYPE(TEST_STRUCT*, umockvalue_stringify_TEST_STRUCT_ptr, umockvalue_are_equal_TEST_STRUCT_ptr, umockvalue_copy_TEST_STRUCT_ptr, umockvalue_free_TEST_STRUCT_ptr);
```

**SRS_UMOCK_C_01_066: [**If only the value_type is specified in the macro invocation then the stringify, are_equal, copy and free function names shall be automatically derived from the type as: umockvalue_stringify_value_type, umockvalue_are_equal_value_type, umockvalue_copy_value_type, umockvalue_free_value_type.**]**

Example:

```c
REGISTER_UMOCK_VALUE_TYPE(TEST_STRUCT);
```

###Extra optional C types

####umockvalue_charptr

**SRS_UMOCK_C_01_067: [**char* and const char* shall be supported out of the box through a separate header, umockvalue_charptr.h.**]**

**SRS_UMOCK_C_01_068: [**In order to enable the usage of char*, the function umockvalue_charptr_register_types can be used in the test suite init.**]**

**SRS_UMOCK_C_01_069: [**The signature shall be:

```c
int umockvalue_charptr_register_types(void);
```
**]**

**SRS_UMOCK_C_01_070: [**umockvalue_charptr_register_types shall return 0 on success and non-zero on failure.

####umockvalue_stdint

**SRS_UMOCK_C_01_071: [**The types in stdint.h shall be supported out of the box by including umockvalue_stdint.h.**]**

**SRS_UMOCK_C_01_072: [**In order to enable the usage of stdint types, the function umockvalue_stdint_register_types shall be used in the test suite init.**]**

```c
int umockvalue_stdint_register_types(void);
```

**SRS_UMOCK_C_01_073: [**umockvalue_stdint_register_types shall return 0 on success and non-zero on failure.**]**

##Call modifiers

XX**SRS_UMOCK_C_01_074: [**When an expected call is recorded a call modifier interface in the form of a structure containing function pointers shall be returned to the caller.**]**

That allows constructs like:

```c
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(44)
        .IgnoreAllArguments();
```

Note that each modifier function shall return a full modifier structure that allows chaining further call modifiers.

XX**SRS_UMOCK_C_01_075: [**The last modifier in a chain overrides previous modifiers if any collision occurs.**]**
Example: A ValidateAllArguments after a previous IgnoreAllArgument will still validate all arguments.

###IgnoreAllArguments(void)

XX**SRS_UMOCK_C_01_076: [**The IgnoreAllArguments call modifier shall record that for that specific call all arguments will be ignored for that specific call.**]**

###ValidateAllArguments(void)

XX**SRS_UMOCK_C_01_077: [**The ValidateAllArguments call modifier shall record that for that specific call all arguments will be validated.**]**

###IgnoreArgument_{arg_name}(void)

XX**SRS_UMOCK_C_01_078: [**The IgnoreArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be ignored for that specific call.**]**

###ValidateArgument_{arg_name}(void)

XX**SRS_UMOCK_C_01_079: [**The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.**]**

###IgnoreArgument(size_t index)

XX**SRS_UMOCK_C_01_080: [**The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.**]**

**SRS_UMOCK_C_01_081: [**If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.**]**

###ValidateArgument(size_t index)

XX**SRS_UMOCK_C_01_082: [**The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.**]**

**SRS_UMOCK_C_01_083: [**If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.**]**

###SetReturn(return_type result)

XX**SRS_UMOCK_C_01_084: [**The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.**]**

###SetFailReturn(return_type result)

**SRS_UMOCK_C_01_085: [**The SetFailReturn call modifier shall record a fail return value.**]**
The fail return value can be recorded for more advanced features that would require failing or succeeding certain calls based on decisions made at runtime.

###CopyOutArgumentBuffer(size_t index, const void* bytes, size_t length)

XX**SRS_UMOCK_C_01_087: [**The CopyOutArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later injected as an out argument when the code under test calls the mock function.**]**

XX**SRS_UMOCK_C_01_088: [**The memory shall be copied. If several calls to CopyOutArgumentBuffer are made, only the last buffer shall be kept.**]**

XX**SRS_UMOCK_C_01_089: [**The buffers for the previous calls shall be freed.**]**

**SRS_UMOCK_C_01_090: [**CopyOutArgumentBuffer shall only be applicable to pointer types.**]**

**SRS_UMOCK_C_01_091: [**If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.**]**

**SRS_UMOCK_C_01_092: [**If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.**]**

XX**SRS_UMOCK_C_01_116: [** The argument targetted by CopyOutArgumentBuffer shall also be marked as ignored. **]**

###CopyOutArgument(arg_type value)

**SRS_UMOCK_C_01_093: [**The CopyOutArgument call modifier shall copy an argument value to be injected as an out argument value when the code under test calls the mock function.**]**

**SRS_UMOCK_C_01_094: [**CopyOutArgument shall only be applicable to pointer types.**]**

###ValidateArgumentBuffer(size_t index, const void* bytes, size_t length)

**SRS_UMOCK_C_01_095: [**The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.**]**

**SRS_UMOCK_C_01_096: [**If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer do not match then this should be treated as any other mismatch in argument comparison for that argument.**]**
**SRS_UMOCK_C_01_097: [**ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.**]**

**SRS_UMOCK_C_01_098: [**ValidateArgumentBuffer shall only be applicable to pointer types.**]**

**SRS_UMOCK_C_01_099: [**If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.**]**

**SRS_UMOCK_C_01_100: [**If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.**]**

###IgnoreAllCalls(void)

**SRS_UMOCK_C_01_101: [**The IgnoreAllCalls call modifier shall record that all calls matching the expected call shall be ignored. If no matching call occurs no missing call shall be reported.**]**
**SRS_UMOCK_C_01_102: [**If multiple matching actual calls occur no unexpected calls shall be reported.**]**
**SRS_UMOCK_C_01_103: [**The call matching shall be done taking into account arguments and call modifiers referring to arguments.**]**

##Global mock modifiers

###REGISTER_GLOBAL_MOCK_RETURN_HOOK

```c
REGISTER_GLOBAL_MOCK_RETURN_HOOK(mock_function, mock_hook_function)
```

**SRS_UMOCK_C_01_104: [**The REGISTER_GLOBAL_MOCK_RETURN_HOOK shall register a mock hook to be called every time the mocked function is called by production code.**]**
**SRS_UMOCK_C_01_105: [**The hook’s result shall be returned by the mock to the production code.**]**

**SRS_UMOCK_C_01_106: [**The signature for the hook shall be assumed to have exactly the same arguments and return as the mocked function.**]**

**SRS_UMOCK_C_01_107: [**If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURN_HOOK, the last one shall take effect over the previous ones.**]**

###REGISTER_GLOBAL_MOCK_RETURN

```c
REGISTER_GLOBAL_MOCK_RETURN(mock_function, return_value)
```

**SRS_UMOCK_C_01_108: [**The REGISTER_GLOBAL_MOCK_RETURN shall register a return value to always be returned by a mock function.**]**

**SRS_UMOCK_C_01_109: [**If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURN, the last one shall take effect over the previous ones.**]**

**SRS_UMOCK_C_01_110: [**If no REGISTER_GLOBAL_MOCK_RETURN is performed for a mocked function, the mock will return a value declared as static of the same type as the functions return type.**]**

###REGISTER_GLOBAL_MOCK_FAIL_RETURN

```c
REGISTER_GLOBAL_MOCK_FAIL_RETURN(mock_function, fail_return_value)
```

**SRS_UMOCK_C_01_111: [**The REGISTER_GLOBAL_MOCK_FAIL_RETURN shall register a fail return value to be returned by a mock function when marked as failed in the expected calls.**]**

**SRS_UMOCK_C_01_112: [**If there are multiple invocations of REGISTER_GLOBAL_FAIL_MOCK_RETURN, the last one shall take effect over the previous ones.**]**

###REGISTER_GLOBAL_MOCK_RETURNS

```c
REGISTER_GLOBAL_MOCK_RETURNS(mock_function, return_value, fail_return_value)
```

**SRS_UMOCK_C_01_113: [**The REGISTER_GLOBAL_MOCK_RETURNS shall register both a success and a fail return value associated with a mock function.**]**

**SRS_UMOCK_C_01_114: [**If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURNS, the last one shall take effect over the previous ones.**]**
