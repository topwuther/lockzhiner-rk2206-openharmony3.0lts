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

#ifndef OMIT_MULTI_VER
#include "multi_ver_sync_state_machine.h"

#include <cmath>
#include <climits>
#include <algorithm>

#include "message_transform.h"
#include "log_print.h"
#include "sync_types.h"
#include "db_common.h"
#include "ref_object.h"
#include "performance_analysis.h"

namespace DistributedDB {
std::vector<StateSwitchTable> MultiVerSyncStateMachine::stateSwitchTables_;
MultiVerSyncStateMachine::MultiVerSyncStateMachine()
    : context_(nullptr),
      multiVerStorage_(nullptr),
      timeSync_(nullptr),
      commitHistorySync_(nullptr),
      multiVerDataSync_(nullptr),
      valueSliceSync_(nullptr)
{
}

MultiVerSyncStateMachine::~MultiVerSyncStateMachine()
{
    Clear();
}

int MultiVerSyncStateMachine::Initialize(ISyncTaskContext *context, IKvDBSyncInterface *syncInterface,
    std::shared_ptr<Metadata> &metadata, ICommunicator *communicator)
{
    if (context == nullptr || syncInterface == nullptr || metadata == nullptr || communicator == nullptr) {
        return -E_INVALID_ARGS;
    }
    int errCode = SyncStateMachine::Initialize(context, syncInterface, metadata, communicator);
    if (errCode != E_OK) {
        return errCode;
    }

    timeSync_ = std::make_unique<TimeSync>();
    commitHistorySync_ = std::make_unique<CommitHistorySync>();
    multiVerDataSync_ = std::make_unique<MultiVerDataSync>();
    valueSliceSync_ = std::make_unique<ValueSliceSync>();

    errCode = timeSync_->Initialize(communicator, metadata, syncInterface, context->GetDeviceId());
    if (errCode != E_OK) {
        LOGE("timeSync_->Initialize failed err %d", errCode);
        goto ERROR_OUT;
    }
    LOGD("timeSync_->Initialize OK");

    // init functions below will never fail
    multiVerStorage_ = static_cast<MultiVerKvDBSyncInterface *>(syncInterface);
    commitHistorySync_->Initialize(multiVerStorage_, communicator);
    multiVerDataSync_->Initialize(multiVerStorage_, communicator);
    valueSliceSync_->Initialize(multiVerStorage_, communicator);

    context_ = static_cast<MultiVerSyncTaskContext *>(context);
    currentState_ = IDLE;
    (void)timeSync_->SyncStart();
    return E_OK;

ERROR_OUT:
    Clear();
    return errCode;
}

void MultiVerSyncStateMachine::SyncStep()
{
    RefObject::IncObjRef(context_);
    RefObject::IncObjRef(communicator_);
    int errCode = RuntimeContext::GetInstance()->ScheduleTask(
        std::bind(&MultiVerSyncStateMachine::SyncStepInnerLocked, this));
    if (errCode != E_OK) {
        LOGE("[MultiVerSyncStateMachine] Schedule SyncStep failed");
        RefObject::DecObjRef(communicator_);
        RefObject::DecObjRef(context_);
    }
}

void MultiVerSyncStateMachine::StepToIdle()
{
    currentState_ = IDLE;
    StopWatchDog();
    context_->Clear();
    PerformanceAnalysis::GetInstance()->TimeRecordEnd();
    LOGD("[MultiVerSyncStateMachine][%s{private}] step to idle", context_->GetDeviceId().c_str());
}

int MultiVerSyncStateMachine::MessageCallbackCheck(const Message *inMsg)
{
    RefObject::AutoLock lock(context_);
    if (context_->IsKilled()) {
        return -E_OBJ_IS_KILLED;
    }
    if (!IsPacketValid(inMsg)) {
        return -E_INVALID_ARGS;
    }
    if ((inMsg->GetMessageType() == TYPE_RESPONSE) && (inMsg->GetMessageId() != TIME_SYNC_MESSAGE)) {
        context_->IncSequenceId();
        int errCode = ResetWatchDog();
        if (errCode != E_OK) {
            LOGW("[MultiVerSyncStateMachine][MessageCallback] ResetWatchDog failed , err %d", errCode);
        }
    }
    return E_OK;
}

int MultiVerSyncStateMachine::ReceiveMessageCallback(Message *inMsg)
{
    if (inMsg != nullptr && inMsg->GetMessageId() == TIME_SYNC_MESSAGE) {
        return TimeSyncPacketRecvCallback(context_, inMsg);
    }
    std::lock_guard<std::mutex> lock(stateMachineLock_);
    int errCode = MessageCallbackCheck(inMsg);
    if (errCode != E_OK) {
        return errCode;
    }
    switch (inMsg->GetMessageId()) {
        case COMMIT_HISTORY_SYNC_MESSAGE:
            errCode = CommitHistorySyncPktRecvCallback(context_, inMsg);
            if ((errCode != -E_NOT_FOUND) && (inMsg->GetMessageType() == TYPE_REQUEST) && (errCode != -E_NOT_PERMIT)) {
                SyncResponseBegin(inMsg->GetSessionId());
            }
            break;
        case MULTI_VER_DATA_SYNC_MESSAGE:
            errCode = MultiVerDataPktRecvCallback(context_, inMsg);
            break;
        case VALUE_SLICE_SYNC_MESSAGE:
            errCode = ValueSlicePktRecvCallback(context_, inMsg);
            break;
        default:
            errCode = -E_NOT_SUPPORT;
            break;
    }
    if (errCode == -E_LAST_SYNC_FRAME) {
        SyncResponseEnd(inMsg->GetSessionId());
        return errCode;
    }
    if (errCode != E_OK && inMsg->GetMessageType() == TYPE_RESPONSE) {
        Abort();
    }
    return errCode;
}

void MultiVerSyncStateMachine::StepToTimeout()
{
    {
        std::lock_guard<std::mutex> lock(stateMachineLock_);
        currentState_ = SYNC_TIME_OUT;
    }
    Abort();
}

int MultiVerSyncStateMachine::CommitHistorySyncStepInner(void)
{
    int errCode = commitHistorySync_->SyncStart(context_);
    if (errCode != E_OK) {
        LOGE("[MultiVerSyncStateMachine][CommitHistorySyncStep] failed, errCode %d", errCode);
    }
    return errCode;
}

int MultiVerSyncStateMachine::MultiVerDataSyncStepInner(void)
{
    return multiVerDataSync_->SyncStart(context_);
}

int MultiVerSyncStateMachine::ValueSliceSyncStepInner(void)
{
    return valueSliceSync_->SyncStart(context_);
}

void MultiVerSyncStateMachine::SyncStepInnerLocked()
{
    if (context_->IncUsedCount() != E_OK) {
        goto SYNC_STEP_OUT;
    }

    LOGD("[MultiVerSyncStateMachine] SyncStep dst=%s{private}, state = %d",
        context_->GetDeviceId().c_str(), currentState_);
    int errCode;
    {
        std::lock_guard<std::mutex> lock(stateMachineLock_);
        switch (currentState_) {
            case COMMIT_HISTORY_SYNC:
                errCode = CommitHistorySyncStepInner();
                if (errCode != E_OK) {
                    Abort();
                }
                break;
            case MULTI_VER_DATA_ENTRY_SYNC:
                errCode = MultiVerDataSyncStepInner();
                if (errCode == -E_NOT_FOUND) {
                    Finish();
                    goto SYNC_STEP_SAFE_OUT;
                }
                break;
            case MULTI_VER_VALUE_SLICE_SYNC:
                errCode = ValueSliceSyncStepInner();
                if (errCode == -E_NOT_FOUND) {
                    int err = OneCommitSyncFinish();
                    if (err != E_OK) {
                        valueSliceSync_->SendFinishedRequest(context_);
                        Abort();
                        goto SYNC_STEP_SAFE_OUT;
                    }
                    currentState_ = MULTI_VER_DATA_ENTRY_SYNC;
                    SyncStep();
                    goto SYNC_STEP_SAFE_OUT;
                }
                break;
            default:
                break;
        }
    }

SYNC_STEP_SAFE_OUT:
    context_->SafeExit();

SYNC_STEP_OUT:
    RefObject::DecObjRef(communicator_);
    RefObject::DecObjRef(context_);
}

void MultiVerSyncStateMachine::SyncStepInner()
{
}

int MultiVerSyncStateMachine::StartSyncInner()
{
    LOGI("[MultiVerSyncStateMachine] StartSync");
    currentState_ = COMMIT_HISTORY_SYNC;
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->TimeRecordStart();
    }
    int errCode = StartWatchDog();
    if (errCode != E_OK) {
        LOGE("[MultiVerSyncStateMachine][StartSync] WatchDog start failed! err:%d", errCode);
        return errCode;
    }
    SyncStep();
    return E_OK;
}

