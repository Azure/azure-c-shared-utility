x509_openssl
=============

## Overview

x509_openssl provides several utility functions. These are:
- a utility function that imports into a SSL context a pair of x509 certificate/private key  
- a utility function that imports from a null terminated string all the certificates in a SSL_CTX*.

## References

[OpenSSL](https://www.openssl.org)

```c
int x509_openssl_add_credentials(SSL_CTX* ssl_ctx, const char* x509certificate, const char* x509privatekey);
int x509_openssl_add_certificates(SSL_CTX, ssl_ctx, const char* certificates);
```

###   x509_openssl_add_credentials
```c
int x509_openssl_add_credentials(SSL_CTX* ssl_ctx, const char* x509certificate, const char* x509privatekey);
```

x509_openssl_add_credentials loads a x509 certificate and a x509 private key into a SSL context. 

**SRS_X509_OPENSSL_02_001: [** If any argument is NULL then x509_openssl_add_credentials shall fail and return a non-zero value. **]**

**SRS_X509_OPENSSL_02_002: [** x509_openssl_add_credentials shall use BIO_new_mem_buf to create a memory BIO from the x509 certificate. **]** 

**SRS_X509_OPENSSL_02_003: [** x509_openssl_add_credentials shall use PEM_read_bio_X509 to read the x509 certificate. **]**

**SRS_X509_OPENSSL_02_004: [** x509_openssl_add_credentials shall use BIO_new_mem_buf to create a memory BIO from the x509 privatekey. **]**

**SRS_X509_OPENSSL_02_005: [** x509_openssl_add_credentials shall use PEM_read_bio_RSAPrivateKey to read the x509 private key. **]**

**SRS_X509_OPENSSL_02_006: [** x509_openssl_add_credentials shall use SSL_CTX_use_certificate to load the certicate into the SSL context. **]**

**SRS_X509_OPENSSL_02_007: [** x509_openssl_add_credentials shall use SSL_CTX_use_RSAPrivateKey to load the private key into the SSL context. **]**

**SRS_X509_OPENSSL_02_008: [** If no error occurs, then x509_openssl_add_credentials shall succeed and return 0. **]**

**SRS_X509_OPENSSL_02_009: [** Otherwise x509_openssl_add_credentials shall fail and return a non-zero number. **]**


###  x509_openssl_add_certificates
```c
int x509_openssl_add_certificates(SSL_CTX* ssl_ctx, const char* certificates);
```

`x509_openssl_add_certificates` adds all the certificates in `certificates` to `ssl_ctx`.

**SRS_X509_OPENSSL_02_010: [** If `ssl_ctx` is `NULL` then `x509_openssl_add_certificates` shall fail and return a non-zero value. **]**

**SRS_X509_OPENSSL_02_011: [** If `certificates` is `NULL` then `x509_openssl_add_certificates` shall fail and return a non-zero value. **]**

**SRS_X509_OPENSSL_02_012: [** `x509_openssl_add_certificates` shall get the memory BIO method function by calling `BIO_s_mem`. **]**

**SRS_X509_OPENSSL_02_013: [** `x509_openssl_add_certificates` shall create a new memory BIO by calling `BIO_new`. **]**

**SRS_X509_OPENSSL_02_014: [** `x509_openssl_add_certificates` shall load `certificates` into the memory BIO by a call to `BIO_puts`. **]**

**SRS_X509_OPENSSL_02_015: [** `x509_openssl_add_certificates` shall retrieve each certificate by a call to `PEM_read_bio_X509`. **]**

**SRS_X509_OPENSSL_02_016: [** `x509_openssl_add_certificates` shall add the certificate to the store by a call to `X509_STORE_add_cert`. **]**

**SRS_X509_OPENSSL_02_017: [** If `X509_STORE_add_cert` returns with error and that error is `X509_R_CERT_ALREADY_IN_HASH_TABLE` then `x509_openssl_add_certificates` shall ignore it as the certificate is already in the store. **]**

**SRS_X509_OPENSSL_02_018: [** In case of any failure `x509_openssl_add_certificates` shall fail and return a non-zero value. **]**

**SRS_X509_OPENSSL_02_019: [** Otherwise, `x509_openssl_add_certificates` shall succeed and return 0. **]**





