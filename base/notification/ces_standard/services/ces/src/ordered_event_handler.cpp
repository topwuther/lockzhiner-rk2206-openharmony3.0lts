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

#include "ordered_event_handler.h"
#include "common_event_control_manager.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
OrderedEventHandler::OrderedEventHandler(
    const std::shared_ptr<EventRunner> &runner, const std::shared_ptr<CommonEventControlManager> &controlManager)
    : EventHandler(runner), controlManager_(controlManager)
{
    EVENT_LOGD("OrderedEventHandler is created");
}

OrderedEventHandler::~OrderedEventHandler()
{
    EVENT_LOGD("OrderedEventHandler is destroyed");
}

void OrderedEventHandler::ProcessEvent(const InnerEvent::Pointer &event)
{
    if (!controlManager_) {
        EVENT_LOGE("CommonEventControlManager ptr is nullptr");
        return;
    }
    switch (event->GetInnerEventId()) {
        case ORDERED_EVENT_START: {
            EVENT_LOGI("ORDERED_EVENT_START");
            controlManager_->ProcessNextOrderedEvent(true);
            break;
        }

        case ORDERED_EVENT_TIMEOUT: {
            EVENT_LOGI("ORDERED_EVENT_TIMEOUT");
            controlManager_->CurrentOrderedEventTimeout(true);
            break;
        }

        default: {
            EVENT_LOGW("the eventId is not supported");
            break;
        }
    }
}
}  // namespace EventFwk
}  // namespace OHOS