void MultiVerSyncStateMachine::AbortInner()
{
    context_->Clear();
    StepToIdle();
    ExecNextTask();
}

const std::vector<StateSwitchTable> &MultiVerSyncStateMachine::GetStateSwitchTables() const
{
    return stateSwitchTables_;
}

int MultiVerSyncStateMachine::PrepareNextSyncTask()
{
    return StartSyncInner();
}

void MultiVerSyncStateMachine::SendSaveDataNotifyPacket(uint32_t sessionId, uint32_t sequenceId)
{
}

void MultiVerSyncStateMachine::CommErrAbort()
{
    std::lock_guard<std::mutex> lock(stateMachineLock_);
    Abort();
    RefObject::DecObjRef(context_);
}

int MultiVerSyncStateMachine::TimeSyncPacketRecvCallback(const MultiVerSyncTaskContext *context, const Message *inMsg)
{
    int errCode;
    if ((context == nullptr) || (inMsg == nullptr) || (inMsg->GetMessageId() != TIME_SYNC_MESSAGE)) {
        return -E_INVALID_ARGS;
    }
    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            errCode = timeSync_->RequestRecv(inMsg);
            return errCode;
        case TYPE_RESPONSE:
            errCode = timeSync_->AckRecv(inMsg);
            if (errCode != E_OK) {
                LOGE("[MultiVerSyncStateMachine] TimeSyncPacketRecvCallback AckRecv failed err %d", errCode);
            }
            return errCode;
        default:
            return -E_INVALID_ARGS;
    }
}

