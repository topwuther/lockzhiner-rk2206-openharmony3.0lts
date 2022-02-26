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
#include "multi_ver_kvdata_storage.h"

#include "db_constant.h"
#include "db_errno.h"
#include "log_print.h"
#include "ikvdb_factory.h"
#include "sqlite_local_kvdb.h"
#include "parcel.h"

namespace DistributedDB {
namespace {
    const uint8_t HASH_COUNT_MAGIC = '$';
    const uint32_t EXPECT_ENTRIES_NUM = 2;
    struct DatabaseIdentifierCfg {
        std::string databaseDir;
        std::string identifier;
        std::string fileName;
    };
}

static IKvDB *OpenKvDB(const DatabaseIdentifierCfg &config, CipherType type, const CipherPassword &passwd, int &errCode)
{
    IKvDBFactory *factory = IKvDBFactory::GetCurrent();
    if (factory == nullptr) {
        LOGE("Failed to open IKvDB! Get factory failed.");
        return nullptr;
    }

    IKvDB *kvDB = factory->CreateKvDb(LOCAL_KVDB, errCode);
    if (kvDB == nullptr) {
        LOGE("Create local kvdb failed:%d", errCode);
        return nullptr;
    }

    KvDBProperties dbProperties;
    dbProperties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    dbProperties.SetStringProp(KvDBProperties::DATA_DIR, config.databaseDir);
    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, config.fileName);
    dbProperties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, config.identifier);
    dbProperties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
    dbProperties.SetPassword(type, passwd);

    errCode = kvDB->Open(dbProperties);
    if (errCode != E_OK) {
        LOGE("Failed to open IKvDB! err:%d", errCode);
        RefObject::KillAndDecObjRef(kvDB);
        kvDB = nullptr;
        return nullptr;
    }
    // Need to refactor in the future
    int version = ((config.fileName == DBConstant::MULTI_VER_VALUE_STORE) ?
        MULTI_VER_VALUESLICE_STORAGE_VERSION_CURRENT : MULTI_VER_METADATA_STORAGE_VERSION_CURRENT);
    errCode = static_cast<SQLiteLocalKvDB *>(kvDB)->SetVersion(dbProperties, version);
    if (errCode != E_OK) {
        LOGE("[KvStorage][OpenDB] SetVersion fail, errCode=%d.", errCode);
        RefObject::KillAndDecObjRef(kvDB);
        kvDB = nullptr;
        return nullptr;
    }

    return kvDB;
}

static int PutData(IKvDBConnection *kvDBConnection, const Key &key, const Value &value)
{
    if (kvDBConnection == nullptr) {
        return -E_INVALID_DB;
    }

    if (key.empty() || key.size() > DBConstant::MAX_KEY_SIZE || value.size() > DBConstant::MAX_VALUE_SIZE) {
        return -E_INVALID_ARGS;
    }

    IOption option;
    int errCode = kvDBConnection->Put(option, key, value);
    if (errCode != E_OK) {
        LOGE("put data failed:%d", errCode);
    }

    return errCode;
}

static int DeleteData(IKvDBConnection *kvDBConnection, const Key &key)
{
    if (kvDBConnection == nullptr) {
        return -E_INVALID_DB;
    }

    if (key.empty() || key.size() > DBConstant::MAX_KEY_SIZE) {
        return -E_INVALID_ARGS;
    }

    IOption option;
    int errCode = kvDBConnection->Delete(option, key);
    if (errCode != E_OK) {
        LOGE("Delete data failed:%d", errCode);
    }

    return errCode;
}

static int GetData(const IKvDBConnection *kvDBConnection, const Key &key, Value &value)
{
    if (kvDBConnection == nullptr) {
        return -E_INVALID_DB;
    }

    if (key.empty() || key.size() > DBConstant::MAX_KEY_SIZE) {
        return -E_INVALID_ARGS;
    }

    IOption option;
    int errCode = kvDBConnection->Get(option, key, value);
    if (errCode != E_OK && errCode != -E_NOT_FOUND) {
        LOGE("Get data failed:%d", errCode);
    }

    return errCode;
}

static int GetEntries(const IKvDBConnection *kvDBConnection, const Key &keyPrefix, std::vector<Entry> &entries)
{
    if (kvDBConnection == nullptr) {
        return -E_INVALID_DB;
    }

    if (keyPrefix.empty() || keyPrefix.size() > DBConstant::MAX_KEY_SIZE) {
        return -E_INVALID_ARGS;
    }

    IOption option;
    int errCode = kvDBConnection->GetEntries(option, keyPrefix, entries);
    if (errCode != E_OK && errCode != -E_NOT_FOUND) {
        LOGE("Get entries failed:%d", errCode);
    }

    return errCode;
}

