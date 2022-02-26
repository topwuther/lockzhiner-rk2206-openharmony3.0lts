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

#include "dm_ability_manager.h"

#include "auth_manager.h"
#include "ability_manager_client.h"
#include "ability_record.h"
#include "ability_manager_service.h"
#include "parameter.h"
#include "semaphore.h"

#include "constants.h"
#include "device_manager_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t ABILITY_START_TIMEOUT = 3; // 3 second
}
IMPLEMENT_SINGLE_INSTANCE(DmAbilityManager);

AbilityRole DmAbilityManager::GetAbilityRole()
{
    return mAbilityStatus_;
}

AbilityStatus DmAbilityManager::StartAbility(AbilityRole role)
{
    std::string roleStr;
    if (role == AbilityRole::ABILITY_ROLE_INITIATIVE) {
        roleStr = "initiative";
    } else if (role == AbilityRole::ABILITY_ROLE_PASSIVE) {
        roleStr = "passive";
    } else {
        DMLOG(DM_LOG_ERROR, "StartAbility, failed, role unknown");
        return AbilityStatus::ABILITY_STATUS_FAILED;
    }

    DMLOG(DM_LOG_ERROR, "StartAbility, role %s", roleStr.c_str());
    mAbilityStatus_ = role;

    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string deviceId = localDeviceId;
    std::string bundleName = "com.ohos.devicemanagerui";
    std::string abilityName = "com.ohos.devicemanagerui.MainAbility";
    int32_t displayOwner = (role == AbilityRole::ABILITY_ROLE_INITIATIVE) ?
        AuthManager::GetInstance().GetDisplayOwner() : DISPLAY_OWNER_SYSTEM;

    mStatus_ = AbilityStatus::ABILITY_STATUS_START;
    AAFwk::Want want;
    AppExecFwk::ElementName element(deviceId, bundleName, abilityName);
    want.SetElement(element);
    if (displayOwner == DISPLAY_OWNER_OTHER) {
        return AbilityStatus::ABILITY_STATUS_SUCCESS;
    }
    AAFwk::AbilityManagerClient::GetInstance()->Connect();
    ErrCode result = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
    if (result == OHOS::ERR_OK) {
        DMLOG(DM_LOG_INFO, "Start Ability succeed");
    } else {
        DMLOG(DM_LOG_INFO, "Start Ability faild");
        mStatus_ = AbilityStatus::ABILITY_STATUS_FAILED;
        return mStatus_;
    }
    waitForTimeout(ABILITY_START_TIMEOUT);
    return mStatus_;
}

void DmAbilityManager::waitForTimeout(uint32_t timeout_s)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_s;
    sem_timedwait(&mSem_, &ts);
}

void DmAbilityManager::StartAbilityDone()
{
    mStatus_ = AbilityStatus::ABILITY_STATUS_SUCCESS;
    sem_post(&mSem_);
}
}
}
