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

#include "system_ability_manager.h"

#include <cinttypes>
#include <unistd.h>

#include "ability_death_recipient.h"
#include "datetime_ex.h"
#include "errors.h"
#include "if_local_ability_manager.h"
#include "local_ability_manager_proxy.h"
#include "nlohmann/json.hpp"

#include "sam_log.h"
#include "string_ex.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"

#include "tools.h"
#include "utils.h"

using namespace std;

namespace OHOS {
namespace {
const string STR_LOCAL_ABILITY_MGR = "localabilitymanager";
const string CORE_SYSTEM_ABILITY_PATH = "/system/profile/core_system_ability.xml";
const string SYSTEM_CAPABILITY_PATH = "/system/profile/system_capability.json";
const string SYSTEM_PARAM_SAMGR_CORESA_INITREADY = "sys.samgr.coresa.initready";
const std::u16string SYSTEM_READY_CALLBACK_INTERFACE_TOKEN = u"ohos.systemReadyCallback.accessToken";
constexpr int32_t MAX_NAME_SIZE = 200;
constexpr int32_t SPLIT_NAME_VECTOR_SIZE = 2;
constexpr int32_t MAX_CAPABILITY_SIZE = 8192;

constexpr int32_t UID_ROOT = 0;
constexpr int32_t UID_SYSTEM = 1000;

constexpr int32_t DUMP_ALL_CORE_SYSTEM_ABILITY = 0;
constexpr int32_t DUMP_REGISTERED_CORE_SYSTEM_ABILITY = 1;
constexpr int32_t DUMP_UNREGISTERED_CORE_SYSTEM_ABILITY = 2;
constexpr int32_t MAX_SYSCAP_NAME_LEN = 64;
}

std::mutex SystemAbilityManager::instanceLock;
sptr<SystemAbilityManager> SystemAbilityManager::instance;

SystemAbilityManager::SystemAbilityManager()
{
    dBinderService_ = DBinderService::GetInstance();
}

SystemAbilityManager::~SystemAbilityManager()
{
}

void SystemAbilityManager::Init()
{
    abilityDeath_ = sptr<IRemoteObject::DeathRecipient>(new AbilityDeathRecipient());
    localAbilityManagerDeath_ = sptr<IRemoteObject::DeathRecipient>(new LocalAbilityManagerDeathRecipient());
    systemReadyCallbackDeath_ = sptr<IRemoteObject::DeathRecipient>(new SystemReadyCallbackDeathRecipient());
    InitCoreSaList();
    InitSysCapMap();
}

const sptr<DBinderService> SystemAbilityManager::GetDBinder() const
{
    return dBinderService_;
}

sptr<SystemAbilityManager> SystemAbilityManager::GetInstance()
{
    std::lock_guard<std::mutex> autoLock(instanceLock);
    if (instance == nullptr) {
        instance = new SystemAbilityManager;
    }
    return instance;
}

void SystemAbilityManager::DoSADataStorageInit()
{
}

void SystemAbilityManager::InitCoreSaList()
{
}

void SystemAbilityManager::RestoreCoreSaId(int32_t saId)
{
}

void SystemAbilityManager::RemoveCompletedCoreSaId(int32_t saId)
{
}

void SystemAbilityManager::SendSystemReadyMessage()
{
}

void SystemAbilityManager::SendSingleSystemReadyMessage(const sptr<IRemoteObject>& systemReadyCallback)
{
}

sptr<IRemoteObject> SystemAbilityManager::GetSystemAbility(int32_t systemAbilityId)
{
    return CheckSystemAbility(systemAbilityId);
}

bool SystemAbilityManager::GetSystemAbilityInfoList(int32_t systemAbilityId,
    const std::u16string& capability, std::list<std::shared_ptr<SystemAbilityInfo>>& saInfoList)
{
    return false;
}

sptr<IRemoteObject> SystemAbilityManager::CheckLocalAbilityManager(const u16string& localAbilityManagerName)
{
    if (localAbilityManagerName.empty()) {
        HILOGW("CheckLocalAbilityManager empty name!");
        return nullptr;
    }

    lock_guard<recursive_mutex> autoLock(localAbilityManagerMapLock_);
    auto iter = localAbilityManagerMap_.find(localAbilityManagerName);
    if (iter != localAbilityManagerMap_.end()) {
        HILOGI("localAbilityMgr %{public}s found", Str16ToStr8(localAbilityManagerName).c_str());
        return iter->second;
    }
    HILOGE("localAbilityMgr %{public}s not exist", Str16ToStr8(localAbilityManagerName).c_str());

    return nullptr;
}

sptr<IRemoteObject> SystemAbilityManager::GetSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    return CheckSystemAbility(systemAbilityId, deviceId);
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId)
{
    HILOGD("%{public}s called, systemAbilityId = %{public}d", __func__, systemAbilityId);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("CheckSystemAbility CheckSystemAbility invalid!");
        return nullptr;
    }

    // find the suited sa
    std::string selectedDeviceId;
    if (CheckRemoteSa(to_string(systemAbilityId), selectedDeviceId)) {
        // already find the remote deviceId's sa is suited
        return CheckSystemAbility(systemAbilityId, selectedDeviceId);
    }

    shared_lock<shared_mutex> readLock(abilityMapLock_);
    auto iter = abilityMap_.find(systemAbilityId);
    if (iter != abilityMap_.end()) {
        auto callingUid = IPCSkeleton::GetCallingUid();
        if (IsSystemApp(callingUid) || CheckPermission(iter->second.permission)) {
            HILOGI("finded service : %{public}d.", systemAbilityId);
            return iter->second.remoteObj;
        }
        HILOGE("CheckSystemAbility systemAbilityId: %{public}d PERMISSION DENIED", systemAbilityId);
        return nullptr;
    }
    HILOGI("NOT finded service : %{public}d", systemAbilityId);
    return nullptr;
}

