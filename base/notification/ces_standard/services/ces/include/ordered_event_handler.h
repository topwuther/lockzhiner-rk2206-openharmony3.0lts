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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_ORDERED_EVENT_HANDLER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_ORDERED_EVENT_HANDLER_H

#include "event_handler.h"

namespace OHOS {
namespace EventFwk {
using EventHandler = OHOS::AppExecFwk::EventHandler;
using EventRunner = OHOS::AppExecFwk::EventRunner;
using InnerEvent = OHOS::AppExecFwk::InnerEvent;

class CommonEventControlManager;

class OrderedEventHandler : public EventHandler {
public:
    OrderedEventHandler(
        const std::shared_ptr<EventRunner> &runner, const std::shared_ptr<CommonEventControlManager> &controlManager);

    virtual ~OrderedEventHandler() override;

    virtual void ProcessEvent(const InnerEvent::Pointer &event) override;

    enum {
        ORDERED_EVENT_START = 1,
        ORDERED_EVENT_TIMEOUT,
    };

private:
    std::shared_ptr<CommonEventControlManager> controlManager_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_ORDERED_EVENT_HANDLER_H