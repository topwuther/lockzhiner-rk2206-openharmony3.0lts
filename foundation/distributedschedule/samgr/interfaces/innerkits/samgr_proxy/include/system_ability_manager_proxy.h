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


#ifndef INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_PROXY_H_
#define INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_PROXY_H_

#include <string>
#include "if_system_ability_manager.h"

namespace OHOS {
class SystemAbilityManagerProxy : public IRemoteProxy<ISystemAbilityManager> {
public:
    explicit SystemAbilityManagerProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<ISystemAbilityManager>(impl) {}
    ~SystemAbilityManagerProxy() = default;

    sptr<IRemoteObject> CheckLocalAbilityManager(const std::u16string& name) override;
    int32_t AddLocalAbilityManager(const std::u16string& name, const sptr<IRemoteObject>& ability) override;
    int32_t RemoveLocalAbilityManager(const std::u16string& name) override;
    std::vector<std::u16string> ListSystemAbilities(unsigned int dumpFlags) override;
    int32_t RecycleOnDemandSystemAbility() override;

    // IntToString adapter interface
    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override;
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) override;
    int32_t RemoveSystemAbility(int32_t systemAbilityId) override;
    int32_t SubscribeSystemAbility(int32_t systemAbilityId, const std::u16string& listenerName) override;
    int32_t UnSubscribeSystemAbility(int32_t systemAbilityId, const std::u16string& listenerName) override;
    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
        const std::u16string& localAbilityManagerName) override;
    sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) override;
    int32_t ConnectSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityConnectionCallback>& connectionCallback) override;
    int32_t DisConnectSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityConnectionCallback>& connectionCallback) override;
    const std::u16string CheckOnDemandSystemAbility(int32_t systemAbilityId) override;
    bool GetSystemAbilityInfoList(int32_t systemAbilityId,
        const std::u16string& capability, std::list<std::shared_ptr<SystemAbilityInfo>>& saInfoList) override;
    bool GetDeviceId(std::string& deviceId) override;

    int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp) override;
    int32_t RegisterSystemReadyCallback(const sptr<IRemoteObject>& systemReadyCallback) override;
    int32_t GetCoreSystemAbilityList(std::vector<int32_t>& coreSaList, int dumpMode) override;
    int32_t AddSystemCapability(const std::string& sysCap) override;
    bool HasSystemCapability(const std::string& sysCap) override;
    std::vector<std::string> GetSystemAvailableCapabilities() override;
private:
    sptr<IRemoteObject> GetSystemAbilityWrapper(int32_t systemAbilityId, const std::string& deviceId = "");
    sptr<IRemoteObject> CheckSystemAbilityWrapper(int32_t code, MessageParcel& data);
    int32_t MarshalSAExtraProp(const SAExtraProp& extraProp, MessageParcel& data) const;
    int32_t AddSystemAbilityWrapper(int32_t code, MessageParcel& data);
    int32_t RemoveSystemAbilityWrapper(int32_t code, MessageParcel& data);
private:
    static inline BrokerDelegator<SystemAbilityManagerProxy> delegator_;
};
} // namespace OHOS

#endif // !defined(INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_MANAGER_PROXY_H_)
