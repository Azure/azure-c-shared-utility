// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UMOCK_C_INTERNAL_H
#define UMOCK_C_INTERNAL_H

/* TODO: 
- Switch to a model where the args are held as an array of generic types in umockcall
- Double check all macros that they return the errors by on_umockc_error
*/

#ifdef __cplusplus
#include <cstdlib>
extern "C" {
#else
#include <stdlib.h>
#endif

#include "macro_utils.h"
#include "umocktypes.h"
#include "umockcall.h"
#include "umock_c.h"

#ifdef ENABLE_MOCKS
#define WITH_MOCK 1
#else
#define WITH_MOCK 0
#endif

extern int umock_c_add_expected_call(UMOCKCALL_HANDLE mock_call);
extern UMOCKCALL_HANDLE umock_c_add_actual_call(UMOCKCALL_HANDLE mock_call);
extern UMOCKCALL_HANDLE umock_c_get_last_expected_call(void);
extern void umock_c_indicate_error(UMOCK_C_ERROR_CODE error_code);

#define EXPAND(A) A

static char* stringify_buffer(const void* bytes, size_t length)
{
    size_t string_length = 2 + (4 * length);
    char* result;
    if (length > 1)
    {
        string_length += length - 1;
    }

    result = (char*)malloc(string_length + 1);
    if (result != NULL)
    {
        size_t i;

        result[0] = '[';
        for (i = 0; i < length; i++)
        {
            if (sprintf(result + 1 + (i * 5), "0x%02X ", ((const unsigned char*)bytes)[i]) < 0)
            {
                break;
            }
        }

        if (i < length)
        {
            free(result);
            result = NULL;
        }
        else
        {
            result[string_length - 1] = ']';
            result[string_length] = '\0';
        }
    }

    return result;
}

typedef struct ARG_BUFFER_TAG
{
    void* bytes;
    size_t length;
} ARG_BUFFER;

#define COUNT_OF(A) (sizeof(A) / sizeof((A)[0]))

/* Codes_SRS_UMOCK_C_01_002: [The macro shall generate a function signature in case ENABLE_MOCKS is not defined.] */
/* Codes_SRS_UMOCK_C_01_005: [**If ENABLE_MOCKS is not defined, MOCKABLE_FUNCTION shall only generate a declaration for the function.] */
#define MOCKABLE_FUNCTION_INTERNAL(result, function, ...) \
	result function(void);

#define COPY_ARG_TO_MOCK_STRUCT(arg_type, arg_name) umocktypes_copy(#arg_type, &mock_call_data->arg_name, &arg_name);
#define DECLARE_MOCK_CALL_STRUCT_STACK(arg_type, arg_name) arg_type arg_name;
#define MARK_ARG_AS_NOT_IGNORED(arg_type, arg_name) mock_call_data->C2(is_ignored_, arg_name) = 0;
#define MARK_ARG_AS_IGNORED(arg_type, arg_name) mock_call_data->C2(is_ignored_, arg_name) = 1;
#define CLEAR_OUT_ARG_BUFFERS(count, arg_type, arg_name) mock_call_data->out_arg_buffers[COUNT_OF(mock_call_data->out_arg_buffers) - DIV2(count)].bytes = NULL;
#define CLEAR_VALIDATE_ARG_BUFFERS(count, arg_type, arg_name) mock_call_data->validate_arg_buffers[COUNT_OF(mock_call_data->validate_arg_buffers) - DIV2(count)].bytes = NULL;
#define FREE_OUT_ARG_BUFFERS(count, arg_type, arg_name) free(typed_mock_call_data->out_arg_buffers[COUNT_OF(typed_mock_call_data->out_arg_buffers) - DIV2(count)].bytes);
#define FREE_VALIDATE_ARG_BUFFERS(count, arg_type, arg_name) free(typed_mock_call_data->validate_arg_buffers[COUNT_OF(typed_mock_call_data->validate_arg_buffers) - DIV2(count)].bytes);
#define ARG_IN_SIGNATURE(count, arg_type, arg_name) arg_type arg_name IFCOMMA(count)
#define ARG_NAME_ONLY_IN_CALL(count, arg_type, arg_name) arg_name IFCOMMA(count)
#define ARG_ASSIGN_IN_ARRAY(arg_type, arg_name) arg_name_local
#define DECLARE_IGNORE_FLAG_FOR_ARG(arg_type, arg_name) unsigned int C2(is_ignored_,arg_name) : 1;
#define COPY_IGNORE_ARG_BY_NAME_TO_MODIFIER(name, arg_type, arg_name) C2(mock_call_modifier->IgnoreArgument_,arg_name) = C4(ignore_argument_func_,name,_,arg_name);
#define COPY_VALIDATE_ARG_BY_NAME_TO_MODIFIER(name, arg_type, arg_name) C2(mock_call_modifier->ValidateArgument_,arg_name) = C4(validate_argument_func_,name,_,arg_name);
#define COPY_OUT_ARG_VALUE_FROM_MATCHED_CALL(count, arg_type, arg_name) \
    if (matched_call_data->out_arg_buffers[COUNT_OF(matched_call_data->out_arg_buffers) - DIV2(count)].bytes != NULL) \
    { \
        (void)memcpy(*((void**)(&arg_name)), matched_call_data->out_arg_buffers[COUNT_OF(matched_call_data->out_arg_buffers) - DIV2(count)].bytes, matched_call_data->out_arg_buffers[COUNT_OF(matched_call_data->out_arg_buffers) - DIV2(count)].length); \
    } \

#define STRINGIFY_ARGS_DECLARE_RESULT_VAR(count, arg_type, arg_name) \
    char* C2(arg_name,_stringified) \
    = (typed_mock_call_data->validate_arg_buffers[COUNT_OF(typed_mock_call_data->out_arg_buffers) - DIV2(count)].bytes != NULL) ? \
        stringify_buffer(typed_mock_call_data->validate_arg_buffers[COUNT_OF(typed_mock_call_data->validate_arg_buffers) - DIV2(count)].bytes, typed_mock_call_data->validate_arg_buffers[COUNT_OF(typed_mock_call_data->validate_arg_buffers) - DIV2(count)].length) : \
        umocktypes_stringify(TOSTRING(arg_type), &typed_mock_call_data->arg_name);

#define STRINGIFY_ARGS_CHECK_ARG_STRINGIFY_SUCCESS(arg_type, arg_name) if (C2(arg_name,_stringified) == NULL) is_error = 1;
#define STRINGIFY_ARGS_DECLARE_ARG_STRING_LENGTH(arg_type, arg_name) size_t C2(arg_name,_stringified_length) = strlen(C2(arg_name,_stringified));
#define STRINGIFY_ARGS_COUNT_LENGTH(arg_type, arg_name) args_string_length += C2(arg_name,_stringified_length);
#define STRINGIFY_ARGS_FREE_STRINGIFIED_ARG(arg_type, arg_name) free(C2(arg_name,_stringified));
#define STRINGIFY_ARGS_COPY_ARG_STRING(arg_type, arg_name) \
    if (arg_index > 0) \
    { \
        result[current_pos] = ','; \
        current_pos++; \
    } \
    (void)memcpy(result + current_pos, C2(arg_name,_stringified), C2(arg_name,_stringified_length) + 1); \
    current_pos += C2(arg_name, _stringified_length); \
    arg_index++;

/* Codes_SRS_UMOCK_C_01_096: [If the content of the code under test buffer and the buffer supplied to ValidateArgumentBuffer does not match then this should be treated as a mismatch in argument comparison for that argument.]*/
#define ARE_EQUAL_FOR_ARG(count, arg_type, arg_name) \
    if (result && \
        ((typed_left->validate_arg_buffers[COUNT_OF(typed_left->validate_arg_buffers) - DIV2(count)].bytes != NULL) && (memcmp(*((void**)&typed_right->arg_name), typed_left->validate_arg_buffers[COUNT_OF(typed_left->validate_arg_buffers) - DIV2(count)].bytes, typed_left->validate_arg_buffers[COUNT_OF(typed_left->validate_arg_buffers) - DIV2(count)].length) != 0)) \
        || ((typed_right->validate_arg_buffers[COUNT_OF(typed_right->validate_arg_buffers) - DIV2(count)].bytes != NULL) && (memcmp(*((void**)&typed_left->arg_name), typed_right->validate_arg_buffers[COUNT_OF(typed_right->validate_arg_buffers) - DIV2(count)].bytes, typed_right->validate_arg_buffers[COUNT_OF(typed_right->validate_arg_buffers) - DIV2(count)].length) != 0)) \
        ) \
    { \
        result = 0; \
    } \
    if (result && !(C2(typed_left->is_ignored_, arg_name) \
        || C2(typed_right->is_ignored_, arg_name)  \
        || (umocktypes_are_equal(#arg_type, &typed_left->arg_name, &typed_right->arg_name)) \
        )) \
    { \
        result = 0; \
    }

#define DECLARE_MOCK_CALL_MODIFIER(name, ...) \
        C2(mock_call_modifier_,name) mock_call_modifier; \
        C2(fill_mock_call_modifier_,name)(&mock_call_modifier);

#define DECLARE_IGNORE_ARGUMENT_FUNCTION_PROTOTYPE(name, arg_type, arg_name) \
    static C2(mock_call_modifier_,name) C4(ignore_argument_func_,name,_,arg_name)(void);

#define DECLARE_VALIDATE_ARGUMENT_FUNCTION_PROTOTYPE(name, arg_type, arg_name) \
    static C2(mock_call_modifier_,name) C4(validate_argument_func_,name,_,arg_name)(void);

#define IGNORE_ARGUMENT_FUNCTION_IN_MODIFIERS(name, arg_type, arg_name) \
    C4(ignore_argument_func_type_,name,_,arg_name) C2(IgnoreArgument_,arg_name);

#define VALIDATE_ARGUMENT_FUNCTION_IN_MODIFIERS(name, arg_type, arg_name) \
    C4(validate_argument_func_type_,name,_,arg_name) C2(ValidateArgument_,arg_name);

#define DECLARE_IGNORE_ARGUMENT_FUNCTION_TYPE(name, arg_type, arg_name) \
    typedef struct C2(_mock_call_modifier_,name) (*C4(ignore_argument_func_type_,name,_,arg_name))(void);

#define DECLARE_VALIDATE_ARGUMENT_FUNCTION_TYPE(name, arg_type, arg_name) \
    typedef struct C2(_mock_call_modifier_,name) (*C4(validate_argument_func_type_,name,_,arg_name))(void);

#define IGNORE_ARGUMENT_FUNCTION_IN_ARRAY(name, arg_type, arg_name) \
    &C4(ignore_argument_func_,name,_,arg_name),

#define VALIDATE_ARGUMENT_FUNCTION_IN_ARRAY(name, arg_type, arg_name) \
    &C4(validate_argument_func_,name,_,arg_name),

#define DECLARE_IGNORE_ARGUMENT_FUNCTION_ARRAY(name, ...) \
    static const C2(ignore_one_argument_func_type_,name) C2(ignore_one_argument_array_,name)[] = \
    { \
        FOR_EACH_2_KEEP_1(IGNORE_ARGUMENT_FUNCTION_IN_ARRAY, name, __VA_ARGS__) \
    };

#define DECLARE_VALIDATE_ARGUMENT_FUNCTION_ARRAY(name, ...) \
    static const C2(validate_one_argument_func_type_,name) C2(validate_one_argument_array_,name)[] = \
    { \
        FOR_EACH_2_KEEP_1(VALIDATE_ARGUMENT_FUNCTION_IN_ARRAY, name, __VA_ARGS__) \
    };

/* These 2 macros are used to check if a type is "void" or not */
#define TEST_void 0
#define IS_NOT_VOID(x) \
    IF(C2(TEST_,x), 1, 0)

/* Codes_SRS_UMOCK_C_01_076: [The IgnoreAllArguments call modifier shall record that for that specific call all arguments will be ignored for that specific call.] */
#define IMPLEMENT_IGNORE_ALL_ARGUMENTS_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_,name) C2(ignore_all_arguments_func_,name)(void) \
    { \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(MARK_ARG_AS_IGNORED, __VA_ARGS__),) \
        return mock_call_modifier; \
    } \

/* Codes_SRS_UMOCK_C_01_077: [The ValidateAllArguments call modifier shall record that for that specific call all arguments will be validated.] */
#define IMPLEMENT_VALIDATE_ALL_ARGUMENTS_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_,name) C2(validate_all_arguments_func_,name)(void) \
    { \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(MARK_ARG_AS_NOT_IGNORED, __VA_ARGS__),) \
        return mock_call_modifier; \
    } \

/* Codes_SRS_UMOCK_C_01_078: [The IgnoreArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be ignored for that specific call.] */
#define IMPLEMENT_IGNORE_ARGUMENT_BY_NAME_FUNCTION(name, arg_type, arg_name) \
    static C2(mock_call_modifier_,name) C4(ignore_argument_func_,name,_,arg_name)(void) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        if (mock_call_data != NULL) \
        { \
            C2(mock_call_data->is_ignored_,arg_name) = 1; \
        } \
        return mock_call_modifier; \
    } \

