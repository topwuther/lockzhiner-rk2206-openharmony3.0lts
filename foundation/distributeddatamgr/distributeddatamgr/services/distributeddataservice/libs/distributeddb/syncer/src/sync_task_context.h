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

#ifndef SYNC_TASK_CONTEXT_H
#define SYNC_TASK_CONTEXT_H

#include <list>
#include <mutex>

#include "isync_task_context.h"
#include "sync_target.h"
#include "semaphore.h"
#include "sync_operation.h"
#include "icommunicator.h"
#include "ikvdb_sync_interface.h"
#include "meta_data.h"
#include "runtime_context.h"
#include "time_helper.h"

namespace DistributedDB {
enum SyncDirectionFlag {
    SEND = 0,
    RECEIVE = 1,
};

class ISyncStateMachine;

class SyncTaskContext : public ISyncTaskContext {
public:
    SyncTaskContext();

    // Add a sync task target to the queue
    int AddSyncTarget(ISyncTarget *target) override;

    // Set the status of this task
    void SetOperationStatus(int status) override;

    // Clear context data
    void Clear() override;

    // remove a sync target by syncId
    int RemoveSyncOperation(int syncId) override;

    // If the requestTargetQueue is empty
    bool IsTargetQueueEmpty() const override;

    // Get the status of this task
    int GetOperationStatus() const override;

    // Set the mode of this task
    void SetMode(int mode) override;

    // Get the mode of this task
    int GetMode() const override;

    // Move to next target to sync
    void MoveToNextTarget() override;

    // Get the current task syncId
    uint32_t GetSyncId() const override;

    // Get the current task deviceId.
    std::string GetDeviceId() const override;

    // Set the sync task queue exec status
    void SetTaskExecStatus(int status) override;

    // Get the sync task queue exec status
    int GetTaskExecStatus() const override;

    // Return if now is doing auto sync
    bool IsAutoSync() const override;

    // Set a Timer used for timeout
    int StartTimer() override;

    // delete timer
    void StopTimer() override;

    // modify timer
    int ModifyTimer(int milliSeconds) override;

    // Set a RetryTime for the sync task
    void SetRetryTime(int retryTime) override;

    // Get a RetryTime for the sync task
    int GetRetryTime() const override;

    // Set Retry status for the sync task
    void SetRetryStatus(int isNeedRetry) override;

    // Get Retry status for the sync task
    int GetRetryStatus() const override;

    TimerId GetTimerId() const override;

    // Inc the current message sequenceId
    void IncSequenceId() override;

    // Get the current initiactive sync session id
    uint32_t GetRequestSessionId() const override;

    // Get the current message sequence id
    uint32_t GetSequenceId() const override;

    void ReSetSequenceId() override;

    void IncPacketId();

    uint64_t GetPacketId() const;

    // Get the current watch timeout time
    int GetTimeoutTime() const override;

    void SetTimeoutCallback(const TimerAction &timeOutCallback) override;

    // Start the sync state machine
    int StartStateMachine() override;

    // Set the timeoffset with the remote device
    void SetTimeOffset(TimeOffset offset) override;

    // Get the timeoffset with the remote device
    TimeOffset GetTimeOffset() const override;

    // Used for sync message callback
    int ReceiveMessageCallback(Message *inMsg) override;

    // used to register a callback, called when new SyncTarget added
    void RegOnSyncTask(const std::function<int(void)> &callback) override;

    // When schedule a new task, should call this function to inc usedcount
    int IncUsedCount() override;

    // When schedule task exit, should call this function to dec usedcount
    void SafeExit() override;

    // Get current local time from TimeHelper
    TimeStamp GetCurrentLocalTime() const override;

    // Set the remount software version num
    void SetRemoteSoftwareVersion(uint32_t version) override;

    // Get the remount software version num
    uint32_t GetRemoteSoftwareVersion() const override;

    // Get the remount software version id, when called GetRemoteSoftwareVersion this id will be increase.
    // Used to check if the version num is is overdue
    uint64_t GetRemoteSoftwareVersionId() const override;

    // Judge if the communicator is normal
    bool IsCommNormal() const override;

    // If ability sync request set version, need call this function.
    // Should be called with ObjLock
    virtual void Abort(int status);

    // Used in send msg, as execution is asynchronous, should use this function to handle result.
    static void CommErrHandlerFunc(int errCode, ISyncTaskContext *context, int32_t sessionId);

    int GetTaskErrCode() const override;

    void SetTaskErrCode(int errCode) override;

protected:
    const static int KILL_WAIT_SECONDS = INT32_MAX;

    ~SyncTaskContext() override;

    virtual int TimeOut(TimerId id);

    virtual void CopyTargetData(const ISyncTarget *target);

    void CommErrHandlerFuncInner(int errCode, uint32_t sessionId);

    void KillWait();

    void ClearSyncOperation();

    void ClearSyncTarget();

    void CancelCurrentSyncRetryIfNeed(int newTargetMode);

    mutable std::mutex targetQueueLock_;
    std::list<ISyncTarget *> requestTargetQueue_;
    std::list<ISyncTarget *> responseTargetQueue_;
    SyncOperation *syncOperation_;
    mutable std::mutex operationLock_;
    uint32_t syncId_;
    int mode_;
    bool isAutoSync_;
    int status_;
    int taskExecStatus_;
    std::string deviceId_;
    IKvDBSyncInterface *syncInterface_;
    ICommunicator *communicator_;
    ISyncStateMachine *stateMachine_;
    TimeOffset timeOffset_ = 0;
    int retryTime_ = 0;
    int isNeedRetry_ = SyncTaskContext::NO_NEED_RETRY;
    uint32_t requestSessionId_ = 0;
    uint32_t sequenceId_ = 1;
    std::function<int(void)> onSyncTaskAdd_;

    // for safe exit
    std::condition_variable safeKill_;
    int usedCount_ = 0;

    // for timeout callback
    std::mutex timerLock_;
    TimerId timerId_ = 0;
    int timeout_ = 1000; // 1000ms
    TimerAction timeOutCallback_;
    std::unique_ptr<TimeHelper> timeHelper_;

    // for version sync
    mutable std::mutex remoteSoftwareVersionLock_;
    uint32_t remoteSoftwareVersion_;
    uint64_t remoteSoftwareVersionId_; // Check if the remoteSoftwareVersion_ is is overdue

    bool isCommNormal_;
    int taskErrCode_;
    uint64_t packetId_ = 0; // used for assignment to reSendMap_.ReSendInfo.packetId in 103 version or above

    // For gloable ISyncTaskContext Set, used by CommErrCallback.
    static std::mutex synTaskContextSetLock_;
    static std::set<ISyncTaskContext *> synTaskContextSet_;
};
} // namespace DistributedDB

#endif // SYNC_TASK_CONTEXT_H
