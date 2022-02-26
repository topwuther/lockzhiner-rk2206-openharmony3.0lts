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

#include "local_ability_manager.h"

#include <chrono>
#include <cinttypes>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#include "datetime_ex.h"
#include "errors.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "safwk_log.h"
#include "samgr_death_recipient.h"
#include "string_ex.h"
#include "system_ability.h"

namespace OHOS {
using std::u16string;
using std::string;
using std::vector;

namespace {
const string TAG = "LocalAbilityManager";

constexpr int32_t RETRY_TIMES_FOR_ONDEMAND = 10;
constexpr int32_t RETRY_TIMES_FOR_SAMGR = 50;
constexpr std::chrono::milliseconds MILLISECONDS_WAITING_SAMGR_ONE_TIME(200);
constexpr std::chrono::milliseconds MILLISECONDS_WAITING_ONDEMAND_ONE_TIME(100);

const u16string BOOT_START_PHASE = u"BootStartPhase";
const u16string CORE_START_PHASE = u"CoreStartPhase";
constexpr int32_t MAX_SA_STARTUP_TIME = 100;

const string PROFILES_DIR = "/system/profile/";
const string DEFAULT_DIR = "/system/usr/";

enum {
    BOOT_START = 1,
    CORE_START = 2,
    OTHER_START = 3,
};
}

IMPLEMENT_SINGLE_INSTANCE(LocalAbilityManager);

LocalAbilityManager::LocalAbilityManager()
{
    currentPid_ = getpid();
    profileParser_ = std::make_shared<SaProfileParser>();
    ondemandPool_.Start(std::thread::hardware_concurrency());
    ondemandPool_.SetMaxTaskNum(MAX_TASK_NUMBER);
}

LocalAbilityManager::~LocalAbilityManager()
{
    ondemandPool_.Stop();
}

void LocalAbilityManager::DoStartSAProcess(const std::string& profilePath)
{
    if (profilePath.length() > PATH_MAX) {
        HILOGE(TAG, "profilePath length too long!");
        return;
    }
    char realProfilePath[PATH_MAX] = {'\0'};
    if (realpath(profilePath.c_str(), realProfilePath) == nullptr) {
        HILOGE(TAG, "xmlDocName path does not exist!");
        return;
    }
    // pathString must begin with "/system/profile/" or begin with "/system/usr/"
    string pathString(realProfilePath);
    if (pathString.find(PROFILES_DIR) != 0 && pathString.find(DEFAULT_DIR) != 0) {
        HILOGE(TAG, "xmlDoc dir is not matched");
        return;
    }
    bool ret = InitSystemAbilityProfiles(pathString);
    if (!ret) {
        HILOGW(TAG, "InitSystemAbilityProfiles failed!");
        return;
    }
    ret = CheckSystemAbilityManagerReady();
    if (!ret) {
        HILOGW(TAG, "CheckSystemAbilityManagerReady failed!");
        return;
    }
    ret = InitializeSaProfiles();
    if (!ret) {
        HILOGW(TAG, "InitializeSaProfiles failed!");
        return;
    }
    ret = Run();
    if (!ret) {
        HILOGW(TAG, "Run failed!");
        return;
    }
    IPCSkeleton::JoinWorkThread();
    ClearResource();
}

bool LocalAbilityManager::CheckSystemAbilityManagerReady()
{
    sptr<ISystemAbilityManager> samgrProxy;
    int32_t timeout = RETRY_TIMES_FOR_SAMGR;
    constexpr int32_t duration = std::chrono::microseconds(MILLISECONDS_WAITING_SAMGR_ONE_TIME).count();
    while (samgrProxy == nullptr) {
        HILOGI(TAG, "waiting for samgr...");
        if (timeout > 0) {
            samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            usleep(duration);
        } else {
            HILOGE(TAG, "wait for samgr time out (10s)");
            return false;
        }
        timeout--;
    }
    return true;
}

bool LocalAbilityManager::InitSystemAbilityProfiles(const std::string& profilePath)
{
    bool ret = profileParser_->ParseSaProfiles(profilePath);
    if (!ret) {
        HILOGW(TAG, "ParseSaProfiles failed!");
        return false;
    }

    profileParser_->OpenSo();
    return true;
}

void LocalAbilityManager::ClearResource()
{
    profileParser_->ClearResource();
}

bool LocalAbilityManager::RecycleOndemandSystemAbility(int32_t systemAbilityId)
{
    std::unique_lock<std::shared_mutex> writeLock(abilityMapLock_);
    auto iter = abilityMap_.find(systemAbilityId);
    if (iter == abilityMap_.end()) {
        HILOGW(TAG, "SA:%{public}d not found", systemAbilityId);
        return false;
    }

    auto abilityInstance = iter->second;
    if (abilityInstance == nullptr) {
        HILOGW(TAG, "SA:%{public}d instance not exist", systemAbilityId);
        return false;
    }

    if (!abilityInstance->isRunning_) {
        delete abilityInstance;
        abilityInstance = nullptr;
        (void)abilityMap_.erase(iter);
    }

    profileParser_->CloseSo(systemAbilityId);
    return true;
}

bool LocalAbilityManager::AddAbility(SystemAbility* ability)
{
    if (ability == nullptr) {
        HILOGW(TAG, "try to add null ability!");
        return false;
    }

    int32_t saId = ability->GetSystemAbilitId();
    std::unique_lock<std::shared_mutex> writeLock(abilityMapLock_);
    auto iter = abilityMap_.find(saId);
    if (iter != abilityMap_.end()) {
        HILOGI(TAG, "try to add exsited ability:%{public}d!", saId);
        return false;
    }

    HILOGI(TAG, "adding SA:%{public}d", saId);
    abilityMap_.emplace(saId, ability);
    return true;
}

bool LocalAbilityManager::RemoveAbility(int32_t systemAbilityId)
{
    if (systemAbilityId <= 0) {
        HILOGW(TAG, "invalid systemAbilityId");
        return false;
    }
    std::unique_lock<std::shared_mutex> writeLock(abilityMapLock_);
    (void)abilityMap_.erase(systemAbilityId);
    return true;
}

bool LocalAbilityManager::SaveAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, listenerSaId);

    if (!CheckInputSysAbilityId(systemAbilityId) || !CheckInputSysAbilityId(listenerSaId)) {
        HILOGW(TAG, "SA:%{public}d or listenerSA:%{public}d invalid!", systemAbilityId, listenerSaId);
        return false;
    }

    auto& listenerSaIdList = listenerMap_[systemAbilityId];
    auto iter = std::find_if(listenerSaIdList.begin(), listenerSaIdList.end(), [listenerSaId](auto SaId) {
        return SaId == listenerSaId;
    });
    if (iter == listenerSaIdList.end()) {
        listenerSaIdList.emplace_back(listenerSaId);
        return true;
    }
    HILOGI(TAG, "SA:%{public}d already exist, listener SA is %{public}d", systemAbilityId, listenerSaId);
    return false;
}

