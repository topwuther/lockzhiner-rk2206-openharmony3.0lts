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

#include "device_manager.h"

#include <algorithm>

#include "message_transform.h"
#include "parcel.h"
#include "db_errno.h"
#include "message.h"
#include "log_print.h"
#include "performance_analysis.h"
#include "sync_types.h"

namespace DistributedDB {
DeviceManager::DeviceManager() : communicator_(nullptr)
{
}

DeviceManager::~DeviceManager()
{
    if (communicator_ != nullptr) {
        RefObject::DecObjRef(communicator_);
        communicator_ = nullptr;
    }
}

uint32_t DeviceManager::CalculateLen()
{
    return Parcel::GetUInt64Len();
}

int DeviceManager::RegisterTransformFunc()
{
    TransformFunc func;
    func.computeFunc = [](const Message *msg) { return DeviceManager::CalculateLen(); };
    // LocalDataChanged has no dataPct
    func.serializeFunc = [](uint8_t *buffer, uint32_t length, const Message *inMsg) { return E_OK; };
    func.deserializeFunc = [](const uint8_t *buffer, uint32_t length, Message *inMsg) { return E_OK; };
    return MessageTransform::RegTransformFunction(LOCAL_DATA_CHANGED, func);
}

// Initialize the DeviceManager
int DeviceManager::Initialize(ICommunicator *communicator, const std::function<void(std::string)> &callback)
{
    if (communicator == nullptr) {
        return -E_INVALID_ARGS;
    }
    RefObject::IncObjRef(communicator);
    communicator_ = communicator;
    RegDeviceOnLineCallBack(callback);

    return E_OK;
}

void DeviceManager::RegDeviceOnLineCallBack(const std::function<void(std::string)> &callback)
{
    onlineCallback_ = callback;
}

void DeviceManager::RegDeviceOffLineCallBack(const std::function<void(std::string)> &callback)
{
    offlineCallback_ = callback;
}

void DeviceManager::OnDeviceConnectCallback(const std::string &targetDev, bool isConnect)
{
    LOGD("[DeviceManager] DeviceConnectCallback dev = %s{private}, status = %d", targetDev.c_str(), isConnect);
    if (targetDev.empty()) {
        LOGE("[DeviceManager] DeviceConnectCallback invalid device!");
    }
    if (isConnect) {
        {
            std::lock_guard<std::mutex> lockOnline(devicesLock_);
            devices_.insert(targetDev);
        }
        if (onlineCallback_) {
            onlineCallback_(targetDev);
            LOGD("[DeviceManager] DeviceConnectCallback call online callback");
        }
    } else {
        {
            std::lock_guard<std::mutex> lockOffline(devicesLock_);
            devices_.erase(targetDev);
        }
        if (offlineCallback_) {
            offlineCallback_(targetDev);
            LOGD("[DeviceManager] DeviceConnectCallback call offline callback");
        }
    }
}

void DeviceManager::GetOnlineDevices(std::vector<std::string> &devices) const
{
    std::lock_guard<std::mutex> lock(devicesLock_);
    devices.assign(devices_.begin(), devices_.end());
}

int DeviceManager::SendBroadCast(uint32_t msgId)
{
    if (msgId == LOCAL_DATA_CHANGED) {
        return SendLocalDataChanged();
    }
    LOGE("[DeviceManager] invalid BroadCast msgId:%d", msgId);
    return -E_INVALID_ARGS;
}

int DeviceManager::SendLocalDataChanged()
{
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_SEND_LOCAL_DATA_CHANGED_TO_COMMIT_REQUEST_RECV);
    }
    std::vector<std::string> copyDevices;
    GetOnlineDevices(copyDevices);
    if (copyDevices.empty()) {
        LOGI("[DeviceManager] no device online to SendLocalDataChanged!");
    }
    for (const auto &deviceId : copyDevices) {
        Message *msg = new (std::nothrow) Message();
        if (msg == nullptr) {
            LOGE("[DeviceManager] Message alloc failed when SendBroadCast!");
            return -E_OUT_OF_MEMORY;
        }
        msg->SetMessageId(LOCAL_DATA_CHANGED);
        msg->SetTarget(deviceId);
        int errCode = communicator_->SendMessage(deviceId, msg, false, SEND_TIME_OUT);
        if (errCode != E_OK) {
            LOGE("[DeviceManager] SendLocalDataChanged to dev %s{private} failed. err %d",
                deviceId.c_str(), errCode);
            delete msg;
            msg = nullptr;
        }
    }
    return E_OK;
}

bool DeviceManager::IsDeviceOnline(const std::string &deviceId) const
{
    std::lock_guard<std::mutex> lock(devicesLock_);
    auto iter = std::find(devices_.begin(), devices_.end(), deviceId);
    return (iter != devices_.end());
}
} // namespace DistributedDB