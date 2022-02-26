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

#define LOG_TAG "processCommunication"

#include "process_communicator_impl.h"

#include "log_print.h"

namespace OHOS {
namespace AppDistributedKv {
using namespace DistributedDB;
ProcessCommunicatorImpl::ProcessCommunicatorImpl()
{
}

ProcessCommunicatorImpl::~ProcessCommunicatorImpl()
{
    ZLOGE("destructor.");
}

DBStatus ProcessCommunicatorImpl::Start(const std::string &processLabel)
{
    ZLOGI("init commProvider");
    thisProcessLabel_ = processLabel;
    PipeInfo pi = {thisProcessLabel_, ""};
    Status errCode = CommunicationProvider::GetInstance().Start(pi);
    if (errCode != Status::SUCCESS) {
        ZLOGE("commProvider_ Start Fail.");
        return DBStatus::DB_ERROR;
    }
    return DBStatus::OK;
}

DBStatus ProcessCommunicatorImpl::Stop()
{
    PipeInfo pi = {thisProcessLabel_, ""};
    Status errCode = CommunicationProvider::GetInstance().Stop(pi);
    if (errCode != Status::SUCCESS) {
        ZLOGE("commProvider_ Stop Fail.");
        return DBStatus::DB_ERROR;
    }
    return DBStatus::OK;
}

DBStatus ProcessCommunicatorImpl::RegOnDeviceChange(const OnDeviceChange &callback)
{
    {
        std::lock_guard<std::mutex> onDeviceChangeLockGard(onDeviceChangeMutex_);
        onDeviceChangeHandler_ = callback;
    }

    PipeInfo pi = {thisProcessLabel_, ""};
    if (callback) {
        Status errCode = CommunicationProvider::GetInstance().StartWatchDeviceChange(this, pi);
        if (errCode != Status::SUCCESS) {
            ZLOGE("commProvider_ StartWatchDeviceChange Fail.");
            return DBStatus::DB_ERROR;
        }
    } else {
        Status errCode = CommunicationProvider::GetInstance().StopWatchDeviceChange(this, pi);
        if (errCode != Status::SUCCESS) {
            ZLOGE("commProvider_ StopWatchDeviceChange Fail.");
            return DBStatus::DB_ERROR;
        }
    }

    return DBStatus::OK;
}

DBStatus ProcessCommunicatorImpl::RegOnDataReceive(const OnDataReceive &callback)
{
    {
        std::lock_guard<std::mutex> onDataReceiveLockGard(onDataReceiveMutex_);
        onDataReceiveHandler_ = callback;
    }

    PipeInfo pi = {thisProcessLabel_, ""};
    if (callback) {
        Status errCode = CommunicationProvider::GetInstance().StartWatchDataChange(this, pi);
        if (errCode != Status::SUCCESS) {
            ZLOGE("commProvider_ StartWatchDataChange Fail.");
            return DBStatus::DB_ERROR;
        }
    } else {
        Status errCode = CommunicationProvider::GetInstance().StopWatchDataChange(this, pi);
        if (errCode != Status::SUCCESS) {
            ZLOGE("commProvider_ StopWatchDataChange Fail.");
            return DBStatus::DB_ERROR;
        }
    }
    return DBStatus::OK;
}

DBStatus ProcessCommunicatorImpl::SendData(const DeviceInfos &dstDevInfo, const uint8_t *data, uint32_t length)
{
    PipeInfo pi = {thisProcessLabel_, ""};
    DeviceId destination;
    destination.deviceId = dstDevInfo.identifier;
    Status errCode = CommunicationProvider::GetInstance().SendData(pi, destination, data, static_cast<int>(length));
    if (errCode != Status::SUCCESS) {
        ZLOGE("commProvider_ SendData Fail.");
        return DBStatus::DB_ERROR;
    }

    return DBStatus::OK;
}

uint32_t ProcessCommunicatorImpl::GetMtuSize()
{
    return MTU_SIZE;
}

uint32_t ProcessCommunicatorImpl::GetMtuSize(const DeviceInfos &devInfo)
{
    ZLOGI("GetMtuSize start");
    std::vector<DeviceInfo> devInfos = CommunicationProvider::GetInstance().GetDeviceList();
    for (auto const &entry : devInfos) {
        ZLOGI("GetMtuSize deviceType: %{public}s", entry.deviceType.c_str());
        bool isWatch = (entry.deviceType == SMART_WATCH_TYPE || entry.deviceType == CHILDREN_WATCH_TYPE);
        if (entry.deviceId == devInfo.identifier && isWatch) {
            return MTU_SIZE_WATCH;
        }
    }
    return MTU_SIZE;
}
DeviceInfos ProcessCommunicatorImpl::GetLocalDeviceInfos()
{
    DeviceInfos localDevInfos;
    DeviceInfo devInfo = CommunicationProvider::GetInstance().GetLocalDevice();
    localDevInfos.identifier = devInfo.deviceId;
    return localDevInfos;
}

std::vector<DeviceInfos> ProcessCommunicatorImpl::GetRemoteOnlineDeviceInfosList()
{
    std::vector<DeviceInfos> remoteDevInfos;
    std::vector<DeviceInfo> devInfoVec = CommunicationProvider::GetInstance().GetDeviceList();
    for (auto const &entry : devInfoVec) {
        DeviceInfos remoteDev;
        remoteDev.identifier = entry.deviceId;
        remoteDevInfos.push_back(remoteDev);
    }
    return remoteDevInfos;
}

bool ProcessCommunicatorImpl::IsSameProcessLabelStartedOnPeerDevice(const DeviceInfos &peerDevInfo)
{
    PipeInfo pi = {thisProcessLabel_, ""};
    DeviceId di = {peerDevInfo.identifier};
    return CommunicationProvider::GetInstance().IsSameStartedOnPeer(pi, di);
}

void ProcessCommunicatorImpl::OnMessage(const DeviceInfo &info, const uint8_t *ptr, const int size,
                                        __attribute__((unused)) const PipeInfo &pipeInfo) const
{
    std::lock_guard<std::mutex> onDataReceiveLockGuard(onDataReceiveMutex_);
    if (onDataReceiveHandler_ == nullptr) {
        ZLOGE("onDataReceiveHandler_ invalid.");
        return;
    }
    DeviceInfos devInfo;
    devInfo.identifier = info.deviceId;
    onDataReceiveHandler_(devInfo, ptr, static_cast<uint32_t>(size));
}

void ProcessCommunicatorImpl::OnDeviceChanged(const DeviceInfo &info, const DeviceChangeType &type) const
{
    std::lock_guard<std::mutex> onDeviceChangeLockGuard(onDeviceChangeMutex_);
    if (onDeviceChangeHandler_ == nullptr) {
        ZLOGE("onDeviceChangeHandler_ invalid.");
        return;
    }
    DeviceInfos devInfo;
    devInfo.identifier = info.deviceId;
    onDeviceChangeHandler_(devInfo, (type == DeviceChangeType::DEVICE_ONLINE));
}
}  // namespace AppDistributedKv
}  // namespace OHOS
