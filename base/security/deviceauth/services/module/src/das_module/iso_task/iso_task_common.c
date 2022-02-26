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

#include "iso_task_common.h"
#include <time.h>
#include "das_common.h"
#include "das_module_defines.h"
#include "hc_log.h"
#include "hc_types.h"
#include "iso_protocol_common.h"

static int GenerateReturnKey(const IsoParams *params, uint8_t *returnKey, uint32_t returnKeyLen)
{
    int hkdfSaltLen = params->baseParams.randPeer.length + params->baseParams.randPeer.length;
    int res;
    uint8_t *hkdfSalt = (uint8_t *)HcMalloc(hkdfSaltLen, 0);
    if (hkdfSalt == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    if (params->isClient) {
        if (memcpy_s(hkdfSalt, hkdfSaltLen, params->baseParams.randSelf.val,
            params->baseParams.randSelf.length) != EOK) {
            LOGE("Copy randSelf failed.");
            res = HC_ERR_MEMORY_COPY;
            goto err;
        }
        if (memcpy_s(hkdfSalt + params->baseParams.randSelf.length, hkdfSaltLen - params->baseParams.randSelf.length,
            params->baseParams.randPeer.val, params->baseParams.randPeer.length) != EOK) {
            LOGE("Copy randPeer failed.");
            res = HC_ERR_MEMORY_COPY;
            goto err;
        }
    } else {
        if (memcpy_s(hkdfSalt, hkdfSaltLen, params->baseParams.randPeer.val,
            params->baseParams.randPeer.length) != EOK) {
            LOGE("Copy randPeer failed.");
            res = HC_ERR_MEMORY_COPY;
            goto err;
        }
        if (memcpy_s(hkdfSalt + params->baseParams.randPeer.length, hkdfSaltLen - params->baseParams.randPeer.length,
            params->baseParams.randSelf.val, params->baseParams.randSelf.length) != EOK) {
            LOGE("Copy randSelf failed.");
            res = HC_ERR_MEMORY_COPY;
            goto err;
        }
    }
    Uint8Buff hkdfSaltBuf = { hkdfSalt, hkdfSaltLen };
    Uint8Buff keyInfoBuf = { (uint8_t *)GENERATE_RETURN_KEY_STR, (uint32_t)strlen(GENERATE_RETURN_KEY_STR) };
    Uint8Buff returnKeyBuf = { returnKey, returnKeyLen };
    res = params->baseParams.loader->computeHkdf(&(params->baseParams.sessionKey), &hkdfSaltBuf, &keyInfoBuf,
        &returnKeyBuf, false);
    if (res != HC_SUCCESS) {
        LOGE("computeHkdf for returnKey failed.");
        goto err;
    }
err:
    HcFree(hkdfSalt);
    return res;
}

int GenerateEncResult(const IsoParams *params, int message, CJson *sendToPeer, const char *aad)
{
    CJson *payload = NULL;
    uint8_t *out = NULL;
    uint8_t nonce[NONCE_SIZE] = { 0 };
    Uint8Buff nonceBuf = { nonce, sizeof(nonce) };
    int ret = params->baseParams.loader->generateRandom(&nonceBuf);
    if (ret != 0) {
        return ret;
    }

    int result = 0;
    Uint8Buff plainBuf = { (uint8_t *)&result, sizeof(int) };
    GcmParam encryptInfo;
    encryptInfo.nonce = nonce;
    encryptInfo.nonceLen = NONCE_SIZE;
    encryptInfo.aad = (uint8_t *)aad;
    encryptInfo.aadLen = (uint32_t)strlen(aad);
    out = (uint8_t *)HcMalloc((sizeof(int) + TAG_LEN), 0);
    if (out == NULL) {
        ret = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    Uint8Buff outBuf = { out, sizeof(int) + TAG_LEN };
    ret = params->baseParams.loader->aesGcmEncrypt(&params->baseParams.sessionKey, &plainBuf,
        &encryptInfo, false, &outBuf);
    if (ret != HC_SUCCESS) {
        goto err;
    }
    payload = CreateJson();
    if (payload == NULL) {
        ret = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    GOTO_ERR_AND_SET_RET(AddByteToJson(payload, FIELD_NONCE, nonce, sizeof(nonce)), ret);
    GOTO_ERR_AND_SET_RET(AddByteToJson(payload, FIELD_ENC_RESULT, out, sizeof(int) + TAG_LEN), ret);
    GOTO_ERR_AND_SET_RET(AddIntToJson(payload, FIELD_OPERATION_CODE, params->opCode), ret);
    GOTO_ERR_AND_SET_RET(AddObjToJson(sendToPeer, FIELD_PAYLOAD, payload), ret);
    GOTO_ERR_AND_SET_RET(AddIntToJson(sendToPeer, FIELD_MESSAGE, message), ret);
    GOTO_ERR_AND_SET_RET(AddIntToJson(sendToPeer, FIELD_AUTH_FORM, AUTH_FORM_ACCOUNT_UNRELATED), ret);
err:
    FreeJson(payload);
    HcFree(out);
    return ret;
}

int SendResultToFinalSelf(const IsoParams *params, CJson *out, bool isNeedReturnKey)
{
    CJson *sendToSelf = CreateJson();
    if (sendToSelf == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    uint8_t *returnSessionKey = NULL;
    int res = 0;
    GOTO_ERR_AND_SET_RET(AddIntToJson(sendToSelf, FIELD_OPERATION_CODE, OP_BIND), res);
    GOTO_ERR_AND_SET_RET(AddIntToJson(sendToSelf, FIELD_AUTH_FORM, AUTH_FORM_ACCOUNT_UNRELATED), res);
    if (isNeedReturnKey) {
        returnSessionKey = (uint8_t *)HcMalloc(params->keyLen, 0);
        if (returnSessionKey == NULL) {
            res = HC_ERR_ALLOC_MEMORY;
            goto err;
        }
        res = GenerateReturnKey(params, returnSessionKey, params->keyLen);
        if (res != 0) {
            LOGE("gen return key failed, res:%d", res);
            goto err;
        }
        GOTO_ERR_AND_SET_RET(AddByteToJson(sendToSelf, FIELD_SESSION_KEY, returnSessionKey, params->keyLen), res);
    }
    GOTO_ERR_AND_SET_RET(AddObjToJson(out, FIELD_SEND_TO_SELF, sendToSelf), res);
err:
    FreeJson(sendToSelf);
    if (returnSessionKey != NULL) {
        (void)memset_s(returnSessionKey, params->keyLen, 0, params->keyLen);
    }
    HcFree(returnSessionKey);
    return res;
}

int GenEncResult(const IsoParams *params, int message, CJson *out, const char *aad, bool isNeedReturnKey)
{
    CJson *sendToSelf = CreateJson();
    if (sendToSelf == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    CJson *sendToPeer = CreateJson();
    if (sendToPeer == NULL) {
        FreeJson(sendToSelf);
        return HC_ERR_ALLOC_MEMORY;
    }

    uint8_t *returnKey = NULL;
    int res = GenerateEncResult(params, message, sendToPeer, aad);
    if (res != 0) {
        goto err;
    }
    GOTO_ERR_AND_SET_RET(AddIntToJson(sendToSelf, FIELD_AUTH_FORM, AUTH_FORM_ACCOUNT_UNRELATED), res);
    if (isNeedReturnKey) {
        returnKey = (uint8_t *)HcMalloc(params->keyLen, 0);
        if (returnKey == NULL) {
            res = HC_ERR_ALLOC_MEMORY;
            goto err;
        }
        res = GenerateReturnKey(params, returnKey, params->keyLen);
        if (res != 0) {
            LOGE("gen return key failed, res:%d", res);
            goto err;
        }
        GOTO_ERR_AND_SET_RET(AddByteToJson(sendToSelf, FIELD_SESSION_KEY, returnKey,
            params->keyLen), res);
    }
    GOTO_ERR_AND_SET_RET(AddIntToJson(sendToSelf, FIELD_OPERATION_CODE, params->opCode), res);
    GOTO_ERR_AND_SET_RET(AddObjToJson(out, FIELD_SEND_TO_PEER, sendToPeer), res);
    GOTO_ERR_AND_SET_RET(AddObjToJson(out, FIELD_SEND_TO_SELF, sendToSelf), res);
err:
    FreeJson(sendToPeer);
    FreeJson(sendToSelf);
    if (returnKey != NULL) {
        (void)memset_s(returnKey, params->keyLen, 0, params->keyLen);
    }
    HcFree(returnKey);
    return res;
}

int CheckEncResult(IsoParams *params, const CJson *in, const char *aad)
{
    int result = 0;
    int res;
    uint8_t *nonce = NULL;
    uint8_t *encResult = NULL;

    nonce = (uint8_t *)HcMalloc(NONCE_SIZE, 0);
    if (nonce == NULL) {
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    GOTO_ERR_AND_SET_RET(GetByteFromJson(in, FIELD_NONCE, nonce, NONCE_SIZE), res);
    encResult = (uint8_t *)HcMalloc(sizeof(int) + TAG_LEN, 0);
    if (encResult == NULL) {
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    GOTO_ERR_AND_SET_RET(GetByteFromJson(in, FIELD_ENC_RESULT, encResult, sizeof(int) + TAG_LEN), res);
    Uint8Buff outBuf = { (uint8_t *)&result, sizeof(int) };
    Uint8Buff encResultBuf = { encResult, sizeof(int) + TAG_LEN };
    GcmParam gcmParam;
    gcmParam.aad = (uint8_t *)aad;
    gcmParam.aadLen = (uint32_t)strlen(aad);
    gcmParam.nonce = nonce;
    gcmParam.nonceLen = NONCE_SIZE;

    res = params->baseParams.loader->aesGcmDecrypt(&params->baseParams.sessionKey, &encResultBuf, &gcmParam, false,
        &outBuf);
    if (res != 0) {
        LOGE("decrypt result failed, res:%d", res);
        goto err;
    }
err:
    HcFree(nonce);
    HcFree(encResult);
    return res;
}

void DeleteAuthCode(const IsoParams *params)
{
    uint8_t *keyAlias = (uint8_t *)HcMalloc(ISO_KEY_ALIAS_LEN, 0);
    if (keyAlias == NULL) {
        return;
    }
    int res = GenerateKeyAliasInIso(params, keyAlias, ISO_KEY_ALIAS_LEN, true);
    if (res != 0) {
        LOGE("GenerateKeyAliasInIso failed, res:%d", res);
        goto err;
    }
    Uint8Buff outKeyAlias = { keyAlias, ISO_KEY_ALIAS_LEN };
    params->baseParams.loader->deleteKey(&outKeyAlias);
err:
    HcFree(keyAlias);
    return;
}

void DestroyIsoParams(IsoParams *params)
{
    if (params == NULL) {
        return;
    }
    if (params->baseParams.sessionKey.val != NULL) {
        (void)memset_s(params->baseParams.sessionKey.val, params->baseParams.sessionKey.length, 0,
            params->baseParams.sessionKey.length);
        HcFree(params->baseParams.sessionKey.val);
        params->baseParams.sessionKey.val = NULL;
    }
    if (params->packageName != NULL) {
        HcFree(params->packageName);
        params->packageName = NULL;
    }
    if (params->serviceType != NULL) {
        HcFree(params->serviceType);
        params->serviceType = NULL;
    }
    if (params->baseParams.randSelf.val != NULL) {
        HcFree(params->baseParams.randSelf.val);
        params->baseParams.randSelf.val = NULL;
    }
    if (params->baseParams.randPeer.val != NULL) {
        HcFree(params->baseParams.randPeer.val);
        params->baseParams.randPeer.val = NULL;
    }
    if (params->baseParams.authIdPeer.val != NULL) {
        HcFree(params->baseParams.authIdPeer.val);
        params->baseParams.authIdPeer.val = NULL;
    }
    if (params->baseParams.authIdSelf.val != NULL) {
        HcFree(params->baseParams.authIdSelf.val);
        params->baseParams.authIdSelf.val = NULL;
    }
    if (params->seed.val != NULL) {
        HcFree(params->seed.val);
        params->seed.val = NULL;
    }
    if (params->pinCodeString != NULL) {
        (void)memset_s(params->pinCodeString, strlen(params->pinCodeString), 0, strlen(params->pinCodeString));
        HcFree(params->pinCodeString);
        params->pinCodeString = NULL;
    }
    (void)memset_s(params, sizeof(IsoParams), 0, sizeof(IsoParams));
}

static int FillAuthId(IsoParams *params, const CJson *in)
{
    const char *authId = GetStringFromJson(in, FIELD_SELF_AUTH_ID);
    if (authId == NULL) {
        LOGE("get self authId failed");
        return HC_ERROR;
    }
    uint32_t authIdLen = strlen(authId);
    if (authIdLen == 0 || authIdLen > MAX_AUTH_ID_LEN) {
        LOGE("Invalid authIdSelfLen");
        return HC_ERR_INVALID_PARAMS;
    }
    params->baseParams.authIdSelf.length = authIdLen;
    params->baseParams.authIdSelf.val = (uint8_t *)HcMalloc(params->baseParams.authIdSelf.length, 0);
    if (params->baseParams.authIdSelf.val == NULL) {
        LOGE("malloc authIdSelf failed");
        return HC_ERROR;
    }
    if (memcpy_s(params->baseParams.authIdSelf.val, params->baseParams.authIdSelf.length,
        authId, strlen(authId)) != EOK) {
        LOGE("Memcpy authIdSelf failed.");
        return HC_ERR_MEMORY_COPY;
    }

    if (params->opCode == OP_BIND) {
        params->baseParams.authIdPeer.length = 0;
        params->baseParams.authIdPeer.val = NULL;
    } else {
        authId = GetStringFromJson(in, FIELD_PEER_AUTH_ID);
        if (authId == NULL) {
            LOGE("get peer authId failed");
            return HC_ERROR;
        }
        authIdLen = strlen(authId);
        if (authIdLen == 0 || authIdLen > MAX_AUTH_ID_LEN) {
            LOGE("Invalid authIdPeerLen");
            return HC_ERR_INVALID_PARAMS;
        }
        params->baseParams.authIdPeer.length = authIdLen;
        params->baseParams.authIdPeer.val = (uint8_t *)HcMalloc(params->baseParams.authIdPeer.length, 0);
        if (params->baseParams.authIdPeer.val == NULL) {
            LOGE("malloc authIdPeer failed");
            return HC_ERROR;
        }
        if (memcpy_s(params->baseParams.authIdPeer.val, params->baseParams.authIdPeer.length,
            authId, strlen(authId)) != EOK) {
            LOGE("Memcpy authIdPeer failed.");
            return HC_ERR_MEMORY_COPY;
        }
    }

    return HC_SUCCESS;
}

static int AllocRandom(IsoParams *params)
{
    int res = HC_SUCCESS;
    params->baseParams.randSelf.length = RAND_BYTE_LEN;
    params->baseParams.randPeer.length = RAND_BYTE_LEN;
    params->baseParams.randSelf.val = (uint8_t *)HcMalloc(params->baseParams.randSelf.length, 0);
    if (params->baseParams.randSelf.val == NULL) {
        LOGE("malloc randSelf failed");
        return HC_ERR_ALLOC_MEMORY;
    }
    params->baseParams.randPeer.val = (uint8_t *)HcMalloc(params->baseParams.randPeer.length, 0);
    if (params->baseParams.randPeer.val == NULL) {
        LOGE("malloc randPeer failed");
        res = HC_ERR_ALLOC_MEMORY;
    }
    return res;
}

static int FillPkgNameAndServiceType(IsoParams *params, const CJson *in)
{
    const char *serviceType = GetStringFromJson(in, FIELD_SERVICE_TYPE);
    if (serviceType == NULL) {
        LOGE("get serviceType failed");
        return HC_ERROR;
    }
    params->serviceType = (char *)HcMalloc((uint32_t)(strlen(serviceType) + 1), 0);
    if (params->serviceType == NULL) {
        LOGE("malloc serviceType failed");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(params->serviceType, strlen(serviceType) + 1, serviceType, strlen(serviceType)) != EOK) {
        LOGE("memcpy serviceType failed.");
        return HC_ERR_MEMORY_COPY;
    }
    const char *packageName = GetStringFromJson(in, FIELD_PKG_NAME);
    if (packageName == NULL) {
        LOGE("get packageName failed");
        return HC_ERROR;
    }
    params->packageName = (char *)HcMalloc((uint32_t)(strlen(packageName) + 1), 0);
    if (params->packageName == NULL) {
        LOGE("malloc packageName failed");
        return HC_ERROR;
    }
    if (memcpy_s(params->packageName, strlen(packageName) + 1, packageName, strlen(packageName)) != EOK) {
        LOGE("memcpy packageName failed.");
        return HC_ERR_MEMORY_COPY;
    }
    return HC_SUCCESS;
}

static int FillPin(IsoParams *params, const CJson *in)
{
    if (params->opCode == OP_BIND) {
        const char *pinString = GetStringFromJson(in, FIELD_PIN_CODE);
        if (pinString == NULL) {
            LOGE("Get pin failed.");
            return HC_ERROR;
        }
        if (strlen(pinString) < MIN_PIN_LEN || strlen(pinString) > MAX_PIN_LEN) {
            LOGE("Pin is too short.");
            return HC_ERR_INVALID_PARAMS;
        }
        params->pinCodeString = (char *)HcMalloc(strlen(pinString) + 1, 0);
        if (params->pinCodeString == NULL) {
            LOGE("malloc pinCode failed.");
            return HC_ERR_ALLOC_MEMORY;
        }
        if (memcpy_s(params->pinCodeString, strlen(pinString) + 1, pinString, strlen(pinString)) != EOK) {
            LOGE("memcpy pinCodeString failed.");
            (void)memset_s(params->pinCodeString, strlen(pinString) + 1, 0, strlen(pinString) + 1);
            return HC_ERR_MEMORY_COPY;
        }
    }
    return HC_SUCCESS;
}

static int AllocSeed(IsoParams *params)
{
    params->seed.val = (uint8_t *)HcMalloc(SEED_LEN, 0);
    if (params->seed.val == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    params->seed.length = SEED_LEN;
    return HC_SUCCESS;
}

static int GetUserType(IsoParams *params, const CJson *in)
{
    int res = GetIntFromJson(in, FIELD_SELF_TYPE, &(params->selfUserType));
    if (res != 0) {
        LOGE("get userType failed: %d", res);
        return res;
    }

    res = GetIntFromJson(in, FIELD_PEER_USER_TYPE, &(params->peerUserType));
    if (res != 0) {
        LOGD("get peer Type failed use default, res: %d", res);
        params->peerUserType = 0; /* fill default value */
        res = HC_SUCCESS;
    }
    return res;
}

static int32_t GetKeyLength(IsoParams *params, const CJson *in)
{
    if (params->opCode == OP_UNBIND || params->opCode == OP_BIND) {
        params->keyLen = 0;
        return HC_SUCCESS;
    }
    int res = GetIntFromJson(in, FIELD_KEY_LENGTH, (int *)&(params->keyLen));
    if (res != HC_SUCCESS) {
        LOGD("get key length failed, use default, res: %d", res);
        params->keyLen = DEFAULT_RETURN_KEY_LENGTH;
    }
    if (params->keyLen < MIN_OUTPUT_KEY_LEN || params->keyLen > MAX_OUTPUT_KEY_LEN) {
        LOGE("Output key length is invalid.");
        return HC_ERR_INVALID_LEN;
    }
    return HC_SUCCESS;
}

int InitIsoParams(IsoParams *params, const CJson *in)
{
    int res = GetIntFromJson(in, FIELD_OPERATION_CODE, &(params->opCode));
    if (res != 0) {
        params->opCode = AUTHENTICATE;
    }
    res = GetBoolFromJson(in, FIELD_IS_CLIENT, &(params->isClient));
    if (res != HC_SUCCESS) {
        LOGE("get isClient failed");
        goto err;
    }
    res = GetKeyLength(params, in);
    if (res != 0) {
        goto err;
    }
    params->baseParams.sessionKey.length = ISO_SESSION_KEY_LEN;
    params->baseParams.loader = GetLoaderInstance();
    if (params->baseParams.loader == NULL) {
        res = HC_ERROR;
        goto err;
    }
    res = GetUserType(params, in);
    if (res != 0) {
        goto err;
    }
    res = FillAuthId(params, in);
    if (res != 0) {
        goto err;
    }
    res = AllocRandom(params);
    if (res != 0) {
        goto err;
    }
    res = FillPkgNameAndServiceType(params, in);
    if (res != 0) {
        goto err;
    }
    res = FillPin(params, in);
    if (res != 0) {
        goto err;
    }
    res = AllocSeed(params);
    if (res != 0) {
        goto err;
    }
    return HC_SUCCESS;
err:
    DestroyIsoParams(params);
    return res;
}

static int AuthGeneratePsk(const Uint8Buff *seed, IsoParams *params)
{
    uint8_t keyAlias[ISO_KEY_ALIAS_LEN] = { 0 };
    int res = GenerateKeyAliasInIso(params, keyAlias, sizeof(keyAlias), true);
    if (res != 0) {
        return res;
    }

    Uint8Buff keyAliasBuf = { keyAlias, sizeof(keyAlias) };
    Uint8Buff pskBuf = { params->baseParams.psk, sizeof(params->baseParams.psk) };
    return params->baseParams.loader->computeHmac(&keyAliasBuf, seed, &pskBuf, true);
}

static int AuthGeneratePskUsePin(const Uint8Buff *seed, IsoParams *params, const char *pinString)
{
    Uint8Buff messageBuf = { (uint8_t *)pinString, (uint32_t)strlen(pinString) };
    Uint8Buff pskBuf = { params->baseParams.psk, sizeof(params->baseParams.psk) };
    uint8_t hash[SHA256_LEN] = { 0 };
    Uint8Buff hashBuf = { hash, sizeof(hash) };
    int res = params->baseParams.loader->sha256(&messageBuf, &hashBuf);
    if (res != 0) {
        LOGE("sha256 failed, res:%d", res);
        return res;
    }
    return params->baseParams.loader->computeHmac(&hashBuf, seed, &pskBuf, false);
}

int GenerateKeyAliasInIso(const IsoParams *params, uint8_t *keyAlias, uint32_t keyAliasLen, bool useOpposite)
{
    if (params == NULL || keyAlias == NULL || keyAliasLen == 0) {
        return HC_ERR_INVALID_PARAMS;
    }
    Uint8Buff pkgName = { (uint8_t *)params->packageName, (uint32_t)strlen(params->packageName) };
    Uint8Buff serviceType = { (uint8_t *)params->serviceType, (uint32_t)strlen(params->serviceType) };
    Uint8Buff authId = { NULL, 0 };
    if (useOpposite) {
        authId.val = params->baseParams.authIdPeer.val;
        authId.length = params->baseParams.authIdPeer.length;
    } else {
        authId.val = params->baseParams.authIdSelf.val;
        authId.length = params->baseParams.authIdSelf.length;
    }
    Uint8Buff outKeyAlias = { keyAlias, keyAliasLen };
    return GenerateKeyAlias(&pkgName, &serviceType, KEY_ALIAS_AUTH_TOKEN, &authId,
        &outKeyAlias);
}

int GeneratePsk(const CJson *in, IsoParams *params)
{
    int res = 0;
    if (!params->isClient) {
        res = GetByteFromJson(in, FIELD_SEED, params->seed.val, params->seed.length);
    }
    if (res != 0) {
        LOGE("get seed failed.");
        return res;
    }
    if (params->opCode == AUTHENTICATE || params->opCode == OP_UNBIND) {
        return AuthGeneratePsk(&params->seed, params);
    }
    return AuthGeneratePskUsePin(&params->seed, params, params->pinCodeString);
}

int GenerateSeed(IsoParams *params)
{
    uint8_t *random = (uint8_t *)HcMalloc(SEED_LEN, 0);
    if (random == NULL) {
        LOGE("malloc random failed");
        return HC_ERR_ALLOC_MEMORY;
    }
    Uint8Buff randomBuf = { random, SEED_LEN };
    int res = params->baseParams.loader->generateRandom(&randomBuf);
    if (res != 0) {
        LOGE("generate random failed, res:%d", res);
        HcFree(random);
        return res;
    }
    clock_t times = 0;
    uint8_t *input = (uint8_t *)HcMalloc(SEED_LEN + sizeof(clock_t), 0);
    if (input == NULL) {
        LOGE("malloc failed");
        HcFree(random);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (memcpy_s(input, SEED_LEN + sizeof(clock_t), random, SEED_LEN) != EOK) {
        LOGE("memcpy seed failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    if (memcpy_s(input + SEED_LEN, sizeof(clock_t), &times, sizeof(clock_t)) != EOK) {
        LOGE("memcpy times failed.");
        res = HC_ERR_MEMORY_COPY;
        goto err;
    }
    Uint8Buff inputBuf = { input, SEED_LEN + sizeof(clock_t) };
    res = params->baseParams.loader->sha256(&inputBuf, &params->seed);
    if (res != HC_SUCCESS) {
        LOGE("sha256 failed.");
        goto err;
    }
err:
    HcFree(random);
    HcFree(input);
    return res;
}