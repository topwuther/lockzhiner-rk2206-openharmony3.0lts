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

#include "auth_manager.h"
#include "anonymous_string.h"
#include "dm_ability_manager.h"
#include "constants.h"
#include "msg_codec.h"
#include "device_manager_log.h"
#include "msg_codec.h"
#include "device_manager_errno.h"
#include "softbus_session.h"
#include "encrypt_utils.h"
#include "ipc_server_listener_adapter.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(AuthManager);

void AuthManager::AuthDeviceGroup(std::string &hostPkgName, const DmDeviceInfo &devReqInfo,
    const DmAppImageInfo &imageInfo, std::string &extras)
{
    AuthAppGroup(hostPkgName, devReqInfo, imageInfo, extras);
}

void AuthManager::OnReceiveMsg(long long channelId, std::string &message)
{
    int32_t msgType = MsgCodec::DecodeMsgType(message);
    DMLOG(DM_LOG_INFO, "message type is, %d", msgType);
    switch (msgType) {
        case MSG_TYPE_RESP_AUTH:
        case MSG_TYPE_REQ_AUTH:
        case MSG_TYPE_AUTH_BY_PIN:
            OnReceiveMessage(channelId, message, msgType);
            break;
        default:
            DMLOG(DM_LOG_INFO, "msgType not support yet, msgType: %d", msgType);
            break;
    }
}

void AuthManager::AuthAppGroup(std::string &hostPkgName, const DmDeviceInfo &devReqInfo,
    const DmAppImageInfo &imageInfo, std::string &extrasJson)
{
    DMLOG(DM_LOG_INFO, "AuthManager::AuthAppGroup started");
    nlohmann::json jsonObject = nlohmann::json::parse(extrasJson, nullptr, false);
    if (jsonObject.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "extrasJson error");
        return;
    }
    if (!jsonObject.contains(AUTH_TYPE)) {
        DMLOG(DM_LOG_ERROR, "AuthAppGroup extrasJson error");
        return;
    }
    if (!jsonObject.contains(TARGET_PKG_NAME_KEY)) {
        DMLOG(DM_LOG_ERROR, "TARGET_PKG_NAME is not in extrasJson");
        return;
    }
    std::string targetPkgName = jsonObject[TARGET_PKG_NAME_KEY];

    if (!jsonObject.contains(DISPLAY_OWNER)) {
        DMLOG(DM_LOG_WARN, "AuthAppGroup DISPLAY_OWNER error");
        displayOwner_ = DISPLAY_OWNER_SYSTEM;
    } else {
        displayOwner_ = jsonObject[DISPLAY_OWNER];
    }

    if (!CanStartNewSession()) {
        DMLOG(DM_LOG_ERROR, "previous session not completed yet");
        mPendingReqSessionPtr_->NotifyHostAppAuthResult(ERROR_DUPLICATE_REQUEST);
        return;
    }
    auto curSessionPtr = std::make_shared<RequestSession>(hostPkgName, targetPkgName, devReqInfo, imageInfo);
    mPendingReqSessionPtr_ = curSessionPtr;
    std::vector<std::string> msgInfo = curSessionPtr->GetRequestCommand(extrasJson);
    int32_t channelId = SoftbusSession::GetInstance().SendMessages(devReqInfo.deviceId, msgInfo);
    if (channelId > 0) {
        DMLOG(DM_LOG_INFO, "open channel succeed, save channelId");
        mPendingReqSessionPtr_->SetChannelId(channelId);
        return;
    }
    DMLOG(DM_LOG_ERROR, "open channel failed, close this session");
    mPendingReqSessionPtr_->Release();
    mPendingReqSessionPtr_->NotifyHostAppAuthResult(ERROR_START_REMOTE_DM);
    mPendingReqSessionPtr_ = nullptr;
}

bool AuthManager::CanStartNewSession()
{
    if (mPendingReqSessionPtr_ == nullptr || mPendingReqSessionPtr_->IsFinished()) {
        return true;
    }
    return false;
}

