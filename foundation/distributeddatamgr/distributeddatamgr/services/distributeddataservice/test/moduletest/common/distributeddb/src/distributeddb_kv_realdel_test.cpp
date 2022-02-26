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
#include <ctime>
#include <cmath>
#include <random>
#include <chrono>
#include <fstream>
#include <string>

#include "distributeddb_data_generator.h"
#include "distributed_test_tools.h"
#include "distributeddb_nb_test_tools.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "types.h"

using namespace std;
using namespace chrono;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbKvRealdel {
static std::condition_variable g_kvBackupVar;
class DistributeddbKvRealdelTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

KvStoreDelegate *g_kvBackupDelegate = nullptr; // the delegate used in this suit.
KvStoreDelegateManager *g_manager = nullptr;
void DistributeddbKvRealdelTest::SetUpTestCase(void)
{
}

void DistributeddbKvRealdelTest::TearDownTestCase(void)
{
}

void DistributeddbKvRealdelTest::SetUp(void)
{
    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    KvOption option = g_kvOption;
    g_kvBackupDelegate = DistributedTestTools::GetDelegateSuccess(g_manager, g_kvdbParameter1, option);
    ASSERT_TRUE(g_manager != nullptr && g_kvBackupDelegate != nullptr);
}

void DistributeddbKvRealdelTest::TearDown(void)
{
    MST_LOG("TearDown after case.");
    EXPECT_TRUE(g_manager->CloseKvStore(g_kvBackupDelegate) == OK);
    g_kvBackupDelegate = nullptr;
    DBStatus status = g_manager->DeleteKvStore(STORE_ID_1);
    EXPECT_TRUE(status == DistributedDB::DBStatus::OK) << "fail to delete exist kvdb";
    delete g_manager;
    g_manager = nullptr;
}

/*
 * @tc.name: KvDeleteAll 001
 * @tc.desc: test that delete interface will real delete data.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvRealdelTest, KvDeleteAll001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. put (k1, v1), (k2, v2) to db, and update (k1, v1) to (k1, v2).
     * @tc.expected: step1. put successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_2, VALUE_2), OK);
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps: step2. use sqlite to open the db and check the data of k1.
     * @tc.expected: step2. there is only one data of k1 in db.
     */
    std::string identifier = g_kvdbParameter1.userId + "-" + g_kvdbParameter1.appId + "-" + g_kvdbParameter1.storeId;
    std::string hashIdentifierRes = TransferStringToHashHexString(identifier);
    const std::string kvDbName = DIRECTOR + hashIdentifierRes + KVMULTIDB;
    std::vector<DistributedDB::Key> keyS1, keyS2;
    keyS1.push_back(KEY_1);
    int count = 0;
    EXPECT_TRUE(DistributedTestTools::RepeatCheckAsyncResult([&kvDbName, &count, &keyS1]()->bool {
        DistributedTestTools::GetRecordCntByKey(kvDbName.c_str(), QUERY_SQL, keyS1, g_kvOption, count);
        return count == ONE_RECORD;
    }, 5, 500)); // query 5 times every 500 ms.
    /**
     * @tc.steps: step3. delete k1 and then check data of k1 and k2;
     * @tc.expected: step3. can't find k1, but can find the value of k2 is v2.
     */
    EXPECT_EQ(g_kvBackupDelegate->Delete(KEY_1), OK);
    Value realValue;
    realValue = DistributedTestTools::Get(*g_kvBackupDelegate, KEY_1);
    EXPECT_TRUE(realValue.size() == 0);
    realValue = DistributedTestTools::Get(*g_kvBackupDelegate, KEY_2);
    EXPECT_TRUE(realValue == VALUE_2);
    /**
     * @tc.steps: step4. wait for some seconds and check the data of k1;
     * @tc.expected: step4. can't find k1.
     */
    EXPECT_TRUE(DistributedTestTools::RepeatCheckAsyncResult([&kvDbName, &count, &keyS1]()->bool {
        DistributedTestTools::GetRecordCntByKey(kvDbName.c_str(), QUERY_SQL, keyS1, g_kvOption, count);
        return count == 0;
    }, 5, 500)); // query 5 times every 500 ms.
}

