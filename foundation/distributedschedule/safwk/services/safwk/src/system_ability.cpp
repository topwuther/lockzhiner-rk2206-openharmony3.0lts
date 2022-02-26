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

#include "system_ability.h"

#include "errors.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "local_ability_manager.h"
#include "safwk_log.h"
#include "string_ex.h"

namespace OHOS {
namespace {
const std::string TAG = "SystemAbility";
}

SystemAbility::SystemAbility(bool runOnCreate)
{
    isRunning_ = false;
    isRunOnCreate_ = runOnCreate;
    isDistributed_ = false;
    dumpLevel_ = 0;
    // timeout for waiting dependency ready, which configed in xml, with DEFAULT_DEPENDENCY_TIMEOUT(6s) by default
    dependTimeout_ = DEFAULT_DEPENDENCY_TIMEOUT;
    capability_ = u"";
}

SystemAbility::SystemAbility(int32_t systemAbilityId, bool runOnCreate) : SystemAbility(runOnCreate)
{
    saId_ = systemAbilityId;
}

SystemAbility::~SystemAbility()
{
    HILOGI(TAG, "SA:%{public}d destroyed", saId_);
}

bool SystemAbility::MakeAndRegisterAbility(SystemAbility* systemAbility)
{
    HILOGD(TAG, "registering system ability...");
    return LocalAbilityManager::GetInstance().AddAbility(systemAbility);
}

bool SystemAbility::AddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, listenerSaId);
    return LocalAbilityManager::GetInstance().AddSystemAbilityListener(systemAbilityId, listenerSaId);
}

bool SystemAbility::AddSystemAbilityListener(int32_t systemAbilityId)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, saId_);
    return LocalAbilityManager::GetInstance().AddSystemAbilityListener(systemAbilityId, saId_);
}

bool SystemAbility::InitAddSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, listenerSaId);
    return LocalAbilityManager::GetInstance().InitAddSystemAbilityListener(systemAbilityId,
        listenerSaId);
}

bool SystemAbility::RemoveSystemAbilityListener(int32_t systemAbilityId, int32_t listenerSaId)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, listenerSaId);
    return LocalAbilityManager::GetInstance().RemoveSystemAbilityListener(systemAbilityId, listenerSaId);
}

bool SystemAbility::RemoveSystemAbilityListener(int32_t systemAbilityId)
{
    HILOGD(TAG, "SA:%{public}d, listenerSA:%{public}d", systemAbilityId, saId_);
    return LocalAbilityManager::GetInstance().RemoveSystemAbilityListener(systemAbilityId, saId_);
}

bool SystemAbility::Publish(sptr<IRemoteObject> systemAbility)
{
    if (systemAbility == nullptr) {
        HILOGE(TAG, "systemAbility is nullptr");
        return false;
    }

    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return false;
    }

    publishObj_ = systemAbility;
    HILOGD(TAG, "SA:%{public}d", saId_);
    ISystemAbilityManager::SAExtraProp saExtra(GetDistributed(), GetDumpLevel(), capability_, permission_);
    int32_t result = samgrProxy->AddSystemAbility(saId_, publishObj_, saExtra);
    HILOGI(TAG, "AddSystemAbility result:%{public}d", result);
    return result == ERR_OK;
}

bool SystemAbility::RePublish()
{
    if (publishObj_ == nullptr) {
        return true;
    }
    return Publish(publishObj_);
}

void SystemAbility::StopAbility(int32_t systemAbilityId)
{
    HILOGD(TAG, "SA:%{public}d", systemAbilityId);

    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return;
    }

    int32_t ret = samgrProxy->RemoveSystemAbility(systemAbilityId);
    HILOGI(TAG, "%{public}s to remove ability", (ret == ERR_OK) ? "success" : "failed");
}

void SystemAbility::Start()
{
    HILOGD(TAG, "starting system ability...");
    if (isRunning_) {
        return;
    }

    OnStart();
    isRunning_ = true;
}

void SystemAbility::Stop()
{
    HILOGD(TAG, "stoping system ability...");

    if (!isRunning_) {
        return;
    }

    OnStop();
    isRunning_ = false;

    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return;
    }

    int32_t ret = samgrProxy->RemoveSystemAbility(saId_);
    HILOGI(TAG, "%{public}s to remove ability", (ret == ERR_OK) ? "success" : "failed");
}

void SystemAbility::SADump()
{
    OnDump();
}

void SystemAbility::Test()
{
    OnTest();
}

void SystemAbility::Debug()
{
    OnDebug();
}

int32_t SystemAbility::GetSystemAbilitId() const
{
    return saId_;
}

void SystemAbility::SetLibPath(const std::u16string& libPath)
{
    libPath_ = libPath;
}

const std::u16string& SystemAbility::GetLibPath() const
{
    return libPath_;
}

void SystemAbility::SetDependSa(const std::vector<std::u16string>& dependSa)
{
    dependSa_ = dependSa;
}

const std::vector<std::u16string>& SystemAbility::GetDependSa() const
{
    return dependSa_;
}

void SystemAbility::SetRunOnCreate(bool isRunOnCreate)
{
    isRunOnCreate_ = isRunOnCreate;
}

bool SystemAbility::IsRunOnCreate() const
{
    return isRunOnCreate_;
}

void SystemAbility::SetDistributed(bool isDistributed)
{
    isDistributed_ = isDistributed;
}

bool SystemAbility::GetDistributed() const
{
    return isDistributed_;
}

bool SystemAbility::GetRunningStatus() const
{
    return isRunning_;
}

void SystemAbility::SetDumpLevel(uint32_t dumpLevel)
{
    dumpLevel_ = dumpLevel;
}

uint32_t SystemAbility::GetDumpLevel() const
{
    return dumpLevel_;
}

void SystemAbility::SetDependTimeout(int32_t dependTimeout)
{
    if (dependTimeout >= MIN_DEPENDENCY_TIMEOUT && dependTimeout <= MAX_DEPENDENCY_TIMEOUT) {
        dependTimeout_ = dependTimeout;
    }
    HILOGD(TAG, "new dependTimeout:%{public}d, orignal dependTimeout:%{public}d", dependTimeout, dependTimeout_);
}

int32_t SystemAbility::GetDependTimeout() const
{
    return dependTimeout_;
}

// The details should be implemented by subclass
void SystemAbility::OnDump()
{
}

// The details should be implemented by subclass
void SystemAbility::OnDebug()
{
}

// The details should be implemented by subclass
void SystemAbility::OnTest()
{
}

// The details should be implemented by subclass
void SystemAbility::OnStart()
{
}

// The details should be implemented by subclass
void SystemAbility::OnStop()
{
}

// The details should be implemented by subclass
void SystemAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
    const sptr<IRemoteObject>& ability)
{
}

// The details should be implemented by subclass
void SystemAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
}

sptr<IRemoteObject> SystemAbility::GetSystemAbility(int32_t systemAbilityId)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE(TAG, "failed to get samgrProxy");
        return nullptr;
    }

    return samgrProxy->GetSystemAbility(systemAbilityId);
}

// Reserved for future usage
void SystemAbility::AddToLocal() const
{
}

// Reserved for future usage
void SystemAbility::DeleteFromLocal() const
{
}

void SystemAbility::SetCapability(const std::u16string& capability)
{
    capability_ = capability;
}

const std::u16string& SystemAbility::GetCapability() const
{
    return capability_;
}

void SystemAbility::SetPermission(const std::u16string& permission)
{
    permission_ = permission;
}
}
