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

#include "advanced_notification_service_ability.h"

namespace OHOS {
namespace Notification {

namespace {
REGISTER_SYSTEM_ABILITY_BY_ID(AdvancedNotificationServiceAbility, ADVANCED_NOTIFICATION_SERVICE_ABILITY_ID, true);
}

AdvancedNotificationServiceAbility::AdvancedNotificationServiceAbility(const int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), service_(nullptr)
{}

AdvancedNotificationServiceAbility::~AdvancedNotificationServiceAbility()
{}

void AdvancedNotificationServiceAbility::OnStart()
{
    if (service_ != nullptr) {
        return;
    }

    service_ = AdvancedNotificationService::GetInstance();
    if (!Publish(service_)) {
        return;
    }
}

void AdvancedNotificationServiceAbility::OnStop()
{
    service_ = nullptr;
}

}  // namespace Notification
}  // namespace OHOS