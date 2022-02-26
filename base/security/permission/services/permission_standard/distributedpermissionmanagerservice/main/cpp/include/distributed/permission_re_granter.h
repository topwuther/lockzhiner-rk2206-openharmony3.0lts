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
#ifndef PERMISSION_RE_GRANTER_H
#define PERMISSION_RE_GRANTER_H

#include "base_remote_command.h"
#include "permission_log.h"
#include "external_deps.h"
#include "permission_fetcher.h"
#include "base_remote_command.h"

namespace OHOS {
namespace Security {
namespace Permission {
class PermissionReGranter final {
public:
    static void ReGrantDuidPermissions(UidBundleBo& uidBundlePermInfo);
    static bool IsGranted(const PermissionDto& permission);
    static bool IsRemoteGranted(const PermissionDto& permission);
    static void SetStatusGranted(const bool isGranted, PermissionDto& permission);
    static void SetFlagRemoteGranted(const bool isRemoteGranted, PermissionDto& permission);

private:
    static void GetPermissionInfoNoThrow(const std::string& permission, PermissionDefParcel& permInfo);
    static bool VerifySignatruePermission(const PermissionDefParcel& permInfo, const UidBundleBo& uidBundlePermInfo);

private:
    static const int FLAG_PERMISSION_DEFINED_BY_SYSTEM = 1 << 0;
    static const int FLAG_PERMISSION_STATUS_DISTRIBUTED_GRANTED = 1 << 0;
    static const int FLAG_PERMISSION_STATUS_DISTRIBUTED_DENIED = 1 << 1;
    static const int FLAG_PERMISSION_STATUS_REMOTE_GRANTED = 1 << 3;
    enum TypeOfGrantMode {
        USER_GRANT = 0,
        SYSTEM_GRANT = 1,
    };

    enum TypeOfAvailableScope {
        AVAILABLE_SCOPE_ALL = 1 << 0,
        AVAILABLE_SCOPE_SIGNATURE = 1 << 1,
        AVAILABLE_SCOPE_RESTRICTED = 1 << 2
    };
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // PERMISSION_RE_GRANTER_H