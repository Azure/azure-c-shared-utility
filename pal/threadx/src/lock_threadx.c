// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Copyright (c) Express Logic.  All rights reserved.
// Please contact support@expresslogic.com for any questions or use the support portal at www.rtos.com

/* This file is used for porting lock between threadx and azure-iot-sdk-c.  */

#include "tx_api.h"
#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/xlogging.h"

/* Define the byte pool that Azure SDK resources will be allocated from.  */

extern TX_BYTE_POOL                threadx_azure_sdk_memory;

/* Define an internal Azure porting layer mutex.  */

extern TX_MUTEX                    threadx_azure_sdk_protection;

/* Process the create lock request.  */

LOCK_HANDLE  Lock_Init(void)
{

TX_MUTEX    *mutex_pointer;
UINT        status;


    /* Obtain the internal SDK mutex protection.  */
    status =  tx_mutex_get(&threadx_azure_sdk_protection, TX_WAIT_FOREVER);

    /* Check for an error.  */
    if (status != TX_SUCCESS)
    {
        
        /* Return a NULL handle.  */
        return(NULL);
    }

    /* Allocate memory for the SDK mutex.  */
    status =  tx_byte_allocate(&threadx_azure_sdk_memory, (void **) &mutex_pointer, sizeof(TX_MUTEX), TX_NO_WAIT);
        
    /* Check for an error.  */
    if (status != TX_SUCCESS)
    {
        
        /* Release internal mutex protection.  */
        tx_mutex_put(&threadx_azure_sdk_protection);
        
        /* Return a NULL handle.  */
        return(NULL);
    }

    /* Now create the underlying ThreadX mutex.  */
    status =  tx_mutex_create(mutex_pointer, "THREADX Azure SDK Mutex", TX_NO_INHERIT);
    
    /* Check for an error.  */
    if (status != TX_SUCCESS)
    {
        
        /* Release the memory for the mutex.  */
        tx_byte_release(mutex_pointer);
        
        /* Release internal mutex protection.  */
        tx_mutex_put(&threadx_azure_sdk_protection);
        
        /* Return a NULL handle.  */
        return(NULL);
    }
    
    /* Release the protection mutex.  */
    tx_mutex_put(&threadx_azure_sdk_protection);

    /* Return the handle.  */
    return((LOCK_HANDLE)mutex_pointer);
}


/* Process the lock request.  */

LOCK_RESULT  Lock(LOCK_HANDLE handle)
{

UINT    status;


    /* Check for a NULL handle.  */
    if (handle == NULL)
    {

        /* Return a LOCK_ERROR status.  */
        return(LOCK_ERROR);    
    }

    /* Now use ThreadX to obtain the lock (mutex).  */
    status = tx_mutex_get((TX_MUTEX *) handle, TX_WAIT_FOREVER);
    
    /* Check for an error return code.  */
    if (status != TX_SUCCESS)
    {
    
        /* Return a LOCK_ERROR status.  */
        return(LOCK_ERROR);            
    }
    
    /* Otherwise, we have the lock. Return success.  */
    return(LOCK_OK);
}


/* Process the unlock request.  */

LOCK_RESULT  Unlock(LOCK_HANDLE handle)
{

UINT    status;


    /* Check for a NULL handle.  */
    if (handle == NULL)
    {

        /* Return a LOCK_ERROR status.  */
        return(LOCK_ERROR);    
    }

    /* Now use ThreadX to release the lock (mutex).  */
    status = tx_mutex_put((TX_MUTEX *) handle);
    
    /* Check for an error return code.  */
    if (status != TX_SUCCESS)
    {
    
        /* Return a LOCK_ERROR status.  */
        return(LOCK_ERROR);            
    }
    
    /* Otherwise, we have released the lock. Return success.  */
    return(LOCK_OK);
}


/* Process the lock deletion.  */

LOCK_RESULT  Lock_Deinit(LOCK_HANDLE handle)
{

    /* Check for a NULL handle.  */
    if (handle == NULL)
    {

        /* Return a LOCK_ERROR status.  */
        return(LOCK_ERROR);    
    }

    /* Obtain the internal SDK mutex protection.  */
    tx_mutex_get(&threadx_azure_sdk_protection, TX_WAIT_FOREVER);

    /* Delete the mutex created by SDK.  */
    tx_mutex_delete((TX_MUTEX *) handle);
    
    /* Release the memory for the SDK mutex.  */
    tx_byte_release((void *) handle);

    /* Release the protection mutex.  */
    tx_mutex_put(&threadx_azure_sdk_protection);

    /* Return success.  */
    return(LOCK_OK);
}




