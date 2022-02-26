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
#ifndef PERMISSION_FETCHER_H
#define PERMISSION_FETCHER_H

#include "base_remote_command.h"
#include "bundle_info.h"

namespace OHOS {
namespace Security {
namespace Permission {

/**
 * permission fetcher class.
 *
 */
class PermissionFetcher {
public:
    PermissionFetcher(
        sptr<AppExecFwk::IBundleMgr> iBundleManager, sptr<Permission::IPermissionManager> iPermissionManager);
    virtual ~PermissionFetcher() = default;
    int32_t GetPermissions(int32_t uid, UidBundleBo &info);
    int32_t GetRegrantedPermissions(UidBundleBo &pInfo, UidBundleBo &rInfo);
    BundlePermissionsDto CreateBundlePermissionsInfo(
        int32_t uid, AppExecFwk::BundleInfo &bundleInfo, std::string &bundleName);
    BundlePermissionsDto MergeBundlePermissionsInfo(BundlePermissionsDto &pInfo);

private:
    int32_t appAttribute_ = -1;
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
    sptr<Permission::IPermissionManager> iPermissionManager_;
    const std::string APP_TYPE_OTHER = "third-party";
    const std::string APP_TYPE_SYSTEM = "system";
};

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // PERMISSION_FETCHER_H