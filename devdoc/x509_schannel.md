x509_schannel
=============

## Overview

x509_schannel aims at building schannel objects required by either winhttp or by schannel for x509 
authentication. The data structure to be build is a [PCCERT_CONTEXT](https://msdn.microsoft.com/query/dev14.query?appId=Dev14IDEF1&l=EN-US&k=k(WINCRYPT%2FPCCERT_CONTEXT) that can be used with
`WinHttpSetOption` or with `SCHANNEL_CRED` structure of SCHANNEL.

## References

[WinHttpSetOption function](https://msdn.microsoft.com/query/dev14.query?appId=Dev14IDEF1&l=EN-US&k=k(WINHTTP%2FWinHttpSetOption) - option `WINHTTP_OPTION_CLIENT_CERT_CONTEXT` 

[SCHANNEL_CRED](https://msdn.microsoft.com/query/dev14.query?appId=Dev14IDEF1&l=EN-US&k=k(SCHANNEL%2FSCHANNEL_CRED) - fields `cCreds` and `paCred`

```c
X509_SCHANNEL_HANDLE x509_schannel_create(const char* x509certificate, const char* x509privatekey);
void x509_schannel_destroy(X509_SCHANNEL_HANDLE, x509_schannel_handle);
PCCERT_CONTEXT x509_schannel_get_certificate_context(X509_SCHANNEL_HANDLE x509_schannel_handle);
```

###   x509_schannel_create
```c
X509_SCHANNEL_HANDLE x509_schannel_create(const char* x509certificate, const char* x509privatekey);
```

x509_schannel_create creates a handle wrapping a PCCERT_CONTEXT and other information. 

**SRS_X509_SCHANNEL_02_001: [** If `x509certificate` or `x509privatekey` are NULL then x509_schannel_create shall fail and return NULL. **]**

**SRS_X509_SCHANNEL_02_002: [** `x509_schannel_create` shall convert the certificate to binary form by calling `CryptStringToBinaryA`. **]**

**SRS_X509_SCHANNEL_02_003: [** `x509_schannel_create` shall convert the private key to binary form by calling `CryptStringToBinaryA`. **]**

**SRS_X509_SCHANNEL_02_004: [** `x509_schannel_create` shall decode the private key by calling `CryptDecodeObjectEx`. **]**

The private key is accepted in any of the following PEM/DER encodings:

| PEM header | Structure | How it is decoded |
|---|---|---|
| `-----BEGIN RSA PRIVATE KEY-----` | PKCS#1 `RSAPrivateKey` | `CryptDecodeObjectEx` with `PKCS_RSA_PRIVATE_KEY` |
| `-----BEGIN EC PRIVATE KEY-----` | RFC 5915 / SEC1 `ECPrivateKey` | `CryptDecodeObjectEx` with `X509_ECC_PRIVATE_KEY` |
| `-----BEGIN PRIVATE KEY-----` | PKCS#8 `PrivateKeyInfo` | `CryptDecodeObjectEx` with `PKCS_PRIVATE_KEY_INFO`, then the wrapped key is decoded as PKCS#1 (`szOID_RSA_RSA`) or RFC 5915 (`szOID_ECC_PUBLIC_KEY`) according to the algorithm identifier |

RFC 5915 requires the `parameters [0]` (curve OID) field of `ECPrivateKey` to be omitted when the key is
carried inside a PKCS#8 `PrivateKeyInfo`. If `X509_ECC_PRIVATE_KEY` rejects that encoding, the private key
scalar is recovered from the raw `ECPrivateKey` `SEQUENCE` instead (`X509_SEQUENCE_OF_ANY` followed by
`X509_OCTET_STRING`); the public point comes from the certificate and the curve from the scalar length, so
the curve OID is not needed.

`-----BEGIN ENCRYPTED PRIVATE KEY-----` (PKCS#8 `EncryptedPrivateKeyInfo`) is not supported. It is detected
so that the failure is reported with an actionable message rather than as a generic decode error.

**SRS_X509_SCHANNEL_07_001: [** `x509_schannel_create` shall determine whether the certificate is of type RSA or ECC. **]** 

**SRS_X509_SCHANNEL_02_005: [** `x509_schannel_create` shall call `CryptAcquireContext`. **]**

**SRS_X509_SCHANNEL_02_006: [** `x509_schannel_create` shall import the private key by calling `CryptImportKey`. **]**

**SRS_X509_SCHANNEL_02_007: [** `x509_schannel_create` shall create a cerficate context by calling `CertCreateCertificateContext`. **]**

**SRS_X509_SCHANNEL_02_008: [** `x509_schannel_create` shall call set the certificate private key by calling `CertSetCertificateContextProperty`. **]**

**SRS_X509_SCHANNEL_02_009: [** If all the operations above succeed, then `x509_schannel_create` shall succeeds and return a non-NULL X509_SCHANNEL_HANDLE. **]**

**SRS_X509_SCHANNEL_02_010: [** Otherwise, `x509_schannel_create` shall fail and return a NULL X509_SCHANNEL_HANDLE. **]**


###  x509_schannel_destroy
```c
void x509_schannel_destroy(X509_SCHANNEL_HANDLE x509_schannel_handle)
```
`x509_schannel_destroy` frees the system resources used by a `X509_SCHANNEL_HANDLE`

**SRS_X509_SCHANNEL_02_011: [** If parameter `x509_schannel_handle` is NULL then `x509_schannel_destroy` shall do nothing. **]**

**SRS_X509_SCHANNEL_02_012: [** Otherwise, `x509_schannel_destroy` shall free all used resources. **]**

###   x509_schannel_get_certificate_context
```c
PCCERT_CONTEXT x509_schannel_get_certificate_context(X509_SCHANNEL_HANDLE x509_schannel_handle)
```

x509_schannel_get_certificate_context returns the constructed certificate context (PCCERT_CONTEXT)

**SRS_X509_SCHANNEL_02_013: [** If parameter `x509_schannel_handle` is NULL then x509_schannel_get_certificate_context shall return NULL. **]**

**SRS_X509_SCHANNEL_02_014: [** Otherwise, `x509_schannel_get_certificate_context` shall return a non-NULL PCCERT_CONTEXT pointer. **]** 