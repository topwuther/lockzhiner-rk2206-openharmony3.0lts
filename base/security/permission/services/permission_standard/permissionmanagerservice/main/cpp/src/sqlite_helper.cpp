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

#include "sqlite_helper.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SqliteHelper"};
}

SqliteHelper::SqliteHelper(const std::string& dbName, const std::string& dbPath, int version)
    : dbName_(dbName), dbPath_(dbPath), currentVersion_(version), db_(nullptr)
{}

SqliteHelper::~SqliteHelper()
{}

void SqliteHelper::Open()
{
    if (db_ != nullptr) {
        return;
    }
    if (dbName_.empty() || dbPath_.empty() || currentVersion_ < 0) {
        return;
    }
    std::string fileName = dbPath_ + dbName_;
    int res = sqlite3_open(fileName.c_str(), &db_);
    if (res != SQLITE_OK) {
        PERMISSION_LOG_ERROR(LABEL, "Failed to open db: %{public}s", sqlite3_errmsg(db_));
        return;
    }

    int version = GetVersion();
    if (version == currentVersion_) {
        return;
    }

    BeginTransaction();
    if (version == 0) {
        OnCreate();
    } else {
        if (version < currentVersion_) {
            OnUpdate();
        }
    }
    SetVersion();
    CommitTransaction();
}

void SqliteHelper::Close()
{
    if (db_ == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: do open data base first!", __func__);
        return;
    }
    int ret = sqlite3_close(db_);
    if (ret != SQLITE_OK) {
        PERMISSION_LOG_WARN(LABEL, "sqlite3_close error, ret=%{public}d", ret);
        return;
    }
    db_ = nullptr;
}

int SqliteHelper::BeginTransaction() const
{
    if (db_ == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: do open data base first!", __func__);
        return GENERAL_ERROR;
    }
    char* errorMessage = nullptr;
    int result = 0;
    int ret = sqlite3_exec(db_, "BEGIN;", nullptr, nullptr, &errorMessage);
    if (ret != SQLITE_OK) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: failed, errorMsg: %{public}s", __func__, errorMessage);
        result = GENERAL_ERROR;
    }
    sqlite3_free(errorMessage);
    return result;
}

int SqliteHelper::CommitTransaction() const
{
    if (db_ == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: do open data base first!", __func__);
        return GENERAL_ERROR;
    }
    char* errorMessage = nullptr;
    int result = 0;
    int ret = sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &errorMessage);
    if (ret != SQLITE_OK) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: failed, errorMsg: %{public}s", __func__, errorMessage);
        result = GENERAL_ERROR;
    }
    sqlite3_free(errorMessage);
    return result;
}

int SqliteHelper::RollbackTransaction() const
{
    if (db_ == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: do open data base first!", __func__);
        return GENERAL_ERROR;
    }
    int result = 0;
    char* errorMessage = nullptr;
    int ret = sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, &errorMessage);
    if (ret != SQLITE_OK) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: failed, errorMsg: %{public}s", __func__, errorMessage);
        result = GENERAL_ERROR;
    }
    sqlite3_free(errorMessage);
    return result;
}

Statement SqliteHelper::Prepare(const std::string& sql) const
{
    return Statement(db_, sql);
}

int SqliteHelper::ExecuteSql(const std::string& sql) const
{
    if (db_ == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: do open data base first!", __func__);
        return GENERAL_ERROR;
    }
    char* errorMessage = nullptr;
    int result = 0;
    int res = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMessage);
    if (res != SQLITE_OK) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: failed, errorMsg: %{public}s", __func__, errorMessage);
        result = GENERAL_ERROR;
    }
    sqlite3_free(errorMessage);
    return result;
}

int SqliteHelper::GetVersion() const
{
    if (db_ == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: do open data base first!", __func__);
        return GENERAL_ERROR;
    }
    auto statement = Prepare(PRAGMA_VERSION_COMMAND);
    int version = 0;
    while (statement.Step() == Statement::State::ROW) {
        version = statement.GetColumnInt(0);
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s: version: %{public}d", __func__, version);
    return version;
}

void SqliteHelper::SetVersion() const
{
    if (db_ == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: do open data base first!", __func__);
        return;
    }
    auto statement = Prepare(PRAGMA_VERSION_COMMAND + " = " + std::to_string(currentVersion_));
    statement.Step();
}

std::string SqliteHelper::SpitError() const
{
    if (db_ == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: do open data base first!", __func__);
        return "";
    }
    return sqlite3_errmsg(db_);
}
} // namespace Permission
} // namespace Security
} // namespace OHOS