/* Codes_SRS_UMOCK_C_01_079: [The ValidateArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be validated for that specific call.]*/
#define IMPLEMENT_VALIDATE_ARGUMENT_BY_NAME_FUNCTION(name, arg_type, arg_name) \
    static C2(mock_call_modifier_,name) C4(validate_argument_func_,name,_,arg_name)(void) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        if (mock_call_data != NULL) \
        { \
            C2(mock_call_data->is_ignored_,arg_name) = 0; \
        } \
        return mock_call_modifier; \
    } \

/* Codes_SRS_UMOCK_C_01_080: [The IgnoreArgument call modifier shall record that the indexth argument will be ignored for that specific call.]*/
/* Codes_SRS_UMOCK_C_01_081: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.] */
#define IMPLEMENT_IGNORE_ARGUMENT_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_,name) C2(ignore_argument_func_,name)(size_t arg_index) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        if (mock_call_data != NULL) \
        { \
            IF(COUNT_ARG(__VA_ARGS__), \
                if ((arg_index < 1) || (arg_index > (sizeof(C2(ignore_one_argument_array_,name)) / sizeof(C2(ignore_one_argument_array_,name)[0])))) \
                { \
                    umock_c_indicate_error(UMOCK_C_ARG_INDEX_OUT_OF_RANGE); \
                } \
                else \
                { \
                    C2(ignore_one_argument_array_,name)[arg_index - 1](); \
                }, \
            ) \
        } \
        return mock_call_modifier; \
    } \

