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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_ICOMMON_EVENT_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_ICOMMON_EVENT_H

#include "common_event_data.h"
#include "common_event_publish_info.h"
#include "common_event_subscribe_info.h"
#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace EventFwk {
class ICommonEvent : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.eventfwk.ICommonEvent");

    virtual bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener) = 0;
    virtual bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener, const uid_t &uid) = 0;
    virtual bool SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscriberInfo, const sptr<IRemoteObject> &commonEventListener) = 0;
    virtual bool UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener) = 0;
    virtual bool GetStickyCommonEvent(const std::string &event, CommonEventData &eventData) = 0;
    virtual bool DumpState(const std::string &event, std::vector<std::string> &state) = 0;
    virtual bool FinishReceiver(
        const sptr<IRemoteObject> &proxy, const int &code, const std::string &receiverData, const bool &abortEvent) = 0;
    virtual bool Freeze(const uid_t &uid) = 0;
    virtual bool Unfreeze(const uid_t &uid) = 0;

    enum class Message {
        CES_PUBLISH_COMMON_EVENT = 0,
        CES_PUBLISH_COMMON_EVENT2,
        CES_SUBSCRIBE_COMMON_EVENT,
        CES_UNSUBSCRIBE_COMMON_EVENT,
        CES_GET_STICKY_COMMON_EVENT,
        CES_FINISH_RECEIVER,
        CES_FREEZE,
        CES_UNFREEZE,
        // ipc id 2001-3000 for tools
        // ipc id for dumping Subscribe State (2001)
        CES_DUMP_STATE = 2001,
    };
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_ICOMMON_EVENT_H