bool LocalAbilityManager::DeleteAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, listenerSaId);

    if (!CheckInputSysAbilityId(systemAbilityId) || !CheckInputSysAbilityId(listenerSaId)) {
        HILOGW(TAG, "SA:%{public}d or listenerSA:%{public}d invalid!",
            systemAbilityId, listenerSaId);
        return false;
    }

    if (listenerMap_.count(systemAbilityId) == 0) {
        return false;
    }
    auto& listenerSaIdList = listenerMap_[systemAbilityId];
    for (auto iter = listenerSaIdList.begin(); iter != listenerSaIdList.end();) {
        if (*iter == listenerSaId) {
            iter = listenerSaIdList.erase(iter);
            return true;
        } else {
            ++iter;
        }
    }
    return false;
}

bool LocalAbilityManager::InitAddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    return SaveAbilityListener(systemAbilityId, listenerSaId);
}

bool LocalAbilityManager::StartAllAddAbilityListener()
{
    std::string localAbilityManagerName = "localabilitymanager" + std::to_string(currentPid_);
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }

    HILOGD(TAG, "localAbilityManagerName:%{public}s", localAbilityManagerName.c_str());
    for (const auto& [saId, listenerSaIdList] : listenerMap_) {
        if (samgrProxy->CheckSystemAbility(saId) != nullptr) {
            HILOGI(TAG, "SA:%{public}d already started, localabilitymanager = %{public}s",
                saId, localAbilityManagerName.c_str());
            std::string deviceId;
            if (!samgrProxy->GetDeviceId(deviceId)) {
                HILOGE(TAG, "failed to get deviceId");
                return false;
            }
            FindAndNotifyAbilityListeners(saId, deviceId, ON_ADD_SYSTEM_ABILITY_TRANSACTION);
        }

        int32_t ret = samgrProxy->SubscribeSystemAbility(saId, OHOS::to_utf16(localAbilityManagerName));
        if (ret != ERR_NONE) {
            HILOGE(TAG, "failed to subscribe SA:%{public}d, localabilitymanager:%{public}s",
                saId, localAbilityManagerName.c_str());
        }
    }
    return true;
}

