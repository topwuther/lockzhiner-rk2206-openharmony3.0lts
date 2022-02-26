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

#include "single_ver_sync_state_machine.h"

#include <cmath>
#include <climits>
#include <algorithm>

#include "db_errno.h"
#include "log_print.h"
#include "sync_operation.h"
#include "message_transform.h"
#include "sync_types.h"
#include "runtime_context.h"
#include "performance_analysis.h"
#include "single_ver_sync_target.h"

namespace DistributedDB {
namespace {
    // used for state switch table
    const int CURRENT_STATE_INDEX = 0;
    const int EVENT_INDEX = 1;
    const int OUTPUT_STATE_INDEX = 2;

    // State switch table v1 and v2, has three columns, CurrentState, Event, and OutSate
    const std::vector<std::vector<uint8_t>> STATE_SWITCH_TABLE_V2 = {
        {IDLE, START_SYNC_EVENT, TIME_SYNC},

        // In TIME_SYNC state
        {TIME_SYNC, TIME_SYNC_FINISHED_EVENT, ABILITY_SYNC},
        {TIME_SYNC, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {TIME_SYNC, INNER_ERR_EVENT, INNER_ERR},

        // In ABILITY_SYNC state, compare version num and schema
        {ABILITY_SYNC, VERSION_NOT_SUPPORT_EVENT, INNER_ERR},
        {ABILITY_SYNC, SWITCH_TO_PROCTOL_V1_EVENT, START_INITIACTIVE_DATA_SYNC},
        {ABILITY_SYNC, ABILITY_SYNC_FINISHED_EVENT, START_INITIACTIVE_DATA_SYNC},
        {ABILITY_SYNC, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {ABILITY_SYNC, INNER_ERR_EVENT, INNER_ERR},

        // In START_INITIACTIVE_DATA_SYNC state, send a sync request, and send first packet of data sync
        {START_INITIACTIVE_DATA_SYNC, SEND_DATA_EVENT, INACTIVE_PUSH_REMAINDER_DATA},
        {START_INITIACTIVE_DATA_SYNC, RESPONSE_PUSH_REMAINDER_EVENT, PASSIVE_PUSH_REMAINDER_DATA},
        {START_INITIACTIVE_DATA_SYNC, NEED_ABILITY_SYNC_EVENT, ABILITY_SYNC},
        {START_INITIACTIVE_DATA_SYNC, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {START_INITIACTIVE_DATA_SYNC, INNER_ERR_EVENT, INNER_ERR},
        {START_INITIACTIVE_DATA_SYNC, RE_SEND_DATA_EVENT, START_INITIACTIVE_DATA_SYNC},
        {START_INITIACTIVE_DATA_SYNC, SEND_FINISHED_EVENT, START_PASSIVE_DATA_SYNC},

        // In INACTIVE_PUSH_REMAINDER_DATA state, do initiactive sync, send remainder pcket of data sync
        {INACTIVE_PUSH_REMAINDER_DATA, SEND_DATA_EVENT, INACTIVE_PUSH_REMAINDER_DATA},
        {INACTIVE_PUSH_REMAINDER_DATA, SEND_FINISHED_EVENT, START_PASSIVE_DATA_SYNC},
        {INACTIVE_PUSH_REMAINDER_DATA, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {INACTIVE_PUSH_REMAINDER_DATA, INNER_ERR_EVENT, INNER_ERR},
        {INACTIVE_PUSH_REMAINDER_DATA, RE_SEND_DATA_EVENT, INACTIVE_PUSH_REMAINDER_DATA},
        {INACTIVE_PUSH_REMAINDER_DATA, NEED_ABILITY_SYNC_EVENT, ABILITY_SYNC},

        // In START_PASSIVE_DATA_SYNC state, do response pull request, and send first packet of data sync
        {START_PASSIVE_DATA_SYNC, SEND_DATA_EVENT, PASSIVE_PUSH_REMAINDER_DATA},
        {START_PASSIVE_DATA_SYNC, SEND_FINISHED_EVENT, START_PASSIVE_DATA_SYNC},
        {START_PASSIVE_DATA_SYNC, RESPONSE_TASK_FINISHED_EVENT, WAIT_FOR_RECEIVE_DATA_FINISH},
        {START_PASSIVE_DATA_SYNC, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {START_PASSIVE_DATA_SYNC, INNER_ERR_EVENT, INNER_ERR},
        {START_PASSIVE_DATA_SYNC, RE_SEND_DATA_EVENT, PASSIVE_PUSH_REMAINDER_DATA},
        {START_PASSIVE_DATA_SYNC, NEED_ABILITY_SYNC_EVENT, ABILITY_SYNC},

        // In PASSIVE_PUSH_REMAINDER_DATA state, do passive sync, send remainder pcket of data sync
        {PASSIVE_PUSH_REMAINDER_DATA, SEND_DATA_EVENT, PASSIVE_PUSH_REMAINDER_DATA},
        {PASSIVE_PUSH_REMAINDER_DATA, SEND_FINISHED_EVENT, START_PASSIVE_DATA_SYNC},
        {PASSIVE_PUSH_REMAINDER_DATA, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {PASSIVE_PUSH_REMAINDER_DATA, INNER_ERR_EVENT, INNER_ERR},
        {PASSIVE_PUSH_REMAINDER_DATA, RE_SEND_DATA_EVENT, PASSIVE_PUSH_REMAINDER_DATA},
        {PASSIVE_PUSH_REMAINDER_DATA, NEED_ABILITY_SYNC_EVENT, ABILITY_SYNC},

        // In WAIT_FOR_RECEIVE_DATA_FINISH,
        {WAIT_FOR_RECEIVE_DATA_FINISH, RECV_FINISHED_EVENT, SYNC_TASK_FINISHED},
        {WAIT_FOR_RECEIVE_DATA_FINISH, START_PULL_RESPONSE_EVENT, START_PASSIVE_DATA_SYNC},
        {WAIT_FOR_RECEIVE_DATA_FINISH, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {WAIT_FOR_RECEIVE_DATA_FINISH, INNER_ERR_EVENT, INNER_ERR},
        {WAIT_FOR_RECEIVE_DATA_FINISH, NEED_ABILITY_SYNC_EVENT, ABILITY_SYNC},

        // In SYNC_TASK_FINISHED,
        {SYNC_TASK_FINISHED, ALL_TASK_FINISHED_EVENT, IDLE},
        {SYNC_TASK_FINISHED, START_SYNC_EVENT, TIME_SYNC},

        // SYNC_TIME_OUT and INNE_ERR state, just do some exception resolve
        {SYNC_TIME_OUT, ANY_EVENT, SYNC_TASK_FINISHED},
        {INNER_ERR, ANY_EVENT, SYNC_TASK_FINISHED},
    };

    // State switch table v3, has three columns, CurrentState, Event, and OutSate
    const std::vector<std::vector<uint8_t>> STATE_SWITCH_TABLE_V3 = {
        {IDLE, START_SYNC_EVENT, TIME_SYNC},

        // In TIME_SYNC state
        {TIME_SYNC, TIME_SYNC_FINISHED_EVENT, ABILITY_SYNC},
        {TIME_SYNC, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {TIME_SYNC, INNER_ERR_EVENT, INNER_ERR},

        // In ABILITY_SYNC state, compare version num and schema
        {ABILITY_SYNC, VERSION_NOT_SUPPORT_EVENT, INNER_ERR},
        {ABILITY_SYNC, ABILITY_SYNC_FINISHED_EVENT, START_INITIACTIVE_SLIDING_DATA_SYNC},
        {ABILITY_SYNC, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {ABILITY_SYNC, INNER_ERR_EVENT, INNER_ERR},

        // In START_INITIACTIVE_SLIDING_DATA_SYNC state, send a sync request, and send first packet of data sync
        {START_INITIACTIVE_SLIDING_DATA_SYNC, NEED_ABILITY_SYNC_EVENT, ABILITY_SYNC},
        {START_INITIACTIVE_SLIDING_DATA_SYNC, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {START_INITIACTIVE_SLIDING_DATA_SYNC, INNER_ERR_EVENT, INNER_ERR},
        {START_INITIACTIVE_SLIDING_DATA_SYNC, SEND_FINISHED_EVENT, START_PASSIVE_SLIDING_DATA_SYNC},
        {START_INITIACTIVE_SLIDING_DATA_SYNC, RE_SEND_DATA_EVENT, START_INITIACTIVE_SLIDING_DATA_SYNC},

        // In START_PASSIVE_SLIDING_DATA_SYNC state, do response pull request, and send first packet of data sync
        {START_PASSIVE_SLIDING_DATA_SYNC, SEND_FINISHED_EVENT, START_PASSIVE_SLIDING_DATA_SYNC},
        {START_PASSIVE_SLIDING_DATA_SYNC, RESPONSE_TASK_FINISHED_EVENT, WAIT_FOR_RECEIVE_DATA_FINISH},
        {START_PASSIVE_SLIDING_DATA_SYNC, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {START_PASSIVE_SLIDING_DATA_SYNC, INNER_ERR_EVENT, INNER_ERR},
        {START_PASSIVE_SLIDING_DATA_SYNC, NEED_ABILITY_SYNC_EVENT, ABILITY_SYNC},
        {START_PASSIVE_SLIDING_DATA_SYNC, RE_SEND_DATA_EVENT, START_PASSIVE_SLIDING_DATA_SYNC},

        // In WAIT_FOR_RECEIVE_DATA_FINISH,
        {WAIT_FOR_RECEIVE_DATA_FINISH, RECV_FINISHED_EVENT, SYNC_TASK_FINISHED},
        {WAIT_FOR_RECEIVE_DATA_FINISH, START_PULL_RESPONSE_EVENT, START_PASSIVE_SLIDING_DATA_SYNC},
        {WAIT_FOR_RECEIVE_DATA_FINISH, TIME_OUT_EVENT, SYNC_TIME_OUT},
        {WAIT_FOR_RECEIVE_DATA_FINISH, INNER_ERR_EVENT, INNER_ERR},
        {WAIT_FOR_RECEIVE_DATA_FINISH, NEED_ABILITY_SYNC_EVENT, ABILITY_SYNC},

        // In SYNC_TASK_FINISHED,
        {SYNC_TASK_FINISHED, ALL_TASK_FINISHED_EVENT, IDLE},
        {SYNC_TASK_FINISHED, START_SYNC_EVENT, TIME_SYNC},

        // SYNC_TIME_OUT and INNE_ERR state, just do some exception resolve
        {SYNC_TIME_OUT, ANY_EVENT, SYNC_TASK_FINISHED},
        {INNER_ERR, ANY_EVENT, SYNC_TASK_FINISHED},
    };
}

std::mutex SingleVerSyncStateMachine::stateSwitchTableLock_;
std::vector<StateSwitchTable> SingleVerSyncStateMachine::stateSwitchTables_;
bool SingleVerSyncStateMachine::isStateSwitchTableInited_ = false;

SingleVerSyncStateMachine::SingleVerSyncStateMachine()
    : context_(nullptr),
      syncInterface_(nullptr),
      timeSync_(nullptr),
      abilitySync_(nullptr),
      dataSync_(nullptr),
      currentRemoteVersionId_(0)
{
}

SingleVerSyncStateMachine::~SingleVerSyncStateMachine()
{
    LOGD("~SingleVerSyncStateMachine");
    Clear();
}

int SingleVerSyncStateMachine::Initialize(ISyncTaskContext *context, IKvDBSyncInterface *syncInterface,
    std::shared_ptr<Metadata> &metaData, ICommunicator *communicator)
{
    if ((context == nullptr) || (syncInterface == nullptr) || (metaData == nullptr) || (communicator == nullptr)) {
        return -E_INVALID_ARGS;
    }

    int errCode = SyncStateMachine::Initialize(context, syncInterface, metaData, communicator);
    if (errCode != E_OK) {
        return errCode;
    }

    timeSync_ = std::make_unique<TimeSync>();
    dataSync_ = std::make_shared<SingleVerDataSync>();
    abilitySync_ = std::make_unique<AbilitySync>();
    dataSyncWithSlidingWindow_ = std::make_unique<SingleVerDataSyncWithSlidingWindow>();

    errCode = timeSync_->Initialize(communicator, metaData, syncInterface, context->GetDeviceId());
    if (errCode != E_OK) {
        goto ERROR_OUT;
    }
    errCode = dataSync_->Initialize(syncInterface, communicator, metaData, context->GetDeviceId());
    if (errCode != E_OK) {
        goto ERROR_OUT;
    }
    errCode = abilitySync_->Initialize(communicator, syncInterface, context->GetDeviceId());
    if (errCode != E_OK) {
        goto ERROR_OUT;
    }

    currentState_ = IDLE;
    context_ = static_cast<SingleVerSyncTaskContext *>(context);
    syncInterface_ = static_cast<SingleVerKvDBSyncInterface *>(syncInterface);
    errCode = dataSyncWithSlidingWindow_->ReceiverInit(context_, dataSync_);
    if (errCode != E_OK) {
        goto ERROR_OUT;
    }
    errCode = dataSyncWithSlidingWindow_->SenderInit(context_, dataSync_);
    if (errCode != E_OK) {
        goto ERROR_OUT;
    }
    InitStateSwitchTables();
    InitStateMapping();
    return E_OK;

ERROR_OUT:
    Clear();
    return errCode;
}

void SingleVerSyncStateMachine::SyncStep()
{
    RefObject::IncObjRef(context_);
    RefObject::IncObjRef(communicator_);
    int errCode = RuntimeContext::GetInstance()->ScheduleTask(
        std::bind(&SingleVerSyncStateMachine::SyncStepInnerLocked, this));
    if (errCode != E_OK) {
        LOGE("[StateMachine][SyncStep] Schedule SyncStep failed");
        RefObject::DecObjRef(communicator_);
        RefObject::DecObjRef(context_);
    }
}

int SingleVerSyncStateMachine::ReceiveMessageCallback(Message *inMsg)
{
    int errCode = MessageCallbackPre(inMsg);
    if (errCode != E_OK) {
        return errCode;
    }

    switch (inMsg->GetMessageId()) {
        case TIME_SYNC_MESSAGE:
            errCode = TimeMarkSyncRecv(inMsg);
            break;
        case ABILITY_SYNC_MESSAGE:
            errCode = AbilitySyncRecv(inMsg);
            break;
        case DATA_SYNC_MESSAGE:
            errCode = DataPktRecv(inMsg);
            break;
        default:
            errCode = -E_NOT_SUPPORT;
    }
    return errCode;
}

void SingleVerSyncStateMachine::SyncStepInnerLocked()
{
    if (context_->IncUsedCount() != E_OK) {
        goto SYNC_STEP_OUT;
    }
    {
        std::lock_guard<std::mutex> lock(stateMachineLock_);
        SyncStepInner();
    }
    context_->SafeExit();

SYNC_STEP_OUT:
    RefObject::DecObjRef(communicator_);
    RefObject::DecObjRef(context_);
}

void SingleVerSyncStateMachine::SyncStepInner()
{
    Event event = INNER_ERR_EVENT;
    do {
        auto iter = stateMapping_.find(currentState_);
        if (iter != stateMapping_.end()) {
            event = static_cast<Event>(iter->second());
        } else {
            LOGE("[StateMachine][SyncStepInner] can not find state=%d,label=%s,dev=%s{private}", currentState_,
                dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
            break;
        }
    } while (event != Event::WAIT_ACK_EVENT && SwitchMachineState(event) == E_OK && currentState_ != IDLE);
}

void SingleVerSyncStateMachine::SetCurStateErrStatus()
{
    currentState_ = State::INNER_ERR;
}

int SingleVerSyncStateMachine::StartSyncInner()
{
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_MACHINE_START_TO_PUSH_SEND);
    }
    int errCode = PrepareNextSyncTask();
    if (errCode == E_OK) {
        SwitchStateAndStep(Event::START_SYNC_EVENT);
    }
    return errCode;
}

void SingleVerSyncStateMachine::AbortInner()
{
    LOGE("[StateMachine][AbortInner] error occurred,abort,label=%s,dev=%s{private}", dataSync_->GetLabel().c_str(),
        context_->GetDeviceId().c_str());
    if (context_->GetMode() == SyncOperation::PUSH_AND_PULL || context_->GetMode() == SyncOperation::PULL ||
        context_->IsKilled()) {
        dataSyncWithSlidingWindow_->ReceiverClear();
    }
    dataSyncWithSlidingWindow_->SenderClear();
    ContinueToken token;
    context_->GetContinueToken(token);
    if (token != nullptr) {
        syncInterface_->ReleaseContinueToken(token);
    }
    context_->SetContinueToken(nullptr);
    context_->Clear();
}

const std::vector<StateSwitchTable> &SingleVerSyncStateMachine::GetStateSwitchTables() const
{
    return stateSwitchTables_;
}

int SingleVerSyncStateMachine::PrepareNextSyncTask()
{
    int errCode = StartWatchDog();
    if (errCode != E_OK) {
        LOGE("[StateMachine][PrepareNextSyncTask] WatchDog start failed,err=%d", errCode);
        return errCode;
    }

    if (currentState_ != State::IDLE && currentState_ != State::SYNC_TASK_FINISHED) {
        LOGW("[StateMachine][PrepareNextSyncTask] PreSync may get an err, state=%d,dev=%s{private}",
            currentState_, context_->GetDeviceId().c_str());
        currentState_ = State::IDLE;
    }
    return E_OK;
}

void SingleVerSyncStateMachine::SendSaveDataNotifyPacket(uint32_t sessionId, uint32_t sequenceId)
{
    dataSync_->SendSaveDataNotifyPacket(context_,
        std::min(context_->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT), sessionId, sequenceId);
}

void SingleVerSyncStateMachine::CommErrAbort()
{
    std::lock_guard<std::mutex> lock(stateMachineLock_);
    if (SwitchMachineState(Event::INNER_ERR_EVENT) == E_OK) {
        SyncStep();
    }
}

void SingleVerSyncStateMachine::InitStateSwitchTables()
{
    if (isStateSwitchTableInited_) {
        return;
    }

    std::lock_guard<std::mutex> lock(stateSwitchTableLock_);
    if (isStateSwitchTableInited_) {
        return;
    }

    InitStateSwitchTable(SINGLE_VER_SYNC_PROCTOL_V2, STATE_SWITCH_TABLE_V2);
    InitStateSwitchTable(SINGLE_VER_SYNC_PROCTOL_V3, STATE_SWITCH_TABLE_V3);
    std::sort(stateSwitchTables_.begin(), stateSwitchTables_.end(),
        [](const auto &tableA, const auto &tableB) {
            return tableA.version > tableB.version;
        }); // descending
    isStateSwitchTableInited_ = true;
}

void SingleVerSyncStateMachine::InitStateSwitchTable(uint32_t version,
    const std::vector<std::vector<uint8_t>> &switchTable)
{
    StateSwitchTable table;
    table.version = version;
    for (const auto &stateSwitch : switchTable) {
        if (stateSwitch.size() <= OUTPUT_STATE_INDEX) {
            LOGE("[StateMachine][InitSwitchTable] stateSwitch size err,size=%llu", stateSwitch.size());
            return;
        }
        if (table.switchTable.count(stateSwitch[CURRENT_STATE_INDEX]) == 0) {
            EventToState eventToState; // new EventToState
            eventToState[stateSwitch[EVENT_INDEX]] = stateSwitch[OUTPUT_STATE_INDEX];
            table.switchTable[stateSwitch[CURRENT_STATE_INDEX]] = eventToState;
        } else { // key stateSwitch[CURRENT_STATE_INDEX] already has EventToState
            EventToState &eventToState = table.switchTable[stateSwitch[CURRENT_STATE_INDEX]];
            eventToState[stateSwitch[EVENT_INDEX]] = stateSwitch[OUTPUT_STATE_INDEX];
        }
    }
    stateSwitchTables_.push_back(table);
}

void SingleVerSyncStateMachine::InitStateMapping()
{
    stateMapping_[TIME_SYNC] = std::bind(&SingleVerSyncStateMachine::DoTimeSync, this);
    stateMapping_[ABILITY_SYNC] = std::bind(&SingleVerSyncStateMachine::DoAbilitySync, this);
    stateMapping_[START_INITIACTIVE_DATA_SYNC] = std::bind(&SingleVerSyncStateMachine::DoInitiactiveDataSync,
        this);
    stateMapping_[START_PASSIVE_DATA_SYNC] = std::bind(&SingleVerSyncStateMachine::DoPassiveDataSync, this);
    stateMapping_[INACTIVE_PUSH_REMAINDER_DATA] = std::bind(&SingleVerSyncStateMachine::DoInitiactivePushRemainderData,
        this);
    stateMapping_[PASSIVE_PUSH_REMAINDER_DATA] = std::bind(&SingleVerSyncStateMachine::DoPassivePushRemainderData,
        this);
    stateMapping_[WAIT_FOR_RECEIVE_DATA_FINISH] = std::bind(&SingleVerSyncStateMachine::DoWaitForDataRecv, this);
    stateMapping_[SYNC_TASK_FINISHED] = std::bind(&SingleVerSyncStateMachine::DoSyncTaskFinished, this);
    stateMapping_[SYNC_TIME_OUT] = std::bind(&SingleVerSyncStateMachine::DoTimeout, this);
    stateMapping_[INNER_ERR] = std::bind(&SingleVerSyncStateMachine::DoInnerErr, this);
    stateMapping_[START_INITIACTIVE_SLIDING_DATA_SYNC] =
        std::bind(&SingleVerSyncStateMachine::DoInitiactiveDataSyncWithSlidingWindow, this);
    stateMapping_[START_PASSIVE_SLIDING_DATA_SYNC] =
        std::bind(&SingleVerSyncStateMachine::DoPassiveDataSyncWithSlidingWindow, this);
}

Event SingleVerSyncStateMachine::DoInitiactiveDataSync()
{
    int errCode = E_OK;
    switch (context_->GetMode()) {
        case SyncOperation::PUSH:
            context_->SetOperationStatus(SyncOperation::RECV_FINISHED);
            errCode = dataSync_->PushStart(context_);
            break;
        case SyncOperation::PULL:
            context_->SetOperationStatus(SyncOperation::SEND_FINISHED);
            errCode = dataSync_->PullRequestStart(context_);
            break;
        case SyncOperation::PUSH_AND_PULL:
            errCode = dataSync_->PushPullStart(context_);
            break;
        case SyncOperation::RESPONSE_PULL:
            // In response pull mode, reminader data should send in
            // PASSIVE_PUSH_REMAINDER_DATA
            return Event::RESPONSE_PUSH_REMAINDER_EVENT;
        default:
            errCode = -E_NOT_SUPPORT;
            break;
    }
    if (errCode == E_OK) {
        return Event::WAIT_ACK_EVENT;
    }
    return TransformErrCodeToEvent(errCode);
}

Event SingleVerSyncStateMachine::DoInitiactiveDataSyncWithSlidingWindow()
{
    LOGD("[StateMachine][activeDataSync] mode=%d,label=%s,dev=%s{private}", context_->GetMode(),
        dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
    int errCode = E_OK;
    switch (context_->GetMode()) {
        case SyncOperation::PUSH:
            context_->SetOperationStatus(SyncOperation::RECV_FINISHED);
            errCode = dataSyncWithSlidingWindow_->SenderStart(SyncOperation::PUSH, context_, dataSync_);
            break;
        case SyncOperation::PULL:
            context_->SetOperationStatus(SyncOperation::SEND_FINISHED);
            errCode = dataSyncWithSlidingWindow_->SenderStart(SyncOperation::PULL, context_, dataSync_);
            break;
        case SyncOperation::PUSH_AND_PULL:
            errCode = dataSyncWithSlidingWindow_->SenderStart(SyncOperation::PUSH_AND_PULL, context_, dataSync_);
            break;
        case SyncOperation::RESPONSE_PULL:
            errCode = dataSyncWithSlidingWindow_->SenderStart(SyncOperation::RESPONSE_PULL, context_, dataSync_);
            break;
        default:
            errCode = -E_NOT_SUPPORT;
            break;
    }
    if (errCode == E_OK) {
        return Event::WAIT_ACK_EVENT;
    }
    // once E_EKEYREVOKED error occurred, PUSH_AND_PULL mode should wait for ack to pull remote data.
    if (context_->GetMode() == SyncOperation::PUSH_AND_PULL && errCode == -E_EKEYREVOKED) {
        return Event::WAIT_ACK_EVENT;
    }
    return TransformErrCodeToEvent(errCode);
}

Event SingleVerSyncStateMachine::DoPassiveDataSync()
{
    {
        RefObject::AutoLock lock(context_);
        if (context_->GetRspTargetQueueSize() != 0) {
            PreStartPullResponse();
        } else {
            return RESPONSE_TASK_FINISHED_EVENT;
        }
    }
    int errCode = dataSync_->PullResponseStart(context_);
    if (errCode == E_OK) {
        return Event::WAIT_ACK_EVENT;
    }
    return TransformErrCodeToEvent(errCode);
}

Event SingleVerSyncStateMachine::DoPassiveDataSyncWithSlidingWindow()
{
    {
        RefObject::AutoLock lock(context_);
        if (context_->GetRspTargetQueueSize() != 0) {
            PreStartPullResponse();
        } else {
            return RESPONSE_TASK_FINISHED_EVENT;
        }
    }
    int errCode = dataSyncWithSlidingWindow_->SenderStart(SyncOperation::RESPONSE_PULL, context_, dataSync_);
    if (errCode == E_OK) {
        return Event::WAIT_ACK_EVENT;
    }
    return TransformErrCodeToEvent(errCode);
}

Event SingleVerSyncStateMachine::DoInitiactivePushRemainderData()
{
    int errCode;
    switch (context_->GetMode()) {
        case SyncOperation::PULL:
        case SyncOperation::RESPONSE_PULL:
            // In pull or response pul mode, don't need to do INACTIVE_PUSH
            return Event::SEND_FINISHED_EVENT;
        case SyncOperation::PUSH:
        case SyncOperation::PUSH_AND_PULL:
            errCode = dataSync_->PushStart(context_);
            break;
        default:
            errCode = -E_INTERNAL_ERROR;
            break;
    }

    if (errCode == E_OK) {
        return Event::WAIT_ACK_EVENT;
    }
    return TransformErrCodeToEvent(errCode);
}

Event SingleVerSyncStateMachine::DoPassivePushRemainderData()
{
    int errCode = dataSync_->PullResponseStart(context_);
    if (errCode == E_OK) {
        return Event::WAIT_ACK_EVENT;
    }
    return TransformErrCodeToEvent(errCode);
}

Event SingleVerSyncStateMachine::DoWaitForDataRecv() const
{
    if (context_->GetRspTargetQueueSize() != 0) {
        return START_PULL_RESPONSE_EVENT;
    }
    if (context_->GetOperationStatus() == SyncOperation::FINISHED_ALL) {
        return RECV_FINISHED_EVENT;
    }
    if (context_->GetMode() == SyncOperation::PUSH_AND_PULL &&
        context_->GetOperationStatus() == SyncOperation::EKEYREVOKED_FAILURE &&
        context_->GetRemoteSoftwareVersion() > SOFTWARE_VERSION_RELEASE_2_0) {
        return RECV_FINISHED_EVENT;
    }
    return Event::WAIT_ACK_EVENT;
}

Event SingleVerSyncStateMachine::DoTimeSync()
{
    if (timeSync_->IsNeedSync()) {
        CommErrHandler handler = nullptr;
        // Auto sync need do retry don't use errHandler to return.
        if (!context_->IsAutoSync()) {
            handler = std::bind(&SyncTaskContext::CommErrHandlerFunc, std::placeholders::_1,
                context_, context_->GetRequestSessionId());
        }
        int errCode = timeSync_->SyncStart(handler);
        if (errCode == E_OK) {
            return Event::WAIT_ACK_EVENT;
        }
        return TransformErrCodeToEvent(errCode);
    }

    return Event::TIME_SYNC_FINISHED_EVENT;
}

Event SingleVerSyncStateMachine::DoAbilitySync()
{
    uint16_t remoteCommunicatorVersion = 0;
    int errCode = communicator_->GetRemoteCommunicatorVersion(context_->GetDeviceId(), remoteCommunicatorVersion);
    if (errCode != E_OK) {
        LOGE("[StateMachine][DoAbilitySync] Get RemoteCommunicatorVersion errCode=%d", errCode);
        return Event::INNER_ERR_EVENT;
    }
    // Fistr version, not support AbilitySync
    if (remoteCommunicatorVersion == 0) {
        context_->SetRemoteSoftwareVersion(SOFTWARE_VERSION_EARLIEST);
        LOGI("remote version is 0, switch to v1 proctol");
        return Event::SWITCH_TO_PROCTOL_V1_EVENT;
    }

    if (abilitySync_->GetAbilitySyncFinishedStatus()) {
        return Event::ABILITY_SYNC_FINISHED_EVENT;
    }

    CommErrHandler handler = std::bind(&SyncTaskContext::CommErrHandlerFunc, std::placeholders::_1,
        context_, context_->GetRequestSessionId());
    LOGI("[StateMachine][AbilitySync] start abilitySync,label=%s,dev=%s{private}", dataSync_->GetLabel().c_str(),
        context_->GetDeviceId().c_str());
    errCode = abilitySync_->SyncStart(context_->GetRequestSessionId(), context_->GetSequenceId(),
        remoteCommunicatorVersion, handler);
    if (errCode != E_OK) {
        LOGE("[StateMachine][DoAbilitySync] ability sync start failed,errCode=%d", errCode);
        return TransformErrCodeToEvent(errCode);
    }
    return Event::WAIT_ACK_EVENT;
}

Event SingleVerSyncStateMachine::DoSyncTaskFinished()
{
    StopWatchDog();
    dataSyncWithSlidingWindow_->SenderClear();
    RefObject::AutoLock lock(syncContext_);
    int errCode = ExecNextTask();
    if (errCode == E_OK) {
        return Event::START_SYNC_EVENT;
    }
    return TransformErrCodeToEvent(errCode);
}

Event SingleVerSyncStateMachine::DoTimeout()
{
    RefObject::AutoLock lock(context_);
    context_->Abort(SyncOperation::TIMEOUT);
    context_->Clear();
    AbortInner();
    return Event::ANY_EVENT;
}

Event SingleVerSyncStateMachine::DoInnerErr()
{
    RefObject::AutoLock lock(context_);
    if (!context_->IsCommNormal()) {
        context_->Abort(SyncOperation::COMM_ABNORMAL);
    } else if (context_->GetTaskErrCode() == -E_SCHEMA_MISMATCH) {
        context_->Abort(SyncOperation::SCHEMA_INCOMPATIBLE);
    } else if (context_->GetTaskErrCode() == -E_EKEYREVOKED) {
        context_->Abort(SyncOperation::EKEYREVOKED_FAILURE);
    } else if (context_->GetTaskErrCode() == -E_SECURITY_OPTION_CHECK_ERROR) {
        context_->Abort(SyncOperation::SECURITY_OPTION_CHECK_FAILURE);
    } else if (context_->GetTaskErrCode() == -E_BUSY) {
        context_->Abort(SyncOperation::BUSY_FAILURE);
    } else {
        context_->Abort(SyncOperation::FAILED);
    }
    context_->Clear();
    AbortInner();
    return Event::ANY_EVENT;
}

int SingleVerSyncStateMachine::AbilitySyncRecv(const Message *inMsg)
{
    if (inMsg->GetMessageType() == TYPE_REQUEST) {
        return abilitySync_->RequestRecv(inMsg, context_);
    }

    if (inMsg->GetMessageType() == TYPE_RESPONSE) {
        int errCode = abilitySync_->AckRecv(inMsg, context_);
        std::lock_guard<std::mutex> lock(stateMachineLock_);
        (void)ResetWatchDog();
        if (errCode != E_OK && errCode != E_FEEDBACK_UNKNOWN_MESSAGE) {
            LOGE("[StateMachine][AbilitySyncRecv] handle ackRecv failed,errCode=%d", errCode);
            SwitchStateAndStep(TransformErrCodeToEvent(errCode));
        } else if (context_->GetRemoteSoftwareVersion() <= SOFTWARE_VERSION_RELEASE_2_0) {
            abilitySync_->SetAbilitySyncFinishedStatus(true);
            LOGI("[StateMachine][AbilitySyncRecv] ability Sync Finished,label=%s,dev=%s{private}",
                dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
            currentRemoteVersionId_ = context_->GetRemoteSoftwareVersionId();
            SwitchStateAndStep(ABILITY_SYNC_FINISHED_EVENT);
        }
        return E_OK;
    }
    if (inMsg->GetMessageType() == TYPE_NOTIFY) {
            const AbilitySyncAckPacket *packet = inMsg->GetObject<AbilitySyncAckPacket>();
            if (packet == nullptr) {
                return -E_INVALID_ARGS;
            }
            int ackCode = packet->GetAckCode();
            if (ackCode != AbilitySync::CHECK_SUCCESS && ackCode != AbilitySync::LAST_NOTIFY) {
                LOGE("[StateMachine][AbilitySyncRecv] ackCode check failed,ackCode=%d", ackCode);
                std::lock_guard<std::mutex> lock(stateMachineLock_);
                SwitchStateAndStep(Event::INNER_ERR_EVENT);
                return E_OK;
            }
            if (ackCode == AbilitySync::LAST_NOTIFY) {
                abilitySync_->SetAbilitySyncFinishedStatus(true);
                LOGI("[StateMachine][AbilitySyncRecv] ability sync finished,label=%s,dev=%s{private}",
                    dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
                currentRemoteVersionId_ = context_->GetRemoteSoftwareVersionId();
                (static_cast<SingleVerSyncTaskContext *>(context_))->SetIsSchemaSync(true);
                std::lock_guard<std::mutex> lock(stateMachineLock_);
                SwitchStateAndStep(ABILITY_SYNC_FINISHED_EVENT);
            } else {
                abilitySync_->AckNotifyRecv(inMsg, context_);
            }
        return E_OK;
    }

    LOGE("[StateMachine][AbilitySyncRecv] msg type invalid");
    return -E_NOT_SUPPORT;
}

int SingleVerSyncStateMachine::HandleDataRequestRecv(const Message *inMsg)
{
    StopFeedDogForSync(SyncDirectionFlag::RECEIVE);
    {
        std::lock_guard<std::mutex> lockWatchDog(stateMachineLock_);
        if (IsNeedResetWatchdog(inMsg)) {
            (void)ResetWatchDog();
        }
    }

    // RequestRecv will save data, it may cost a long time.
    // So we need to send save data notify to keep remote alive.
    bool isNeedStop = StartSaveDataNotify(inMsg->GetSessionId(), inMsg->GetSequenceId());
    WaterMark pullEndWaterkark = 0;
    int errCode = dataSync_->RequestRecv(context_, inMsg, pullEndWaterkark);
    if (isNeedStop) {
        StopSaveDataNotify();
    }
    // only higher than 102 version receive this errCode here.
    // while both RequestSessionId is not equal,but get this errCode;slwr would seem to handle first secquencid.
    // so while receive the same secquencid after abiitysync it wouldn't handle.
    if (errCode == -E_NEED_ABILITY_SYNC) {
        return errCode;
    }
    std::lock_guard<std::mutex> lock(stateMachineLock_);
    if (IsNeedErrCodeHandle(inMsg->GetSessionId())) {
        switch (errCode) {
            case E_OK:
                break;
            case -E_RECV_FINISHED:
                context_->SetOperationStatus(SyncOperation::RECV_FINISHED);
                SwitchStateAndStep(Event::RECV_FINISHED_EVENT);
                break;
            case -E_EKEYREVOKED:
                PushPullDataRequestEvokeErrHandle(context_);
                break;
            case -E_BUSY:
                context_->SetTaskErrCode(-E_BUSY);
                SwitchStateAndStep(Event::INNER_ERR_EVENT);
                break;
            case -E_SECURITY_OPTION_CHECK_ERROR:
                context_->SetTaskErrCode(-E_SECURITY_OPTION_CHECK_ERROR);
                SwitchStateAndStep(Event::INNER_ERR_EVENT);
                break;
            case -E_NEED_ABILITY_SYNC:
                return errCode;
            default:
                SwitchStateAndStep(Event::INNER_ERR_EVENT);
                break;
        }
    }
    if (pullEndWaterkark > 0) {
        AddPullResponseTarget(pullEndWaterkark, inMsg->GetSessionId());
    }
    return E_OK;
}

int SingleVerSyncStateMachine::PreHandleAckRecv(const Message *inMsg)
{
    if (context_->GetRemoteSoftwareVersion() > SOFTWARE_VERSION_RELEASE_2_0) {
        return dataSyncWithSlidingWindow_->PreHandleSenderAckRecv(inMsg);
    }
    return E_OK;
}

void SingleVerSyncStateMachine::HandleDataAckRecvWithSlidingWindow(int errCode, const Message *inMsg)
{
    if (errCode == -E_RE_SEND_DATA) { // LOCAL_WATER_MARK_NOT_INIT
        dataSyncWithSlidingWindow_->SenderClear();
    }
    if (errCode == -E_NO_DATA_SEND || errCode == -E_SEND_DATA) {
        int ret = dataSyncWithSlidingWindow_->SenderAckRecv(inMsg);
        if (ret == -E_FINISHED) {
            SwitchStateAndStep(Event::SEND_FINISHED_EVENT);
        } else if (ret != E_OK) {
            SwitchStateAndStep(TransformErrCodeToEvent(ret));
        }
    } else {
        SwitchStateAndStep(TransformErrCodeToEvent(errCode));
    }
}

void SingleVerSyncStateMachine::NeedAbilitySyncHandle()
{
    // if the remote device version num is overdue,
    // mean the version num has been reset when syncing data,
    // there should not clear the new version cache again.
    if (currentRemoteVersionId_ == context_->GetRemoteSoftwareVersionId()) {
        LOGI("[StateMachine] set remote version 0, currentRemoteVersionId_ = %llu", currentRemoteVersionId_);
        context_->SetRemoteSoftwareVersion(0);
    } else {
        currentRemoteVersionId_ = context_->GetRemoteSoftwareVersionId();
    }
    abilitySync_->SetAbilitySyncFinishedStatus(false);
    dataSyncWithSlidingWindow_->SenderClear();
}

int SingleVerSyncStateMachine::HandleDataAckRecv(const Message *inMsg)
{
    StopFeedDogForSync(SyncDirectionFlag::SEND);
    std::lock_guard<std::mutex> lock(stateMachineLock_);
    if (IsNeedResetWatchdog(inMsg)) {
        (void)ResetWatchDog();
    }
    int errCode = PreHandleAckRecv(inMsg);
    if (errCode != E_OK) {
        // packetId not match but sequence id matched scene, means resend map has be rebuilt
        // this is old ack, shoulb be dropped and wait for the same packetId sequence.
        if (errCode == -E_SLIDING_WINDOW_RECEIVER_INVALID_MSG) {
            return E_OK;
        }
        // this means error happened,should stop the sync task.
        SwitchStateAndStep(TransformErrCodeToEvent(errCode));
        return errCode;
    }
    // AckRecv will save meta data, it may cost a long time. if another thread is saving data
    // So we need to send save data notify to keep remote alive.
    // eg. remote do pull sync
    bool isNeedStop = StartSaveDataNotify(inMsg->GetSessionId(), inMsg->GetSequenceId());
    errCode = dataSync_->AckRecv(context_, inMsg);
    if (isNeedStop) {
        StopSaveDataNotify();
    }

    switch (errCode) {
        case -E_NEED_ABILITY_SYNC:
            NeedAbilitySyncHandle();
            break;
        case -E_NO_DATA_SEND:
            // when version is higher than 102, this step will be handle in slide windows function.
            if (context_->GetRemoteSoftwareVersion() < SOFTWARE_VERSION_RELEASE_3_0) {
                context_->SetOperationStatus(SyncOperation::SEND_FINISHED);
            }
            break;
        case -E_NOT_PERMIT:
            context_->SetOperationStatus(SyncOperation::PERMISSION_CHECK_FAILED);
            break;
        case -E_EKEYREVOKED:
        case -E_SECURITY_OPTION_CHECK_ERROR:
        case -E_BUSY:
            context_->SetTaskErrCode(errCode);
            break;
        case -E_SAVE_DATA_NOTIFY:
            return errCode;
        default:
            break;
    }
    if (context_->GetRemoteSoftwareVersion() < SOFTWARE_VERSION_RELEASE_3_0) {
        SwitchStateAndStep(TransformErrCodeToEvent(errCode));
        return errCode;
    }
    HandleDataAckRecvWithSlidingWindow(errCode, inMsg);
    return errCode;
}

int SingleVerSyncStateMachine::DataPktRecv(Message *inMsg)
{
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    int errCode;
    TimeOffset offset = 0;

    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            // If message is data sync request, we should check timeoffset.
            errCode = timeSync_->GetTimeOffset(offset);
            if (errCode != E_OK) {
                LOGE("[StateMachine][DataPktRecv] GetTimeOffset err! errCode=%d", errCode);
                return errCode;
            }
            context_->SetTimeOffset(offset);
            if (performance != nullptr) {
                performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_DATA_REQUEST_RECV_TO_SEND_ACK);
            }
            if (context_->GetRemoteSoftwareVersion() < SOFTWARE_VERSION_RELEASE_3_0) {
                // higher than 102 version may go to here, but it is ok not use slwr,after abilitysync would go slwr.
                errCode = HandleDataRequestRecv(inMsg);
            } else {
                errCode = dataSyncWithSlidingWindow_->Receive(inMsg);
            }
            break;
        case TYPE_RESPONSE:
        case TYPE_NOTIFY:
            if (performance != nullptr) {
                performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_DATA_SEND_REQUEST_TO_ACK_RECV);
                performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_ACK_RECV_TO_USER_CALL_BACK);
            }
            errCode = HandleDataAckRecv(inMsg);
            break;
        default:
            errCode = -E_INVALID_ARGS;
            break;
    }
    return errCode;
}

void SingleVerSyncStateMachine::StepToTimeout()
{
    std::lock_guard<std::mutex> lock(stateMachineLock_);
    SwitchStateAndStep(Event::TIME_OUT_EVENT);
}

int SingleVerSyncStateMachine::TimeMarkSyncRecv(const Message *inMsg)
{
    LOGD("[StateMachine][TimeMarkSyncRecv] type=%d,label=%s,dev=%s{private}", inMsg->GetMessageType(),
        dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
    {
        std::lock_guard<std::mutex> lock(stateMachineLock_);
        (void)ResetWatchDog();
    }
    if (inMsg->GetMessageType() == TYPE_REQUEST) {
        return timeSync_->RequestRecv(inMsg);
    } else if (inMsg->GetMessageType() == TYPE_RESPONSE) {
        int errCode = timeSync_->AckRecv(inMsg);
        if (errCode != E_OK) {
            LOGE("[StateMachine][TimeMarkSyncRecv] AckRecv failed errCode=%d", errCode);
            return errCode;
        }
        std::lock_guard<std::mutex> lock(stateMachineLock_);
        SwitchStateAndStep(TIME_SYNC_FINISHED_EVENT);
        return E_OK;
    } else {
        return -E_INVALID_ARGS;
    }
}

void SingleVerSyncStateMachine::Clear()
{
    dataSyncWithSlidingWindow_ = nullptr;
    dataSync_ = nullptr;
    timeSync_ = nullptr;
    abilitySync_ = nullptr;
    context_ = nullptr;
    syncInterface_ = nullptr;
}

bool SingleVerSyncStateMachine::IsPacketValid(const Message *inMsg) const
{
    if (inMsg == nullptr) {
        return false;
    }

    if ((inMsg->GetMessageId() < TIME_SYNC_MESSAGE) || (inMsg->GetMessageId() > ABILITY_SYNC_MESSAGE)) {
        LOGE("[StateMachine][IsPacketValid] Message is invalid, id=%d", inMsg->GetMessageId());
        return false;
    }

    if (inMsg->GetMessageId() == TIME_SYNC_MESSAGE || inMsg->GetMessageId() == ABILITY_SYNC_MESSAGE) {
        return true;
    }
    if (context_->GetRemoteSoftwareVersion() > SOFTWARE_VERSION_RELEASE_2_0) {
        return true;
    }

    if (inMsg->GetMessageType() == TYPE_RESPONSE) {
        if ((inMsg->GetSequenceId() != context_->GetSequenceId()) ||
            ((inMsg->GetSessionId() != context_->GetRequestSessionId()) &&
            (inMsg->GetSessionId() != context_->GetResponseSessionId()))) {
            LOGE("[StateMachine][IsPacketValid] Message is invalid,inMsg SequenceId=%d,seqId=%d,syncId=%d",
                inMsg->GetSequenceId(), context_->GetSequenceId(), context_->GetSyncId());
            return false;
        }
    }

    return true;
}

void SingleVerSyncStateMachine::PreStartPullResponse()
{
    SingleVerSyncTarget target;
    context_->PopResponseTarget(target);
    context_->SetEndMark(target.GetEndWaterMark());
    context_->SetResponseSessionId(target.GetResponseSessionId());
    context_->SetMode(SyncOperation::RESPONSE_PULL);
    context_->ReSetSequenceId();
}

bool SingleVerSyncStateMachine::IsRightDataResponsePkt(const Message *inMsg) const
{
    if (inMsg->GetMessageId() != DATA_SYNC_MESSAGE) {
        return false;
    }

    if (context_->GetRemoteSoftwareVersion() > SOFTWARE_VERSION_RELEASE_2_0) {
        return false;
    }

    if ((context_->GetMode() != SyncOperation::INVALID) && (inMsg->GetMessageType() == TYPE_RESPONSE)) {
        return true;
    }

    return false;
}

bool SingleVerSyncStateMachine::CheckIsStartPullResponse() const
{
    // Other state will step to do pull response, only this statem we need to step the statemachine
    if (currentState_ == WAIT_FOR_RECEIVE_DATA_FINISH) {
        return true;
    }
    return false;
}

int SingleVerSyncStateMachine::MessageCallbackPre(const Message *inMsg)
{
    RefObject::AutoLock lock(context_);
    if (context_->IsKilled()) {
        return -E_OBJ_IS_KILLED;
    }

    if (!IsPacketValid(inMsg)) {
        return -E_INVALID_ARGS;
    }

    if (IsRightDataResponsePkt(inMsg)) {
        context_->IncSequenceId();
    }
    return E_OK;
}

void SingleVerSyncStateMachine::AddPullResponseTarget(WaterMark pullEndWatermark, uint32_t sessionId)
{
    if (pullEndWatermark == 0) {
        LOGE("[StateMachine][AddPullResponseTarget] pullEndWatermark is 0!");
        return;
    }
    SingleVerSyncTarget *targetTmp = new (std::nothrow) SingleVerSyncTarget;
    if (targetTmp == nullptr) {
        LOGE("[StateMachine][AddPullResponseTarget] add failed, may oom");
        return;
    }
    targetTmp->SetTaskType(ISyncTarget::RESPONSE);
    targetTmp->SetMode(SyncOperation::RESPONSE_PULL);
    targetTmp->SetEndWaterMark(pullEndWatermark);
    targetTmp->SetResponseSessionId(sessionId);
    if (context_->AddSyncTarget(targetTmp) != E_OK) {
        delete targetTmp;
        return;
    }
    if (CheckIsStartPullResponse()) {
        SwitchStateAndStep(TransformErrCodeToEvent(-E_NEED_PULL_REPONSE));
    }
}

Event SingleVerSyncStateMachine::TransformErrCodeToEvent(int errCode)
{
    switch (errCode) {
        case -E_TIMEOUT:
            return TransforTimeOutErrCodeToEvent();
        case -VERSION_NOT_SUPPORT_EVENT:
            return Event::VERSION_NOT_SUPPORT_EVENT;
        case -E_SEND_DATA:
            return Event::SEND_DATA_EVENT;
        case -E_NO_DATA_SEND:
            return Event::SEND_FINISHED_EVENT;
        case -E_RECV_FINISHED:
            return Event::RECV_FINISHED_EVENT;
        case -E_NEED_ABILITY_SYNC:
            return Event::NEED_ABILITY_SYNC_EVENT;
        case -E_NO_SYNC_TASK:
            return Event::ALL_TASK_FINISHED_EVENT;
        case -E_NEED_PULL_REPONSE:
            return Event::START_PULL_RESPONSE_EVENT;
        case -E_RE_SEND_DATA:
            return Event::RE_SEND_DATA_EVENT;
        default:
            return Event::INNER_ERR_EVENT;
    }
}

bool SingleVerSyncStateMachine::IsNeedResetWatchdog(const Message *inMsg) const
{
    if (inMsg == nullptr) {
        return false;
    }

    if (IsNeedErrCodeHandle(inMsg->GetSessionId())) {
        return true;
    }

    int msgType = inMsg->GetMessageType();
    if (msgType == TYPE_RESPONSE || msgType == TYPE_NOTIFY) {
        if (inMsg->GetSessionId() == context_->GetResponseSessionId()) {
            // Pull response ack also should reset watchdog
            return true;
        }
    }

    return false;
}

Event SingleVerSyncStateMachine::TransforTimeOutErrCodeToEvent()
{
    if (syncContext_->IsAutoSync() && (syncContext_->GetRetryTime() < RETRY_TIME)) {
        return Event::WAIT_TIME_OUT_EVENT;
    }
    return Event::TIME_OUT_EVENT;
}

void SingleVerSyncStateMachine::SetSlidingWindowSenderErr(bool isErr)
{
    dataSyncWithSlidingWindow_->SetSenderErr(isErr);
}

bool SingleVerSyncStateMachine::IsNeedErrCodeHandle(uint32_t sessionId) const
{
    // version_102 omit to set sessionId so version_3 should skip to compare sessionid.
    if (sessionId == context_->GetRequestSessionId() || context_->GetRequestSessionId() == 0 ||
        context_->GetRemoteSoftwareVersion() == SOFTWARE_VERSION_RELEASE_2_0) {
        return true;
    } else {
        return false;
    }
}

void SingleVerSyncStateMachine::PushPullDataRequestEvokeErrHandle(SingleVerSyncTaskContext *context)
{
    // the pushpull sync task should wait for send finished after remote dev get data occur E_EKEYREVOKED error.
    if (context->GetRemoteSoftwareVersion() > SOFTWARE_VERSION_RELEASE_2_0 &&
        context->GetMode() == SyncOperation::PUSH_AND_PULL) {
        LOGI("data request errCode = %d, wait for send finished", -E_EKEYREVOKED);
        context->SetTaskErrCode(-E_EKEYREVOKED);
        context->SetOperationStatus(SyncOperation::RECV_FINISHED);
        SwitchStateAndStep(Event::RECV_FINISHED_EVENT);
    } else {
        context->SetTaskErrCode(-E_EKEYREVOKED);
        SwitchStateAndStep(Event::INNER_ERR_EVENT);
    }
}
} // namespace DistributedDB
