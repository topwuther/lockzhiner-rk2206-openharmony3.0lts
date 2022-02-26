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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_SERVICES_NOTIFICATIONMGR_INCLUDE_NOTIFICATION_SUBSCRIBER_MANAGER_H
#define BASE_NOTIFICATION_ANS_STANDARD_SERVICES_NOTIFICATIONMGR_INCLUDE_NOTIFICATION_SUBSCRIBER_MANAGER_H

#include <list>
#include <memory>
#include <mutex>

#include "errors.h"
#include "event_handler.h"
#include "event_runner.h"
#include "nocopyable.h"
#include "refbase.h"
#include "singleton.h"

#include "ans_subscriber_interface.h"
#include "notification_constant.h"
#include "notification_request.h"
#include "notification_sorting_map.h"
#include "notification_subscribe_info.h"

namespace OHOS {
namespace Notification {
class NotificationSubscriberManager : public DelayedSingleton<NotificationSubscriberManager> {
public:
    ErrCode AddSubscriber(const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &subscribeInfo);
    ErrCode RemoveSubscriber(
        const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &subscribeInfo);

    void NotifyConsumed(const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap);
    void NotifyCanceled(
        const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap, int deleteReason);
    void NotifyUpdated(const sptr<NotificationSortingMap> &notificationMap);
    void NotifyDisturbModeChanged(const NotificationConstant::DisturbMode &mode);
    void OnRemoteDied(const wptr<IRemoteObject> &object);

private:
    struct SubscriberRecord;

    std::shared_ptr<SubscriberRecord> FindSubscriberRecord(const wptr<IRemoteObject> &object);
    std::shared_ptr<SubscriberRecord> FindSubscriberRecord(const sptr<IAnsSubscriber> &subscriber);
    std::shared_ptr<SubscriberRecord> CreateSubscriberRecord(const sptr<IAnsSubscriber> &subscriber);
    void AddRecordInfo(std::shared_ptr<SubscriberRecord> &record, const sptr<NotificationSubscribeInfo> &subscribeInfo);
    void RemoveRecordInfo(
        std::shared_ptr<SubscriberRecord> &record, const sptr<NotificationSubscribeInfo> &subscribeInfo);
    void AddSubscriberInner(
        const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &subscribeInfo);
    void RemoveSubscriberInner(
        const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &subscribeInfo);

    void NotifyConsumedInner(
        const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap);
    void NotifyCanceledInner(
        const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap, int deleteReason);
    void NotifyUpdatedInner(const sptr<NotificationSortingMap> &notificationMap);
    void NotifyDisturbModeChangedInner(const NotificationConstant::DisturbMode &mode);

private:
    std::list<std::shared_ptr<SubscriberRecord>> subscriberRecordList_ {};
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner_ {};
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler_ {};
    sptr<IAnsSubscriber> ansSubscriberProxy_ {};
    sptr<IRemoteObject::DeathRecipient> recipient_ {};

    DECLARE_DELAYED_SINGLETON(NotificationSubscriberManager);
    DISALLOW_COPY_AND_MOVE(NotificationSubscriberManager);
};

}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_SERVICES_NOTIFICATIONMGR_INCLUDE_NOTIFICATION_SUBSCRIBER_MANAGER_H