bool SystemAbilityManager::CheckDistributedPermission()
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != UID_ROOT && callingUid != UID_SYSTEM) {
        return false;
    }
    return true;
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    sptr<DBinderServiceStub> remoteBinder = nullptr;
    if (dBinderService_ != nullptr) {
        string strName = to_string(systemAbilityId);
        remoteBinder = dBinderService_->MakeRemoteBinder(Str8ToStr16(strName), deviceId, systemAbilityId, 0);
        HILOGI("CheckSystemAbility, MakeRemoteBinder, systemAbilityId is %{public}d, deviceId is %s",
            systemAbilityId, deviceId.c_str());
        if (remoteBinder == nullptr) {
            HILOGE("MakeRemoteBinder error, remoteBinder is null");
        }
    }
    return remoteBinder;
}

int32_t SystemAbilityManager::FindSystemAbilityManagerNotify(int32_t systemAbilityId, int32_t code)
{
    return ERR_NO_INIT;
}

int32_t SystemAbilityManager::FindSystemAbilityManagerNotify(int32_t systemAbilityId, const std::string& deviceId,
    int32_t code)
{
    HILOGI("%s called:systemAbilityId = %{public}d, code = %{public}d", __func__, systemAbilityId, code);
    lock_guard<recursive_mutex> autoLock(listenerMapLock_);
    auto iter = listenerMap_.find(systemAbilityId);
    if (iter != listenerMap_.end()) {
        auto& listenerNames = iter->second;
        for (const auto& listenerName : listenerNames) {
            HILOGI("%s called:listenerName = %{public}s", __func__, Str16ToStr8(listenerName).c_str());
            sptr<IRemoteObject> object = CheckLocalAbilityManager(listenerName);
            sptr<ILocalAbilityManager> localAbilityManagerService = iface_cast<ILocalAbilityManager>(object);
            if (localAbilityManagerService == nullptr) {
                HILOGE(" %s get service fail", __func__);
                continue;
            }

            switch (code) {
                case ADD_SYSTEM_ABILITY_TRANSACTION: {
                    localAbilityManagerService->OnAddSystemAbility(systemAbilityId, deviceId);
                    break;
                }
                case REMOVE_SYSTEM_ABILITY_TRANSACTION: {
                    localAbilityManagerService->OnRemoveSystemAbility(systemAbilityId, deviceId);
                    break;
                }
                default:
                    break;
            }
        }
    }

    return ERR_OK;
}

