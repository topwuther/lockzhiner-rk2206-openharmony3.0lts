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

#include "hks_client_service.h"

#include "hks_access.h"
#include "hks_client_check.h"
#include "hks_client_service_adapter.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_operation.h"
#include "hks_storage.h"
#ifdef _STORAGE_LITE_
#include "hks_storage_adapter.h"
#endif

#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA
#include "hks_upgrade_storage_data.h"
#endif

#define MAX_KEY_COUNT 256
#define MAX_STORAGE_SIZE (2 * 1024 * 1024)

#ifndef _CUT_AUTHENTICATE_
#ifdef _STORAGE_LITE_
static int32_t GetKeyData(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    struct HksBlob *key, int32_t mode)
{
    int32_t ret = HksStoreGetKeyBlob(processName, keyAlias, mode, key);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get key blob from storage failed, ret = %d", ret);
    }
    return ret;
}

static int32_t CheckKeyCondition(const struct HksBlob *processName, const struct HksBlob *keyAlias)
{
    /* check is enough buffer to store */
    uint32_t size = 0;
    int32_t ret = HksStoreGetToatalSize(&size);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get total size from storage failed, ret = %d", ret);
        return ret;
    }

    if (size >= MAX_STORAGE_SIZE) {
        /* is key exist */
        ret = HksStoreIsKeyBlobExist(processName, keyAlias, HKS_STORAGE_TYPE_KEY);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("buffer exceeds limit");
            return HKS_ERROR_STORAGE_FAILURE;
        }
    }

    return HKS_SUCCESS;
}

static int32_t GetKeyParamSet(const struct HksBlob *key, struct HksParamSet *paramSet)
{
    struct HksParamSet *tmpParamSet = NULL;
    int32_t ret = TranslateKeyInfoBlobToParamSet(NULL, key, &tmpParamSet);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    if (paramSet->paramSetSize < tmpParamSet->paramSetSize) {
        HksFreeParamSet(&tmpParamSet);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }
    if (memcpy_s(paramSet, paramSet->paramSetSize, tmpParamSet, tmpParamSet->paramSetSize) != EOK) {
        HKS_LOG_E("memcpy paramSet failed");
        ret = HKS_ERROR_BAD_STATE;
    }

    HksFreeParamSet(&tmpParamSet);
    return ret;
}

int32_t HksServiceGetKeyInfoList(const struct HksBlob *processName, struct HksKeyInfo *keyInfoList,
    uint32_t *listCount)
{
    int32_t ret = HksCheckGetKeyInfoListParams(processName, keyInfoList, listCount);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    return HksStoreGetKeyInfoList(keyInfoList, listCount);
}
#else
static int32_t GetKeyData(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    struct HksBlob *key, int32_t mode)
{
    uint32_t size;
    int32_t ret = HksStoreGetKeyBlobSize(processName, keyAlias, mode, &size);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get keyblob size from storage failed, ret = %d.", ret);
        return ret;
    }
    if (size > MAX_STORAGE_SIZE) {
        HKS_LOG_E("invalid storage size, size = %u", size);
        return HKS_ERROR_INVALID_KEY_FILE;
    }

    key->data = (uint8_t *)HksMalloc(size);
    if (key->data == NULL) {
        HKS_LOG_E("get key data: malloc failed");
        return HKS_ERROR_MALLOC_FAIL;
    }

    key->size = size;
    ret = HksStoreGetKeyBlob(processName, keyAlias, mode, key);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get keyblob from storage failed, ret = %d", ret);
        HKS_FREE_BLOB(*key);
    }

    return ret;
}

static int32_t CheckKeyCondition(const struct HksBlob *processName, const struct HksBlob *keyAlias)
{
    /* delete old key and certchain before obtaining the number of keys */
    int32_t ret = HksStoreDeleteKeyBlob(processName, keyAlias, HKS_STORAGE_TYPE_KEY);
    if ((ret != HKS_SUCCESS) && (ret != HKS_ERROR_NOT_EXIST)) {
        HKS_LOG_E("delete keyblob from storage failed, ret = %d", ret);
        return ret;
    }

    ret = HksStoreDeleteKeyBlob(processName, keyAlias, HKS_STORAGE_TYPE_CERTCHAIN);
    if ((ret != HKS_SUCCESS) && (ret != HKS_ERROR_NOT_EXIST)) {
        HKS_LOG_E("delete certchain from storage failed, ret = %d", ret);
        return ret;
    }

    uint32_t fileCount;
    ret = HksGetKeyCountByProcessName(processName, &fileCount);
    if (ret != HKS_SUCCESS) {
        return ret;
    }
    if (fileCount >= MAX_KEY_COUNT) {
        HKS_LOG_E("fileCount is no less than max count");
        ret = HKS_ERROR_STORAGE_FAILURE;
    }

    return ret;
}

