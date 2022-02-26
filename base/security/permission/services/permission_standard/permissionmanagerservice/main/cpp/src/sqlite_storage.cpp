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

#include "sqlite_storage.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SqliteStorage"};
}

SqliteStorage& SqliteStorage::GetInstance()
{
    static SqliteStorage instance;
    return instance;
}

SqliteStorage::~SqliteStorage()
{
    Close();
}

void SqliteStorage::OnCreate()
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called.", __func__);
    CreatePermissionDefinitionTable();
    CreateSystemGrantedPermissionStateTable();
    CreateUserGrantedPermissionStateTable();
}

void SqliteStorage::OnUpdate()
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called.", __func__);
}

SqliteStorage::SqliteStorage() : SqliteHelper(DATABASE_NAME, DATABASE_PATH, DATABASE_VERSION)
{
    SqliteTable permissionDefTable;
    permissionDefTable.tableName_ = PERMISSION_DEF_TABLE;
    permissionDefTable.tableColumnNames_ = {
        FIELD_PERMISSION_NAME,
        FIELD_BUNDLE_NAME,
        FIELD_GRANT_MODE,
        FIELD_AVAILABLE_SCOPE,
        FIELD_LABEL,
        FIELD_LABEL_ID,
        FIELD_DESCRIPTION,
        FIELD_DESCRIPTION_ID
    };

    SqliteTable sysGrantPermStatTable;
    sysGrantPermStatTable.tableName_ = SYS_GRANTED_PERMISSION_STATE_TABLE;
    sysGrantPermStatTable.tableColumnNames_ = {
        FIELD_BUNDLE_NAME,
        FIELD_PERMISSION_NAME,
        FIELD_GRANTED,
        FIELD_FLAGS
    };

    SqliteTable userGrantPermStatTable;
    userGrantPermStatTable.tableName_ = USER_GRANTED_PERMISSION_STATE_TABLE;
    userGrantPermStatTable.tableColumnNames_ = {
        FIELD_BUNDLE_NAME,
        FIELD_PERMISSION_NAME,
        FIELD_USER_ID,
        FIELD_GRANTED,
        FIELD_FLAGS
    };

    dataTypeToSqlTable_ = {
        {PERMISSION_DEF, permissionDefTable},
        {PERMISSIONS_STAT_SYSTEM_GRANTED, sysGrantPermStatTable},
        {PERMISSIONS_STAT_USER_GRANTED, userGrantPermStatTable}
    };

    Open();
}

int SqliteStorage::Add(const DataType type, const std::vector<GenericValues>& values)
{
    OHOS::Utils::UniqueWriteGuard<OHOS::Utils::RWLock> lock(this->rwLock_);
    std::string prepareSql = CreateInsertPrepareSqlCmd(type);
    auto statement = Prepare(prepareSql);
    BeginTransaction();
    bool isExecuteSuccessfully = true;
    for (auto value : values) {
        std::vector<std::string> columnNames = value.GetAllKeys();
        for (auto columnName : columnNames) {
            statement.Bind(columnName, value.Get(columnName));
        }
        int ret = statement.Step();
        if (ret != Statement::State::DONE) {
            PERMISSION_LOG_ERROR(LABEL, "%{public}s: failed, errorMsg: %{public}s", __func__, SpitError().c_str());
            isExecuteSuccessfully = false;
        }
        statement.Reset();
    }
    if (!isExecuteSuccessfully) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: rollback transaction.", __func__);
        RollbackTransaction();
        return FAILURE;
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s: commit transaction.", __func__);
    CommitTransaction();
    return SUCCESS;
}

int SqliteStorage::Remove(const DataType type, const GenericValues& conditions)
{
    OHOS::Utils::UniqueWriteGuard<OHOS::Utils::RWLock> lock(this->rwLock_);
    std::vector<std::string> columnNames = conditions.GetAllKeys();
    std::string prepareSql = CreateDeletePrepareSqlCmd(type, columnNames);
    auto statement = Prepare(prepareSql);
    for (auto columnName : columnNames) {
        statement.Bind(columnName, conditions.Get(columnName));
    }
    int ret = statement.Step();
    return (ret == Statement::State::DONE) ? SUCCESS : FAILURE;
}