/*
 * @tc.name: KvDeleteAll 002
 * @tc.desc: test that deleteBatch interface will real delete data.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvRealdelTest, KvDeleteAll002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. put (k1, v1), (k2, v2), (k3, v3) to db and use deletebatch interface to delete k1, k2.
     * @tc.expected: step1. put and delete successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_2, VALUE_2), OK);
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_3, VALUE_3), OK);
    std::vector<DistributedDB::Key> keyS;
    keyS.push_back(KEY_1);
    keyS.push_back(KEY_2);
    EXPECT_EQ(g_kvBackupDelegate->DeleteBatch(keyS), OK);
    /**
     * @tc.steps: step2. open the db and check the data of k1, k2, k3.
     * @tc.expected: step2. there is only k3 in db and can't get k1, k2.
     */
    Value realValue;
    realValue = DistributedTestTools::Get(*g_kvBackupDelegate, KEY_1);
    EXPECT_TRUE(realValue.size() == 0);
    realValue = DistributedTestTools::Get(*g_kvBackupDelegate, KEY_2);
    EXPECT_TRUE(realValue.size() == 0);
    realValue = DistributedTestTools::Get(*g_kvBackupDelegate, KEY_3);
    EXPECT_TRUE(realValue == VALUE_3);
    /**
     * @tc.steps: step3. wait for some seconds and check the data of k1, k2 by sqlite;
     * @tc.expected: step3. can't find k1, k2.
     */
    std::string identifier = g_kvdbParameter1.userId + "-" + g_kvdbParameter1.appId + "-" + g_kvdbParameter1.storeId;
    std::string hashIdentifierRes = TransferStringToHashHexString(identifier);
    const std::string kvDbName = DIRECTOR + hashIdentifierRes + KVMULTIDB;
    int count = 0;
    EXPECT_TRUE(DistributedTestTools::RepeatCheckAsyncResult([&kvDbName, &count, &keyS]()->bool {
        DistributedTestTools::GetRecordCntByKey(kvDbName.c_str(), MULTI_KEY_QUERY_SQL, keyS, g_kvOption, count);
        return count == 0;
    }, 5, 500)); // query 5 times every 500 ms.
}

/*
 * @tc.name: KvDeleteAll 003
 * @tc.desc: test that clear interface will real delete data.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvRealdelTest, KvDeleteAll003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. put (k1, v1), (k2, v2) to db and update (k1, v1) to (k1, v2).
     * @tc.expected: step1. put and update successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_2, VALUE_2), OK);
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps: step2. clear the db, and check the data of k1, k2.
     * @tc.expected: step2. clear and can't find k1, k2.
     */
    EXPECT_EQ(g_kvBackupDelegate->Clear(), OK);
    Value realValue;
    realValue = DistributedTestTools::Get(*g_kvBackupDelegate, KEY_1);
    EXPECT_TRUE(realValue.size() == 0);
    realValue = DistributedTestTools::Get(*g_kvBackupDelegate, KEY_2);
    EXPECT_TRUE(realValue.size() == 0);

    /**
     * @tc.steps: step3. wait for 5 seconds and check the data of k1, k2;
     * @tc.expected: step3. can't find k1, k2.
     */
    std::string identifier = g_kvdbParameter1.userId + "-" + g_kvdbParameter1.appId + "-" + g_kvdbParameter1.storeId;
    std::string hashIdentifierRes = TransferStringToHashHexString(identifier);
    const std::string kvDbName = DIRECTOR + hashIdentifierRes + KVMULTIDB;
    std::vector<DistributedDB::Key> keyS;
    keyS.push_back(KEY_1);
    keyS.push_back(KEY_2);
    int count = 0;
    EXPECT_TRUE(DistributedTestTools::RepeatCheckAsyncResult([&kvDbName, &count, &keyS]()->bool {
        DistributedTestTools::GetRecordCntByKey(kvDbName.c_str(), MULTI_KEY_QUERY_SQL, keyS, g_kvOption, count);
        return count == 0;
    }, 5, 500)); // query 5 times every 500 ms.

    EncrypteAttribute attribute = {g_kvOption.isEncryptedDb, g_kvOption.passwd};
    EXPECT_TRUE(DistributedTestTools::RepeatCheckAsyncResult([&kvDbName, &attribute, &count]()->bool {
        DistributedTestTools::QuerySpecifiedData(kvDbName, SYNC_MULTI_VER_QUERY_SQL, attribute, count);
        return count == ONE_RECORD;
    }, 5, 500)); // query 5 times every 500 ms.
}

/*
 * @tc.name: KvDeleteAll 004
 * @tc.desc: test that it can get record when snapshot is register, and can't get records after unregister snapshor.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvRealdelTest, KvDeleteAll004, TestSize.Level2)
{
    /**
     * @tc.steps: step1. put (k1, v1), update (k1, v1) to (k1, v2).
     * @tc.expected: step1. put and update successfully.
     */
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvBackupDelegate->Put(KEY_1, VALUE_2), OK);
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapShot = DistributedTestTools::RegisterSnapObserver(g_kvBackupDelegate, &observer);
    EXPECT_NE(snapShot, nullptr);
    /**
     * @tc.steps: step2. delete k1 and then check data of k1;
     * @tc.expected: step2. there is only one records in the db.
     */
    EXPECT_EQ(g_kvBackupDelegate->Delete(KEY_1), OK);
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND));
    std::string identifier = g_kvdbParameter1.userId + "-" + g_kvdbParameter1.appId + "-" + g_kvdbParameter1.storeId;
    std::string hashIdentifierRes = TransferStringToHashHexString(identifier);
    const std::string kvDbName = DIRECTOR + hashIdentifierRes + KVMULTIDB;

    std::vector<DistributedDB::Key> keyS;
    keyS.push_back(KEY_1);
    int count = 0;
    DistributedTestTools::GetRecordCntByKey(kvDbName.c_str(), QUERY_SQL, keyS, g_kvOption, count);
    EXPECT_EQ(count, ONE_RECORD);

    /**
     * @tc.steps: step3. after release the snap shot query the records;
     * @tc.expected: step3. can't find the k1 in db.
     */
    EXPECT_EQ(g_kvBackupDelegate->ReleaseKvStoreSnapshot(snapShot), OK);
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND));
    DistributedTestTools::GetRecordCntByKey(kvDbName.c_str(), QUERY_SQL, keyS, g_kvOption, count);
    EXPECT_TRUE(count == 0);
}