bool LocalAbilityManager::AddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, listenerSaId);

    if (!CheckInputSysAbilityId(systemAbilityId) || !CheckInputSysAbilityId(listenerSaId)) {
        HILOGW(TAG, "SA:%{public}d or listenerSA:%{public}d invalid!",
            systemAbilityId, listenerSaId);
        return false;
    }

    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }

    std::string localAbilityManagerName = "localabilitymanager" + std::to_string(currentPid_);
    if (!SaveAbilityListener(systemAbilityId, listenerSaId)) {
        HILOGE(TAG, "failed to save ability listener");
        return false;
    }

    if (samgrProxy->CheckSystemAbility(systemAbilityId) != nullptr) {
        HILOGE(TAG, "SA:%{public}d already start, localabilitymanager:%{public}s", systemAbilityId,
            localAbilityManagerName.c_str());
        std::string deviceId;
        if (!samgrProxy->GetDeviceId(deviceId)) {
            HILOGE(TAG, "failed to get deviceId");
            return false;
        }
        NotifyAbilityListener(systemAbilityId, listenerSaId, deviceId, ON_ADD_SYSTEM_ABILITY_TRANSACTION);
    }

    int32_t ret = samgrProxy->SubscribeSystemAbility(systemAbilityId, OHOS::to_utf16(localAbilityManagerName));
    if (ret) {
        HILOGE(TAG, "failed to subscribe sa:%{public}d, localabilitymanager:%{public}s", systemAbilityId,
            localAbilityManagerName.c_str());
        return false;
    }

    return true;
}

bool LocalAbilityManager::RemoveSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, listenerSaId);

    if (!CheckInputSysAbilityId(systemAbilityId) || !CheckInputSysAbilityId(listenerSaId)) {
        HILOGW(TAG, "SA:%{public}d or listenerSA:%{public}d invalid!",
            systemAbilityId, listenerSaId);
        return false;
    }

    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }

    std::string localAbilityManagerName = "localabilitymanager" + std::to_string(currentPid_);
    if (!DeleteAbilityListener(systemAbilityId, listenerSaId)) {
        HILOGE(TAG, "failed to delete ability listener");
        return false;
    }

    int32_t ret = samgrProxy->UnSubscribeSystemAbility(systemAbilityId, to_utf16(localAbilityManagerName));
    if (ret) {
        HILOGE(TAG, "failed to unsubscribe SA:%{public}d, localAbilityManager:%{public}s",
            systemAbilityId, localAbilityManagerName.c_str());
        return false;
    }

    return true;
}

bool LocalAbilityManager::NotifyAbilityListener(int32_t systemAbilityId, int32_t listenerSaId, int32_t code)
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }
    std::string deviceId;
    if (!samgrProxy->GetDeviceId(deviceId)) {
        HILOGE(TAG, "failed to get deviceId");
        return false;
    }
    return NotifyAbilityListener(systemAbilityId, listenerSaId, deviceId, code);
}

