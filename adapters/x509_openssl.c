// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/x509_openssl.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/const_defines.h"
#include "azure_c_shared_utility/tlsio_cryptodev.h"
#include "openssl/bio.h"
#include "openssl/rsa.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/err.h"

#ifdef __APPLE__
    #ifndef EVP_PKEY_id
        #define EVP_PKEY_id(evp_key) evp_key->type
    #endif // EVP_PKEY_id
#endif // __APPLE__

static int x509_rsa_ex_data_idx = -1;

static void log_ERR_get_error(const char* message)
{
    char buf[128];
    AZURE_UNREFERENCED_PARAMETER(buf);
    unsigned long error;
    int i;

    if (message != NULL)
    {
        LogError("%s", message);
    }

    error = ERR_get_error();

    for (i = 0; 0 != error; i++)
    {
        LogError("  [%d] %s", i, ERR_error_string(error, buf));
        error = ERR_get_error();
    }
}

static int load_certificate_chain(SSL_CTX* ssl_ctx, const char* certificate)
{
    int result;
    BIO* bio_cert;
    X509* x509_value;

    if ((bio_cert = BIO_new_mem_buf((char*)certificate, -1)) == NULL)
    {
        log_ERR_get_error("cannot create BIO");
        result = MU_FAILURE;
    }
    else
    {
        /* Codes_SRS_X509_OPENSSL_07_005: [ x509_openssl_add_ecc_credentials shall load the cert chain by calling PEM_read_bio_X509_AUX and SSL_CTX_use_certification. ] */
        if ((x509_value = PEM_read_bio_X509_AUX(bio_cert, NULL, NULL, NULL)) == NULL)
        {
            log_ERR_get_error("Failure PEM_read_bio_X509_AUX");
            result = MU_FAILURE;
        }
        else
        {
            if (SSL_CTX_use_certificate(ssl_ctx, x509_value) != 1)
            {
                log_ERR_get_error("Failure PEM_read_bio_X509_AUX");
                result = MU_FAILURE;
            }
            else
            {
                X509* ca_chain;

                result = 0;
                // If we could set up our certificate, now proceed to the CA
                // certificates.

                /* Codes_SRS_X509_OPENSSL_07_006: [ If successful x509_openssl_add_ecc_credentials shall to import each certificate in the cert chain. ] */
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && (OPENSSL_VERSION_NUMBER < 0x20000000L)
                SSL_CTX_clear_extra_chain_certs(ssl_ctx);
#else
                if (ssl_ctx->extra_certs != NULL)
                {
                    sk_X509_pop_free(ssl_ctx->extra_certs, X509_free);
                    ssl_ctx->extra_certs = NULL;
                }
#endif
                while ((ca_chain = PEM_read_bio_X509(bio_cert, NULL, NULL, NULL)) != NULL)
                {
                    if (SSL_CTX_add_extra_chain_cert(ssl_ctx, ca_chain) != 1)
                    {
                        X509_free(ca_chain);
                        result = MU_FAILURE;
                        break;
                    }
                }
                if (result != 0)
                {
                    // When the while loop ends, it's usually just EOF.
                    unsigned long err_value = ERR_peek_last_error();
                    if (ERR_GET_LIB(err_value) == ERR_LIB_PEM && ERR_GET_REASON(err_value) == PEM_R_NO_START_LINE)
                    {
                        ERR_clear_error();
                        result = 0;
                    }
                    else
                    {
                        result = MU_FAILURE;
                    }
                }
            }
            X509_free(x509_value);
        }
        BIO_free(bio_cert);
    }
    /* Codes_SRS_X509_OPENSSL_07_007: [ If any failure is encountered x509_openssl_add_ecc_credentials shall return a non-zero value. ] */
    return result;
}

static int load_ecc_key(SSL_CTX* ssl_ctx, EVP_PKEY* evp_key)
{
    int result;
    /* Codes_SRS_X509_OPENSSL_07_004: [ x509_openssl_add_ecc_credentials shall import the certification using by the EVP_PKEY. ] */
    if (SSL_CTX_use_PrivateKey(ssl_ctx, evp_key) != 1)
    {
        LogError("Failed SSL_CTX_use_PrivateKey");
        result = MU_FAILURE;
    }
    else
    {
        result = 0;
    }
    /* Codes_SRS_X509_OPENSSL_07_007: [ If any failure is encountered x509_openssl_add_ecc_credentials shall return a non-zero value. ] */
    return result;
}

