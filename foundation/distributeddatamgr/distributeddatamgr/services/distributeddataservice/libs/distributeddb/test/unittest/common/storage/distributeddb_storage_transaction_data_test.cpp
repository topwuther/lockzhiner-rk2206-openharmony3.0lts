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
#include <cstdlib>
#include <ctime>
#include <string>
#include <openssl/rand.h>
#include <thread>

#include "db_common.h"
#include "db_errno.h"
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "log_print.h"
#include "multi_ver_natural_store.h"
#include "multi_ver_natural_store_commit_storage.h"
#include "multi_ver_natural_store_connection.h"
#include "default_factory.h"
#include "sqlite_multi_ver_data_storage.h"
#include "sqlite_utils.h"
#include "db_constant.h"
#include "process_communicator_test_stub.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    const int WAIT_TIME = 1000;
    const uint64_t INVALID_TIMESTAMP = 0;
    const uint64_t OPERATION_ADD = 1;
    const uint64_t OPERATION_DELETE = 2;
    const uint64_t OPERATION_CLEAR = 3;

    string g_testDir;
    KvDBProperties g_prop;
    SQLiteMultiVerTransaction *g_transaction = nullptr;
    MultiVerNaturalStore *g_naturalStore = nullptr;
    MultiVerNaturalStoreConnection *g_naturalStoreConnection = nullptr;
    Version g_version = 0;
    const std::string CREATE_TABLE =
        "CREATE TABLE IF NOT EXISTS version_data(key BLOB, value BLOB, oper_flag INTEGER, version INTEGER, " \
        "timestamp INTEGER, ori_timestamp INTEGER, hash_key BLOB, " \
        "PRIMARY key(hash_key, version));";
}

class DistributedDBStorageTransactionDataTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

static void GetReadTransaction()
{
    if (g_transaction == nullptr) {
        g_transaction = new (std::nothrow) SQLiteMultiVerTransaction();
        ASSERT_NE(g_transaction, nullptr);
        std::string dir = g_testDir + "/31/multi_ver/multi_ver_data.db";
        LOGI("%s", dir.c_str());
        CipherPassword passwd;
        int errCode = g_transaction->Initialize(dir, true, CipherType::AES_256_GCM, passwd);
        ASSERT_EQ(errCode, E_OK);
    }
    Version versionInfo;
    ASSERT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, versionInfo), E_OK);
    g_version = versionInfo;
    g_transaction->SetVersion(versionInfo);
}

static void ValueEqual(const Value &read, const Value &origin)
{
    EXPECT_EQ(read.size(), origin.size());
    if (read.size() != origin.size()) {
        DBCommon::PrintHexVector(origin, __LINE__, "Orig");
    }

    EXPECT_EQ(read, origin);
}

static int RunSyncMergeForOneCommit(std::vector<MultiVerKvEntry *> &entries)
{
    MultiVerCommitNode multiVerCommit;
    multiVerCommit.commitId.resize(20); // commit id size
    RAND_bytes(multiVerCommit.commitId.data(), 20);
    multiVerCommit.deviceInfo = DBCommon::TransferHashString("deviceB") + "deviceB1";

    // Put the multiver commit of other device.
    int errCode = g_naturalStore->PutCommitData(multiVerCommit, entries, "deviceB");
    if (errCode != E_OK) {
        return errCode;
    }

    std::vector<MultiVerCommitNode> multiVerCommits;
    multiVerCommits.push_back(multiVerCommit);

    // Merge the multiver commit of other device.
    errCode = g_naturalStore->MergeSyncCommit(multiVerCommit, multiVerCommits);

    for (auto &item : entries) {
        if (item != nullptr) {
            delete item;
            item = nullptr;
        }
    }
    entries.clear();

    return errCode;
}

static uint64_t GetCommitTimestamp(const CommitID& commitId)
{
    MultiVerNaturalStoreCommitStorage *commitStorage = new (std::nothrow) MultiVerNaturalStoreCommitStorage();
    if (commitStorage == nullptr) {
        return 0;
    }
    TimeStamp timestamp = INVALID_TIMESTAMP;
    CommitID newCommitId;
    IKvDBCommit *commit = nullptr;
    IKvDBCommitStorage::Property property;
    property.isNeedCreate = false;
    property.path = g_testDir;
    property.identifierName = "31";
    int errCode = commitStorage->Open(property);
    if (errCode != E_OK) {
        goto END;
    }

    if (commitId.empty()) {
        newCommitId = commitStorage->GetHeader(errCode);
        if (newCommitId.empty()) {
            return 0;
        }
    } else {
        newCommitId = commitId;
    }

    commit = commitStorage->GetCommit(newCommitId, errCode);
    if (commit == nullptr) {
        LOGE("Can't get the commit:%d", errCode);
        goto END;
    }

    timestamp = commit->GetTimestamp();
END:
    if (commit != nullptr) {
        commitStorage->ReleaseCommit(commit);
        commit = nullptr;
    }

    delete commitStorage;
    commitStorage = nullptr;

    return timestamp;
}

static uint64_t GetMaxTimestamp()
{
    CommitID commitId;
    return GetCommitTimestamp(commitId);
}

static void PutAndCommitEntry(const Key &key, const Value &value)
{
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->Put(option, key, value), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);
}

static void PushOneEntry(uint64_t opr, uint64_t timestamp, const Key &key, const Value &value,
    std::vector<MultiVerKvEntry *> &entries)
{
    GenericMultiVerKvEntry *entry = new (std::nothrow) GenericMultiVerKvEntry;
    if (entry != nullptr) {
        // set key
        entry->SetKey(key);
        // set value
        MultiVerValueObject valueObject;
        valueObject.SetValue(value);
        Value objectSerial;
        valueObject.GetSerialData(objectSerial);
        entry->SetValue(objectSerial);
        // set timestamp
        entry->SetTimestamp(timestamp);

        // set open_flag
        entry->SetOperFlag(opr);
        if (opr == OPERATION_DELETE) {
            Key hashKey;
            DBCommon::CalcValueHash(key, hashKey);
            entry->SetKey(hashKey);
        } else if (opr == OPERATION_CLEAR) {
            Key clearKey = {'c', 'l', 'e', 'a', 'r'};
            entry->SetKey(clearKey);
        }

        entries.push_back(entry);
    }
}

