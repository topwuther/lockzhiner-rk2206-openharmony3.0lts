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

#include "local_database_oper.h"

#include "log_print.h"
#include "platform_specific.h"
#include "db_errno.h"
#include "db_constant.h"
#include "db_common.h"

namespace DistributedDB {
LocalDatabaseOper::LocalDatabaseOper(SQLiteLocalKvDB *localKvDb, SQLiteStorageEngine *storageEngine)
    : localKvDb_(localKvDb),
      storageEngine_(storageEngine)
{}

int LocalDatabaseOper::Rekey(const CipherPassword &passwd)
{
    if (localKvDb_ == nullptr || storageEngine_ == nullptr) {
        return -E_INVALID_DB;
    }

    return ExecuteRekey(passwd, localKvDb_->GetDbProperties());
}

int LocalDatabaseOper::Import(const std::string &filePath, const CipherPassword &passwd)
{
    if (localKvDb_ == nullptr || storageEngine_ == nullptr) {
        return -E_INVALID_DB;
    }

    return ExecuteImport(filePath, passwd, localKvDb_->GetDbProperties());
}

int LocalDatabaseOper::Export(const std::string &filePath, const CipherPassword &passwd) const
{
    return ExecuteExport(filePath, passwd, localKvDb_->GetDbProperties());
}

bool LocalDatabaseOper::RekeyPreHandle(const CipherPassword &passwd, int &errCode)
{
    CipherType cipherType;
    CipherPassword cachePasswd;
    localKvDb_->GetDbProperties().GetPassword(cipherType, cachePasswd);

    if (cachePasswd.GetSize() == 0 && passwd.GetSize() == 0) {
        errCode = E_OK;
        return false;
    }

    // need invoke sqlite3 rekey
    if (cachePasswd.GetSize() > 0 && passwd.GetSize() > 0) {
        errCode = localKvDb_->RunRekeyLogic(cipherType, passwd);
        return false;
    }

    return true;
}

int LocalDatabaseOper::BackupDb(const CipherPassword &passwd) const
{
    std::string filePrefix;
    int errCode = GetCtrlFilePrefix(localKvDb_->GetDbProperties(), filePrefix);
    if (errCode != E_OK) {
        return errCode;
    }

    // create backup dir
    std::string backupDir = filePrefix + DBConstant::PATH_BACKUP_POSTFIX;
    errCode = DBCommon::CreateDirectory(backupDir);
    if (errCode != E_OK) {
        LOGE("create backup dir failed:%d.", errCode);
        return errCode;
    }

    // export db to backup
    CipherType cipherType;
    CipherPassword oldPasswd;
    localKvDb_->GetDbProperties().GetPassword(cipherType, oldPasswd);
    std::string backupDbName = backupDir + "/" + DBConstant::LOCAL_DATABASE_NAME + DBConstant::SQLITE_DB_EXTENSION;
    return localKvDb_->RunExportLogic(cipherType, passwd, backupDbName);
}

int LocalDatabaseOper::CloseStorages()
{
    // close old db
    storageEngine_->Release();
    int errCode = RekeyRecover(localKvDb_->GetDbProperties());
    if (errCode != E_OK) {
        LOGE("Recover failed after rekey ok:%d.", errCode);
        int innerCode = localKvDb_->InitDatabaseContext(localKvDb_->GetDbProperties());
        if (innerCode != E_OK) {
            LOGE("ReInit the handlePool failed:%d", innerCode);
        }
    }
    return errCode;
}

int LocalDatabaseOper::RekeyPostHandle(const CipherPassword &passwd)
{
    CipherType cipherType;
    CipherPassword oldPasswd;
    localKvDb_->GetDbPropertyForUpdate().GetPassword(cipherType, oldPasswd);
    localKvDb_->GetDbPropertyForUpdate().SetPassword(cipherType, passwd);
    return localKvDb_->InitDatabaseContext(localKvDb_->GetDbProperties());
}

int LocalDatabaseOper::ExportAllDatabases(const std::string &currentDir, const CipherPassword &passwd,
    const std::string &dbDir) const
{
    std::string backupDbName = dbDir + DBConstant::LOCAL_DATABASE_NAME + DBConstant::SQLITE_DB_EXTENSION;
    std::string currentDb = currentDir + "/" + DBConstant::LOCAL_DATABASE_NAME + DBConstant::SQLITE_DB_EXTENSION;

    CipherType cipherType;
    CipherPassword currPasswd;
    localKvDb_->GetDbProperties().GetPassword(cipherType, currPasswd);
    int errCode = SQLiteUtils::ExportDatabase(currentDb, cipherType, currPasswd, backupDbName, passwd);
    if (errCode != E_OK) {
        LOGE("Export the database failed:%d", errCode);
    }
    return errCode;
}

int LocalDatabaseOper::BackupCurrentDatabase(const ImportFileInfo &info) const
{
    storageEngine_->Release();
    // create the pre flag file.
    int errCode = OS::CreateFileByFileName(info.curValidFile);
    if (errCode != E_OK) {
        LOGE("create ctrl file failed:%d.", errCode);
        return errCode;
    }

    // create backup dir
    errCode = DBCommon::CreateDirectory(info.backupDir);
    if (errCode != E_OK) {
        LOGE("Create backup dir failed:%d.", errCode);
        (void)RemoveFile(info.curValidFile);
        return errCode;
    }

    std::string currentFile = info.currentDir + DBConstant::LOCAL_DATABASE_NAME +
        DBConstant::SQLITE_DB_EXTENSION;
    std::string backupFile = info.backupDir + DBConstant::LOCAL_DATABASE_NAME +
        DBConstant::SQLITE_DB_EXTENSION;
    errCode = DBCommon::CopyFile(currentFile, backupFile);
    if (errCode != E_OK) {
        LOGE("Backup the current database error:%d", errCode);
        return errCode;
    }
    int innerCode = rename(info.curValidFile.c_str(), info.backValidFile.c_str());
    if (innerCode != 0) {
        LOGE("Failed to rename the file after the backup:%d", errno);
        errCode = -E_SYSTEM_API_FAIL;
    }
    return errCode;
}

int LocalDatabaseOper::ImportUnpackedDatabase(const ImportFileInfo &info, const CipherPassword &srcPasswd) const
{
    // create backup dir
    int errCode = DBCommon::RemoveAllFilesOfDirectory(info.currentDir, false);
    if (errCode != E_OK) {
        return errCode;
    }

    std::string unpackedFile = info.unpackedDir + DBConstant::LOCAL_DATABASE_NAME + DBConstant::SQLITE_DB_EXTENSION;
    std::string currentFile = info.currentDir + DBConstant::LOCAL_DATABASE_NAME + DBConstant::SQLITE_DB_EXTENSION;
    CipherType cipherType;
    CipherPassword passwd;
    localKvDb_->GetDbProperties().GetPassword(cipherType, passwd);
    errCode = SQLiteUtils::ExportDatabase(unpackedFile, cipherType, srcPasswd, currentFile, passwd);
    DBCommon::RemoveAllFilesOfDirectory(info.unpackedDir);
    if (errCode != E_OK) {
        LOGE("export the unpacked database to current error:%d", errCode);
        errCode = -E_INVALID_FILE;
        return errCode;
    }

    // reinitialize the database, and delete the backup database.
    errCode = localKvDb_->InitDatabaseContext(localKvDb_->GetDbProperties());
    if (errCode != E_OK) {
        LOGE("InitDatabaseContext error:%d", errCode);
        return errCode;
    }

    // rename the flag file.
    int innerCode = rename(info.backValidFile.c_str(), info.curValidFile.c_str());
    if (innerCode != E_OK) {
        LOGE("Failed to rename after the import operation:%d", errno);
        errCode = -E_SYSTEM_API_FAIL;
    }

    return errCode;
}

int LocalDatabaseOper::ImportPostHandle() const
{
    return localKvDb_->InitDatabaseContext(localKvDb_->GetDbProperties());
}
} // namespace DistributedDB
