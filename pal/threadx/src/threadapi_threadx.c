// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Copyright (c) Express Logic.  All rights reserved.
// Please contact support@expresslogic.com for any questions or use the support portal at www.rtos.com

/* This file is used for porting threadapi between threadx and azure-iot-sdk-c.  */

#include "tx_api.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"

MU_DEFINE_ENUM_STRINGS(THREADAPI_RESULT, THREADAPI_RESULT_VALUES);

/* Define the resources needed by the THREADX IoT Platform for the Azure SDK porting layer.  */

/* Define the amount of memory for the memory pool ThreadX will utilize for the Azure porting layer. 
   The user can override this via -D command line option or via project settings. */
#ifndef THREADX_AZURE_SDK_MEMORY_POOL_SIZE
#define THREADX_AZURE_SDK_MEMORY_POOL_SIZE      8192
#endif /* THREADX_AZURE_SDK_MEMORY_POOL_SIZE */

/* Define the default thread attributes, priority, stack size, etc.  The user can override this 
   via -D command line option or via project settings.  */
#ifndef THREADX_AZURE_SDK_THREAD_PRIORITY
#define THREADX_AZURE_SDK_THREAD_PRIORITY       16
#endif /* THREADX_AZURE_SDK_MEMORY_POOL_SIZE */

#ifndef THREADX_AZURE_SDK_THREAD_STACK_SIZE
#define THREADX_AZURE_SDK_THREAD_STACK_SIZE     2048
#endif /* THREADX_AZURE_SDK_MEMORY_POOL_SIZE */

/* Define the Azure thread structure for running on ThreadX (THREADX IoT Platform).  */

typedef struct THREADX_AZURE_SDK_THREAD_STRUCT
{

    TX_THREAD               threadx_azure_sdk_thread;               /* Azure thread in ThreadX    */
    TX_EVENT_FLAGS_GROUP    threadx_azure_sdk_thread_join;          /* Azure thread join resource */
    struct THREADX_AZURE_SDK_THREAD_STRUCT
                            *threadx_azure_sdk_thread_cleanup_next; /* Next pointer for cleanup   */

} THREADX_AZURE_SDK_THREAD;


/* Define the byte pool and memory area that Azure SDK resources will be allocated from.  */

TX_BYTE_POOL                threadx_azure_sdk_memory;
UCHAR                       threadx_azure_sdk_memory_area[THREADX_AZURE_SDK_MEMORY_POOL_SIZE];


/* Define the head pointer to the linked list of thread resources to cleanup. When this pointer is NULL,
   there is nothing to cleanup.  Otherwise, the linked list is traversed to cleanup the thread
   resources.  */

THREADX_AZURE_SDK_THREAD    *threadx_azure_sdk_thread_cleanup_list;


/* Define an internal Azure porting layer mutex.  */

TX_MUTEX                    threadx_azure_sdk_protection;


/* Define the THREADX IoT Platform initialization function.  This is typically called from
   platform_init().  */

void  threadx_azure_sdk_initialize(void)
{

    /* Create the memory pool used to allocate THREADX IoT Resources from.  */
    tx_byte_pool_create(&threadx_azure_sdk_memory, "THREADX Azure SDK Memory Pool", threadx_azure_sdk_memory_area, sizeof(threadx_azure_sdk_memory_area));

    /* Create a mutex used to protect internal resources of the SDK porting layer.  */
    tx_mutex_create(&threadx_azure_sdk_protection, "THREADX Azure SDK Internal Protection", TX_NO_INHERIT);
}


/* Define the THREADX IoT Platform thread resource cleanup function.  */

void  threadx_azure_sdk_thread_resource_cleanup(void)
{

THREADX_AZURE_SDK_THREAD  *current_thread;
THREADX_AZURE_SDK_THREAD  *next_thread;


    /* Setup the current thread pointer.  */
    current_thread =  threadx_azure_sdk_thread_cleanup_list;
    
    /* Walk the resource cleanup list - assuming SDK protection is in force already.  */
    while (current_thread != TX_NULL)
    {
    
        /* Pickup next thread pointer.  */
        next_thread =  current_thread -> threadx_azure_sdk_thread_cleanup_next;
 
        /* Terminate the thread.  */
        tx_thread_terminate(&(current_thread-> threadx_azure_sdk_thread));

        /* Delete the thread.  */
        tx_thread_delete(&(current_thread-> threadx_azure_sdk_thread));
        
        /* Release the thread's stack.  */
        tx_byte_release(current_thread -> threadx_azure_sdk_thread.tx_thread_stack_start);
        
        /* Release the thread's control block.  */
        tx_byte_release((void *) current_thread);
        
        /* Move the next pointer to the current pointer.  */
        current_thread =  next_thread; 
    }   
    
    /* At this point, the list is clear.  Set the list head to NULL.  */
    threadx_azure_sdk_thread_cleanup_list =  TX_NULL;
}


/* Define the THREADX IoT Platform de-initialize function.  This is typically called from 
   platform_deinit().  */

