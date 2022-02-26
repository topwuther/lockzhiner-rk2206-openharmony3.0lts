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

#include "single_ver_database_upgrader.h"
#include "db_errno.h"
#include "log_print.h"
#include "version.h"

namespace DistributedDB {
int SingleVerDatabaseUpgrader::Upgrade()
{
    LOGI("[SingleUp][Upgrade] Enter, CurVersion=%d.", SINGLE_VER_STORE_VERSION_CURRENT);
    int errCode = GetDatabaseVersion(dbVersion_);
    if (errCode != E_OK) {
        LOGE("[SingleUp][Upgrade] GetVersion fail, errCode=%d.", errCode);
        return errCode;
    }
    if (dbVersion_ > SINGLE_VER_STORE_VERSION_CURRENT) {
        LOGE("[SingleUp][Upgrade] DbVersion=%d is newer.", dbVersion_);
        return -E_VERSION_NOT_SUPPORT;
    }
    LOGI("[SingleUp][Upgrade] DbVersion=%d.", dbVersion_);

    errCode = BeginUpgrade();
    if (errCode != E_OK) {
        LOGE("[SingleUp][Upgrade] Begin fail, errCode=%d.", errCode);
        return errCode;
    }
    errCode = ExecuteUpgrade();
    if (errCode != E_OK) {
        LOGE("[SingleUp][Upgrade] Execute fail, errCode=%d.", errCode);
        EndUpgrade(false);
        return errCode;
    }

    if (dbVersion_ < SINGLE_VER_STORE_VERSION_CURRENT) {
        errCode = SetDatabaseVersion(SINGLE_VER_STORE_VERSION_CURRENT);
        if (errCode != E_OK) {
            LOGE("[SingleUp][Upgrade] SetVersion fail, errCode=%d.", errCode);
            EndUpgrade(false);
            return errCode;
        }
    }

    errCode = EndUpgrade(true);
    if (errCode != E_OK) {
        LOGE("[SingleUp][Upgrade] End fail, errCode=%d.", errCode);
        return errCode;
    }

    LOGI("[SingleUp][Upgrade] Exit Successfully.");
    return E_OK;
}

int SingleVerDatabaseUpgrader::ExecuteUpgrade()
{
    if (dbVersion_ <= SINGLE_VER_STORE_VERSION_CURRENT) {
        return UpgradeFromDatabaseVersion(dbVersion_);
    }
    return E_OK;
}
} // namespace DistributedDB
