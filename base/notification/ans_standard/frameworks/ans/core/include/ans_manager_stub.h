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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_MANAGER_STUB_H
#define BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_MANAGER_STUB_H

#include <functional>
#include <map>

#include "ans_manager_interface.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Notification {
class AnsManagerStub : public IRemoteStub<IAnsManager> {
public:
    AnsManagerStub();
    ~AnsManagerStub() override;
    DISALLOW_COPY_AND_MOVE(AnsManagerStub);

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    virtual ErrCode Publish(const std::string &label, const sptr<NotificationRequest> &notification) override;
    virtual ErrCode PublishToDevice(
        const sptr<NotificationRequest> &notification, const std::string &deviceId) override;
    virtual ErrCode Cancel(int notificationId, const std::string &label) override;
    virtual ErrCode CancelAll() override;
    virtual ErrCode AddSlotByType(NotificationConstant::SlotType slotType) override;
    virtual ErrCode AddSlots(const std::vector<sptr<NotificationSlot>> &slots) override;
    virtual ErrCode RemoveSlotByType(const NotificationConstant::SlotType &slotType) override;
    virtual ErrCode RemoveAllSlots() override;
    virtual ErrCode AddSlotGroups(std::vector<sptr<NotificationSlotGroup>> groups) override;
    virtual ErrCode GetSlotByType(
        const NotificationConstant::SlotType &slotType, sptr<NotificationSlot> &slot) override;
    virtual ErrCode GetSlots(std::vector<sptr<NotificationSlot>> &slots) override;
    virtual ErrCode GetSlotGroup(const std::string &groupId, sptr<NotificationSlotGroup> &group) override;
    virtual ErrCode GetSlotGroups(std::vector<sptr<NotificationSlotGroup>> &groups) override;
    virtual ErrCode GetSlotNumAsBundle(const sptr<NotificationBundleOption> &bundleOption, int &num) override;
    virtual ErrCode RemoveSlotGroups(const std::vector<std::string> &groupIds) override;
    virtual ErrCode GetActiveNotifications(std::vector<sptr<NotificationRequest>> &notifications) override;
    virtual ErrCode GetActiveNotificationNums(int &num) override;
    virtual ErrCode GetAllActiveNotifications(std::vector<sptr<Notification>> &notifications) override;
    virtual ErrCode GetSpecialActiveNotifications(
        const std::vector<std::string> &key, std::vector<sptr<Notification>> &notifications) override;
    virtual ErrCode SetNotificationAgent(const std::string &agent) override;
    virtual ErrCode GetNotificationAgent(std::string &agent) override;
    virtual ErrCode CanPublishAsBundle(const std::string &representativeBundle, bool &canPublish) override;
    virtual ErrCode PublishAsBundle(
        const sptr<NotificationRequest> notification, const std::string &representativeBundle) override;
    virtual ErrCode SetNotificationBadgeNum(int num) override;
    virtual ErrCode GetBundleImportance(int &importance) override;
    virtual ErrCode SetDisturbMode(NotificationConstant::DisturbMode mode) override;
    virtual ErrCode GetDisturbMode(NotificationConstant::DisturbMode &mode) override;
    virtual ErrCode HasNotificationPolicyAccessPermission(bool &granted) override;
    virtual ErrCode SetPrivateNotificationsAllowed(bool allow) override;
    virtual ErrCode GetPrivateNotificationsAllowed(bool &allow) override;
    virtual ErrCode RemoveNotification(
        const sptr<NotificationBundleOption> &bundleOption, int notificationId, const std::string &label) override;
    virtual ErrCode RemoveAllNotifications(const sptr<NotificationBundleOption> &bundleOption) override;
    virtual ErrCode Delete(const std::string &key) override;
    virtual ErrCode DeleteByBundle(const sptr<NotificationBundleOption> &bundleOption) override;
    virtual ErrCode DeleteAll() override;
    virtual ErrCode GetSlotsByBundle(
        const sptr<NotificationBundleOption> &bundleOption, std::vector<sptr<NotificationSlot>> &slots) override;
    virtual ErrCode UpdateSlots(
        const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlot>> &slots) override;
    virtual ErrCode UpdateSlotGroups(const sptr<NotificationBundleOption> &bundleOption,
        const std::vector<sptr<NotificationSlotGroup>> &groups) override;
    virtual ErrCode SetNotificationsEnabledForBundle(const std::string &deviceId, bool enabled) override;
    virtual ErrCode SetNotificationsEnabledForAllBundles(const std::string &deviceId, bool enabled) override;
    virtual ErrCode SetNotificationsEnabledForSpecialBundle(
        const std::string &deviceId, const sptr<NotificationBundleOption> &bundleOption, bool enabled) override;
    virtual ErrCode SetShowBadgeEnabledForBundle(
        const sptr<NotificationBundleOption> &bundleOption, bool enabled) override;
    virtual ErrCode GetShowBadgeEnabledForBundle(
        const sptr<NotificationBundleOption> &bundleOption, bool &enabled) override;
    virtual ErrCode GetShowBadgeEnabled(bool &enabled) override;
    virtual ErrCode Subscribe(
        const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info) override;
    virtual ErrCode Unsubscribe(
        const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info) override;
    virtual ErrCode AreNotificationsSuspended(bool &suspended) override;
    virtual ErrCode GetCurrentAppSorting(sptr<NotificationSortingMap> &sortingMap) override;
    virtual ErrCode IsAllowedNotify(bool &allowed) override;
    virtual ErrCode IsSpecialBundleAllowedNotify(
        const sptr<NotificationBundleOption> &bundleOption, bool &allowed) override;

