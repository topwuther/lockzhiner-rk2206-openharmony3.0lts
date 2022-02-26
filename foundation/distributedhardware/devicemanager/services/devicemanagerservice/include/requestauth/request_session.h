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

#ifndef OHOS_REQUEST_SESSION_H
#define OHOS_REQUEST_SESSION_H

#include <string>
#include <vector>

#include "single_instance.h"
#include "dm_app_image_info.h"
#include "dm_device_info.h"
#include "msg_response_auth.h"

namespace OHOS {
namespace DistributedHardware {
class RequestSession {
public:
    RequestSession(std::string &hostPkgName, std::string &targetPkgName, const DmDeviceInfo &devReqInfo,
        const DmAppImageInfo &imageInfo);
    ~RequestSession() = default;
    std::vector<std::string> GetRequestCommand(std::string &extrasJson);
    int32_t GetPinToken();
    void SetChannelId(long long channelId);
    void Release();
    bool IsFinished();
    bool IsMyChannelId(long long channelId);
    void OnReceiveMsg(std::string &msg);
    bool IsWaitingForScan();
    std::string GetToken();
    bool IsMyPinToken(int32_t pinToken);
    void OnReceivePinCode(int32_t pinCode);
    void NotifyHostAppAuthResult(int32_t errorCode);
    void OnUserOperate(int32_t action);
    int64_t GetRequestId();
    std::string GetRequestDeviceId();

private:
    int32_t StartFaService();
    std::string GetHostPkgName();
    std::string GetTargetPkgName();
    int32_t GetSessionType();
    void CloseChannel();
    int32_t ParseRespMsg(std::string &msg);
    void SyncDmPrivateGroup(std::vector<std::string> &remoteGroupList);

private:
    enum StatusType : int32_t {
        STATUS_INIT = 0,
        STATUS_WAITING_REPLY = 2,
        STATUS_WATING_SCAN_OR_INPUT = 3,
        STATUS_WAITING_ADD_GROUP = 4,
        STATUS_FINISH = 6,
    };
    int32_t mSessionType_ {SESSION_TYPE_IS_APP_AUTH};
    int32_t mStatus_ {StatusType::STATUS_INIT};
    std::string mHostPkgName_;
    std::string mTargetPkgName;
    std::string mToken_;
    int32_t mPinToken_ {MIN_PIN_TOKEN};
    DmDeviceInfo mDevInfo_;
    DmAppImageInfo mImageInfo_;
    long long mChannelId_ {-1};
    bool mIsChannelOpened_ {false};
    std::string mRemoteDeviceId_;
    std::string mRemoteNetId_;
    std::string mRemoteGroupId_;
    std::string mRemoteGroupName_;
    int64_t mRequestId_ {-1};
    std::shared_ptr<MsgResponseAuth> responseMsgPtr_;
};
}
}
#endif
