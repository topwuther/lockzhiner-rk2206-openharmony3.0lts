/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "appspawn_service.h"
#include <stdlib.h>

#ifdef OHOS_DEBUG
#include <errno.h>
#include <time.h>
#endif // OHOS_DEBUG

#include "appspawn_message.h"
#include "appspawn_process.h"
#include "iproxy_server.h"
#include "iunknown.h"
#include "liteipc_adapter.h"
#include "log.h"
#include "message.h"
#include "ohos_errno.h"
#include "ohos_init.h"
#include "samgr_lite.h"
#include "service.h"


static const int INVALID_PID = -1;

typedef struct AppSpawnFeatureApi {
    INHERIT_SERVER_IPROXY;
} AppSpawnFeatureApi;

typedef struct AppSpawnService {
    INHERIT_SERVICE;
    INHERIT_IUNKNOWNENTRY(AppSpawnFeatureApi);
    Identity identity;
} AppSpawnService;

static const char* GetName(Service* service)
{
    (void)service;
    HILOG_INFO(HILOG_MODULE_HIVIEW, "[appspawn] get service name %{public}s.", APPSPAWN_SERVICE_NAME);
    return APPSPAWN_SERVICE_NAME;
}

static BOOL Initialize(Service* service, Identity identity)
{
    if (service == NULL) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] initialize, service NULL!");
        return FALSE;
    }

    AppSpawnService* spawnService = (AppSpawnService*)service;
    spawnService->identity = identity;

    HILOG_INFO(HILOG_MODULE_HIVIEW, "[appspawn] initialize, identity<%{public}d, %{public}d, %{public}p>",\
        identity.serviceId, identity.featureId, identity.queueId);
    return TRUE;
}

static BOOL MessageHandle(Service* service, Request* msg)
{
    (void)service;
    (void)msg;
    HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] message handle not support yet!");
    return FALSE;
}

static TaskConfig GetTaskConfig(Service* service)
{
    (void)service;
    TaskConfig config = {LEVEL_HIGH, PRI_BELOW_NORMAL, 0x800, 20, SHARED_TASK};
    return config;
}

#ifdef OHOS_DEBUG
static void GetCurTime(struct timespec* tmCur)
{
    if (tmCur == NULL) {
        return;
    }
    if (clock_gettime(CLOCK_REALTIME, tmCur) != 0) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] invoke, get time failed! err %{public}d", errno);
    }
}
#endif // OHOS_DEBUG

static int GetMessageSt(MessageSt* msgSt, IpcIo* req)
{
    if (msgSt == NULL || req == NULL) {
        return EC_FAILURE;
    }
#ifdef __LINUX__
    size_t len = 0;
    char* str = IpcIoPopString(req, &len);
    if (str == NULL || len == 0) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] invoke, get data failed.");
        return EC_FAILURE;
    }

    int ret = SplitMessage(str, len, msgSt);    // after split message, str no need to free(linux version)
#else
    BuffPtr* dataPtr = IpcIoPopDataBuff(req);
    if (dataPtr == NULL) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] invoke, get data failed.");
        return EC_FAILURE;
    }

    int ret = SplitMessage((char*)dataPtr->buff, dataPtr->buffSz, msgSt);

    // release buffer
    if (FreeBuffer(NULL, dataPtr->buff) != LITEIPC_OK) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] invoke, free buffer failed!");
    }
#endif
    return ret;
}

static int Invoke(IServerProxy* iProxy, int funcId, void* origin, IpcIo* req, IpcIo* reply)
{
#ifdef OHOS_DEBUG
    struct timespec tmStart = {0};
    GetCurTime(&tmStart);
#endif // OHOS_DEBUG

    (void)iProxy;
    (void)origin;

    if (reply == NULL || funcId != ID_CALL_CREATE_SERVICE || req == NULL) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] invoke, funcId %{public}d invalid, reply %{public}d.",\
            funcId, INVALID_PID);
        IpcIoPushInt64(reply, INVALID_PID);
        return EC_BADPTR;
    }

    MessageSt msgSt = {0};
    if (GetMessageSt(&msgSt, req) != EC_SUCCESS) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] invoke, parse failed! reply %{public}d.", INVALID_PID);
        IpcIoPushInt64(reply, INVALID_PID);
        return EC_FAILURE;
    }

    HILOG_INFO(HILOG_MODULE_HIVIEW, "[appspawn] invoke, msg<%{public}s,%{public}s,%{public}d,%{public}d>",\
        msgSt.bundleName, msgSt.identityID, msgSt.uID, msgSt.gID);

    pid_t newPid = CreateProcess(&msgSt);
    FreeMessageSt(&msgSt);
    IpcIoPushInt64(reply, newPid);

#ifdef OHOS_DEBUG
    struct timespec tmEnd = {0};
    GetCurTime(&tmEnd);

    // 1s = 1000000000ns
    long timeUsed = (tmEnd.tv_sec - tmStart.tv_sec) * 1000000000L + (tmEnd.tv_nsec - tmStart.tv_nsec);
    HILOG_INFO(HILOG_MODULE_HIVIEW, "[appspawn] invoke, reply pid %{public}d, timeused %{public}ld ns.",\
        newPid, timeUsed);
#else
    HILOG_INFO(HILOG_MODULE_HIVIEW, "[appspawn] invoke, reply pid %{public}d.", newPid);
#endif // OHOS_DEBUG

    return ((newPid > 0) ? EC_SUCCESS : EC_FAILURE);
}

static AppSpawnService g_appSpawnService = {
    .GetName = GetName,
    .Initialize = Initialize,
    .MessageHandle = MessageHandle,
    .GetTaskConfig = GetTaskConfig,
    SERVER_IPROXY_IMPL_BEGIN,
    .Invoke = Invoke,
    IPROXY_END,
};

void AppSpawnInit(void)
{
    if (SAMGR_GetInstance()->RegisterService((Service *)&g_appSpawnService) != TRUE) {
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] register service failed!");
        return;
    }

    HILOG_INFO(HILOG_MODULE_HIVIEW, "[appspawn] register service succeed. %{public}p.", &g_appSpawnService);

    if (SAMGR_GetInstance()->RegisterDefaultFeatureApi(APPSPAWN_SERVICE_NAME, \
        GET_IUNKNOWN(g_appSpawnService)) != TRUE) {
        (void)SAMGR_GetInstance()->UnregisterService(APPSPAWN_SERVICE_NAME);
        HILOG_ERROR(HILOG_MODULE_HIVIEW, "[appspawn] register featureapi failed!");
        return;
    }

    HILOG_INFO(HILOG_MODULE_HIVIEW, "[appspawn] register featureapi succeed.");
}

SYSEX_SERVICE_INIT(AppSpawnInit);