static int load_key_RSA(SSL_CTX* ssl_ctx, EVP_PKEY* evp_key)
{
    int result;
    /*Codes_SRS_X509_OPENSSL_02_005: [ x509_openssl_add_credentials shall use PEM_read_bio_RSAPrivateKey to read the x509 private key. ]*/
    RSA* privatekey = EVP_PKEY_get1_RSA(evp_key);
    if (privatekey == NULL)
    {
        /*Codes_SRS_X509_OPENSSL_02_009: [ Otherwise x509_openssl_add_credentials shall fail and return a non-zero number. ]*/
        log_ERR_get_error("Failure reading RSA private key");
        result = MU_FAILURE;
    }
    else
    {
        /*Codes_SRS_X509_OPENSSL_02_007: [ x509_openssl_add_credentials shall use SSL_CTX_use_RSAPrivateKey to load the private key into the SSL context. ]*/
        if (SSL_CTX_use_RSAPrivateKey(ssl_ctx, privatekey) != 1)
        {
            /*Codes_SRS_X509_OPENSSL_02_009: [ Otherwise x509_openssl_add_credentials shall fail and return a non-zero number. ]*/
            log_ERR_get_error("Failure calling SSL_CTX_use_RSAPrivateKey");
            result = MU_FAILURE;
        }
        else
        {
            /*all is fine*/
            /*Codes_SRS_X509_OPENSSL_02_008: [ If no error occurs, then x509_openssl_add_credentials shall succeed and return 0. ]*/
            result = 0;
        }
        RSA_free(privatekey);
    }
    return result;
}

static int x509_openssl_rsa_sign(int type, const unsigned char* m, unsigned int m_length, unsigned char* sigret, unsigned int* siglen, const RSA* rsa) {
    (void) type;

    TLSIO_CRYPTODEV_PKEY *pkey = (TLSIO_CRYPTODEV_PKEY*) RSA_get_ex_data(rsa, x509_rsa_ex_data_idx);

    if (pkey == NULL) {
        return 0;
    }

    int siglen_signed;

    int res = pkey->sign(m, m_length, sigret, &siglen_signed, pkey->private_data);
    if (res && siglen_signed < 0) {
      res = 0;
    } else {
      *siglen = (unsigned int) siglen_signed;
    }

    return res;
}

static int x509_openssl_rsa_decrypt(int flen, const unsigned char* from, unsigned char* to, RSA* rsa, int padding) {
    (void) padding;

    TLSIO_CRYPTODEV_PKEY *pkey = (TLSIO_CRYPTODEV_PKEY*) RSA_get_ex_data(rsa, x509_rsa_ex_data_idx);

    if (pkey == NULL) {
        return 0;
    }

    int to_len;
    return pkey->decrypt(from, flen, to, &to_len, pkey->private_data);
}

static int x509_openssl_rsa_finish(RSA* rsa) {
    TLSIO_CRYPTODEV_PKEY *pkey = (TLSIO_CRYPTODEV_PKEY*) RSA_get_ex_data(rsa, x509_rsa_ex_data_idx);

    if (pkey == NULL) {
        return 0;
    }

    if (!pkey->destroy(pkey->private_data)) {
        return 0;
    }
    RSA_set_ex_data(rsa, x509_rsa_ex_data_idx, NULL);

    return 1;
}

