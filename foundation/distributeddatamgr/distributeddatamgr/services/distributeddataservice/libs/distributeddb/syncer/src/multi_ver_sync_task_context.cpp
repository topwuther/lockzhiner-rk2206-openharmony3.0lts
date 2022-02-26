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
#include "multi_ver_sync_task_context.h"
#include "multi_ver_sync_state_machine.h"
#include "multi_ver_sync_target.h"
#include "log_print.h"

namespace DistributedDB {
DEFINE_OBJECT_TAG_FACILITIES(MultiVerSyncTaskContext)

MultiVerSyncTaskContext::~MultiVerSyncTaskContext()
{
}

int MultiVerSyncTaskContext::Initialize(const std::string &deviceId, IKvDBSyncInterface *syncInterface,
    std::shared_ptr<Metadata> &metadata, ICommunicator *communicator)
{
    if (deviceId.empty() || (syncInterface == nullptr) || (communicator == nullptr)) {
        return -E_INVALID_ARGS;
    }
    syncInterface_ = syncInterface;
    communicator_ = communicator;
    deviceId_ = deviceId;
    taskExecStatus_ = INIT;
    isAutoSync_ = true;
    timeHelper_ = std::make_unique<TimeHelper>();
    int errCode = timeHelper_->Initialize(syncInterface, metadata);
    if (errCode != E_OK) {
        LOGE("[MultiVerSyncTaskContext] timeHelper Initialize failed, err %d.", errCode);
        return errCode;
    }

    stateMachine_ = new (std::nothrow) MultiVerSyncStateMachine;
    if (stateMachine_ == nullptr) {
        return -E_OUT_OF_MEMORY;
    }

    errCode = stateMachine_->Initialize(this, syncInterface, metadata, communicator);
    TimerAction timeOutCallback = std::bind(&SyncStateMachine::TimeoutCallback,
        static_cast<MultiVerSyncStateMachine *>(stateMachine_),
        std::placeholders::_1);
    SetTimeoutCallback(timeOutCallback);
    OnKill([this]() { this->KillWait(); });
    {
        std::lock_guard<std::mutex> lock(synTaskContextSetLock_);
        synTaskContextSet_.insert(this);
    }
    return errCode;
}

int MultiVerSyncTaskContext::AddSyncOperation(SyncOperation *operation)
{
    if (operation == nullptr) {
        return -E_INVALID_ARGS;
    }

    if (operation->IsAutoSync() && !IsTargetQueueEmpty()) {
        LOGI("[MultiVerSyncTaskContext] Exist operation in queue, skip it!");
        operation->SetStatus(deviceId_, SyncOperation::FINISHED_ALL);
        return E_OK;
    }

    MultiVerSyncTarget *target = new (std::nothrow) MultiVerSyncTarget;
    if (target == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    target->SetSyncOperation(operation);
    target->SetTaskType(ISyncTarget::REQUEST);
    AddSyncTarget(target);
    return E_OK;
}

int MultiVerSyncTaskContext::GetCommitIndex() const
{
    return commitsIndex_;
}

void MultiVerSyncTaskContext::SetCommitIndex(int index)
{
    commitsIndex_ = index;
}

int MultiVerSyncTaskContext::GetEntriesIndex() const
{
    return entriesIndex_;
}

void MultiVerSyncTaskContext::SetEntriesIndex(int index)
{
    entriesIndex_ = index;
}

int  MultiVerSyncTaskContext::GetValueSlicesIndex() const
{
    return valueSlicesIndex_;
}

void MultiVerSyncTaskContext::SetValueSlicesIndex(int index)
{
    valueSlicesIndex_ = index;
}

void MultiVerSyncTaskContext::GetCommits(std::vector<MultiVerCommitNode> &commits)
{
    commits = commits_;
}

void MultiVerSyncTaskContext::SetCommits(const std::vector<MultiVerCommitNode> &commits)
{
    commits_ = commits;
}

void MultiVerSyncTaskContext::GetCommit(int index, MultiVerCommitNode &commit) const
{
    commit = commits_[index];
}

void MultiVerSyncTaskContext::SetCommit(int index, const MultiVerCommitNode &commit)
{
    commits_[index] = commit;
}

void MultiVerSyncTaskContext::SetEntries(const std::vector<MultiVerKvEntry *> &entries)
{
    entries_ = entries;
}

void MultiVerSyncTaskContext::ReleaseEntries(void)
{
    for (auto &item : entries_) {
        if (syncInterface_ != nullptr) {
            static_cast<MultiVerKvDBSyncInterface *>(syncInterface_)->ReleaseKvEntry(item);
        }
        item = nullptr;
    }
    entries_.clear();
    entries_.shrink_to_fit();
}

void MultiVerSyncTaskContext::GetEntries(std::vector<MultiVerKvEntry *> &entries) const
{
    entries = entries_;
}

void MultiVerSyncTaskContext::GetEntry(int index, MultiVerKvEntry *&entry)
{
    entry = entries_[index];
}

void MultiVerSyncTaskContext::SetCommitsSize(int commitsSize)
{
    commitsSize_ = commitsSize;
}

int MultiVerSyncTaskContext::GetCommitsSize() const
{
    return commitsSize_;
}

void MultiVerSyncTaskContext::SetEntriesSize(int entriesSize)
{
    entriesSize_ = entriesSize;
}

int MultiVerSyncTaskContext::GetEntriesSize() const
{
    return entriesSize_;
}

void MultiVerSyncTaskContext::SetValueSlicesSize(int valueSlicesSize)
{
    valueSlicesSize_ = valueSlicesSize;
}

int MultiVerSyncTaskContext::GetValueSlicesSize() const
{
    return valueSlicesSize_;
}

void MultiVerSyncTaskContext::GetValueSliceHashNode(int index, ValueSliceHash &hashNode) const
{
    hashNode = valueSliceHashNodes_[index];
}

void MultiVerSyncTaskContext::SetValueSliceHashNodes(const std::vector<ValueSliceHash> &valueSliceHashNodes)
{
    valueSliceHashNodes_ = valueSliceHashNodes;
}

void MultiVerSyncTaskContext::GetValueSliceHashNodes(std::vector<ValueSliceHash> &valueSliceHashNodes) const
{
    valueSliceHashNodes = valueSliceHashNodes_;
}

void MultiVerSyncTaskContext::Clear()
{
    commits_.clear();
    commits_.shrink_to_fit();
    ReleaseEntries();
    valueSliceHashNodes_.clear();
    valueSliceHashNodes_.shrink_to_fit();
    commitsIndex_ = 0;
    commitsSize_ = 0;
    entriesIndex_ = 0;
    entriesSize_ = 0;
    valueSlicesIndex_ = 0;
    valueSlicesSize_ = 0;
    retryTime_ = 0;
    isNeedRetry_ = NO_NEED_RETRY;
    StopTimer();
    sequenceId_ = 1; // minimum valid ID : 1
    syncId_ = 0;
}

void MultiVerSyncTaskContext::CopyTargetData(const ISyncTarget *target)
{
    SyncTaskContext::CopyTargetData(target);
}
}
#endif