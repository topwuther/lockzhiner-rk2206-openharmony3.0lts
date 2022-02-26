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

#include "ipc_client_manager.h"

#include "ipc_remote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "device_manager_notify.h"

#include "ipc_register_listener_req.h"

namespace OHOS {
namespace DistributedHardware {
void DmDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    (void)remote;
    DMLOG(DM_LOG_WARN, "DmDeathRecipient : OnRemoteDied");
    DeviceManagerNotify::GetInstance().OnRemoteDied();
}

int32_t IpcClientManager::ClientInit()
{
    DMLOG(DM_LOG_INFO, "InitDeviceManagerService start");
    if (dmInterface_ != nullptr) {
        DMLOG(DM_LOG_INFO, "DeviceManagerService Already Init");
        return DEVICEMANAGER_OK;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DMLOG(DM_LOG_ERROR, "Get SystemAbilityManager Failed");
        return DEVICEMANAGER_SERVICE_NOT_READY;
    }

    auto object = samgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (object == nullptr) {
        DMLOG(DM_LOG_ERROR, "Get DeviceManager SystemAbility Failed");
        return DEVICEMANAGER_SERVICE_NOT_READY;
    }

    if (dmRecipient_ == nullptr) {
        dmRecipient_ = sptr<DmDeathRecipient>(new DmDeathRecipient());
    }
    if (!object->AddDeathRecipient(dmRecipient_)) {
        DMLOG(DM_LOG_ERROR, "InitDeviceManagerService: AddDeathRecipient Failed");
    }
    dmInterface_ = iface_cast<IpcRemoteBroker>(object);
    DMLOG(DM_LOG_INFO, "DeviceManager::InitDeviceManagerService completed");
    return DEVICEMANAGER_OK;
}

int32_t IpcClientManager::Init(std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    int32_t ret = ClientInit();
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "InitDeviceManager Failed with ret %d", ret);
        return ret;
    }
    if (dmListener_.count(pkgName) > 0) {
        DMLOG(DM_LOG_INFO, "dmListener_ Already Init");
        return DEVICEMANAGER_OK;
    }

    sptr<IpcClientStub> listener = sptr<IpcClientStub>(new IpcClientStub());
    std::shared_ptr<IpcRegisterListenerReq> req = std::make_shared<IpcRegisterListenerReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetListener(listener);
    ret = dmInterface_->SendCmd(REGISTER_DEVICE_MANAGER_LISTENER, req, rsp);
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "InitDeviceManager: RegisterDeviceManagerListener Failed with ret %d", ret);
        return ret;
    }
    ret = rsp->GetErrCode();
    if (ret != DEVICEMANAGER_OK) {
        return ret;
    }
    dmListener_[pkgName] = listener;
    DMLOG(DM_LOG_INFO, "completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}

int32_t IpcClientManager::UnInit(std::string &pkgName)
{
    DMLOG(DM_LOG_INFO, "in, pkgName %s", pkgName.c_str());
    if (dmInterface_ == nullptr) {
        DMLOG(DM_LOG_ERROR, "DeviceManager not Init");
        return DEVICEMANAGER_SERVICE_NOT_READY;
    }

    std::lock_guard<std::mutex> autoLock(lock_);
    if (dmListener_.count(pkgName) > 0) {
        std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
        std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
        req->SetPkgName(pkgName);
        int32_t ret = dmInterface_->SendCmd(UNREGISTER_DEVICE_MANAGER_LISTENER, req, rsp);
        if (ret != DEVICEMANAGER_OK) {
            DMLOG(DM_LOG_ERROR, "UnRegisterDeviceManagerListener Failed with ret %d", ret);
            return ret;
        }
        dmListener_.erase(pkgName);
    }
    if (dmListener_.empty()) {
        dmInterface_ = nullptr;
        dmRecipient_ = nullptr;
    }
    DMLOG(DM_LOG_INFO, "completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}

int32_t IpcClientManager::SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    std::string pkgName = req->GetPkgName();
    if (!IsInit(pkgName)) {
        return DEVICEMANAGER_SERVICE_NOT_READY;
    }
    return dmInterface_->SendCmd(cmdCode, req, rsp);
}

bool IpcClientManager::IsInit(std::string &pkgName)
{
    if (dmInterface_ == nullptr) {
        DMLOG(DM_LOG_ERROR, "DeviceManager not Init");
        return false;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (dmListener_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "dmListener_ not Init for %s", pkgName.c_str());
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
