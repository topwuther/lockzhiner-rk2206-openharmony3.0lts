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
#include <string>

#include "distributeddb_data_generator.h"
#include "distributed_test_tools.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "types.h"

using namespace std;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace std::placeholders;
using namespace DistributedDBDataGenerator;

namespace DistributeddbKvBatchCrud {
class DistributeddbKvBatchCrudTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

const bool IS_LOCAL = false;
KvStoreDelegate *g_kvStoreBatchDelegate = nullptr; // the delegate used in this suit.
KvStoreDelegateManager *g_batchManager = nullptr;
void DistributeddbKvBatchCrudTest::SetUpTestCase(void)
{
    MST_LOG("SetUpTestCase before all cases local[%d].", IS_LOCAL);
    RemoveDir(DIRECTOR);
    KvOption option = g_kvOption;
    option.localOnly = IS_LOCAL;
    g_kvStoreBatchDelegate = DistributedTestTools::GetDelegateSuccess(g_batchManager,
        g_kvdbParameter1, option);
    ASSERT_TRUE(g_batchManager != nullptr && g_kvStoreBatchDelegate != nullptr);
}

void DistributeddbKvBatchCrudTest::TearDownTestCase(void)
{
    MST_LOG("TearDownTestCase after all cases.");
    EXPECT_EQ(g_batchManager->CloseKvStore(g_kvStoreBatchDelegate), OK);
    g_kvStoreBatchDelegate = nullptr;
    DBStatus status = g_batchManager->DeleteKvStore(STORE_ID_1);
    EXPECT_EQ(status, OK) << "fail to delete exist kvdb";
    delete g_batchManager;
    g_batchManager = nullptr;
    RemoveDir(DIRECTOR);
}

void DistributeddbKvBatchCrudTest::SetUp(void)
{
    ASSERT_TRUE(g_kvStoreBatchDelegate != nullptr);
    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());
}

void DistributeddbKvBatchCrudTest::TearDown(void)
{
}

class KvStoreObserverSnapImpl final : public KvStoreObserver {
public:
    void OnChange(const KvStoreChangedData &data)
    {
        changed_++;
        onChangeTime_ = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
        MST_LOG("comes a change,changed[%ld]!!!", changed_);
        MST_LOG("GetEntriesInserted().size() = %zu, GetEntriesUpdated() = %zu, GetEntriesDeleted() = %zu",
            data.GetEntriesInserted().size(), data.GetEntriesUpdated().size(), data.GetEntriesDeleted().size());
        insertCrudList_.assign(data.GetEntriesInserted().begin(), data.GetEntriesInserted().end());
        updateCrudList_.assign(data.GetEntriesUpdated().begin(), data.GetEntriesUpdated().end());
        deleteCrudList_.assign(data.GetEntriesDeleted().begin(), data.GetEntriesDeleted().end());
    }

    KvStoreObserverSnapImpl()
    {
        insertCrudList_.clear();
        updateCrudList_.clear();
        deleteCrudList_.clear();
        changed_ = 0;
        onChangeTime_ = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    }

    ~KvStoreObserverSnapImpl()
    {
        changed_ = 0;
    }

    long GetChanged()
    {
        return changed_;
    }

    const list<Entry> GetInsertList()
    {
        return insertCrudList_;
    }

    const list<Entry> GetDeleteList()
    {
        return deleteCrudList_;
    }

    const list<Entry> GetUpdateList()
    {
        return updateCrudList_;
    }

    microClock_type GetOnChangeTime()
    {
        return onChangeTime_;
    }

    void Clear()
    {
        insertCrudList_.clear();
        deleteCrudList_.clear();
        updateCrudList_.clear();
        changed_ = 0;
    }

private:
    list<Entry> insertCrudList_ = {};
    list<Entry> deleteCrudList_ = {};
    list<Entry> updateCrudList_ = {};
    long changed_ = 0;
    microClock_type onChangeTime_
        = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
};

/*
 * @tc.name: SimpleData 001
 * @tc.desc: Verify that can PutBatch(keys,values) and get.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData001, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);

    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries2;
    entries2.push_back(ENTRY_3);

    /**
     * @tc.steps: step1. create kv db and PutBatch (k1,v1)(k2,v2) then get.
     * @tc.expected: step1. PutBatch successfully and get the value of k1 is v1,get the value of k2 is v2.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1);
    EXPECT_EQ(status, OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));

    /**
     * @tc.steps: step2. create kv db and PutBatch (k3,v3) then get.
     * @tc.expected: step2. PutBatch successfully and get the value of k3 is v3.
     */
    DBStatus status2 = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries2);
    EXPECT_EQ(status2, OK);
    Value valueResult2 = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_3);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult2, VALUE_3));

    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
}

