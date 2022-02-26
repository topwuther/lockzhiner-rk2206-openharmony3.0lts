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

#ifndef OHOS_DISTRIBUTED_SCHED_ABILITY_SHELL_H
#define OHOS_DISTRIBUTED_SCHED_ABILITY_SHELL_H

#include <list>
#include <map>
#include <mutex>

#include "continuation_callback_death_recipient.h"
#include "iremote_object.h"
#include "ohos/aafwk/content/want.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedSchedAbilityShell {
    DECLARE_SINGLE_INSTANCE_BASE(DistributedSchedAbilityShell);

public:
    int32_t RegisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
        const sptr<IRemoteObject>& continuationCallback);
    int32_t UnregisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
        const sptr<IRemoteObject>& continuationCallback);
    sptr<IRemoteObject> GetContinuationCallback(const sptr<IRemoteObject>& abilityToken);
    int32_t ScheduleCompleteContinuation(const sptr<IRemoteObject>& abilityToken, int32_t isSuccess);
    void RemoveContinuationCallback(const sptr<IRemoteObject>& continuationCallback);

private:
    DistributedSchedAbilityShell();
    ~DistributedSchedAbilityShell() = default;

    std::map<sptr<IRemoteObject>, std::list<sptr<IRemoteObject>>> regAbilityMap_;
    std::mutex regAbilityLock_;
    sptr<IRemoteObject::DeathRecipient> death_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_ABILITY_SHELL_H