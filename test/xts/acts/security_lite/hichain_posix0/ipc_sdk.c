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

#include "ipc_sdk.h"

#include "common_defs.h"
#include "device_auth_defines.h"
#include "device_auth.h"
#include "hc_log.h"
#include "hc_mutex.h"

#include "ipc_adapt.h"
#include "securec.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_DATA_CACHES_3 3
#define IPC_DATA_CACHES_4 4
#define REPLAY_CACHE_NUM(caches) (sizeof(caches) / sizeof(IpcDataInfo))
#define IPC_APPID_LEN 128

#define IS_STRING_VALID(str) (((str) != NULL) && ((str)[0] != 0))
#define IS_COMM_DATA_VALID(dPtr, dLen) (((dPtr) != NULL) && ((dLen) > 0) && ((dLen) <= 4096))

typedef struct {
    uintptr_t inst;
    char appId[IPC_APPID_LEN];
} IpcProxyCbInfo;
static IpcProxyCbInfo g_ipcProxyCbList = {0};
static IpcProxyCbInfo g_ipcListenerCbList = {0};
static HcMutex g_ipcMutex;

static void DelIpcCliCallbackCtx(const char *appId, IpcProxyCbInfo *cbCache)
{
    int32_t ret;

    if (cbCache->appId[0] == 0) {
        return;
    }
    g_ipcMutex.lock(&g_ipcMutex);
    ret = memcmp(appId, cbCache->appId, strlen(cbCache->appId) + 1);
    if (ret == 0) {
        cbCache->appId[0] = 0;
    }
    g_ipcMutex.unlock(&g_ipcMutex);
    return;
}

static void AddIpcCliCallbackCtx(const char *appId, uintptr_t cbInst, IpcProxyCbInfo *cbCache)
{
    errno_t eno;

    LOGI("starting ...");
    g_ipcMutex.lock(&g_ipcMutex);
    eno = memcpy_s(cbCache->appId, IPC_APPID_LEN, appId, strlen(appId) + 1);
    if (eno != EOK) {
        g_ipcMutex.unlock(&g_ipcMutex);
        LOGE("memory copy failed");
        return;
    }
    cbCache->inst = cbInst;
    g_ipcMutex.unlock(&g_ipcMutex);
    LOGI("success, appid: %s", appId);
    return;
}

static void GetIpcReplyByType(const IpcDataInfo *ipcData,
    int32_t dataNum, int32_t type, uint8_t *outCache, int32_t *cacheLen)
{
    int32_t i;
    int32_t ret = HC_ERR_IPC_BAD_MSG_TYPE;
    errno_t eno;

    LOGI("type %d", type);
    for (i = 0; i < dataNum; i++) {
        if (ipcData[i].type != type) {
            continue;
        }
        ret = HC_SUCCESS;
        switch (type) {
            case PARAM_TYPE_REG_INFO:
            case PARAM_TYPE_MGR_APPID:
            case PARAM_TYPE_FRIEND_APPID:
            case PARAM_TYPE_DEVICE_INFO:
            case PARAM_TYPE_GROUP_INFO:
                *(uint8_t **)outCache = ipcData[i].val;
                if (cacheLen != NULL) {
                    *cacheLen = ipcData[i].valSz;
                }
                break;
            case PARAM_TYPE_IPC_RESULT:
            case PARAM_TYPE_IPC_RESULT_NUM:
            case PARAM_TYPE_COMM_DATA:
            case PARAM_TYPE_DATA_NUM:
                eno = memcpy_s(outCache, *cacheLen, ipcData[i].val, ipcData[i].valSz);
                if (eno != EOK) {
                    ret = HC_ERR_MEMORY_COPY;
                    break;
                }
                *cacheLen = ipcData[i].valSz;
                break;
            default:
                ret = HC_ERR_IPC_BAD_MSG_TYPE;
                LOGE("un-expectation type case");
                break;
        }
    }
    LOGI("process done, type %d, result %d", type, ret);
    return;
}

static int32_t IpcGmRegCallback(const char *appId, const DeviceAuthCallback *callback)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId)) {
        LOGE("invalid params");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_DEV_AUTH_CB, (const uint8_t *)callback, sizeof(*callback));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_DEV_AUTH_CB);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    SetCbCtxToDataCtx(callCtx, IPC_CALL_BACK_STUB_BIND_ID);
    ret = DoBinderCall(callCtx, IPC_CALL_ID_REG_CB, true);
    if (ret == HC_SUCCESS) {
        AddIpcCliCallbackCtx(appId, 0, &g_ipcProxyCbList);
    }
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return (ret == HC_SUCCESS) ? HC_SUCCESS : HC_ERR_IPC_PROC_FAILED;
}

