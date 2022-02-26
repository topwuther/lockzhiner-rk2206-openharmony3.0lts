/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "huks_adapter.h"
#include "common_util.h"
#include "hc_error.h"
#include "hc_file.h"
#include "hc_log.h"
#include "hks_api.h"
#include "hks_param.h"
#include "hks_type.h"

static int32_t BaseCheckParams(const Uint8Buff **inParams, const char **paramTags, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        CHECK_PTR_RETURN_HAL_ERROR_CODE(inParams[i], paramTags[i]);
        CHECK_PTR_RETURN_HAL_ERROR_CODE(inParams[i]->val, paramTags[i]);
        CHECK_LEN_ZERO_RETURN_ERROR_CODE(inParams[i]->length, paramTags[i]);
    }
    return HAL_SUCCESS;
}

static int32_t ConstructParamSet(struct HksParamSet **out, const struct HksParam *inParam,
    const uint32_t inParamNum)
{
    struct HksParamSet *paramSet = NULL;
    int32_t ret = HksInitParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("init param set failed, ret = %d", ret);
        return HAL_ERR_INIT_PARAM_SET_FAILED;
    }

    ret = HksAddParams(paramSet, inParam, inParamNum);
    if (ret != HKS_SUCCESS) {
        LOGE("add param failed, ret = %d", ret);
        HksFreeParamSet(&paramSet);
        return HAL_ERR_ADD_PARAM_FAILED;
    }

    ret = HksBuildParamSet(&paramSet);
    if (ret != HKS_SUCCESS) {
        LOGE("build param set failed, ret = %d", ret);
        HksFreeParamSet(&paramSet);
        return HAL_ERR_BUILD_PARAM_SET_FAILED;
    }

    *out = paramSet;
    return HAL_SUCCESS;
}

static int32_t InitHks()
{
    int32_t res = HksInitialize();
    if (res == HKS_SUCCESS) {
        return HAL_SUCCESS;
    }

    if ((res != HKS_ERROR_INVALID_KEY_FILE) && (res != HKS_ERROR_CRYPTO_ENGINE_ERROR) &&
        (res != HKS_ERROR_UPDATE_ROOT_KEY_MATERIAL_FAIL)) {
        LOGE("Hks: Init hks failed, res: %d.", res);
        return HAL_ERR_INIT_FAILED;
    }

    LOGD("Hks: The local hks file needs to be refreshed!");
    LOGI("Start to delete local database file!");
    HcFileRemove(FILE_ID_GROUP);
    LOGI("Delete local database file successfully!");
    res = HksRefreshKeyInfo();
    if (res != HKS_SUCCESS) {
        LOGE("Hks: HksRefreshKeyInfo failed, res: %d.", res);
        return HAL_ERR_INIT_FAILED;
    }
    res = HksInitialize();
    if (res != HKS_SUCCESS) {
        LOGE("Hks: Init hks failed, res: %d.", res);
        return HAL_ERR_INIT_FAILED;
    }

    return HAL_SUCCESS;
}

static int32_t Sha256(const Uint8Buff *message, Uint8Buff *hash)
{
    CHECK_PTR_RETURN_HAL_ERROR_CODE(message, "message");
    CHECK_PTR_RETURN_HAL_ERROR_CODE(message->val, "message->val");
    CHECK_LEN_ZERO_RETURN_ERROR_CODE(message->length, "message->length");

    CHECK_PTR_RETURN_HAL_ERROR_CODE(hash, "hash");
    CHECK_PTR_RETURN_HAL_ERROR_CODE(hash->val, "hash->val");
    CHECK_LEN_EQUAL_RETURN(hash->length, SHA256_LEN, "hash->length");

    struct HksBlob srcBlob = { message->length, message->val };
    struct HksBlob hashBlob = { hash->length, hash->val };
    struct HksParamSet *paramSet = NULL;
    struct HksParam digestParam[] = {
        {
            .tag = HKS_TAG_DIGEST,
            .uint32Param = HKS_DIGEST_SHA256
        }
    };
    int32_t ret = ConstructParamSet(&paramSet, digestParam, CAL_ARRAY_SIZE(digestParam));
    if (ret != HAL_SUCCESS) {
        LOGE("construct param set failed, ret = %d", ret);
        return ret;
    }

    ret = HksHash(paramSet, &srcBlob, &hashBlob);
    if (ret != HKS_SUCCESS || hashBlob.size != SHA256_LEN) {
        HksFreeParamSet(&paramSet);
        return HAL_FAILED;
    }

    HksFreeParamSet(&paramSet);
    return HAL_SUCCESS;
}

