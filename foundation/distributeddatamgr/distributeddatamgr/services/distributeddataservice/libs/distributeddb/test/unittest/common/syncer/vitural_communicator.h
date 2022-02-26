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

#ifndef VIRTUAL_COMMUNICATOR_H
#define VIRTUAL_COMMUNICATOR_H

#include <string>
#include <cstdint>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "ref_object.h"
#include "serial_buffer.h"
#include "icommunicator.h"
#include "vitural_device.h"

namespace DistributedDB {
class VirtualCommunicatorAggregator;

class VituralDevice;

class VirtualCommunicator : public ICommunicator {
public:
    VirtualCommunicator(const std::string &deviceId, VirtualCommunicatorAggregator *communicatorAggregator);
    ~VirtualCommunicator() override;

    DISABLE_COPY_ASSIGN_MOVE(VirtualCommunicator);

    int RegOnMessageCallback(const OnMessageCallback &onMessage, const Finalizer &inOper) override;
    int RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper) override;
    int RegOnSendableCallback(const std::function<void(void)> &onSendable, const Finalizer &inOper) override;

    void Activate() override;

    uint32_t GetCommunicatorMtuSize() const override;
    uint32_t GetCommunicatorMtuSize(const std::string &target) const override;
    int GetLocalIdentity(std::string &outTarget) const override;

    int SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock, uint32_t timeout) override;
    int SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock, uint32_t timeout,
        const OnSendEnd &onEnd) override;

    int GetRemoteCommunicatorVersion(const std::string &deviceId, uint16_t &version) const override;

    void CallbackOnMessage(const std::string &srcTarget, Message *inMsg) const;

    void CallbackOnConnect(const std::string &target, bool isConnec) const;

    int GeneralVirtualSyncId();

    void Disable();

    void Enable();

    void SetDeviceId(const std::string &deviceId);

    std::string GetDeviceId() const;

    bool IsEnabled() const;

private:
    int TimeSync();
    int DataSync();
    int WaterMarkSync();

    mutable std::mutex onMessageLock_;
    OnMessageCallback onMessage_;

    mutable std::mutex onConnectLock_;
    OnConnectCallback onConnect_;

    std::string remoteDeviceId_ = "real_device";
    std::mutex syncIdLock_;
    int currentSyncId_ = 1000;
    bool isEnable_ = true;
    std::string deviceId_;

    std::mutex onAggregatorLock_;
    VirtualCommunicatorAggregator *communicatorAggregator_;
};
} // namespace DistributedDB

#endif // VIRTUAL_COMMUNICATOR_H