/* Codes_SRS_UMOCK_C_01_082: [The ValidateArgument call modifier shall record that the indexth argument will be validated for that specific call.]*/
/* Codes_SRS_UMOCK_C_01_083: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
#define IMPLEMENT_VALIDATE_ARGUMENT_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_,name) C2(validate_argument_func_,name)(size_t arg_index) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        if (mock_call_data != NULL) \
        { \
            IF(COUNT_ARG(__VA_ARGS__), \
                if ((arg_index < 1) || (arg_index > (sizeof(C2(validate_one_argument_array_,name)) / sizeof(C2(validate_one_argument_array_,name)[0])))) \
                { \
                    umock_c_indicate_error(UMOCK_C_ARG_INDEX_OUT_OF_RANGE); \
                } \
                else \
                { \
                    C2(validate_one_argument_array_,name)[arg_index - 1](); \
                }, \
            ) \
        } \
        return mock_call_modifier; \
    } \

/* Codes_SRS_UMOCK_C_01_084: [The SetReturn call modifier shall record that when an actual call is matched with the specific expected call, it shall return the result value to the code under test.] */
#define IMPLEMENT_SET_RETURN_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_,name) C2(set_return_func_,name)(return_type return_value) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        if (mock_call_data != NULL) \
        { \
            mock_call_data->return_value_set = 1; \
            (void)umocktypes_copy(#return_type, &mock_call_data->return_value, &return_value); \
        } \
        return mock_call_modifier; \
    }
/* Codes_SRS_UMOCK_C_01_085: [The SetFailReturn call modifier shall record a fail return value.]*/
#define IMPLEMENT_SET_FAIL_RETURN_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_,name) C2(set_fail_return_func_,name)(return_type return_value) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        if (mock_call_data != NULL) \
        { \
            mock_call_data->fail_return_value_set = 1; \
            (void)umocktypes_copy(#return_type, &mock_call_data->fail_return_value, &return_value); \
        } \
        return mock_call_modifier; \
    }

