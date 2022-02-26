/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "avrcp_ct_state_machine.h"
#include "avrcp_ct_pass_through.h"
#include "avrcp_ct_vendor.h"

namespace bluetooth {
/// The name of state "CONNECTING".
const std::string AVRC_CT_SM_STATE_CONNECTING = "CONNECTING";
/// The name of state "CONNECTED".
const std::string AVRC_CT_SM_STATE_CONNECTED = "CONNECTED";
/// The name of state "DISCONNECTING".
const std::string AVRC_CT_SM_STATE_DISCONNECTING = "DISCONNECTING";
/// The name of state "PENDING".
const std::string AVRC_CT_SM_STATE_PENDING = "PENDING";
/// The name of state "BUSY".
const std::string AVRC_CT_SM_STATE_BUSY = "BUSY";
/// The name of state "CONTINUATION".
const std::string AVRC_CT_SM_STATE_CONTINUATION = "CONTINUATION";
/// The name of state "DISABLE".
const std::string AVRC_CT_SM_STATE_DISABLE = "DISABLE";

AvrcCtStateMachineManager *AvrcCtStateMachineManager::GetInstance(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    static AvrcCtStateMachineManager instance;

    return &instance;
}

AvrcCtStateMachineManager::~AvrcCtStateMachineManager()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);
    stateMachines_.clear();
}

void AvrcCtStateMachineManager::ShutDown(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    stateMachines_.clear();
}

int AvrcCtStateMachineManager::AddControlStateMachine(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    int result = RET_NO_ERROR;

    if (GetPairOfStateMachine(rawAddr.GetAddress()) == nullptr) {
        std::unique_ptr<StateMachine> stateMachine =
            std::make_unique<StateMachine>(StateMachine::Type::AVRC_CT_SM_TYPE_CONTROL, rawAddr);
        stateMachine->AddStates();
        stateMachine->InitState(AVRC_CT_SM_STATE_CONNECTING);
        stateMachines_.insert(std::make_pair(rawAddr.GetAddress(), std::make_pair(std::move(stateMachine), nullptr)));
    } else {
        result = RET_BAD_PARAM;
        LOG_DEBUG("[AVRCP CT] The control state machine exists! - Address[%{public}s]", rawAddr.GetAddress().c_str());
    }

    return result;
}

int AvrcCtStateMachineManager::AddBrowseStateMachine(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    int result = RET_NO_ERROR;

    StateMachinePair *pair = GetPairOfStateMachine(rawAddr.GetAddress());
    if (pair == nullptr) {
        result = RET_BAD_PARAM;
        LOG_DEBUG(
            "[AVRCP CT] The pair of state machines doest not exists! - Address[%{public}s]", rawAddr.GetAddress().c_str());
    } else if (pair->second == nullptr) {
        pair->second = std::make_unique<StateMachine>(StateMachine::Type::AVRC_CT_SM_TYPE_BROWSE, rawAddr);
        pair->second->AddStates();
        pair->second->InitState(AVRC_CT_SM_STATE_CONNECTING);
    } else {
        result = RET_BAD_PARAM;
        LOG_DEBUG("[AVRCP CT] The browse state machine exists! - Address[%{public}s]", rawAddr.GetAddress().c_str());
    }

    return result;
}

void AvrcCtStateMachineManager::DeletePairOfStateMachine(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    StateMachinePair *pair = GetPairOfStateMachine(rawAddr.GetAddress());
    if (pair != nullptr) {
        stateMachines_.erase(rawAddr.GetAddress());
    } else {
        LOG_DEBUG("[AVRCP CT] The pair of state machines does not exist! - Address[%{public}s]", rawAddr.GetAddress().c_str());
    }
}

void AvrcCtStateMachineManager::DeleteBrowseStateMachine(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    StateMachinePair *pair = GetPairOfStateMachine(rawAddr.GetAddress());
    if (pair != nullptr && pair->second != nullptr) {
        pair->second = nullptr;
    } else {
        LOG_DEBUG("[AVRCP CT] The browse state machine does not exist! - Address[%{public}s]", rawAddr.GetAddress().c_str());
    }
}

