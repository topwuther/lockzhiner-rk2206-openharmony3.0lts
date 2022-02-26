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

#ifndef OHOS_DISTRIBUTED_SCHED_PROXY_H
#define OHOS_DISTRIBUTED_SCHED_PROXY_H

#include "distributed_sched_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedSchedProxy : public IRemoteProxy<IDistributedSched> {
public:
    explicit DistributedSchedProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IDistributedSched>(impl) {}
    ~DistributedSchedProxy() {}
    int32_t StartRemoteAbility(const OHOS::AAFwk::Want& want, const OHOS::AppExecFwk::AbilityInfo& abilityInfo,
        int32_t requestCode) override;
    int32_t StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
        const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo) override;
    int32_t StartContinuation(const OHOS::AAFwk::Want& want, const OHOS::AppExecFwk::AbilityInfo& abilityInfo,
        const sptr<IRemoteObject>& abilityToken) override;
    void NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess) override;
    int32_t NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess) override;
    int32_t RegisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
        const sptr<IRemoteObject>& continuationCallback) override;
    int32_t UnregisterAbilityToken(const sptr<IRemoteObject>& abilityToken,
        const sptr<IRemoteObject>& continuationCallback) override;
    int32_t ConnectRemoteAbility(const OHOS::AAFwk::Want& want, const AppExecFwk::AbilityInfo& abilityInfo,
        const sptr<IRemoteObject>& connect) override;
    int32_t DisconnectRemoteAbility(const sptr<IRemoteObject>& connect) override;
    int32_t ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want, const AppExecFwk::AbilityInfo& abilityInfo,
        const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo, const AccountInfo& accountInfo) override;
    int32_t DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
        int32_t uid, const std::string& sourceDeviceId) override;
    int32_t NotifyProcessDiedFromRemote(const CallerInfo& callerInfo) override;

private:
    static inline BrokerDelegator<DistributedSchedProxy> delegator_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_PROXY_H
