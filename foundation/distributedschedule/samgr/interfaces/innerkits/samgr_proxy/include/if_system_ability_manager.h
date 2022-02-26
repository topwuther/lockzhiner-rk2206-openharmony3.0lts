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

#ifndef INTERFACES_INNERKITS_SAMGR_INCLUDE_IF_SYSTEM_ABILITY_MANAGER_H_
#define INTERFACES_INNERKITS_SAMGR_INCLUDE_IF_SYSTEM_ABILITY_MANAGER_H_

#include <string>
#include <list>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "system_ability_info.h"
#include "if_system_ability_connection_callback.h"

namespace OHOS {
class ISystemAbilityManager : public IRemoteBroker {
public:
    // Retrieve an existing localAbilityManager
    virtual sptr<IRemoteObject> CheckLocalAbilityManager(const std::u16string& name) = 0;
    // Register an localAbilityManager just for sa_main
    virtual int32_t AddLocalAbilityManager(const std::u16string& name, const sptr<IRemoteObject>& ability) = 0;
    // Remove an localAbilityManager just for sa_main
    virtual int32_t RemoveLocalAbilityManager(const std::u16string& name) = 0;

    // Return list of all existing abilities.
    virtual std::vector<std::u16string> ListSystemAbilities(unsigned int dumpFlags = DUMP_FLAG_PRIORITY_ALL) = 0;
    // Recycle ondemand ability.
    virtual int32_t RecycleOnDemandSystemAbility() = 0;
    // Register system ready callback.
    virtual int32_t RegisterSystemReadyCallback(const sptr<IRemoteObject>& systemReadyCallback) = 0;
    // Get core system ability list.
    virtual int32_t GetCoreSystemAbilityList(std::vector<int32_t>& coreSaList, int dumpMode) = 0;

    enum {
        SHEEFT_CRITICAL = 0,
        SHEEFT_HIGH,
        SHEEFT_NORMAL,
        SHEEFT_DEFAULT,
        SHEEFT_PROTO,
    };

    static const unsigned int DUMP_FLAG_PRIORITY_CRITICAL = 1 << SHEEFT_CRITICAL;
    static const unsigned int DUMP_FLAG_PRIORITY_HIGH = 1 << SHEEFT_HIGH;
    static const unsigned int DUMP_FLAG_PRIORITY_NORMAL = 1 << SHEEFT_NORMAL;

    static const unsigned int DUMP_FLAG_PRIORITY_DEFAULT = 1 << SHEEFT_DEFAULT;
    static const unsigned int DUMP_FLAG_PRIORITY_ALL = DUMP_FLAG_PRIORITY_CRITICAL |
        DUMP_FLAG_PRIORITY_HIGH | DUMP_FLAG_PRIORITY_NORMAL | DUMP_FLAG_PRIORITY_DEFAULT;
    static const unsigned int DUMP_FLAG_PROTO = 1 << SHEEFT_PROTO;

    enum {
        GET_SYSTEM_ABILITY_TRANSACTION = 1,
        CHECK_SYSTEM_ABILITY_TRANSACTION = 2,
        ADD_SYSTEM_ABILITY_TRANSACTION = 3,
        REMOVE_SYSTEM_ABILITY_TRANSACTION = 4,
        LIST_SYSTEM_ABILITY_TRANSACTION = 5,
        SUBSCRIBE_SYSTEM_ABILITY_TRANSACTION = 6,
        CHECK_REMOTE_SYSTEM_ABILITY_TRANSACTION = 9,
        ADD_ONDEMAND_SYSTEM_ABILITY_TRANSACTION = 10,
        RECYCLE_ONDEMAND_SYSTEM_ABILITY_TRANSACTION = 11,
        CHECK_SYSTEM_ABILITY_IMMEDIATELY_TRANSACTION = 12,
        CONNECTION_SYSTEM_ABILITY_TRANSACTION = 13,
        DISCONNECTION_SYSTEM_ABILITY_TRANSACTION = 14,
        CHECK_ONDEMAND_SYSTEM_ABILITY_TRANSACTION = 15,
        CHECK_REMOTE_SYSTEM_ABILITY_FOR_JAVA_TRANSACTION = 16,
        GET_SYSTEM_ABILITYINFOLIST_TRANSACTION = 17,
        UNSUBSCRIBE_SYSTEM_ABILITY_TRANSACTION = 18,
        GET_LOCAL_DEVICE_ID_TRANSACTION = 19,
        ADD_LOCAL_ABILITY_TRANSACTION = 20,
        CHECK_LOCAL_ABILITY_TRANSACTION = 22,
        REMOVE_LOCAL_ABILITY_TRANSACTION = 23,
        REGISTER_SYSTEM_READY_CALLBACK = 24,
        GET_CORE_SYSTEM_ABILITY_LIST = 25,
        ADD_SYSTEM_CAPABILITY = 26,
        HAS_SYSTEM_CAPABILITY = 27,
        GET_AVAILABLE_SYSTEM_CAPABILITY = 28
    };