bool SystemAbilityManager::IsNameInValid(const std::u16string& name)
{
    HILOGI("%{public}s called:name = %{public}s", __func__, Str16ToStr8(name).c_str());
    bool ret = false;
    if (name.empty() || name.size() > MAX_NAME_SIZE || DeleteBlank(name).empty()) {
        ret = true;
    }

    return ret;
}

int32_t SystemAbilityManager::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& localAbilityManagerName)
{
    HILOGI("%{public}s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId) || IsNameInValid(localAbilityManagerName)) {
        HILOGI("var is invalid.");
        return ERR_INVALID_VALUE;
    }

    lock_guard<recursive_mutex> autoLock(onDemandAbilityMapLock_);
    auto onDemandSaSize = onDemandAbilityMap_.size();
    if (onDemandSaSize >= MAX_SERVICES) {
        HILOGE("map size error, (Has been greater than %{public}zu)",
            onDemandAbilityMap_.size());
        return ERR_INVALID_VALUE;
    }

    onDemandAbilityMap_[systemAbilityId] = localAbilityManagerName;
    HILOGI("insert %{public}d. size : %{public}zu", systemAbilityId, onDemandAbilityMap_.size());
    return ERR_OK;
}

int32_t SystemAbilityManager::RecycleOnDemandSystemAbility()
{
    HILOGI("%{public}s called", __func__);
    bool ret = false;
    int32_t refCount = 0;

    lock_guard<recursive_mutex> autoLock(onDemandAbilityMapLock_);
    for (const auto& [systemAbilityId, localName] : onDemandAbilityMap_) {
        sptr<IRemoteObject> abilityProxy = CheckSystemAbility(systemAbilityId);
        if (abilityProxy == nullptr) {
            continue;
        }
        refCount = abilityProxy->GetSptrRefCount();
        if (refCount == 1) {
            sptr<ILocalAbilityManager> localAbilityManagerService =
                iface_cast<ILocalAbilityManager>(CheckLocalAbilityManager(localName));
            if (localAbilityManagerService == nullptr) {
                HILOGI("get local ability %{public}s fail", Str16ToStr8(localName).c_str());
                continue;
            }
            ret = localAbilityManagerService->StopAbility(systemAbilityId);
            if (!ret) {
                HILOGI("stop ability %{public}d fail.", systemAbilityId);
                continue;
            }
            ret = localAbilityManagerService->RecycleOndemandSystemAbility(systemAbilityId);
            if (!ret) {
                HILOGI("RecycleOndemandSystemAbility ability %{public}d fail.", systemAbilityId);
                continue;
            }
        }
    }

    return ERR_OK;
}

int32_t SystemAbilityManager::StartOnDemandAbility(int32_t systemAbilityId)
{
    HILOGI("%{public}s called, systemAbilityId is %{public}d", __func__, systemAbilityId);
    lock_guard<recursive_mutex> onDemandAbilityLock(onDemandAbilityMapLock_);
    auto iter = onDemandAbilityMap_.find(systemAbilityId);
    if (iter != onDemandAbilityMap_.end()) {
        HILOGI("finded onDemandAbility: %{public}d.", systemAbilityId);
        sptr<ILocalAbilityManager> localAbilityManagerService =
            iface_cast<ILocalAbilityManager>(CheckLocalAbilityManager(iter->second));
        if (localAbilityManagerService == nullptr) {
            HILOGI("get local ability %{public}s fail", Str16ToStr8(iter->second).c_str());
            return ERR_OK;
        }
        localAbilityManagerService->StartAbilityAsyn(systemAbilityId);
        startingAbilityList_.emplace_back(systemAbilityId);
        return ERR_OK;
    }

    return ERR_INVALID_VALUE;
}

sptr<IRemoteObject> SystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        return nullptr;
    }
    sptr<IRemoteObject> abilityProxy = CheckSystemAbility(systemAbilityId);
    if (abilityProxy == nullptr) {
        {
            lock_guard<recursive_mutex> autoLock(onDemandAbilityMapLock_);
            for (int32_t startingAbilityId : startingAbilityList_) {
                if (startingAbilityId == systemAbilityId) {
                    isExist = true;
                    return nullptr;
                }
            }
        }

        int32_t ret = StartOnDemandAbility(systemAbilityId);
        if (ret == ERR_OK) {
            isExist = true;
            return nullptr;
        }

        HILOGI("ability %{public}d is not found", systemAbilityId);
        isExist = false;
        return nullptr;
    }

    isExist = true;
    return abilityProxy;
}

