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

#ifdef HKS_SUPPORT_AES_C

#include "hks_mbedtls_aes.h"

#include <mbedtls/aes.h>
#include <mbedtls/ccm.h>
#include <mbedtls/cipher.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/gcm.h>

#include "hks_log.h"
#include "hks_mbedtls_common.h"
#include "hks_mem.h"

#define HKS_AES_CBC_NOPADDING_IV_SIZE 16

#ifdef HKS_SUPPORT_AES_GENERATE_KEY
int32_t HksMbedtlsAesGenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    const uint32_t keyByteLen = spec->keyLen / HKS_BITS_PER_BYTE;

    uint8_t *outKey = (uint8_t *)HksMalloc(keyByteLen);
    if (outKey == NULL) {
        return HKS_ERROR_MALLOC_FAIL;
    }

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctrDrbg;
    int32_t ret = HksCtrDrbgSeed(&ctrDrbg, &entropy);
    if (ret != HKS_SUCCESS) {
        HKS_FREE_PTR(outKey);
        return ret;
    }

    do {
        ret = mbedtls_ctr_drbg_random(&ctrDrbg, outKey, keyByteLen);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls ctr drbg random failed! mbedtls ret = 0x%X", ret);
            (void)memset_s(outKey, keyByteLen, 0, keyByteLen);
            HKS_FREE_PTR(outKey);
            break;
        }

        key->data = outKey;
        key->size = keyByteLen;
    } while (0);

    mbedtls_ctr_drbg_free(&ctrDrbg);
    mbedtls_entropy_free(&entropy);
    return ret;
}
#endif /* HKS_SUPPORT_AES_GENERATE_KEY */

#ifdef HKS_SUPPORT_AES_CBC_NOPADDING
static int32_t AesCbcNoPaddingCrypt(const struct HksBlob *key, const struct HksCipherParam *cipherParam,
    const struct HksBlob *message, const bool encrypt, struct HksBlob *cipherText)
{
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);

    int32_t ret;
    do {
        if (encrypt) {
            ret = mbedtls_aes_setkey_enc(&ctx, key->data, key->size * HKS_BITS_PER_BYTE);
        } else {
            ret = mbedtls_aes_setkey_dec(&ctx, key->data, key->size * HKS_BITS_PER_BYTE);
        }
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls aes set key failed! mbedtls ret = 0x%X", ret);
            break;
        }

        /* mbedtls_aes_crypt_cbc will refresh iv, so need a temp iv */
        uint8_t tmpIv[HKS_AES_CBC_NOPADDING_IV_SIZE];
        if (memcpy_s(tmpIv, HKS_AES_CBC_NOPADDING_IV_SIZE, cipherParam->iv.data, cipherParam->iv.size) != EOK) {
            HKS_LOG_E("Memcpy temp iv failed!");
            break;
        }

        ret = mbedtls_aes_crypt_cbc(&ctx, (encrypt ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT),
            message->size, tmpIv, message->data, cipherText->data);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtks aes cbc crypt failed! mbedtls ret = 0x%X", ret);
            (void)memset_s(cipherText->data, cipherText->size, 0, cipherText->size);
            break;
        }
        cipherText->size = message->size;
    } while (0);

    mbedtls_aes_free(&ctx);
    return ret;
}
#endif /* HKS_SUPPORT_AES_CBC_NOPADDING */

#ifdef HKS_SUPPORT_AES_CBC_PKCS7
static int32_t AesCbcPkcs7Crypt(const struct HksBlob *key, const struct HksCipherParam *cipherParam,
    const struct HksBlob *message, const bool encrypt, struct HksBlob *cipherText)
{
    const uint32_t keyBitLen = key->size * HKS_BITS_PER_BYTE;
    const mbedtls_cipher_info_t *info =
        mbedtls_cipher_info_from_values(MBEDTLS_CIPHER_ID_AES, keyBitLen, MBEDTLS_MODE_CBC);

    mbedtls_cipher_context_t ctx;
    mbedtls_cipher_init(&ctx);

    int32_t ret;
    do {
        ret = mbedtls_cipher_setup(&ctx, info);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("Mbedtls cbc pkcs7 setup ctx failed! mbedtls ret = 0x%X", ret);
            break;
        }

        ret = mbedtls_cipher_setkey(&ctx, key->data, keyBitLen, (encrypt ? MBEDTLS_ENCRYPT : MBEDTLS_DECRYPT));
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("Mbedtls cbc pkcs7 set key failed! mbedtls ret = 0x%X", ret);
            break;
        }

        ret = mbedtls_cipher_crypt(&ctx, cipherParam->iv.data, cipherParam->iv.size,
            message->data, message->size, cipherText->data, (size_t *)&(cipherText->size));
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls cbc pkcs7 crypt failed! mbedtls ret = 0x%X", ret);
            (void)memset_s(cipherText->data, cipherText->size, 0, cipherText->size);
        }
    } while (0);

    mbedtls_cipher_free(&ctx);
    return ret;
}
#endif /* HKS_SUPPORT_AES_CBC_PKCS7 */