static int32_t IpcGmUnRegCallback(const char *appId)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d", ret);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_UNREG_CB, true);
    if (ret == HC_SUCCESS) {
        DelIpcCliCallbackCtx(appId, &g_ipcProxyCbList);
    }
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", HC_SUCCESS);
    return HC_SUCCESS;
}

static int32_t IpcGmRegDataChangeListener(const char *appId, const DataChangeListener *listener)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || (listener == NULL)) {
        LOGE("invalid params");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_LISTERNER, (const uint8_t *)listener, sizeof(*listener));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_LISTERNER);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    SetCbCtxToDataCtx(callCtx, IPC_CALL_BACK_STUB_BIND_ID);
    ret = DoBinderCall(callCtx, IPC_CALL_ID_REG_LISTENER, true);
    if (ret == HC_SUCCESS) {
        AddIpcCliCallbackCtx(appId, 0, &g_ipcListenerCbList);
    }
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return (ret == HC_SUCCESS) ? HC_SUCCESS : HC_ERR_IPC_PROC_FAILED;
}

static int32_t IpcGmUnRegDataChangeListener(const char *appId)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d", ret);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_UNREG_LISTENER, true);
    if (ret == HC_SUCCESS) {
        DelIpcCliCallbackCtx(appId, &g_ipcListenerCbList);
    }
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done");
    return HC_SUCCESS;
}

static int32_t IpcGmCreateGroup(int64_t requestId, const char *appid, const char *createParams)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(createParams) || !IS_STRING_VALID(appid)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)&requestId, sizeof(requestId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appid, strlen(appid) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_CREATE_PARAMS,
                                  (const uint8_t *)createParams, strlen(createParams) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_CREATE_PARAMS);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_CREATE_GROUP, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmDelGroup(int64_t requestId, const char *appId, const char *delParams)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(delParams) || !IS_STRING_VALID(appId)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)&requestId, sizeof(requestId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_DEL_PARAMS, (const uint8_t *)delParams, strlen(delParams) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_DEL_PARAMS);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_DEL_GROUP, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmAddMemberToGroup(int64_t requestId, const char *appId, const char *addParams)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(addParams)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)&requestId, sizeof(requestId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_ADD_PARAMS, (const uint8_t *)addParams, strlen(addParams) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_ADD_PARAMS);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_ADD_GROUP_MEMBER, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmDelMemberFromGroup(int64_t requestId, const char *appId, const char *delParams)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(delParams)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)&requestId, sizeof(requestId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_DEL_PARAMS, (const uint8_t *)delParams, strlen(delParams) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_DEL_PARAMS);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_DEL_GROUP_MEMBER, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmProcessData(int64_t requestId, const uint8_t *data, uint32_t dataLen)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_COMM_DATA_VALID(data, dataLen)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)&requestId, sizeof(requestId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_COMM_DATA, data, dataLen);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_COMM_DATA);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GM_PROC_DATA, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmConfirmRequest(int64_t requestId, const char *appId, const char *cfmParams)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(cfmParams)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)&requestId, sizeof(requestId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQ_CFM, (const uint8_t *)cfmParams, strlen(cfmParams) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQ_CFM);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_CFM_REQUEST, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmBindPeer(int64_t requestId, const char *appId, const char *bindParams)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(bindParams)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)&requestId, sizeof(requestId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_BIND, (const uint8_t *)bindParams, strlen(bindParams) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_BIND);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_BIND_PEER, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmUnBindPeer(int64_t requestId, const char *appId, const char *unBindParams)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(unBindParams)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)&requestId, sizeof(requestId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_UNBIND, (const uint8_t *)unBindParams, strlen(unBindParams) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_UNBIND);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_UNBIND_PEER, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmProcessLiteData(int64_t requestId, const char *appId, const uint8_t *data, uint32_t dataLen)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_COMM_DATA_VALID(data, dataLen) || !IS_STRING_VALID(appId)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)(&requestId), sizeof(requestId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_COMM_DATA, data, dataLen);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_COMM_DATA);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_PROC_LIGHT_DATA, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmProcCredential(int32_t operationCode, const char *credential, char **returnJsonStr)
{
    LOGI("starting ...");
    (void)operationCode;
    if (!IS_STRING_VALID(credential) || (returnJsonStr == NULL)) {
        LOGE("The input groupType is invalid or credential is NULL!");
        return HC_ERR_INVALID_PARAMS;
    }

    return HC_ERR_NOT_SUPPORT;
}

