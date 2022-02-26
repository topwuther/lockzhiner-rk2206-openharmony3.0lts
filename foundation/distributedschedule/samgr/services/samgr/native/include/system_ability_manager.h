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


#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H_
#define SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H_

#include "system_ability_manager_stub.h"
#include <map>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <utility>

#include "system_ability_definition.h"
#include "dbinder_service.h"
#include "dbinder_service_stub.h"
#include "system_ability_info.h"

namespace OHOS {
struct SAInfo {
    sptr<IRemoteObject> remoteObj;
    bool isDistributed = false;
    std::u16string capability;
    std::string permission;
};

enum {
    UUID = 0,
    NODE_ID,
    UNKNOWN,
};

class SystemAbilityManager : public SystemAbilityManagerStub {
public:
    virtual ~SystemAbilityManager();
    static sptr<SystemAbilityManager> GetInstance();

    sptr<IRemoteObject> CheckLocalAbilityManager(const std::u16string& localAbilityManagerName) override;
    int32_t AddLocalAbilityManager(const std::u16string& localAbilityManagerName,
        const sptr<IRemoteObject>& localAbilityManager) override;
    int32_t RemoveLocalAbilityManager(const std::u16string& localAbilityManagerName) override;

    int32_t RemoveSystemAbility(const sptr<IRemoteObject>& ability);
    int32_t RemoveLocalAbilityManager(const sptr<IRemoteObject>& localAbilityManager);

    std::vector<std::u16string> ListSystemAbilities(uint32_t dumpFlags) override;

    int32_t RecycleOnDemandSystemAbility() override;

    int32_t RegisterSystemReadyCallback(const sptr<IRemoteObject>& systemReadyCallback) override;

    int32_t GetCoreSystemAbilityList(std::vector<int32_t>& coreSaList, int dumpMode) override;

    int32_t RemoveSystemReadyCallback(const sptr<IRemoteObject>& callback);

    void SetDeviceName(const std::u16string &name);

    const std::u16string& GetDeviceName() const;

    bool GetDeviceId(std::string& deviceId) override;

    const sptr<DBinderService> GetDBinder() const;

    void DoSADataStorageInit();

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
    void NotifyRemoteSaDied(const std::u16string& name);
    void NotifyRemoteDeviceOffline(const std::string& deviceId);
    int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp) override;
    std::string TransformDeviceId(const std::string& deviceId, int32_t type, bool isPrivate);
    std::string GetLocalNodeId();
    int32_t AddSystemCapability(const std::string& sysCap) override;
    bool HasSystemCapability(const std::string& sysCap) override;
    std::vector<std::string> GetSystemAvailableCapabilities() override;

    void Init();
private:
    SystemAbilityManager();
    std::u16string GetSystemAbilityName(int32_t index) override;
    void DoInsertSaData(const std::u16string& name, const sptr<IRemoteObject>& ability, const SAExtraProp& extraProp);
    bool IsNameInValid(const std::u16string& name);
    int32_t StartOnDemandAbility(int32_t systemAbilityId);
    void DeleteStartingAbilityMember(int32_t systemAbilityId);
    bool CheckCapability(const std::u16string& capability);
    void ParseRemoteSaName(const std::u16string& name, std::string& deviceId, std::u16string& saName);
    void OnDemandConnected(int32_t systemAbilityId, const sptr<IRemoteObject>& ability);
    bool IsLocalDeviceId(const std::string& deviceId);
    bool CheckRemoteSa(const std::string& saName, std::string& selectedDeviceId);
    bool CheckDistributedPermission();
    int32_t AddSystemAbility(const std::u16string& name, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp);
    int32_t FindSystemAbilityManagerNotify(int32_t systemAbilityId, int32_t code);
    int32_t FindSystemAbilityManagerNotify(int32_t systemAbilityId, const std::string& deviceId, int32_t code);
    bool CheckPermission(const std::string& permission);

    void InitCoreSaList();
    void RestoreCoreSaId(int32_t saId);
    void RemoveCompletedCoreSaId(int32_t saId);
    void SendSystemReadyMessage();
    void SendSingleSystemReadyMessage(const sptr<IRemoteObject>& systemReadyCallback);
    void InitSysCapMap();
    void AddDefaultCoreSa(std::set<int32_t>& coreSaIdSet) const;

    std::u16string deviceName_;
    static sptr<SystemAbilityManager> instance;
    static std::mutex instanceLock;
    sptr<IRemoteObject::DeathRecipient> abilityDeath_;
    sptr<IRemoteObject::DeathRecipient> localAbilityManagerDeath_;
    sptr<IRemoteObject::DeathRecipient> systemReadyCallbackDeath_;
    sptr<DBinderService> dBinderService_;
    bool isDbinderStart_ = false;

    // must hold abilityMapLock_ never access other locks
    std::shared_mutex abilityMapLock_;
    std::map<int32_t, SAInfo> abilityMap_;

    // must hold localAbilityManagerMapLock_ never access other locks
    std::recursive_mutex localAbilityManagerMapLock_;
    std::map<std::u16string, sptr<IRemoteObject>> localAbilityManagerMap_;

    // maybe hold listenerMapLock_ and then access localAbilityMapLock_
    std::recursive_mutex listenerMapLock_;
    std::map<int32_t, std::list<std::u16string>> listenerMap_;

    // maybe hold onDemandAbilityMapLock_ and then access localAbilityMapLock_
    std::recursive_mutex onDemandAbilityMapLock_;
    std::map<int32_t, std::u16string> onDemandAbilityMap_;
    std::list<int32_t> startingAbilityList_;
    std::map<int32_t, sptr<ISystemAbilityConnectionCallback>> connectionCallbackMap_;
    std::set<int32_t> coreSaIdSet_;
    std::set<int32_t> coreSaIdSetBackup_;
    std::map<sptr<IRemoteObject>, int32_t> systemCallbackMap_;
    std::map<int32_t, int32_t> callingPidCountMap_; // key:callintPid value:callingCount
    std::condition_variable parseCoreSaCV_;
    std::mutex parseCoreSaMtx_;
    bool parseCoreSaReady_ = false;
    bool isCoreSaInitReady_ = false;
    std::mutex sysCapMapLock_;
    std::map<std::string, bool> sysCapMap_;
};
} // namespace OHOS

#endif // !defined(SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_MANAGER_H_)
