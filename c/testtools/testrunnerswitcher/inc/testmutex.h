// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TESTMUTEX_H
#define TESTMUTEX_H

#ifdef WIN32
#include "windows.h"

#ifdef __cpluslplus
extern "C" {
#endif

typedef void* TEST_MUTEX_HANDLE;

TEST_MUTEX_HANDLE testmutex_create(void);
extern "C" int testmutex_acquire(TEST_MUTEX_HANDLE mutex);
extern "C" int testmutex_release(TEST_MUTEX_HANDLE mutex);
extern "C" void testmutex_destroy(TEST_MUTEX_HANDLE mutex);

#ifdef __cpluslplus
extern "C" {
#endif

#endif /* WIN32 */

#endif /*MICROMOCKTESTMUTEX_H*/

