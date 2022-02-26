/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ability_helper.h"

#include "log.h"
#include "napi/n_func_arg.h"
#include "napi/uni_header.h"

namespace OHOS {
namespace DistributedFS {
using namespace std;
using OHOS::AppExecFwk::Ability;
using OHOS::AppExecFwk::AbilityContext;

Ability* AbilityHelper::GetJsAbility(napi_env env)
{
    napi_value global = nullptr;
    napi_value abilityContext = nullptr;
    
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        HILOGE("Cannot get global instance for %{public}d", status);
        return nullptr;
    }

    status = napi_get_named_property(env, global, "ability", &abilityContext);
    if (status != napi_ok || abilityContext == nullptr) {
        HILOGE("Cannot get ability context for %{public}d", status);
        return nullptr;
    }

    Ability *ability = nullptr;
    status = napi_get_value_external(env, abilityContext, (void **)&ability);
    if (status != napi_ok || ability == nullptr) {
        HILOGE("Get ability form property failed for %{public}d", status);
    }

    return ability;
}
} // namespace DistributedFS
} // namespace OHOS