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
#include "external_deps.h"
namespace OHOS {
namespace Security {
namespace Permission {
sptr<AppExecFwk::IBundleMgr> ExternalDeps::GetBundleManager(sptr<AppExecFwk::IBundleMgr> iBundleManager_)
{
    return nullptr;
}
sptr<Permission::IPermissionManager> ExternalDeps::GetPermissionManager(
    sptr<Permission::IPermissionManager> iPermissionManager_)
{
    return nullptr;
}
sptr<AAFwk::IAbilityManager> ExternalDeps::GetAbilityManager(sptr<AAFwk::IAbilityManager> iAbilityManager_)
{
    return nullptr;
}
sptr<IRemoteObject> ExternalDeps::GetSystemAbility(const Constant::ServiceId systemAbilityId)
{
    return nullptr;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS