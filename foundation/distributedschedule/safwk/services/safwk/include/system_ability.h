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

#ifndef SYSTEM_ABILITY_H_
#define SYSTEM_ABILITY_H_

#include <string>
#include <map>
#include <vector>
#include <chrono>
#include "iremote_object.h"

namespace OHOS {
#define REGISTER_SYSTEM_ABILITY_BY_ID(abilityClassName, systemAbilityId, runOnCreate) \
    const bool abilityClassName##_##RegisterResult = \
    SystemAbility::MakeAndRegisterAbility(new abilityClassName(systemAbilityId, runOnCreate));

#define INIT_LISTEN_SYSTEM_ABILITY_BY_ID(abilityClassName, \
    abilityClassNameListenerName, systemAbilityId, listenerName) \
    const bool abilityClassName##_##abilityClassNameListenerName##_##RegisterResult = \
    SystemAbility::InitAddSystemAbilityListener(systemAbilityId, listenerName);

#define DECLEAR_SYSTEM_ABILITY(className) \
public: \
virtual std::string GetClassName() override { \
return #className; \
}

#define DECLEAR_BASE_SYSTEM_ABILITY(className) \
public: \
virtual std::string GetClassName() = 0;

#define DECLARE_SYSTEM_ABILITY(className) \
public: \
virtual std::string GetClassName() override { \
return #className; \
}

#define DECLARE_BASE_SYSTEM_ABILITY(className) \
public: \
virtual std::string GetClassName() = 0;


#define DECLARE_SYSTEM_ABILITY(className) \
public: \
virtual std::string GetClassName() override { \
return #className; \
}

#define DECLARE_BASE_SYSTEM_ABILITY(className) \
public: \
virtual std::string GetClassName() = 0;

class SystemAbility {
    DECLARE_BASE_SYSTEM_ABILITY(SystemAbility);

public:
    static bool MakeAndRegisterAbility(SystemAbility* systemAbility);

    // IntToString adapter interface
    static bool InitAddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    bool AddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    bool AddSystemAbilityListener(int32_t systemAbilityId);
    bool RemoveSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    bool RemoveSystemAbilityListener(int32_t systemAbilityId);

protected:
    virtual void OnDump();
    virtual void OnDebug();
    virtual void OnTest();
    virtual void OnStart();
    virtual void OnStop();
    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<IRemoteObject>& ability);
    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId);

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId);
    bool Publish(sptr<IRemoteObject> systemAbility);
    void StopAbility(int32_t systemAbilityId);

    SystemAbility(bool runOnCreate = false);
    SystemAbility(int32_t systemAbilityId, bool runOnCreate = false);
    virtual ~SystemAbility();

private:
    void Start();
    void Stop();
    void SADump();
    void Debug();
    void Test();
    int32_t GetSystemAbilitId() const;
    void SetLibPath(const std::u16string& libPath);
    const std::u16string& GetLibPath() const;
    void SetDependSa(const std::vector<std::u16string>& dependSa);
    const std::vector<std::u16string>& GetDependSa() const;
    void SetRunOnCreate(bool isRunOnCreate);
    bool IsRunOnCreate() const;
    void SetDistributed(bool isDistributed);
    bool GetDistributed() const;
    void SetDumpLevel(unsigned int dumpLevel);
    unsigned int GetDumpLevel() const;
    void SetDependTimeout(int dependTimeout);
    int GetDependTimeout() const;
    bool GetRunningStatus() const;
    void AddToLocal() const;
    void DeleteFromLocal() const;
    void SetCapability(const std::u16string& capability);
    const std::u16string& GetCapability() const;
    void SetPermission(const std::u16string& defPerm);
    bool RePublish();

    friend class LocalAbilityManager;

private:
    int32_t saId_ = 0;
    std::u16string libPath_;
    std::vector<std::u16string> dependSa_;
    bool isRunOnCreate_;
    bool isDistributed_;
    unsigned int dumpLevel_;
    int dependTimeout_;
    bool isRunning_;
    std::u16string capability_;
    sptr<IRemoteObject> publishObj_;
    std::u16string permission_;
};
}

#endif
