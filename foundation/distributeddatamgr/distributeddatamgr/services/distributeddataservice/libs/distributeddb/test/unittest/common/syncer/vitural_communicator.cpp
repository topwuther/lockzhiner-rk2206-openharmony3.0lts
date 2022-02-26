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

#include "vitural_communicator_aggregator.h"
#include "sync_engine.h"
#include "log_print.h"

namespace DistributedDB {
int VirtualCommunicator::RegOnMessageCallback(const OnMessageCallback &onMessage, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> lock(onMessageLock_);
    onMessage_ = onMessage;
    return E_OK;
}

int VirtualCommunicator::RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> lock(onConnectLock_);
    onConnect_ = onConnect;
    return E_OK;
}

int VirtualCommunicator::RegOnSendableCallback(const std::function<void(void)> &onSendable, const Finalizer &inOper)
{
    return E_OK;
}

void VirtualCommunicator::Activate()
{
}

int VirtualCommunicator::SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock,
    uint32_t timeout)
{
    return SendMessage(dstTarget, inMsg, nonBlock, timeout, nullptr);
}

int VirtualCommunicator::SendMessage(const std::string &dstTarget, const Message *inMsg,
    bool nonBlock, uint32_t timeout, const OnSendEnd &onEnd)
{
    AutoLock lock(this);
    if (IsKilled()) {
        return -E_OBJ_IS_KILLED;
    }
    if (!isEnable_) {
        LOGD("[VirtualCommunicator] the VirtualCommunicator disabled!");
        return -E_PERIPHERAL_INTERFACE_FAIL;
    }
    if (dstTarget == deviceId_) {
        delete inMsg;
        inMsg = nullptr;
        return E_OK;
    }
    communicatorAggregator_->DispatchMessage(deviceId_, dstTarget, inMsg, onEnd);
    return E_OK;
}

int VirtualCommunicator::GetRemoteCommunicatorVersion(const std::string &deviceId, uint16_t &version) const
{
    version = 1;
    return E_OK;
}

void VirtualCommunicator::CallbackOnMessage(const std::string &srcTarget, Message *inMsg) const
{
    std::lock_guard<std::mutex> lock(onMessageLock_);
    if (isEnable_ && onMessage_ && (srcTarget != deviceId_)) {
        onMessage_(srcTarget, inMsg);
    } else {
        delete inMsg;
        inMsg = nullptr;
    }
}

void VirtualCommunicator::CallbackOnConnect(const std::string &target, bool isConnect) const
{
    std::lock_guard<std::mutex> lock(onConnectLock_);
    if (isEnable_ && onConnect_ && (target != deviceId_)) {
        onConnect_(target, isConnect);
    }
}

uint32_t VirtualCommunicator::GetCommunicatorMtuSize() const
{
    return 5 * 1024 * 1024; // 5M
}

uint32_t VirtualCommunicator::GetCommunicatorMtuSize(const std::string &target) const
{
    return GetCommunicatorMtuSize();
}

int VirtualCommunicator::GetLocalIdentity(std::string &outTarget) const
{
    outTarget = deviceId_;
    return E_OK;
}

int VirtualCommunicator::GeneralVirtualSyncId()
{
    std::lock_guard<std::mutex> lock(syncIdLock_);
    currentSyncId_++;
    return currentSyncId_;
}

void VirtualCommunicator::Disable()
{
    isEnable_ = false;
}

void VirtualCommunicator::Enable()
{
    isEnable_ = true;
}

void VirtualCommunicator::SetDeviceId(const std::string &deviceId)
{
    deviceId_ = deviceId;
}

std::string VirtualCommunicator::GetDeviceId() const
{
    return deviceId_;
}

bool VirtualCommunicator::IsEnabled() const
{
    return isEnable_;
}

VirtualCommunicator::~VirtualCommunicator()
{
}

VirtualCommunicator::VirtualCommunicator(const std::string &deviceId,
    VirtualCommunicatorAggregator *communicatorAggregator)
    : deviceId_(deviceId), communicatorAggregator_(communicatorAggregator)
{
}
} // namespace DistributedDB