#include <stdlib.h>
#include <stdint.h>
#ifdef FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/portable.h"
#include "freertos/task.h"
#endif

void* malloc_wrapper ( size_t size )
{
#ifdef FREERTOS
    return pvPortMalloc ( size );
#else
    return malloc ( size );
#endif
}

void* calloc_wrapper ( size_t nmemb , size_t size )
{
    void *pvReturn;

#ifdef FREERTOS
    vTaskSuspendAll ( );
    {
        pvReturn = calloc ( nmemb , size );
    }
    xTaskResumeAll ( );
#else
    pvReturn = calloc ( nmemb , size );
#endif
    return pvReturn;
}

void* realloc_wrapper ( void* ptr , size_t size )
{
    void *pvReturn;

#ifdef FREERTOS
    vTaskSuspendAll ( );
    {
        pvReturn = realloc ( ptr , size );
    }
    xTaskResumeAll ( );
#else
    pvReturn = realloc ( ptr , size );
#endif
    return pvReturn;
}

void free_wrapper ( void* ptr )
{
#ifdef FREERTOS
    vPortFree ( ptr );
#else
    free ( ptr );
#endif
    ptr = NULL;
}
