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

#ifndef SINGLE_VER_SYNC_STATE_MACHINE_H
#define SINGLE_VER_SYNC_STATE_MACHINE_H

#include <condition_variable>
#include <memory>

#include "sync_state_machine.h"
#include "sync_target.h"
#include "semaphore.h"
#include "message.h"
#include "single_ver_sync_task_context.h"
#include "time_sync.h"
#include "time_helper.h"
#include "single_ver_data_sync.h"
#include "meta_data.h"
#include "ability_sync.h"
#include "single_ver_data_sync_with_sliding_window.h"

namespace DistributedDB {
namespace {
    enum State {
        IDLE = 0,
        TIME_SYNC,
        ABILITY_SYNC,
        START_INITIACTIVE_DATA_SYNC, // used for do sync started by local device
        START_PASSIVE_DATA_SYNC,  // used for do sync response remote device
        INACTIVE_PUSH_REMAINDER_DATA, // push remainded data if initactive sync data more than on frame
        PASSIVE_PUSH_REMAINDER_DATA, // push remainded data if passive sync data more than on frame
        WAIT_FOR_RECEIVE_DATA_FINISH, // all data send finished, wait for data revice if has pull request
        SYNC_TASK_FINISHED, // current sync task finished, try to schedule next sync task
        SYNC_TIME_OUT,
        INNER_ERR,
        START_INITIACTIVE_SLIDING_DATA_SYNC, // used for do sync started by local device, use sliding window
        START_PASSIVE_SLIDING_DATA_SYNC // used for do pull response, use sliding window
    };

    enum Event {
        START_SYNC_EVENT = 1,
        TIME_SYNC_FINISHED_EVENT,
        ABILITY_SYNC_FINISHED_EVENT,
        VERSION_NOT_SUPPORT_EVENT,
        SWITCH_TO_PROCTOL_V1_EVENT,
        SEND_DATA_EVENT,
        SEND_FINISHED_EVENT,
        RECV_FINISHED_EVENT,
        NEED_ABILITY_SYNC_EVENT,
        RESPONSE_PUSH_REMAINDER_EVENT,
        RESPONSE_TASK_FINISHED_EVENT,
        START_PULL_RESPONSE_EVENT,
        WAIT_ACK_EVENT,
        ALL_TASK_FINISHED_EVENT,
        TIME_OUT_EVENT,
        INNER_ERR_EVENT,
        WAIT_TIME_OUT_EVENT,
        RE_SEND_DATA_EVENT,
        ANY_EVENT
    };
}

using stateMappingHandler = std::function<uint8_t(void)>;
class SingleVerSyncStateMachine final : public SyncStateMachine {
public:

    SingleVerSyncStateMachine();
    ~SingleVerSyncStateMachine() override;

    // Init the SingleVerSyncStateMachine
    int Initialize(ISyncTaskContext *context, IKvDBSyncInterface *syncInterface, std::shared_ptr<Metadata> &metadata,
        ICommunicator *communicator) override;

    // send Message to the StateMachine
    int ReceiveMessageCallback(Message *inMsg) override;

    // Called by CommErrHandler, used to abort sync when handle err
    void CommErrAbort() override;

    int HandleDataRequestRecv(const Message *inMsg);

    void SetSlidingWindowSenderErr(bool isErr);

    bool IsNeedErrCodeHandle(uint32_t sessionId) const;

    void PushPullDataRequestEvokeErrHandle(SingleVerSyncTaskContext *context);

protected:
    // Step the SingleVerSyncStateMachine
    void SyncStep() override;

    // SyncOperation is timeout, step to timeout state
    void StepToTimeout() override;

    void SyncStepInnerLocked() override;

    // Do state machine step with no lock, for inner use
    void SyncStepInner() override;

    int StartSyncInner() override;

    void AbortInner() override;

    void SetCurStateErrStatus() override;

    // Used to get instance class' stateSwitchTables
    const std::vector<StateSwitchTable> &GetStateSwitchTables() const override;

    // Do some init for run a next sync task
    int PrepareNextSyncTask() override;

    // Called by StartSaveDataNotifyTimer, used to send a save data notify packet
    void SendSaveDataNotifyPacket(uint32_t sessionId, uint32_t sequenceId) override;

private:
    // Used to init sync state machine switchbables
    static void InitStateSwitchTables();

    // To generate the statemachine switchtable with the given version
    static void InitStateSwitchTable(uint32_t version, const std::vector<std::vector<uint8_t>> &switchTable);

    void InitStateMapping();

    // Do TimeSync, for first sync
    Event DoTimeSync();

    // Do AbilitySync, for first sync
    Event DoAbilitySync();

    // Do data packet sync for initiactive sync operation, if need pull, there need send a pull request.
    Event DoInitiactiveDataSync();

    // Do data packet sync for response pull request.
    Event DoPassiveDataSync();

    // Push remainder data to remote
    Event DoInitiactivePushRemainderData();

    // Push remainder data to remote for response pull request
    Event DoPassivePushRemainderData();

    // Waiting for pull data revice finish, if coming a pull request, should goto START_PASSIVE_DATA_SYNC state
    Event DoWaitForDataRecv() const;

    // Sync task finished, should do some data clear and exec next task.
    Event DoSyncTaskFinished();

    // Do something when sync timeout.
    Event DoTimeout();

    // Do something when sync get some err.
    Event DoInnerErr();

    Event DoInitiactiveDataSyncWithSlidingWindow();

    Event DoPassiveDataSyncWithSlidingWindow();

    int TimeMarkSyncRecv(const Message *inMsg);

    int AbilitySyncRecv(const Message *inMsg);

    int DataPktRecv(Message *inMsg);

    void NeedAbilitySyncHandle();

    int HandleDataAckRecv(const Message *inMsg);

    int PreHandleAckRecv(const Message *inMsg);

    void HandleDataAckRecvWithSlidingWindow(int errCode, const Message *inMsg);

    void Clear();

    bool IsPacketValid(const Message *inMsg) const;

    void PreStartPullResponse();

    bool IsRightDataResponsePkt(const Message *inMsg) const;

    bool CheckIsStartPullResponse() const;

    int MessageCallbackPre(const Message *inMsg);

    void AddPullResponseTarget(WaterMark pullEndWatermark, uint32_t sessionId);

    Event TransformErrCodeToEvent(int errCode);

    bool IsNeedResetWatchdog(const Message *inMsg) const;

    Event TransforTimeOutErrCodeToEvent();

    DISABLE_COPY_ASSIGN_MOVE(SingleVerSyncStateMachine);

    static std::mutex stateSwitchTableLock_;
    static bool isStateSwitchTableInited_;
    static std::vector<StateSwitchTable> stateSwitchTables_;
    SingleVerSyncTaskContext *context_;
    SingleVerKvDBSyncInterface *syncInterface_;
    std::unique_ptr<TimeSync> timeSync_;
    std::unique_ptr<AbilitySync> abilitySync_;
    std::shared_ptr<SingleVerDataSync> dataSync_;
    std::unique_ptr<SingleVerDataSyncWithSlidingWindow> dataSyncWithSlidingWindow_;
    uint64_t currentRemoteVersionId_;
    std::map<uint8_t, stateMappingHandler> stateMapping_;
};
} // namespace DistributedDB

#endif // SINGLE_VER_SYNC_STATE_MACHINE_H
