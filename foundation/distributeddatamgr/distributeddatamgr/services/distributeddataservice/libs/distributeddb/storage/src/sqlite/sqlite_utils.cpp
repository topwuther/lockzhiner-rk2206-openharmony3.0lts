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

#include "sqlite_utils.h"

#include <climits>
#include <cstring>
#include <chrono>
#include <thread>
#include <mutex>
#include <map>
#include <algorithm>

#include "sqlite_import.h"
#include "securec.h"
#include "db_constant.h"
#include "db_errno.h"
#include "db_common.h"
#include "log_print.h"
#include "value_object.h"
#include "schema_utils.h"
#include "time_helper.h"
#include "platform_specific.h"

namespace DistributedDB {
namespace {
    const int BUSY_TIMEOUT_MS = 3000; // 3000ms for sqlite busy timeout.
    const int BUSY_SLEEP_TIME = 50; // sleep for 50us
    const int NO_SIZE_LIMIT = -1;
    const int MAX_STEP_TIMES = 8000;
    const int BIND_KEY_INDEX = 1;
    const int BIND_VAL_INDEX = 2;
    const int USING_STR_LEN = -1;
    const std::string WAL_MODE_SQL = "PRAGMA journal_mode=WAL;";
    const std::string SYNC_MODE_FULL_SQL = "PRAGMA synchronous=FULL;";
    const std::string SYNC_MODE_NORMAL_SQL = "PRAGMA synchronous=NORMAL;";
    const std::string USER_VERSION_SQL = "PRAGMA user_version;";
    const std::string BEGIN_SQL = "BEGIN TRANSACTION";
    const std::string BEGIN_IMMEDIATE_SQL = "BEGIN IMMEDIATE TRANSACTION";
    const std::string COMMIT_SQL = "COMMIT TRANSACTION";
    const std::string ROLLBACK_SQL = "ROLLBACK TRANSACTION";
    const std::string JSON_EXTRACT_BY_PATH_TEST_CREATED = "SELECT json_extract_by_path('{\"field\":0}', '$.field', 0);";
    const std::string DEFAULT_ATTACH_CIPHER = "PRAGMA cipher_default_attach_cipher=";
    const std::string DEFAULT_ATTACH_KDF_ITER = "PRAGMA cipher_default_attach_kdf_iter=5000";
    const std::string EXPORT_BACKUP_SQL = "SELECT export_database('backup');";
    const std::string CIPHER_CONFIG_SQL = "PRAGMA codec_cipher=";
    const std::string KDF_ITER_CONFIG_SQL = "PRAGMA codec_kdf_iter=5000;";
    const std::string BACK_CIPHER_CONFIG_SQL = "PRAGMA backup.codec_cipher=";
    const std::string BACK_KDF_ITER_CONFIG_SQL = "PRAGMA backup.codec_kdf_iter=5000;";
    const std::string META_CIPHER_CONFIG_SQL = "PRAGMA meta.codec_cipher=";
    const std::string META_KDF_ITER_CONFIG_SQL = "PRAGMA meta.codec_kdf_iter=5000;";
    const std::string DETACH_BACKUP_SQL = "DETACH 'backup'";
    bool g_configLog = false;
    std::mutex g_logMutex;

