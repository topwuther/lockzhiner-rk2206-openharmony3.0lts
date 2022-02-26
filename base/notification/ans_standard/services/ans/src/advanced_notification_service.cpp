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

#include "advanced_notification_service.h"

#include <functional>
#include <iomanip>
#include <sstream>

#include "ans_const_define.h"
#include "ans_inner_errors.h"
#include "ans_log_wrapper.h"
#include "bundle_manager_helper.h"
#include "disturb_filter.h"
#include "ipc_skeleton.h"
#include "notification_constant.h"
#include "notification_filter.h"
#include "notification_preferences.h"
#include "notification_slot.h"
#include "notification_slot_filter.h"
#include "notification_subscriber_manager.h"
#include "permission_filter.h"

namespace OHOS {
namespace Notification {

namespace {
static const std::string ACTIVE_NOTIFICATION_OPTION = "active";
static const std::string RECENT_NOTIFICATION_OPTION = "recent";
static const std::string SET_RECENT_COUNT_OPTION = "setRecentCount";

static const int32_t NOTIFICATION_MIN_COUNT = 0;
static const int32_t NOTIFICATION_MAX_COUNT = 1024;

static const int32_t DEFAULT_RECENT_COUNT = 16;

struct RecentNotification {
    sptr<Notification> notification = nullptr;
    bool isActive = false;
    int deleteReason = 0;
    int64_t deleteTime = 0;
};
}  // namespace

struct AdvancedNotificationService::RecentInfo {
    std::list<std::shared_ptr<RecentNotification>> list;
    size_t recentCount = DEFAULT_RECENT_COUNT;
};

sptr<AdvancedNotificationService> AdvancedNotificationService::instance_;
std::mutex AdvancedNotificationService::instanceMutex_;

static const std::shared_ptr<INotificationFilter> NOTIFICATION_FILTERS[] = {
    std::make_shared<PermissionFilter>(),
    std::make_shared<NotificationSlotFilter>(),
    std::make_shared<DisturbFilter>(),
};

inline std::string GetClientBundleName()
{
    std::string bundle;

    int callingUid = IPCSkeleton::GetCallingUid();

    std::shared_ptr<BundleManagerHelper> bundleManager = BundleManagerHelper::GetInstance();
    if (bundleManager != nullptr) {
        bundle = bundleManager->GetBundleNameByUid(callingUid);
    }

    return bundle;
}

inline bool IsSystemApp()
{
    bool isSystemApp = false;

    int callingUid = IPCSkeleton::GetCallingUid();

    std::shared_ptr<BundleManagerHelper> bundleManager = BundleManagerHelper::GetInstance();
    if (bundleManager != nullptr) {
        isSystemApp = bundleManager->IsSystemApp(callingUid);
    }

    return isSystemApp;
}

inline ErrCode AssignValidNotificationSlot(const std::shared_ptr<NotificationRecord> &record)
{
    sptr<NotificationSlot> slot;
    NotificationConstant::SlotType slotType = record->request->GetSlotType();
    ErrCode result = NotificationPreferences::GetInstance().GetNotificationSlot(record->bundleOption, slotType, slot);
    if ((result == ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST) ||
        (result == ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST)) {
        slot = new NotificationSlot(slotType);
        std::vector<sptr<NotificationSlot>> slots;
        slots.push_back(slot);
        result = NotificationPreferences::GetInstance().AddNotificationSlots(record->bundleOption, slots);
    }
    if (result == ERR_OK) {
        record->slot = slot;
    }
    return result;
}

inline ErrCode CheckPictureSize(const sptr<NotificationRequest> &request)
{
    ErrCode result = ERR_OK;

    auto content = request->GetContent();
    if (content != nullptr && content->GetContentType() == NotificationContent::Type::PICTURE) {
        std::shared_ptr<NotificationPictureContent> pictureContent =
            std::static_pointer_cast<NotificationPictureContent>(content->GetNotificationContent());
        if (pictureContent != nullptr) {
            auto picture = pictureContent->GetBigPicture();
            if (picture != nullptr && (uint32_t)picture->GetByteCount() > MAX_PICTURE_SIZE) {
                result = ERR_ANS_PICTURE_OVER_SIZE;
            }
        }
    }

    auto littleIcon = request->GetLittleIcon();
    if (littleIcon != nullptr && (uint32_t)littleIcon->GetByteCount() > MAX_ICON_SIZE) {
        result = ERR_ANS_ICON_OVER_SIZE;
    }

    auto bigIcon = request->GetBigIcon();
    if (bigIcon != nullptr && (uint32_t)bigIcon->GetByteCount() > MAX_ICON_SIZE) {
        result = ERR_ANS_ICON_OVER_SIZE;
    }

    return result;
}

inline ErrCode PrepereNotificationRequest(const sptr<NotificationRequest> &request)
{
    std::string bundle = GetClientBundleName();
    if (bundle.empty()) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    request->SetOwnerBundleName(bundle);
    request->SetCreatorBundleName(bundle);

    int uid = IPCSkeleton::GetCallingUid();
    int pid = IPCSkeleton::GetCallingPid();
    request->SetCreatorUid(uid);
    request->SetCreatorPid(pid);

    ErrCode result = CheckPictureSize(request);

    return result;
}

sptr<AdvancedNotificationService> AdvancedNotificationService::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);

