// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#undef MOCKABLE_FUNCTION

#ifdef ENABLE_MOCKS
/* Codes_SRS_UMOCK_C_LIB_01_001: [MOCKABLE_FUNCTION shall be used to wrap function definition allowing the user to declare a function that can be mocked.]*/
#define MOCKABLE_FUNCTION(result, function, ...) \
    MOCKABLE_FUNCTION_UMOCK_INTERNAL_WITH_MOCK(result, function, __VA_ARGS__)
#else
/* Codes_SRS_UMOCK_C_LIB_01_001: [MOCKABLE_FUNCTION shall be used to wrap function definition allowing the user to declare a function that can be mocked.]*/
#define MOCKABLE_FUNCTION(result, function, ...) \
    MOCKABLE_FUNCTION_UMOCK_INTERNAL(result, function, __VA_ARGS__)
#endif

#include "umock_c.h"