static int GetSliceCount(std::vector<Entry> &&entries, uint32_t &count)
{
    std::vector<uint8_t> buffer = (entries[0].key.size() > entries[1].key.size()) ?
        std::move(entries[0].value) : std::move(entries[1].value);
    Parcel parcel(buffer.data(), buffer.size());
    uint32_t size = parcel.ReadUInt32(count);
    if (size != sizeof(count) || parcel.IsError()) {
        LOGE("Get slice count size:%u", size);
        return -E_PARSE_FAIL;
    }
    return E_OK;
}

static int PutSliceCount(IKvDBConnection *kvDBConnection, const Key &sliceKey, uint32_t count)
{
    Key countKey(sliceKey);
    countKey.push_back(HASH_COUNT_MAGIC);
    std::vector<uint8_t> buffer(sizeof(uint32_t), 0);
    Parcel parcel(buffer.data(), buffer.size());
    int errCode = parcel.WriteUInt32(count);
    if (errCode != E_OK) {
        return errCode;
    }
    IOption option;
    errCode = kvDBConnection->Put(option, countKey, buffer);
    if (errCode != E_OK) {
        LOGE("Put slice count failed:%d", errCode);
    }
    return errCode;
}

static int PutSlice(IKvDBConnection *kvDBConnection, const Key &key, const Value &value, bool isAddCount)
{
    std::vector<Entry> entries;
    int errCode = GetEntries(kvDBConnection, key, entries);
    uint32_t dataCount = 1;
    switch (errCode) {
        case E_OK:
            if (entries.size() != EXPECT_ENTRIES_NUM) {
                return -E_INCORRECT_DATA;
            }
            errCode = GetSliceCount(std::move(entries), dataCount);
            if (errCode != E_OK) {
                return errCode;
            }
            dataCount++;
            errCode = PutSliceCount(kvDBConnection, key, dataCount);
            return errCode;
        case -E_NOT_FOUND:
            errCode = PutData(kvDBConnection, key, value);
            if (errCode != E_OK) {
                return errCode;
            }
            dataCount = isAddCount ? 1 : 0;
            errCode = PutSliceCount(kvDBConnection, key, dataCount);
            return errCode;
        default:
            return errCode;
    }
}

static int DeleteSlice(IKvDBConnection *kvDBConnection, const Key &key)
{
    std::vector<Entry> entries;
    int errCode = GetEntries(kvDBConnection, key, entries);
    if (errCode != E_OK) {
        return errCode;
    }
    if (entries.size() != EXPECT_ENTRIES_NUM) {
        return -E_INCORRECT_DATA;
    }
    uint32_t dataCount = 0;
    Key countKey(key);
    errCode = GetSliceCount(std::move(entries), dataCount);
    if (errCode != E_OK) {
        return errCode;
    }
    if (dataCount > 1) {
        dataCount--;
        errCode = PutSliceCount(kvDBConnection, key, dataCount);
        return errCode;
    } else {
        errCode = DeleteData(kvDBConnection, key);
        if (errCode != E_OK) {
            return errCode;
        }
        countKey.push_back(HASH_COUNT_MAGIC);
        errCode = DeleteData(kvDBConnection, countKey);
        return errCode;
    }
}

MultiVerKvDataStorage::MultiVerKvDataStorage()
    : kvStorage_(nullptr),
      metaStorage_(nullptr),
      kvStorageConnection_(nullptr),
      metaStorageConnection_(nullptr)
{}

MultiVerKvDataStorage::~MultiVerKvDataStorage()
{
    Close();
}

int MultiVerKvDataStorage::CheckVersion(const Property &property, bool &isDbAllExist) const
{
    int metaDataVer = 0;
    int valueSliceVer = 0;
    bool isMetaDbExist = false;
    bool isSliceDbExist = false;
    int errCode = GetVersion(property, metaDataVer, isMetaDbExist, valueSliceVer, isSliceDbExist);
    if (errCode != E_OK) {
        LOGE("[KvStorage][CheckVer] GetVersion failed, errCode=%d.", errCode);
        return errCode;
    }
    if (isMetaDbExist != isSliceDbExist) {
        // In case failure happens during open progress, some dbFile will not exist, we should recover from this
        LOGW("[KvStorage][CheckVer] Detect File Lost, isMetaDbExist=%d, isSliceDbExist=%d.",
            isMetaDbExist, isSliceDbExist);
    }
    isDbAllExist = isMetaDbExist && isSliceDbExist;
    if (!isMetaDbExist && !isSliceDbExist) {
        // If both dbFile not exist, just return.
        return E_OK;
    }
    LOGD("[KvStorage][CheckVer] MetaDbVer=%d, CurMetaVer=%d, SliceDbVer=%d, CurSliceVer=%d.", metaDataVer,
        MULTI_VER_METADATA_STORAGE_VERSION_CURRENT, valueSliceVer, MULTI_VER_VALUESLICE_STORAGE_VERSION_CURRENT);
    // For the dbFile not exist, version value will be 0, do not affect version check below
    if (metaDataVer > MULTI_VER_METADATA_STORAGE_VERSION_CURRENT ||
        valueSliceVer > MULTI_VER_VALUESLICE_STORAGE_VERSION_CURRENT) {
        LOGE("[KvStorage][CheckVer] Version Not Support!");
        return -E_VERSION_NOT_SUPPORT;
    }
    return E_OK;
}

