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

#include "dtbschedmgr_device_info_storage.h"

#include <chrono>
#include <thread>

#include "distributed_device_node_listener.h"
#include "distributed_sched_adapter.h"
#include "dtbschedmgr_log.h"

#include "ipc_object_proxy.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std::chrono_literals;

namespace {
constexpr int32_t RETRY_TIMES = 30;
constexpr int32_t NON_ANONYMIZE_LENGTH = 6;
const std::string EMPTY_DEVICE_ID = "";
constexpr int32_t CONNECT_SOFTBUS_RETRY_TIMES = 60;
}

IMPLEMENT_SINGLE_INSTANCE(DtbschedmgrDeviceInfoStorage);

bool DtbschedmgrDeviceInfoStorage::Init()
{
    if (initHandler_ == nullptr) {
        auto deviceInfoStorageRunner = AppExecFwk::EventRunner::Create("DmsDeviceInfoStorageManager");
        initHandler_ = std::make_shared<AppExecFwk::EventHandler>(deviceInfoStorageRunner);
    }

    auto func = [this]() {
        HILOGI("DtbschedmgrDeviceInfoStorage begin connect softbus");
        for (int32_t retryTimes = 0; retryTimes <= CONNECT_SOFTBUS_RETRY_TIMES; retryTimes++) {
            if (ConnectSoftbus()) {
                return;
            }
            HILOGE("DtbschedmgrDeviceInfoStorage retry connect softbus %{public}d times", retryTimes);
            std::this_thread::sleep_for(1s);
        }
    };
    if (!initHandler_->PostTask(func)) {
        HILOGE("DtbschedmgrDeviceInfoStorage::Init handler postTask failed");
        return false;
    }
    return true;
}

bool DtbschedmgrDeviceInfoStorage::ConnectSoftbus()
{
    bool isReady = WaitForDnetworkReady();
    if (!isReady) {
        HILOGE("DtbschedmgrDeviceInfoStorage::ConnectSoftbus wait Dnetwork failed!");
        return false;
    }
    std::shared_ptr<DnetworkAdapter> dnetworkAdapter = DnetworkAdapter::GetInstance();
    if (dnetworkAdapter == nullptr) {
        HILOGE("DtbschedmgrDeviceInfoStorage DnetworkAdapter::GetInstance() null");
        return false;
    }
    if (!InitNodeIdManager(dnetworkAdapter)) {
        HILOGE("DtbschedmgrDeviceInfoStorage::InitNodeIdManager failed");
        return false;
    }
    HILOGI("DtbschedmgrDeviceInfoStorage::ConnectSoftbus success");
    return true;
}

bool DtbschedmgrDeviceInfoStorage::InitNodeIdManager(std::shared_ptr<DnetworkAdapter> dnetworkAdapter)
{
    if (nodeIdMgrHandler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("DmsNodeIdManager");
        nodeIdMgrHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }

    deviceNodeListener_ = std::make_shared<DistributedDeviceNodeListener>();
    if (!dnetworkAdapter->AddDeviceChangeListener(deviceNodeListener_)) {
        HILOGE("DtbschedmgrDeviceInfoStorage::AddDeviceChangeListener failed");
        deviceNodeListener_ = nullptr;
        return false;
    }
    return true;
}

void DtbschedmgrDeviceInfoStorage::Stop()
{
    if (deviceNodeListener_ != nullptr) {
        DnetworkAdapter::GetInstance()->RemoveDeviceChangeListener(deviceNodeListener_);
        deviceNodeListener_ = nullptr;
    }
}

bool DtbschedmgrDeviceInfoStorage::WaitForDnetworkReady()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HILOGE("WaitForDnetworkReady failed to get samgr!");
        return false;
    }
    int32_t retryTimeout = RETRY_TIMES;
    do {
        auto dnetwork = samgr->CheckSystemAbility(SOFTBUS_SERVER_SA_ID);
        if (dnetwork != nullptr) {
            IPCObjectProxy* proxy = reinterpret_cast<IPCObjectProxy*>(dnetwork.GetRefPtr());
            // make sure the proxy is not dead
            if (proxy != nullptr && !proxy->IsObjectDead()) {
                return true;
            }
        }
        HILOGI("DtbschedmgrDeviceInfoStorage::Waiting for dnentwork service...");
        std::this_thread::sleep_for(1s);
        if (--retryTimeout <= 0) {
            HILOGI("DtbschedmgrDeviceInfoStorage::Waiting for dnentwork service timeout(30)s");
            return false;
        }
    } while (true);
    return false;
}