static int32_t IpcGmGetRegisterInfo(char **registerInfo)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_3] = {{0}};
    char *outInfo = NULL;

    LOGI("starting ...");
    if (registerInfo == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_APPLY_REG_INFO, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if ((ret != HC_SUCCESS) || (inOutLen != sizeof(int32_t))) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT_NUM, (uint8_t *)&ret, &inOutLen);
    if ((ret < IPC_RESULT_NUM_1) || (inOutLen != sizeof(int32_t))) {
        LOGE("done, ret %d", HC_ERR_IPC_OUT_DATA_NUM);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_OUT_DATA_NUM;
    }
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_REG_INFO, (uint8_t *)&outInfo, NULL);
    if ((outInfo == NULL) || (strlen(outInfo) == 0)) {
        LOGE("done, ret %d", HC_ERR_IPC_OUT_DATA);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_OUT_DATA;
    }
    *registerInfo = strdup(outInfo);
    DestroyCallCtx(&callCtx, NULL);
    return (*registerInfo != NULL) ? HC_SUCCESS : HC_ERR_NULL_PTR;
}

static int32_t IpcGmGetLocalConnectInfo(char **outLocalConnInfo)
{
    LOGI("starting ...");
    if (outLocalConnInfo == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }

    return HC_ERR_NOT_SUPPORT;
}

static int32_t IpcGmAddGroupManager(const char *appId, const char *groupId, const char *managerAppId)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(groupId) || !IS_STRING_VALID(managerAppId)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_MGR_APPID,
                                  (const uint8_t *)managerAppId, strlen(managerAppId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_MGR_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_ADD_GROUP_MGR, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmAddGroupFriend(const char *appId, const char *groupId, const char *friendAppId)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(groupId) || !IS_STRING_VALID(friendAppId) || !IS_STRING_VALID(appId)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_FRIEND_APPID,
                                  (const uint8_t *)friendAppId, strlen(friendAppId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_FRIEND_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_ADD_GROUP_FRIEND, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmDelGroupManager(const char *appId, const char *groupId, const char *managerAppId)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(groupId) || !IS_STRING_VALID(managerAppId) || !IS_STRING_VALID(appId)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_MGR_APPID,
                                  (const uint8_t *)managerAppId, strlen(managerAppId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_MGR_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_DEL_GROUP_MGR, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t IpcGmDelGroupFriend(const char *appId, const char *groupId, const char *friendAppId)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(groupId) || !IS_STRING_VALID(friendAppId) || !IS_STRING_VALID(appId)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_FRIEND_APPID,
                                  (const uint8_t *)friendAppId, strlen(friendAppId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_FRIEND_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_DEL_GROUP_FRIEND, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("process done, ret %d", ret);
    return ret;
}

static int32_t GetGroupManagersIpcResult(const IpcDataInfo *replies, int32_t cacheNum,
                                         char **outManagers, uint32_t *outSize)
{
    int32_t inOutLen;
    int32_t ret;

    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_IPC_RESULT_NUM, (uint8_t *)&ret, &inOutLen);
    if ((ret < IPC_RESULT_NUM_2) || (inOutLen != sizeof(int32_t))) {
        return HC_ERR_IPC_OUT_DATA_NUM;
    }
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_MGR_APPID, (uint8_t *)outManagers, NULL);
    if ((*outManagers == NULL) || (strlen(*outManagers) == 0)) {
        return HC_ERR_IPC_OUT_DATA;
    }
    *outManagers = strdup(*outManagers);
    if (*outManagers == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_DATA_NUM, (uint8_t *)outSize, &inOutLen);
    return HC_SUCCESS;
}

static int32_t IpcGmGetGroupManagers(const char *appId, const char *groupId, char **outManagers, uint32_t *outSize)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_4] = {{0}};

    LOGI("starting ...");
    if (!IS_STRING_VALID(groupId) || !IS_STRING_VALID(appId) || (outManagers == NULL) || (outSize == NULL)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GET_GROUP_MGR, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = GetGroupManagersIpcResult(replyCache, REPLAY_CACHE_NUM(replyCache), outManagers, outSize);
    LOGI("proc result done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static int32_t GetGroupFriendsIpcResult(const IpcDataInfo *replies,
    int32_t cacheNum, char **outFriends, uint32_t *outSize)
{
    int32_t inOutLen;
    int32_t ret;

    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_IPC_RESULT_NUM, (uint8_t *)&ret, &inOutLen);
    if ((ret < IPC_RESULT_NUM_2) || (inOutLen != sizeof(int32_t))) {
        return HC_ERR_IPC_OUT_DATA_NUM;
    }
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_FRIEND_APPID, (uint8_t *)outFriends, NULL);
    if ((*outFriends == NULL) || (strlen(*outFriends) == 0)) {
        return HC_ERR_IPC_OUT_DATA;
    }
    *outFriends = strdup(*outFriends);
    if (*outFriends == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_DATA_NUM, (uint8_t *)outSize, &inOutLen);
    return HC_SUCCESS;
}

