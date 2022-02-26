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

#include "device_auth.h"

#include "alg_loader.h"
#include "callback_manager.h"
#include "channel_manager.h"
#include "common_util.h"
#include "dev_auth_module_manager.h"
#include "device_auth_defines.h"
#include "group_auth_manager.h"
#include "group_manager.h"
#include "hc_init_protection.h"
#include "hc_log.h"
#include "json_utils.h"
#include "securec.h"
#include "session_manager.h"
#include "task_manager.h"

typedef struct {
    HcTaskBase base;
    int64_t requestId;
    CJson *jsonParams;
    const DeviceAuthCallback *callback;
} GroupManagerTask;

static GroupAuthManager *g_groupAuthManager =  NULL;
static DeviceGroupManager *g_groupManagerInstance = NULL;

static int32_t BindCallbackToTask(GroupManagerTask *task, const CJson *jsonParams)
{
    const char *appId = GetStringFromJson(jsonParams, FIELD_APP_ID);
    if (appId == NULL) {
        LOGE("Failed to get appId from jsonParams!");
        return HC_ERR_JSON_GET;
    }
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    task->callback = instance->getGmCallbackByAppId(appId);
    if (task->callback == NULL) {
        LOGE("Failed to find callback by appId!");
        return HC_ERR_CALLBACK_NOT_FOUND;
    }
    LOGI("Bind service callback to task successfully! [AppId]: %s", appId);
    return HC_SUCCESS;
}

static void DoCreateGroup(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute group creation! [RequestId]: %" PRId64, realTask->requestId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return;
    }
    instance->createPeerToPeerGroup(realTask->requestId, realTask->jsonParams, realTask->callback);
}

static void DoDeleteGroup(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute group deletion! [RequestId]: %" PRId64, realTask->requestId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return;
    }
    instance->deletePeerToPeerGroup(realTask->requestId, realTask->jsonParams, realTask->callback);
}

static void DoAddMember(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute member addition! [RequestId]: %" PRId64, realTask->requestId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return;
    }
    instance->addMemberToPeerToPeerGroup(realTask->requestId, realTask->jsonParams, realTask->callback);
}

static void DoDeleteMember(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute member deletion! [RequestId]: %" PRId64, realTask->requestId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return;
    }
    instance->deleteMemberFromPeerToPeerGroup(realTask->requestId, realTask->jsonParams, realTask->callback);
}

static void DoConfirmRequest(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute request confirmation! [RequestId]: %" PRId64, realTask->requestId);
    OnConfirmationReceived(realTask->requestId, realTask->jsonParams);
}

static void DoProcessBindData(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    if (IsRequestExist(realTask->requestId)) {
        LOGI("The task thread starts to process data! [RequestId]: %" PRId64, realTask->requestId);
        int ret = ProcessSession(realTask->requestId, BIND_TYPE, realTask->jsonParams);
        if (ret != CONTINUE) {
            DestroySession(realTask->requestId);
        }
        return;
    }
    LOGI("The task thread starts to create a server binding session! [RequestId]: %" PRId64, realTask->requestId);
    if ((BindCallbackToTask(realTask, realTask->jsonParams) != HC_SUCCESS) ||
        (CheckMsgRepeatability(realTask->jsonParams, DAS_MODULE) != HC_SUCCESS)) {
        return;
    }
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return;
    }
    instance->processBindData(realTask->requestId, realTask->jsonParams, realTask->callback);
}

static void DestroyGroupAuthTask(HcTaskBase *task)
{
    AuthDeviceTask *realTask = (AuthDeviceTask *)task;
    FreeJson(realTask->authParams);
}

static void DestroyGroupManagerTask(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    FreeJson(realTask->jsonParams);
}

static bool InitAuthDeviceTask(AuthDeviceTask *task, int64_t authReqId, CJson *authParams,
    const DeviceAuthCallback *gaCallback)
{
    task->base.doAction = DoAuthDevice;
    task->base.destroy = DestroyGroupAuthTask;
    task->authReqId = authReqId;
    if (AddByteToJson(authParams, FIELD_REQUEST_ID, (const uint8_t*)&authReqId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to add requestId to json!");
        return false;
    }
    task->authParams = authParams;
    task->callback = gaCallback;
    if (task->callback == NULL) {
        LOGE("The input auth callback is null!");
        return false;
    }
    return true;
}

static bool InitProcessDataTask(AuthDeviceTask *task, int64_t authReqId, CJson *receivedData,
    const DeviceAuthCallback *gaCallback)
{
    task->base.doAction = DoProcessData;
    task->base.destroy = DestroyGroupAuthTask;
    task->authReqId = authReqId;
    if (AddByteToJson(receivedData, FIELD_REQUEST_ID, (const uint8_t*)&authReqId, sizeof(int64_t)) != HC_SUCCESS) {
        LOGE("Failed to add requestId to json!");
        return false;
    }
    task->authParams = receivedData;
    task->callback = gaCallback;
    if (task->callback == NULL) {
        LOGE("The input auth callback is null!");
        return false;
    }
    return true;
}

static void InitProcessBindDataTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoProcessBindData;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
}

static int32_t InitCreateGroupTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoCreateGroup;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return BindCallbackToTask(task, jsonParams);
}

static int32_t InitDeleteGroupTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoDeleteGroup;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return BindCallbackToTask(task, jsonParams);
}

static int32_t InitAddMemberTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoAddMember;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return BindCallbackToTask(task, jsonParams);
}

