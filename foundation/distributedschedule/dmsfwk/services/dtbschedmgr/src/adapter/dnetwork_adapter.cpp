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

#include "adapter/dnetwork_adapter.h"

#include <chrono>
#include <mutex>
#include <thread>
#include <unistd.h>

#include "datetime_ex.h"
#include "dtbschedmgr_log.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std::chrono_literals;

namespace {
constexpr int32_t DEVICE_ID_SIZE = 65;
constexpr int32_t RETRY_REGISTER_CALLBACK_TIMES = 5;
const std::string PKG_NAME = "DBinderBus_" + std::to_string(getpid());

constexpr int32_t NON_ANONYMIZED_LENGTH = 6;
const std::string EMPTY_DEVICE_ID = "";
}

std::shared_ptr<AppExecFwk::EventHandler> DnetworkAdapter::dnetworkHandler_;
std::mutex DnetworkAdapter::listenerSetMutex_;
std::set<std::shared_ptr<DeviceListener>> DnetworkAdapter::listenerSet_;

std::shared_ptr<DnetworkAdapter> DnetworkAdapter::GetInstance()
{
    static auto instance = std::make_shared<DnetworkAdapter>();
    return instance;
}

void DnetworkAdapter::Init()
{
    nodeStateCb_.events = EVENT_NODE_STATE_MASK;
    nodeStateCb_.onNodeOnline = OnNodeOnline;
    nodeStateCb_.onNodeOffline = OnNodeOffline;
    nodeStateCb_.onNodeBasicInfoChanged = OnNodeBasicInfoChanged;

    auto runner = AppExecFwk::EventRunner::Create("dmsDnetwork");
    dnetworkHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
}

void DnetworkAdapter::OnNodeOnline(NodeBasicInfo* info)
{
    if (info == nullptr) {
        HILOGE("OnNodeOnline invalid parameter");
        return;
    }

    HILOGI("OnNodeOnline netwokId = %{public}s", AnonymizeDeviceId(info->networkId).c_str());
    auto onlineNotifyTask = [info = *info]() {
        std::lock_guard<std::mutex> autoLock(listenerSetMutex_);
        for (auto& listener : listenerSet_) {
            listener->OnDeviceOnline(&info);
        }
    };
    if (!dnetworkHandler_->PostTask(onlineNotifyTask)) {
        HILOGE("OnNodeOnline post task failed");
        return;
    }
}

void DnetworkAdapter::OnNodeOffline(NodeBasicInfo* info)
{
    if (info == nullptr) {
        HILOGE("OnNodeOffline invalid parameter");
        return;
    }

    HILOGI("OnNodeOffline networkId = %{public}s", AnonymizeDeviceId(info->networkId).c_str());
    auto offlineNotifyTask = [info = *info]() {
        std::lock_guard<std::mutex> autoLock(listenerSetMutex_);
        for (auto& listener : listenerSet_) {
            listener->OnDeviceOffline(&info);
        }
    };
    if (!dnetworkHandler_->PostTask(offlineNotifyTask)) {
        HILOGE("OnNodeOffline post task failed");
        return;
    }
}

void DnetworkAdapter::OnNodeBasicInfoChanged(NodeBasicInfoType type, NodeBasicInfo* info)
{
    HILOGD("OnNodeBasicInfoChanged called");
}

bool DnetworkAdapter::AddDeviceChangeListener(const std::shared_ptr<DeviceListener>& listener)
{
    HILOGD("AddDeviceChangeListener called");
    if (dnetworkHandler_ == nullptr) {
        HILOGE("DnetworkAdapter handler is null");
        return false;
    }

    {
        std::lock_guard<std::mutex> autoLock(listenerSetMutex_);
        if (listenerSet_.find(listener) == listenerSet_.end()) {
            listenerSet_.insert(listener);
        }
        if (listenerSet_.size() > 1) {
            return true;
        }
    }

    auto registerTask = [this]() {
        HILOGD("AddDeviceChangeListener register task...");
        int32_t retryTimes = 0;
        int32_t errCode = ERR_OK;
        while (retryTimes++ < RETRY_REGISTER_CALLBACK_TIMES) {
            errCode = RegNodeDeviceStateCb(PKG_NAME.c_str(), &nodeStateCb_);
            if (errCode == ERR_OK) {
                break;
            }

            HILOGD("AddDeviceChangeListener Reg errCode = %{public}d, retrying...", errCode);
            errCode = UnregNodeDeviceStateCb(&nodeStateCb_);
            HILOGD("AddDeviceChangeListener Unreg errCode = %{public}d", errCode);
            std::this_thread::sleep_for(1s);
        }
        HILOGI("AddDeviceChangeListener %{public}s", (errCode == ERR_OK) ? "success" : "timeout");
    };
    if (!dnetworkHandler_->PostTask(registerTask)) {
        HILOGE("AddDeviceChangeListener post task failed");
        return false;
    }
    return true;
}

void DnetworkAdapter::RemoveDeviceChangeListener(const std::shared_ptr<DeviceListener>& listener)
{
    HILOGD("RemoveDeviceChangeListener called");
    {
        std::lock_guard<std::mutex> autoLock(listenerSetMutex_);
        listenerSet_.erase(listener);
        if (listenerSet_.size() > 0) {
            return;
        }
    }

    int32_t errCode = UnregNodeDeviceStateCb(&nodeStateCb_);
    if (errCode != ERR_OK) {
        HILOGE("RemoveDeviceChangeListener remove failed, errCode = %{public}d", errCode);
    }
    HILOGE("RemoveDeviceChangeListener remove ok");
}

std::shared_ptr<NodeBasicInfo> DnetworkAdapter::GetLocalBasicInfo()
{
    auto info = std::make_shared<NodeBasicInfo>();
    int32_t errCode = GetLocalNodeDeviceInfo(PKG_NAME.c_str(), info.get());
    if (errCode != ERR_OK) {
        HILOGE("DnetworkAdapter::GetLocalBasicInfo errCode = %{public}d", errCode);
        return nullptr;
    }
    return info;
}

std::string DnetworkAdapter::GetUdidByNodeId(const std::string& nodeId)
{
    return GetUuidOrUdidByNodeId(nodeId, NodeDeivceInfoKey::NODE_KEY_UDID);
}

std::string DnetworkAdapter::GetUuidByNodeId(const std::string& nodeId)
{
    return GetUuidOrUdidByNodeId(nodeId, NodeDeivceInfoKey::NODE_KEY_UUID);
}

std::string DnetworkAdapter::GetUuidOrUdidByNodeId(const std::string& nodeId, NodeDeivceInfoKey keyType)
{
    if (nodeId.empty()) {
        return std::string();
    }

    char uuidOrUdid[DEVICE_ID_SIZE] = {0};
    int32_t ret = GetNodeKeyInfo(PKG_NAME.c_str(), nodeId.c_str(), keyType,
        reinterpret_cast<uint8_t*>(uuidOrUdid), DEVICE_ID_SIZE);
    return (ret == ERR_OK) ? std::string(uuidOrUdid) : std::string();
}

std::string DnetworkAdapter::AnonymizeDeviceId(const std::string& deviceId)
{
    if (deviceId.length() < NON_ANONYMIZED_LENGTH) {
        return EMPTY_DEVICE_ID;
    }
    std::string anonDeviceId = deviceId.substr(0, NON_ANONYMIZED_LENGTH);
    anonDeviceId.append("******");
    return anonDeviceId;
}
} // namespace DistributedSchedule
} // namespace OHOS
