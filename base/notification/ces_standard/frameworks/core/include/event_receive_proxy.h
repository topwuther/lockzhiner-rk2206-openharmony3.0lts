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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_EVENT_RECEIVE_PROXY_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_EVENT_RECEIVE_PROXY_H

#include "common_event_data.h"
#include "ievent_receive.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace EventFwk {
class EventReceiveProxy : public IRemoteProxy<IEventReceive> {
public:
    explicit EventReceiveProxy(const sptr<IRemoteObject> &object);
    virtual ~EventReceiveProxy() override;
    virtual void NotifyEvent(const CommonEventData &commonEventData, const bool &ordered, const bool &sticky) override;

private:
    static inline BrokerDelegator<EventReceiveProxy> delegator_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_EVENT_RECEIVE_PROXY_H