static int32_t IpcGmGetGroupFriends(const char *appId, const char *groupId, char **outFriends, uint32_t *outSize)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_4] = {{0}};

    LOGI("starting ...");
    if (!IS_STRING_VALID(groupId) || !IS_STRING_VALID(appId) || (outFriends == NULL) || (outSize == NULL)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GET_GROUP_FRIEND, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = GetGroupFriendsIpcResult(replyCache, REPLAY_CACHE_NUM(replyCache), outFriends, outSize);
    LOGI("proc result done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static int32_t GroupInfoIpcResult(const IpcDataInfo *replies, int32_t cacheNum, char **outGroupInfo)
{
    int32_t inOutLen;
    int32_t ret;

    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_IPC_RESULT_NUM, (uint8_t *)&ret, &inOutLen);
    if ((ret < IPC_RESULT_NUM_1) || (inOutLen != sizeof(int32_t))) {
        return HC_ERR_IPC_OUT_DATA_NUM;
    }
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_GROUP_INFO, (uint8_t *)outGroupInfo, NULL);
    if (*outGroupInfo == NULL) {
        return HC_ERR_IPC_OUT_DATA;
    }
    *outGroupInfo = strdup(*outGroupInfo);
    if (*outGroupInfo == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    return HC_SUCCESS;
}

static int32_t IpcGmGetGroupInfoById(const char *appId, const char *groupId, char **outGroupInfo)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_3] = {{0}};

    LOGI("starting ...");
    if (!IS_STRING_VALID(groupId) || !IS_STRING_VALID(appId) || (outGroupInfo == NULL)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GET_GROUP_INFO, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = GroupInfoIpcResult(replyCache, REPLAY_CACHE_NUM(replyCache), outGroupInfo);
    DestroyCallCtx(&callCtx, NULL);
    LOGI("proc result done, ret %d", ret);
    return ret;
}

static int32_t SearchGroupsIpcResult(const IpcDataInfo *replies,
    int32_t cacheNum, char **outGroupVec, uint32_t *groupNum)
{
    int32_t ret;
    int32_t inOutLen;

    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_IPC_RESULT_NUM, (uint8_t *)&ret, &inOutLen);
    if ((ret < IPC_RESULT_NUM_2) || (inOutLen != sizeof(int32_t))) {
        return HC_ERR_IPC_OUT_DATA_NUM;
    }
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_GROUP_INFO, (uint8_t *)outGroupVec, NULL);
    if (*outGroupVec == NULL) {
        return HC_ERR_IPC_OUT_DATA;
    }
    *outGroupVec = strdup(*outGroupVec);
    if (*outGroupVec == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_DATA_NUM, (uint8_t *)groupNum, &inOutLen);
    return HC_SUCCESS;
}

