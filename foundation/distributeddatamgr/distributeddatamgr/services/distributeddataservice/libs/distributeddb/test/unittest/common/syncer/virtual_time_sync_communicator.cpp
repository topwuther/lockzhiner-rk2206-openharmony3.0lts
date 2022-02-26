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

#include "virtual_time_sync_communicator.h"

#include "log_print.h"

namespace DistributedDB {
VirtualTimeSyncCommunicator::VirtualTimeSyncCommunicator()
    : srcTimeSync_(nullptr),
      dstTimeSync_(nullptr),
      timeOffset_(0),
      deviceID_(""),
      syncTaskcontext_(nullptr),
      isEnable_(true)
{
}

VirtualTimeSyncCommunicator::~VirtualTimeSyncCommunicator() {}

int VirtualTimeSyncCommunicator::RegOnMessageCallback(const OnMessageCallback &onMessage, const Finalizer &inOper)
{
    return 0;
}

int VirtualTimeSyncCommunicator::RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper)
{
    return 0;
}

int VirtualTimeSyncCommunicator::RegOnSendableCallback(const std::function<void(void)> &onSendable,
    const Finalizer &inOper)
{
    return 0;
}

void VirtualTimeSyncCommunicator::Activate()
{
}

// return maximum allowed data size
uint32_t VirtualTimeSyncCommunicator::GetCommunicatorMtuSize() const
{
    return 0;
}

uint32_t VirtualTimeSyncCommunicator::GetCommunicatorMtuSize(const std::string &target) const
{
    return GetCommunicatorMtuSize();
}

// Get local target name for identify self
int VirtualTimeSyncCommunicator::GetLocalIdentity(std::string &outTarget) const
{
    return 0;
}

int VirtualTimeSyncCommunicator::SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock,
    uint32_t timeout)
{
    return SendMessage(dstTarget, inMsg, nonBlock, timeout, nullptr);
}

int VirtualTimeSyncCommunicator::SendMessage(const std::string &dstTarget, const Message *inMsg,
    bool nonBlock, uint32_t timeout, const OnSendEnd &onEnd)
{
    if (!isEnable_) {
        LOGD("[VirtualTimeSyncCommunicator]the VirtualTimeSyncCommunicator disabled!");
        return -E_PERIPHERAL_INTERFACE_FAIL;
    }
    LOGD("VirtualTimeSyncCommunicator::sendMessage dev = %s, syncid = %d", dstTarget.c_str(), inMsg->GetSequenceId());
    int errCode;
    if (dstTarget == deviceID_) {
        if (srcTimeSync_ == nullptr) {
            LOGD("srcTimeSync_ = nullprt");
            return -E_INVALID_ARGS;
        }
        if (syncTaskcontext_ == nullptr) {
            LOGD("syncTaskcontext_ = nullprt");
            return -E_INVALID_ARGS;
        }
        errCode = srcTimeSync_->AckRecv(inMsg);
    } else {
        if (dstTimeSync_ == nullptr) {
            LOGD("dstTimeSync_ is nullprt");
            return -E_INVALID_ARGS;
        }
        Message *msgTmp = const_cast<Message *>(inMsg);
        errCode = dstTimeSync_->RequestRecv(msgTmp);
    }
    if (inMsg != nullptr) {
        delete inMsg;
        inMsg = nullptr;
    }
    return errCode;
}

int VirtualTimeSyncCommunicator::GetRemoteCommunicatorVersion(const std::string &deviceId, uint16_t &version) const
{
    version = 0;
    return E_OK;
}

void VirtualTimeSyncCommunicator::SetTimeSync(TimeSync *srcTimeSync, TimeSync *dstTimeSync,
    const std::string &deviceID, SyncTaskContext *syncTaskcontext)
{
    srcTimeSync_ = srcTimeSync;
    dstTimeSync_ = dstTimeSync;
    deviceID_ = deviceID;
    syncTaskcontext_ = syncTaskcontext;
}

void VirtualTimeSyncCommunicator::GetTimeOffset(TimeOffset &timeOffset) const
{
    timeOffset = timeOffset_;
}

void VirtualTimeSyncCommunicator::Disable()
{
    isEnable_ = false;
}
} // namespace DistributedDB