/* Codes_SRS_UMOCK_C_01_116: [ The argument targetted by CopyOutArgument shall also be marked as ignored. ] */
/* Codes_SRS_UMOCK_C_01_088: [The memory shall be copied.]*/
/* Codes_SRS_UMOCK_C_01_091: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
/* Codes_SRS_UMOCK_C_01_117: [ If any memory allocation error occurs, umock_c shall raise an error with the code UMOCK_C_MALLOC_ERROR. ]*/
/* Codes_SRS_UMOCK_C_01_118: [ If any other error occurs, umock_c shall raise an error with the code UMOCK_C_ERROR. ]*/
/* Codes_SRS_UMOCK_C_01_092: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
/* Codes_SRS_UMOCK_C_01_089: [The buffers for previous CopyOutArgumentBuffer calls shall be freed.]*/
/* Codes_SRS_UMOCK_C_01_133: [ If several calls to CopyOutArgumentBuffer are made, only the last buffer shall be kept. ]*/
#define IMPLEMENT_COPY_OUT_ARGUMENT_BUFFER_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_, name) C2(copy_out_argument_buffer_func_, name)(size_t index, const void* bytes, size_t length) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        if ((index < 1) || (index > DIV2(COUNT_ARG(__VA_ARGS__)))) \
        { \
            umock_c_indicate_error(UMOCK_C_ARG_INDEX_OUT_OF_RANGE); \
        } \
        else if ((bytes == NULL) || (length == 0)) \
        { \
            umock_c_indicate_error(UMOCK_C_INVALID_ARGUMENT_BUFFER); \
        } \
        else \
        { \
            C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
            if (mock_call_data == NULL) \
            { \
                umock_c_indicate_error(UMOCK_C_ERROR); \
            } \
            else \
            { \
                free(mock_call_data->out_arg_buffers[index - 1].bytes); \
                mock_call_data->out_arg_buffers[index - 1].bytes = malloc(length); \
                if (mock_call_data->out_arg_buffers[index - 1].bytes == NULL) \
                { \
                    umock_c_indicate_error(UMOCK_C_MALLOC_ERROR); \
                } \
                else \
                { \
                    (void)memcpy(mock_call_data->out_arg_buffers[index - 1].bytes, bytes, length); \
                    mock_call_data->out_arg_buffers[index - 1].length = length; \
                    mock_call_modifier.IgnoreArgument(index); \
                } \
            } \
        } \
        return mock_call_modifier; \
    } \

#define IMPLEMENT_COPY_OUT_ARGUMENT_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_,name) C2(copy_out_argument_func_,name)(size_t arg_index, void* value) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        return mock_call_modifier; \
    } \

/* Codes_SRS_UMOCK_C_01_095: [The ValidateArgumentBuffer call modifier shall copy the memory pointed to by bytes and being length bytes so that it is later compared against a pointer type argument when the code under test calls the mock function.] */
/* Codes_SRS_UMOCK_C_01_097: [ValidateArgumentBuffer shall implicitly perform an IgnoreArgument on the indexth argument.]*/
/* Codes_SRS_UMOCK_C_01_099: [If the index is out of range umock_c shall raise an error with the code UMOCK_C_ARG_INDEX_OUT_OF_RANGE.]*/
/* Codes_SRS_UMOCK_C_01_100: [If bytes is NULL or length is 0, umock_c shall raise an error with the code UMOCK_C_INVALID_ARGUMENT_BUFFER.] */
/* Codes_SRS_UMOCK_C_01_131: [ The memory pointed by bytes shall be copied. ]*/
/* Codes_SRS_UMOCK_C_01_132: [ If several calls to ValidateArgumentBuffer are made, only the last buffer shall be kept. ]*/
/* Codes_SRS_UMOCK_C_01_130: [ The buffers for previous ValidateArgumentBuffer calls shall be freed. ]*/
#define IMPLEMENT_VALIDATE_ARGUMENT_BUFFER_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_,name) C2(validate_argument_buffer_func_,name)(size_t index, const void* bytes, size_t length) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        if ((index < 1) || (index > DIV2(COUNT_ARG(__VA_ARGS__)))) \
        { \
            umock_c_indicate_error(UMOCK_C_ARG_INDEX_OUT_OF_RANGE); \
        } \
        else if ((bytes == NULL) || (length == 0)) \
        { \
            umock_c_indicate_error(UMOCK_C_INVALID_ARGUMENT_BUFFER); \
        } \
        else \
        { \
            C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
            if (mock_call_data == NULL) \
            { \
                umock_c_indicate_error(UMOCK_C_ERROR); \
            } \
            else \
            { \
                free(mock_call_data->validate_arg_buffers[index - 1].bytes); \
                mock_call_data->validate_arg_buffers[index - 1].bytes = malloc(length); \
                if (mock_call_data->validate_arg_buffers[index - 1].bytes == NULL) \
                { \
                    umock_c_indicate_error(UMOCK_C_MALLOC_ERROR); \
                } \
                else \
                { \
                    (void)memcpy(mock_call_data->validate_arg_buffers[index - 1].bytes, bytes, length); \
                    mock_call_data->validate_arg_buffers[index - 1].length = length; \
                    mock_call_modifier.IgnoreArgument(index); \
                } \
            } \
        } \
        return mock_call_modifier; \
    } \

#define IMPLEMENT_IGNORE_ALL_CALLS_FUNCTION(return_type, name, ...) \
    static C2(mock_call_modifier_,name) C2(ignore_all_calls_func_,name)(void) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        return mock_call_modifier; \
    } \

#define IMPLEMENT_MOCK_FUNCTION(function_prefix, args_ignored, return_type, name, ...) \
	C2(mock_call_modifier_,name) C2(function_prefix,name)(IF(COUNT_ARG(__VA_ARGS__),FOR_EACH_2_COUNTED(ARG_IN_SIGNATURE, __VA_ARGS__),void)) \
	{ \
        UMOCKCALL_HANDLE mock_call; \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_,name)* mock_call_data = (C2(mock_call_,name)*)malloc(sizeof(C2(mock_call_,name))); \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(COPY_ARG_TO_MOCK_STRUCT, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), IF(args_ignored, FOR_EACH_2(MARK_ARG_AS_IGNORED, __VA_ARGS__), FOR_EACH_2(MARK_ARG_AS_NOT_IGNORED, __VA_ARGS__)),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(CLEAR_OUT_ARG_BUFFERS, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(CLEAR_VALIDATE_ARG_BUFFERS, __VA_ARGS__),) \
        IF(IS_NOT_VOID(return_type),mock_call_data->return_value_set = 0;,) \
        IF(IS_NOT_VOID(return_type),mock_call_data->fail_return_value_set = 0;,) \
        mock_call = umockcall_create(#name, mock_call_data, C2(mock_call_data_free_func_,name), C2(mock_call_data_stringify_,name), C2(mock_call_data_are_equal_,name)); \
        (void)umock_c_add_expected_call(mock_call); \
		return mock_call_modifier; \
	} \