static void ValueEqualByKey(const Key &key, const Value &value)
{
    IOption option;
    Value valueRead;
    int errCode = g_naturalStoreConnection->Get(option, key, valueRead);
    EXPECT_EQ(errCode, E_OK);
    if (errCode != E_OK) {
        DBCommon::PrintHexVector(key, __LINE__, "key");
    }
    ValueEqual(value, valueRead);
}

void DistributedDBStorageTransactionDataTest::SetUpTestCase(void)
{
    IKvDBFactory *factory = new (std::nothrow) DefaultFactory();
    ASSERT_TRUE(factory != nullptr);
    IKvDBFactory::Register(factory);
}

void DistributedDBStorageTransactionDataTest::TearDownTestCase(void)
{
    if (g_transaction != nullptr) {
        delete g_transaction;
        g_transaction = nullptr;
    }
    auto factory = IKvDBFactory::GetCurrent();
    if (factory != nullptr) {
        delete factory;
        factory = nullptr;
    }
    IKvDBFactory::Register(nullptr);
}

void DistributedDBStorageTransactionDataTest::SetUp(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);

    // KvDBProperties prop;
    g_prop.SetStringProp(KvDBProperties::APP_ID, "app0");
    g_prop.SetStringProp(KvDBProperties::STORE_ID, "store0");
    g_prop.SetStringProp(KvDBProperties::USER_ID, "user0");
    g_prop.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    g_prop.SetStringProp(KvDBProperties::IDENTIFIER_DIR, "31");
    g_prop.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);

    g_naturalStore = new (std::nothrow) MultiVerNaturalStore();
    ASSERT_NE(g_naturalStore, nullptr);
    EXPECT_EQ(g_naturalStore->Open(g_prop), E_OK);

    int errCode = 0;
    IKvDBConnection *connection = g_naturalStore->GetDBConnection(errCode);
    ASSERT_NE(connection, nullptr);
    g_naturalStoreConnection = static_cast<MultiVerNaturalStoreConnection *>(connection);

    LOGI("read directory :%s", g_testDir.c_str());
}

void DistributedDBStorageTransactionDataTest::TearDown(void)
{
    if (g_transaction != nullptr) {
        delete g_transaction;
        g_transaction = nullptr;
    }

    if (g_naturalStore != nullptr) {
        if (g_naturalStoreConnection != nullptr) {
            g_naturalStoreConnection->Close();
            g_naturalStoreConnection = nullptr;
        }
        g_naturalStore->DecObjRef(g_naturalStore);
        g_naturalStore = nullptr;
    }
    DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir + "/31/" + DBConstant::MULTI_SUB_DIR);
}

/**
  * @tc.name: StorageInsert001
  * @tc.desc: Put the non-empty key, non-empty value into the database.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageInsert001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the data(non-empty key, non-empty value) into the database.
     * @tc.expected: step1. Put returns E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_1), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step2. Get the data.
     * @tc.expected: step2. Get returns E_OK and the value is equal to the put value.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_1, valueRead), E_OK);
    ValueEqual(VALUE_1, valueRead);
    /**
     * @tc.steps: step3. Clear the data.
     */
    g_naturalStoreConnection->Clear(option);
    /**
     * @tc.steps: step4. Put another data(non-empty key, non-empty value) into the database.
     * @tc.expected: step4. Put returns E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_2, VALUE_2), E_OK);
    EXPECT_NE(g_naturalStoreConnection->Commit(), E_OK);
    /**
     * @tc.steps: step5. Get the data.
     * @tc.expected: step5. Get returns E_OK and the value is equal to the second put value.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_2, valueRead), E_OK);
    ValueEqual(VALUE_2, valueRead);
}

 /**
  * @tc.name: StorageInsert002
  * @tc.desc: Put the empty key, non-empty value into the database.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageInsert002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the data(empty key, non-empty value) into the database.
     * @tc.expected: step1. Put returns -E_INVALID_ARGS.
     */
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->Put(option, NULL_KEY_1, VALUE_1), -E_INVALID_ARGS);
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);
}

