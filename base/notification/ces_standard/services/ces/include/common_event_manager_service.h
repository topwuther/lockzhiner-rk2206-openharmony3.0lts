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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_MANAGER_SERVICE_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_MANAGER_SERVICE_H

#include "common_event_stub.h"
#include "event_handler.h"
#include "inner_common_event_manager.h"
#include "nocopyable.h"
#include "system_ability.h"

namespace OHOS {
namespace EventFwk {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class CommonEventManagerService : public SystemAbility, public CommonEventStub {
public:
    DECLEAR_SYSTEM_ABILITY(CommonEventManagerService);

    CommonEventManagerService();

    virtual ~CommonEventManagerService() override;

    virtual bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener) override;

    virtual bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener, const uid_t &uid) override;

    virtual bool SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscriberInfo, const sptr<IRemoteObject> &commonEventListener) override;

    virtual bool UnsubscribeCommonEvent(const sptr<IRemoteObject> &commonEventListener) override;

    virtual bool GetStickyCommonEvent(const std::string &event, CommonEventData &eventData) override;

    virtual bool DumpState(const std::string &event, std::vector<std::string> &state) override;

    virtual bool FinishReceiver(const sptr<IRemoteObject> &proxy, const int &code, const std::string &receiverData,
        const bool &abortEvent) override;
    virtual bool Freeze(const uid_t &uid) override;
    virtual bool Unfreeze(const uid_t &uid) override;

private:
    ErrCode Init();

    // the function that overrided from SystemAbility
    virtual void OnStart() override;

    virtual void OnStop() override;

    bool IsReady() const;

    bool PublishCommonEventDetailed(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const sptr<IRemoteObject> &commonEventListener, const uid_t &uid, const pid_t &pid);

private:
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager_;
    ServiceRunningState serviceRunningState_ = ServiceRunningState::STATE_NOT_START;
    std::shared_ptr<EventRunner> runner_;
    std::shared_ptr<EventHandler> handler_;

    DISALLOW_COPY_AND_MOVE(CommonEventManagerService);
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_MANAGER_SERVICE_H