static int32_t GenerateRandom(Uint8Buff *rand)
{
    CHECK_PTR_RETURN_HAL_ERROR_CODE(rand, "rand");
    CHECK_PTR_RETURN_HAL_ERROR_CODE(rand->val, "rand->val");
    CHECK_LEN_ZERO_RETURN_ERROR_CODE(rand->length, "rand->length");

    struct HksBlob randBlob = { rand->length, rand->val };
    int32_t ret = HksGenerateRandom(NULL, &randBlob);
    if (ret != HKS_SUCCESS) {
        LOGE("Generate random failed, ret: %d", ret);
        return HAL_FAILED;
    }

    return HAL_SUCCESS;
}

static int32_t CheckKeyExist(const Uint8Buff *keyAlias)
{
    CHECK_PTR_RETURN_HAL_ERROR_CODE(keyAlias, "keyAlias");
    CHECK_PTR_RETURN_HAL_ERROR_CODE(keyAlias->val, "keyAlias->val");
    CHECK_LEN_ZERO_RETURN_ERROR_CODE(keyAlias->length, "keyAlias->length");

    struct HksBlob keyAliasBlob = { keyAlias->length, keyAlias->val };
    int32_t ret = HksKeyExist(&keyAliasBlob, NULL);
    if (ret != HKS_SUCCESS) {
        LOGE("Check key exist failed, ret = %d", ret);
        return HAL_FAILED;
    }

    return HAL_SUCCESS;
}

static int32_t DeleteKey(const Uint8Buff *keyAlias)
{
    CHECK_PTR_RETURN_HAL_ERROR_CODE(keyAlias, "keyAlias");
    CHECK_PTR_RETURN_HAL_ERROR_CODE(keyAlias->val, "keyAlias->val");
    CHECK_LEN_ZERO_RETURN_ERROR_CODE(keyAlias->length, "keyAlias->length");

    struct HksBlob keyAliasBlob = { keyAlias->length, keyAlias->val };
    int32_t ret = HksDeleteKey(&keyAliasBlob, NULL);
    if (ret == HKS_ERROR_NOT_EXIST) {
        LOGI("Key not exists.");
        return HAL_SUCCESS;
    }
    if (ret != HKS_SUCCESS) {
        LOGE("Delete key failed, ret = %d", ret);
        return HAL_FAILED;
    }

    return HAL_SUCCESS;
}

static int32_t ComputeHmac(const Uint8Buff *key, const Uint8Buff *message, Uint8Buff *outHmac, bool isAlias)
{
    const Uint8Buff *inParams[] = { key, message, outHmac };
    const char *paramTags[] = {"key", "message", "outHmac"};
    int32_t ret = BaseCheckParams(inParams, paramTags, CAL_ARRAY_SIZE(inParams));
    if (ret != HAL_SUCCESS) {
        return ret;
    }
    CHECK_LEN_EQUAL_RETURN(outHmac->length, HMAC_LEN, "outHmac->length");

    struct HksBlob keyBlob = { key->length, key->val };
    struct HksBlob srcBlob = { message->length, message->val };
    struct HksBlob hmacBlob = { outHmac->length, outHmac->val };
    struct HksParamSet *paramSet = NULL;
    struct HksParam hmacParam[] = {
        {
            .tag = HKS_TAG_PURPOSE,
            .uint32Param = HKS_KEY_PURPOSE_MAC
        }, {
            .tag = HKS_TAG_DIGEST,
            .uint32Param = HKS_DIGEST_SHA256
        }, {
            .tag = HKS_TAG_IS_KEY_ALIAS,
            .boolParam = isAlias
        }
    };
    ret = ConstructParamSet(&paramSet, hmacParam, CAL_ARRAY_SIZE(hmacParam));
    if (ret != HAL_SUCCESS) {
        LOGE("construct param set failed, ret = %d", ret);
        return ret;
    }

    ret = HksMac(&keyBlob, paramSet, &srcBlob, &hmacBlob);
    if (ret != HKS_SUCCESS  || hmacBlob.size != HMAC_LEN) {
        LOGE("Hmac failed, ret: %d", ret);
        HksFreeParamSet(&paramSet);
        return HAL_FAILED;
    }

    HksFreeParamSet(&paramSet);
    return HAL_SUCCESS;
}

