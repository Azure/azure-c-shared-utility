// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef X509_SCHANNEL_H
#define X509_SCHANNEL_H

#include "windows.h"

#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct X509_SCHANNEL_HANDLE_DATA_TAG* X509_SCHANNEL_HANDLE;

MOCKABLE_FUNCTION(, X509_SCHANNEL_HANDLE, x509_schannel_create, const char*, x509certificate, const char*, x509privatekey);
MOCKABLE_FUNCTION(, void, x509_schannel_destroy, X509_SCHANNEL_HANDLE, x509_schannel_handle);
MOCKABLE_FUNCTION(, PCCERT_CONTEXT, x509_schannel_get_certificate_context, X509_SCHANNEL_HANDLE, x509_schannel_handle);

/**
 * @brief Verifies that pCertContextToVerify chains up to trustedCertificate.
 * @warning Does not check the server name. To authenticate a TLS server use
 *          x509_verify_certificate_in_chain_for_hostname.
 * @param trustedCertificate PEM certificate(s) the chain must end in.
 * @param pCertContextToVerify Certificate to verify.
 * @return 0 on success, non-zero otherwise.
 */
MOCKABLE_FUNCTION(, int, x509_verify_certificate_in_chain, const char*, trustedCertificate, PCCERT_CONTEXT, pCertContextToVerify);

/**
 * @brief Verifies that pCertContextToVerify chains up to trustedCertificate and is valid for hostname.
 * @param trustedCertificate PEM certificate(s) the chain must end in.
 * @param pCertContextToVerify Server certificate to verify.
 * @param hostname Server name the certificate must match. Must not be NULL or empty.
 * @return 0 on success, non-zero otherwise.
 */
MOCKABLE_FUNCTION(, int, x509_verify_certificate_in_chain_for_hostname, const char*, trustedCertificate, PCCERT_CONTEXT, pCertContextToVerify, const char*, hostname);

#ifdef __cplusplus
}
#endif

#endif