int32_t SystemAbilityManager::ConnectSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityConnectionCallback>& connectionCallback)
{
    HILOGI("%{public}s called, systemAbilityId is %{public}d", __func__, systemAbilityId);
    if (connectionCallback == nullptr) {
        HILOGI("connectionAbility is null.");
        return ERR_INVALID_VALUE;
    }

    lock_guard<recursive_mutex> autoLock(onDemandAbilityMapLock_);
    auto iter = onDemandAbilityMap_.find(systemAbilityId);
    if (iter != onDemandAbilityMap_.end()) {
        HILOGI("finded onDemandAbility: %{public}d.", systemAbilityId);
        sptr<ILocalAbilityManager> localAbilityManagerService =
            iface_cast<ILocalAbilityManager>(CheckLocalAbilityManager(iter->second));
        if (localAbilityManagerService == nullptr) {
            HILOGI("%{public}s:get local service fail!", __func__);
            connectionCallback->OnConnectedSystemAbility(nullptr);
            return ERR_INVALID_VALUE;
        }
        localAbilityManagerService->StartAbilityAsyn(systemAbilityId);
    }
    connectionCallbackMap_[systemAbilityId] = connectionCallback;
    HILOGI("insert %{public}d. size : %{public}zu", systemAbilityId, connectionCallbackMap_.size());

    return ERR_OK;
}

int32_t SystemAbilityManager::DisConnectSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityConnectionCallback>& connectionCallback)
{
    HILOGI("%{public}s called, systemAbilityId is %{public}d", __func__, systemAbilityId);
    if (connectionCallback == nullptr) {
        HILOGI("connectionCallback is null.");
        return ERR_INVALID_VALUE;
    }

    {
        lock_guard<recursive_mutex> autoLock(onDemandAbilityMapLock_);
        (void)connectionCallbackMap_.erase(systemAbilityId);
    }

    connectionCallback->OnDisConnectedSystemAbility(systemAbilityId);
    return ERR_OK;
}

bool SystemAbilityManager::CheckCapability(const std::u16string& capability)
{
    if (capability.empty()) {
        return true;
    }
    if (capability.size() > MAX_CAPABILITY_SIZE) {
        return false;
    }

    auto json = nlohmann::json::parse(Str16ToStr8(capability), nullptr, false);
    if (json.is_discarded()) {
        HILOGW("SystemAbilityManager::CheckCapability exception");
        return false;
    }
    return true;
}

int32_t SystemAbilityManager::AddLocalAbilityManager(const u16string& localAbilityManagerName,
    const sptr<IRemoteObject>& localAbilityManager)
{
    if (localAbilityManagerName.empty() || localAbilityManager == nullptr) {
        HILOGE("SystemAbilityManager::AddLocalAbilityManager empty name or null manager object!");
        return ERR_INVALID_VALUE;
    }
    if (Str16ToStr8(localAbilityManagerName) != (STR_LOCAL_ABILITY_MGR + to_string(IPCSkeleton::GetCallingPid()))) {
        HILOGE("SystemAbilityManager::AddLocalAbilityManager please call AddSystemAbility function!");
        return ERR_INVALID_VALUE;
    }

    lock_guard<recursive_mutex> autoLock(localAbilityManagerMapLock_);
    size_t managerNum = localAbilityManagerMap_.size();
    if (managerNum >= MAX_SERVICES) {
        HILOGE("SystemAbilityManager::AddLocalAbilityManager map size reach MAX_SERVICES already");
        return ERR_INVALID_VALUE;
    }
    localAbilityManagerMap_[localAbilityManagerName] = localAbilityManager;

    if (localAbilityManagerDeath_ != nullptr) {
        localAbilityManager->AddDeathRecipient(localAbilityManagerDeath_);
    }

    return ERR_OK;
}

void SystemAbilityManager::DoInsertSaData(const u16string& strName,
    const sptr<IRemoteObject>& ability, const SAExtraProp& extraProp)
{
    HILOGW("AddSystemAbility samgrHandler_ Insert fail");
}

