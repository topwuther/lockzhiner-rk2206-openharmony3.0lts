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

#include "msg_codec.h"
#include "device_manager_log.h"
#include "msg_head.h"

#include "constants.h"
#include "device_manager_log.h"
#include "device_manager_errno.h"

namespace OHOS {
namespace DistributedHardware {
int32_t MsgCodec::DecodeMsgType(std::string &jsonStr)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "DecodeMsgType jsonStr error");
        return DmMsgType::MSG_TYPE_UNKNOWN;
    }
    MsgHead mMsgHead;
    auto msgHeadPtr = mMsgHead.Decode(jsonObject);
    if (msgHeadPtr == nullptr) {
        DMLOG(DM_LOG_ERROR, "mMsgHead decode error");
        return DEVICEMANAGER_NULLPTR;
    }
    return msgHeadPtr->GetMsgType();
}

std::string MsgCodec::EncodeSyncGroup(std::vector<std::string> &groupIdList, std::string &deviceId)
{
    nlohmann::json jsonObject;
    MsgSyncGroup mMsgSyncGroup(groupIdList, deviceId);
    mMsgSyncGroup.Encode(jsonObject);
    return jsonObject.dump();
}

std::vector<std::string> MsgCodec::EncodeReqAppAuth(std::string &token, std::string hostPkg, std::string targetPkg,
    const DmDeviceInfo &devReqInfo, const DmAppImageInfo &imageInfo, std::string &extras)
{
    MsgRequestAuth mMsgRequestAuth(token, hostPkg, targetPkg, devReqInfo, imageInfo, extras);
    return mMsgRequestAuth.Encode();
}

std::string MsgCodec::EncodeAcceptRespAuth(int32_t reply, int64_t requestId, std::string &groupId,
    std::string &groupName, std::string &reqDeviceId)
{
    nlohmann::json jsonObject;
    MsgResponseAuth mMsgResponseAuth(reply, requestId, groupId, groupName, reqDeviceId);
    mMsgResponseAuth.Encode(jsonObject);
    return jsonObject.dump();
}

std::string MsgCodec::EncodeRefuseRespAuth(int32_t reply, std::string &reqDeviceId)
{
    nlohmann::json jsonObject;
    MsgResponseAuth mMsgResponseAuth(reply, reqDeviceId);
    mMsgResponseAuth.Encode(jsonObject);
    return jsonObject.dump();
}

std::shared_ptr<MsgRequestAuth> MsgCodec::DecodeRequestAuth(std::string &jsonStr,
    std::shared_ptr<MsgRequestAuth> msgRequestAuth)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "DecodeRequestAuth jsonStr error");
        return nullptr;
    }
    return MsgRequestAuth::Decode(jsonObject, msgRequestAuth);
}

std::shared_ptr<MsgResponseAuth> MsgCodec::DecodeResponseAuth(std::string &jsonStr)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "DecodeResponseAuth jsonStr error");
        return nullptr;
    }
    std::shared_ptr<MsgResponseAuth> msgResponseAuthPtr = std::make_shared<MsgResponseAuth>();
    if (msgResponseAuthPtr->Decode(jsonObject) == 0) {
        return msgResponseAuthPtr;
    }
    return nullptr;
}
}
}
