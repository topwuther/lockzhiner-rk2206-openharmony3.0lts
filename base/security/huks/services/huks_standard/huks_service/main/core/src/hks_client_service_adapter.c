/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#include "hks_client_service_adapter.h"

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#include "hks_log.h"
#include "hks_mem.h"

#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C)
static int32_t EvpKeyToX509Format(EVP_PKEY *pkey, struct HksBlob *x509Key)
{
    int32_t length = i2d_PUBKEY(pkey, NULL);
    if (length <= 0) {
        HKS_LOG_E("i2d_PUBKEY error %s", ERR_reason_error_string(ERR_get_error()));
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    uint8_t *key = HksMalloc(length);
    if (key == NULL) {
        HKS_LOG_E("malloc key fail");
        return HKS_ERROR_MALLOC_FAIL;
    }

    /* tmp will be modified in i2d_PUBKEY */
    uint8_t *tmp = key;
    if (i2d_PUBKEY(pkey, &tmp) != length) {
        HKS_LOG_E("i2d_PUBKEY error %s", ERR_reason_error_string(ERR_get_error()));
        HksFree(key);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    x509Key->size = length;
    x509Key->data = key;
    return HKS_SUCCESS;
}

#ifdef HKS_SUPPORT_RSA_C
static int32_t RsaToX509PublicKey(const struct HksBlob *mod, const struct HksBlob *e, struct HksBlob *x509Key)
{
    RSA *rsa = NULL;
    BIGNUM *rsaN = NULL;
    BIGNUM *rsaE = NULL;
    EVP_PKEY *pkey = NULL;
    int32_t result;

    do {
        result = HKS_ERROR_CRYPTO_ENGINE_ERROR;
        rsa = RSA_new();
        if (rsa == NULL) {
            HKS_LOG_E("rsa is null");
            break;
        }
        rsaN = BN_bin2bn(mod->data, mod->size, NULL);
        rsaE = BN_bin2bn(e->data, e->size, NULL);
        if (rsaN == NULL || rsaE == NULL) {
            HKS_LOG_E("BN_bin2bn error %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        if (RSA_set0_key(rsa, rsaN, rsaE, NULL) == 0) {
            HKS_LOG_E("RSA_set0_key error %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        pkey = EVP_PKEY_new();
        if (pkey == NULL) {
            HKS_LOG_E("pkey is null");
            break;
        }
        if (EVP_PKEY_set1_RSA(pkey, rsa) == 0) {
            HKS_LOG_E("EVP_PKEY_set1_RSA error %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }
        /* rsa need to be set to null to prevent Double-Free */
        rsa = NULL;
        result = EvpKeyToX509Format(pkey, x509Key);
    } while (0);

    SELF_FREE_PTR(rsa, RSA_free);
    SELF_FREE_PTR(rsaN, BN_free);
    SELF_FREE_PTR(rsaE, BN_free);
    SELF_FREE_PTR(pkey, EVP_PKEY_free);
    return result;
}
#endif

#ifdef HKS_SUPPORT_ECC_C
static int32_t GetEccNid(uint32_t keySize, int32_t *nid)
{
    int32_t nids[][2] = { /* 2 is size */
        { 256, NID_X9_62_prime256v1 },
        { 384, NID_secp384r1 },
        { 521, NID_secp521r1 }
    };

    uint32_t nidCount = sizeof(nids) / sizeof(nids[0]);

    for (uint32_t i = 0; i < nidCount; i++) {
        if (keySize == (uint32_t)nids[i][0]) {
            *nid = nids[i][1];
            return HKS_SUCCESS;
        }
    }

    HKS_LOG_E("not found nid!");
    return HKS_ERROR_INVALID_ARGUMENT;
}

static int32_t EccToX509PublicKey(uint32_t keySize, const struct HksBlob *x, const struct HksBlob *y,
    struct HksBlob *x509Key)
{
    int32_t nid;
    int32_t ret = GetEccNid(keySize, &nid);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("GetNidFromKeySize fail");
        return ret;
    }

    EC_KEY *ecKey = NULL;
    BIGNUM *ecX = NULL;
    BIGNUM *ecY = NULL;
    EVP_PKEY *pkey = NULL;
    ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
    do {
        ecKey = EC_KEY_new_by_curve_name(nid);
        if (ecKey == NULL) {
            HKS_LOG_E("EC_KEY_new_by_curve_name error %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        ecX = BN_bin2bn(x->data, x->size, NULL);
        ecY = BN_bin2bn(y->data, y->size, NULL);
        if (ecX == NULL || ecY == NULL) {
            HKS_LOG_E("x y point is null");
            break;
        }

        if (EC_KEY_set_public_key_affine_coordinates(ecKey, ecX, ecY) == 0) {
            HKS_LOG_E("EC_KEY_set_public_key_affine_coordinates error %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        EC_KEY_set_conv_form(ecKey, POINT_CONVERSION_UNCOMPRESSED);
        pkey = EVP_PKEY_new();
        if (pkey == NULL) {
            HKS_LOG_E("pkey is null");
            break;
        }

        if (EVP_PKEY_set1_EC_KEY(pkey, ecKey) == 0) {
            HKS_LOG_E("EVP_PKEY_set1_EC_KEY error %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        /* ecKey need to be set to null to prevent Double-Free */
        ecKey = NULL;
        ret = EvpKeyToX509Format(pkey, x509Key);
    } while (0);

    SELF_FREE_PTR(ecKey, EC_KEY_free);
    SELF_FREE_PTR(ecX, BN_free);
    SELF_FREE_PTR(ecY, BN_free);
    SELF_FREE_PTR(pkey, EVP_PKEY_free);
    return ret;
}
#endif
#endif

#if defined(HKS_SUPPORT_X25519_C) || defined(HKS_SUPPORT_ED25519_C)
static int32_t Curve25519ToX509PublicKey(const struct HksBlob *publicKey, struct HksBlob *x509Key)
{
    if (publicKey->size != HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256)) {
        HKS_LOG_E("Invalid public key size! key size = 0x%X", publicKey->size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    x509Key->data = (uint8_t *)HksMalloc(publicKey->size);
    if (x509Key->data == NULL) {
        HKS_LOG_E("X25519/Ed25519 to x509 public key malloc x509 key data failed!");
        return HKS_ERROR_MALLOC_FAIL;
    }

    if (memcpy_s(x509Key->data, publicKey->size, publicKey->data, publicKey->size) != EOK) {
        HKS_LOG_E("X25519/Ed25519 to x509 public key memcpy to x509 key data failed!");
        HKS_FREE_PTR(x509Key->data);
        return HKS_ERROR_BAD_STATE;
    }
    x509Key->size = publicKey->size;

    return HKS_SUCCESS;
}
#endif

int32_t TranslateToX509PublicKey(const struct HksBlob *publicKey, struct HksBlob *x509Key)
{
    if ((publicKey == NULL) || (publicKey->data == NULL) || (publicKey->size == 0) || (x509Key == NULL)) {
        HKS_LOG_E("translate to x509 public key invalid args");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (publicKey->size < sizeof(struct HksPubKeyInfo)) {
        HKS_LOG_E("translate to x509 public key invalid publicKey size");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksPubKeyInfo *publicKeyInfo = (struct HksPubKeyInfo *)publicKey->data;
    uint32_t offset = sizeof(struct HksPubKeyInfo);
    if ((publicKey->size - offset) < publicKeyInfo->nOrXSize) {
        HKS_LOG_E("translate to x509 public key invalid nOrXSize size");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksBlob material1 = { publicKeyInfo->nOrXSize, publicKey->data + offset };
    offset += publicKeyInfo->nOrXSize;
    if ((publicKey->size - offset) < publicKeyInfo->eOrYSize) {
        HKS_LOG_E("translate to x509 public key invalid eOrYSize size");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksBlob material2 = { publicKeyInfo->eOrYSize, publicKey->data + offset };
    switch (publicKeyInfo->keyAlg) {
#ifdef HKS_SUPPORT_RSA_C
        case HKS_ALG_RSA:
            return RsaToX509PublicKey(&material1, &material2, x509Key);
#endif
#ifdef HKS_SUPPORT_ECC_C
        case HKS_ALG_ECC:
            return EccToX509PublicKey(publicKeyInfo->keySize, &material1, &material2, x509Key);
#endif
#if defined(HKS_SUPPORT_X25519_C) || defined(HKS_SUPPORT_ED25519_C)
        case HKS_ALG_X25519:
        case HKS_ALG_ED25519:
            return Curve25519ToX509PublicKey(&material1, x509Key);
#endif
        default:
            HKS_LOG_E("Unsupport alg type! type = 0x%X", publicKeyInfo->keyAlg);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C)
#ifdef HKS_SUPPORT_RSA_C
static int32_t X509PublicKeyToRsa(EVP_PKEY *pkey, struct HksBlob *rsaPublicKey)
{
    RSA *rsa = EVP_PKEY_get1_RSA(pkey);
    if (rsa == NULL) {
        HKS_LOG_E("EVP_PKEY_get1_RSA error %s", ERR_reason_error_string(ERR_get_error()));
        return HKS_ERROR_NULL_POINTER;
    }

    int32_t nSize = BN_num_bytes(RSA_get0_n(rsa));
    int32_t eSize = BN_num_bytes(RSA_get0_e(rsa));
    if (nSize <= 0 || eSize <= 0) {
        HKS_LOG_E("X509PublicKeyToRsa BN_num_bytes failed");
        return HKS_ERROR_INTERNAL_ERROR;
    }

    /* n and e in RSA algorithm is small, will never overflow. */
    uint32_t totalSize = nSize + eSize + sizeof(struct  HksPubKeyInfo);
    uint8_t *keyBuffer = HksMalloc(totalSize);
    if (keyBuffer == NULL) {
        HKS_LOG_E("X509PublicKeyToRsa keyBuffer failed");
        return HKS_ERROR_MALLOC_FAIL;
    }

    struct HksPubKeyInfo *pubKeyInfo = (struct HksPubKeyInfo *)keyBuffer;
    pubKeyInfo->keyAlg = HKS_ALG_RSA;
    pubKeyInfo->keySize = RSA_size(rsa);
    pubKeyInfo->nOrXSize = nSize;
    pubKeyInfo->eOrYSize = eSize;
    if (BN_bn2bin(RSA_get0_n(rsa), keyBuffer + sizeof(struct HksPubKeyInfo)) == 0 ||
        BN_bn2bin(RSA_get0_e(rsa), keyBuffer + sizeof(struct HksPubKeyInfo) + nSize) == 0) {
        HKS_LOG_E("BN_bn2bin error %s", ERR_reason_error_string(ERR_get_error()));
        HKS_FREE_PTR(keyBuffer);
        return HKS_ERROR_INTERNAL_ERROR;
    }

    rsaPublicKey->data = keyBuffer;
    rsaPublicKey->size = totalSize;
    return HKS_SUCCESS;
}
#endif

#ifdef HKS_SUPPORT_ECC_C
static int32_t EcKeyToPublicKey(EC_KEY *ecKey, struct HksBlob *eccPublicKey)
{
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();
    int32_t ret;
    do {
        ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
        if (x == NULL || y == NULL) {
            HKS_LOG_E("X509PublicKeyToEcc BN_new failed");
            break;
        }

        if (EC_POINT_get_affine_coordinates_GFp(EC_KEY_get0_group(ecKey),
            EC_KEY_get0_public_key(ecKey), x, y, NULL) == 0) {
            HKS_LOG_E("EC_POINT_get_affine_coordinates_GFp error %s", ERR_reason_error_string(ERR_get_error()));
            break;
        }

        int32_t xSize = BN_num_bytes(x);
        int32_t ySize = BN_num_bytes(y);
        if (xSize <= 0 || ySize <= 0) {
            HKS_LOG_E("X509PublicKeyToEcc get x or y size failed");
            break;
        }

        /* x and y in ECC algorithm is small, will never overflow. */
        uint32_t totalSize = xSize + ySize + sizeof(struct HksPubKeyInfo);
        uint8_t *keyBuffer = HksMalloc(totalSize);
        if (keyBuffer == NULL) {
            HKS_LOG_E("X509PublicKeyToRsa keyBuffer failed");
            break;
        }

        struct HksPubKeyInfo *pubKeyInfo = (struct HksPubKeyInfo *)keyBuffer;
        pubKeyInfo->keyAlg = HKS_ALG_ECC;
        pubKeyInfo->keySize = EC_GROUP_order_bits(EC_KEY_get0_group(ecKey));
        pubKeyInfo->nOrXSize = xSize;
        pubKeyInfo->eOrYSize = ySize;
        if (BN_bn2bin(x, keyBuffer + sizeof(struct HksPubKeyInfo)) == 0 ||
            BN_bn2bin(y, keyBuffer + sizeof(struct HksPubKeyInfo) + xSize) == 0) {
            HKS_LOG_E("BN_bn2bin error %s", ERR_reason_error_string(ERR_get_error()));
            HKS_FREE_PTR(keyBuffer);
            break;
        }

        ret = HKS_SUCCESS;
        eccPublicKey->data = keyBuffer;
        eccPublicKey->size = totalSize;
    } while (0);

    SELF_FREE_PTR(x, BN_free);
    SELF_FREE_PTR(y, BN_free);
    return ret;
}

static int32_t X509PublicKeyToEcc(EVP_PKEY *pkey, struct HksBlob *eccPublicKey)
{
    EC_KEY *ecKey = EVP_PKEY_get1_EC_KEY(pkey);
    if (ecKey == NULL) {
        HKS_LOG_E("EVP_PKEY_get1_EC_KEY error %s", ERR_reason_error_string(ERR_get_error()));
        return HKS_ERROR_NULL_POINTER;
    }

    return EcKeyToPublicKey(ecKey, eccPublicKey);
}
#endif

int32_t TranslateFromX509PublicKey(const struct HksBlob *x509Key, struct HksBlob *publicKey)
{
    if (x509Key == NULL || x509Key->data == NULL || x509Key->size == 0 || publicKey == NULL) {
        HKS_LOG_E("TranslateFromX509PublicKey invalid args");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    EVP_PKEY *pkey = d2i_PUBKEY(NULL, (const unsigned char **)&x509Key->data, x509Key->size);
    if (pkey == NULL) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    int32_t ret;
    int32_t keyType = EVP_PKEY_base_id(pkey);
    if (keyType == EVP_PKEY_RSA) {
#ifdef HKS_SUPPORT_RSA_C
        ret = X509PublicKeyToRsa(pkey, publicKey);
#else
        ret = HKS_ERROR_INVALID_ALGORITHM;
#endif
    } else if (keyType == EVP_PKEY_EC) {
#ifdef HKS_SUPPORT_ECC_C
        ret = X509PublicKeyToEcc(pkey, publicKey);
#else
        ret = HKS_ERROR_INVALID_ALGORITHM;
#endif
    } else {
        HKS_LOG_E("Unsupport alg type!");
        ret = HKS_ERROR_INVALID_ARGUMENT;
    }

    SELF_FREE_PTR(pkey, EVP_PKEY_free);
    return ret;
}
#endif

#if defined(HKS_SUPPORT_ED25519_C) || defined(HKS_SUPPORT_X25519_C)
int32_t TranslateToInnerCurve25519Format(const uint32_t alg, const struct HksBlob *key, struct HksBlob *publicKey)
{
    if (key->size != HKS_KEY_BYTES(HKS_CURVE25519_KEY_SIZE_256)) {
        HKS_LOG_E("Invalid curve25519 public key size! key size = 0x%X", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    uint32_t totalSize = sizeof(struct HksPubKeyInfo) + key->size;
    uint8_t *buffer = (uint8_t *)HksMalloc(totalSize);
    if (buffer == NULL) {
        HKS_LOG_E("malloc failed! %u", totalSize);
        return HKS_ERROR_MALLOC_FAIL;
    }
    (void)memset_s(buffer, totalSize, 0, totalSize);

    struct HksPubKeyInfo *curve25519Key = (struct HksPubKeyInfo *)buffer;
    curve25519Key->keyAlg = alg;
    curve25519Key->keySize = HKS_CURVE25519_KEY_SIZE_256;
    curve25519Key->nOrXSize = key->size; /* curve25519 public key */

    uint32_t offset = sizeof(struct HksPubKeyInfo);
    if (memcpy_s(buffer + offset, totalSize - offset, key->data, key->size) != EOK) {
        HKS_LOG_E("copy pub key failed!");
        HKS_FREE_PTR(buffer);
        return HKS_ERROR_BAD_STATE;
    }
    publicKey->data = buffer;
    publicKey->size = totalSize;
    return HKS_SUCCESS;
}
#endif

#ifdef HKS_SUPPORT_AES_C
int32_t TranslateToInnerAesFormat(const struct HksBlob *key, struct HksBlob *outKey)
{
    if ((key->size != HKS_KEY_BYTES(HKS_AES_KEY_SIZE_128)) && (key->size != HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256))) {
        HKS_LOG_E("Invalid aes key size! key size = 0x%X", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    uint8_t *keyBuffer = (uint8_t *)HksMalloc(key->size);
    if (keyBuffer == NULL) {
        HKS_LOG_E("public key to inner key format malloc keyBuffer failed!");
        return HKS_ERROR_MALLOC_FAIL;
    }

    (void)memcpy_s(keyBuffer, key->size, key->data, key->size);
    outKey->data = keyBuffer;
    outKey->size = key->size;
    return HKS_SUCCESS;
}
#endif
