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

#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_manager.h"

namespace OHOS {
SystemAbilityManagerClient &SystemAbilityManagerClient::GetInstance()
{
    HILOG_ERROR(" SystemAbilityManagerClient::GetInstance()");
    static auto instance = new SystemAbilityManagerClient();
    return *instance;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    HILOG_ERROR(" SystemAbilityManagerClient::GetSystemAbilityManager()");
    return SystemAbilityManager::GetInstance();
}

sptr<IRemoteObject> SystemAbilityManagerClient::GetRegistryRemoteObject()
{
    HILOG_ERROR("SystemAbilityManagerClient::GetRegistryRemoteObject()");
    return nullptr;
}

void SystemAbilityManagerClient::DestroySystemAbilityManagerObject()
{
    HILOG_ERROR("SystemAbilityManagerClient::DestroySystemAbilityManagerObject()");
}
}  // namespace OHOS
