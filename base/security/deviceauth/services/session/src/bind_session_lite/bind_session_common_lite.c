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

#include "bind_session_common_lite.h"
#include "callback_manager.h"
#include "channel_manager.h"
#include "das_module_defines.h"
#include "dev_auth_module_manager.h"
#include "hc_log.h"
#include "securec.h"
#include "session_common.h"

static int32_t SendLiteBindSessionData(const LiteBindSession *session, const CJson *sendData)
{
    char *sendDataStr = PackJsonToString(sendData);
    if (sendDataStr == NULL) {
        LOGE("An error occurred when converting json to string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = HcSendMsg(session->channelType, session->requestId,
        session->channelId, session->base.callback, sendDataStr);
    FreeJsonString(sendDataStr);
    return result;
}

static void InformPeerModuleErrorIfNeed(CJson *out, const LiteBindSession *session)
{
    CJson *errorData = GetObjFromJson(out, FIELD_SEND_TO_PEER);
    if (errorData == NULL) {
        return;
    }
    if (AddInt64StringToJson(errorData, FIELD_REQUEST_ID, session->requestId) != HC_SUCCESS) {
        LOGE("Failed to add requestId to errorData!");
        return;
    }
    int32_t result = SendLiteBindSessionData(session, errorData);
    if (result != HC_SUCCESS) {
        LOGE("An error occurred when notifying the peer service!");
        return;
    }
    LOGI("Succeeded in notifying the peer device that an error occurred at the local end!");
}

static int32_t ProcessModule(bool isAccountBind, const LiteBindSession *session, CJson *in, CJson *out, int *status)
{
    int moduleType = (isAccountBind ? TCIS_MODULE : DAS_MODULE);
    LOGI("Start to process %s task!", ((moduleType) == TCIS_MODULE ? "TCIS module" : "DAS module"));
    if (isAccountBind) {
        CJson *receivedData = GetObjFromJson(in, FIELD_RECEIVED_DATA);
        AddIntToJson(receivedData, FIELD_OPERATION_CODE, OP_BIND);
    }
    int32_t result = ProcessTask(session->curTaskId, in, out, status, moduleType);
    if (result != HC_SUCCESS) {
        LOGE("An error occurs when the module processes task! [ErrorCode]: %d", result);
        return result;
    }
    LOGI("Process %s task successfully!", ((moduleType) == TCIS_MODULE ? "TCIS module" : "DAS module"));
    return HC_SUCCESS;
}

static int32_t CreateAndProcessModule(bool isAccountBind, LiteBindSession *session, CJson *in, CJson *out)
{
    int status = 0;
    int moduleType = (isAccountBind ? TCIS_MODULE : DAS_MODULE);
    LOGI("Start to create and process %s task!", ((moduleType) == TCIS_MODULE ? "TCIS module" : "DAS module"));
    if (isAccountBind) {
        CJson *receivedData = GetObjFromJson(in, FIELD_RECEIVED_DATA);
        AddIntToJson(receivedData, FIELD_OPERATION_CODE, OP_BIND);
    }
    int32_t result = CreateTask(&(session->curTaskId), in, out, moduleType);
    if (result != HC_SUCCESS) {
        LOGE("An error occurs when creating a module task! [ErrorCode]: %d", result);
        return result;
    }
    result = ProcessTask(session->curTaskId, in, out, &status, moduleType);
    if (result != HC_SUCCESS) {
        LOGE("An error occurs when the module processes task! [ErrorCode]: %d", result);
        return result;
    }
    LOGI("Create and process %s task successfully!", ((moduleType) == TCIS_MODULE ? "TCIS module" : "DAS module"));
    return HC_SUCCESS;
}

static int32_t OnSessionFinish(bool isAccountBind, const LiteBindSession *session, CJson *out)
{
    int32_t result;
    CJson *sendData = GetObjFromJson(out, FIELD_SEND_TO_PEER);
    /* The last packet may need to be sent */
    if (sendData != NULL) {
        if (isAccountBind && (AddIntToJson(sendData, FIELD_GROUP_OP, ACCOUNT_BIND) != HC_SUCCESS)) {
            LOGE("Failed to add groupOp to sendData!");
            return HC_ERR_JSON_FAIL;
        }
        result = SendLiteBindSessionData(session, sendData);
        if (result != HC_SUCCESS) {
            return result;
        }
    }
    uint8_t sessionKey[DEFAULT_RETURN_KEY_LENGTH] = { 0 };
    if (GetByteFromJson(out, FIELD_SESSION_KEY, sessionKey, DEFAULT_RETURN_KEY_LENGTH) == HC_SUCCESS) {
        ProcessSessionKeyCallback(session->requestId, sessionKey, DEFAULT_RETURN_KEY_LENGTH, session->base.callback);
        (void)memset_s(sessionKey, DEFAULT_RETURN_KEY_LENGTH, 0, DEFAULT_RETURN_KEY_LENGTH);
        ClearSensitiveStringInJson(out, FIELD_SESSION_KEY);
    }
    const CJson *returnData = GetObjFromJson(out, FIELD_SEND_TO_SELF);
    char *returnDataStr = PackJsonToString(returnData);
    ProcessFinishCallback(session->requestId, session->operationCode, returnDataStr, session->base.callback);
    FreeJsonString(returnDataStr);
    LOGI("The device is successfully %s!", ((session->operationCode == OP_BIND) ? "bound" : "unbound"));
    SetAuthResult(session->channelType, session->channelId);
    CloseChannel(session->channelType, session->channelId);
    return HC_SUCCESS;
}

static int32_t CheckPeerStatus(const CJson *jsonParams, bool *isNeedInform)
{
    int32_t errorCode = 0;
    if (GetIntFromJson(jsonParams, FIELD_GROUP_ERROR_MSG, &errorCode) == HC_SUCCESS) {
        LOGE("An error occurs in the peer service! [ErrorCode]: %d", errorCode);
        *isNeedInform = false;
        return errorCode;
    }
    return HC_SUCCESS;
}

static CJson *GenerateGroupErrorMsg(int32_t errorCode, int64_t requestId)
{
    CJson *errorData = CreateJson();
    if (errorData == NULL) {
        LOGE("Failed to allocate errorData memory!");
        return NULL;
    }
    if (AddIntToJson(errorData, FIELD_GROUP_ERROR_MSG, errorCode) != HC_SUCCESS) {
        LOGE("Failed to add errorCode to errorData!");
        FreeJson(errorData);
        return NULL;
    }
    if (AddInt64StringToJson(errorData, FIELD_REQUEST_ID, requestId) != HC_SUCCESS) {
        LOGE("Failed to add requestId to errorData!");
        FreeJson(errorData);
        return NULL;
    }
    return errorData;
}

static int32_t ProcessLiteBindSessionInner(LiteBindSession *session, CJson *jsonParams, int32_t *status,
    bool *isNeedInform)
{
    int32_t result = CheckPeerStatus(jsonParams, isNeedInform);
    if (result != HC_SUCCESS) {
        return result;
    }

    CJson *out = CreateJson();
    if (out == NULL) {
        LOGE("Failed to allocate out memory!");
        return HC_ERR_JSON_FAIL;
    }

    bool isAccountBind = false;
    (void)GetBoolFromJson(jsonParams, FIELD_IS_ACCOUNT_BIND, &isAccountBind);
    result = ProcessModule(isAccountBind, session, jsonParams, out, status);
    DeleteAllItem(jsonParams);
    if (result != HC_SUCCESS) {
        *isNeedInform = false;
        InformPeerModuleErrorIfNeed(out, session);
        FreeJson(out);
        return result;
    }

    if (*status == IGNORE_MSG) {
        FreeJson(out);
        return HC_SUCCESS;
    } else if (*status == CONTINUE) {
        CJson *sendData = DetachItemFromJson(out, FIELD_SEND_TO_PEER);
        FreeJson(out);
        if (sendData == NULL) {
            LOGE("Failed to get sendToPeer from out!");
            return HC_ERR_JSON_GET;
        }
        if (isAccountBind && (AddIntToJson(sendData, FIELD_GROUP_OP, ACCOUNT_BIND) != HC_SUCCESS)) {
            LOGE("Failed to add groupOp to sendData!");
            FreeJson(sendData);
            return HC_ERR_JSON_FAIL;
        }
        result = SendLiteBindSessionData(session, sendData);
        FreeJson(sendData);
        return result;
    }

    result = OnSessionFinish(isAccountBind, session, out);
    FreeJson(out);
    return result;
}

static int32_t ProcessLiteBindSession(Session *session, CJson *jsonParams)
{
    if ((session == NULL) || (jsonParams == NULL)) {
        LOGE("The input session or jsonParams is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }

    LiteBindSession *realSession = (LiteBindSession *)session;

    bool isNeedInform = true;
    int32_t status = CONTINUE;
    int32_t result = ProcessLiteBindSessionInner(realSession, jsonParams, &status, &isNeedInform);
    if (result != HC_SUCCESS) {
        LOGE("An error occurs during processing lite bind session. We need to notify the service!");
        LiteInformPeerErrorIfNeed(isNeedInform, result, realSession);
        ProcessErrorCallback(realSession->requestId, realSession->operationCode, result, NULL,
            realSession->base.callback);
        CloseChannel(realSession->channelType, realSession->channelId);
        return result;
    }
    if (status == FINISH) {
        return status;
    }
    return result;
}

static int32_t LitePrepareData(LiteBindSession *session, CJson **sendData, bool *isNeedInform)
{
    CJson *out = CreateJson();
    if (out == NULL) {
        LOGE("Failed to allocate out memory!");
        return HC_ERR_JSON_FAIL;
    }

    bool isAccountBind = false;
    (void)GetBoolFromJson(session->params, FIELD_IS_ACCOUNT_BIND, &isAccountBind);
    int32_t result = CreateAndProcessModule(isAccountBind, session, session->params, out);
    if (result != HC_SUCCESS) {
        *isNeedInform = false;
        InformPeerModuleErrorIfNeed(out, session);
        FreeJson(out);
        return result;
    }

    DeleteAllItem(session->params);
    *sendData = DetachItemFromJson(out, FIELD_SEND_TO_PEER);
    FreeJson(out);
    if (*sendData == NULL) {
        LOGE("Failed to get sendToPeer from out!");
        return HC_ERR_JSON_GET;
    }
    if (isAccountBind && (AddIntToJson(*sendData, FIELD_GROUP_OP, ACCOUNT_BIND) != HC_SUCCESS)) {
        LOGE("Failed to add groupOp to sendData!");
        FreeJson(*sendData);
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

int32_t LitePrepareAndSendData(LiteBindSession *session, bool *isNeedInform)
{
    CJson *sendData = NULL;
    int32_t result = LitePrepareData(session, &sendData, isNeedInform);
    if (result != HC_SUCCESS) {
        return result;
    }

    result = SendLiteBindSessionData(session, sendData);
    FreeJson(sendData);
    return result;
}

void DestroyLiteBindSession(Session *session)
{
    if (session == NULL) {
        return;
    }
    LiteBindSession *realSession = (LiteBindSession *)session;
    DestroyTask(realSession->curTaskId, DAS_MODULE);
    FreeJson(realSession->params);
    realSession->params = NULL;
    HcFree(realSession);
    realSession = NULL;
}

void LiteInformPeerErrorIfNeed(bool isNeedInform, int32_t errorCode, LiteBindSession *session)
{
    if (!isNeedInform) {
        return;
    }
    CJson *errorData = GenerateGroupErrorMsg(errorCode, session->requestId);
    if (errorData == NULL) {
        return;
    }
    int32_t result = SendLiteBindSessionData(session, errorData);
    FreeJson(errorData);
    if (result != HC_SUCCESS) {
        LOGE("An error occurred when notifying the peer service!");
        return;
    }
    LOGI("Succeeded in notifying the peer device that an error occurred at the local end!");
}

int32_t LiteSaveReceivedData(LiteBindSession *session, const CJson *jsonParams)
{
    if (session->params == NULL) {
        session->params = CreateJson();
        if (session->params == NULL) {
            LOGE("Failed to allocate session params memory!");
            return HC_ERR_ALLOC_MEMORY;
        }
    }
    if (AddObjToJson(session->params, FIELD_RECEIVED_DATA, jsonParams) != HC_SUCCESS) {
        LOGE("Failed to add received data to session params!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

void InitLiteBindSession(int bindType, int operationCode, int64_t requestId, LiteBindSession *session,
    const DeviceAuthCallback *callback)
{
    if (session == NULL) {
        LOGE("The input session is NULL!");
        return;
    }
    session->base.process = ProcessLiteBindSession;
    session->base.destroy = DestroyLiteBindSession;
    session->curTaskId = 0;
    session->base.callback = callback;
    int res = GenerateSessionOrTaskId(&session->base.sessionId);
    if (res != 0) {
        return;
    };
    session->base.type = bindType;
    session->operationCode = operationCode;
    session->requestId = requestId;
    session->channelType = NO_CHANNEL;
    session->channelId = DEFAULT_CHANNEL_ID;
    session->isWaiting = false;
    session->params = NULL;
    session->onChannelOpened = NULL;
    session->onConfirmationReceived = NULL;
}