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

#include "sqlite_local_storage_executor.h"

#include "log_print.h"
#include "db_errno.h"
#include "sqlite_utils.h"

namespace DistributedDB {
namespace {
    const std::string CLEAR_SQL = "DELETE FROM data;";
    const std::string SELECT_SQL = "SELECT value FROM data WHERE key=?;";
    const std::string SELECT_BATCH_SQL =
        "SELECT key, value FROM data WHERE key>=? AND key<=? ORDER BY key ASC;";
    const std::string INSERT_SQL = "INSERT OR REPLACE INTO data VALUES(?,?);";
    const std::string DELETE_SQL = "DELETE FROM data WHERE key=?;";

    const int BIND_KEY_INDEX = 1;
    const int BIND_VAL_INDEX = 2;

    const int SELECT_BIND_KEY_INDEX = 1; // index of the binding key index for select one entry.

    const int SELECT_RESULT_KEY_INDEX = 0;
    const int SELECT_RESULT_VAL_INDEX = 1;
}

SQLiteLocalStorageExecutor::SQLiteLocalStorageExecutor(sqlite3 *dbHandle, bool writable, bool isMemDb)
    : SQLiteStorageExecutor(dbHandle, writable, isMemDb)
{}

SQLiteLocalStorageExecutor::~SQLiteLocalStorageExecutor()
{}

int SQLiteLocalStorageExecutor::Get(const Key &key, Value &value) const
{
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_SQL, statement);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, SELECT_BIND_KEY_INDEX, key, false);
    if (errCode != E_OK) {
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = -E_NOT_FOUND;
        goto END;
    } else if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        goto END;
    }

    errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, value);

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteLocalStorageExecutor::Clear()
{
    int errCode = SQLiteUtils::ExecuteRawSQL(dbHandle_, CLEAR_SQL);
    return CheckCorruptedStatus(errCode);
}

int SQLiteLocalStorageExecutor::GetEntries(const Key &keyPrefix,
    std::vector<Entry> &entries) const
{
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_BATCH_SQL, statement);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    Entry entry;
    errCode = SQLiteUtils::BindPrefixKey(statement, SELECT_BIND_KEY_INDEX, keyPrefix); // first arg is prefix key
    if (errCode != E_OK) {
        goto END;
    }

    do {
        errCode = SQLiteUtils::StepWithRetry(statement);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            errCode = SQLiteUtils::GetColumnBlobValue(statement, SELECT_RESULT_KEY_INDEX, entry.key);
            if (errCode != E_OK) {
                goto END;
            }

            errCode = SQLiteUtils::GetColumnBlobValue(statement, SELECT_RESULT_VAL_INDEX, entry.value);
            if (errCode != E_OK) {
                goto END;
            }

            entries.push_back(std::move(entry));
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            break;
        } else {
            LOGE("SQLite step failed:%d", errCode);
            goto END;
        }
    } while (true);

    // if select no result, return the -E_NOT_FOUND.
    if (entries.empty()) {
        errCode = -E_NOT_FOUND;
    } else {
        errCode = E_OK;
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteLocalStorageExecutor::PutBatch(const std::vector<Entry> &entries)
{
    if (entries.empty()) {
        return -E_INVALID_ARGS;
    }

    for (const auto &entry : entries) {
        // first argument is key and second argument is value.
        int errCode = Put(entry.key, entry.value);
        if (errCode != E_OK) {
            LOGE("PutBatch failed: %d", errCode);
            return CheckCorruptedStatus(errCode);
        }
    }

    return E_OK;
}

int SQLiteLocalStorageExecutor::DeleteBatch(const std::vector<Key> &keys)
{
    if (keys.empty()) {
        return -E_INVALID_ARGS;
    }

    bool isAllNoExisted = true;

    for (const auto &key : keys) {
        int errCode = Delete(key);
        if (errCode != E_OK && errCode != -E_NOT_FOUND) {
            return CheckCorruptedStatus(errCode);
        }
        if (errCode != -E_NOT_FOUND && isAllNoExisted == true) {
            isAllNoExisted = false;
        }
    }

    if (isAllNoExisted) {
        return -E_NOT_FOUND;
    }

    return E_OK;
}

int SQLiteLocalStorageExecutor::StartTransaction()
{
    int errCode = SQLiteUtils::BeginTransaction(dbHandle_);
    return CheckCorruptedStatus(errCode);
}

int SQLiteLocalStorageExecutor::Commit()
{
    int errCode = SQLiteUtils::CommitTransaction(dbHandle_);
    return CheckCorruptedStatus(errCode);
}

int SQLiteLocalStorageExecutor::RollBack()
{
    int errCode = SQLiteUtils::RollbackTransaction(dbHandle_);
    return CheckCorruptedStatus(errCode);
}

int SQLiteLocalStorageExecutor::Put(const Key &key, const Value &value)
{
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, INSERT_SQL, statement);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_KEY_INDEX, key, false);
    if (errCode != E_OK) {
        LOGE("Failed to bind the key.");
        goto END;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_VAL_INDEX, value, true);
    if (errCode != E_OK) {
        LOGE("Failed to bind the value");
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
    } else {
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteLocalStorageExecutor::Delete(const Key &key)
{
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, DELETE_SQL, statement);
    if (errCode != E_OK) {
        LOGE("Failed to get the delete statememt.");
        return CheckCorruptedStatus(errCode);
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_KEY_INDEX, key, false);
    if (errCode != E_OK) {
        LOGE("Bind key failed");
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        LOGE("Delete step error:%d", errCode);
    } else {
        if (sqlite3_changes(dbHandle_) > 0) {
            errCode = E_OK;
        } else {
            errCode = -E_NOT_FOUND;
        }
    }
END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}
} // namespace DistributedDB
