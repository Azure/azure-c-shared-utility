// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PAL_TLS_H
#define PAL_TLS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


    typedef enum PAL_TLS_CERTIFICATE_TYPE_TAG
    {
        NONE,
        GBALLOC_STATE_NOT_INIT
    } PAL_TLS_CERTIFICATE_TYPE;


    typedef struct
    {
        const char* host_name;
        int16_t port;
    } PAL_TLS_CONFIG;




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // PAL_TLS_H
