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

#ifndef PERMISSION_STATE_MANAGER_H
#define PERMISSION_STATE_MANAGER_H

#include <vector>

#include "field_const.h"

#include "nocopyable.h"
#include "thread_pool.h"
#include "rwlock.h"

namespace OHOS {
namespace Security {
namespace Permission {
class PermissionStateManager final {
public:
    static PermissionStateManager& GetInstance();

    virtual ~PermissionStateManager();

    void Init();

    void AddUserGrantedReqPermissions(
        const std::string& bundleName, const std::vector<std::string>& permList, int userId);

    void RemoveUserGrantedReqPermissions(const std::string& bundleName, int userId);

    void GrantUserGrantedPermission(const std::string& bundleName, const std::string& permissionName, int userId);

    void RevokeUserGrantedPermission(const std::string& bundleName, const std::string& permissionName, int userId);

    void AddSystemGrantedReqPermissions(const std::string& bundleName, const std::vector<std::string>& permList);

    void RemoveSystemGrantedReqPermissions(const std::string& bundleName);

    void GrantSystemGrantedPermission(const std::string& bundleName, const std::string& permissionName);

    void RevokeSystemGrantedPermission(const std::string& bundleName, const std::string& permissionName);

    int VerifyPermission(const std::string& bundleName, const std::string& permissionName, int userId) const;

    bool CanRequestPermission(const std::string& bundleName, const std::string& permissionName, int userId) const;

private:
    PermissionStateManager();

    DISALLOW_COPY_AND_MOVE(PermissionStateManager);

    void RefreshSystemGrantedPersistentDataIfNeeded();

    void RefreshUserGrantedPersistentDataIfNeeded();

    OHOS::ThreadPool systemGrantedDataAccessWorker_;

    OHOS::ThreadPool userGrantedDataAccessWorker_;

    bool hasInited_;

    OHOS::Utils::RWLock rwLock_;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS

#endif // PERMISSION_STATE_MANAGER_H
