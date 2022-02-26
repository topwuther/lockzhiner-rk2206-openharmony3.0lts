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

#include "distributeddb_tools_unit_test.h"
#include "platform_specific.h"

#include <fstream>
#include <set>
#include <cstring>

#include <openssl/rand.h>
#include <sys/types.h>
#include <dirent.h>

#include "db_common.h"
#include "value_hash_calc.h"
#include "db_constant.h"
#include "generic_single_ver_kv_entry.h"

using namespace DistributedDB;

namespace DistributedDBUnitTest {
namespace {
    const std::string CREATE_LOCAL_TABLE_SQL =
        "CREATE TABLE IF NOT EXISTS local_data(" \
        "key BLOB PRIMARY KEY," \
        "value BLOB," \
        "timestamp INT," \
        "hash_key BLOB);";

    const std::string CREATE_META_TABLE_SQL =
        "CREATE TABLE IF NOT EXISTS meta_data("  \
        "key    BLOB PRIMARY KEY  NOT NULL," \
        "value  BLOB);";

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

    const std::string CREATE_SYNC_TABLE_INDEX_SQL =
        "CREATE INDEX IF NOT EXISTS key_index ON sync_data (key);";

    const std::string CREATE_TABLE_SQL =
        "CREATE TABLE IF NOT EXISTS version_data(key BLOB, value BLOB, oper_flag INTEGER, version INTEGER, " \
        "timestamp INTEGER, ori_timestamp INTEGER, hash_key BLOB, " \
        "PRIMARY key(hash_key, version));";

    const std::string CREATE_SQL =
        "CREATE TABLE IF NOT EXISTS data(key BLOB PRIMARY key, value BLOB);";

