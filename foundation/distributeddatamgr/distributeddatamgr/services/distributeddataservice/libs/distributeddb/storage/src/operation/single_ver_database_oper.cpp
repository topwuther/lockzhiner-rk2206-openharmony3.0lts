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

#include "single_ver_database_oper.h"

#include "db_errno.h"
#include "log_print.h"
#include "db_constant.h"
#include "db_common.h"
#include "platform_specific.h"

namespace DistributedDB {
SingleVerDatabaseOper::SingleVerDatabaseOper(SQLiteSingleVerNaturalStore *naturalStore,
    SQLiteStorageEngine *storageEngine)
    : singleVerNaturalStore_(naturalStore),
      storageEngine_(storageEngine)
{}

int SingleVerDatabaseOper::SetSecOpt(const std::string &path, bool isDir) const
{
    std::string currentMetaPath = path + "/" + DBConstant::METADB_DIR;
    std::string currentMainPath = path + "/" + DBConstant::MAINDB_DIR;
    if (!isDir) {
        currentMetaPath = currentMetaPath + "/" + DBConstant::SINGLE_VER_META_STORE + DBConstant::SQLITE_DB_EXTENSION;
        currentMainPath = currentMainPath + "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    }
    SecurityOption option;
    int mainSecLabel = singleVerNaturalStore_->GetDbProperties().GetSecLabel();
    option.securityLabel = ((mainSecLabel >= SecurityLabel::S2) ? SecurityLabel::S2 : mainSecLabel);
    int errCode = RuntimeContext::GetInstance()->SetSecurityOption(currentMetaPath, option);
    if (errCode != E_OK && errCode != -E_NOT_SUPPORT) {
        return errCode;
    }

    option.securityLabel = singleVerNaturalStore_->GetDbProperties().GetSecLabel();
    option.securityFlag = singleVerNaturalStore_->GetDbProperties().GetSecFlag();
    errCode = RuntimeContext::GetInstance()->SetSecurityOption(currentMainPath, option);
    if (errCode != E_OK && errCode != -E_NOT_SUPPORT) {
        return errCode;
    }
    return E_OK;
}

int SingleVerDatabaseOper::Rekey(const CipherPassword &passwd)
{
    if (singleVerNaturalStore_ == nullptr || storageEngine_ == nullptr) {
        return -E_INVALID_DB;
    }

    return ExecuteRekey(passwd, singleVerNaturalStore_->GetDbProperties());
}

int SingleVerDatabaseOper::Import(const std::string &filePath, const CipherPassword &passwd)
{
    if (singleVerNaturalStore_ == nullptr || storageEngine_ == nullptr) {
        return -E_INVALID_DB;
    }

    return ExecuteImport(filePath, passwd, singleVerNaturalStore_->GetDbProperties());
}

int SingleVerDatabaseOper::Export(const std::string &filePath, const CipherPassword &passwd) const
{
    if (singleVerNaturalStore_ == nullptr || storageEngine_ == nullptr) {
        return -E_INVALID_DB;
    }

    return ExecuteExport(filePath, passwd, singleVerNaturalStore_->GetDbProperties());
}

bool SingleVerDatabaseOper::RekeyPreHandle(const CipherPassword &passwd, int &errCode)
{
    if (singleVerNaturalStore_->GetDbProperties().GetBoolProp(KvDBProperties::MEMORY_MODE, false)) {
        errCode = -E_NOT_SUPPORT;
        return false;
    }

    CipherType cipherType;
    CipherPassword cachePasswd;
    singleVerNaturalStore_->GetDbProperties().GetPassword(cipherType, cachePasswd);

    if (cachePasswd.GetSize() == 0 && passwd.GetSize() == 0) {
        errCode = E_OK;
        return false;
    }

    // need invoke sqlite3 rekey
    if (cachePasswd.GetSize() > 0 && passwd.GetSize() > 0) {
        errCode = RunRekeyLogic(cipherType, passwd);
        return false;
    }

    return true;
}

int SingleVerDatabaseOper::BackupDb(const CipherPassword &passwd) const
{
    std::string filePrefix;
    int errCode = GetCtrlFilePrefix(singleVerNaturalStore_->GetDbProperties(), filePrefix);
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

    std::vector<std::string> dbDir {DBConstant::MAINDB_DIR, DBConstant::METADB_DIR, DBConstant::CACHEDB_DIR};
    for (const auto &item : dbDir) {
        if (DBCommon::CreateDirectory(backupDir + "/" + item) != E_OK) {
            return -E_SYSTEM_API_FAIL;
        }
    }

    errCode = SetSecOpt(backupDir, true);
    if (errCode != E_OK) {
        LOGE("Set backup dir secOption failed, errCode = [%d]", errCode);
        return errCode;
    }

    // export db to backup
    errCode = RunExportLogic(passwd, filePrefix);
    if (errCode != E_OK) {
        return errCode;
    }

    return SetSecOpt(backupDir, false); // set file SecOpt
}

int SingleVerDatabaseOper::CloseStorages()
{
    // close old db
    storageEngine_->Release();
    int errCode = RekeyRecover(singleVerNaturalStore_->GetDbProperties());
    if (errCode != E_OK) {
        LOGE("Recover failed after rekey ok:%d.", errCode);
        int innerCode = InitStorageEngine();
        if (innerCode != E_OK) {
            LOGE("ReInit the handlePool failed:%d", innerCode);
        }
    }
    return errCode;
}

int SingleVerDatabaseOper::RekeyPostHandle(const CipherPassword &passwd)
{
    CipherType cipherType;
    CipherPassword oldPasswd;
    singleVerNaturalStore_->GetDbPropertyForUpdate().GetPassword(cipherType, oldPasswd);
    singleVerNaturalStore_->GetDbPropertyForUpdate().SetPassword(cipherType, passwd);
    singleVerNaturalStore_->GetDbPropertyForUpdate().SetBoolProp(
        KvDBProperties::ENCRYPTED_MODE, (passwd.GetSize() == 0) ? false : true);

    return InitStorageEngine();
}

int SingleVerDatabaseOper::ExportMainDB(const std::string &currentDir, const CipherPassword &passwd,
    const std::string &dbDir) const
{
    std::string backupDbName = dbDir + DBConstant::MAINDB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    std::string currentDb = currentDir + "/" + DBConstant::MAINDB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE +
        DBConstant::SQLITE_DB_EXTENSION;

    CipherType cipherType;
    CipherPassword currPasswd;
    singleVerNaturalStore_->GetDbProperties().GetPassword(cipherType, currPasswd);
    LOGI("Begin the sqlite main database export!");
    int errCode = SQLiteUtils::ExportDatabase(currentDb, cipherType, currPasswd, backupDbName, passwd);
    if (errCode != E_OK) {
        LOGE("Export the database failed:%d", errCode);
    }

    return errCode;
}

int SingleVerDatabaseOper::ExportMetaDB(const std::string &currentDir, const CipherPassword &passwd,
    const std::string &dbDir) const
{
    std::string backupDbName = dbDir + DBConstant::METADB_DIR + "/" + DBConstant::SINGLE_VER_META_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    std::string currentDb = currentDir + "/" + DBConstant::METADB_DIR  + "/" + DBConstant::SINGLE_VER_META_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    if (!OS::CheckPathExistence(currentDb)) { // Is S2 label, can access
        LOGD("No metaDB, no need Export metaDB.");
        return E_OK;
    }

    // Set metaDB db passwd same as mainDB temp, may be not need
    LOGI("Begin the sqlite meta database export.");
    int errCode = SQLiteUtils::ExportDatabase(currentDb, CipherType::DEFAULT, CipherPassword(),
        backupDbName, CipherPassword());
    if (errCode != E_OK) {
        LOGE("Export the database failed:%d", errCode);
    }

    return errCode;
}

int SingleVerDatabaseOper::ExportAllDatabases(const std::string &currentDir, const CipherPassword &passwd,
    const std::string &dbDir) const
{
    int errCode = ExportMainDB(currentDir, passwd, dbDir);
    if (errCode != E_OK) {
        LOGE("Export MainDB fail, errCode = [%d]", errCode);
        return errCode;
    }

    errCode = ExportMetaDB(currentDir, passwd, dbDir);
    if (errCode != E_OK) {
        LOGE("Export MetaDB fail, errCode = [%d]", errCode);
        return errCode;
    }
    return errCode;
}

int SingleVerDatabaseOper::BackupDatabase(const ImportFileInfo &info) const
{
    std::string currentMainFile = info.currentDir + DBConstant::MAINDB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    std::string backupMainFile = info.backupDir + DBConstant::MAINDB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    int errCode = DBCommon::CopyFile(currentMainFile, backupMainFile);
    if (errCode != E_OK) {
        LOGE("Backup the current database error:%d", errCode);
        return errCode;
    }

    std::string currentMetaFile = info.currentDir + DBConstant::METADB_DIR + "/" + DBConstant::SINGLE_VER_META_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    if (OS::CheckPathExistence(currentMetaFile)) {
        std::string backupMetaFile = info.backupDir + DBConstant::METADB_DIR + "/" + DBConstant::SINGLE_VER_META_STORE +
            DBConstant::SQLITE_DB_EXTENSION;
        errCode = DBCommon::CopyFile(currentMetaFile, backupMetaFile);
        if (errCode != E_OK) {
            LOGE("Backup the current database error:%d", errCode);
            return errCode;
        }
    }
    return E_OK;
}

int SingleVerDatabaseOper::BackupCurrentDatabase(const ImportFileInfo &info) const
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
        return errCode;
    }

    std::vector<std::string> dbDir {DBConstant::MAINDB_DIR, DBConstant::METADB_DIR, DBConstant::CACHEDB_DIR};
    for (const auto &item : dbDir) {
        if (DBCommon::CreateDirectory(info.backupDir + "/" + item) != E_OK) {
            return -E_SYSTEM_API_FAIL;
        }
    }

    errCode = SetSecOpt(info.backupDir, true);
    if (errCode != E_OK) {
        LOGE("[singleVer][BackupCurrentDatabase]Set secOpt to dir fail, errCode = [%d]", errCode);
        return errCode;
    }

    errCode = BackupDatabase(info);
    if (errCode != E_OK) {
        LOGE("[SingleVerDatabaseOper][BackupCurrentDatabase] backup current database fail, errCode = [%d]", errCode);
        return errCode;
    }

    // Protect the loss of label information when the abnormal scene is restored
    errCode = SetSecOpt(info.backupDir, false);
    if (errCode != E_OK) {
        LOGE("[singleVer][BackupCurrentDatabase]Set secOpt to file fail, errCode = [%d]", errCode);
        return errCode;
    }

    // rename
    int innerCode = rename(info.curValidFile.c_str(), info.backValidFile.c_str());
    if (innerCode != 0) {
        LOGE("Failed to rename the file after the backup:%d", errno);
        errCode = -E_SYSTEM_API_FAIL;
    }
    return errCode;
}

