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

#include "inner_common_event_manager.h"
#include "bundle_manager_helper.h"
#include "common_event_support.h"
#include "common_event_subscriber_manager.h"
#include "common_event_sticky_manager.h"
#include "event_log_wrapper.h"
#include "system_time.h"

namespace OHOS {
namespace EventFwk {
InnerCommonEventManager::InnerCommonEventManager() : controlPtr_(std::make_shared<CommonEventControlManager>())
{}

bool InnerCommonEventManager::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const sptr<IRemoteObject> &commonEventListener, const struct tm &recordTime, const pid_t &pid, const uid_t &uid,
    const std::string &bundleName)
{
    EVENT_LOGI("enter pid = %{public}d, uid = %{public}d", pid, uid);

    if (data.GetWant().GetAction().empty()) {
        EVENT_LOGE("the commonEventdata action is null");
        return false;
    }

    if ((!publishInfo.IsOrdered()) && (commonEventListener != nullptr)) {
        EVENT_LOGE("When publishing unordered events, the subscriber object is not required.");
        return false;
    }

    std::string action = data.GetWant().GetAction();
    bool isSystemEvent = DelayedSingleton<CommonEventSupport>::GetInstance()->IsSystemEvent(action);
    bool isSystemApp = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckIsSystemAppByUid(uid);
    if (isSystemEvent) {
        EVENT_LOGI("System common event");
        if (!isSystemApp) {
            EVENT_LOGE(
                "No permission to send a system common event from %{public}s(pid = %{public}d, uid = %{public}d)",
                bundleName.c_str(),
                pid,
                uid);
            return false;
        }
    }

    CommonEventRecord eventRecord;
    eventRecord.commonEventData = std::make_shared<CommonEventData>(data);
    eventRecord.publishInfo = std::make_shared<CommonEventPublishInfo>(publishInfo);
    eventRecord.recordTime = recordTime;
    eventRecord.pid = pid;
    eventRecord.uid = uid;
    eventRecord.bundleName = bundleName;
    eventRecord.isSystemEvent = isSystemEvent;

    if (publishInfo.IsSticky()) {
        const std::string permission = "ohos.permission.COMMONEVENT_STICKY";
        bool ret = DelayedSingleton<BundleManagerHelper>::GetInstance()->CheckPermission(bundleName, permission);
        if (!ret) {
            EVENT_LOGE(
                "No permission to send a sticky common event from %{public}s (pid = %{public}d, uid = %{public}d)",
                bundleName.c_str(),
                pid,
                uid);
            return false;
        }
        DelayedSingleton<CommonEventStickyManager>::GetInstance()->UpdateStickyEvent(eventRecord);
    }

    if (!controlPtr_) {
        EVENT_LOGE("CommonEventControlManager ptr is nullptr");
        return false;
    }

    controlPtr_->PublishCommonEvent(eventRecord, commonEventListener);

    return true;
}

bool InnerCommonEventManager::SubscribeCommonEvent(const CommonEventSubscribeInfo &subscriberInfo,
    const sptr<IRemoteObject> &commonEventListener, const struct tm &recordTime, const pid_t &pid, const uid_t &uid,
    const std::string &bundleName)
{
    EVENT_LOGI("enter");

    if (subscriberInfo.GetMatchingSkills().CountEvent() == 0) {
        EVENT_LOGE("the subscriber has no event");
        return false;
    }
    if (commonEventListener == nullptr) {
        EVENT_LOGE("InnerCommonEventManager::SubscribeCommonEvent:commonEventListener == nullptr");
        return false;
    }

    std::shared_ptr<CommonEventSubscribeInfo> sp = std::make_shared<CommonEventSubscribeInfo>(subscriberInfo);
    DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertSubscriber(
        sp, commonEventListener, recordTime, pid, uid, bundleName);

    return true;
};

bool InnerCommonEventManager::UnsubscribeCommonEvent(sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    if (commonEventListener == nullptr) {
        EVENT_LOGE("InnerCommonEventManager::UnsubscribeCommonEvent:commonEventListener == nullptr");
        return false;
    }

    if (!controlPtr_) {
        EVENT_LOGE("CommonEventControlManager ptr is nullptr");
        return false;
    }

    std::shared_ptr<OrderedEventRecord> sp = controlPtr_->GetMatchingOrderedReceiver(commonEventListener);
    if (sp) {
        EVENT_LOGI("Unsubscribe the subscriber who is waiting to receive finish feedback");
        int code = sp->commonEventData->GetCode();
        std::string data = sp->commonEventData->GetData();
        controlPtr_->FinishReceiverAction(sp, code, data, sp->resultAbort);
    }

    DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->RemoveSubscriber(commonEventListener);

    return true;
}

bool InnerCommonEventManager::GetStickyCommonEvent(const std::string &event, CommonEventData &eventData)
{
    EVENT_LOGI("enter");

    return DelayedSingleton<CommonEventStickyManager>::GetInstance()->GetStickyCommonEvent(event, eventData);
}

void InnerCommonEventManager::DumpState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->DumpState(event, state);

    DelayedSingleton<CommonEventStickyManager>::GetInstance()->DumpState(event, state);

    if (!controlPtr_) {
        EVENT_LOGE("CommonEventControlManager ptr is nullptr");
        return;
    }
    controlPtr_->DumpState(event, state);

    controlPtr_->DumpHistoryState(event, state);
    return;
}

void InnerCommonEventManager::FinishReceiver(
    const sptr<IRemoteObject> &proxy, const int &code, const std::string &receiverData, const bool &abortEvent)
{
    EVENT_LOGI("enter");

    if (!controlPtr_) {
        EVENT_LOGE("CommonEventControlManager ptr is nullptr");
        return;
    }

    std::shared_ptr<OrderedEventRecord> sp = controlPtr_->GetMatchingOrderedReceiver(proxy);
    if (sp) {
        controlPtr_->FinishReceiverAction(sp, code, receiverData, abortEvent);
    }

    return;
}

bool InnerCommonEventManager::Freeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->UpdateFreezeInfo(
        uid, true, SystemTime::GetNowSysTime());

    return true;
}

bool InnerCommonEventManager::Unfreeze(const uid_t &uid)
{
    EVENT_LOGI("enter");

    DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->UpdateFreezeInfo(uid, false);

    return controlPtr_->PublishFreezeCommonEvent(uid);
}

}  // namespace EventFwk
}  // namespace OHOS
