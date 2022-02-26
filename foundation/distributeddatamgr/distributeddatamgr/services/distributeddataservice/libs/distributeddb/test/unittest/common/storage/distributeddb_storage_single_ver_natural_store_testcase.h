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

#include <gtest/gtest.h>
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "sqlite_utils.h"
#include "sqlite_single_ver_natural_store.h"
#include "sqlite_single_ver_natural_store_connection.h"
#include "sqlite_local_kvdb.h"
#include "multi_ver_natural_store.h"
#include "db_errno.h"

#ifndef DISTRIBUTEDDB_STORAGE_SINGLE_VER_NATURAL_STORE_TESTCASE_H
#define DISTRIBUTEDDB_STORAGE_SINGLE_VER_NATURAL_STORE_TESTCASE_H
struct SyncData {
    std::vector<uint8_t> hashKey;
    std::vector<uint8_t> key;
    std::vector<uint8_t> value;
    uint64_t timeStamp;
    uint64_t flag;
    std::string deviceInfo;
};

class DistributedDBStorageSingleVerNaturalStoreTestCase final {
public:
    DistributedDBStorageSingleVerNaturalStoreTestCase() {};
    ~DistributedDBStorageSingleVerNaturalStoreTestCase() {};

    static void GetSyncData001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void GetSyncData002(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void GetSyncData003(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void GetSyncData004(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void GetSyncData005(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void GetSyncData006(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void PutSyncData001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void PutSyncData002(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void PutSyncData003(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void PutMetaData001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void GetMetaData001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void GetCurrentMaxTimeStamp001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void GetCurrentMaxTimeStamp002(DistributedDB::SQLiteSingleVerNaturalStore *&store);

    static void LocalDatabaseOperate001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void LocalDatabaseOperate002(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void LocalDatabaseOperate003(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void SyncDatabaseOperate001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void SyncDatabaseOperate002(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void SyncDatabaseOperate003(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void SyncDatabaseOperate004(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void SyncDatabaseOperate005(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void SyncDatabaseOperate006(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void ClearRemoteData001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void DeleteUserKeyValue001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &dbName);

    static void MemoryDbDeleteUserKeyValue001(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &dbName);

    static void DeleteUserKeyValue002(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &dbName);

    static void DeleteUserKeyValue003(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &dbName);

    static void DeleteUserKeyValue004(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &dbName);

    static void MemoryDbDeleteUserKeyValue004(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &dbName);

    static void DeleteUserKeyValue005(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &dbName);

    static void MemoryDbDeleteUserKeyValue005(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &dbName);

    static void DeleteUserKeyValue006(DistributedDB::SQLiteSingleVerNaturalStore *&store,
        DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &dbName);
    static int GetRawSyncData(const std::string &dbName, const std::string &strSql, std::vector<SyncData> &vecSyncData);

private:
    static void CreateMemDb(DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, int &errCode);

    static bool IsSqlinteExistKey(const std::vector<SyncData> &vecSyncData, const std::vector<uint8_t> &key);

    static void TestMetaDataPutAndGet(DistributedDB::SQLiteSingleVerNaturalStore *&store,
    DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection);

    static void DataBaseCommonPutOperate(DistributedDB::SQLiteSingleVerNaturalStore *&store,
     DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, DistributedDB::IOption option);

    static void DataBaseCommonDeleteOperate(DistributedDB::SQLiteSingleVerNaturalStore *&store,
    DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, DistributedDB::IOption option);

    static void DataBaseCommonGetOperate(DistributedDB::SQLiteSingleVerNaturalStore *&store,
    DistributedDB::SQLiteSingleVerNaturalStoreConnection *&connection, DistributedDB::IOption option);
};
#endif