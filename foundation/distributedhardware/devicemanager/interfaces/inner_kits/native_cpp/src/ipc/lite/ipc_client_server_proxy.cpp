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

#include "ipc_client_server_proxy.h"

#include "device_manager_log.h"
#include "device_manager_errno.h"
#include "device_manager_notify.h"

#include "ipc_def.h"
#include "ipc_cmd_register.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const int32_t INVALID_CB_ID = 0xFF;
}
static uint32_t g_deathCbId = INVALID_CB_ID;
static SvcIdentity g_svcIdentity;
static std::shared_ptr<IpcRsp> pCurRsp;

void __attribute__((weak)) HOS_SystemInit(void)
{
    SAMGR_Bootstrap();
    return;
}

int32_t DMDeathCallback(const IpcContext *ctx, void *ipcMsg, IpcIo *data, void *arg)
{
    (void)ctx;
    (void)ipcMsg;
    (void)data;
    (void)arg;
    DMLOG(DM_LOG_INFO, "ATTENTION  SERVICE (%s) DEAD !!!\n", DEVICE_MANAGER_SERVICE_NAME);
    UnregisterDeathCallback(g_svcIdentity, g_deathCbId);
    g_deathCbId = INVALID_CB_ID;
    g_svcIdentity.handle = 0;
    g_svcIdentity.token = 0;
    g_svcIdentity.cookie = 0;
    DeviceManagerNotify::GetInstance().OnRemoteDied();
    return DEVICEMANAGER_OK;
}

static int32_t SendCmdResultCb(IOwner owner, int32_t code, IpcIo *reply)
{
    (void)code;
    int32_t cmdCode = *(int32_t *)owner;
    DMLOG(DM_LOG_INFO, "SendCmdResultCb code:%d", cmdCode);
    (void)IpcCmdRegister::GetInstance().ReadResponse(cmdCode, *reply, pCurRsp);
    return DEVICEMANAGER_OK;
}

IClientProxy *IpcClientServerProxy::GetServerProxy(void)
{
    IClientProxy *clientProxy = nullptr;
    IUnknown *iUnknown = nullptr;

    DMLOG(DM_LOG_INFO, "start get client proxy");
    iUnknown = SAMGR_GetInstance()->GetDefaultFeatureApi(DEVICE_MANAGER_SERVICE_NAME);
    if (iUnknown == nullptr) {
        return nullptr;
    }
    if (iUnknown->QueryInterface(iUnknown, CLIENT_PROXY_VER, (void **)&clientProxy) != DEVICEMANAGER_OK ||
        clientProxy == nullptr) {
        DMLOG(DM_LOG_ERROR, "QueryInterface failed");
    }
    return clientProxy;
}

int IpcClientServerProxy::RegisterServerDeathCb(void)
{
    g_svcIdentity = SAMGR_GetRemoteIdentity(DEVICE_MANAGER_SERVICE_NAME, nullptr);
    g_deathCbId = INVALID_CB_ID;
    if (RegisterDeathCallback(nullptr, g_svcIdentity, DMDeathCallback, nullptr, &g_deathCbId) != EC_SUCCESS) {
        DMLOG(DM_LOG_ERROR, "reg death callback failed");
        return DEVICEMANAGER_FAILED;
    }
    return DEVICEMANAGER_OK;
}

int32_t IpcClientServerProxy::SendCmd(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    DMLOG(DM_LOG_INFO, "SendCmd:%d", cmdCode);
    uint8_t data[MAX_DM_IPC_LEN] = {0};
    IpcIo request;

    if (IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, request, data, MAX_DM_IPC_LEN) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_FAILED;
    }
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        pCurRsp = rsp;
        if (serviceProxy_ != nullptr &&
            serviceProxy_->Invoke(serviceProxy_, cmdCode, &request, &cmdCode, SendCmdResultCb) != 0) {
            DMLOG(DM_LOG_ERROR, "serviceProxy_ invoke failed.");
            return DEVICEMANAGER_FAILED;
        }
    }
    DMLOG(DM_LOG_INFO, "SendCmd:%d end", cmdCode);
    return DEVICEMANAGER_OK;
}

int32_t IpcClientServerProxy::Init(void)
{
    if (serviceProxy_ != nullptr) {
        DMLOG(DM_LOG_INFO, "ServerProxy already Init");
        return DEVICEMANAGER_OK;
    }
    HOS_SystemInit();
    serviceProxy_ = GetServerProxy();
    if (serviceProxy_ == nullptr) {
        DMLOG(DM_LOG_ERROR, "get ipc client proxy failed");
        return DEVICEMANAGER_FAILED;
    }
    if (RegisterServerDeathCb() != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "register server death cb failed");
        return DEVICEMANAGER_FAILED;
    }
    DMLOG(DM_LOG_INFO, "ServerProxyInit ok");
    return DEVICEMANAGER_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
