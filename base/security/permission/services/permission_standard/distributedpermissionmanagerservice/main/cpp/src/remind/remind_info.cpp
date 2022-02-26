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

#include "remind_info.h"
#include "notification_helper.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "RemindInfo"};
}
RemindInfo &RemindInfo::GetInstance()
{
    static RemindInfo instance;
    return instance;
}

/**
 * Remove permission reminder infomation when the device is disconnection
 *
 * @param deviceId the deviceId
 */
void RemindInfo::DeviceDisConnection(const std::string &deviceId)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    std::vector<PermissionRemindInfo>::iterator item;
    for (item = permRemindInfos_.begin(); item != permRemindInfos_.end(); item++) {
        if (item->deviceId == deviceId) {

            PERMISSION_LOG_INFO(LABEL, "The Stopping NotificationInfo :");
            PERMISSION_LOG_INFO(LABEL, "deviceId        : %{public}s" , deviceId.c_str());
            PERMISSION_LOG_INFO(LABEL, "notificationId  : %{public}d" , item->notificationId);

            for (auto perm = item->permissions.begin(); perm != item->permissions.end(); perm++) {
                PERMISSION_LOG_INFO(LABEL, "permissionName  : %{public}s" , perm->c_str());
            }

            Notification::NotificationHelper::CancelNotification(item->label, item->notificationId);
            permRemindInfos_.erase(item);
        }
    }
}

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS