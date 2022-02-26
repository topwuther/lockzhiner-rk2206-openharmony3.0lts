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

#ifndef FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_STUB_H
#define FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_STUB_H

#include "icommon_event.h"
#include "iremote_stub.h"
#include "nocopyable.h"

namespace OHOS {
namespace EventFwk {
class CommonEventStub : public IRemoteStub<ICommonEvent> {
public:
    CommonEventStub();
    virtual ~CommonEventStub() override;

    DECLARE_INTERFACE_DESCRIPTOR(u"ICommonEvent");

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    virtual bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener) override;
    virtual bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener, const uid_t &uid) override;
    virtual bool SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscriberInfo, const sptr<IRemoteObject> &commonEventListener) override;
    virtual bool UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener) override;
    virtual bool GetStickyCommonEvent(const std::string &event, CommonEventData &eventData) override;
    virtual bool DumpState(const std::string &event, std::vector<std::string> &state) override;
    virtual bool Freeze(const uid_t &uid) override;
    virtual bool Unfreeze(const uid_t &uid) override;

private:
    DISALLOW_COPY_AND_MOVE(CommonEventStub);
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_INNERKITS_INCLUDE_COMMON_EVENT_STUB_H
