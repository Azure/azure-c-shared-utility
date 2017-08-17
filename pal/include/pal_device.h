// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PAL_DEVICE_H
#define PAL_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    // Initialize low-level device settings such as pinout and IRQ
    int pal_device_init();
    void pal_device_deinit();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAL_DEVICE_H */