int SingleVerDatabaseOper::ClearCurrentDatabase(const ImportFileInfo &info) const
{
    int errCode = DBCommon::RemoveAllFilesOfDirectory(info.currentDir, false);
    if (errCode != E_OK) {
        return errCode;
    }

    std::vector<std::string> dbExtensionVec { DBConstant::MAINDB_DIR, DBConstant::METADB_DIR, DBConstant::CACHEDB_DIR };
    for (const auto &item : dbExtensionVec) {
        if (DBCommon::CreateDirectory(info.currentDir + "/" + item) != E_OK) {
            return -E_SYSTEM_API_FAIL;
        }
    }
    return errCode;
}

int SingleVerDatabaseOper::ImportUnpackedMainDatabase(const ImportFileInfo &info,
    const CipherPassword &srcPasswd) const
{
    std::string unpackedMainFile = info.unpackedDir + DBConstant::MAINDB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    std::string currentMainFile = info.currentDir + DBConstant::MAINDB_DIR + "/" +
        DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    CipherType cipherType;
    CipherPassword passwd;
    singleVerNaturalStore_->GetDbProperties().GetPassword(cipherType, passwd);

    std::string unpackedOldMainFile = info.unpackedDir + "/" + DBConstant::SINGLE_VER_DATA_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    bool isMainDbExisted = OS::CheckPathExistence(unpackedMainFile);
    bool isOldMainDbExisted = OS::CheckPathExistence(unpackedOldMainFile); // version < 3, mainDb in singer_ver/
    if (isMainDbExisted && isOldMainDbExisted) {
        LOGE("Unpacked dir existed two diff version mainDb!");
        return -E_INVALID_FILE;
    }

    int errCode = E_OK;
    if (isMainDbExisted) {
        errCode = SQLiteUtils::ExportDatabase(unpackedMainFile, cipherType, srcPasswd, currentMainFile, passwd);
        if (errCode != E_OK) {
            LOGE("Export the unpacked main database to current error:%d", errCode);
            return -E_INVALID_FILE;
        }
    }

    if (isOldMainDbExisted) {
        errCode = SQLiteUtils::ExportDatabase(unpackedOldMainFile, cipherType, srcPasswd, currentMainFile, passwd);
        if (errCode != E_OK) {
            LOGE("Export the unpacked old version(<3) main database to current error:%d", errCode);
            return -E_INVALID_FILE;
        }
    }
    return errCode;
}