/**
  * @tc.name: StorageInsert003
  * @tc.desc: Put the non-empty key, empty value into the database.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageInsert003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the data(non-empty key, empty value) into the database.
     * @tc.expected: step1. Put returns E_OK.
     */
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, NULL_VALUE_1), E_OK);
    GetReadTransaction();
    Value valueRead;
    Value valueTmp;
    /**
     * @tc.steps: step2. Get the data.
     * @tc.expected: step2. Get returns E_OK and the value is empty.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_1, valueRead), E_OK);
    ValueEqual(NULL_VALUE_1, valueRead);
}

/**
  * @tc.name: StorageUpdate001
  * @tc.desc: Update the value to non-empty
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageUpdate001, TestSize.Level1)
{
    IOption option;
    /**
     * @tc.steps: step1. Put one valid data into the database.
     * @tc.expected: step1. Put returns E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_1), E_OK);
    /**
     * @tc.steps: step2. Put another data whose key is same to the first put data and value(non-empty) is different.
     * @tc.expected: step2. Put returns E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_2), E_OK);
    GetReadTransaction();
    Value valueRead;
    /**
     * @tc.steps: step3. Get the data.
     * @tc.expected: step3. Get returns E_OK and the value is equal the second put value.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_1, valueRead), E_OK);
    ValueEqual(VALUE_2, valueRead);
}

/**
  * @tc.name: StorageUpdate002
  * @tc.desc: Update the value to empty
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageUpdate002, TestSize.Level1)
{
    IOption option;
    /**
     * @tc.steps: step1. Put one valid data into the database.
     * @tc.expected: step1. Put returns E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_1), E_OK);
    /**
     * @tc.steps: step2. Put another data whose key is same to the first put data and value is empty.
     * @tc.expected: step2. Put returns E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, NULL_VALUE_1), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step3. Get the data.
     * @tc.expected: step3. Get returns E_OK and the value is empty.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_1, valueRead), E_OK);
    ValueEqual(NULL_VALUE_1, valueRead);
}

/**
  * @tc.name: StorageDelete001
  * @tc.desc: Delete the existed data
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageDelete001, TestSize.Level1)
{
    IOption option;
    /**
     * @tc.steps: step1. Put one valid data.
     */
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_1), E_OK);
    /**
     * @tc.steps: step2. Delete the data.
     */
    EXPECT_EQ(g_naturalStoreConnection->Delete(option, KEY_1), E_OK);
    GetReadTransaction();
    Value valueRead;
    /**
     * @tc.steps: step3. Get the data.
     * @tc.expected: step3. Get returns -E_NOT_FOUND.
     */
    EXPECT_EQ(g_transaction->Get(KEY_1, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: StorageDelete002
  * @tc.desc: Delete the non-existed data
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageDelete002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Delete one non-existed data.
     * @tc.expected: step1. Delete returns -E_NOT_FOUND.
     */
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->Delete(option, KEY_1), -E_NOT_FOUND);
    GetReadTransaction();
    Value valueRead;
    /**
     * @tc.steps: step2. Get the non-existed data.
     * @tc.expected: step2. Get returns -E_NOT_FOUND.
     */
    EXPECT_EQ(g_transaction->Get(KEY_1, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: StorageDelete003
  * @tc.desc: Delete the invalid key data
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageDelete003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Delete the empty-key data.
     * @tc.expected: step1. Delete returns not E_OK.
     */
    IOption option;
    EXPECT_NE(g_naturalStoreConnection->Delete(option, NULL_KEY_1), E_OK);
}

/**
  * @tc.name: StorageClear001
  * @tc.desc: Clear the data
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageClear001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. put one data.
     */
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_1), E_OK);
    /**
     * @tc.steps: step2. clear the data.
     * @tc.expected: step2. Returns E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->Clear(option), E_OK);
    GetReadTransaction();
    /**
     * @tc.steps: step3. Check the data.
     * @tc.expected: step3. Getting the data result -E_NOT_FOUND.
     */
    Value valueRead;
    EXPECT_EQ(g_transaction->Get(KEY_1, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: StorageInsertBatch001
  * @tc.desc: Put the valid batch data
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageInsertBatch001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the batch data.
     * @tc.expected: step1. Returns E_OK.
     */
    std::vector<Entry> entries;
    entries.push_back(KV_ENTRY_1);
    entries.push_back(KV_ENTRY_2);
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entries), E_OK);
    Value valueRead;

    /**
     * @tc.steps: step2. Check the data.
     * @tc.expected: step2. Get the data from the database and the value are equal to the data put before.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_1, valueRead), E_OK);
    ValueEqual(VALUE_1, valueRead);
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_2, valueRead), E_OK);
    ValueEqual(VALUE_2, valueRead);
}

/**
  * @tc.name: StorageInsertBatch002
  * @tc.desc: Put the partially valid batch data
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageInsertBatch002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the batch data(partially valid, partially invalid).
     * @tc.expected: step1. Returns not E_OK.
     */
    std::vector<Entry> entries;
    Entry entry;
    entries.push_back(KV_ENTRY_1);
    entries.push_back(entry);
    IOption option;
    EXPECT_NE(g_naturalStoreConnection->PutBatch(option, entries), E_OK);
    GetReadTransaction();
    Value valueRead;
    /**
     * @tc.steps: step2. Check the data.
     * @tc.expected: step2. Getting the data results not E_OK.
     */
    EXPECT_NE(g_transaction->Get(KEY_1, valueRead), E_OK);
}

/**
  * @tc.name: StorageUpdateBatch001
  * @tc.desc: Update the batch data
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageUpdateBatch001, TestSize.Level1)
{
    std::vector<Entry> entries1;
    entries1.push_back(KV_ENTRY_1);
    entries1.push_back(KV_ENTRY_2);

    Entry kvEntry1 = {KEY_1, VALUE_2};
    Entry kvEntry2 = {KEY_2, VALUE_1};
    std::vector<Entry> entries2;
    entries2.push_back(kvEntry1);
    entries2.push_back(kvEntry2);

    IOption option;
    /**
     * @tc.steps: step1. Put the batch data.
     */
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entries1), E_OK);
    /**
     * @tc.steps: step2. Update the batch data.
     * @tc.expected: step2. Returns E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entries2), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step3. Check the data.
     * @tc.expected: step3. Get the data from the database and check whether the data have been updated.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_1, valueRead), E_OK);
    ValueEqual(VALUE_2, valueRead);
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_2, valueRead), E_OK);
    ValueEqual(VALUE_1, valueRead);
}

/**
  * @tc.name: StorageUpdateBatch002
  * @tc.desc: Update the batch data(partially invalid data)
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageUpdateBatch002, TestSize.Level1)
{
    std::vector<Entry> entrys1;
    entrys1.push_back(KV_ENTRY_1);
    entrys1.push_back(KV_ENTRY_2);

    Entry kvEntry1 = {KEY_1, VALUE_2};
    Entry kvEntry;
    Entry kvEntry2 = {KEY_2, VALUE_1};
    std::vector<Entry> entrys2;
    entrys2.push_back(kvEntry1);
    entrys2.push_back(kvEntry);
    entrys2.push_back(kvEntry2);

    IOption option;
    /**
     * @tc.steps: step1. Put the batch data.
     */
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entrys1), E_OK);
    /**
     * @tc.steps: step2. Update the batch data(partially empty key).
     * @tc.expected: step2. Returns not E_OK.
     */
    EXPECT_NE(g_naturalStoreConnection->PutBatch(option, entrys2), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step3. Check the data.
     * @tc.expected: step3. The getting result data are the first put batch .
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_1, valueRead), E_OK);
    ValueEqual(VALUE_1, valueRead);
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_2, valueRead), E_OK);
    ValueEqual(VALUE_2, valueRead);
}

