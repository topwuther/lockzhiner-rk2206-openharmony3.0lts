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

#include <algorithm>
#include <thread>

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "device_manager_errno.h"
#include "device_manager_log.h"

#include "ipc_server_adapter.h"
#include "ipc_cmd_register.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(IpcServerStub);

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(&IpcServerStub::GetInstance());

IpcServerStub::IpcServerStub() : SystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, true)
{
    registerToService_ = false;
    state_ = ServiceRunningState::STATE_NOT_START;
}

void IpcServerStub::OnStart()
{
    DMLOG(DM_LOG_INFO, "IpcServerStub::OnStart start");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        DMLOG(DM_LOG_DEBUG, "IpcServerStub has already started.");
        return;
    }
    if (!Init()) {
        DMLOG(DM_LOG_ERROR, "failed to init IpcServerStub");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
}

bool IpcServerStub::Init()
{
    DMLOG(DM_LOG_INFO, "IpcServerStub::Init ready to init.");
    if (!registerToService_) {
        bool ret = Publish(this);
        if (!ret) {
            DMLOG(DM_LOG_ERROR, "IpcServerStub::Init Publish failed!");
            return false;
        }
        registerToService_ = true;
    }

    std::thread {
        [] {
            IpcServerAdapter::GetInstance().ModuleInit();
        }
    }.detach();
    return true;
}

void IpcServerStub::OnStop()
{
    DMLOG(DM_LOG_INFO, "IpcServerStub::OnStop ready to stop service.");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
}

int32_t IpcServerStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    DMLOG(DM_LOG_INFO, "code = %d, flags= %d.", code, option.GetFlags());
    int32_t ret = DEVICEMANAGER_OK;
    ret = IpcCmdRegister::GetInstance().OnIpcCmd(code, data, reply);
    if (ret == DEVICEMANAGER_IPC_NOT_REGISTER_FUNC) {
        DMLOG(DM_LOG_WARN, "unsupport code: %d", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}

int32_t IpcServerStub::SendCmd(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    return DEVICEMANAGER_OK;
}

ServiceRunningState IpcServerStub::QueryServiceState() const
{
    return state_;
}

int32_t IpcServerStub::RegisterDeviceManagerListener(std::string &pkgName, sptr<IRemoteObject> listener)
{
    if (pkgName.empty() || listener == nullptr) {
        DMLOG(DM_LOG_ERROR, "Error: parameter invalid");
        return DEVICEMANAGER_NULLPTR;
    }

    DMLOG(DM_LOG_INFO, "In, pkgName: %s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    auto iter = dmListener_.find(pkgName);
    if (iter != dmListener_.end()) {
        DMLOG(DM_LOG_INFO, "RegisterDeviceManagerListener: listener already exists");
        return DEVICEMANAGER_OK;
    }

    sptr<AppDeathRecipient> appRecipient = sptr<AppDeathRecipient>(new AppDeathRecipient());
    if (!listener->AddDeathRecipient(appRecipient)) {
        DMLOG(DM_LOG_ERROR, "RegisterDeviceManagerListener: AddDeathRecipient Failed");
    }
    dmListener_[pkgName] = listener;
    appRecipient_[pkgName] = appRecipient;
    return DEVICEMANAGER_OK;
}

int32_t IpcServerStub::UnRegisterDeviceManagerListener(std::string &pkgName)
{
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "Error: parameter invalid");
        return DEVICEMANAGER_NULLPTR;
    }

    DMLOG(DM_LOG_INFO, "In, pkgName: %s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(listenerLock_);
    auto listenerIter = dmListener_.find(pkgName);
    if (listenerIter == dmListener_.end()) {
        DMLOG(DM_LOG_INFO, "UnRegisterDeviceManagerListener: listener not exists");
        return DEVICEMANAGER_OK;
    }

    auto recipientIter = appRecipient_.find(pkgName);
    if (recipientIter == appRecipient_.end()) {
        DMLOG(DM_LOG_INFO, "UnRegisterDeviceManagerListener: appRecipient not exists");
        dmListener_.erase(pkgName);
        return DEVICEMANAGER_OK;
    }

    auto listener = listenerIter->second;
    auto appRecipient = recipientIter->second;
    listener->RemoveDeathRecipient(appRecipient);
    appRecipient_.erase(pkgName);
    dmListener_.erase(pkgName);
    return DEVICEMANAGER_OK;
}

const std::map<std::string, sptr<IRemoteObject>> &IpcServerStub::GetDmListener()
{
    return dmListener_;
}

const sptr<IpcRemoteBroker> IpcServerStub::GetDmListener(std::string pkgName) const
{
    auto iter = dmListener_.find(pkgName);
    if (iter == dmListener_.end()) {
        return nullptr;
    }
    auto remote = iter->second;
    sptr<IpcRemoteBroker> dmListener = iface_cast<IpcRemoteBroker>(remote);
    return dmListener;
}

void AppDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DMLOG(DM_LOG_WARN, "AppDeathRecipient: OnRemoteDied");
    std::map<std::string, sptr<IRemoteObject>> listeners = IpcServerStub::GetInstance().GetDmListener();
    std::string pkgName;
    for (auto iter : listeners) {
        if (iter.second == remote.promote()) {
            pkgName = iter.first;
            break;
        }
    }
    if (pkgName.empty()) {
        DMLOG(DM_LOG_ERROR, "AppDeathRecipient: OnRemoteDied, no pkgName matched");
        return;
    }
    DMLOG(DM_LOG_INFO, "AppDeathRecipient: OnRemoteDied for %s", pkgName.c_str());
    IpcServerStub::GetInstance().UnRegisterDeviceManagerListener(pkgName);
}
} // namespace DistributedHardware
} // namespace OHOS