void SystemAbilityManager::DeleteStartingAbilityMember(int32_t systemAbilityId)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        return;
    }
    lock_guard<recursive_mutex> autoLock(onDemandAbilityMapLock_);
    for (auto iter = startingAbilityList_.begin(); iter != startingAbilityList_.end();) {
        if (*iter == systemAbilityId) {
            iter = startingAbilityList_.erase(iter);
        } else {
            ++iter;
        }
    }
}

int32_t SystemAbilityManager::RemoveSystemAbility(int32_t systemAbilityId)
{
    HILOGI("%s called (name)", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        HILOGW("RemoveSystemAbility systemAbilityId:%{public}d", systemAbilityId);
        return ERR_INVALID_VALUE;
    }
    {
        unique_lock<shared_mutex> writeLock(abilityMapLock_);
        auto itSystemAbility = abilityMap_.find(systemAbilityId);
        if (itSystemAbility == abilityMap_.end()) {
            HILOGI("SystemAbilityManager::RemoveSystemAbility not found!");
            return ERR_INVALID_VALUE;
        }
        sptr<IRemoteObject> ability = itSystemAbility->second.remoteObj;
        if (ability != nullptr && abilityDeath_ != nullptr) {
            ability->RemoveDeathRecipient(abilityDeath_);
        }
        (void)abilityMap_.erase(itSystemAbility);
        RestoreCoreSaId(systemAbilityId);
        HILOGI("%s called, systemAbilityId : %{public}d, size : %{public}zu", __func__, systemAbilityId,
            abilityMap_.size());
    }

    FindSystemAbilityManagerNotify(systemAbilityId, REMOVE_SYSTEM_ABILITY_TRANSACTION);
    DeleteStartingAbilityMember(systemAbilityId);
    return ERR_OK;
}

