// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PAL_THREADING_H
#define PAL_THREADING_H

#include <time.h>

#ifdef __cplusplus
#include <cstdbool>
extern "C" {
#include <stdbool.h>
#endif /* __cplusplus */

    typedef enum { LOCK_OK, LOCK_ERROR } LOCK_RESULT;
    typedef struct PAL_LOCK_INSTANCE_TAG* LOCK_HANDLE;
    typedef struct PAL_THREAD_INSTANCE_TAG* THREAD_HANDLE;
    typedef int(*THREAD_START_FUNC)(void *);

    // Returns a lock handle that can be used in subsequent calls to lock/unlock.
    LOCK_HANDLE pal_lock_create();
    // Release a lock's resources
    void pal_lock_destroy(LOCK_HANDLE lock);

    LOCK_RESULT pal_lock(LOCK_HANDLE lock);
    LOCK_RESULT pal_unlock(LOCK_HANDLE lock);

    // Create a thread with the specified entry point and argument. Returns NULL on failure.
    THREAD_HANDLE pal_thread_create(THREAD_START_FUNC func, void * arg);

    // Wait for the specified thread to exit, and receive its result in exit_code. The
    // exit_code parameter may be NULL. Returns 0 on success.
    int pal_thread_join(THREAD_HANDLE thread_handle, int* exit_code);

    // Called by the created thread when exiting.
    void pal_thread_exit(int exit_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAL_THREADING_H */