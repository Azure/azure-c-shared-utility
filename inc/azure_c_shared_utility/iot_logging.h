// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGING_H
#define LOGGING_H

/*no logging is useful when time and fprintf are mocked*/
#ifdef NO_LOGGING
#define LogInfo(...)
#define LogError(FORMAT, ...)
#else

#include <stdio.h>
#include "azure_c_shared_utility/agenttime.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define STRINGIFY(a) (#a)

#define LOG_LINE 0x01

//Adding a do while(0) to force the user to add ; after LogInfo and LogError.
#if defined _MSC_VER
#define LogInfo(FORMAT, ...) do{(void)fprintf(stdout,"Info: " FORMAT "\r\n", __VA_ARGS__); }while(0)
#else
#define LogInfo(FORMAT, ...) do{(void)fprintf(stdout,"Info: " FORMAT "\r\n", ##__VA_ARGS__); }while(0)
#endif

#if defined _MSC_VER
#define LogError(FORMAT, ...) do{ time_t t = time(NULL); (void)fprintf(stderr,"Error: Time:%.24s File:%s Func:%s Line:%d " FORMAT "\r\n", ctime(&t), __FILE__, __FUNCDNAME__, __LINE__, __VA_ARGS__); }while(0)
#else
#define LogError(FORMAT, ...) do{ time_t t = time(NULL); (void)fprintf(stderr,"Error: Time:%.24s File:%s Func:%s Line:%d " FORMAT "\r\n", ctime(&t), __FILE__, __func__, __LINE__, ##__VA_ARGS__); }while(0)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif /* LOGGING_H */