static int32_t CheckBeforeDeleteParam(const struct HksParamSet *paramSet, uint32_t tag)
{
    int32_t ret = HksCheckParamSet(paramSet, paramSet->paramSetSize);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("check paramSet failed, ret = %d", ret);
        return ret;
    }

    if (paramSet->paramsCnt == 0) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    for (uint32_t i = 0; i < paramSet->paramsCnt; i++) {
        if (paramSet->params[i].tag == tag) {
            return HKS_SUCCESS;
        }
    }

    return HKS_ERROR_PARAM_NOT_EXIST;
}

static int32_t DeleteTagFromParamSet(uint32_t tag, const struct HksParamSet *paramSet,
    struct HksParamSet **outParamSet)
{
    int32_t ret = CheckBeforeDeleteParam(paramSet, tag);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("check failed before delete param, ret = %d", ret);
        return ret;
    }

    ret = HksFreshParamSet((struct HksParamSet *)paramSet, false);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("fresh paramset failed");
        return ret;
    }

    struct HksParamSet *newParamSet = NULL;
    ret = HksInitParamSet(&newParamSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("init param set failed");
        return ret;
    }

    for (uint32_t i = 0; i < paramSet->paramsCnt; ++i) {
        if (paramSet->params[i].tag != tag) {
            ret = HksAddParams(newParamSet, &paramSet->params[i], 1);
            if (ret != HKS_SUCCESS) {
                HKS_LOG_E("add in params failed");
                HKS_FREE_PTR(newParamSet);
                return ret;
            }
        }
    }

    ret = HksBuildParamSet(&newParamSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("build paramset failed");
        HksFreeParamSet(&newParamSet);
        return ret;
    }

    *outParamSet = newParamSet;
    return HKS_SUCCESS;
}

static int32_t GetKeyParamSet(const struct HksBlob *key, struct HksParamSet *paramSet)
{
    if (key->size < sizeof(struct HksParamSet)) {
        HKS_LOG_E("get key paramset: invalid key size: %u", key->size);
        return HKS_ERROR_INVALID_KEY_INFO;
    }

    const struct HksParamSet *tmpParamSet = (const struct HksParamSet *)key->data;
    struct HksParamSet *outParamSet = NULL;
    int32_t ret = DeleteTagFromParamSet(HKS_TAG_KEY, tmpParamSet, &outParamSet);
    if (ret == HKS_ERROR_PARAM_NOT_EXIST) {
        if (paramSet->paramSetSize < key->size) {
            return HKS_ERROR_BUFFER_TOO_SMALL;
        }
        if (memcpy_s(paramSet, paramSet->paramSetSize, key->data, key->size) != EOK) {
            HKS_LOG_E("memcpy key failed");
            return HKS_ERROR_BAD_STATE;
        }
        return HKS_SUCCESS;
    }

    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("delete tag from paramSet failed, ret = %d.", ret);
        return ret;
    }

    if (paramSet->paramSetSize < outParamSet->paramSetSize) {
        HksFreeParamSet(&outParamSet);
        return HKS_ERROR_BUFFER_TOO_SMALL;
    }
    if (memcpy_s(paramSet, paramSet->paramSetSize, outParamSet, outParamSet->paramSetSize) != EOK) {
        HKS_LOG_E("memcpy outParamSet failed");
        ret = HKS_ERROR_BAD_STATE;
    }

    HksFreeParamSet(&outParamSet);
    return ret;
}

int32_t HksServiceGetKeyInfoList(const struct HksBlob *processName, struct HksKeyInfo *keyInfoList,
    uint32_t *listCount)
{
    int32_t ret = HksCheckGetKeyInfoListParams(processName, keyInfoList, listCount);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    ret = HksGetKeyAliasByProcessName(processName, keyInfoList, listCount);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get key alias list from storage failed, ret = %d", ret);
        return ret;
    }

    for (uint32_t i = 0; i < *listCount; ++i) {
        struct HksBlob keyFromFile = { 0, NULL };
        ret = GetKeyData(processName, &(keyInfoList[i].alias), &keyFromFile, HKS_STORAGE_TYPE_KEY);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get key data failed, ret = %d", ret);
            return ret;
        }

        ret = GetKeyParamSet(&keyFromFile, keyInfoList[i].paramSet);
        HKS_FREE_BLOB(keyFromFile);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get key paramSet failed, ret = %d", ret);
            return ret;
        }
    }

    return ret;
}
#endif
#endif /* _CUT_AUTHENTICATE_ */

