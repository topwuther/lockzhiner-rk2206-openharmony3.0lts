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

#include "notification_helper.h"
#include "ans_notification.h"
#include "singleton.h"

namespace OHOS {
namespace Notification {
ErrCode NotificationHelper::AddNotificationSlot(const NotificationSlot &slot)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->AddNotificationSlot(slot);
}

ErrCode NotificationHelper::AddSlotByType(const NotificationConstant::SlotType &slotType)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->AddSlotByType(slotType);
}

ErrCode NotificationHelper::AddNotificationSlots(const std::vector<NotificationSlot> &slots)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->AddNotificationSlots(slots);
}

ErrCode NotificationHelper::RemoveNotificationSlot(const NotificationConstant::SlotType &slotType)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->RemoveNotificationSlot(slotType);
}

ErrCode NotificationHelper::RemoveAllSlots()
{
    return DelayedSingleton<AnsNotification>::GetInstance()->RemoveAllSlots();
}

ErrCode NotificationHelper::GetNotificationSlot(
    const NotificationConstant::SlotType &slotType, sptr<NotificationSlot> &slot)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetNotificationSlot(slotType, slot);
}

ErrCode NotificationHelper::GetNotificationSlots(std::vector<sptr<NotificationSlot>> &slots)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetNotificationSlots(slots);
}

ErrCode NotificationHelper::AddNotificationSlotGroup(const NotificationSlotGroup &slotGroup)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->AddNotificationSlotGroup(slotGroup);
}

ErrCode NotificationHelper::AddNotificationSlotGroups(const std::vector<NotificationSlotGroup> &slotGroups)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->AddNotificationSlotGroups(slotGroups);
}

ErrCode NotificationHelper::RemoveNotificationSlotGroup(const std::string &slotGroupId)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->RemoveNotificationSlotGroup(slotGroupId);
}

ErrCode NotificationHelper::GetNotificationSlotGroup(const std::string &groupId, sptr<NotificationSlotGroup> &group)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetNotificationSlotGroup(groupId, group);
}

ErrCode NotificationHelper::GetNotificationSlotGroups(std::vector<sptr<NotificationSlotGroup>> &groups)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetNotificationSlotGroups(groups);
}

ErrCode NotificationHelper::GetNotificationSlotNumAsBundle(const NotificationBundleOption &bundleOption, int &num)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetNotificationSlotNumAsBundle(bundleOption, num);
}

ErrCode NotificationHelper::PublishNotification(const NotificationRequest &request)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->PublishNotification(request);
}

ErrCode NotificationHelper::PublishNotification(const std::string &label, const NotificationRequest &request)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->PublishNotification(label, request);
}

ErrCode NotificationHelper::PublishNotification(const NotificationRequest &request, const std::string &deviceId)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->PublishNotification(request, deviceId);
}

ErrCode NotificationHelper::CancelNotification(int32_t notificationId)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->CancelNotification(notificationId);
}

ErrCode NotificationHelper::CancelNotification(const std::string &label, int32_t notificationId)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->CancelNotification(label, notificationId);
}

ErrCode NotificationHelper::CancelAllNotifications()
{
    return DelayedSingleton<AnsNotification>::GetInstance()->CancelAllNotifications();
}

ErrCode NotificationHelper::GetActiveNotificationNums(int32_t &num)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetActiveNotificationNums(num);
}

ErrCode NotificationHelper::GetActiveNotifications(std::vector<sptr<NotificationRequest>> &request)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetActiveNotifications(request);
}

ErrCode NotificationHelper::GetCurrentAppSorting(sptr<NotificationSortingMap> &sortingMap)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetCurrentAppSorting(sortingMap);
}

ErrCode NotificationHelper::SetNotificationAgent(const std::string &agent)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SetNotificationAgent(agent);
}

ErrCode NotificationHelper::GetNotificationAgent(std::string &agent)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetNotificationAgent(agent);
}

ErrCode NotificationHelper::CanPublishNotificationAsBundle(const std::string &representativeBundle, bool &canPublish)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->CanPublishNotificationAsBundle(
        representativeBundle, canPublish);
}

ErrCode NotificationHelper::PublishNotificationAsBundle(
    const std::string &representativeBundle, const NotificationRequest &request)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->PublishNotificationAsBundle(representativeBundle, request);
}

ErrCode NotificationHelper::SetNotificationBadgeNum()
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SetNotificationBadgeNum();
}

ErrCode NotificationHelper::SetNotificationBadgeNum(int32_t num)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SetNotificationBadgeNum(num);
}

ErrCode NotificationHelper::IsAllowedNotify(bool &allowed)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->IsAllowedNotify(allowed);
}

ErrCode NotificationHelper::AreNotificationsSuspended(bool &suspended)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->AreNotificationsSuspended(suspended);
}

