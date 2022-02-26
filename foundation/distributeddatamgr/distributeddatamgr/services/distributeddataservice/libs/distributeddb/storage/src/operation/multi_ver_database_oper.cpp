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

#ifndef OMIT_MULTI_VER
#include "multi_ver_database_oper.h"

#include "db_errno.h"
#include "log_print.h"
#include "db_constant.h"
#include "db_common.h"
#include "platform_specific.h"
#include "sqlite_multi_ver_data_storage.h"
#include "multi_ver_natural_store_commit_storage.h"

namespace DistributedDB {
MultiVerDatabaseOper::MultiVerDatabaseOper(MultiVerNaturalStore *multiVerNaturalStore,
    IKvDBMultiVerDataStorage *multiVerData, IKvDBCommitStorage *commitHistory, MultiVerKvDataStorage *multiVerKvStorage)
    : multiVerNaturalStore_(multiVerNaturalStore),
      multiVerData_(multiVerData),
      commitHistory_(commitHistory),
      multiVerKvStorage_(multiVerKvStorage)
{}

int MultiVerDatabaseOper::Rekey(const CipherPassword &passwd)
{
    if (multiVerNaturalStore_ == nullptr || multiVerData_ == nullptr || commitHistory_ == nullptr ||
        multiVerKvStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    return ExecuteRekey(passwd, multiVerNaturalStore_->GetDbProperties());
}

int MultiVerDatabaseOper::Import(const std::string &filePath, const CipherPassword &passwd)
{
    if (multiVerNaturalStore_ == nullptr || multiVerData_ == nullptr || commitHistory_ == nullptr ||
        multiVerKvStorage_ == nullptr) {
        return -E_INVALID_DB;
    }

    return ExecuteImport(filePath, passwd, multiVerNaturalStore_->GetDbProperties());
}

int MultiVerDatabaseOper::Export(const std::string &filePath, const CipherPassword &passwd) const
{
    if (multiVerNaturalStore_ == nullptr || multiVerData_ == nullptr || commitHistory_ == nullptr ||
        multiVerKvStorage_ == nullptr) {
        return -E_INVALID_DB;
    }
    return ExecuteExport(filePath, passwd, multiVerNaturalStore_->GetDbProperties());
}

bool MultiVerDatabaseOper::RekeyPreHandle(const CipherPassword &passwd, int &errCode)
{
    CipherType cipherType;
    CipherPassword cachePasswd;
    multiVerNaturalStore_->GetDbProperties().GetPassword(cipherType, cachePasswd);

    if (cachePasswd.GetSize() == 0 && passwd.GetSize() == 0) {
        errCode = E_OK;
        return false;
    }

    return true;
}

int MultiVerDatabaseOper::BackupDb(const CipherPassword &passwd) const
{
    std::string filePrefix;
    int errCode = GetCtrlFilePrefix(multiVerNaturalStore_->GetDbProperties(), filePrefix);
    if (errCode != E_OK) {
        return errCode;
    }

    // create backup dir
    std::string currentDir = filePrefix;
    std::string backupDir = filePrefix + DBConstant::PATH_BACKUP_POSTFIX;

    // export db to backup
    return ExportAllDatabases(currentDir, passwd, backupDir);
}

int MultiVerDatabaseOper::CloseStorages()
{
    if (commitHistory_ != nullptr) {
        commitHistory_->Close();
    }
    if (multiVerData_ != nullptr) {
        multiVerData_->Close();
    }
    if (multiVerKvStorage_ != nullptr) {
        multiVerKvStorage_->Close();
    }

    // rm old dir -> rename backup dir to prime dir -> rm ctrl file
    int errCode = RekeyRecover(multiVerNaturalStore_->GetDbProperties());
    if (errCode != E_OK) {
        LOGE("Recover failed after run all export ok: %d.", errCode);
    }
    return errCode;
}

int MultiVerDatabaseOper::RekeyPostHandle(const CipherPassword &passwd)
{
    CipherType cipherType;
    CipherPassword oldPasswd;
    multiVerNaturalStore_->GetDbPropertyForUpdate().GetPassword(cipherType, oldPasswd);
    multiVerNaturalStore_->GetDbPropertyForUpdate().SetPassword(cipherType, passwd);

    int errCode = multiVerNaturalStore_->InitStorages(multiVerNaturalStore_->GetDbProperties());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = RekeyRecover(multiVerNaturalStore_->GetDbProperties());
    return E_OK;
}

int MultiVerDatabaseOper::ExportAllDatabases(const std::string &currentDir, const CipherPassword &passwd,
    const std::string &dbDir) const
{
    int errCode = DBCommon::CreateDirectory(dbDir);
    if (errCode != E_OK) {
        return errCode;
    }
    CipherType cipherType;
    CipherPassword oldPasswd;
    multiVerNaturalStore_->GetDbPropertyForUpdate().GetPassword(cipherType, oldPasswd);
    errCode = static_cast<SQLiteMultiVerDataStorage *>(multiVerData_)->RunExportLogic(cipherType, passwd, dbDir);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = static_cast<MultiVerNaturalStoreCommitStorage *>(commitHistory_)->RunExportLogic(cipherType,
        passwd, dbDir);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = multiVerKvStorage_->RunExportLogic(cipherType, passwd, dbDir);
    if (errCode != E_OK) {
        return errCode;
    }

    std::string versionFile = currentDir + "/version";
    if (OS::CheckPathExistence(versionFile)) {
        std::string targetVerFile = dbDir + "/version";
        DBCommon::CopyFile(versionFile, targetVerFile);
    }

    return E_OK;
}

int MultiVerDatabaseOper::BackupCurrentDatabase(const ImportFileInfo &info) const
{
    if (multiVerKvStorage_ == nullptr || commitHistory_ == nullptr || multiVerData_ == nullptr) {
        return -E_INVALID_DB;
    }
    commitHistory_->Close();
    multiVerData_->Close();
    multiVerKvStorage_->Close();

    // Create the file which imply that the current database files is valid.
    int errCode = OS::CreateFileByFileName(info.curValidFile);
    if (errCode != E_OK) {
        LOGE("Create current valid file failed:%d.", errCode);
        return errCode;
    }

    std::string dataDir = multiVerNaturalStore_->GetDbProperties().GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string id = multiVerNaturalStore_->GetDbProperties().GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    bool isNeedCreate = multiVerNaturalStore_->GetDbProperties().GetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);

    CipherType cipherType;
    CipherPassword passwd;
    multiVerNaturalStore_->GetDbProperties().GetPassword(cipherType, passwd);

    IKvDBMultiVerDataStorage::Property multiVerProp = {dataDir, id, isNeedCreate, cipherType, passwd};
    IKvDBCommitStorage::Property commitProp = {dataDir, id, isNeedCreate, cipherType, passwd};
    MultiVerKvDataStorage::Property multiVerKvProp = {dataDir, id, isNeedCreate, cipherType, passwd};

    errCode = DBCommon::CreateDirectory(info.backupDir);
    if (errCode != E_OK) {
        LOGE("Create backup dir failed");
        RemoveFile(info.curValidFile);
        return errCode;
    }

    errCode = multiVerData_->BackupCurrentDatabase(multiVerProp, info.backupDir);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = commitHistory_->BackupCurrentDatabase(commitProp, info.backupDir);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = multiVerKvStorage_->BackupCurrentDatabase(multiVerKvProp, info.backupDir);
    if (errCode != E_OK) {
        return errCode;
    }

    (void)DBCommon::CopyFile(info.currentDir + "/version", info.backupDir + "/version");
    int innerCode = rename(info.curValidFile.c_str(), info.backValidFile.c_str());
    if (innerCode != 0) {
        LOGE("Failed to rename the file after the backup:%d", errno);
        return -E_SYSTEM_API_FAIL;
    }
    return E_OK;
}

int MultiVerDatabaseOper::ImportUnpackedDatabase(const ImportFileInfo &info, const CipherPassword &srcPasswd) const
{
    // create backup dir
    int errCode = DBCommon::RemoveAllFilesOfDirectory(info.currentDir, false);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = ImportDatabase(info.unpackedDir, srcPasswd);
    DBCommon::CopyFile(info.unpackedDir + "/version", info.currentDir + "/version");
    (void)DBCommon::RemoveAllFilesOfDirectory(info.unpackedDir);
    if (errCode != E_OK) {
        LOGE("export the unpacked database to current error:%d", errCode);
        errCode = -E_INVALID_FILE;
        return errCode;
    }

    // reinitialize the database, and delete the backup database.
    errCode = multiVerNaturalStore_->InitStorages(multiVerNaturalStore_->GetDbProperties(), true);
    if (errCode != E_OK) {
        LOGE("InitStorages error:%d", errCode);
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

int MultiVerDatabaseOper::ImportPostHandle() const
{
    return multiVerNaturalStore_->InitStorages(multiVerNaturalStore_->GetDbProperties());
}

// private
int MultiVerDatabaseOper::ImportDatabase(const std::string &dir, const CipherPassword &passwd) const
{
    if (multiVerKvStorage_ == nullptr || commitHistory_ == nullptr || multiVerData_ == nullptr) {
        return -E_INVALID_DB;
    }

    std::string dataDir = multiVerNaturalStore_->GetDbProperties().GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string id = multiVerNaturalStore_->GetDbProperties().GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");

    CipherType cipherType;
    CipherPassword currPasswd;
    multiVerNaturalStore_->GetDbProperties().GetPassword(cipherType, currPasswd);

    IKvDBMultiVerDataStorage::Property multiVerProp = {dataDir, id, true, cipherType, currPasswd};
    IKvDBCommitStorage::Property commitProp = {dataDir, id, true, cipherType, currPasswd};
    MultiVerKvDataStorage::Property multiVerKvProp = {dataDir, id, true, cipherType, currPasswd};
    int errCode = multiVerData_->ImportDatabase(multiVerProp, dir, passwd);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = commitHistory_->ImportDatabase(commitProp, dir, passwd);
    if (errCode != E_OK) {
        return errCode;
    }
    return multiVerKvStorage_->ImportDatabase(multiVerKvProp, dir, passwd);
}
} // namespace DistributedDB
#endif