static int32_t AppendToNewParamSet(const struct HksParamSet *paramSet, struct HksParamSet **outParamSet)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;

    do {
        ret = HksCheckParamSet(paramSet, paramSet->paramSetSize);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check paramSet failed");
            break;
        }

        ret = HksFreshParamSet((struct HksParamSet *)paramSet, false);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append fresh paramset failed");
            break;
        }

        ret = HksInitParamSet(&newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append init operation param set failed");
            break;
        }

        ret = HksAddParams(newParamSet, paramSet->params, paramSet->paramsCnt);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append params failed");
            break;
        }

        *outParamSet = newParamSet;
        return ret;
    } while (0);

    HksFreeParamSet(&newParamSet);
    return ret;
}

static int32_t AppendProcessNameTag(const struct HksParamSet *paramSet, const struct HksBlob *processName,
    struct HksParamSet **outParamSet)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;

    do {
        if (paramSet != NULL) {
            ret = AppendToNewParamSet(paramSet, &newParamSet);
        } else {
            ret = HksInitParamSet(&newParamSet);
        }

        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append client service tag failed");
            break;
        }

        struct HksParam tmpParam;
        tmpParam.tag = HKS_TAG_PROCESS_NAME;
        tmpParam.blob = *processName;

        ret = HksAddParams(newParamSet, &tmpParam, 1);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("add param failed");
            break;
        }

        ret = HksBuildParamSet(&newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("build paramset failed");
            break;
        }

        *outParamSet = newParamSet;
        return ret;
    } while (0);

    HksFreeParamSet(&newParamSet);
    return ret;
}

#ifndef _CUT_AUTHENTICATE_
static int32_t GetKeyAndNewParamSet(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *key, struct HksParamSet **outParamSet)
{
    int32_t ret = AppendProcessNameTag(paramSet, processName, outParamSet);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("append tag processName failed, ret = %d", ret);
        return ret;
    }

    ret = GetKeyData(processName, keyAlias, key, HKS_STORAGE_TYPE_KEY);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get key data failed, ret = %d.", ret);
        HksFreeParamSet(outParamSet);
    }

    return ret;
}

static int32_t GetHksInnerKeyFormat(const struct HksParamSet *paramSet, const struct HksBlob *key,
    struct HksBlob *outKey)
{
    struct HksParam *algParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &algParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get alg param failed");
        return HKS_ERROR_CHECK_GET_ALG_FAIL;
    }

    switch (algParam->uint32Param) {
#ifdef HKS_SUPPORT_AES_C
        case HKS_ALG_AES:
            return TranslateToInnerAesFormat(key, outKey);
#endif
#if defined(HKS_SUPPORT_X25519_C) || defined(HKS_SUPPORT_ED25519_C)
        case HKS_ALG_ED25519:
        case HKS_ALG_X25519:
            return TranslateToInnerCurve25519Format(algParam->uint32Param, key, outKey);
#endif
#if defined(HKS_SUPPORT_RSA_C) || defined(HKS_SUPPORT_ECC_C)
        case HKS_ALG_RSA:
        case HKS_ALG_ECC:
        case HKS_ALG_ECDH:
            return TranslateFromX509PublicKey(key, outKey);
#endif
        default:
            return HKS_ERROR_INVALID_ALGORITHM;
    }
}