    if (instance_ == nullptr) {
        instance_ = new AdvancedNotificationService();
    }
    return instance_;
}

AdvancedNotificationService::AdvancedNotificationService()
{
    runner_ = OHOS::AppExecFwk::EventRunner::Create();
    handler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner_);
    recentInfo_ = std::make_shared<RecentInfo>();
    distributedKvStoreDeathRecipient_ = std::make_shared<DistributedKvStoreDeathRecipient>(
        std::bind(&AdvancedNotificationService::OnDistributedKvStoreDeathRecipient, this));

    StartFilters();

    ISystemEvent iSystemEvent = {
        std::bind(&AdvancedNotificationService::OnBundleRemoved, this, std::placeholders::_1),
    };
    systemEventObserver_ = std::make_shared<SystemEventObserver>(iSystemEvent);

    dataManager_.RegisterKvStoreServiceDeathRecipient(distributedKvStoreDeathRecipient_);
}

AdvancedNotificationService::~AdvancedNotificationService()
{
    dataManager_.UnRegisterKvStoreServiceDeathRecipient(distributedKvStoreDeathRecipient_);

    StopFilters();
}

sptr<NotificationBundleOption> AdvancedNotificationService::GenerateBundleOption()
{
    sptr<NotificationBundleOption> bundleOption = nullptr;
    std::string bundle = GetClientBundleName();
    if (bundle.empty()) {
        return nullptr;
    }
    int uid = IPCSkeleton::GetCallingUid();
    bundleOption = new NotificationBundleOption(bundle, uid);
    return bundleOption;
}

sptr<NotificationBundleOption> AdvancedNotificationService::GenerateValidBundleOption(
    const sptr<NotificationBundleOption> &bundleOption)
{
    sptr<NotificationBundleOption> validBundleOption = nullptr;
    if (bundleOption->GetUid() <= 0) {
        std::shared_ptr<BundleManagerHelper> bundleManager = BundleManagerHelper::GetInstance();
        if (bundleManager != nullptr) {
            int uid = bundleManager->GetDefaultUidByBundleName(bundleOption->GetBundleName());
            if (uid > 0) {
                validBundleOption = new NotificationBundleOption(bundleOption->GetBundleName(), uid);
            }
        }
    } else {
        validBundleOption = bundleOption;
    }
    return validBundleOption;
}

ErrCode AdvancedNotificationService::Publish(const std::string &label, const sptr<NotificationRequest> &request)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if ((request->GetSlotType() == NotificationConstant::SlotType::CUSTOM) && !IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    ErrCode result = PrepereNotificationRequest(request);
    if (result != ERR_OK) {
        return result;
    }

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    std::shared_ptr<NotificationRecord> record = std::make_shared<NotificationRecord>();
    record->request = request;
    record->notification = new Notification(request);
    record->bundleOption = bundleOption;

    handler_->PostSyncTask(std::bind([&]() {
        result = AssignValidNotificationSlot(record);
        if (result != ERR_OK) {
            ANS_LOGE("Can not assign valid slot!");
            return;
        }

        result = Filter(record);
        if (result != ERR_OK) {
            ANS_LOGE("Reject by filters: %{public}d", result);
            return;
        }

        if (!IsNotificationExists(record->notification->GetKey())) {
            result = FlowControl(record);
            if (result != ERR_OK) {
                return;
            }
        } else {
            if (record->request->IsAlertOneTime()) {
                record->notification->SetEnableLight(false);
                record->notification->SetEnableSound(false);
                record->notification->SetEnableViration(false);
            }
            UpdateInNotificationList(record);
        }

        UpdateRecentNotification(record->notification, false, 0);
        sptr<NotificationSortingMap> sortingMap = GenerateSortingMap();
        NotificationSubscriberManager::GetInstance()->NotifyConsumed(record->notification, sortingMap);
    }));

    return result;
}

bool AdvancedNotificationService::IsNotificationExists(const std::string &key)
{
    bool isExists = false;

    for (auto item : notificationList_) {
        if (item->notification->GetKey() == key) {
            isExists = true;
            break;
        }
    }

    return isExists;
}

ErrCode AdvancedNotificationService::Filter(const std::shared_ptr<NotificationRecord> &record)
{
    ErrCode result = ERR_OK;

    for (auto filter : NOTIFICATION_FILTERS) {
        result = filter->OnPublish(record);
        if (result != ERR_OK) {
            break;
        }
    }

    return result;
}

void AdvancedNotificationService::AddToNotificationList(const std::shared_ptr<NotificationRecord> &record)
{
    notificationList_.push_back(record);
    SortNotificationList();
}

void AdvancedNotificationService::UpdateInNotificationList(const std::shared_ptr<NotificationRecord> &record)
{
    auto iter = notificationList_.begin();
    while (iter != notificationList_.end()) {
        if ((*iter)->notification->GetKey() == record->notification->GetKey()) {
            *iter = record;
            break;
        }
        iter++;
    }

    SortNotificationList();
}

