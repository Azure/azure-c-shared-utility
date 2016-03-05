// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TEST_DEPENDENCY_H
#define TEST_DEPENDENCY_H

#include "umock_c.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TEST_STRUCT_TAG
    {
        int x;
    } TEST_STRUCT;

    /* Tests_SRS_UMOCK_C_01_001: [MOCKABLE_FUNCTION shall be used to wrap function definition allowing the user to declare a function that can be mocked.]*/
    MOCKABLE_FUNCTION(int, test_dependency_no_args);
    MOCKABLE_FUNCTION(int, test_dependency_1_arg, int, a);
    MOCKABLE_FUNCTION(int, test_dependency_2_args, int, a, int, b);
    MOCKABLE_FUNCTION(int, test_dependency_struct_arg, TEST_STRUCT, s);
    MOCKABLE_FUNCTION(int, test_dependency_char_star_arg, char*, s);

#ifdef __cplusplus
}
#endif

#endif /* TEST_DEPENDENCY_H */