#ifdef HKS_SUPPORT_ED25519_TO_X25519
static int32_t GetAgreeStoreKey(uint32_t keyAliasTag, const struct HksBlob *processName,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
    struct HksParam *keyAliasParam = NULL;
    int32_t ret = HksGetParam(paramSet, keyAliasTag, &keyAliasParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get agree key alias tag failed");
        return ret;
    }

    if (keyAliasParam->blob.size > HKS_MAX_KEY_ALIAS_LEN) {
        HKS_LOG_E("invalid main key size: %u", keyAliasParam->blob.size);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    return GetKeyData(processName, &(keyAliasParam->blob), key, HKS_STORAGE_TYPE_KEY);
}

static int32_t GetAgreePublicKey(const uint32_t alg, const struct HksBlob *processName,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
    struct HksParam *isKeyAliasParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_AGREE_PUBLIC_KEY_IS_KEY_ALIAS, &isKeyAliasParam);
    if ((ret == HKS_SUCCESS) && (!(isKeyAliasParam->boolParam))) {
        struct HksParam *keyParam = NULL;
        ret = HksGetParam(paramSet, HKS_TAG_AGREE_PUBLIC_KEY, &keyParam);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get agree public key tag fail");
            return ret;
        }
        return TranslateToInnerCurve25519Format(alg, &(keyParam->blob), key);
    }

    return GetAgreeStoreKey(HKS_TAG_AGREE_PUBLIC_KEY, processName, paramSet, key);
}

static int32_t GetAgreePrivateKey(const struct HksBlob *processName,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
    return GetAgreeStoreKey(HKS_TAG_AGREE_PRIVATE_KEY_ALIAS, processName, paramSet, key);
}

static int32_t ConbineIntoKeyPair(const struct HksBlob *privateKey,
    const struct HksBlob *publicKey, struct HksBlob *keyPair)
{
    uint32_t size = sizeof(struct Hks25519KeyPair) + privateKey->size + publicKey->size; /* size has been checked */
    uint8_t *buffer = (uint8_t *)HksMalloc(size);
    if (buffer == NULL) {
        return HKS_ERROR_MALLOC_FAIL;
    }
    (void)memset_s(buffer, size, 0, size);

    struct Hks25519KeyPair keyPairStruct = { publicKey->size, privateKey->size };
    uint32_t offset = 0;
    int32_t ret = HKS_ERROR_BAD_STATE;
    do {
        if (memcpy_s(buffer + offset, size, &keyPairStruct, sizeof(keyPairStruct)) != EOK) {
            break;
        }
        offset += sizeof(keyPairStruct);

        if (memcpy_s(buffer  + offset, size - offset, publicKey->data, publicKey->size) != EOK) {
            break;
        }
        offset += publicKey->size;

        if (memcpy_s(buffer  + offset, size - offset, privateKey->data, privateKey->size) != EOK) {
            break;
        }

        keyPair->data = buffer;
        keyPair->size = size;
        return HKS_SUCCESS;
    } while (0);

    HKS_FREE_PTR(buffer);
    return ret;
}

static int32_t GetAgreeKeyPair(const uint32_t alg, const struct HksBlob *processName,
    const struct HksParamSet *paramSet, struct HksBlob *key)
{
    int32_t ret;
    struct HksBlob privateKey = { 0, NULL };
    struct HksBlob publicKey = { 0, NULL };
    do {
        ret = GetAgreePublicKey(alg, processName, paramSet, &publicKey);
        if (ret != HKS_SUCCESS) {
            break;
        }

        ret = GetAgreePrivateKey(processName, paramSet, &privateKey);
        if (ret != HKS_SUCCESS) {
            break;
        }

        ret = ConbineIntoKeyPair(&privateKey, &publicKey, key);
    } while (0);

    HKS_FREE_BLOB(privateKey);
    HKS_FREE_BLOB(publicKey);
    return ret;
}
#endif

static int32_t GetAgreeBaseKey(const struct HksBlob *processName, const struct HksParamSet *paramSet,
    struct HksBlob *key)
{
    (void)key;
    (void)processName;
    struct HksParam *keyAlgParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_ALGORITHM, &keyAlgParam);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("get alg tag fail");
        return HKS_ERROR_CHECK_GET_ALG_FAIL;
    }

    if (keyAlgParam->uint32Param != HKS_ALG_AES) {
        HKS_LOG_I("not an aes key, no need check main key and derive factor");
        return HKS_SUCCESS;
    }

#ifdef HKS_SUPPORT_ED25519_TO_X25519
    struct HksParam *agreeAlgParam = NULL;
    ret = HksGetParam(paramSet, HKS_TAG_AGREE_ALG, &agreeAlgParam);
    if (ret != HKS_SUCCESS) {
        return HKS_ERROR_CHECK_GET_ALG_FAIL;
    }

    if ((agreeAlgParam->uint32Param != HKS_ALG_X25519) && (agreeAlgParam->uint32Param != HKS_ALG_ED25519)) {
        return HKS_ERROR_INVALID_ALGORITHM;
    }

    return GetAgreeKeyPair(agreeAlgParam->uint32Param, processName, paramSet, key);