int SqliteStorage::Modify(const DataType type, const GenericValues& modifyValues, const GenericValues& conditions)
{
    OHOS::Utils::UniqueWriteGuard<OHOS::Utils::RWLock> lock(this->rwLock_);
    std::vector<std::string> modifyColumns = modifyValues.GetAllKeys();
    std::vector<std::string> conditionColumns = conditions.GetAllKeys();
    std::string prepareSql = CreateUpdatePrepareSqlCmd(type, modifyColumns, conditionColumns);
    auto statement = Prepare(prepareSql);
    for (auto columnName : modifyColumns) {
        statement.Bind(columnName, modifyValues.Get(columnName));
    }
    for (auto columnName : conditionColumns) {
        statement.Bind(columnName, conditions.Get(columnName));
    }
    int ret = statement.Step();
    return (ret == Statement::State::DONE) ? SUCCESS : FAILURE;
}

int SqliteStorage::Find(const DataType type, std::vector<GenericValues>& results)
{
    OHOS::Utils::UniqueWriteGuard<OHOS::Utils::RWLock> lock(this->rwLock_);
    std::string prepareSql = CreateSelectPrepareSqlCmd(type);
    auto statement = Prepare(prepareSql);
    while (statement.Step() == Statement::State::ROW) {
        int columnCount = statement.GetColumnCount();
        GenericValues value;
        for (int i = 0; i < columnCount; i++) {
            value.Put(statement.GetColumnName(i), statement.GetValue(i));
        }
        results.emplace_back(value);
    }
    return SUCCESS;
}

int SqliteStorage::RefreshAll(const DataType type, const std::vector<GenericValues>& values)
{
    OHOS::Utils::UniqueWriteGuard<OHOS::Utils::RWLock> lock(this->rwLock_);
    std::string deleteSql = CreateDeletePrepareSqlCmd(type);
    std::string insertSql = CreateInsertPrepareSqlCmd(type);
    auto deleteStatement = Prepare(deleteSql);
    auto insertStatement = Prepare(insertSql);
    BeginTransaction();
    bool canCommit = deleteStatement.Step() == Statement::State::DONE;
    for (auto value : values) {
        std::vector<std::string> columnNames = value.GetAllKeys();
        for (auto columnName : columnNames) {
            insertStatement.Bind(columnName, value.Get(columnName));
        }
        int ret = insertStatement.Step();
        if (ret != Statement::State::DONE) {
            PERMISSION_LOG_ERROR(
                LABEL, "%{public}s: insert failed, errorMsg: %{public}s", __func__, SpitError().c_str());
            canCommit = false;
        }
        insertStatement.Reset();
    }
    if (!canCommit) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: rollback transaction.", __func__);
        RollbackTransaction();
        return FAILURE;
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s: commit transaction.", __func__);
    CommitTransaction();
    return SUCCESS;
}

std::string SqliteStorage::CreateInsertPrepareSqlCmd(const DataType type) const
{
    auto it = dataTypeToSqlTable_.find(type);
    if (it == dataTypeToSqlTable_.end()) {
        return std::string();
    }
    std::string sql = "insert into " + it->second.tableName_ + " values(";
    int i = 1;
    for (const auto& columnName : it->second.tableColumnNames_) {
        sql.append(":" + columnName);
        if (i < (int) it->second.tableColumnNames_.size()) {
            sql.append(",");
        }
        i += 1;
    }
    sql.append(")");
    return sql;
}

std::string SqliteStorage::CreateDeletePrepareSqlCmd(
    const DataType type, const std::vector<std::string>& columnNames) const
{
    auto it = dataTypeToSqlTable_.find(type);
    if (it == dataTypeToSqlTable_.end()) {
        return std::string();
    }
    std::string sql = "delete from " + it->second.tableName_ + " where 1 = 1";
    for (auto columnName : columnNames) {
        sql.append(" and ");
        sql.append(columnName + "=:" + columnName);
    }
    return sql;
}

