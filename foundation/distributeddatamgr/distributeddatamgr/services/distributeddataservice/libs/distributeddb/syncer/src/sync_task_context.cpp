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

#include "sync_task_context.h"

#include <algorithm>

#include "db_errno.h"
#include "log_print.h"
#include "isync_state_machine.h"
#include "hash.h"
#include "time_helper.h"
#include "db_constant.h"

namespace DistributedDB {
std::mutex SyncTaskContext::synTaskContextSetLock_;
std::set<ISyncTaskContext *> SyncTaskContext::synTaskContextSet_;

SyncTaskContext::SyncTaskContext()
    : syncOperation_(nullptr),
      syncId_(0),
      mode_(0),
      isAutoSync_(false),
      status_(0),
      taskExecStatus_(0),
      syncInterface_(nullptr),
      communicator_(nullptr),
      stateMachine_(nullptr),
      requestSessionId_(0),
      timeHelper_(nullptr),
      remoteSoftwareVersion_(0),
      remoteSoftwareVersionId_(0),
      isCommNormal_(true),
      taskErrCode_(E_OK)
{
}

SyncTaskContext::~SyncTaskContext()
{
    if (stateMachine_ != nullptr) {
        delete stateMachine_;
        stateMachine_ = nullptr;
    }
    ClearSyncOperation();
    ClearSyncTarget();
    syncInterface_ = nullptr;
    communicator_ = nullptr;
}

int SyncTaskContext::AddSyncTarget(ISyncTarget *target)
{
    if (target == nullptr) {
        return -E_INVALID_ARGS;
    }
    int targetMode = target->GetMode();
    {
        std::lock_guard<std::mutex> lock(targetQueueLock_);
        if (target->GetTaskType() == ISyncTarget::REQUEST) {
            requestTargetQueue_.push_back(target);
        } else if (target->GetTaskType() == ISyncTarget::RESPONSE) {
            responseTargetQueue_.push_back(target);
        } else {
            return -E_INVALID_ARGS;
        }
    }
    CancelCurrentSyncRetryIfNeed(targetMode);
    if (taskExecStatus_ == RUNNING) {
        return E_OK;
    }
    if (onSyncTaskAdd_) {
        RefObject::IncObjRef(this);
        int errCode = RuntimeContext::GetInstance()->ScheduleTask([this]() {
            onSyncTaskAdd_();
            RefObject::DecObjRef(this);
        });
        if (errCode != E_OK) {
            RefObject::DecObjRef(this);
        }
    }
    return E_OK;
}

void SyncTaskContext::SetOperationStatus(int status)
{
    std::lock_guard<std::mutex> lock(operationLock_);
    if (syncOperation_ == nullptr) {
        LOGD("[SyncTaskContext][SetStatus] syncOperation is null");
        return;
    }

    int finalStatus = status;
    int operationStatus = syncOperation_->GetStatus(deviceId_);
    if (status == SyncOperation::SEND_FINISHED && operationStatus == SyncOperation::RECV_FINISHED) {
        if (GetTaskErrCode() == -E_EKEYREVOKED) {
            finalStatus = SyncOperation::EKEYREVOKED_FAILURE;
        } else {
            finalStatus = SyncOperation::FINISHED_ALL;
        }
    } else if (status == SyncOperation::RECV_FINISHED && operationStatus == SyncOperation::SEND_FINISHED) {
        if (GetTaskErrCode() == -E_EKEYREVOKED) {
            finalStatus = SyncOperation::EKEYREVOKED_FAILURE;
        } else {
            finalStatus = SyncOperation::FINISHED_ALL;
        }
    }
    syncOperation_->SetStatus(deviceId_, finalStatus);
    if (syncOperation_->CheckIsAllFinished()) {
        syncOperation_->Finished();
    }
}

void SyncTaskContext::Clear()
{
    StopTimer();
    retryTime_ = 0;
    sequenceId_ = 1;
    syncId_ = 0;
    isAutoSync_ = false;
    requestSessionId_ = 0;
    isNeedRetry_ = NO_NEED_RETRY;
    mode_ = SyncOperation::INVALID;
    status_ = SyncOperation::WAITING;
    taskErrCode_ = E_OK;
    packetId_ = 0;
}

int SyncTaskContext::RemoveSyncOperation(int syncId)
{
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    auto iter = std::find_if(requestTargetQueue_.begin(), requestTargetQueue_.end(),
        [syncId](const ISyncTarget *target) {
            if (target == nullptr) {
                return false;
            }
            return target->GetSyncId() == syncId;
        });
    if (iter != requestTargetQueue_.end()) {
        if (*iter != nullptr) {
            delete *iter;
            *iter = nullptr;
        }
        requestTargetQueue_.erase(iter);
        return E_OK;
    }
    return -E_INVALID_ARGS;
}

void SyncTaskContext::ClearSyncTarget()
{
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    for (auto &requestTarget : requestTargetQueue_) {
        if (requestTarget != nullptr) {
            delete requestTarget;
            requestTarget = nullptr;
        }
    }
    requestTargetQueue_.clear();

    for (auto &responseTarget : responseTargetQueue_) {
        if (responseTarget != nullptr) {
            delete responseTarget;
            responseTarget = nullptr;
        }
    }
    responseTargetQueue_.clear();
}

bool SyncTaskContext::IsTargetQueueEmpty() const
{
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    return requestTargetQueue_.empty() && responseTargetQueue_.empty();
}

int SyncTaskContext::GetOperationStatus() const
{
    std::lock_guard<std::mutex> lock(operationLock_);
    if (syncOperation_ == nullptr) {
        return SyncOperation::FINISHED_ALL;
    }
    return syncOperation_->GetStatus(deviceId_);
}
void SyncTaskContext::SetMode(int mode)
{
    mode_ = mode;
}

int SyncTaskContext::GetMode() const
{
    return mode_;
}
void SyncTaskContext::MoveToNextTarget()
{
    ClearSyncOperation();
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    while (!requestTargetQueue_.empty() || !responseTargetQueue_.empty()) {
        ISyncTarget *tmpTarget = nullptr;
        if (!requestTargetQueue_.empty()) {
            tmpTarget = requestTargetQueue_.front();
            requestTargetQueue_.pop_front();
        } else {
            tmpTarget = responseTargetQueue_.front();
            responseTargetQueue_.pop_front();
        }
        if (tmpTarget == nullptr) {
            LOGE("[SyncTaskContext][MoveToNextTarget] currentTarget is null skip!");
            continue;
        }
        SyncOperation *tmpOperation = nullptr;
        tmpTarget->GetSyncOperation(tmpOperation);
        if ((tmpOperation != nullptr) && tmpOperation->IsKilled()) {
            // if killed skip this syncOperation_.
            delete tmpTarget;
            tmpTarget = nullptr;
            continue;
        }
        CopyTargetData(tmpTarget);
        delete tmpTarget;
        tmpTarget = nullptr;
        break;
    }
}

uint32_t SyncTaskContext::GetSyncId() const
{
    return syncId_;
}

// Get the current task deviceId.
std::string SyncTaskContext::GetDeviceId() const
{
    return deviceId_;
}

void SyncTaskContext::SetTaskExecStatus(int status)
{
    taskExecStatus_ = status;
}

int SyncTaskContext::GetTaskExecStatus() const
{
    return taskExecStatus_;
}

bool SyncTaskContext::IsAutoSync() const
{
    return isAutoSync_;
}

int SyncTaskContext::StartTimer()
{
    if (!isAutoSync_) {
        timeout_ = DBConstant::MANUAL_SYNC_TIMEOUT;
    } else {
        timeout_ = DBConstant::AUTO_SYNC_TIMEOUT;
    }
    std::lock_guard<std::mutex> lockGuard(timerLock_);
    if (timerId_ > 0) {
        return -E_UNEXPECTED_DATA;
    }
    TimerId timerId = 0;
    RefObject::IncObjRef(this);
    TimerAction timeOutCallback = std::bind(&SyncTaskContext::TimeOut, this, std::placeholders::_1);
    int errCode = RuntimeContext::GetInstance()->SetTimer(timeout_, timeOutCallback,
        [this]() {
            int errCode = RuntimeContext::GetInstance()->ScheduleTask([this](){ RefObject::DecObjRef(this); });
            if (errCode != E_OK) {
                LOGE("[SyncTaskContext] timer finalizer ScheduleTask, errCode %d", errCode);
            }
        }, timerId);
    if (errCode != E_OK) {
        RefObject::DecObjRef(this);
        return errCode;
    }
    timerId_ = timerId;
    return errCode;
}

void SyncTaskContext::StopTimer()
{
    TimerId timerId;
    {
        std::lock_guard<std::mutex> lockGuard(timerLock_);
        timerId = timerId_;
        if (timerId_ == 0) {
            return;
        }
        timerId_ = 0;
    }
    RuntimeContext::GetInstance()->RemoveTimer(timerId);
}

int SyncTaskContext::ModifyTimer(int milliSeconds)
{
    std::lock_guard<std::mutex> lockGuard(timerLock_);
    if (timerId_ == 0) {
        return -E_UNEXPECTED_DATA;
    }
    return RuntimeContext::GetInstance()->ModifyTimer(timerId_, milliSeconds);
}

void SyncTaskContext::SetRetryTime(int retryTime)
{
    retryTime_ = retryTime;
}

int SyncTaskContext::GetRetryTime() const
{
    return retryTime_;
}

void SyncTaskContext::SetRetryStatus(int isNeedRetry)
{
    isNeedRetry_ = isNeedRetry;
}

int SyncTaskContext::GetRetryStatus() const
{
    return isNeedRetry_;
}

TimerId SyncTaskContext::GetTimerId() const
{
    return timerId_;
}

uint32_t SyncTaskContext::GetRequestSessionId() const
{
    return requestSessionId_;
}

void SyncTaskContext::IncSequenceId()
{
    sequenceId_++;
}

uint32_t SyncTaskContext::GetSequenceId() const
{
    return sequenceId_;
}

void SyncTaskContext::ReSetSequenceId()
{
    sequenceId_ = 1;
}

void SyncTaskContext::IncPacketId()
{
    packetId_++;
}

uint64_t SyncTaskContext::GetPacketId() const
{
    return packetId_;
}

int SyncTaskContext::GetTimeoutTime() const
{
    return timeout_;
}

void SyncTaskContext::SetTimeoutCallback(const TimerAction &timeOutCallback)
{
    timeOutCallback_ = timeOutCallback;
}

void SyncTaskContext::SetTimeOffset(TimeOffset offset)
{
    timeOffset_ = offset;
}

TimeOffset SyncTaskContext::GetTimeOffset() const
{
    return timeOffset_;
}

int SyncTaskContext::StartStateMachine()
{
    return stateMachine_->StartSync();
}

int SyncTaskContext::ReceiveMessageCallback(Message *inMsg)
{
    int errCode = E_OK;
    if (IncUsedCount() == E_OK) {
        errCode = stateMachine_->ReceiveMessageCallback(inMsg);
        SafeExit();
    }
    return errCode;
}

void SyncTaskContext::RegOnSyncTask(const std::function<int(void)> &callback)
{
    onSyncTaskAdd_ = callback;
}

int SyncTaskContext::IncUsedCount()
{
    AutoLock lock(this);
    if (IsKilled()) {
        LOGI("[SyncTaskContext] IncUsedCount isKilled");
        return -E_OBJ_IS_KILLED;
    }
    usedCount_++;
    return E_OK;
}

void SyncTaskContext::SafeExit()
{
    AutoLock lock(this);
    usedCount_--;
    if (usedCount_ < 1) {
        safeKill_.notify_one();
    }
}

TimeStamp SyncTaskContext::GetCurrentLocalTime() const
{
    if (timeHelper_ == nullptr) {
        return TimeHelper::INVALID_TIMESTAMP;
    }
    return timeHelper_->GetTime();
}

void SyncTaskContext::Abort(int status)
{
    Clear();
}

void SyncTaskContext::CommErrHandlerFunc(int errCode, ISyncTaskContext *context, int32_t sessionId)
{
    {
        std::lock_guard<std::mutex> lock(synTaskContextSetLock_);
        if (synTaskContextSet_.count(context) == 0) {
            LOGI("[SyncTaskContext][CommErrHandle] context has been killed");
            return;
        }

        // IncObjRef to maker sure context not been killed. after the lock_guard
        RefObject::IncObjRef(context);
    }

    static_cast<SyncTaskContext *>(context)->CommErrHandlerFuncInner(errCode, sessionId);
    RefObject::DecObjRef(context);
}

void SyncTaskContext::SetRemoteSoftwareVersion(uint32_t version)
{
    std::lock_guard<std::mutex> lock(remoteSoftwareVersionLock_);
    remoteSoftwareVersion_ = version;
    remoteSoftwareVersionId_++;
}

uint32_t SyncTaskContext::GetRemoteSoftwareVersion() const
{
    std::lock_guard<std::mutex> lock(remoteSoftwareVersionLock_);
    return remoteSoftwareVersion_;
}

uint64_t SyncTaskContext::GetRemoteSoftwareVersionId() const
{
    std::lock_guard<std::mutex> lock(remoteSoftwareVersionLock_);
    return remoteSoftwareVersionId_;
}

bool SyncTaskContext::IsCommNormal() const
{
    return isCommNormal_;
}

void SyncTaskContext::CommErrHandlerFuncInner(int errCode, uint32_t sessionId)
{
    {
        RefObject::AutoLock lock(this);
        if (sessionId != requestSessionId_) {
            return;
        }

        if (errCode == E_OK) {
            isCommNormal_ = true;
            return;
        }

        isCommNormal_ = false;
    }
    LOGE("[SyncTaskContext][CommErr] errCode %d", errCode);
    stateMachine_->CommErrAbort();
}

int SyncTaskContext::TimeOut(TimerId id)
{
    int errCode = E_OK;
    if (!timeOutCallback_) {
        return errCode;
    }
    if (IncUsedCount() == E_OK) {
        errCode = timeOutCallback_(id);
        SafeExit();
    }
    return errCode;
}

void SyncTaskContext::CopyTargetData(const ISyncTarget *target)
{
    mode_ = target->GetMode();
    status_ = SyncOperation::SYNCING;
    isNeedRetry_ = SyncTaskContext::NO_NEED_RETRY;
    taskErrCode_ = E_OK;
    packetId_ = 0;
    target->GetSyncOperation(syncOperation_);
    ReSetSequenceId();

    if (syncOperation_ != nullptr) {
        // IncRef for syncOperation_ to make sure syncOperation_ is valid, when setStatus
        RefObject::IncObjRef(syncOperation_);
        syncId_ = syncOperation_->GetSyncId();
        isAutoSync_ = syncOperation_->IsAutoSync();
        requestSessionId_ = Hash::Hash32Func(deviceId_ + std::to_string(syncId_) +
            std::to_string(TimeHelper::GetSysCurrentTime()));
        LOGI("[SyncTaskContext][copyTarget] mode_ = %d, syncId_ = %d, isAutoSync_ = %d, dev = %s{private}",
            mode_, syncId_, isAutoSync_, deviceId_.c_str());
    } else {
        isAutoSync_ = false;
        LOGI("[SyncTaskContext][copyTarget] for response data dev %s{private} ", deviceId_.c_str());
    }
}

void SyncTaskContext::KillWait()
{
    StopTimer();
    stateMachine_->Abort();
    LOGW("[SyncTaskContext] Try to kill a context, now wait.");
    bool noDeadLock = WaitLockedUntil(
        safeKill_,
        [this]() {
            if (usedCount_ < 1) {
                return true;
            }
            return false;
        },
        KILL_WAIT_SECONDS);
    if (!noDeadLock) {
        LOGE("[SyncTaskContext] Dead lock maybe happen, we stop waiting the task exit.");
    } else {
        LOGW("[SyncTaskContext] Wait the task exit ok.");
    }
    std::lock_guard<std::mutex> lock(synTaskContextSetLock_);
    synTaskContextSet_.erase(this);
}

void SyncTaskContext::ClearSyncOperation()
{
    std::lock_guard<std::mutex> lock(operationLock_);
    if (syncOperation_ != nullptr) {
        RefObject::DecObjRef(syncOperation_);
        syncOperation_ = nullptr;
    }
}

void SyncTaskContext::CancelCurrentSyncRetryIfNeed(int newTargetMode)
{
    AutoLock(this);
    if (!isAutoSync_) {
        return;
    }
    if (newTargetMode == mode_ || newTargetMode == SyncOperation::PUSH_AND_PULL) {
        SetRetryTime(ISyncStateMachine::RETRY_TIME);
        ModifyTimer(DBConstant::AUTO_SYNC_TIMEOUT);
    }
}

int SyncTaskContext::GetTaskErrCode() const
{
    return taskErrCode_;
}

void SyncTaskContext::SetTaskErrCode(int errCode)
{
    taskErrCode_ = errCode;
}
} // namespace DistributedDB