#else
    return HKS_ERROR_INVALID_ARGUMENT; /* if aes generated by agree but x25519/ed25519 is ot support, return error */
#endif
}

static int32_t GetDeriveMainKey(const struct HksBlob *processName, const struct HksParamSet *paramSet,
    struct HksBlob *key)
{
    struct HksParam *keyGenTypeParam = NULL;
    int32_t ret = HksGetParam(paramSet, HKS_TAG_KEY_GENERATE_TYPE, &keyGenTypeParam);
    if (ret != HKS_SUCCESS) {
        return HKS_SUCCESS; /* not set tag KEY_GENERATE_TYPE, gen key by default type */
    }

    if (keyGenTypeParam->uint32Param == HKS_KEY_GENERATE_TYPE_AGREE) {
        return GetAgreeBaseKey(processName, paramSet, key);
    } else if (keyGenTypeParam->uint32Param == HKS_KEY_GENERATE_TYPE_DEFAULT) {
        return HKS_SUCCESS;
    }
    return HKS_ERROR_INVALID_ARGUMENT;
}

static int32_t GetKeyIn(const struct HksBlob *processName, const struct HksParamSet *paramSet,
    struct HksBlob *key)
{
    int32_t ret = GetDeriveMainKey(processName, paramSet, key);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    /* if not generate by derive, init keyIn by default value(ca to ta not accept null pointer) */
    if (key->data == NULL) {
        key->size = 1; /* malloc least buffer as keyIn buffer */
        key->data = (uint8_t *)HksMalloc(key->size);
        if (key->data == NULL) {
            HKS_LOG_E("malloc failed");
            return HKS_ERROR_MALLOC_FAIL;
        }
        key->data[0] = 0;
    }
    return HKS_SUCCESS;
}

int32_t HksServiceGenerateKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSetIn, struct HksBlob *keyOut)
{
    struct HksParamSet *newParamSet = NULL;
    uint8_t *keyOutBuffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
    if (keyOutBuffer == NULL) {
        return HKS_ERROR_MALLOC_FAIL;
    }
    struct HksBlob output = { MAX_KEY_SIZE, keyOutBuffer };
    struct HksBlob keyIn = { 0, NULL };

    int32_t ret;
    do {
        /* if user don't pass the key out buffer, we will use a tmp key out buffer */
        if ((keyOut != NULL) && (keyOut->data != NULL) && (keyOut->size != 0)) {
            output = *keyOut;
        }

        ret = HksCheckGenAndImportKeyParams(processName, keyAlias, paramSetIn, &output);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check generate key params failed, ret = %d", ret);
            break;
        }

        ret = CheckKeyCondition(processName, keyAlias);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check key condition failed, ret = %d", ret);
            break;
        }

        ret = AppendProcessNameTag(paramSetIn, processName, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append processName tag failed, ret = %d", ret);
            break;
        }

        ret = GetKeyIn(processName, newParamSet, &keyIn);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get keyIn failed, ret = %d", ret);
            break;
        }

        ret = HksAccessGenerateKey(keyAlias, newParamSet, &keyIn, &output);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("access level generate key failed, ret = %d", ret);
            break;
        }

        ret = HksStoreKeyBlob(processName, keyAlias, HKS_STORAGE_TYPE_KEY, &output);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("store keyblob to storage failed, ret = %d", ret);
        }
    } while (0);

    HKS_FREE_PTR(keyOutBuffer);
    HKS_FREE_PTR(keyIn.data);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceSign(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *signature)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckAllParams(processName, keyAlias, paramSet, srcData, signature);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check sign params failed, ret = %d", ret);
            break;
        }

        ret = GetKeyAndNewParamSet(processName, keyAlias, paramSet, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("sign: get key and new paramSet failed, ret = %d", ret);
            break;
        }

        ret = HksAccessSign(&keyFromFile, newParamSet, srcData, signature);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceVerify(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, const struct HksBlob *signature)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckAllParams(processName, keyAlias, paramSet, srcData, signature);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check verify params failed, ret = %d", ret);
            break;
        }

        ret = GetKeyAndNewParamSet(processName, keyAlias, paramSet, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("verify: get key and new paramSet failed, ret = %d", ret);
            break;
        }

        ret = HksAccessVerify(&keyFromFile, newParamSet, srcData, signature);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceEncrypt(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *plainText, struct HksBlob *cipherText)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckAllParams(processName, keyAlias, paramSet, plainText, cipherText);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check encrypt failed, ret = %d", ret);
            break;
        }

        ret = GetKeyAndNewParamSet(processName, keyAlias, paramSet, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("encrypt: get key and new paramSet failed, ret = %d", ret);
            break;
        }

        ret = HksAccessEncrypt(&keyFromFile, newParamSet, plainText, cipherText);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceDecrypt(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *cipherText, struct HksBlob *plainText)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckAllParams(processName, keyAlias, paramSet, cipherText, plainText);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check decrypt failed, ret = %d", ret);
            break;
        }

        ret = GetKeyAndNewParamSet(processName, keyAlias, paramSet, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("decrypt: get key and new paramSet failed, ret = %d", ret);
            break;
        }

        ret = HksAccessDecrypt(&keyFromFile, newParamSet, cipherText, plainText);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceDeleteKey(const struct HksBlob *processName, const struct HksBlob *keyAlias)
{
    int32_t ret = HksCheckProcessNameAndKeyAlias(processName, keyAlias);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    /*
     * Detele key first, record log if failed; then delete cert chain, return error if failed;
     * Return error code of deleteKey in the end.
     */
    ret = HksStoreDeleteKeyBlob(processName, keyAlias, HKS_STORAGE_TYPE_KEY);
    if ((ret != HKS_SUCCESS) && (ret != HKS_ERROR_NOT_EXIST)) {
        HKS_LOG_E("service delete main key failed, ret = %d", ret);
    }

    int32_t deleteCertRet = HksStoreDeleteKeyBlob(processName, keyAlias, HKS_STORAGE_TYPE_CERTCHAIN);
    if ((deleteCertRet != HKS_SUCCESS) && (deleteCertRet != HKS_ERROR_NOT_EXIST)) {
        HKS_LOG_E("service delete cert chain failed, ret = %d", ret);
        return deleteCertRet;
    }

    return ret;
}

