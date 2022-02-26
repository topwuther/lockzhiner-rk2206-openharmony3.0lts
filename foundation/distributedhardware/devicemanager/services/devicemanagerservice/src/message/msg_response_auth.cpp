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

#include "msg_response_auth.h"
#include "device_manager_log.h"
#include "device_manager_errno.h"
#include "constants.h"
#include "parameter.h"
#include "hichain_connector.h"
#include "anonymous_string.h"

namespace OHOS {
namespace DistributedHardware {
MsgResponseAuth::MsgResponseAuth(int32_t reply, std::string &reqDeviceId)
{
    mHead_ = std::make_shared<MsgHead>(MSG_TYPE_RESP_AUTH);
    mReply_ = reply;
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    mDeviceId_ = localDeviceId;
    std::vector<GroupInfo> groupList;
    HichainConnector::GetInstance().GetRelatedGroups(reqDeviceId, groupList);
    HichainConnector::GetInstance().GetSyncGroupList(groupList, mSyncGroupList_);
}

MsgResponseAuth::MsgResponseAuth(int32_t reply, int64_t requestId, std::string &groupId, std::string &groupName,
    std::string &reqDeviceId)
{
    mHead_ = std::make_shared<MsgHead>(MSG_TYPE_RESP_AUTH);
    mReply_ = reply;
    mNetId_ = "";
    mGroupId_ = groupId;
    mGroupName_ = groupName;
    mRequestId_ = requestId;
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    mDeviceId_ = localDeviceId;
    std::vector<GroupInfo> groupList;
    HichainConnector::GetInstance().GetRelatedGroups(reqDeviceId, groupList);
    HichainConnector::GetInstance().GetSyncGroupList(groupList, mSyncGroupList_);
}

void MsgResponseAuth::Encode(nlohmann::json &jsonObj)
{
    DMLOG(DM_LOG_INFO, "MsgResponseAuth encode started");
    mHead_->Encode(jsonObj);
    jsonObj[TAG_REPLY] = mReply_;
    jsonObj[TAG_DEVICE_ID] = mDeviceId_;
    jsonObj[TAG_GROUPIDS] = mSyncGroupList_;
    if (mReply_ == SESSION_REPLY_ACCEPT) {
        jsonObj[TAG_NET_ID] = mNetId_;
        jsonObj[TAG_REQUEST_ID] = mRequestId_;
        jsonObj[TAG_GROUP_ID] = mGroupId_;
        jsonObj[TAG_GROUP_NAME] = mGroupName_;
        DMLOG(DM_LOG_INFO, "MsgResponseAuth encode completed");
    }
    DMLOG(DM_LOG_INFO, "encode mReply_ is : %d", mReply_);
    DMLOG(DM_LOG_INFO, "encode deviceId is : %s", GetAnonyString(mDeviceId_).c_str());
    DMLOG(DM_LOG_INFO, "encode netId is : %s", GetAnonyString(mNetId_).c_str());
    DMLOG(DM_LOG_INFO, "encode mGroupId_ is %s", GetAnonyString(mGroupId_).c_str());
    DMLOG(DM_LOG_INFO, "encode mGroupName_ is %s", mGroupName_.c_str());
    DMLOG(DM_LOG_INFO, "encode mRequestId_ is %d", mRequestId_);
}

int32_t MsgResponseAuth::Decode(nlohmann::json &jsonObj)
{
    DMLOG(DM_LOG_INFO, "MsgResponseAuth decode started");
    if (!jsonObj.contains(TAG_REPLY) || !jsonObj.contains(TAG_DEVICE_ID) || !jsonObj.contains(TAG_GROUPIDS)) {
        DMLOG(DM_LOG_ERROR, "MsgResponseAuth::decode, err json string, first time");
        return MSG_DECODE_PARA_FAILED;
    }

    MsgHead msgHead;
    mHead_ = msgHead.Decode(jsonObj);
    mReply_ = jsonObj[TAG_REPLY];
    mDeviceId_ = jsonObj[TAG_DEVICE_ID];
    std::vector<std::string> groupList = jsonObj[TAG_GROUPIDS];
    for (auto str : groupList) {
        mSyncGroupList_.push_back(str);
    }

    if (mReply_ == SESSION_REPLY_ACCEPT) {
        if (!jsonObj.contains(TAG_NET_ID) || !jsonObj.contains(TAG_GROUP_ID) || !jsonObj.contains(TAG_GROUP_NAME) ||
            !jsonObj.contains(TAG_REQUEST_ID)) {
            DMLOG(DM_LOG_ERROR, "MsgResponseAuth::decode, err json string, second time");
            return MSG_DECODE_PARA_FAILED;
        }
        mNetId_ = jsonObj[TAG_NET_ID];
        mGroupId_ = jsonObj[TAG_GROUP_ID];
        mGroupName_ = jsonObj[TAG_GROUP_NAME];
        mRequestId_ = jsonObj[TAG_REQUEST_ID];
    }
    DMLOG(DM_LOG_INFO, "decode mReply_ is : %d", mReply_);
    DMLOG(DM_LOG_INFO, "decode mGroupId_ is %s", GetAnonyString(mGroupId_).c_str());
    DMLOG(DM_LOG_INFO, "decode mGroupName_ is %s", GetAnonyString(mGroupName_).c_str());
    DMLOG(DM_LOG_INFO, "decode deviceId is : %s", GetAnonyString(mDeviceId_).c_str());
    DMLOG(DM_LOG_INFO, "decode netId is : %s", GetAnonyString(mNetId_).c_str());
    DMLOG(DM_LOG_INFO, "decode mRequestId_ is %d", mRequestId_);
    DMLOG(DM_LOG_INFO, "MsgResponseAuth decode completed");
    return DEVICEMANAGER_OK;
}

int32_t MsgResponseAuth::GetReply()
{
    return mReply_;
}

std::string MsgResponseAuth::GetNetId()
{
    return mNetId_;
}

std::string MsgResponseAuth::GetGroupId()
{
    return mGroupId_;
}

std::string MsgResponseAuth::GetDeviceId()
{
    return mDeviceId_;
}

std::string MsgResponseAuth::GetGroupName()
{
    return mGroupName_;
}

int64_t MsgResponseAuth::GetRequestId()
{
    return mRequestId_;
}

std::vector<std::string> MsgResponseAuth::GetSyncGroupList()
{
    return mSyncGroupList_;
}

int32_t MsgResponseAuth::GetPinCode()
{
    return mPinCode_;
}

void MsgResponseAuth::SavePinCode(int32_t pinCode)
{
    mPinCode_ = pinCode;
}
}
}