void AuthManager::OnUserOperate(int32_t action)
{
    if (action == FaAction::USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT) {
        if (mPendingReqSessionPtr_ == nullptr) {
            DMLOG(DM_LOG_ERROR, "mPendingReqSessionPtr_ not exist");
            return;
        }
        mPendingReqSessionPtr_->OnUserOperate(action);
        return;
    }

    if (mPendingRespSessionPtr == nullptr) {
        DMLOG(DM_LOG_ERROR, "mPendingRespSessionPtr not exist");
        return;
    }

    mPendingRespSessionPtr->OnUserOperate(action);
}

void AuthManager::OnReceiveMessage(long long channelId, std::string &message, int32_t msgType)
{
    DMLOG(DM_LOG_INFO, "AuthManager::OnReceiveMessage started");
    if (msgType == MSG_TYPE_RESP_AUTH) {
        DMLOG(DM_LOG_ERROR, "message type is MSG_TYPE_RESP_AUTH");
        if (mPendingReqSessionPtr_ == nullptr || !mPendingReqSessionPtr_->IsMyChannelId(channelId)) {
            DMLOG(DM_LOG_ERROR, "receive error message");
            return;
        }
        mPendingReqSessionPtr_->OnReceiveMsg(message);
        if (mPendingReqSessionPtr_->IsWaitingForScan()) {
            DMLOG(DM_LOG_ERROR, "move Session To Wait Scan Map ");
            MoveSessionToWaitScanMap();
        }
    } else if (msgType == MSG_TYPE_REQ_AUTH || msgType == MSG_TYPE_AUTH_BY_PIN) {
        DMLOG(DM_LOG_INFO, "message type is MSG_TYPE_REQ_AUTH");
        if (mPendingRespSessionPtr == nullptr) {
            mPendingRespSessionPtr = std::make_shared<ResponseSession>();
        }

        if (!mPendingRespSessionPtr->IsMyChannelId(channelId)) {
            DMLOG(DM_LOG_ERROR, "mPendingRespSessionPtr IsMyChannelId false!");
            return;
        }
        mPendingRespSessionPtr->OnReceiveMsg(channelId, message);
    } else {
        DMLOG(DM_LOG_ERROR, "error message type");
    }
}

void AuthManager::MoveSessionToWaitScanMap()
{
    DMLOG(DM_LOG_ERROR, "AuthManager::MoveSessionToWaitScanMap");
    mWaitScanReqSessionMap_[mPendingReqSessionPtr_->GetToken()] = mPendingReqSessionPtr_;
    mPendingReqSessionPtr_ = nullptr;
}

int32_t AuthManager::CheckAuthentication(std::string &authPara)
{
    DMLOG(DM_LOG_INFO, "AuthManager::CheckAuthentication started");
    nlohmann::json authJson = nlohmann::json::parse(authPara, nullptr, false);
    if (authJson.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "HichainAdapter::GetGroupIdByName parse group info error, json invalid.");
        return AUTH_PARA_INVALID;
    }
    if (!authJson.contains(AUTH_TYPE) || authJson[AUTH_TYPE] != AUTH_TYPE_PIN) {
        DMLOG(DM_LOG_ERROR, "error authPara msg");
        return AUTH_PARA_INVALID;
    }
    DMLOG(DM_LOG_INFO, "PIN Code Auth");
    authParam_ = authPara;
    return CheckAuthenticationByPin(authJson);
}

void AuthManager::NotifyHostOnCheckAuthResult(int64_t requestId, int errorCode)
{
    DMLOG(DM_LOG_INFO, "notify host checkResult, requestId: %lld, errorcode: %d", requestId, errorCode);
    for (auto iter = mWaitScanReqSessionMap_.begin(); iter != mWaitScanReqSessionMap_.end(); iter++) {
        auto requestSessionPtr = iter->second;
        if (requestSessionPtr != nullptr && requestSessionPtr->GetRequestId() == requestId) {
            std::string deviceId = requestSessionPtr->GetRequestDeviceId();
            DMLOG(DM_LOG_INFO, "notify host checkResult, deviceId: %s, requestId: %lld",
                GetAnonyString(deviceId).c_str(), requestId);
            IpcServerListenerAdapter::GetInstance().OnCheckAuthResult(deviceId, errorCode, 0);
            return;
        }
    }

    DMLOG(DM_LOG_ERROR, "notify host checkResult error, requestId: %lld", requestId);
}

