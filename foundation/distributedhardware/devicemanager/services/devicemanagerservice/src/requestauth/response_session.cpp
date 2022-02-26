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

#include "parameter.h"

#include "dm_ability_manager.h"
#include "response_session.h"
#include "constants.h"
#include "msg_codec.h"
#include "device_manager_log.h"
#include "device_manager_errno.h"
#include "encrypt_utils.h"
#include "ipc_server_adapter.h"
#include "softbus_adapter.h"
#include "softbus_session.h"
#include "hichain_connector.h"
#include "pthread.h"
#include "ipc_server_adapter.h"
#include "dm_device_info.h"
#include "ipc_server_listener_adapter.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t SESSION_CANCEL_TIMEOUT = 0;
const int32_t SESSION_MSG_RECEIVE_TIMEOUT = 5;
const int32_t SESSION_WAIT_MEMBER_JOIN_TIMEOUT = 120;
const int32_t CANCEL_PICODE_DISPLAY = 1;
const int32_t DEVICE_ID_HALF = 2;
}

ResponseSession::ResponseSession()
{
    mSessionStatus_ = ResponseSessionStatus::SESSION_INIT;
    sem_init(&mSem_, 0, 0);
    mRequestId_ = -1;
    mGroupId_ = "";
    mGroupName_ = "";
    mReqDeviceId_ = "";
    mMsgRequestAuthPtr_ = nullptr;
    mChannelId_ = -1;
    mPincode_ = -1;
}

int64_t ResponseSession::GetRequestId()
{
    return mRequestId_;
}

int32_t ResponseSession::SendResponseMessage(int32_t reply)
{
    DMLOG(DM_LOG_INFO, "ResponseSession::SendResponseMessage in, reply:%d", reply);
    std::string msg;
    if (reply == SESSION_REPLY_ACCEPT) {
        msg = MsgCodec::EncodeAcceptRespAuth(reply, mRequestId_, mGroupId_, mGroupName_, mReqDeviceId_);
    } else {
        msg = MsgCodec::EncodeRefuseRespAuth(reply, mReqDeviceId_);
    }

    int32_t ret = SoftbusSession::GetInstance().SendMsg(mChannelId_, msg);
    if (ret != SUCCESS) {
        DMLOG(DM_LOG_INFO, "SendResponseMessage failed ret:%d, reply:%d", ret, reply);
        return ret;
    }
    DMLOG(DM_LOG_INFO, "SendResponseMessage successed, reply:%d", reply);
    return SUCCESS;
}

void ResponseSession::OnUserOperate(int32_t action)
{
    switch (action) {
        case FaAction::USER_OPERATION_TYPE_ALLOW_AUTH: {
            OnUserConfirm();
            break;
        }
        case FaAction::USER_OPERATION_TYPE_CANCEL_AUTH: {
            OnUserReject(ERROR_USER_REJECT);
            break;
        }
        case FaAction::USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT: {
            OnUserReject(SESSION_REPLY_AUTH_CONFIRM_TIMEOUT);
            break;
        }
        case FaAction::USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY: {
            DMLOG(DM_LOG_ERROR, "cancle pincode display");
            Release();
            break;
        }
        default: {
            DMLOG(DM_LOG_ERROR, "action %d not support", action);
            break;
        }
    }
}

int32_t ResponseSession::DecodeReqMsg(std::string &message)
{
    std::shared_ptr<MsgRequestAuth> authMsg = MsgCodec::DecodeRequestAuth(message, mMsgRequestAuthPtr_);
    if (authMsg == nullptr) {
        DMLOG(DM_LOG_ERROR, "DecodeRequestAuth error, authMsg(nullptr)");
        return FAIL;
    }

    if (authMsg->GetMsgSlice() == FAIL) {
        DMLOG(DM_LOG_ERROR, "DecodeRequestAuth error, GetMsgSlice() FAIL");
        return FAIL;
    }

    mMsgRequestAuthPtr_ = authMsg;
    return SUCCESS;
}

bool ResponseSession::IsMyChannelId(long long channelId)
{
    if (mSessionStatus_ == ResponseSessionStatus::SESSION_INIT) {
        return true;
    }

    return channelId == mChannelId_ && mChannelId_ > 0;
}

void OnReceiveTimeOut(void *data)
{
    DMLOG(DM_LOG_ERROR, "OnReceiveTimeOut TimeOut called");
    ResponseSession *respSess = (ResponseSession*)data;
    if (respSess == nullptr || respSess->GetStatus() == ResponseSessionStatus::SESSION_INIT) {
        DMLOG(DM_LOG_ERROR, "respSess Status is INIT");
        return;
    }

    respSess->Release();
}

