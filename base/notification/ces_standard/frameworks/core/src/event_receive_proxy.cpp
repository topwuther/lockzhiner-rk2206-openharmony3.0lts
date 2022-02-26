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

#include "event_receive_proxy.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
EventReceiveProxy::EventReceiveProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IEventReceive>(object)
{
    EVENT_LOGD("event receive proxy instance is created");
}

EventReceiveProxy::~EventReceiveProxy()
{
    EVENT_LOGD("event receive proxy is destoryed");
}

void EventReceiveProxy::NotifyEvent(const CommonEventData &commonEventData, const bool &ordered, const bool &sticky)
{
    EVENT_LOGD("start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        EVENT_LOGE("Failed to send cmd to service due to remote objetc is null");
        return;
    }

    if (!data.WriteParcelable(&commonEventData)) {
        EVENT_LOGE("Failed to write parcelable");
        return;
    }

    if (!data.WriteBool(ordered)) {
        EVENT_LOGE("Failed to write Bool");
        return;
    }

    if (!data.WriteBool(sticky)) {
        EVENT_LOGE("Failed to write Bool");
        return;
    }

    int32_t result = remote->SendRequest(
        static_cast<uint32_t>(IEventReceive::Message::CES_NOTIFY_COMMON_EVENT), data, reply, option);
    if (result != OHOS::NO_ERROR) {
        EVENT_LOGE("Failed to SendRequest, error code: %{public}d", result);
        return;
    }

    EVENT_LOGD("end");
}
}  // namespace EventFwk
}  // namespace OHOS