/*
 * @tc.name: SimpleData 002
 * @tc.desc: Verify that can PutBatch(keys,values) and update them.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData002, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);

    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries1Up;
    entries1Up.push_back(ENTRY_1_2);
    entries1Up.push_back(ENTRY_2_3);

    /**
     * @tc.steps: step1. create kv db and PutBatch (k1,v1)(k2,v2) then get.
     * @tc.expected: step1. PutBatch successfully and get the value of k1 is v1,get the value of k2 is v2.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1);
    EXPECT_EQ(status, OK);

    Value valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));

    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));

    /**
     * @tc.steps: step2. update (k1,v1)(k2,v2) to (k1,v2)(k2,v3).
     * @tc.expected: step2. update successfully and get the value of k1 is v2,get the value of k2 is v3.
     */
    status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1Up);
    EXPECT_EQ(status, DBStatus::OK);

    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));

    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_3));

    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
}

/*
 * @tc.name: SimpleData 003
 * @tc.desc: Verify that can deletebatch list.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData003, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    /**
     * @tc.steps: step1. create kv db and PutBatch (k1,v1)(k2,v2) then get.
     * @tc.expected: step1. PutBatch successfully and get the value of k1 is v1,get the value of k2 is v2.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1);
    ASSERT_EQ(status, DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));

    /**
     * @tc.steps: step2. deleteBatch (k1)(k2) then get.
     * @tc.expected: step2. deleteBatch successfully and get the value of k1 is null,get the value of k2 is null.
     */
    vector<Key> keys1;
    keys1.push_back(ENTRY_1.key);
    keys1.push_back(ENTRY_2.key);

    DBStatus status2 = DistributedTestTools::DeleteBatch(*g_kvStoreBatchDelegate, keys1);

    ASSERT_EQ(status2, DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_EQ(valueResult.size(), size_t(0));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_EQ(valueResult.size(), size_t(0));
}

/*
 * @tc.name: SimpleData 004
 * @tc.desc: Verify that can put batch with some null value.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData004, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);

    vector<Entry> entries1;
    entries1.push_back(ENTRY_1_NULL); // KEY_1 with null value.
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. create kv db and PutBatch (k1,v1)(k2,v2) that v1=null.
     * @tc.expected: step1. PutBatch successfully.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1);
    ASSERT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. get k1,k2 from db.
     * @tc.expected: step2. get the value of k1 is null,get the value of k2 is not null.
     */
    Value valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_EQ(valueResult.size(), size_t(0));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_NE(valueResult.size(), size_t(0));
}

/*
 * @tc.name: SimpleData 005
 * @tc.desc: Verify that can not put batch with some null key.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData005, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);

    vector<Entry> entries1;
    entries1.push_back(ENTRY_NULL_1); // null key with VALUE_1.
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. create kv db and PutBatch (k1,v1)(k2,v2) that k1=null.
     * @tc.expected: step1. PutBatch failed.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1);
    ASSERT_TRUE(status != DBStatus::OK);

    /**
     * @tc.steps: step2. get k1,k2 from db.
     * @tc.expected: step2. get the value of k1 is null,get the value of k2 is null.
     */
    Value valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_EQ(valueResult.size(), size_t(0));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_EQ(valueResult.size(), size_t(0));
}

/*
 * @tc.name: SimpleData 006
 * @tc.desc: Verify that get batch none exist data will get null.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData006, TestSize.Level0)
{
    /**
     * @tc.steps: step1. clear kv db.
     * @tc.expected: step1. Construct that none exist data.
     */
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);

    /**
     * @tc.steps: step2. get batch(k1)(k2).
     * @tc.expected: step2. get null.
     */
    vector<Entry> valueResult = DistributedTestTools::GetEntries(*g_kvStoreBatchDelegate, KEY_SEARCH);
    ASSERT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: SimpleData 007
 * @tc.desc: Verify that delete batch none exist data will return OK.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData007, TestSize.Level0)
{
    /**
     * @tc.steps: step1. clear kv db.
     * @tc.expected: step1. Construct that none exist data.
     */
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);

    vector<Key> keys1;
    keys1.push_back(ENTRY_1.key);
    keys1.push_back(ENTRY_2.key);

    /**
     * @tc.steps: step2. delete batch none exist data.
     * @tc.expected: step2. delete failed but return OK.
     */
    DBStatus status = DistributedTestTools::DeleteBatch(*g_kvStoreBatchDelegate, keys1);
    ASSERT_TRUE(status == DBStatus::OK);
}

/*
 * @tc.name: SimpleData 008
 * @tc.desc: Verify that can get entries with prefix-key.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData008, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_A_1);
    entries1.push_back(ENTRY_A_2);
    entries1.push_back(ENTRY_A_3);
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1);
    ASSERT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step1. get entries that prefix-key="ab".
     * @tc.expected: step1. get 3 records which are key="abc","abcdasd","abcds".
     */
    vector<Entry> valueResult = DistributedTestTools::GetEntries(*g_kvStoreBatchDelegate, KEY_SEARCH);
    MST_LOG("value size %zu", valueResult.size());
    EXPECT_EQ(valueResult.size(), THREE_RECORDS);
    /**
     * @tc.steps: step2. get entries that prefix-key="abcde".
     * @tc.expected: step2. get 0 record.
     */
    valueResult = DistributedTestTools::GetEntries(*g_kvStoreBatchDelegate, KEY_SEARCH_2);
    EXPECT_EQ(valueResult.size(), size_t(0));
}

