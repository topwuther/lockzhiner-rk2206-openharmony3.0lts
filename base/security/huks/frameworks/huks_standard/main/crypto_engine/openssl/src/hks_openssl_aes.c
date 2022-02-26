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

#include "hks_openssl_aes.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include "hks_log.h"
#include "hks_mem.h"
#include "hks_openssl_engine.h"
#include "hks_type_inner.h"

static int32_t AesGenKeyCheckParam(const struct HksKeySpec *spec)
{
    if ((spec->keyLen != HKS_AES_KEY_SIZE_128) && (spec->keyLen != HKS_AES_KEY_SIZE_256)) {
        HKS_LOG_E("Invlid aes key len %x!", spec->keyLen);
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

int32_t HksOpensslAesGenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    if (AesGenKeyCheckParam(spec) != HKS_SUCCESS) {
        HKS_LOG_E("aes generate key invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t keySizeByte = spec->keyLen / BIT_NUM_OF_UINT8;
    int32_t ret = HKS_FAILURE;

    uint8_t *tmpKey = (uint8_t *)HksMalloc(keySizeByte);
    if (tmpKey == NULL) {
        HKS_LOG_E("malloc buffer failed");
        return HKS_ERROR_MALLOC_FAIL;
    }

    do {
        if (RAND_bytes(tmpKey, keySizeByte) <= 0) {
            HKS_LOG_E("generate key is failed:0x%x", ret);
            break;
        }

        key->data = tmpKey;
        key->size = keySizeByte;
        ret = HKS_SUCCESS;
    } while (0);

    if (ret != HKS_SUCCESS) {
        (void)memset_s(tmpKey, keySizeByte, 0, keySizeByte);
        HksFree(tmpKey);
    }
    return ret;
}

static const EVP_CIPHER *GetGcmCipherType(uint32_t keySize)
{
    switch (keySize) {
        case (HKS_AES_KEY_SIZE_128 / HKS_BITS_PER_BYTE):
            return EVP_aes_128_gcm();
        case (HKS_AES_KEY_SIZE_192 / HKS_BITS_PER_BYTE):
            return EVP_aes_192_gcm();
        case (HKS_AES_KEY_SIZE_256 / HKS_BITS_PER_BYTE):
            return EVP_aes_256_gcm();
        default:
            return NULL;
    }
}

static const EVP_CIPHER *GetAeadCipherType(uint32_t keySize, uint32_t mode)
{
    if (mode == HKS_MODE_GCM) {
        return GetGcmCipherType(keySize);
    }
    return NULL;
}

static int32_t OpensslAesAeadInit(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    bool isEncrypt, EVP_CIPHER_CTX **ctx)
{
    int32_t ret;
    struct HksAeadParam *aeadParam = (struct HksAeadParam *)usageSpec->algParam;

    *ctx = EVP_CIPHER_CTX_new();
    if (*ctx == NULL) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(*ctx, GetAeadCipherType(key->size, usageSpec->mode), NULL, NULL, NULL);
    } else {
        ret = EVP_DecryptInit_ex(*ctx, GetAeadCipherType(key->size, usageSpec->mode), NULL, NULL, NULL);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    ret = EVP_CIPHER_CTX_ctrl(*ctx, EVP_CTRL_AEAD_SET_IVLEN, aeadParam->nonce.size, NULL);
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (isEncrypt) {
        ret = EVP_EncryptInit_ex(*ctx, NULL, NULL, key->data, aeadParam->nonce.data);
    } else {
        ret = EVP_DecryptInit_ex(*ctx, NULL, NULL, key->data, aeadParam->nonce.data);
    }
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        EVP_CIPHER_CTX_free(*ctx);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t OpensslAesAeadEncryptFinal(EVP_CIPHER_CTX *ctx, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    int outLen = 0;
    struct HksAeadParam *aeadParam = (struct HksAeadParam *)usageSpec->algParam;

    if (EVP_EncryptUpdate(ctx, NULL, &outLen, aeadParam->aad.data, aeadParam->aad.size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (EVP_EncryptUpdate(ctx, cipherText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    cipherText->size = outLen;

    if (EVP_EncryptFinal_ex(ctx, cipherText->data, &outLen) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, HKS_AE_TAG_LEN, tagAead->data) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

static int32_t OpensslAesAeadDecryptFinal(EVP_CIPHER_CTX *ctx, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *plainText)
{
    int outLen = 0;
    struct HksAeadParam *aeadParam = (struct HksAeadParam *)usageSpec->algParam;

    if (EVP_DecryptUpdate(ctx, NULL, &outLen, aeadParam->aad.data, aeadParam->aad.size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (EVP_DecryptUpdate(ctx, plainText->data, &outLen, message->data, message->size) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    plainText->size = outLen;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, aeadParam->tagDec.size, aeadParam->tagDec.data)
        != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    if (EVP_DecryptFinal_ex(ctx, plainText->data, &outLen) != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    return HKS_SUCCESS;
}

int32_t HksOpensslAesEncrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    EVP_CIPHER_CTX *ctx = NULL;
    struct HksBlob tmpCipherText = *cipherText;

    int32_t ret;
    switch (usageSpec->mode) {
        case HKS_MODE_GCM:
            ret = OpensslAesAeadInit(key, usageSpec, true, &ctx);
            if (ret != HKS_SUCCESS) {
                HKS_LOG_E("OpensslAesAeadInit fail, ret = %d", ret);
                return ret;
            }

            ret = OpensslAesAeadEncryptFinal(ctx, usageSpec, message, &tmpCipherText, tagAead);
            if (ret != HKS_SUCCESS) {
                HKS_LOG_E("OpensslAesAeadEncryptFinal fail, ret = %d", ret);
                EVP_CIPHER_CTX_free(ctx);
                return ret;
            }
            break;
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%x", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    cipherText->size = tmpCipherText.size;
    EVP_CIPHER_CTX_free(ctx);
    return HKS_SUCCESS;
}

int32_t HksOpensslAesDecrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *plainText)
{
    EVP_CIPHER_CTX *ctx = NULL;
    struct HksBlob tmpPlainText = *plainText;

    int32_t ret;
    switch (usageSpec->mode) {
        case HKS_MODE_GCM:
            ret = OpensslAesAeadInit(key, usageSpec, false, &ctx);
            if (ret != HKS_SUCCESS) {
                HKS_LOG_E("OpensslAesAeadDecryptInit fail, ret = %d", ret);
                return ret;
            }

            ret = OpensslAesAeadDecryptFinal(ctx, usageSpec, message, &tmpPlainText);
            if (ret != HKS_SUCCESS) {
                HKS_LOG_E("OpensslAesAeadDecryptFinal fail, ret = %d", ret);
                EVP_CIPHER_CTX_free(ctx);
                return ret;
            }
            break;
        default:
            HKS_LOG_E("Unsupport aes mode! mode = 0x%x", usageSpec->mode);
            return HKS_ERROR_INVALID_ARGUMENT;
    }

    plainText->size = tmpPlainText.size;
    EVP_CIPHER_CTX_free(ctx);
    return ret;
}