bool DtbschedmgrDeviceInfoStorage::GetLocalDeviceId(std::string& deviceId)
{
    return GetLocalDeviceFromDnet(deviceId);
}

bool DtbschedmgrDeviceInfoStorage::GetLocalDeviceFromDnet(std::string& deviceId)
{
    auto dnetworkAdapter = DnetworkAdapter::GetInstance();
    if (dnetworkAdapter == nullptr) {
        HILOGE("DtbschedmgrDeviceInfoStorage::GetLocalDeviceFromDnet dnetworkAdapter null");
        return false;
    }
    std::shared_ptr<NodeBasicInfo> localNode = dnetworkAdapter->GetLocalBasicInfo();
    if (localNode != nullptr) {
        deviceId = localNode->networkId;
        HILOGI("get local deviceId from DnetworkAdapter, deviceId = %{public}s",
            AnonymizeDeviceId(deviceId).c_str());
        return true;
    }
    HILOGE("DtbschedmgrDeviceInfoStorage::GetLocalDeviceFromDnet localNode null");
    return false;
}

std::string DtbschedmgrDeviceInfoStorage::AnonymizeDeviceId(const std::string& deviceId)
{
    if (deviceId.length() < NON_ANONYMIZE_LENGTH) {
        return EMPTY_DEVICE_ID;
    }
    std::string anonDeviceId = deviceId.substr(0, NON_ANONYMIZE_LENGTH);
    anonDeviceId.append("******");
    return anonDeviceId;
}

void DtbschedmgrDeviceInfoStorage::DeviceOnlineNotify(const std::shared_ptr<DmsDeviceInfo> devInfo)
{
    if (devInfo == nullptr) {
        HILOGE("DtbschedmgrDeviceInfoStorage::DeviceOnlineNotify devInfo null");
        return;
    }
    std::string devId = devInfo->GetDeviceId();
    HILOGE("DtbschedmgrDeviceInfoStorage::DeviceOnlineNotify deviceId = %{public}s",
        AnonymizeDeviceId(devId).c_str());

    if (nodeIdMgrHandler_ == nullptr) {
        HILOGE("DtbschedmgrDeviceInfoStorage::DeviceOnlineNotify nodeIdMgrHandler null");
        return;
    }
    auto nodeOnline = [this, devInfo]() {
        HILOGI("DtbschedmgrDeviceInfoStorage::DeviceOnlineNotify called");
        std::string deviceId = devInfo->GetDeviceId();
        DistributedSchedAdapter::GetInstance().DeviceOnline(deviceId);
    };
    if (!nodeIdMgrHandler_->PostTask(nodeOnline)) {
        HILOGE("DtbschedmgrDeviceInfoStorage::DeviceOnlineNotify handler postTask failed");
    }
}

void DtbschedmgrDeviceInfoStorage::DeviceOfflineNotify(const std::string& deviceId)
{
    if (deviceId.empty()) {
        HILOGE("DtbschedmgrDeviceInfoStorage::DeviceOfflineNotify deviceId empty");
        return;
    }
    if (nodeIdMgrHandler_ == nullptr) {
        HILOGE("DtbschedmgrDeviceInfoStorage::DeviceOfflineNotify nodeIdMgrHandler null");
        return;
    }
    auto nodeOffline = [this, deviceId]() {
        HILOGI("DtbschedmgrDeviceInfoStorage::DeviceOfflineNotify called");
        DistributedSchedAdapter::GetInstance().DeviceOffline(deviceId);
    };
    if (!nodeIdMgrHandler_->PostTask(nodeOffline)) {
        HILOGE("DtbschedmgrDeviceInfoStorage::DeviceOfflineNotify handler postTask failed");
    }
}

void DtbschedmgrDeviceInfoStorage::OnDeviceInfoChanged(const std::string& deviceId, DeviceInfoType type)
{
    HILOGI("DtbschedmgrDeviceInfoStorage::OnDeviceInfoChanged called");
}

void DnetServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("DnetServiceDeathRecipient::OnRemoteDied dnetwork service died");
    DtbschedmgrDeviceInfoStorage::GetInstance().Init();
}
}
}