void  threadx_azure_sdk_deinitialize(void)
{

    /* Release all the deferred thread resources.  */
    threadx_azure_sdk_thread_resource_cleanup();
    
    /* Delete the internal SDK protection mutex.  */
    tx_mutex_delete(&threadx_azure_sdk_protection);
    
    /* Delete the SDK memory pool.  */
    tx_byte_pool_delete(&threadx_azure_sdk_memory);
}


/* Define the default thread entry function for ThreadX create function. The actual SDK thread entry 
   is passed to this function via the id parameter.  */

VOID    threadx_azure_sdk_thread_entry(ULONG  id)
{

TX_THREAD       *current_thread;


    /* Note that the ID field represents the current ThreadX thread.  */
    current_thread =  (TX_THREAD *) id;
    
    /* Call the SDK thread entry function.  */
    (current_thread -> tx_thread_azure_sdk_thread_entry)(current_thread -> tx_thread_azure_sdk_arg);     
}


/* Define the THREADX IoT Platform ThreadAPI_Create mapping to ThreadX.  */

THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{

THREADX_AZURE_SDK_THREAD      *new_thread;
VOID                        *thread_stack;
UINT                        status;
THREADAPI_RESULT            result;


    /* Check for bat threadHandle.  */
    if ((threadHandle == NULL) ||
        (func == NULL))
    {
        result = THREADAPI_INVALID_ARG;
        LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
        return(result);
    }
    else
    {

        /* Initialize thread handle to NULL.  */
        *threadHandle =  NULL;

        /* Obtain the internal SDK mutex protection.  */
        status =  tx_mutex_get(&threadx_azure_sdk_protection, TX_WAIT_FOREVER);
        
        /* Check for an error.  */
        if (status != TX_SUCCESS)
        {
        
            /* Return an API error.  */
            result =  THREADAPI_ERROR;
            LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
            return(result);
        }

        /* Determine if there is any cleanup that needs to be done.  */
        if (threadx_azure_sdk_thread_cleanup_list != TX_NULL)
        {
        
            /* Call the deferred thread cleanup processing.  */
            threadx_azure_sdk_thread_resource_cleanup();
        }

        /* Allocate the memory for the thread control block.  */
        status =  tx_byte_allocate(&threadx_azure_sdk_memory, (void **) &new_thread, sizeof(THREADX_AZURE_SDK_THREAD), TX_NO_WAIT);
        
        /* Check for an error.  */
        if (status != TX_SUCCESS)
        {
        
            /* Release internal mutex protection.  */
            tx_mutex_put(&threadx_azure_sdk_protection);
        
            /* Return a memory error.  */
            result =  THREADAPI_NO_MEMORY;
            LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
            return(result);
        }

        /* Allocate the memory for the thread's stack.  */
        status =  tx_byte_allocate(&threadx_azure_sdk_memory, (void **) &thread_stack, THREADX_AZURE_SDK_THREAD_STACK_SIZE, TX_NO_WAIT);
        
        /* Check for an error.  */
        if (status != TX_SUCCESS)
        {
        
            /* Release the memory allocated for the control block.  */
            tx_byte_release((void *) new_thread);
        
            /* Release internal mutex protection.  */
            tx_mutex_put(&threadx_azure_sdk_protection);
        
            /* Return a memory error.  */
            result =  THREADAPI_NO_MEMORY;
            LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
            return(result);
        }

        /* Now create the thread in ThreadX.  */
        status =  tx_thread_create(&(new_thread -> threadx_azure_sdk_thread), "THREADX Azure SDK Thread",
                                   threadx_azure_sdk_thread_entry, (ULONG) &(new_thread -> threadx_azure_sdk_thread),
                                   thread_stack, THREADX_AZURE_SDK_THREAD_STACK_SIZE, THREADX_AZURE_SDK_THREAD_PRIORITY,
                                   THREADX_AZURE_SDK_THREAD_PRIORITY, TX_NO_TIME_SLICE, TX_DONT_START);

        /* Check for an error.  */
        if (status != TX_SUCCESS)
        {

            /* Release the memory allocated for the control block.  */
            tx_byte_release((void *) new_thread);
        
            /* Release the memory allocated for the stack.  */
            tx_byte_release((void *) new_thread);

            /* Release internal mutex protection.  */
            tx_mutex_put(&threadx_azure_sdk_protection);

            /* Return an API error.  */
            result =  THREADAPI_ERROR;
            LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
            return(result);
        }

        /* At this point populate the thread entry function and parameter in the ThreadX control block.  */
        new_thread -> threadx_azure_sdk_thread.tx_thread_azure_sdk_thread_entry =  func;
        new_thread -> threadx_azure_sdk_thread.tx_thread_azure_sdk_arg =  arg;

        /* Create the event flag group that will be used by other threads that try to join this thread.  */
        tx_event_flags_create(&(new_thread -> threadx_azure_sdk_thread_join), "THREADX Azure SDK Thread Join Event");

        /* Return the threadHandle.  */
        *threadHandle =  (THREAD_HANDLE *) new_thread;

        /* Resume the new thread.  */
        tx_thread_resume(&(new_thread -> threadx_azure_sdk_thread));
        
        /* Release the SDK protection.  */
        tx_mutex_put(&threadx_azure_sdk_protection);

        /* Return success.  */
        return(THREADAPI_OK);
    }
}