int MultiVerSyncStateMachine::CommitHistorySyncPktRecvCallback(MultiVerSyncTaskContext *context, const Message *inMsg)
{
    if ((context == nullptr) || (inMsg == nullptr) || (inMsg->GetMessageId() != COMMIT_HISTORY_SYNC_MESSAGE)) {
        return -E_INVALID_ARGS;
    }
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    int errCode;
    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            if (performance != nullptr) {
                performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_SEND_LOCAL_DATA_CHANGED_TO_COMMIT_REQUEST_RECV);
            }
            return commitHistorySync_->RequestRecvCallback(context, inMsg);
        case TYPE_RESPONSE:
            if (performance != nullptr) {
                performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_COMMIT_SEND_REQUEST_TO_ACK_RECV);
            }
            errCode = commitHistorySync_->AckRecvCallback(context, inMsg);
            if (errCode != E_OK) {
                return errCode;
            }
            currentState_ = MULTI_VER_DATA_ENTRY_SYNC;
            SyncStep();
            return errCode;
        default:
            return -E_INVALID_ARGS;
    }
}

int MultiVerSyncStateMachine::MultiVerDataPktRecvCallback(MultiVerSyncTaskContext *context, const Message *inMsg)
{
    if ((context == nullptr) || (inMsg == nullptr) || (inMsg->GetMessageId() != MULTI_VER_DATA_SYNC_MESSAGE)) {
        return -E_INVALID_ARGS;
    }
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    int errCode;
    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            return multiVerDataSync_->RequestRecvCallback(context, inMsg);
        case TYPE_RESPONSE:
            if (performance != nullptr) {
                performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_DATA_ENTRY_SEND_REQUEST_TO_ACK_RECV);
            }
            errCode = multiVerDataSync_->AckRecvCallback(context, inMsg);
            if (errCode != E_OK) {
                multiVerDataSync_->SendFinishedRequest(context);
                return errCode;
            }
            currentState_ = MULTI_VER_VALUE_SLICE_SYNC;
            SyncStep();
            return errCode;
        default:
            return -E_INVALID_ARGS;
    }
}

int MultiVerSyncStateMachine::ValueSlicePktRecvCallback(MultiVerSyncTaskContext *context, const Message *inMsg)
{
    if ((context == nullptr) || (inMsg == nullptr) || (inMsg->GetMessageId() != VALUE_SLICE_SYNC_MESSAGE)) {
        return -E_INVALID_ARGS;
    }
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    int errCode;
    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            return valueSliceSync_->RequestRecvCallback(context, inMsg);
        case TYPE_RESPONSE:
            if (performance != nullptr) {
                performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_VALUE_SLICE_SEND_REQUEST_TO_ACK_RECV);
            }
            errCode = valueSliceSync_->AckRecvCallback(context, inMsg);
            if (errCode != E_OK) {
                valueSliceSync_->SendFinishedRequest(context);
                return errCode;
            }
            currentState_ = MULTI_VER_VALUE_SLICE_SYNC;
            SyncStep();
            return errCode;
        default:
            return -E_INVALID_ARGS;
    }
}

