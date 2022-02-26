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

#include "permission_change_receiver.h"
#include "common_event_support.h"
#include "monitor_manager.h"
#include "subject_device_permission_manager.h"
#include "object_device_permission_manager.h"
#include "permission_bms_manager.h"

namespace OHOS {
namespace Security {
namespace Permission {

namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionChangeReceiver"};
}  // namespace

void PermissionChangeReceiver::Register()
{
    const std::vector<std::string> eventList = {
        EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED,
    };

    EventFwk::MatchingSkills matchingSkills;
    for (const auto &e : eventList) {
        matchingSkills.AddEvent(e);
    }

    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    std::shared_ptr<AppEventSubscriber> subscriber_ = std::make_shared<AppEventSubscriber>(subscribeInfo);
    EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void PermissionChangeReceiver::OnReceive(const EventFwk::CommonEventData &data)
{
    int uid = 0;
    std::string key = "uid";

    auto eventName = data.GetWant().GetAction();
    uid = data.GetWant().GetIntParam(key, uid);

    if (eventName.compare(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) == 0) {
        // uid -> userId
        UserRemoved(uid / Constant::PER_USER_RANGE);
    } else if (eventName.compare(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED) == 0) {
        PackageAdded(uid);
    } else if (eventName.compare(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) == 0) {
        PackageRemoved(uid);
    } else if (eventName.compare(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REPLACED) == 0) {
        PackageReplaced(uid);
    } else {
        PERMISSION_LOG_DEBUG(LABEL, "event(%{public}s) ignored", eventName.c_str());
    }
}

void PermissionChangeReceiver::AppEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    OnReceive(data);
}

/////////////// private methods

void PermissionChangeReceiver::PackageAdded(int32_t uid)
{
    PERMISSION_LOG_DEBUG(LABEL, "package added event, uid: %{public}d", uid);
    MonitorManager::GetInstance().OnPermissionChange(uid, "");
    SubjectDevicePermissionManager::GetInstance().ReGrantAllDuidPermissions();
}

void PermissionChangeReceiver::PackageRemoved(int32_t uid)
{
    PERMISSION_LOG_DEBUG(LABEL, "package removed event, uid: %{public}d", uid);
    MonitorManager::GetInstance().OnPermissionChange(-uid, "");
    SubjectDevicePermissionManager::GetInstance().ReGrantAllDuidPermissions();
    PermissionRecordManager::GetInstance().DeletePermissionUsedRecords(uid);
}

void PermissionChangeReceiver::PackageReplaced(int32_t uid)
{
    PERMISSION_LOG_DEBUG(LABEL, "package replaced event, uid: %{public}d", uid);
    MonitorManager::GetInstance().OnPermissionChange(uid, "");
    SubjectDevicePermissionManager::GetInstance().ReGrantAllDuidPermissions();
}

void PermissionChangeReceiver::UserRemoved(int32_t userId)
{
    PERMISSION_LOG_DEBUG(LABEL, "user removed, userId: %{public}d", userId);
    ObjectDevicePermissionManager::GetInstance().RemoveNotifyPermissionMonitorUserId(userId);
}

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS