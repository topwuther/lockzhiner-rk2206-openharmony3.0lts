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

#include "common_event.h"
#include "common_event_death_recipient.h"
#include "event_log_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace EventFwk {
bool CommonEvent::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    EVENT_LOGI("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        return false;
    }
    return commonEventProxy_->PublishCommonEvent(data, publishInfo, commonEventListener);
}

bool CommonEvent::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid)
{
    EVENT_LOGI("enter");
    sptr<IRemoteObject> commonEventListener = nullptr;
    if (!PublishParameterCheck(data, publishInfo, subscriber, commonEventListener)) {
        return false;
    }
    return commonEventProxy_->PublishCommonEvent(data, publishInfo, commonEventListener, uid);
}

bool CommonEvent::PublishParameterCheck(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");
    if (data.GetWant().GetAction() == std::string()) {
        EVENT_LOGE("the commonEventdata action is null");
        return false;
    }

    if ((publishInfo.IsOrdered() == false) && (subscriber != nullptr)) {
        EVENT_LOGE("When publishing unordered events, the subscriber object is not required.");
        return false;
    }

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy_ is null");
        return false;
    }

    if (subscriber) {
        if (CreateCommonEventListener(subscriber, commonEventListener) == SUBSCRIBE_FAILD) {
            return false;
        }
    }

    return true;
}

bool CommonEvent::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    EVENT_LOGI("enter");

    if (subscriber == nullptr) {
        EVENT_LOGE("the subscriber is null");
        return false;
    }

    if (subscriber->GetSubscribeInfo().GetMatchingSkills().CountEvent() == 0) {
        EVENT_LOGE("the subscriber has no event");
        return false;
    }

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy_ is null");
        return false;
    }

    sptr<IRemoteObject> commonEventListener = nullptr;
    int subscribeState_ = CreateCommonEventListener(subscriber, commonEventListener);
    if (subscribeState_ == INITIAL_SUBSCRIPTION) {
        return commonEventProxy_->SubscribeCommonEvent(subscriber->GetSubscribeInfo(), commonEventListener);
    } else if (subscribeState_ == ALREADY_SUBSCRIBED) {
        return true;
    } else {
        return false;
    }
}

bool CommonEvent::UnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    EVENT_LOGI("enter");

    if (subscriber == nullptr) {
        EVENT_LOGE("the subscriber is null");
        return false;
    }

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }

    std::lock_guard<std::mutex> lock(eventListenersMutex_);
    auto eventListener = eventListeners_.find(subscriber);
    if (eventListener != eventListeners_.end()) {
        if (commonEventProxy_->UnsubscribeCommonEvent(eventListener->second->AsObject())) {
            eventListener->second->Stop();
            eventListeners_.erase(eventListener);
            return true;
        }
        return false;
    } else {
        EVENT_LOGW("No specified subscriber has been registered");
    }

    return true;
}

bool CommonEvent::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGI("enter");

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }

    if (event.empty()) {
        EVENT_LOGE("event is empty");
        return false;
    }

    return commonEventProxy_->GetStickyCommonEvent(event, eventData);
}

bool CommonEvent::FinishReceiver(
    const sptr<IRemoteObject> &proxy, const int &code, const std::string &data, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (proxy == nullptr) {
        EVENT_LOGE("the proxy is null");
        return false;
    }

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }

    return commonEventProxy_->FinishReceiver(proxy, code, data, abortEvent);
}

bool CommonEvent::DumpState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }

    return commonEventProxy_->DumpState(event, state);
}

void CommonEvent::ResetCommonEventProxy()
{
    EVENT_LOGI("enter");

    std::lock_guard<std::mutex> lock(mutex_);
    if ((commonEventProxy_ != nullptr) && (commonEventProxy_->AsObject() != nullptr)) {
        commonEventProxy_->AsObject()->RemoveDeathRecipient(recipient_);
    }
    commonEventProxy_ = nullptr;
}

bool CommonEvent::Freeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }

    return commonEventProxy_->Freeze(uid);
}

bool CommonEvent::Unfreeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    if (!GetCommonEventProxy()) {
        EVENT_LOGE("the commonEventProxy is null");
        return false;
    }

    return commonEventProxy_->Unfreeze(uid);
}

bool CommonEvent::GetCommonEventProxy()
{
    EVENT_LOGI("enter");

    if (!commonEventProxy_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!commonEventProxy_) {
            sptr<ISystemAbilityManager> systemAbilityManager =
                SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (!systemAbilityManager) {
                EVENT_LOGE("Failed to get system ability mgr.");
                return false;
            }

            sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(COMMON_EVENT_SERVICE_ID);
            if (!remoteObject) {
                EVENT_LOGE("Failed to get COMMON Event Manager.");
                return false;
            }

            commonEventProxy_ = iface_cast<ICommonEvent>(remoteObject);
            if ((!commonEventProxy_) || (!commonEventProxy_->AsObject())) {
                EVENT_LOGE("Failed to get COMMON Event Manager's proxy");
                return false;
            }

            recipient_ = new CommonEventDeathRecipient();
            if (!recipient_) {
                EVENT_LOGE("Failed to create death Recipient ptr CommonEventDeathRecipient!");
                return false;
            }
            commonEventProxy_->AsObject()->AddDeathRecipient(recipient_);
        }
    }

    return true;
}

int CommonEvent::CreateCommonEventListener(
    const std::shared_ptr<CommonEventSubscriber> &subscriber, sptr<IRemoteObject> &commonEventListener)
{
    if (subscriber == nullptr) {
        EVENT_LOGE("subscriber is null");
        return SUBSCRIBE_FAILD;
    }

    std::lock_guard<std::mutex> lock(eventListenersMutex_);

    auto eventListener = eventListeners_.find(subscriber);
    if (eventListener != eventListeners_.end()) {
        commonEventListener = eventListener->second->AsObject();
        EVENT_LOGW("subscriber has common event listener");
        return ALREADY_SUBSCRIBED;
    } else {

        if (eventListeners_.size() == SUBSCRIBER_MAX_SIZE) {
            EVENT_LOGE("the maximum number of subscriptions has been reached");
            return SUBSCRIBE_FAILD;
        }

        sptr<CommonEventListener> listener = new CommonEventListener(subscriber);
        if (!listener) {
            EVENT_LOGE("the common event listener is null");
            return SUBSCRIBE_FAILD;
        }
        commonEventListener = listener->AsObject();
        eventListeners_[subscriber] = listener;
    }

    return INITIAL_SUBSCRIPTION;
}
}  // namespace EventFwk
}  // namespace OHOS