/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "setting_main_ability.h"

namespace OHOS {
REGISTER_AA(SettingMainAbility)

void SettingMainAbility::OnStart(const Want& want)
{
    SetMainRoute("MainAbilitySlice");
    Ability::OnStart(want);
}

void SettingMainAbility::OnInactive()
{
    Ability::OnInactive();
}

void SettingMainAbility::OnActive(const Want& want)
{
    Ability::OnActive(want);
}

void SettingMainAbility::OnBackground()
{
    Ability::OnBackground();
}

void SettingMainAbility::OnStop()
{

    Ability::OnStop();
}
} // namespace OHOS