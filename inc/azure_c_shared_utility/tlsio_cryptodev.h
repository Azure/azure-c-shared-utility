#ifndef TLSIO_CRYPTODEV_H
#define TLSIO_CRYPTODEV_H

#include <stdint.h>

/* @brief sign data
 * @param data - data to sign
 * @param datalen - size of the data in bytes
 * @param signature - output buffer for the signature
 * @param signature_len - size of the signature in bytes
 * @param priv - private data passed to every call
 * @return - 1 on success, 0 on error
 */
typedef int (*tlsio_cryptodev_pkey_sign_t)(const uint8_t* data, int datalen, uint8_t* signature, int *signature_len, void* priv);

/* @brief decrypt data
 * @param cipher - data to decrypt
 * @param cipherlen - size of the data in bytes or a negative number to use the algorithm's default
 * @param plain - output buffer for the plaintext
 * @param plain_len - size of the plaintext in bytes
 * @param priv - private data passed to every call
 * @return - 1 on success, 0 on error
 */
typedef int (*tlsio_cryptodev_pkey_decrypt_t)(const uint8_t* cipher, int cipherlen, uint8_t* plain, int *plain_len, void* priv);

/* @brief destroy private data
 * @param priv data to destroy
 * @return - 1 on success, 0 on error
 */
typedef int (*tlsio_cryptodev_pkey_destroy_t)(void* priv);

typedef enum TLSIO_CRYPTODEV_PKEY_TYPE_TAG {
  TLSIO_CRYPTODEV_PKEY_TYPE_ECC = 0,
  TLSIO_CRYPTODEV_PKEY_TYPE_RSA = 1,
} TLSIO_CRYPTODEV_PKEY_TYPE;

typedef struct TLSIO_CRYPTODEV_PKEY_TAG
{
    tlsio_cryptodev_pkey_sign_t sign;
    tlsio_cryptodev_pkey_decrypt_t decrypt;
    tlsio_cryptodev_pkey_destroy_t destroy;
    TLSIO_CRYPTODEV_PKEY_TYPE type;
    void* private_data;
} TLSIO_CRYPTODEV_PKEY;

#endif /* TLSIO_CRYPTODEV_H */
