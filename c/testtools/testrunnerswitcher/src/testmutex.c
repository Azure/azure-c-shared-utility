// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testmutex.h"

#ifdef WIN32

#include "windows.h"

TEST_MUTEX_HANDLE testmutex_create(void)
{
    return (TEST_MUTEX_HANDLE)CreateMutexW(NULL, FALSE, NULL);
}

int testmutex_acquire(TEST_MUTEX_HANDLE mutex)
{
    return (WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0) ? 0 : 1;
}

void testmutex_destroy(TEST_MUTEX_HANDLE mutex)
{
    (void)CloseHandle(mutex);
}

int testmutex_release(TEST_MUTEX_HANDLE mutex)
{   
    return ReleaseMutex(mutex);
}
#endif
