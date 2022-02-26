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

#include "database_oper.h"

#include "db_errno.h"
#include "db_constant.h"
#include "db_common.h"
#include "log_print.h"
#include "platform_specific.h"
#include "package_file.h"
#include "res_finalizer.h"
#include "runtime_context.h"

namespace DistributedDB {
void DatabaseOper::SetLocalDevId(const std::string &deviceId)
{
    deviceId_ = deviceId;
}

int DatabaseOper::ExecuteRekey(const CipherPassword &passwd, const KvDBProperties &property)
{
    int errCode = E_OK;
    if (!RekeyPreHandle(passwd, errCode)) {
        LOGE("Rekey fail when RekeyPre Handle, errCode = [%d]", errCode);
        return errCode;
    }

    std::string ctrlFileName;
    std::string newFileName;
    errCode = CreateStatusCtrlFile(property, ctrlFileName, newFileName);
    if (errCode != E_OK) {
        return errCode;
    }

    LOGI("Backup the current file while rekey.");
    errCode = BackupDb(passwd);
    if (errCode != E_OK) {
        LOGE("ExecuteRekey backup db failed! errCode = [%d]", errCode);
        (void)RekeyRecover(property);
        return errCode;
    }

    errCode = RenameStatusCtrlFile(ctrlFileName, newFileName);
    if (errCode != E_OK) {
        (void)RekeyRecover(property);
        LOGE("ExecuteRekey rename status ctrl failed! errCode = [%d]", errCode);
        return errCode;
    }

    errCode = CloseStorages();
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = RekeyPostHandle(passwd);
    if (errCode == -E_EKEYREVOKED) {
        errCode = -E_FORBID_CACHEDB;
        LOGI("Can not reopen database after rekey for the access controlled. errCode = [%d]", errCode);
    }
    return errCode;
}

int DatabaseOper::GetCtrlFilePrefix(const KvDBProperties &property, std::string &filePrefix) const
{
    std::string baseDir;
    int errCode = GetWorkDir(property, baseDir);
    if (errCode != E_OK) {
        return errCode;
    }

    int dbType = property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    std::string dbSubDir = KvDBProperties::GetStoreSubDirectory(dbType);
    filePrefix = baseDir + "/" + dbSubDir;
    return E_OK;
}

int DatabaseOper::RekeyRecover(const KvDBProperties &property)
{
    std::string workDir;
    int errCode = GetWorkDir(property, workDir);
    if (errCode != E_OK) {
        return errCode;
    }

    int dbType = property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    std::string dbSubDir = KvDBProperties::GetStoreSubDirectory(dbType);

    std::string preCtrlFileName = workDir + "/" + dbSubDir + DBConstant::REKEY_FILENAME_POSTFIX_PRE;
    bool isPreCtrlFileExist = OS::CheckPathExistence(preCtrlFileName);

    std::string endCtrlFileName = workDir + "/" + dbSubDir + DBConstant::REKEY_FILENAME_POSTFIX_OK;
    bool isEndCtrlFileExist = OS::CheckPathExistence(endCtrlFileName);

    std::string currentDir = workDir + "/" + dbSubDir;
    bool isPrimeDbDirExist = OS::CheckPathExistence(currentDir);

    std::string backupDir = workDir + "/" + dbSubDir + DBConstant::PATH_BACKUP_POSTFIX;
    bool isBackupDbDirExist = OS::CheckPathExistence(backupDir);

    // remove the backup directory and ctrl file if Rekey not finish
    // name of ctrl file is pre
    if (isPreCtrlFileExist) {
        LOGI("Rekey recovery:Remove the backup files");
        return RecoverPrehandle(dbType, backupDir, preCtrlFileName);
    }
    // no ctrl file means nothing need to do
    if (!isEndCtrlFileExist) {
        return E_OK;
    }

    // name of ctrl file is ok
    if (isBackupDbDirExist) {
        if (isPrimeDbDirExist) {
            // scenario 1: both prime and bak dir exist
            // rm prime dir -> rename backup dir to prime dir -> rm ctrl file
            LOGI("Rekey recovery:Remove the current files");
            if (DBCommon::RemoveAllFilesOfDirectory(currentDir, true) != E_OK) {
                LOGE("Remove the prime dir failed: %d", errno);
                return -E_REMOVE_FILE;
            }
        }

        // scenario 2: only bak dir exist
        // rename backup dir to prime dir -> rm ctrl file
        if (rename(backupDir.c_str(), currentDir.c_str()) != E_OK) {
            LOGE("Rename the bak dir to prime dir failed:%d.", errno);
            return -E_SYSTEM_API_FAIL;
        }
    }
    // scenario 3: only prime dir exist
    // scenario 4: both prime and bak dir not exist
    // remove ctrl file
    if (RemoveFile(endCtrlFileName) != E_OK) {
        LOGE("Remove the end ctrl file failed: %d", errno);
        return -E_REMOVE_FILE;
    }
    return E_OK;
}

int DatabaseOper::CheckSecurityOption(const std::string &filePath, const KvDBProperties &property) const
{
    SecurityOption secOption;
    int errCode = RuntimeContext::GetInstance()->GetSecurityOption(filePath, secOption);
    if (errCode != E_OK && errCode != -E_NOT_SUPPORT) {
        LOGE("Get import package security option fail! errCode = [%d]", errCode);
        return errCode;
    }

    SecurityOption dbSecOpt;
    dbSecOpt.securityFlag = property.GetSecFlag();
    dbSecOpt.securityLabel = property.GetSecLabel();

    if (dbSecOpt == secOption || secOption.securityLabel == SecurityLabel::NOT_SET) {
        return E_OK;
    }
    LOGE("Import package secOpt %d %d vs database %d %d",
        secOption.securityFlag, secOption.securityLabel, dbSecOpt.securityFlag, dbSecOpt.securityLabel);
    return -E_SECURITY_OPTION_CHECK_ERROR;
}

int DatabaseOper::ExecuteImport(const std::string &filePath, const CipherPassword &passwd,
    const KvDBProperties &property) const
{
    ImportFileInfo importInfo;
    InitImportFileInfo(importInfo, property);

    int errCode = CheckSecurityOption(filePath, property);
    if (errCode != E_OK) {
        return errCode;
    }

    // 1. unpack and check the file.
    LOGI("Unpack the imported file");
    errCode = UnpackAndCheckImportedFile(filePath, importInfo, property);
    if (errCode != E_OK) {
        return errCode;
    }

    // Using RAII define tempState clean when object finalize execute
    ResFinalizer tempStateClean([&errCode, &property, this]() {
        int innerCode = this->ClearImportTempFile(property);
        if (innerCode != E_OK) {
            LOGE("Failed to clean the intermediate import files, errCode = [%d]", innerCode);
        }
        // Finish. reinitialize the database.
        if (errCode != E_OK) {
            innerCode = this->ImportPostHandle();
            LOGE("Reinit the database after import, errCode = [%d]", innerCode);
        }
    });

    // 2. backup the current database.
    LOGI("Backup the current database while import.");
    errCode = BackupCurrentDatabase(importInfo);
    if (errCode != E_OK) {
        LOGE("Failed to backup current databases, errCode = [%d]", errCode);
        return errCode;
    }

    // 3. export the unpacked file to the current database.
    LOGI("Import the unpacked database.");
    errCode = ImportUnpackedDatabase(importInfo, passwd);
    if (errCode != E_OK) {
        LOGE("Failed to import from the unpacked databases, errCode = [%d]", errCode);
    }
    DBCommon::RemoveAllFilesOfDirectory(importInfo.unpackedDir);
    return errCode;
}

int DatabaseOper::CreateBackupDirForExport(const KvDBProperties &property, std::string &currentDir,
    std::string &backupDir) const
{
    std::string baseDir;
    int errCode = GetWorkDir(property, baseDir);
    if (errCode != E_OK) {
        LOGE("Get work dir failed:%d.", errCode);
        return errCode;
    }

    int databaseType = property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    std::string subDir = KvDBProperties::GetStoreSubDirectory(databaseType);
    currentDir = baseDir + "/" + subDir;

    backupDir = baseDir + "/" + subDir + DBConstant::PATH_POSTFIX_EXPORT_BACKUP + "/";
    errCode = DBCommon::CreateDirectory(backupDir);
    if (errCode != E_OK) {
        return errCode;
    }
    std::vector<std::string> dbDir {DBConstant::MAINDB_DIR, DBConstant::METADB_DIR, DBConstant::CACHEDB_DIR};
    for (const auto &item : dbDir) {
        if (DBCommon::CreateDirectory(backupDir + "/" + item) != E_OK) {
            return -E_SYSTEM_API_FAIL;
        }
    }
    return errCode;
}

int DatabaseOper::ExecuteExport(const std::string &filePath, const CipherPassword &passwd,
    const KvDBProperties &property) const
{
    if (deviceId_.empty()) {
        return -E_NOT_INIT;
    }

    std::string currentDir;
    std::string backupDir;
    int errCode = CreateBackupDirForExport(property, currentDir, backupDir);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = ExportAllDatabases(currentDir, passwd, backupDir);
    if (errCode != E_OK) {
        LOGE("Export databases fail!:%d.", errCode);
        (void)ClearExportedTempFiles(property);
        return errCode;
    }

    errCode = PackExportedDatabase(backupDir, filePath, property);
    if (errCode != E_OK) {
        OS::RemoveFile(filePath); // Pack file failed, need rollback delete Intermediate state package file
        LOGE("[DatabaseOper][ExecuteExport] Pack files fail! errCode = [%d], errno = [%d].", errCode, errno);
        (void)ClearExportedTempFiles(property);
        return errCode;
    }

    SecurityOption secOption {property.GetSecLabel(), property.GetSecFlag()};
    // RuntimeContext can make sure GetInstance not nullptr
    errCode = RuntimeContext::GetInstance()->SetSecurityOption(filePath, secOption);
    if (errCode != E_OK) {
        if (errCode == -E_NOT_SUPPORT) {
            (void)ClearExportedTempFiles(property);
            return E_OK;
        }
        OS::RemoveFile(filePath);
        LOGE("[DatabaseOper][ExecuteExport] Set security option fail! errCode = [%d].", errCode);
    }

    (void)ClearExportedTempFiles(property);
    return errCode;
}

// private begin
int DatabaseOper::CreateStatusCtrlFile(const KvDBProperties &property, std::string &orgCtrlFile,
    std::string &newCtrlFile)
{
    std::string filePrefix;
    int errCode = GetCtrlFilePrefix(property, filePrefix);
    if (errCode != E_OK) {
        return errCode;
    }

    // create control file
    newCtrlFile = filePrefix + DBConstant::REKEY_FILENAME_POSTFIX_OK;
    orgCtrlFile = filePrefix + DBConstant::REKEY_FILENAME_POSTFIX_PRE;
    return OS::CreateFileByFileName(orgCtrlFile);
}

int DatabaseOper::RenameStatusCtrlFile(const std::string &orgCtrlFile, const std::string &newCtrlFile)
{
    int errCode = rename(orgCtrlFile.c_str(), newCtrlFile.c_str());
    if (errCode != E_OK) {
        LOGE("change ctrl file name to ok failed: %d.", errCode);
        return -E_SYSTEM_API_FAIL;
    }
    return E_OK;
}

int DatabaseOper::RecoverPrehandle(int dbType, const std::string &dir, const std::string &fileName)
{
    if (DBCommon::RemoveAllFilesOfDirectory(dir, true) != E_OK) {
        LOGE("Remove the backup dir failed:%d", errno);
        return -E_REMOVE_FILE;
    }
    if (RemoveFile(fileName) != E_OK) {
        LOGE("Remove the pre ctrl file failed:%d", errno);
        return -E_REMOVE_FILE;
    }
    return E_OK;
}

int DatabaseOper::RemoveDbDir(const std::string &dir, int dbType, bool isNeedDelDir)
{
    if (!OS::CheckPathExistence(dir)) {
        return E_OK;
    }

    if (dbType == DBConstant::DB_TYPE_LOCAL) {
        std::vector<std::string> dbNameList = {
            DBConstant::LOCAL_DATABASE_NAME
        };
        return RemoveDbFiles(dir, dbNameList, isNeedDelDir);
    }
    if (dbType == DBConstant::DB_TYPE_SINGLE_VER) {
        std::vector<std::string> dbNameList = {
            DBConstant::SINGLE_VER_DATA_STORE
        };
        return RemoveDbFiles(dir, dbNameList, isNeedDelDir);
    }
    if (dbType == DBConstant::DB_TYPE_MULTI_VER) {
        std::vector<std::string> dbNameList = {
            DBConstant::MULTI_VER_DATA_STORE, DBConstant::MULTI_VER_COMMIT_STORE,
            DBConstant::MULTI_VER_VALUE_STORE, DBConstant::MULTI_VER_META_STORE
        };
        return RemoveDbFiles(dir, dbNameList, isNeedDelDir);
    }
    return -E_NOT_SUPPORT;
}

int DatabaseOper::RemoveFile(const std::string &fileName)
{
    if (!OS::CheckPathExistence(fileName)) {
        return E_OK;
    }

    if (remove(fileName.c_str()) != 0) {
        LOGE("Remove file failed:%d", errno);
        return -E_REMOVE_FILE;
    }
    return E_OK;
}

int DatabaseOper::GetWorkDir(const KvDBProperties &property, std::string &workDir)
{
    std::string dataDir = property.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifierDir = property.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    if (dataDir.empty()) {
        return -E_INVALID_ARGS;
    }

    workDir = dataDir + "/" + identifierDir;
    return E_OK;
}

// Only for remove the backup directory while rekey.
int DatabaseOper::RemoveDbFiles(const std::string &dir, const std::vector<std::string> &dbNameList, bool isNeedDelDir)
{
    for (const auto &iter : dbNameList) {
        // remove
        std::string dbFile = dir + "/" + iter + ".db";
        if (RemoveFile(dbFile) != E_OK) {
            LOGE("Remove the db file failed:%d", errno);
            return -E_REMOVE_FILE;
        }

        dbFile = dir + "/" + iter + ".db-wal";
        if (RemoveFile(dbFile) != E_OK) {
            LOGE("Remove the wal file failed:%d", errno);
            return -E_REMOVE_FILE;
        }

        dbFile = dir + "/" + iter + ".db-shm";
        if (RemoveFile(dbFile) != E_OK) {
            LOGE("Remove the shm file failed:%d", errno);
            return -E_REMOVE_FILE;
        }
    }
    if (isNeedDelDir && OS::RemoveDBDirectory(dir) != E_OK) {
        LOGE("Remove directory:%d", errno);
        return -E_REMOVE_FILE;
    }
    return E_OK;
}

void DatabaseOper::InitImportFileInfo(ImportFileInfo &info, const KvDBProperties &property)
{
    std::string dataDir = property.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifierDir = property.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    int databaseType = property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
    std::string subDir = KvDBProperties::GetStoreSubDirectory(databaseType);

    std::string baseDir = dataDir + "/" + identifierDir + "/" + subDir;
    info.backupDir = baseDir + DBConstant::PATH_POSTFIX_IMPORT_BACKUP + "/";
    info.unpackedDir = baseDir + DBConstant::PATH_POSTFIX_UNPACKED + "/";
    info.currentDir = baseDir + "/";
    info.curValidFile = baseDir + DBConstant::PATH_POSTFIX_IMPORT_ORIGIN; // origin directory is valid.
    info.backValidFile = baseDir + DBConstant::PATH_POSTFIX_IMPORT_DUP; // the back directory is valid.
}

int DatabaseOper::UnpackAndCheckImportedFile(const std::string &srcFile, const ImportFileInfo &info,
    const KvDBProperties &property) const
{
    int errCode = DBCommon::CreateDirectory(info.unpackedDir);
    if (errCode != E_OK) {
        return errCode;
    }

    FileInfo fileInfo;
    errCode = PackageFile::UnpackFile(srcFile, info.unpackedDir, fileInfo);
    if (errCode != E_OK) {
        DBCommon::RemoveAllFilesOfDirectory(info.unpackedDir);
        LOGE("Failed to unpack the imported file:%d", errCode);
        return errCode;
    }
    int dbType = property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    if (fileInfo.dbType != static_cast<uint32_t>(dbType) || fileInfo.deviceID != deviceId_) {
        DBCommon::RemoveAllFilesOfDirectory(info.unpackedDir);
        LOGE("Check db type [%u] vs [%u] or devicesId fail!", fileInfo.dbType, static_cast<uint32_t>(dbType));
        return -E_INVALID_FILE;
    }
    return E_OK;
}

int DatabaseOper::RecoverImportedBackFiles(const std::string &dir, const std::string &fileName, int dbType) const
{
    std::string backupDir = dir + DBConstant::PATH_POSTFIX_IMPORT_BACKUP;
    // if backup directory is not existed
    if (!OS::CheckPathExistence(backupDir)) {
        goto END;
    }

    if (DBCommon::RemoveAllFilesOfDirectory(dir, true) != E_OK) {
        LOGE("Remove the current db dir failed");
        return -E_REMOVE_FILE;
    }

    if (rename(backupDir.c_str(), dir.c_str()) != E_OK) {
        LOGE("Rename the backfile error:%d", errno);
        return -E_SYSTEM_API_FAIL;
    }

END:
    if (RemoveFile(fileName) != E_OK) {
        LOGE("Remove the pre ctrl file failed:%d", errno);
        return -E_REMOVE_FILE;
    }
    return E_OK;
}

int DatabaseOper::RemoveImportedBackFiles(const std::string &backupDir, const std::string &ctrlFileName, int dbType)
    const
{
    if (DBCommon::RemoveAllFilesOfDirectory(backupDir, true) != E_OK) {
        LOGE("Remove the backup dir failed");
        return -E_REMOVE_FILE;
    }

    if (RemoveFile(ctrlFileName) != E_OK) {
        LOGE("Remove the pre ctrl file failed");
        return -E_REMOVE_FILE;
    }
    return E_OK;
}

int DatabaseOper::ClearImportTempFile(const KvDBProperties &property) const
{
    // get work directory
    std::string workDir;
    int errCode = GetWorkDir(property, workDir);
    if (errCode != E_OK) {
        return errCode;
    }

    int dbType = property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    std::string dbSubDir = KvDBProperties::GetStoreSubDirectory(dbType);

    std::string oriKeepFile = workDir + "/" + dbSubDir + DBConstant::PATH_POSTFIX_IMPORT_ORIGIN;
    bool isOriKeepFileExist = OS::CheckPathExistence(oriKeepFile);

    std::string backKeepFile = workDir + "/" + dbSubDir + DBConstant::PATH_POSTFIX_IMPORT_DUP;
    bool isBakKeepFileExist = OS::CheckPathExistence(backKeepFile);

    std::string currentDir = workDir + "/" + dbSubDir;
    std::string backupDir = workDir + "/" + dbSubDir + DBConstant::PATH_POSTFIX_IMPORT_BACKUP;
    bool isBackupDbDirExist = OS::CheckPathExistence(backupDir);
    std::string exportBackupDir = workDir + "/" + dbSubDir + DBConstant::PATH_POSTFIX_UNPACKED;
    DBCommon::RemoveAllFilesOfDirectory(exportBackupDir);

    LOGI("Clear the files while import");
    if (isOriKeepFileExist && isBakKeepFileExist) {
        LOGE("Origin and backup file shouldn't exist concurrently");
    }

    // Clear the backup dir and the ctrl file
    if (isOriKeepFileExist) {
        return RemoveImportedBackFiles(backupDir, oriKeepFile, dbType);
    }

    // remove the main directory and restore the backup files.
    if (isBakKeepFileExist) {
        return RecoverImportedBackFiles(currentDir, backKeepFile, dbType);
    }

    if (isBackupDbDirExist) {
        // Import success, clean backupdir
        if (DBCommon::RemoveAllFilesOfDirectory(backupDir, true) != E_OK) {
            LOGE("Remove the backup dir failed");
            return -E_REMOVE_FILE;
        }
    }

    return E_OK;
}

int DatabaseOper::ClearExportedTempFiles(const KvDBProperties &property) const
{
    std::string workDir;
    int errCode = GetWorkDir(property, workDir);
    if (errCode != E_OK) {
        return errCode;
    }

    int dbType = property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    std::string dbSubDir = KvDBProperties::GetStoreSubDirectory(dbType);
    std::string backupDir = workDir + "/" + dbSubDir + DBConstant::PATH_POSTFIX_EXPORT_BACKUP;
    LOGI("Remove the exported files.");
    errCode = DBCommon::RemoveAllFilesOfDirectory(backupDir);
    if (errCode != E_OK) {
        LOGE("Remove the exported backup dir failed");
        return -E_REMOVE_FILE;
    }

    return errCode;
}

int DatabaseOper::PackExportedDatabase(const std::string &fileDir, const std::string &packedFile,
    const KvDBProperties &property) const
{
    LOGI("Pack the exported database.");
    int databaseType = property.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
    FileInfo fileInfo = {static_cast<uint32_t>(databaseType), deviceId_};
    int errCode = PackageFile::PackageFiles(fileDir, packedFile, fileInfo);
    if (errCode != E_OK) {
        LOGE("Pack the database error:%d", errCode);
    }

    return errCode;
}
} // namespace DistributedDB
