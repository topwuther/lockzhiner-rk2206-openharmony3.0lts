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

#include "pake_task_common.h"
#include "common_util.h"
#include "das_asy_token_manager.h"
#include "das_common.h"
#include "das_module_defines.h"
#include "hc_log.h"
#include "hc_types.h"
#include "module_common.h"
#include "protocol_common.h"
#include "standard_client_bind_exchange_task.h"

#define ASCII_CASE_DIFFERENCE_VALUE 32

int32_t ConstructOutJson(const PakeParams *params, CJson *out)
{
    int32_t res;
    CJson *payload = NULL;
    CJson *sendToPeer = NULL;

    payload = CreateJson();
    if (payload == NULL) {
        LOGE("Create payload json failed.");
        res =  HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    sendToPeer = CreateJson();
    if (sendToPeer == NULL) {
        LOGE("Create sendToPeer json failed.");
        res =  HC_ERR_ALLOC_MEMORY;
        goto err;
    }

    if (params->opCode == AUTHENTICATE) {
        res = AddIntToJson(sendToPeer, FIELD_AUTH_FORM, AUTH_FORM_ACCOUNT_UNRELATED);
        if (res != HC_SUCCESS) {
            LOGE("Add authForm failed, res: %d.", res);
            goto err;
        }
    }
    res = AddObjToJson(sendToPeer, FIELD_PAYLOAD, payload);
    if (res != HC_SUCCESS) {
        LOGE("Add payload to sendToPeer failed, res: %d.", res);
        goto err;
    }
    
    res = AddObjToJson(out, FIELD_SEND_TO_PEER, sendToPeer);
    if (res != HC_SUCCESS) {
        LOGE("Add sendToPeer to out failed, res: %d.", res);
        goto err;
    }
err:
    FreeJson(payload);
    FreeJson(sendToPeer);
    return res;
}

static int32_t GenerateOutputKey(PakeParams *params)
{
    Uint8Buff keyInfo = { (uint8_t *)HICHAIN_RETURN_KEY, strlen(HICHAIN_RETURN_KEY) };
    int32_t res = params->baseParams.loader->computeHkdf(&(params->baseParams.sessionKey), &(params->baseParams.salt),
        &keyInfo, &(params->returnKey), false);
    if (res != HC_SUCCESS) {
        LOGE("generate returnKey failed");
        FreeAndCleanKey(&(params->baseParams.sessionKey));
        FreeAndCleanKey(&(params->returnKey));
        return res;
    }
    return res;
}

int32_t SendResultToSelf(PakeParams *params, CJson *out)
{
    int res = HC_SUCCESS;
    CJson *sendToSelf = CreateJson();
    if (sendToSelf == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    GOTO_ERR_AND_SET_RET(AddIntToJson(sendToSelf, FIELD_OPERATION_CODE, OP_BIND), res);
    GOTO_ERR_AND_SET_RET(AddIntToJson(sendToSelf, FIELD_AUTH_FORM, AUTH_FORM_ACCOUNT_UNRELATED), res);

    if (params->returnKey.length != 0) { /* keyLen == 0 means unbind, needn't to generate returnKey */
        res = GenerateOutputKey(params);
        if (res != HC_SUCCESS) {
            LOGE("GenerateOutputKey failed, res:%d", res);
            goto err;
        }
        GOTO_ERR_AND_SET_RET(AddByteToJson(sendToSelf, FIELD_SESSION_KEY, params->returnKey.val,
            params->returnKey.length), res);
    }

    GOTO_ERR_AND_SET_RET(AddObjToJson(out, FIELD_SEND_TO_SELF, sendToSelf), res);
err:
    FreeAndCleanKey(&(params->baseParams.sessionKey));
    FreeAndCleanKey(&(params->returnKey));
    FreeJson(sendToSelf);
    return res;
}

static int32_t FillPskWithPin(PakeParams *params, const CJson *in)
{
    const char *pinString = GetStringFromJson(in, FIELD_PIN_CODE);
    if (pinString == NULL) {
        LOGE("Get pin code failed.");
        return HC_ERR_JSON_GET;
    }
    if (strlen(pinString) < MIN_PIN_LEN || strlen(pinString) > MAX_PIN_LEN) {
        LOGE("Pin code is too short.");
        return HC_ERR_INVALID_LEN;
    }

    int res = InitSingleParam(&(params->baseParams.psk), strlen(pinString));
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for psk failed, res: %d.", res);
        return res;
    }
    if (memcpy_s(params->baseParams.psk.val, params->baseParams.psk.length,
        pinString, strlen(pinString)) != HC_SUCCESS) {
        LOGE("Memcpy for pin code failed.");
        FreeAndCleanKey(&params->baseParams.psk);
        return HC_ERR_MEMORY_COPY;
    }

    return HC_SUCCESS;
}

static void UpperToLowercase(Uint8Buff *hex)
{
    for (uint32_t i = 0; i < hex->length; i++) {
        if (hex->val[i] >= 'A' && hex->val[i] <= 'F') {
            hex->val[i] += ASCII_CASE_DIFFERENCE_VALUE;
        }
    }
}

static int32_t ConvertPsk(const Uint8Buff *srcPsk, PakeParams *params)
{
    int res;
    res = InitSingleParam(&(params->baseParams.psk), PAKE_PSK_LEN * BYTE_TO_HEX_OPER_LENGTH + 1);
    if (res != HC_SUCCESS) {
        LOGE("InitSingleParam for psk failed, res: %d.", res);
        return res;
    }
    /* For compatibility, to be same with HiChain 2.0 */
    res = ByteToHexString(srcPsk->val, srcPsk->length, (char *)params->baseParams.psk.val,
        params->baseParams.psk.length);
    if (res != HC_SUCCESS) {
        LOGE("Convert psk from byte to hex string failed, res: %d.", res);
        return res;
    }
    params->baseParams.psk.length = params->baseParams.psk.length - 1; // do not need include '\0' when using psk
    (void)UpperToLowercase(&(params->baseParams.psk));
    return res;
}

int32_t FillPskWithDerivedKey(PakeParams *params)
{
    int32_t res;
    if (!(params->baseParams.isClient)) {
        res = params->baseParams.loader->generateRandom(&(params->nonce));
        if (res != HC_SUCCESS) {
            LOGE("Generate nonce failed, res: %d.", res);
            return res;
        }
    }
    uint8_t pskKeyAliasVal[PAKE_KEY_ALIAS_LEN] = { 0 };
    Uint8Buff pskKeyAlias = { pskKeyAliasVal, PAKE_KEY_ALIAS_LEN };
    Uint8Buff packageName = { (uint8_t *)params->packageName, strlen(params->packageName) };
    Uint8Buff serviceType = { (uint8_t *)params->serviceType, strlen(params->serviceType) };
    res = GenerateKeyAlias(&packageName, &serviceType, KEY_ALIAS_PSK, &(params->baseParams.idPeer), &pskKeyAlias);
    if (res != HC_SUCCESS) {
        LOGE("GenerateKeyAlias for psk failed, res: %d.", res);
        return res;
    }

    if (params->baseParams.loader->checkKeyExist(&pskKeyAlias) != HC_SUCCESS) {
        res = GetAsyTokenManagerInstance()->computeAndSavePsk(params);
        if (res != HC_SUCCESS) {
            LOGE("ComputeAndSavePsk failed, res: %d.", res);
            return res;
        }
    }

    Uint8Buff pskByte = { NULL, PAKE_PSK_LEN };
    pskByte.val = (uint8_t *)HcMalloc(PAKE_PSK_LEN, 0);
    if (pskByte.val == NULL) {
        LOGE("Malloc for pskByte failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    Uint8Buff keyInfo = { (uint8_t *)TMP_AUTH_KEY_FACTOR, strlen(TMP_AUTH_KEY_FACTOR) };
    res = params->baseParams.loader->computeHkdf(&pskKeyAlias, &(params->nonce), &keyInfo, &pskByte, true);
    if (res != HC_SUCCESS) {
        LOGE("ComputeHkdf for psk failed, res: %d.", res);
        goto err;
    }

    res = ConvertPsk(&pskByte, params);
    if (res != HC_SUCCESS) {
        LOGE("ConvertPsk failed, res: %d.", res);
        goto err;
    }
    goto out;
err:
    FreeAndCleanKey(&(params->baseParams.psk));
out:
    FreeAndCleanKey(&pskByte);
    return res;
}

static int32_t FillAuthId(PakeParams *params, const CJson *in)
{
    const char *authId = GetStringFromJson(in, FIELD_SELF_AUTH_ID);
    if (authId == NULL) {
        LOGE("Get self authId failed.");
        return HC_ERR_JSON_GET;
    }
    uint32_t authIdLen = strlen(authId);
    if (authIdLen == 0 || authIdLen > MAX_AUTH_ID_LEN) {
        LOGE("Invalid self authId length.");
        return HC_ERR_INVALID_LEN;
    }
    params->baseParams.idSelf.length = authIdLen;
    params->baseParams.idSelf.val = (uint8_t *)HcMalloc(params->baseParams.idSelf.length, 0);
    if (params->baseParams.idSelf.val == NULL) {
        LOGE("Malloc for idSelf failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(params->baseParams.idSelf.val, params->baseParams.idSelf.length, authId, strlen(authId)) != EOK) {
        LOGE("Memcpy for idSelf failed.");
        return HC_ERR_MEMORY_COPY;
    }

    if (params->opCode == AUTHENTICATE || params->opCode == OP_UNBIND) {
        authId = GetStringFromJson(in, FIELD_PEER_AUTH_ID);
        if (authId == NULL) {
            LOGE("Get peer authId failed.");
            return HC_ERR_JSON_GET;
        }
        authIdLen = strlen(authId);
        if (authIdLen == 0 || authIdLen > MAX_AUTH_ID_LEN) {
            LOGE("Invalid peer authId length.");
            return HC_ERR_INVALID_LEN;
        }
        params->baseParams.idPeer.length = authIdLen;
        params->baseParams.idPeer.val = (uint8_t *)HcMalloc(params->baseParams.idPeer.length, 0);
        if (params->baseParams.idPeer.val == NULL) {
            LOGE("Malloc for idPeer failed.");
            return HC_ERR_ALLOC_MEMORY;
        }
        if (memcpy_s(params->baseParams.idPeer.val, params->baseParams.idPeer.length, authId, strlen(authId)) != EOK) {
            LOGE("Memcpy for idPeer failed.");
            return HC_ERR_MEMORY_COPY;
        }
    }

    return HC_SUCCESS;
}

static int32_t FillUserType(PakeParams *params, const CJson *in)
{
    int32_t res = GetIntFromJson(in, FIELD_SELF_TYPE, &(params->userType));
    if (res != HC_SUCCESS) {
        LOGE("Get userType failed: %d", res);
        return res;
    }

    res = GetIntFromJson(in, FIELD_PEER_USER_TYPE, &(params->userTypePeer));
    if (res != HC_SUCCESS) {
        LOGD("Get peer userType failed, use default, res: %d", res);
        params->userTypePeer = DEVICE_TYPE_ACCESSORY;
    }
    return HC_SUCCESS;
}

static int32_t FillPkgNameAndServiceType(PakeParams *params, const CJson *in)
{
    const char *packageName = GetStringFromJson(in, FIELD_PKG_NAME);
    if (packageName == NULL) {
        LOGE("Get packageName failed.");
        return HC_ERR_JSON_GET;
    }
    params->packageName = (char *)HcMalloc(strlen(packageName) + 1, 0);
    if (params->packageName == NULL) {
        LOGE("Malloc for packageName failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(params->packageName, strlen(packageName) + 1, packageName, strlen(packageName)) != EOK) {
        LOGE("Memcpy for packageName failed.");
        return HC_ERR_MEMORY_COPY;
    }

    const char *serviceType = GetStringFromJson(in, FIELD_SERVICE_TYPE);
    if (serviceType == NULL) {
        LOGE("Get serviceType failed.");
        return HC_ERR_JSON_GET;
    }
    params->serviceType = (char *)HcMalloc(strlen(serviceType) + 1, 0);
    if (params->serviceType == NULL) {
        LOGE("Malloc for serviceType failed.");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(params->serviceType, strlen(serviceType) + 1, serviceType, strlen(serviceType)) != EOK) {
        LOGE("Memcpy for serviceType failed.");
        return HC_ERR_MEMORY_COPY;
    }

    return HC_SUCCESS;
}

static int32_t FillNonce(PakeParams *params, const CJson *in)
{
    (void)in;
    if (params->opCode == AUTHENTICATE || params->opCode == OP_UNBIND) {
        params->nonce.length = PAKE_NONCE_LEN;
        params->nonce.val = (uint8_t *)HcMalloc(params->nonce.length, 0);
        if (params->nonce.val == NULL) {
            LOGE("Malloc for nonce failed.");
            return HC_ERR_ALLOC_MEMORY;
        }
    } else {
        params->nonce.length = 0;
        params->nonce.val = NULL;
    }
    return HC_SUCCESS;
}

int32_t FillDasPakeParams(PakeParams *params, const CJson *in)
{
    int32_t res = GetIntFromJson(in, FIELD_OPERATION_CODE, &(params->opCode));
    if (res != HC_SUCCESS) {
        LOGD("Get opCode failed, use default, res: %d", res);
        params->opCode = AUTHENTICATE;
    }

    res = GetBoolFromJson(in, FIELD_IS_CLIENT, &(params->baseParams.isClient));
    if (res != HC_SUCCESS) {
        LOGE("Get isClient failed, res: %d.", res);
        return res;
    }

    res = FillNonce(params, in);
    if (res != HC_SUCCESS) {
        return res;
    }

    res = FillUserType(params, in);
    if (res != HC_SUCCESS) {
        return res;
    }

    res = FillPkgNameAndServiceType(params, in);
    if (res != HC_SUCCESS) {
        return res;
    }

    res = FillAuthId(params, in);
    if (res != HC_SUCCESS) {
        return res;
    }

    if (params->opCode == OP_BIND) {
        res = FillPskWithPin(params, in);
        if (res != HC_SUCCESS) {
            return res;
        }
    }

    params->baseParams.curveType = CURVE_25519;
    return HC_SUCCESS;
}