    virtual ErrCode ShellDump(const std::string &dumpOption, std::vector<std::string> &dumpInfo) override;

private:
    static const std::map<uint32_t, std::function<ErrCode(AnsManagerStub *, MessageParcel &, MessageParcel &)>>
        interfaces_;

    ErrCode HandlePublish(MessageParcel &data, MessageParcel &reply);
    ErrCode HandlePublishToDevice(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleCancel(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleCancelAll(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleAddSlotByType(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleAddSlots(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleRemoveSlotByType(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleRemoveAllSlots(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleAddSlotGroups(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetSlots(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetSlotByType(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetSlotGroup(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetSlotGroups(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetSlotNumAsBundle(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleRemoveSlotGroups(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetActiveNotifications(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetActiveNotificationNums(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetAllActiveNotifications(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetSpecialActiveNotifications(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSetNotificationAgent(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetNotificationAgent(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleCanPublishAsBundle(MessageParcel &data, MessageParcel &reply);
    ErrCode HandlePublishAsBundle(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSetNotificationBadgeNum(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetBundleImportance(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSetDisturbMode(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetDisturbMode(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleIsNotificationPolicyAccessGranted(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSetPrivateNotificationsAllowed(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetPrivateNotificationsAllowed(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleRemoveNotification(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleRemoveAllNotifications(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleDelete(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleDeleteByBundle(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleDeleteAll(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetSlotsByBundle(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleUpdateSlots(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleUpdateSlotGroups(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSetNotificationsEnabledForBundle(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSetNotificationsEnabledForAllBundles(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSetNotificationsEnabledForSpecialBundle(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSetShowBadgeEnabledForBundle(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetShowBadgeEnabledForBundle(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetShowBadgeEnabled(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleSubscribe(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleUnsubscribe(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleAreNotificationsSuspended(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleGetCurrentAppSorting(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleIsAllowedNotify(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleIsSpecialBundleAllowedNotify(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleShellDump(MessageParcel &data, MessageParcel &reply);

    template<typename T>
    bool WriteParcelableVector(const std::vector<sptr<T>> &parcelableVector, MessageParcel &reply, ErrCode &result);

    template<typename T>
    bool ReadParcelableVector(std::vector<sptr<T>> &parcelableInfos, MessageParcel &data);
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_MANAGER_STUB_H