    bool CompareEntry(const DistributedDB::Entry &a, const DistributedDB::Entry &b)
    {
        return (a.key < b.key);
    }
}

// OpenDbProperties.uri do not need
int DistributedDBToolsUnitTest::CreateMockSingleDb(DatabaseInfo &dbInfo, OpenDbProperties &properties)
{
    std::string identifier = dbInfo.userId + "-" + dbInfo.appId + "-" + dbInfo.storeId;
    std::string hashIdentifier = DBCommon::TransferHashString(identifier);
    std::string identifierName = DBCommon::TransferStringToHex(hashIdentifier);

    if (OS::GetRealPath(dbInfo.dir, properties.uri) != E_OK) {
        LOGE("Failed to canonicalize the path.");
        return -E_INVALID_ARGS;
    }

    int errCode = DBCommon::CreateStoreDirectory(dbInfo.dir, identifierName, DBConstant::SINGLE_SUB_DIR, true);
    if (errCode != E_OK) {
        return errCode;
    }

    properties.uri = dbInfo.dir + "/" + identifierName + "/" +
        DBConstant::SINGLE_SUB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    if (properties.sqls.empty()) {
        std::vector<std::string> defaultCreateTableSqls = {
            CREATE_LOCAL_TABLE_SQL,
            CREATE_META_TABLE_SQL,
            CREATE_SYNC_TABLE_SQL,
            CREATE_SYNC_TABLE_INDEX_SQL
        };
        properties.sqls = defaultCreateTableSqls;
    }

    sqlite3 *db = nullptr;
    errCode = SQLiteUtils::OpenDatabase(properties, db);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = SQLiteUtils::SetUserVer(properties, dbInfo.dbUserVersion);
    if (errCode != E_OK) {
        return errCode;
    }

    (void)sqlite3_close_v2(db);
    db = nullptr;
    return errCode;
}

static int CreatMockMultiDb(OpenDbProperties &properties, DatabaseInfo &dbInfo)
{
    sqlite3 *db = nullptr;
    (void)SQLiteUtils::OpenDatabase(properties, db);
    int errCode = SQLiteUtils::SetUserVer(properties, dbInfo.dbUserVersion);
    (void)sqlite3_close_v2(db);
    db = nullptr;
    if (errCode != E_OK) {
        return errCode;
    }
    return errCode;
}

int DistributedDBToolsUnitTest::OpenMockMultiDb(DatabaseInfo &dbInfo, OpenDbProperties &properties)
{
    std::string identifier = dbInfo.userId + "-" + dbInfo.appId + "-" + dbInfo.storeId;
    std::string hashIdentifier = DBCommon::TransferHashString(identifier);
    std::string identifierName = DBCommon::TransferStringToHex(hashIdentifier);

    OpenDbProperties commitProperties = properties;
    commitProperties.uri = dbInfo.dir + "/" + identifierName + "/" + DBConstant::MULTI_SUB_DIR +
        "/commit_logs" + DBConstant::SQLITE_DB_EXTENSION;

    commitProperties.sqls = {CREATE_SQL};

    OpenDbProperties kvStorageProperties = commitProperties;
    kvStorageProperties.uri = dbInfo.dir + "/" + identifierName + "/" +
        DBConstant::MULTI_SUB_DIR + "/value_storage" + DBConstant::SQLITE_DB_EXTENSION;
    OpenDbProperties metaStorageProperties = commitProperties;
    metaStorageProperties.uri = dbInfo.dir + "/" + identifierName + "/" +
        DBConstant::MULTI_SUB_DIR + "/meta_storage" + DBConstant::SQLITE_DB_EXTENSION;

    // test code, Don't needpay too much attention to exception handling
    int errCode = CreatMockMultiDb(properties, dbInfo);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = CreatMockMultiDb(kvStorageProperties, dbInfo);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = CreatMockMultiDb(metaStorageProperties, dbInfo);
    if (errCode != E_OK) {
        return errCode;
    }

    return errCode;
}

// OpenDbProperties.uri do not need
int DistributedDBToolsUnitTest::CreateMockMultiDb(DatabaseInfo &dbInfo, OpenDbProperties &properties)
{
    std::string identifier = dbInfo.userId + "-" + dbInfo.appId + "-" + dbInfo.storeId;
    std::string hashIdentifier = DBCommon::TransferHashString(identifier);
    std::string identifierName = DBCommon::TransferStringToHex(hashIdentifier);

    if (OS::GetRealPath(dbInfo.dir, properties.uri) != E_OK) {
        LOGE("Failed to canonicalize the path.");
        return -E_INVALID_ARGS;
    }

    int errCode = DBCommon::CreateStoreDirectory(dbInfo.dir, identifierName, DBConstant::MULTI_SUB_DIR, true);
    if (errCode != E_OK) {
        return errCode;
    }

    properties.uri = dbInfo.dir + "/" + identifierName + "/" + DBConstant::MULTI_SUB_DIR +
        "/" + DBConstant::MULTI_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;

    if (properties.sqls.empty()) {
        properties.sqls = {CREATE_TABLE_SQL};
    }

    OpenMockMultiDb(dbInfo, properties);

    return errCode;
}

int DistributedDBToolsUnitTest::GetResourceDir(std::string& dir)
{
    int errCode = GetCurrentDir(dir);
    if (errCode != E_OK) {
        return -E_INVALID_PATH;
    }

    return E_OK;
}

int DistributedDBToolsUnitTest::GetCurrentDir(std::string &dir)
{
    static const int maxFileLength = 1024;
    dir = "";
    char buffer[maxFileLength] = {0};
    int length = readlink("/proc/self/exe", buffer, maxFileLength);
    if (length < 0 || length >= maxFileLength) {
        LOGE("read directory err length:%d", length);
        return -E_LENGTH_ERROR;
    }
    LOGD("DIR = %s", buffer);
    dir = buffer;
    if (std::string::npos == dir.rfind("/") && std::string::npos == dir.rfind("\\")) {
        LOGE("current patch format err");
        return -E_INVALID_PATH;
    }

    if (dir.rfind("/") != std::string::npos) {
        dir.erase(dir.rfind("/") + 1);
    }
    return E_OK;
}

void DistributedDBToolsUnitTest::TestDirInit(std::string& dir)
{
    if (GetCurrentDir(dir) != E_OK) {
        dir = "/";
    }

    dir.append("testDbDir");
    DIR *dirTmp = opendir(dir.c_str());
    if (dirTmp == nullptr) {
        if (OS::MakeDBDirectory(dir) != 0) {
            LOGI("MakeDirectory err!");
            dir = "/";
            return;
        }
    } else {
        closedir(dirTmp);
    }
}

int DistributedDBToolsUnitTest::RemoveTestDbFiles(const std::string& dir)
{
    bool isExisted = OS::CheckPathExistence(dir);
    if (!isExisted) {
        return E_OK;
    }

    int nFile = 0;
    std::string dirName;
    struct dirent *direntPtr = nullptr;
    DIR *dirPtr = opendir(dir.c_str());
    if (dirPtr == nullptr) {
        LOGE("opendir error!");
        return -E_INVALID_PATH;
    }
    while (true) {
        direntPtr = readdir(dirPtr);
        // condition to exit the loop
        if (direntPtr == nullptr) {
            break;
        }
        // only remove all *.db files
        std::string str(direntPtr->d_name);
        if (str == "." || str == "..") {
            continue;
        }
        dirName.clear();
        dirName.append(dir).append("/").append(str);
        if (direntPtr->d_type == DT_DIR) {
            RemoveTestDbFiles(dirName);
            rmdir(dirName.c_str());
        } else if (remove(dirName.c_str()) != 0) {
            LOGI("remove file: %s failed!", dirName.c_str());
            continue;
        }
        nFile++;
    }
    closedir(dirPtr);
    LOGI("Total %d test db files are removed!", nFile);
    return 0;
}

void DistributedDBToolsUnitTest::KvStoreDelegateCallback(
    DBStatus statusSrc, KvStoreDelegate *kvStoreSrc, DBStatus &statusDst, KvStoreDelegate *&kvStoreDst)
{
    statusDst = statusSrc;
    kvStoreDst = kvStoreSrc;
}

void DistributedDBToolsUnitTest::KvStoreNbDelegateCallback(
    DBStatus statusSrc, KvStoreNbDelegate* kvStoreSrc, DBStatus &statusDst, KvStoreNbDelegate *&kvStoreDst)
{
    statusDst = statusSrc;
    kvStoreDst = kvStoreSrc;
}
void DistributedDBToolsUnitTest::SnapshotDelegateCallback(
    DBStatus statusSrc, KvStoreSnapshotDelegate* snapshot, DBStatus &statusDst, KvStoreSnapshotDelegate *&snapshotDst)
{
    statusDst = statusSrc;
    snapshotDst = snapshot;
}

void DistributedDBToolsUnitTest::ValueCallback(
    DBStatus statusSrc, const Value &valueSrc, DBStatus &statusDst, Value &valueDst)
{
    statusDst = statusSrc;
    valueDst = valueSrc;
}

void DistributedDBToolsUnitTest::EntryVectorCallback(DBStatus statusSrc, const std::vector<Entry> &entrySrc,
    DBStatus &statusDst, unsigned long &matchSize, std::vector<Entry> &entryDst)
{
    statusDst = statusSrc;
    matchSize = static_cast<unsigned long>(entrySrc.size());
    entryDst = entrySrc;
}

// size need bigger than prefixkey length
std::vector<uint8_t> DistributedDBToolsUnitTest::GetRandPrefixKey(const std::vector<uint8_t> &prefixKey, uint32_t size)
{
    std::vector<uint8_t> value;
    if (size <= prefixKey.size()) {
        return value;
    }
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, size - prefixKey.size());
    std::vector<uint8_t> res(prefixKey);
    res.insert(res.end(), value.begin(), value.end());
    return res;
}

