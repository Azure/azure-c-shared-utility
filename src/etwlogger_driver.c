// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "windows.h"


#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/etwlogger.h"

/*returns a string as if printed by vprintf*/
static char* vprintf_alloc(const char* format, va_list va)
{
    char* result;
    int neededSize = vsnprintf(NULL, 0, format, va);
    if (neededSize < 0)
    {
        result = NULL;
    }
    else
    {
        result = (char*)malloc(neededSize + 1);
        if (result == NULL)
        {
            /*return as is*/
        }
        else
        {
            if (vsnprintf(result, neededSize + 1, format, va) != neededSize)
            {
                free(result);
                result = NULL;
            }
        }
    }
    return result;
}

/*returns a string as if printed by printf*/
static char* printf_alloc(const char* format, ...)
{
    char* result;
    va_list va;
    va_start(va, format);
    result = vprintf_alloc(format, va);
    va_end(va);
    return result;
}

/*returns NULL if it fails*/
static char* lastErrorToString(DWORD lastError)
{
    char* result;
    if (lastError == 0)
    {
        result = printf_alloc(""); /*no error should appear*/
        if (result == NULL)
        {
            (void)printf("failure in printf_alloc");
        }
        else
        {
            /*return as is*/
        }
    }
    else
    {
        char temp[MESSAGE_BUFFER_SIZE];
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), temp, MESSAGE_BUFFER_SIZE, NULL) == 0)
        {
            result = printf_alloc("GetLastError()=0X%x", lastError);
            if (result == NULL)
            {
                (void)printf("failure in printf_alloc\n");
                /*return as is*/
            }
            else
            {
                /*return as is*/
            }
        }
        else
        {
            /*eliminate the \r or \n from the string*/
            /*one replace of each is enough*/
            char* whereAreThey;
            if ((whereAreThey = strchr(temp, '\r')) != NULL)
            {
                *whereAreThey = '\0';
            }
            if ((whereAreThey = strchr(temp, '\n')) != NULL)
            {
                *whereAreThey = '\0';
            }

            result = printf_alloc("GetLastError()==0X%x (%s)", lastError, temp);

            if (result == NULL)
            {
                (void)printf("failure in printf_alloc\n");
                /*return as is*/
            }
            else
            {
                /*return as is*/
            }
        }
    }
    return result;
}
/*the function will also attempt to produce some human readable strings for GetLastError*/
void etwlogger_log_with_GetLastError(const char* file, const char* func, int line, const char* format, ...)
{
    DWORD lastError;
    char* lastErrorAsString;


    va_list args;
    va_start(args, format);

    /*this is what this case will do:
    1. snip the last error
    2. create a string with what that last error means
    3. printf the system message (__FILE__, __LINE__ etc) + the last error + whatever the user wanted
    */
    /*1. snip the last error*/
    lastError = GetLastError();

    /*2. create a string with what that last error means*/
    SYSTEMTIME t;
    GetSystemTime(&t);

    lastErrorAsString = lastErrorToString(lastError);
    if (lastErrorAsString == NULL)
    {
        char* userMessage = vprintf_alloc(format, args);
        if (userMessage == NULL)
        {
            if (EventWriteLogErrorEvent("unable to print user error or last error", file, &t, func, line) != ERROR_SUCCESS)
            {
                (void)printf("failure in EventWriteLogErrorEvent");
            }
        }
        else
        {
            if (EventWriteLogErrorEvent(userMessage, file, &t, func, line) != ERROR_SUCCESS)
            {
                (void)printf("failure in EventWriteLogErrorEvent");
            }
            free(userMessage);
        }
    }
    else
    {
        char* userMessage = vprintf_alloc(format, args);
        if (userMessage == NULL)
        {
            if (EventWriteLogErrorEvent(lastErrorAsString, file, &t, func, line) != ERROR_SUCCESS)
            {
                (void)printf("failure in EventWriteLogErrorEvent");
            }
        }
        else
        {
            if (EventWriteLogLastError(userMessage, file, &t, func, line, lastErrorAsString) != ERROR_SUCCESS)
            {
                (void)printf("failure in EventWriteLogErrorEvent");
            }
            free(userMessage);
        }
        free(lastErrorAsString);
    }

    
    va_end(args);
}


static int isETWLoggerInit = 0;

void etwlogger_log(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...)
{
    (void)options;
    /*lazily init the logger*/
    if (isETWLoggerInit == 0)
    {
        if (EventRegisterMicrosoft_ServiceBus() != ERROR_SUCCESS)
        {
            /*go back to printf, maybe somebody notices*/
            printf("failure in EventRegisterMicrosoft_ServiceBus");
        }
        else
        {
            /*should check if the provider is registered with the system... maybe at a later time*/
            isETWLoggerInit = 1; /*and stays 1 until the process exits*/ /*sorry, no graceful exit with EventUnregisterMicrosoft_ServiceBus*/
            LogInfo("EventRegisterMicrosoft_ServiceBus success"); /*selflogging that the log service has started*/
        }
    }
    
    
    va_list args;
    va_start(args, format);
    char* text = vprintf_alloc(format, args);
    if (text == NULL)
    {
        switch (log_category)
        {
            case AZ_LOG_INFO:
                if (EventWriteLogInfoEvent("INTERNAL LOGGING ERROR: failed in vprintf_alloc") != ERROR_SUCCESS)
                {
                    /*fallback on printf...*/
                    (void)printf("failed in EventWriteLogInfoEvent");
                }
                break;
            case AZ_LOG_ERROR:
            {
                SYSTEMTIME t;
                GetSystemTime(&t);
                if (EventWriteLogErrorEvent("INTERNAL LOGGING ERROR: failed in vprintf_alloc", file, &t, func, line) != ERROR_SUCCESS)
                {
                    /*fallback on printf...*/
                    (void)printf("failed in EventWriteLogErrorEvent");
                }
                break;
            }
            default:
                break;
        }
    }
    else
    { 
        switch (log_category)
        {
        case AZ_LOG_INFO:
            if (EventWriteLogInfoEvent(text) != ERROR_SUCCESS)
            {
                /*fallback on printf...*/
                (void)printf("failed in EventWriteLogInfoEvent");
            }
            break;
        case AZ_LOG_ERROR:
        {
            SYSTEMTIME t;
            GetSystemTime(&t);
            if (EventWriteLogErrorEvent(text, file, &t, func, line) != ERROR_SUCCESS)
            {
                /*fallback on printf...*/
                (void)printf("failed in EventWriteLogErrorEvent");
            }
            break;
        }
        default:
            break;
        }
        free(text);
    }
    va_end(args);
}

