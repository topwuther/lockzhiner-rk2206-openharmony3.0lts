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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_IEVENT_RECEIVE_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_IEVENT_RECEIVE_H

#include "common_event_data.h"
#include "iremote_broker.h"

namespace OHOS {
namespace EventFwk {
class IEventReceive : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.eventfwk.IEventReceive");

    /**
     * Notify event.
     *
     * @param commonEventData  the common event data
     * @param ordered          whether it is an ordered common event
     * @param sticky               whether it is a sticky common event
     *
     */
    virtual void NotifyEvent(const CommonEventData &commonEventData, const bool &ordered, const bool &sticky) = 0;

    enum class Message {
        CES_NOTIFY_COMMON_EVENT = 1,
    };
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_IEVENT_RECEIVE_H
