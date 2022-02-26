/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#include "hks_crypto_hal.h"

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

#include "hks_crypto_ed25519.h"
#include "hks_log.h"
#include "hks_mbedtls_aes.h"
#include "hks_mbedtls_bn.h"
#include "hks_mbedtls_common.h"
#include "hks_mbedtls_ecc.h"
#include "hks_mbedtls_ecdh.h"
#include "hks_mbedtls_ecdsa.h"
#include "hks_mbedtls_hash.h"
#include "hks_mbedtls_hmac.h"
#include "hks_mbedtls_kdf.h"
#include "hks_mbedtls_rsa.h"
#include "hks_mbedtls_x25519.h"
#include "hks_rkc.h"

#ifdef _CUT_AUTHENTICATE_
#undef HKS_SUPPORT_HASH_C
#undef HKS_SUPPORT_RSA_C
#undef HKS_SUPPORT_ECC_C
#undef HKS_SUPPORT_X25519_C
#undef HKS_SUPPORT_ED25519_C
#undef HKS_SUPPORT_KDF_PBKDF2
#endif

int32_t HksCryptoHalHmac(const struct HksBlob *key, uint32_t digestAlg, const struct HksBlob *msg,
    struct HksBlob *mac)
{
#ifdef HKS_SUPPORT_HMAC_C
    return HksMbedtlsHmac(key, digestAlg, msg, mac);
#else
    HKS_LOG_E("HKS_SUPPORT_HMAC_C macro is not on!");
    return HKS_ERROR_INVALID_ARGUMENT;
#endif
}

#ifndef _CUT_AUTHENTICATE_
int32_t HksCryptoHalHash(uint32_t alg, const struct HksBlob *msg, struct HksBlob *hash)
{
#ifdef HKS_SUPPORT_HASH_C
    return HksMbedtlsHash(alg, msg, hash);
#else
    HKS_LOG_E("HKS_SUPPORT_HASH_C macro is not on!");
    return HKS_ERROR_INVALID_ARGUMENT;
#endif
}
#endif /* _CUT_AUTHENTICATE_ */

int32_t HksCryptoHalBnExpMod(struct HksBlob *x, const struct HksBlob *a,
    const struct HksBlob *e, const struct HksBlob *n)
{
#ifdef HKS_SUPPORT_BN_C
    return HksMbedtlsBnExpMod(a, e, n, x);
#else
    HKS_LOG_E("HKS_SUPPORT_BN_C macro is not on!");
    return HKS_ERROR_INVALID_ARGUMENT;
#endif
}

#ifndef _CUT_AUTHENTICATE_
int32_t HksCryptoHalGenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    switch (spec->algType) {
#if defined(HKS_SUPPORT_AES_C) && defined(HKS_SUPPORT_AES_GENERATE_KEY)
        case HKS_ALG_AES:
            return HksMbedtlsAesGenerateKey(spec, key);
#endif
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_GENERATE_KEY)
        case HKS_ALG_RSA:
            return HksMbedtlsRsaGenerateKey(spec, key);
#endif
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECC_GENERATE_KEY)
        case HKS_ALG_ECC:
            return HksMbedtlsEccGenerateKey(spec, key);
#endif
#if defined(HKS_SUPPORT_X25519_C) && defined(HKS_SUPPORT_X25519_GENERATE_KEY)
        case HKS_ALG_X25519:
            return HksMbedtlsX25519GenerateKey(key);
#endif
#if defined(HKS_SUPPORT_ED25519_C) && defined(HKS_SUPPORT_ED25519_GENERATE_KEY)
        case HKS_ALG_ED25519:
            return HksEd25519GenerateKey(key);
#endif
        default:
            HKS_LOG_E("Unsupport alg type or macro is not on! type = 0x%X", spec->algType);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalGetMainKey(const struct HksBlob *message, struct HksBlob *mainKey)
{
    (void)message;

#ifndef _HARDWARE_ROOT_KEY_
    return HksRkcGetMainKey(mainKey);
#else
    /*
    * Currently, root key is implemented using stubs. 
    * Product adaptation needs to be performed based on hardware capabilities.
    */
    uint8_t stubBuf[] = {
        0x0c, 0xb4, 0x29, 0x39, 0xb7, 0x46, 0xa6, 0x4b,
        0xdd, 0xf3, 0x75, 0x4c, 0xe0, 0x73, 0x91, 0x51,
        0xc4, 0x88, 0xbe, 0xa4, 0xe1, 0x87, 0xb5, 0x42,
        0x06, 0x27, 0x08, 0x21, 0xe2, 0x8f, 0x9b, 0xc1,
    };

    if (memcpy_s(mainKey->data, mainKey->size, stubBuf, sizeof(stubBuf)) != EOK) {
        HKS_LOG_E("memcpy failed, get stub main key failed");
        return HKS_ERROR_BAD_STATE;
    }
    return HKS_SUCCESS;
#endif
}

int32_t HksCryptoHalGetPubKey(const struct HksBlob *keyIn, struct HksBlob *keyOut)
{
    /* KeyMaterialRsa, KeyMaterialEcc, KeyMaterial25519's size are same */
    if (keyIn->size < sizeof(struct KeyMaterialRsa)) {
        return HKS_ERROR_INVALID_KEY_SIZE;
    }

    struct KeyMaterialRsa *key = (struct KeyMaterialRsa *)(keyIn->data);
    switch (key->keyAlg) {
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_GET_PUBLIC_KEY)
        case HKS_ALG_RSA:
            return HksMbedtlsGetRsaPubKey(keyIn, keyOut);
#endif
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECC_GET_PUBLIC_KEY)
        case HKS_ALG_ECC:
            return HksMbedtlsGetEccPubKey(keyIn, keyOut);
