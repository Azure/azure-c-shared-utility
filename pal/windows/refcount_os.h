// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file gets included into refcount.h as a means of extending the behavior of
// atomic increment, decrement, and test. It gets included in two separate phases
// in order to make the macro definitions work properly.

// The first phase defines COUNT_TYPE
#ifndef REFCOUNT_OS_H__WINDOWS
#define REFCOUNT_OS_H__WINDOWS

#include "windows.h"
// The Windows atomic operations work on LONG
#define COUNT_TYPE LONG

#endif // REFCOUNT_OS_H__WINDOWS


// The second phase defines DEC_RETURN_ZERO, INC_REF, and DEC_REF
#ifdef REFCOUNT_OS_H__PHASE_TWO
#undef REFCOUNT_OS_H__PHASE_TWO
#ifndef REFCOUNT_OS_H__PHASE_TWO__IMPL
#define REFCOUNT_OS_H__PHASE_TWO__IMPL

/*if macro DEC_REF returns DEC_RETURN_ZERO that means the ref count has reached zero.*/
#define DEC_RETURN_ZERO (0)
#define INC_REF(type, var) InterlockedIncrement(&(((REFCOUNT_TYPE(type)*)var)->count))
#define DEC_REF(type, var) InterlockedDecrement(&(((REFCOUNT_TYPE(type)*)var)->count))

#endif // REFCOUNT_OS_H__PHASE_TWO__IMPL
#endif // REFCOUNT_OS_H__PHASE_TWO
