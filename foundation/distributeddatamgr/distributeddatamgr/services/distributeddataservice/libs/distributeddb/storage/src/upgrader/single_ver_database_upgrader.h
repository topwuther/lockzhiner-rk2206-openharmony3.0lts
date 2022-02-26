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

#ifndef SINGLE_VER_DATABASE_UPGRADER_H
#define SINGLE_VER_DATABASE_UPGRADER_H

#include "database_upgrader.h"

namespace DistributedDB {
class SingleVerDatabaseUpgrader : virtual public DatabaseUpgrader {
public:
    ~SingleVerDatabaseUpgrader() override {};
    int Upgrade() override;
protected:
    virtual int BeginUpgrade() = 0;
    virtual int ExecuteUpgrade();
    virtual int EndUpgrade(bool isSuccess) = 0;

    // Database structure related upgrade
    virtual int GetDatabaseVersion(int &version) const = 0;
    virtual int SetDatabaseVersion(int version) = 0;
    virtual int UpgradeFromDatabaseVersion(int version) = 0;

    // Database version only increased when the structure of database changed
    int dbVersion_ = 0;
};
} // namespace DistributedDB
#endif // SINGLE_VER_DATABASE_UPGRADER_H