/**
  * @tc.name: StorageDeleteBatch001
  * @tc.desc: Delete the batch data
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageDeleteBatch001, TestSize.Level1)
{
    std::vector<Entry> entries;
    entries.push_back(KV_ENTRY_1);
    entries.push_back(KV_ENTRY_2);
    std::vector<Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);

    IOption option;
    /**
     * @tc.steps: step1. Put the batch data.
     */
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entries), E_OK);
    /**
     * @tc.steps: step2. Delete the batch data.
     * @tc.expected: step2. Return E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->DeleteBatch(option, keys), E_OK);
    GetReadTransaction();
    Value valueRead;
    /**
     * @tc.steps: step3. Check the data.
     * @tc.expected: step3. Getting the data results -E_NOT_FOUND.
     */
    EXPECT_EQ(g_transaction->Get(KEY_1, valueRead), -E_NOT_FOUND);
    EXPECT_EQ(g_transaction->Get(KEY_2, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: StorageDeleteBatch002
  * @tc.desc: Delete the batch data(partially non-existed)
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageDeleteBatch002, TestSize.Level1)
{
    std::vector<Entry> entries;
    entries.push_back(KV_ENTRY_1);
    entries.push_back(KV_ENTRY_2);
    std::vector<Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);
    std::vector<uint8_t> k3 = {'3'};
    keys.push_back(k3);

    IOption option;
    /**
     * @tc.steps: step1. Put the batch data.
     */
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entries), E_OK);
    /**
     * @tc.steps: step2. Delete the batch data(partially non-existed).
     * @tc.expected: step2. Return E_OK.
     */
    EXPECT_EQ(g_naturalStoreConnection->DeleteBatch(option, keys), E_OK);
    GetReadTransaction();
    Value valueRead;
    /**
     * @tc.steps: step3. Check the data.
     * @tc.expected: step3. Cannot Get the delete data in the database.
     */
    EXPECT_NE(g_transaction->Get(KEY_1, valueRead), E_OK);
    EXPECT_NE(g_transaction->Get(KEY_2, valueRead), E_OK);
}

/**
  * @tc.name: StorageDeleteBatch003
  * @tc.desc: Delete the batch data(partially invalid)
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageDeleteBatch003, TestSize.Level1)
{
    std::vector<Entry> entries;
    entries.push_back(KV_ENTRY_1);
    entries.push_back(KV_ENTRY_2);
    std::vector<Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);
    Key k3;
    keys.push_back(k3);

    IOption option;
    /**
     * @tc.steps: step1. Put the batch data.
     */
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entries), E_OK);
    /**
     * @tc.steps: step2. Delete the batch data(partially invalid).
     * @tc.expected: step2. Return E_OK.
     */
    EXPECT_NE(g_naturalStoreConnection->DeleteBatch(option, keys), E_OK);
    GetReadTransaction();
    Value valueRead;
    /**
     * @tc.steps: step3. Check the data.
     * @tc.expected: step3. Can get the put origined data.
     */
    EXPECT_EQ(g_transaction->Get(KEY_1, valueRead), E_OK);
    EXPECT_EQ(g_transaction->Get(KEY_2, valueRead), E_OK);
}

/**
  * @tc.name: StorageTransactionCombo001
  * @tc.desc: Multiple operation within the transaction
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, StorageTransactionCombo001, TestSize.Level1)
{
    Entry kvEntry3;
    Entry kvEntry4;
    kvEntry3.key = {'3'};
    kvEntry4.key = {'4'};
    kvEntry3.value = {'c'};
    kvEntry4.value = {'d'};
    // inserted data
    std::vector<Entry> entrys1;
    entrys1.push_back(KV_ENTRY_1);
    entrys1.push_back(KV_ENTRY_2);
    entrys1.push_back(kvEntry3);
    entrys1.push_back(kvEntry4);

    kvEntry3.value = {'e'};
    kvEntry4.value = {'f'};
    // updated data
    std::vector<Entry> entrys2;
    entrys2.push_back(kvEntry3);
    entrys2.push_back(kvEntry4);

    // deleted data
    std::vector<Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);

    IOption option;
    /**
     * @tc.steps: step1. Start the transaction.
     */
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    /**
     * @tc.steps: step2. Put the batch data.
     */
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entrys1), E_OK);
    /**
     * @tc.steps: step3. Delete the batch data.
     */
    EXPECT_EQ(g_naturalStoreConnection->DeleteBatch(option, keys), E_OK);
    /**
     * @tc.steps: step4. Update the batch data.
     */
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entrys2), E_OK);
    /**
     * @tc.steps: step5. Commit the transaction.
     */
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step6. Check the data.
     * @tc.expected: step6. Can get the updated data.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, kvEntry3.key, valueRead), E_OK);
    ValueEqual(kvEntry3.value, valueRead);
    EXPECT_EQ(g_naturalStoreConnection->Get(option, kvEntry4.key, valueRead), E_OK);
    ValueEqual(kvEntry4.value, valueRead);
}

/**
  * @tc.name: TransactionRollback001
  * @tc.desc: Multiple operation within the transaction
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, TransactionRollback001, TestSize.Level1)
{
    std::vector<Entry> entries;
    entries.push_back(KV_ENTRY_1);
    entries.push_back(KV_ENTRY_2);
    IOption option;
    /**
     * @tc.steps: step1. Start the transaction.
     */
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    /**
     * @tc.steps: step2. Put the batch data.
     */
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entries), E_OK);
    /**
     * @tc.steps: step3. Rollback the transaction.
     */
    EXPECT_EQ(g_naturalStoreConnection->RollBack(), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step4. Check the data.
     * @tc.expected: step4. Couldn't find the data in the database.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_1, valueRead), -E_NOT_FOUND);
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_2, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: TransactionGetCommitData001
  * @tc.desc: Get the commit data of one transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, TransactionGetCommitData001, TestSize.Level1)
{
    std::vector<Entry> entries;
    entries.push_back(KV_ENTRY_1);
    entries.push_back(KV_ENTRY_2);
    IOption option;
    /**
     * @tc.steps: step1. Put the batch data within in one transaction.
     */
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->PutBatch(option, entries), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);

    Value valueRead;
    /**
     * @tc.steps: step2. Check the put batch data, and could get the put data.
     */
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_1, valueRead), E_OK);
    ValueEqual(valueRead, KV_ENTRY_1.value);
    EXPECT_EQ(g_naturalStoreConnection->Get(option, KEY_2, valueRead), E_OK);
    ValueEqual(valueRead, KV_ENTRY_2.value);
    /**
     * @tc.steps: step3. Get one commit data.
     */
    GetReadTransaction();
    std::vector<MultiVerKvEntry *> multiVerKvEntries;
    EXPECT_EQ(g_transaction->GetEntriesByVersion(g_version, multiVerKvEntries), OK);
    ASSERT_EQ(multiVerKvEntries.size(), 2UL);
    auto entry1 = static_cast<GenericMultiVerKvEntry *>(multiVerKvEntries[0]);
    ASSERT_NE(entry1, nullptr);
    valueRead.clear();
    EXPECT_EQ(entry1->GetValue(valueRead), E_OK);
    /**
     * @tc.steps: step4. Check the commit data.
     * @tc.expected: step4. Could find the batch put data in the commit data.
     */
    auto entry2 = static_cast<GenericMultiVerKvEntry *>(multiVerKvEntries[1]);
    ASSERT_NE(entry2, nullptr);
    valueRead.clear();
    EXPECT_EQ(entry2->GetValue(valueRead), E_OK);
    for (auto &item : multiVerKvEntries) {
        delete item;
        item = nullptr;
    }
}

