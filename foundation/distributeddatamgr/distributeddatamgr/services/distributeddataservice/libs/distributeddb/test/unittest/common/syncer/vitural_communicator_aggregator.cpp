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
#include "vitural_communicator.h"

#include <cstdint>
#include <thread>

#include "db_errno.h"
#include "vitural_communicator_aggregator.h"
#include "log_print.h"

namespace DistributedDB {
int VirtualCommunicatorAggregator::Initialize(IAdapter *inAdapter)
{
    return E_OK;
}

void VirtualCommunicatorAggregator::Finalize()
{
}

// If not success, return nullptr and set outErrorNo
ICommunicator *VirtualCommunicatorAggregator::AllocCommunicator(uint64_t commLabel, int &outErrorNo)
{
    if (isEnable_) {
        return AllocCommunicator(remoteDeviceId_, outErrorNo);
    }
    return nullptr;
}

ICommunicator *VirtualCommunicatorAggregator::AllocCommunicator(const LabelType &commLabel, int &outErrorNo)
{
    if (isEnable_) {
        return AllocCommunicator(remoteDeviceId_, outErrorNo);
    }
    return nullptr;
}

void VirtualCommunicatorAggregator::ReleaseCommunicator(ICommunicator *inCommunicator)
{
    // Called in main thread only
    VirtualCommunicator *communicator = static_cast<VirtualCommunicator *>(inCommunicator);
    OfflineDevice(communicator->GetDeviceId());
    {
        std::lock_guard<std::mutex> lock(communicatorsLock_);
        communicators_.erase(communicator->GetDeviceId());
    }
    RefObject::KillAndDecObjRef(communicator);
    communicator = nullptr;
}

int VirtualCommunicatorAggregator::RegCommunicatorLackCallback(const CommunicatorLackCallback &onCommLack,
    const Finalizer &inOper)
{
    onCommLack_ = onCommLack;
    return E_OK;
}

int VirtualCommunicatorAggregator::RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper)
{
    onConnect_ = onConnect;
    RunOnConnectCallback("deviceId", true);
    return E_OK;
}

void VirtualCommunicatorAggregator::RunCommunicatorLackCallback(const LabelType &commLabel)
{
    if (onCommLack_) {
        onCommLack_(commLabel);
    }
}

void VirtualCommunicatorAggregator::RunOnConnectCallback(const std::string &target, bool isConnect)
{
    if (onConnect_) {
        onConnect_(target, isConnect);
    }
}

void VirtualCommunicatorAggregator::OnlineDevice(const std::string &deviceId) const
{
    if (!isEnable_) {
        return;
    }

    // Called in main thread only
    for (const auto &iter : communicators_) {
        VirtualCommunicator *communicatorTmp = static_cast<VirtualCommunicator *>(iter.second);
        if (iter.first != deviceId) {
            communicatorTmp->CallbackOnConnect(deviceId, true);
        }
    }
}

void VirtualCommunicatorAggregator::OfflineDevice(const std::string &deviceId) const
{
    if (!isEnable_) {
        return;
    }

    // Called in main thread only
    for (const auto &iter : communicators_) {
        VirtualCommunicator *communicatorTmp = static_cast<VirtualCommunicator *>(iter.second);
        if (iter.first != deviceId) {
            communicatorTmp->CallbackOnConnect(deviceId, false);
        }
    }
}

ICommunicator *VirtualCommunicatorAggregator::AllocCommunicator(const std::string &deviceId, int &outErrorNo)
{
    // Called in main thread only
    VirtualCommunicator *communicator = new (std::nothrow) VirtualCommunicator(deviceId, this);
    if (communicator == nullptr) {
        outErrorNo = -E_OUT_OF_MEMORY;
    }
    {
        std::lock_guard<std::mutex> lock(communicatorsLock_);
        communicators_.insert(std::pair<std::string, ICommunicator *>(deviceId, communicator));
    }
    OnlineDevice(deviceId);
    return communicator;
}

ICommunicator *VirtualCommunicatorAggregator::GetCommunicator(const std::string &deviceId) const
{
    std::lock_guard<std::mutex> lock(communicatorsLock_);
    auto iter = communicators_.find(deviceId);
    if (iter != communicators_.end()) {
        VirtualCommunicator *communicator = static_cast<VirtualCommunicator *>(iter->second);
        return communicator;
    }
    return nullptr;
}

void VirtualCommunicatorAggregator::DispatchMessage(const std::string &srcTarget, const std::string &dstTarget,
    const Message *inMsg, const OnSendEnd &onEnd)
{
    if (VirtualCommunicatorAggregator::GetBlockValue()) {
        std::unique_lock<std::mutex> lock(blockLock_);
        conditionVar_.wait(lock);
    }

    if (!isEnable_) {
        LOGD("[VirtualCommunicatorAggregator] DispatchMessage, VirtualCommunicatorAggregator is disabled");
        delete inMsg;
        inMsg = nullptr;
        return CallSendEnd(-E_PERIPHERAL_INTERFACE_FAIL, onEnd);
    }
    std::lock_guard<std::mutex> lock(communicatorsLock_);
    auto iter = communicators_.find(dstTarget);
    if (iter != communicators_.end()) {
        LOGE("[VirtualCommunicatorAggregator] DispatchMessage, find dstTarget %s", dstTarget.c_str());
        VirtualCommunicator *communicator = static_cast<VirtualCommunicator *>(iter->second);
        if (!communicator->IsEnabled()) {
            LOGE("[VirtualCommunicatorAggregator] DispatchMessage, find dstTarget %s disabled", dstTarget.c_str());
            delete inMsg;
            inMsg = nullptr;
            return CallSendEnd(-E_PERIPHERAL_INTERFACE_FAIL, onEnd);
        }
        Message *msg = const_cast<Message *>(inMsg);
        msg->SetTarget(srcTarget);
        RefObject::IncObjRef(communicator);
        std::thread thread([communicator, srcTarget, msg]() {
            communicator->CallbackOnMessage(srcTarget, msg);
            RefObject::DecObjRef(communicator);
        });
        thread.detach();
        CallSendEnd(OK, onEnd);
    } else {
        LOGE("[VirtualCommunicatorAggregator] DispatchMessage, can't find dstTarget %s", dstTarget.c_str());
        delete inMsg;
        inMsg = nullptr;
        CallSendEnd(-E_NOT_FOUND, onEnd);
    }
}

void VirtualCommunicatorAggregator::SetBlockValue(bool value)
{
    std::unique_lock<std::mutex> lock(blockLock_);
    isBlock_ = value;
    if (!value) {
        conditionVar_.notify_all();
    }
}

bool VirtualCommunicatorAggregator::GetBlockValue() const
{
    return isBlock_;
}

void VirtualCommunicatorAggregator::Disable()
{
    isEnable_ = false;
}

void VirtualCommunicatorAggregator::Enable()
{
    LOGD("[VirtualCommunicatorAggregator] enable");
    isEnable_ = true;
}

void VirtualCommunicatorAggregator::CallSendEnd(int errCode, const OnSendEnd &onEnd)
{
    if (onEnd) {
        (void)RuntimeContext::GetInstance()->ScheduleTask([errCode, onEnd]() {
            onEnd(errCode);
        });
    }
}
} // namespace DistributedDB

