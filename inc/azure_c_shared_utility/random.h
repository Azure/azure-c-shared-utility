// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef RANDOM_H
#define RANDOM_H

#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

MOCKABLE_FUNCTION(, void, RANDOM_seed, unsigned int, seed);
MOCKABLE_FUNCTION(, int, RANDOM_generate);

#ifdef __cplusplus
}
#endif

#endif /* RANDOM_H */
