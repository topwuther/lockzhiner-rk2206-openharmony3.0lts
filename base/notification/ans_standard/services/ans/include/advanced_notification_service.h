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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_ADVANCED_NOTIFICATION_SERVICE_H
#define BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_ADVANCED_NOTIFICATION_SERVICE_H

#include <ctime>
#include <list>
#include <memory>
#include <mutex>

#include "event_handler.h"
#include "event_runner.h"
#include "refbase.h"

#include "ans_manager_stub.h"
#include "distributed_kv_data_manager.h"
#include "distributed_kvstore_death_recipient.h"
#include "notification.h"
#include "notification_bundle_option.h"
#include "notification_record.h"
#include "notification_sorting_map.h"
#include "system_event_observer.h"

namespace OHOS {
namespace Notification {

class AdvancedNotificationService final : public AnsManagerStub {
public:
    ~AdvancedNotificationService() override;
    DISALLOW_COPY_AND_MOVE(AdvancedNotificationService);

    static sptr<AdvancedNotificationService> GetInstance();

    // AnsManagerStub
    ErrCode Publish(const std::string &label, const sptr<NotificationRequest> &request) override;
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
    ErrCode SetNotificationsEnabledForBundle(const std::string &bundle, bool enabled) override;
    ErrCode SetNotificationsEnabledForAllBundles(const std::string &deviceId, bool enabled) override;
    ErrCode SetNotificationsEnabledForSpecialBundle(
        const std::string &deviceId, const sptr<NotificationBundleOption> &bundleOption, bool enabled) override;
    ErrCode SetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool enabled) override;
    ErrCode GetShowBadgeEnabledForBundle(const sptr<NotificationBundleOption> &bundleOption, bool &enabled) override;
    ErrCode GetShowBadgeEnabled(bool &enabled) override;
    ErrCode Subscribe(const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info) override;
    ErrCode Unsubscribe(const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info) override;
    ErrCode IsAllowedNotify(bool &allowed) override;
    ErrCode IsSpecialBundleAllowedNotify(const sptr<NotificationBundleOption> &bundleOption, bool &allowed) override;

    ErrCode ShellDump(const std::string &dumpOption, std::vector<std::string> &dumpInfo) override;

    // SystemEvent
    void OnBundleRemoved(const sptr<NotificationBundleOption> &bundleOption);

    // Distributed KvStore
    void OnDistributedKvStoreDeathRecipient();

private:
    struct RecentInfo;
    AdvancedNotificationService();

    void StartFilters();
    void StopFilters();
    ErrCode Filter(const std::shared_ptr<NotificationRecord> &record);

    void AddToNotificationList(const std::shared_ptr<NotificationRecord> &record);
    void UpdateInNotificationList(const std::shared_ptr<NotificationRecord> &record);
    ErrCode RemoveFromNotificationList(const sptr<NotificationBundleOption> &bundleOption, const std::string &label,
        int notificationId, sptr<Notification> &notification, bool isCancel = false);
    ErrCode RemoveFromNotificationList(const std::string &key, sptr<Notification> &notification, bool isCancel = false);
    std::vector<std::string> GetNotificationKeys(const sptr<NotificationBundleOption> &bundleOption);
    bool IsNotificationExists(const std::string &key);
    void SortNotificationList();
    static bool NotificationCompare(
        const std::shared_ptr<NotificationRecord> &first, const std::shared_ptr<NotificationRecord> &second);
    ErrCode FlowControl(const std::shared_ptr<NotificationRecord> &record);

    sptr<NotificationSortingMap> GenerateSortingMap();
    sptr<NotificationBundleOption> GenerateBundleOption();
    sptr<NotificationBundleOption> GenerateValidBundleOption(const sptr<NotificationBundleOption> &bundleOption);

    std::string TimeToString(int64_t time);
    int64_t GetNowSysTime();
    ErrCode ActiveNotificationDump(std::vector<std::string> &dumpInfo);
    ErrCode RecentNotificationDump(std::vector<std::string> &dumpInfo);
    ErrCode SetRecentNotificationCount(const std::string arg);
    void UpdateRecentNotification(sptr<Notification> &notification, bool isDelete, int reason);

private:
    static sptr<AdvancedNotificationService> instance_;
    static std::mutex instanceMutex_;

    std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler_ = nullptr;
    std::list<std::shared_ptr<NotificationRecord>> notificationList_;
    std::list<std::chrono::system_clock::time_point> flowControlTimestampList_;
    std::shared_ptr<RecentInfo> recentInfo_ = nullptr;
    std::shared_ptr<DistributedKvStoreDeathRecipient> distributedKvStoreDeathRecipient_ = nullptr;
    std::shared_ptr<SystemEventObserver> systemEventObserver_ = nullptr;
    DistributedKv::DistributedKvDataManager dataManager_;
};

}  // namespace Notification
}  // namespace OHOS

#endif  // ADVANCED_NOTIFICATION_SERVICE_H