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

#ifndef OHOS_MSG_HEAD_H
#define OHOS_MSG_HEAD_H

#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
enum DmMsgType : int32_t {
    MSG_TYPE_UNKNOWN = 0,
    MSG_TYPE_REQ_AUTH = 100,
    MSG_TYPE_INVITE_AUTH_INFO = 102,
    MSG_TYPE_RESP_AUTH = 200,
    MSG_TYPE_JOIN_AUTH_INFO = 201,
    MSG_TYPE_CHANNEL_CLOSED = 300,
    MSG_TYPE_SYNC_GROUP = 400,
    MSG_TYPE_AUTH_BY_PIN = 500,
};

class MsgHead {
public:
    MsgHead(): mMsgType_(0), mItfVer_("") {};
    MsgHead(int32_t msg): mMsgType_(msg), mItfVer_("") {};
    ~MsgHead() = default;
    void Encode(nlohmann::json &json);
    static std::shared_ptr<MsgHead> Decode(nlohmann::json &json);
    int32_t GetMsgType();
private:
    int32_t mMsgType_;
    std::string mItfVer_;
};
}
}
#endif