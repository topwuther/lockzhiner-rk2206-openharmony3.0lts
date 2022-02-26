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

#ifndef SINGLE_VER_SCHEMA_DATABASE_UPGRADER_H
#define SINGLE_VER_SCHEMA_DATABASE_UPGRADER_H

#include "schema_object.h"
#include "single_ver_database_upgrader.h"

namespace DistributedDB {
class SingleVerSchemaDatabaseUpgrader : virtual public SingleVerDatabaseUpgrader {
public:
    // An invalid SchemaObject indicate no schema
    explicit SingleVerSchemaDatabaseUpgrader(const SchemaObject &newSchema);
    ~SingleVerSchemaDatabaseUpgrader() override {};
protected:
    int ExecuteUpgrade() override;
    // Database content related upgrade
    int ExecuteUpgradeSchema();

    // Get an empty string with return_code E_OK indicate no schema but everything normally
    virtual int GetDatabaseSchema(std::string &dbSchema) const = 0;
    // Set or update schema into database file
    virtual int SetDatabaseSchema(const std::string &dbSchema) = 0;

    virtual int UpgradeValues() = 0;
    virtual int UpgradeIndexes(const IndexDifference &indexDiffer) = 0;

    SchemaObject newSchema_;
private:
    int RestoreSchemaObjectFromDatabase(SchemaObject &outOriSchema) const;
};
} // namespace DistributedDB
#endif // SINGLE_VER_SCHEMA_DATABASE_UPGRADER_H
