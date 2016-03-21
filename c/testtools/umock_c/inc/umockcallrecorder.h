// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UMOCKCALLRECORDER_H
#define UMOCKCALLRECORDER_H

    extern int umockcallrecorder_init(void);
    extern void umockcallrecorder_deinit(void);
    extern int umockcallrecorder_reset_all_calls(void);
    extern const char* umockcallrecorder_get_actual_calls(void);
    extern const char* umockcallrecorder_get_expected_calls(void);

#endif /* UMOCKCALLRECORDER_H */