void DistributedDBToolsUnitTest::GetRandomKeyValue(std::vector<uint8_t> &value, uint32_t defaultSize)
{
    uint32_t randSize = 0;
    if (defaultSize == 0) {
        uint8_t simSize = 0;
        RAND_bytes(&simSize, 1);
        randSize = (simSize == 0) ? 1 : simSize;
    } else {
        randSize = defaultSize;
    }

    value.resize(randSize);
    RAND_bytes(value.data(), randSize);
}

bool DistributedDBToolsUnitTest::IsValueEqual(const DistributedDB::Value &read, const DistributedDB::Value &origin)
{
    if (read != origin) {
        DBCommon::PrintHexVector(read, __LINE__, "read");
        DBCommon::PrintHexVector(origin, __LINE__, "origin");
        return false;
    }

    return true;
}

bool DistributedDBToolsUnitTest::IsEntryEqual(const DistributedDB::Entry &entryOrg,
    const DistributedDB::Entry &entryRet)
{
    if (entryOrg.key != entryRet.key) {
        LOGD("key not equal, entryOrg key size is [%d], entryRet key size is [%d]", entryOrg.key.size(),
            entryRet.key.size());
        return false;
    }

    if (entryOrg.value != entryRet.value) {
        LOGD("value not equal, entryOrg value size is [%d], entryRet value size is [%d]", entryOrg.value.size(),
            entryRet.value.size());
        return false;
    }

    return true;
}