ErrCode NotificationHelper::HasNotificationPolicyAccessPermission(bool &hasPermission)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->HasNotificationPolicyAccessPermission(hasPermission);
}

ErrCode NotificationHelper::GetBundleImportance(NotificationSlot::NotificationLevel &importance)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetBundleImportance(importance);
}

ErrCode NotificationHelper::SubscribeNotification(const NotificationSubscriber &subscriber)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SubscribeNotification(subscriber);
}

ErrCode NotificationHelper::SubscribeNotification(
    const NotificationSubscriber &subscriber, const NotificationSubscribeInfo &subscribeInfo)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SubscribeNotification(subscriber, subscribeInfo);
}

ErrCode NotificationHelper::UnSubscribeNotification(NotificationSubscriber &subscriber)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->UnSubscribeNotification(subscriber);
}

ErrCode NotificationHelper::UnSubscribeNotification(
    NotificationSubscriber &subscriber, NotificationSubscribeInfo subscribeInfo)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->UnSubscribeNotification(subscriber, subscribeInfo);
}

ErrCode NotificationHelper::RemoveNotification(const std::string &key)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->RemoveNotification(key);
}

ErrCode NotificationHelper::RemoveNotification(
    const NotificationBundleOption &bundleOption, const int32_t notificationId, const std::string &label)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->RemoveNotification(bundleOption, notificationId, label);
}

ErrCode NotificationHelper::RemoveAllNotifications(const NotificationBundleOption &bundleOption)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->RemoveAllNotifications(bundleOption);
}

ErrCode NotificationHelper::RemoveNotificationsByBundle(const NotificationBundleOption &bundleOption)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->RemoveNotificationsByBundle(bundleOption);
}

ErrCode NotificationHelper::RemoveNotifications()
{
    return DelayedSingleton<AnsNotification>::GetInstance()->RemoveNotifications();
}

ErrCode NotificationHelper::GetNotificationSlotsForBundle(
    const NotificationBundleOption &bundleOption, std::vector<sptr<NotificationSlot>> &slots)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetNotificationSlotsForBundle(bundleOption, slots);
}

ErrCode NotificationHelper::UpdateNotificationSlots(
    const NotificationBundleOption &bundleOption, const std::vector<sptr<NotificationSlot>> &slots)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->UpdateNotificationSlots(bundleOption, slots);
}

ErrCode NotificationHelper::UpdateNotificationSlotGroups(
    const NotificationBundleOption &bundleOption, const std::vector<sptr<NotificationSlotGroup>> &groups)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->UpdateNotificationSlotGroups(bundleOption, groups);
}

ErrCode NotificationHelper::GetAllActiveNotifications(std::vector<sptr<Notification>> &notification)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetAllActiveNotifications(notification);
}

ErrCode NotificationHelper::GetAllActiveNotifications(
    const std::vector<std::string> key, std::vector<sptr<Notification>> &notification)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetAllActiveNotifications(key, notification);
}

ErrCode NotificationHelper::IsAllowedNotify(const NotificationBundleOption &bundleOption, bool &allowed)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->IsAllowedNotify(bundleOption, allowed);
}

ErrCode NotificationHelper::SetNotificationsEnabledForAllBundles(const std::string &deviceId, bool enabled)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SetNotificationsEnabledForAllBundles(deviceId, enabled);
}

ErrCode NotificationHelper::SetNotificationsEnabledForDefaultBundle(const std::string &deviceId, bool enabled)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SetNotificationsEnabledForDefaultBundle(deviceId, enabled);
}

ErrCode NotificationHelper::SetNotificationsEnabledForSpecifiedBundle(
    const NotificationBundleOption &bundleOption, std::string &deviceId, bool enabled)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SetNotificationsEnabledForSpecifiedBundle(
        bundleOption, deviceId, enabled);
}

ErrCode NotificationHelper::SetShowBadgeEnabledForBundle(const NotificationBundleOption &bundleOption, bool enabled)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SetShowBadgeEnabledForBundle(bundleOption, enabled);
}

ErrCode NotificationHelper::GetShowBadgeEnabledForBundle(const NotificationBundleOption &bundleOption, bool &enabled)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetShowBadgeEnabledForBundle(bundleOption, enabled);
}

ErrCode NotificationHelper::GetShowBadgeEnabled(bool &enabled)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetShowBadgeEnabled(enabled);
}

ErrCode NotificationHelper::SetDisturbMode(NotificationConstant::DisturbMode mode)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->SetDisturbMode(mode);
}

ErrCode NotificationHelper::GetDisturbMode(NotificationConstant::DisturbMode &disturbMode)
{
    return DelayedSingleton<AnsNotification>::GetInstance()->GetDisturbMode(disturbMode);
}
}  // namespace Notification
}  // namespace OHOS