bool LocalAbilityManager::NotifyAbilityListener(int32_t systemAbilityId, int32_t listenerSaId,
    const std::string& deviceId, int32_t code)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d, code:%{public}d", systemAbilityId, listenerSaId, code);

    auto ability = GetAbility(listenerSaId);
    if (ability == nullptr) {
        HILOGE(TAG, "failed to get listener SA:%{public}d", listenerSaId);
        return false;
    }

    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }

    switch (code) {
        case ON_ADD_SYSTEM_ABILITY_TRANSACTION: {
            HILOGD(TAG, "OnAddSystemAbility, SA:%{public}d", listenerSaId);
            sptr<IRemoteObject> saObject = samgrProxy->GetSystemAbility(systemAbilityId);
            ability->OnAddSystemAbility(systemAbilityId, deviceId, saObject);
            break;
        }
        case ON_REMOVE_SYSTEM_ABILITY_TRANSACTION: {
            HILOGD(TAG, "OnRemoveSystemAbility, SA:%{public}d", listenerSaId);
            ability->OnRemoveSystemAbility(systemAbilityId, deviceId);
            break;
        }
        default:
            break;
    }

    return true;
}

bool LocalAbilityManager::FindAndNotifyAbilityListeners(int32_t systemAbilityId,
    const std::string& deviceId, int32_t code)
{
    HILOGD(TAG, "SA:%{public}d, code:%{public}d", systemAbilityId, code);

    auto iter = listenerMap_.find(systemAbilityId);
    if (iter != listenerMap_.end()) {
        auto& listenerSaIdList = iter->second;
        for (auto listenerSaId : listenerSaIdList) {
            NotifyAbilityListener(systemAbilityId, listenerSaId, deviceId, code);
        }
    } else {
        HILOGW(TAG, "SA:%{public}d not found", systemAbilityId);
    }

    return true;
}

bool LocalAbilityManager::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGD(TAG, "SA:%{public}d added", systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW(TAG, "SA:%{public}d is invalid!", systemAbilityId);
        return false;
    }

    return FindAndNotifyAbilityListeners(systemAbilityId, deviceId, ON_ADD_SYSTEM_ABILITY_TRANSACTION);
}

bool LocalAbilityManager::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGD(TAG, "SA:%{public}d removed", systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW(TAG, "SA:%{public}d is invalid!", systemAbilityId);
        return false;
    }

    return FindAndNotifyAbilityListeners(systemAbilityId, deviceId, ON_REMOVE_SYSTEM_ABILITY_TRANSACTION);
}

bool LocalAbilityManager::StartAbility(int32_t systemAbilityId)
{
    HILOGD(TAG, "try to start SA:%{public}d", systemAbilityId);
    auto ability = GetAbility(systemAbilityId);
    if (ability == nullptr) {
        return false;
    }
    ability->Start();
    return true;
}

bool LocalAbilityManager::StopAbility(int32_t systemAbilityId)
{
    HILOGD(TAG, "try to stop SA:%{public}d", systemAbilityId);
    auto ability = GetAbility(systemAbilityId);
    if (ability == nullptr) {
        return false;
    }
    ability->Stop();
    return true;
}

bool LocalAbilityManager::HandoffAbilityAfter(const u16string& begin, const u16string& after)
{
    return true;
}

bool LocalAbilityManager::HandoffAbilityBegin(int32_t systemAbilityId)
{
    return true;
}

SystemAbility* LocalAbilityManager::GetAbility(int32_t systemAbilityId)
{
    std::shared_lock<std::shared_mutex> readLock(abilityMapLock_);
    auto it = abilityMap_.find(systemAbilityId);
    if (it == abilityMap_.end()) {
        return nullptr;
    }

    return it->second;
}