#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7)
static int32_t AesCbcCrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const bool encrypt, struct HksBlob *cipherText)
{
    const struct HksCipherParam *cipherParam = (struct HksCipherParam *)(usageSpec->algParam);

    switch (usageSpec->padding) {
#ifdef HKS_SUPPORT_AES_CBC_NOPADDING
        case HKS_PADDING_NONE:
            return AesCbcNoPaddingCrypt(key, cipherParam, message, encrypt, cipherText);
#endif
#ifdef HKS_SUPPORT_AES_CBC_PKCS7
        case HKS_PADDING_PKCS7:
            return AesCbcPkcs7Crypt(key, cipherParam, message, encrypt, cipherText);
#endif
        default:
            HKS_LOG_E("Unsupport padding! mode = 0x%X", usageSpec->padding);
            return HKS_ERROR_INVALID_PADDING;
    }
}
#endif /* HKS_SUPPORT_AES_CBC_NOPADDING or HKS_SUPPORT_AES_CBC_PKCS7 */

#ifdef HKS_SUPPORT_AES_GCM
static int32_t AesEncryptGcm(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    int32_t ret;
    do {
        ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key->data, key->size * HKS_BITS_PER_BYTE);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls aes gcm set key failed! mbedtls ret = 0x%X", ret);
            break;
        }

        const struct HksAeadParam *aeadParam = (struct HksAeadParam *)(usageSpec->algParam);
        ret = mbedtls_gcm_crypt_and_tag(&ctx, MBEDTLS_GCM_ENCRYPT, message->size,
            aeadParam->nonce.data, aeadParam->nonce.size, aeadParam->aad.data, aeadParam->aad.size,
            message->data, cipherText->data, tagAead->size, tagAead->data);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls aes gcm encryot failed! mbedtls ret = 0x%X", ret);
            (void)memset_s(cipherText->data, cipherText->size, 0, cipherText->size);
            (void)memset_s(tagAead->data, tagAead->size, 0, tagAead->size);
            break;
        }
        cipherText->size = message->size;
    } while (0);

    mbedtls_gcm_free(&ctx);
    return ret;
}

static int32_t AesDecryptGcm(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    int32_t ret;
    do {
        ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key->data, key->size * HKS_BITS_PER_BYTE);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls aes gcm set key failed! mbedtls ret = 0x%X", ret);
            break;
        }

        const struct HksAeadParam *aeadParam = (struct HksAeadParam *)(usageSpec->algParam);
        ret = mbedtls_gcm_auth_decrypt(&ctx, message->size, aeadParam->nonce.data, aeadParam->nonce.size,
            aeadParam->aad.data, aeadParam->aad.size, aeadParam->tagDec.data, aeadParam->tagDec.size,
            message->data, cipherText->data);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls aes gcm decrypt failed! mbedtls ret = 0x%X", ret);
            (void)memset_s(cipherText->data, cipherText->size, 0, cipherText->size);
            break;
        }
        cipherText->size = message->size;
    } while (0);

    mbedtls_gcm_free(&ctx);
    return ret;
}
#endif /* HKS_SUPPORT_AES_GCM */