/* Join the specified thread, which actually means wait until the processing is complete of the specified 
   thread before the calling thread continues.  */

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int *res)
{

THREADAPI_RESULT        result;
TX_THREAD               *calling_thread;
THREADX_AZURE_SDK_THREAD  *sdk_thread;
UINT                    status;
ULONG                   actual_flags;


    /* Initialize the status to OK.  */
    result = THREADAPI_OK;

    /* Obtain the internal SDK mutex protection.  */
    status =  tx_mutex_get(&threadx_azure_sdk_protection, TX_WAIT_FOREVER);

    /* Check for an error.  */
    if (status != TX_SUCCESS)
    {

        /* Return an API error.  */
        result =  THREADAPI_ERROR;
        LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
        return(result);
    }

    /* Pickup the calling ThreadX thread.  */
    calling_thread =  tx_thread_identify();
    
    /* Also setup a pointer to the SDK thread structure.  */
    sdk_thread =  (THREADX_AZURE_SDK_THREAD *) threadHandle;

    /* Check for NULL handle.  */
    if (threadHandle == NULL)
    {

        /* Release the SDK protection.  */
        tx_mutex_put(&threadx_azure_sdk_protection);

        /* Thread Handle is NULL return an error.  */
        result = THREADAPI_INVALID_ARG;
        LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
        return(result);
    }

    /* Check for thread attempting to join itself.  */
    else if (threadHandle == ((THREAD_HANDLE *) calling_thread))
    {

        /* Release the SDK protection.  */
        tx_mutex_put(&threadx_azure_sdk_protection);

        /* Thread attempting to join itself - return an error.  */
        result = THREADAPI_INVALID_ARG;
        LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
        return(result);
    }
    else
    {

        /* Release the SDK protection.  */
        tx_mutex_put(&threadx_azure_sdk_protection);

        /* Attempt to wait on the event flag.  */
        status =   tx_event_flags_get(&(sdk_thread -> threadx_azure_sdk_thread_join), 0x1, TX_OR_CLEAR, &actual_flags, TX_WAIT_FOREVER);

        /* Determine if there was an error.  */
        if (status != TX_SUCCESS)
        {

            result = THREADAPI_INVALID_ARG;
            LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
            return(result);
        }

        /* Pickup the exit code from the thread.  */
        *res =  sdk_thread -> threadx_azure_sdk_thread.tx_thread_azure_sdk_exit_code;

        /* Return success.  */
        return(THREADAPI_OK);
    }
}

/* Process the thread exit function. This function will setup the deferral list for cleaning up the thread
   resources and handle any join requests before terminating.  */

void ThreadAPI_Exit(int res)
{

THREADX_AZURE_SDK_THREAD  *sdk_thread;


    /* Pickup the calling ThreadX thread.  */
    sdk_thread =  (THREADX_AZURE_SDK_THREAD *)  tx_thread_identify();

    /* Save the exit code in the SDK thread's storage.  */
    sdk_thread -> threadx_azure_sdk_thread.tx_thread_azure_sdk_exit_code =  res;

    /* Obtain the internal SDK mutex protection.  */
    tx_mutex_get(&threadx_azure_sdk_protection, TX_WAIT_FOREVER);

    /* Release all the deferred thread resources.  */
    threadx_azure_sdk_thread_resource_cleanup();

    /* Now set the event flag to wakeup all threads that have attempted to join this
       thread.  */
    tx_event_flags_set(&(sdk_thread -> threadx_azure_sdk_thread_join), 0x1, TX_OR);

    /* Relinquish to allow all other thread to complete their join processing. But they will be
       blocked if they attempt to get into the SDK porting layer again since we still have 
       protection.  */
    tx_thread_relinquish();
    
    /* Now delete the event flags group. This will prevent other threads from joining this 
       thread since it is now being exited.  */
    tx_event_flags_delete(&(sdk_thread -> threadx_azure_sdk_thread_join));

    /* Link this thread to the deferred cleanup list.  */
    sdk_thread -> threadx_azure_sdk_thread_cleanup_next =  threadx_azure_sdk_thread_cleanup_list;
    threadx_azure_sdk_thread_cleanup_list =  sdk_thread;

    /* Release the mutex protection.  */
    tx_mutex_put(&threadx_azure_sdk_protection);    
    
    /* Terminate the underlying ThreadX thread.  */
    tx_thread_terminate(&(sdk_thread -> threadx_azure_sdk_thread));
}


/* Process the thread sleep functionality of the SDK.  */

void ThreadAPI_Sleep(unsigned int milliseconds)
{                                     

UINT    ticks;

    /* Change milliseconds to ticks.  */
    ticks = (milliseconds * TX_TIMER_TICKS_PER_SECOND) / 1000;

    /* Check if ticks is zero.  */
    if (ticks == 0)
        ticks = 1;

    tx_thread_sleep(ticks);
}

