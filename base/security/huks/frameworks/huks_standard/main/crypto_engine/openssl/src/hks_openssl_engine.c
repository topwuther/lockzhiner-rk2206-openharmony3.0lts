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

#include "hks_openssl_engine.h"

#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

#include "hks_crypto_hal.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_openssl_aes.h"
#include "hks_openssl_curve25519.h"
#include "hks_openssl_ecc.h"
#include "hks_openssl_ed25519tox25519.h"
#include "hks_openssl_hash.h"
#include "hks_openssl_hmac.h"
#include "hks_openssl_kdf.h"
#include "hks_type_inner.h"

void HksLogOpensslError()
{
    char szErr[HKS_OPENSSL_ERROR_LEN] = {0};
    unsigned long errCode;

    errCode = ERR_get_error();
    ERR_error_string_n(errCode, szErr, HKS_OPENSSL_ERROR_LEN);

    HKS_LOG_E("Openssl engine fail, error code = %lu, error string = %s", errCode, szErr);
}

inline int32_t HksOpensslCheckBlob(const struct HksBlob *blob)
{
    if ((blob == NULL) || (blob->data == NULL) || (blob->size == 0)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t GenKeyCheckParam(const struct HksKeySpec *spec, struct HksBlob *key)
{
    if ((spec == NULL) || (key == NULL)) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return HKS_SUCCESS;
}

static int32_t SignVerifyCheckParam(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const struct HksBlob *signature)
{
    if (HksOpensslCheckBlob(key) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid param key!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (HksOpensslCheckBlob(message) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid param message!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (HksOpensslCheckBlob(signature) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid param signature!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (usageSpec == NULL) {
        HKS_LOG_E("Invalid param usageSpec!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t DeriveKeyCheckParam(const struct HksBlob *mainKey, const struct HksKeySpec *derivationSpec,
    struct HksBlob *derivedKey)
{
    if (HksOpensslCheckBlob(mainKey) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid mainKey params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if ((derivationSpec == NULL) || (derivationSpec->algParam == NULL)) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (derivedKey == NULL) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t AgreeKeyCheckParam(const struct HksBlob *nativeKey, const struct HksBlob *pubKey,
    const struct HksKeySpec *spec, struct HksBlob *sharedKey)
{
    if (HksOpensslCheckBlob(nativeKey) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid nativeKey params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (HksOpensslCheckBlob(pubKey) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid pubKey params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (spec == NULL) {
        HKS_LOG_E("Invalid spec params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (sharedKey == NULL) {
        HKS_LOG_E("Invalid sharedKey params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t EncryptCheckParam(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    if (HksOpensslCheckBlob(key) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid param key!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (HksOpensslCheckBlob(message) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid param message!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (HksOpensslCheckBlob(cipherText) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid param cipherText!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (usageSpec == NULL) {
        HKS_LOG_E("Invalid param usageSpec!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static int32_t DecryptCheckParam(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    return EncryptCheckParam(key, usageSpec, message, cipherText);
}

const EVP_MD *GetOpensslAlg(uint32_t alg)
{
    switch (alg) {
        case HKS_DIGEST_SHA256:
            return EVP_sha256();
        case HKS_DIGEST_SHA384:
            return EVP_sha384();
        case HKS_DIGEST_SHA512:
            return EVP_sha512();
        default:
            return NULL;
    }
}

int32_t HksCryptoHalFillRandom(struct HksBlob *randomData)
{
    if (HksOpensslCheckBlob(randomData) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    RAND_bytes(randomData->data, randomData->size);
    if (randomData->size == 1) {
        return HKS_SUCCESS;
    }

    uint32_t j = 0;

    for (uint32_t i = 0; i < randomData->size; i++) {
        if (randomData->data[i] == 0) {
            j++;
        }
    }
    if (j == randomData->size) {
        HKS_LOG_E("fill random failed, size %x", randomData->size);
        return HKS_ERROR_UNKNOWN_ERROR;
    }
    HKS_LOG_E("generate random success");
    return HKS_SUCCESS;
}

int32_t HksCryptoHalGetPubKey(const struct HksBlob *keyIn, struct HksBlob *keyOut)
{
    /* KeyMaterialRsa, KeyMaterialEcc, KeyMaterial25519's size are same */
    if (keyIn->size < sizeof(struct KeyMaterialRsa)) {
        return HKS_ERROR_INVALID_KEY_SIZE;
    }

    struct KeyMaterialRsa *key = (struct KeyMaterialRsa *)(keyIn->data);
    switch (key->keyAlg) {
        case HKS_ALG_ED25519:
            return HksOpensslGetEd25519PubKey(keyIn, keyOut);
        default:
            HKS_LOG_E("Unsupport key mode or macro is not on! mode = 0x%X", key->keyAlg);
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalGetMainKey(const struct HksBlob *message, struct HksBlob *mainKey)
{
    (void)message;
    return 0;
}

int32_t HksCryptoHalHmac(const struct HksBlob *key, uint32_t digestAlg, const struct HksBlob *msg,
    struct HksBlob *mac)
{
    return HksOpensslHmac(key, digestAlg, msg, mac);
}

int32_t HksCryptoHalHash(uint32_t alg, const struct HksBlob *msg, struct HksBlob *hash)
{
    return HksOpensslHash(alg, msg, hash);
}

static void BnFreeParams(struct HksBnExpModParams *bnParams)
{
    BN_free(bnParams->bnX);
    BN_free(bnParams->bnA);
    BN_free(bnParams->bnE);
    BN_free(bnParams->bnN);
    BN_CTX_free(bnParams->ctx);
}

static int32_t BnBuildParams(struct HksBnExpModParams *bnParams, const struct HksBlob *a,
    const struct HksBlob *e, const struct HksBlob *n)
{
    bnParams->ctx = BN_CTX_new();
    bnParams->bnX = BN_new();
    bnParams->bnA = BN_bin2bn(a->data, a->size, NULL);
    bnParams->bnE = BN_bin2bn(e->data, e->size, NULL);
    bnParams->bnN = BN_bin2bn(n->data, n->size, NULL);
    if ((bnParams->ctx == NULL) || (bnParams->bnX == NULL) || (bnParams->bnA == NULL) ||
        (bnParams->bnE == NULL) || (bnParams->bnN == NULL)) {
        BnFreeParams(bnParams);
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    return HKS_SUCCESS;
}

static int32_t BnExpModExport(BIGNUM *bnX, struct HksBlob *x)
{
    int32_t outLen = BN_num_bytes(bnX);
    if ((outLen < 0) || (x->size < (uint32_t)outLen)) {
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    (void)memset_s(x->data, x->size, 0, x->size);
    if (outLen == 0) {
        return HKS_SUCCESS;
    }

    uint8_t *bnOutput = (uint8_t *)HksMalloc(outLen);
    if (bnOutput == NULL) {
        HKS_LOG_E("malloc fail");
        return HKS_ERROR_MALLOC_FAIL;
    }

    int32_t ret = HKS_SUCCESS;
    do {
        int32_t realOutLen = BN_bn2bin(bnX, bnOutput);
        if (realOutLen != outLen) {
            HKS_LOG_E("BN_bn2bin fail");
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }

        uint32_t i = x->size - 1;
        int32_t j = realOutLen - 1; /* realOutLen is greater than 0; x->size is no less than realOutLen */
        for (; j >= 0; --i, --j) {  /* i is no less than j */
            x->data[i] = bnOutput[j];
        }
    } while (0);

    HksFree(bnOutput);
    return ret;
}

int32_t HksCryptoHalBnExpMod(struct HksBlob *x, const struct HksBlob *a,
    const struct HksBlob *e, const struct HksBlob *n)
{
    struct HksBnExpModParams bnParams;
    (void)memset_s(&bnParams, sizeof(bnParams), 0, sizeof(bnParams));
    int32_t ret = BnBuildParams(&bnParams, a, e, n);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("BnInitParams fail");
        return ret;
    }

    do {
        /* mod 0 is not supported */
        if (BN_is_zero(bnParams.bnN)) {
            HKS_LOG_E("not support mod 0 operation.");
            ret = HKS_ERROR_INVALID_ARGUMENT;
            break;
        }

        ret = BN_mod_exp(bnParams.bnX, bnParams.bnA, bnParams.bnE, bnParams.bnN, bnParams.ctx);
        if (ret != HKS_OPENSSL_SUCCESS) {
            HKS_LOG_E("BN_mod_exp fail, ret = %d", ret);
            ret = HKS_ERROR_CRYPTO_ENGINE_ERROR;
            break;
        }

        ret = BnExpModExport(bnParams.bnX, x);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("BnExpModExport fail");
            break;
        }
    } while (0);

    BnFreeParams(&bnParams);
    return ret;
}

int32_t HksCryptoHalGenerateKey(const struct HksKeySpec *spec, struct HksBlob *key)
{
    int32_t ret = GenKeyCheckParam(spec, key);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    HKS_LOG_I("generate key type %x", spec->algType);
    switch (spec->algType) {
        case HKS_ALG_ECC:
            return HksOpensslEccGenerateKey(spec, key);
        case HKS_ALG_AES:
            return HksOpensslAesGenerateKey(spec, key);
        case HKS_ALG_X25519:
        case HKS_ALG_ED25519:
            return HksOpensslCurve25519GenerateKey(spec, key);
        default:
            HKS_LOG_E("Unsupport algType now!");
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalAgreeKey(const struct HksBlob *nativeKey, const struct HksBlob *pubKey,
    const struct HksKeySpec *spec, struct HksBlob *sharedKey)
{
    int32_t ret = AgreeKeyCheckParam(nativeKey, pubKey, spec, sharedKey);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    switch (spec->algType) {
        case HKS_ALG_ECDH:
            return HksOpensslEcdhAgreeKey(nativeKey, pubKey, spec, sharedKey);
        case HKS_ALG_X25519:
            return HksOpensslX25519AgreeKey(nativeKey, pubKey, sharedKey);
        case HKS_ALG_ED25519:
            return HksOpensslEd25519AgreeKey(nativeKey, pubKey, sharedKey);
        default:
            HKS_LOG_E("Unsupport alg now!");
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalSign(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *signature)
{
    int32_t ret = SignVerifyCheckParam(key, usageSpec, message, signature);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    switch (usageSpec->algType) {
        case HKS_ALG_ECC:
            return HksOpensslEcdsaSign(key, usageSpec, message, signature);
        case HKS_ALG_ED25519:
            return HksOpensslEd25519Sign(key, message, signature);
        default:
            HKS_LOG_E("Unsupport alg now!");
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalVerify(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const struct HksBlob *signature)
{
    int32_t ret = SignVerifyCheckParam(key, usageSpec, message, signature);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    switch (usageSpec->algType) {
        case HKS_ALG_ECC:
            return HksOpensslEcdsaVerify(key, message, signature);
        case HKS_ALG_ED25519:
            return HksOpensslEd25519Verify(key, message, signature);
        default:
            HKS_LOG_E("Unsupport alg now!");
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalDeriveKey(const struct HksBlob *masterKey, const struct HksKeySpec *derivationSpec,
    struct HksBlob *derivedKey)
{
    int32_t ret = DeriveKeyCheckParam(masterKey, derivationSpec, derivedKey);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    switch (derivationSpec->algType) {
        case HKS_ALG_HKDF:
            return HksOpensslHkdf(masterKey, derivationSpec, derivedKey);
        case HKS_ALG_PBKDF2:
            return HksOpensslPbkdf2(masterKey, derivationSpec, derivedKey);
        default:
            HKS_LOG_E("Unsupport Derive Key alg!");
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalEncrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText, struct HksBlob *tagAead)
{
    int32_t ret = EncryptCheckParam(key, usageSpec, message, cipherText);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    switch (usageSpec->algType) {
        case HKS_ALG_AES:
            return HksOpensslAesEncrypt(key, usageSpec, message, cipherText, tagAead);
        default:
            HKS_LOG_E("Unsupport alg now!");
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}

int32_t HksCryptoHalDecrypt(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *cipherText)
{
    int32_t ret = DecryptCheckParam(key, usageSpec, message, cipherText);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Invalid params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    switch (usageSpec->algType) {
        case HKS_ALG_AES:
            return HksOpensslAesDecrypt(key, usageSpec, message, cipherText);
        default:
            HKS_LOG_E("Unsupport alg now!");
            return HKS_ERROR_INVALID_ARGUMENT;
    }
}