    // Retrieve an existing ability, blocking for a few seconds if it doesn't ye exist.
    virtual sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) = 0;

    // Retrieve an existing ability, no-blocking.
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId) = 0;

    // Remove an ability.
    virtual int32_t RemoveSystemAbility(int32_t systemAbilityId) = 0;

    // Subscribe an ability's status by listener name,
    // so the listener need to inherit from SystemAbilityListener class and implement the notify interfaces.
    virtual int32_t SubscribeSystemAbility(int32_t systemAbilityId, const std::u16string& listenerName) = 0;

    // UnSubscribe an ability's status by listener name,
    virtual int32_t UnSubscribeSystemAbility(int32_t systemAbilityId, const std::u16string& listenerName) = 0;

    // Retrieve an existing ability, blocking for a few seconds if it doesn't ye exist.
    virtual sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId) = 0;

    // Retrieve an existing ability, no-blocking
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId) = 0;

    // Add ondemand ability info.
    virtual int32_t AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
        const std::u16string& localAbilityManagerName) = 0;

    // Retrieve an ability, no-blocking.
    virtual sptr<IRemoteObject> CheckSystemAbility(int32_t systemAbilityId, bool& isExist) = 0;

    // connect system ability
    virtual int32_t ConnectSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityConnectionCallback>& connectionCallback) = 0;

    // disconnect system ability
    virtual int32_t DisConnectSystemAbility(int32_t systemAbilityId,
        const sptr<ISystemAbilityConnectionCallback>& connectionCallback) = 0;

    // check ondemand system ability
    virtual const std::u16string CheckOnDemandSystemAbility(int32_t systemAbilityId) = 0;

    // Retrieve an ability info list that satisfies capability
    virtual bool GetSystemAbilityInfoList(int32_t systemAbilityId,
        const std::u16string& capability, std::list<std::shared_ptr<SystemAbilityInfo>>& saInfoList) = 0;

    // get local device id
    virtual bool GetDeviceId(std::string& deviceId) = 0;

    struct SAExtraProp {
        SAExtraProp() = default;
        SAExtraProp(bool isDistributed, unsigned int dumpFlags, const std::u16string& capability,
            const std::u16string& permission)
        {
            this->isDistributed = isDistributed;
            this->dumpFlags = dumpFlags;
            this->capability = capability;
            this->permission = permission;
        }

        bool isDistributed = false;
        unsigned int dumpFlags = DUMP_FLAG_PRIORITY_DEFAULT;
        std::u16string capability;
        std::u16string permission;
    };
    virtual int32_t AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp = SAExtraProp(false, DUMP_FLAG_PRIORITY_DEFAULT, u"", u"")) = 0;
    virtual int32_t AddSystemCapability(const std::string& sysCap) = 0;
    virtual bool HasSystemCapability(const std::string& sysCap) = 0;
    virtual std::vector<std::string> GetSystemAvailableCapabilities() = 0;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.ISystemAbilityManager");
protected:
    static constexpr int32_t FIRST_SYS_ABILITY_ID = 0x00000001;
    static constexpr int32_t LAST_SYS_ABILITY_ID = 0x00ffffff;
    bool CheckInputSysAbilityId(int32_t sysAbilityId) const
    {
        if (sysAbilityId >= FIRST_SYS_ABILITY_ID && sysAbilityId <= LAST_SYS_ABILITY_ID) {
            return true;
        }
        return false;
    }
};
} // namespace OHOS

#endif // !defined(INTERFACES_INNERKITS_SAMGR_INCLUDE_IF_SYSTEM_ABILITY_MANAGER_H_ )