std::string SqliteStorage::CreateUpdatePrepareSqlCmd(const DataType type, const std::vector<std::string>& modifyColumns,
    const std::vector<std::string>& conditionColumns) const
{
    if (modifyColumns.empty()) {
        return std::string();
    }

    auto it = dataTypeToSqlTable_.find(type);
    if (it == dataTypeToSqlTable_.end()) {
        return std::string();
    }

    std::string sql = "update " + it->second.tableName_ + " set ";
    int i = 1;
    for (const auto& columnName : modifyColumns) {
        sql.append(columnName + "=:" + columnName);
        if (i < (int) modifyColumns.size()) {
            sql.append(",");
        }
        i += 1;
    }

    if (!conditionColumns.empty()) {
        sql.append(" where 1 = 1");
        for (const auto& columnName : conditionColumns) {
            sql.append(" and ");
            sql.append(columnName + "=:" + columnName);
        }
    }
    return sql;
}

std::string SqliteStorage::CreateSelectPrepareSqlCmd(const DataType type) const
{
    auto it = dataTypeToSqlTable_.find(type);
    if (it == dataTypeToSqlTable_.end()) {
        return std::string();
    }
    std::string sql = "select * from " + it->second.tableName_;
    return sql;
}

int SqliteStorage::CreatePermissionDefinitionTable() const
{
    auto it = dataTypeToSqlTable_.find(DataType::PERMISSION_DEF);
    if (it == dataTypeToSqlTable_.end()) {
        return FAILURE;
    }
    std::string sql = "create table if not exists ";
    sql.append(it->second.tableName_ + " (")
        .append(FIELD_PERMISSION_NAME + " text not null,")
        .append(FIELD_BUNDLE_NAME + " text not null,")
        .append(FIELD_GRANT_MODE + " integer not null,")
        .append(FIELD_AVAILABLE_SCOPE + " integer not null,")
        .append(FIELD_LABEL + " text not null,")
        .append(FIELD_LABEL_ID + " integer not null,")
        .append(FIELD_DESCRIPTION + " text not null,")
        .append(FIELD_DESCRIPTION_ID + " integer not null,")
        .append("primary key(" + FIELD_PERMISSION_NAME)
        .append("))");
    return ExecuteSql(sql);
}

int SqliteStorage::CreateSystemGrantedPermissionStateTable() const
{
    auto it = dataTypeToSqlTable_.find(DataType::PERMISSIONS_STAT_SYSTEM_GRANTED);
    if (it == dataTypeToSqlTable_.end()) {
        return FAILURE;
    }
    std::string sql = "create table if not exists ";
    sql.append(it->second.tableName_ + " (")
        .append(FIELD_BUNDLE_NAME + " text not null,")
        .append(FIELD_PERMISSION_NAME + " text not null,")
        .append(FIELD_GRANTED + " integer not null,")
        .append(FIELD_FLAGS + " integer not null,")
        .append("primary key(" + FIELD_BUNDLE_NAME)
        .append("," + FIELD_PERMISSION_NAME)
        .append("))");
    return ExecuteSql(sql);
}

int SqliteStorage::CreateUserGrantedPermissionStateTable() const
{
    auto it = dataTypeToSqlTable_.find(DataType::PERMISSIONS_STAT_USER_GRANTED);
    if (it == dataTypeToSqlTable_.end()) {
        return FAILURE;
    }
    std::string sql = "create table if not exists ";
    sql.append(it->second.tableName_ + " (")
        .append(FIELD_BUNDLE_NAME + " text not null,")
        .append(FIELD_PERMISSION_NAME + " text not null,")
        .append(FIELD_USER_ID + " integer not null,")
        .append(FIELD_GRANTED + " integer not null,")
        .append(FIELD_FLAGS + " integer not null,")
        .append("primary key(" + FIELD_BUNDLE_NAME)
        .append("," + FIELD_PERMISSION_NAME)
        .append("," + FIELD_USER_ID)
        .append("))");
    return ExecuteSql(sql);
}
} // namespace Permission
} // namespace Security
} // namespace OHOS