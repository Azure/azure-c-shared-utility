// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UMOCK_C_INTERNAL_H
#define UMOCK_C_INTERNAL_H

#ifdef __cplusplus
#include <cstdlib>
extern "C" {
#else
#include <stdlib.h>
#endif

#include "macro_utils.h"
#include "umockvalue.h"
#include "umockcall.h"

#ifdef ENABLE_MOCKS
#define WITH_MOCK 1
#else
#define WITH_MOCK 0
#endif

extern int umock_c_add_expected_call(UMOCKCALL_HANDLE mock_call);
extern UMOCKCALL_HANDLE umock_c_add_actual_call(UMOCKCALL_HANDLE mock_call);
extern UMOCKCALL_HANDLE umock_c_get_last_expected_call(void);

#define EXPAND(A) A

/* Codes_SRS_UMOCK_C_01_002: [The macro shall generate a function signature in case ENABLE_MOCKS is not defined.] */
/* Codes_SRS_UMOCK_C_01_005: [**If ENABLE_MOCKS is not defined, MOCKABLE_FUNCTION shall only generate a declaration for the function.] */
#define MOCKABLE_FUNCTION_INTERNAL(result, function, ...) \
	result function(void);

#define COPY_ARG_TO_MOCK_STRUCT(arg_type, arg_name) umockvalue_copy(#arg_type, &mock_call_data->arg_name, &arg_name);
#define DECLARE_MOCK_CALL_STRUCT_STACK(arg_type, arg_name) arg_type arg_name;
#define DECLARE_IGNORE_FLAG_FOR_ARG(arg_type, arg_name) unsigned int C2(is_ignored_, arg_name);
#define MARK_ARG_AS_NOT_IGNORED(arg_type, arg_name) mock_call_data->C2(is_ignored_, arg_name) = 0;
#define MARK_ARG_AS_IGNORED(arg_type, arg_name) mock_call_data->C2(is_ignored_, arg_name) = 1;
#define ARG_IN_SIGNATURE(count, arg_type, arg_name) arg_type arg_name IFCOMMA(count)
#define ARG_ASSIGN_IN_ARRAY(arg_type, arg_name) arg_name_local
#define COPY_IGNORE_ARG_BY_NAME_TO_MODIFIER(name, arg_type, arg_name) C2(mock_call_modifier->IgnoreArgument_,arg_name) = C4(ignore_argument_func_,name,_,arg_name);
#define COPY_VALIDATE_ARG_BY_NAME_TO_MODIFIER(name, arg_type, arg_name) C2(mock_call_modifier->ValidateArgument_,arg_name) = C4(validate_argument_func_,name,_,arg_name);

#define STRINGIFY_ARGS_DECLARE_RESULT_VAR(arg_type, arg_name) char* C2(arg_name,_stringified) = umockvalue_stringify(#arg_type, &typed_mock_call_data->arg_name);
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

#define ARE_EQUAL_FOR_ARG(arg_type, arg_name) \
    if (result && !(C2(typed_left->is_ignored_, arg_name) || C2(typed_right->is_ignored_, arg_name) || (umockvalue_are_equal(#arg_type, &typed_left->arg_name, &typed_right->arg_name)))) \
    { \
        result = 0; \
    }

#define DECLARE_MOCK_CALL_MODIFIER(name, ...) \
        C2(mock_call_modifier_,name) mock_call_modifier; \
        C2(fill_mock_call_modifier_,name)(&mock_call_modifier);

#define IMPLEMENT_IGNORE_ARGUMENT_FUNCTION(name, arg_type, arg_name) \
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

#define IMPLEMENT_VALIDATE_ARGUMENT_FUNCTION(name, arg_type, arg_name) \
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

/* Codes_SRS_UMOCK_C_01_003: [If ENABLE_MOCKS is defined, MOCKABLE_FUNCTION shall generate all the boilerplate code needed by the macros in umock API to function to record the calls. Note: a lot of code (including function definitions and bodies, global variables (both static and extern).] */
/* Codes_SRS_UMOCK_C_01_004: [If ENABLE_MOCKS is defined, MOCKABLE_FUNCTION shall generate the declaration of the function and code for the mocked function, thus allowing setting up of expectations in test functions.] */
/* Codes_SRS_UMOCK_C_01_014: [For each argument the argument value shall be stored for later comparison with actual calls.] */
/* Codes_SRS_UMOCK_C_01_017: [No arguments shall be saved by default, unless other modifiers state it.]*/
/* Codes_SRS_UMOCK_C_01_074: [When an expected call is recorded a call modifier interface in the form of a structure containing function pointers shall be returned to the caller.] */
/* Codes_SRS_UMOCK_C_01_075: [The last modifier in a chain overrides previous modifiers if any collision occurs.]*/
/* Codes_SRS_UMOCK_C_01_076: [The IgnoreAllArguments call modifier shall record that for that specific call all arguments will be ignored for that specific call.] */
/* Codes_SRS_UMOCK_C_01_078: [The IgnoreArgument_{arg_name} call modifier shall record that the argument identified by arg_name will be ignored for that specific call.] */
#define MOCKABLE_FUNCTION_INTERNAL_WITH_MOCK(return_type, name, ...) \
    struct C2(_mock_call_modifier_,name); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(ignore_all_arguments_func_type_,name))(void); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(validate_all_arguments_func_type_,name))(void); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(set_return_func_type_,name))(return_type return_value); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(set_fail_return_func_type_,name))(return_type return_value); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(copy_out_argument_func_type_,name))(size_t arg_index, void* value); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(ignore_argument_func_type_,name))(size_t arg_index); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(validate_argument_func_type_,name))(size_t arg_index); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(ignore_all_calls_func_type_,name))(void); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(validate_argument_buffer_func_type_,name))(size_t index, const unsigned char* bytes, size_t length); \
    typedef struct C2(_mock_call_modifier_,name) (*C2(copy_out_argument_buffer_func_type_,name))(size_t index, const unsigned char* bytes, size_t length); \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(DECLARE_IGNORE_ARGUMENT_FUNCTION_TYPE, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(DECLARE_VALIDATE_ARGUMENT_FUNCTION_TYPE, name, __VA_ARGS__),) \
    typedef struct C2(_mock_call_modifier_,name) \
    { \
        C2(ignore_all_arguments_func_type_,name) IgnoreAllArguments; \
        C2(validate_all_arguments_func_type_,name) ValidateAllArguments; \
        C2(set_return_func_type_,name) SetReturn; \
        C2(set_fail_return_func_type_,name) SetFailReturn; \
        C2(copy_out_argument_func_type_,name) CopyOutArgument; \
        C2(ignore_argument_func_type_,name) IgnoreArgument; \
        C2(validate_argument_func_type_,name) ValidateArgument; \
        C2(ignore_all_calls_func_type_,name) IgnoreAllCalls; \
        C2(validate_argument_buffer_func_type_,name) ValidateArgumentBuffer; \
        C2(copy_out_argument_buffer_func_type_,name) CopyOutArgumentBuffer; \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(IGNORE_ARGUMENT_FUNCTION_IN_MODIFIERS, name, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(VALIDATE_ARGUMENT_FUNCTION_IN_MODIFIERS, name, __VA_ARGS__),) \
    } C2(mock_call_modifier_,name); \
    static C2(mock_call_modifier_,name) C2(set_return_func_,name)(return_type return_value); \
    static C2(mock_call_modifier_,name) C2(set_fail_return_func_,name)(return_type return_value); \
    static C2(mock_call_modifier_,name) C2(ignore_all_arguments_func_,name)(void); \
    static C2(mock_call_modifier_,name) C2(validate_all_arguments_func_,name)(void); \
    static C2(mock_call_modifier_,name) C2(copy_out_argument_func_,name)(size_t arg_index, void* value); \
    static C2(mock_call_modifier_,name) C2(ignore_argument_func_,name)(size_t arg_index); \
    static C2(mock_call_modifier_,name) C2(validate_argument_func_,name)(size_t arg_index); \
    static C2(mock_call_modifier_,name) C2(ignore_all_calls_func_,name)(void); \
    static C2(mock_call_modifier_,name) C2(validate_argument_buffer_func_,name)(size_t index, const unsigned char* bytes, size_t length); \
    static C2(mock_call_modifier_,name) C2(copy_out_argument_buffer_func_,name)(size_t index, const unsigned char* bytes, size_t length); \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(DECLARE_IGNORE_ARGUMENT_FUNCTION_PROTOTYPE, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(DECLARE_VALIDATE_ARGUMENT_FUNCTION_PROTOTYPE, name, __VA_ARGS__),) \
    static void C2(fill_mock_call_modifier_,name)(C2(mock_call_modifier_,name)* mock_call_modifier) \
    { \
        mock_call_modifier->SetReturn = C2(set_return_func_,name); \
        mock_call_modifier->SetFailReturn = C2(set_fail_return_func_,name); \
        mock_call_modifier->IgnoreAllArguments = C2(ignore_all_arguments_func_,name); \
        mock_call_modifier->ValidateAllArguments = C2(validate_all_arguments_func_,name); \
        mock_call_modifier->CopyOutArgument = C2(copy_out_argument_func_,name); \
        mock_call_modifier->IgnoreArgument = C2(ignore_argument_func_,name); \
        mock_call_modifier->ValidateArgument = C2(validate_argument_func_,name); \
        mock_call_modifier->IgnoreAllCalls = C2(ignore_all_calls_func_,name); \
        mock_call_modifier->ValidateArgumentBuffer = C2(validate_argument_buffer_func_,name); \
        mock_call_modifier->CopyOutArgumentBuffer = C2(copy_out_argument_buffer_func_,name); \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(COPY_IGNORE_ARG_BY_NAME_TO_MODIFIER, name, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(COPY_VALIDATE_ARG_BY_NAME_TO_MODIFIER, name, __VA_ARGS__),) \
    } \
    typedef struct C2(_mock_call_,name) \
    { \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(DECLARE_MOCK_CALL_STRUCT_STACK, __VA_ARGS__),) \
        return_type return_value; \
        return_type fail_return_value; \
        unsigned int fail_return_value_set : 1; \
        unsigned int return_value_set : 1; \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(DECLARE_IGNORE_FLAG_FOR_ARG, __VA_ARGS__),) \
    } C2(mock_call_,name); \
    char* C2(mock_call_data_stringify_,name)(void* mock_call_data) \
    { \
        char* result; \
        size_t counter = DIV2(COUNT_ARG(__VA_ARGS__)); \
        C2(mock_call_,name)* typed_mock_call_data = (C2(mock_call_,name)*)mock_call_data; \
        int is_error = 0; \
        size_t args_string_length = 0; \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(STRINGIFY_ARGS_DECLARE_RESULT_VAR, __VA_ARGS__), ) \
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
            IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(ARE_EQUAL_FOR_ARG, __VA_ARGS__), ) \
        } \
        return result; \
    } \
    void C2(mock_call_data_free_func_,name)(void* mock_call_data) \
    { \
        free(mock_call_data); \
    } \
	return_type name(IF(COUNT_ARG(__VA_ARGS__),FOR_EACH_2_COUNTED(ARG_IN_SIGNATURE, __VA_ARGS__),void)) \
	{ \
        UMOCKCALL_HANDLE mock_call; \
        UMOCKCALL_HANDLE matched_call; \
        static return_type result; \
        C2(mock_call_,name)* matched_call_data; \
        C2(mock_call_,name)* mock_call_data = (C2(mock_call_,name)*)malloc(sizeof(C2(mock_call_,name))); \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(COPY_ARG_TO_MOCK_STRUCT, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(MARK_ARG_AS_NOT_IGNORED, __VA_ARGS__),) \
        mock_call = umockcall_create(#name, mock_call_data, C2(mock_call_data_free_func_,name), C2(mock_call_data_stringify_,name), C2(mock_call_data_are_equal_,name)); \
        matched_call = umock_c_add_actual_call(mock_call); \
        if (matched_call != NULL) \
        { \
            matched_call_data = (C2(mock_call_,name)*)umockcall_get_call_data(matched_call); \
            if (matched_call_data != NULL) \
            { \
                if (matched_call_data->return_value_set) \
                { \
                    (void)umockvalue_copy(#return_type, &result, &matched_call_data->return_value); \
                } \
            } \
            umockcall_destroy(matched_call); \
        } \
		return result; \
	} \
    static C2(mock_call_modifier_,name) C2(set_return_func_,name)(return_type return_value) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        if (mock_call_data != NULL) \
        { \
            mock_call_data->return_value_set = 1; \
            (void)umockvalue_copy(#return_type, &mock_call_data->return_value, &return_value); \
        } \
        return mock_call_modifier; \
    } \
    static C2(mock_call_modifier_,name) C2(set_fail_return_func_,name)(return_type return_value) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        if (mock_call_data != NULL) \
        { \
            mock_call_data->fail_return_value_set = 1; \
            (void)umockvalue_copy(#return_type, &mock_call_data->fail_return_value, &return_value); \
        } \
        return mock_call_modifier; \
    } \
    static C2(mock_call_modifier_,name) C2(copy_out_argument_func_,name)(size_t arg_index, void* value) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        return mock_call_modifier; \
    } \
    static C2(mock_call_modifier_,name) C2(ignore_argument_func_,name)(size_t arg_index) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        return mock_call_modifier; \
    } \
    static C2(mock_call_modifier_,name) C2(validate_argument_func_,name)(size_t arg_index) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        return mock_call_modifier; \
    } \
    static C2(mock_call_modifier_,name) C2(ignore_all_calls_func_,name)(void) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        return mock_call_modifier; \
    } \
    static C2(mock_call_modifier_,name) C2(validate_argument_buffer_func_,name)(size_t index, const unsigned char* bytes, size_t length) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        return mock_call_modifier; \
    } \
    static C2(mock_call_modifier_,name) C2(copy_out_argument_buffer_func_,name)(size_t index, const unsigned char* bytes, size_t length) \
    { \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        return mock_call_modifier; \
    } \
    static C2(mock_call_modifier_,name) C2(ignore_all_arguments_func_,name)(void) \
    { \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(MARK_ARG_AS_IGNORED, __VA_ARGS__),) \
        return mock_call_modifier; \
    } \
    static C2(mock_call_modifier_,name) C2(validate_all_arguments_func_,name)(void) \
    { \
        C2(mock_call_, name)* mock_call_data = (C2(mock_call_, name)*)umockcall_get_call_data(umock_c_get_last_expected_call()); \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(MARK_ARG_AS_NOT_IGNORED, __VA_ARGS__),) \
        return mock_call_modifier; \
    } \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(IMPLEMENT_IGNORE_ARGUMENT_FUNCTION, name, __VA_ARGS__),) \
    IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2_KEEP_1(IMPLEMENT_VALIDATE_ARGUMENT_FUNCTION, name, __VA_ARGS__),) \
	C2(mock_call_modifier_,name) C2(umock_c_expected_,name)(IF(COUNT_ARG(__VA_ARGS__),FOR_EACH_2_COUNTED(ARG_IN_SIGNATURE, __VA_ARGS__),void)) \
	{ \
        UMOCKCALL_HANDLE mock_call; \
        DECLARE_MOCK_CALL_MODIFIER(name) \
        C2(mock_call_,name)* mock_call_data = (C2(mock_call_,name)*)malloc(sizeof(C2(mock_call_,name))); \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(COPY_ARG_TO_MOCK_STRUCT, __VA_ARGS__),) \
        IF(COUNT_ARG(__VA_ARGS__), FOR_EACH_2(MARK_ARG_AS_NOT_IGNORED, __VA_ARGS__),) \
        mock_call_data->return_value_set = 0; \
        mock_call_data->fail_return_value_set = 0; \
        mock_call = umockcall_create(#name, mock_call_data, C2(mock_call_data_free_func_,name), C2(mock_call_data_stringify_,name), C2(mock_call_data_are_equal_,name)); \
        (void)umock_c_add_expected_call(mock_call); \
		return mock_call_modifier; \
	} \
    typedef return_type (*C2(mock_hook_func_type_, name))(IF(COUNT_ARG(__VA_ARGS__),FOR_EACH_2_COUNTED(ARG_IN_SIGNATURE, __VA_ARGS__),void)); \
    static C2(mock_hook_func_type_,name) C2(mock_hook_,name); \
    void C2(set_global_mock_return_,name)(return_type return_value) \
    { \
    } \
    void C2(set_global_mock_fail_return_,name)(return_type return_value) \
    { \
    }

#ifdef __cplusplus
	}
#endif

#endif /* UMOCK_C_INTERNAL_H */