void OnMemberJoinTimeOut(void *data)
{
    DMLOG(DM_LOG_ERROR, "OnMemberJoinTimeOut TimeOut caled");
    ResponseSession *respSess = (ResponseSession*)data;
    if (respSess == nullptr || respSess->GetStatus() == ResponseSessionStatus::SESSION_INIT) {
        DMLOG(DM_LOG_ERROR, "respSess Status is INIT");
        return;
    }

    respSess->SendResponseMessage(SESSION_REPLY_TIMEOUT);
    respSess->CancelDisplay();
    respSess->Release();
}

void ResponseSession::OnReceiveMsg(long long channelId, std::string &message)
{
    int32_t ret =  DecodeReqMsg(message);
    if (ret != SUCCESS) {
        DMLOG(DM_LOG_ERROR, "DecodeReqMsg failed");
        return;
    }

    if (!mReceiveTimerPtr_) {
        std::string mReceiveTimerName = "mReceiveTimer";
        mReceiveTimerPtr_ = std::make_shared<DmTimer>(mReceiveTimerName);
    }

    mReceiveTimerPtr_->Start(SESSION_MSG_RECEIVE_TIMEOUT, OnReceiveTimeOut, this);
    if (mMsgRequestAuthPtr_->GetMsgCnt() != mMsgRequestAuthPtr_->GetMsgSlice()) {
        return;
    }

    mReceiveTimerPtr_->Stop(SESSION_CANCEL_TIMEOUT);
    mReqDeviceId_ = mMsgRequestAuthPtr_->GetRequestDeviceId();
    mChannelId_ = channelId;
    mPincode_ = GeneratePincode();
    if (StartFaService() != SUCCESS) {
        OnUserReject(ERROR_FA_START_FAIL);
        return;
    }

    mSessionStatus_ = ResponseSessionStatus::SESSION_WAITTING_USER_CONFIRM;
    if (!mMemberJoinTimerPtr_) {
        std::string mMemberJoinTimerName = "mMemberJoinTimer";
        mMemberJoinTimerPtr_ = std::make_shared<DmTimer>(mMemberJoinTimerName);
    }

    mMemberJoinTimerPtr_->Start(SESSION_WAIT_MEMBER_JOIN_TIMEOUT, OnMemberJoinTimeOut, this);
}

int32_t ResponseSession::GetPinCodeByReqId(int64_t requestId)
{
    if (requestId != mRequestId_) {
        DMLOG(DM_LOG_ERROR, "ResponseSession:: GetPinCodeByReqId FAIL : requestId(not match)");
        return FAIL;
    }
    return mPincode_;
}

void ResponseSession::BuildAuthenticationInfo(DmAuthParam &authParam)
{
    DMLOG(DM_LOG_INFO, "ResponseSession::BuildAuthenticationInfo in");
    authParam.direction = (int32_t)DmAbilityManager::GetInstance().GetAbilityRole();
    if (mMsgRequestAuthPtr_ == nullptr) {
        DMLOG(DM_LOG_ERROR, "ResponseSession::BuildAuthenticationInfo mMsgRequestAuthPtr_ is nullptr");
        return;
    }

    authParam.packageName = mMsgRequestAuthPtr_->mTargetPkg_;
    authParam.appName = mMsgRequestAuthPtr_->mAppName_;
    authParam.appDescription = mMsgRequestAuthPtr_->mAppDescription_;
    authParam.authType = mMsgRequestAuthPtr_->mAuthType_;
    authParam.business = BUSINESS_FA_MIRGRATION; // currently, only support BUSINESS_FA_MIRGRATION
    authParam.pincode = mPincode_;

    if (mMsgRequestAuthPtr_ != nullptr) {
        authParam.imageinfo = mMsgRequestAuthPtr_->mImageInfo_;
    }
}

int32_t ResponseSession::StartFaService()
{
    DMLOG(DM_LOG_INFO, "ResponseSession::StartFaService in");
    AbilityStatus status = DmAbilityManager::GetInstance().StartAbility(AbilityRole::ABILITY_ROLE_PASSIVE);
    if (status != AbilityStatus::ABILITY_STATUS_SUCCESS) {
        DMLOG(DM_LOG_ERROR, "ResponseSession::StartFaService timeout");
        return FAIL;
    }
    return SUCCESS;
}

