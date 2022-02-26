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

#ifndef SQLITE_MULTI_VER_TRANSACTION_H
#define SQLITE_MULTI_VER_TRANSACTION_H

#ifndef OMIT_MULTI_VER
#include <string>
#include <vector>
#include <mutex>

#include "sqlite_import.h"
#include "db_types.h"
#include "kvdb_properties.h"
#include "ikvdb_multi_ver_transaction.h"
#include "macro_utils.h"
#include "multi_ver_value_object.h"
#include "generic_multi_ver_kv_entry.h"

namespace DistributedDB {
class SQLiteMultiVerTransaction : public IKvDBMultiVerTransaction {
public:
    SQLiteMultiVerTransaction();
    ~SQLiteMultiVerTransaction() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteMultiVerTransaction);

    int Initialize(const std::string &uri, bool isReadOnly, CipherType type, const CipherPassword &passwd);

    int Put(const Key &key, const Value &value) override;

    int Delete(const Key &key) override;

    int Clear() override;

    int Get(const Key &key, Value &value) const override;

    int GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const override;

    int PutBatch(const std::vector<Entry> &entries);

    int PutBatch(const std::vector<MultiVerKvEntry *> &entries, bool isLocal, std::vector<Value> &values) override;

    int GetDiffEntries(const Version &begin, const Version &end, MultiVerDiffData &data) const override;

    int GetMaxVersion(MultiVerDataType type, Version &maxVersion) const override;

    int ClearEntriesByVersion(const Version &versionInfo) override;

    int StartTransaction() override;

    int RollBackTransaction() override;

    int CommitTransaction() override;

    int GetEntriesByVersion(Version version, std::list<MultiVerTrimedVersionData> &data) const override;

    // Get Entries from the version.
    int GetEntriesByVersion(const Version &versionInfo, std::vector<MultiVerKvEntry *> &entries) const override;

    // Update the timestamp of the version.
    int UpdateTimestampByVersion(const Version &version, TimeStamp stamp) const override;

    bool IsDataChanged() const override;

    // Get the max timestamp to generate the new version for the writing transaction
    TimeStamp GetCurrentMaxTimestamp() const override;

    // Reset the version.
    void ResetVersion();

    // Reset the transaction while committing failed.
    int Reset(CipherType type, const CipherPassword &passwd);

    // Check if the entry already cleared
    bool IsRecordCleared(const TimeStamp timestamp) const override;

    void SetVersion(const Version &versionInfo) override;

    Version GetVersion() const override;

    int GetOverwrittenClearTypeEntries(Version clearVersion, std::list<MultiVerTrimedVersionData> &data) const override;

    int GetOverwrittenNonClearTypeEntries(Version version, const Key &hashKey,
        std::list<MultiVerTrimedVersionData> &data) const override;

    int DeleteEntriesByHashKey(Version version, const Key &hashKey) override;

    int GetValueForTrimSlice(const Key &hashKey, const Version version, Value &value) const override;

    int CheckIfNeedSaveRecord(sqlite3_stmt *statement, const MultiVerKvEntry *multiVerKvEntry,
        bool &isNeedSave, Value &origVal) const;

private:
    struct GetEntriesStatements {
        sqlite3_stmt *getEntriesStatement = nullptr;
        sqlite3_stmt *hashFilterStatement = nullptr;
    };

    enum EntryOperator {
        INSERT,
        UPDATE,
        DELETE,
        CLEAR,
        FAIL,
    };

    static int GetRawMultiVerEntry(sqlite3_stmt *statement, MultiVerEntryData &keyEntry);

    static int GetRawDataByVersion(sqlite3_stmt *&statement, const Version &version,
        std::vector<MultiVerEntryData> &entries);

    static int GetDiffOperator(int errCode, uint64_t flag);

    static int BindAddRecordKeysToStatement(sqlite3_stmt *statement, const Key &key,
        const MultiVerEntryAuxData &data);

    int AddRecord(const Key &key, const Value &value, const MultiVerEntryAuxData &data);

    void ClassifyDiffEntries(int errCode, uint64_t flag, const Value &value,
        MultiVerEntryData &item, MultiVerDiffData &data) const;

