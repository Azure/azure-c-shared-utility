// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UMOCKVALUE_CHARPTR_H
#define UMOCKVALUE_CHARPTR_H

#ifdef __cplusplus
extern "C" {
#endif

extern char* umockvalue_stringify_charptr(const char** value);
extern int umockvalue_are_equal_charptr(const char** left, const char** right);
extern int umockvalue_copy_charptr(char** destination, const char** source);
extern void umockvalue_free_charptr(char** value);

#ifdef __cplusplus
}
#endif

#endif /* UMOCKVALUE_CHARPTR_H */
