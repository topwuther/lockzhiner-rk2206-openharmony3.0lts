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

#ifndef COMMUNICATOR_LINKER_H
#define COMMUNICATOR_LINKER_H

#include <set>
#include <map>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <cstdlib>
#include <functional>
#include "ref_object.h"
#include "serial_buffer.h"
#include "communicator_type_define.h"

namespace DistributedDB {
class CommunicatorAggregator; // Forward Declaration

class CommunicatorLinker : public virtual RefObject {
public:
    explicit CommunicatorLinker(CommunicatorAggregator *inAggregator);
    ~CommunicatorLinker();

    DISABLE_COPY_ASSIGN_MOVE(CommunicatorLinker);

    void Initialize();

    // Create async task to send out label_exchange and waiting for label_exchange_ack.
    // If waiting timeout, pass the send&wait task to overrall timing retry task.
    int TargetOnline(const std::string &inTarget, std::set<LabelType> &outRelatedLabels);

    // Clear all labels related to this target. Let no longer waiting for ack of this target.
    // The caller should notify all related communicator about this target offline.
    int TargetOffline(const std::string &inTarget, std::set<LabelType> &outRelatedLabels);

    // Add local label. Create async task to send out label_exchange and waiting for label_exchange_ack.
    // If waiting timeout, pass the send&wait task to overrall timing retry task.
    // Find out targets for this label that is already online.
    // The caller should notify communicator of this label about already online target.
    int IncreaseLocalLabel(const LabelType &inLabel, std::set<std::string> &outOnlineTarget);

    // Del local label. Create async task to send out label_exchange and waiting for label_exchange_ack.
    // If waiting timeout, pass the send&wait task to overrall timing retry task.
    int DecreaseLocalLabel(const LabelType &inLabel);

    // Compare the latest labels with previous Label, find out label changes.
    // The caller should notify the target changes according to label changes.
    // Update the online labels of this target. Send out label_exchange_ack.
    int ReceiveLabelExchange(const std::string &inTarget, const std::set<LabelType> &inLatestLabels,
        uint64_t inDistinctValue, uint64_t inSequenceId, std::map<LabelType, bool> &outChangeLabels);

    // Waiting finish if the ack is what linker wait by check inSequenceId
    // Similarly, stop the retry task of this Target.
    int ReceiveLabelExchangeAck(const std::string &inTarget, uint64_t inDistinctValue, uint64_t inSequenceId);

    std::set<std::string> GetOnlineRemoteTarget() const;

    bool IsRemoteTargetOnline(const std::string &inTarget) const;
private:
    DECLARE_OBJECT_TAG(CommunicatorLinker);

    // inCountDown is in millisecond
    void SuspendByOnceTimer(const std::function<void(void)> &inAction, uint32_t inCountDown);

    // This function should be called under protection of entireInfoMutex_
    void DetectDistinctValueChange(const std::string &inTarget, uint64_t inDistinctValue);

    int TriggerLabelExchangeEvent(const std::string &toTarget);
    int TriggerLabelExchangeAckEvent(const std::string &toTarget, uint64_t inSequenceId);

    void SendLabelExchange(const std::string &toTarget, SerialBuffer *inBuff, uint64_t inSequenceId,
        uint32_t inRetransmitCount);
    void SendLabelExchangeAck(const std::string &toTarget, SerialBuffer *inBuff, uint64_t inSequenceId,
        uint64_t inAckTriggerId);

    uint64_t localDistinctValue_ = 0;
    std::atomic<uint64_t> incSequenceId_;
    std::atomic<uint64_t> incAckTriggerId_;
    CommunicatorAggregator *aggregator_ = nullptr;

    mutable std::mutex entireInfoMutex_;
    // Point out the distinctValue for each target in order to detect malfunctioning "target offline"
    std::map<std::string, uint64_t> targetDistinctValue_;
    // Point out the largest sequenceId of LabelExchange that ever received for each target
    std::map<std::string, uint64_t> topRecvLabelSeq_;
    // Point out currently which sequenceId of ack is being waited for each target
    std::map<std::string, uint64_t> waitAckSeq_;
    // Point out the largest sequenceId of LabelExchangeAck that ever received for each target
    std::map<std::string, uint64_t> recvAckSeq_;
    // Point out the latest ackTriggerId for each target in order to abort outdated triggered event
    std::map<std::string, uint64_t> ackTriggerId_;
    // Core Info : Online Labels
    std::set<LabelType> localOnlineLabels_;
    std::set<std::string> remoteOnlineTarget_;
    // remember the opened labels no matter target now online or offline
    std::map<std::string, std::set<LabelType>> targetMapOnlineLabels_;
};
}

#endif