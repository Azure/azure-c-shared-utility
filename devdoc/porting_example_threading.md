# Optional threading functions for porting

This file demonstrates the optional threading functions required for porting the Azure IoT 
SDK to a new device. This example is from Linux.

```c
#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"

// TODO: add includes for your system's threading functions here

DEFINE_ENUM_STRINGS(THREADAPI_RESULT, THREADAPI_RESULT_VALUES);

typedef struct THREAD_INSTANCE_TAG
{
    pthread_t Pthread_handle;
    THREAD_START_FUNC ThreadStartFunc;
    void* Arg;
} THREAD_INSTANCE;

LOCK_HANDLE Lock_Init(void)
{
    pthread_mutex_t* result = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (result == NULL)
    {
        LogError("malloc failed.");
    }
    else
    {
        if (pthread_mutex_init(result, NULL) != 0)
        {
            LogError("pthread_mutex_init failed.");
            free(result);
            result = NULL;
        }
    }
    return (LOCK_HANDLE)result;
}

LOCK_RESULT Lock(LOCK_HANDLE handle)
{
    LOCK_RESULT result;
    if (handle == NULL)
    {
        LogError("Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        if (pthread_mutex_lock((pthread_mutex_t*)handle) == 0)
        {
            result = LOCK_OK;
        }
        else
        {
            LogError("pthread_mutex_lock failed.");
            result = LOCK_ERROR;
        }
    }
    return result;
}

LOCK_RESULT Unlock(LOCK_HANDLE handle)
{
    LOCK_RESULT result;
    if (handle == NULL)
    {
        LogError("Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        if (pthread_mutex_unlock((pthread_mutex_t*)handle) == 0)
        {
            result = LOCK_OK;
        }
        else
        {
            LogError("pthread_mutex_unlock failed.");
            result = LOCK_ERROR;
        }
    }
    return result;
}

LOCK_RESULT Lock_Deinit(LOCK_HANDLE handle)
{
    LOCK_RESULT result;
    if (NULL == handle)
    {
        LogError("Invalid argument; handle is NULL.");
        result = LOCK_ERROR;
    }
    else
    {
        if(pthread_mutex_destroy((pthread_mutex_t*)handle) == 0)
        {
            free(handle);
            handle = NULL;
            result = LOCK_OK;
        }
        else
        {
            LogError("pthread_mutex_destroy failed;");
            result = LOCK_ERROR;
        }
    }
    return result;
}

static void* ThreadWrapper(void* threadInstanceArg)
{
    THREAD_INSTANCE* threadInstance = (THREAD_INSTANCE*)threadInstanceArg;
    int result = threadInstance->ThreadStartFunc(threadInstance->Arg);
    return (void*)(intptr_t)result;
}

THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, 
    THREAD_START_FUNC func, void* arg)
{
    THREADAPI_RESULT result;

    if ((threadHandle == NULL) || (func == NULL))
    {
        result = THREADAPI_INVALID_ARG;
        LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
    }
    else
    {
        THREAD_INSTANCE* threadInstance = malloc(sizeof(THREAD_INSTANCE));
        if (threadInstance == NULL)
        {
            result = THREADAPI_NO_MEMORY;
            LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
        }
        else
        {
            threadInstance->ThreadStartFunc = func;
            threadInstance->Arg = arg;
            int createResult = pthread_create(&threadInstance->Pthread_handle, 
                NULL, ThreadWrapper, threadInstance);
            switch (createResult)
            {
            default:
                free(threadInstance);
                result = THREADAPI_ERROR;
                LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
                break;
            case 0:
                *threadHandle = threadInstance;
                result = THREADAPI_OK;
                break;
            case EAGAIN:
                free(threadInstance);
                result = THREADAPI_NO_MEMORY;
                LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
                break;
            }
        }
    }
    return result;
}

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* res)
{
    THREADAPI_RESULT result;

    THREAD_INSTANCE* threadInstance = (THREAD_INSTANCE*)threadHandle;
    if (threadInstance == NULL)
    {
        result = THREADAPI_INVALID_ARG;
        LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
    }
    else
    {
        void* threadResult;
        if (pthread_join(threadInstance->Pthread_handle, &threadResult) != 0)
        {
            result = THREADAPI_ERROR;
            LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
        }
        else
        {
            if (res != NULL)
            {
                *res = (int)(intptr_t)threadResult;
            }
            result = THREADAPI_OK;
        }
        free(threadInstance);
    }
    return result;
}

void ThreadAPI_Exit(int res)
{
    pthread_exit((void*)(intptr_t)res);
}
```