/**
  * @tc.name: TransactionSqliteKvEntry001
  * @tc.desc: Serialize the kv entry and deserialize the data.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, TransactionSqliteKvEntry001, TestSize.Level1)
{
    GenericMultiVerKvEntry entry;
    entry.SetOperFlag(17);
    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    entry.SetKey(key);
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, 20);
    /**
     * @tc.steps: step1. Initialize the multi version kv entry.
     */
    MultiVerValueObject valueObject;
    valueObject.SetValue(value);
    Value objectSerial;

    valueObject.GetSerialData(objectSerial);
    entry.SetValue(objectSerial);
    /**
     * @tc.steps: step2. Get the serial data of the entry.
     */
    std::vector<uint8_t> serialData;
    EXPECT_EQ(entry.GetSerialData(serialData), E_OK);
    /**
     * @tc.steps: step3. Deserial the data.
     */
    GenericMultiVerKvEntry deEntry;
    EXPECT_EQ(deEntry.DeSerialData(serialData), E_OK);
    Key keyRead;
    Value valueRead;
    Value valueTmp;
    uint64_t flag;
    deEntry.GetKey(keyRead);
    deEntry.GetValue(valueTmp);
    deEntry.GetOperFlag(flag);
    ValueEqual(keyRead, key);
    MultiVerValueObject objectRead;
    EXPECT_EQ(objectRead.DeSerialData(valueTmp), E_OK);
    objectRead.GetValue(valueRead);
    /**
     * @tc.steps: step4. Check the deserialized data.
     * @tc.expected: step4. the deserialized value is equal to the set value.
     */
    ValueEqual(valueRead, value);
    EXPECT_EQ(flag, 17UL);
}

/**
  * @tc.name: TransactionPutForeignData001
  * @tc.desc: Put the remote commit data into the current device database.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, TransactionPutForeignData001, TestSize.Level1)
{
    GenericMultiVerKvEntry entry;
    entry.SetOperFlag(1);
    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    entry.SetKey(key);
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    MultiVerValueObject valueObject;
    valueObject.SetValue(value);
    Value objectSerial;
    valueObject.GetSerialData(objectSerial);
    entry.SetValue(objectSerial);

    std::vector<MultiVerKvEntry *> entries;
    entries.push_back(&entry);
    /**
     * @tc.steps: step1. Create the multiver commit.
     */
    MultiVerCommitNode multiVerCommit;
    multiVerCommit.commitId.resize(20); // commit id size
    RAND_bytes(multiVerCommit.commitId.data(), 20);
    multiVerCommit.deviceInfo = DBCommon::TransferHashString("deviceB") + "deviceB1";
    /**
     * @tc.steps: step2. Put the multiver commit of other device.
     */
    EXPECT_EQ(g_naturalStore->PutCommitData(multiVerCommit, entries, "deviceB"), E_OK);
    std::vector<MultiVerCommitNode> multiVerCommits;
    multiVerCommits.push_back(multiVerCommit);
    /**
     * @tc.steps: step3. Merge the multiver commit of other device.
     */
    EXPECT_EQ(g_naturalStore->MergeSyncCommit(multiVerCommit, multiVerCommits), E_OK);

    /**
     * @tc.steps: step4. Get the commit data, the foreign synced data would be got from sync.
     */
    std::vector<MultiVerKvEntry *> readEntries;
    EXPECT_EQ(g_naturalStore->GetCommitData(multiVerCommit, readEntries), E_OK);
    ASSERT_EQ(readEntries.size(), 0UL);
}

/**
  * @tc.name: DefaultConflictResolution001
  * @tc.desc: Merge data without conflicts
  * @tc.type: FUNC
  * @tc.require: AR000CQE13 AR000CQE14
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, DefaultConflictResolution001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the local data(KEY_1, VALUE_1) into the database.
     * @tc.expected: step1. Put returns E_OK.
     */
    PutAndCommitEntry(KEY_1, VALUE_1);
    /**
     * @tc.steps: step2. Put the external data(KEY_2, VALUE_2) into the database.
     * @tc.expected: step2. Put returns E_OK
     */
    std::vector<MultiVerKvEntry *> entries;
    PushOneEntry(OPERATION_ADD, 1, KEY_2, VALUE_2, entries);
    EXPECT_EQ(RunSyncMergeForOneCommit(entries), E_OK);
    /**
     * @tc.steps: step3. Get value1 and value2
     * @tc.expected: step3. Value1 and value2 are correct.
     */
    ValueEqualByKey(KEY_1, VALUE_1);
    ValueEqualByKey(KEY_2, VALUE_2);
}

