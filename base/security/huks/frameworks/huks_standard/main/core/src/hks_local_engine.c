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

#include "hks_local_engine.h"

#include "hks_check_paramset.h"
#include "hks_cmd_id.h"
#include "hks_common_check.h"
#include "hks_crypto_adapter.h"
#include "hks_crypto_hal.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_param.h"

#define MAX_DEGIST_SIZE 64
#define X25519_KEY_BYTE_SIZE 32

#ifndef _CUT_AUTHENTICATE_
int32_t HksLocalHash(const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *hash)
{
    if (HksCheckBlob2AndParamSet(srcData, hash, paramSet) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksParam *digestAlg = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_DIGEST, &digestAlg);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get param digest failed");
        return HKS_ERROR_CHECK_GET_DIGEST_FAIL;
    }

    uint32_t digestLen;
    ret = HksGetDigestLen(digestAlg->uint32Param, &digestLen);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get digest len failed, ret = %d", ret);
        return ret;
    }

    if (hash->size < digestLen) {
        HKS_LOG_E("hash len too small, size = %u", hash->size);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }

    return HksCryptoHalHash(digestAlg->uint32Param, srcData, hash);
}
#endif

int32_t HksLocalMac(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *mac)
{
    if (HksCheckBlob3AndParamSet(key, srcData, mac, paramSet) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    int32_t ret = HksCoreCheckMacParams(key, paramSet, srcData, mac, true);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    struct HksParam *digestAlg = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_DIGEST, &digestAlg);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get param digest failed");
        return HKS_ERROR_CHECK_GET_DIGEST_FAIL;
    }

    return HksCryptoHalHmac(key, digestAlg->uint32Param, srcData, mac);
}

int32_t HksLocalBnExpMod(struct HksBlob *x, const struct HksBlob *a, const struct HksBlob *e, const struct HksBlob *n)
{
    int32_t ret = HksCheckBlob4(x, a, e, n);
    if (ret != HKS_SUCCESS) {
        return ret;
    }
    return HksCryptoHalBnExpMod(x, a, e, n);
}

#ifndef _CUT_AUTHENTICATE_
static int32_t CheckLocalGenerateKeyParams(const struct HksParamSet *paramSetIn, struct HksParamSet *paramSetOut)
{
    if ((HksCheckParamSetValidity(paramSetIn) != HKS_SUCCESS) || (paramSetOut == NULL)) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSetIn, HKS_TAG_ALGORITHM, &algParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get param alg failed");
        return HKS_ERROR_CHECK_GET_ALG_FAIL;
    }

    if (algParam->uint32Param != HKS_ALG_X25519) {
        HKS_LOG_E("invalid alg: %u", algParam->uint32Param);
        return HKS_ERROR_INVALID_ALGORITHM;
    }

    struct HksParam *keySizeParam = NULL;
    ret = HksGetParam(paramSetIn, HKS_TAG_KEY_SIZE, &keySizeParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get param key size failed");
        return HKS_ERROR_CHECK_GET_KEY_SIZE_FAIL;
    }

    if (keySizeParam->uint32Param != HKS_CURVE25519_KEY_SIZE_256) {
        HKS_LOG_E("invalid key size: %u", keySizeParam->uint32Param);
        return HKS_ERROR_INVALID_KEY_SIZE;
    }
    return HKS_SUCCESS;
}

int32_t HksLocalGenerateKey(const struct HksParamSet *paramSetIn, struct HksParamSet *paramSetOut)
{
    int32_t ret = CheckLocalGenerateKeyParams(paramSetIn, paramSetOut);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    struct HksKeySpec spec = {0};
    HksFillKeySpec(paramSetIn, &spec);
    struct HksBlob key = { 0, NULL };

    ret = HksCryptoHalGenerateKey(&spec, &key);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("local engine generate key failed, ret:%x!", ret);
        return ret;
    }

    ret = HksFormatKeyFromMaterial(spec.algType, &key, paramSetOut);
    (void)memset_s(key.data, key.size, 0, key.size);
    HKS_FREE_PTR(key.data);
    return ret;
}

int32_t HksLocalAgreeKey(const struct HksParamSet *paramSet, const struct HksBlob *privateKey,
    const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey)
{
    if (HksCheckBlob3AndParamSet(privateKey, peerPublicKey, agreedKey, paramSet) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    int32_t ret = HksCoreCheckAgreeKeyParams(paramSet, privateKey, peerPublicKey, agreedKey, true);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    struct HksKeySpec spec = {0};
    HksFillKeySpec(paramSet, &spec);

    struct HksBlob privateKeyMaterial = { 0, NULL };
    ret = HksSetKeyToMaterial(spec.algType, false, privateKey, &privateKeyMaterial);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("set prikey to materail failed, ret:%x!", ret);
        return ret;
    }

    struct HksBlob publicKeyMaterial = { 0, NULL };
    ret = HksSetKeyToMaterial(spec.algType, true, peerPublicKey, &publicKeyMaterial);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("set pubkey to materail failed, ret:%x!", ret);
        (void)memset_s(privateKeyMaterial.data, privateKeyMaterial.size, 0, privateKeyMaterial.size);
        HKS_FREE_PTR(privateKeyMaterial.data);
        return ret;
    }

    ret = HksCryptoHalAgreeKey(&privateKeyMaterial, &publicKeyMaterial, &spec, agreedKey);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("local engine agree key failed, ret:%x!", ret);
    }

    (void)memset_s(privateKeyMaterial.data, privateKeyMaterial.size, 0, privateKeyMaterial.size);
    HKS_FREE_PTR(privateKeyMaterial.data);
    (void)memset_s(publicKeyMaterial.data, publicKeyMaterial.size, 0, publicKeyMaterial.size);
    HKS_FREE_PTR(publicKeyMaterial.data);
    return ret;
}
#endif