/*
 * @tc.name: SimpleData 009
 * @tc.desc: Verify that get entries with null key will return all records.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData009, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_A_1);
    entries1.push_back(ENTRY_A_2);
    entries1.push_back(ENTRY_A_3);
    /**
     * @tc.steps: step1. PutBatch (k1=abc,v1=a1)(k2=abcdasd,v2=a2)(k3=abcds,v3=a3).
     * @tc.expected: step1. PutBatch successfully to construct exist data.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1);
    ASSERT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. GetEntries with key=null.
     * @tc.expected: step2. GetEntries successfully and return all records in db.
     */
    vector<Entry> valueResult = DistributedTestTools::GetEntries(*g_kvStoreBatchDelegate, KEY_EMPTY);
    ASSERT_TRUE(valueResult.size() == entries1.size());
}

/*
 * @tc.name: SimpleData 010
 * @tc.desc: Verify that can release snapshot.
 * @tc.type: FUN
 * @tc.require: SR000CQDTF
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, SimpleData010, TestSize.Level0)
{
    DelegateCallback delegateCallback;
    function<void(DBStatus, KvStoreSnapshotDelegate *)> function
        = bind(&DelegateCallback::Callback, &delegateCallback, _1, _2);

    g_kvStoreBatchDelegate->GetKvStoreSnapshot(nullptr, function);
    DBStatus status = delegateCallback.GetStatus();
    EXPECT_EQ(status, DBStatus::OK);
    KvStoreSnapshotDelegate *snapshot
        = const_cast<KvStoreSnapshotDelegate *>(delegateCallback.GetKvStoreSnapshot());
    EXPECT_NE(snapshot, nullptr);
    /**
     * @tc.steps: step1. release snapshot.
     * @tc.expected: step1. release snapshot successfully.
     */
    EXPECT_EQ(g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snapshot), OK);
}

bool PutBatchTwoRecords(vector<Entry> &entries)
{
    bool result = true;
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries);
    result = result && (status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, entries[0].key);
    result = result && (valueResult.size() != 0);
    if (!result) {
        MST_LOG("value.size() of entries[0].key is 0, and failed");
    }
    result = result && (DistributedTestTools::IsValueEquals(valueResult, entries[0].value));
    if (!result) {
        MST_LOG("value.size() Got of entries[0].key is not equal to the value entries[0].key Expected");
    }
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, entries[1].key);
    result = result && (valueResult.size() != 0);
    if (!result) {
        MST_LOG("value.size() of entries[1].key is 0, and failed");
    }
    result = result && (DistributedTestTools::IsValueEquals(valueResult, entries[1].value));
    if (!result) {
        MST_LOG("value.size() Got of entries[1].key is not equal to the value entries[1].key Expected");
    }
    return result;
}

