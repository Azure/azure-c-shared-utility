x509_openssl
=============

## Overview

x509_openssl provides a utility function that imports into a SSL context a pair of x509 certificate/private key  

## References

[OpenSSL](https://www.openssl.org)

```c
int x509_openssl_add_credentials(SSL_CTX* ssl_ctx, const char* x509certificate, const char* x509privatekey);
```

###  x509_openssl_add_credentials
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