static int32_t CipherEncrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksUsageSpec *usageSpec, const struct HksBlob *inData, struct HksBlob *outData)
{
    struct HksBlob tag = { 0, NULL };
    int32_t ret = HksGetEncryptAeTag(paramSet, inData, outData, &tag);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("cipher encrypt get ae tag failed!");
        return ret;
    }

    ret = HksCryptoHalEncrypt(key, usageSpec, inData, outData, &tag);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("cipher encrypt failed!");
        return ret;
    }

    outData->size += tag.size;
    return HKS_SUCCESS;
}

static int32_t CheckLocalCipherParams(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *inputText, struct HksBlob *outputText)
{
    if (HksCheckBlob3AndParamSet(key, inputText, outputText, paramSet) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HksLocalCheckCipherParams(cmdId, key, paramSet, inputText, outputText);
}

static int32_t EncryptAndDecrypt(uint32_t cmdId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *inputText, struct HksBlob *outputText)
{
    int32_t ret = CheckLocalCipherParams(cmdId, key, paramSet, inputText, outputText);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    struct HksUsageSpec *usageSpec = NULL;
    bool isEncrypt = (cmdId == HKS_CMD_ID_ENCRYPT);
    struct HksBlob tmpInputText = { inputText->size, inputText->data };
    ret = HksBuildCipherUsageSpec(paramSet, isEncrypt, &tmpInputText, &usageSpec);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("build cipher usagespec failed, ret:%x!", ret);
        return ret;
    }

    if (cmdId == HKS_CMD_ID_ENCRYPT) {
        ret = CipherEncrypt(key, paramSet, usageSpec, &tmpInputText, outputText);
    } else {
        ret = HksCryptoHalDecrypt(key, usageSpec, &tmpInputText, outputText);
    }
    HksFreeUsageSpec(&usageSpec);

    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("local engine EncryptDecrypt cmdId = %u failed, ret:%x!", cmdId, ret);
    }
    return ret;
}

int32_t HksLocalEncrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *plainText, struct HksBlob *cipherText)
{
    return EncryptAndDecrypt(HKS_CMD_ID_ENCRYPT, key, paramSet, plainText, cipherText);
}

int32_t HksLocalDecrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *cipherText, struct HksBlob *plainText)
{
    return EncryptAndDecrypt(HKS_CMD_ID_DECRYPT, key, paramSet, cipherText, plainText);
}

int32_t HksLocalDeriveKey(const struct HksParamSet *paramSet, const struct HksBlob *mainKey,
    struct HksBlob *derivedKey)
{
    if (HksCheckBlob2AndParamSet(mainKey, derivedKey, paramSet) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    int32_t ret = HksCoreCheckDeriveKeyParams(paramSet, mainKey, derivedKey, true);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    struct HksKeyDerivationParam derivationParam = { { 0, NULL }, { 0, NULL }, 0, 0 };
    struct HksKeySpec derivationSpec = { 0, 0, &derivationParam };

    HksFillKeySpec(paramSet, &derivationSpec);
    HksFillKeyDerivationParam(paramSet, &derivationParam);

    return HksCryptoHalDeriveKey(mainKey, &derivationSpec, derivedKey);
}

#ifndef _CUT_AUTHENTICATE_
static int32_t CheckLocalVerifyParams(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *signature)
{
    if (HksCheckBlob3AndParamSet(key, srcData, signature, paramSet) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    if (key->size != (HKS_CURVE25519_KEY_SIZE_256 / HKS_BITS_PER_BYTE)) {
        HKS_LOG_E("invalid key size: %u", key->size);
        return HKS_ERROR_INVALID_KEY_SIZE;
    }

    if (signature->size < HKS_SIGNATURE_MIN_SIZE) {
        HKS_LOG_E("signature size too small, size: %u", signature->size);
        return HKS_ERROR_INVALID_SIGNATURE_SIZE;
    }

    struct HksParam *purposeParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_PURPOSE, &purposeParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get param purpose failed");
        return HKS_ERROR_CHECK_GET_PURPOSE_FAIL;
    }
    if ((purposeParam->uint32Param & HKS_KEY_PURPOSE_VERIFY) == 0) {
        HKS_LOG_E("invalid purpose: 0x%x", purposeParam->uint32Param);
        return HKS_ERROR_INVALID_PURPOSE;
    }

    return HKS_SUCCESS;
}

int32_t HksLocalVerify(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *signature)
{
    int32_t ret = CheckLocalVerifyParams(key, paramSet, srcData, signature);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    struct HksUsageSpec usageSpec = {0};
    HksFillUsageSpec(paramSet, &usageSpec);

    struct HksBlob keyMaterial = { 0, NULL };
    ret = HksSetKeyToMaterial(usageSpec.algType, true, key, &keyMaterial);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("set key to material failed, ret:%x!", ret);
        return ret;
    }

    /* Only ed25519 is allowed here */
    ret = HksCryptoHalVerify(&keyMaterial, &usageSpec, srcData, signature);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("local engine verify failed, ret:%x!", ret);
    }

    (void)memset_s(keyMaterial.data, keyMaterial.size, 0, keyMaterial.size);
    HKS_FREE_PTR(keyMaterial.data);
    return ret;
}
#endif