bool AvrcCtStateMachineManager::SendMessageToControlStateMachine(const RawAddress &rawAddr, const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP CT] msg[%x]", msg.what_);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    bool result = false;

    StateMachinePair *pair = GetPairOfStateMachine(rawAddr.GetAddress());
    if (pair != nullptr && pair->first != nullptr) {
        result = pair->first->ProcessMessage(msg);
        if (!result) {
            LOG_DEBUG("[AVRCP CT] Unknown message[%{public}d]! - Address[%{public}s]", msg.what_, rawAddr.GetAddress().c_str());
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The control state machine does not exist! - Address[%{public}s]", rawAddr.GetAddress().c_str());
    }

    return result;
}

void AvrcCtStateMachineManager::SendMessageToAllControlStateMachine(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP CT] msg[%x]", msg.what_);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    for (auto &pairSm : stateMachines_) {
        if (pairSm.second.first != nullptr) {
            pairSm.second.first->ProcessMessage(msg);
        }
    }
}

bool AvrcCtStateMachineManager::SendMessageToBrowseStateMachine(const RawAddress &rawAddr, const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP CT] msg[%x]", msg.what_);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    bool result = false;

    StateMachinePair *pair = GetPairOfStateMachine(rawAddr.GetAddress());
    if (pair != nullptr && pair->second != nullptr) {
        result = pair->second->ProcessMessage(msg);
        if (!result) {
            LOG_DEBUG("[AVRCP CT] Unknown message[%{public}d]! - Address[%{public}s]", msg.what_, rawAddr.GetAddress().c_str());
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The browse state machine does not exist! - Address[%{public}s]", rawAddr.GetAddress().c_str());
    }

    return result;
}

void AvrcCtStateMachineManager::SendMessageToAllBrowseStateMachine(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP CT] msg[%x]", msg.what_);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    for (auto &pairSm : stateMachines_) {
        if (pairSm.second.second != nullptr) {
            pairSm.second.second->ProcessMessage(msg);
        }
    }
}

bool AvrcCtStateMachineManager::IsControlConnectingState(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    return IsControlSpecifiedState(rawAddr.GetAddress(), AVRC_CT_SM_STATE_CONNECTING);
}

bool AvrcCtStateMachineManager::IsControlConnectedState(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    return IsControlSpecifiedState(rawAddr.GetAddress(), AVRC_CT_SM_STATE_CONNECTED);
}

bool AvrcCtStateMachineManager::IsControlDisconnectingState(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    return IsControlSpecifiedState(rawAddr.GetAddress(), AVRC_CT_SM_STATE_DISCONNECTING);
}

bool AvrcCtStateMachineManager::IsControlPendingState(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    return IsControlSpecifiedState(rawAddr.GetAddress(), AVRC_CT_SM_STATE_PENDING);
}

bool AvrcCtStateMachineManager::IsBrowsePendingState(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    return IsBrowseSpecifiedState(rawAddr.GetAddress(), AVRC_CT_SM_STATE_PENDING);
}

bool AvrcCtStateMachineManager::IsControlContinuationState(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    return IsControlSpecifiedState(rawAddr.GetAddress(), AVRC_CT_SM_STATE_CONTINUATION);
}

bool AvrcCtStateMachineManager::IsControlDisableState(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    return IsControlSpecifiedState(rawAddr.GetAddress(), AVRC_CT_SM_STATE_DISABLE);
}

AvrcCtStateMachineManager::AvrcCtStateMachineManager()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);
}

AvrcCtStateMachineManager::StateMachinePair *AvrcCtStateMachineManager::GetPairOfStateMachine(const std::string &addr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    StateMachinePair *pair = nullptr;

    auto iter = stateMachines_.find(addr);
    if (iter != stateMachines_.end()) {
        pair = &iter->second;
    }

    return pair;
}

bool AvrcCtStateMachineManager::IsControlSpecifiedState(const std::string &addr, std::string stateName)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    int result = false;

    StateMachinePair *pair = GetPairOfStateMachine(addr);
    if (pair != nullptr && pair->first != nullptr) {
        if (pair->first->GetState()->Name().compare(stateName) == 0) {
            result = true;
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The pair of state machines does not exist!");
    }

    return result;
}

