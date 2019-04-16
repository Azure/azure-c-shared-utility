// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef ENV_VARIABLE_H
#define ENV_VARIABLE_H

#include "azure_macro_utils/macro_utils.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

#include "umock_c/umock_c_prod.h"


MOCKABLE_FUNCTION(, const char*, environment_get_variable, const char*, variable_name);

#ifdef __cplusplus
}
#endif

#endif /* ENV_VARIABLE_H */
