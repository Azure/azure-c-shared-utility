// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SOME_REFCOUNT_IMPL_H
#define SOME_REFCOUNT_IMPL_H

#include "refcount.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct pos
{
    int x;
}pos ;

DECLARE_REFCOUNT_TYPE(pos);

#ifdef __cplusplus
}
#endif

#endif /*SOME_REFCOUNT_IMPL_H*/
