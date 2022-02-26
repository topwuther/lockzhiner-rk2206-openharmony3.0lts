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

#ifndef I_SYNC_TASK_CONTEXT_H
#define I_SYNC_TASK_CONTEXT_H

#include "sync_target.h"
#include "sync_operation.h"
#include "icommunicator.h"
#include "ikvdb_sync_interface.h"
#include "meta_data.h"
#include "time_helper.h"
#include "runtime_context.h"

namespace DistributedDB {
using CommErrHandler = std::function<void(int)>;

class ISyncTaskContext : public virtual RefObject {
public:
    enum RETRY_STATUS { NO_NEED_RETRY, NEED_RETRY };

    enum TASK_EXEC_STATUS { INIT, RUNNING, FAILED, FINISHED };

    // Initialize the context
    virtual int Initialize(const std::string &deviceId, IKvDBSyncInterface *syncInterface,
        std::shared_ptr<Metadata> &metadata, ICommunicator *communicator) = 0;

    // Add a sync task target with the operation to the queue
    virtual int AddSyncOperation(SyncOperation *operation) = 0;

    // Add a sync task target to the queue
    virtual int AddSyncTarget(ISyncTarget *target) = 0;

    // Set the status of this task cotext
    virtual void SetOperationStatus(int status) = 0;

    // Clear the data of this context
    virtual void Clear() = 0;

    // Remove a sync target by syncId
    virtual int RemoveSyncOperation(int syncId) = 0;

    // If the targetQueue is empty
    virtual bool IsTargetQueueEmpty() const = 0;

    // Get the status of this task
    virtual int GetOperationStatus() const = 0;

    // Set the mode of this task
    virtual void SetMode(int mode) = 0;

    // Get the mode of this task
    virtual int GetMode() const = 0;

    // Move to next target to sync
    virtual void MoveToNextTarget() = 0;

    // Get the current task syncId
    virtual uint32_t GetSyncId() const = 0;

    // Get the current task deviceId.
    virtual std::string GetDeviceId() const = 0;

    virtual void SetTaskExecStatus(int status) = 0;

    virtual int GetTaskExecStatus() const = 0;

    virtual bool IsAutoSync() const = 0;

    // Set a Timer used for timeout
    virtual int StartTimer() = 0;

    // delete timer
    virtual void StopTimer() = 0;

    // modify timer
    virtual int ModifyTimer(int milliSeconds) = 0;

    // Set a RetryTime for the sync task
    virtual void SetRetryTime(int retryTime) = 0;

    // Get a RetryTime for the sync task
    virtual int GetRetryTime() const = 0;

    // Set Retry status for the sync task
    virtual void SetRetryStatus(int isNeedRetry) = 0;

    // Get Retry status for the sync task
    virtual int GetRetryStatus() const = 0;

    virtual TimerId GetTimerId() const = 0;

    virtual void IncSequenceId() = 0;

    virtual uint32_t GetSequenceId() const = 0;

    virtual void ReSetSequenceId() = 0;

    virtual uint32_t GetRequestSessionId() const = 0;

    virtual int GetTimeoutTime() const = 0;

    virtual void SetTimeOffset(TimeOffset offset) = 0;

    virtual TimeOffset GetTimeOffset() const = 0;

    virtual void SetTimeoutCallback(const TimerAction &timeOutCallback) = 0;

    virtual int StartStateMachine() = 0;

    virtual int ReceiveMessageCallback(Message *inMsg) = 0;

    virtual void RegOnSyncTask(const std::function<int(void)> &callback) = 0;

    virtual int IncUsedCount() = 0;

    virtual void SafeExit() = 0;

    // Get current localtime from TimeHelper
    virtual TimeStamp GetCurrentLocalTime() const = 0;

    // Set the remount software version num
    virtual void SetRemoteSoftwareVersion(uint32_t version) = 0;

    // Get the remount software version num
    virtual uint32_t GetRemoteSoftwareVersion() const = 0;

    // Get the remount software version id, when called GetRemoteSoftwareVersion this id will be increase.
    // Used to check if the version num is is overdue
    virtual uint64_t GetRemoteSoftwareVersionId() const = 0;

    // Judge if the communicator is normal
    virtual bool IsCommNormal() const = 0;

    // Judge if the sec option check is err
    virtual int GetTaskErrCode() const = 0;

    virtual void SetTaskErrCode(int errCode)  = 0;

protected:
    virtual ~ISyncTaskContext() {};
};
} // namespace DistributedDB

#endif // I_SYNC_TASK_CONTEXT_H
