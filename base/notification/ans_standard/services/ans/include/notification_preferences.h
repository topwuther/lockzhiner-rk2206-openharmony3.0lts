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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_SERVICES_NOTIFICATION_PREFERENCES_H
#define BASE_NOTIFICATION_ANS_STANDARD_SERVICES_NOTIFICATION_PREFERENCES_H

#include "refbase.h"
#include "singleton.h"

#include "notification_preferences_database.h"


namespace OHOS {
namespace Notification {
class NotificationPreferences final {
public:
    DISALLOW_COPY_AND_MOVE(NotificationPreferences);
    static NotificationPreferences &GetInstance();
    ErrCode AddNotificationSlots(const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlot>> &slots);
    ErrCode AddNotificationSlotGroups(
        const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlotGroup>> &groups);
    ErrCode AddNotificationBundleProperty(const sptr<NotificationBundleOption> &bundleOption);
    ErrCode RemoveNotificationSlot(const sptr<NotificationBundleOption> &bundleOption, const NotificationConstant::SlotType &slotType);
    ErrCode RemoveNotificationAllSlots(const sptr<NotificationBundleOption> &bundleOption);
    ErrCode RemoveNotificationSlotGroups(const sptr<NotificationBundleOption> &bundleOption, const std::vector<std::string> &groupIds);
    ErrCode RemoveNotificationForBundle(const sptr<NotificationBundleOption> &bundleOption);
    ErrCode UpdateNotificationSlots(const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlot>> &slot);
    ErrCode UpdateNotificationSlotGroups(
        const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlotGroup>> &groups);
    ErrCode GetNotificationSlot(
        const sptr<NotificationBundleOption> &bundleOption, const NotificationConstant::SlotType &type, sptr<NotificationSlot> &slot);
    ErrCode GetNotificationAllSlots(const sptr<NotificationBundleOption> &bundleOption, std::vector<sptr<NotificationSlot>> &slots);
    ErrCode GetNotificationSlotsNumForBundle(const sptr<NotificationBundleOption> &bundleOption, int &num);
    ErrCode GetNotificationSlotGroup(
        const sptr<NotificationBundleOption> &bundleOption, const std::string &groupId, sptr<NotificationSlotGroup> &group);
    ErrCode GetNotificationAllSlotGroups(
        const sptr<NotificationBundleOption> &bundleOption, std::vector<sptr<NotificationSlotGroup>> &groups);
    ErrCode GetNotificationAllSlotInSlotGroup(
        const sptr<NotificationBundleOption> &bundleOption, const std::string &groupId, std::vector<sptr<NotificationSlot>> &slots);
    ErrCode IsShowBadge(const sptr<NotificationBundleOption> &bundleOption, bool &enable);
    ErrCode SetShowBadge(const sptr<NotificationBundleOption> &bundleOption, const bool enable);
    ErrCode GetImportance(const sptr<NotificationBundleOption> &bundleOption, int &importance);
    ErrCode SetImportance(const sptr<NotificationBundleOption> &bundleOption, const int &importance);
    ErrCode GetTotalBadgeNums(const sptr<NotificationBundleOption> &bundleOption, int &totalBadgeNum);
    ErrCode SetTotalBadgeNums(const sptr<NotificationBundleOption> &bundleOption, const int num);
    ErrCode GetPrivateNotificationsAllowed(const sptr<NotificationBundleOption> &bundleOption, bool &allow);
    ErrCode SetPrivateNotificationsAllowed(const sptr<NotificationBundleOption> &bundleOption, const bool allow);
    ErrCode GetNotificationsEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool &enabled);
    ErrCode SetNotificationsEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, const bool enabled);
    ErrCode GetNotificationsEnabled(bool &enabled);
    ErrCode SetNotificationsEnabled(const bool &enabled);
    ErrCode GetDisturbMode(NotificationConstant::DisturbMode &mode);
    ErrCode SetDisturbMode(const NotificationConstant::DisturbMode &mode);
    ErrCode ClearNotificationInRestoreFactorySettings();

    void OnDistributedKvStoreDeathRecipient();

private:
    ErrCode CheckSlotForCreateSlot(const sptr<NotificationBundleOption> &bundleOption, const sptr<NotificationSlot> &slot,
        NotificationPreferencesInfo &preferencesInfo) const;
    ErrCode CheckGroupForCreateSlotGroup(const sptr<NotificationBundleOption> &bundleOption, const sptr<NotificationSlotGroup> &group,
        NotificationPreferencesInfo &preferencesInfo) const;
    ErrCode CheckSlotForRemoveSlot(const sptr<NotificationBundleOption> &bundleOption, const NotificationConstant::SlotType &slotType,
        NotificationPreferencesInfo &preferencesInfo) const;
    ErrCode CheckGroupForRemoveSlotGroup(
        const sptr<NotificationBundleOption> &bundleOption, const std::string &groupId, NotificationPreferencesInfo &preferencesInfo) const;
    ErrCode CheckSlotForUpdateSlot(const sptr<NotificationBundleOption> &bundleOption, const sptr<NotificationSlot> &slot,
        NotificationPreferencesInfo &preferencesInfo) const;
    ErrCode CheckGroupForUpdateSlotGroup(const sptr<NotificationBundleOption> &bundleOption, const sptr<NotificationSlotGroup> &group,
        NotificationPreferencesInfo &preferencesInfo) const;
    template<typename T>
    ErrCode SetBundleProperty(NotificationPreferencesInfo &preferencesInfo, const sptr<NotificationBundleOption> &bundleOption,
        const BundleType &type, const T &value);
    template<typename T>
    ErrCode SaveBundleProperty(NotificationPreferencesInfo::BundleInfo &bundleInfo, const sptr<NotificationBundleOption> &bundleOption,
        const BundleType &type, const T &value);
    template<typename T>
    ErrCode GetBundleProperty(const sptr<NotificationBundleOption> &bundleOption, const BundleType &type, T &value) const;
    std::string GenerateBundleKey(const sptr<NotificationBundleOption> &bundleOption) const;

private:
    NotificationPreferencesInfo preferencesInfo_ {};
    std::unique_ptr<NotificationPreferencesDatabase> preferncesDB_ = nullptr;
    DECLARE_DELAYED_REF_SINGLETON(NotificationPreferences);
};

}  // namespace Notification
}  // namespace OHOS

#endif  // NOTIFICATION_PREFERENCES_H