static int32_t IpcGmGetGroupInfo(const char *appId, const char *queryParams, char **outGroupVec, uint32_t *groupNum)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_4] = {{0}};

    LOGI("starting ...");
    if (!IS_STRING_VALID(queryParams) || !IS_STRING_VALID(appId) || (outGroupVec == NULL) || (groupNum == NULL)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_QUERY_PARAMS,
                                  (const uint8_t *)queryParams, strlen(queryParams) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_QUERY_PARAMS);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_SEARCH_GROUPS, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = SearchGroupsIpcResult(replyCache, REPLAY_CACHE_NUM(replyCache), outGroupVec, groupNum);
    LOGI("proc result done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static int32_t JoinedGroupsIpcResult(const IpcDataInfo *replies,
    int32_t cacheNum, char **outGroupVec, uint32_t *groupNum)
{
    int32_t ret;
    int32_t inOutLen;

    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_IPC_RESULT_NUM, (uint8_t *)&ret, &inOutLen);
    if ((ret < IPC_RESULT_NUM_2) || (inOutLen != sizeof(int32_t))) {
        return HC_ERR_IPC_OUT_DATA_NUM;
    }
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_GROUP_INFO, (uint8_t *)outGroupVec, NULL);
    if (*outGroupVec == NULL) {
        return HC_ERR_IPC_OUT_DATA;
    }
    *outGroupVec = strdup(*outGroupVec);
    if (*outGroupVec == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_DATA_NUM, (uint8_t *)groupNum, &inOutLen);
    return HC_SUCCESS;
}

static int32_t IpcGmGetJoinedGroups(const char *appId, int32_t groupType, char **outGroupVec, uint32_t *groupNum)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_4] = {{0}};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || (outGroupVec == NULL) ||
        (groupNum == NULL) || (groupType != ACROSS_ACCOUNT_AUTHORIZE_GROUP)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUP_TYPE, (const uint8_t *)&groupType, sizeof(groupType));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUP_TYPE);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GET_JOINED_GROUPS, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = JoinedGroupsIpcResult(replyCache, REPLAY_CACHE_NUM(replyCache), outGroupVec, groupNum);
    LOGI("proc result done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static int32_t RelatedGroupsIpcResult(const IpcDataInfo *replies,
    int32_t cacheNum, char **outGroupVec, uint32_t *groupNum)
{
    int32_t ret;
    int32_t inOutLen;

    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_IPC_RESULT_NUM, (uint8_t *)&ret, &inOutLen);
    if ((ret < IPC_RESULT_NUM_2) || (inOutLen != sizeof(int32_t))) {
        return HC_ERR_IPC_OUT_DATA_NUM;
    }
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_GROUP_INFO, (uint8_t *)outGroupVec, NULL);
    if (*outGroupVec == NULL) {
        return HC_ERR_IPC_OUT_DATA;
    }
    *outGroupVec = strdup(*outGroupVec);
    if (*outGroupVec == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_DATA_NUM, (uint8_t *)groupNum, &inOutLen);
    return HC_SUCCESS;
}

static int32_t IpcGmGetRelatedGroups(const char *appId, const char *peerUdid, char **outGroupVec, uint32_t *groupNum)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_4] = {{0}};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(peerUdid) || (outGroupVec == NULL) || (groupNum == NULL)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_UDID, (const uint8_t *)peerUdid, strlen(peerUdid) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_UDID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GET_RELATED_GROUPS, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = RelatedGroupsIpcResult(replyCache, REPLAY_CACHE_NUM(replyCache), outGroupVec, groupNum);
    LOGI("proc result done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static int32_t DevInfoByIdIpcResult(const IpcDataInfo *replies, int32_t cacheNum, char **outDevInfo)
{
    int32_t ret;
    int32_t inOutLen;

    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_IPC_RESULT_NUM, (uint8_t *)&ret, &inOutLen);
    if ((ret < IPC_RESULT_NUM_1) || (inOutLen != sizeof(int32_t))) {
        return HC_ERR_IPC_OUT_DATA_NUM;
    }
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_DEVICE_INFO, (uint8_t *)outDevInfo, NULL);
    if (*outDevInfo == NULL) {
        return HC_ERR_IPC_OUT_DATA;
    }
    *outDevInfo = strdup(*outDevInfo);
    if (*outDevInfo == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    return HC_SUCCESS;
}

static int32_t FormParamsForGettingDeviceInfo(const char *appId,
    const char *peerUdid, const char *groupId, uintptr_t callCtx)
{
    int32_t ret;
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_UDID, (const uint8_t *)peerUdid, strlen(peerUdid) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_UDID);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    return HC_SUCCESS;
}