int32_t AuthManager::CheckAuthenticationByPin(nlohmann::json &authJson)
{
    int32_t pinCode = authJson.contains(PIN_CODE_KEY) ? (int32_t)authJson[PIN_CODE_KEY] : DEFAULT_PIN_CODE;
    int32_t pinToken = authJson.contains(PIN_TOKEN) ? (int32_t)authJson[PIN_TOKEN] : DEFAULT_PIN_TOKEN;
    if (pinCode < MIN_PIN_CODE || pinCode >= (MIN_PIN_CODE + MAX_PIN_CODE)) {
        DMLOG(DM_LOG_ERROR, "pinCode err, please check pinCode");
        return PIN_CODE_CHECK_FAILED;
    }

    if (pinToken < MIN_PIN_TOKEN || pinToken >= (MIN_PIN_TOKEN + MAX_PIN_TOKEN)) {
        DMLOG(DM_LOG_ERROR, "pinToken err, please check pinToken.");
        return PIN_TOKEN_CHECK_FAILED;
    }
    OnPinInputResult(pinCode, pinToken);
    DMLOG(DM_LOG_INFO, "CheckAuthenticationByPin authPara end");
    return DEVICEMANAGER_OK;
}

void AuthManager::OnPinInputResult(int32_t pinCode, int32_t pinToken)
{
    DMLOG(DM_LOG_ERROR, "AuthManager::OnPinInputResult");
    for (auto iter = mWaitScanReqSessionMap_.begin(); iter != mWaitScanReqSessionMap_.end(); iter++) {
        auto requestSessionPtr = iter->second;
        if (requestSessionPtr != nullptr && requestSessionPtr->IsMyPinToken(pinToken)) {
            DMLOG(DM_LOG_INFO, "AuthManager:: OnPinInputResult");
            requestSessionPtr->OnReceivePinCode(pinCode);
        }
    }
}

int32_t AuthManager::GetAuthenticationParam(DmAuthParam &authParam)
{
    AbilityRole role = DmAbilityManager::GetInstance().GetAbilityRole();
    DMLOG(DM_LOG_INFO, "GetAuthenticationParam:: role = %d", (int32_t)role);
    if (role == AbilityRole::ABILITY_ROLE_INITIATIVE) {
        if (mPendingReqSessionPtr_ == nullptr) {
            DMLOG(DM_LOG_ERROR, "AuthManager:: Get Auth params FAIL : mPendingReqSessionPtr_(nullptr)");
            return FAIL;
        }
        authParam.authType = AUTH_TYPE_PIN; // Currently, only Support PinCode, authType not save.
        authParam.direction = (int32_t)DmAbilityManager::GetInstance().GetAbilityRole();
        authParam.pinToken = mPendingReqSessionPtr_->GetPinToken();
        DMLOG(DM_LOG_INFO, "GetAuthenticationParam, role is ABILITY_ROLE_INITIATIVE");
        return SUCCESS;
    }

    if (mPendingRespSessionPtr == nullptr) {
        DMLOG(DM_LOG_ERROR, "AuthManager:: Get Auth params FAIL : mPendingRespSessionPtr(nullptr)");
        return FAIL;
    }

    mPendingRespSessionPtr->BuildAuthenticationInfo(authParam);
    return SUCCESS;
}

int32_t AuthManager::GetPincode(int64_t requestId)
{
    if (mPendingRespSessionPtr == nullptr) {
        DMLOG(DM_LOG_ERROR, "AuthManager:: GetPincode FAIL : mPendingRespSessionPtr(nullptr)");
        return FAIL;
    }

    return mPendingRespSessionPtr->GetPinCodeByReqId(requestId);
}

std::string AuthManager::GetAuthPara()
{
    return authParam_;
}

int32_t AuthManager::GetDisplayOwner()
{
    return displayOwner_;
}
}
}
