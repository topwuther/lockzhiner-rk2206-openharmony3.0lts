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

#include "msg_head.h"
#include "device_manager_log.h"
#include "constants.h"

namespace OHOS {
namespace DistributedHardware {
void MsgHead::Encode(nlohmann::json &json)
{
    json[TAG_VER] = DM_ITF_VER;
    json[TAG_TYPE] = mMsgType_;
}

std::shared_ptr<MsgHead> MsgHead::Decode(nlohmann::json &json)
{
    if (json.contains(TAG_TYPE) == false || json.contains(TAG_VER) == false) {
        DMLOG(DM_LOG_ERROR, "err json string");
        return nullptr;
    }

    auto msgHeadPtr = std::make_shared<MsgHead>();
    msgHeadPtr->mItfVer_ = json[TAG_VER];

    if (DM_ITF_VER.compare(msgHeadPtr->mItfVer_) == 0) {
        msgHeadPtr->mMsgType_ = json[TAG_TYPE];
    } else {
        DMLOG(DM_LOG_ERROR, "msg head version mismatch");
        msgHeadPtr->mMsgType_ = DmMsgType::MSG_TYPE_UNKNOWN;
    }
    return msgHeadPtr;
}

int32_t MsgHead::GetMsgType()
{
    return mMsgType_;
}
}
}