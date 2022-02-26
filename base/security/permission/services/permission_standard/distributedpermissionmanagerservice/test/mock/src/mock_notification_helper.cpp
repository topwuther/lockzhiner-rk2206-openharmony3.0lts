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
    return 0;
}

ErrCode NotificationHelper::AddNotificationSlots(const std::vector<NotificationSlot> &slots)
{
    return 0;
}

ErrCode NotificationHelper::RemoveNotificationSlot(const NotificationConstant::SlotType &slotType)
{
    return 0;
}

ErrCode NotificationHelper::GetNotificationSlot(
    const NotificationConstant::SlotType &slotType, sptr<NotificationSlot> &slot)
{
    return 0;
}

ErrCode NotificationHelper::GetNotificationSlots(std::vector<sptr<NotificationSlot>> &slots)
{
    return 0;
}

ErrCode NotificationHelper::AddNotificationSlotGroup(const NotificationSlotGroup &slotGroup)
{
    return 0;
}

ErrCode NotificationHelper::AddNotificationSlotGroups(const std::vector<NotificationSlotGroup> &slotGroups)
{
    return 0;
}

ErrCode NotificationHelper::RemoveNotificationSlotGroup(const std::string &slotGroupId)
{
    return 0;
}

ErrCode NotificationHelper::GetNotificationSlotGroup(const std::string &groupId, sptr<NotificationSlotGroup> &group)
{
    return 0;
}

ErrCode NotificationHelper::GetNotificationSlotGroups(std::vector<sptr<NotificationSlotGroup>> &groups)
{
    return 0;
}

ErrCode NotificationHelper::PublishNotification(const NotificationRequest &request)
{
    return 0;
}

ErrCode NotificationHelper::PublishNotification(const std::string &label, const NotificationRequest &request)
{
    return 0;
}

ErrCode NotificationHelper::PublishNotification(const NotificationRequest &request, const std::string &deviceId)
{
    return 0;
}

ErrCode NotificationHelper::CancelNotification(int32_t notificationId)
{
    return 0;
}

ErrCode NotificationHelper::CancelNotification(const std::string &label, int32_t notificationId)
{
    return 0;
}

ErrCode NotificationHelper::CancelAllNotifications()
{
    return 0;
}

ErrCode NotificationHelper::GetActiveNotificationNums(int32_t &num)
{
    return 0;
}

ErrCode NotificationHelper::GetActiveNotifications(std::vector<sptr<NotificationRequest>> &request)
{
    return 0;
}

ErrCode NotificationHelper::GetCurrentAppSorting(sptr<NotificationSortingMap> &sortingMap)
{
    return 0;
}

ErrCode NotificationHelper::SetNotificationAgent(const std::string &agent)
{
    return 0;
}

ErrCode NotificationHelper::GetNotificationAgent(std::string &agent)
{
    return 0;
}

ErrCode NotificationHelper::CanPublishNotificationAsBundle(const std::string &representativeBundle, bool &canPublish)
{
    return 0;
}

ErrCode NotificationHelper::PublishNotificationAsBundle(
    const std::string &representativeBundle, const NotificationRequest &request)
{
    return 0;
}

ErrCode NotificationHelper::SetNotificationBadgeNum()
{
    return 0;
}

ErrCode NotificationHelper::SetNotificationBadgeNum(int32_t num)
{
    return 0;
}

ErrCode NotificationHelper::IsAllowedNotify(bool &allowed)
{
    return 0;
}

ErrCode NotificationHelper::AreNotificationsSuspended(bool &suspended)
{
    return 0;
}

ErrCode NotificationHelper::HasNotificationPolicyAccessPermission(bool &hasPermission)
{
    return 0;
}

ErrCode NotificationHelper::GetBundleImportance(NotificationSlot::NotificationLevel &importance)
{
    return 0;
}

ErrCode NotificationHelper::SubscribeNotification(const NotificationSubscriber &subscriber)
{
    return 0;
}

ErrCode NotificationHelper::SubscribeNotification(
    const NotificationSubscriber &subscriber, const NotificationSubscribeInfo &subscribeInfo)
{
    return 0;
}

ErrCode NotificationHelper::UnSubscribeNotification(NotificationSubscriber &subscriber)
{
    return 0;
}

ErrCode NotificationHelper::UnSubscribeNotification(
    NotificationSubscriber &subscriber, NotificationSubscribeInfo subscribeInfo)
{
    return 0;
}

ErrCode NotificationHelper::RemoveNotification(const std::string &key)
{
    return 0;
}

// ErrCode NotificationHelper::RemoveNotifications(const std::string &bundleName)
// {
//     return 0;
// }

ErrCode NotificationHelper::RemoveNotifications()
{
    return 0;
}

// ErrCode NotificationHelper::GetNotificationSlotsForBundle(
//     const std::string &bundleName, std::vector<sptr<NotificationSlot>> &slots)
// {
//     return 0;
// }

ErrCode NotificationHelper::GetAllActiveNotifications(std::vector<sptr<Notification>> &notification)
{
    return 0;
}

ErrCode NotificationHelper::GetAllActiveNotifications(
    const std::vector<std::string> key, std::vector<sptr<Notification>> &notification)
{
    return 0;
}

// ErrCode NotificationHelper::IsAllowedNotify(const NotificationBundleOption &bundleOption, bool &allowed)
// {
//     return 0;
// }

ErrCode NotificationHelper::SetNotificationsEnabledForAllBundles(const std::string &deviceId, bool enabled)
{
    return 0;
}

ErrCode NotificationHelper::SetNotificationsEnabledForDefaultBundle(const std::string &deviceId, bool enabled)
{
    return 0;
}

// ErrCode NotificationHelper::SetNotificationsEnabledForSpecifiedBundle(
//     const NotificationBundleOption &bundleOption, std::string &deviceId, bool enabled)
// {
//     return 0;
// }

ErrCode NotificationHelper::SetDisturbMode(NotificationConstant::DisturbMode mode)
{
    return 0;
}

ErrCode NotificationHelper::GetDisturbMode(NotificationConstant::DisturbMode &disturbMode)
{
    return 0;
}
}  // namespace Notification
}  // namespace OHOS