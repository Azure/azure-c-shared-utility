// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


/*this header contains macros for ref_couting an immutable variable. The immutability is not ensured by
this header, only the REFCOUNTing.

There are no upper bound checks related to uint32_t overflow because we expect that bigger issues are in
the system when more than 4 billion references exist to the same variable. In the case when such an overflow
occurs, the object's ref count will reach zero (while still having 0xFFFFFFFF references) and likely the
controlling code will take the decision to free the object's resources. Then, any of the 0xFFFFFFFF references
will interact with deallocated memory / resources resulting in an undefined behavior.
*/

#ifndef REFCOUNT_H
#define REFCOUNT_H

#ifdef __cplusplus
#include <cstdlib>
#include <cstdint>
extern "C" 
{
#else
#include <stdlib.h>
#include <stdint.h>
#endif


#include "macro_utils.h"

#define REFCOUNT_TYPE(type) \
struct C2(C2(REFCOUNT_, type), _TAG)

#define REFCOUNT_SHORT_TYPE(type) \
C2(REFCOUNT_, type)

#define REFCOUNT_TYPE_CREATE_TYPE(type) C2(REFCOUNT_SHORT_TYPE(type), _Create)
#define REFCOUNT_TYPE_CREATE(type) C2(REFCOUNT_SHORT_TYPE(type), _Create)()

/*this introduces a new refcount'd type based on another type
and an initializer for that new type that also sets the ref count to 1. The type must not have a flexible array*/
/*the newly allocated memory shall be free'd by free()*/

/*foprward declaration to be used in header, if at all*/
#define DECLARE_REFCOUNT_TYPE(type)                                           \
REFCOUNT_TYPE(type)                                                           \
{                                                                              \
    type counted;                                                              \
    uint32_t count;                                                            \
};                                                                                 \
extern type* REFCOUNT_TYPE_CREATE_TYPE(type) (void)                           \

#define DEFINE_REFCOUNT_TYPE(type)                                             \
                                                          \
type* REFCOUNT_TYPE_CREATE_TYPE(type) (void)                                   \
{                                                                              \
    REFCOUNT_TYPE(type)* result = (REFCOUNT_TYPE(type)*)malloc(sizeof(REFCOUNT_TYPE(type)));         \
    if (result != NULL)                                                        \
    {                                                                          \
        result->count = 1;                                                     \
    }                                                                          \
    return (type*)result;                                                      \
}                                                                              \

/*the following macros increment/decrement a ref count in an atomic way, depending on the platform*/
/*The following mechanisms are considered in this order
C11 
    - will result in #include <stdatomic.h> 
    - will use atomic_fetch_add/sub; 
    - about the return value: "Atomically, the value pointed to by object immediately before the effects"
windows 
    - will result in #include "windows.h"
    - will use InterlockedIncrement/InterlockedDecrement; 
    - about the return value: https://msdn.microsoft.com/en-us/library/windows/desktop/ms683614%28v=vs.85%29.aspx "The function returns the resulting incremented value."
gcc
    - will result in no include (for gcc these are intrinsics build in)
    - will use __sync_fetch_and_add/sub
    - about the return value: "... returns the value that had previously been in memory." (https://gcc.gnu.org/onlinedocs/gcc-4.4.3/gcc/Atomic-Builtins.html#Atomic-Builtins)

It seems windows is "one off" because it returns the value "before" the decrement, as opposed to C11 standard and gcc that return the value "before". 
The macro DEC_RETURN_ZERO will be "0" on windows, and "1" on the other cases.
*/

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ == 201112) && (__STDC_NO_ATOMICS__!=1)
#include <stdatomic.h>
#define DEC_RETURN_ZERO (1)
#define INC_REF(type, var) atomic_fetch_add((&((REFCOUNT_TYPE(type)*)var)->count), 1)
#define DEC_REF(type, var) atomic_fetch_sub((&((REFCOUNT_TYPE(type)*)var)->count), 1)

#elif defined(WIN32)
#include "windows.h"
#define DEC_RETURN_ZERO (0)
#define INC_REF(type, var) InterlockedIncrement(&(((REFCOUNT_TYPE(type)*)var)->count))
#define DEC_REF(type, var) InterlockedDecrement(&(((REFCOUNT_TYPE(type)*)var)->count))

#elif defined(__GNUC__)
#define DEC_RETURN_ZERO (0)
#define INC_REF(type, var) __sync_fetch_and_add((&((REFCOUNT_TYPE(type)*)var)->count), 1)
#define DEC_REF(type, var) __sync_fetch_and_sub((&((REFCOUNT_TYPE(type)*)var)->count), 1)

#else
#error don't know how to atomically increment and decrement a uint32_t :(. Platform support needs to be extended to your platform.
#endif



#ifdef __cplusplus
}
#endif

#endif /*REFCOUNT_H*/


