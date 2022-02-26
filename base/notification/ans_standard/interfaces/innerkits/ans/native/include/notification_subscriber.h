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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SUBSCRIBER_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SUBSCRIBER_H

#include <memory>

#include "ans_manager_interface.h"
#include "ans_subscriber_stub.h"
#include "notification_constant.h"
#include "notification_request.h"
#include "notification_sorting.h"
#include "notification_sorting_map.h"

namespace OHOS {
namespace Notification {

class NotificationSubscriber {
public:
    /**
     * @brief Default constructor used to create a instance.
     */
    NotificationSubscriber();

    /**
     * @brief Default destructor.
     */
    virtual ~NotificationSubscriber();

    /**
     * @brief Called back when a notification is canceled.
     *
     * @param request Indicates the canceled Notification object.
     **/
    virtual void OnCanceled(const std::shared_ptr<Notification> &request) = 0;

    /**
     * @brief Called back when a notification is canceled.
     *
     * @param request Indicates the canceled Notification object.
     * @param sortingMap Indicates the sorting map used by the current subscriber
     * to obtain notification ranking information.
     * @param deleteReason Indicates the reason for the deletion. For details, see NotificationConstant.
     **/
    virtual void OnCanceled(const std::shared_ptr<Notification> &request,
        const std::shared_ptr<NotificationSortingMap> &sortingMap, int deleteReason) = 0;

    /**
     * @brief Called back when the subscriber is connected to the Advanced Notification Service (ANS).
     *
     * @param result Indicates subscribe result.
     **/
    virtual void OnSubscribeResult(NotificationConstant::SubscribeResult result) = 0;

    /**
     * @brief Called back when the subscriber receives a new notification.
     *
     * @param request Indicates the received Notification object.
     **/
    virtual void OnConsumed(const std::shared_ptr<Notification> &request) = 0;

    /**
     * @brief Called back when the subscriber receives a new notification.
     *
     * @param request Indicates the received Notification object.
     * @param sortingMap Indicates the sorting map used by the current subscriber to obtain
     * notification ranking information.
     **/
    virtual void OnConsumed(
        const std::shared_ptr<Notification> &request, const std::shared_ptr<NotificationSortingMap> &sortingMap) = 0;

    /**
     * @brief Called back when the subscriber is disconnected from the ANS.
     *
     * @param result Indicates unsubscribe result.
     **/
    virtual void OnUnsubscribeResult(NotificationConstant::SubscribeResult result) = 0;

    /**
     * @brief Called back when the ranking information about the current notification changes.
     *
     * @param sortingMap Indicates the sorting map used to obtain notification ranking information.
     **/
    virtual void OnUpdate(const std::shared_ptr<NotificationSortingMap> &sortingMap) = 0;

    /**
     * @brief Called back when connection to the ANS has died.
     **/
    virtual void OnDied() = 0;

    /**
     * @brief Called when the Do Not Disturb mode type changes.
     *
     * @param disturbMode Indicates the current Do Not Disturb mode type.
     **/
    virtual void OnDisturbModeChanged(int disturbMode) = 0;

private:
    class SubscriberImpl final : public AnsSubscriberStub {
    public:
        class DeathRecipient final : public IRemoteObject::DeathRecipient {
        public:
            DeathRecipient(SubscriberImpl &subscriberImpl);

            ~DeathRecipient();

            void OnRemoteDied(const wptr<IRemoteObject> &object) override;

        private:
            SubscriberImpl &subscriberImpl_;
        };

    public:
        SubscriberImpl(NotificationSubscriber &subscriber);
        ~SubscriberImpl(){};

        void OnSubscribeResult(NotificationConstant::SubscribeResult result) override;

        void OnUnsubscribeResult(NotificationConstant::SubscribeResult result) override;

        void OnConsumed(const sptr<Notification> &notification) override;

        void OnConsumed(
            const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap) override;

        void OnCanceled(const sptr<Notification> &notification) override;

        void OnCanceled(const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap,
            int deleteReason) override;

        void OnUpdated(const sptr<NotificationSortingMap> &notificationMap) override;

        void OnDisturbModeChanged(NotificationConstant::DisturbMode mode) override;

        bool GetAnsManagerProxy();

    public:
        NotificationSubscriber &subscriber_;
        sptr<DeathRecipient> recipient_ {nullptr};
        sptr<IAnsManager> proxy_ {nullptr};
        std::mutex mutex_ {};
    };

private:
    const sptr<SubscriberImpl> GetImpl() const;

private:
    sptr<SubscriberImpl> impl_ = nullptr;

    friend class AnsNotification;
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_INCLUDE_NOTIFICATION_SUBSCRIBER_H