void AdvancedNotificationService::SortNotificationList()
{
    notificationList_.sort(AdvancedNotificationService::NotificationCompare);
}

bool AdvancedNotificationService::NotificationCompare(
    const std::shared_ptr<NotificationRecord> &first, const std::shared_ptr<NotificationRecord> &second)
{
    // sorting notifications by create time
    return (first->request->GetCreateTime() < second->request->GetCreateTime());
}

sptr<NotificationSortingMap> AdvancedNotificationService::GenerateSortingMap()
{
    std::vector<NotificationSorting> sortingList;
    for (auto record : notificationList_) {
        NotificationSorting sorting;
        sorting.SetRanking((int32_t)sortingList.size());
        sorting.SetKey(record->notification->GetKey());
        sorting.SetSlot(record->slot);
        sortingList.push_back(sorting);
    }

    sptr<NotificationSortingMap> sortingMap = new NotificationSortingMap(sortingList);

    return sortingMap;
}

void AdvancedNotificationService::StartFilters()
{
    for (auto filter : NOTIFICATION_FILTERS) {
        filter->OnStart();
    }
}

void AdvancedNotificationService::StopFilters()
{
    for (auto filter : NOTIFICATION_FILTERS) {
        filter->OnStop();
    }
}

ErrCode AdvancedNotificationService::Cancel(int notificationId, const std::string &label)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        sptr<Notification> notification = nullptr;
        result = RemoveFromNotificationList(bundleOption, label, notificationId, notification, true);
        if (result != ERR_OK) {
            return;
        }

        if (notification != nullptr) {
            int reason = NotificationConstant::APP_CANCEL_REASON_DELETE;
            UpdateRecentNotification(notification, true, reason);
            sptr<NotificationSortingMap> sortingMap = GenerateSortingMap();
            NotificationSubscriberManager::GetInstance()->NotifyCanceled(notification, sortingMap, reason);
        }
    }));
    return result;
}

ErrCode AdvancedNotificationService::CancelAll()
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;

    handler_->PostSyncTask(std::bind([&]() {
        sptr<Notification> notification = nullptr;

        std::vector<std::string> keys = GetNotificationKeys(bundleOption);
        for (auto key : keys) {
            result = RemoveFromNotificationList(key, notification, true);
            if (result != ERR_OK) {
                continue;
            }

            if (notification != nullptr) {
                int reason = NotificationConstant::APP_CANCEL_ALL_REASON_DELETE;
                UpdateRecentNotification(notification, true, reason);
                sptr<NotificationSortingMap> sortingMap = GenerateSortingMap();
                NotificationSubscriberManager::GetInstance()->NotifyCanceled(notification, sortingMap, reason);
            }
        }

        result = ERR_OK;
    }));
    return result;
}

ErrCode AdvancedNotificationService::AddSlots(const std::vector<sptr<NotificationSlot>> &slots)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    if (slots.size() == 0) {
        return ERR_ANS_INVALID_PARAM;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        std::vector<sptr<NotificationSlot>> addSlots;
        for (auto slot : slots) {
            sptr<NotificationSlot> originalSlot;
            result =
                NotificationPreferences::GetInstance().GetNotificationSlot(bundleOption, slot->GetType(), originalSlot);
            if ((result == ERR_OK) && (originalSlot != nullptr)) {
                continue;
            } else {
                addSlots.push_back(slot);
            }
        }

        if (addSlots.size() == 0) {
            result = ERR_OK;
        } else {
            result = NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption, addSlots);
        }
    }));
    return result;
}

ErrCode AdvancedNotificationService::AddSlotGroups(std::vector<sptr<NotificationSlotGroup>> groups)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind(
        [&]() { result = NotificationPreferences::GetInstance().AddNotificationSlotGroups(bundleOption, groups); }));
    return result;
}

ErrCode AdvancedNotificationService::GetSlots(std::vector<sptr<NotificationSlot>> &slots)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind(
        [&]() { result = NotificationPreferences::GetInstance().GetNotificationAllSlots(bundleOption, slots); }));
    return result;
}

ErrCode AdvancedNotificationService::GetSlotGroup(const std::string &groupId, sptr<NotificationSlotGroup> &group)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        result = NotificationPreferences::GetInstance().GetNotificationSlotGroup(bundleOption, groupId, group);
    }));
    return result;
}

ErrCode AdvancedNotificationService::GetSlotGroups(std::vector<sptr<NotificationSlotGroup>> &groups)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind(
        [&]() { result = NotificationPreferences::GetInstance().GetNotificationAllSlotGroups(bundleOption, groups); }));
    return result;
}

ErrCode AdvancedNotificationService::RemoveSlotGroups(const std::vector<std::string> &groupIds)
{
    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        result = NotificationPreferences::GetInstance().RemoveNotificationSlotGroups(bundleOption, groupIds);
    }));
    return result;
}