static int32_t ComputeHkdf(const Uint8Buff *baseKey, const Uint8Buff *salt, const Uint8Buff *keyInfo,
    Uint8Buff *outHkdf, bool isAlias)
{
    const Uint8Buff *inParams[] = { baseKey, salt, outHkdf };
    const char *paramTags[] = { "baseKey", "salt", "outHkdf" };
    int32_t ret = BaseCheckParams(inParams, paramTags, CAL_ARRAY_SIZE(inParams));
    if (ret != HAL_SUCCESS) {
        return ret;
    }

    struct HksBlob srcKeyBlob = { baseKey->length, baseKey->val };
    struct HksBlob saltBlob = { salt->length, salt->val };
    struct HksBlob keyInfoBlob = { 0, NULL };
    if (keyInfo != NULL) {
        keyInfoBlob.size = keyInfo->length;
        keyInfoBlob.data = keyInfo->val;
    }
    struct HksBlob derivedKeyBlob = { outHkdf->length, outHkdf->val };

    struct HksParamSet *paramSet = NULL;
    struct HksParam hkdfParam[] = {
        {
            .tag = HKS_TAG_PURPOSE,
            .uint32Param = HKS_KEY_PURPOSE_DERIVE
        }, {
            .tag = HKS_TAG_ALGORITHM,
            .uint32Param = HKS_ALG_HKDF
        }, {
            .tag = HKS_TAG_DIGEST,
            .uint32Param = HKS_DIGEST_SHA256
        }, {
            .tag = HKS_TAG_SALT,
            .blob = saltBlob
        }, {
            .tag = HKS_TAG_INFO,
            .blob = keyInfoBlob
        }, {
            .tag = HKS_TAG_IS_KEY_ALIAS,
            .boolParam = isAlias
        }
    };

    ret = ConstructParamSet(&paramSet, hkdfParam, CAL_ARRAY_SIZE(hkdfParam));
    if (ret != HAL_SUCCESS) {
        return ret;
    }

    ret = HksDeriveKey(paramSet, &srcKeyBlob, &derivedKeyBlob);
    if (ret != HKS_SUCCESS) {
        LOGE("Key derivation failed, ret: %d", ret);
        HksFreeParamSet(&paramSet);
        return HAL_FAILED;
    }

    HksFreeParamSet(&paramSet);
    return HAL_SUCCESS;
}

static int32_t CheckAesGcmEncryptParam(const Uint8Buff *key, const Uint8Buff *plain, const GcmParam *encryptInfo,
    Uint8Buff *outCipher)
{
    const Uint8Buff *inParams[] = { key, plain, outCipher };
    const char* paramTags[] = { "key", "plain", "outCipher" };
    int32_t ret = BaseCheckParams(inParams, paramTags, CAL_ARRAY_SIZE(inParams));
    if (ret != HAL_SUCCESS) {
        return ret;
    }

    CHECK_PTR_RETURN_HAL_ERROR_CODE(encryptInfo, "encryptInfo");
    CHECK_PTR_RETURN_HAL_ERROR_CODE(encryptInfo->aad, "aad");
    CHECK_LEN_ZERO_RETURN_ERROR_CODE(encryptInfo->aadLen, "aadLen");
    CHECK_PTR_RETURN_HAL_ERROR_CODE(encryptInfo->nonce, "nonce");
    CHECK_LEN_LOWER_RETURN(encryptInfo->nonceLen, HKS_AE_NONCE_LEN, "nonceLen");
    CHECK_LEN_LOWER_RETURN(outCipher->length, plain->length + HKS_AE_TAG_LEN, "outCipher");

    return HAL_SUCCESS;
}

