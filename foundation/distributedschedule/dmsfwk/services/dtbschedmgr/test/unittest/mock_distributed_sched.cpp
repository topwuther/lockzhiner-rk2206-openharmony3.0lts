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

#include "mock_distributed_sched.h"
#include "dtbschedmgr_log.h"
#include "string_ex.h"

using namespace std;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace DistributedSchedule {
int32_t MockDistributedSched::StartRemoteAbility(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode)
{
    return 0;
}

int32_t MockDistributedSched::StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode, const CallerInfo& callerInfo,
    const AccountInfo& accountInfo)
{
    return 0;
}

int32_t MockDistributedSched::StartContinuation(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& abilityToken)
{
    return 0;
}

void MockDistributedSched::NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess)
{
    (void)isSuccess;
}

int32_t MockDistributedSched::NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess)
{
    (void)isSuccess;
    return 0;
}

int32_t MockDistributedSched::RegisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    return 0;
}

int32_t MockDistributedSched::UnregisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
    const sptr<IRemoteObject>& continuationCallback)
{
    return 0;
}

int32_t MockDistributedSched::ConnectRemoteAbility(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect)
{
    return 0;
}

int32_t MockDistributedSched::DisconnectRemoteAbility(const sptr<IRemoteObject>& connect)
{
    return 0;
}

int32_t MockDistributedSched::ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    return 0;
}

int32_t MockDistributedSched::DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
    int32_t uid, const std::string& sourceDeviceId)
{
    return 0;
}

int32_t MockDistributedSched::NotifyProcessDiedFromRemote(const CallerInfo& callerInfo)
{
    return 0;
}
} // namespace DistributedSchedule
} // namespace OHOS