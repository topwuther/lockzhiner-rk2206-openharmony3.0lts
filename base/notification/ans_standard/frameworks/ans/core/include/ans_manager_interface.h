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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_MANAGER_INTERFACE_H
#define BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_MANAGER_INTERFACE_H

#include <string>
#include <vector>

#include "ans_subscriber_interface.h"
#include "iremote_broker.h"
#include "notification_bundle_option.h"
#include "notification_constant.h"
#include "notification_request.h"
#include "notification_slot.h"
#include "notification_slot_group.h"
#include "notification_sorting.h"
#include "notification_subscribe_info.h"

namespace OHOS {
namespace Notification {
class IAnsManager : public IRemoteBroker {
public:
    IAnsManager() = default;
    virtual ~IAnsManager() override = default;
    DISALLOW_COPY_AND_MOVE(IAnsManager);

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Notification.IAnsManager");

    virtual ErrCode Publish(const std::string &label, const sptr<NotificationRequest> &notification) = 0;
    virtual ErrCode PublishToDevice(const sptr<NotificationRequest> &notification, const std::string &deviceId) = 0;
    virtual ErrCode Cancel(int notificationId, const std::string &label) = 0;
    virtual ErrCode CancelAll() = 0;
    virtual ErrCode AddSlotByType(NotificationConstant::SlotType slotType) = 0;
    virtual ErrCode AddSlots(const std::vector<sptr<NotificationSlot>> &slots) = 0;
    virtual ErrCode RemoveSlotByType(const NotificationConstant::SlotType &slotType) = 0;
    virtual ErrCode RemoveAllSlots() = 0;
    virtual ErrCode AddSlotGroups(std::vector<sptr<NotificationSlotGroup>> groups) = 0;
    virtual ErrCode GetSlotByType(const NotificationConstant::SlotType &slotType, sptr<NotificationSlot> &slot) = 0;
    virtual ErrCode GetSlots(std::vector<sptr<NotificationSlot>> &slots) = 0;
    virtual ErrCode GetSlotGroup(const std::string &groupId, sptr<NotificationSlotGroup> &group) = 0;
    virtual ErrCode GetSlotGroups(std::vector<sptr<NotificationSlotGroup>> &groups) = 0;

    virtual ErrCode GetSlotNumAsBundle(const sptr<NotificationBundleOption> &bundleOption, int &num) = 0;

    virtual ErrCode RemoveSlotGroups(const std::vector<std::string> &groupIds) = 0;
    virtual ErrCode GetActiveNotifications(std::vector<sptr<NotificationRequest>> &notifications) = 0;
    virtual ErrCode GetActiveNotificationNums(int &num) = 0;
    virtual ErrCode GetAllActiveNotifications(std::vector<sptr<Notification>> &notifications) = 0;
    virtual ErrCode GetSpecialActiveNotifications(
        const std::vector<std::string> &key, std::vector<sptr<Notification>> &notifications) = 0;
    virtual ErrCode SetNotificationAgent(const std::string &agent) = 0;
    virtual ErrCode GetNotificationAgent(std::string &agent) = 0;
    virtual ErrCode CanPublishAsBundle(const std::string &representativeBundle, bool &canPublish) = 0;
    virtual ErrCode PublishAsBundle(
        const sptr<NotificationRequest> notification, const std::string &representativeBundle) = 0;
    virtual ErrCode SetNotificationBadgeNum(int num) = 0;
    virtual ErrCode GetBundleImportance(int &importance) = 0;
    virtual ErrCode SetDisturbMode(NotificationConstant::DisturbMode mode) = 0;
    virtual ErrCode GetDisturbMode(NotificationConstant::DisturbMode &mode) = 0;
    virtual ErrCode HasNotificationPolicyAccessPermission(bool &granted) = 0;
    virtual ErrCode SetPrivateNotificationsAllowed(bool allow) = 0;
    virtual ErrCode GetPrivateNotificationsAllowed(bool &allow) = 0;
    virtual ErrCode Delete(const std::string &key) = 0;

    virtual ErrCode RemoveNotification(
        const sptr<NotificationBundleOption> &bundleOption, int notificationId, const std::string &label) = 0;

    virtual ErrCode RemoveAllNotifications(const sptr<NotificationBundleOption> &bundleOption) = 0;

    virtual ErrCode DeleteByBundle(const sptr<NotificationBundleOption> &bundleOption) = 0;