int32_t HksServiceKeyExist(const struct HksBlob *processName, const struct HksBlob *keyAlias)
{
    int32_t ret = HksCheckProcessNameAndKeyAlias(processName, keyAlias);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    return HksStoreIsKeyBlobExist(processName, keyAlias, HKS_STORAGE_TYPE_KEY);
}

int32_t HksServiceGetKeyParamSet(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    struct HksParamSet *paramSet)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckGetKeyParamSetParams(processName, keyAlias, paramSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check get key paramSet params failed, ret = %d", ret);
            break;
        }

        ret = GetKeyAndNewParamSet(processName, keyAlias, NULL, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get key paramSet: get key and new paramSet failed, ret = %d", ret);
            break;
        }

        ret = HksAccessCheckKeyValidity(newParamSet, &keyFromFile);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("access level check key validity failed, ret = %d", ret);
            break;
        }

        ret = GetKeyParamSet(&keyFromFile, paramSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get Key paramSet failed, ret = %d", ret);
        }
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceImportKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, const struct HksBlob *x509Key)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;

    do {
        ret = HksCheckGenAndImportKeyParams(processName, keyAlias, paramSet, x509Key);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check import key params failed, ret = %d", ret);
            break;
        }

        ret = CheckKeyCondition(processName, keyAlias);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("import: check key condition failed, ret = %d", ret);
            break;
        }

        ret = AppendProcessNameTag(paramSet, processName, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append processName tag failed, ret = %d", ret);
            break;
        }

        struct HksBlob publicKey = { 0, NULL };
        ret = GetHksInnerKeyFormat(newParamSet, x509Key, &publicKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get public key from x509 format failed, ret = %d", ret);
            break;
        }

        uint8_t *keyOutBuffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
        if (keyOutBuffer == NULL) {
            ret = HKS_ERROR_MALLOC_FAIL;
            (void)memset_s(publicKey.data, publicKey.size, 0, publicKey.size);
            HKS_FREE_BLOB(publicKey);
            break;
        }
        struct HksBlob keyOut = { MAX_KEY_SIZE, keyOutBuffer };

        ret = HksAccessImportKey(keyAlias, &publicKey, newParamSet, &keyOut);
        (void)memset_s(publicKey.data, publicKey.size, 0, publicKey.size);
        HKS_FREE_BLOB(publicKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("access level import public key failed, ret = %d", ret);
            HKS_FREE_PTR(keyOutBuffer);
            break;
        }

        ret = HksStoreKeyBlob(processName, keyAlias, HKS_STORAGE_TYPE_KEY, &keyOut);
        HKS_FREE_PTR(keyOutBuffer);
    } while (0);

    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceExportPublicKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    struct HksBlob *key)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckExportPublicKeyParams(processName, keyAlias, key);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check export public key params failed, ret = %d", ret);
            break;
        }

        ret = GetKeyAndNewParamSet(processName, keyAlias, NULL, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("export public: get key and new paramSet failed, ret = %d", ret);
            break;
        }

        uint8_t *buffer = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
        if (buffer == NULL) {
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }
        struct HksBlob publicKey = { MAX_KEY_SIZE, buffer };

        ret = HksAccessExportPublicKey(&keyFromFile, newParamSet, &publicKey);
        if (ret == HKS_SUCCESS) {
            struct HksBlob x509Key = { 0, NULL };
            ret = TranslateToX509PublicKey(&publicKey, &x509Key);
            if (ret != HKS_SUCCESS) {
                HKS_FREE_PTR(buffer);
                break;
            }

            if (memcpy_s(key->data, key->size, x509Key.data, x509Key.size) != EOK) {
                ret = HKS_ERROR_BAD_STATE;
                HKS_LOG_E("memcpy failed");
            } else {
                key->size = x509Key.size;
            }

            HKS_FREE_BLOB(x509Key);
        }
        HKS_FREE_PTR(buffer);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceAgreeKey(const struct HksBlob *processName, const struct HksParamSet *paramSet,
    const struct HksBlob *privateKey, const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckAllParams(processName, privateKey, paramSet, peerPublicKey, agreedKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check agree key params failed, ret = %d", ret);
            break;
        }

        ret = GetKeyAndNewParamSet(processName, privateKey, paramSet, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("agree: get key and new paramSet failed, ret = %d", ret);
            break;
        }

        struct HksBlob publicKey = { 0, NULL };
        ret = GetHksInnerKeyFormat(newParamSet, peerPublicKey, &publicKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("get public key from x509 format failed, ret = %d.", ret);
            break;
        }

        ret = HksAccessAgreeKey(newParamSet, &keyFromFile, &publicKey, agreedKey);
        HKS_FREE_BLOB(publicKey);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceDeriveKey(const struct HksBlob *processName, const struct HksParamSet *paramSet,
    const struct HksBlob *mainKey, struct HksBlob *derivedKey)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckDeriveKeyParams(processName, paramSet, mainKey, derivedKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check derive key params failed, ret = %d", ret);
            break;
        }

        ret = GetKeyAndNewParamSet(processName, mainKey, paramSet, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("derive: get key and new paramSet failed, ret = %d", ret);
            break;
        }

        ret = HksAccessDeriveKey(newParamSet, &keyFromFile, derivedKey);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceMac(const struct HksBlob *processName, const struct HksBlob *key,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *mac)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckAllParams(processName, key, paramSet, srcData, mac);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check mac params failed, ret = %d", ret);
            break;
        }

        ret = GetKeyAndNewParamSet(processName, key, paramSet, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("mac: get key and new paramSet failed, ret = %d", ret);
            break;
        }

        ret = HksAccessMac(&keyFromFile, newParamSet, srcData, mac);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceInitialize(void)
{
    int32_t ret = HKS_SUCCESS;

#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA
    ret = HksUpgradeStorageData();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("hks update storage data failed, ret = %d", ret);
        return ret;
    }