static int copy_cert_params_rsa(RSA* privkey, const char* x509certificate) {
    BIO* bio_cert;
    X509* x509_value;
    EVP_PKEY* pubkey_evp;
    RSA* pubkey;

    if ((bio_cert = BIO_new_mem_buf((char*)x509certificate, -1)) == NULL)
    {
        log_ERR_get_error("cannot create BIO");
        return MU_FAILURE;
    }
    x509_value = PEM_read_bio_X509_AUX(bio_cert, NULL, NULL, NULL);
    BIO_free(bio_cert);
    if (x509_value == NULL)
    {
        log_ERR_get_error("Failure PEM_read_bio_X509_AUX");
        return MU_FAILURE;
    }

    pubkey_evp = X509_get_pubkey(x509_value);
    X509_free(x509_value);

    if (pubkey_evp == NULL)
    {
        log_ERR_get_error("Failure X509_get_pubkey");
        return MU_FAILURE;
    }

#if OPENSSL_VERSION_NUMBER >= 0x10100003L
    pubkey = EVP_PKEY_get0_RSA(pubkey_evp);
#else
    pubkey = pubkey_evp->pkey.rsa;
#endif

    if (pubkey == NULL)
    {
        log_ERR_get_error("Can't get RSA key");
        return MU_FAILURE;
    }

#if OPENSSL_VERSION_NUMBER >= 0x10100003L
    const BIGNUM* pub_n;
    const BIGNUM* pub_e;
    RSA_get0_key(pubkey, &pub_n, &pub_e, NULL);
    RSA_set0_key(privkey, BN_dup(pub_n), BN_dup(pub_e), NULL);
#else
    privkey->n = BN_dup(pubkey->n);
    privkey->e = BN_dup(pubkey->e);
#endif
    return 0;
}

