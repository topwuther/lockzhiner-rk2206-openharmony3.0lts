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

#include "bind_session_server_lite.h"
#include "callback_manager.h"
#include "channel_manager.h"
#include "device_auth_defines.h"
#include "hc_log.h"
#include "session_manager.h"

static int32_t RequestConfirmation(const LiteBindSession *session, char **returnStr)
{
    char *returnDataStr = ProcessRequestCallback(session->requestId, session->operationCode, NULL,
        session->base.callback);
    if (returnDataStr == NULL) {
        LOGE("The OnRequest callback is fail!");
        return HC_ERR_REQ_REJECTED;
    }
    *returnStr = returnDataStr;
    LOGI("The server receives the data returned by the service!");
    return HC_SUCCESS;
}

static int32_t CombineInputData(const CJson *returnData, CJson *jsonParams)
{
    CJson *tempReturnData = DuplicateJson(returnData);
    if (tempReturnData == NULL) {
        LOGE("Failed to copy returnData!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddObjToJson(jsonParams, FIELD_RETURN_DATA, tempReturnData) != HC_SUCCESS) {
        LOGE("Failed to add returnData to jsonParams!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static bool IsAcceptRequest(const CJson *returnData)
{
    uint32_t confirmation = REQUEST_REJECTED;
    if (GetUnsignedIntFromJson(returnData, FIELD_CONFIRMATION, &confirmation) != HC_SUCCESS) {
        LOGE("Failed to get confirmation from returnData!");
        return false;
    }
    return (confirmation == REQUEST_ACCEPTED);
}

static int32_t LitePrepareServer(LiteBindSession *session, CJson *returnData, bool *isNeedInform)
{
    if ((session->isWaiting) && (!IsAcceptRequest(returnData))) {
        LOGE("The service rejects the request!");
        return HC_ERR_REQ_REJECTED;
    }
    int32_t result = CombineInputData(returnData, session->params);
    /* Release the memory in advance to reduce the memory usage. */
    DeleteAllItem(returnData);
    session->isWaiting = false;
    if (result != HC_SUCCESS) {
        return result;
    }
    return LitePrepareAndSendData(session, isNeedInform);
}

static int32_t JudgeConfirmation(CJson *returnData, CJson *jsonParams, LiteBindSession *session, bool *isNeedInform)
{
    uint32_t confirmation = REQUEST_ACCEPTED;
    if (GetUnsignedIntFromJson(returnData, FIELD_CONFIRMATION, &confirmation) != HC_SUCCESS) {
        LOGE("Failed to get confirmation from returnData!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result;
    switch (confirmation) {
        case REQUEST_WAITING:
            LOGI("The service wants us to wait for its signal!");
            result = LiteSaveReceivedData(session, jsonParams);
            session->isWaiting = true;
            return result;
        case REQUEST_ACCEPTED:
            LOGI("The service accepts the request!");
            result = LiteSaveReceivedData(session, jsonParams);
            if (result != HC_SUCCESS) {
                return result;
            }
            /* Release the memory in advance to reduce the memory usage. */
            DeleteAllItem(jsonParams);
            return LitePrepareServer(session, returnData, isNeedInform);
        case REQUEST_REJECTED:
            LOGE("The service rejects the request!");
            return HC_ERR_REQ_REJECTED;
        default:
            LOGE("Enter the exception case!");
            return HC_ERR_CASE;
    }
}

static int32_t HandleRequest(CJson *jsonParams, LiteBindSession *session, bool *isNeedInform)
{
    char *returnDataStr = NULL;
    int32_t result = RequestConfirmation(session, &returnDataStr);
    if (result != HC_SUCCESS) {
        return result;
    }

    CJson *returnData = CreateJsonFromString(returnDataStr);
    FreeJsonString(returnDataStr);
    if (returnData == NULL) {
        LOGE("Failed to create returnData from string!");
        return HC_ERR_JSON_FAIL;
    }
    result = JudgeConfirmation(returnData, jsonParams, session, isNeedInform);
    FreeJson(returnData);
    return result;
}

static void OnLiteBindConfirmationReceived(Session *session, CJson *returnData)
{
    if ((session == NULL) || (returnData == NULL)) {
        LOGE("The input session or returnData is NULL!");
        return;
    }
    LiteBindSession *realSession = (LiteBindSession *)session;

    bool isNeedInform = true;
    int32_t result = LitePrepareServer(realSession, returnData, &isNeedInform);
    if (result != HC_SUCCESS) {
        LOGI("An error occurs after the server receives the response to the request. We need to notify the service!");
        LiteInformPeerErrorIfNeed(isNeedInform, result, realSession);
        ProcessErrorCallback(realSession->requestId, realSession->operationCode, result, NULL,
            realSession->base.callback);
        CloseChannel(realSession->channelType, realSession->channelId);
        DestroySession(realSession->requestId);
    }
}

static void InitServerChannel(const CJson *jsonParams, LiteBindSession *session)
{
    int64_t channelId = DEFAULT_CHANNEL_ID;
    if (GetByteFromJson(jsonParams, FIELD_CHANNEL_ID, (uint8_t *)&channelId, sizeof(int64_t)) == HC_SUCCESS) {
        session->channelType = SOFT_BUS;
        session->channelId = channelId;
    } else {
        session->channelType = SERVICE_CHANNEL;
    }
}

Session *CreateLiteServerBindSession(CJson *jsonParams, const DeviceAuthCallback *callback)
{
    int64_t requestId = DEFAULT_REQUEST_ID;
    if (GetInt64FromJson(jsonParams, FIELD_REQUEST_ID, &requestId) != HC_SUCCESS) {
        LOGE("Failed to get requestId from jsonParams!");
        return NULL;
    }
    int operationCode = OP_BIND;
    (void)(GetIntFromJson(jsonParams, FIELD_OPERATION_CODE, &operationCode));
    LOGI("Start to create lite server bind session! [RequestId]: %" PRId64 ", [OperationCode]: %d",
        requestId, operationCode);

    LiteBindSession *session = (LiteBindSession *)HcMalloc(sizeof(LiteBindSession), 0);
    if (session == NULL) {
        LOGE("Failed to allocate session memory!");
        return NULL;
    }
    InitLiteBindSession(TYPE_SERVER_BIND_SESSION_LITE, operationCode, requestId, session, callback);
    InitServerChannel(jsonParams, session);
    /* The server may receive the confirm request message. */
    session->onConfirmationReceived = OnLiteBindConfirmationReceived;

    bool isNeedInform = true;
    int32_t result = HandleRequest(jsonParams, session, &isNeedInform);
    if (result != HC_SUCCESS) {
        LiteInformPeerErrorIfNeed(isNeedInform, result, session);
        ProcessErrorCallback(session->requestId, session->operationCode, result, NULL,
            session->base.callback);
        CloseChannel(session->channelType, session->channelId);
        DestroyLiteBindSession((Session *)session);
        return NULL;
    }
    LOGI("Create lite server bind session successfully! [RequestId]: %" PRId64 ", [OperationCode]: %d",
        requestId, operationCode);
    return (Session *)session;
}