bool LocalAbilityManager::GetRunningStatus(int32_t systemAbilityId)
{
    auto ability = GetAbility(systemAbilityId);
    if (ability == nullptr) {
        return false;
    }

    return ability->GetRunningStatus();
}

bool LocalAbilityManager::Debug(int32_t systemAbilityId)
{
    auto ability = GetAbility(systemAbilityId);
    if (ability == nullptr) {
        return false;
    }
    ability->Debug();
    return true;
}

bool LocalAbilityManager::Test(int32_t systemAbilityId)
{
    auto ability = GetAbility(systemAbilityId);
    if (ability == nullptr) {
        return false;
    }

    ability->Test();
    return true;
}

bool LocalAbilityManager::SADump(int32_t systemAbilityId)
{
    auto ability = GetAbility(systemAbilityId);
    if (ability == nullptr) {
        return false;
    }

    ability->SADump();
    return true;
}

void LocalAbilityManager::StartOndemandSystemAbility(int32_t systemAbilityId)
{
    HILOGI(TAG, "systemAbilityId is %{public}d", systemAbilityId);
    bool isExist = profileParser_->LoadSaLib(systemAbilityId);
    if (isExist) {
        int32_t timeout = RETRY_TIMES_FOR_ONDEMAND;
        constexpr int32_t duration = std::chrono::microseconds(MILLISECONDS_WAITING_ONDEMAND_ONE_TIME).count();
        {
            std::shared_lock<std::shared_mutex> readLock(abilityMapLock_);
            auto it = abilityMap_.find(systemAbilityId);
            while (it == abilityMap_.end()) {
                HILOGI(TAG, "waiting for SA:%{public}d...", systemAbilityId);
                if (timeout > 0) {
                    usleep(duration);
                    it = abilityMap_.find(systemAbilityId);
                } else {
                    HILOGE(TAG, "waiting for SA:%{public}d time out (1s)", systemAbilityId);
                    return;
                }
                timeout--;
            }
        }

        if (!StartAbility(systemAbilityId)) {
            HILOGE(TAG, "failed to start ability:%{public}d", systemAbilityId);
        }
    } else {
        HILOGW(TAG, "SA:%{public}d not found", systemAbilityId);
    }
}

void LocalAbilityManager::StartAbilityAsyn(int32_t systemAbilityId)
{
    auto task = std::bind(&LocalAbilityManager::StartOndemandSystemAbility, this, systemAbilityId);
    ondemandPool_.AddTask(task);
}

bool LocalAbilityManager::InitializeSaProfiles()
{
    HILOGD(TAG, "initializing sa profiles...");
    auto& saProfileList = profileParser_->GetAllSaProfiles();
    if (saProfileList.empty()) {
        HILOGW(TAG, "sa profile is empty");
        return false;
    }

    std::unique_lock<std::shared_mutex> writeLock(abilityMapLock_);
    for (const auto& saProfile : saProfileList) {
        auto iterProfile = abilityMap_.find(saProfile.saId);
        if (iterProfile == abilityMap_.end()) {
            HILOGW(TAG, "SA:%{public}d not found", saProfile.saId);
            continue;
        }
        auto systemAbility = iterProfile->second;
        if (systemAbility == nullptr) {
            HILOGW(TAG, "SA:%{public}d is null", saProfile.saId);
            continue;
        }
        HILOGI(TAG, "set profile attributes for SA:%{public}d", saProfile.saId);
        systemAbility->SetLibPath(saProfile.libPath);
        systemAbility->SetRunOnCreate(saProfile.runOnCreate);
        systemAbility->SetDependSa(saProfile.dependSa);
        systemAbility->SetDependTimeout(saProfile.dependTimeout);
        systemAbility->SetDistributed(saProfile.distributed);
        systemAbility->SetDumpLevel(saProfile.dumpLevel);
        systemAbility->SetCapability(saProfile.capability);
        systemAbility->SetPermission(saProfile.permission);

        uint32_t phase = OTHER_START;
        if (saProfile.bootPhase == BOOT_START_PHASE) {
            phase = BOOT_START;
        } else if (saProfile.bootPhase == CORE_START_PHASE) {
            phase = CORE_START;
        }
        auto& saList = abilityPhaseMap_[phase];
        saList.emplace_back(systemAbility);
    }
    return true;
}