ErrCode AdvancedNotificationService::GetActiveNotifications(std::vector<sptr<NotificationRequest>> &notifications)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        notifications.clear();
        for (auto record : notificationList_) {
            if ((record->bundleOption->GetBundleName() == bundleOption->GetBundleName()) &&
                (record->bundleOption->GetUid() == bundleOption->GetUid())) {
                notifications.push_back(record->request);
            }
        }
    }));
    return result;
}

ErrCode AdvancedNotificationService::GetActiveNotificationNums(int &num)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        int count = 0;
        for (auto record : notificationList_) {
            if ((record->bundleOption->GetBundleName() == bundleOption->GetBundleName()) &&
                (record->bundleOption->GetUid() == bundleOption->GetUid())) {
                count += 1;
            }
        }
        num = count;
    }));
    return result;
}

ErrCode AdvancedNotificationService::SetNotificationAgent(const std::string &agent)
{
    return ERR_INVALID_OPERATION;
}
ErrCode AdvancedNotificationService::GetNotificationAgent(std::string &agent)
{
    return ERR_INVALID_OPERATION;
}
ErrCode AdvancedNotificationService::CanPublishAsBundle(const std::string &representativeBundle, bool &canPublish)
{
    return ERR_INVALID_OPERATION;
}
ErrCode AdvancedNotificationService::PublishAsBundle(
    const sptr<NotificationRequest> notification, const std::string &representativeBundle)
{
    return ERR_INVALID_OPERATION;
}

ErrCode AdvancedNotificationService::SetNotificationBadgeNum(int num)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(
        std::bind([&]() { result = NotificationPreferences::GetInstance().SetTotalBadgeNums(bundleOption, num); }));
    return result;
}

ErrCode AdvancedNotificationService::GetBundleImportance(int &importance)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(
        std::bind([&]() { result = NotificationPreferences::GetInstance().GetImportance(bundleOption, importance); }));
    return result;
}

ErrCode AdvancedNotificationService::SetDisturbMode(NotificationConstant::DisturbMode mode)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        result = NotificationPreferences::GetInstance().SetDisturbMode(mode);
        if (result == ERR_OK) {
            NotificationSubscriberManager::GetInstance()->NotifyDisturbModeChanged(mode);
        }
    }));
    return result;
}

ErrCode AdvancedNotificationService::GetDisturbMode(NotificationConstant::DisturbMode &mode)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() { result = NotificationPreferences::GetInstance().GetDisturbMode(mode); }));
    return result;
}

ErrCode AdvancedNotificationService::HasNotificationPolicyAccessPermission(bool &granted)
{
    return ERR_OK;
}

ErrCode AdvancedNotificationService::SetPrivateNotificationsAllowed(bool allow)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        result = NotificationPreferences::GetInstance().SetPrivateNotificationsAllowed(bundleOption, allow);
    }));
    return result;
}

ErrCode AdvancedNotificationService::GetPrivateNotificationsAllowed(bool &allow)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        result = NotificationPreferences::GetInstance().GetPrivateNotificationsAllowed(bundleOption, allow);
    }));
    return result;
}

ErrCode AdvancedNotificationService::Delete(const std::string &key)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    ErrCode result = ERR_OK;

    handler_->PostSyncTask(std::bind([&]() {
        sptr<Notification> notification = nullptr;
        result = RemoveFromNotificationList(key, notification);
        if (result != ERR_OK) {
            return;
        }

        if (notification != nullptr) {
            int reason = NotificationConstant::CANCEL_REASON_DELETE;
            UpdateRecentNotification(notification, true, reason);
            sptr<NotificationSortingMap> sortingMap = GenerateSortingMap();
            NotificationSubscriberManager::GetInstance()->NotifyCanceled(notification, sortingMap, reason);
        }
    }));

    return result;
}

ErrCode AdvancedNotificationService::DeleteByBundle(const sptr<NotificationBundleOption> &bundleOption)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        std::vector<std::string> keys = GetNotificationKeys(bundle);
        for (auto key : keys) {
            sptr<Notification> notification = nullptr;

            result = RemoveFromNotificationList(key, notification);
            if (result != ERR_OK) {
                continue;
            }

            if (notification != nullptr) {
                int reason = NotificationConstant::CANCEL_REASON_DELETE;
                UpdateRecentNotification(notification, true, reason);
                sptr<NotificationSortingMap> sortingMap = GenerateSortingMap();
                NotificationSubscriberManager::GetInstance()->NotifyCanceled(notification, sortingMap, reason);
            }
        }

        result = ERR_OK;
    }));

    return result;
}

ErrCode AdvancedNotificationService::DeleteAll()
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        std::vector<std::string> keys = GetNotificationKeys(nullptr);
        for (auto key : keys) {
            sptr<Notification> notification = nullptr;

            result = RemoveFromNotificationList(key, notification);
            if (result != ERR_OK) {
                continue;
            }

            if (notification != nullptr) {
                int reason = NotificationConstant::CANCEL_ALL_REASON_DELETE;
                UpdateRecentNotification(notification, true, reason);
                sptr<NotificationSortingMap> sortingMap = GenerateSortingMap();
                NotificationSubscriberManager::GetInstance()->NotifyCanceled(notification, sortingMap, reason);
            }
        }

        result = ERR_OK;
    }));

    return result;
}

