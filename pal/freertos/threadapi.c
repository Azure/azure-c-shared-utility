// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/xlogging.h"

/*Codes_SRS_THREADAPI_FREERTOS_30_001: [ The threadapi_freertos shall implement the method ThreadAPI_Sleep defined in threadapi.h ]*/
#include "azure_c_shared_utility/threadapi.h"

#include "azure_c_shared_utility/gballoc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

typedef struct THREAD_INSTANCE_TAG
{
    TaskHandle_t xTaskHandle;
    THREAD_START_FUNC ThreadStartFunc;
    void* Arg;
    SemaphoreHandle_t xJoinBarrier; /**< Synchronizes the two callers of pthread_join. */
    SemaphoreHandle_t xJoinMutex; /**< Ensures that only one other thread may join this thread. */
} THREAD_INSTANCE;

/*Codes_SRS_THREADAPI_FREERTOS_30_002: [ The ThreadAPI_Sleep shall receive a time in milliseconds. ]*/
/*Codes_SRS_THREADAPI_FREERTOS_30_003: [ The ThreadAPI_Sleep shall stop the thread for the specified time. ]*/
void ThreadAPI_Sleep(unsigned int milliseconds)
{
    vTaskDelay((milliseconds * CONFIG_FREERTOS_HZ) / 1000);
}

static void* ThreadWrapper ( void* threadInstanceArg )
{
    THREAD_INSTANCE* threadInstance = ( THREAD_INSTANCE* ) threadInstanceArg;
    int result = threadInstance->ThreadStartFunc ( threadInstance->Arg );
    return ( void* ) ( intptr_t ) result;
}

THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{
    THREADAPI_RESULT result;
    BaseType_t xReturned;

    if ( ( threadHandle == NULL ) || ( func == NULL ) )
    {
        result = THREADAPI_INVALID_ARG;
        LogError( "(result = %s)" , ENUM_TO_STRING(THREADAPI_RESULT, result) );
    }
    else
    {
        THREAD_INSTANCE* threadInstance = malloc ( sizeof(THREAD_INSTANCE) );
        if ( threadInstance == NULL )
        {
            result = THREADAPI_NO_MEMORY;
            LogError( "(result = %s)" , ENUM_TO_STRING(THREADAPI_RESULT, result) );
        }
        else
        {
            /* Suspend all tasks to create a critical section. This ensures that
             * the new thread doesn't exit before a tag is assigned. */
            vTaskSuspendAll ( );

            threadInstance->ThreadStartFunc = func;
            threadInstance->Arg = arg;

            /* Stack:
             * The default stack size for POSIX Thread is 2 Mbytes, so for FreeRTOS port we will
             * use temporary 10000 word (Stack size for FreeRTOS shall be in words)
             * TODO: Try to be optimize stack size to be close the real memory need
             * Priority:
             * Will use the minimal priority for freeRTOS task 1 (note that priority 0 is reserved for idle task
             * created automatically by FreeRTOS scheduler)
             * Function name:
             * This parameter is used for debug issue only, we will use "ThreadAPI_Create" as thread name*/

            xReturned = xTaskCreate ( ( TaskFunction_t ) ThreadWrapper ,
                                      "ThreadAPI_Create" ,
                                      10000 ,
                                      threadInstance ,
                                      1 ,
                                      &threadInstance->xTaskHandle );

            if ( xReturned == pdPASS )
            {
                /// The task was created
                result = THREADAPI_OK;

                /// These calls will not fail when their arguments aren't NULL
                threadInstance->xJoinMutex = xSemaphoreCreateMutex ( );
                threadInstance->xJoinBarrier = xSemaphoreCreateBinary ( );

                /// Store the pointer to the task in the task tag
                vTaskSetApplicationTaskTag ( threadInstance->xTaskHandle , ( TaskHookFunction_t ) threadInstance );
            }
            else
            {
                result = THREADAPI_ERROR;
                LogError( "(result = %s)" , ENUM_TO_STRING(THREADAPI_RESULT, result) );
            }

            /// End the critical section
            xTaskResumeAll ( );
        }
    }

    return result;
}

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* res)
{
    THREADAPI_RESULT result = THREADAPI_OK;
    THREAD_INSTANCE* threadInstance = ( THREAD_INSTANCE* ) threadHandle;
    ( void ) res;
    BaseType_t ret = pdTRUE;
    TaskHandle_t ThisTaskHandle;

    do
    {
        if ( threadInstance == NULL )
        {
            result = THREADAPI_INVALID_ARG;
            LogError( "(result = %s)" , ENUM_TO_STRING(THREADAPI_RESULT, result) );
            break;
        }

        /* Only one thread may attempt to join another. Lock the join mutex
         * to prevent other threads from calling ThreadAPI_Join
         */
        ret = xSemaphoreTake( ( SemaphoreHandle_t ) &threadInstance->xJoinMutex, 0 );

        if ( ret != pdPASS )
        {
            result = THREADAPI_ERROR;
            LogError("Another thread has already joined the requested thread");
            break;
        }

        /// The calling thread cannot join its self
        ThisTaskHandle = ( TaskHandle_t ) xTaskGetApplicationTaskTag ( NULL );
        if ( ( TaskHandle_t ) ThisTaskHandle == ( TaskHandle_t ) threadInstance->xTaskHandle )
        {
            result = THREADAPI_ERROR;
            LogError("Attempting to join the calling thread !");
            break;
        }

        /* Wait for the joining thread to finish. Because this call waits forever,
         * it should never fail. */
        ( void ) xSemaphoreTake( ( SemaphoreHandle_t ) &threadInstance->xJoinBarrier, portMAX_DELAY );

        /// Create a critical section to clean up the joined thread. */
        vTaskSuspendAll ( );

        /// Release xJoinBarrier and delete it. */
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &threadInstance->xJoinBarrier );
        vSemaphoreDelete( ( SemaphoreHandle_t ) &threadInstance->xJoinBarrier );

        /// Release xJoinMutex and delete it. */
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) &threadInstance->xJoinMutex );
        vSemaphoreDelete( ( SemaphoreHandle_t ) &threadInstance->xJoinMutex );

        /// Delete the FreeRTOS task that ran the thread. */
        vTaskDelete ( threadInstance->xTaskHandle );

        free ( threadInstance );

        /// End the critical section. */
        xTaskResumeAll ( );

    } while ( 0 );

    return result;
}

void ThreadAPI_Exit(int res)
{
    (void)res;
    vTaskDelete ( NULL );
}


