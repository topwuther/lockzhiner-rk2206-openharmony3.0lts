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

#include "single_ver_sync_task_context.h"

#include <algorithm>

#include "db_errno.h"
#include "log_print.h"
#include "isyncer.h"
#include "single_ver_sync_state_machine.h"
#include "single_ver_sync_target.h"

namespace DistributedDB {
SingleVerSyncTaskContext::SingleVerSyncTaskContext()
    : SyncTaskContext(),
      token_(nullptr),
      endMark_(0),
      needClearRemoteStaleData_(false)
{}

SingleVerSyncTaskContext::~SingleVerSyncTaskContext()
{
    token_ = nullptr;
}

int SingleVerSyncTaskContext::Initialize(const std::string &deviceId,
    IKvDBSyncInterface *syncInterface, std::shared_ptr<Metadata> &metadata, ICommunicator *communicator)
{
    if (deviceId.empty() || syncInterface == nullptr || metadata == nullptr ||
        communicator == nullptr) {
        return -E_INVALID_ARGS;
    }
    stateMachine_ = new (std::nothrow) SingleVerSyncStateMachine;
    if (stateMachine_ == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    deviceId_ = deviceId;
    TimerAction timeOutCallback;
    int errCode = stateMachine_->Initialize(this, syncInterface, metadata, communicator);
    if (errCode != E_OK) {
        LOGE("[SingleVerSyncTaskContext] stateMachine Initialize failed, err %d.", errCode);
        goto ERROR_OUT;
    }

    timeHelper_ = std::make_unique<TimeHelper>();
    errCode = timeHelper_->Initialize(syncInterface, metadata);
    if (errCode != E_OK) {
        LOGE("[SingleVerSyncTaskContext] timeHelper Initialize failed, err %d.", errCode);
        goto ERROR_OUT;
    }
    timeOutCallback = std::bind(&SyncStateMachine::TimeoutCallback,
        static_cast<SingleVerSyncStateMachine *>(stateMachine_),
        std::placeholders::_1);
    SetTimeoutCallback(timeOutCallback);

    syncInterface_ = syncInterface;
    communicator_ = communicator;
    taskExecStatus_ = INIT;
    OnKill([this]() { this->KillWait(); });
    {
        std::lock_guard<std::mutex> lock(synTaskContextSetLock_);
        synTaskContextSet_.insert(this);
    }
    return errCode;

ERROR_OUT:
    delete stateMachine_;
    stateMachine_ = nullptr;
    return errCode;
}

int SingleVerSyncTaskContext::AddSyncOperation(SyncOperation *operation)
{
    if (operation == nullptr) {
        return -E_INVALID_ARGS;
    }

    // If auto sync, just update the end watermark
    if (operation->IsAutoSync()) {
        std::lock_guard<std::mutex> lock(targetQueueLock_);
        auto iter = std::find_if(requestTargetQueue_.begin(), requestTargetQueue_.end(), [](const ISyncTarget *target) {
            if (target == nullptr) {
                return false;
            }
            return target->IsAutoSync();
        });
        if (iter != requestTargetQueue_.end()) {
            static_cast<SingleVerSyncTarget *>(*iter)->SetEndWaterMark(timeHelper_->GetTime());
            operation->SetStatus(deviceId_, SyncOperation::FINISHED_ALL);
            return E_OK;
        }
    }

    auto *newTarget = new (std::nothrow) SingleVerSyncTarget;
    if (newTarget == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    newTarget->SetSyncOperation(operation);
    TimeStamp timstamp = timeHelper_->GetTime();
    newTarget->SetEndWaterMark(timstamp);
    newTarget->SetTaskType(ISyncTarget::REQUEST);
    AddSyncTarget(newTarget);
    return E_OK;
}

void SingleVerSyncTaskContext::SetEndMark(WaterMark endMark)
{
    endMark_ = endMark;
}

WaterMark SingleVerSyncTaskContext::GetEndMark() const
{
    return endMark_;
}

void SingleVerSyncTaskContext::GetContinueToken(ContinueToken &outToken) const
{
    outToken = token_;
}

void SingleVerSyncTaskContext::SetContinueToken(ContinueToken token)
{
    token_ = token;
    return;
}

void SingleVerSyncTaskContext::ReleaseContinueToken()
{
    if (token_ != nullptr) {
        static_cast<SingleVerKvDBSyncInterface *>(syncInterface_)->ReleaseContinueToken(token_);
        token_ = nullptr;
    }
}

int SingleVerSyncTaskContext::PopResponseTarget(SingleVerSyncTarget &target)
{
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    LOGD("[SingleVerSyncTaskContext] GetFrontExtWaterMarak size = %d", responseTargetQueue_.size());
    if (!responseTargetQueue_.empty()) {
        ISyncTarget *tmpTarget = responseTargetQueue_.front();
        responseTargetQueue_.pop_front();
        target = *(static_cast<SingleVerSyncTarget *>(tmpTarget));
        delete tmpTarget;
        tmpTarget = nullptr;
        return E_OK;
    }
    return -E_LENGTH_ERROR;
}

int SingleVerSyncTaskContext::GetRspTargetQueueSize() const
{
    std::lock_guard<std::mutex> lock(targetQueueLock_);
    return responseTargetQueue_.size();
}

void SingleVerSyncTaskContext::SetResponseSessionId(uint32_t responseSessionId)
{
    responseSessionId_ = responseSessionId;
}

uint32_t SingleVerSyncTaskContext::GetResponseSessionId() const
{
    return responseSessionId_;
}

void SingleVerSyncTaskContext::CopyTargetData(const ISyncTarget *target)
{
    const SingleVerSyncTarget *targetTmp = static_cast<const SingleVerSyncTarget *>(target);
    SyncTaskContext::CopyTargetData(target);
    mode_ = targetTmp->GetMode();
    endMark_ = targetTmp->GetEndWaterMark();
    if (mode_ == SyncOperation::RESPONSE_PULL) {
        responseSessionId_ = targetTmp->GetResponseSessionId();
    }
}

void SingleVerSyncTaskContext::Clear()
{
    retryTime_ = 0;
    ClearSyncOperation();
    SyncTaskContext::Clear();
    SetMode(SyncOperation::INVALID);
    syncId_ = 0;
    isAutoSync_ = false;
    SetOperationStatus(SyncOperation::WAITING);
    SetEndMark(0);
    SetResponseSessionId(0);
}

void SingleVerSyncTaskContext::Abort(int status)
{
    {
        std::lock_guard<std::mutex> lock(operationLock_);
        if (syncOperation_ != nullptr) {
            syncOperation_->SetStatus(deviceId_, status);
            if ((status >= SyncOperation::FINISHED_ALL)) {
                UnlockObj();
                if (syncOperation_->CheckIsAllFinished()) {
                    syncOperation_->Finished();
                }
                LockObj();
            }
        }
    }
    StopFeedDogForSync(SyncDirectionFlag::SEND);
    StopFeedDogForSync(SyncDirectionFlag::RECEIVE);
    Clear();
}

void SingleVerSyncTaskContext::EnableClearRemoteStaleData(bool enable)
{
    needClearRemoteStaleData_ = enable;
}

bool SingleVerSyncTaskContext::IsNeedClearRemoteStaleData() const
{
    return needClearRemoteStaleData_;
}

bool SingleVerSyncTaskContext::StartFeedDogForSync(uint32_t time, SyncDirectionFlag flag)
{
    return stateMachine_->StartFeedDogForSync(time, flag);
}

void SingleVerSyncTaskContext::StopFeedDogForSync(SyncDirectionFlag flag)
{
    stateMachine_->StopFeedDogForSync(flag);
}

void SingleVerSyncTaskContext::SetSequenceStartAndEndTimeStamp(TimeStamp start, TimeStamp end)
{
    sequenceStartTimeStamp_ = start;
    sequenceEndTimeStamp_ = end;
}

TimeStamp SingleVerSyncTaskContext::GetSequenceStartTimeStamp() const
{
    return sequenceStartTimeStamp_;
}

TimeStamp SingleVerSyncTaskContext::GetSequenceEndTimeStamp() const
{
    return sequenceEndTimeStamp_;
}

void SingleVerSyncTaskContext::SetSessionEndTimeStamp(TimeStamp end)
{
    sessionEndTimeStamp_ = end;
}

TimeStamp SingleVerSyncTaskContext::GetSessionEndTimeStamp() const
{
    return sessionEndTimeStamp_;
}

int SingleVerSyncTaskContext::HandleDataRequestRecv(const Message *msg)
{
    return static_cast<SingleVerSyncStateMachine *>(stateMachine_)->HandleDataRequestRecv(msg);
}

bool SingleVerSyncTaskContext::IsReceiveWaterMarkErr() const
{
    return isReceiveWaterMarkErr_;
}

void SingleVerSyncTaskContext::SetReceiveWaterMarkErr(bool isErr)
{
    isReceiveWaterMarkErr_ = isErr;
}

void SingleVerSyncTaskContext::SetSlidingWindowSenderErr(bool isErr)
{
    static_cast<SingleVerSyncStateMachine *>(stateMachine_)->SetSlidingWindowSenderErr(isErr);
}

void SingleVerSyncTaskContext::SetRemoteSeccurityOption(SecurityOption secOption)
{
    remoteSecOption_ = secOption;
}

SecurityOption SingleVerSyncTaskContext::GetRemoteSeccurityOption() const
{
    return remoteSecOption_;
}

void SingleVerSyncTaskContext::SetReceivcPermitCheck(bool isChecked)
{
    isReceivcPermitChecked_ = isChecked;
}

bool SingleVerSyncTaskContext::GetReceivcPermitCheck() const
{
    return isReceivcPermitChecked_;
}

void SingleVerSyncTaskContext::SetSendPermitCheck(bool isChecked)
{
    isSendPermitChecked_ = isChecked;
}

bool SingleVerSyncTaskContext::GetSendPermitCheck() const
{
    return isSendPermitChecked_;
}

void SingleVerSyncTaskContext::SetSyncStrategy(SyncStrategy strategy)
{
    syncStrategy_.permitSync = strategy.permitSync;
    syncStrategy_.convertOnSend = strategy.convertOnSend;
    syncStrategy_.convertOnReceive = strategy.convertOnReceive;
    syncStrategy_.checkOnReceive = strategy.checkOnReceive;
}

SyncStrategy SingleVerSyncTaskContext::GetSyncStrategy() const
{
    return syncStrategy_;
}

void SingleVerSyncTaskContext::SetIsSchemaSync(bool isSchemaSync)
{
    isSchemaSync_ = isSchemaSync;
}

bool SingleVerSyncTaskContext::GetIsSchemaSync() const
{
    return isSchemaSync_;
}

bool SingleVerSyncTaskContext::IsSkipTimeoutError(int errCode) const
{
    if (errCode == -E_TIMEOUT && IsAutoSync() && (GetRetryTime() < ISyncStateMachine::RETRY_TIME)) {
        LOGE("[SingleVerSyncTaskContext] send message timeout error occurred");
        return true;
    } else {
        return false;
    }
}
DEFINE_OBJECT_TAG_FACILITIES(SingleVerSyncTaskContext)
} // namespace DistributedDB