std::vector<std::string> AdvancedNotificationService::GetNotificationKeys(
    const sptr<NotificationBundleOption> &bundleOption)
{
    std::vector<std::string> keys;

    for (auto record : notificationList_) {
        if ((bundleOption != nullptr) && (record->bundleOption->GetBundleName() != bundleOption->GetBundleName()) &&
            (record->bundleOption->GetUid() != bundleOption->GetUid())) {
            continue;
        }
        keys.push_back(record->notification->GetKey());
    }

    return keys;
}

ErrCode AdvancedNotificationService::GetSlotsByBundle(
    const sptr<NotificationBundleOption> &bundleOption, std::vector<sptr<NotificationSlot>> &slots)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(
        std::bind([&]() { result = NotificationPreferences::GetInstance().GetNotificationAllSlots(bundle, slots); }));
    return result;
}

ErrCode AdvancedNotificationService::UpdateSlots(
    const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlot>> &slots)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(
        std::bind([&]() { result = NotificationPreferences::GetInstance().UpdateNotificationSlots(bundle, slots); }));
    return result;
}

ErrCode AdvancedNotificationService::UpdateSlotGroups(
    const sptr<NotificationBundleOption> &bundleOption, const std::vector<sptr<NotificationSlotGroup>> &groups)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind(
        [&]() { result = NotificationPreferences::GetInstance().UpdateNotificationSlotGroups(bundle, groups); }));
    return result;
}

ErrCode AdvancedNotificationService::SetNotificationsEnabledForBundle(const std::string &deviceId, bool enabled)
{
    return ERR_INVALID_OPERATION;
}

ErrCode AdvancedNotificationService::SetShowBadgeEnabledForBundle(
    const sptr<NotificationBundleOption> &bundleOption, bool enabled)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(
        std::bind([&]() { result = NotificationPreferences::GetInstance().SetShowBadge(bundle, enabled); }));
    return result;
}

ErrCode AdvancedNotificationService::GetShowBadgeEnabledForBundle(
    const sptr<NotificationBundleOption> &bundleOption, bool &enabled)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(
        std::bind([&]() { result = NotificationPreferences::GetInstance().IsShowBadge(bundle, enabled); }));
    return result;
}

ErrCode AdvancedNotificationService::GetShowBadgeEnabled(bool &enabled)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(
        std::bind([&]() { result = NotificationPreferences::GetInstance().IsShowBadge(bundleOption, enabled); }));
    return result;
}

ErrCode AdvancedNotificationService::RemoveFromNotificationList(const sptr<NotificationBundleOption> &bundleOption,
    const std::string &label, int notificationId, sptr<Notification> &notification, bool isCancel)
{
    for (auto record : notificationList_) {
        if (((record->bundleOption->GetBundleName() == bundleOption->GetBundleName()) &&
                (record->bundleOption->GetUid() == bundleOption->GetUid())) &&
            (record->notification->GetLabel() == label) && (record->notification->GetId() == notificationId)) {
            if (!isCancel && record->request->IsUnremovable()) {
                return ERR_ANS_NOTIFICATION_IS_UNREMOVABLE;
            }
            notification = record->notification;
            notificationList_.remove(record);
            return ERR_OK;
        }
    }

    return ERR_ANS_NOTIFICATION_NOT_EXISTS;
}

ErrCode AdvancedNotificationService::RemoveFromNotificationList(
    const std::string &key, sptr<Notification> &notification, bool isCancel)
{
    for (auto record : notificationList_) {
        if (record->notification->GetKey() == key) {
            if (!isCancel && record->request->IsUnremovable()) {
                return ERR_ANS_NOTIFICATION_IS_UNREMOVABLE;
            }
            notification = record->notification;
            notificationList_.remove(record);
            return ERR_OK;
        }
    }

    return ERR_ANS_NOTIFICATION_NOT_EXISTS;
}

ErrCode AdvancedNotificationService::Subscribe(
    const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (subscriber == nullptr) {
        return ERR_ANS_INVALID_PARAM;
    }

    if (!IsSystemApp()) {
        ANS_LOGE("Client is not a system app");
        return ERR_ANS_NON_SYSTEM_APP;
    }

    return NotificationSubscriberManager::GetInstance()->AddSubscriber(subscriber, info);
}

ErrCode AdvancedNotificationService::Unsubscribe(
    const sptr<IAnsSubscriber> &subscriber, const sptr<NotificationSubscribeInfo> &info)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (subscriber == nullptr) {
        ANS_LOGE("Client is not a system app");
        return ERR_ANS_INVALID_PARAM;
    }

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    return NotificationSubscriberManager::GetInstance()->RemoveSubscriber(subscriber, info);
}

ErrCode AdvancedNotificationService::GetSlotByType(
    const NotificationConstant::SlotType &slotType, sptr<NotificationSlot> &slot)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind(
        [&]() { result = NotificationPreferences::GetInstance().GetNotificationSlot(bundleOption, slotType, slot); }));
    return result;
}