static int32_t IpcGmGetDeviceInfoById(const char *appId, const char *peerUdid, const char *groupId, char **outDevInfo)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_3] = {{0}};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(peerUdid) || !IS_STRING_VALID(groupId) || (outDevInfo == NULL)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = FormParamsForGettingDeviceInfo(appId, peerUdid, groupId, callCtx);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GET_DEV_INFO_BY_ID, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = DevInfoByIdIpcResult(replyCache, REPLAY_CACHE_NUM(replyCache), outDevInfo);
    LOGI("proc result done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static int32_t TrustedDevIpcResult(const IpcDataInfo *replies, int32_t cacheNum, char **outDevInfoVec, uint32_t *devNum)
{
    int32_t ret;
    int32_t inOutLen;

    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_IPC_RESULT_NUM, (uint8_t *)&ret, &inOutLen);
    if ((ret < IPC_RESULT_NUM_2) || (inOutLen != sizeof(int32_t))) {
        return HC_ERR_IPC_OUT_DATA_NUM;
    }
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_DEVICE_INFO, (uint8_t *)outDevInfoVec, NULL);
    if (*outDevInfoVec == NULL) {
        return HC_ERR_IPC_OUT_DATA;
    }
    *outDevInfoVec = strdup(*outDevInfoVec);
    if (*outDevInfoVec == NULL) {
        return HC_ERR_ALLOC_MEMORY;
    }
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replies, cacheNum, PARAM_TYPE_DATA_NUM, (uint8_t *)devNum, &inOutLen);
    return HC_SUCCESS;
}

static int32_t IpcGmGetTrustedDevices(const char *appId,
    const char *groupId, char **outDevInfoVec, uint32_t *deviceNum)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_4] = {{0}};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(groupId) ||
        (outDevInfoVec == NULL) || (deviceNum == NULL)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GET_TRUST_DEVICES, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = TrustedDevIpcResult(replyCache, REPLAY_CACHE_NUM(replyCache), outDevInfoVec, deviceNum);
    LOGI("proc result done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static bool IpcGmIsDeviceInGroup(const char *appId, const char *groupId, const char *udid)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(groupId) || !IS_STRING_VALID(udid)) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_APPID, (const uint8_t *)appId, strlen(appId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_APPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_UDID, (const uint8_t *)udid, strlen(udid) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_UDID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_IS_DEV_IN_GROUP, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return (ret == HC_SUCCESS) ? true : false;
}

static void IpcGmDestroyInfo(char **returnInfo)
{
    if ((returnInfo == NULL) || (*returnInfo == NULL)) {
        return;
    }
    FreeJsonString(*returnInfo);
    *returnInfo = NULL;
}

static int32_t IpcGmAuthKeyAgree(int64_t requestId, const char *appId, const char *buildParams)
{
    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_STRING_VALID(buildParams)) {
        return HC_ERR_INVALID_PARAMS;
    }

    (void)requestId;
    return HC_ERR_NOT_SUPPORT;
}

static int32_t IpcGmProcessKeyAgreeData(int64_t requestId, const char *appId, const uint8_t *data, uint32_t dataLen)
{
    LOGI("starting ...");
    if (!IS_STRING_VALID(appId) || !IS_COMM_DATA_VALID(data, dataLen)) {
        return HC_ERR_INVALID_PARAMS;
    }

    (void)requestId;
    return HC_ERR_NOT_SUPPORT;
}

static void InitIpcGmMethods(DeviceGroupManager *gmMethodObj)
{
    LOGI("starting ...");
    gmMethodObj->regCallback = IpcGmRegCallback;
    gmMethodObj->unRegCallback = IpcGmUnRegCallback;
    gmMethodObj->regDataChangeListener = IpcGmRegDataChangeListener;
    gmMethodObj->unRegDataChangeListener = IpcGmUnRegDataChangeListener;
    gmMethodObj->createGroup = IpcGmCreateGroup;
    gmMethodObj->deleteGroup = IpcGmDelGroup;
    gmMethodObj->addMemberToGroup = IpcGmAddMemberToGroup;
    gmMethodObj->deleteMemberFromGroup = IpcGmDelMemberFromGroup;
    gmMethodObj->processData = IpcGmProcessData;
    gmMethodObj->confirmRequest = IpcGmConfirmRequest;
    gmMethodObj->bindPeer = IpcGmBindPeer;
    gmMethodObj->unbindPeer = IpcGmUnBindPeer;
    gmMethodObj->processLiteData = IpcGmProcessLiteData;
    gmMethodObj->processCredential = IpcGmProcCredential;
    gmMethodObj->getRegisterInfo = IpcGmGetRegisterInfo;
    gmMethodObj->getLocalConnectInfo = IpcGmGetLocalConnectInfo;
    gmMethodObj->addGroupManager = IpcGmAddGroupManager;
    gmMethodObj->addGroupFriend = IpcGmAddGroupFriend;
    gmMethodObj->deleteGroupManager = IpcGmDelGroupManager;
    gmMethodObj->deleteGroupFriend = IpcGmDelGroupFriend;
    gmMethodObj->getGroupManagers = IpcGmGetGroupManagers;
    gmMethodObj->getGroupFriends = IpcGmGetGroupFriends;
    gmMethodObj->getGroupInfoById = IpcGmGetGroupInfoById;
    gmMethodObj->getGroupInfo = IpcGmGetGroupInfo;
    gmMethodObj->getJoinedGroups = IpcGmGetJoinedGroups;
    gmMethodObj->getRelatedGroups = IpcGmGetRelatedGroups;
    gmMethodObj->getDeviceInfoById = IpcGmGetDeviceInfoById;
    gmMethodObj->getTrustedDevices = IpcGmGetTrustedDevices;
    gmMethodObj->isDeviceInGroup = IpcGmIsDeviceInGroup;
    gmMethodObj->destroyInfo = IpcGmDestroyInfo;
    gmMethodObj->authKeyAgree = IpcGmAuthKeyAgree;
    gmMethodObj->processKeyAgreeData = IpcGmProcessKeyAgreeData;
    LOGI("process done");
    return;
}

