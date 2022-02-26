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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_SERVICES_NOTIFICATION_PREFERENCES_INFO_H
#define BASE_NOTIFICATION_ANS_STANDARD_SERVICES_NOTIFICATION_PREFERENCES_INFO_H

#include <map>
#include <string>
#include <vector>

#include "notification_bundle_option.h"
#include "notification_slot.h"
#include "notification_slot_group.h"
#include "preferences_constant.h"

namespace OHOS {
namespace Notification {
class NotificationPreferencesInfo final {
public:
    class BundleInfo final {
    public:
        BundleInfo();
        ~BundleInfo();
        void SetBundleName(const std::string &name);
        std::string GetBundleName() const;
        void SetImportance(const int &level);
        int GetImportance() const;
        void SetIsShowBadge(const bool &isShowBadge);
        bool GetIsShowBadge() const;
        void SetBadgeTotalNum(const int &num);
        int GetBadgeTotalNum() const;
        void SetIsPrivateAllowed(const bool &isPrivateAllowed);
        bool GetIsPrivateAllowed() const;
        void SetEnableNotification(const bool &enable);
        bool GetEnableNotification() const;
        void SetSlot(const sptr<NotificationSlot> &slot);
        bool GetSlot(const NotificationConstant::SlotType &type, sptr<NotificationSlot> &slot) const;
        bool GetAllSlots(std::vector<sptr<NotificationSlot>> &slots);
        uint32_t GetAllSlotsSize();
        bool GetAllSlotsInGroup(const std::string &groupId, std::vector<sptr<NotificationSlot>> &slots);
        bool GetAllSlotsInGroup(const std::string &groupId, std::vector<NotificationSlot> &slots);
        void SetGroup(const sptr<NotificationSlotGroup> &group);
        bool GetGroup(const std::string &groupId, sptr<NotificationSlotGroup> &group);
        bool GetAllGroups(std::vector<sptr<NotificationSlotGroup>> &group);
        uint32_t GetGroupSize() const;
        bool IsExsitSlot(const NotificationConstant::SlotType &type) const;
        bool IsExsitSlotGroup(const std::string &groupId) const;
        bool RemoveSlot(const NotificationConstant::SlotType &type);
        void RemoveAllSlots();
        bool RemoveSlotGroup(const std::string &groupId);
        void SetBundleUid(const int &uid);
        int GetBundleUid() const;

    private:
        std::string bundleName_;
        int uid_ = 0;
        int importance_ = BUNDLE_IMPORTANCE;
        bool isShowBadge_ = BUNDLE_SHOW_BADGE;
        int badgeTotalNum_ = BUNDLE_BADGE_TOTAL_NUM;
        bool isPrivateAllowed_ = BUNDLE_PRIVATE_ALLOWED;
        bool isEnabledNotification_ = BUNDLE_ENABLE_NOTIFICATION;
        std::map<NotificationConstant::SlotType, sptr<NotificationSlot>> slots_;
        std::map<std::string, sptr<NotificationSlotGroup>> groups_;
    };

    NotificationPreferencesInfo()
    {}
    ~NotificationPreferencesInfo()
    {}
    void SetEnabledAllNotification(const bool &value);
    bool GetEnabledAllNotification() const;
    void SetDisturbMode(const NotificationConstant::DisturbMode &mode);
    NotificationConstant::DisturbMode GetDisturbMode() const;
    void SetBundleInfo(const BundleInfo &info);
    bool GetBundleInfo(const sptr<NotificationBundleOption> &bundleOption, BundleInfo &info) const;
    bool RemoveBundleInfo(const sptr<NotificationBundleOption> &bundleOption);
    bool IsExsitBundleInfo(const sptr<NotificationBundleOption> &bundleOption) const;
    void ClearBundleInfo();

private:
    bool isEnabledAllNotification_ = true;
    NotificationConstant::DisturbMode disturbMode_ = NotificationConstant::DisturbMode::ALLOW_ALL;
    std::map<std::string, BundleInfo> infos_;
};
}  // namespace Notification
}  // namespace OHOS
#endif  // BASE_NOTIFICATION_ANS_STANDARD_SERVICES_NOTIFICATION_PREFERENCES_INFO_H