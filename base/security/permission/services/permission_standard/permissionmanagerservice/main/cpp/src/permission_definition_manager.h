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

#ifndef PERMISSION_DEFINITION_MANAGER_H
#define PERMISSION_DEFINITION_MANAGER_H

#include <vector>

#include "permission_def_parcel.h"
#include "field_const.h"
#include "nocopyable.h"
#include "thread_pool.h"
#include "rwlock.h"

namespace OHOS {
namespace Security {
namespace Permission {
class PermissionDefinitionManager final {
public:
    static PermissionDefinitionManager& GetInstance();

    void Init();

    virtual ~PermissionDefinitionManager();

    void AddDefPermissions(const std::vector<PermissionDefParcel>& permList);

    void RemoveDefPermissions(const std::string& bundleName);

    int GetDefPermission(const std::string& permissionName, PermissionDefParcel& permissionDefParcel) const;

private:
    PermissionDefinitionManager();

    DISALLOW_COPY_AND_MOVE(PermissionDefinitionManager);

    bool IsGrantModeValid(const int grantMode) const;

    bool IsAvailableScopeValid(const int availableScope) const;

    bool IsPermissionDefValid(const PermissionDef& permissionDef) const;

    void RefreshPersistentDataIfNeeded();

    OHOS::ThreadPool permissionDefDataAccessWorker_;

    bool hasInited_;

    OHOS::Utils::RWLock rwLock_;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS

#endif // PERMISSION_DEFINITION_MANAGER_H
