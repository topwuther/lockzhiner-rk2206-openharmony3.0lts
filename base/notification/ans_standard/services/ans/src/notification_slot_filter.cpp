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

#include "notification_slot_filter.h"

#include "ans_inner_errors.h"
#include "ans_log_wrapper.h"
#include "notification_preferences.h"

namespace OHOS {
namespace Notification {

void NotificationSlotFilter::OnStart()
{}

void NotificationSlotFilter::OnStop()
{}

ErrCode NotificationSlotFilter::OnPublish(const std::shared_ptr<NotificationRecord> &record)
{
    if (record->slot != nullptr) {
        if (record->slot->CanEnableLight()) {
            record->notification->SetEnableLight(true);
            record->notification->SetLedLightColor(record->slot->GetLedLightColor());
        } else {
            record->notification->SetEnableLight(false);
        }

        if (record->slot->CanVibrate()) {
            record->notification->SetEnableViration(true);
            record->notification->SetVibrationStyle(record->slot->GetVibrationStyle());
        } else {
            record->notification->SetEnableViration(false);
        }

        auto sound = record->slot->GetSound();
        if (!sound.ToString().empty()) {
            record->notification->SetEnableSound(true);
            record->notification->SetSound(record->slot->GetSound());
        } else {
            record->notification->SetEnableSound(false);
        }

        if (record->request->GetVisibleness() == NotificationConstant::VisiblenessType::NO_OVERRIDE) {
            record->request->SetVisibleness(record->slot->GetLockScreenVisibleness());
        }
    } else {
        ANS_LOGE("Non valid slot!");
        return ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_NOT_EXIST;
    }

    return ERR_OK;
}

}  // namespace Notification
}  // namespace OHOS