    void SqliteLogCallback(void *data, int err, const char *msg)
    {
        bool verboseLog = (data != nullptr);
        auto errType = static_cast<unsigned int>(err);
        errType &= 0xFF;
        if (errType == 0 || errType == SQLITE_CONSTRAINT || errType == SQLITE_SCHEMA ||
            errType == SQLITE_NOTICE || err == SQLITE_WARNING_AUTOINDEX) {
            if (verboseLog) {
                LOGD("[SQLite] Error[%d] sys[%d] %s ", err, errno, sqlite3_errstr(err));
            }
        } else if (errType == SQLITE_WARNING || errType == SQLITE_IOERR ||
            errType == SQLITE_CORRUPT || errType == SQLITE_CANTOPEN) {
            LOGI("[SQLite] Error[%d], sys[%d], %s", err, errno, sqlite3_errstr(err));
        } else {
            LOGE("[SQLite] Error[%d], sys[%d]", err, errno);
        }
    }
}

int SQLiteUtils::CreateDataBase(const OpenDbProperties &properties, sqlite3 *&dbTemp)
{
    uint64_t flag = SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE;
    if (properties.createIfNecessary) {
        flag |= SQLITE_OPEN_CREATE;
    }
    std::string cipherName = GetCipherName(properties.cipherType);
    if (cipherName.empty()) {
        LOGE("[SQLite] GetCipherName failed");
        return -E_INVALID_ARGS;
    }
    std::string defaultAttachCipher = DEFAULT_ATTACH_CIPHER + cipherName + ";";
    std::vector<std::string> sqls {WAL_MODE_SQL, defaultAttachCipher, DEFAULT_ATTACH_KDF_ITER};

    std::string fileUrl = DBConstant::SQLITE_URL_PRE + properties.uri;
    int errCode = sqlite3_open_v2(fileUrl.c_str(), &dbTemp, flag, nullptr);
    if (errCode != SQLITE_OK) {
        LOGE("[SQLite] open database failed: %d - sys err(%d)", errCode, errno);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto END;
    }

    errCode = SetDataBaseProperty(dbTemp, properties, sqls);
    if (errCode != SQLITE_OK) {
        LOGE("[SQLite] SetDataBaseProperty failed: %d", errCode);
        goto END;
    }

END:
    if (errCode != E_OK && dbTemp != nullptr) {
        (void)sqlite3_close_v2(dbTemp);
        dbTemp = nullptr;
    }

    return errCode;
}

int SQLiteUtils::OpenDatabase(const OpenDbProperties &properties, sqlite3 *&db)
{
    {
        // Only for register the sqlite3 log callback
        std::lock_guard<std::mutex> lock(g_logMutex);
        if (!g_configLog) {
            sqlite3_config(SQLITE_CONFIG_LOG, &SqliteLogCallback, &properties.createIfNecessary);
            g_configLog = true;
        }
    }
    sqlite3 *dbTemp = nullptr;
    int errCode = CreateDataBase(properties, dbTemp);
    if (errCode != E_OK) {
        goto END;
    }
    errCode = RegisterJsonFunctions(dbTemp);
    if (errCode != E_OK) {
        goto END;
    }
    // Set the synchroized mode, default for full mode.
    errCode = ExecuteRawSQL(dbTemp, SYNC_MODE_FULL_SQL);
    if (errCode != E_OK) {
        LOGE("SQLite sync mode failed: %d", errCode);
    }
END:
    if (errCode != E_OK && dbTemp != nullptr) {
        (void)sqlite3_close_v2(dbTemp);
        dbTemp = nullptr;
    }
    if (errCode != E_OK && errno == EKEYREVOKED) {
        errCode = -E_EKEYREVOKED;
    }

    db = dbTemp;
    return errCode;
}

int SQLiteUtils::GetStatement(sqlite3 *db, const std::string &sql, sqlite3_stmt *&statement)
{
    if (db == nullptr) {
        LOGE("Invalid db for statement");
        return -E_INVALID_DB;
    }

    // Prepare the new statement only when the input parameter is not null
    if (statement != nullptr) {
        return E_OK;
    }

    int errCode = sqlite3_prepare_v2(db, sql.c_str(), NO_SIZE_LIMIT, &statement, nullptr);
    if (errCode != SQLITE_OK) {
        LOGE("Prepare SQLite statement failed:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        SQLiteUtils::ResetStatement(statement, true, errCode);
        return errCode;
    }

    if (statement == nullptr) {
        return -E_INVALID_DB;
    }

    return E_OK;
}

int SQLiteUtils::BindTextToStatement(sqlite3_stmt *statement, int index, const std::string &str)
{
    if (statement == nullptr) {
        return -E_INVALID_ARGS;
    }

    // Check empty value.
    if (str.empty()) {
        LOGI("[SQLiteUtil][Bind Text] Invalid value");
        return -E_INVALID_ARGS;
    }

    int errCode = sqlite3_bind_text(statement, index, str.c_str(), str.length(), SQLITE_TRANSIENT);
    if (errCode != SQLITE_OK) {
        LOGE("[SQLiteUtil][Bind text]Failed to bind the value:%d", errCode);
        return SQLiteUtils::MapSQLiteErrno(errCode);
    }

    return E_OK;
}

int SQLiteUtils::BindInt64ToStatement(sqlite3_stmt *statement, int index, int64_t value)
{
    // statement check outSide
    int errCode = sqlite3_bind_int64(statement, index, value);
    if (errCode != SQLITE_OK) {
        LOGE("[SQLiteUtil][Bind int64]Failed to bind the value:%d", errCode);
        return SQLiteUtils::MapSQLiteErrno(errCode);
    }

    return E_OK;
}

int SQLiteUtils::BindBlobToStatement(sqlite3_stmt *statement, int index, const std::vector<uint8_t> &value,
    bool permEmpty)
{
    if (statement == nullptr) {
        return -E_INVALID_ARGS;
    }

    // Check empty value.
    if (value.empty() && !permEmpty) {
        LOGI("[SQLiteUtil][Bind blob]Invalid value");
        return -E_INVALID_ARGS;
    }

    int errCode;
    if (value.empty()) {
        errCode = sqlite3_bind_zeroblob(statement, index, -1); // -1 for zero-length blob.
    } else {
        errCode = sqlite3_bind_blob(statement, index, static_cast<const void *>(value.data()),
            value.size(), SQLITE_TRANSIENT);
    }

    if (errCode != SQLITE_OK) {
        LOGE("[SQLiteUtil][Bind blob]Failed to bind the value:%d", errCode);
        return SQLiteUtils::MapSQLiteErrno(errCode);
    }

    return E_OK;
}

void SQLiteUtils::ResetStatement(sqlite3_stmt *&statement, bool isNeedFinalize, int &errCode)
{
    if (statement == nullptr) {
        return;
    }

    int innerCode = SQLITE_OK;
    // if need finalize the statement, just goto finalize.
    if (isNeedFinalize) {
        goto FINAL;
    }

    // reset the statement firstly.
    innerCode = sqlite3_reset(statement);
    if (innerCode != SQLITE_OK) {
        LOGE("[SQLiteUtils] reset statement error:%d, sys:%d", innerCode, errno);
        isNeedFinalize = true;
    } else {
        sqlite3_clear_bindings(statement);
    }

FINAL:
    if (isNeedFinalize) {
        int finalizeResult = sqlite3_finalize(statement);
        if (finalizeResult != SQLITE_OK) {
            LOGD("[SQLiteUtils] finalize statement error:%d, sys:%d", finalizeResult, errno);
            innerCode = finalizeResult;
        }
        statement = nullptr;
    }

    if (innerCode != SQLITE_OK) { // the sqlite error code has higher priority.
        errCode = SQLiteUtils::MapSQLiteErrno(innerCode);
    }
}

int SQLiteUtils::StepWithRetry(sqlite3_stmt *statement, bool isMemDb)
{
    if (statement == nullptr) {
        return -E_INVALID_ARGS;
    }

    int errCode = E_OK;
    int retryCount = 0;
    do {
        errCode = sqlite3_step(statement);
        if ((errCode == SQLITE_LOCKED) && isMemDb) {
            std::this_thread::sleep_for(std::chrono::microseconds(BUSY_SLEEP_TIME));
            retryCount++;
        } else {
            break;
        }
    } while (retryCount <= MAX_STEP_TIMES);

    if (errCode != SQLITE_DONE && errCode != SQLITE_ROW) {
        LOGE("[SQLiteUtils] Step error:%d, sys:%d", errCode, errno);
    }

    return SQLiteUtils::MapSQLiteErrno(errCode);
}

int SQLiteUtils::BindPrefixKey(sqlite3_stmt *statement, int index, const Key &keyPrefix)
{
    if (statement == nullptr) {
        return -E_INVALID_ARGS;
    }

    const size_t maxKeySize = DBConstant::MAX_KEY_SIZE;
    // bind the first prefix key
    int errCode = BindBlobToStatement(statement, index, keyPrefix, true);
    if (errCode != SQLITE_OK) {
        LOGE("Bind the prefix first error:%d", errCode);
        return SQLiteUtils::MapSQLiteErrno(errCode);
    }

    // bind the second prefix key
    uint8_t end[maxKeySize];
    errno_t status = memset_s(end, maxKeySize, UCHAR_MAX, maxKeySize); // max byte value is 0xFF.
    if (status != EOK) {
        LOGE("memset error:%d", status);
        return -E_SECUREC_ERROR;
    }

    if (!keyPrefix.empty()) {
        status = memcpy_s(end, maxKeySize, keyPrefix.data(), keyPrefix.size());
        if (status != EOK) {
            LOGE("memcpy error:%d", status);
            return -E_SECUREC_ERROR;
        }
    }

    // index wouldn't be too large, just add one to the first index.
    errCode = sqlite3_bind_blob(statement, index + 1, end, maxKeySize, SQLITE_TRANSIENT);
    if (errCode != SQLITE_OK) {
        LOGE("Bind the prefix second error:%d", errCode);
        return SQLiteUtils::MapSQLiteErrno(errCode);
    }

    return E_OK;
}

int SQLiteUtils::BeginTransaction(sqlite3 *db, TransactType type)
{
    if (type == TransactType::IMMEDIATE) {
        return ExecuteRawSQL(db, BEGIN_IMMEDIATE_SQL);
    }

    return ExecuteRawSQL(db, BEGIN_SQL);
}

int SQLiteUtils::CommitTransaction(sqlite3 *db)
{
    return ExecuteRawSQL(db, COMMIT_SQL);
}

int SQLiteUtils::RollbackTransaction(sqlite3 *db)
{
    return ExecuteRawSQL(db, ROLLBACK_SQL);
}

int SQLiteUtils::ExecuteRawSQL(sqlite3 *db, const std::string &sql)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }
    char *errMsg = nullptr;
    int errCode = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errCode != SQLITE_OK) {
        LOGE("[SQLiteUtils][ExecuteSQL] failed(%d), sys(%d)", errCode, errno);
    }

    if (errMsg != nullptr) {
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }
    return SQLiteUtils::MapSQLiteErrno(errCode);
}