static int32_t AesGcmEncrypt(const Uint8Buff *key, const Uint8Buff *plain,
    const GcmParam *encryptInfo, bool isAlias, Uint8Buff *outCipher)
{
    int32_t ret = CheckAesGcmEncryptParam(key, plain, encryptInfo, outCipher);
    if (ret != HAL_SUCCESS) {
        return ret;
    }

    struct HksBlob keyBlob = { key->length, key->val };
    struct HksBlob plainBlob = { plain->length, plain->val };
    struct HksBlob cipherBlob = { outCipher->length, outCipher->val };

    struct HksParamSet *paramSet = NULL;
    struct HksParam encryptParam[] = {
        {
            .tag = HKS_TAG_PURPOSE,
            .uint32Param = HKS_KEY_PURPOSE_ENCRYPT
        }, {
            .tag = HKS_TAG_ALGORITHM,
            .uint32Param = HKS_ALG_AES
        }, {
            .tag = HKS_TAG_BLOCK_MODE,
            .uint32Param = HKS_MODE_GCM
        }, {
            .tag = HKS_TAG_PADDING,
            .uint32Param = HKS_PADDING_NONE
        }, {
            .tag = HKS_TAG_NONCE,
            .blob = { encryptInfo->nonceLen, encryptInfo->nonce }
        }, {
            .tag = HKS_TAG_ASSOCIATED_DATA,
            .blob = { encryptInfo->aadLen, encryptInfo->aad }
        }, {
            .tag = HKS_TAG_IS_KEY_ALIAS,
            .boolParam = isAlias
        }
    };

    ret = ConstructParamSet(&paramSet, encryptParam, CAL_ARRAY_SIZE(encryptParam));
    if (ret != HAL_SUCCESS) {
        LOGE("construct param set failed, ret = %d", ret);
        return ret;
    }

    ret = HksEncrypt(&keyBlob, paramSet, &plainBlob, &cipherBlob);
    if (ret != HKS_SUCCESS) {
        LOGE("Aes-gcm encrypt failed, ret: %d", ret);
        HksFreeParamSet(&paramSet);
        return HAL_FAILED;
    }

    HksFreeParamSet(&paramSet);
    return HAL_SUCCESS;
}

static int32_t CheckAesGcmDecryptParam(const Uint8Buff *key, const Uint8Buff *cipher, const GcmParam *decryptInfo,
    Uint8Buff *outPlain)
{
    const Uint8Buff *inParams[] = { key, cipher, outPlain };
    const char *paramTags[] = { "key", "cipher", "outPlain" };
    int32_t ret = BaseCheckParams(inParams, paramTags, CAL_ARRAY_SIZE(inParams));
    if (ret != HAL_SUCCESS) {
        return ret;
    }

    CHECK_PTR_RETURN_HAL_ERROR_CODE(decryptInfo, "decryptInfo");
    CHECK_PTR_RETURN_HAL_ERROR_CODE(decryptInfo->aad, "aad");
    CHECK_LEN_ZERO_RETURN_ERROR_CODE(decryptInfo->aadLen, "aadLen");
    CHECK_PTR_RETURN_HAL_ERROR_CODE(decryptInfo->nonce, "nonce");
    CHECK_LEN_LOWER_RETURN(decryptInfo->nonceLen, HKS_AE_NONCE_LEN, "nonceLen");
    CHECK_LEN_LOWER_RETURN(outPlain->length, cipher->length - HKS_AE_TAG_LEN, "outPlain");

    return HAL_SUCCESS;
}

static int32_t AesGcmDecrypt(const Uint8Buff *key, const Uint8Buff *cipher,
    const GcmParam *decryptInfo, bool isAlias, Uint8Buff *outPlain)
{
    int32_t ret = CheckAesGcmDecryptParam(key, cipher, decryptInfo, outPlain);
    if (ret != HAL_SUCCESS) {
        return ret;
    }

    struct HksBlob keyBlob = { key->length, key->val };
    struct HksBlob cipherBlob = { cipher->length, cipher->val };
    struct HksBlob plainBlob = { outPlain->length, outPlain->val };

    struct HksParamSet *paramSet = NULL;
    struct HksParam decryptParam[] = {
        {
            .tag = HKS_TAG_PURPOSE,
            .uint32Param = HKS_KEY_PURPOSE_DECRYPT
        }, {
            .tag = HKS_TAG_ALGORITHM,
            .uint32Param = HKS_ALG_AES
        }, {
            .tag = HKS_TAG_BLOCK_MODE,
            .uint32Param = HKS_MODE_GCM
        }, {
            .tag = HKS_TAG_PADDING,
            .uint32Param = HKS_PADDING_NONE
        }, {
            .tag = HKS_TAG_NONCE,
            .blob = { decryptInfo->nonceLen, decryptInfo->nonce }
        }, {
            .tag = HKS_TAG_ASSOCIATED_DATA,
            .blob = { decryptInfo->aadLen, decryptInfo->aad }
        }, {
            .tag = HKS_TAG_IS_KEY_ALIAS,
            .boolParam = isAlias
        }
    };

    ret = ConstructParamSet(&paramSet, decryptParam, CAL_ARRAY_SIZE(decryptParam));
    if (ret != HAL_SUCCESS) {
        LOGE("construct param set failed, ret = %d", ret);
        return ret;
    }

    ret = HksDecrypt(&keyBlob, paramSet, &cipherBlob, &plainBlob);
    if (ret != HKS_SUCCESS) {
        LOGE("Aes-gcm decrypt failed, ret: %d", ret);
        HksFreeParamSet(&paramSet);
        return HAL_FAILED;
    }

    HksFreeParamSet(&paramSet);
    return HAL_SUCCESS;
}