bool DistributedDBToolsUnitTest::IsEntriesEqual(const std::vector<DistributedDB::Entry> &entriesOrg,
    const std::vector<DistributedDB::Entry> &entriesRet, bool needSort)
{
    LOGD("entriesOrg size is [%d], entriesRet size is [%d]", entriesOrg.size(),
        entriesRet.size());

    if (entriesOrg.size() != entriesRet.size()) {
        return false;
    }
    std::vector<DistributedDB::Entry> entries1 = entriesOrg;
    std::vector<DistributedDB::Entry> entries2 = entriesRet;

    if (needSort) {
        sort(entries1.begin(), entries1.end(), CompareEntry);
        sort(entries2.begin(), entries2.end(), CompareEntry);
    }

    for (size_t i = 0; i < entries1.size(); i++) {
        if (entries1[i].key != entries2[i].key) {
            LOGE("IsEntriesEqual failed, key of index[%d] not match", i);
            return false;
        }
        if (entries1[i].value != entries2[i].value) {
            LOGE("IsEntriesEqual failed, value of index[%d] not match", i);
            return false;
        }
    }

    return true;
}

bool DistributedDBToolsUnitTest::CheckObserverResult(const std::vector<DistributedDB::Entry> &orgEntries,
    const std::list<DistributedDB::Entry> &resultLst)
{
    LOGD("orgEntries.size() is [%d], resultLst.size() is [%d]", orgEntries.size(),
        resultLst.size());

    if (orgEntries.size() != resultLst.size()) {
        return false;
    }

    int index = 0;
    for (auto &entry : resultLst) {
        if (entry.key != orgEntries[index].key) {
            LOGE("CheckObserverResult failed, key of index[%d] not match", index);
            return false;
        }
        if (entry.value != orgEntries[index].value) {
            LOGE("CheckObserverResult failed, value of index[%d] not match", index);
            return false;
        }
        index++;
    }

    return true;
}

bool DistributedDBToolsUnitTest::IsEntryExist(const DistributedDB::Entry &entry,
    const std::vector<DistributedDB::Entry> &entries)
{
    std::set<std::vector<uint8_t>> sets;
    for (const auto &iter : entries) {
        sets.insert(iter.key);
    }

    if (entries.size() != sets.size()) {
        return false;
    }
    sets.clear();
    bool isFound = false;
    for (const auto &iter : entries) {
        if (entry.key == iter.key) {
            if (entry.value == iter.value) {
                isFound = true;
            }
            break;
        }
    }
    return isFound;
}

bool DistributedDBToolsUnitTest::IsItemValueExist(const DistributedDB::DataItem &item,
    const std::vector<DistributedDB::DataItem> &items)
{
    std::set<Key> sets;
    for (const auto &iter : items) {
        sets.insert(iter.key);
    }

    if (items.size() != sets.size()) {
        return false;
    }
    sets.clear();
    bool isFound = false;
    for (const auto &iter : items) {
        if (item.key == iter.key) {
            if (item.value == iter.value) {
                isFound = true;
            }
            break;
        }
    }
    return isFound;
}

bool DistributedDBToolsUnitTest::IsKvEntryExist(const DistributedDB::Entry &entry,
    const std::vector<DistributedDB::Entry> &entries)
{
    std::set<std::vector<uint8_t>> sets;
    for (const auto &iter : entries) {
        sets.insert(iter.key);
    }

    if (entries.size() != sets.size()) {
        return false;
    }
    sets.clear();
    bool isFound = false;
    for (const auto &iter : entries) {
        if (entry.key == iter.key) {
            if (entry.value == iter.value) {
                isFound = true;
            }
            break;
        }
    }

    return isFound;
}