ErrCode AdvancedNotificationService::RemoveSlotByType(const NotificationConstant::SlotType &slotType)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind(
        [&]() { result = NotificationPreferences::GetInstance().RemoveNotificationSlot(bundleOption, slotType); }));
    return result;
}

ErrCode AdvancedNotificationService::GetAllActiveNotifications(std::vector<sptr<Notification>> &notifications)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        notifications.clear();
        for (auto record : notificationList_) {
            notifications.push_back(record->notification);
        }
    }));
    return result;
}

inline bool IsContained(const std::vector<std::string> &vec, const std::string &target)
{
    bool isContained = false;

    auto iter = vec.begin();
    while (iter != vec.end()) {
        if (*iter == target) {
            isContained = true;
            break;
        }
        iter++;
    }

    return isContained;
}

ErrCode AdvancedNotificationService::GetSpecialActiveNotifications(
    const std::vector<std::string> &key, std::vector<sptr<Notification>> &notifications)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        for (auto record : notificationList_) {
            if (IsContained(key, record->notification->GetKey())) {
                notifications.push_back(record->notification);
            }
        }
    }));
    return result;
}

ErrCode AdvancedNotificationService::SetNotificationsEnabledForAllBundles(const std::string &deviceId, bool enabled)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        if (deviceId.empty()) {
            // Local device
            result = NotificationPreferences::GetInstance().SetNotificationsEnabled(enabled);
        } else {
            // Remote device
        }
    }));
    return result;
}

ErrCode AdvancedNotificationService::SetNotificationsEnabledForSpecialBundle(
    const std::string &deviceId, const sptr<NotificationBundleOption> &bundleOption, bool enabled)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        if (deviceId.empty()) {
            // Local device
            result = NotificationPreferences::GetInstance().SetNotificationsEnabledForBundle(bundle, enabled);
        } else {
            // Remote revice
        }
    }));
    return result;
}

ErrCode AdvancedNotificationService::IsAllowedNotify(bool &allowed)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        allowed = false;
        result = NotificationPreferences::GetInstance().GetNotificationsEnabled(allowed);
    }));
    return result;
}

ErrCode AdvancedNotificationService::IsSpecialBundleAllowedNotify(
    const sptr<NotificationBundleOption> &bundleOption, bool &allowed)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> clientBundle = GenerateBundleOption();
    if (clientBundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    sptr<NotificationBundleOption> targetBundle = nullptr;
    if (bundleOption == nullptr) {
        targetBundle = clientBundle;
    } else {
        if ((clientBundle->GetBundleName() == bundleOption->GetBundleName()) &&
            (clientBundle->GetUid() == bundleOption->GetUid())) {
            targetBundle = bundleOption;
        } else {
            if (!IsSystemApp()) {
                return ERR_ANS_NON_SYSTEM_APP;
            }
            targetBundle = GenerateValidBundleOption(bundleOption);
        }
    }

    if (targetBundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        allowed = false;
        result = NotificationPreferences::GetInstance().GetNotificationsEnabled(allowed);
        if (result == ERR_OK && allowed) {
            result = NotificationPreferences::GetInstance().GetNotificationsEnabledForBundle(targetBundle, allowed);
        }
    }));
    return result;
}

ErrCode AdvancedNotificationService::ShellDump(const std::string &dumpOption, std::vector<std::string> &dumpInfo)
{
    ANS_LOGD("%{public}s", __FUNCTION__);
    ErrCode result = ERR_ANS_NOT_ALLOWED;

    handler_->PostSyncTask(std::bind([&]() {
        if (dumpOption == ACTIVE_NOTIFICATION_OPTION) {
            result = ActiveNotificationDump(dumpInfo);
        } else if (dumpOption == RECENT_NOTIFICATION_OPTION) {
            result = RecentNotificationDump(dumpInfo);
        } else if (dumpOption.substr(0, dumpOption.find_first_of(" ", 0)) == SET_RECENT_COUNT_OPTION) {
            result = SetRecentNotificationCount(dumpOption.substr(dumpOption.find_first_of(" ", 0) + 1));
        } else {
            result = ERR_ANS_INVALID_PARAM;
        }
    }));

    return result;
}

ErrCode AdvancedNotificationService::ActiveNotificationDump(std::vector<std::string> &dumpInfo)
{
    ANS_LOGD("%{public}s", __FUNCTION__);
    std::stringstream stream;
    for (auto record : notificationList_) {
        stream.clear();
        stream << "\tBundleName: " << record->notification->GetBundleName() << "\n";

        stream << "\tCreateTime: " << TimeToString(record->notification->GetNotificationRequest().GetCreateTime())
               << "\n";

        stream << "\tNotification:\n";
        stream << "\t\tId: " << record->notification->GetId() << "\n";
        stream << "\t\tLabel: " << record->notification->GetLabel() << "\n";
        stream << "\t\tClassification: " << record->notification->GetNotificationRequest().GetClassification() << "\n";

        dumpInfo.push_back(stream.str());
    }

    return ERR_OK;
}

