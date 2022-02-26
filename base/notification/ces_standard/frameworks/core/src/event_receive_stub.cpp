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

#include "event_receive_stub.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
EventReceiveStub::EventReceiveStub()
{
    EVENT_LOGD("event receive stub instance is created");
}

EventReceiveStub::~EventReceiveStub()
{
    EVENT_LOGD("event receive stub instance is destroyed");
}

int EventReceiveStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(IEventReceive::Message::CES_NOTIFY_COMMON_EVENT): {
            std::unique_ptr<CommonEventData> eventData(data.ReadParcelable<CommonEventData>());
            bool ordered = data.ReadBool();
            bool sticky = data.ReadBool();
            if (eventData == nullptr) {
                EVENT_LOGE("callback stub receive common event data is nullptr");
                return ERR_INVALID_VALUE;
            }
            NotifyEvent(*eventData, ordered, sticky);
            break;
        }

        default:
            EVENT_LOGW("event receive stub receives unknown code, code = %{public}u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return NO_ERROR;
}
}  // namespace EventFwk
}  // namespace OHOS