/**
  * @tc.name: DefaultConflictResolution002
  * @tc.desc: Merge data with conflicts ,no clear operation in the external data and local data
  * @tc.type: FUNC
  * @tc.require: AR000CQE13 AR000CQE14
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, DefaultConflictResolution002, TestSize.Level1)
{
    PutAndCommitEntry(KEY_1, VALUE_1);
    /**
     * @tc.steps: step1. Put the [KEY_2,V2] and [KEY_3,V3] into the database and delete [KEY_1,V1]
     * @tc.expected: step1. Both Put and Delete operation returns E_OK.
     */
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_2, VALUE_2), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_3, VALUE_3), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Delete(option, KEY_1), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);
    /**
     * @tc.steps: step2. Get latest timestamp
     */
    TimeStamp t1 = GetMaxTimestamp();
    EXPECT_TRUE(t1 > 0);
    /**
     * @tc.steps: step3. Put the external entry[KEY_2,VALUE_4,T2] into the database.
     * @tc.expected: step3. Put returns E_OK
     */
    std::vector<MultiVerKvEntry *> entriesV4;
    PushOneEntry(OPERATION_ADD, t1 - 1, KEY_2, VALUE_4, entriesV4);
    EXPECT_EQ(RunSyncMergeForOneCommit(entriesV4), E_OK);
    /**
     * @tc.steps: step4. Get value of K2
     * @tc.expected: step4. value of K2 is equals V2
     */
    ValueEqualByKey(KEY_2, VALUE_2);
    /**
     * @tc.steps: step5. Put the external entry[KEY_2,VALUE_5,T3] into the database.
     * @tc.expected: step5. Put returns E_OK
     */
    std::vector<MultiVerKvEntry *> entriesV5;
    PushOneEntry(OPERATION_ADD, t1 + 1, KEY_2, VALUE_5, entriesV5);
    EXPECT_EQ(RunSyncMergeForOneCommit(entriesV5), E_OK);
    /**
     * @tc.steps: step6. Get value of K2
     * @tc.expected: step6. value of K2 is equals V5
     */
    ValueEqualByKey(KEY_2, VALUE_5);
    /**
     * @tc.steps: step7. Put the external Delete entry[KEY_3,T2] into the database.
     * @tc.expected: step7. Put returns E_OK
     */
    std::vector<MultiVerKvEntry *> entriesV6;
    PushOneEntry(OPERATION_DELETE, t1 - 1, KEY_3, VALUE_6, entriesV6);
    EXPECT_EQ(RunSyncMergeForOneCommit(entriesV6), E_OK);
    /**
     * @tc.steps: step8. Get value of K3
     * @tc.expected: step8. value of K3 is equals V3
     */
    ValueEqualByKey(KEY_3, VALUE_3);
    /**
     * @tc.steps: step9. Put the external Delete entry[KEY_3,T2] into the database.
     * @tc.expected: step9. Put returns E_OK
     */
    std::vector<MultiVerKvEntry *> entriesV7;
    PushOneEntry(OPERATION_DELETE, t1 + 1, KEY_3, VALUE_7, entriesV7);
    EXPECT_EQ(RunSyncMergeForOneCommit(entriesV7), E_OK);
    /**
     * @tc.steps: step10. Get value of K3
     * @tc.expected: step10. Return NOT_FOUND
     */
    GetReadTransaction();
    Value valueTmp;
    EXPECT_EQ(g_transaction->Get(KEY_3, valueTmp), -E_NOT_FOUND);
    /**
     * @tc.steps: step11. Put the external entry[KEY_1,VALUE_6,T2] into the database.
     * @tc.expected: step11. Put returns E_OK
     */
    std::vector<MultiVerKvEntry *> entriesV8;
    PushOneEntry(OPERATION_ADD, t1 - 1, KEY_1, VALUE_6, entriesV8);
    EXPECT_EQ(RunSyncMergeForOneCommit(entriesV8), E_OK);
    /**
     * @tc.steps: step12. Get value of K1
     * @tc.expected: step12. Return NOT_FOUND
     */
    GetReadTransaction();
    EXPECT_EQ(g_transaction->Get(KEY_1, valueTmp), -E_NOT_FOUND);
    /**
     * @tc.steps: step13. Put the external entry[KEY_1,VALUE_7,T2] into the database.
     * @tc.expected: step13. Put returns E_OK
     */
    std::vector<MultiVerKvEntry *> entriesV9;
    PushOneEntry(OPERATION_ADD, t1 + 1, KEY_1, VALUE_7, entriesV9);
    EXPECT_EQ(RunSyncMergeForOneCommit(entriesV9), E_OK);
    /**
     * @tc.steps: step14. Get value of K1
     * @tc.expected: step14. value of K1 is V7
     */
    ValueEqualByKey(KEY_1, VALUE_7);
}

/**
  * @tc.name: DefaultConflictResolution003
  * @tc.desc: Merge data with conflicts, clear operation is in the external data
  * @tc.type: FUNC
  * @tc.require: AR000CQE13 AR000CQE14
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, DefaultConflictResolution003, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Put the local data(KEY_1, VALUE_1) into the database.
     * @tc.expected: step1. Put returns E_OK.
     */
    PutAndCommitEntry(KEY_1, VALUE_1);
    /**
     * @tc.steps: step2. Get timestampV1
     */
    TimeStamp timestampV1 = GetMaxTimestamp();
    TimeStamp timestampClear = timestampV1 + 1;
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    /**
     * @tc.steps: step3. Put the local data(KEY_2, VALUE_2) into the database.
     * @tc.expected: step3. Put returns E_OK.
     */
    PutAndCommitEntry(KEY_2, VALUE_2);
    /**
     * @tc.steps: step4. Get timestampV2
     */
    TimeStamp timestampV2 = GetMaxTimestamp();
    /**
     * @tc.steps: step5. Put the external clear entry into the database.
     * @tc.expected: step5. Put returns E_OK
     */
    std::vector<MultiVerKvEntry *> entries;
    PushOneEntry(OPERATION_CLEAR, timestampClear, KEY_3, VALUE_3, entries);
    EXPECT_EQ(RunSyncMergeForOneCommit(entries), E_OK);

    EXPECT_TRUE(timestampV1 < timestampClear);
    EXPECT_TRUE(timestampClear < timestampV2);

    /**
     * @tc.steps: step6. Get value1 and value2
     * @tc.expected: step6. Get Value1 return NOT_FOUND , value2 is correct.
     */
    Value valueTmp;
    GetReadTransaction();
    EXPECT_EQ(g_transaction->Get(KEY_1, valueTmp), -E_NOT_FOUND);

    ValueEqualByKey(KEY_2, VALUE_2);
}