#endif

#ifndef _HARDWARE_ROOT_KEY_
    ret = HksAccessInitialize();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("hks core service initialize failed! ret = %d", ret);
        return ret;
    }
#endif

#ifdef _STORAGE_LITE_
    ret = HksLoadFileToBuffer();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("load file to buffer failed, ret = %d", ret);
        return ret;
    }
#endif
    return ret;
}

int32_t HksServiceRefreshKeyInfo(const struct HksBlob *processName)
{
    int32_t ret = HksStoreDestory(processName);
    HKS_LOG_I("destroy storage files ret = 0x%X", ret); /* only recode log */

#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA
    ret = HksDestroyOldVersionFiles();
    HKS_LOG_I("destroy old version files ret = 0x%X", ret); /* only recode log */
#endif

#ifndef _HARDWARE_ROOT_KEY_
    ret = HksAccessRefresh();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Hks core service refresh info failed! ret = 0x%X", ret);
        return ret;
    }
#endif

#ifdef _STORAGE_LITE_
    ret = HksFileBufferRefresh();
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("load file to buffer failed, ret = %d", ret);
        return ret;
    }
#endif
    return ret;
}

int32_t HksServiceProcessInit(uint32_t msgId, const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, uint64_t *operationHandle)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;
    struct HksBlob keyFromFile = { 0, NULL };

    do {
        ret = HksCheckInitParams(processName, keyAlias, paramSet, operationHandle);
        if (ret != HKS_SUCCESS) {
            break;
        }

        ret = GetKeyAndNewParamSet(processName, keyAlias, paramSet, &keyFromFile, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("GetKeyAndNewParamSet, ret = %d", ret);
            break;
        }

        ret = HksAccessProcessInit(msgId, &keyFromFile, newParamSet, operationHandle);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksAccessProcessInit fail");
            break;
        }

        ret = CreateOperation(processName, *operationHandle, true);
    } while (0);

    HKS_FREE_BLOB(keyFromFile);
    HksFreeParamSet(&newParamSet);
    return ret;
}

