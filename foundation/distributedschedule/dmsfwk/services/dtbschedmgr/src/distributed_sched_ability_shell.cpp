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

#include "distributed_sched_ability_shell.h"

#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "parcel_helper.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::u16string ABILITYSHELL_INTERFACE_TOKEN = u"ohos.abilityshell.ContinuationScheduler";
constexpr int64_t SCHEDULE_COMPLETE_CONTINUE = 1;
}
using namespace OHOS::HiviewDFX;
IMPLEMENT_SINGLE_INSTANCE(DistributedSchedAbilityShell);

DistributedSchedAbilityShell::DistributedSchedAbilityShell()
{
    death_ = sptr<IRemoteObject::DeathRecipient>(new ContinuationCallbackDeathRecipient());
}

int32_t DistributedSchedAbilityShell::RegisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    if (abilityToken == nullptr || continuationCallback == nullptr) {
        HILOGE("RegisterAbilityToken abilityToken or continuationCallback is null");
        return INVALID_PARAMETERS_ERR;
    }
    std::lock_guard<std::mutex> autoLock(regAbilityLock_);
    auto itAbility = regAbilityMap_.find(continuationCallback);
    if (itAbility == regAbilityMap_.end()) {
        // new continuationCallback, add death recipient
        continuationCallback->AddDeathRecipient(death_);
    }
    auto& tokenList = regAbilityMap_[continuationCallback];
    for (const auto& tokenItem : tokenList) {
        if (tokenItem == abilityToken) {
            // already have reg abilityToken
            return REG_REPEAT_ABILITY_TOKEN_ERR;
        }
    }
    // add abilityToken
    tokenList.emplace_back(abilityToken);
    return ERR_OK;
}

int32_t DistributedSchedAbilityShell::UnregisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    if (abilityToken == nullptr || continuationCallback == nullptr) {
        HILOGE("UnregisterAbilityToken abilityToken or continuationCallback is null");
        return INVALID_PARAMETERS_ERR;
    }
    std::lock_guard<std::mutex> autoLock(regAbilityLock_);
    auto itAbility = regAbilityMap_.find(continuationCallback);
    if (itAbility != regAbilityMap_.end()) {
        std::list<sptr<IRemoteObject>>& tokenList = itAbility->second;
        int32_t sizeBefore = tokenList.size();
        if (sizeBefore == 0) {
            return NO_ABILITY_TOKEN_ERR;
        }
        tokenList.remove(abilityToken);
        int32_t sizeAfter = tokenList.size();
        // if list is empty and erase it from map
        if (sizeAfter == 0) {
            if (itAbility->first != nullptr) {
                itAbility->first->RemoveDeathRecipient(death_);
            }
            regAbilityMap_.erase(itAbility);
        }
        if (sizeBefore == sizeAfter) {
            return NO_ABILITY_TOKEN_ERR;
        }
        return ERR_OK;
    }
    // not find continuationCallback, return NO_APP_THREAD_ERR
    return NO_APP_THREAD_ERR;
}

sptr<IRemoteObject> DistributedSchedAbilityShell::GetContinuationCallback(const sptr<IRemoteObject>& abilityToken)
{
    if (abilityToken == nullptr) {
        HILOGE("GetContinuationCallback abilityToken is null");
        return nullptr;
    }
    std::lock_guard<std::mutex> autoLock(regAbilityLock_);
    for (const auto& regAbility : regAbilityMap_) {
        const std::list<sptr<IRemoteObject>>& tokenList = regAbility.second;
        for (const auto& tokenItem : tokenList) {
            if (tokenItem == abilityToken) {
                // find abilityToken
                return regAbility.first;
            }
        }
    }
    return nullptr;
}

void DistributedSchedAbilityShell::RemoveContinuationCallback(const sptr<IRemoteObject>& continuationCallback)
{
    if (continuationCallback == nullptr) {
        HILOGE("RemoveContinuationCallback continuationCallback is null");
        return;
    }
    std::lock_guard<std::mutex> autoLock(regAbilityLock_);
    continuationCallback->RemoveDeathRecipient(death_);
    regAbilityMap_.erase(continuationCallback);
}

int32_t DistributedSchedAbilityShell::ScheduleCompleteContinuation(const sptr<IRemoteObject>& abilityToken,
    int32_t isSuccess)
{
    if (abilityToken == nullptr) {
        HILOGE("ScheduleCompleteContinuation ability abilityToken is null");
        return INVALID_PARAMETERS_ERR;
    }

    sptr<IRemoteObject> continuationCallback = GetContinuationCallback(abilityToken);
    if (continuationCallback == nullptr) {
        HILOGE("ScheduleCompleteContinuation continuationCallback is null");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(ABILITYSHELL_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, isSuccess);
    MessageParcel reply;
    MessageOption option;
    int32_t error = continuationCallback->SendRequest(SCHEDULE_COMPLETE_CONTINUE, data, reply, option);
    HILOGI("ScheduleCompleteContinuation transact result: %{public}d", error);
    return error;
}
} // namespace DistributedSchedule
} // namespace OHOS