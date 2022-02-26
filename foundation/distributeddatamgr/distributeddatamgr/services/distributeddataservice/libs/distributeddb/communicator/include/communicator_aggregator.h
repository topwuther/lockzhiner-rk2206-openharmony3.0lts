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

#ifndef COMMUNICATORAGGREGATOR_H
#define COMMUNICATORAGGREGATOR_H

#include <map>
#include <mutex>
#include <string>
#include <atomic>
#include <thread>
#include <cstdint>
#include <condition_variable>
#include "iadapter.h"
#include "parse_result.h"
#include "icommunicator.h"
#include "frame_combiner.h"
#include "frame_retainer.h"
#include "send_task_scheduler.h"
#include "icommunicator_aggregator.h"

namespace DistributedDB {
// Forward Declarations
class Communicator;
class SerialBuffer;
class CommunicatorLinker;

struct TaskConfig {
    bool nonBlock;
    uint32_t timeout;
    Priority prio;
};

/*
 * Upper layer Module should comply with calling convention, Inner Module interface will not do excessive check
 */
class CommunicatorAggregator : public ICommunicatorAggregator {
public:
    CommunicatorAggregator();
    ~CommunicatorAggregator() override;

    DISABLE_COPY_ASSIGN_MOVE(CommunicatorAggregator);

    // See ICommunicatorAggregator for detail
    int Initialize(IAdapter *inAdapter) override;
    // Must not call any other functions if Finalize had been called. In fact, Finalize has no chance to be called.
    void Finalize() override;

    ICommunicator *AllocCommunicator(uint64_t commLabel, int &outErrorNo) override;
    ICommunicator *AllocCommunicator(const LabelType &commLabel, int &outErrorNo) override;

    void ReleaseCommunicator(ICommunicator *inCommunicator) override;

    int RegCommunicatorLackCallback(const CommunicatorLackCallback &onCommLack, const Finalizer &inOper) override;
    int RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper) override;

    // return optimal allowed data size(Some header is taken into account and subtract)
    uint32_t GetCommunicatorAggregatorMtuSize() const;
    uint32_t GetCommunicatorAggregatorMtuSize(const std::string &target) const;
    int GetLocalIdentity(std::string &outTarget) const;
    // Get the protocol version of remote target. Return -E_NOT_FOUND if no record.
    int GetRemoteCommunicatorVersion(const std::string &target, uint16_t &outVersion) const;

    // Called by communicator to make itself really in work
    void ActivateCommunicator(const LabelType &commLabel);

    // SerialBuffer surely is heap memory, CreateSendTask responsible for lifecycle
    int CreateSendTask(const std::string &dstTarget, SerialBuffer *inBuff, FrameType inType,
        const TaskConfig &inConfig, const OnSendEnd &onEnd = nullptr);

    static void EnableCommunicatorNotFoundFeedback(bool isEnable);

private:
    // Working in a dedicated thread
    void SendDataRoutine();
    void SendPacketsAndDisposeTask(const SendTask &inTask,
        const std::vector<std::pair<const uint8_t *, uint32_t>> &eachPacket);

    int RetryUntilTimeout(SendTask &inTask, uint32_t timeout, Priority inPrio);
    void TaskFinalizer(const SendTask &inTask, int result);
    void NotifySendableToAllCommunicator();

    // Call from Adapter by register these function
    void OnBytesReceive(const std::string &srcTarget, const uint8_t *bytes, uint32_t length);
    void OnTargetChange(const std::string &target, bool isConnect);
    void OnSendable(const std::string &target);

    void OnFragmentReceive(const std::string &srcTarget, const uint8_t *bytes, uint32_t length,
        const ParseResult &inResult);

    int OnCommLayerFrameReceive(const std::string &srcTarget, const ParseResult &inResult);
    int OnAppLayerFrameReceive(const std::string &srcTarget, const uint8_t *bytes,
        uint32_t length, const ParseResult &inResult);
    int OnAppLayerFrameReceive(const std::string &srcTarget, SerialBuffer *&inFrameBuffer, const ParseResult &inResult);
    // Function with suffix NoMutex should be called with mutex in the caller
    int TryDeliverAppLayerFrameToCommunicatorNoMutex(const std::string &srcTarget, SerialBuffer *&inFrameBuffer,
        const LabelType &toLabel);

    // Auxiliary function for cutting short primary function
    int RegCallbackToAdapter();
    void UnRegCallbackFromAdapter();
    void GenerateLocalSourceId();
    bool ReGenerateLocalSourceIdIfNeed();

    // Feedback related functions
    void TriggerVersionNegotiation(const std::string &dstTarget);
    void TryToFeedbackWhenCommunicatorNotFound(const std::string &dstTarget, const LabelType &dstLabel,
        const SerialBuffer *inOriFrame);
    void TriggerCommunicatorNotFoundFeedback(const std::string &dstTarget, const LabelType &dstLabel, Message* &oriMsg);

    // Record the protocol version of remote target.
    void SetRemoteCommunicatorVersion(const std::string &target, uint16_t version);

    DECLARE_OBJECT_TAG(CommunicatorAggregator);

    static std::atomic<bool> isCommunicatorNotFoundFeedbackEnable_;

    std::atomic<bool> shutdown_;
    std::atomic<uint32_t> incFrameId_;
    std::atomic<uint64_t> localSourceId_;
    // Handle related
    mutable std::mutex commMapMutex_;
    std::map<LabelType, std::pair<Communicator *, bool>> commMap_; // bool true indicate communicator activated
    FrameCombiner combiner_;
    FrameRetainer retainer_;
    SendTaskScheduler scheduler_;
    IAdapter *adapterHandle_ = nullptr;
    CommunicatorLinker *commLinker_ = nullptr;
    // Thread related
    std::thread exclusiveThread_;
    bool wakingSignal_ = false;
    mutable std::mutex wakingMutex_;
    std::condition_variable wakingCv_;
    // RetryCreateTask related
    mutable std::mutex retryMutex_;
    std::condition_variable retryCv_;
    // Remote target version related
    mutable std::mutex versionMapMutex_;
    std::map<std::string, uint16_t> versionMap_;
    // CommLack Callback related
    CommunicatorLackCallback onCommLackHandle_;
    Finalizer onCommLackFinalizer_;
    mutable std::mutex onCommLackMutex_;
    // Connect Callback related
    OnConnectCallback onConnectHandle_;
    Finalizer onConnectFinalizer_;
    mutable std::mutex onConnectMutex_;
};
} // namespace DistributedDB

#endif // COMMUNICATORAGGREGATOR_H