/*
 * @tc.name: ComplexData 001
 * @tc.desc: Verify that can batch crud for same keys repeatedly.
 * @tc.type: FUN
 * @tc.require: SR000CQDTF
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, ComplexData001, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);

    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries12;
    entries12.push_back(ENTRY_1_2);
    entries12.push_back(ENTRY_2_3);

    /**
     * @tc.steps: step1. PutBatch (k1,v1)(k2,v2) and then get.
     * @tc.expected: step1. PutBatch successfully get v1 of k1,v2 of k2.
     */
    EXPECT_TRUE(PutBatchTwoRecords(entries1));

    /**
     * @tc.steps: step2. DeleteBatch (k1)(k2) and then get.
     * @tc.expected: step2. DeleteBatch successfully get null of k1,k2.
     */
    DBStatus status2 = DistributedTestTools::DeleteBatch(*g_kvStoreBatchDelegate, KEYS_1);
    ASSERT_EQ(status2, DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_EQ(valueResult.size(), size_t(0));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_EQ(valueResult.size(), size_t(0));

    /**
     * @tc.steps: step3. PutBatch (k1,v1)(k2,v2) and then get.
     * @tc.expected: step3. PutBatch successfully get v1,v2 of k1,k2.
     */
    EXPECT_TRUE(PutBatchTwoRecords(entries1));

    /**
     * @tc.steps: step4. PutBatch (k1,v2)(k2,v3) and then get.
     * @tc.expected: step4. PutBatch successfully get v2 of k1,v3 of k2.
     */
    EXPECT_TRUE(PutBatchTwoRecords(entries12));

    /**
     * @tc.steps: step5. DeleteBatch (k1)(k2) and then get.
     * @tc.expected: step5. DeleteBatch successfully get null of k1,k2.
     */
    status2 = DistributedTestTools::DeleteBatch(*g_kvStoreBatchDelegate, KEYS_1);
    EXPECT_EQ(status2, DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_EQ(valueResult.size(), size_t(0));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_EQ(valueResult.size(), size_t(0));
}

void CheckBatchCrud(vector<Key> keys23)
{
    /**
     * @tc.steps: step3. Put(k1,v3) Put(k2,v4) Put(k3,v3) and then get.
     * @tc.expected: step3. Put successfully get v3 of k1,v4 of k2,v3 of k3.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreBatchDelegate, ENTRY_1_3.key, ENTRY_1_3.value);
    ASSERT_EQ(status, DBStatus::OK);
    status = DistributedTestTools::Put(*g_kvStoreBatchDelegate, ENTRY_2_4.key, ENTRY_2_4.value);
    ASSERT_EQ(status, DBStatus::OK);
    status = DistributedTestTools::Put(*g_kvStoreBatchDelegate, ENTRY_3.key, ENTRY_3.value);
    ASSERT_EQ(status, DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_3));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_4));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_3);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_3));
    /**
     * @tc.steps: step4. DeleteBatch (k2)(k3) and then get.
     * @tc.expected: step4. DeleteBatch successfully get not null of k1,null of k2,k3.
     */
    DBStatus status2 = DistributedTestTools::DeleteBatch(*g_kvStoreBatchDelegate, keys23);
    ASSERT_TRUE(status2 == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_NE(valueResult.size(), size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_3));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_TRUE(valueResult.size() == size_t(0));
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_3);
    EXPECT_TRUE(valueResult.size() == size_t(0));
    /**
     * @tc.steps: step5. clear all and then Put(k1,v1) then get.
     * @tc.expected: step5. clear and Put successfully, get v1 of k1.
     */
    status2 = DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
    ASSERT_TRUE(status2 == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == size_t(0));
    status = DistributedTestTools::Put(*g_kvStoreBatchDelegate, ENTRY_1.key, ENTRY_1.value);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != size_t(0));
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
}

/*
 * @tc.name: ComplexData 002
 * @tc.desc: Verify that can batch and single crud continuously.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, ComplexData002, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);

    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries12;
    entries12.push_back(ENTRY_1_2);
    entries12.push_back(ENTRY_2_3);
    vector<Entry> entries23;
    entries23.push_back(ENTRY_2);
    entries23.push_back(ENTRY_3);
    vector<Key> keys23;
    keys23.push_back(ENTRY_2.key);
    keys23.push_back(ENTRY_3.key);

    /**
     * @tc.steps: step1. PutBatch (k1,v1)(k2,v2) and then get.
     * @tc.expected: step1. PutBatch successfully get v1 of k1,v2 of k2.
     */
    EXPECT_TRUE(PutBatchTwoRecords(entries1));

    /**
     * @tc.steps: step2. delete k1 and then get.
     * @tc.expected: step2. delete successfully get null of k1,v2 of k2.
     */
    DBStatus status2 = DistributedTestTools::Delete(*g_kvStoreBatchDelegate, KEY_1);
    ASSERT_TRUE(status2 == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
    valueResult = DistributedTestTools::Get(*g_kvStoreBatchDelegate, KEY_2);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));

    CheckBatchCrud(keys23);
}

