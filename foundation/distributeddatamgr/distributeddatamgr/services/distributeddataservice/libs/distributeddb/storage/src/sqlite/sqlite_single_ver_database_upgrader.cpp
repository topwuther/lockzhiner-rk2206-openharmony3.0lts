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

#include "sqlite_single_ver_database_upgrader.h"
#include "db_errno.h"
#include "log_print.h"
#include "version.h"
#include "db_constant.h"
#include "platform_specific.h"
#include "param_check_utils.h"
#include "runtime_context.h"

namespace DistributedDB {
namespace {
    const std::string CREATE_LOCAL_TABLE_SQL =
        "CREATE TABLE IF NOT EXISTS local_data(" \
            "key BLOB PRIMARY KEY," \
            "value BLOB," \
            "timestamp INT," \
            "hash_key BLOB);";

    const std::string CREATE_SYNC_TABLE_SQL =
        "CREATE TABLE IF NOT EXISTS sync_data(" \
            "key         BLOB NOT NULL," \
            "value       BLOB," \
            "timestamp   INT  NOT NULL," \
            "flag        INT  NOT NULL," \
            "device      BLOB," \
            "ori_device  BLOB," \
            "hash_key    BLOB PRIMARY KEY NOT NULL," \
            "w_timestamp INT);";

    const std::string CREATE_META_TABLE_SQL =
        "CREATE TABLE IF NOT EXISTS meta_data("  \
            "key    BLOB PRIMARY KEY  NOT NULL," \
            "value  BLOB);";

    const std::string CREATE_SINGLE_META_TABLE_SQL =
        "CREATE TABLE IF NOT EXISTS meta.meta_data("  \
            "key    BLOB PRIMARY KEY  NOT NULL," \
            "value  BLOB);";

    const std::string CREATE_SYNC_TABLE_INDEX_SQL =
        "CREATE INDEX IF NOT EXISTS key_index ON sync_data (key, flag);" \
        "CREATE INDEX IF NOT EXISTS time_index ON sync_data (timestamp);" \
        "CREATE INDEX IF NOT EXISTS dev_index ON sync_data (device);" \
        "CREATE INDEX IF NOT EXISTS local_hashkey_index ON local_data (hash_key);";