int SQLiteUtils::SetKey(sqlite3 *db, CipherType type, const CipherPassword &passwd)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }

    if (passwd.GetSize() != 0) {
#ifndef OMIT_ENCRYPT
        int errCode = sqlite3_key(db, static_cast<const void *>(passwd.GetData()), static_cast<int>(passwd.GetSize()));
        if (errCode != SQLITE_OK) {
            LOGE("[SQLiteUtils][Setkey] config key failed:(%d)", errCode);
            return SQLiteUtils::MapSQLiteErrno(errCode);
        }

        errCode = SQLiteUtils::SetCipherSettings(db, type);
        if (errCode != E_OK) {
            LOGE("[SQLiteUtils][Setkey] set cipher settings failed:%d", errCode);
            return errCode;
        }
#else
        return -E_NOT_SUPPORT;
#endif
    }

    // verify key
    int errCode = SQLiteUtils::ExecuteRawSQL(db, USER_VERSION_SQL);
    if (errCode != E_OK) {
        LOGE("[SQLiteUtils][Setkey] verify version failed:%d", errCode);
        if (errno == EKEYREVOKED) {
            return -E_EKEYREVOKED;
        }
        return -E_INVALID_PASSWD_OR_CORRUPTED_DB;
    }
    return E_OK;
}

int SQLiteUtils::GetColumnBlobValue(sqlite3_stmt *statement, int index, std::vector<uint8_t> &value)
{
    if (statement == nullptr) {
        return -E_INVALID_ARGS;
    }

    int keySize = sqlite3_column_bytes(statement, index);
    auto keyRead = static_cast<const uint8_t *>(sqlite3_column_blob(statement, index));

    if (keySize < 0) {
        LOGE("[SQLiteUtils][Column blob] size:%d", keySize);
        return -E_INVALID_DATA;
    } else if (keySize == 0 || keyRead == nullptr) {
        value.resize(0);
    } else {
        value.resize(keySize);
        value.assign(keyRead, keyRead + keySize);
    }

    return E_OK;
}

int SQLiteUtils::AttachNewDatabase(sqlite3 *db, CipherType type, const CipherPassword &passwd,
    const std::string &attachDbAbsPath, const std::string &attachAsName)
{
    // example: "ATTACH '../new.db' AS backup KEY XXXX;"
    std::string attachSql = "ATTACH ? AS " + attachAsName + " KEY ?;"; // Internal interface not need verify alias name

    sqlite3_stmt* statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db, attachSql, statement);
    if (errCode != E_OK) {
        return errCode;
    }
    // 1st is name.
    errCode = sqlite3_bind_text(statement, 1, attachDbAbsPath.c_str(), attachDbAbsPath.length(), SQLITE_TRANSIENT);
    if (errCode != SQLITE_OK) {
        LOGE("Bind the attached db name failed:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto END;
    }
    // Passwords do not allow vector operations, so we can not use function BindBlobToStatement here.
    errCode = sqlite3_bind_blob(statement, 2, static_cast<const void *>(passwd.GetData()),
        passwd.GetSize(), SQLITE_TRANSIENT); // 2nd para is password.
    if (errCode != SQLITE_OK) {
        LOGE("Bind the attached key failed:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        LOGE("Execute the SQLite attach failed:%d", errCode);
        goto END;
    }
    errCode = SQLiteUtils::ExecuteRawSQL(db, WAL_MODE_SQL);
    if (errCode != E_OK) {
        LOGE("Set journal mode failed: %d", errCode);
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteUtils::CreateMetaDatabase(const std::string &metaDbPath)
{
    OpenDbProperties metaProperties {metaDbPath, true, false};
    sqlite3 *db = nullptr;
    int errCode = SQLiteUtils::OpenDatabase(metaProperties, db);
    if (errCode != E_OK) {
        LOGE("[CreateMetaDatabase] Failed to create the meta database[%d]", errCode);
    }
    if (db != nullptr) {
        (void)sqlite3_close_v2(db);
        db = nullptr;
    }
    return errCode;
}

#ifndef OMIT_ENCRYPT
int SQLiteUtils::ExportDatabase(sqlite3 *db, CipherType type, const CipherPassword &passwd,
    const std::string &newDbName)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = AttachNewDatabase(db, type, passwd, newDbName);
    if (errCode != E_OK) {
        LOGE("Attach New Db fail!");
        return errCode;
    }
    errCode = SQLiteUtils::ExecuteRawSQL(db, EXPORT_BACKUP_SQL);
    if (errCode != E_OK) {
        LOGE("Execute the SQLite export failed:%d", errCode);
    }

    int detachError = SQLiteUtils::ExecuteRawSQL(db, DETACH_BACKUP_SQL);
    if (errCode == E_OK) {
        errCode = detachError;
        if (detachError != E_OK) {
            LOGE("Execute the SQLite detach failed:%d", errCode);
        }
    }
    return errCode;
}

int SQLiteUtils::Rekey(sqlite3 *db, const CipherPassword &passwd)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = sqlite3_rekey(db, static_cast<const void *>(passwd.GetData()), static_cast<int>(passwd.GetSize()));
    if (errCode != E_OK) {
        LOGE("SQLite rekey failed:(%d)", errCode);
        return SQLiteUtils::MapSQLiteErrno(errCode);
    }

    return E_OK;
}
#else
int SQLiteUtils::ExportDatabase(sqlite3 *db, CipherType type, const CipherPassword &passwd,
    const std::string &newDbName)
{
    return -E_NOT_SUPPORT;
}

int SQLiteUtils::Rekey(sqlite3 *db, const CipherPassword &passwd)
{
    return -E_NOT_SUPPORT;
}
#endif

int SQLiteUtils::GetVersion(const OpenDbProperties &properties, int &version)
{
    if (properties.uri.empty()) {
        return -E_INVALID_ARGS;
    }

    sqlite3 *dbTemp = nullptr;
    // Please make sure the database file exists and is working properly
    std::string fileUrl = DBConstant::SQLITE_URL_PRE + properties.uri;
    int errCode = sqlite3_open_v2(fileUrl.c_str(), &dbTemp, SQLITE_OPEN_URI | SQLITE_OPEN_READONLY, nullptr);
    if (errCode != SQLITE_OK) {
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        LOGE("Open database failed: %d, sys:%d", errCode, errno);
        goto END;
    }
    errCode = SQLiteUtils::SetKey(dbTemp, properties.cipherType, properties.passwd);
    if (errCode != E_OK) {
        LOGE("Set key failed: %d", errCode);
        goto END;
    }

    errCode = GetVersion(dbTemp, version);

END:
    if (dbTemp != nullptr) {
        (void)sqlite3_close_v2(dbTemp);
        dbTemp = nullptr;
    }
    return errCode;
}

int SQLiteUtils::GetVersion(sqlite3 *db, int &version)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }

    std::string strSql = "PRAGMA user_version;";
    sqlite3_stmt *statement = nullptr;
    int errCode = sqlite3_prepare(db, strSql.c_str(), -1, &statement, nullptr);
    if (errCode != SQLITE_OK || statement == nullptr) {
        LOGE("[SqlUtil][GetVer] sqlite3_prepare failed.");
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        return errCode;
    }

    if (sqlite3_step(statement) == SQLITE_ROW) {
        // Get pragma user_version at first column
        version = sqlite3_column_int(statement, 0);
        LOGD("[SqlUtil][GetVer] db version=%d", version);
    } else {
        LOGE("[SqlUtil][GetVer] Get db user_version failed.");
        errCode = SQLiteUtils::MapSQLiteErrno(SQLITE_ERROR);
    }

    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteUtils::SetUserVer(const OpenDbProperties &properties, int version)
{
    if (properties.uri.empty()) {
        return -E_INVALID_ARGS;
    }

    // Please make sure the database file exists and is working properly
    sqlite3 *db = nullptr;
    int errCode = SQLiteUtils::OpenDatabase(properties, db);
    if (errCode != E_OK) {
        return errCode;
    }

    // Set user version
    errCode = SQLiteUtils::SetUserVer(db, version);
    if (errCode != E_OK) {
        LOGE("Set user version fail: %d", errCode);
        goto END;
    }

END:
    if (db != nullptr) {
        (void)sqlite3_close_v2(db);
        db = nullptr;
    }

    return errCode;
}

