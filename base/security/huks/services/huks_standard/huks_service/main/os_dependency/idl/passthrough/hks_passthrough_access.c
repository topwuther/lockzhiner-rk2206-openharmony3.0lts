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

#include "hks_access.h"
#include "hks_core_service.h"

#ifndef _CUT_AUTHENTICATE_
int32_t HksAccessGenerateKey(const struct HksBlob *keyBlob, const struct HksParamSet *paramSetIn,
    const struct HksBlob *keyIn, struct HksBlob *keyOut)
{
    return HksCoreGenerateKey(keyBlob, paramSetIn, keyIn, keyOut);
}

int32_t HksAccessSign(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *signature)
{
    return HksCoreSign(key, paramSet, srcData, signature);
}

int32_t HksAccessVerify(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, const struct HksBlob *signature)
{
    return HksCoreVerify(key, paramSet, srcData, signature);
}

int32_t HksAccessEncrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *plainText, struct HksBlob *cipherText)
{
    return HksCoreEncrypt(key, paramSet, plainText, cipherText);
}

int32_t HksAccessDecrypt(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *cipherText, struct HksBlob *plainText)
{
    return HksCoreDecrypt(key, paramSet, cipherText, plainText);
}

int32_t HksAccessCheckKeyValidity(const struct HksParamSet *paramSet, const struct HksBlob *key)
{
    return HksCheckKeyValidity(paramSet, key);
}

int32_t HksAccessImportKey(const struct HksBlob *keyAlias, const struct HksBlob *key,
    const struct HksParamSet *paramSet, struct HksBlob *keyOut)
{
    return HksCoreImportKey(keyAlias, key, paramSet, keyOut);
}

int32_t HksAccessExportPublicKey(const struct HksBlob *key, const struct HksParamSet *paramSet,
    struct HksBlob *keyOut)
{
    return HksCoreExportPublicKey(key, paramSet, keyOut);
}

int32_t HksAccessAgreeKey(const struct HksParamSet *paramSet, const struct HksBlob  *privateKey,
    const struct HksBlob *peerPublicKey, struct HksBlob *agreedKey)
{
    return HksCoreAgreeKey(paramSet, privateKey, peerPublicKey, agreedKey);
}

int32_t HksAccessDeriveKey(const struct HksParamSet *paramSet, const struct HksBlob *kdfKey,
    struct HksBlob *derivedKey)
{
    return HksCoreDeriveKey(paramSet, kdfKey, derivedKey);
}

int32_t HksAccessMac(const struct HksBlob *key, const struct HksParamSet *paramSet,
    const struct HksBlob *srcData, struct HksBlob *mac)
{
    return HksCoreMac(key, paramSet, srcData, mac);
}

int32_t HksAccessInitialize(void)
{
    return HksCoreInitialize();
}

int32_t HksAccessProcessInit(uint32_t msgId, const struct HksBlob *key, const struct HksParamSet *paramSet,
    uint64_t *operationHandle)
{
    return 0;
}

int32_t HksAccessProcessMultiUpdate(uint32_t msgId, uint64_t operationHandle, const struct HksBlob *inData,
    struct HksBlob *outData)
{
    return 0;
}

int32_t HksAccessProcessFinal(uint32_t msgId, uint64_t operationHandle, const struct HksBlob *inData,
    struct HksBlob *outData)
{
    return 0;
}

int32_t HksAccessRefresh(void)
{
    return HksCoreRefreshKeyInfo();
}

#ifdef _STORAGE_LITE_
int32_t HksAccessCalcHeaderMac(const struct HksParamSet *paramSet, const struct HksBlob *salt,
    const struct HksBlob *srcData, struct HksBlob *mac)
{
    return HksCoreCalcMacHeader(paramSet, salt, srcData, mac);
}
#endif

#ifdef HKS_SUPPORT_UPGRADE_STORAGE_DATA
int32_t HksAccessUpgradeKeyInfo(const struct HksBlob *keyAlias, const struct HksBlob *keyInfo, struct HksBlob *keyOut)
{
    return HksCoreUpgradeKeyInfo(keyAlias, keyInfo, keyOut);
}

#endif
#endif /* _CUT_AUTHENTICATE_ */

int32_t HksAccessGenerateRandom(const struct HksParamSet *paramSet, struct HksBlob *random)
{
    return HksCoreGenerateRandom(paramSet, random);
}