void MultiVerSyncStateMachine::Finish()
{
    MultiVerCommitNode commit;
    std::vector<MultiVerCommitNode> commits;
    int commitsSize = context_->GetCommitsSize();
    if (commitsSize > 0) {
        context_->GetCommit(commitsSize - 1, commit);
        context_->GetCommits(commits);
        LOGD("MultiVerSyncStateMachine::Finish merge src=%s{private}", context_->GetDeviceId().c_str());
        PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
        if (performance != nullptr) {
            performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_MERGE);
        }
        int errCode = multiVerDataSync_->MergeSyncCommit(commit, commits);
        LOGD("MultiVerSyncStateMachine::Finish merge src=%s{private}, MergeSyncCommit errCode:%d",
            context_->GetDeviceId().c_str(), errCode);
        if (performance != nullptr) {
            performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_MERGE);
        }
    }
    RefObject::AutoLock lock(context_);
    context_->SetOperationStatus(SyncOperation::FINISHED_ALL);
    StepToIdle();
    ExecNextTask();
}

int MultiVerSyncStateMachine::OneCommitSyncFinish()
{
    MultiVerCommitNode commit;
    std::vector<MultiVerKvEntry *> entries;
    std::string deviceName;
    TimeOffset outOffset = 0;
    int errCode = E_OK;
    int commitIndex = context_->GetCommitIndex();

    LOGD("MultiVerSyncStateMachine::OneCommitSyncFinish  src=%s{private}, commitIndex = %d,",
        context_->GetDeviceId().c_str(), commitIndex);
    if (commitIndex > 0) {
        context_->GetCommit(commitIndex - 1, commit);
        deviceName = context_->GetDeviceId();
        context_->GetEntries(entries);
        LOGD("MultiVerSyncStateMachine::OneCommitSyncFinish src=%s{private}, entries size = %lu",
            context_->GetDeviceId().c_str(), entries.size());
        errCode = timeSync_->GetTimeOffset(outOffset);
        if (errCode != E_OK) {
            LOGI("MultiVerSyncStateMachine::OneCommitSyncFinish GetTimeOffset fail errCode:%d", errCode);
            return errCode;
        }
        TimeStamp currentLocalTime = context_->GetCurrentLocalTime();
        commit.timestamp -= outOffset;

        // Due to time sync error, commit timestamp may bigger than currentLocalTime, we need to fix the timestamp
        TimeOffset timefixOffset = (commit.timestamp < currentLocalTime) ? 0 : (commit.timestamp - currentLocalTime);
        LOGD("MultiVerSyncStateMachine::OneCommitSyncFinish src=%s{private}, timefixOffset = %lld",
            context_->GetDeviceId().c_str(), timefixOffset);
        commit.timestamp -= timefixOffset;
        for (MultiVerKvEntry *entry : entries) {
            TimeStamp timeStamp;
            entry->GetTimestamp(timeStamp);
            timeStamp = timeStamp - outOffset - timefixOffset;
            entry->SetTimestamp(timeStamp);
        }
        PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
        if (performance != nullptr) {
            performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_PUT_COMMIT_DATA);
        }
        errCode = multiVerDataSync_->PutCommitData(commit, entries, deviceName);
        LOGD("MultiVerSyncStateMachine::OneCommitSyncFinish PutCommitData src=%s{private}, errCode = %d",
            context_->GetDeviceId().c_str(), errCode);
        if (performance != nullptr) {
            performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_PUT_COMMIT_DATA);
        }
        if (errCode == E_OK) {
            context_->ReleaseEntries();
        }
    }
    DBCommon::PrintHexVector(commit.commitId, __LINE__);
    return errCode;
}

