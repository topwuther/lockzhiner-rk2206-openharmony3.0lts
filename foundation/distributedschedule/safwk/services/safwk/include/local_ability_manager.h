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

#ifndef LOCAL_ABILITY_MANAGER_H
#define LOCAL_ABILITY_MANAGER_H

#include <string>
#include <map>
#include <list>
#include <unistd.h>
#include <condition_variable>
#include <shared_mutex>
#include "local_ability_manager_stub.h"
#include "system_ability.h"
#include "thread_pool.h"
#include "sa_profile_parser.h"
#include "ilocal_ability_manager.h"
#include "single_instance.h"

namespace OHOS {
// Check all dependencies's availability before the timeout period ended, [200, 60000].
const int32_t MIN_DEPENDENCY_TIMEOUT = 200;
const int32_t MAX_DEPENDENCY_TIMEOUT = 60000;
const int32_t DEFAULT_DEPENDENCY_TIMEOUT = 6000;

class LocalAbilityManager : public LocalAbilityManagerStub, public ILocalAbilityManagerKit {
    DECLARE_SINGLE_INSTANCE_BASE(LocalAbilityManager);

public:
    bool Run();

public:
    bool InitializeSaProfiles();
    bool AddAbility(SystemAbility* ability);
    bool RemoveAbility(int32_t systemAbilityId);
    SystemAbility* GetAbility(int32_t systemAbilityId);
    bool GetRunningStatus(int32_t systemAbilityId);
    bool SaveAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    bool DeleteAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    bool StartAllAddAbilityListener();
    bool NotifyAbilityListener(int32_t systemAbilityId, int32_t listenerSaId, int32_t code);
    bool NotifyAbilityListener(int32_t systemAbilityId, int32_t listenerSaId,
        const std::string& deviceId, int32_t code);
    bool InitAddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    bool AddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    bool RemoveSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId);
    bool FindAndNotifyAbilityListeners(int32_t systemAbilityId, const std::string& deviceId, int32_t code);
    std::vector<std::u16string> CheckDependencyStatus(const std::vector<std::u16string>& dependSas);
    void StartSystemAbilityTask(SystemAbility* sa);
    bool CheckSystemAbilityManagerReady();
    bool InitSystemAbilityProfiles(const std::string& profilePath);
    void ClearResource();
    void StartOndemandSystemAbility(int32_t systemAbilityId);

    bool HandoffAbilityAfter(const std::u16string& begin, const std::u16string& after) override;
    bool HandoffAbilityBegin(int32_t systemAbilityId) override;
    bool StartAbility(int32_t systemAbilityId) override;
    bool StopAbility(int32_t systemAbilityId) override;
    bool OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId = "") override;
    bool OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId = "") override;
    bool Debug(int32_t systemAbilityId) override;
    bool Test(int32_t systemAbilityId) override;
    bool SADump(int32_t systemAbilityId) override;
    void StartAbilityAsyn(int32_t systemAbilityId) override;
    bool RecycleOndemandSystemAbility(int32_t systemAbilityId) override;

    void DoStartSAProcess(const std::string& profilePath) override;
    bool ReRegisterSA();

private:
    LocalAbilityManager();
    ~LocalAbilityManager();

    bool AddLocalAbilityManager(const std::string& localAbilityMgrName);
    void RegisterOnDemandSystemAbility();
    void FindAndStartPhaseTasks();
    void CreateDirectories();
    void AddSamgrDeathRecipient();
    void StartPhaseTasks(const std::list<SystemAbility*>& startTasks);

    std::map<int32_t, SystemAbility*> abilityMap_;
    std::map<uint32_t, std::list<SystemAbility*>> abilityPhaseMap_;
    std::shared_mutex abilityMapLock_;
    sptr<LocalAbilityManager> localAbilityManager_;
    // Thread pool used to start system abilities in parallel.
    ThreadPool pool_;
    // Thread pool used to start ondemand system abilities in parallel.
    ThreadPool ondemandPool_;
    // Max task number in pool is 20.
    const int32_t MAX_TASK_NUMBER = 20;
    // Check dependent sa status every 50 ms, it equals 50000us.
    const int32_t CHECK_DEPENDENT_SA_PERIOD = 50000;
    pid_t currentPid_ = 0;
    std::map<int32_t, std::vector<int32_t>> listenerMap_;

    std::shared_ptr<SaProfileParser> profileParser_;

    std::condition_variable startPhaseCV_;
    std::mutex startPhaseLock_;
    int32_t startTaskNum_ = 0;
};
}
#endif
