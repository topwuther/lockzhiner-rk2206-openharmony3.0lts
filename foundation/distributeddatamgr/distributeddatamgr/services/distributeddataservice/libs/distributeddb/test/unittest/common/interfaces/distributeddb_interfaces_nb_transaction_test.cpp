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
#include "db_errno.h"
#include "log_print.h"
#include "db_common.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    const int BATCH_BASE_SIZE = 60;
    const Key NULL_KEY;
    const Key NULL_VALUE;
    const int CONFLICT_ALL = 15;
    const auto OLD_VALUE_TYPE = KvStoreNbConflictData::ValueType::OLD_VALUE;
    const auto NEW_VALUE_TYPE = KvStoreNbConflictData::ValueType::NEW_VALUE;

    const int OBSERVER_SLEEP_TIME = 30;

    // define some variables to init a KvStoreDelegateManager object.
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    string g_testDir;
    KvStoreConfig g_config;

    // define the g_kvNbDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;

    struct SingleVerConflictData {
        KvStoreNbConflictType type = CONFLICT_NATIVE_ALL;
        Key key;
        Value oldValue;
        Value newValue;
        bool oldIsDeleted = false;
        bool newIsDeleted = false;
        bool oldIsNative = false;
        bool newIsNative = false;
        int getoldValueErrCode = 0;
        int getNewValueErrCode = 0;
        bool operator==(const SingleVerConflictData &comparedData) const
        {
            if (this->type == comparedData.type &&
                this->key == comparedData.key &&
                this->oldValue == comparedData.oldValue &&
                this->newValue == comparedData.newValue &&
                this->oldIsDeleted == comparedData.oldIsDeleted &&
                this->newIsDeleted == comparedData.newIsDeleted &&
                this->oldIsNative == comparedData.oldIsNative &&
                this->newIsNative == comparedData.newIsNative &&
                this->getoldValueErrCode == comparedData.getoldValueErrCode &&
                this->getNewValueErrCode == comparedData.getNewValueErrCode) {
                return true;
            }
            LOGD("type = %d, ctype = %d", this->type, comparedData.type);
            DBCommon::PrintHexVector(this->key, __LINE__, "key");
            DBCommon::PrintHexVector(comparedData.key, __LINE__, "ckey");
            DBCommon::PrintHexVector(this->oldValue, __LINE__, "oldValue");
            DBCommon::PrintHexVector(comparedData.oldValue, __LINE__, "coldValue");
            DBCommon::PrintHexVector(this->newValue, __LINE__, "newValue");
            DBCommon::PrintHexVector(comparedData.newValue, __LINE__, "cnewValue");

            LOGD("oldIsDeleted = %d, coldIsDeleted = %d", this->oldIsDeleted, comparedData.oldIsDeleted);
            LOGD("newIsDeleted = %d, cnewIsDeleted = %d", this->newIsDeleted, comparedData.newIsDeleted);
            LOGD("oldIsNative = %d, coldIsNative = %d", this->oldIsNative, comparedData.oldIsNative);
            LOGD("newIsNative = %d, cnewIsNative = %d", this->newIsNative, comparedData.newIsNative);
            LOGD("getoldValueErrCode = %d, cgetoldValueErrCode = %d", this->getoldValueErrCode,
                comparedData.getoldValueErrCode);
            LOGD("getNewValueErrCode = %d, cgetNewValueErrCode = %d", this->getNewValueErrCode,
                comparedData.getNewValueErrCode);

            return false;
        }
    };
    std::vector<SingleVerConflictData> g_conflictData;

    void KvStoreNbDelegateCallback(DBStatus statusSrc, KvStoreNbDelegate *kvStoreSrc,
        DBStatus *statusDst, KvStoreNbDelegate **kvStoreDst)
    {
        *statusDst = statusSrc;
        *kvStoreDst = kvStoreSrc;
    }

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, &g_kvDelegateStatus, &g_kvNbDelegatePtr);

    void NotifierCallback(const KvStoreNbConflictData &data)
    {
        Key key;
        Value oldValue;
        Value newValue;
        data.GetKey(key);
        data.GetValue(OLD_VALUE_TYPE, oldValue);
        LOGD("Get new value status:%d", data.GetValue(NEW_VALUE_TYPE, newValue));
        LOGD("Type:%d", data.GetType());
        DBCommon::PrintHexVector(oldValue, __LINE__);
        DBCommon::PrintHexVector(newValue, __LINE__);
        LOGD("Type:IsDeleted %d vs %d, IsNative %d vs %d", data.IsDeleted(OLD_VALUE_TYPE),
            data.IsDeleted(NEW_VALUE_TYPE), data.IsNative(OLD_VALUE_TYPE), data.IsNative(NEW_VALUE_TYPE));
        g_conflictData.push_back({data.GetType(), key, oldValue, newValue, data.IsDeleted(OLD_VALUE_TYPE),
            data.IsDeleted(NEW_VALUE_TYPE), data.IsNative(OLD_VALUE_TYPE), data.IsNative(NEW_VALUE_TYPE),
            data.GetValue(OLD_VALUE_TYPE, oldValue), data.GetValue(NEW_VALUE_TYPE, newValue)});
    }
}

class DistributedDBInterfacesNBTransactionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesNBTransactionTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.SetProcessLabel("DistributedDBInterfacesNBTransactionTest", "test");
}

void DistributedDBInterfacesNBTransactionTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesNBTransactionTest::SetUp(void)
{
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvNbDelegatePtr = nullptr;
}

void DistributedDBInterfacesNBTransactionTest::TearDown(void) {}

/**
  * @tc.name: start001
  * @tc.desc: Test the nb transaction start twice.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, start001, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_start001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    /**
     * @tc.steps:step2. Start transaction again.
     * @tc.expected: step2. return DB_ERROR.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), DB_ERROR);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_start001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: start002
  * @tc.desc: Test the nb transaction begin and end not match.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, start002, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_start002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Rollback transaction.
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);
    /**
     * @tc.steps:step3. Commit transaction.
     * @tc.expected: step3. return DB_ERROR.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), DB_ERROR);
    /**
     * @tc.steps:step4. Start transaction.
     * @tc.expected: step4. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step5. Commit transaction.
     * @tc.expected: step5. return DB_ERROR.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);
    /**
     * @tc.steps:step6. Rollback transaction.
     * @tc.expected: step6. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), DB_ERROR);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_start002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: start003
  * @tc.desc: Test the nb transaction rollback automatically when db close.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, start003, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_start003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    /**
     * @tc.steps:step2. Put (key1,value2)
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps:step3. Close db
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    /**
     * @tc.steps:step4. Open db again
     * @tc.expected: step4. return OK.
     */
    g_mgr.GetKvStore("distributed_nb_transaction_start003", option, g_kvNbDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    /**
     * @tc.steps:step5. Get key1
     * @tc.expected: step5. return OK, value of key1 is value1.
     */
    Value value;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_1);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_start003"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: start004
  * @tc.desc: Test the nb operations return BUSY after transaction started.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, start004, TestSize.Level4)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_start004", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Local data and sync data can be simultaneously operated in transactions.
     * @tc.expected: step2. From September 2020 return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_3, VALUE_3), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocal(KEY_1), OK);

    CipherPassword password;
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(password), BUSY);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(KEY_3, OBSERVER_CHANGES_NATIVE, observer), BUSY);
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), NOT_FOUND);
    delete observer;
    observer = nullptr;

    std::string filePath = g_testDir + "test.txt";
    EXPECT_EQ(g_kvNbDelegatePtr->Export(filePath, password), BUSY);

    KvStoreResultSet *readResultSet = nullptr;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(KEY_4, readResultSet), BUSY);
    EXPECT_EQ(g_kvNbDelegatePtr->CloseResultSet(readResultSet), INVALID_ARGS);
    /**
     * @tc.steps:step1. Commit transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_start004"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: commit001
  * @tc.desc: Test the nb transaction commit without start.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, commit001, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_commit001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps:step1. Commit transaction.
     * @tc.expected: step1. return DB_ERROR.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), DB_ERROR);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_commit001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: commit002
  * @tc.desc: Test the nb transaction commit twice.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, commit002, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_commit002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Get key1
     * @tc.expected: step2. return OK, value of key1 is value1.
     */
    Value value1;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, value1), OK);
    EXPECT_EQ(value1, VALUE_1);
    /**
     * @tc.steps:step3. Put (key2,value2)
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_2), OK);
    /**
     * @tc.steps:step4. Get key2
     * @tc.expected: step4. return OK, value of key2 is value2.
     */
    Value value2;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, value2), OK);
    EXPECT_EQ(value2, VALUE_2);
    /**
     * @tc.steps:step5. Commit transaction.
     * @tc.expected: step5. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);
    /**
     * @tc.steps:step6. Commit transaction again.
     * @tc.expected: step6. return DB_ERROR.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), DB_ERROR);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_commit002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: commit003
  * @tc.desc: Test the entry size exceed the maximum limit in one transaction
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, commit003, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_commit003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Put (key1,value1)
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps:step3. Delete key1
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_1), OK);
    /**
     * @tc.steps:step4. PutBatch 64 records (from key2 to key65)
     * @tc.expected: step4. return OK.
     */
    vector<Entry> entrysBase;
    vector<Key> keysBase;
    DistributedDBUnitTest::GenerateRecords(BATCH_BASE_SIZE + 5, entrysBase, keysBase);

    vector<Entry> entrys1(entrysBase.begin() + 1, entrysBase.end());
    EXPECT_EQ(entrys1.size(), 64UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys1), OK);
    /**
     * @tc.steps:step5. DeleteBatch 63 records (from key2 to key64)
     * @tc.expected: step5. return OK.
     */
    vector<Key> keys1(keysBase.begin() + 1, keysBase.end() - 1);
    EXPECT_EQ(keys1.size(), 63UL);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys1), OVER_MAX_LIMITS);
    /**
     * @tc.steps:step6. DeleteBatch 60 records (from key1 to key60)
     * @tc.expected: step6. return OK.
     */
    vector<Key> keys2(keysBase.begin(), keysBase.begin() + 60);
    EXPECT_EQ(keys2.size(), 60UL);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys2), OK);
    /**
     * @tc.steps:step6. Commit.
     * @tc.expected: step6. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);
    /**
     * @tc.steps:step7. GetEntries.
     * @tc.expected: step7. return OK.
     */
    vector<Entry> entriesExpect(entrysBase.begin() + 60, entrysBase.end());
    EXPECT_EQ(entriesExpect.size(), 5UL);
    const Key prefix;
    vector<Entry> entriesRet;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(prefix, entriesRet), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(entriesExpect, entriesRet));

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_commit003"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: commit004
  * @tc.desc: Test the nb normal operations in one transaction
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, commit004, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_commit004", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_NATIVE, observer), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Put (key1,value1) and (key2,value2)
     * @tc.expected: step2. put OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_2), OK);
    /**
     * @tc.steps:step3. Delete key2
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_2), OK);
    /**
     * @tc.steps:step4. PutBatch 65 records (from key3 to key67)
     * @tc.expected: step4. return OK.
     */
    vector<Entry> entrysBase;
    vector<Key> keysBase;
    DistributedDBUnitTest::GenerateRecords(BATCH_BASE_SIZE + 7, entrysBase, keysBase);

    vector<Entry> entrys1(entrysBase.begin() + 2, entrysBase.end());
    EXPECT_EQ(entrys1.size(), 65UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys1), OK);
    /**
     * @tc.steps:step5. DeleteBatch 60 records (from key8 to key67)
     * @tc.expected: step5. return OK.
     */
    vector<Key> keys(keysBase.begin() + 7, keysBase.end());
    EXPECT_EQ(keys.size(), 60UL);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys), OK);
    /**
     * @tc.steps:step6. Commit.
     * @tc.expected: step6. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);
    /**
     * @tc.steps:step7. Check observer data.
     * @tc.expected: step6. return OK.
     */
    vector<Entry> entriesRet;
    Entry entry1 = {KEY_1, VALUE_1};
    entriesRet.push_back(entry1);
    entriesRet.insert(entriesRet.end(), entrysBase.begin() + 2, entrysBase.begin() + 7);
    EXPECT_EQ(entriesRet.size(), 6UL);

    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entriesRet, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step8. GetEntries.
     * @tc.expected: step8. return OK.
     */
    const Key prefix;
    vector<Entry> entries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(prefix, entries), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(entriesRet, entries, true));

    // finilize
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_commit004"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: commit005
  * @tc.desc: Test the conflict data report normally in one transaction
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, commit005, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_commit005", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_TRUE(g_kvNbDelegatePtr->SetConflictNotifier(CONFLICT_ALL, NotifierCallback) == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_2), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Put (key1,value3) and (key2,value4)
     * @tc.expected: step2. put OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_3), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_4), OK);
    /**
     * @tc.steps:step3. Delete key2
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_2), OK);
    /**
     * @tc.steps:step4. put (key3 ,value5) (key3 ,value6)
     * @tc.expected: step4. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_3, VALUE_5), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_3, VALUE_6), OK);
    /**
     * @tc.steps:step5. Commit.
     * @tc.expected: step5. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);
    /**
     * @tc.steps:step6. Check conflict report data.
     * @tc.expected: step6. return OK.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_conflictData.size(), 2UL);
    if (g_conflictData.size() == 2) {
        SingleVerConflictData expectNotifyData1 = {KvStoreNbConflictType::CONFLICT_NATIVE_ALL,
            KEY_1, VALUE_1, VALUE_3, false, false, true, true, OK, OK};
        EXPECT_EQ(g_conflictData[0], expectNotifyData1);

        SingleVerConflictData expectNotifyData2 = {KvStoreNbConflictType::CONFLICT_NATIVE_ALL,
            KEY_2, VALUE_2, NULL_VALUE, false, true, true, true, OK, DB_ERROR};
        EXPECT_EQ(g_conflictData[1], expectNotifyData2);
    }

    // finilize
    g_conflictData.clear();
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_commit005"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: commit006
  * @tc.desc: Test the conflict data report and observer function both be normal in one transaction
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, commit006, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_commit006", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_1), OK);

    EXPECT_TRUE(g_kvNbDelegatePtr->SetConflictNotifier(CONFLICT_ALL, NotifierCallback) == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_NATIVE, observer), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Put (key1,value2)
     * @tc.expected: step2. put OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps:step3. Commit.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    /**
     * @tc.steps:step4. Get value of key1.
     * @tc.expected: step4. return OK, value of key1 is value2.
     */
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_TRUE(readValue == VALUE_2);
    /**
     * @tc.steps:step5. Check observer data.
     * @tc.expected: step5. return OK.
     */
    vector<Entry> entriesRet = {{KEY_1, VALUE_2}};
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entriesRet, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step6. Check conflict report data.
     * @tc.expected: step6. return OK.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_conflictData.size(), 1UL);
    if (g_conflictData.size() == 1) {
        SingleVerConflictData expectNotifyData = {KvStoreNbConflictType::CONFLICT_NATIVE_ALL,
            KEY_1, {}, VALUE_2, true, false, true, true, DB_ERROR, OK};
        EXPECT_EQ(g_conflictData[0], expectNotifyData);
    }

    // finilize
    g_conflictData.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_commit006"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: rollback001
  * @tc.desc: Test the transaction rollback without start.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, rollback001, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_rollback001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps:step1. Transaction rollback without start.
     * @tc.expected: step1. return DB_ERROR.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), DB_ERROR);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_rollback001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: rollback002
  * @tc.desc: Test the transaction rollback twice
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, rollback002, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_rollback002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step1. Rollback.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);
    /**
     * @tc.steps:step1. Transaction rollback without start.
     * @tc.expected: step1. return DB_ERROR.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), DB_ERROR);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_rollback002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: rollback003
  * @tc.desc: Test the Put operation rollback
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, rollback003, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_rollback003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Put (key2,value2)
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_2), OK);
    /**
     * @tc.steps:step3. Rollback.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);
    /**
     * @tc.steps:step4. GetEntries.
     * @tc.expected: step4. return OK.
     */
    const Key prefix;
    vector<Entry> entries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(prefix, entries), OK);
    EXPECT_EQ(entries.size(), 1UL);
    if (entries.size() > 0) {
        EXPECT_EQ(entries[0].key, KEY_1);
        EXPECT_EQ(entries[0].value, VALUE_1);
    }

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_rollback003"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: rollback004
  * @tc.desc: Test the PutBatch operation rollback
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, rollback004, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_rollback004", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. PutBatch 10 records
     * @tc.expected: step2. return OK.
     */
    vector<Entry> entrysBase;
    vector<Key> keysBase;
    DistributedDBUnitTest::GenerateRecords(10, entrysBase, keysBase);

    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrysBase), OK);
    /**
     * @tc.steps:step3. Rollback.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);
    /**
     * @tc.steps:step4. GetEntries.
     * @tc.expected: step4. return OK.
     */
    const Key prefix;
    vector<Entry> entries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(prefix, entries), OK);
    EXPECT_EQ(entries.size(), 1UL);
    if (entries.size() > 0) {
        EXPECT_EQ(entries[0].key, KEY_1);
        EXPECT_EQ(entries[0].value, VALUE_1);
    }

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_rollback004"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: rollback005
  * @tc.desc: Test the modify operation rollback
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, rollback005, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_rollback005", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Put (key1,value2)
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps:step3. Rollback.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);
    /**
     * @tc.steps:step4. GetEntries.
     * @tc.expected: step4. return OK.
     */
    const Key prefix;
    vector<Entry> entries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(prefix, entries), OK);
    EXPECT_EQ(entries.size(), 1UL);
    if (entries.size() > 0) {
        EXPECT_EQ(entries[0].key, KEY_1);
        EXPECT_EQ(entries[0].value, VALUE_1);
    }

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_rollback005"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: rollback006
  * @tc.desc: Test the Delete operation rollback
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, rollback006, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_rollback006", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Delete key1
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_1), OK);
    /**
     * @tc.steps:step3. Rollback.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);
    /**
     * @tc.steps:step4. GetEntries.
     * @tc.expected: step4. return OK.
     */
    const Key prefix;
    vector<Entry> entries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(prefix, entries), OK);
    EXPECT_EQ(entries.size(), 1UL);
    if (entries.size() > 0) {
        EXPECT_EQ(entries[0].key, KEY_1);
        EXPECT_EQ(entries[0].value, VALUE_1);
    }

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_rollback006"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: rollback007
  * @tc.desc: Test the DeleteBatch operation rollback
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, rollback007, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_rollback007", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    vector<Entry> entries;
    vector<Key> keys;
    DistributedDBUnitTest::GenerateRecords(10, entries, keys);

    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entries), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. DeleteBatch from key1 to key10
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys), OK);
    /**
     * @tc.steps:step3. Rollback.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);
    /**
     * @tc.steps:step4. GetEntries.
     * @tc.expected: step4. return OK.
     */
    const Key prefix;
    vector<Entry> entriesRet;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(prefix, entriesRet), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(entries, entriesRet));

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_rollback007"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: rollback008
  * @tc.desc: Test the multiple operations rollback
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ9
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBTransactionTest, rollback008, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_transaction_rollback008", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_2), OK);
    /**
     * @tc.steps:step1. Start transaction.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);
    /**
     * @tc.steps:step2. Put (key3,value3) (key1,value4)
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_3, VALUE_3), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_4), OK);
    /**
     * @tc.steps:step3. Delete key2
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_2), OK);
    /**
     * @tc.steps:step4. PutBatch 10 records (from key3 to key12)
     * @tc.expected: step4. return OK.
     */
    vector<Entry> entrysBase;
    vector<Key> keysBase;
    DistributedDBUnitTest::GenerateRecords(12, entrysBase, keysBase);

    vector<Entry> entrys1(entrysBase.begin() + 2, entrysBase.end());
    EXPECT_EQ(entrys1.size(), 10UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entrys1), OK);
    /**
     * @tc.steps:step5. DeleteBatch 5 records (from key3 to key7)
     * @tc.expected: step5. return OK.
     */
    vector<Key> keys(keysBase.begin() + 2, keysBase.begin() + 7);
    EXPECT_EQ(keys.size(), 5UL);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys), OK);
    /**
     * @tc.steps:step6. Commit.
     * @tc.expected: step6. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);
    /**
     * @tc.steps:step7. GetEntries.
     * @tc.expected: step7. return OK.
     */
    const Key prefix;
    vector<Entry> entries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(prefix, entries), OK);
    EXPECT_EQ(entries.size(), 2UL);
    if (entries.size() > 1) {
        EXPECT_EQ(entries[0].key, KEY_1);
        EXPECT_EQ(entries[0].value, VALUE_1);
        EXPECT_EQ(entries[1].key, KEY_2);
        EXPECT_EQ(entries[1].value, VALUE_2);
    }

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_transaction_rollback008"), OK);
    g_kvNbDelegatePtr = nullptr;
}