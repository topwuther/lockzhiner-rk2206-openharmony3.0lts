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

#include "permission_filter.h"

#include "ans_inner_errors.h"
#include "ans_log_wrapper.h"
#include "notification_preferences.h"

namespace OHOS {
namespace Notification {

void PermissionFilter::OnStart()
{}

void PermissionFilter::OnStop()
{}

ErrCode PermissionFilter::OnPublish(const std::shared_ptr<NotificationRecord> &record)
{
    bool enable = false;
    ErrCode result =
        NotificationPreferences::GetInstance().GetNotificationsEnabledForBundle(record->bundleOption, enable);
    if (result == ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST) {
        result = ERR_OK;
        enable = true;
    }
    if (result == ERR_OK) {
        if (!enable) {
            ANS_LOGE("Enable notifications for bundle is OFF");
            return ERR_ANS_NOT_ALLOWED;
        }

        if (record->notification->GetBundleName() != record->notification->GetCreateBundle()) {
            // Publish as bundle
        }
    }
    return result;
}

}  // namespace Notification
}  // namespace OHOS