vector<Entry> MultiSnapCheck1(vector<Entry> &entries1, vector<Entry> &entries2, DistributedDB::Key &keyPrefix,
    KvStoreObserverSnapImpl &observer3, KvStoreObserverSnapImpl &observer4)
{
    /**
     * @tc.steps: step3. Register snap2 then getEntries with keyPrefix="k".
     * @tc.expected: step3. operate successfully and getEntries are null.
     */
    KvStoreObserverSnapImpl observer2;
    KvStoreSnapshotCallback kvStoreSnapshotCallback2;
    function<void(DBStatus, const std::vector<Entry>&)> function2
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback2, _1, _2);
    KvStoreSnapshotDelegate *snap2 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer2);
    EXPECT_NE(snap2, nullptr);
    snap2->GetEntries(keyPrefix, function2);
    vector<Entry> resultEntries2 = kvStoreSnapshotCallback2.GetEntries();
    vector<Entry> resultEmptyEntry;
    EXPECT_TRUE(CompareEntriesVector(resultEntries2, resultEmptyEntry));
    /**
     * @tc.steps: step4. putBatch (k1,v1)(k2,v2) again and Register snap3 then getEntries with keyPrefix="k".
     * @tc.expected: step4. operate successfully and getEntries are (k1,v1)(k2,v2).
     */
    vector<Entry> entriesInDB;
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1);
    EXPECT_TRUE(status == DBStatus::OK);
    for (auto eachEntry : entries1) {
        entriesInDB.push_back(eachEntry);
    }
    KvStoreSnapshotCallback kvStoreSnapshotCallback3;
    function<void(DBStatus, const std::vector<Entry>&)> function3
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback3, _1, _2);
    KvStoreSnapshotDelegate *snap3 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer3);
    EXPECT_NE(snap3, nullptr);
    snap3->GetEntries(keyPrefix, function3);
    vector<Entry> resultEntries3 = kvStoreSnapshotCallback3.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries3, entries1));
    /**
     * @tc.steps: step5. putBatch (k1,v2)(k2,v3) again, Register snap4 then getEntries with keyPrefix="k".
     * @tc.expected: step5. operate successfully and getEntries are (k1,v2)(k2,v3).
     */
    status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries2);
    for (auto eachEntry : entries2) {
        PutUniqueKey(entriesInDB, eachEntry.key, eachEntry.value);
    }
    KvStoreSnapshotCallback kvStoreSnapshotCallback4;
    function<void(DBStatus, const std::vector<Entry>&)> function4
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback4, _1, _2);
    KvStoreSnapshotDelegate *snap4 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer4);
    EXPECT_NE(snap4, nullptr);
    snap4->GetEntries(keyPrefix, function4);
    vector<Entry> resultEntries4 = kvStoreSnapshotCallback4.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries4, entries2));
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap2);
    EXPECT_TRUE(status == DBStatus::OK);
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap3);
    EXPECT_TRUE(status == DBStatus::OK);
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap4);
    EXPECT_TRUE(status == DBStatus::OK);
    return entriesInDB;
}

void SnapWithTransactionCheck(vector<Key> keys1, DistributedDB::Key keyPrefix,
    KvStoreObserverSnapImpl observer5, vector<Entry> entriesInDB)
{
    /**
     * @tc.steps: step6. StartTransaction then deleteBatch (k1,v2)(k2,v3).
     * @tc.expected: step6. operate successfully to construct there is no data in db.
     */
    KvStoreSnapshotCallback kvStoreSnapshotCallback5;
    function<void(DBStatus, const std::vector<Entry>&)> function5
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback5, _1, _2);

    DBStatus statusStart = g_kvStoreBatchDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus status = DistributedTestTools::DeleteBatch(*g_kvStoreBatchDelegate, keys1);
    ASSERT_TRUE(status == DBStatus::OK);
    for (unsigned long idxKey = 0; idxKey < keys1.size(); ++idxKey) {
        for (unsigned long idxEntry = 0; idxEntry < entriesInDB.size(); ++idxEntry) {
            if (CompareVector(keys1[idxKey], entriesInDB[idxEntry].key)) {
                entriesInDB.erase(entriesInDB.begin() + idxEntry);
            }
        }
    }
    /**
     * @tc.steps: step7. put(k1,v1)(k2,v3) alone and commit, Register snap5 to getEntries with prefix-key='k'.
     * @tc.expected: step7. operate successfully and getEntries are (k1,v1)(k2,v3).
     */
    status = DistributedTestTools::Put(*g_kvStoreBatchDelegate, KEY_1, VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    PutUniqueKey(entriesInDB, KEY_1, VALUE_1);
    status = DistributedTestTools::Put(*g_kvStoreBatchDelegate, KEY_2, VALUE_2);
    ASSERT_TRUE(status == DBStatus::OK);
    PutUniqueKey(entriesInDB, KEY_2, VALUE_2);
    status = DistributedTestTools::Put(*g_kvStoreBatchDelegate, KEY_3, VALUE_3);
    ASSERT_TRUE(status == DBStatus::OK);
    PutUniqueKey(entriesInDB, KEY_3, VALUE_3);
    status = DistributedTestTools::Put(*g_kvStoreBatchDelegate, KEY_2, VALUE_3);
    ASSERT_TRUE(status == DBStatus::OK);
    PutUniqueKey(entriesInDB, KEY_2, VALUE_3);
    status = DistributedTestTools::Delete(*g_kvStoreBatchDelegate, KEY_3);
    ASSERT_TRUE(status == DBStatus::OK);
    for (unsigned long idx = 0; idx < entriesInDB.size(); ++idx) {
        if (CompareVector(entriesInDB[idx].key, KEY_3)) {
            entriesInDB.erase(entriesInDB.begin() + idx);
        }
    }
    DBStatus statusCommit = g_kvStoreBatchDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    KvStoreSnapshotDelegate *snap5 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer5);
    EXPECT_NE(snap5, nullptr);
    snap5->GetEntries(keyPrefix, function5);
    vector<Entry> resultEntries5 = kvStoreSnapshotCallback5.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries5, entriesInDB));
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap5);
    ASSERT_TRUE(status == DBStatus::OK);
}

/*
 * @tc.name: ComplexSnap 001
 * @tc.desc: Verify that can read multiple snapshots correctly after crud repeatedly.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, ComplexSnap001, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries2;
    entries2.push_back(ENTRY_1_2);
    entries2.push_back(ENTRY_2_3);
    KvStoreObserverSnapImpl observer1;
    KvStoreObserverSnapImpl observer3;
    KvStoreObserverSnapImpl observer4;
    KvStoreObserverSnapImpl observer5;
    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step1. putBatch successfully to construct data in db.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries1);
    ASSERT_TRUE(status == DBStatus::OK);
    KvStoreSnapshotCallback kvStoreSnapshotCallback;
    function<void(DBStatus, const std::vector<Entry>&)> function1
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback, _1, _2);
    /**
     * @tc.steps: step2. Register snap1 then getEntries with keyPrefix="k" and then delete.
     * @tc.expected: step2. operate successfully and getEntries are (k1,v1)(k2,v2).
     */
    KvStoreSnapshotDelegate *snap1 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer1);
    EXPECT_NE(snap1, nullptr);
    vector<Key> keys1;
    for (unsigned long time = 0; time < entries1.size(); ++time) {
        keys1.push_back(entries1.at(time).key);
    }
    DistributedDB::Key keyPrefix = { 'k' };
    snap1->GetEntries(keyPrefix, function1);
    vector<Entry> resultEntries = kvStoreSnapshotCallback.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries, entries1));
    status = DistributedTestTools::DeleteBatch(*g_kvStoreBatchDelegate, keys1);
    ASSERT_TRUE(status == DBStatus::OK);
    vector<Entry> entriesInDB = MultiSnapCheck1(entries1, entries2, keyPrefix, observer3, observer4);
    SnapWithTransactionCheck(keys1, keyPrefix, observer5, entriesInDB);

    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap1);
    ASSERT_TRUE(status == DBStatus::OK);
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
}

void RegisterSnapAgainAndCheck1(KvStoreObserverSnapImpl &observer1, KvStoreSnapshotDelegate *&snap1,
    vector<Entry> &entries, DistributedDB::Key &keyPrefix)
{
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries);
    ASSERT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step2. ReleaseKvStoreSnapshot then RegisterSnapObserver snap1 and getEntries with keyPrefix="k".
     * @tc.expected: step2. operate successfully and getEntries are (k1,v1)(k2,v2).
     */
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap1);
    ASSERT_TRUE(status == DBStatus::OK);
    snap1 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer1);
    EXPECT_NE(snap1, nullptr);
    KvStoreSnapshotCallback kvStoreSnapshotCallback1;
    function<void(DBStatus, const std::vector<Entry>&)> function1
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback1, _1, _2);

    snap1->GetEntries(keyPrefix, function1);
    vector<Entry> resultEntries = kvStoreSnapshotCallback1.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries, entries));
}

vector<Key> RegisterSnapAgainAndCheck2(KvStoreObserverSnapImpl &observer2, KvStoreSnapshotDelegate *&snap2,
    vector<Entry> &entries, DistributedDB::Key &keyPrefix)
{
    /**
     * @tc.steps: step3. deleteBatch and RegisterSnapObserver snap2 then getEntries with keyPrefix="k".
     * @tc.expected: step3. operate successfully and getEntries are null.
     */
    vector<Key> keys1;
    for (unsigned long time = 0; time < entries.size(); ++time) {
        keys1.push_back(entries.at(time).key);
    }
    DBStatus status = DistributedTestTools::DeleteBatch(*g_kvStoreBatchDelegate, keys1);
    EXPECT_TRUE(status == DBStatus::OK);
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap2);
    EXPECT_TRUE(status == DBStatus::OK);
    snap2 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer2);
    EXPECT_NE(snap2, nullptr);
    KvStoreSnapshotCallback kvStoreSnapshotCallback2;
    function<void(DBStatus, const std::vector<Entry>&)> function2
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback2, _1, _2);
    snap2->GetEntries(keyPrefix, function2);
    vector<Entry> resultEntries2 = kvStoreSnapshotCallback2.GetEntries();
    vector<Entry> resultEmptyEntry;
    EXPECT_TRUE(CompareEntriesVector(resultEntries2, resultEmptyEntry));
    return keys1;
}