int MultiVerKvDataStorage::GetVersion(const Property &property, int &metaVer, bool &isMetaDbExist,
    int &sliceVer, bool &isSliceDbExist) const
{
    SQLiteLocalKvDB *localKvdb = new (std::nothrow) SQLiteLocalKvDB();
    if (localKvdb == nullptr) {
        return -E_INVALID_DB;
    }

    KvDBProperties dbProperties;
    dbProperties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, property.isNeedCreate);
    dbProperties.SetStringProp(KvDBProperties::DATA_DIR, property.dataDir);
    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_VALUE_STORE);
    dbProperties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, property.identifierName);
    dbProperties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
    dbProperties.SetPassword(property.cipherType, property.passwd);
    int errCode = localKvdb->GetVersion(dbProperties, sliceVer, isSliceDbExist);
    if (errCode != E_OK) {
        LOGE("[KvStorage][GetVer] Get valueSlice storage version fail, errCode=%d.", errCode);
        RefObject::DecObjRef(localKvdb);
        localKvdb = nullptr;
        return errCode;
    }

    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_META_STORE);
    errCode = localKvdb->GetVersion(dbProperties, metaVer, isMetaDbExist);
    if (errCode != E_OK) {
        LOGE("[KvStorage][GetVer] Get metaData storage version fail, errCode=%d.", errCode);
        RefObject::DecObjRef(localKvdb);
        localKvdb = nullptr;
        return errCode;
    }

    RefObject::DecObjRef(localKvdb);
    localKvdb = nullptr;
    return E_OK;
}

int MultiVerKvDataStorage::Open(const Property &property)
{
    int errCode = E_OK;
    if (kvStorage_ == nullptr) {
        DatabaseIdentifierCfg config = {property.dataDir, property.identifierName, DBConstant::MULTI_VER_VALUE_STORE};
        kvStorage_ = OpenKvDB(config, property.cipherType, property.passwd, errCode);
        if (kvStorage_ == nullptr) {
            LOGE("open kv storage failed");
            goto END;
        }
    }

    if (metaStorage_ == nullptr) {
        DatabaseIdentifierCfg config = {property.dataDir, property.identifierName, DBConstant::MULTI_VER_META_STORE};
        metaStorage_ = OpenKvDB(config, property.cipherType, property.passwd, errCode);
        if (metaStorage_ == nullptr) {
            LOGE("open meta storage failed");
            goto END;
        }
    }

    kvStorageConnection_ = kvStorage_->GetDBConnection(errCode);
    if (errCode != E_OK) {
        goto END;
    }

    metaStorageConnection_ = metaStorage_->GetDBConnection(errCode);
    if (errCode != E_OK) {
        goto END;
    }

END:
    if (errCode != E_OK) {
        Close();
    }
    return errCode;
}

void MultiVerKvDataStorage::Close()
{
    if (kvStorageConnection_ != nullptr) {
        kvStorageConnection_->Close();
        kvStorageConnection_ = nullptr;
    }

    if (metaStorageConnection_ != nullptr) {
        metaStorageConnection_->Close();
        metaStorageConnection_ = nullptr;
    }

    if (kvStorage_ != nullptr) {
        RefObject::KillAndDecObjRef(kvStorage_);
        kvStorage_ = nullptr;
    }

    if (metaStorage_ != nullptr) {
        RefObject::KillAndDecObjRef(metaStorage_);
        metaStorage_ = nullptr;
    }
}

int MultiVerKvDataStorage::PutMetaData(const Key &key, const Value &value)
{
    return PutData(metaStorageConnection_, key, value);
}

int MultiVerKvDataStorage::GetMetaData(const Key &key, Value &value) const
{
    return GetData(metaStorageConnection_, key, value);
}