/*
 * @tc.name: DeleteAll 006
 * @tc.desc: value slices will be real deleted if reference count is 0 caused by clear.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvRealdelTest, KvDeleteAll006, TestSize.Level2)
{
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    delegate2 = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter2, g_kvOption);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);
    /**
     * @tc.steps: step1. put (k1,v1) and (k2,v2), each value of them are 4M.
     * @tc.expected: step1. call successfully.
     */
    EntrySize entrySize = { KEY_SIX_BYTE, FOUR_M_LONG_STRING };
    std::vector<DistributedDB::Entry> entries;
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, RECORDS_SMALL_CNT, { 'K' }, { 'v' });
    for (auto entry : entries) {
        EXPECT_EQ(delegate2->Put(entry.key, entry.value), OK);
    }
    /**
     * @tc.steps: step2. clear and verify the records of ValueSlice by sqlite interface.
     * @tc.expected: step1. count is 0.
     */
    EXPECT_EQ(delegate2->Clear(), OK);
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND));
    std::string identifier = g_kvdbParameter2.userId + "-" + g_kvdbParameter2.appId +
        "-" + g_kvdbParameter2.storeId;
    std::string hashIdentifierRes = TransferStringToHashHexString(identifier);
    const std::string dbName = DIRECTOR + hashIdentifierRes + MULTIDB;
    int count = 0;
    EncrypteAttribute attribute = {g_kvOption.isEncryptedDb, g_kvOption.passwd};
    EXPECT_TRUE(DistributedTestTools::QuerySpecifiedData(dbName, SYNC_VALUE_SLICE_QUERY_SQL, attribute, count));
    EXPECT_EQ(count, 0); // there are no ValueSlices in db.

    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    EXPECT_EQ(manager2->DeleteKvStore(STORE_ID_2), OK);
    delete manager2;
    manager2 = nullptr;
}

/*
 * @tc.name: DeleteAll 007
 * @tc.desc: value slices will be real deleted if reference count is 0 caused by clear.
 * @tc.type: FUNC
 * @tc.require: SR000D4878
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvRealdelTest, KvDeleteAll007, TestSize.Level2)
{
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    delegate2 = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter2, g_kvOption);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);
    /**
     * @tc.steps: step1. put (k1,v1) and (k2,v1), each value of them are 4M.
     * @tc.expected: step1. call successfully.
     */
    EntrySize entrySize = { KEY_SIX_BYTE, FOUR_M_LONG_STRING };
    std::vector<DistributedDB::Entry> entries;
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, RECORDS_SMALL_CNT, { 'K' }, { 'v' });
    entries[INDEX_FIRST].value = entries[INDEX_ZEROTH].value;
    for (auto entry : entries) {
        EXPECT_EQ(delegate2->Put(entry.key, entry.value), OK);
    }
    /**
     * @tc.steps: step2. delete k1 and verify the records of ValueSlice by sqlite interface.
     * @tc.expected: step1. count is not equal to 0.
     */
    std::string identifier = g_kvdbParameter2.userId + "-" + g_kvdbParameter2.appId +
        "-" + g_kvdbParameter2.storeId;
    std::string hashIdentifierRes = TransferStringToHashHexString(identifier);
    const std::string dbName = DIRECTOR + hashIdentifierRes + MULTIDB;
    EXPECT_EQ(delegate2->Delete(entries[INDEX_ZEROTH].key), OK);
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND));
    int count = 0;
    EncrypteAttribute attribute = {g_kvOption.isEncryptedDb, g_kvOption.passwd};
    EXPECT_TRUE(DistributedTestTools::QuerySpecifiedData(dbName, SYNC_VALUE_SLICE_QUERY_SQL, attribute, count));
    EXPECT_EQ(count, 0); // there are some ValueSlices in db.
    /**
     * @tc.steps: step3. delete k2 and verify the records of ValueSlice by sqlite interface.
     * @tc.expected: step1. count is 0.
     */
    EXPECT_EQ(delegate2->Delete(entries[INDEX_FIRST].key), OK);
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND));
    count = 0;
    EXPECT_TRUE(DistributedTestTools::QuerySpecifiedData(dbName, SYNC_VALUE_SLICE_QUERY_SQL, attribute, count));
    EXPECT_EQ(count, 0); // there are no ValueSlices in db.

    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    EXPECT_EQ(manager2->DeleteKvStore(STORE_ID_2), OK);
    delete manager2;
    manager2 = nullptr;
}
}