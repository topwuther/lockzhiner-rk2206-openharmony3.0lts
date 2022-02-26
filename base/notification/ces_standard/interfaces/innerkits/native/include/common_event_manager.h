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

#ifndef FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_MANAGER_H

#include "common_event_data.h"
#include "common_event_publish_info.h"
#include "common_event_subscriber.h"

namespace OHOS {
namespace EventFwk {
class CommonEventManager {
public:
    /**
     * Publish a standard common event.
     *
     * @param data the common event data
     */
    static bool PublishCommonEvent(const CommonEventData &data);

    /**
     * Publish an ordered, sticky, or standard common event.
     *
     * @param data the common event data
     * @param publishInfo the publish info
     */
    static bool PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo);

    /**
     * Publish an ordered, sticky, or standard common event.
     *
     * @param data the common event data
     * @param publishInfo the publish info
     * @param subscriber the common event subscriber
     */
    static bool PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber);

    /**
     * Subscribe to common events.
     *
     * @param subscriber the common event subscriber
     */
    static bool SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber);

    /**
     * Unsubscribe from common events.
     *
     * @param subscriber the common event subscriber
     */
    static bool UnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber);

    /**
     * Gets the current sticky common event
     *
     * @param event the common event
     * @param commonEventData the common event data
     */
    static bool GetStickyCommonEvent(const std::string &event, CommonEventData &commonEventData);
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_MANAGER_H