vector<Entry> RegisterSnapAgainAndCheck3(KvStoreObserverSnapImpl &observer3, KvStoreSnapshotDelegate *&snap3,
    vector<Entry> &entries, DistributedDB::Key &keyPrefix)
{
    /**
     * @tc.steps: step4. putBatch (k1,v1)(k2,v2) again and RegisterSnapObserver snap3 then getEntries with keyPrefix="k"
     * @tc.expected: step4. operate successfully and getEntries are (k1,v1)(k2,v2).
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<Entry> entriesInDB;
    for (auto eachEntry : entries) {
        entriesInDB.push_back(eachEntry);
    }
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap3);
    EXPECT_TRUE(status == DBStatus::OK);
    snap3 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer3);
    EXPECT_NE(snap3, nullptr);
    KvStoreSnapshotCallback kvStoreSnapshotCallback3;
    function<void(DBStatus, const std::vector<Entry>&)> function3
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback3, _1, _2);
    snap3->GetEntries(keyPrefix, function3);
    vector<Entry> resultEntries3 = kvStoreSnapshotCallback3.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries3, entries));
    return entriesInDB;
}

void RegisterSnapAgainAndCheck4(KvStoreObserverSnapImpl &observer4, KvStoreSnapshotDelegate *&snap4,
    vector<Entry> &entries, DistributedDB::Key &keyPrefix, vector<Entry> &entriesInDB)
{
    /**
     * @tc.steps: step5. putBatch (k1,v2)(k2,v3) again and RegisterSnapObserver snap4 then getEntries with keyPrefix="k"
     * @tc.expected: step5. operate successfully and getEntries are (k1,v2)(k2,v3).
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreBatchDelegate, entries);
    ASSERT_TRUE(status == DBStatus::OK);
    for (auto eachEntry : entries) {
        PutUniqueKey(entriesInDB, eachEntry.key, eachEntry.value);
    }
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap4);
    ASSERT_TRUE(status == DBStatus::OK);
    snap4 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer4);
    EXPECT_NE(snap4, nullptr);
    KvStoreSnapshotCallback kvStoreSnapshotCallback4;
    function<void(DBStatus, const std::vector<Entry>&)> function4
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback4, _1, _2);
    snap4->GetEntries(keyPrefix, function4);
    vector<Entry> resultEntries4 = kvStoreSnapshotCallback4.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries4, entries));
}

void PutAndCheck(DistributedDB::Key KEY_1, DistributedDB::Value VALUE_1, vector<Entry> &entriesInDB)
{
    DBStatus status = DistributedTestTools::Put(*g_kvStoreBatchDelegate, KEY_1, VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    PutUniqueKey(entriesInDB, KEY_1, VALUE_1);
}

void RegisterSnapAgainAndCheck5(KvStoreObserverSnapImpl &observer5, KvStoreSnapshotDelegate *&snap5,
    vector<Entry> &entries1, DistributedDB::Key &keyPrefix, vector<Entry> &entriesInDB)
{
    vector<Key> keys1;
    for (unsigned long time = 0; time < entries1.size(); ++time) {
        keys1.push_back(entries1.at(time).key);
    }
    /**
     * @tc.steps: step6. StartTransaction then deleteBatch (k1,v2)(k2,v3).
     * @tc.expected: step6. operate successfully to construct there is no data in db.
     */
    ASSERT_TRUE((g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap5)) == OK);
    snap5 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer5);
    EXPECT_NE(snap5, nullptr);
    KvStoreSnapshotCallback kvStoreSnapshotCallback5;
    function<void(DBStatus, const std::vector<Entry>&)> function5
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback5, _1, _2);
    EXPECT_TRUE((g_kvStoreBatchDelegate->StartTransaction()) == OK);
    ASSERT_TRUE((DistributedTestTools::DeleteBatch(*g_kvStoreBatchDelegate, keys1)) == OK);
    for (unsigned long idxKey = 0; idxKey < keys1.size(); ++idxKey) {
        for (unsigned long idxEntry = 0; idxEntry < entriesInDB.size(); ++idxEntry) {
            if (CompareVector(keys1[idxKey], entriesInDB[idxEntry].key)) {
                entriesInDB.erase(entriesInDB.begin() + idxEntry);
            }
        }
    }
    /**
     * @tc.steps: step7. put(k1,v1)(k2,v3) alone and commit, Register snap5 to getEntries with prefix-key='k'.
     * @tc.expected: step7. operate successfully and getEntries are (k1,v1)(k2,v3).
     */
    PutAndCheck(KEY_1, VALUE_1, entriesInDB);
    PutAndCheck(KEY_2, VALUE_2, entriesInDB);
    PutAndCheck(KEY_3, VALUE_3, entriesInDB);
    PutAndCheck(KEY_2, VALUE_3, entriesInDB);
    ASSERT_TRUE((DistributedTestTools::Delete(*g_kvStoreBatchDelegate, KEY_3)) == OK);
    for (unsigned long idx = 0; idx < entriesInDB.size(); ++idx) {
        if (CompareVector(entriesInDB[idx].key, KEY_3)) {
            entriesInDB.erase(entriesInDB.begin() + idx);
        }
    }
    EXPECT_TRUE((g_kvStoreBatchDelegate->Commit()) == OK);
    ASSERT_TRUE((g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap5)) == OK);
    snap5 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer5);
    EXPECT_NE(snap5, nullptr);
    snap5->GetEntries(keyPrefix, function5);
    vector<Entry> resultEntries5 = kvStoreSnapshotCallback5.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries5, entriesInDB));
}