bool AvrcCtStateMachineManager::IsBrowseSpecifiedState(const std::string &addr, std::string stateName)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtStateMachineManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    int result = false;

    StateMachinePair *pair = GetPairOfStateMachine(addr);
    if (pair != nullptr && pair->second != nullptr) {
        if (pair->second->GetState()->Name().compare(stateName) == 0) {
            result = true;
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The pair of state machines does not exist!");
    }

    return result;
}

AvrcCtStateMachineManager::StateMachine::StateMachine(Type type, const RawAddress &rawAddr)
    : type_(type), rawAddr_(rawAddr)
{
    LOG_DEBUG("[AVRCP CT] StateMachine::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::AddStates(void)
{
    LOG_DEBUG("[AVRCP CT] StateMachine::%{public}s", __func__);

    if (type_ == Type::AVRC_CT_SM_TYPE_CONTROL) {
        /// CONNECTING state.

        std::unique_ptr<utility::StateMachine::State> ctConnectingPtr =
            std::make_unique<CtConnecting>(AVRC_CT_SM_STATE_CONNECTING, *this, rawAddr_);

        /// CONNECTED state.

        std::unique_ptr<utility::StateMachine::State> ctConnectedPtr =
            std::make_unique<CtConnected>(AVRC_CT_SM_STATE_CONNECTED, *this, rawAddr_, *ctConnectingPtr.get());

        /// DISCONNECTING state.

        std::unique_ptr<utility::StateMachine::State> ctDisconnectingPtr =
            std::make_unique<CtDisconnecting>(AVRC_CT_SM_STATE_DISCONNECTING, *this, rawAddr_, *ctConnectedPtr.get());

        /// PENDING state.

        std::unique_ptr<utility::StateMachine::State> ctPendingPtr =
            std::make_unique<CtPending>(AVRC_CT_SM_STATE_PENDING, *this, rawAddr_, *ctConnectedPtr.get());

        /// CONTINUATION state.

        std::unique_ptr<utility::StateMachine::State> ctContinuationPtr =
            std::make_unique<CtContinuation>(AVRC_CT_SM_STATE_CONTINUATION, *this, rawAddr_, *ctConnectedPtr.get());

        /// DISABLE state.

        std::unique_ptr<utility::StateMachine::State> ctDisablePtr =
            std::make_unique<CtDisable>(AVRC_CT_SM_STATE_DISABLE, *this, rawAddr_, *ctConnectedPtr.get());

        Move(ctDisconnectingPtr);
        Move(ctPendingPtr);
        Move(ctContinuationPtr);
        Move(ctDisablePtr);
        Move(ctConnectedPtr);
        Move(ctConnectingPtr);
    } else if (type_ == Type::AVRC_CT_SM_TYPE_BROWSE) {
        /// CONNECTING state.

        std::unique_ptr<utility::StateMachine::State> brConnectingPtr =
            std::make_unique<BrConnecting>(AVRC_CT_SM_STATE_CONNECTING, *this, rawAddr_);

        /// CONNECTED state.

        std::unique_ptr<utility::StateMachine::State> brConnectedPtr =
            std::make_unique<BrConnected>(AVRC_CT_SM_STATE_CONNECTED, *this, rawAddr_, *brConnectingPtr.get());

        /// DISCONNECTING state.

        std::unique_ptr<utility::StateMachine::State> brDisconnectingPtr =
            std::make_unique<BrDisconnecting>(AVRC_CT_SM_STATE_DISCONNECTING, *this, rawAddr_, *brConnectedPtr.get());

        /// PENDING state.

        std::unique_ptr<utility::StateMachine::State> brPendingPtr =
            std::make_unique<BrPending>(AVRC_CT_SM_STATE_PENDING, *this, rawAddr_, *brConnectedPtr.get());

        Move(brPendingPtr);
        Move(brDisconnectingPtr);
        Move(brConnectedPtr);
        Move(brConnectingPtr);
    } else {
        LOG_DEBUG("[AVRCP CT] The type of the state machine is wrong!");
    }
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::State                 *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::State::State(const std::string &name, utility::StateMachine &stateMachine, const RawAddress &rawAddr, utility::StateMachine::State &parent)
    : utility::StateMachine::State(name, stateMachine, parent), rawAddr_(rawAddr)
{
    LOG_DEBUG("[AVRCP CT] State::%{public}s", __func__);
}

AvrcCtStateMachineManager::StateMachine::State::State(
    const std::string &name, utility::StateMachine &stateMachine, const RawAddress &rawAddr)
    : utility::StateMachine::State(name, stateMachine), rawAddr_(rawAddr)
{
    LOG_DEBUG("[AVRCP CT] State::%{public}s", __func__);
}

AvrcCtStateMachineManager::StateMachine::State::~State()
{
    LOG_DEBUG("[AVRCP CT] State::%{public}s", __func__);
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::CtConnecting          *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::CtConnecting::CtConnecting(
    const std::string &name, utility::StateMachine &stateMachine, const RawAddress &rawAddr)
    : State(name, stateMachine, rawAddr)
{
    LOG_DEBUG("[AVRCP CT] CtConnecting::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::CtConnecting::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] CtConnecting::%{public}s", __func__);

    const AvrcCtConnectInfo *info = AvrcCtConnectManager::GetInstance()->GetConnectInfo(rawAddr_);

    AVCT_Register(info->controlMtu_, info->browseMtu_, AVCT_CT);

    uint8_t connectId = 0x00;
    AvctConnectParam param = {info->eventCallback_, info->msgCallback_, AVRC_CT_AV_REMOTE_CONTROL, info->role_,
        nullptr};
    BtAddr btAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    rawAddr_.ConvertToUint8(btAddr.addr);

    if (AVCT_ConnectReq(&connectId, &param, &btAddr) != AVCT_SUCCESS) {
        info->eventCallback_(connectId, AVCT_CONNECT_CFM_EVT, AVCT_FAILED, &btAddr, nullptr);
        LOG_DEBUG("[AVRCP CT] Call - AVCT_ConnectReq Failed! - Address[%{public}s]", rawAddr_.GetAddress().c_str());
    }
}

void AvrcCtStateMachineManager::StateMachine::CtConnecting::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] CtConnecting::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::CtConnecting::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] CtConnecting::%{public}s", __func__);

    bool result = true;

    switch (msg.what_) {
        case AVRC_CT_SM_EVENT_TO_DISABLE_STATE:
            Transition(AVRC_CT_SM_STATE_DISABLE);
            break;
        case AVRC_CT_SM_EVENT_TO_CONNECTED_STATE:
            Transition(AVRC_CT_SM_STATE_CONNECTED);
            break;
        default:
            result = false;
            break;
    }

    return result;
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::CtConnected           *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::CtConnected::CtConnected(const std::string &name,
    utility::StateMachine &stateMachine, const RawAddress &rawAddr, utility::StateMachine::State &parent)
    : State(name, stateMachine, rawAddr, parent)
{
    LOG_DEBUG("[AVRCP CT] CtConnected::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::CtConnected::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] CtConnected::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::CtConnected::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] CtConnected::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::CtConnected::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] CtConnected::%{public}s", __func__);

    bool result = true;
    AvrcCtConnectManager *cnManager = AvrcCtConnectManager::GetInstance();

    switch (msg.what_) {
        case AVRC_CT_SM_EVENT_TO_DISCONNECTING_STATE:
            Transition(AVRC_CT_SM_STATE_DISCONNECTING);
            break;
        case AVRC_CT_SM_EVENT_PASS_THROUGH: {
            std::shared_ptr<AvrcCtPassPacket> packet = cnManager->GetPassPacket(rawAddr_);

            AVCT_SendMsgReq(
                cnManager->GetConnectId(rawAddr_), AVRC_DEFAULT_LABEL, AVCT_COMMAND, packet->AssemblePacket());
            break;
        }
        case AVRC_CT_SM_EVENT_UNIT_INFO:
            /// FALL THROUGH
        case AVRC_CT_SM_EVENT_SUB_UNIT_INFO: {
            std::shared_ptr<AvrcCtUnitPacket> packet = cnManager->GetUnitPacket(rawAddr_);

            AVCT_SendMsgReq(
                cnManager->GetConnectId(rawAddr_), AVRC_DEFAULT_LABEL, AVCT_COMMAND, packet->AssemblePacket());

            Transition(AVRC_CT_SM_STATE_PENDING);
            break;
        }
        case AVRC_CT_SM_EVENT_REGISTER_NOTIFICATION:
        case AVRC_CT_SM_EVENT_GET_CAPABILITIES:
        case AVRC_CT_SM_EVENT_LIST_PLAYER_APPLICATION_SETTING_ATTRIBUTES:
        case AVRC_CT_SM_EVENT_LIST_PLAYER_APPLICATION_SETTING_VALUES:
        case AVRC_CT_SM_EVENT_GET_CURRENT_PLAYER_APPLICATION_SETTING_VALUE:
        case AVRC_CT_SM_EVENT_SET_PLAYER_APPLICATION_SETTING_VALUE:
        case AVRC_CT_SM_EVENT_GET_PLAYER_APPLICATION_SETTING_ATTRIBUTE_TEXT:
        case AVRC_CT_SM_EVENT_GET_PLAYER_APPLICATION_SETTING_VALUE_TEXT:
        case AVRC_CT_SM_EVENT_GET_ELEMENT_ATTRIBUTES:
        case AVRC_CT_SM_EVENT_GET_PLAY_STATUS:
        case AVRC_CT_SM_EVENT_REQUEST_CONTINUING_RESPONSE:
        case AVRC_CT_SM_EVENT_ABORT_CONTINUING_RESPONSE:
        case AVRC_CT_SM_EVENT_SET_ADDRESSED_PLAYER:
        case AVRC_CT_SM_EVENT_PLAY_ITEM:
        case AVRC_CT_SM_EVENT_ADD_TO_NOW_PLAYING:
        case AVRC_CT_SM_EVENT_SET_ABSOLUTE_VOLUME: {
            std::shared_ptr<AvrcCtVendorPacket> packet = cnManager->GetVendorPacket(rawAddr_);

            AVCT_SendMsgReq(
                cnManager->GetConnectId(rawAddr_), AVRC_DEFAULT_LABEL, AVCT_COMMAND, packet->AssemblePacket());

            Transition(AVRC_CT_SM_STATE_PENDING);
            break;
        }
        default:
            result = false;
            break;
    }

    return result;
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::CtDisconnecting       *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::CtDisconnecting::CtDisconnecting(const std::string &name,
    utility::StateMachine &stateMachine, const RawAddress &rawAddr, utility::StateMachine::State &parent)
    : State(name, stateMachine, rawAddr, parent)
{
    LOG_DEBUG("[AVRCP CT] CtDisconnecting::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::CtDisconnecting::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] CtDisconnecting::%{public}s", __func__);

    const AvrcCtConnectInfo *info = AvrcCtConnectManager::GetInstance()->GetConnectInfo(rawAddr_);

    if (AVCT_DisconnectReq(info->connectId_) != AVCT_SUCCESS) {
        BtAddr btAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
        rawAddr_.ConvertToUint8(btAddr.addr);
        info->eventCallback_(info->connectId_, AVCT_DISCONNECT_CFM_EVT, AVCT_FAILED, &btAddr, nullptr);
        LOG_DEBUG("[AVRCP CT] Call - AVCT_DisconnectReq Failed! - Address[%{public}s]", rawAddr_.GetAddress().c_str());
    }
}

void AvrcCtStateMachineManager::StateMachine::CtDisconnecting::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] CtDisconnecting::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::CtDisconnecting::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] CtDisconnecting::%{public}s", __func__);

    bool result = true;

    switch (msg.what_) {
        default:
            result = false;
            break;
    }

    return result;
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::CtPending             *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::CtPending::CtPending(const std::string &name,
    utility::StateMachine &stateMachine, const RawAddress &rawAddr, utility::StateMachine::State &parent)
    : State(name, stateMachine, rawAddr, parent)
{
    LOG_DEBUG("[AVRCP CT] CtPending::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::CtPending::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] CtPending::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::CtPending::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] CtPending::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::CtPending::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] CtPending::%{public}s", __func__);

    bool result = true;

    switch (msg.what_) {
        case AVRC_CT_SM_EVENT_TO_CONNECTED_STATE:
            Transition(AVRC_CT_SM_STATE_CONNECTED);
            break;
        default:
            result = false;
            break;
    }

    return result;
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::CtContinuation        *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::CtContinuation::CtContinuation(const std::string &name,
    utility::StateMachine &stateMachine, const RawAddress &rawAddr, utility::StateMachine::State &parent)
    : State(name, stateMachine, rawAddr, parent)
{
    LOG_DEBUG("[AVRCP CT] CtContinuation::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::CtContinuation::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] CtContinuation::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::CtContinuation::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] CtContinuation::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::CtContinuation::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] CtContinuation::%{public}s", __func__);

    bool result = true;

    switch (msg.what_) {
        case AVRC_CT_SM_EVENT_TO_CONNECTED_STATE:
            Transition(AVRC_CT_SM_STATE_CONNECTED);
            break;

        default:
            result = false;
            break;
    }

    return result;
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::CtDisable             *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::CtDisable::CtDisable(const std::string &name,
    utility::StateMachine &stateMachine, const RawAddress &rawAddr, utility::StateMachine::State &parent)
    : State(name, stateMachine, rawAddr, parent)
{
    LOG_DEBUG("[AVRCP CT] CtDisable::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::CtDisable::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] CtDisable::%{public}s", __func__);

    AVCT_Deregister();

    const AvrcCtConnectInfo *info = AvrcCtConnectManager::GetInstance()->GetConnectInfo(rawAddr_);

    if (AVCT_DisconnectReq(info->connectId_) != AVCT_SUCCESS) {
        BtAddr btAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
        rawAddr_.ConvertToUint8(btAddr.addr);

        LOG_DEBUG("[AVRCP CT] Call - AVCT_DisconnectReq Failed! - Address[%{public}s]", rawAddr_.GetAddress().c_str());
        info->eventCallback_(info->connectId_, AVCT_DISCONNECT_CFM_EVT, AVCT_FAILED, &btAddr, nullptr);
    }
}

void AvrcCtStateMachineManager::StateMachine::CtDisable::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] CtDisable::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::CtDisable::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] CtDisable::%{public}s", __func__);

    bool result = true;

    switch (msg.what_) {
        default:
            result = false;
            break;
    }

    return result;
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::BrConnecting          *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::BrConnecting::BrConnecting(
    const std::string &name, utility::StateMachine &stateMachine, const RawAddress &rawAddr)
    : State(name, stateMachine, rawAddr)
{
    LOG_DEBUG("[AVRCP CT] BrConnecting::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::BrConnecting::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] BrConnecting::%{public}s", __func__);

    const AvrcCtConnectInfo *info = AvrcCtConnectManager::GetInstance()->GetConnectInfo(rawAddr_);

    if (AVCT_BrConnectReq(info->connectId_, info->role_) != AVCT_SUCCESS) {
        BtAddr btAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
        rawAddr_.ConvertToUint8(btAddr.addr);
        info->eventCallback_(info->connectId_, AVCT_BR_CONNECT_CFM_EVT, AVCT_FAILED, &btAddr, nullptr);
        LOG_DEBUG("[AVRCP CT] Call - AVCT_BrConnectReq Failed! - Address[%{public}s]", rawAddr_.GetAddress().c_str());
    }
}

void AvrcCtStateMachineManager::StateMachine::BrConnecting::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] BrConnecting::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::BrConnecting::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] BrConnecting::%{public}s", __func__);

    bool result = true;

    switch (msg.what_) {
        case AVRC_CT_SM_EVENT_TO_DISABLE_STATE:
            Transition(AVRC_CT_SM_STATE_DISABLE);
            break;
        case AVRC_CT_SM_EVENT_TO_CONNECTED_STATE:
            Transition(AVRC_CT_SM_STATE_CONNECTED);
            break;
        default:
            result = false;
            break;
    }

    return result;
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::BrConnected           *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::BrConnected::BrConnected(const std::string &name,
    utility::StateMachine &stateMachine, const RawAddress &rawAddr, utility::StateMachine::State &parent)
    : State(name, stateMachine, rawAddr, parent)
{
    LOG_DEBUG("[AVRCP CT] BrConnected::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::BrConnected::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] BrConnected::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::BrConnected::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] BrConnected::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::BrConnected::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] BrConnected::%{public}s", __func__);

    bool result = true;
    AvrcCtConnectManager *cnManager = AvrcCtConnectManager::GetInstance();

    switch (msg.what_) {
        case AVRC_CT_SM_EVENT_TO_DISCONNECTING_STATE:
            Transition(AVRC_CT_SM_STATE_DISCONNECTING);
            break;
        case AVRC_CT_SM_EVENT_SET_BROWSED_PLAYER:
        case AVRC_CT_SM_EVENT_CHANGE_PATH:
        case AVRC_CT_SM_EVENT_GET_FOLDER_ITEMS:
        case AVRC_CT_SM_EVENT_GET_ITEM_ATTRIBUTES:
        case AVRC_CT_SM_EVENT_GET_TOTAL_NUMBER_OF_ITEMS: {
            std::shared_ptr<AvrcCtBrowsePacket> packet = cnManager->GetBrowsePacket(rawAddr_);

            AVCT_BrSendMsgReq(
                cnManager->GetConnectId(rawAddr_), AVRC_DEFAULT_LABEL, AVCT_COMMAND, packet->AssemblePacket());

            Transition(AVRC_CT_SM_STATE_PENDING);
            break;
        }
        default:
            result = false;
            break;
    }

    return result;
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::BrDisconnecting       *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::BrDisconnecting::BrDisconnecting(const std::string &name,
    utility::StateMachine &stateMachine, const RawAddress &rawAddr, utility::StateMachine::State &parent)
    : State(name, stateMachine, rawAddr, parent)
{
    LOG_DEBUG("[AVRCP CT] BrDisconnecting::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::BrDisconnecting::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] BrDisconnecting::%{public}s", __func__);

    const AvrcCtConnectInfo *info = AvrcCtConnectManager::GetInstance()->GetConnectInfo(rawAddr_);

    if (AVCT_BrDisconnectReq(info->connectId_) != AVCT_SUCCESS) {
        BtAddr btAddr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
        rawAddr_.ConvertToUint8(btAddr.addr);
        info->eventCallback_(info->connectId_, AVCT_BR_DISCONNECT_CFM_EVT, AVCT_FAILED, &btAddr, nullptr);
        LOG_DEBUG("[AVRCP CT] Call - AVCT_BrDisconnectReq Failed! - Address[%{public}s]", rawAddr_.GetAddress().c_str());
    }
}

void AvrcCtStateMachineManager::StateMachine::BrDisconnecting::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] BrDisconnecting::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::BrDisconnecting::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] BrDisconnecting::%{public}s", __func__);

    bool result = true;

    switch (msg.what_) {
        default:
            result = false;
            break;
    }

    return result;
}

/******************************************************************
 * AvrcCtStateMachineManager::StateMachine::BrPending             *
 ******************************************************************/

AvrcCtStateMachineManager::StateMachine::BrPending::BrPending(const std::string &name,
    utility::StateMachine &stateMachine, const RawAddress &rawAddr, utility::StateMachine::State &parent)
    : State(name, stateMachine, rawAddr, parent)
{
    LOG_DEBUG("[AVRCP CT] BrPending::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::BrPending::Entry(void)
{
    LOG_DEBUG("[AVRCP CT] BrPending::%{public}s", __func__);
}

void AvrcCtStateMachineManager::StateMachine::BrPending::Exit(void)
{
    LOG_DEBUG("[AVRCP CT] BrPending::%{public}s", __func__);
}

bool AvrcCtStateMachineManager::StateMachine::BrPending::Dispatch(const utility::Message &msg)
{
    LOG_DEBUG("[AVRCP CT] BrPending::%{public}s", __func__);

    bool result = true;

    switch (msg.what_) {
        case AVRC_CT_SM_EVENT_TO_CONNECTED_STATE:
            Transition(AVRC_CT_SM_STATE_CONNECTED);
            break;
        default:
            result = false;
            break;
    }

    return result;
}
}  // namespace bluetooth