static int32_t InitDeleteMemberTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoDeleteMember;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return BindCallbackToTask(task, jsonParams);
}

static int32_t InitConfirmRequestTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoConfirmRequest;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return BindCallbackToTask(task, jsonParams);
}

static int32_t AuthDevice(int64_t authReqId, const char *authParams, const DeviceAuthCallback *gaCallback)
{
    LOGI("Begin AuthDevice.");
    if (authParams == NULL) {
        LOGE("The input auth params is null!");
        return HC_ERR_INVALID_PARAMS;
    }
    CJson *jsonParams = CreateJsonFromString(authParams);
    if (jsonParams == NULL) {
        LOGE("Create json from params failed!");
        return HC_ERR_JSON_FAIL;
    }
    AuthDeviceTask *task = (AuthDeviceTask *)HcMalloc(sizeof(AuthDeviceTask), 0);
    if (task == NULL) {
        FreeJson(jsonParams);
        LOGE("Failed to allocate memory for task!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (!InitAuthDeviceTask(task, authReqId, jsonParams, gaCallback)) {
        LOGE("Failed to init task!");
        FreeJson(jsonParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(jsonParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("Push AuthDevice task successfully.");
    return HC_SUCCESS;
}

static int32_t ProcessData(int64_t authReqId, const uint8_t *data, uint32_t dataLen,
    const DeviceAuthCallback *gaCallback)
{
    LOGI("Begin ProcessData.");
    if ((data == NULL) || (dataLen > MAX_DATA_BUFFER_SIZE)) {
        LOGE("Invalid input for ProcessData!");
        return HC_ERR_INVALID_PARAMS;
    }
    CJson *receivedData = CreateJsonFromString((const char *)data);
    if (receivedData == NULL) {
        LOGE("Create Json for input data failed!");
        return HC_ERR_JSON_FAIL;
    }
    AuthDeviceTask *task = (AuthDeviceTask *)HcMalloc(sizeof(AuthDeviceTask), 0);
    if (task == NULL) {
        FreeJson(receivedData);
        LOGE("Failed to allocate memory for task!");
        return HC_ERR_ALLOC_MEMORY;
    }
    if (!InitProcessDataTask(task, authReqId, receivedData, gaCallback)) {
        LOGE("Failed to init task!");
        FreeJson(receivedData);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(receivedData);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("Push ProcessData task successfully.");
    return HC_SUCCESS;
}

static int GetOperationCodeWhenAdd(CJson *jsonParams)
{
    bool isAdmin = true;
    /* The isAdmin parameter is optional. Default value is true. */
    (void)GetBoolFromJson(jsonParams, FIELD_IS_ADMIN, &isAdmin);
    /* Release the memory in advance to reduce the memory usage. */
    (void)DeleteItemFromJson(jsonParams, FIELD_IS_ADMIN);
    return isAdmin ? MEMBER_INVITE : MEMBER_JOIN;
}

static int32_t AddReqInfoToJson(int64_t requestId, const char *appId, CJson *jsonParams)
{
    if (AddInt64StringToJson(jsonParams, FIELD_REQUEST_ID, requestId) != HC_SUCCESS) {
        LOGE("Failed to add requestId to json!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(jsonParams, FIELD_APP_ID, appId) != HC_SUCCESS) {
        LOGE("Failed to add appId to json!");
        return HC_ERR_JSON_FAIL;
    }
    return HC_SUCCESS;
}

static int32_t AddBindParamsToJson(int operationCode, int64_t requestId, const char *appId, CJson *jsonParams)
{
    if (AddIntToJson(jsonParams, FIELD_OPERATION_CODE, operationCode) != HC_SUCCESS) {
        LOGE("Failed to add operationCode to json!");
        return HC_ERR_JSON_FAIL;
    }
    return AddReqInfoToJson(requestId, appId, jsonParams);
}

static int32_t AddClientParamsToJson(bool isClient, int operationCode, int64_t requestId, const char *appId,
    CJson *jsonParams)
{
    if (AddBoolToJson(jsonParams, FIELD_IS_CLIENT, isClient) != HC_SUCCESS) {
        LOGE("Failed to add isClient to json!");
        return HC_ERR_JSON_FAIL;
    }
    return AddBindParamsToJson(operationCode, requestId, appId, jsonParams);
}

static int32_t AddServerParamsToJson(bool isClient, int64_t requestId, const char *appId, CJson *jsonParams)
{
    if (AddBoolToJson(jsonParams, FIELD_IS_CLIENT, isClient) != HC_SUCCESS) {
        LOGE("Failed to add isClient to json!");
        return HC_ERR_JSON_FAIL;
    }
    return AddReqInfoToJson(requestId, appId, jsonParams);
}

static int32_t RegGroupManagerCallback(const char *appId, const DeviceAuthCallback *callback)
{
    if ((appId == NULL) || (callback == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: Service register GM callback! [AppId]: %s", appId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->regGroupManagerCallback(appId, callback);
}

static int32_t UnRegGroupManagerCallback(const char *appId)
{
    if (appId == NULL) {
        LOGE("The input appId is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: Service deregister GM callback! [AppId]: %s", appId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->unRegGroupManagerCallback(appId);
}

static int32_t RequestCreateGroup(int64_t requestId, const char *appId, const char *createParams)
{
    if ((appId == NULL) || (createParams == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestCreateGroup! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *jsonCreateParams = CreateJsonFromString(createParams);
    if (jsonCreateParams == NULL) {
        LOGE("Failed to create json from string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = AddBindParamsToJson(GROUP_CREATE, requestId, appId, jsonCreateParams);
    if (result != HC_SUCCESS) {
        FreeJson(jsonCreateParams);
        return result;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(jsonCreateParams);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitCreateGroupTask(task, requestId, jsonCreateParams) != HC_SUCCESS) {
        FreeJson(jsonCreateParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(jsonCreateParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the creating group task successfully! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    return HC_SUCCESS;
}

static int32_t RequestDeleteGroup(int64_t requestId, const char *appId, const char *disbandParams)
{
    if ((appId == NULL) || (disbandParams == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestDeleteGroup! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *jsonDisBandParams = CreateJsonFromString(disbandParams);
    if (jsonDisBandParams == NULL) {
        LOGE("Failed to create json from string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = AddBindParamsToJson(GROUP_DISBAND, requestId, appId, jsonDisBandParams);
    if (result != HC_SUCCESS) {
        FreeJson(jsonDisBandParams);
        return result;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(jsonDisBandParams);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitDeleteGroupTask(task, requestId, jsonDisBandParams) != HC_SUCCESS) {
        FreeJson(jsonDisBandParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(jsonDisBandParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the deleting group task successfully! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    return HC_SUCCESS;
}

static int32_t RequestAddMemberToGroup(int64_t requestId, const char *appId, const char *addParams)
{
    if ((appId == NULL) || (addParams == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestAddMemberToGroup! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *jsonAddParams = CreateJsonFromString(addParams);
    if (jsonAddParams == NULL) {
        LOGE("Failed to create json from string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = AddBindParamsToJson(GetOperationCodeWhenAdd(jsonAddParams), requestId, appId, jsonAddParams);
    if (result != HC_SUCCESS) {
        FreeJson(jsonAddParams);
        return result;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(jsonAddParams);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitAddMemberTask(task, requestId, jsonAddParams) != HC_SUCCESS) {
        FreeJson(jsonAddParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(jsonAddParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the adding member task successfully! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    return HC_SUCCESS;
}

static int32_t RequestDeleteMemberFromGroup(int64_t requestId, const char *appId, const char *deleteParams)
{
    if ((appId == NULL) || (deleteParams == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestDeleteMemberFromGroup! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *jsonDeleteParams = CreateJsonFromString(deleteParams);
    if (jsonDeleteParams == NULL) {
        LOGE("Failed to create json from string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = AddBindParamsToJson(MEMBER_DELETE, requestId, appId, jsonDeleteParams);
    if (result != HC_SUCCESS) {
        FreeJson(jsonDeleteParams);
        return result;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(jsonDeleteParams);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitDeleteMemberTask(task, requestId, jsonDeleteParams) != HC_SUCCESS) {
        FreeJson(jsonDeleteParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(jsonDeleteParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the deleting member task successfully! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    return HC_SUCCESS;
}

static int32_t RequestProcessBindData(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    if ((data == NULL) || (dataLen == 0) || (dataLen > MAX_DATA_BUFFER_SIZE)) {
        LOGE("The input data is invalid!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestProcessBindData! [RequestId]: %" PRId64, requestId);
    CJson *dataJson = CreateJsonFromString((const char *)data);
    if (dataJson == NULL) {
        LOGE("Failed to create json from string!");
        return HC_ERR_JSON_FAIL;
    }
    int64_t tempRequestId = DEFAULT_REQUEST_ID;
    if (GetInt64FromJson(dataJson, FIELD_REQUEST_ID, &tempRequestId) != HC_SUCCESS) {
        LOGE("Failed to get requestId from json!");
        FreeJson(dataJson);
        return HC_ERR_JSON_GET;
    }
    if (tempRequestId != requestId) {
        LOGE("The requestId transferred by the service is inconsistent with that in the packet! "
            "[ServiceRequestId]: %" PRId64 ", [RequestId]: %" PRId64, requestId, tempRequestId);
        FreeJson(dataJson);
        return HC_ERR_INVALID_PARAMS;
    }
    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(dataJson);
        return HC_ERR_ALLOC_MEMORY;
    }
    InitProcessBindDataTask(task, requestId, dataJson);
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(dataJson);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the processing data task successfully! [RequestId]: %" PRId64, requestId);
    return HC_SUCCESS;
}

static int32_t ConfirmRequest(int64_t requestId, const char *appId, const char *confirmParams)
{
    if ((appId == NULL) || (confirmParams == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: ConfirmRequest! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *jsonConfirmParams = CreateJsonFromString(confirmParams);
    if (jsonConfirmParams == NULL) {
        LOGE("Failed to create json from string!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddStringToJson(jsonConfirmParams, FIELD_APP_ID, appId) != HC_SUCCESS) {
        LOGE("Failed to add appId to json!");
        FreeJson(jsonConfirmParams);
        return HC_ERR_JSON_FAIL;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(jsonConfirmParams);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitConfirmRequestTask(task, requestId, jsonConfirmParams) != HC_SUCCESS) {
        FreeJson(jsonConfirmParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(jsonConfirmParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the confirming request task successfully! [AppId]: %s, [RequestId]: %" PRId64,
        appId, requestId);
    return HC_SUCCESS;
}

static int32_t GetModuleType(const CJson *jsonParams)
{
    bool isAccountBind = false;
    if (GetBoolFromJson(jsonParams, FIELD_IS_ACCOUNT_BIND, &isAccountBind) != HC_SUCCESS) {
        return DAS_MODULE;
    }
    if (isAccountBind == false) {
        return DAS_MODULE;
    }
    return TCIS_MODULE;
}

static void DoProcessLiteData(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute the processing lite data task! [RequestId]: %" PRId64, realTask->requestId);
    if (IsRequestExist(realTask->requestId)) {
        int ret = ProcessSession(realTask->requestId, BIND_TYPE, realTask->jsonParams);
        if (ret != CONTINUE) {
            DestroySession(realTask->requestId);
        }
        return;
    }
    if ((BindCallbackToTask(realTask, realTask->jsonParams) != HC_SUCCESS) ||
        ((CheckMsgRepeatability(realTask->jsonParams, GetModuleType(realTask->jsonParams))) != HC_SUCCESS)) {
        return;
    }
    int32_t result = CreateSession(realTask->requestId, TYPE_SERVER_BIND_SESSION_LITE,
        realTask->jsonParams, realTask->callback);
    if (result != HC_SUCCESS) {
        LOGE("Failed to create server lite bind session. We need to notify the service!");
    }
}

static void DoBindPeer(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute the binding peer device task! [RequestId]: %" PRId64, realTask->requestId);
    int32_t result;
    ChannelType channelType = GetChannelType(realTask->callback, realTask->jsonParams);
    do {
        if (channelType == NO_CHANNEL) {
            LOGE("No available channels found!");
            result = HC_ERR_CHANNEL_NOT_EXIST;
            break;
        }
        result = CreateSession(realTask->requestId, TYPE_CLIENT_BIND_SESSION_LITE,
            realTask->jsonParams, realTask->callback);
    } while (0);
    if (result != HC_SUCCESS) {
        LOGE("Failed to create client lite bind session. We need to notify the service!");
        ProcessErrorCallback(realTask->requestId, OP_BIND, result, NULL, realTask->callback);
        return;
    }
    /* If service open the channel by itself,
     * a channel opened message needs to be triggered to unify the channel usage policy. */
    if (channelType == SERVICE_CHANNEL) {
        /* Release the memory in advance to reduce the memory usage. */
        DeleteAllItem(realTask->jsonParams);
        OnChannelOpened(realTask->requestId, DEFAULT_CHANNEL_ID);
    }
}

static void DoUnbindPeer(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute the unbinding peer device task! [RequestId]: %" PRId64,
        realTask->requestId);
    int32_t result;
    ChannelType channelType = GetChannelType(realTask->callback, realTask->jsonParams);
    do {
        if (channelType == NO_CHANNEL) {
            LOGE("No available channels found!");
            result = HC_ERR_CHANNEL_NOT_EXIST;
            break;
        }
        result = CreateSession(realTask->requestId, TYPE_CLIENT_BIND_SESSION_LITE,
            realTask->jsonParams, realTask->callback);
    } while (0);
    if (result != HC_SUCCESS) {
        LOGE("Failed to create client lite bind session. We need to notify the service!");
        ProcessErrorCallback(realTask->requestId, OP_UNBIND, result, NULL, realTask->callback);
        return;
    }
    /* If service open the channel by itself,
     * a channel opened message needs to be triggered to unify the channel usage policy. */
    if (channelType == SERVICE_CHANNEL) {
        /* Release the memory in advance to reduce the memory usage. */
        DeleteAllItem(realTask->jsonParams);
        OnChannelOpened(realTask->requestId, DEFAULT_CHANNEL_ID);
    }
}

static void DoAuthAgreeKey(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute the key agreement task! [RequestId]: %" PRId64, realTask->requestId);
    int32_t result;
    ChannelType channelType = GetChannelType(realTask->callback, realTask->jsonParams);
    do {
        if (channelType == NO_CHANNEL) {
            LOGE("No available channels found!");
            result = HC_ERR_CHANNEL_NOT_EXIST;
            break;
        }
        result = CreateSession(realTask->requestId, TYPE_CLIENT_KEY_AGREE_SESSION,
            realTask->jsonParams, realTask->callback);
    } while (0);
    if (result != HC_SUCCESS) {
        LOGE("Failed to create client key agreement session. We need to notify the service!");
        ProcessErrorCallback(realTask->requestId, AUTH_KEY_AGREEMENT, result, NULL, realTask->callback);
        return;
    }
    /* If service open the channel by itself,
     * a channel opened message needs to be triggered to unify the channel usage policy. */
    if (channelType == SERVICE_CHANNEL) {
        /* Release the memory in advance to reduce the memory usage. */
        DeleteAllItem(realTask->jsonParams);
        OnChannelOpened(realTask->requestId, DEFAULT_CHANNEL_ID);
    }
}

static void DoProcessKeyAgreeData(HcTaskBase *task)
{
    if (task == NULL) {
        LOGE("The input task is NULL!");
        return;
    }
    GroupManagerTask *realTask = (GroupManagerTask *)task;
    LOGI("The task thread starts to execute the processing key agreement data task! [RequestId]: %" PRId64,
        realTask->requestId);
    if (IsRequestExist(realTask->requestId)) {
        int ret = ProcessSession(realTask->requestId, BIND_TYPE, realTask->jsonParams);
        if (ret != CONTINUE) {
            DestroySession(realTask->requestId);
        }
        return;
    }
    if ((BindCallbackToTask(realTask, realTask->jsonParams) != HC_SUCCESS) ||
        ((CheckMsgRepeatability(realTask->jsonParams, DAS_MODULE)) != HC_SUCCESS)) {
        return;
    }
    int32_t result = CreateSession(realTask->requestId, TYPE_SERVER_KEY_AGREE_SESSION,
        realTask->jsonParams, realTask->callback);
    if (result != HC_SUCCESS) {
        LOGE("Failed to create server lite bind session. We need to notify the service!");
    }
}

static int32_t InitProcessLiteDataTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoProcessLiteData;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return HC_SUCCESS;
}

static int32_t InitBindPeerTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoBindPeer;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return BindCallbackToTask(task, jsonParams);
}

static int32_t InitUnbindPeerTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoUnbindPeer;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return BindCallbackToTask(task, jsonParams);
}

static int32_t InitAuthAgreeKeyTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoAuthAgreeKey;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return BindCallbackToTask(task, jsonParams);
}

static int32_t InitProcessKeyAgreeDataTask(GroupManagerTask *task, int64_t requestId, CJson *jsonParams)
{
    task->base.doAction = DoProcessKeyAgreeData;
    task->base.destroy = DestroyGroupManagerTask;
    task->requestId = requestId;
    task->jsonParams = jsonParams;
    return HC_SUCCESS;
}

static int32_t AddLiteDataToReceivedData(CJson *receivedData, int64_t requestId, const char *appId)
{
    int groupOp = ACCOUNT_BIND;
    (void)GetIntFromJson(receivedData, FIELD_GROUP_OP, &groupOp);
    if (groupOp == ACCOUNT_BIND) {
        if (AddBoolToJson(receivedData, FIELD_IS_ACCOUNT_BIND, true) != HC_SUCCESS) {
            LOGE("Failed to add isAccountBind to json!");
            return HC_ERR_JSON_FAIL;
        }
        return AddBindParamsToJson(groupOp, requestId, appId, receivedData);
    }
    return AddServerParamsToJson(true, requestId, appId, receivedData);
}

static int32_t RequestProcessLiteData(int64_t requestId, const char *appId, const uint8_t *data, uint32_t dataLen)
{
    if ((appId == NULL) || (data == NULL) || (dataLen > MAX_DATA_BUFFER_SIZE)) {
        LOGE("The input data is NULL or dataLen is beyond max size!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestProcessLiteData! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *receivedData = CreateJsonFromString((const char *)data);
    if (receivedData == NULL) {
        LOGE("Failed to create received json object from string!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddLiteDataToReceivedData(receivedData, requestId, appId) != HC_SUCCESS) {
        FreeJson(receivedData);
        return HC_ERR_JSON_FAIL;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(receivedData);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitProcessLiteDataTask(task, requestId, receivedData) != HC_SUCCESS) {
        FreeJson(receivedData);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(receivedData);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the processing lite data task successfully! [AppId]: %s, [RequestId]: %" PRId64,
        appId, requestId);
    return HC_SUCCESS;
}

static int32_t RequestBindPeer(int64_t requestId, const char *appId, const char *bindParams)
{
    if ((appId == NULL) || (bindParams == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestBindPeer! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *jsonBindParams = CreateJsonFromString(bindParams);
    if (jsonBindParams == NULL) {
        LOGE("Failed to create json from string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = AddClientParamsToJson(true, OP_BIND, requestId, appId, jsonBindParams);
    if (result != HC_SUCCESS) {
        FreeJson(jsonBindParams);
        return result;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(jsonBindParams);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitBindPeerTask(task, requestId, jsonBindParams) != HC_SUCCESS) {
        FreeJson(jsonBindParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(jsonBindParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the binding peer device task successfully! [AppId]: %s, [RequestId]: %" PRId64,
        appId, requestId);
    return HC_SUCCESS;
}

static int32_t RequestUnbindPeer(int64_t requestId, const char *appId, const char *unBindParams)
{
    if ((appId == NULL) || (unBindParams == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestUnbindPeer, [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *jsonUnbindParams = CreateJsonFromString(unBindParams);
    if (jsonUnbindParams == NULL) {
        LOGE("Failed to create json from string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = AddClientParamsToJson(true, OP_UNBIND, requestId, appId, jsonUnbindParams);
    if (result != HC_SUCCESS) {
        FreeJson(jsonUnbindParams);
        return result;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(jsonUnbindParams);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitUnbindPeerTask(task, requestId, jsonUnbindParams) != HC_SUCCESS) {
        FreeJson(jsonUnbindParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(jsonUnbindParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the unbinding peer device task successfully! [AppId]: %s, [RequestId]: %" PRId64,
        appId, requestId);
    return HC_SUCCESS;
}

static int32_t RequestAuthKeyAgree(int64_t requestId, const char *appId, const char *agreeParams)
{
    if ((appId == NULL) || (agreeParams == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestAuthKeyAgree! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *jsonAgreeParams = CreateJsonFromString(agreeParams);
    if (jsonAgreeParams == NULL) {
        LOGE("Failed to create json from string!");
        return HC_ERR_JSON_FAIL;
    }
    int32_t result = AddClientParamsToJson(true, AUTH_KEY_AGREEMENT, requestId, appId, jsonAgreeParams);
    if (result != HC_SUCCESS) {
        FreeJson(jsonAgreeParams);
        return result;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(jsonAgreeParams);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitAuthAgreeKeyTask(task, requestId, jsonAgreeParams) != HC_SUCCESS) {
        FreeJson(jsonAgreeParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(jsonAgreeParams);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the key agreement task successfully! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    return HC_SUCCESS;
}

static int32_t RequestProcessKeyAgreeData(int64_t requestId, const char *appId, const uint8_t *data, uint32_t dataLen)
{
    if ((appId == NULL) || (data == NULL) || (dataLen > MAX_DATA_BUFFER_SIZE)) {
        LOGE("The input data is NULL or dataLen is beyond max size!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: RequestProcessKeyAgreeData! [AppId]: %s, [RequestId]: %" PRId64, appId, requestId);
    CJson *receivedData = CreateJsonFromString((const char *)data);
    if (receivedData == NULL) {
        LOGE("Failed to create received json object from string!");
        return HC_ERR_JSON_FAIL;
    }
    if (AddServerParamsToJson(false, requestId, appId, receivedData) != HC_SUCCESS) {
        FreeJson(receivedData);
        return HC_ERR_JSON_FAIL;
    }

    GroupManagerTask *task = (GroupManagerTask *)HcMalloc(sizeof(GroupManagerTask), 0);
    if (task == NULL) {
        LOGE("Failed to allocate task memory!");
        FreeJson(receivedData);
        return HC_ERR_ALLOC_MEMORY;
    }
    if (InitProcessKeyAgreeDataTask(task, requestId, receivedData) != HC_SUCCESS) {
        FreeJson(receivedData);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    if (PushTask((HcTaskBase*)task) != HC_SUCCESS) {
        FreeJson(receivedData);
        HcFree(task);
        return HC_ERR_INIT_TASK_FAIL;
    }
    LOGI("[End]: Create the processing key agreement data task successfully! [AppId]: %s, [RequestId]: %" PRId64, appId,
        requestId);
    return HC_SUCCESS;
}

static int32_t ProcessCredential(int operationCode, const char *reqJsonStr, char **returnJsonStr)
{
    LOGE("Process credential is not supported!");
    (void)operationCode;
    (void)reqJsonStr;
    (void)returnJsonStr;
    return HC_ERR_NOT_SUPPORT;
}

static int32_t GetRegisterInfo(char **returnRegisterInfo)
{
    LOGE("Get register information is not supported!");
    (void)returnRegisterInfo;
    return HC_ERR_NOT_SUPPORT;
}

static int32_t AddGroupManagerIfAccessible(const char *appId, const char *groupId, const char *managerAppId)
{
    if ((appId == NULL) || (groupId == NULL) || (managerAppId == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: AddGroupManager! [AppId]: %s, [ManagerAppId]: %s", appId, managerAppId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->addGroupRole(true, appId, groupId, managerAppId);
}

static int32_t AddGroupFriendIfAccessible(const char *appId, const char *groupId, const char *friendAppId)
{
    if ((appId == NULL) || (groupId == NULL) || (friendAppId == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: AddGroupFriend! [AppId]: %s, [FriendAppId]: %s", appId, friendAppId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->addGroupRole(false, appId, groupId, friendAppId);
}

static int32_t DeleteGroupManagerIfAccessible(const char *appId, const char *groupId, const char *managerAppId)
{
    if ((appId == NULL) || (groupId == NULL) || (managerAppId == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: DeleteGroupManager! [AppId]: %s, [ManagerAppId]: %s", appId, managerAppId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->deleteGroupRole(true, appId, groupId, managerAppId);
}

static int32_t DeleteGroupFriendIfAccessible(const char *appId, const char *groupId, const char *friendAppId)
{
    if ((appId == NULL) || (groupId == NULL) || (friendAppId == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: DeleteGroupFriend! [AppId]: %s, [FriendAppId]: %s", appId, friendAppId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->deleteGroupRole(false, appId, groupId, friendAppId);
}

static int32_t GetGroupManagersIfAccessible(const char *appId, const char *groupId, char **returnManagers,
    uint32_t *returnSize)
{
    if ((appId == NULL) || (groupId == NULL) || (returnManagers == NULL) || (returnSize == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: GetGroupManagers! [AppId]: %s", appId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getGroupRole(true, appId, groupId, returnManagers, returnSize);
}

static int32_t GetGroupFriendsIfAccessible(const char *appId, const char *groupId, char **returnFriends,
    uint32_t *returnSize)
{
    if ((appId == NULL) || (groupId == NULL) || (returnFriends == NULL) || (returnSize == NULL)) {
        LOGE("The input parameters contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: GetGroupFriends! [AppId]: %s", appId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getGroupRole(false, appId, groupId, returnFriends, returnSize);
}

static int32_t RegListener(const char *appId, const DataChangeListener *listener)
{
    if ((appId == NULL) || (listener == NULL)) {
        LOGE("The input parameter contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: Service register listener! [AppId]: %s", appId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->regListener(appId, listener);
}

static int32_t UnRegListener(const char *appId)
{
    if (appId == NULL) {
        LOGE("The input appId is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: Service deregister listener! [AppId]: %s", appId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->unRegListener(appId);
}

static int32_t GetAccessibleGroupInfoById(const char *appId, const char *groupId, char **returnGroupInfo)
{
    if ((appId == NULL) || (groupId == NULL) || (returnGroupInfo == NULL)) {
        LOGE("The input parameter contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: GetAccessibleGroupInfoById! [AppId]: %s, [GroupId]: %s", appId, groupId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getAccessibleGroupInfoById(appId, groupId, returnGroupInfo);
}

static int32_t GetAccessibleGroupInfo(const char *appId, const char *queryParams, char **returnGroupVec,
    uint32_t *groupNum)
{
    if ((appId == NULL) || (queryParams == NULL) || (returnGroupVec == NULL) || (groupNum == NULL)) {
        LOGE("The input parameter contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: GetAccessibleGroupInfo! [AppId]: %s, [QueryParams]: %s", appId, queryParams);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getAccessibleGroupInfo(appId, queryParams, returnGroupVec, groupNum);
}

static int32_t GetAccessibleJoinedGroups(const char *appId, int groupType, char **returnGroupVec, uint32_t *groupNum)
{
    if ((appId == NULL) || (returnGroupVec == NULL) || (groupNum == NULL)) {
        LOGE("The input parameter contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: GetAccessibleJoinedGroups! [AppId]: %s, [GroupType]: %d", appId, groupType);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getAccessibleJoinedGroups(appId, groupType, returnGroupVec, groupNum);
}

static int32_t GetAccessibleRelatedGroups(const char *appId, const char *peerDeviceId, char **returnGroupVec,
    uint32_t *groupNum)
{
    if ((appId == NULL) || (peerDeviceId == NULL) || (returnGroupVec == NULL) || (groupNum == NULL)) {
        LOGE("The input parameter contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: GetAccessibleRelatedGroups! [AppId]: %s", appId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getAccessibleRelatedGroups(appId, peerDeviceId, returnGroupVec, groupNum);
}

static int32_t GetAccessibleDeviceInfoById(const char *appId, const char *deviceId, const char *groupId,
    char **returnDeviceInfo)
{
    if ((appId == NULL) || (deviceId == NULL) || (groupId == NULL) || (returnDeviceInfo == NULL)) {
        LOGE("The input parameter contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: GetAccessibleDeviceInfoById! [AppId]: %s, [GroupId]: %s", appId, groupId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getAccessibleDeviceInfoById(appId, deviceId, groupId, returnDeviceInfo);
}

static int32_t GetAccessibleTrustedDevices(const char *appId, const char *groupId, char **returnDevInfoVec,
    uint32_t *deviceNum)
{
    if ((appId == NULL) || (groupId == NULL) || (returnDevInfoVec == NULL) || (deviceNum == NULL)) {
        LOGE("The input parameter contains NULL value!");
        return HC_ERR_INVALID_PARAMS;
    }
    LOGI("[Start]: GetAccessibleTrustedDevices! [AppId]: %s, [GroupId]: %s", appId, groupId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return HC_ERR_NOT_SUPPORT;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return HC_ERR_NULL_PTR;
    }
    return instance->getAccessibleTrustedDevices(appId, groupId, returnDevInfoVec, deviceNum);
}

static bool IsDeviceInAccessibleGroup(const char *appId, const char *groupId, const char *deviceId)
{
    if ((appId == NULL) || (groupId == NULL) || (deviceId == NULL)) {
        LOGE("The input parameter contains NULL value!");
        return false;
    }
    LOGI("[Start]: IsDeviceInAccessibleGroup! [AppId]: %s, [GroupId]: %s", appId, groupId);
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return false;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return false;
    }
    return instance->isDeviceInAccessibleGroup(appId, groupId, deviceId);
}

static void DestroyInfo(char **returnInfo)
{
    if (returnInfo == NULL) {
        LOGI("The input returnInfo is NULL!");
        return;
    }
    LOGI("[Start]: DestroyInfo!");
    if (!IsGroupManagerSupported()) {
        LOGE("Group manager is not supported!");
        return;
    }
    GroupManager *instance = GetGroupManagerInstance();
    if (instance == NULL) {
        LOGE("Failed to get groupManager instance!");
        return;
    }
    instance->destroyInfo(returnInfo);
}

static int32_t AllocGmAndGa(void)
{
    if (g_groupManagerInstance == NULL) {
        g_groupManagerInstance = (DeviceGroupManager *)HcMalloc(sizeof(DeviceGroupManager), 0);
        if (g_groupManagerInstance == NULL) {
            LOGE("Failed to allocate groupManagerInstance memory!");
            return HC_ERR_ALLOC_MEMORY;
        }
    }
    if (g_groupAuthManager == NULL) {
        g_groupAuthManager = (GroupAuthManager *)HcMalloc(sizeof(GroupAuthManager), 0);
        if (g_groupAuthManager == NULL) {
            LOGE("Alloc memory failed!");
            HcFree(g_groupManagerInstance);
            g_groupManagerInstance = NULL;
            return HC_ERR_ALLOC_MEMORY;
        }
    }
    return HC_SUCCESS;
}

static int32_t InitAlgorithm(void)
{
    const AlgLoader *loader = GetLoaderInstance();
    if (loader == NULL) {
        LOGE("Get loader failed.");
        return HC_ERROR;
    }
    int32_t res = loader->initAlg();
    if (res != HC_SUCCESS) {
        LOGE("Failed to init algorithm!");
        return res;
    }
    return res;
}

static void DestroyGmAndGa(void)
{
    if (g_groupAuthManager != NULL) {
        HcFree(g_groupAuthManager);
        g_groupAuthManager = NULL;
    }
    if (g_groupManagerInstance != NULL) {
        HcFree(g_groupManagerInstance);
        g_groupManagerInstance = NULL;
    }
}

DEVICE_AUTH_API_PUBLIC int InitDeviceAuthService(void)
{
    LOGI("[Service]: Start to init device auth service!");
    if (CheckInit() == FINISH_INIT) {
        LOGI("[End]: [Service]: Device auth service is running!");
        return HC_SUCCESS;
    }
    int32_t res = InitAlgorithm();
    if (res != HC_SUCCESS) {
        LOGE("[End]: [Service]: Failed to init algorithm module!");
        goto err;
    }
    res = AllocGmAndGa();
    if (res != HC_SUCCESS) {
        LOGE("[End]: [Service]: Failed to init group manage and group auth instances!");
        goto err;
    }
    res = InitModules();
    if (res != HC_SUCCESS) {
        LOGE("[End]: [Service]: Failed to init all authenticator modules!");
        goto free_gm_ga;
    }
    res = InitGroupManager();
    if (res != HC_SUCCESS) {
        goto free_module;
    }
    InitSessionManager();
    res = InitTaskManager();
    if (res != HC_SUCCESS) {
        LOGE("[End]: [Service]: Failed to init worker thread!");
        goto free_all;
    }
    SetInitStatus();
    LOGI("[End]: [Service]: Init device auth service successfully!");
    return res;
free_all:
    DestroySessionManager();
    DestroyGroupManager();
free_module:
    DestroyModules();
free_gm_ga:
    DestroyGmAndGa();
err:
    SetDeInitStatus();
    return res;
}

DEVICE_AUTH_API_PUBLIC void DestroyDeviceAuthService(void)
{
    LOGI("[Service]: Start to destroy device auth service!");
    if (CheckDestroy() == FINISH_DESTROY) {
        LOGI("[End]: [Service]: The service has not been initialized, so it does not need to be destroyed!");
        return;
    }
    DestroyTaskManager();
    DestroyChannelManager();
    DestroySessionManager();
    DestroyGroupManager();
    DestroyModules();
    DestroyGmAndGa();
    SetDeInitStatus();
    LOGI("[End]: [Service]: Destroy device auth service successfully!");
}

DEVICE_AUTH_API_PUBLIC const DeviceGroupManager *GetGmInstance()
{
    if (g_groupManagerInstance == NULL) {
        LOGE("Service not init.");
        return NULL;
    }

    g_groupManagerInstance->regCallback = RegGroupManagerCallback;
    g_groupManagerInstance->unRegCallback = UnRegGroupManagerCallback;
    g_groupManagerInstance->createGroup = RequestCreateGroup;
    g_groupManagerInstance->deleteGroup = RequestDeleteGroup;
    g_groupManagerInstance->addMemberToGroup = RequestAddMemberToGroup;
    g_groupManagerInstance->deleteMemberFromGroup = RequestDeleteMemberFromGroup;
    g_groupManagerInstance->processData = RequestProcessBindData;
    g_groupManagerInstance->confirmRequest = ConfirmRequest;
    g_groupManagerInstance->bindPeer = RequestBindPeer;
    g_groupManagerInstance->unbindPeer = RequestUnbindPeer;
    g_groupManagerInstance->processLiteData = RequestProcessLiteData;
    g_groupManagerInstance->authKeyAgree = RequestAuthKeyAgree;
    g_groupManagerInstance->processKeyAgreeData = RequestProcessKeyAgreeData;
    g_groupManagerInstance->processCredential = ProcessCredential;
    g_groupManagerInstance->getRegisterInfo = GetRegisterInfo;
    g_groupManagerInstance->getLocalConnectInfo = NULL;
    g_groupManagerInstance->addGroupManager = AddGroupManagerIfAccessible;
    g_groupManagerInstance->deleteGroupManager = DeleteGroupManagerIfAccessible;
    g_groupManagerInstance->getGroupManagers = GetGroupManagersIfAccessible;
    g_groupManagerInstance->addGroupFriend = AddGroupFriendIfAccessible;
    g_groupManagerInstance->deleteGroupFriend = DeleteGroupFriendIfAccessible;
    g_groupManagerInstance->getGroupFriends = GetGroupFriendsIfAccessible;
    g_groupManagerInstance->regDataChangeListener = RegListener;
    g_groupManagerInstance->unRegDataChangeListener = UnRegListener;
    g_groupManagerInstance->getGroupInfoById = GetAccessibleGroupInfoById;
    g_groupManagerInstance->getGroupInfo = GetAccessibleGroupInfo;
    g_groupManagerInstance->getJoinedGroups = GetAccessibleJoinedGroups;
    g_groupManagerInstance->getRelatedGroups = GetAccessibleRelatedGroups;
    g_groupManagerInstance->getDeviceInfoById = GetAccessibleDeviceInfoById;
    g_groupManagerInstance->getTrustedDevices = GetAccessibleTrustedDevices;
    g_groupManagerInstance->checkAccessToGroup = NULL;
    g_groupManagerInstance->isDeviceInGroup = IsDeviceInAccessibleGroup;
    g_groupManagerInstance->destroyInfo = DestroyInfo;
    return g_groupManagerInstance;
}

DEVICE_AUTH_API_PUBLIC const GroupAuthManager *GetGaInstance()
{
    if (g_groupAuthManager == NULL) {
        LOGE("Service not init.");
        return NULL;
    }

    g_groupAuthManager->processData = ProcessData;
    g_groupAuthManager->queryTrustedDeviceNum = QueryTrustedDeviceNum;
    g_groupAuthManager->isTrustedDevice = IsTrustedDevice;
    g_groupAuthManager->getAuthState = GetAuthState;
    g_groupAuthManager->authDevice = AuthDevice;
    g_groupAuthManager->informDeviceDisconnection = InformDeviceDisconnection;
    return g_groupAuthManager;
}