static int32_t CheckImportAsymmetricKeyParam(const Uint8Buff *keyAlias, const Uint8Buff *authToken,
    const Uint8Buff *authId, const int32_t userType, const int32_t pairType)
{
    const Uint8Buff *inParams[] = { keyAlias, authToken, authId };
    const char *paramTags[] = { "keyAlias", "authToken", "authId" };
    int32_t ret = BaseCheckParams(inParams, paramTags, CAL_ARRAY_SIZE(inParams));
    if (ret != HAL_SUCCESS) {
        return ret;
    }

    CHECK_LEN_HIGHER_RETURN(pairType, PAIR_TYPE_END - 1, "pairType");

    return HAL_SUCCESS;
}

static int32_t ConstructImportAsymmetricKeyParam(struct HksParamSet **paramSet, uint32_t keyLen, uint32_t roleInfo,
    const struct HksBlob *authIdBlob)
{
    struct HksParam importParam[] = {
        {
            .tag = HKS_TAG_ALGORITHM,
            .uint32Param = HKS_ALG_AES
        }, {
            .tag = HKS_TAG_KEY_SIZE,
            .uint32Param = keyLen * BITS_PER_BYTE
        }, {
            .tag = HKS_TAG_PADDING,
            .uint32Param = HKS_PADDING_NONE
        }, {
            .tag = HKS_TAG_KEY_AUTH_ID,
            .blob = *authIdBlob
        }, {
            .tag = HKS_TAG_IS_ALLOWED_WRAP,
            .boolParam = false
        }, {
            .tag = HKS_TAG_PURPOSE,
            .uint32Param = HKS_KEY_PURPOSE_MAC
        }, {
            .tag = HKS_TAG_KEY_ROLE,
            .uint32Param = roleInfo
        }, {
            .tag = HKS_TAG_BLOCK_MODE,
            .uint32Param = HKS_MODE_GCM
        }, {
            .tag = HKS_TAG_DIGEST,
            .uint32Param = HKS_DIGEST_SHA256
        }
    };

    int ret = ConstructParamSet(paramSet, importParam, CAL_ARRAY_SIZE(importParam));
    if (ret != HAL_SUCCESS) {
        LOGE("construct decrypt param set failed, ret = %d", ret);
        return ret;
    }

    return ret;
}

static int32_t ImportAsymmetricKey(const Uint8Buff *keyAlias, const Uint8Buff *authToken, const ExtraInfo *exInfo)
{
    int32_t ret = CheckImportAsymmetricKeyParam(keyAlias, authToken, &exInfo->authId, exInfo->userType,
        exInfo->pairType);
    if (ret != HAL_SUCCESS) {
        return ret;
    }

    struct HksBlob keyAliasBlob = { keyAlias->length, keyAlias->val };
    struct HksBlob pubKeyBlob = { authToken->length, authToken->val };

    struct HksBlob authIdBlob = { exInfo->authId.length, exInfo->authId.val };
    union KeyRoleInfoUnion roleInfoUnion;
    roleInfoUnion.roleInfoStruct.userType = (uint8_t)exInfo->userType;
    roleInfoUnion.roleInfoStruct.pairType = (uint8_t)exInfo->pairType;
    roleInfoUnion.roleInfoStruct.reserved1 = (uint8_t)0;
    roleInfoUnion.roleInfoStruct.reserved2 = (uint8_t)0;

    struct HksParamSet *paramSet = NULL;
    ret = ConstructImportAsymmetricKeyParam(&paramSet, authToken->length, roleInfoUnion.roleInfo, &authIdBlob);
    if (ret != HAL_SUCCESS) {
        LOGE("construct param set failed, ret = %d", ret);
        return ret;
    }

    ret = HksImportKey(&keyAliasBlob, paramSet, &pubKeyBlob);
    if (ret != HKS_SUCCESS) {
        LOGE("HksImportKey failed, ret: %d", ret);
        HksFreeParamSet(&paramSet);
        return ret;
    }

    HksFreeParamSet(&paramSet);
    return HAL_SUCCESS;
}

static const AlgLoader g_huksLoader = {
    InitHks,
    Sha256,
    GenerateRandom,
    ComputeHmac,
    ComputeHkdf,
    ImportAsymmetricKey,
    CheckKeyExist,
    DeleteKey,
    AesGcmEncrypt,
    AesGcmDecrypt,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

const AlgLoader *GetRealLoaderInstance()
{
    return &g_huksLoader;
}