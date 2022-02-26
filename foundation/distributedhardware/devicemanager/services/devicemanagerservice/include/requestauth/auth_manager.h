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

#ifndef OHOS_AUTH_MANAGER_H
#define OHOS_AUTH_MANAGER_H

#include <map>
#include <string>

#include "nlohmann/json.hpp"

#include "single_instance.h"
#include "dm_device_info.h"
#include "dm_app_image_info.h"
#include "request_session.h"
#include "response_session.h"

namespace OHOS {
namespace DistributedHardware {
class AuthManager {
    DECLARE_SINGLE_INSTANCE(AuthManager);
public:
    void AuthDeviceGroup(std::string &hostPkgName, const DmDeviceInfo &devReqInfo, const DmAppImageInfo &imageInfo,
        std::string &extras);
    void OnReceiveMsg(long long channelId, std::string &message);
    int32_t CheckAuthentication(std::string &authPara);
    int32_t GetAuthenticationParam(DmAuthParam &authParam);
    void OnUserOperate(int32_t action);
    int32_t GetPincode(int64_t requestId);
    std::string GetAuthPara();
    int32_t GetDisplayOwner();
    void NotifyHostOnCheckAuthResult(int64_t requestId, int errorCode);

private:
    void SyncDmPrivateGroup(std::string &message);
    void AuthAppGroup(std::string &hostPkgName, const DmDeviceInfo &devReqInfo, const DmAppImageInfo &imageInfo,
        std::string &extrasJson);
    void OnReceiveMessage(long long channelId, std::string &message, int32_t msgType);
    bool CanStartNewSession();
    void MoveSessionToWaitScanMap();
    int32_t CheckAuthenticationByPin(nlohmann::json &authJson);
    void OnPinInputResult(int32_t pinCode, int32_t pinToken);

private:
    std::string authParam_ = "";
    int32_t displayOwner_ = 0;
    std::shared_ptr<RequestSession> mPendingReqSessionPtr_ {nullptr};
    std::shared_ptr<ResponseSession> mPendingRespSessionPtr {};
    std::map<std::string, std::shared_ptr<RequestSession>> mWaitScanReqSessionMap_ = {};
};
}
}
#endif