/**
  * @tc.name: DefaultConflictResolution004
  * @tc.desc: Merge data with conflicts, clear operation is in the local data
  * @tc.type: FUNC
  * @tc.require: AR000CQE13 AR000CQE14
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, DefaultConflictResolution004, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Put the local data(KEY_1, VALUE_1) into the database and get the latest timestamp.
     * @tc.expected: step1. Put returns E_OK.
     */
    PutAndCommitEntry(KEY_1, VALUE_1);
    TimeStamp t1 = GetMaxTimestamp();
    EXPECT_TRUE(t1 > 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    /**
     * @tc.steps: step2. Put the local data(KEY_2, VALUE_2) into the database and get the latest timestamp.
     * @tc.expected: step2. Put returns E_OK.
     */
    PutAndCommitEntry(KEY_2, VALUE_2);
    TimeStamp t2 = GetMaxTimestamp();
    /**
     * @tc.steps: step3. Execute Clear() operation and get the latest timestamp.
     */
    IOption option;
    g_naturalStoreConnection->Clear(option);
    TimeStamp t3 = GetMaxTimestamp();
    EXPECT_TRUE(t3 > 0);
    /**
     * @tc.steps: step4. Put the local data(KEY_3, VALUE_3) into the database and get the latest timestamp.
     * @tc.expected: step4. Put returns E_OK.
     */
    PutAndCommitEntry(KEY_3, VALUE_3);
    /**
     * @tc.steps: step5. Put the external data [Clear, T5],[KEY_4,VALUE_4,T6],[KEY_5,VALUE_5,T7] into the database.
     * @tc.expected: step5. Put returns E_OK
     */
    EXPECT_TRUE(t1 + 1 < t2);
    std::vector<MultiVerKvEntry *> entries;
    // put clear entry
    PushOneEntry(OPERATION_CLEAR, t1 + 1, KEY_7, VALUE_7, entries);
    // put K4 entry
    PushOneEntry(OPERATION_ADD, t3 - 1, KEY_4, VALUE_4, entries);
    // put K5 entry
    PushOneEntry(OPERATION_ADD, t3 + 1, KEY_5, VALUE_5, entries);
    // merge data
    EXPECT_EQ(RunSyncMergeForOneCommit(entries), E_OK);
    /**
     * @tc.steps: step6. Get value of KEY_1,KEY_2,KEY_3,KEY_4,K5
     */
    GetReadTransaction();
    Value valueTmp;
    EXPECT_EQ(g_transaction->Get(KEY_1, valueTmp), -E_NOT_FOUND);
    EXPECT_EQ(g_transaction->Get(KEY_2, valueTmp), -E_NOT_FOUND);
    EXPECT_EQ(g_transaction->Get(KEY_4, valueTmp), -E_NOT_FOUND);

    ValueEqualByKey(KEY_3, VALUE_3);
    ValueEqualByKey(KEY_5, VALUE_5);
}

/**
  * @tc.name: CommitTimestamp001
  * @tc.desc: Test the timestamp of the native commit.
  * @tc.type: FUNC
  * @tc.require: AR000CQE11
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, CommitTimestamp001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. Put in some data(non-empty key, non-empty value) within one transaction.
     */
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_1), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_2, VALUE_2), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);

    /**
     * @tc.steps: step2. Add different operations(add,update,delete,clear) within one transaction.
     */
    std::srand(std::time(0)); // set the current time to the seed.
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_4, VALUE_4), E_OK); // add
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000));
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_3), E_OK); // update
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000));
    EXPECT_EQ(g_naturalStoreConnection->Delete(option, KEY_2), E_OK);  // delete
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000));
    EXPECT_EQ(g_naturalStoreConnection->Clear(option), E_OK); // clear
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000));
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);

    /**
     * @tc.steps: step3. Get all the record data in the newest commit.
     */
    GetReadTransaction();
    std::vector<MultiVerKvEntry *> entries;
    g_transaction->GetEntriesByVersion(g_version, entries);
    ASSERT_EQ(entries.size(), 4UL); // add, update, delete and clear for 4 operation.

    std::set<uint64_t> timeSet;
    for (auto item : entries) {
        uint64_t timestamp = 0;
        item->GetTimestamp(timestamp);
        timeSet.insert(timestamp);
    }

    ASSERT_EQ(timeSet.size(), 1UL); // only one timestamp in one commit.
    // Tobe compare the timestamp.
    CommitID commitId;
    TimeStamp commitTimestamp = GetCommitTimestamp(commitId);
    LOGD("TimeRecord:%llu, TimeCommit:%llu", *(timeSet.begin()), commitTimestamp);
    ASSERT_EQ(*(timeSet.begin()), commitTimestamp);
    ASSERT_NE(commitTimestamp, 0UL);

    for (auto &item : entries) {
        g_naturalStore->ReleaseKvEntry(item);
        item = nullptr;
    }
}

static bool PutFirstSyncCommitData(MultiVerCommitNode &multiVerCommit)
{
    GenericMultiVerKvEntry entry;
    entry.SetOperFlag(1); // add the new data.
    entry.SetKey(KEY_2);

    MultiVerValueObject valueObject;
    valueObject.SetValue(VALUE_2);
    Value objectSerial;
    valueObject.GetSerialData(objectSerial);
    entry.SetValue(objectSerial);
    entry.SetTimestamp(1000UL); // the first data timestamp.

    std::vector<MultiVerKvEntry *> entries;
    entries.push_back(&entry);
    /**
     * @tc.steps: step1. Create the multiver commit.
     */
    multiVerCommit.commitId.resize(20); // commit id size
    RAND_bytes(multiVerCommit.commitId.data(), 20); // commit id size
    multiVerCommit.deviceInfo = DBCommon::TransferHashString("deviceB") + "deviceB1";
    multiVerCommit.timestamp = 1000UL; // the first data timestamp.
    /**
     * @tc.steps: step2. Put the multiver commit of other device.
     */
    int errCode = g_naturalStore->PutCommitData(multiVerCommit, entries, "deviceB");
    if (errCode != E_OK) {
        return false;
    }
    return true;
}

static bool PutSecondSyncCommitData(const MultiVerCommitNode &multiVerCommit, MultiVerCommitNode &newCommit)
{
    GenericMultiVerKvEntry entry;
    entry.SetOperFlag(1); // add flag
    entry.SetKey(KEY_3);

    MultiVerValueObject valueObject;
    valueObject.SetValue(VALUE_3);
    Value objectSerial;
    valueObject.GetSerialData(objectSerial);
    entry.SetValue(objectSerial);
    entry.SetTimestamp(2000UL); // bigger than the first one.

    std::vector<MultiVerKvEntry *> entries;
    entries.push_back(&entry);

    newCommit.commitId.resize(20); // commit id size
    RAND_bytes(newCommit.commitId.data(), 20); // commit id size.
    newCommit.leftParent = multiVerCommit.commitId;
    newCommit.deviceInfo = DBCommon::TransferHashString("deviceB") + "deviceB1";
    newCommit.timestamp = 2000UL; // bigger than the first one.

    int errCode = g_naturalStore->PutCommitData(newCommit, entries, "deviceB");
    if (errCode != E_OK) {
        return false;
    }
    std::vector<MultiVerCommitNode> multiVerCommits;
    multiVerCommits.push_back(multiVerCommit);
    multiVerCommits.push_back(newCommit);

    errCode = g_naturalStore->MergeSyncCommit(newCommit, multiVerCommits);
    if (errCode != E_OK) {
        return false;
    }
    return true;
}

