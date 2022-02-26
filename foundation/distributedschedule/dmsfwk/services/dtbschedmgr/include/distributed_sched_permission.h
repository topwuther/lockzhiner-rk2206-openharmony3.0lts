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

#ifndef DISTRIBUTED_SCHED_PERMISSION_H
#define DISTRIBUTED_SCHED_PERMISSION_H

#include <string>

#include "distributed_sched_interface.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedSchedPermission {
    DECLARE_SINGLE_INSTANCE(DistributedSchedPermission);

public:
    using AccountInfo = IDistributedSched::AccountInfo;

    int32_t CheckDPermission(const AAFwk::Want& want, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo, const AppExecFwk::AbilityInfo& abilityInfo,
        const std::string& localDeviceId);
    bool CheckComponentAccessPermission(const AppExecFwk::AbilityInfo& targetAbility,
        const CallerInfo& callerInfo, const AccountInfo& accountInfo, const AAFwk::Want& want) const;

private:
    bool getTargetAbility(const AAFwk::Want& want, const AppExecFwk::AbilityInfo& abilityInfo,
        const std::string& localDeviceId, AppExecFwk::AbilityInfo& targetAbility, const CallerInfo& callerInfo) const;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // DISTRIBUTED_SCHED_PERMISSION_H