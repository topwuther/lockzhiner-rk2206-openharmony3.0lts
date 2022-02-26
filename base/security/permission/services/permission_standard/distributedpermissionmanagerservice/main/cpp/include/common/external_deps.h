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
#ifndef EXTERNAL_DEPS_H
#define EXTERNAL_DEPS_H

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "bundle_mgr_interface.h"
#include "constant.h"
#include "i_permission_manager.h"
#include "ability_manager_interface.h"
#include "if_system_ability_manager.h"
#include "ans_manager_interface.h"

namespace OHOS {
namespace Security {
namespace Permission {
class ExternalDeps {
public:
    ExternalDeps() = default;
    virtual ~ExternalDeps() = default;
    sptr<AppExecFwk::IBundleMgr> GetBundleManager(sptr<AppExecFwk::IBundleMgr> iBundleManager_);
    sptr<Permission::IPermissionManager> GetPermissionManager(sptr<Permission::IPermissionManager> iPermissionManager_);
    sptr<AAFwk::IAbilityManager> GetAbilityManager(sptr<AAFwk::IAbilityManager> iAbilityManager_);

private:
    OHOS::sptr<ISystemAbilityManager> saMgr_;
    std::mutex saMutex_;
    sptr<IRemoteObject> GetSystemAbility(const Constant::ServiceId systemAbilityId);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // EXTERNAL_DEPS_H