void RegisterSnapAgainAndCheck6(KvStoreSnapshotDelegate *snap6, KvStoreSnapshotDelegate *snap7,
    KvStoreSnapshotDelegate *snap8, DistributedDB::Key &keyPrefix)
{
    /**
     * @tc.steps: step8. RegisterSnapObserver snap6,snap7,snap8 to getEntries with prefix-key='k'.
     * @tc.expected: step8. getEntries are empty.
     */
    KvStoreSnapshotCallback kvStoreSnapshotCallback6;
    function<void(DBStatus, const std::vector<Entry>&)> function6
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback6, _1, _2);
    snap6->GetEntries(keyPrefix, function6);
    vector<Entry> resultEntries6 = kvStoreSnapshotCallback6.GetEntries();
    vector<Entry> resultEmptyEntry;
    EXPECT_TRUE(CompareEntriesVector(resultEntries6, resultEmptyEntry));
    KvStoreSnapshotCallback kvStoreSnapshotCallback7;
    function<void(DBStatus, const std::vector<Entry>&)> function7
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback7, _1, _2);
    snap7->GetEntries(keyPrefix, function7);
    vector<Entry> resultEntries7 = kvStoreSnapshotCallback7.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries7, resultEmptyEntry));
    KvStoreSnapshotCallback kvStoreSnapshotCallback8;
    function<void(DBStatus, const std::vector<Entry>&)> function8
        = bind(&KvStoreSnapshotCallback::Callback, &kvStoreSnapshotCallback8, _1, _2);
    snap8->GetEntries(keyPrefix, function8);
    vector<Entry> resultEntries8 = kvStoreSnapshotCallback8.GetEntries();
    EXPECT_TRUE(CompareEntriesVector(resultEntries8, resultEmptyEntry));
}

void ReleaseSnap(KvStoreSnapshotDelegate *&snap1, KvStoreSnapshotDelegate *&snap2,
    KvStoreSnapshotDelegate *&snap3, KvStoreSnapshotDelegate *&snap4)
{
    DBStatus status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap1);
    ASSERT_TRUE(status == DBStatus::OK);
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap2);
    ASSERT_TRUE(status == DBStatus::OK);
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap3);
    ASSERT_TRUE(status == DBStatus::OK);
    status = g_kvStoreBatchDelegate->ReleaseKvStoreSnapshot(snap4);
    ASSERT_TRUE(status == DBStatus::OK);
}

/*
 * @tc.name: ComplexSnap 002
 * @tc.desc: Verify that release the registered snapshot, register again successfully.
 * @tc.type: FUN
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvBatchCrudTest, ComplexSnap002, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries12;
    entries12.push_back(ENTRY_1_2);
    entries12.push_back(ENTRY_2_3);
    KvStoreObserverSnapImpl observer1, observer2, observer3, observer4, observer5, observer6, observer7, observer8;
    KvStoreSnapshotDelegate *snap1 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer1);
    EXPECT_NE(snap1, nullptr);
    KvStoreSnapshotDelegate *snap2 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer2);
    EXPECT_NE(snap2, nullptr);
    KvStoreSnapshotDelegate *snap3 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer3);
    EXPECT_NE(snap3, nullptr);
    KvStoreSnapshotDelegate *snap4 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer4);
    EXPECT_NE(snap4, nullptr);
    KvStoreSnapshotDelegate *snap5 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer5);
    EXPECT_NE(snap5, nullptr);
    KvStoreSnapshotDelegate *snap6 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer6);
    EXPECT_NE(snap6, nullptr);
    KvStoreSnapshotDelegate *snap7 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer7);
    EXPECT_NE(snap7, nullptr);
    KvStoreSnapshotDelegate *snap8 = DistributedTestTools::RegisterSnapObserver(g_kvStoreBatchDelegate, &observer8);
    EXPECT_NE(snap8, nullptr);
    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step1. putBatch successfully to construct data in db.
     */
    DistributedDB::Key keyPrefix = { 'k' };
    RegisterSnapAgainAndCheck1(observer1, snap1, entries1, keyPrefix);
    vector<Key> keys1 = RegisterSnapAgainAndCheck2(observer2, snap2, entries1, keyPrefix);
    vector<Entry> entriesInDB = RegisterSnapAgainAndCheck3(observer3, snap3, entries1, keyPrefix);
    RegisterSnapAgainAndCheck4(observer4, snap4, entries1, keyPrefix, entriesInDB);
    RegisterSnapAgainAndCheck5(observer5, snap5, entries1, keyPrefix, entriesInDB);
    RegisterSnapAgainAndCheck6(snap6, snap7, snap8, keyPrefix);
    ReleaseSnap(snap1, snap2, snap3, snap4);
    ReleaseSnap(snap5, snap6, snap7, snap8);
    DistributedTestTools::Clear(*g_kvStoreBatchDelegate);
}
}
