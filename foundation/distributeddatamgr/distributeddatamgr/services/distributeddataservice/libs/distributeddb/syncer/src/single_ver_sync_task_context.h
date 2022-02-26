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

#ifndef SINGLE_VER_SYNC_TASK_CONTEXT_H
#define SINGLE_VER_SYNC_TASK_CONTEXT_H

#include <list>
#include <mutex>
#include <string>

#include "sync_target.h"
#include "sync_task_context.h"
#include "time_helper.h"
#include "single_ver_sync_target.h"

namespace DistributedDB {
class SingleVerSyncTaskContext final : public SyncTaskContext {
public:

    SingleVerSyncTaskContext();

    DISABLE_COPY_ASSIGN_MOVE(SingleVerSyncTaskContext);

    // Init SingleVerSyncTaskContext
    int Initialize(const std::string &deviceId, IKvDBSyncInterface *syncInterface, std::shared_ptr<Metadata> &metadata,
        ICommunicator *communicator) override;

    // Add a sync task target with the operation to the queue
    int AddSyncOperation(SyncOperation *operation) override;

    // Set the end water mark of this task
    void SetEndMark(WaterMark endMark);

    // Get the end water mark of this task
    WaterMark GetEndMark() const;

    void GetContinueToken(ContinueToken &outToken) const;

    void SetContinueToken(ContinueToken token);

    void ReleaseContinueToken();

    int PopResponseTarget(SingleVerSyncTarget &target);

    int GetRspTargetQueueSize() const;

    // responseSessionId used for mark the pull response task
    void SetResponseSessionId(uint32_t responseSessionId);

    // responseSessionId used for mark the pull response task
    uint32_t GetResponseSessionId() const;

    void Clear() override;

    void Abort(int status) override;

    // If set true, remote stale data will be clear when remote db rebuiled.
    void EnableClearRemoteStaleData(bool enable);

    // Check if need to clear remote device stale data in syncing, when the remote db rebuilt.
    bool IsNeedClearRemoteStaleData() const;

    // start a timer to ResetWatchDog when sync data one (key,value) size bigger than mtu
    bool StartFeedDogForSync(uint32_t time, SyncDirectionFlag flag);

    // stop timer to ResetWatchDog when sync data one (key,value) size bigger than mtu
    void StopFeedDogForSync(SyncDirectionFlag flag);

    // if sended by sliding window, get the start timeStamp of data in a sequence
    TimeStamp GetSequenceStartTimeStamp() const;

    // if sended by sliding window, get the end timeStamp of data in a sequence
    TimeStamp GetSequenceEndTimeStamp() const;

    int HandleDataRequestRecv(const Message *msg);

    // if sended by sliding window, set the start and and timeStamp of data in a sequence
    void SetSequenceStartAndEndTimeStamp(TimeStamp start, TimeStamp end);

    // if sended by sliding window, set the last data timeStamp in a sync session
    void SetSessionEndTimeStamp(TimeStamp end);

    // if sended by sliding window, get the last data timeStamp in a sync session
    TimeStamp GetSessionEndTimeStamp() const;

    // is receive warterMark err
    bool IsReceiveWaterMarkErr() const;

    // set receive warterMark err
    void SetReceiveWaterMarkErr(bool isErr);

    // set sliding window sender err
    void SetSlidingWindowSenderErr(bool isErr);

    void SetRemoteSeccurityOption(SecurityOption secOption);

    SecurityOption GetRemoteSeccurityOption() const;

    void SetReceivcPermitCheck(bool isChecked);

    bool GetReceivcPermitCheck() const;

    void SetSendPermitCheck(bool isChecked);

    bool GetSendPermitCheck() const;

    void SetSyncStrategy(SyncStrategy strategy);

    SyncStrategy GetSyncStrategy() const;

    void SetIsSchemaSync(bool isChecked);

    bool GetIsSchemaSync() const;

    bool IsSkipTimeoutError(int errCode) const;
protected:
    ~SingleVerSyncTaskContext() override;
    void CopyTargetData(const ISyncTarget *target) override;

private:
    constexpr static int64_t REDUNDACE_WATER_MARK = 1 * 1000LL * 1000LL * 10LL; // 1s

    DECLARE_OBJECT_TAG(SingleVerSyncTaskContext);

    ContinueToken token_;
    WaterMark endMark_;
    uint32_t responseSessionId_ = 0;

    bool needClearRemoteStaleData_;
    SecurityOption remoteSecOption_ = {0, 0}; // remote targe can handle secOption data or not.
    bool isReceivcPermitChecked_ = false;
    bool isSendPermitChecked_ = false;
    SyncStrategy syncStrategy_;
    bool isSchemaSync_ = false;

    // in a sync session, if sended by sliding window, the min timeStamp of data in a sequence
    TimeStamp sequenceStartTimeStamp_ = 0;
    // in a sync session, if sended by sliding window, the max timeStamp of data in a sequence
    TimeStamp sequenceEndTimeStamp_ = 0;
    // in a sync session, the last data timeStamp
    TimeStamp sessionEndTimeStamp_ = 0;
    // is receive waterMark err, peerWaterMark bigger than remote localWaterMark
    bool isReceiveWaterMarkErr_ = false;
};
} // namespace DistributedDB

#endif // SYNC_TASK_CONTEXT_H
