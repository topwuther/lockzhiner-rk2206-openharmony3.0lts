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

#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <set>
#include <mutex>
#include <chrono>
#include <string>
#include <cstdint>
#include <functional>
#include <condition_variable>
#include "serial_buffer.h"
#include "icommunicator.h"
#include "communicator_aggregator.h"

namespace DistributedDB {
class Communicator : public ICommunicator {
public:
    Communicator(CommunicatorAggregator *inCommAggregator, const LabelType &inLabel);
    ~Communicator() override;

    DISABLE_COPY_ASSIGN_MOVE(Communicator);

    int RegOnMessageCallback(const OnMessageCallback &onMessage, const Finalizer &inOper) override;
    int RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper) override;
    int RegOnSendableCallback(const std::function<void(void)> &onSendable, const Finalizer &inOper) override;

    void Activate() override;

    uint32_t GetCommunicatorMtuSize() const override;
    uint32_t GetCommunicatorMtuSize(const std::string &target) const override;
    int GetLocalIdentity(std::string &outTarget) const override;
    // Get the protocol version of remote target. Return -E_NOT_FOUND if no record.
    int GetRemoteCommunicatorVersion(const std::string &target, uint16_t &outVersion) const override;

    int SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock, uint32_t timeout) override;
    int SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock, uint32_t timeout,
        const OnSendEnd &onEnd) override;

    // Call by CommunicatorAggregator directly
    void OnBufferReceive(const std::string &srcTarget, const SerialBuffer *inBuf);

    // Call by CommunicatorAggregator directly
    void OnConnectChange(const std::string &target, bool isConnect);

    // Call by CommunicatorAggregator directly
    void OnSendAvailable();

    // Call by CommunicatorAggregator directly
    LabelType GetCommunicatorLabel() const;

private:
    void TriggerVersionNegotiation(const std::string &dstTarget);
    void TriggerUnknownMessageFeedback(const std::string &dstTarget, Message* &oriMsg);

    DECLARE_OBJECT_TAG(Communicator);

    CommunicatorAggregator *commAggrHandle_ = nullptr;
    LabelType commLabel_;

    std::set<std::string> onlineTargets_; // Actually protected by connectHandleMutex_

    OnMessageCallback onMessageHandle_;
    OnConnectCallback onConnectHandle_;
    std::function<void(void)> onSendableHandle_;
    Finalizer onMessageFinalizer_;
    Finalizer onConnectFinalizer_;
    Finalizer onSendableFinalizer_;
    std::mutex messageHandleMutex_;
    std::mutex connectHandleMutex_;
    std::mutex sendableHandleMutex_;
};
} // namespace DistributedDB

#endif // COMMUNICATOR_H
