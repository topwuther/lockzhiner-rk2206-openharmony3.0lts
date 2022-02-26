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

#include <sstream>
#include "nlohmann/json.hpp"

#include "dm_ability_manager.h"
#include "request_session.h"
#include "constants.h"
#include "device_manager_log.h"
#include "msg_codec.h"
#include "device_manager_errno.h"
#include "ipc_server_adapter.h"
#include "encrypt_utils.h"
#include "softbus_adapter.h"
#include "ipc_server_listener_adapter.h"
#include "hichain_connector.h"
#include "softbus_session.h"

namespace OHOS {
namespace DistributedHardware {
RequestSession::RequestSession(std::string &hostPkgName, std::string &targetPkgName, const DmDeviceInfo &devReqInfo,
    const DmAppImageInfo &imageInfo)
{
    DMLOG(DM_LOG_INFO, "RequestSession construction started");
    mDevInfo_ = devReqInfo;
    mImageInfo_ = imageInfo;
    mHostPkgName_ = hostPkgName;
    mTargetPkgName = targetPkgName;
    mPinToken_ = IpcServerAdapter::GenRandInt(MIN_PIN_TOKEN, MAX_PIN_TOKEN);
    char randStr[TOKEN_LEN] = {0};
    bool res = EncryptUtils::MbedtlsGenRandomStr(randStr, sizeof(randStr), false);
    if (res == false) {
        DMLOG(DM_LOG_ERROR, "get Random string failed");
        mToken_ = "";
        return;
    }
    mToken_ = randStr;
    DMLOG(DM_LOG_INFO, "RequestSession construction completed");
}

std::vector<std::string> RequestSession::GetRequestCommand(std::string &extrasJson)
{
    return MsgCodec::EncodeReqAppAuth(mToken_, mHostPkgName_, mTargetPkgName, mDevInfo_, mImageInfo_, extrasJson);
}

int32_t RequestSession::GetPinToken()
{
    return mPinToken_;
}

bool RequestSession::IsWaitingForScan()
{
    return mStatus_ == StatusType::STATUS_WATING_SCAN_OR_INPUT;
}

std::string RequestSession::GetToken()
{
    return mToken_;
}

bool RequestSession::IsMyPinToken(int32_t pinToken)
{
    return pinToken == mPinToken_;
}

void RequestSession::OnReceivePinCode(int32_t pinCode)
{
    if (mStatus_ != StatusType::STATUS_WATING_SCAN_OR_INPUT) {
        DMLOG(DM_LOG_ERROR, "mStatus_ is incorrect, mStatus_: %d", mStatus_);
        return;
    }

    DMLOG(DM_LOG_INFO, "RequestSession::OnReceivePinCode");
    responseMsgPtr_->SavePinCode(pinCode);
    HichainConnector::GetInstance().AddMemeber(mDevInfo_.deviceId, responseMsgPtr_);
    mStatus_ = StatusType::STATUS_WAITING_ADD_GROUP;
}

void RequestSession::OnUserOperate(int32_t action)
{
    if (action == FaAction::USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT) {
        Release();
        return;
    }
}

int64_t RequestSession::GetRequestId()
{
    return mRequestId_;
}

std::string RequestSession::GetRequestDeviceId()
{
    return mRemoteDeviceId_;
}

int32_t RequestSession::StartFaService()
{
    DMLOG(DM_LOG_INFO, "RequestSession::StartFaService in");
    AbilityStatus status = DmAbilityManager::GetInstance().StartAbility(AbilityRole::ABILITY_ROLE_INITIATIVE);
    if (status != AbilityStatus::ABILITY_STATUS_SUCCESS) {
        DMLOG(DM_LOG_ERROR, "RequestSession::StartFaService timeout");
        return FAIL;
    }
    return SUCCESS;
}

bool RequestSession::IsFinished()
{
    if (mStatus_ == StatusType::STATUS_INIT || mStatus_ == StatusType::STATUS_FINISH) {
        return true;
    }
    return false;
}

bool RequestSession::IsMyChannelId(long long channelId)
{
    return channelId == mChannelId_;
}

void RequestSession::OnReceiveMsg(std::string &msg)
{
    if (mStatus_ != StatusType::STATUS_WAITING_REPLY) {
        DMLOG(DM_LOG_ERROR, "StatusType is not waiting reply");
        return;
    }
    int32_t reply = ParseRespMsg(msg);
    DMLOG(DM_LOG_INFO, "reply is : %d", reply);
    if (reply == SESSION_REPLY_ACCEPT) {
        mStatus_ = StatusType::STATUS_WATING_SCAN_OR_INPUT;
        CloseChannel();
    } else {
        Release();
    }
    NotifyHostAppAuthResult(reply);
}

std::string RequestSession::GetHostPkgName()
{
    return mHostPkgName_;
}

std::string RequestSession::GetTargetPkgName()
{
    return mTargetPkgName;
}

int32_t RequestSession::GetSessionType()
{
    return mSessionType_;
}

void RequestSession::Release()
{
    if (mStatus_ == StatusType::STATUS_FINISH || mStatus_ == StatusType::STATUS_INIT) {
        DMLOG(DM_LOG_INFO, "session is already closed");
        return;
    }
    DMLOG(DM_LOG_INFO, "close this session");
    mStatus_ = StatusType::STATUS_FINISH;
    CloseChannel();
}

void RequestSession::CloseChannel()
{
    if (mIsChannelOpened_) {
        SoftbusSession::GetInstance().CloseSession(mChannelId_);
        DMLOG(DM_LOG_ERROR, "RequestSession:: close the channel");
        mIsChannelOpened_ = false;
    }
}

int32_t RequestSession::ParseRespMsg(std::string &msg)
{
    DMLOG(DM_LOG_INFO, "ParseRespMsg started");
    auto msgResponseAuthPtr = MsgCodec::DecodeResponseAuth(msg);
    if (msgResponseAuthPtr == nullptr) {
        DMLOG(DM_LOG_ERROR, "DecodeResponseAuth error");
        return SESSION_REPLY_UNKNOWN;
    }
    responseMsgPtr_ = msgResponseAuthPtr;
    int32_t reply = msgResponseAuthPtr->GetReply();
    mRemoteDeviceId_ = msgResponseAuthPtr->GetDeviceId();
    if (reply == SESSION_REPLY_ACCEPT) {
        mRemoteNetId_ = msgResponseAuthPtr->GetNetId();
        mRemoteGroupId_ = msgResponseAuthPtr->GetGroupId();
        mRemoteGroupName_ = msgResponseAuthPtr->GetGroupName();
        mRequestId_ = msgResponseAuthPtr->GetRequestId();

        auto remoteGroupList = msgResponseAuthPtr->GetSyncGroupList();
        SyncDmPrivateGroup(remoteGroupList);
        DMLOG(DM_LOG_INFO, "user accepted the auth");
    }
    DMLOG(DM_LOG_INFO, "ParseRespMsg completed");
    return reply;
}

void RequestSession::SetChannelId(long long channelId)
{
    mChannelId_ = channelId;
    mIsChannelOpened_ = true;
    mStatus_ = StatusType::STATUS_WAITING_REPLY;
}

void RequestSession::SyncDmPrivateGroup(std::vector<std::string> &remoteGroupList)
{
    HichainConnector::GetInstance().SyncGroups(mRemoteDeviceId_, remoteGroupList);
    DMLOG(DM_LOG_INFO, "RequestSession::syncDmPrivateGroup started");
    std::vector<std::string> localGroups = {};
    std::string synGroupMsg = MsgCodec::EncodeSyncGroup(localGroups, mRemoteDeviceId_);
    SoftbusSession::GetInstance().SendMsg(mChannelId_, synGroupMsg);
    DMLOG(DM_LOG_INFO, "RequestSession::SyncDmPrivateGroup completed");
}

void RequestSession::NotifyHostAppAuthResult(int32_t errorCode)
{
    if (mSessionType_ != SESSION_TYPE_IS_APP_AUTH) {
        DMLOG(DM_LOG_ERROR, "wrong session type: %d", mSessionType_);
        return;
    }

    std::string deviceId = mDevInfo_.deviceId;
    if (errorCode != SESSION_REPLY_ACCEPT) {
        IpcServerListenerAdapter::GetInstance().OnAuthResult(mHostPkgName_, deviceId, mPinToken_, FAIL, errorCode);
        DMLOG(DM_LOG_INFO, "notify host result, errorcode: %d", errorCode);
        return;
    }

    if (StartFaService() != SUCCESS) {
        DMLOG(DM_LOG_INFO, "RequestSession::StartFaService failed");
        return;
    }
    DMLOG(DM_LOG_INFO, "RequestSession::StartFaService success");
    IpcServerListenerAdapter::GetInstance().OnAuthResult(mHostPkgName_, deviceId, mPinToken_, SUCCESS, errorCode);
}
}
}