static int32_t IpcGaProcessData(int64_t authReqId,
    const uint8_t *data, uint32_t dataLen, const DeviceAuthCallback *callback)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_COMM_DATA_VALID(data, dataLen) || (callback == NULL)) {
        LOGE("invalid params");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)(&authReqId), sizeof(authReqId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_COMM_DATA, (const uint8_t *)data, dataLen);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_COMM_DATA);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_DEV_AUTH_CB, (const uint8_t *)callback, sizeof(*callback));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_DEV_AUTH_CB);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    SetCbCtxToDataCtx(callCtx, 0x0);
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GA_PROC_DATA, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static int32_t IpcGaQueryTrustedDeviceNum(void)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_QUERY_TRUST_DEV_NUM, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, device num %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static bool IpcGaIsTrustedDevice(const char *udid)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (udid == NULL) {
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    uint32_t udidLen = strlen(udid) + 1;
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_COMM_DATA, (const uint8_t *)udid, udidLen);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_COMM_DATA);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_IS_TRUST_DEVICE, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return (ret == HC_SUCCESS) ? true : false;
}

static int32_t FormParamsForGettingAuthState(int64_t authReqId, const char *groupId,
    const char *peerUdid, uintptr_t callCtx)
{
    int32_t ret;
    int32_t inOutLen;
    inOutLen = sizeof(authReqId);
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)&authReqId, inOutLen);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_REQID);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_GROUPID, (const uint8_t *)groupId, strlen(groupId) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_GROUPID);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_UDID, (const uint8_t *)peerUdid, strlen(peerUdid) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_UDID);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    return HC_SUCCESS;
}

static int32_t IpcGaGetAuthState(int64_t authReqId, const char *groupId,
    const char *peerUdid, uint8_t *out, uint32_t *outLen)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache[IPC_DATA_CACHES_3] = {{0}};

    LOGI("starting ...");
    if (!IS_STRING_VALID(peerUdid) || !IS_STRING_VALID(groupId) || (out == NULL) || (outLen == NULL)) {
        LOGE("invalid params");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }
    ret = FormParamsForGettingAuthState(authReqId, groupId, peerUdid, callCtx);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_GET_AUTH_STATE, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    if (ret != HC_SUCCESS) {
        DestroyCallCtx(&callCtx, NULL);
        return ret;
    }
    GetIpcReplyByType(replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_COMM_DATA, out, (int32_t *)outLen);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static int32_t IpcGaAuthDevice(int64_t authReqId, const char *authParams, const DeviceAuthCallback *callback)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;
    int32_t inOutLen;
    IpcDataInfo replyCache = {0};

    LOGI("starting ...");
    if (!IS_STRING_VALID(authParams) || (callback == NULL)) {
        LOGE("invalid params");
        return HC_ERR_INVALID_PARAMS;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return HC_ERROR;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return HC_ERR_IPC_INIT;
    }

    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_REQID, (const uint8_t *)(&authReqId), sizeof(authReqId));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_REQID);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_AUTH_PARAMS, (const uint8_t *)authParams, strlen(authParams) + 1);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_AUTH_PARAMS);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_DEV_AUTH_CB, (const uint8_t *)callback, sizeof(*callback));
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, type %d", ret, PARAM_TYPE_DEV_AUTH_CB);
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_BUILD_PARAM;
    }
    SetCbCtxToDataCtx(callCtx, IPC_CALL_BACK_STUB_AUTH_ID);
    ret = DoBinderCall(callCtx, IPC_CALL_ID_AUTH_DEVICE, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call failed");
        DestroyCallCtx(&callCtx, NULL);
        return HC_ERR_IPC_PROC_FAILED;
    }
    DecodeCallReply(callCtx, &replyCache, REPLAY_CACHE_NUM(replyCache));
    ret = HC_ERR_IPC_UNKNOW_REPLAY;
    inOutLen = sizeof(int32_t);
    GetIpcReplyByType(&replyCache, REPLAY_CACHE_NUM(replyCache), PARAM_TYPE_IPC_RESULT, (uint8_t *)&ret, &inOutLen);
    LOGI("process done, ret %d", ret);
    DestroyCallCtx(&callCtx, NULL);
    return ret;
}