bool MultiVerSyncStateMachine::IsPacketValid(const Message *inMsg) const
{
    if (inMsg == nullptr) {
        return false;
    }

    if ((inMsg->GetMessageId() < TIME_SYNC_MESSAGE) || (inMsg->GetMessageId() > VALUE_SLICE_SYNC_MESSAGE) ||
        (inMsg->GetMessageId() == DATA_SYNC_MESSAGE)) {
        LOGE("[MultiVerSyncStateMachine] Message is invalid, id = %d", inMsg->GetMessageId());
        return false;
    }
    if (inMsg->GetMessageId() == TIME_SYNC_MESSAGE) {
        return true;
    }
    if (inMsg->GetMessageType() == TYPE_RESPONSE) {
        if ((inMsg->GetSequenceId() != context_->GetSequenceId()) ||
            (inMsg->GetSessionId() != context_->GetRequestSessionId())) {
            LOGE("[MultiVerSyncStateMachine] Message is invalid, inMsg SequenceId = %d, context seq = %d,"
                "msg session id = %d, context session = %d", inMsg->GetSequenceId(), context_->GetSequenceId(),
                inMsg->GetSessionId(), context_->GetRequestSessionId());
            return false;
        }
    }
    return true;
}

void MultiVerSyncStateMachine::Clear()
{
    commitHistorySync_ = nullptr;
    multiVerDataSync_ = nullptr;
    timeSync_ = nullptr;
    valueSliceSync_ = nullptr;
    multiVerStorage_ = nullptr;
    context_ = nullptr;
}

void MultiVerSyncStateMachine::SyncResponseBegin(uint32_t sessionId)
{
    {
        std::lock_guard<std::mutex> lock(responseInfosLock_);
        auto iter = std::find_if(responseInfos_.begin(), responseInfos_.end(), [sessionId](const ResponseInfo &info) {
            return info.sessionId == sessionId;
        });
        if (iter != responseInfos_.end()) {
            LOGE("[MultiVerSyncStateMachine][SyncResponseEnd] sessionId existed! exit.");
            return;
        }
        TimerAction timeOutCallback =
            std::bind(&MultiVerSyncStateMachine::SyncResponseTimeout, this, std::placeholders::_1);
        // To make sure context_ alive in timeout callback, we should IncObjRef for the context_.
        RefObject::IncObjRef(context_);
        TimerId timerId = 0;
        int errCode = RuntimeContext::GetInstance()->SetTimer(
            RESPONSE_TIME_OUT, timeOutCallback,
            [this]() {
                int errCode = RuntimeContext::GetInstance()->ScheduleTask([this](){ RefObject::DecObjRef(context_); });
                if (errCode != E_OK) {
                    LOGE("[MultiVerSyncStateMachine][SyncResponseEnd] timer finalizer ScheduleTask, errCode %d",
                        errCode);
                }
            },
            timerId);
        if (errCode != E_OK) {
            LOGE("[MultiVerSyncStateMachine][ResponseSessionBegin] SetTimer failed err %d", errCode);
            RefObject::DecObjRef(context_);
            return;
        }
        ResponseInfo info{sessionId, timerId};
        responseInfos_.push_back(info);
        LOGI("[MultiVerSyncStateMachine][SyncResponseBegin] begin");
    }
    multiVerStorage_->NotifyStartSyncOperation();
}

void MultiVerSyncStateMachine::SyncResponseEnd(uint32_t sessionId)
{
    {
        std::lock_guard<std::mutex> lock(responseInfosLock_);
        auto iter = std::find_if(responseInfos_.begin(), responseInfos_.end(), [sessionId](const ResponseInfo &info) {
            return info.sessionId == sessionId;
        });
        if (iter == responseInfos_.end()) {
            LOGW("[MultiVerSyncStateMachine][SyncResponseEnd] Can't find sync response %d", sessionId);
            return;
        }
        RuntimeContext::GetInstance()->RemoveTimer(iter->timerId);
        responseInfos_.erase(iter);
        LOGI("[MultiVerSyncStateMachine][SyncResponseBegin] end response");
    }
    multiVerStorage_->NotifyFinishSyncOperation();
}

int MultiVerSyncStateMachine::SyncResponseTimeout(TimerId timerId)
{
    uint32_t sessionId;
    {
        std::lock_guard<std::mutex> lock(responseInfosLock_);
        auto iter = std::find_if(responseInfos_.begin(), responseInfos_.end(), [timerId](const ResponseInfo &info) {
            return info.timerId == timerId;
        });
        if (iter == responseInfos_.end()) {
            LOGW("[MultiVerSyncStateMachine][SyncResponseTimeout] Can't find sync response timerId %d", timerId);
            return E_OK;
        }
        sessionId = iter->sessionId;
    }
    SyncResponseEnd(sessionId);
    return E_OK;
}
}
#endif