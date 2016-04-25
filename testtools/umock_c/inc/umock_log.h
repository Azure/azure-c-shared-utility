// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UMOCK_LOG_H
#define UMOCK_LOG_H

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#define UMOCK_LOG(...) \
    (void)printf(__VA_ARGS__); \
    (void)printf("\r\n");

#endif /* UMOCK_LOG_H */