    const std::string DROP_META_TABLE_SQL = "DROP TABLE IF EXISTS main.meta_data;";
    const std::string COPY_META_TABLE_SQL = "INSERT OR REPLACE INTO meta.meta_data SELECT * FROM meta_data "
        "where (SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='main.meta_data') > 0;";
}

SQLiteSingleVerDatabaseUpgrader::SQLiteSingleVerDatabaseUpgrader(sqlite3 *db,
    const SecurityOption &secopt, bool isMemDb)
    : db_(db),
      secOpt_(secopt),
      isMemDB_(isMemDb),
      isMetaUpgrade_(false)
{
}

SQLiteSingleVerDatabaseUpgrader::~SQLiteSingleVerDatabaseUpgrader()
{
    db_ = nullptr;
}

int SQLiteSingleVerDatabaseUpgrader::TransferDatabasePath(const std::string &parentDir,
    const OpenDbProperties &option)
{
    std::string dbFilePath = parentDir + "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    std::string upgradeLockFile = parentDir + "/" + DBConstant::UPGRADE_POSTFIX;

    if (OS::CheckPathExistence(upgradeLockFile)) {
        return MoveDatabaseToNewDir(parentDir, upgradeLockFile);
    }
    if (OS::CheckPathExistence(dbFilePath)) {
        int currentVersion = 0;
        int errCode = GetDbVersion(dbFilePath, option, currentVersion);
        if (errCode != E_OK) {
            LOGE("[SQLiteSinVerUp] Get version of old database failed");
            return errCode;
        }
        if (currentVersion == 0) {
            LOGI("The database file has not been initialized, maybe invalid database");
            if (OS::RemoveFile(dbFilePath) != E_OK) {
                LOGE("[SQLiteSinVerUp] Remove the uninitialized database failed, errno[%d]", errno);
                return -E_SYSTEM_API_FAIL;
            }
        }
        if (currentVersion >= SINGLE_VER_STORE_VERSION_V1 && currentVersion <= SINGLE_VER_STORE_VERSION_V2) {
            LOGI("[SQLiteSinVerUp] Old version[%d] database exists.", currentVersion);
            if (OS::CreateFileByFileName(upgradeLockFile) != E_OK) {
                return -E_SYSTEM_API_FAIL;
            }
            return MoveDatabaseToNewDir(parentDir, upgradeLockFile);
        }
    }
    return E_OK;
}

int SQLiteSingleVerDatabaseUpgrader::BeginUpgrade()
{
    return SQLiteUtils::BeginTransaction(db_, TransactType::IMMEDIATE);
}

int SQLiteSingleVerDatabaseUpgrader::EndUpgrade(bool isSuccess)
{
    if (isSuccess) {
        return SQLiteUtils::CommitTransaction(db_);
    } else {
        int errCode = SQLiteUtils::RollbackTransaction(db_);
        std::string secOptUpgradeFile = subDir_ + "/" + DBConstant::SET_SECOPT_POSTFIX;
        if (errCode == E_OK && OS::CheckPathExistence(secOptUpgradeFile) &&
            (OS::RemoveFile(secOptUpgradeFile) != E_OK)) {
            LOGW("[EndUpgrade] Delete secure upgrade file failed");
            return -E_SYSTEM_API_FAIL;
        }
        return errCode;
    }
}

int SQLiteSingleVerDatabaseUpgrader::GetDatabaseVersion(int &version) const
{
    return SQLiteUtils::GetVersion(db_, version);
}

int SQLiteSingleVerDatabaseUpgrader::SetDatabaseVersion(int version)
{
    return SQLiteUtils::SetUserVer(db_, version);
}

void SQLiteSingleVerDatabaseUpgrader::SetUpgradeSqls(int version, std::vector<std::string> &sqls,
    bool &isCreateUpgradeFile) const
{
    if (version == 0) { // no write version.
        if ((!isMemDB_) && ParamCheckUtils::IsS3SECEOpt(secOpt_)) {
            sqls = {
                CREATE_LOCAL_TABLE_SQL,
                CREATE_SINGLE_META_TABLE_SQL,
                CREATE_SYNC_TABLE_SQL,
                CREATE_SYNC_TABLE_INDEX_SQL
            };
        } else {
            sqls = {
                CREATE_LOCAL_TABLE_SQL,
                CREATE_META_TABLE_SQL,
                CREATE_SYNC_TABLE_SQL,
                CREATE_SYNC_TABLE_INDEX_SQL
            };
        }
    } else {
        if (version <= SINGLE_VER_STORE_VERSION_V1) {
            sqls = {
                "DROP INDEX key_index;",
                "CREATE INDEX IF NOT EXISTS key_index ON sync_data (key, flag);",
                "ALTER TABLE sync_data ADD w_timestamp INT;",
                "UPDATE sync_data SET w_timestamp=timestamp;",
                "ALTER TABLE local_data ADD timestamp INT;",
                "ALTER TABLE local_data ADD hash_key BLOB;",
                "UPDATE local_data SET hash_key=calc_hash_key(key), timestamp=0;",
                "CREATE INDEX IF NOT EXISTS local_hashkey_index ON local_data (hash_key);"
            };
        }
        if ((version <= SINGLE_VER_STORE_VERSION_V2 && ParamCheckUtils::IsS3SECEOpt(secOpt_)) ||
            (version == SINGLE_VER_STORE_VERSION_CURRENT && isMetaUpgrade_ == true)) {
            sqls.push_back(CREATE_SINGLE_META_TABLE_SQL);
            sqls.push_back(COPY_META_TABLE_SQL);
            sqls.push_back(DROP_META_TABLE_SQL);
            isCreateUpgradeFile = true;
        }
    }
}

int SQLiteSingleVerDatabaseUpgrader::UpgradeFromDatabaseVersion(int version)
{
    std::vector<std::string> sqls;
    bool isCreateUpgradeFile = false;
    LOGI("[SqlSingleUp]version=%d, metaSplit[%d], secLabel[%d], secFlag[%d]",
        version, isMetaUpgrade_, secOpt_.securityLabel, secOpt_.securityFlag);
    SetUpgradeSqls(version, sqls, isCreateUpgradeFile);
    for (const auto &item : sqls) {
        int errCode = SQLiteUtils::ExecuteRawSQL(db_, item);
        if (errCode != E_OK) {
            LOGE("[SqlSingleUp][UpFrom] Execute upgrade sql failed:%d", errCode);
            return errCode;
        }
    }
    if (isCreateUpgradeFile) {
        std::string secOptUpgradeFile = subDir_ + "/" + DBConstant::SET_SECOPT_POSTFIX;
        if (!OS::CheckPathExistence(secOptUpgradeFile) && (OS::CreateFileByFileName(secOptUpgradeFile) != E_OK)) {
            LOGE("[SqlSingleUp][UpFrom] Create s3sece flag file failed");
            return -E_SYSTEM_API_FAIL;
        }
        LOGD("[SqlSingleUp][UpFrom] Create s3sece mark file success");
    }
    return E_OK;
}

int SQLiteSingleVerDatabaseUpgrader::GetDbVersion(const std::string &dbPath, const OpenDbProperties &option,
    int &version)
{
    OpenDbProperties optionTmp(option);
    optionTmp.uri = dbPath;
    sqlite3 *db = nullptr;
    int errCode = SQLiteUtils::OpenDatabase(optionTmp, db);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = SQLiteUtils::GetVersion(db, version);
    (void)sqlite3_close_v2(db);
    db = nullptr;
    return errCode;
}

void SQLiteSingleVerDatabaseUpgrader::SetMetaUpgrade(const SecurityOption &currentOpt,
    const SecurityOption &expectOpt, const std::string &subDir)
{
    std::string secOptUpgradeFile = subDir + "/" + DBConstant::SET_SECOPT_POSTFIX;
    // the same version should upgrade while user open db with s3sece.
    if ((!OS::CheckPathExistence(secOptUpgradeFile)) && currentOpt.securityLabel == SecurityLabel::NOT_SET &&
        ParamCheckUtils::IsS3SECEOpt(expectOpt)) {
        isMetaUpgrade_ = true;
    } else {
        isMetaUpgrade_ = false;
    }
}

void SQLiteSingleVerDatabaseUpgrader::SetSubdir(const std::string &parentDir)
{
    subDir_ = parentDir;
}

int SQLiteSingleVerDatabaseUpgrader::SetPathSecOptWithCheck(const std::string &path, const SecurityOption &secOption,
    const std::string &dbStore, bool isWithChecked)
{
    SecurityOption dbOpt;
    std::vector<std::string> dbFilePathVec {DBConstant::SQLITE_DB_EXTENSION};
    std::string dbFilePath = path + "/" + dbStore + DBConstant::SQLITE_DB_EXTENSION;
    if (OS::CheckPathExistence(dbFilePath) && isWithChecked) {
        int errCode = RuntimeContext::GetInstance()->GetSecurityOption(dbFilePath, dbOpt);
        if (errCode != E_OK) {
            LOGE("[SetPathSecOptWithCheck] GetSecurityOption failed:%d", errCode);
            if (errCode == -E_NOT_SUPPORT) {
                dbOpt = SecurityOption();
            } else {
                return errCode;
            }
        }
    }

    for (const auto &item : dbFilePathVec) {
        std::string dbItemFilePath = path + "/" + dbStore + item;
        if (!OS::CheckPathExistence(dbItemFilePath)) {
            continue;
        }
        if (OS::CheckPathExistence(dbItemFilePath) && dbOpt.securityLabel == NOT_SET) {
            int errCode = RuntimeContext::GetInstance()->SetSecurityOption(dbItemFilePath, secOption);
            if (errCode != E_OK) {
                LOGE("[SetPathSecOptWithCheck] SetSecurityOption failed.");
                return errCode;
            }
        } else if (dbOpt == secOption) {
            LOGI("[SetPathSecOptWithCheck] already set secoption");
        } else {
            LOGE("[SetPathSecOptWithCheck] already set secoption,but different from early option.");
            return -E_INVALID_ARGS;
        }
    }
    return E_OK;
}

int SQLiteSingleVerDatabaseUpgrader::SetSecOption(const std::string &path, const SecurityOption &secOption,
    bool isWithChecked)
{
    if (!ParamCheckUtils::CheckSecOption(secOption)) {
        return -E_INVALID_ARGS;
    }
    if (secOption.securityLabel == NOT_SET) {
        return E_OK;
    }
    std::string secOptUpgradeFile = path + "/" + DBConstant::SET_SECOPT_POSTFIX;
    if (OS::CheckPathExistence(secOptUpgradeFile) && !ParamCheckUtils::IsS3SECEOpt(secOption)) {
        LOGE("[SingleVerUp][SetSec] Security option is invalid");
        return -E_INVALID_ARGS;
    }
    int errCode = E_OK;
    if (secOption.securityLabel != NOT_SET) {
        std::string mainDbPath = path + "/" + DBConstant::MAINDB_DIR;
        std::string cacheDbPath = path + "/" + DBConstant::CACHEDB_DIR;
        std::string metaDbPath = path + "/" + DBConstant::METADB_DIR;
        errCode = SetPathSecOptWithCheck(mainDbPath, secOption, DBConstant::SINGLE_VER_DATA_STORE, isWithChecked);
        if (errCode != E_OK) {
            return errCode;
        }
        errCode = SetPathSecOptWithCheck(cacheDbPath, secOption, DBConstant::SINGLE_VER_CACHE_STORE, isWithChecked);
        if (errCode != E_OK) {
            LOGE("[SQLiteSingleVerDatabaseUpgrader] cacheDb SetSecurityOption failed.");
            return errCode;
        }
        SecurityOption metaSecOpt;
        metaSecOpt.securityLabel = ((secOption.securityLabel >= SecurityLabel::S2) ?
            SecurityLabel::S2 : secOption.securityLabel);
        errCode = SetPathSecOptWithCheck(metaDbPath, metaSecOpt, DBConstant::SINGLE_VER_META_STORE, false);
        if (errCode != E_OK) {
            LOGE("[SQLiteSingleVerDatabaseUpgrader] metaDb SetSecurityOption failed.");
            return errCode;
        }
    }
    if (OS::CheckPathExistence(secOptUpgradeFile) && (OS::RemoveFile(secOptUpgradeFile) != E_OK)) {
        return -E_SYSTEM_API_FAIL;
    }

    return errCode;
}

int SQLiteSingleVerDatabaseUpgrader::MoveDatabaseToNewDir(const std::string &parentDir,
    const std::string &upgradeLockFile)
{
    std::vector<std::string> dbFilePathVec {DBConstant::SQLITE_DB_EXTENSION, ".db-wal", ".db-shm"};
    for (const auto &item : dbFilePathVec) {
        std::string oldDbPath = parentDir + "/" + DBConstant::SINGLE_VER_DATA_STORE + item;
        std::string currentDbPath = parentDir + "/" + DBConstant::MAINDB_DIR + "/" +
            DBConstant::SINGLE_VER_DATA_STORE + item;
        if (OS::CheckPathExistence(oldDbPath)) {
            if (OS::RenameFilePath(oldDbPath, currentDbPath) != E_OK) {
                LOGE("[SQLiteSinVerUp] Move database file to the new directory failed, errno:%d", errno);
                return -E_SYSTEM_API_FAIL;
            }
        }
    }
    int errCode = OS::RemoveFile(upgradeLockFile);
    if (errCode != E_OK) {
        LOGE("[SQLiteSinVerUp] Remove upgrade flag file failed, errno:%d", errno);
    }
    return errCode;
}
} // namespace DistributedDB