int SQLiteUtils::SetUserVer(sqlite3 *db, int version)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }
    std::string userVersionSql = "PRAGMA user_version=" + std::to_string(version) + ";";
    return SQLiteUtils::ExecuteRawSQL(db, userVersionSql);
}

int SQLiteUtils::MapSQLiteErrno(int errCode)
{
    if (errCode == SQLITE_OK) {
        return E_OK;
    } else if (errCode == SQLITE_IOERR) {
        if (errno == EKEYREVOKED) {
            return -E_EKEYREVOKED;
        }
    } else if (errCode == SQLITE_CORRUPT || errCode == SQLITE_NOTADB) {
        return -E_INVALID_PASSWD_OR_CORRUPTED_DB;
    } else if (errCode == SQLITE_LOCKED || errCode == SQLITE_BUSY) {
        return -E_BUSY;
    } else if (errCode == SQLITE_ERROR && errno == EKEYREVOKED) {
        return -E_EKEYREVOKED;
    }
    return -errCode;
}

int SQLiteUtils::SetBusyTimeout(sqlite3 *db, int timeout)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }

    // Set the default busy handler to retry automatically before returning SQLITE_BUSY.
    int errCode = sqlite3_busy_timeout(db, timeout);
    if (errCode != SQLITE_OK) {
        LOGE("[SQLite] set busy timeout failed:%d", errCode);
    }

    return SQLiteUtils::MapSQLiteErrno(errCode);
}

