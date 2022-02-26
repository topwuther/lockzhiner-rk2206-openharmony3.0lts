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

#ifndef VIRTUAL_ICOMMUNICATORAGGREGATOR_H
#define VIRTUAL_ICOMMUNICATORAGGREGATOR_H

#include <cstdint>

#include "icommunicator_aggregator.h"
#include "vitural_device.h"
#include "vitural_communicator.h"

namespace DistributedDB {
class ICommunicator;  // Forward Declaration

class VituralDevice;

class VirtualCommunicatorAggregator : public ICommunicatorAggregator {
public:
    // Return 0 as success. Return negative as error
    int Initialize(IAdapter *inAdapter) override;

    void Finalize() override;

    // If not success, return nullptr and set outErrorNo
    ICommunicator *AllocCommunicator(uint64_t commLabel, int &outErrorNo) override;
    ICommunicator *AllocCommunicator(const LabelType &commLabel, int &outErrorNo) override;

    void ReleaseCommunicator(ICommunicator *inCommunicator) override;

    int RegCommunicatorLackCallback(const CommunicatorLackCallback &onCommLack, const Finalizer &inOper) override;
    int RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper) override;
    void RunCommunicatorLackCallback(const LabelType &commLabel);
    void RunOnConnectCallback(const std::string &target, bool isConnect);

    // online a virtual device to the VirtualCommunicator, should call in main thread
    void OnlineDevice(const std::string &deviceId) const;

    // offline a virtual device to the VirtualCommunicator, should call in main thread
    void OfflineDevice(const std::string &deviceId) const;

    void DispatchMessage(const std::string &srcTarget, const std::string &dstTarget, const Message *inMsg,
        const OnSendEnd &onEnd);

    // If not success, return nullptr and set outErrorNo
    ICommunicator *AllocCommunicator(const std::string &deviceId, int &outErrorNo);

    ICommunicator *GetCommunicator(const std::string &deviceId) const;

    void Disable();

    void Enable();

    void SetBlockValue(bool value);

    bool GetBlockValue() const;

    ~VirtualCommunicatorAggregator() {};
    VirtualCommunicatorAggregator() {};

private:
    void CallSendEnd(int errCode, const OnSendEnd &onEnd);

    mutable std::mutex communicatorsLock_;
    std::map<std::string, ICommunicator *> communicators_;
    std::string remoteDeviceId_ = "real_device";
    std::mutex blockLock_;
    std::condition_variable conditionVar_;
    bool isEnable_ = true;
    bool isBlock_ = false;
    CommunicatorLackCallback onCommLack_;
    OnConnectCallback onConnect_;
};
} // namespace DistributedDB

#endif // VIRTUAL_ICOMMUNICATORAGGREGATOR_H