int32_t SystemAbilityManager::RemoveLocalAbilityManager(const u16string& localAbilityManagerName)
{
    if (localAbilityManagerName.empty()) {
        HILOGE("SystemAbilityManager::RemoveLocalAbilityManager empty name!");
        return ERR_INVALID_VALUE;
    }

    lock_guard<recursive_mutex> autoLock(localAbilityManagerMapLock_);
    auto iter = localAbilityManagerMap_.find(localAbilityManagerName);
    if (iter == localAbilityManagerMap_.end()) {
        HILOGE("SystemAbilityManager::RemoveLocalAbilityManager name not exist!");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> localAbilityManager = iter->second;
    if (localAbilityManager != nullptr && localAbilityManagerDeath_ != nullptr) {
        localAbilityManager->RemoveDeathRecipient(localAbilityManagerDeath_);
    }
    localAbilityManagerMap_.erase(iter);

    return ERR_OK;
}

int32_t SystemAbilityManager::RemoveLocalAbilityManager(const sptr<IRemoteObject>& localAbilityManager)
{
    HILOGI("SystemAbilityManager::RemoveLocalAbilityManager called");
    if (localAbilityManager == nullptr) {
        HILOGW("SystemAbilityManager::RemoveLocalAbilityManager null manager object!");
        return ERR_INVALID_VALUE;
    }

    lock_guard<recursive_mutex> autoLock(localAbilityManagerMapLock_);
    for (const auto& [managerName, manager] : localAbilityManagerMap_) {
        if (manager == localAbilityManager) {
            (void)localAbilityManagerMap_.erase(managerName);
            if (localAbilityManagerDeath_ != nullptr) {
                localAbilityManager->RemoveDeathRecipient(localAbilityManagerDeath_);
            }
            break;
        }
    }

    return ERR_OK;
}

int32_t SystemAbilityManager::RemoveSystemAbility(const sptr<IRemoteObject>& ability)
{
    HILOGI("%s called, (ability)", __func__);
    if (ability == nullptr) {
        HILOGW("ability is nullptr ");
        return ERR_INVALID_VALUE;
    }

    int32_t saId = 0;
    {
        unique_lock<shared_mutex> writeLock(abilityMapLock_);
        for (auto iter = abilityMap_.begin(); iter != abilityMap_.end(); ++iter) {
            if (iter->second.remoteObj == ability) {
                saId = iter->first;
                (void)abilityMap_.erase(iter);
                RestoreCoreSaId(saId);
                if (abilityDeath_ != nullptr) {
                    ability->RemoveDeathRecipient(abilityDeath_);
                }
                HILOGI("%s called, systemAbilityId:%{public}d removed, size : %zu", __func__, saId,
                    abilityMap_.size());
                break;
            }
        }
    }

    if (saId != 0) {
        FindSystemAbilityManagerNotify(saId, REMOVE_SYSTEM_ABILITY_TRANSACTION);
        DeleteStartingAbilityMember(saId);
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::RemoveSystemReadyCallback(const sptr<IRemoteObject>& callback)
{
    return ERR_INVALID_VALUE;
}

vector<u16string> SystemAbilityManager::ListSystemAbilities(uint32_t dumpFlags)
{
    vector<u16string> list;
    shared_lock<shared_mutex> readLock(abilityMapLock_);
    for (auto iter = abilityMap_.begin(); iter != abilityMap_.end(); iter++) {
        list.emplace_back(Str8ToStr16(to_string(iter->first)));
    }
    return list;
}

u16string SystemAbilityManager::GetSystemAbilityName(int32_t index)
{
    shared_lock<shared_mutex> readLock(abilityMapLock_);
    if (index < 0 || static_cast<uint32_t>(index) >= abilityMap_.size()) {
        return u16string();
    }
    int32_t count = 0;
    for (auto iter = abilityMap_.begin(); iter != abilityMap_.end(); iter++) {
        if (count++ == index) {
            return Str8ToStr16(to_string(iter->first));
        }
    }
    return u16string();
}

int32_t SystemAbilityManager::SubscribeSystemAbility(int32_t systemAbilityId, const u16string& listenerName)
{
    HILOGI("%s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId) || listenerName.empty()) {
        HILOGW("SubscribeSystemAbility systemAbilityId or listenerName invalid!");
        return ERR_INVALID_VALUE;
    }

    lock_guard<recursive_mutex> autoLock(listenerMapLock_);
    HILOGD("systemAbilityId = %{public}d, listenerName = %{public}s, size = %{public}zu",
        systemAbilityId, Str16ToStr8(listenerName).c_str(), listenerMap_.size());
    auto& listenerNames = listenerMap_[systemAbilityId];
    bool found = false;
    for (const auto& itemName : listenerNames) {
        if (itemName == listenerName) {
            found = true;
            HILOGI("already exist systemAbilityId = %{public}d, listenerName = %{public}s",
                systemAbilityId, Str16ToStr8(listenerName).c_str());
            break;
        }
    }
    if (!found) {
        listenerNames.emplace_back(listenerName);
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::UnSubscribeSystemAbility(int32_t systemAbilityId, const u16string& listenerName)
{
    HILOGI("%s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId) || listenerName.empty()) {
        HILOGW("UnSubscribeSystemAbility systemAbilityId or listenerSaId invalid!");
        return ERR_INVALID_VALUE;
    }

    lock_guard<recursive_mutex> autoLock(listenerMapLock_);
    HILOGD("systemAbilityId = %{public}d, listenerName = %{public}s, size = %{public}zu",
        systemAbilityId, Str16ToStr8(listenerName).c_str(), listenerMap_.size());
    auto& listenerNames = listenerMap_[systemAbilityId];
    for (auto iter = listenerNames.begin(); iter != listenerNames.end();) {
        if (*iter == listenerName) {
            iter = listenerNames.erase(iter);
            break;
        } else {
            ++iter;
        }
    }
    return ERR_OK;
}

const std::u16string SystemAbilityManager::CheckOnDemandSystemAbility(int32_t systemAbilityId)
{
    HILOGI("%{public}s called, systemAbilityId is %{public}d", __func__, systemAbilityId);
    lock_guard<recursive_mutex> autoLock(onDemandAbilityMapLock_);
    auto iter = onDemandAbilityMap_.find(systemAbilityId);
    if (iter != onDemandAbilityMap_.end()) {
        HILOGI("finded onDemandAbility: %{public}d.", systemAbilityId);
        return iter->second;
    }

    return u16string();
}

void SystemAbilityManager::SetDeviceName(const u16string &name)
{
    deviceName_ = name;
}

const u16string& SystemAbilityManager::GetDeviceName() const
{
    return deviceName_;
}

bool SystemAbilityManager::GetDeviceId(string& deviceId)
{
    return false;
}

void SystemAbilityManager::NotifyRemoteSaDied(const std::u16string& name)
{
    std::u16string saName;
    std::string deviceId;
    ParseRemoteSaName(name, deviceId, saName);
    if (dBinderService_ != nullptr) {
        std::string nodeId = TransformDeviceId(deviceId, NODE_ID, false);
        dBinderService_->NoticeServiceDie(saName, nodeId);
        HILOGD("NotifyRemoteSaDied, serviceName is %s, deviceId is %s",
            Str16ToStr8(saName).c_str(), nodeId.c_str());
    }
}

void SystemAbilityManager::NotifyRemoteDeviceOffline(const std::string& deviceId)
{
    if (dBinderService_ != nullptr) {
        dBinderService_->NoticeDeviceDie(deviceId);
        HILOGD("NotifyRemoteDeviceOffline, deviceId is %s", deviceId.c_str());
    }
}

void SystemAbilityManager::ParseRemoteSaName(const std::u16string& name, std::string& deviceId, std::u16string& saName)
{
    vector<string> strVector;
    SplitStr(Str16ToStr8(name), "_", strVector);
    if (strVector.size() == SPLIT_NAME_VECTOR_SIZE) {
        deviceId = strVector[0];
        saName = Str8ToStr16(strVector[1]);
    }
}

void SystemAbilityManager::OnDemandConnected(int32_t systemAbilityId, const sptr<IRemoteObject>& ability)
{
    if (!CheckInputSysAbilityId(systemAbilityId)) {
        return;
    }
    sptr<ISystemAbilityConnectionCallback> connectionAbility;
    {
        lock_guard<recursive_mutex> autoLock(onDemandAbilityMapLock_);
        auto iter = connectionCallbackMap_.find(systemAbilityId);
        if (iter != connectionCallbackMap_.end()) {
            connectionAbility = iter->second;
        }
    }
    if (connectionAbility != nullptr) {
        connectionAbility->OnConnectedSystemAbility(ability);
    }
}

int32_t SystemAbilityManager::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
    const SAExtraProp& extraProp)
{
    HILOGI("%s called", __func__);
    if (!CheckInputSysAbilityId(systemAbilityId) || ability == nullptr || (!CheckCapability(extraProp.capability))) {
        HILOGE("AddSystemAbilityExtra input params is invalid.");
        return ERR_INVALID_VALUE;
    }
    {
        unique_lock<shared_mutex> writeLock(abilityMapLock_);
        auto saSize = abilityMap_.size();
        if (saSize >= MAX_SERVICES) {
            HILOGE("map size error, (Has been greater than %zu)", saSize);
            return ERR_INVALID_VALUE;
        }
        SAInfo saInfo;
        saInfo.remoteObj = ability;
        saInfo.isDistributed = extraProp.isDistributed;
        saInfo.capability = extraProp.capability;
        saInfo.permission = Str16ToStr8(extraProp.permission);
        abilityMap_[systemAbilityId] = std::move(saInfo);
        RemoveCompletedCoreSaId(systemAbilityId);
        HILOGI("insert %{public}d. size : %zu,", systemAbilityId, abilityMap_.size());
    }
    if (abilityDeath_ != nullptr) {
        ability->AddDeathRecipient(abilityDeath_);
    }
    FindSystemAbilityManagerNotify(systemAbilityId, ADD_SYSTEM_ABILITY_TRANSACTION);
    u16string strName = Str8ToStr16(to_string(systemAbilityId));
    if (extraProp.isDistributed && dBinderService_ != nullptr) {
        dBinderService_->RegisterRemoteProxy(strName, systemAbilityId);
        HILOGD("AddSystemAbility RegisterRemoteProxy, serviceId is %{public}d", systemAbilityId);
    }
    if (systemAbilityId == SOFTBUS_SERVER_SA_ID && !isDbinderStart_) {
        if (dBinderService_ != nullptr) {
            bool ret = dBinderService_->StartDBinderService();
            HILOGI("startd dbinder service result is %{public}s", ret ? "ok" : "fail");
            isDbinderStart_ = true;
        }
    }
    return ERR_OK;
}

int32_t SystemAbilityManager::RegisterSystemReadyCallback(const sptr<IRemoteObject>& systemReadyCallback)
{
    return ERR_OK;
}

int32_t SystemAbilityManager::GetCoreSystemAbilityList(vector<int32_t>& coreSaList, int dumpMode)
{
    if (dumpMode > DUMP_UNREGISTERED_CORE_SYSTEM_ABILITY || dumpMode < DUMP_ALL_CORE_SYSTEM_ABILITY) {
        return ERR_INVALID_VALUE;
    }
    set<int32_t> coreSaIdSet;
    bool result = Utils::ParseCoreSaList(CORE_SYSTEM_ABILITY_PATH, coreSaIdSet);
    HILOGD("coreSa::GetCoreSystemAbilityList coreSaIdSet: %{public}zu result:%d", coreSaIdSet.size(), result);
    if (coreSaIdSet.empty()) {
        HILOGE("coreSa::GetCoreSystemAbilityList parse core salist error, add the default core sa!!");
        AddDefaultCoreSa(coreSaIdSet);
    }
    for (int32_t saId : coreSaIdSet) {
        if (dumpMode == DUMP_REGISTERED_CORE_SYSTEM_ABILITY) {
            if (GetSystemAbility(saId) == nullptr) {
                continue;
            }
        } else if (dumpMode == DUMP_UNREGISTERED_CORE_SYSTEM_ABILITY) {
            if (GetSystemAbility(saId) != nullptr) {
                continue;
            }
        }
        coreSaList.push_back(saId);
    }
    return ERR_OK;
}

bool SystemAbilityManager::IsLocalDeviceId(const std::string& deviceId)
{
    return false;
}

bool SystemAbilityManager::CheckRemoteSa(const std::string& saName, std::string& selectedDeviceId)
{
    return false;
}

bool SystemAbilityManager::CheckPermission(const std::string& permission)
{
    return true;
}

std::string SystemAbilityManager::TransformDeviceId(const std::string& deviceId, int32_t type, bool isPrivate)
{
    return isPrivate ? std::string() : deviceId;
}

std::string SystemAbilityManager::GetLocalNodeId()
{
    return std::string();
}

int32_t SystemAbilityManager::AddSystemCapability(const string& sysCap)
{
    int32_t ret = ERR_INVALID_VALUE;
    if (sysCap.empty() || sysCap.length() > MAX_SYSCAP_NAME_LEN) {
        HILOGE("SystemAbilityManager::AddSystemCapability syscap error!");
        return ret;
    }
    lock_guard<mutex> autoLock(sysCapMapLock_);
    auto iter = sysCapMap_.find(sysCap);
    if (iter != sysCapMap_.end()) {
        HILOGD("SystemAbilityManager::AddSystemCapability In %s!", sysCap.c_str());
        iter->second = true;
        ret = ERR_OK;
    }
    return ret;
}

bool SystemAbilityManager::HasSystemCapability(const string& sysCap)
{
    if (sysCap.empty() || sysCap.length() > MAX_SYSCAP_NAME_LEN) {
        HILOGE("SystemAbilityManager::HasSystemCapability syscap error!");
        return false;
    }
    lock_guard<mutex> autoLock(sysCapMapLock_);
    const auto iter = sysCapMap_.find(sysCap);
    if (iter != sysCapMap_.end()) {
        return iter->second;
    }
    return false;
}

vector<string> SystemAbilityManager::GetSystemAvailableCapabilities()
{
    vector<string> sysCaps;
    lock_guard<mutex> autoLock(sysCapMapLock_);
    for (const auto& [sysCap, isReg] : sysCapMap_) {
        if (!isReg) {
            continue;
        }
        sysCaps.emplace_back(sysCap);
    }
    return sysCaps;
}

void SystemAbilityManager::InitSysCapMap()
{
}

void SystemAbilityManager::AddDefaultCoreSa(set<int32_t>& coreSaIdSet) const
{
    coreSaIdSet.emplace(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN);
    coreSaIdSet.emplace(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    coreSaIdSet.emplace(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    coreSaIdSet.emplace(WORK_SCHEDULE_SERVICE_ID);
    coreSaIdSet.emplace(DISTRIBUTED_SCHED_SA_ID);
    coreSaIdSet.emplace(COMMON_EVENT_SERVICE_ABILITY_ID);
}
} // namespace OHOS