#ifndef OMIT_ENCRYPT
int SQLiteUtils::ExportDatabase(const std::string &srcFile, CipherType type, const CipherPassword &srcPasswd,
    const std::string &targetFile, const CipherPassword &passwd)
{
    std::vector<std::string> createTableSqls;
    OpenDbProperties option = {srcFile, true, false, createTableSqls, type, srcPasswd};
    sqlite3 *db = nullptr;
    int errCode = SQLiteUtils::OpenDatabase(option, db);
    if (errCode != E_OK) {
        LOGE("Open db error while exporting:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::ExportDatabase(db, type, passwd, targetFile);
    if (db != nullptr) {
        (void)sqlite3_close_v2(db);
        db = nullptr;
    }
    return errCode;
}
#else
int SQLiteUtils::ExportDatabase(const std::string &srcFile, CipherType type, const CipherPassword &srcPasswd,
    const std::string &targetFile, const CipherPassword &passwd)
{
    return -E_NOT_SUPPORT;
}
#endif

int SQLiteUtils::SaveSchema(const OpenDbProperties &properties)
{
    if (properties.uri.empty()) {
        return -E_INVALID_ARGS;
    }

    sqlite3 *db = nullptr;
    int errCode = OpenDatabase(properties, db);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SaveSchema(db, properties.schema);
    (void)sqlite3_close_v2(db);
    db = nullptr;
    return errCode;
}

int SQLiteUtils::SaveSchema(sqlite3 *db, const std::string &strSchema)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }

    sqlite3_stmt *statement = nullptr;
    std::string sql = "INSERT OR REPLACE INTO meta_data VALUES(?,?);";
    int errCode = GetStatement(db, sql, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    Key schemaKey;
    DBCommon::StringToVector(DBConstant::SCHEMA_KEY, schemaKey);
    errCode = BindBlobToStatement(statement, BIND_KEY_INDEX, schemaKey, false);
    if (errCode != E_OK) {
        ResetStatement(statement, true, errCode);
        return errCode;
    }

    Value schemaValue;
    DBCommon::StringToVector(strSchema, schemaValue);
    errCode = BindBlobToStatement(statement, BIND_VAL_INDEX, schemaValue, false);
    if (errCode != E_OK) {
        ResetStatement(statement, true, errCode);
        return errCode;
    }

    errCode = StepWithRetry(statement); // memory db does not support schema
    if (errCode != MapSQLiteErrno(SQLITE_DONE)) {
        LOGE("[SqlUtil][SetSchema] StepWithRetry fail, errCode=%d.", errCode);
        ResetStatement(statement, true, errCode);
        return errCode;
    }
    errCode = E_OK;
    ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteUtils::GetSchema(const OpenDbProperties &properties, std::string &strSchema)
{
    sqlite3 *db = nullptr;
    int errCode = OpenDatabase(properties, db);
    if (errCode != E_OK) {
        return errCode;
    }

    int version = 0;
    errCode = GetVersion(db, version);
    if (version <= 0 || errCode != E_OK) {
        // if version does exist, it represents database is error
        (void)sqlite3_close_v2(db);
        db = nullptr;
        return -E_INVALID_VERSION;
    }

    errCode = GetSchema(db, strSchema);
    (void)sqlite3_close_v2(db);
    db = nullptr;
    return errCode;
}

int SQLiteUtils::GetSchema(sqlite3 *db, std::string &strSchema)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }

    sqlite3_stmt *statement = nullptr;
    std::string sql = "SELECT value FROM meta_data WHERE key=?;";
    int errCode = GetStatement(db, sql, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    Key schemakey;
    DBCommon::StringToVector(DBConstant::SCHEMA_KEY, schemakey);
    errCode = BindBlobToStatement(statement, 1, schemakey, false);
    if (errCode != E_OK) {
        ResetStatement(statement, true, errCode);
        return errCode;
    }

    errCode = StepWithRetry(statement); // memory db does not support schema
    if (errCode == MapSQLiteErrno(SQLITE_DONE)) {
        ResetStatement(statement, true, errCode);
        return -E_NOT_FOUND;
    } else if (errCode != MapSQLiteErrno(SQLITE_ROW)) {
        ResetStatement(statement, true, errCode);
        return errCode;
    }

    Value schemaValue;
    errCode = GetColumnBlobValue(statement, 0, schemaValue);
    if (errCode != E_OK) {
        ResetStatement(statement, true, errCode);
        return errCode;
    }
    DBCommon::VectorToString(schemaValue, strSchema);
    ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteUtils::IncreaseIndex(sqlite3 *db, const IndexName &name, const IndexInfo &info, SchemaType type,
    uint32_t skipSize)
{
    if (db == nullptr) {
        LOGE("[IncreaseIndex] Sqlite DB not exists.");
        return -E_INVALID_DB;
    }
    if (name.empty()) {
        LOGE("[IncreaseIndex] Name can not be empty.");
        return -E_NOT_PERMIT;
    }
    if (info.empty()) {
        LOGE("[IncreaseIndex] Info can not be empty.");
        return -E_NOT_PERMIT;
    }
    std::string indexName = SchemaUtils::FieldPathString(name);
    std::string sqlCommand = "CREATE INDEX IF NOT EXISTS '" + indexName + "' ON sync_data (";
    for (uint32_t i = 0; i < info.size(); i++) {
        if (i != 0) {
            sqlCommand += ", ";
        }
        std::string extractSql = SchemaObject::GenerateExtractSQL(type, info[i].first, info[i].second,
            skipSize);
        if (extractSql.empty()) { // Unlikely
            LOGE("[IncreaseIndex] GenerateExtractSQL fail at field=%u.", i);
            return -E_INTERNAL_ERROR;
        }
        sqlCommand += extractSql;
    }
    sqlCommand += ") WHERE (flag&0x01=0);";
    return SQLiteUtils::ExecuteRawSQL(db, sqlCommand);
}

int SQLiteUtils::ChangeIndex(sqlite3 *db, const IndexName &name, const IndexInfo &info, SchemaType type,
    uint32_t skipSize)
{
    // Currently we change index by drop it then create it, SQLite "REINDEX" may be used in the future
    int errCode = DecreaseIndex(db, name);
    if (errCode != OK) {
        LOGE("[ChangeIndex] Decrease fail=%d.", errCode);
        return errCode;
    }
    errCode = IncreaseIndex(db, name, info, type, skipSize);
    if (errCode != OK) {
        LOGE("[ChangeIndex] Increase fail=%d.", errCode);
        return errCode;
    }
    return E_OK;
}

int SQLiteUtils::DecreaseIndex(sqlite3 *db, const IndexName &name)
{
    if (db == nullptr) {
        LOGE("[DecreaseIndex] Sqlite DB not exists.");
        return -E_INVALID_DB;
    }
    if (name.empty()) {
        LOGE("[DecreaseIndex] Name can not be empty.");
        return -E_NOT_PERMIT;
    }
    std::string indexName = SchemaUtils::FieldPathString(name);
    std::string sqlCommand = "DROP INDEX IF EXISTS '" + indexName + "';";
    return ExecuteRawSQL(db, sqlCommand);
}

int SQLiteUtils::RegisterJsonFunctions(sqlite3 *db)
{
    if (db == nullptr) {
        LOGE("Sqlite DB not exists.");
        return -E_INVALID_DB;
    }
    int errCode = sqlite3_create_function_v2(db, "calc_hash_key", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC,
        nullptr, &CalcHashKey, nullptr, nullptr, nullptr);
    if (errCode != SQLITE_OK) {
        LOGE("sqlite3_create_function_v2 about calc_hash_key returned %d", errCode);
        return MapSQLiteErrno(errCode);
    }
#ifdef USING_DB_JSON_EXTRACT_AUTOMATICALLY
    errCode = ExecuteRawSQL(db, JSON_EXTRACT_BY_PATH_TEST_CREATED);
    if (errCode == E_OK) {
        LOGI("json_extract_by_path already created.");
    } else {
        // Specify need 3 parameter in json_extract_by_path function
        errCode = sqlite3_create_function_v2(db, "json_extract_by_path", 3, SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr, &JsonExtractByPath, nullptr, nullptr, nullptr);
        if (errCode != SQLITE_OK) {
            LOGE("sqlite3_create_function_v2 about json_extract_by_path returned %d", errCode);
            return MapSQLiteErrno(errCode);
        }
    }
#endif
    return E_OK;
}

namespace {
void SchemaObjectDestructor(SchemaObject *inObject)
{
    delete inObject;
    inObject = nullptr;
}
}

int SQLiteUtils::RegisterFlatBufferFunction(sqlite3 *db, const std::string &inSchema)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }
    auto heapSchemaObj = new (std::nothrow) SchemaObject;
    if (heapSchemaObj == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    int errCode = heapSchemaObj->ParseFromSchemaString(inSchema);
    if (errCode != E_OK) { // Unlikely, it has been parsed before
        delete heapSchemaObj;
        heapSchemaObj = nullptr;
        return -E_INTERNAL_ERROR;
    }
    if (heapSchemaObj->GetSchemaType() != SchemaType::FLATBUFFER) { // Do not need to register FlatBufferExtract
        delete heapSchemaObj;
        heapSchemaObj = nullptr;
        return E_OK;
    }
    errCode = sqlite3_create_function_v2(db, SchemaObject::GetExtractFuncName(SchemaType::FLATBUFFER).c_str(),
        3, SQLITE_UTF8 | SQLITE_DETERMINISTIC, heapSchemaObj, &FlatBufferExtractByPath, nullptr, nullptr, // 3 args
        reinterpret_cast<void(*)(void*)>(SchemaObjectDestructor));
    // About the release of heapSchemaObj: SQLite guarantee that at following case, sqlite will invoke the destructor
    // (that is SchemaObjectDestructor) we passed to it. See sqlite.org for more information.
    // The destructor is invoked when the function is deleted, either by being overloaded or when the database
    // connection closes. The destructor is also invoked if the call to sqlite3_create_function_v2() fails
    if (errCode != SQLITE_OK) {
        LOGE("sqlite3_create_function_v2 about flatbuffer_extract_by_path return=%d.", errCode);
        // As mentioned above, SQLite had invoked the SchemaObjectDestructor to release the heapSchemaObj
        return MapSQLiteErrno(errCode);
    }
    return E_OK;
}

struct ValueParseCache {
    ValueObject valueParsed;
    std::vector<uint8_t> valueOriginal;
};

namespace {
inline bool IsDeleteRecord(const uint8_t *valueBlob, int valueBlobLen)
{
    return (valueBlob == nullptr) || (valueBlobLen <= 0); // In fact, sqlite guarantee valueBlobLen not negative
}

// Use the same cache id as sqlite use for json_extract which is substituted by our json_extract_by_path
// A negative cache-id enables sharing of cache between different operation during the same statement
constexpr int VALUE_CACHE_ID = -429938;

void ValueParseCacheFree(ValueParseCache *inCache)
{
    delete inCache;
    inCache = nullptr;
}

// We don't use cache array since we only cache value column of sqlite table, see sqlite implementation for compare.
const ValueObject *ParseValueThenCacheOrGetFromCache(sqlite3_context *ctx, const uint8_t *valueBlob,
    uint32_t valueBlobLen, uint32_t offset)
{
    // Note: All parameter had already been check inside JsonExtractByPath, only called by JsonExtractByPath
    auto cached = static_cast<ValueParseCache *>(sqlite3_get_auxdata(ctx, VALUE_CACHE_ID));
    if (cached != nullptr) { // A previous cache exist
        if (cached->valueOriginal.size() == valueBlobLen) {
            if (std::memcmp(cached->valueOriginal.data(), valueBlob, valueBlobLen) == 0) {
                // Cache match
                return &(cached->valueParsed);
            }
        }
    }
    // No cache or cache mismatch
    auto newCache = new (std::nothrow) ValueParseCache;
    if (newCache == nullptr) {
        sqlite3_result_error(ctx, "[ParseValueCache] OOM.", USING_STR_LEN);
        LOGE("[ParseValueCache] OOM.");
        return nullptr;
    }
    int errCode = newCache->valueParsed.Parse(valueBlob, valueBlob + valueBlobLen, offset);
    if (errCode != E_OK) {
        sqlite3_result_error(ctx, "[ParseValueCache] Parse fail.", USING_STR_LEN);
        LOGE("[ParseValueCache] Parse fail, errCode=%d.", errCode);
        delete newCache;
        newCache = nullptr;
        return nullptr;
    }
    newCache->valueOriginal.assign(valueBlob, valueBlob + valueBlobLen);
    sqlite3_set_auxdata(ctx, VALUE_CACHE_ID, newCache, reinterpret_cast<void(*)(void*)>(ValueParseCacheFree));
    // If sqlite3_set_auxdata fail, it will immediately call ValueParseCacheFree to delete newCache;
    // Next time sqlite3_set_auxdata will call ValueParseCacheFree to delete newCache of this time;
    // At the end, newCache will be eventually deleted when call sqlite3_reset or sqlite3_finalize;
    // Since sqlite3_set_auxdata may fail, we have to call sqlite3_get_auxdata other than return newCache directly.
    auto cacheInAuxdata = static_cast<ValueParseCache *>(sqlite3_get_auxdata(ctx, VALUE_CACHE_ID));
    if (cacheInAuxdata == nullptr) {
        return nullptr;
    }
    return &(cacheInAuxdata->valueParsed);
}
}

void SQLiteUtils::JsonExtractByPath(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
    if (ctx == nullptr || argc != 3 || argv == nullptr) { // 3 parameters, which are value, path and offset
        LOGE("[JsonExtract] Invalid parameter, argc=%d.", argc);
        return;
    }
    auto valueBlob = static_cast<const uint8_t *>(sqlite3_value_blob(argv[0]));
    int valueBlobLen = sqlite3_value_bytes(argv[0]);
    if (IsDeleteRecord(valueBlob, valueBlobLen)) {
        // Currently delete records are filtered out of query and create-index sql, so not allowed here.
        sqlite3_result_error(ctx, "[JsonExtract] Delete record not allowed.", USING_STR_LEN);
        LOGE("[JsonExtract] Delete record not allowed.");
        return;
    }
    auto path = reinterpret_cast<const char *>(sqlite3_value_text(argv[1]));
    int offset = sqlite3_value_int(argv[2]); // index 2 is the third parameter
    if ((path == nullptr) || (offset < 0)) {
        sqlite3_result_error(ctx, "[JsonExtract] Path nullptr or offset invalid.", USING_STR_LEN);
        LOGE("[JsonExtract] Path nullptr or offset=%d invalid.", offset);
        return;
    }
    FieldPath outPath;
    int errCode = SchemaUtils::ParseAndCheckFieldPath(path, outPath);
    if (errCode != E_OK) {
        sqlite3_result_error(ctx, "[JsonExtract] Path illegal.", USING_STR_LEN);
        LOGE("[JsonExtract] Path=%s illegal.", path);
        return;
    }
    // Parameter Check Done Here
    const ValueObject *valueObj = ParseValueThenCacheOrGetFromCache(ctx, valueBlob, static_cast<uint32_t>(valueBlobLen),
        static_cast<uint32_t>(offset));
    if (valueObj == nullptr) {
        return; // Necessary had been printed in ParseValueThenCacheOrGetFromCache
    }
    JsonExtractInnerFunc(ctx, *valueObj, outPath);
}

namespace {
inline bool IsExtractableType(FieldType inType)
{
    return (inType != FieldType::LEAF_FIELD_NULL && inType != FieldType::LEAF_FIELD_ARRAY &&
        inType != FieldType::LEAF_FIELD_OBJECT && inType != FieldType::INTERNAL_FIELD_OBJECT);
}
}

void SQLiteUtils::JsonExtractInnerFunc(sqlite3_context *ctx, const ValueObject &inValue, const FieldPath &inPath)
{
    FieldType outType = FieldType::LEAF_FIELD_NULL; // Default type null for invalid-path(path not exist)
    int errCode = inValue.GetFieldTypeByFieldPath(inPath, outType);
    if (errCode != E_OK && errCode != -E_INVALID_PATH) {
        sqlite3_result_error(ctx, "[JsonExtract] GetFieldType fail.", USING_STR_LEN);
        LOGE("[JsonExtract] GetFieldType fail, errCode=%d.", errCode);
        return;
    }
    FieldValue outValue;
    if (IsExtractableType(outType)) {
        errCode = inValue.GetFieldValueByFieldPath(inPath, outValue);
        if (errCode != E_OK) {
            sqlite3_result_error(ctx, "[JsonExtract] GetFieldValue fail.", USING_STR_LEN);
            LOGE("[JsonExtract] GetFieldValue fail, errCode=%d.", errCode);
            return;
        }
    }
    // FieldType null, array, object do not have value, all these FieldValue will be regarded as null in JsonReturn.
    ExtractReturn(ctx, outType, outValue);
}

// NOTE!!! This function is performance sensitive !!! Carefully not to allocate memory often!!!
void SQLiteUtils::FlatBufferExtractByPath(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
    if (ctx == nullptr || argc != 3 || argv == nullptr) { // 3 parameters, which are value, path and offset
        LOGE("[FlatBufferExtract] Invalid parameter, argc=%d.", argc);
        return;
    }
    auto schema = static_cast<SchemaObject *>(sqlite3_user_data(ctx));
    if (schema == nullptr || !schema->IsSchemaValid() || (schema->GetSchemaType() != SchemaType::FLATBUFFER)) {
        sqlite3_result_error(ctx, "[FlatBufferExtract] No SchemaObject or invalid.", USING_STR_LEN);
        LOGE("[FlatBufferExtract] No SchemaObject or invalid.");
        return;
    }
    // Get information from argv
    auto valueBlob = static_cast<const uint8_t *>(sqlite3_value_blob(argv[0]));
    int valueBlobLen = sqlite3_value_bytes(argv[0]);
    if (IsDeleteRecord(valueBlob, valueBlobLen)) {
        // Currently delete records are filtered out of query and create-index sql, so not allowed here.
        sqlite3_result_error(ctx, "[FlatBufferExtract] Delete record not allowed.", USING_STR_LEN);
        LOGE("[FlatBufferExtract] Delete record not allowed.");
        return;
    }
    auto path = reinterpret_cast<const char *>(sqlite3_value_text(argv[1]));
    int offset = sqlite3_value_int(argv[2]); // index 2 is the third parameter
    if ((path == nullptr) || (offset < 0) || (static_cast<uint32_t>(offset) != schema->GetSkipSize())) {
        sqlite3_result_error(ctx, "[FlatBufferExtract] Path null or offset invalid.", USING_STR_LEN);
        LOGE("[FlatBufferExtract] Path null or offset=%d(skipsize=%u) invalid.", offset, schema->GetSkipSize());
        return;
    }
    FlatBufferExtractInnerFunc(ctx, *schema, RawValue{valueBlob, valueBlobLen}, path);
}

namespace {
constexpr uint32_t FLATBUFFER_MAX_CACHE_SIZE = 102400; // 100 KBytes

void FlatBufferCacheFree(std::vector<uint8_t> *inCahce)
{
    delete inCahce;
    inCahce = nullptr;
}
}

void SQLiteUtils::FlatBufferExtractInnerFunc(sqlite3_context *ctx, const SchemaObject &schema, const RawValue &inValue,
    RawString inPath)
{
    // All parameter had already been check inside FlatBufferExtractByPath, only called by FlatBufferExtractByPath
    if (schema.GetSkipSize() % SECURE_BYTE_ALIGN == 0) {
        TypeValue outExtract;
        int errCode = schema.ExtractValue(ValueSource::FROM_DBFILE, inPath, inValue, outExtract, nullptr);
        if (errCode != E_OK) {
            sqlite3_result_error(ctx, "[FlatBufferExtract] ExtractValue fail.", USING_STR_LEN);
            LOGE("[FlatBufferExtract] ExtractValue fail, errCode=%d.", errCode);
            return;
        }
        ExtractReturn(ctx, outExtract.first, outExtract.second);
        return;
    }
    // Not byte-align secure, we have to make a cache for copy. Check whether cache had already exist.
    auto cached = static_cast<std::vector<uint8_t> *>(sqlite3_get_auxdata(ctx, VALUE_CACHE_ID)); // Share the same id
    if (cached == nullptr) {
        // Make the cache
        auto newCache = new (std::nothrow) std::vector<uint8_t>;
        if (newCache == nullptr) {
            sqlite3_result_error(ctx, "[FlatBufferExtract] OOM.", USING_STR_LEN);
            LOGE("[FlatBufferExtract] OOM.");
            return;
        }
        newCache->resize(FLATBUFFER_MAX_CACHE_SIZE);
        sqlite3_set_auxdata(ctx, VALUE_CACHE_ID, newCache, reinterpret_cast<void(*)(void*)>(FlatBufferCacheFree));
        // If sqlite3_set_auxdata fail, it will immediately call FlatBufferCacheFree to delete newCache;
        // Next time sqlite3_set_auxdata will call FlatBufferCacheFree to delete newCache of this time;
        // At the end, newCache will be eventually deleted when call sqlite3_reset or sqlite3_finalize;
        // Since sqlite3_set_auxdata may fail, we have to call sqlite3_get_auxdata other than return newCache directly.
        // See sqlite.org for more information.
        cached = static_cast<std::vector<uint8_t> *>(sqlite3_get_auxdata(ctx, VALUE_CACHE_ID));
    }
    if (cached == nullptr) {
        LOGW("[FlatBufferExtract] Something wrong with Auxdata, but it is no matter without cache.");
    }
    TypeValue outExtract;
    int errCode = schema.ExtractValue(ValueSource::FROM_DBFILE, inPath, inValue, outExtract, cached);
    if (errCode != E_OK) {
        sqlite3_result_error(ctx, "[FlatBufferExtract] ExtractValue fail.", USING_STR_LEN);
        LOGE("[FlatBufferExtract] ExtractValue fail, errCode=%d.", errCode);
        return;
    }
    ExtractReturn(ctx, outExtract.first, outExtract.second);
}

void SQLiteUtils::ExtractReturn(sqlite3_context *ctx, FieldType type, const FieldValue &value)
{
    if (ctx == nullptr) {
        return;
    }
    switch (type) {
        case FieldType::LEAF_FIELD_BOOL:
            sqlite3_result_int(ctx, (value.boolValue ? 1 : 0));
            break;
        case FieldType::LEAF_FIELD_INTEGER:
            sqlite3_result_int(ctx, value.integerValue);
            break;
        case FieldType::LEAF_FIELD_LONG:
            sqlite3_result_int64(ctx, value.longValue);
            break;
        case FieldType::LEAF_FIELD_DOUBLE:
            sqlite3_result_double(ctx, value.doubleValue);
            break;
        case FieldType::LEAF_FIELD_STRING:
            // The SQLITE_TRANSIENT value means that the content will likely change in the near future and
            // that SQLite should make its own private copy of the content before returning.
            sqlite3_result_text(ctx, value.stringValue.c_str(), -1, SQLITE_TRANSIENT); // -1 mean use the string length
            break;
        default:
            // All other type regard as null
            sqlite3_result_null(ctx);
    }
    return;
}

void SQLiteUtils::CalcHashKey(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
    // 1 means that the function only needs one parameter, namely key
    if (ctx == nullptr || argc != 1 || argv == nullptr) {
        LOGE("Parameter does not meet restrictions.");
        return;
    }
    auto keyBlob = static_cast<const uint8_t *>(sqlite3_value_blob(argv[0]));
    if (keyBlob == nullptr) {
        sqlite3_result_error(ctx, "Parameters is invalid.", USING_STR_LEN);
        LOGE("Parameters is invalid.");
        return;
    }
    int blobLen = sqlite3_value_bytes(argv[0]);
    std::vector<uint8_t> value(keyBlob, keyBlob + blobLen);
    std::vector<uint8_t> hashValue;
    int errCode = DBCommon::CalcValueHash(value, hashValue);
    if (errCode != E_OK) {
        sqlite3_result_error(ctx, "Get hash value error.", USING_STR_LEN);
        LOGE("Get hash value error.");
        return;
    }
    sqlite3_result_blob(ctx, hashValue.data(), hashValue.size(), SQLITE_TRANSIENT);
    return;
}

int SQLiteUtils::GetDbSize(const std::string &dir, const std::string &dbName, uint64_t &size)
{
    std::string dataDir = dir + "/" + dbName + DBConstant::SQLITE_DB_EXTENSION;
    uint64_t localDbSize = 0;
    int errCode = OS::CalFileSize(dataDir, localDbSize);
    if (errCode < 0) {
        LOGE("Failed to get the db file size, errno:%d", errno);
        if (errno == ENOENT) {
            return -E_NOT_FOUND;
        }
        return -E_INVALID_DB;
    }

    std::string shmFileName = dataDir + "-shm";
    uint64_t localshmFileSize = 0;
    errCode = OS::CalFileSize(shmFileName, localshmFileSize);
    if (errCode < 0) {
        localshmFileSize = 0;
    }

    std::string walFileName = dataDir + "-wal";
    uint64_t localWalFileSize = 0;
    errCode = OS::CalFileSize(walFileName, localWalFileSize);
    if (errCode < 0) {
        localWalFileSize = 0;
    }

    // 64-bit system is Suffice. Computer storage is less than uint64_t max
    size += (localDbSize + localshmFileSize + localWalFileSize);
    return E_OK;
}

int SQLiteUtils::ExplainPlan(sqlite3 *db, const std::string &execSql, bool isQueryPlan)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }

    sqlite3_stmt *statement = nullptr;
    std::string explainSql = (isQueryPlan ? "explain query plan " : "explain ") + execSql;
    int errCode = GetStatement(db, explainSql, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    bool isFirst = true;
    errCode = StepWithRetry(statement); // memory db does not support schema
    while (errCode == MapSQLiteErrno(SQLITE_ROW)) {
        int nCol = sqlite3_column_count(statement);
        nCol = std::min(nCol, 8); // Read 8 column at most

        if (isFirst) {
            std::string rowString;
            for (int i = 0; i < nCol; i++) {
                if (sqlite3_column_name(statement, i) != nullptr) {
                    rowString += sqlite3_column_name(statement, i);
                }
                int blankFill = (i + 1) * 16 - rowString.size(); // each column width 16
                rowString.append(static_cast<std::string::size_type>((blankFill > 0) ? blankFill : 0), ' ');
            }
            LOGD("#### %s", rowString.c_str());
            isFirst = false;
        }

        std::string rowString;
        for (int i = 0; i < nCol; i++) {
            if (sqlite3_column_text(statement, i) != nullptr) {
                rowString += reinterpret_cast<const std::string::value_type *>(sqlite3_column_text(statement, i));
            }
            int blankFill = (i + 1) * 16 - rowString.size(); // each column width 16
            rowString.append(static_cast<std::string::size_type>((blankFill > 0) ? blankFill : 0), ' ');
        }
        LOGD("#### %s", rowString.c_str());
        errCode = StepWithRetry(statement);
    }
    if (errCode != MapSQLiteErrno(SQLITE_DONE)) {
        LOGE("[SqlUtil][Explain] StepWithRetry fail, errCode=%d.", errCode);
        ResetStatement(statement, true, errCode);
        return errCode;
    }
    errCode = E_OK;
    ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteUtils::SetDataBaseProperty(sqlite3 *db, const OpenDbProperties &properties,
    const std::vector<std::string> &sqls)
{
    // Set the default busy handler to retry automatically before returning SQLITE_BUSY.
    int errCode = SetBusyTimeout(db, BUSY_TIMEOUT_MS);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = SQLiteUtils::SetKey(db, properties.cipherType, properties.passwd);
    if (errCode != E_OK) {
        LOGD("SQLiteUtils::SetKey fail!!![%d]", errCode);
        return errCode;
    }

    for (const auto &sql : sqls) {
        errCode = SQLiteUtils::ExecuteRawSQL(db, sql);
        if (errCode != E_OK) {
            LOGE("[SQLite] execute sql failed: %d", errCode);
            return errCode;
        }
    }
    // Create table if not exist according the sqls.
    if (properties.createIfNecessary) {
        for (const auto &sql : properties.sqls) {
            errCode = SQLiteUtils::ExecuteRawSQL(db, sql);
            if (errCode != E_OK) {
                LOGE("[SQLite] execute preset sqls failed");
                return errCode;
            }
        }
    }
    return E_OK;
}

#ifndef OMIT_ENCRYPT
int SQLiteUtils::SetCipherSettings(sqlite3 *db, CipherType type)
{
    if (db == nullptr) {
        return -E_INVALID_DB;
    }
    std::string cipherName = GetCipherName(type);
    if (cipherName.empty()) {
        return -E_INVALID_ARGS;
    }
    std::string cipherConfig = CIPHER_CONFIG_SQL + cipherName + ";";
    int errCode = SQLiteUtils::ExecuteRawSQL(db, cipherConfig);
    if (errCode != E_OK) {
        LOGE("[SQLiteUtils][SetCipherSettings] config cipher failed:%d", errCode);
        return errCode;
    }
    errCode = SQLiteUtils::ExecuteRawSQL(db, KDF_ITER_CONFIG_SQL);
    if (errCode != E_OK) {
        LOGE("[SQLiteUtils][SetCipherSettings] config iter failed:%d", errCode);
        return errCode;
    }
    return errCode;
}

std::string SQLiteUtils::GetCipherName(CipherType type)
{
    if (type == CipherType::AES_256_GCM || type == CipherType::DEFAULT) {
        return "'aes-256-gcm'";
    }
    return "";
}
#endif
} // namespace DistributedDB