#ifdef HKS_SUPPORT_AES_CCM
static int32_t AesEncryptCcm(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    mbedtls_ccm_context ctx;
    mbedtls_ccm_init(&ctx);

    int32_t ret;
    do {
        ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key->data, key->size * HKS_BITS_PER_BYTE);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls aes ccm set key failed! mbedtls ret = 0x%X", ret);
            break;
        }

        const struct HksAeadParam *aeadParam = (struct HksAeadParam *)(usageSpec->algParam);
        ret = mbedtls_ccm_encrypt_and_tag(&ctx, message->size,
            aeadParam->nonce.data, aeadParam->nonce.size, aeadParam->aad.data, aeadParam->aad.size,
            message->data, cipherText->data, tagAead->data, tagAead->size);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls aes ccm encrypt failed! mbedtls ret = 0x%X", ret);
            (void)memset_s(cipherText->data, cipherText->size, 0, cipherText->size);
            (void)memset_s(tagAead->data, tagAead->size, 0, tagAead->size);
            break;
        }
        cipherText->size = message->size;
    } while (0);

    mbedtls_ccm_free(&ctx);
    return ret;
}

static int32_t AesDecryptCcm(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    mbedtls_ccm_context ctx;
    mbedtls_ccm_init(&ctx);

    int32_t ret;
    do {
        ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key->data, key->size * HKS_BITS_PER_BYTE);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls aes ccm set key failed! mbedtls ret = 0x%X", ret);
            break;
        }

        const struct HksAeadParam *aeadParam = (struct HksAeadParam *)(usageSpec->algParam);
        ret = mbedtls_ccm_auth_decrypt(&ctx, message->size, aeadParam->nonce.data, aeadParam->nonce.size,
            aeadParam->aad.data, aeadParam->aad.size, message->data, cipherText->data,
            aeadParam->tagDec.data, aeadParam->tagDec.size);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls aes ccm decrypt failed! mbedtls ret = 0x%X", ret);
            (void)memset_s(cipherText->data, cipherText->size, 0, cipherText->size);
            break;
        }
        cipherText->size = message->size;
    } while (0);

    mbedtls_ccm_free(&ctx);
    return ret;
}
#endif /* HKS_SUPPORT_AES_CCM */

static int32_t CheckKeySize(const struct HksBlob *key)
{
    if ((key->size != HKS_KEY_BYTES(HKS_AES_KEY_SIZE_128)) && (key->size != HKS_KEY_BYTES(HKS_AES_KEY_SIZE_192)) &&
        (key->size != HKS_KEY_BYTES(HKS_AES_KEY_SIZE_256))) {
        return HKS_ERROR_INVALID_KEY_SIZE;
    }

    return HKS_SUCCESS;
}

int32_t HksMbedtlsAesEncrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    if (CheckKeySize(key) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid aes keySiz = 0x%X", key->size);
        return HKS_ERROR_INVALID_KEY_SIZE;
    }

    switch (usageSpec->mode) {
#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7)
        case HKS_MODE_CBC:
            return AesCbcCrypt(key, usageSpec, message, true, cipherText);
#endif
#ifdef HKS_SUPPORT_AES_GCM
        case HKS_MODE_GCM:
            return AesEncryptGcm(key, usageSpec, message, cipherText, tagAead);
#endif
#ifdef HKS_SUPPORT_AES_CCM
        case HKS_MODE_CCM:
            return AesEncryptCcm(key, usageSpec, message, cipherText, tagAead);
#endif
        default:
            HKS_LOG_E("Unsupport key alg! mode = 0x%X", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksMbedtlsAesDecrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    if (CheckKeySize(key) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid aes keySize = 0x%X", key->size);
        return HKS_ERROR_INVALID_KEY_SIZE;
    }

    switch (usageSpec->mode) {
#if defined(HKS_SUPPORT_AES_CBC_NOPADDING) || defined(HKS_SUPPORT_AES_CBC_PKCS7)
        case HKS_MODE_CBC:
            return AesCbcCrypt(key, usageSpec, message, false, cipherText);
#endif
#ifdef HKS_SUPPORT_AES_GCM
        case HKS_MODE_GCM:
            return AesDecryptGcm(key, usageSpec, message, cipherText);
#endif
#ifdef HKS_SUPPORT_AES_CCM
        case HKS_MODE_CCM:
            return AesDecryptCcm(key, usageSpec, message, cipherText);
#endif
        default:
            HKS_LOG_E("Unsupport key alg! mode = 0x%X", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}
#endif /* HKS_SUPPORT_AES_C */