int SingleVerDatabaseOper::ImportUnpackedMetaDatabase(const ImportFileInfo &info) const
{
    LOGI("MetaDB existed, need import, no need upgrade!");
    std::string unpackedMetaFile = info.unpackedDir + DBConstant::METADB_DIR + "/" +
        DBConstant::SINGLE_VER_META_STORE + DBConstant::SQLITE_DB_EXTENSION;
    std::string currentMetaFile = info.currentDir + DBConstant::METADB_DIR + "/" +
        DBConstant::SINGLE_VER_META_STORE + DBConstant::SQLITE_DB_EXTENSION;
    int errCode = SQLiteUtils::ExportDatabase(unpackedMetaFile, CipherType::DEFAULT, CipherPassword(),
        currentMetaFile, CipherPassword());
    if (errCode != E_OK) {
        LOGE("export the unpacked meta database to current error:%d", errCode);
        errCode = -E_INVALID_FILE;
    }
    return errCode;
}

int SingleVerDatabaseOper::ImportUnpackedDatabase(const ImportFileInfo &info, const CipherPassword &srcPasswd) const
{
    std::string unpackedMetaFile = info.unpackedDir + DBConstant::METADB_DIR + "/" +
        DBConstant::SINGLE_VER_META_STORE + DBConstant::SQLITE_DB_EXTENSION;
    bool metaDbExisted = OS::CheckPathExistence(unpackedMetaFile);
    int errCode = ClearCurrentDatabase(info);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = ImportUnpackedMainDatabase(info, srcPasswd);
    if (errCode != E_OK) {
        LOGE("import unpacked mainDb fail, errCode = [%d]", errCode);
        return errCode;
    }

    if (metaDbExisted) { // Is S2 label, no need deal
        errCode = ImportUnpackedMetaDatabase(info);
        if (errCode != E_OK) {
            LOGE("import unpacked metaDb fail, errCode = [%d]", errCode);
            return errCode;
        }
    }

    (void)SetSecOpt(info.currentDir, false); // not care err, Make sure to set the label

    // reinitialize the database, and delete the backup database.
    errCode = singleVerNaturalStore_->InitDatabaseContext(singleVerNaturalStore_->GetDbProperties(), true);
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

int SingleVerDatabaseOper::ImportPostHandle() const
{
    return singleVerNaturalStore_->InitDatabaseContext(singleVerNaturalStore_->GetDbProperties(), true);
}

// private begin
int SingleVerDatabaseOper::RunExportLogic(const CipherPassword &passwd, const std::string &filePrefix) const
{
    std::string currentMainDb = filePrefix + "/" + DBConstant::MAINDB_DIR + "/" +
        DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    CipherType cipherType;
    CipherPassword currPasswd;
    singleVerNaturalStore_->GetDbProperties().GetPassword(cipherType, currPasswd);

    // get backup db name
    std::string backupMainDbName = filePrefix + DBConstant::PATH_BACKUP_POSTFIX + "/" + DBConstant::MAINDB_DIR + "/" +
        DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;

    int errCode = SQLiteUtils::ExportDatabase(currentMainDb, cipherType, currPasswd, backupMainDbName, passwd);
    if (errCode != E_OK) {
        LOGE("single ver database export mainDb fail, errCode = [%d]", errCode);
        return errCode;
    }

    std::string currentMetaDb = filePrefix + "/" + DBConstant::METADB_DIR + "/" +
        DBConstant::SINGLE_VER_META_STORE + DBConstant::SQLITE_DB_EXTENSION;
    if (!OS::CheckPathExistence(currentMetaDb)) {
        LOGD("No metaDB, no need Export metaDB.");
        return E_OK;
    }

    LOGI("Begin export metaDB to back up!");
    std::string backupMetaDbName = filePrefix + DBConstant::PATH_BACKUP_POSTFIX + "/" + DBConstant::METADB_DIR + "/" +
        DBConstant::SINGLE_VER_META_STORE + DBConstant::SQLITE_DB_EXTENSION;
    // Set metaDB db passwd same as mainDB temp, may be not need
    errCode = SQLiteUtils::ExportDatabase(currentMetaDb, CipherType::DEFAULT, CipherPassword(),
        backupMetaDbName, CipherPassword());
    if (errCode != E_OK) {
        LOGE("single ver database export metaDb fail, errCode = [%d]", errCode);
        return errCode;
    }
    return errCode;
}

int SingleVerDatabaseOper::InitStorageEngine()
{
    OpenDbProperties option;
    InitDataBaseOption(option);
    bool isMemoryMode = singleVerNaturalStore_->GetDbProperties().GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    // Use 1 read handle to check passwd
    StorageEngineAttr poolSize = {0, 1, 1, 16}; // at most 1 write 16 read.
    if (isMemoryMode) {
        poolSize.minWriteNum = 1; // keep at least one connection.
    }

    std::string identify = singleVerNaturalStore_->GetDbProperties().GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    int errCode = storageEngine_->InitSQLiteStorageEngine(poolSize, option, identify);
    if (errCode != E_OK) {
        LOGE("[SingleVerOper]Init the sqlite storage engine failed:%d", errCode);
    }
    return errCode;
}

void SingleVerDatabaseOper::InitDataBaseOption(OpenDbProperties &option) const
{
    const KvDBProperties properties = singleVerNaturalStore_->GetDbProperties();
    const std::string dataDir = properties.GetStringProp(KvDBProperties::DATA_DIR, "");
    const std::string identifierDir = properties.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    std::string uri = dataDir + "/" + identifierDir + "/" + DBConstant::SINGLE_SUB_DIR + "/" +
        DBConstant::MAINDB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    bool isMemoryDb  = properties.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    if (isMemoryDb) {
        uri = identifierDir + DBConstant::SQLITE_MEMDB_IDENTIFY;
        LOGD("Begin create memory natural store database");
    }

    std::vector<std::string> createTableSqls;
    CipherType cipherType;
    CipherPassword passwd;
    properties.GetPassword(cipherType, passwd);
    bool isCreate = properties.GetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);

    SecurityOption securityOpt;
    securityOpt.securityLabel = properties.GetSecLabel();
    securityOpt.securityFlag = properties.GetSecFlag();

    option = {uri, isCreate, isMemoryDb, createTableSqls, cipherType, passwd};
    std::string dirPath = dataDir + "/" + identifierDir + "/" + DBConstant::SINGLE_SUB_DIR;
    option.subdir = dirPath;
    option.securityOpt = securityOpt;
    option.conflictReslovePolicy = properties.GetIntProp(KvDBProperties::CONFLICT_RESOLVE_POLICY, 0);
}

