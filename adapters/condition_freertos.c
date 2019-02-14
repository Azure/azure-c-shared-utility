// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/condition.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/gballoc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/**
 * @brief Condition variable.
 */
typedef struct CONDITION_TAG
{
    BaseType_t IsInitialized;
    SemaphoreHandle_t CondMutex; /// Prevents concurrent accesses to WaitingThreads
    SemaphoreHandle_t CondWaitSemaphore; /// Threads block on this semaphore in pthread_cond_wait
    int WaitingThreads;/// The number of threads currently waiting on this condition variable
} CONDITION;

DEFINE_ENUM_STRINGS( COND_RESULT , COND_RESULT_VALUES );

COND_HANDLE Condition_Init ( void )
{
    CONDITION* pxCond = NULL;

    do
    {
        /// Allocate condition structure
        pxCond = ( CONDITION * ) malloc ( sizeof(CONDITION) );
        if ( pxCond == NULL )
        {
            LogError( "Failed to allocate memory" );
            break;
        }

        /// Create Mutex to protect the data "WaitingThreads"
        pxCond->CondMutex = xSemaphoreCreateMutex ( );
        if ( pxCond->CondMutex == NULL )
        {
            LogError( "Failed to create Mutex" );
            free ( pxCond );
            break;
        }

        /// Create a semaphore which blocks the thread that waiting for condition
        pxCond->CondWaitSemaphore = xSemaphoreCreateCounting ( portMAX_DELAY , 0 );
        if ( pxCond->CondWaitSemaphore == NULL)
        {
            LogError( "Failed to create Counting Semaphore" );
            free ( pxCond );
            break;
        }

        /// Update condition status
        pxCond->IsInitialized = pdTRUE;
        pxCond->WaitingThreads = 0;

    } while ( 0 );

    return ( COND_HANDLE ) pxCond;
}

COND_RESULT Condition_Post ( COND_HANDLE handle )
{
    COND_RESULT result = COND_ERROR;
    CONDITION * pxCond = ( CONDITION * ) ( handle );
    BaseType_t ret = pdTRUE;

    do
    {
        /// Sanity check
        if ( handle == NULL )
        {
            LogError( "Invalid condition handle" );
            result = COND_INVALID_ARG;
            break;
        }

        /// Verify that at least one thread is waiting for this condition
        if ( pxCond->WaitingThreads == 0 )
        {
            LogError( "No thread waiting for this condition ..." );
            result = COND_ERROR;
            break;
        }

        /// Take the mutex of the protect data "WaitingThreads"
        ret = xSemaphoreTake( ( SemaphoreHandle_t ) pxCond->CondMutex , portMAX_DELAY );
        if ( ret != pdPASS )
        {
            LogError( "Failed to take access to protect data WaitingThreads" );
            result = COND_ERROR;
            break;
        }

        /* Verify again that at least one thread is waiting for this condition
         * because taking the mutex can take a long time
         */
        if ( pxCond->WaitingThreads == 0 )
        {
            LogError( "No more thread is waiting for this condition ..." );
            result = COND_ERROR;
            break;
        }

        /// Try de unblock the thread waiting for the condition
        ret = xSemaphoreGive( ( SemaphoreHandle_t ) pxCond->CondWaitSemaphore );
        if ( ret != pdPASS )
        {
            LogError( "Failed to unblock thread" );
            result = COND_ERROR;
            break;
        }

        /// Decrease the number of waiting threads
        pxCond->WaitingThreads--;

        /// Release CondMutex
        ret = xSemaphoreGive( ( SemaphoreHandle_t ) pxCond->CondMutex );
        if ( ret != pdPASS )
        {
            LogError( "Failed to release the mutex" );
            result = COND_ERROR;
            break;
        }

    } while ( 0 );

    return result;
}

COND_RESULT Condition_Wait ( COND_HANDLE handle , LOCK_HANDLE lock , int timeout_milliseconds )
{
    COND_RESULT result = COND_OK;
    TickType_t Delay = 0;
    CONDITION * pxCond = ( CONDITION * ) ( handle );
    int ret;

    do
    {
        /// Convert timeout to tick
        Delay = pdMS_TO_TICKS( timeout_milliseconds );

        /// Take the mutex to manage the data WaitingThreads saftely
        ret = xSemaphoreTake( ( SemaphoreHandle_t ) pxCond->CondMutex , portMAX_DELAY );
        if ( ret != pdPASS )
        {
            LogError( "Failed to take the mutex" );
            break;
        }

        /// Increase the counter of threads blocking on condition variable
        pxCond->WaitingThreads++;

        /// Release the mutex
        ret = xSemaphoreGive( ( SemaphoreHandle_t ) pxCond->CondMutex );
        if ( ret != pdPASS )
        {
            LogError( "Failed to release the mutex" );
            break;
        }

        /// Unlock the condition mutex
        result = ( COND_RESULT ) Unlock ( lock );
        if ( result != 0 )
        {
            LogError( "Failed to unlock the mutex" );
            result = COND_ERROR;
            break;
        }

        /// Wait on the condition variable
        ret = xSemaphoreTake( ( SemaphoreHandle_t ) pxCond->CondWaitSemaphore , Delay );
        if ( ret != pdPASS )
        {
            /// Timeout. Relock mutex and decrement number of waiting threads
            LogError( "Failed to take semaphore..timeOut reached" );
            ( void ) Lock ( lock );
            ( void ) xSemaphoreTake( ( SemaphoreHandle_t ) pxCond->CondMutex , portMAX_DELAY );
            pxCond->WaitingThreads--;
            ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) pxCond->CondMutex );
            result = COND_TIMEOUT;
            break;
        }

        /// When successful, relock mutex
        result = ( COND_RESULT ) Lock ( lock );
        if ( result != 0 )
        {
            LogError( "Failed to relock the mutex" );
            result = COND_ERROR;
            break;
        }

    } while ( 0 );

    return result;
}

void Condition_Deinit ( COND_HANDLE handle )
{
    if ( handle != NULL )
    {
        CONDITION * pxCond = ( CONDITION * ) ( handle );

        /// Free all resources in use by the cond.
        vSemaphoreDelete( ( SemaphoreHandle_t ) pxCond->CondMutex );
        vSemaphoreDelete( ( SemaphoreHandle_t ) pxCond->CondWaitSemaphore );
        free ( pxCond );
    }
    else
    {
        LogError( "Failed to deinit condition" );
    }
}