    void GetClearId() const;

    int BindClearIdAndVersion(sqlite3_stmt *statement, int index) const;

    int BindQueryEntryArgs(sqlite3_stmt *statement, const Key &key) const;

    int BindQueryEntriesArgs(sqlite3_stmt *statement, const Key &key) const;

    int BindAddRecordArgs(sqlite3_stmt *statement, const Key &key, const Value &value,
        const MultiVerEntryAuxData &data) const;

    int GetOneEntry(const GetEntriesStatements &statements, const Key &lastKey, Entry &entry, int &errCode) const;

    int RemovePrePutEntries(const Version &versionInfo, TimeStamp timestamp);

    int CheckToSaveRecord(const MultiVerKvEntry *entry, bool &isNeedSave, std::vector<Value> &values);

    // Check if the entry with later timestamp already exist in the database
    int CheckIfNeedSaveRecord(const MultiVerKvEntry *multiVerKvEntry, bool &isNeedSave, Value &value) const;

    int PrepareForGetEntries(const Key &keyPrefix, GetEntriesStatements &statements) const;

    int ReleaseGetEntriesStatements(GetEntriesStatements &statements) const;

    int GetKeyAndValueByHashKey(sqlite3_stmt *statement, const Key &hashKey, Key &key, Value &value,
        bool isNeedReadKey) const;

    int GetOriginKeyValueByHash(MultiVerEntryData &item, Value &value) const;

    int GetPrePutValues(const Version &versionInfo, TimeStamp timestamp, std::vector<Value> &values) const;

    static const std::string CREATE_TABLE_SQL;
    static const std::string SELECT_ONE_SQL; // select the rowid
    static const std::string SELECT_BATCH_SQL; // select the rowid and the key
    static const std::string SELECT_HASH_ENTRY_SQL; // select the data according the hash key
    static const std::string SELECT_ONE_VER_SQL; // select the rowid
    static const std::string SELECT_BATCH_VER_SQL; // select the rowid and the key
    static const std::string SELECT_ONE_VER_RAW_SQL;
    static const std::string INSERT_SQL; // insert or replace the values
    static const std::string DELETE_SQL; // delete the key-value record
    static const std::string SELECT_ONE_BY_KEY_TIMESTAMP_SQL; // get one by key and timestamp

    static const std::string DELETE_VER_SQL;
    static const std::string SELECT_PRE_PUT_VER_DATA_SQL;
    static const std::string DELETE_PRE_PUT_VER_DATA_SQL;
    static const std::string SELECT_LATEST_CLEAR_ID;
    static const std::string SELECT_MAX_LOCAL_VERSION;
    static const std::string SELECT_MAX_VERSION;
    static const std::string SELECT_MAX_TIMESTAMP;
    static const std::string UPDATE_VERSION_TIMESTAMP;
    static const std::string SELECT_OVERWRITTEN_CLEAR_TYPE;
    static const std::string SELECT_OVERWRITTEN_NO_CLEAR_TYPE;
    static const std::string DELETE_BY_VER_HASHKEY_SQL;
    static const std::string SELECT_BY_HASHKEY_VER_SQL;

    static const uint64_t ADD_FLAG = 0x01; // add or replace the record.
    static const uint64_t DEL_FLAG = 0x02; // delete the record.
    static const uint64_t CLEAR_FLAG = 0x03; // clear all the record.

    static const uint64_t LOCAL_FLAG = 0x08; // local flag for read.
    static const uint64_t OPERATE_MASK = 0x07; // operate mask for add, delete

    std::mutex resetMutex_;
    mutable std::mutex readMutex_;

    mutable int64_t clearId_; // for query the result after the clear operation in the same commit.
    mutable int64_t clearTime_; // for query the result after the clear operation
    mutable uint64_t currentMaxTimestamp_;
    Version version_; // the read version or the current commit version.
    sqlite3 *db_;
    std::string uri_;
    bool isReadOnly_;
    bool isDataChanged_; // whether the transaction has new record.
};
} // namespace DistributedDB

#endif // SQLITE_MULTI_VER_TRANSACTION_H
#endif