int SingleVerDatabaseOper::RunRekeyLogic(CipherType type, const CipherPassword &passwd)
{
    OpenDbProperties option;
    InitDataBaseOption(option);
    option.createIfNecessary = true;
    option.cipherType = type;
    sqlite3 *db = nullptr;

    // open one temporary connection.
    int errCode = SQLiteUtils::OpenDatabase(option, db);
    if (errCode != E_OK) {
        LOGE("[RunRekeyLogic] Open database new connect fail!, errCode = [%d]", errCode);
        goto END;
    }

    errCode = SQLiteUtils::Rekey(db, passwd);
    if (errCode != E_OK) {
        LOGE("[RunRekeyLogic] Rekey fail!, errCode = [%d]", errCode);
        goto END;
    }

    // Release all the connections, update the passwd and re-initialize the storage engine.
    storageEngine_->Release();
    singleVerNaturalStore_->GetDbPropertyForUpdate().SetPassword(type, passwd);
    errCode = InitStorageEngine();
    if (errCode != E_OK) {
        LOGE("Init storage engine while rekey open failed:%d", errCode);
    }

    // Rekey while locked before init storage engine, it can not open file, but rekey successfully
    if (storageEngine_->GetEngineState() != EngineState::MAINDB && errCode == -E_EKEYREVOKED) {
        LOGI("Rekey successfully, locked state init state successfully, need ignore open file failed!");
        errCode = -E_FORBID_CACHEDB;
    }

END:
    if (db != nullptr) {
        (void)sqlite3_close_v2(db);
        db = nullptr;
    }
    return errCode;
}
} // namespace DistributedDB
