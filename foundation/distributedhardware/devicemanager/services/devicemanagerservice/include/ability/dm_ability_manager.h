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

#ifndef OHOS_DM_ABILITY_MANAGER_H
#define OHOS_DM_ABILITY_MANAGER_H

#include <string>
#include <vector>
#include <mutex>
#include <semaphore.h>

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
enum AbilityRole : int32_t {
    ABILITY_ROLE_PASSIVE = 0,
    ABILITY_ROLE_INITIATIVE = 1,
    ABILITY_ROLE_UNKNOWN = 2
};

enum AbilityStatus : int32_t {
    ABILITY_STATUS_FAILED = 0,
    ABILITY_STATUS_SUCCESS = 1,
    ABILITY_STATUS_START = 2
};

typedef enum FaAction {
    USER_OPERATION_TYPE_ALLOW_AUTH = 0,
    USER_OPERATION_TYPE_CANCEL_AUTH = 1,
    USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT = 2,
    USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY = 3,
    USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT = 4
} FaAction;

class DmAbilityManager {
DECLARE_SINGLE_INSTANCE(DmAbilityManager);
public:
    AbilityRole GetAbilityRole();
    AbilityStatus StartAbility(AbilityRole role);
    void StartAbilityDone();

private:
    void waitForTimeout(uint32_t timeout_s);

private:
    sem_t mSem_;
    AbilityStatus mStatus_;
    AbilityRole mAbilityStatus_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
