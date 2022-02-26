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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_SYSTEM_EVENT_SUBSCRIBER_H
#define BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_SYSTEM_EVENT_SUBSCRIBER_H

#include <functional>

#include "common_event_subscriber.h"

namespace OHOS {
namespace Notification {

class SystemEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    SystemEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        const std::function<void(const EventFwk::CommonEventData &)> &callback)
        : EventFwk::CommonEventSubscriber(subscribeInfo), callback_(callback)
    {}

    ~SystemEventSubscriber()
    {}

    void OnReceiveEvent(const EventFwk::CommonEventData &data) override
    {
        if (callback_ != nullptr) {
            callback_(data);
        }
    }

private:
    std::function<void(const EventFwk::CommonEventData &)> callback_;
};

}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_SYSTEM_EVENT_SUBSCRIBER_H