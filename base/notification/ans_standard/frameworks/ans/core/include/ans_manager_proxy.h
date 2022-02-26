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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_MANAGER_PROXY_H
#define BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_MANAGER_PROXY_H

#include "ans_manager_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Notification {
class AnsManagerProxy : public IRemoteProxy<IAnsManager> {
public:
    AnsManagerProxy() = delete;
    explicit AnsManagerProxy(const sptr<IRemoteObject> &impl);
    ~AnsManagerProxy() override;
    DISALLOW_COPY_AND_MOVE(AnsManagerProxy);

    ErrCode Publish(const std::string &label, const sptr<NotificationRequest> &notification) override;
    ErrCode PublishToDevice(const sptr<NotificationRequest> &notification, const std::string &deviceId) override;
    ErrCode Cancel(int notificationId, const std::string &label) override;
    ErrCode CancelAll() override;
    ErrCode AddSlotByType(NotificationConstant::SlotType slotType) override;
    ErrCode AddSlots(const std::vector<sptr<NotificationSlot>> &slots) override;
    ErrCode RemoveSlotByType(const NotificationConstant::SlotType &slotType) override;
    ErrCode RemoveAllSlots() override;
    ErrCode AddSlotGroups(std::vector<sptr<NotificationSlotGroup>> groups) override;
    ErrCode GetSlotByType(const NotificationConstant::SlotType &slotType, sptr<NotificationSlot> &slot) override;
    ErrCode GetSlots(std::vector<sptr<NotificationSlot>> &slots) override;
    ErrCode GetSlotGroup(const std::string &groupId, sptr<NotificationSlotGroup> &group) override;
    ErrCode GetSlotGroups(std::vector<sptr<NotificationSlotGroup>> &groups) override;

    ErrCode GetSlotNumAsBundle(const sptr<NotificationBundleOption> &bundleOption, int &num) override;

    ErrCode RemoveSlotGroups(const std::vector<std::string> &groupIds) override;
    ErrCode GetActiveNotifications(std::vector<sptr<NotificationRequest>> &notifications) override;
    ErrCode GetActiveNotificationNums(int &num) override;
    ErrCode GetAllActiveNotifications(std::vector<sptr<Notification>> &notifications) override;
    ErrCode GetSpecialActiveNotifications(
        const std::vector<std::string> &key, std::vector<sptr<Notification>> &notifications) override;
    ErrCode SetNotificationAgent(const std::string &agent) override;
    ErrCode GetNotificationAgent(std::string &agent) override;
    ErrCode CanPublishAsBundle(const std::string &representativeBundle, bool &canPublish) override;
    ErrCode PublishAsBundle(
        const sptr<NotificationRequest> notification, const std::string &representativeBundle) override;
    ErrCode SetNotificationBadgeNum(int num) override;
    ErrCode GetBundleImportance(int &importance) override;
    ErrCode SetDisturbMode(NotificationConstant::DisturbMode mode) override;
    ErrCode GetDisturbMode(NotificationConstant::DisturbMode &mode) override;
    ErrCode HasNotificationPolicyAccessPermission(bool &granted) override;
    ErrCode SetPrivateNotificationsAllowed(bool allow) override;
    ErrCode GetPrivateNotificationsAllowed(bool &allow) override;

    ErrCode RemoveNotification(
        const sptr<NotificationBundleOption> &bundleOption, int notificationId, const std::string &label) override;

    ErrCode RemoveAllNotifications(const sptr<NotificationBundleOption> &bundleOption) override;

    ErrCode Delete(const std::string &key) override;

    ErrCode DeleteByBundle(const sptr<NotificationBundleOption> &bundleOption) override;

    ErrCode DeleteAll() override;
    ErrCode GetSlotsByBundle(
        const sptr<NotificationBundleOption> &bundleOption, std::vector<sptr<NotificationSlot>> &slots) override;
    ErrCode UpdateSlots(
        const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlot>> &slots) override;
    ErrCode UpdateSlotGroups(const sptr<NotificationBundleOption> &bundleOption,
        const std::vector<sptr<NotificationSlotGroup>> &groups) override;
    ErrCode SetNotificationsEnabledForBundle(const std::string &deviceId, bool enabled) override;
    ErrCode SetNotificationsEnabledForAllBundles(const std::string &deviceId, bool enabled) override;
    ErrCode SetNotificationsEnabledForSpecialBundle(
        const std::string &deviceId, const sptr<NotificationBundleOption> &bundleOption, bool enabled) override;
    ErrCode SetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool enabled) override;
    ErrCode GetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool &enabled) override;
    ErrCode GetShowBadgeEnabled(bool &enabled) override;
    ErrCode Subscribe(const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info) override;
    ErrCode Unsubscribe(const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info) override;
    ErrCode AreNotificationsSuspended(bool &suspended) override;
    ErrCode GetCurrentAppSorting(sptr<NotificationSortingMap> &sortingMap) override;
    ErrCode IsAllowedNotify(bool &allowed) override;
    ErrCode IsSpecialBundleAllowedNotify(const sptr<NotificationBundleOption> &bundleOption, bool &allowed) override;

    ErrCode ShellDump(const std::string &dumpOption, std::vector<std::string> &dumpInfo) override;

private:
    ErrCode InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);

    template<typename T>
    bool WriteParcelableVector(const std::vector<sptr<T>> &parcelableVector, MessageParcel &data);

    template<typename T>
    bool ReadParcelableVector(std::vector<sptr<T>> &parcelableInfos, MessageParcel &reply, ErrCode &result);
    static inline BrokerDelegator<AnsManagerProxy> delegator_;
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_MANAGER_PROXY_H