int32_t HksServiceProcessUpdate(uint32_t msgId, uint64_t operationHandle, const struct HksBlob *inData,
    struct HksBlob *outData)
{
    int32_t ret;

    do {
        ret = CheckBlob(inData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("CheckBlob failed");
            break;
        }

        if (QueryOperation(operationHandle) == NULL) {
            HKS_LOG_E("operationHandle is not exist");
            ret = HKS_ERROR_REQUEST_OVERFLOWS;
            break;
        }

        ret = HksAccessProcessMultiUpdate(msgId, operationHandle, inData, outData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksAccessProcessMultiUpdate fail");
            DeleteOperation(operationHandle);
        }
    } while (0);

    return ret;
}

int32_t HksServiceProcessFinal(uint32_t msgId, uint64_t operationHandle, const struct HksBlob *inData,
    struct HksBlob *outData)
{
    int32_t ret;

    do {
        ret = CheckBlob(inData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("CheckBlob inData failed");
            break;
        }

        ret = CheckBlob(outData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("CheckBlob outData failed");
            break;
        }

        if (QueryOperation(operationHandle) == NULL) {
            HKS_LOG_E("operationHandle is not exist");
            ret = HKS_ERROR_REQUEST_OVERFLOWS;
            break;
        }

        ret = HksAccessProcessFinal(msgId, operationHandle, inData, outData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksAccessProcessFinal fail");
        }

        DeleteOperation(operationHandle);
    } while (0);

    return ret;
}

int32_t HksServiceSignWithDeviceKey(const struct HksBlob *processName, uint32_t keyId,
    const struct HksParamSet *paramSet, const struct HksBlob *srcData, struct HksBlob *signature)
{
    return 0;
}

int32_t HksServiceAttestKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *certChain)
{
    return 0;
}

int32_t HksServiceGetCertificateChain(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksParamSet *paramSet, struct HksBlob *certChain)
{
    return 0;
}

int32_t HksServiceWrapKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksBlob *targetKeyAlias, const struct HksParamSet *paramSet, struct HksBlob *wrappedData)
{
    return 0;
}

int32_t HksServiceUnwrapKey(const struct HksBlob *processName, const struct HksBlob *keyAlias,
    const struct HksBlob *targetKeyAlias, const struct HksBlob *wrappedData, const struct HksParamSet *paramSet)
{
    return 0;
}

int32_t HksServiceProvision(const struct HksBlob *srcData, const struct HksBlob *challengeIn)
{
    return 0;
}

int32_t HksServiceProvisionVerify(const struct HksBlob *srcData, const struct HksBlob *challengeIn)
{
    return 0;
}

int32_t HksServiceExportTrustCerts(const struct HksBlob *processName, struct HksBlob *certChain)
{
    return 0;
}
#endif

int32_t HksServiceGenerateRandom(const struct HksBlob *processName, struct HksBlob *random)
{
    int32_t ret;
    struct HksParamSet *newParamSet = NULL;

    do {
        ret = HksCheckGenerateRandomParams(processName, random);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("check generate random params failed, ret = %d", ret);
            break;
        }

        ret = AppendProcessNameTag(NULL, processName, &newParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("append processName tag failed, ret = %d", ret);
            break;
        }

        ret = HksAccessGenerateRandom(newParamSet, random);
    } while (0);

    HksFreeParamSet(&newParamSet);
    return ret;
}

