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

#ifndef OHOS_MSG_RESPONSE_AUTH_H
#define OHOS_MSG_RESPONSE_AUTH_H

#include <vector>
#include <string>

#include "constants.h"
#include "nlohmann/json.hpp"

#include "msg_head.h"

namespace OHOS {
namespace DistributedHardware {
class MsgResponseAuth {
public:
    MsgResponseAuth() = default;
    MsgResponseAuth(int32_t reply, std::string &reqDeviceId);
    MsgResponseAuth(int32_t reply, int64_t requestId, std::string &groupId, std::string &groupName,
        std::string &reqDeviceId);
    ~MsgResponseAuth() = default;
    void Encode(nlohmann::json &jsonObj);
    int32_t Decode(nlohmann::json &jsonObj);
    int32_t GetReply();
    std::string GetNetId();
    std::string GetGroupId();
    std::string GetDeviceId();
    std::string GetGroupName();
    int32_t GetPinCode();
    void SavePinCode(int32_t pinCode);
    int64_t GetRequestId();
    std::vector<std::string> GetSyncGroupList();
private:
    std::shared_ptr<MsgHead> mHead_ {nullptr};
    int32_t mReply_ {SESSION_REPLY_UNKNOWN};
    std::string mNetId_;
    std::string mGroupId_;
    std::string mGroupName_;
    std::string mDeviceId_;
    int32_t mPinCode_ {-1};
    int64_t mRequestId_ {-1};
    std::vector<std::string> mSyncGroupList_;
};
}
}
#endif