vector<u16string> LocalAbilityManager::CheckDependencyStatus(const vector<u16string>& dependSa)
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGW(TAG, "failed to get samgrProxy");
        return dependSa;
    }
    vector<u16string> checkSaStatusResult;
    for (const auto& saName : dependSa) {
        int32_t systemAbilityId = atoi(Str16ToStr8(saName).c_str());
        if (CheckInputSysAbilityId(systemAbilityId)) {
            sptr<IRemoteObject> saObject = samgrProxy->CheckSystemAbility(systemAbilityId);
            if (saObject == nullptr) {
                checkSaStatusResult.emplace_back(saName);
            }
        } else {
            HILOGW(TAG, "dependency's id:%{public}s is invalid", Str16ToStr8(saName).c_str());
        }
    }

    return checkSaStatusResult;
}

void LocalAbilityManager::StartSystemAbilityTask(SystemAbility* ability)
{
    if (ability != nullptr) {
        HILOGD(TAG, "StartSystemAbility is called for %{public}d", ability->GetSystemAbilitId());
        if (ability->GetDependSa().empty()) {
            ability->Start();
        } else {
            int64_t start = GetTickCount();
            int64_t dependTimeout = ability->GetDependTimeout();
            while (!CheckDependencyStatus(ability->GetDependSa()).empty()) {
                int64_t end = GetTickCount();
                int64_t duration = ((end >= start) ? (end - start) : (INT64_MAX - end + start));
                if (duration < dependTimeout) {
                    usleep(CHECK_DEPENDENT_SA_PERIOD);
                } else {
                    break;
                }
            }
            vector<u16string> unpreparedDeps = CheckDependencyStatus(ability->GetDependSa());
            if (unpreparedDeps.empty()) {
                ability->Start();
            } else {
                for (const auto& unpreparedDep : unpreparedDeps) {
                    HILOGI(TAG, "%{public}d's dependency:%{public}s not started in %{public}d ms",
                        ability->GetSystemAbilitId(), Str16ToStr8(unpreparedDep).c_str(), ability->GetDependTimeout());
                }
            }
        }
    }

    std::lock_guard<std::mutex> lock(startPhaseLock_);
    if (startTaskNum_ > 0) {
        --startTaskNum_;
    }
    startPhaseCV_.notify_one();
}

void LocalAbilityManager::RegisterOnDemandSystemAbility()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGI(TAG, "failed to get samgrProxy");
        return;
    }

    std::string localAbilityManagerName = "localabilitymanager" + std::to_string(currentPid_);
    auto& saProfileList = profileParser_->GetAllSaProfiles();
    for (const auto& saProfile : saProfileList) {
        HILOGD(TAG, "register ondemand ability:%{public}d to samgr", saProfile.saId);
        if (!saProfile.runOnCreate) {
            int32_t ret = samgrProxy->AddOnDemandSystemAbilityInfo(saProfile.saId,
                OHOS::to_utf16(localAbilityManagerName));
            if (ret != ERR_OK) {
                HILOGI(TAG, "failed to add ability info for on-demand SA:%{public}d", saProfile.saId);
            }
        }
    }
}

void LocalAbilityManager::AddSamgrDeathRecipient()
{
    sptr<IRemoteObject> registryObject = SystemAbilityManagerClient::GetInstance().GetRegistryRemoteObject();
    if (registryObject == nullptr) {
        HILOGD(TAG, "failed to get registry object");
        return;
    }
    auto recipient = sptr<IRemoteObject::DeathRecipient>(new SamgrDeathRecipient());
    bool result = registryObject->AddDeathRecipient(recipient);
    HILOGD(TAG, "%{public}s to add death recipient", result ? "success" : "failed");
}