ErrCode AdvancedNotificationService::RecentNotificationDump(std::vector<std::string> &dumpInfo)
{
    ANS_LOGD("%{public}s", __FUNCTION__);
    std::stringstream stream;
    for (auto recentNotification : recentInfo_->list) {
        stream.clear();
        stream << "\tBundleName: " << recentNotification->notification->GetBundleName() << "\n";

        stream << "\tCreateTime: "
               << TimeToString(recentNotification->notification->GetNotificationRequest().GetCreateTime()) << "\n";

        stream << "\tNotification:\n";
        stream << "\t\tId: " << recentNotification->notification->GetId() << "\n";
        stream << "\t\tLabel: " << recentNotification->notification->GetLabel() << "\n";
        stream << "\t\tClassification: "
               << recentNotification->notification->GetNotificationRequest().GetClassification() << "\n";

        if (!recentNotification->isActive) {
            stream << "\t DeleteTime: " << TimeToString(recentNotification->deleteTime) << "\n";
            stream << "\t DeleteReason: " << recentNotification->deleteReason << "\n";
        }

        dumpInfo.push_back(stream.str());
    }
    return ERR_OK;
}

ErrCode AdvancedNotificationService::SetRecentNotificationCount(const std::string arg)
{
    ANS_LOGD("%{public}s arg = %{public}s", __FUNCTION__, arg.c_str());
    int count = atoi(arg.c_str());

    if (count < NOTIFICATION_MIN_COUNT || count > NOTIFICATION_MAX_COUNT) {
        return ERR_ANS_INVALID_PARAM;
    }

    recentInfo_->recentCount = count;
    while (recentInfo_->list.size() > recentInfo_->recentCount) {
        recentInfo_->list.pop_back();
    }
    return ERR_OK;
}

std::string AdvancedNotificationService::TimeToString(int64_t time)
{
    auto timePoint = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(time));
    auto timeT = std::chrono::system_clock::to_time_t(timePoint);

    std::stringstream stream;
    stream << std::put_time(std::localtime(&timeT), "%F, %T");
    return stream.str();
}

int64_t AdvancedNotificationService::GetNowSysTime()
{
    std::chrono::time_point<std::chrono::system_clock> nowSys = std::chrono::system_clock::now();
    auto epoch = nowSys.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    int64_t duration = value.count();
    return duration;
}

void AdvancedNotificationService::UpdateRecentNotification(sptr<Notification> &notification, bool isDelete, int reason)
{
    for (auto recentNotification : recentInfo_->list) {
        if (recentNotification->notification->GetKey() == notification->GetKey()) {
            if (!isDelete) {
                recentInfo_->list.remove(recentNotification);
                recentNotification->isActive = true;
                recentNotification->notification = notification;
                recentInfo_->list.emplace_front(recentNotification);
            } else {
                recentNotification->isActive = false;
                recentNotification->deleteReason = reason;
                recentNotification->deleteTime = GetNowSysTime();
            }
            return;
        }
    }

    if (!isDelete) {
        if (recentInfo_->list.size() >= recentInfo_->recentCount) {
            recentInfo_->list.pop_back();
        }
        auto recentNotification = std::make_shared<RecentNotification>();
        recentNotification->isActive = true;
        recentNotification->notification = notification;
        recentInfo_->list.emplace_front(recentNotification);
    }
}

inline void RemoveExpired(
    std::list<std::chrono::system_clock::time_point> &list, const std::chrono::system_clock::time_point &now)
{
    auto iter = list.begin();
    while (iter != list.end()) {
        if (now - *iter > std::chrono::seconds(1)) {
            iter = list.erase(iter);
        } else {
            break;
        }
    }
}

static bool SortNotificationsByLevelAndTime(
    const std::shared_ptr<NotificationRecord> &first, const std::shared_ptr<NotificationRecord> &second)
{
    if (first->slot->GetLevel() != second->slot->GetLevel()) {
        return (first->slot->GetLevel() < second->slot->GetLevel());
    } else {
        return (first->request->GetCreateTime() < second->request->GetCreateTime());
    }
}

ErrCode AdvancedNotificationService::FlowControl(const std::shared_ptr<NotificationRecord> &record)
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    RemoveExpired(flowControlTimestampList_, now);
    if (flowControlTimestampList_.size() >= MAX_ACTIVE_NUM_PERSECOND) {
        return ERR_ANS_OVER_MAX_ACITVE_PERSECOND;
    }

    flowControlTimestampList_.push_back(now);

    std::list<std::shared_ptr<NotificationRecord>> bundleList;
    for (auto item : notificationList_) {
        if (record->notification->GetBundleName() == item->notification->GetBundleName()) {
            bundleList.push_back(item);
        }
    }

    if (bundleList.size() >= MAX_ACTIVE_NUM_PERAPP) {
        bundleList.sort(SortNotificationsByLevelAndTime);
        notificationList_.remove(bundleList.front());
    }

    if (notificationList_.size() >= MAX_ACTIVE_NUM) {
        if (bundleList.size() > 0) {
            bundleList.sort(SortNotificationsByLevelAndTime);
            notificationList_.remove(bundleList.front());
        } else {
            std::list<std::shared_ptr<NotificationRecord>> sorted = notificationList_;
            sorted.sort(SortNotificationsByLevelAndTime);
            notificationList_.remove(sorted.front());
        }
    }

    AddToNotificationList(record);

    return ERR_OK;
}