int MultiVerKvDataStorage::RunRekeyLogic(CipherType type, const CipherPassword &passwd)
{
    int errCode = static_cast<SQLiteLocalKvDB *>(kvStorage_)->RunRekeyLogic(type, passwd);
    if (errCode != E_OK) {
        LOGE("value storage rekey failed:%d", errCode);
        return errCode;
    }
    errCode = static_cast<SQLiteLocalKvDB *>(metaStorage_)->RunRekeyLogic(type, passwd);
    if (errCode != E_OK) {
        LOGE("meta storage rekey failed:%d", errCode);
        return errCode;
    }
    return E_OK;
}

int MultiVerKvDataStorage::RunExportLogic(CipherType type, const CipherPassword &passwd, const std::string &dbDir) const
{
    // execute export
    std::string valueDbName = dbDir + "/value_storage.db";
    int errCode = static_cast<SQLiteLocalKvDB *>(kvStorage_)->RunExportLogic(type, passwd, valueDbName);
    if (errCode != E_OK) {
        LOGE("value storage export failed:%d", errCode);
        return errCode;
    }

    std::string metaDbName = dbDir + "/meta_storage.db";
    errCode = static_cast<SQLiteLocalKvDB *>(metaStorage_)->RunExportLogic(type, passwd, metaDbName);
    if (errCode != E_OK) {
        LOGE("meta storage export failed:%d", errCode);
        return errCode;
    }
    return E_OK;
}

int MultiVerKvDataStorage::BackupCurrentDatabase(const Property &property, const std::string &dir)
{
    KvDBProperties dbProperties;
    dbProperties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    dbProperties.SetStringProp(KvDBProperties::DATA_DIR, property.dataDir);
    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_META_STORE);
    dbProperties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, property.identifierName);
    dbProperties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
    dbProperties.SetPassword(property.cipherType, property.passwd);
    int errCode = SQLiteLocalKvDB::BackupCurrentDatabase(dbProperties, dir);
    if (errCode != E_OK) {
        return errCode;
    }

    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_VALUE_STORE);
    return SQLiteLocalKvDB::BackupCurrentDatabase(dbProperties, dir);
}

int MultiVerKvDataStorage::ImportDatabase(const Property &property, const std::string &dir,
    const CipherPassword &passwd)
{
    KvDBProperties dbProperties;
    dbProperties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    dbProperties.SetStringProp(KvDBProperties::DATA_DIR, property.dataDir);
    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_META_STORE);
    dbProperties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, property.identifierName);
    dbProperties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
    dbProperties.SetPassword(property.cipherType, property.passwd);
    int errCode = SQLiteLocalKvDB::ImportDatabase(dbProperties, dir, passwd);
    if (errCode != E_OK) {
        return errCode;
    }

    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_VALUE_STORE);
    return SQLiteLocalKvDB::ImportDatabase(dbProperties, dir, passwd);
}

SliceTransaction *MultiVerKvDataStorage::GetSliceTransaction(bool isWrite, int &errCode)
{
    auto connect = kvStorage_->GetDBConnection(errCode);
    if (connect == nullptr) {
        return nullptr;
    }
    auto transaction = new (std::nothrow) SliceTransaction(isWrite, connect);
    if (transaction == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        connect->Close();
        return nullptr;
    }
    errCode = E_OK;
    return transaction;
}

void MultiVerKvDataStorage::ReleaseSliceTransaction(SliceTransaction *&transaction)
{
    if (transaction == nullptr) {
        return;
    }
    transaction->Close();
    delete transaction;
    transaction = nullptr;
    return;
}

SliceTransaction::SliceTransaction(bool isWrite, IKvDBConnection *connect)
    : isWrite_(isWrite),
      connect_(connect)
{}

SliceTransaction::~SliceTransaction()
{}

int SliceTransaction::Close()
{
    if (connect_ == nullptr) {
        return E_OK;
    }
    return connect_->Close();
}

int SliceTransaction::PutData(const Key &key, const Value &value, bool isAddCount)
{
    if (!isWrite_) {
        return -E_INVALID_CONNECTION;
    }
    return PutSlice(connect_, key, value, isAddCount);
}

int SliceTransaction::GetData(const Key &key, Value &value) const
{
    return ::DistributedDB::GetData(connect_, key, value);
}

int SliceTransaction::DeleteData(const Key &key)
{
    if (!isWrite_) {
        return -E_INVALID_CONNECTION;
    }
    return DeleteSlice(connect_, key);
}

int SliceTransaction::StartTransaction()
{
    if (connect_ == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return connect_->StartTransaction();
}

int SliceTransaction::CommitTransaction()
{
    if (connect_ == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return connect_->Commit();
}

int SliceTransaction::RollbackTransaction()
{
    if (connect_ == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return connect_->RollBack();
}
} // namespace DistributedDB
#endif