void LocalAbilityManager::StartPhaseTasks(const std::list<SystemAbility*>& systemAbilityList)
{
    if (systemAbilityList.empty()) {
        return;
    }

    for (auto systemAbility : systemAbilityList) {
        if (systemAbility != nullptr && systemAbility->IsRunOnCreate()) {
            HILOGD(TAG, "add phase task for SA:%{public}d", systemAbility->GetSystemAbilitId());
            std::lock_guard<std::mutex> autoLock(startPhaseLock_);
            ++startTaskNum_;
            auto task = std::bind(&LocalAbilityManager::StartSystemAbilityTask, this, systemAbility);
            pool_.AddTask(task);
        }
    }

    int64_t begin = GetTickCount();
    HILOGI(TAG, "start waiting for all tasks!");
    std::unique_lock<std::mutex> lck(startPhaseLock_);
    auto now = std::chrono::system_clock::now();
    if (!startPhaseCV_.wait_until(lck, now + std::chrono::seconds(MAX_SA_STARTUP_TIME),
        [this] () { return startTaskNum_ == 0; })) {
        HILOGW(TAG, "start timeout!");
    }
    startTaskNum_ = 0;
    int64_t end = GetTickCount();
    HILOGI(TAG, "start tasks finished and spend %{public}" PRId64 " ms", (end - begin));
}

void LocalAbilityManager::FindAndStartPhaseTasks()
{
    std::shared_lock<std::shared_mutex> readLock(abilityMapLock_);
    for (uint32_t startType = BOOT_START; startType <= OTHER_START; ++startType) {
        auto iter = abilityPhaseMap_.find(startType);
        if (iter != abilityPhaseMap_.end()) {
            StartPhaseTasks(iter->second);
        }
    }
}

bool LocalAbilityManager::Run()
{
    HILOGD(TAG, "local ability manager is running...");
    std::string localAbilityManagerName = "localabilitymanager" + std::to_string(currentPid_);
    bool addResult = AddLocalAbilityManager(localAbilityManagerName);
    if (!addResult) {
        HILOGE(TAG, "failed to add local abilitymanager");
        return false;
    }
    HILOGI(TAG, "success to add local ability manager:%{public}s", localAbilityManagerName.c_str());

    AddSamgrDeathRecipient();
    bool startResult = StartAllAddAbilityListener();
    HILOGI(TAG, "%{public}s to start all Listeners for ability add", startResult ? "success" : "failed");

    uint32_t concurrentThreads = std::thread::hardware_concurrency();
    HILOGD(TAG, "concurrentThreads is %{public}d", concurrentThreads);
    pool_.Start(concurrentThreads);
    pool_.SetMaxTaskNum(MAX_TASK_NUMBER);

    FindAndStartPhaseTasks();
    RegisterOnDemandSystemAbility();
    pool_.Stop();
    return true;
}

bool LocalAbilityManager::AddLocalAbilityManager(const std::string& localAbilityMgrName)
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }

    if (localAbilityManager_ == nullptr) {
        localAbilityManager_ = this;
    }
    int32_t ret = samgrProxy->AddLocalAbilityManager(OHOS::to_utf16(localAbilityMgrName), localAbilityManager_);
    return ret == ERR_OK;
}

bool LocalAbilityManager::ReRegisterSA()
{
    HILOGI(TAG, "try to register SA again...");
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }

    std::string localAbilityManagerName = "localabilitymanager" + std::to_string(currentPid_);
    int32_t ret = samgrProxy->AddLocalAbilityManager(OHOS::to_utf16(localAbilityManagerName), this);
    HILOGD(TAG, "%{public}s to add local ability manager", (ret == ERR_OK) ? "success" : "failed");

    std::shared_lock<std::shared_mutex> readLock(abilityMapLock_);
    for (const auto& [abilityId, ability] : abilityMap_) {
        if (ability != nullptr) {
            HILOGD(TAG, "%{public}s to republish ability:%{public}d", ability->RePublish() ? "success" : "failed",
                abilityId);
        }
    }
    return true;
}
}
