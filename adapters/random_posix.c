// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "azure_c_shared_utility/random.h"

void RANDOM_seed(unsigned int seed)
{
  srandom(seed);
}

int RANDOM_generate()
{
  return random();
}