int DistributedDBToolsUnitTest::ModifyDatabaseFile(const std::string &fileDir)
{
    LOGI("Modify database file:%s", fileDir.c_str());
    std::fstream dataFile(fileDir, std::fstream::binary | std::fstream::out | std::fstream::in);
    if (!dataFile.is_open()) {
        LOGD("Open the database file failed");
        return -E_UNEXPECTED_DATA;
    }

    if (!dataFile.seekg(0, std::fstream::end)) {
        return -E_UNEXPECTED_DATA;
    }

    uint64_t fileSize;
    std::ios::pos_type pos = dataFile.tellg();
    if (pos < 0) {
        return -E_UNEXPECTED_DATA;
    } else {
        fileSize = static_cast<uint64_t>(pos);
        if (fileSize < 1024) { // the least page size is 1024 bytes.
            LOGE("Invalid database file:%llu.", fileSize);
            return -E_UNEXPECTED_DATA;
        }
    }

    const int sqliteCountPos = 0;
    if (!dataFile.seekp(sqliteCountPos)) {
        return -E_UNEXPECTED_DATA;
    }
    uint32_t currentCount = 0x1F1F1F1F; // add the random value to corrupt the head.
    for (int i = 0; i < 256; i++) { // 256 is 1024 / 4 times.
        if (!dataFile.write(reinterpret_cast<char *>(&currentCount), sizeof(uint32_t))) {
            return -E_UNEXPECTED_DATA;
        }
    }

    dataFile.flush();
    return E_OK;
}

int DistributedDBToolsUnitTest::GetSyncDataTest(const SyncInputArg &syncInputArg, SQLiteSingleVerNaturalStore *store,
    std::vector<DataItem> &dataItems, ContinueToken &continueStmtToken)
{
    std::vector<SingleVerKvEntry *> entries;
    DataSizeSpecInfo syncDataSizeInfo = {syncInputArg.blockSize_, DBConstant::MAX_HPMODE_PACK_ITEM_SIZE};
    int errCode = store->GetSyncData(syncInputArg.begin_, syncInputArg.end_, entries,
        continueStmtToken, syncDataSizeInfo);

    ConvertSingleVerEntryToItems(entries, dataItems);
    return errCode;
}

int DistributedDBToolsUnitTest::GetSyncDataNextTest(SQLiteSingleVerNaturalStore *store, uint32_t blockSize,
    std::vector<DataItem> &dataItems, ContinueToken &continueStmtToken)
{
    std::vector<SingleVerKvEntry *> entries;
    DataSizeSpecInfo syncDataSizeInfo = {blockSize, DBConstant::MAX_HPMODE_PACK_ITEM_SIZE};
    int errCode = store->GetSyncDataNext(entries, continueStmtToken, syncDataSizeInfo);

    ConvertSingleVerEntryToItems(entries, dataItems);
    return errCode;
}

int DistributedDBToolsUnitTest::PutSyncDataTest(SQLiteSingleVerNaturalStore *store,
    const std::vector<DataItem> &dataItems, const std::string &deviceName)
{
    std::vector<SingleVerKvEntry *> entries;
    std::vector<DistributedDB::DataItem> items = dataItems;
    for (auto &item : items) {
        GenericSingleVerKvEntry *entry = new (std::nothrow) GenericSingleVerKvEntry();
        if (entry == nullptr) {
            ReleaseSingleVerEntry(entries);
            return -E_OUT_OF_MEMORY;
        }
        entry->SetEntryData(std::move(item));
        entry->SetWriteTimestamp(entry->GetTimestamp());
        entries.push_back(entry);
    }

    int errCode = store->PutSyncData(entries, deviceName);
    ReleaseSingleVerEntry(entries);
    return errCode;
}

int DistributedDBToolsUnitTest::ConvertItemsToSingleVerEntry(const std::vector<DistributedDB::DataItem> &dataItems,
    std::vector<DistributedDB::SingleVerKvEntry *> &entries)
{
    std::vector<DistributedDB::DataItem> items = dataItems;
    for (auto &item : items) {
        GenericSingleVerKvEntry *entry = new (std::nothrow) GenericSingleVerKvEntry();
        if (entry == nullptr) {
            ReleaseSingleVerEntry(entries);
            return -E_OUT_OF_MEMORY;
        }
        entry->SetEntryData(std::move(item));
        entries.push_back(entry);
    }
    return E_OK;
}

