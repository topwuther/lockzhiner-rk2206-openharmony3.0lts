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

#ifndef OHOS_MSG_CODEC_H
#define OHOS_MSG_CODEC_H

#include <vector>
#include <string>

#include "nlohmann/json.hpp"

#include "msg_request_auth.h"
#include "msg_sync_group.h"
#include "msg_response_auth.h"

namespace OHOS {
namespace DistributedHardware {
class MsgCodec {
public:
    static int32_t DecodeMsgType(std::string &jsonStr);
    static std::string EncodeSyncGroup(std::vector<std::string> &groupIdList, std::string &deviceId);
    static std::vector<std::string> EncodeReqAppAuth(std::string &token, std::string hostPkg, std::string targetPkg,
        const DmDeviceInfo &devReqInfo, const DmAppImageInfo &imageInfo, std::string &extras);
    static std::string EncodeAcceptRespAuth(int32_t reply, int64_t requestId, std::string &groupId,
        std::string &groupName, std::string &reqDeviceId);
    static std::string EncodeRefuseRespAuth(int32_t reply, std::string &reqDeviceId);
    static std::shared_ptr<MsgRequestAuth> DecodeRequestAuth(std::string &jsonStr,
        std::shared_ptr<MsgRequestAuth> msgRequestAuth);
    static std::shared_ptr<MsgResponseAuth> DecodeResponseAuth(std::string &jsonStr);
};
}
}
#endif
