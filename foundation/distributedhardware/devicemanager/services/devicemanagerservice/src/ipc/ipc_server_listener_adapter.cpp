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

#include "ipc_server_listener_adapter.h"

#include <memory>

#include "securec.h"

#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "anonymous_string.h"

#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_check_auth_result_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_dmfa_result_req.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(IpcServerListenerAdapter);

void IpcServerListenerAdapter::OnDeviceStateChange(DmDeviceState state, DmDeviceInfo &deviceInfo)
{
    DMLOG(DM_LOG_INFO, "OnDeviceStateChange");
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetDeviceState(state);
    pReq->SetDeviceInfo(deviceInfo);
    ipcServerListener_.SendAll(SERVER_DEVICE_STATE_NOTIFY, pReq, pRsp);
}

void IpcServerListenerAdapter::OnDeviceFound(std::string &pkgName, uint16_t originId,
    DmDeviceInfo &deviceInfo)
{
    DMLOG(DM_LOG_INFO, "call OnDeviceFound for %s, originId %d, deviceId %s",
        pkgName.c_str(), originId, GetAnonyString(std::string(deviceInfo.deviceId)).c_str());
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::make_shared<IpcNotifyDeviceFoundReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetSubscribeId(originId);
    pReq->SetDeviceInfo(deviceInfo);
    ipcServerListener_.SendRequest(SERVER_DEVICE_FOUND, pReq, pRsp);
}

void IpcServerListenerAdapter::OnDiscoverFailed(std::string &pkgName, uint16_t originId,
    DiscoveryFailReason failReason)
{
    DMLOG(DM_LOG_INFO, "OnDiscoverFailed");
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::make_shared<IpcNotifyDiscoverResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetSubscribeId(originId);
    pReq->SetResult(failReason);
    ipcServerListener_.SendRequest(SERVER_DISCOVER_FINISH, pReq, pRsp);
}

void IpcServerListenerAdapter::OnDiscoverySuccess(std::string &pkgName, uint16_t originId)
{
    DMLOG(DM_LOG_INFO, "OnDiscoverySuccess");
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::make_shared<IpcNotifyDiscoverResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetSubscribeId(originId);
    pReq->SetResult(DEVICEMANAGER_OK);
    ipcServerListener_.SendRequest(SERVER_DISCOVER_FINISH, pReq, pRsp);
}

void IpcServerListenerAdapter::OnAuthResult(std::string &pkgName, std::string &deviceId, int32_t pinToken,
    uint32_t status, uint32_t reason)
{
    DMLOG(DM_LOG_INFO, "%s, package: %s, deviceId: %s", __FUNCTION__, pkgName.c_str(),
        GetAnonyString(deviceId).c_str());
    std::shared_ptr<IpcNotifyAuthResultReq> pReq = std::make_shared<IpcNotifyAuthResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetDeviceId(deviceId);
    pReq->SetPinToken(pinToken);
    pReq->SetStatus(status);
    pReq->SetReason(reason);
    ipcServerListener_.SendRequest(SERVER_AUTH_RESULT, pReq, pRsp);
}

void IpcServerListenerAdapter::OnCheckAuthResult(std::string &authParam, int32_t resultCode,
    int32_t flag)
{
    DMLOG(DM_LOG_INFO, "OnCheckResult, authParam: %s, errorCode: %d",
        GetAnonyString(authParam).c_str(), resultCode);
    std::shared_ptr<IpcNotifyCheckAuthResultReq> pReq = std::make_shared<IpcNotifyCheckAuthResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetDeviceId(authParam);
    pReq->SetResult(resultCode);
    pReq->SetFlag(flag);
    ipcServerListener_.SendAll(SERVER_CHECK_AUTH_RESULT, pReq, pRsp);
}

void IpcServerListenerAdapter::OnFaCall(std::string &pkgName, std::string &paramJson)
{
    DMLOG(DM_LOG_INFO, "OnFaCall in");
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::make_shared<IpcNotifyDMFAResultReq>();
    std::shared_ptr<IpcRsp> pRsp = std::make_shared<IpcRsp>();

    pReq->SetPkgName(pkgName);
    pReq->SetJsonParam(paramJson);
    ipcServerListener_.SendRequest(SERVER_DEVICEMANAGER_FA_NOTIFY, pReq, pRsp);
}
} // namespace DistributedHardware
} // namespace OHOS
