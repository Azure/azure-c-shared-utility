// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "shim_openssl.h"

#include <stdio.h>
#include <dlfcn.h>
#include "azure_c_shared_utility/xlogging.h"

// N.B. at this stage, the define's are already in places, and references to
//      shimmed function X as a symbol are already being replaced by X_ptr.

static void *libssl;

// Define all function pointers.
#define REQUIRED_FUNCTION(fn) __typeof(fn) fn##_ptr;
FOR_ALL_OPENSSL_FUNCTIONS
#undef REQUIRED_FUNCTION

#define COUNT_OF(a) (sizeof(a) / sizeof((a)[0]))

int load_libssl()
{
#if USE_OPENSSL_1_1_0_OR_UP
    static char *soNames[] = {
        "libssl.so.1.1"
    };
#else
    static char *soNames[] = {
        "libssl.so.1.0.2", // Debian
        "libssl.so.1.0.0", // Ubuntu
        "libssl.so.10" // RedHat
    };
#endif

    if (libssl)
    {
        // Already initialized.
        return 0;
    }

    int soNameIndex;
    for (soNameIndex = 0; soNameIndex < COUNT_OF(soNames); soNameIndex++)
    {
        if (libssl = dlopen(soNames[soNameIndex], RTLD_LAZY)) {
            break;
        }
    }

    if (!libssl) {
        LogError("libssl could not be loaded\n");
        return 1;
    }

#define REQUIRED_FUNCTION(fn) \
    if (!(fn##_ptr = (__typeof(fn))(dlsym(libssl, #fn)))) { \
        failures++; \
        LogError("Cannot get required symbol " #fn " from libssl\n"); \
    }

    int failures = 0;
    int runtimeVersion = 0;

    // Discover the version and check it.

#if USE_OPENSSL_1_1_0_OR_UP
    const int minVersion = 0x10100000L;
    const int maxVersion = 0x20000000L;
    REQUIRED_FUNCTION(OpenSSL_version_num);
    if (OpenSSL_version_num_ptr) {
        runtimeVersion = OpenSSL_version_num();
    }
#else
    const int minVersion = 0x10002000L;
    const int maxVersion = 0x10003000L;
    REQUIRED_FUNCTION(SSLeay);
    if (SSLeay_ptr) {
        runtimeVersion = SSLeay();
    }
#endif

    if (failures) {
        LogError("Unable to check dynamic OpenSSL version, symbol missing.\n");
    }

    if (runtimeVersion < minVersion || runtimeVersion >= maxVersion)
    {
        LogError("Unsupported libssl version %x\n", runtimeVersion);
        libssl = NULL;
        return 1;
    }

    LogInfo("Loaded %s (version %x) at 0x%lx\n", soNames[soNameIndex], runtimeVersion, libssl);

    // Now bind all remaining functions.
    FOR_ALL_OPENSSL_FUNCTIONS
#undef REQUIRED_FUNCTION

    if (failures) {
        LogError("Missing required symbols from libssl.\n");
        libssl = NULL;
        return 1;
    }

    return 0;
}
