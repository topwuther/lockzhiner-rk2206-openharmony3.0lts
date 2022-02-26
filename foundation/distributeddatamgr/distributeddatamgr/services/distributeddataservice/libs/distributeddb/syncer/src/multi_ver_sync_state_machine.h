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

#ifndef MULTI_VER_SYNC_STATE_MACHINE_H
#define MULTI_VER_SYNC_STATE_MACHINE_H

#ifndef OMIT_MULTI_VER
#include <memory>

#include "sync_state_machine.h"
#include "multi_ver_sync_task_context.h"
#include "meta_data.h"
#include "time_sync.h"
#include "commit_history_sync.h"
#include "multi_ver_data_sync.h"
#include "value_slice_sync.h"
#include "db_types.h"

namespace DistributedDB {
class MultiVerSyncStateMachine final : public SyncStateMachine {
public:
    struct ResponseInfo {
        uint32_t sessionId = 0;
        TimerId timerId = 0;
    };

    MultiVerSyncStateMachine();
    ~MultiVerSyncStateMachine() override;

    // Init the MultiVerSyncStateMachine
    int Initialize(ISyncTaskContext *context, IKvDBSyncInterface *syncInterface, std::shared_ptr<Metadata> &metadata,
        ICommunicator *communicator) override;

    // send Message to the StateMachine
    int ReceiveMessageCallback(Message *inMsg) override;

    // Called by CommErrHandler, used to abort sync when handle err
    void CommErrAbort() override;

    DISABLE_COPY_ASSIGN_MOVE(MultiVerSyncStateMachine);

protected:
    // Step the MultiVerSyncStateMachine
    void SyncStep() override;

    // SyncOperation is timeout, step to timeout state
    void StepToTimeout() override;

    void SyncStepInnerLocked() override;

    void SyncStepInner() override;

    void AbortInner() override;

    int StartSyncInner() override;

    const std::vector<StateSwitchTable> &GetStateSwitchTables() const override;

    // Do some init for run a next sync task
    int PrepareNextSyncTask() override;

    // Called by StartSaveDataNotifyTimer, used to send a save data notify packet
    void SendSaveDataNotifyPacket(uint32_t sessionId, uint32_t sequenceId) override;

private:
    enum State {
        IDLE,
        TIME_SYNC,
        COMMIT_HISTORY_SYNC,
        MULTI_VER_DATA_ENTRY_SYNC,
        MULTI_VER_VALUE_SLICE_SYNC,
        SYNC_TIME_OUT,
        INNER_ERR
    };

    void StepToIdle();

    int MessageCallbackCheck(const Message *inMsg);

    int CommitHistorySyncStepInner(void);

    int MultiVerDataSyncStepInner(void);

    int ValueSliceSyncStepInner(void);

    int TimeSyncPacketRecvCallback(const MultiVerSyncTaskContext *context, const Message *inMsg);

    int CommitHistorySyncPktRecvCallback(MultiVerSyncTaskContext *context, const Message *inMsg);

    int MultiVerDataPktRecvCallback(MultiVerSyncTaskContext *context, const Message *inMsg);

    int ValueSlicePktRecvCallback(MultiVerSyncTaskContext *context, const Message *inMsg);

    void Finish();

    int OneCommitSyncFinish();

    bool IsPacketValid(const Message *inMsg) const;

    void Clear();

    // Mark sync response is begin now, we should disable real delete
    void SyncResponseBegin(uint32_t sessionId);

    // Mark sync response is finished, we should enable real delete
    void SyncResponseEnd(uint32_t sessionId);

    // Mark sync response may has an err, has not received finish ack, we should enable real delete
    int SyncResponseTimeout(TimerId timerId);

    static const int RESPONSE_TIME_OUT = 30 * 1000; // 30s

    static std::vector<StateSwitchTable> stateSwitchTables_;
    MultiVerSyncTaskContext *context_;
    MultiVerKvDBSyncInterface *multiVerStorage_;
    std::mutex responseInfosLock_;
    std::list<ResponseInfo> responseInfos_;
    std::unique_ptr<TimeSync> timeSync_;
    std::unique_ptr<CommitHistorySync> commitHistorySync_;
    std::unique_ptr<MultiVerDataSync> multiVerDataSync_;
    std::unique_ptr<ValueSliceSync> valueSliceSync_;
};
} // namespace DistributedDB

#endif // MULTI_VER_SYNC_STATE_MACHINE_H
#endif
