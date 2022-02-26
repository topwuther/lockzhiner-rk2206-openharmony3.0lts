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

#include "iso_client_unbind_exchange_task.h"
#include "hc_log.h"
#include "hc_types.h"
#include "iso_task_common.h"
#include "securec.h"

enum {
    TASK_TYPE_BEGIN = 1,
    TASK_TYPE_FINAL,
};

static CurTaskType GetTaskType(void)
{
    return TASK_TYPE_UNBIND_LITE_EXCHANGE;
}

static void DestroyClientUnbindExchangeTask(struct SymBaseCurTaskT *task)
{
    IsoClientUnbindExchangeTask *realTask = (IsoClientUnbindExchangeTask *)task;
    HcFree(realTask);
}

static int Process(struct SymBaseCurTaskT *task, IsoParams *params, const CJson *in, CJson *out, int *status)
{
    IsoClientUnbindExchangeTask *realTask = (IsoClientUnbindExchangeTask *)task;
    if (realTask->taskBase.taskStatus < TASK_TYPE_BEGIN) {
        LOGE("taskStatus err %d", realTask->taskBase.taskStatus);
        return HC_ERR_BAD_MESSAGE;
    }

    if (realTask->taskBase.taskStatus > TASK_TYPE_BEGIN) {
        LOGI("The message is repeated, ignore it, status :%d", realTask->taskBase.taskStatus);
        *status = IGNORE_MSG;
        return HC_SUCCESS;
    }
    int res;
    uint8_t *keyAlias = NULL;
    int message = 0;
    res = GetIntFromJson(in, FIELD_MESSAGE, &message);
    if (res != 0 || message != ISO_SERVER_UNBIND_EXCHANGE_RET) {
        return HC_ERR_BAD_MESSAGE;
    }
    res = CheckEncResult(params, in, UNBIND_ADD_RESPONSE);
    if (res != 0) {
        LOGE("CheckEncResult failed, res:%d", res);
        return res;
    }
    keyAlias = (uint8_t *)HcMalloc(ISO_KEY_ALIAS_LEN, 0);
    if (keyAlias == NULL) {
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    res = GenerateKeyAliasInIso(params, keyAlias, ISO_KEY_ALIAS_LEN, true);
    if (res != 0) {
        LOGE("GenerateKeyAliasInIso failed, res:%d", res);
        goto err;
    }
    Uint8Buff outKeyAlias = { (uint8_t *)keyAlias, ISO_KEY_ALIAS_LEN };
    res = params->baseParams.loader->deleteKey(&outKeyAlias);
    if (res != 0) {
        LOGE("delete auth code failed, res:%d", res);
        goto err;
    }
    res = SendResultToFinalSelf(params, out, false);
    if (res == 0) {
        realTask->taskBase.taskStatus = TASK_TYPE_FINAL;
        *status = FINISH;
    }
err:
    HcFree(keyAlias);
    return res;
}

static int PackDataForStartUnbind(const IsoParams *params, const Uint8Buff *encDataBuf, const Uint8Buff *nonceBuf,
    CJson *out)
{
    int res = 0;
    CJson *payload = NULL;
    CJson *sendToPeer = NULL;

    payload = CreateJson();
    if (payload == NULL) {
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    sendToPeer = CreateJson();
    if (sendToPeer == NULL) {
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    GOTO_ERR_AND_SET_RET(AddIntToJson(sendToPeer, FIELD_MESSAGE, ISO_CLIENT_UNBIND_EXCHANGE_CMD), res);
    GOTO_ERR_AND_SET_RET(AddIntToJson(payload, FIELD_OPERATION_CODE, params->opCode), res);
    GOTO_ERR_AND_SET_RET(AddByteToJson(payload, FIELD_ENC_DATA, encDataBuf->val, encDataBuf->length), res);
    GOTO_ERR_AND_SET_RET(AddByteToJson(payload, FIELD_NONCE, nonceBuf->val, nonceBuf->length), res);
    GOTO_ERR_AND_SET_RET(AddObjToJson(sendToPeer, FIELD_PAYLOAD, payload), res);
    GOTO_ERR_AND_SET_RET(AddObjToJson(out, FIELD_SEND_TO_PEER, sendToPeer), res);
err:
    FreeJson(payload);
    FreeJson(sendToPeer);
    return res;
}

static int GenerateEncRemoveInfo(const IsoParams *params, const Uint8Buff *nonceBuf, CJson *out)
{
    int res;
    char *rmvInfoStr = NULL;
    uint8_t *encData = NULL;
    CJson *rmvInfoJson = CreateJson();
    if (rmvInfoJson == NULL) {
        LOGE("Create json failed");
        return HC_ERR_ALLOC_MEMORY;
    }
    GOTO_ERR_AND_SET_RET(AddByteToJson(rmvInfoJson, FIELD_RMV_ID, params->baseParams.authIdSelf.val,
        params->baseParams.authIdSelf.length), res);
    GOTO_ERR_AND_SET_RET(AddIntToJson(rmvInfoJson, FIELD_RMV_TYPE, params->selfUserType), res);
    rmvInfoStr = PackJsonToString(rmvInfoJson);
    if (rmvInfoStr == NULL) {
        LOGE("rmvInfoStr PackJsonToString failed");
        res = HC_ERR_PACKAGE_JSON_TO_STRING_FAIL;
        goto err;
    }

    Uint8Buff removeInfoBuf = { (uint8_t *)rmvInfoStr, strlen(rmvInfoStr) };

    int encDataLen = strlen(rmvInfoStr) + TAG_LEN;
    encData = (uint8_t *)HcMalloc(encDataLen, 0);
    if (encData == NULL) {
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    Uint8Buff encDataBuf = { encData, encDataLen };
    GcmParam gcmParam;
    gcmParam.aad = (uint8_t *)UNBIND_ADD_REQUEST;
    gcmParam.aadLen = (uint32_t)strlen(UNBIND_ADD_REQUEST);
    gcmParam.nonce = nonceBuf->val;
    gcmParam.nonceLen = nonceBuf->length;
    res = params->baseParams.loader->aesGcmEncrypt(&params->baseParams.sessionKey, &removeInfoBuf, &gcmParam, false,
        &encDataBuf);
    if (res != 0) {
        LOGE("encrypt removeInfo failed, res:%d", res);
        goto err;
    }
    res = PackDataForStartUnbind(params, &encDataBuf, nonceBuf, out);
    if (res != HC_SUCCESS) {
        LOGE("PackDataForStartUnbind failed, res:%d", res);
    }
err:
    FreeJson(rmvInfoJson);
    FreeJsonString(rmvInfoStr);
    HcFree(encData);
    return res;
}


static int ClientUnbindExchangeStart(const IsoParams *params, IsoClientUnbindExchangeTask *task, CJson *out,
    int *status)
{
    int res;
    uint8_t *nonce = NULL;

    nonce = (uint8_t *)HcMalloc(NONCE_SIZE, 0);
    if (nonce == NULL) {
        res = HC_ERR_ALLOC_MEMORY;
        goto err;
    }
    Uint8Buff nonceBuf = { nonce, NONCE_SIZE };
    res = params->baseParams.loader->generateRandom(&nonceBuf);
    if (res != 0) {
        LOGE("generate nonce failed, res:%d", res);
        goto err;
    }

    res = GenerateEncRemoveInfo(params, &nonceBuf, out);
    if (res != 0) {
        LOGE("GenerateEncRemoveInfo failed, res:%d", res);
        goto err;
    }
    task->taskBase.taskStatus = TASK_TYPE_BEGIN;
    *status = CONTINUE;
err:
    HcFree(nonce);
    return res;
}

SymBaseCurTask *CreateClientUnbindExchangeTask(IsoParams *params, const CJson *in, CJson *out, int *status)
{
    (void)in;
    IsoClientUnbindExchangeTask *task = (IsoClientUnbindExchangeTask *)HcMalloc(sizeof(IsoClientUnbindExchangeTask), 0);
    if (task == NULL) {
        return NULL;
    }
    task->taskBase.destroyTask = DestroyClientUnbindExchangeTask;
    task->taskBase.process = Process;
    task->taskBase.getCurTaskType = GetTaskType;
    int res = ClientUnbindExchangeStart(params, task, out, status);
    if (res != 0) {
        DestroyClientUnbindExchangeTask((struct SymBaseCurTaskT *)task);
        return NULL;
    }
    return (SymBaseCurTask *)task;
}