void AdvancedNotificationService::OnBundleRemoved(const sptr<NotificationBundleOption> &bundleOption)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    handler_->PostTask(std::bind([&]() {
        ErrCode result = NotificationPreferences::GetInstance().RemoveNotificationForBundle(bundleOption);
        if (result != ERR_OK) {
            ANS_LOGE("NotificationPreferences::RemoveNotificationForBundle failed: %{public}d", result);
        }
    }));
}

void AdvancedNotificationService::OnDistributedKvStoreDeathRecipient()
{
    ANS_LOGD("%{public}s", __FUNCTION__);
    handler_->PostTask(
        std::bind([&]() { NotificationPreferences::GetInstance().OnDistributedKvStoreDeathRecipient(); }));
}

ErrCode AdvancedNotificationService::RemoveAllSlots()
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(
        std::bind([&]() { result = NotificationPreferences::GetInstance().RemoveNotificationAllSlots(bundleOption); }));
    return result;
}

ErrCode AdvancedNotificationService::AddSlotByType(NotificationConstant::SlotType slotType)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    sptr<NotificationBundleOption> bundleOption = GenerateBundleOption();
    if (bundleOption == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;
    handler_->PostSyncTask(std::bind([&]() {
        sptr<NotificationSlot> slot;
        result = NotificationPreferences::GetInstance().GetNotificationSlot(bundleOption, slotType, slot);
        if ((result == ERR_OK) && (slot != nullptr)) {
            return;
        } else {
            slot = new NotificationSlot(slotType);
            std::vector<sptr<NotificationSlot>> slots;
            slots.push_back(slot);
            result = NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption, slots);
        }
    }));
    return result;
}

ErrCode AdvancedNotificationService::RemoveNotification(
    const sptr<NotificationBundleOption> &bundleOption, int notificationId, const std::string &label)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_ANS_NOTIFICATION_NOT_EXISTS;

    handler_->PostSyncTask(std::bind([&]() {
        sptr<Notification> notification = nullptr;

        for (auto record : notificationList_) {
            if ((record->bundleOption->GetBundleName() == bundle->GetBundleName()) &&
                (record->bundleOption->GetUid() == bundleOption->GetUid()) &&
                (record->notification->GetId() == notificationId) && (record->notification->GetLabel() == label)) {
                if (record->request->IsUnremovable()) {
                    result = ERR_ANS_NOTIFICATION_IS_UNREMOVABLE;
                    break;
                }
                notification = record->notification;
                notificationList_.remove(record);
                result = ERR_OK;
                break;
            }
        }

        if (notification != nullptr) {
            int reason = NotificationConstant::CANCEL_REASON_DELETE;
            UpdateRecentNotification(notification, true, reason);
            sptr<NotificationSortingMap> sortingMap = GenerateSortingMap();
            NotificationSubscriberManager::GetInstance()->NotifyCanceled(notification, sortingMap, reason);
        }
    }));

    return result;
}

ErrCode AdvancedNotificationService::RemoveAllNotifications(const sptr<NotificationBundleOption> &bundleOption)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    handler_->PostSyncTask(std::bind([&]() {
        std::vector<std::shared_ptr<NotificationRecord>> removeList;
        for (auto record : notificationList_) {
            if ((record->bundleOption->GetBundleName() == bundleOption->GetBundleName()) &&
                (record->bundleOption->GetUid() == bundleOption->GetUid()) && !record->request->IsUnremovable()) {
                removeList.push_back(record);
            }
        }

        for (auto record : removeList) {
            notificationList_.remove(record);

            if (record->notification != nullptr) {
                int reason = NotificationConstant::CANCEL_REASON_DELETE;
                UpdateRecentNotification(record->notification, true, reason);
                sptr<NotificationSortingMap> sortingMap = GenerateSortingMap();
                NotificationSubscriberManager::GetInstance()->NotifyCanceled(record->notification, sortingMap, reason);
            }
        }
    }));

    return ERR_OK;
}

ErrCode AdvancedNotificationService::GetSlotNumAsBundle(const sptr<NotificationBundleOption> &bundleOption, int &num)
{
    ANS_LOGD("%{public}s", __FUNCTION__);

    if (!IsSystemApp()) {
        return ERR_ANS_NON_SYSTEM_APP;
    }

    sptr<NotificationBundleOption> bundle = GenerateValidBundleOption(bundleOption);
    if (bundle == nullptr) {
        return ERR_ANS_INVALID_BUNDLE;
    }

    ErrCode result = ERR_OK;

    handler_->PostSyncTask(std::bind(
        [&]() { result = NotificationPreferences::GetInstance().GetNotificationSlotsNumForBundle(bundle, num); }));

    return result;
}

}  // namespace Notification
}  // namespace OHOS