/**
  * @tc.name: CommitTimestamp002
  * @tc.desc: Test the timestamp of the native commits.
  * @tc.type: FUNC
  * @tc.require: AR000CQE11
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, CommitTimestamp002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put in some data(non-empty key, non-empty value) within one transaction.
     */
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_1), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_2, VALUE_2), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);

    CommitID commitId;
    TimeStamp stampFirst = GetCommitTimestamp(commitId);
    ASSERT_NE(stampFirst, 0UL); // non-zero

    /**
     * @tc.steps: step2. Add another data within one transaction.
     */
    std::srand(std::time(0)); // set the current time to the seed.
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000));
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_4, VALUE_4), E_OK);
    /**
     * @tc.steps: step3. Check the timestamp of the two commits.
     * @tc.expected: step3. the timestamp of the second commit is greater than the timestamp of the first commit.
     */
    TimeStamp stampSecond = GetCommitTimestamp(commitId);
    ASSERT_NE(stampSecond, 0UL); // non-zero
    LOGD("TimeFirst:%llu, TimeSecond:%llu", stampFirst, stampSecond);
    ASSERT_GT(stampSecond, stampFirst);
}
static void ReleaseKvEntries(std::vector<MultiVerKvEntry *> &entries)
{
    for (auto &item : entries) {
        if (item == nullptr) {
            continue;
        }
        g_naturalStore->ReleaseKvEntry(item);
        item = nullptr;
    }
}

/**
  * @tc.name: CommitTimestamp003
  * @tc.desc: Test the timestamp of the foreign commits.
  * @tc.type: FUNC
  * @tc.require: AR000CQE11
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, CommitTimestamp003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put in some data(non-empty key, non-empty value) within one transaction.
     */
    EXPECT_EQ(g_naturalStoreConnection->StartTransaction(), E_OK);
    IOption option;
    EXPECT_EQ(g_naturalStoreConnection->Put(option, KEY_1, VALUE_1), E_OK);
    EXPECT_EQ(g_naturalStoreConnection->Commit(), E_OK);

    /**
     * @tc.steps: step2. Add different operations(add,update,delete,clear) within one transaction.
     */
    std::srand(std::time(0)); // set the current time to the seed.
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000));
    MultiVerCommitNode commit;
    ASSERT_EQ(PutFirstSyncCommitData(commit), true); // add

    GetReadTransaction();
    std::vector<MultiVerKvEntry *> entries;
    g_transaction->GetEntriesByVersion(g_version, entries);
    ASSERT_EQ(entries.size(), 1UL); // sync commit have only one entry.
    ASSERT_NE(entries[0], nullptr);
    uint64_t timestamp = 0;
    entries[0]->GetTimestamp(timestamp);
    ReleaseKvEntries(entries);

    /**
     * @tc.steps: step3. Check the timestamp of the commit and the data.
     * @tc.steps: expected. the timestamp of the sync commit is equal to the timestamp of the data record.
     */
    TimeStamp commitTimestamp = GetCommitTimestamp(commit.commitId);
    LOGD("TimeRecord:%llu, TimeCommit:%llu", timestamp, commitTimestamp);
    ASSERT_EQ(timestamp, commitTimestamp);
    ASSERT_NE(commitTimestamp, 0UL);
}

/**
  * @tc.name: CommitTimestamp004
  * @tc.desc: Test the timestamp of the merge commits.
  * @tc.type: FUNC
  * @tc.require: AR000CQE11
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, CommitTimestamp004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Add the first sync commit.
     */
    MultiVerCommitNode commit;
    ASSERT_EQ(PutFirstSyncCommitData(commit), true); // add the first sync commit.

    std::srand(std::time(0)); // set the current time to the seed.
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000)); // sleep for random interval.

    /**
     * @tc.steps: step2. Add the second sync commit and merge the commit.
     */
    MultiVerCommitNode newCommit;
    ASSERT_EQ(PutSecondSyncCommitData(commit, newCommit), true);

    /**
     * @tc.steps: step3. Get the newest entries.
     */
    GetReadTransaction();
    std::vector<MultiVerKvEntry *> entries;
    g_transaction->GetEntriesByVersion(g_version, entries);
    ASSERT_EQ(entries.size(), 2UL); // merge node has 2 entries.

    std::set<uint64_t> timeSet;
    for (auto &item : entries) {
        ASSERT_NE(item, nullptr);
        uint64_t timestamp = 0;
        item->GetTimestamp(timestamp);
        g_naturalStore->ReleaseKvEntry(item);
        item = nullptr;
        timeSet.insert(timestamp);
    }
    /**
     * @tc.steps: step4. Get the timestamp of newest entries.
     * @tc.expected: step4. The merged commit have different timestamp.
     */
    ASSERT_EQ(timeSet.size(), 2UL); // entries have different timestamp.
    for (auto item : timeSet) {
        ASSERT_NE(item, 0UL);
    }
}

/**
  * @tc.name: GetBranchTag
  * @tc.desc: Test the branch tag of the commits.
  * @tc.type: FUNC
  * @tc.require: AR000CQE11
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionDataTest, GetBranchTag001, TestSize.Level1)
{
    KvStoreDelegateManager::SetProcessLabel("123", "456");
    KvStoreDelegateManager::SetProcessCommunicator(std::make_shared<ProcessCommunicatorTestStub>());
    ASSERT_NE(g_naturalStore, nullptr);
    std::vector<uint8_t> vectTag;
    g_naturalStore->GetCurrentTag(vectTag);
    DBCommon::PrintHexVector(vectTag);
    for (int i = 0; i < 10; i++) {
        if (g_naturalStore != nullptr) {
            if (g_naturalStoreConnection != nullptr) {
                g_naturalStoreConnection->Close();
                g_naturalStoreConnection = nullptr;
            }
            g_naturalStore->DecObjRef(g_naturalStore);
            g_naturalStore = new (std::nothrow) MultiVerNaturalStore;
            ASSERT_NE(g_naturalStore, nullptr);
            EXPECT_EQ(g_naturalStore->Open(g_prop), E_OK);
            std::vector<uint8_t> readTag;
            g_naturalStore->GetCurrentTag(readTag);
            DBCommon::PrintHexVector(readTag);
            EXPECT_EQ(vectTag, readTag);
        }
    }
}
