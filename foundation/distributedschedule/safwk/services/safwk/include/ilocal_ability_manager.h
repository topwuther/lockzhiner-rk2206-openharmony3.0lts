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

#ifndef SAFWK_ILOCAL_ABILITY_MANAGER_H
#define SAFWK_ILOCAL_ABILITY_MANAGER_H

#include <string>

namespace OHOS {
class ILocalAbilityManagerKit {
public:
    /**
     * Abstract class, all functions implement in the derived class.
     *
     * @return Returns the only one derived class object.
     */
    static ILocalAbilityManagerKit& GetInstance();

    /**
     * Start System Ability.
     *
     * @param profilePath xml parh of the SAProcess.
     */
    virtual void DoStartSAProcess(const std::string& profilePath) = 0;
protected:
    ILocalAbilityManagerKit() = default;
    virtual ~ILocalAbilityManagerKit() = default;
};
} // namespace OHOS

#endif // SAFWK_ILOCAL_ABILITY_MANAGER_H
