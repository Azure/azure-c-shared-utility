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
#include "agenttime.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define STRINGIFY(a) (#a)

#define LOG_LINE 0x01

#define LogInfo(...) (void)printf("Info: " __VA_ARGS__)

#if defined _MSC_VER
#define LogError(FORMAT, ...) { time_t t = time(NULL); (void)fprintf(stderr,"Error: Time:%.24s File:%s Func:%s Line:%d " FORMAT, ctime(&t), __FILE__, __FUNCDNAME__, __LINE__, __VA_ARGS__); }
#else
#define LogError(FORMAT, ...) { time_t t = time(NULL); (void)fprintf(stderr,"Error: Time:%.24s File:%s Func:%s Line:%d " FORMAT, ctime(&t), __FILE__, __func__, __LINE__, ##__VA_ARGS__); }
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif /* LOGGING_H */
