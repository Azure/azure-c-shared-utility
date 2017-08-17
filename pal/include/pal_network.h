// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PAL_NETWORK_H
#define PAL_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    // Initialize network connection and optionally connect to a WiFi access point
    int pal_network_init(const char* ap_ssid, const char* ap_password);
    void pal_network_deinit();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAL_NETWORK_H */