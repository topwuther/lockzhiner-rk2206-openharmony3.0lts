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

#include "disturb_filter.h"

#include "ans_inner_errors.h"
#include "notification_preferences.h"

namespace OHOS {
namespace Notification {

void DisturbFilter::OnStart()
{}

void DisturbFilter::OnStop()
{}

ErrCode DisturbFilter::OnPublish(const std::shared_ptr<NotificationRecord> &record)
{
    NotificationConstant::DisturbMode mode;
    ErrCode result = NotificationPreferences::GetInstance().GetDisturbMode(mode);
    if (result != ERR_OK) {
        return result;
    }

    bool disturb = false;

    switch (mode) {
        case NotificationConstant::DisturbMode::ALLOW_ALARMS:
            if (record->request->GetClassification() == NotificationRequest::CLASSIFICATION_ALARM) {
                disturb = true;
            } else {
                if (record->slot == nullptr) {
                    result = ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_NOT_EXIST;
                    break;
                }
                disturb = record->slot->IsEnableBypassDnd();
            }
            break;
        case NotificationConstant::DisturbMode::ALLOW_ALL:
            disturb = true;
            break;
        case NotificationConstant::DisturbMode::ALLOW_NONE:
            if (record->slot == nullptr) {
                result = ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_NOT_EXIST;
                break;
            }
            disturb = record->slot->IsEnableBypassDnd();
            break;
        case NotificationConstant::DisturbMode::ALLOW_PRIORITY:
            break;
        case NotificationConstant::DisturbMode::ALLOW_UNKNOWN:
            // DO NOTHING
            break;
        default:
            break;
    }

    if (!disturb) {
        record->notification->SetEnableLight(false);
        record->notification->SetEnableSound(false);
        record->notification->SetEnableViration(false);
    }

    return result;
}

}  // namespace Notification
}  // namespace OHOS