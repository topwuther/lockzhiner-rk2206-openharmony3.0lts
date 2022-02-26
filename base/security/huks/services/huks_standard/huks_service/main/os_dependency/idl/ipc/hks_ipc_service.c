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

#include "hks_ipc_service.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "hks_client_service.h"
#include "hks_cmd_id.h"
#include "hks_ipc_serialization.h"
#include "hks_log.h"
#include "hks_mem.h"
#include "hks_response.h"

#define MAX_KEY_SIZE         2048
#define SIZE_OF_CHALLENGE    16

void HksIpcServiceProvision(const struct HksBlob *srcData, const uint8_t *context)
{
    return;
}

void HksIpcServiceProvisionVerify(const struct HksBlob *srcData, const uint8_t *context)
{
    return;
}

void HksIpcServiceGenerateKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob keyOut = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;
    bool isNoneResponse = false;

    do {
        ret = HksGenerateKeyUnpack(srcData, &keyAlias, &inParamSet, &keyOut);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGenerateKeyUnpack Ipc fail");
            break;
        }
        if (keyOut.data == NULL) {
            isNoneResponse = true;
            keyOut.data = (uint8_t *)HksMalloc(MAX_KEY_SIZE);
            if (keyOut.data == NULL) {
                HKS_LOG_E("malloc fail.");
                ret = HKS_ERROR_MALLOC_FAIL;
                break;
            }
            keyOut.size = MAX_KEY_SIZE;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceGenerateKey(&processName, &keyAlias, inParamSet, &keyOut);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceGenerateKey fail, ret = %d", ret);
        }
        HKS_LOG_E("key out size = %x", keyOut.size);
    } while (0);

    if (isNoneResponse) {
        HksSendResponse(context, ret, NULL);
    } else {
        HksSendResponse(context, ret, &keyOut);
    }

    HKS_FREE_BLOB(keyOut);
    HKS_FREE_BLOB(processName);
}

void HksIpcServiceImportKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *paramSet = NULL;
    struct HksBlob key = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret  = HksImportKeyUnpack(srcData, &keyAlias, &paramSet, &key);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksImportKeyUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret =  HksServiceImportKey(&processName, &keyAlias, paramSet, &key);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceImportKey fail, ret = %d", ret);
        }
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processName);
}

void HksIpcServiceExportPublicKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksBlob key = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret  = HksExportPublicKeyUnpack(srcData, &keyAlias, &key);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksExportKeyUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceExportPublicKey(&processName, &keyAlias, &key);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceExportPublicKey fail, ret = %d", ret);
            break;
        }
        HksSendResponse(context, ret, &key);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(key);
    HKS_FREE_BLOB(processName);
}

void HksIpcServiceDeleteKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob processName = { 0, NULL };
    int32_t ret;
    do {
        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceDeleteKey(&processName, srcData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksIpcServiceDeleteKey fail");
        }
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processName);
}

void HksIpcServiceGetKeyParamSet(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *paramSet = NULL;
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksGetKeyParamSetUnpack(srcData, &keyAlias, &paramSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGenerateKeyUnpack Ipc fail");
            return HksSendResponse(context, ret, NULL);
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceGetKeyParamSet(&processName, &keyAlias, paramSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceGetKeyParamSet fail, ret = %d", ret);
            break;
        }
        struct HksBlob paramSetOut = { paramSet->paramSetSize, (uint8_t *)paramSet };
        HksSendResponse(context, ret, &paramSetOut);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(processName);
    HKS_FREE_PTR(paramSet);
}

void HksIpcServiceKeyExist(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceKeyExist(&processName, srcData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceKeyExist fail, ret = %d", ret);
        }
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processName);
}

void HksIpcServiceGenerateRandom(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob processName = { 0, NULL };
    struct HksBlob random = { 0, NULL };
    int32_t ret;

    do {
        if ((srcData == NULL) || (srcData->data == NULL) || (srcData->size < sizeof(uint32_t))) {
            HKS_LOG_E("invalid srcData");
            ret = HKS_ERROR_INVALID_ARGUMENT;
            break;
        }

        random.size = *((uint32_t *)(srcData->data));
        if (IsInvalidLength(random.size)) {
            HKS_LOG_E("invalid size %u", random.size);
            ret = HKS_ERROR_INVALID_ARGUMENT;
            break;
        }

        random.data = (uint8_t *)HksMalloc(random.size);
        if (random.data == NULL) {
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceGenerateRandom(&processName, &random);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceGenerateRandom fail, ret = %d", ret);
            break;
        }
        HksSendResponse(context, ret, &random);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(random);
    HKS_FREE_BLOB(processName);
}

void HksIpcServiceSign(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob unsignedData = { 0, NULL };
    struct HksBlob signature = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksSignUnpack(srcData, &keyAlias, &inParamSet, &unsignedData, &signature);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksSignUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceSign(&processName, &keyAlias, inParamSet, &unsignedData, &signature);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceSign fail");
            break;
        }
        HksSendResponse(context, ret, &signature);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(signature);
    HKS_FREE_BLOB(processName);
}