#define IMPLEMENT_STRICT_EXPECTED_MOCK(return_type, name, ...) \
    IMPLEMENT_MOCK_FUNCTION(umock_c_strict_expected_, 0, return_type, name, __VA_ARGS__)

#define IMPLEMENT_EXPECTED_MOCK(return_type, name, ...) \
    IMPLEMENT_MOCK_FUNCTION(umock_c_expected_, 1, return_type, name, __VA_ARGS__)

/* Codes_SRS_UMOCK_C_01_104: [The REGISTER_GLOBAL_MOCK_HOOK shall register a mock hook to be called every time the mocked function is called by production code.]*/
/* Codes_SRS_UMOCK_C_01_107: [If there are multiple invocations of REGISTER_GLOBAL_MOCK_HOOK, the last one shall take effect over the previous ones.] */
/* Codes_SRS_UMOCK_C_01_134: [ REGISTER_GLOBAL_MOCK_HOOK called with a NULL hook unregisters a previously registered hook. ]*/
#define IMPLEMENT_REGISTER_GLOBAL_MOCK_HOOK(return_type, name, ...) \
    void C2(set_global_mock_hook_,name)(C2(mock_hook_func_type_, name) mock_return_hook) \
    { \
        C2(mock_hook_,name) = mock_return_hook; \
    } \

/* Codes_SRS_UMOCK_C_01_108: [The REGISTER_GLOBAL_MOCK_RETURN shall register a return value to always be returned by a mock function.]*/
/* Codes_SRS_UMOCK_C_01_109: [If there are multiple invocations of REGISTER_GLOBAL_MOCK_RETURN, the last one shall take effect over the previous ones.]*/
#define IMPLEMENT_REGISTER_GLOBAL_MOCK_RETURN(return_type, name, ...) \
    IF(IS_NOT_VOID(return_type), void C2(set_global_mock_return_, name)(return_type return_value) \
    { \
        if (umocktypes_copy(TOSTRING(return_type), &C2(mock_call_default_result_,name), &return_value) != 0) \
        { \
            umock_c_indicate_error(UMOCK_C_ERROR); \
        } \
    }, ) \

