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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_INNER_COMMON_EVENT_MANAGER_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_INNER_COMMON_EVENT_MANAGER_H

#include "common_event_control_manager.h"
#include "icommon_event.h"

namespace OHOS {
namespace EventFwk {
class InnerCommonEventManager {
public:
    InnerCommonEventManager();

    virtual ~InnerCommonEventManager(){};

    bool PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener, const struct tm &recordTime, const pid_t &pid, const uid_t &uid,
        const std::string &bundleName);

    bool SubscribeCommonEvent(const CommonEventSubscribeInfo &subscriberInfo,
        const sptr<IRemoteObject> &commonEventListener, const struct tm &recordTime, const pid_t &pid, const uid_t &uid,
        const std::string &bundleName);

    bool UnsubscribeCommonEvent(sptr<IRemoteObject> &commonEventListener);

    bool GetStickyCommonEvent(const std::string &event, CommonEventData &eventData);

    void DumpState(const std::string &event, std::vector<std::string> &state);

    void FinishReceiver(
        const sptr<IRemoteObject> &proxy, const int &code, const std::string &receiverData, const bool &abortEvent);

    bool Freeze(const uid_t &uid);

    bool Unfreeze(const uid_t &uid);

private:
    int64_t GetNowSysTime();

private:
    std::shared_ptr<CommonEventControlManager> controlPtr_;
    DISALLOW_COPY_AND_MOVE(InnerCommonEventManager);
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_INNER_COMMON_EVENT_MANAGER_H
