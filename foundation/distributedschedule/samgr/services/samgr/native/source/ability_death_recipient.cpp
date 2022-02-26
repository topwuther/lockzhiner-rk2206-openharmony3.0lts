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

#include "ability_death_recipient.h"

#include "iremote_proxy.h"
#include "sam_log.h"
#include "system_ability_manager.h"

namespace OHOS {
void AbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("%s called", __func__);
    SystemAbilityManager::GetInstance()->RemoveSystemAbility(remote.promote());
    HILOGI("recv death notice success");
}

void LocalAbilityManagerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("LocalAbilityManagerDeathRecipient::OnRemoteDied called!");
    SystemAbilityManager::GetInstance()->RemoveLocalAbilityManager(remote.promote());
    HILOGI("LocalAbilityManagerDeathRecipient::OnRemoteDied death notice success");
}
void SystemReadyCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("SystemReadyCallbackDeathRecipient::coreSa::OnRemoteDied called!");
    SystemAbilityManager::GetInstance()->RemoveSystemReadyCallback(remote.promote());
    HILOGI("SystemReadyCallbackDeathRecipient::coreSa::OnRemoteDied death notice success");
}
} // namespace OHOS
