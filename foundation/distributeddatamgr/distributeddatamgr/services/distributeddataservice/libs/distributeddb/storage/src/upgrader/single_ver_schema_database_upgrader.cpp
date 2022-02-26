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

#include "single_ver_schema_database_upgrader.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
SingleVerSchemaDatabaseUpgrader::SingleVerSchemaDatabaseUpgrader(const SchemaObject &newSchema)
    : newSchema_(newSchema)
{
}

int SingleVerSchemaDatabaseUpgrader::ExecuteUpgrade()
{
    // Calling the base class to upgrade the database structure first
    int errCode = SingleVerDatabaseUpgrader::ExecuteUpgrade();
    if (errCode != E_OK) {
        LOGE("[SingleSchemaUp][ExecUp] Upgrade database structure fail, errCode=%d.", errCode);
        return errCode;
    }
    return ExecuteUpgradeSchema();
}

int SingleVerSchemaDatabaseUpgrader::ExecuteUpgradeSchema()
{
    // Upgrade value or index according to newSchema and oriDbSchema
    LOGD("[SingleSchemaUp][ExecUp] Enter.");
    if (!newSchema_.IsSchemaValid()) {
        LOGI("[SingleSchemaUp][ExecUp] No schema newly designated.");
        return E_OK;
    }
    SchemaObject oriSchemaObject;
    int errCode = RestoreSchemaObjectFromDatabase(oriSchemaObject);
    if (errCode != E_OK) {
        return errCode;
    }
    // Judge and gather upgrade information
    bool valueNeedUpgrade = false;
    IndexDifference indexDiffer;
    if (oriSchemaObject.IsSchemaValid()) {
        errCode = oriSchemaObject.CompareAgainstSchemaObject(newSchema_, indexDiffer);
        if (errCode == -E_SCHEMA_EQUAL_EXACTLY) {
            LOGI("[SingleSchemaUp][ExecUp] NewSchema equal exactly with oriDbSchema.");
            return E_OK;
        } else if (errCode == -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
            return -E_SCHEMA_MISMATCH;
        } else if (errCode == -E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE) {
            valueNeedUpgrade = true;
        }
        // ValueNeedUpgrade false for case E_SCHEMA_UNEQUAL_COMPATIBLE
    } else {
        // Upgrade normalDb to schemaDb
        valueNeedUpgrade = true;
        indexDiffer.increase = newSchema_.GetIndexInfo();
    }
    // Remember to upgrade value first, upgrade index later, for both Json-Schema and FlatBuffer-Schema
    if (valueNeedUpgrade) {
        errCode = UpgradeValues();
        if (errCode != E_OK) {
            return errCode;
        }
    }
    errCode = UpgradeIndexes(indexDiffer);
    if (errCode != E_OK) {
        return errCode;
    }
    // Update schema into database file
    errCode = SetDatabaseSchema(newSchema_.ToSchemaString());
    if (errCode != E_OK) {
        return errCode;
    }
    return E_OK;
}

int SingleVerSchemaDatabaseUpgrader::RestoreSchemaObjectFromDatabase(SchemaObject &outOriSchema) const
{
    std::string oriDbSchema;
    int errCode = GetDatabaseSchema(oriDbSchema); // If no schema in db should return E_OK with an empty string
    if (errCode != E_OK) {
        return errCode;
    }
    if (!oriDbSchema.empty()) {
        errCode = outOriSchema.ParseFromSchemaString(oriDbSchema);
        if (errCode != E_OK) {
            LOGW("[SingleSchemaUp][ExecUp] Schema in dbFile parse fail, regard as no schema.");
        }
    }
    return E_OK;
}
} // namespace DistributedDB