static int x509_openssl_add_credentials_common(SSL_CTX* ssl_ctx, const char* x509certificate, EVP_PKEY* evp_key) {
    int result = MU_FAILURE;

    // Check the type for the EVP key
    int evp_type = EVP_PKEY_id(evp_key);
    if (evp_type == EVP_PKEY_RSA || evp_type == EVP_PKEY_RSA2)
    {
        if (load_key_RSA(ssl_ctx, evp_key) != 0)
        {
            LogError("failure loading RSA private key cert");
            result = MU_FAILURE;
        }
        else
        {
            result = 0;
        }
    }
    else
    {
        if (load_ecc_key(ssl_ctx, evp_key) != 0)
        {
            LogError("failure loading ECC private key cert");
            result = MU_FAILURE;
        }
        else
        {
            result = 0;
        }
    }
    EVP_PKEY_free(evp_key);


    if (result == 0)
    {
        // Load the certificate chain
        if (load_certificate_chain(ssl_ctx, x509certificate) != 0)
        {
            LogError("failure loading private key cert");
            result = MU_FAILURE;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

int x509_openssl_add_credentials_cryptodev(SSL_CTX* ssl_ctx, const char* x509certificate, TLSIO_CRYPTODEV_PKEY* x509cryptodevprivatekey) {
    int result = MU_FAILURE;
    EVP_PKEY* evp_key = NULL;

    if (ssl_ctx == NULL || x509certificate == NULL || x509cryptodevprivatekey == NULL)
    {
        /*Codes_SRS_X509_OPENSSL_02_009: [ Otherwise x509_openssl_add_credentials shall fail and return a non-zero number. ]*/
        LogError("invalid parameter detected: ssl_ctx=%p, x509certificate=%p, x509privatekey=%p", ssl_ctx, x509certificate, x509cryptodevprivatekey);
        result = MU_FAILURE;
    }
    else {
        // TODO: ECC keys
        static RSA_METHOD* x509_rsa_meth = NULL;
        if (x509_rsa_meth == NULL) {
#if OPENSSL_VERSION_NUMBER >= 0x10100005L
            x509_rsa_meth = RSA_meth_dup(RSA_get_default_method());
            if (x509_rsa_meth == NULL) {
              return MU_FAILURE;
            }
            RSA_meth_set1_name(x509_rsa_meth, "Azure Tlsio RSA method");
            RSA_meth_set_flags(x509_rsa_meth, 0);
            RSA_meth_set_sign(x509_rsa_meth, x509_openssl_rsa_sign);
            RSA_meth_set_priv_dec(x509_rsa_meth, x509_openssl_rsa_decrypt);
            RSA_meth_set_finish(x509_rsa_meth, x509_openssl_rsa_finish);
#else
            x509_rsa_meth = (RSA_METHOD*) OPENSSL_malloc(sizeof(RSA_METHOD));
            if (x509_rsa_meth == NULL) {
              return MU_FAILURE;
            }
            memcpy(x509_rsa_meth, RSA_get_default_method(), sizeof(RSA_METHOD));

            x509_rsa_meth->name = OPENSSL_strdup("Azure Tlsio RSA method");
            x509_rsa_meth->flags = RSA_FLAG_SIGN_VER;
            x509_rsa_meth->rsa_sign = x509_openssl_rsa_sign;
            x509_rsa_meth->rsa_priv_dec = x509_openssl_rsa_decrypt;
            x509_rsa_meth->finish = x509_openssl_rsa_finish;
#endif
        }
        RSA* rsa_key = RSA_new();

        RSA_set_method(rsa_key, x509_rsa_meth);
        if (x509_rsa_ex_data_idx < 0) {
          x509_rsa_ex_data_idx = RSA_get_ex_new_index(0, NULL, NULL, NULL, NULL);
        }
        RSA_set_ex_data(rsa_key, x509_rsa_ex_data_idx, x509cryptodevprivatekey);

        if(x509cryptodevprivatekey != NULL) {
            // cryptodev keys need params to be set, otherwise checks in openssl will fail
            if(copy_cert_params_rsa(rsa_key, x509certificate) != 0) {
                RSA_free(rsa_key);
                return MU_FAILURE;
            }
        }
        evp_key = EVP_PKEY_new();
        EVP_PKEY_set1_RSA(evp_key, rsa_key);
        result = 0;
    }

    if (result == 0) {
        return x509_openssl_add_credentials_common(ssl_ctx, x509certificate, evp_key);
    }

    return result;
}

int x509_openssl_add_credentials(SSL_CTX* ssl_ctx, const char* x509certificate, const char* x509privatekey)
{
    int result = MU_FAILURE;
    EVP_PKEY* evp_key = NULL;

    if (ssl_ctx == NULL || x509certificate == NULL || x509privatekey == NULL)
    {
        /*Codes_SRS_X509_OPENSSL_02_009: [ Otherwise x509_openssl_add_credentials shall fail and return a non-zero number. ]*/
        LogError("invalid parameter detected: ssl_ctx=%p, x509certificate=%p, x509privatekey=%p", ssl_ctx, x509certificate, x509privatekey);
        result = MU_FAILURE;
    }

    BIO* bio_key = BIO_new_mem_buf((char*)x509privatekey, -1); /*taking off the const from the pointer is needed on older versions of OPENSSL*/
    if (bio_key == NULL)
    {
        log_ERR_get_error("cannot create private key BIO");
        result = MU_FAILURE;
    }
    else
    {
        // Get the Private Key type
        evp_key = PEM_read_bio_PrivateKey(bio_key, NULL, NULL, NULL);
        if (evp_key == NULL)
        {
            log_ERR_get_error("Failure creating private key evp_key");
            result = MU_FAILURE;
        }
        BIO_free(bio_key);
    }
    result = 0;

    if (result == 0) {
        return x509_openssl_add_credentials_common(ssl_ctx, x509certificate, evp_key);
    }

    return result;
}

/*return 0 if everything was ok, any other number to signal an error*/
/*this function inserts a x509certificate+x509privatekey to a SSL_CTX (ssl context) in order to authenticate the device with the service*/
int x509_openssl_add_certificates(SSL_CTX* ssl_ctx, const char* certificates)
{
    int result;

    /*Codes_SRS_X509_OPENSSL_02_010: [ If ssl_ctx is NULL then x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
    /*Codes_SRS_X509_OPENSSL_02_011: [ If certificates is NULL then x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
    if ((certificates == NULL) || (ssl_ctx == NULL))
    {
        /*Codes_SRS_X509_OPENSSL_02_018: [ In case of any failure x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
        LogError("invalid argument SSL_CTX* ssl_ctx=%p, const char* certificates=%s", ssl_ctx, MU_P_OR_NULL(certificates));
        result = MU_FAILURE;
    }
    else
    {
        X509_STORE* cert_store = SSL_CTX_get_cert_store(ssl_ctx);
        if (cert_store == NULL)
        {
            /*Codes_SRS_X509_OPENSSL_02_018: [ In case of any failure x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
            log_ERR_get_error("failure in SSL_CTX_get_cert_store.");
            result = MU_FAILURE;
        }
        else
        {
            /*Codes_SRS_X509_OPENSSL_02_012: [ x509_openssl_add_certificates shall get the memory BIO method function by calling BIO_s_mem. ]*/
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && (OPENSSL_VERSION_NUMBER < 0x20000000L)
            const BIO_METHOD* bio_method;
#else
            BIO_METHOD* bio_method;
#endif
            bio_method = BIO_s_mem();
            if (bio_method == NULL)
            {
                /*Codes_SRS_X509_OPENSSL_02_018: [ In case of any failure x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
                log_ERR_get_error("failure in BIO_s_mem");
                result = MU_FAILURE;
            }
            else
            {
                /*Codes_SRS_X509_OPENSSL_02_013: [ x509_openssl_add_certificates shall create a new memory BIO by calling BIO_new. ]*/
                BIO* cert_memory_bio = BIO_new(bio_method);
                if (cert_memory_bio == NULL)
                {
                    /*Codes_SRS_X509_OPENSSL_02_018: [ In case of any failure x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
                    log_ERR_get_error("failure in BIO_new");
                    result = MU_FAILURE;
                }
                else
                {
                    /*Codes_SRS_X509_OPENSSL_02_014: [ x509_openssl_add_certificates shall load certificates into the memory BIO by a call to BIO_puts. ]*/
                    int puts_result = BIO_puts(cert_memory_bio, certificates);
                    if ((puts_result<0) || (puts_result != (int)strlen(certificates)))
                    {
                        /*Codes_SRS_X509_OPENSSL_02_018: [ In case of any failure x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
                        log_ERR_get_error("failure in BIO_puts");
                        result = MU_FAILURE;
                    }
                    else
                    {
                        X509* certificate;
                        /*Codes_SRS_X509_OPENSSL_02_015: [ x509_openssl_add_certificates shall retrieve each certificate by a call to PEM_read_bio_X509. ]*/
                        while ((certificate = PEM_read_bio_X509(cert_memory_bio, NULL, NULL, NULL)) != NULL)
                        {
                            /*Codes_SRS_X509_OPENSSL_02_016: [ x509_openssl_add_certificates shall add the certificate to the store by a call to X509_STORE_add_cert. ]*/
                            if (!X509_STORE_add_cert(cert_store, certificate))
                            {
                                /*Codes_SRS_X509_OPENSSL_02_017: [ If X509_STORE_add_cert returns with error and that error is X509_R_CERT_ALREADY_IN_HASH_TABLE then x509_openssl_add_certificates shall ignore it as the certificate is already in the store. ]*/
                                /*if the certificate is already in the store, then ERR_peek_error would return REASON == X509_R_CERT_ALREADY_IN_HASH_TABLE, so that's no a "real" error*/
                                unsigned long error = ERR_peek_error();
                                if (ERR_GET_REASON(error) == X509_R_CERT_ALREADY_IN_HASH_TABLE)
                                {
                                    /*not an error*/
                                }
                                else
                                {
                                    /*Codes_SRS_X509_OPENSSL_02_018: [ In case of any failure x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
                                    log_ERR_get_error("failure in X509_STORE_add_cert");
                                    X509_free(certificate);
                                    break;
                                }

                            }
                            X509_free(certificate);
                        }

                        if (certificate == NULL)
                        {
                            /*Codes_SRS_X509_OPENSSL_02_019: [ Otherwise, x509_openssl_add_certificates shall succeed and return 0. ]*/
                            result = 0;/*all is fine*/
                        }
                        else
                        {
                            /*previous while loop terminated unfortunately*/
                            /*Codes_SRS_X509_OPENSSL_02_018: [ In case of any failure x509_openssl_add_certificates shall fail and return a non-zero value. ]*/
                            result = MU_FAILURE;
                        }
                    }
                    BIO_free(cert_memory_bio);
                }
            }
        }
    }
    return result;


}