#endif
#if defined(HKS_SUPPORT_ED25519_C) && defined(HKS_SUPPORT_ED2519_GET_PUBLIC_KEY)
        case HKS_ALG_ED25519:
            return HksGetEd25519PubKey(keyIn, keyOut);
#endif
#if defined(HKS_SUPPORT_X25519_C) && defined(HKS_SUPPORT_X25519_GET_PUBLIC_KEY)
        case HKS_ALG_X25519:
            return HksMbedtlsGetX25519PubKey(keyIn, keyOut);
#endif
        default:
            HKS_LOG_E("Unsupport key mode or macro is not on! mode = 0x%X", key->keyAlg);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}
#endif /* _CUT_AUTHENTICATE_ */

int32_t HksCryptoHalDeriveKey(const struct HksBlob *mainKey,
    const struct HksKeySpec *derivationSpec, struct HksBlob *derivedKey)
{
#ifdef HKS_SUPPORT_KDF_C
    return HksMbedtlsDeriveKey(mainKey, derivationSpec, derivedKey);
#else
    HKS_LOG_E("HKS_SUPPORT_KDF_C macro is not on!");
    return HKS_ERROR_INVALID_ARGUMENT;
#endif
}

#ifndef _CUT_AUTHENTICATE_
int32_t HksCryptoHalAgreeKey(const struct HksBlob *nativeKey, const struct HksBlob *pubKey,
    const struct HksKeySpec *spec, struct HksBlob *sharedKey)
{
    switch (spec->algType) {
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECDH_C)
        case HKS_ALG_ECDH:
            return HksMbedtlsEcdh(nativeKey, pubKey, spec, sharedKey);
#endif
#if defined(HKS_SUPPORT_X25519_C) && defined(HKS_SUPPORT_X25519_AGREE_KEY)
        case HKS_ALG_X25519:
            return HksMbedtlsX25519KeyAgreement(nativeKey, pubKey, sharedKey);
#endif
#ifdef HKS_SUPPORT_ED25519_TO_X25519
        case HKS_ALG_ED25519:
            return HksMbedtlsEd25519KeyAgreement(nativeKey, pubKey, sharedKey);
#endif
        default:
            HKS_LOG_E("Unsupport alg or macro is not on! alg = 0x%X", spec->algType);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalSign(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *signature)
{
    switch (usageSpec->algType) {
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_SIGN_VERIFY)
        case HKS_ALG_RSA:
            return HksMbedtlsRsaSign(key, usageSpec, message, signature);
#endif
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECDSA_C)
        case HKS_ALG_ECC:
            return HksMbedtlsEcdsaSign(key, usageSpec, message, signature);
#endif
#if defined(HKS_SUPPORT_ED25519_C) && defined(HKS_SUPPORT_ED25519_SIGN_VERIFY)
        case HKS_ALG_ED25519:
            return HksEd25519Sign(key, message, signature);
#endif
        default:
            HKS_LOG_E("Unsupport alg or macro is not on! alg = 0x%X", usageSpec->algType);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalVerify(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const struct HksBlob *signature)
{
    switch (usageSpec->algType) {
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_SIGN_VERIFY)
        case HKS_ALG_RSA:
            return HksMbedtlsRsaVerify(key, usageSpec, message, signature);
#endif
#if defined(HKS_SUPPORT_ECC_C) && defined(HKS_SUPPORT_ECDSA_C)
        case HKS_ALG_ECC:
            return HksMbedtlsEcdsaVerify(key, usageSpec, message, signature);
#endif
#if defined(HKS_SUPPORT_ED25519_C) && defined(HKS_SUPPORT_ED25519_SIGN_VERIFY)
        case HKS_ALG_ED25519:
            return HksEd25519Verify(key, message, signature);
#endif
        default:
            HKS_LOG_E("Unsupport alg or macro is not on! alg = 0x%X", usageSpec->algType);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}
#endif /* _CUT_AUTHENTICATE_ */

int32_t HksCryptoHalFillRandom(struct HksBlob *randomData)
{
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctrDrbg;
    int32_t ret = HksCtrDrbgSeed(&ctrDrbg, &entropy);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    do {
        ret = mbedtls_ctr_drbg_random(&ctrDrbg, randomData->data, randomData->size);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls random failed! mbedtls ret = 0x%X", ret);
            (void)memset_s(randomData->data, randomData->size, 0, randomData->size);
        }
    } while (0);

    mbedtls_ctr_drbg_free(&ctrDrbg);
    mbedtls_entropy_free(&entropy);
    return ret;
}

int32_t HksCryptoHalEncrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    switch (usageSpec->algType) {
#ifdef HKS_SUPPORT_AES_C
        case HKS_ALG_AES:
            return HksMbedtlsAesEncrypt(key, usageSpec, message, cipherText, tagAead);
#endif
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_CRYPT)
        case HKS_ALG_RSA:
            return HksMbedtlsRsaCrypt(key, usageSpec, message, true, cipherText);
#endif
        default:
            HKS_LOG_E("Unsupport alg or macro is not on! alg = 0x%X", usageSpec->algType);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalDecrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    switch (usageSpec->algType) {
#ifdef HKS_SUPPORT_AES_C
        case HKS_ALG_AES:
            return HksMbedtlsAesDecrypt(key, usageSpec, message, cipherText);
#endif
#if defined(HKS_SUPPORT_RSA_C) && defined(HKS_SUPPORT_RSA_CRYPT)
        case HKS_ALG_RSA:
            return HksMbedtlsRsaCrypt(key, usageSpec, message, false, cipherText);
#endif
        default:
            HKS_LOG_E("Unsupport alg or macro is not on! alg = 0x%X", usageSpec->algType);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}