void DistributedDBToolsUnitTest::ConvertSingleVerEntryToItems(std::vector<DistributedDB::SingleVerKvEntry *> &entries,
    std::vector<DistributedDB::DataItem> &dataItems)
{
    for (auto &itemEntry : entries) {
        GenericSingleVerKvEntry *entry = reinterpret_cast<GenericSingleVerKvEntry *>(itemEntry);
        if (entry != nullptr) {
            DataItem item;
            item.origDev = entry->GetOrigDevice();
            item.flag = entry->GetFlag();
            item.timeStamp = entry->GetTimestamp();
            entry->GetKey(item.key);
            entry->GetValue(item.value);
            dataItems.push_back(item);
            // clear vector entry
            delete itemEntry;
            itemEntry = nullptr;
        }
    }
    entries.clear();
}

void DistributedDBToolsUnitTest::ReleaseSingleVerEntry(std::vector<DistributedDB::SingleVerKvEntry *> &entries)
{
    for (auto &item : entries) {
        delete item;
        item = nullptr;
    }
    entries.clear();
}

void DistributedDBToolsUnitTest::CalcHash(const std::vector<uint8_t> &value, std::vector<uint8_t> &hashValue)
{
    ValueHashCalc hashCalc;
    hashCalc.Initialize();
    hashCalc.Update(value);
    hashCalc.GetResult(hashValue);
}

KvStoreObserverUnitTest::KvStoreObserverUnitTest() : callCount_(0), isCleared_(false)
{}

void KvStoreObserverUnitTest::OnChange(const KvStoreChangedData& data)
{
    callCount_++;
    inserted_ = data.GetEntriesInserted();
    updated_ = data.GetEntriesUpdated();
    deleted_ = data.GetEntriesDeleted();
    isCleared_ = data.IsCleared();
    LOGD("Onchangedata :%lu -- %lu -- %lu -- %d", inserted_.size(), updated_.size(), deleted_.size(), isCleared_);
    LOGD("Onchange() called success!");
}

void KvStoreObserverUnitTest::ResetToZero()
{
    callCount_ = 0;
    isCleared_ = false;
    inserted_.clear();
    updated_.clear();
    deleted_.clear();
}

unsigned long KvStoreObserverUnitTest::GetCallCount() const
{
    return callCount_;
}

const std::list<Entry> &KvStoreObserverUnitTest::GetEntriesInserted() const
{
    return inserted_;
}

const std::list<Entry> &KvStoreObserverUnitTest::GetEntriesUpdated() const
{
    return updated_;
}

const std::list<Entry> &KvStoreObserverUnitTest::GetEntriesDeleted() const
{
    return deleted_;
}

bool KvStoreObserverUnitTest::IsCleared() const
{
    return isCleared_;
}

DBStatus DistributedDBToolsUnitTest::SyncTest(KvStoreNbDelegate* delegate,
    const std::vector<std::string>& devices, SyncMode mode,
    std::map<std::string, DBStatus>& statuses, bool wait)
{
    statuses.clear();
    DBStatus callStatus = delegate->Sync(devices, mode,
        [&statuses, this](const std::map<std::string, DBStatus>& statusMap) {
            statuses = statusMap;
            std::unique_lock<std::mutex> innerlock(this->syncLock_);
            this->syncCondVar_.notify_one();
        }, wait);
    if (!wait) {
        std::unique_lock<std::mutex> lock(syncLock_);
        syncCondVar_.wait(lock, [callStatus, &statuses](){
            if (callStatus != OK) {
                return true;
            }
            if (statuses.size() != 0) {
                return true;
            }
            return false;
        });
    }
    return callStatus;
}

void KvStoreCorruptInfo::CorruptCallBack(const std::string &appId, const std::string &userId,
    const std::string &storeId)
{
    DatabaseInfo databaseInfo;
    databaseInfo.appId = appId;
    databaseInfo.userId = userId;
    databaseInfo.storeId = storeId;
    LOGD("appId :%s, userId:%s, storeId:%s", appId.c_str(), userId.c_str(), storeId.c_str());
    databaseInfoVect_.push_back(databaseInfo);
}

size_t KvStoreCorruptInfo::GetDatabaseInfoSize() const
{
    return databaseInfoVect_.size();
}

bool KvStoreCorruptInfo::IsDataBaseCorrupted(const std::string &appId, const std::string &userId,
    const std::string &storeId) const
{
    for (const auto &item : databaseInfoVect_) {
        if (item.appId == appId &&
            item.userId == userId &&
            item.storeId == storeId) {
            return true;
        }
    }
    return false;
}

void KvStoreCorruptInfo::Reset()
{
    databaseInfoVect_.clear();
}
} // namespace DistributedDBUnitTest