/* Codes_SRS_UMOCK_C_01_003: [If ENABLE_MOCKS is defined, MOCKABLE_FUNCTION shall generate all the boilerplate code needed by the macros in umock API to function to record the calls. Note: a lot of code (including function definitions and bodies, global variables (both static and extern).] */
/* Codes_SRS_UMOCK_C_01_004: [If ENABLE_MOCKS is defined, MOCKABLE_FUNCTION shall generate the declaration of the function and code for the mocked function, thus allowing setting up of expectations in test functions.] */
/* Codes_SRS_UMOCK_C_01_014: [For each argument the argument value shall be stored for later comparison with actual calls.] */
/* Codes_SRS_UMOCK_C_01_017: [No arguments shall be saved by default, unless other modifiers state it.]*/
/* Codes_SRS_UMOCK_C_01_074: [When an expected call is recorded a call modifier interface in the form of a structure containing function pointers shall be returned to the caller.] */
/* Codes_SRS_UMOCK_C_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
/* Codes_SRS_UMOCK_C_01_127: [ IgnoreAllArguments shall only be available for mock functions that have arguments. ]*/
/* Codes_SRS_UMOCK_C_01_126: [ ValidateAllArguments shall only be available for mock functions that have arguments. ]*/
/* Codes_SRS_UMOCK_C_01_125: [ IgnoreArgument_{arg_name} shall only be available for mock functions that have arguments. ]*/
/* Codes_SRS_UMOCK_C_01_124: [ ValidateArgument_{arg_name} shall only be available for mock functions that have arguments. **]*/
/* Codes_SRS_UMOCK_C_01_123: [ IgnoreArgument shall only be available for mock functions that have arguments. ]*/
/* Codes_SRS_UMOCK_C_01_122: [ ValidateArgument shall only be available for mock functions that have arguments. ]*/
/* Codes_SRS_UMOCK_C_01_121: [ SetReturn shall only be available if the return type is not void. ]*/
/* Codes_SRS_UMOCK_C_01_120: [ SetFailReturn shall only be available if the return type is not void. ]*/
/* Codes_SRS_UMOCK_C_01_119: [ CopyOutArgumentBuffer shall only be available for mock functions that have arguments. ]*/
/* Codes_SRS_UMOCK_C_01_128: [ CopyOutArgument shall only be available for mock functions that have arguments. ]*/
/* Codes_SRS_UMOCK_C_01_129: [ ValidateArgumentBuffer shall only be available for mock functions that have arguments. ]*/
/* Codes_SRS_UMOCK_C_01_105: [The hook’s result shall be returned by the mock to the production code.]*/
/* Codes_SRS_UMOCK_C_01_106: [The signature for the hook shall be assumed to have exactly the same arguments and return as the mocked function.]*/
/* Codes_SRS_UMOCK_C_01_135: [ All parameters passed to the mock shall be passed down to the mock hook. ]*/
#define MOCKABLE_FUNCTION_INTERNAL_WITH_MOCK(return_type, name, ...) \
    typedef return_type (*C2(mock_hook_func_type_, name))(IF(COUNT_ARG(__VA_ARGS__),FOR_EACH_2_COUNTED(ARG_IN_SIGNATURE, __VA_ARGS__),void)); \
    static C2(mock_hook_func_type_,name) C2(mock_hook_,name) = NULL; \
    struct C2(_mock_call_modifier_,name); \
    IF(COUNT_ARG(__VA_ARGS__),typedef struct C2(_mock_call_modifier_,name) (*C2(ignore_all_arguments_func_type_,name))(void);,) \
    IF(COUNT_ARG(__VA_ARGS__),typedef struct C2(_mock_call_modifier_,name) (*C2(validate_all_arguments_func_type_,name))(void);,) \
    IF(IS_NOT_VOID(return_type),typedef struct C2(_mock_call_modifier_,name) (*C2(set_return_func_type_,name))(return_type return_value);,) \
    IF(IS_NOT_VOID(return_type),typedef struct C2(_mock_call_modifier_,name) (*C2(set_fail_return_func_type_,name))(return_type return_value);,) \
    IF(COUNT_ARG(__VA_ARGS__),typedef struct C2(_mock_call_modifier_,name) (*C2(copy_out_argument_func_type_,name))(size_t arg_index, void* value);,) \
    IF(COUNT_ARG(__VA_ARGS__),typedef struct C2(_mock_call_modifier_,name) (*C2(ignore_argument_func_type_,name))(size_t arg_index);,) \
    IF(COUNT_ARG(__VA_ARGS__),typedef struct C2(_mock_call_modifier_,name) (*C2(validate_argument_func_type_,name))(size_t arg_index);,) \
    typedef struct C2(_mock_call_modifier_,name) (*C2(ignore_all_calls_func_type_,name))(void); \
    IF(COUNT_ARG(__VA_ARGS__),typedef struct C2(_mock_call_modifier_,name) (*C2(validate_argument_buffer_func_type_,name))(size_t index, const void* bytes, size_t length);,) \
    IF(COUNT_ARG(__VA_ARGS__),typedef struct C2(_mock_call_modifier_,name) (*C2(copy_out_argument_buffer_func_type_,name))(size_t index, const void* bytes, size_t length);,) \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(DECLARE_IGNORE_ARGUMENT_FUNCTION_TYPE, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(DECLARE_VALIDATE_ARGUMENT_FUNCTION_TYPE, name, __VA_ARGS__),) \
    typedef struct C2(_mock_call_modifier_,name) \
    { \
        IF(COUNT_ARG(__VA_ARGS__),C2(ignore_all_arguments_func_type_,name) IgnoreAllArguments;,) \
        IF(COUNT_ARG(__VA_ARGS__),C2(validate_all_arguments_func_type_,name) ValidateAllArguments;,) \
        IF(IS_NOT_VOID(return_type),C2(set_return_func_type_,name) SetReturn;,) \
        IF(IS_NOT_VOID(return_type),C2(set_fail_return_func_type_,name) SetFailReturn;,) \
        IF(COUNT_ARG(__VA_ARGS__),C2(copy_out_argument_func_type_,name) CopyOutArgument;,) \
        IF(COUNT_ARG(__VA_ARGS__),C2(ignore_argument_func_type_,name) IgnoreArgument;,) \
        IF(COUNT_ARG(__VA_ARGS__),C2(validate_argument_func_type_,name) ValidateArgument;,) \
        C2(ignore_all_calls_func_type_,name) IgnoreAllCalls; \
        IF(COUNT_ARG(__VA_ARGS__),C2(validate_argument_buffer_func_type_,name) ValidateArgumentBuffer;,) \
        IF(COUNT_ARG(__VA_ARGS__),C2(copy_out_argument_buffer_func_type_,name) CopyOutArgumentBuffer;,) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(IGNORE_ARGUMENT_FUNCTION_IN_MODIFIERS, name, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(VALIDATE_ARGUMENT_FUNCTION_IN_MODIFIERS, name, __VA_ARGS__),) \
    } C2(mock_call_modifier_,name); \
    IF(IS_NOT_VOID(return_type),static C2(mock_call_modifier_,name) C2(set_return_func_,name)(return_type return_value);,) \
    IF(IS_NOT_VOID(return_type),static C2(mock_call_modifier_,name) C2(set_fail_return_func_,name)(return_type return_value);,) \
    IF(COUNT_ARG(__VA_ARGS__),static C2(mock_call_modifier_,name) C2(ignore_all_arguments_func_,name)(void);,) \
    IF(COUNT_ARG(__VA_ARGS__),static C2(mock_call_modifier_,name) C2(validate_all_arguments_func_,name)(void);,) \
    IF(COUNT_ARG(__VA_ARGS__),static C2(mock_call_modifier_,name) C2(copy_out_argument_func_,name)(size_t arg_index, void* value);,) \
    IF(COUNT_ARG(__VA_ARGS__),static C2(mock_call_modifier_,name) C2(ignore_argument_func_,name)(size_t arg_index);,) \
    IF(COUNT_ARG(__VA_ARGS__),static C2(mock_call_modifier_,name) C2(validate_argument_func_,name)(size_t arg_index);,) \
    static C2(mock_call_modifier_,name) C2(ignore_all_calls_func_,name)(void); \
    IF(COUNT_ARG(__VA_ARGS__),static C2(mock_call_modifier_,name) C2(validate_argument_buffer_func_,name)(size_t index, const void* bytes, size_t length);,) \
    IF(COUNT_ARG(__VA_ARGS__),static C2(mock_call_modifier_,name) C2(copy_out_argument_buffer_func_,name)(size_t index, const void* bytes, size_t length);,) \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(DECLARE_IGNORE_ARGUMENT_FUNCTION_PROTOTYPE, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(DECLARE_VALIDATE_ARGUMENT_FUNCTION_PROTOTYPE, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__),typedef struct C2(_mock_call_modifier_,name) (*C2(ignore_one_argument_func_type_,name))(void);,) \
    IF(COUNT_ARG(__VA_ARGS__), DECLARE_IGNORE_ARGUMENT_FUNCTION_ARRAY(name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__),typedef struct C2(_mock_call_modifier_,name) (*C2(validate_one_argument_func_type_,name))(void);,) \
    IF(COUNT_ARG(__VA_ARGS__), DECLARE_VALIDATE_ARGUMENT_FUNCTION_ARRAY(name, __VA_ARGS__),) \
    static void C2(fill_mock_call_modifier_,name)(C2(mock_call_modifier_,name)* mock_call_modifier) \
    { \
        IF(IS_NOT_VOID(return_type),mock_call_modifier->SetReturn = C2(set_return_func_,name);,) \
        IF(IS_NOT_VOID(return_type),mock_call_modifier->SetFailReturn = C2(set_fail_return_func_,name);,) \
        IF(COUNT_ARG(__VA_ARGS__),mock_call_modifier->IgnoreAllArguments = C2(ignore_all_arguments_func_,name);,) \
        IF(COUNT_ARG(__VA_ARGS__),mock_call_modifier->ValidateAllArguments = C2(validate_all_arguments_func_,name);,) \
        IF(COUNT_ARG(__VA_ARGS__),mock_call_modifier->CopyOutArgument = C2(copy_out_argument_func_,name);,) \
        IF(COUNT_ARG(__VA_ARGS__),mock_call_modifier->IgnoreArgument = C2(ignore_argument_func_,name);,) \
        IF(COUNT_ARG(__VA_ARGS__),mock_call_modifier->ValidateArgument = C2(validate_argument_func_,name);,) \
        mock_call_modifier->IgnoreAllCalls = C2(ignore_all_calls_func_,name); \
        IF(COUNT_ARG(__VA_ARGS__),mock_call_modifier->ValidateArgumentBuffer = C2(validate_argument_buffer_func_,name);,) \
        IF(COUNT_ARG(__VA_ARGS__),mock_call_modifier->CopyOutArgumentBuffer = C2(copy_out_argument_buffer_func_,name);,) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(COPY_IGNORE_ARG_BY_NAME_TO_MODIFIER, name, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(COPY_VALIDATE_ARG_BY_NAME_TO_MODIFIER, name, __VA_ARGS__),) \
    } \
    typedef struct C2(_mock_call_,name) \
    { \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(DECLARE_MOCK_CALL_STRUCT_STACK, __VA_ARGS__),) \
        IF(IS_NOT_VOID(return_type),return_type return_value;,) \
        IF(IS_NOT_VOID(return_type),return_type fail_return_value;,) \
        IF(COUNT_ARG(__VA_ARGS__),ARG_BUFFER out_arg_buffers[IF(COUNT_ARG(__VA_ARGS__), DIV2(COUNT_ARG(__VA_ARGS__)),1)];,) \
        IF(COUNT_ARG(__VA_ARGS__),ARG_BUFFER validate_arg_buffers[IF(COUNT_ARG(__VA_ARGS__), DIV2(COUNT_ARG(__VA_ARGS__)),1)];,) \
        IF(IS_NOT_VOID(return_type),unsigned int fail_return_value_set : 1;,) \
        IF(IS_NOT_VOID(return_type),unsigned int return_value_set : 1;,) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(DECLARE_IGNORE_FLAG_FOR_ARG, __VA_ARGS__),) \
    } C2(mock_call_,name); \
    char* C2(mock_call_data_stringify_,name)(void* mock_call_data) \
    { \
        char* result; \
        size_t counter = DIV2(COUNT_ARG(__VA_ARGS__)); \
        C2(mock_call_,name)* typed_mock_call_data = (C2(mock_call_,name)*)mock_call_data; \
        int is_error = 0; \
        size_t args_string_length = 0; \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(STRINGIFY_ARGS_DECLARE_RESULT_VAR, __VA_ARGS__), ) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(STRINGIFY_ARGS_CHECK_ARG_STRINGIFY_SUCCESS, __VA_ARGS__), ) \
        if (is_error != 0) \
        { \
            result = NULL; \
        } \
        else \
        { \
            IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(STRINGIFY_ARGS_DECLARE_ARG_STRING_LENGTH, __VA_ARGS__), ) \
            IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(STRINGIFY_ARGS_COUNT_LENGTH, __VA_ARGS__), ) \
            IF(COUNT_ARG(__VA_ARGS__), args_string_length += COUNT_ARG(__VA_ARGS__) - 1;,) \
            result = (char*)malloc(args_string_length + 1); \
            if (result != NULL) \
            { \
                if (args_string_length == 0) \
                { \
                    result[0] = '\0'; \
                } \
                else \
                { \
                    size_t current_pos = 0; \
                    size_t arg_index = 0; \
                    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(STRINGIFY_ARGS_COPY_ARG_STRING, __VA_ARGS__), ) \
                } \
            } \
        } \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(STRINGIFY_ARGS_FREE_STRINGIFIED_ARG, __VA_ARGS__), ) \
        return result; \
    } \
    int C2(mock_call_data_are_equal_,name)(void* left, void* right) \
    { \
        int result; \
        if (left == right) \
        { \
            result = 1; \
        } \
        else if ((left == NULL) || (right == NULL)) \
        { \
            result = 0; \
        } \
        else \
        { \
            C2(mock_call_,name)* typed_left = (C2(mock_call_,name)*)left; \
            C2(mock_call_,name)* typed_right = (C2(mock_call_,name)*)right; \
            result = 1; \
            IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(ARE_EQUAL_FOR_ARG, __VA_ARGS__), ) \
        } \
        return result; \
    } \
    void C2(mock_call_data_free_func_,name)(void* mock_call_data) \
    { \
        C2(mock_call_,name)* typed_mock_call_data = (C2(mock_call_,name)*)mock_call_data; \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(FREE_OUT_ARG_BUFFERS, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(FREE_VALIDATE_ARG_BUFFERS, __VA_ARGS__),) \
        IF(IS_NOT_VOID(return_type),if (typed_mock_call_data->return_value_set) \
        { \
            umocktypes_free(TOSTRING(return_type), &typed_mock_call_data->return_value); \
        } \
        if (typed_mock_call_data->fail_return_value_set) \
        { \
            umocktypes_free(TOSTRING(return_type), &typed_mock_call_data->fail_return_value); \
        },) \
        free(mock_call_data); \
    } \
    IF(IS_NOT_VOID(return_type),static return_type C2(mock_call_default_result_,name);,) \
    IF(COUNT_ARG(__VA_ARGS__),IMPLEMENT_IGNORE_ALL_ARGUMENTS_FUNCTION(return_type, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__),IMPLEMENT_VALIDATE_ALL_ARGUMENTS_FUNCTION(return_type, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(IMPLEMENT_IGNORE_ARGUMENT_BY_NAME_FUNCTION, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(IMPLEMENT_VALIDATE_ARGUMENT_BY_NAME_FUNCTION, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__),IMPLEMENT_IGNORE_ARGUMENT_FUNCTION(return_type, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__),IMPLEMENT_VALIDATE_ARGUMENT_FUNCTION(return_type, name, __VA_ARGS__),) \
    IF(IS_NOT_VOID(return_type),IMPLEMENT_SET_RETURN_FUNCTION(return_type, name, __VA_ARGS__),) \
    IF(IS_NOT_VOID(return_type),IMPLEMENT_SET_FAIL_RETURN_FUNCTION(return_type, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__),IMPLEMENT_COPY_OUT_ARGUMENT_BUFFER_FUNCTION(return_type, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__),IMPLEMENT_COPY_OUT_ARGUMENT_FUNCTION(return_type, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__),IMPLEMENT_VALIDATE_ARGUMENT_BUFFER_FUNCTION(return_type, name, __VA_ARGS__),) \
    IMPLEMENT_IGNORE_ALL_CALLS_FUNCTION(return_type, name, __VA_ARGS__) \
	return_type name(IF(COUNT_ARG(__VA_ARGS__),FOR_EACH_2_COUNTED(ARG_IN_SIGNATURE, __VA_ARGS__),void)) \
	{ \
        UMOCKCALL_HANDLE mock_call; \
        UMOCKCALL_HANDLE matched_call; \
        IF(IS_NOT_VOID(return_type),return_type result;,) \
        C2(mock_call_,name)* matched_call_data; \
        C2(mock_call_,name)* mock_call_data = (C2(mock_call_,name)*)malloc(sizeof(C2(mock_call_,name))); \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(COPY_ARG_TO_MOCK_STRUCT, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(MARK_ARG_AS_NOT_IGNORED, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(CLEAR_OUT_ARG_BUFFERS, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(CLEAR_VALIDATE_ARG_BUFFERS, __VA_ARGS__),) \
        mock_call = umockcall_create(#name, mock_call_data, C2(mock_call_data_free_func_,name), C2(mock_call_data_stringify_,name), C2(mock_call_data_are_equal_,name)); \
        matched_call = umock_c_add_actual_call(mock_call); \
        if (matched_call != NULL) \
        { \
            matched_call_data = (C2(mock_call_,name)*)umockcall_get_call_data(matched_call); \
            IF(IS_NOT_VOID(return_type),if (matched_call_data != NULL) \
            { \
                if (matched_call_data->return_value_set) \
                { \
                    (void)umocktypes_copy(#return_type, &result, &matched_call_data->return_value); \
                } \
                else \
                { \
                    if (C2(mock_hook_, name) != NULL) \
                    { \
                        IF(IS_NOT_VOID(return_type),result =,) C2(mock_hook_, name)(IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(ARG_NAME_ONLY_IN_CALL, __VA_ARGS__),)); \
                    } \
                    IF(IS_NOT_VOID(return_type),else \
                    { \
                        (void)umocktypes_copy(TOSTRING(return_type), &result, &C2(mock_call_default_result_,name)); \
                    },) \
                } \
            },) \
            IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(COPY_OUT_ARG_VALUE_FROM_MATCHED_CALL, __VA_ARGS__),) \
            umockcall_destroy(matched_call); \
        } \
        else \
        { \
            if (C2(mock_hook_, name) != NULL) \
            { \
                IF(IS_NOT_VOID(return_type),result =,) C2(mock_hook_, name)(IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_COUNTED(ARG_NAME_ONLY_IN_CALL, __VA_ARGS__),)); \
            } \
            IF(IS_NOT_VOID(return_type),else \
            { \
                (void)umocktypes_copy(TOSTRING(return_type), &result, &C2(mock_call_default_result_,name)); \
            },) \
        } \
		IF(IS_NOT_VOID(return_type),return result;,) \
	} \
    IMPLEMENT_STRICT_EXPECTED_MOCK(return_type, name, __VA_ARGS__) \
    IMPLEMENT_EXPECTED_MOCK(return_type, name, __VA_ARGS__) \
    IMPLEMENT_REGISTER_GLOBAL_MOCK_HOOK(return_type, name, __VA_ARGS__) \
    IMPLEMENT_REGISTER_GLOBAL_MOCK_RETURN(return_type, name, __VA_ARGS__) \
    IF(IS_NOT_VOID(return_type),void C2(set_global_mock_fail_return_,name)(return_type return_value) \
    { \
    },)

#ifdef __cplusplus
	}
#endif

#endif /* UMOCK_C_INTERNAL_H */
