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

#ifndef OHOS_MSG_REQUEST_AUTH_H
#define OHOS_MSG_REQUEST_AUTH_H

#include <vector>
#include <string>

#include "nlohmann/json.hpp"

#include "msg_head.h"
#include "dm_app_image_info.h"
#include "dm_device_info.h"
#include "constants.h"

namespace OHOS {
namespace DistributedHardware {
class MsgRequestAuth {
public:
    MsgRequestAuth() = default;
    ~MsgRequestAuth() = default;
    MsgRequestAuth(std::string &token, std::string hostPkgName, std::string targetPkgName,
        const DmDeviceInfo &devReqInfo, const DmAppImageInfo &imageInfo, std::string &extras) : MsgRequestAuth(
        token, hostPkgName, targetPkgName, GROUP_VISIBILITY_IS_PRIVATE, devReqInfo, imageInfo, extras) {};
    MsgRequestAuth(std::string &token, std::string hostPkgName, std::string targetPkgName,
        const int32_t groupVisibility, const DmDeviceInfo &devReqInfo, const DmAppImageInfo &imageInfo,
        std::string &extras);
    std::vector<std::string> Encode();
    static std::shared_ptr<MsgRequestAuth> Decode(nlohmann::json &json, std::shared_ptr<MsgRequestAuth> msgIn);
    static void SetThumbnailSize(nlohmann::json &json, std::shared_ptr<MsgRequestAuth> msg);
    int32_t GetMsgSlice();
    int32_t GetMsgCnt();
    std::string GetRequestDeviceId();
public:
    std::shared_ptr<MsgHead> mHead_ {nullptr};
    std::string mHostPkg_;
    std::string mTargetPkg_;
    std::string mDeviceName_;
    std::string mToken_;
    std::string mDeviceId_;
    std::string mDeviceType_;
    std::string mAppName_;
    std::string mAppDescription_;
    int32_t mAuthType_ {AUTH_TYPE_PIN};
    int32_t mGroupVisibility_ {GROUP_VISIBILITY_IS_PRIVATE};
    int32_t mMsgSlice_ {0};
    int32_t mMsgCnt_ {0};
    int32_t mThumbnailSize_ {0};
    int32_t mAppIconSize_ {0};
    DmAppImageInfo mImageInfo_;
private:
    std::string ToHexString(int32_t value);
    std::string EncodeDevInfo();
    static void DecodeDeviceInfo(nlohmann::json &json, std::shared_ptr<MsgRequestAuth> msg);
    int32_t GetEncodedAppInfo(const uint8_t *dataSrc, int32_t srcLen, std::string &outString);
    void GetDecodeAppInfo(const std::string appString, uint8_t **outBuffer, int32_t &outBufferLen);
    static bool IsMsgValid(std::shared_ptr<MsgRequestAuth> msgIn, nlohmann::json &json,  std::string &deviceId,
        int32_t index);
    static bool IsAppInfoValid(nlohmann::json &json);
    static void SetAuthType(nlohmann::json &json, std::shared_ptr<MsgRequestAuth> msg);
};
}
}


#endif