    virtual ErrCode DeleteAll() = 0;
    virtual ErrCode GetSlotsByBundle(
        const sptr<NotificationBundleOption> &bundleOption, std::vector<sptr<NotificationSlot>> &slots) = 0;
    virtual ErrCode UpdateSlots(
        const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlot>> &slots) = 0;
    virtual ErrCode UpdateSlotGroups(
        const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlotGroup>> &groups) = 0;
    virtual ErrCode SetNotificationsEnabledForBundle(const std::string &deviceId, bool enabled) = 0;
    virtual ErrCode SetNotificationsEnabledForAllBundles(const std::string &deviceId, bool enabled) = 0;
    virtual ErrCode SetNotificationsEnabledForSpecialBundle(
        const std::string &deviceId, const sptr<NotificationBundleOption> &bundleOption, bool enabled) = 0;
    virtual ErrCode SetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool enabled) = 0;
    virtual ErrCode GetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool &enabled) = 0;
    virtual ErrCode GetShowBadgeEnabled(bool &enabled) = 0;
    virtual ErrCode Subscribe(const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info) = 0;
    virtual ErrCode Unsubscribe(
        const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info) = 0;
    virtual ErrCode AreNotificationsSuspended(bool &suspended) = 0;
    virtual ErrCode GetCurrentAppSorting(sptr<NotificationSortingMap> &sortingMap) = 0;
    virtual ErrCode IsAllowedNotify(bool &allowed) = 0;
    virtual ErrCode IsSpecialBundleAllowedNotify(const sptr<NotificationBundleOption> &bundleOption, bool &allowed) = 0;

    virtual ErrCode ShellDump(const std::string &dumpOption, std::vector<std::string> &dumpInfo) = 0;

protected:
    enum TransactId : uint32_t {
        PUBLISH_NOTIFICATION = FIRST_CALL_TRANSACTION,
        PUBLISH_NOTIFICATION_TO_DEVICE,
        CANCEL_NOTIFICATION,
        CANCEL_ALL_NOTIFICATIONS,
        ADD_SLOT_BY_TYPE,
        ADD_SLOTS,
        REMOVE_SLOT_BY_TYPE,
        REMOVE_ALL_SLOTS,
        ADD_SLOT_GROUPS,
        GET_SLOT_BY_TYPE,
        GET_SLOTS,
        GET_SLOT_GROUP,
        GET_SLOT_GROUPS,
        GET_SLOT_NUM_AS_BUNDLE,
        REMOVE_SLOT_GROUPS,
        GET_ACTIVE_NOTIFICATIONS,
        GET_ACTIVE_NOTIFICATION_NUMS,
        GET_ALL_ACTIVE_NOTIFICATIONS,
        GET_SPECIAL_ACTIVE_NOTIFICATIONS,
        SET_NOTIFICATION_AGENT,
        GET_NOTIFICATION_AGENT,
        CAN_PUBLISH_AS_BUNDLE,
        PUBLISH_AS_BUNDLE,
        SET_NOTIFICATION_BADGE_NUM,
        GET_BUNDLE_IMPORTANCE,
        SET_DISTURB_MODE,
        GET_DISTURB_MODE,
        IS_NOTIFICATION_POLICY_ACCESS_GRANTED,
        SET_PRIVATIVE_NOTIFICATIONS_ALLOWED,
        GET_PRIVATIVE_NOTIFICATIONS_ALLOWED,
        REMOVE_NOTIFICATION,
        REMOVE_ALL_NOTIFICATIONS,
        DELETE_NOTIFICATION,
        DELETE_NOTIFICATION_BY_BUNDLE,
        DELETE_ALL_NOTIFICATIONS,
        GET_SLOTS_BY_BUNDLE,
        UPDATE_SLOTS,
        UPDATE_SLOT_GROUPS,
        SET_NOTIFICATION_ENABLED_FOR_BUNDLE,
        SET_NOTIFICATION_ENABLED_FOR_ALL_BUNDLE,
        SET_NOTIFICATION_ENABLED_FOR_SPECIAL_BUNDLE,
        SET_SHOW_BADGE_ENABLED_FOR_BUNDLE,
        GET_SHOW_BADGE_ENABLED_FOR_BUNDLE,
        GET_SHOW_BADGE_ENABLED,
        SUBSCRIBE_NOTIFICATION,
        UNSUBSCRIBE_NOTIFICATION,
        ARE_NOTIFICATION_SUSPENDED,
        GET_CURRENT_APP_SORTING,
        IS_ALLOWED_NOTIFY,
        IS_SPECIAL_BUNDLE_ALLOWED_NOTIFY,
        SHELL_DUMP,
    };
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_MANAGER_INTERFACE_H