static void IpcGaInformDeviceDisconn(const char *udid)
{
    uintptr_t callCtx = 0x0;
    int32_t ret;

    LOGI("entering ...");
    if (udid == NULL) {
        LOGE("invalid params");
        return;
    }
    if (!IsServiceRunning()) {
        LOGE("service is not activity");
        return;
    }
    ret = CreateCallCtx(&callCtx, NULL);
    if (ret != HC_SUCCESS) {
        LOGE("CreateCallCtx failed, ret %d", ret);
        return;
    }
    uint32_t udidLen = strlen(udid) + 1;
    ret = SetCallRequestParamInfo(callCtx, PARAM_TYPE_COMM_DATA, (const uint8_t *)udid, udidLen);
    if (ret != HC_SUCCESS) {
        LOGE("set request param failed, ret %d, param id %d", ret, PARAM_TYPE_COMM_DATA);
        DestroyCallCtx(&callCtx, NULL);
        return;
    }
    ret = DoBinderCall(callCtx, IPC_CALL_ID_INFORM_DEV_DISCONN, true);
    if (ret == HC_ERR_IPC_INTERNAL_FAILED) {
        LOGE("ipc call error");
        DestroyCallCtx(&callCtx, NULL);
        return;
    }
    LOGI("process done");
    DestroyCallCtx(&callCtx, NULL);
    return;
}

static void InitIpcGaMethods(GroupAuthManager *gaMethodObj)
{
    LOGI("entering...");
    gaMethodObj->processData = IpcGaProcessData;
    gaMethodObj->queryTrustedDeviceNum = IpcGaQueryTrustedDeviceNum;
    gaMethodObj->isTrustedDevice = IpcGaIsTrustedDevice;
    gaMethodObj->getAuthState = IpcGaGetAuthState;
    gaMethodObj->authDevice = IpcGaAuthDevice;
    gaMethodObj->informDeviceDisconnection = IpcGaInformDeviceDisconn;
    LOGI("process done");
    return;
}

DEVICE_AUTH_API_PUBLIC int InitDeviceAuthService(void)
{
    InitHcMutex(&g_ipcMutex);
    return HC_SUCCESS;
}

DEVICE_AUTH_API_PUBLIC void DestroyDeviceAuthService(void)
{
    DestroyHcMutex(&g_ipcMutex);
}

DEVICE_AUTH_API_PUBLIC const GroupAuthManager *GetGaInstance(void)
{
    static GroupAuthManager gaInstCtx = {NULL};
    static GroupAuthManager *gaInstPtr = NULL;

    LOGI("Enter InitIpcMethods...");
    if (gaInstPtr == NULL) {
        InitIpcGaMethods(&gaInstCtx);
        gaInstPtr = &gaInstCtx;
    }
    LOGI("InitIpcMethods done");
    return (const GroupAuthManager *)(gaInstPtr);
}

DEVICE_AUTH_API_PUBLIC const DeviceGroupManager *GetGmInstance(void)
{
    static DeviceGroupManager gmInstCtx = {NULL};
    static DeviceGroupManager *gmInstPtr = NULL;

    LOGI("Enter InitIpcMethods...");
    if (gmInstPtr == NULL) {
        InitIpcGmMethods(&gmInstCtx);
        gmInstPtr = &gmInstCtx;
    }
    LOGI("InitIpcMethods done");
    return (const DeviceGroupManager *)(gmInstPtr);
}

#ifdef __cplusplus
}
#endif