static void IpcServiceProcessInit(uint32_t cmdId, const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksInitUnpack(srcData, &keyAlias, &inParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksInitUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        uint64_t operationHandle;
        ret = HksServiceProcessInit(cmdId, &processName, &keyAlias, inParamSet, &operationHandle);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceProcessInit fail");
            break;
        }
        struct HksBlob handleBlob = { sizeof(uint64_t), (uint8_t *)&operationHandle };
        HksSendResponse(context, ret, &handleBlob);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(processName);
}

static void IpcServiceProcessUpdate(uint32_t cmdId, const struct HksBlob *srcData, const uint8_t *context)
{
    uint64_t operationHandle;
    struct HksBlob inputData = { 0, NULL };
    struct HksBlob *outPtr = NULL;
    struct HksBlob outputData = { 0, NULL };
    int32_t ret;

    do {
        if ((cmdId == HKS_CMD_ID_ENCRYPT_UPDATE) || (cmdId == HKS_CMD_ID_DECRYPT_UPDATE)) {
            outPtr = &outputData;
        }

        ret = HksUpdateUnpack(srcData, &operationHandle, &inputData, outPtr);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksUpdateUnpack fail");
            break;
        }

        ret = HksServiceProcessUpdate(cmdId, operationHandle, &inputData, outPtr);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceProcessUpdate fail");
            break;
        }

        HksSendResponse(context, ret, outPtr);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    if (outPtr != NULL) {
        HKS_FREE_BLOB(*outPtr);
    }
}

static void IpcServiceProcessFinal(uint32_t cmdId, const struct HksBlob *srcData, const uint8_t *context)
{
    uint64_t operationHandle;
    struct HksBlob inputData = { 0, NULL };
    struct HksBlob outputData = { 0, NULL };
    struct HksBlob *noFree = NULL;
    struct HksBlob *needFree = NULL;
    int32_t ret;

    do {
        if (cmdId == HKS_CMD_ID_VERIFY_FINAL) {
            noFree = &outputData;
        } else {
            needFree = &outputData;
        }

        ret = HksFinalUnpack(srcData, &operationHandle, &inputData, noFree, needFree);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksFinalUnpack fail");
            break;
        }

        ret = HksServiceProcessFinal(cmdId, operationHandle, &inputData, &outputData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceProcessFinal fail");
            break;
        }
        HksSendResponse(context, ret, needFree);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    if (needFree != NULL) {
        HKS_FREE_BLOB(*needFree);
    }
}

void HksIpcServiceSignInit(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessInit(HKS_CMD_ID_SIGN_INIT, srcData, context);
}

void HksIpcServiceSignUpdate(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessUpdate(HKS_CMD_ID_SIGN_UPDATE, srcData, context);
}

void HksIpcServiceSignFinal(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessFinal(HKS_CMD_ID_SIGN_FINAL, srcData, context);
}

void HksIpcServiceVerify(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob unsignedData = { 0, NULL };
    struct HksBlob signature = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksVerifyUnpack(srcData, &keyAlias, &inParamSet, &unsignedData, &signature);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksVerifyUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceVerify(&processName, &keyAlias, inParamSet, &unsignedData, &signature);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceVerify fail");
        }
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processName);
}

void HksIpcServiceVerifyInit(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessInit(HKS_CMD_ID_VERIFY_INIT, srcData, context);
}

void HksIpcServiceVerifyUpdate(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessUpdate(HKS_CMD_ID_VERIFY_UPDATE, srcData, context);
}

void HksIpcServiceVerifyFinal(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessFinal(HKS_CMD_ID_VERIFY_FINAL, srcData, context);
}

void HksIpcServiceEncrypt(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob plainText = { 0, NULL };
    struct HksBlob cipherText = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksEncryptDecryptUnpack(srcData, &keyAlias, &inParamSet, &plainText, &cipherText);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksEncryptDecryptUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceEncrypt(&processName, &keyAlias, inParamSet, &plainText, &cipherText);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceEncrypt fail");
            break;
        }
        HksSendResponse(context, ret, &cipherText);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(cipherText);
    HKS_FREE_BLOB(processName);
}

void HksIpcServiceEncryptInit(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessInit(HKS_CMD_ID_ENCRYPT_INIT, srcData, context);
}

void HksIpcServiceEncryptUpdate(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessUpdate(HKS_CMD_ID_ENCRYPT_UPDATE, srcData, context);
}

void HksIpcServiceEncryptFinal(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessFinal(HKS_CMD_ID_ENCRYPT_FINAL, srcData, context);
}

void HksIpcServiceDecrypt(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob plainText = { 0, NULL };
    struct HksBlob cipherText = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksEncryptDecryptUnpack(srcData, &keyAlias, &inParamSet, &cipherText, &plainText);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksEncryptDecryptUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceDecrypt(&processName, &keyAlias, inParamSet, &cipherText, &plainText);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceDecrypt fail");
            break;
        }
        HksSendResponse(context, ret, &plainText);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(plainText);
    HKS_FREE_BLOB(processName);
}

void HksIpcServiceDecryptInit(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessInit(HKS_CMD_ID_DECRYPT_INIT, srcData, context);
}

void HksIpcServiceDecryptUpdate(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessUpdate(HKS_CMD_ID_DECRYPT_UPDATE, srcData, context);
}

void HksIpcServiceDecryptFinal(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessFinal(HKS_CMD_ID_DECRYPT_FINAL, srcData, context);
}

void HksIpcServiceAgreeKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob privateKey = { 0, NULL };
    struct HksBlob peerPublicKey = { 0, NULL };
    struct HksBlob agreedKey = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksAgreeKeyUnpack(srcData, &inParamSet, &privateKey, &peerPublicKey, &agreedKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksAgreeKeyUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceAgreeKey(&processName, inParamSet, &privateKey, &peerPublicKey, &agreedKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceAgreeKey fail, ret = %d", ret);
            break;
        }
        HksSendResponse(context, ret, &agreedKey);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(agreedKey);
    HKS_FREE_BLOB(processName);
}

void HksIpcServiceDeriveKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob masterKey = { 0, NULL };
    struct HksBlob derivedKey = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksDeriveKeyUnpack(srcData, &inParamSet, &masterKey, &derivedKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksDeriveKeyUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceDeriveKey(&processName, inParamSet, &masterKey, &derivedKey);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceDeriveKey fail, ret = %d", ret);
            break;
        }
        HksSendResponse(context, ret, &derivedKey);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(derivedKey);
    HKS_FREE_BLOB(processName);
}

void HksIpcServiceMac(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob key = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob inputData = { 0, NULL };
    struct HksBlob mac = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksHmacUnpack(srcData, &key, &inParamSet, &inputData, &mac);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksHmacUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceMac(&processName, &key, inParamSet, &inputData, &mac);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceMac fail, ret = %d", ret);
            break;
        }
        HksSendResponse(context, ret, &mac);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(mac);
    HKS_FREE_BLOB(processName);
}

void HksIpcServiceMacInit(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessInit(HKS_CMD_ID_MAC_INIT, srcData, context);
}

void HksIpcServiceMacUpdate(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessUpdate(HKS_CMD_ID_MAC_UPDATE, srcData, context);
}

void HksIpcServiceMacFinal(const struct HksBlob *srcData, const uint8_t *context)
{
    IpcServiceProcessFinal(HKS_CMD_ID_MAC_FINAL, srcData, context);
}

static void FreeKeyInfo(uint32_t listCount, struct HksKeyInfo **keyInfoList)
{
    if ((keyInfoList == NULL) || (*keyInfoList == NULL)) {
        return;
    }

    for (uint32_t i = 0; i < listCount; ++i) {
        if ((*keyInfoList)[i].alias.data != NULL) {
            HKS_FREE_BLOB((*keyInfoList)[i].alias);
        }
        if ((*keyInfoList)[i].paramSet != NULL) {
            HksFree((*keyInfoList)[i].paramSet);
            (*keyInfoList)[i].paramSet = NULL;
        }
    }

    HKS_FREE_PTR(*keyInfoList);
}

void HksIpcServiceGetKeyInfoList(const struct HksBlob *srcData, const uint8_t *context)
{
    uint32_t inputCount = 0;
    struct HksKeyInfo *keyInfoList = NULL;
    struct HksBlob processName = { 0, NULL };
    struct HksBlob keyInfoListBlob = { 0, NULL };
    int32_t ret;

    do {
        ret = HksGetKeyInfoListUnpack(srcData, &inputCount, &keyInfoList);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetKeyInfoListUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        uint32_t listCount = inputCount;
        ret = HksServiceGetKeyInfoList(&processName, keyInfoList, &listCount);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceGetKeyInfoList fail, ret = %d", ret);
            break;
        }

        keyInfoListBlob.size = sizeof(listCount);
        for (uint32_t i = 0; i < listCount; ++i) {
            keyInfoListBlob.size += sizeof(keyInfoList[i].alias.size) + ALIGN_SIZE(keyInfoList[i].alias.size) +
                ALIGN_SIZE(keyInfoList[i].paramSet->paramSetSize);
        }

        keyInfoListBlob.data = (uint8_t *)HksMalloc(keyInfoListBlob.size);
        if (keyInfoListBlob.data == NULL) {
            ret = HKS_ERROR_MALLOC_FAIL;
            break;
        }

        ret = HksGetKeyInfoListPackFromService(&keyInfoListBlob, listCount, keyInfoList);
        if (ret != HKS_SUCCESS) {
            break;
        }

        HksSendResponse(context, ret, &keyInfoListBlob);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    FreeKeyInfo(inputCount, &keyInfoList);
    HKS_FREE_BLOB(processName);
    HKS_FREE_BLOB(keyInfoListBlob);
}

void HksIpcServiceAttestKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob certChainBlob = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksCertificateChainUnpack(srcData, &keyAlias, &inParamSet, &certChainBlob);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksCertificateChainUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceAttestKey(&processName, &keyAlias, inParamSet, &certChainBlob);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceAttestKey fail, ret = %d", ret);
            break;
        }
        HksSendResponse(context, ret, &certChainBlob);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(processName);
    HKS_FREE_BLOB(certChainBlob);
}

void HksIpcServiceGetCertificateChain(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob certChainBlob = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksCertificateChainUnpack(srcData, &keyAlias, &inParamSet, &certChainBlob);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksCertificateChainUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceGetCertificateChain(&processName, &keyAlias, inParamSet, &certChainBlob);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceGetCertificateChain fail, ret = %d", ret);
            break;
        }
        HksSendResponse(context, ret, &certChainBlob);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(processName);
    HKS_FREE_BLOB(certChainBlob);
}

void HksIpcServiceWrapKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksBlob targetKeyAlias = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob wrappedData = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksWrapKeyUnpack(srcData, &keyAlias, &targetKeyAlias, &inParamSet, &wrappedData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksWrapKeyUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceWrapKey(&processName, &keyAlias, &targetKeyAlias, inParamSet, &wrappedData);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceWrapKey fail, ret = %d", ret);
            break;
        }
        HksSendResponse(context, ret, &wrappedData);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(processName);
    HKS_FREE_BLOB(wrappedData);
}

void HksIpcServiceUnwrapKey(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob keyAlias = { 0, NULL };
    struct HksBlob targetKeyAlias = { 0, NULL };
    struct HksBlob unwrappedData = { 0, NULL };
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksUnwrapKeyUnpack(srcData, &keyAlias, &targetKeyAlias, &unwrappedData, &inParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksUnwrapKeyUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceUnwrapKey(&processName, &keyAlias, &targetKeyAlias, &unwrappedData, inParamSet);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceUnwrapKey fail, ret = %d", ret);
        }
    } while (0);

    HksSendResponse(context, ret, NULL);

    HKS_FREE_BLOB(processName);
}

void HksIpcServiceSignWithDeviceKey(const struct HksBlob *srcData, const uint8_t *context)
{
    uint32_t keyId = 0;
    struct HksParamSet *inParamSet = NULL;
    struct HksBlob unsignedData = { 0, NULL };
    struct HksBlob signature = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksSignWithDeviceKeyUnpack(srcData, &keyId, &inParamSet, &unsignedData, &signature);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksSignParse key Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceSignWithDeviceKey(&processName, keyId, inParamSet, &unsignedData, &signature);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksSign key Ipc fail");
            break;
        }
        HksSendResponse(context, ret, &signature);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(processName);
    HKS_FREE_BLOB(signature);
}

void HksIpcServiceExportTrustCerts(const struct HksBlob *srcData, const uint8_t *context)
{
    struct HksBlob certChainBlob = { 0, NULL };
    struct HksBlob processName = { 0, NULL };
    int32_t ret;

    do {
        ret = HksTrustCertsUnpack(srcData, &certChainBlob);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksTrustCertsUnpack Ipc fail");
            break;
        }

        ret = HksGetProcessNameForIPC(context, &processName);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksGetProcessNameForIPC fail, ret = %d", ret);
            break;
        }

        ret = HksServiceExportTrustCerts(&processName, &certChainBlob);
        if (ret != HKS_SUCCESS) {
            HKS_LOG_E("HksServiceExportTrustCerts fail, ret = %d", ret);
            break;
        }
        HksSendResponse(context, ret, &certChainBlob);
    } while (0);

    if (ret != HKS_SUCCESS) {
        HksSendResponse(context, ret, NULL);
    }

    HKS_FREE_BLOB(processName);
    HKS_FREE_BLOB(certChainBlob);
}
