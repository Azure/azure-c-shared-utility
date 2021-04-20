// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SHARED_UTIL_OPTIONS_H
#define SHARED_UTIL_OPTIONS_H

#include "azure_c_shared_utility/const_defines.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct HTTP_PROXY_OPTIONS_TAG
    {
        const char* host_address;
        int port;
        const char* username;
        const char* password;
    } HTTP_PROXY_OPTIONS;

    static STATIC_VAR_UNUSED const char* const OPTION_HTTP_PROXY = "proxy_data";
    static STATIC_VAR_UNUSED const char* const OPTION_HTTP_TIMEOUT = "timeout";

    static STATIC_VAR_UNUSED const char* const OPTION_TRUSTED_CERT = "TrustedCerts";

    static STATIC_VAR_UNUSED const char* const OPTION_DISABLE_CRL_CHECK = "DisableCrlCheck";
    static STATIC_VAR_UNUSED const char* const OPTION_CONTINUE_ON_CRL_DOWNLOAD_FAILURE = "ContinueOnCrlDownloadFailure";
    static STATIC_VAR_UNUSED const char* const OPTION_DISABLE_DEFAULT_VERIFY_PATHS = "DisableDefaultVerifyPath";
    static STATIC_VAR_UNUSED const char* const OPTION_SSL_CRL_MAX_SIZE_IN_KB = "SSLCRLMaxSizeInKB";

    static STATIC_VAR_UNUSED const char* const SU_OPTION_X509_CERT = "x509certificate";
    static STATIC_VAR_UNUSED const char* const SU_OPTION_X509_PRIVATE_KEY = "x509privatekey";

    static STATIC_VAR_UNUSED const char* const OPTION_X509_ECC_CERT = "x509EccCertificate";
    static STATIC_VAR_UNUSED const char* const OPTION_X509_ECC_KEY = "x509EccAliasKey";

    static STATIC_VAR_UNUSED const char* const OPTION_CURL_LOW_SPEED_LIMIT = "CURLOPT_LOW_SPEED_LIMIT";
    static STATIC_VAR_UNUSED const char* const OPTION_CURL_LOW_SPEED_TIME = "CURLOPT_LOW_SPEED_TIME";
    static STATIC_VAR_UNUSED const char* const OPTION_CURL_FRESH_CONNECT = "CURLOPT_FRESH_CONNECT";
    static STATIC_VAR_UNUSED const char* const OPTION_CURL_FORBID_REUSE = "CURLOPT_FORBID_REUSE";
    static STATIC_VAR_UNUSED const char* const OPTION_CURL_VERBOSE = "CURLOPT_VERBOSE";

    static STATIC_VAR_UNUSED const char* const OPTION_NET_INT_MAC_ADDRESS = "net_interface_mac_address";

    static STATIC_VAR_UNUSED const char* const OPTION_TLS_VERSION = "tls_version";

    typedef enum TLSIO_VERSION_TAG
    {
        OPTION_TLS_VERSION_1_0 = 10,
        OPTION_TLS_VERSION_1_1,
        OPTION_TLS_VERSION_1_2,
    } TLSIO_VERSION;

#ifdef __cplusplus
}
#endif

#endif /* SHARED_UTIL_OPTIONS_H */