void ResponseSession::OnGroupCreated(int64_t requestId, const std::string &groupId)
{
    DMLOG(DM_LOG_INFO, "ResponseSession OnGroupCreated in requestId= %lld groupId = %s ", requestId, groupId.c_str());

    if (requestId != mRequestId_ || groupId.length() == 0) {
        DMLOG(DM_LOG_ERROR, "ResponseSession::OnGroupCreated failed, requestId %lld, requestId %lld, groupId %s",
            requestId, mRequestId_, groupId.c_str());
        SendResponseMessage(SESSION_REPLY_CREAT_GROUP_FAILED);
        CancelDisplay();
        mSessionStatus_ = ResponseSessionStatus::SESSION_INIT;
        return;
    }

    // save requestId for peer to join, need to be checked in HiChain OnRequest and OnFinish(AddMemberToGroup) callback
    mRequestId_ = HichainConnector::GetInstance().GenRequestId();
    mGroupId_ = groupId;
    int32_t ret = SendResponseMessage(SESSION_REPLY_ACCEPT);
    if (ret != SUCCESS) {
        CancelDisplay();
        mSessionStatus_ = ResponseSessionStatus::SESSION_INIT;
        return;
    }

    DMLOG(DM_LOG_INFO, "Send Accept Resoponse success, wait for peer to AddMemberToGroup");
    mSessionStatus_ = ResponseSessionStatus::SESSION_WAITTING_PIN_CODE;
}

void ResponseSession::OnMemberJoin(int64_t requestId, int32_t status)
{
    DMLOG(DM_LOG_INFO, "ResponseSession::OnMemberJoin, result: %d", status);
    CancelDisplay();
    mMemberJoinTimerPtr_->Stop(SESSION_CANCEL_TIMEOUT);
    Release();
}

std::string ResponseSession::GenerateGroupName()
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string sLocalDeviceID = localDeviceId;
    std::string groupName = mMsgRequestAuthPtr_->mTargetPkg_ + mMsgRequestAuthPtr_->mHostPkg_ +
        sLocalDeviceID.substr(0, sLocalDeviceID.size() / DEVICE_ID_HALF);
    return groupName;
}

void ResponseSession::OnUserConfirm()
{
    DMLOG(DM_LOG_INFO, "ResponseSession::OnUserConfirm in");
    mGroupName_ = GenerateGroupName();
    HichainConnector::GetInstance().RegisterConnectorCallback(shared_from_this());

    // save requestId for CreateGroup, need to be checked in callback: OnGroupCreated
    mRequestId_ = HichainConnector::GetInstance().GenRequestId();
    int32_t ret = HichainConnector::GetInstance().CreateGroup(mRequestId_, mGroupName_);
    if (ret != SUCCESS) {
        CancelDisplay();
        mSessionStatus_ = ResponseSessionStatus::SESSION_INIT;
        return;
    }
    mSessionStatus_ = ResponseSessionStatus::SESSION_WAITTING_GROUP_CREATED;
}

void ResponseSession::OnUserReject(int32_t errorCode)
{
    DMLOG(DM_LOG_INFO, "ResponseSession::OnUserReject in, errcode %d", errorCode);
    SendResponseMessage(errorCode);
    Release();
}

void ResponseSession::Release()
{
    DMLOG(DM_LOG_INFO, "ResponseSession::Release in");
    mSessionStatus_ = ResponseSessionStatus::SESSION_INIT;
    mRequestId_ = -1;
    mGroupId_ = "";
    mGroupName_ = "";
    mReqDeviceId_ = "";
    mMsgRequestAuthPtr_ = nullptr;
    mChannelId_ = -1;
    mPincode_ = -1;
}

void ResponseSession::CancelDisplay()
{
    DMLOG(DM_LOG_INFO, "Cancel PinCode Display in");
    nlohmann::json jsonObj;
    jsonObj[CANCEL_DISPLAY_KEY] = CANCEL_PICODE_DISPLAY;
    std::string paramJson = jsonObj.dump();
    std::string pkgName = "com.ohos.devicemanagerui";
    IpcServerListenerAdapter::GetInstance().OnFaCall(pkgName, paramJson);
    DMLOG(DM_LOG_INFO, "Cancel PinCode Display success");
}

int32_t ResponseSession::GetStatus()
{
    return mSessionStatus_;
}

int32_t ResponseSession::GeneratePincode()
{
    return EncryptUtils::GenRandInt(MIN_PIN_CODE, MAX_PIN_CODE);
}
} // namespace DistributedHardware
} // namespace OHOS
