// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef XLOGGING_H
#define XLOGGING_H

#ifdef __cplusplus
#include <cstdio>
extern "C" {
#else
#include <stdio.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/agenttime.h"

/*no logging is useful when time and fprintf are mocked*/
#ifdef NO_LOGGING
#define LOG(...)
#define LogInfo(...)
#define LogError(...)
#define xlogging_get_log_function() NULL
#else

typedef enum LOG_CATEGORY_TAG
{
    LOG_ERROR,
    LOG_INFO,
    LOG_TRACE
} LOG_CATEGORY;

typedef void(*LOGGER_LOG)(LOG_CATEGORY log_category, unsigned int options, const char* format, ...);

#define LOG_LINE 0x01

#if defined _MSC_VER
#define LOG(log_category, log_options, format, ...) { LOGGER_LOG l = xlogging_get_log_function(); if (l != NULL) l(log_category, log_options, format, __VA_ARGS__); }
#else
#define LOG(log_category, log_options, format, ...) { LOGGER_LOG l = xlogging_get_log_function(); if (l != NULL) l(log_category, log_options, format, ##__VA_ARGS__); }
#endif

#if defined _MSC_VER
#define LogInfo(FORMAT, ...) do{LOG(LOG_INFO, LOG_LINE, "Info: " FORMAT, __VA_ARGS__); }while(0)
#else
#define LogInfo(FORMAT, ...) do{LOG(LOG_INFO, LOG_LINE, "Info: " FORMAT, ##__VA_ARGS__); }while(0)
#endif

#if defined _MSC_VER
#define LogError(FORMAT, ...) do{ time_t t = time(NULL); LOG(LOG_ERROR, LOG_LINE, "Error: Time:%.24s File:%s Func:%s Line:%d " FORMAT, ctime(&t), __FILE__, __FUNCDNAME__, __LINE__, __VA_ARGS__); }while(0)
#else
#define LogError(FORMAT, ...) do{ time_t t = time(NULL); LOG(LOG_ERROR, LOG_LINE, "Error: Time:%.24s File:%s Func:%s Line:%d " FORMAT, ctime(&t), __FILE__, __func__, __LINE__, ##__VA_ARGS__); }while(0)
#endif

extern void xlogging_set_log_function(LOGGER_LOG log_function);
extern LOGGER_LOG xlogging_get_log_function(void);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XLOGGING_H */
