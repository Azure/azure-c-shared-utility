// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef XIO_ASYNC_RESULT_H
#define XIO_ASYNC_RESULT_H

#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#define XIO_ASYNC_RESULT_VALUES \
    XIO_ASYNC_RESULT_SUCCESS = 1, \
    XIO_ASYNC_RESULT_WAITING = 0, \
    XIO_ASYNC_RESULT_FAILURE = -1

DEFINE_ENUM(XIO_ASYNC_RESULT, XIO_ASYNC_RESULT_VALUES);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIO_ASYNC_RESULT_H */
