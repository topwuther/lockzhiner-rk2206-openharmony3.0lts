/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ipc_server_stub.h"

#include "securec.h"

#include "liteipc_adapter.h"
#include "ohos_init.h"
#include "samgr_lite.h"
#include "iproxy_server.h"

#include "device_manager_log.h"
#include "device_manager_errno.h"
#include "dm_subscribe_info.h"

#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "ipc_server_listenermgr.h"
#include "ipc_server_adapter.h"

namespace {
    const int32_t WAIT_FOR_SERVER = 2;
    const int32_t STACK_SIZE = 0x800;
    const int32_t QUEUE_SIZE = 20;
}

using namespace OHOS::DistributedHardware;

struct DefaultFeatureApi {
    INHERIT_SERVER_IPROXY;
};

struct DeviceManagerSamgrService {
    INHERIT_SERVICE;
    INHERIT_IUNKNOWNENTRY(DefaultFeatureApi);
    Identity identity;
};

static int32_t DeathCb(const IpcContext *context, void *ipcMsg, IpcIo *data, void *arg)
{
    (void)context;
    (void)ipcMsg;
    (void)data;
    if (arg == NULL) {
        DMLOG(DM_LOG_ERROR, "package name is NULL.");
        return DEVICEMANAGER_INVALID_PARAM;
    }
    CommonSvcId svcId = {0};
    std::string pkgName = (const char *)arg;
    if (IpcServerListenermgr::GetInstance().GetListenerByPkgName(pkgName, &svcId) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "not found client by package name.");
        free(arg);
        arg = NULL;
        return DEVICEMANAGER_FAILED;
    }
    IpcServerListenermgr::GetInstance().UnregisterListener(pkgName);
    free(arg);
    arg = NULL;
#ifdef __LINUX__
    BinderRelease(svcId.ipcCtx, svcId.handle);
#endif
    SvcIdentity sid = {0};
    sid.handle = svcId.handle;
    sid.token = svcId.token;
    sid.cookie = svcId.cookie;
    UnregisterDeathCallback(sid, svcId.cbId);
    return DEVICEMANAGER_OK;
}

int32_t RegisterDeviceManagerListener(IpcIo *req, IpcIo *reply)
{
    DMLOG(DM_LOG_INFO, "register service listener.");
    size_t len = 0;
    uint8_t *name = IpcIoPopString(req, &len);
    SvcIdentity *svc = IpcIoPopSvc(req);
    if (name == NULL || svc == NULL || len == 0) {
        DMLOG(DM_LOG_ERROR, "get para failed");
        return DEVICEMANAGER_INVALID_PARAM;
    }

    CommonSvcId svcId = {0};
    svcId.handle = svc->handle;
    svcId.token = svc->token;
    svcId.cookie = svc->cookie;

    SvcIdentity sid = *svc;
#ifdef __LINUX__
    svcId.ipcCtx = svc->ipcContext;
    BinderAcquire(svcId.ipcCtx, svcId.handle);
    free(svc);
    svc = NULL;
#endif
    char *pkgName = (char *)malloc(len + 1);
    if (pkgName == NULL) {
        DMLOG(DM_LOG_ERROR, "malloc failed!");
        return DEVICEMANAGER_MALLOC_ERROR;
    }
    if (strcpy_s(pkgName, len + 1, (const char *)name) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "strcpy_s failed!");
        free(pkgName);
        return DEVICEMANAGER_COPY_FAILED;
    }
    uint32_t cbId = 0;
    RegisterDeathCallback(NULL, sid, DeathCb, pkgName, &cbId);
    svcId.cbId = cbId;
    std::string strPkgName = (const char *)name;
    return IpcServerListenermgr::GetInstance().RegisterListener(strPkgName, &svcId);
}

int32_t UnRegisterDeviceManagerListener(IpcIo *req, IpcIo *reply)
{
    DMLOG(DM_LOG_INFO, "unregister service listener.");
    size_t len = 0;
    std::string pkgName = (const char *)IpcIoPopString(req, &len);
    if (pkgName == "" || len == 0) {
        DMLOG(DM_LOG_ERROR, "get para failed");
        return DEVICEMANAGER_FAILED;
    }
    CommonSvcId svcId;
    if (IpcServerListenermgr::GetInstance().GetListenerByPkgName(pkgName, &svcId) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "not found listener by package name.");
        return DEVICEMANAGER_FAILED;
    }
    int32_t ret = IpcServerListenermgr::GetInstance().UnregisterListener(pkgName);
    if (ret == DEVICEMANAGER_OK) {
#ifdef __LINUX__
        BinderRelease(svcId.ipcCtx, svcId.handle);
#endif
        SvcIdentity sid;
        sid.handle = svcId.handle;
        sid.token = svcId.token;
        sid.cookie = svcId.cookie;
        ret = UnregisterDeathCallback(sid, svcId.cbId);
    }
    return ret;
}

static const char *GetName(Service *service)
{
    (void)service;
    return DEVICE_MANAGER_SERVICE_NAME;
}

static BOOL Initialize(Service *service, Identity identity)
{
    if (service == NULL) {
        DMLOG(DM_LOG_WARN, "invalid param");
        return FALSE;
    }

    DeviceManagerSamgrService *mgrService = (DeviceManagerSamgrService *)service;
    mgrService->identity = identity;
    return TRUE;
}

static BOOL MessageHandle(Service *service, Request *request)
{
    if ((service == NULL) || (request == NULL)) {
        DMLOG(DM_LOG_WARN, "invalid param");
        return FALSE;
    }
    return TRUE;
}

static TaskConfig GetTaskConfig(Service *service)
{
    (void)service;
    TaskConfig config = {LEVEL_HIGH, PRI_BELOW_NORMAL, STACK_SIZE, QUEUE_SIZE, SHARED_TASK};
    return config;
}

static int32_t OnRemoteRequest(IServerProxy *iProxy, int32_t funcId, void *origin,
    IpcIo *req, IpcIo *reply)
{
    DMLOG(DM_LOG_INFO, "Receive funcId:%d", funcId);
    (void)origin;
    return IpcCmdRegister::GetInstance().OnIpcServerCmd(funcId, *req, *reply);
}

static void HOS_SystemInit(void)
{
    SAMGR_Bootstrap();
    return;
}

int32_t IpcServerStubInit(void)
{
    HOS_SystemInit();
    return DEVICEMANAGER_OK;
}

static void DevMgrSvcInit(void)
{
    sleep(WAIT_FOR_SERVER);
    static DeviceManagerSamgrService service = {
        .GetName = GetName,
        .Initialize = Initialize,
        .MessageHandle = MessageHandle,
        .GetTaskConfig = GetTaskConfig,
        SERVER_IPROXY_IMPL_BEGIN,
        .Invoke = OnRemoteRequest,
        IPROXY_END,
    };

    if (!SAMGR_GetInstance()->RegisterService((Service *)&service)) {
        DMLOG(DM_LOG_ERROR, "%s, RegisterService failed", DEVICE_MANAGER_SERVICE_NAME);
        return;
    }
    if (!SAMGR_GetInstance()->RegisterDefaultFeatureApi(DEVICE_MANAGER_SERVICE_NAME, GET_IUNKNOWN(service))) {
        DMLOG(DM_LOG_ERROR, "%s, RegisterDefaultFeatureApi failed", DEVICE_MANAGER_SERVICE_NAME);
        return;
    }
    DMLOG(DM_LOG_INFO, "%s, init success", DEVICE_MANAGER_SERVICE_NAME);
}
SYSEX_SERVICE_INIT(DevMgrSvcInit);
