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

#ifndef PERMISSION_BMS_MANAGER_H
#define PERMISSION_BMS_MANAGER_H

#include <string>
#include <memory>
#include <functional>
#include "refbase.h"
#include "external_deps.h"

#include "app_info_fetcher.h"
#include "permission_fetcher.h"
#include "permission_listener.h"
#include "base_remote_command.h"
#include "permission_re_granter.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
class PermissionBmsManager final {
public:
    ~PermissionBmsManager();

    /**
     * @brief Get single instance of PmsAdapter.
     *
     * @return instance.
     * @since 1.0
     * @version 1.0
     */
    static PermissionBmsManager &GetInstance();

    /**
     * @brief Init, register permission changed listener.
     * @return 0 for success, -1 for failure
     * @since 1.0
     * @version 1.0
     */
    int Init();

    /**
     * @brief Call at subject side to get granted system permissions and all app permissions.
     *
     * @param uid The uid of application.
     * @param info output permissions, Granted system permissions and all app permissions.
     * @return 0 for success, -1 for failure
     * @since 1.0
     * @version 1.0
     */
    int32_t GetPermissions(int32_t uid, UidBundleBo &info);

    /**
     * @brief Call at object side to regrant app permissions.
     *
     * @param pInfo permissions passed by subject side.
     * @param rInfo output permissions regranted to object side. Regranted permission include system permissions and app
     * permissions.
     * @return 0 for success, -1 for failure
     * @since 1.0
     * @version 1.0
     */
    int32_t GetRegrantedPermissions(UidBundleBo &pInfo, UidBundleBo &rInfo);

    /**
     * @brief Check if a uid is <10000 or a platform signature uid
     *
     * @param uid a uid to check
     * @return true: Uid is <10000, or a platform signature uid, otherwise false
     * @since 1.0
     * @version 1.0
     */
    bool IsSystemSignatureUid(const int32_t &uid);

    /**
     * @brief Initialize system defined permissions set at the right time.
     *
     * @return 0 for success, -1 for failure
     * @since 1.0
     * @version 1.0
     */
    int InitSystemDefinedPermissions();

    /**
     * @brief Regrant cached permissions for subject device when the permission definer may change.
     *
     * @param uidBundlePermInfo a object contains bundleName and all related permissions.
     * @since 1.0
     * @version 1.0
     */
    void ReGrantDuidPermissions(UidBundleBo &uidBundlePermInfo);

public:
    // Range of uids allocated for a user ,normally 100000.
    // redirected to Constant class.
    static constexpr int32_t PER_USER_RANGE = Constant::PER_USER_RANGE;

private:
    PermissionBmsManager();

private:
    // temp object for permissionFetcher
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
    sptr<Permission::IPermissionManager> iPermissionManager_;

private:
    // permission fetch proxy
    std::shared_ptr<PermissionFetcher> permissionFetcher_;

    // app info fetch proxy
    std::shared_ptr<AppInfoFetcher> appInfoFetcher_;
};

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif
