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
#include <string>

#include "distributeddb_nb_test_tools.h"
#include "process_communicator_test_stub.h"

using namespace std;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbNbBatchCrud {
static std::condition_variable g_conditionNbBatchVar;
const int TWENTY_RECORDS = 20;
const int TWENTYFIVE_RECORDS = 25;
const int OBSERVER_COUNT = 8;
enum BatchTag {
    PUTBATCH = 0,
    DELETEBATCH = 1,
    GETENTRIES = 2,
};
KvStoreNbDelegate *g_nbBatchCrudDelegate = nullptr;
KvStoreDelegateManager *g_manager = nullptr;

DistributedDB::CipherPassword g_passwd1;

class DistributeddbNbBatchCrudTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbBatchCrudTest::SetUpTestCase(void)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    manager->SetProcessLabel("MST", "GetDevicesID");
    manager->SetProcessCommunicator(std::make_shared<ProcessCommunicatorTestStub>());
    delete manager;
    manager = nullptr;
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
}

void DistributeddbNbBatchCrudTest::TearDownTestCase(void)
{
}

void DistributeddbNbBatchCrudTest::SetUp(void)
{
    RemoveDir(NB_DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    g_nbBatchCrudDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, g_option);
    ASSERT_TRUE(g_manager != nullptr && g_nbBatchCrudDelegate != nullptr);
}

void DistributeddbNbBatchCrudTest::TearDown(void)
{
    MST_LOG("TearDownTestCase after case.");
    ASSERT_NE(g_manager, nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(g_manager, g_nbBatchCrudDelegate, STORE_ID_1, g_option.isMemoryDb));
    RemoveDir(NB_DIRECTOR);
}

/**
 * @tc.name: SimpleData 001
 * @tc.desc: Verify that single-ver db can support putbatch<keys,values> to db file.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData001, TestSize.Level0)
{
    vector<Entry> entries1, entries2;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries2.push_back(ENTRY_3);
    entries2.push_back(ENTRY_4);

    /**
     * @tc.steps: step1. call PutBatch interface to Put (k1, v1), (k2, v2) to DB and check the data in DB.
     * @tc.expected: step1. PutBatch successfully and the (k1, v1), (k2, v2) is in DB.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);

    /**
     * @tc.steps: step2. call PutBatch interface to Put (k3, v3), (k4, v4) to DB and check the data in DB.
     * @tc.expected: step2. PutBatch successfully and (k3, v3), (k4, v4) is in DB.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_3, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_3);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_4, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_4);
}

/**
 * @tc.name: SimpleData 002
 * @tc.desc: Verify that single-ver db can support update entries in batches.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData002, TestSize.Level0)
{
    vector<Entry> entries1, entries2, entriesGot;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries2.push_back(ENTRY_1_2);
    entries2.push_back(ENTRY_2_3);

    /**
     * @tc.steps: step1. call PutBatch interface to Put (k1, v1), (k2, v2) to DB and check the data in DB.
     * @tc.expected: step1. PutBatch successfully and can find (k1, v1), (k2, v2) in DB.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step2. call PutBatch interface to update (k1, v1), (k2, v2) to (k1, v2), (k2, v3)
     *    and check the data in DB.
     * @tc.expected: step2. PutBatch successfully and the value of k1 is v2, the value of k2 is v3.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_3);
}

/**
 * @tc.name: SimpleData 003
 * @tc.desc: Verify that single-ver db can support DeleteBatch<keys> from db file.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData003, TestSize.Level0)
{
    vector<Entry> entries, entryResults;
    entries.push_back(ENTRY_1);
    entries.push_back(ENTRY_2);
    entries.push_back(ENTRY_3);
    entries.push_back(ENTRY_4);
    std::vector<DistributedDB::Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);
    keys.push_back(KEY_3);
    keys.push_back(KEY_4);
    /**
     * @tc.steps: step1. call PutBatch interface to Put (k1, v1), (k2, v2), (k3, v3), (k4, v4) to DB
     *    and check the data in DB.
     * @tc.expected: step1. PutBatch successfully and can find (k1, v1), (k2, v2), (k3, v3), (k4, v4) in DB.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), OK);
    EXPECT_TRUE(CompareEntriesVector(entryResults, entries));

    /**
     * @tc.steps: step2. call DeleteBatch interface to delete  (k1, v1), (k2, v2), (k3, v3), (k4, v4) from DB
     *    and check the data in DB.
     * @tc.expected: step2. DeleteBatch successfully and the DB is empty.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), NOT_FOUND);
}

/**
 * @tc.name: SimpleData 004
 * @tc.desc: Verify that single-ver db can putbatch <keys,values> value of which is null, but can't putBatch
 *    <keys,values> value of which is bigger than 4M.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData004, TestSize.Level0)
{
    vector<Entry> entries1, entries2, entryResults;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2_NULL);
    entries2.push_back(ENTRY_3);
    DistributedDB::Entry entry;
    entry.key = KEY_4;
    entry.value.assign(FOUR_M_LONG_STRING + 1, 'v'); // the size of value is 4M + 1
    entries2.push_back(entry);

    /**
     * @tc.steps: step1. call PutBatch interface to Put (k1, v1), (k2, NULL) to DB and check the data in DB.
     * @tc.expected: step1. PutBatch successfully and the value of k1 is v1, and the value of k2 is null.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_EMPTY);

    /**
     * @tc.steps: step2. call PutBatch interface to Put (k3, v3), entry value of which is supper bigger than 4M
     *    and check the data in DB.
     * @tc.expected: step2. PutBatch failed and returned INVALID_ARGS.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), INVALID_ARGS);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), OK);
    EXPECT_EQ(entryResults.size(), entries1.size());
}

/**
 * @tc.name: SimpleData 005
 * @tc.desc: Verify that single-ver db can't putbatch <keys,values> key of which is null bigger than 1k.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData005, TestSize.Level0)
{
    vector<Entry> entries1, entries2, entryResults;
    DistributedDB::Entry entry1, entry2;
    entries1.push_back(ENTRY_1);
    entry1.key = KEY_EMPTY;
    entry1.value = VALUE_2;
    entries1.push_back(entry1);
    entries2.push_back(ENTRY_3);
    entry2.key.assign(ONE_K_LONG_STRING + 1, 'k'); // the size of key is 1k + 1
    entry2.value = VALUE_4;
    entries2.push_back(entry2);

    /**
     * @tc.steps: step1. call PutBatch interface to Put (k1, v1), (NULL, v2) to DB and check the data in DB.
     * @tc.expected: step1. PutBatch failed and return INVALID_ARGS.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), INVALID_ARGS);

    /**
     * @tc.steps: step2. call PutBatch interface to Put (k3, v3), entry value of which is supper bigger than 4M
     *    and check the data in DB.
     * @tc.expected: step2. PutBatch failed and returned INVALID_ARGS.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), INVALID_ARGS);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), NOT_FOUND);
}

/**
 * @tc.name: SimpleData 006
 * @tc.desc: Verify that single-ver db can't putbatch <keys,values> key of which is null bigger than 1k.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData006, TestSize.Level0)
{
    vector<Entry> entries, entryResults;
    entries.push_back(ENTRY_1);
    entries.push_back(ENTRY_2);
    entries.push_back(ENTRY_3);
    entries.push_back(ENTRY_4);

    vector<Key> keys1, keys2;
    keys1.push_back(KEY_1);
    DistributedDB::Key supperKey;
    supperKey.assign(ONE_K_LONG_STRING + 1, 'k'); // the size of key is 1k + 1
    keys1.push_back(supperKey);
    keys2.push_back(KEY_2);
    keys2.push_back(KEY_EMPTY);

    /**
     * @tc.steps: step1. PutBatch (k1,v1)(k2,v2)(k3,v3)(k4,v4) to db.
     * @tc.expected: step1. PutBatch successfully.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), OK);
    EXPECT_TRUE(CompareEntriesVector(entryResults, entries));

    /**
     * @tc.steps: step2. call DeleteBatch interface to delete k1, supperKey from DB and check the data in DB.
     * @tc.expected: step2. DeleteBatch failed and the data in DB is not changed.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys1), INVALID_ARGS);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), OK);
    EXPECT_EQ(entryResults.size(), entries.size());
    /**
     * @tc.steps: step3. call DeleteBatch interface to delete k2 which is KEY_EMPTY from DB and check the data in DB.
     * @tc.expected: step3. DeleteBatch failed and the data in DB is not changed.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys2), INVALID_ARGS);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), OK);
    EXPECT_EQ(entryResults.size(), entries.size());
}

/**
 * @tc.name: SimpleData 007
 * @tc.desc: Verify that single-ver db deletebatch the data that are noexist party will return OK, but will
 * return NOT_FOUND when the keys are all noexist.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData007, TestSize.Level0)
{
    vector<Entry> entries, entryResults;
    entries.push_back(ENTRY_1);
    entries.push_back(ENTRY_2);
    std::vector<DistributedDB::Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);
    keys.push_back(KEY_3);

    /**
     * @tc.steps: step1. call PutBatch interface to Put (k1, v1), (k2, v2) to DB
     *    and check the data in DB.
     * @tc.expected: step1. PutBatch successfully and can find (k1, v1), (k2, v2) in DB.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), OK);
    EXPECT_TRUE(CompareEntriesVector(entryResults, entries));

    /**
     * @tc.steps: step2. call DeleteBatch interface to delete  (k1,k2,k3) from DB
     *    and check the data in DB.
     * @tc.expected: step2. DeleteBatch successfully and the DB is empty.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), NOT_FOUND);

    /**
     * @tc.steps: step3. call DeleteBatch interface to delete  (k1,k2,k3) from DB
     *    and check the data in DB.
     * @tc.expected: step3. DeleteBatch return OK and the DB is empty.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), NOT_FOUND);
}

/**
 * @tc.name: SimpleData 008
 * @tc.desc: Verify that when the items of batch operation over 128, the interface will return INVALID_ARGS.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData008, TestSize.Level0)
{
    vector<Entry> entriesBatch, entryResults;
    vector<Key> allKeys;
    GenerateRecords(BATCH_RECORDS + 1, DEFAULT_START, allKeys, entriesBatch);

    /**
     * @tc.steps: step1. PutBatch 129 items to DB and GetEntries().
     * @tc.expected: step1. PutBatch failed and GetEntries return NOT_FOUND.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->PutBatch(entriesBatch), INVALID_ARGS);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), NOT_FOUND);

    /**
     * @tc.steps: step2. DeleteBatch 129 items to DB and GetEntries().
     * @tc.expected: step2. DeleteBatch failed and GetEntries return NOT_FOUND.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->DeleteBatch(allKeys), INVALID_ARGS);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), NOT_FOUND);
}

/**
 * @tc.name: SimpleData 009
 * @tc.desc: Verify that rekey will return busy when do batch operation.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData009, TestSize.Level2)
{
    vector<Entry> entriesBatch, entryResults;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, TEN_RECORDS, ONE_K_LONG_STRING, ONE_M_LONG_STRING);
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entriesBatch), OK);

    /**
     * @tc.steps: step1. PutBatch <keys,values> to db.
     * @tc.expected: step1. PutBatch successfully.
     */
    bool subPutBatchFlag = false;
    thread subThread([&]() {
        DBStatus status = g_nbBatchCrudDelegate->PutBatch(entriesBatch);
        EXPECT_TRUE(status == OK || status == BUSY);
        subPutBatchFlag = true;
        g_conditionNbBatchVar.notify_all();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call Rekey when step1 is running.
     * @tc.expected: step2. rekey return BUSY.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_LONG_TIME));
    DBStatus rekeyStatus = g_nbBatchCrudDelegate->Rekey(g_passwd1);
    EXPECT_TRUE(rekeyStatus == OK || rekeyStatus == BUSY);

    /**
     * @tc.steps: step3. GetEntries() from db.
     * @tc.expected: step3. GetEntries the data that step1 putbatch.
     */
    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_conditionNbBatchVar.wait(lck, [&]{return subPutBatchFlag;});
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), OK);
    EXPECT_EQ(entryResults.size(), entriesBatch.size());
}

/**
 * @tc.name: SimpleData 010
 * @tc.desc: Verify that import will return busy when do batch operation.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, SimpleData010, TestSize.Level2)
{
    const std::string importPath = NB_DIRECTOR + "export";
    SetDir(importPath);
    std::string filePath = importPath + "/bkpDB.bin";
    EXPECT_EQ(g_nbBatchCrudDelegate->Export(filePath, NULL_PASSWD), OK);
    vector<Entry> entriesBatch, entryResults;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, NB_PREDATA_NUM, ONE_K_LONG_STRING, ONE_M_LONG_STRING);
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entriesBatch), OK);

    /**
     * @tc.steps: step1. DeleteBatch data.
     * @tc.expected: step1. operation successfully.
     */
    bool subDeleteBatchFlag = false;
    thread subThread([&]() {
        DBStatus deleteStatus = g_nbBatchCrudDelegate->DeleteBatch(allKeys);
        EXPECT_TRUE(deleteStatus == OK || deleteStatus == BUSY);
        subDeleteBatchFlag = true;
        g_conditionNbBatchVar.notify_all();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. call import interface when step1 is running.
     * @tc.expected: step2. import return BUSY.
     */
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND));
    DBStatus importStatus = g_nbBatchCrudDelegate->Import(filePath, NULL_PASSWD);
    EXPECT_TRUE(importStatus == OK || importStatus == BUSY);

    /**
     * @tc.steps: step3. GetEntries() from db.
     * @tc.expected: step3. GetEntries return NOT_FOUND.
     */
    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_conditionNbBatchVar.wait(lck, [&]{return subDeleteBatchFlag;});
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), NOT_FOUND);
}

void TestBatchComplexAction(vector<Entry> &entries1, vector<Entry> &entries12)
{
    /**
     * @tc.steps: step3. PutBatch (k1,v1)(k2,v2) and then get.
     * @tc.expected: step3. PutBatch successfully get null of k1,k2.
     */
    DBStatus status = DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1);
    EXPECT_EQ(status, OK);
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);

    /**
     * @tc.steps: step4. PutBatch (k1,v2)(k2,v3) and then get.
     * @tc.expected: step4. PutBatch successfully get v2 of k1,v3 of k2.
     */
    status = DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries12);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_3);
}

/**
 * @tc.name: ComplexData 001
 * @tc.desc: Verify that single-ver db can support batch CRUD for the same keys continuously.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, ComplexData001, TestSize.Level0)
{
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries12;
    entries12.push_back(ENTRY_1_2);
    entries12.push_back(ENTRY_2_3);

    vector<Key> keys1;
    keys1.push_back(ENTRY_1.key);
    keys1.push_back(ENTRY_2.key);

    /**
     * @tc.steps: step1. PutBatch (k1,v1)(k2,v2) and then get.
     * @tc.expected: step1. PutBatch successfully get v1 of k1,v2 of k2.
     */
    DBStatus status = DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1);
    EXPECT_EQ(status, OK);
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);

    /**
     * @tc.steps: step2. DeleteBatch (k1)(k2) and then get.
     * @tc.expected: step2. DeleteBatch successfully get return NOT_FOUND.
     */
    status = DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys1);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), NOT_FOUND);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), NOT_FOUND);

    TestBatchComplexAction(entries1, entries12);

    /**
     * @tc.steps: step5. DeleteBatch (k1)(k2) and then get.
     * @tc.expected: step5. DeleteBatch successfully get return NOT_FOUND.
     */
    status = DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys1);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), NOT_FOUND);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), NOT_FOUND);
}

/**
 * @tc.name: ComplexData 002
 * @tc.desc: Verify that single-ver db can support batch CRUD for the different keys continuously.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, ComplexData002, TestSize.Level2)
{
    vector<Entry> entriesBatch, entriesRes;
    vector<Key> allKeys;
    /**
     * @tc.steps: step1-2. PutBatch 100 items of (keys,values) then query and deletebatch for 100 times.
     * @tc.expected: step1-2. operate successfully.
     */
    GenerateFixedLenRandRecords(entriesBatch, allKeys, ONE_HUNDRED_RECORDS, KEY_SIX_BYTE, KEY_ONE_HUNDRED_BYTE);
    for (int count = 0; count < ONE_HUNDRED_RECORDS; count++) {
        EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entriesBatch), OK);
        EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesRes), OK);
        EXPECT_EQ(entriesRes.size(), entriesBatch.size());
        EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, allKeys), OK);
        entriesRes.clear();
    }

    /**
     * @tc.steps: step3. PutBatch 20 items of (keys,values) and then query.
     * @tc.expected: step3. PutBatch successfully GetEntries is right.
     */
    entriesBatch.clear();
    GenerateRecords(TWENTY_RECORDS, DEFAULT_START, allKeys, entriesBatch);
    DBStatus status = DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entriesBatch);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesRes), OK);
    EXPECT_EQ(entriesRes.size(), entriesBatch.size());
}

void CheckBatchCrud(vector<Entry> entries, vector<Key> keys)
{
    /**
     * @tc.steps: step3. Put(k1,v3) Put(k2,v4) Put(k3,v3) and then get.
     * @tc.expected: step3. Put successfully get v3 of k1,v4 of k2,v3 of k3.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, ENTRY_1_3.key, ENTRY_1_3.value), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, ENTRY_2_4.key, ENTRY_2_4.value), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, ENTRY_3.key, ENTRY_3.value), OK);
    vector<Entry> entriesRes;
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesRes), OK);
    EXPECT_TRUE(CompareEntriesVector(entriesRes, entries));
    /**
     * @tc.steps: step4. DeleteBatch (k2)(k3) and then get.
     * @tc.expected: step4. DeleteBatch successfully get v3 of k1,get return NOT_FOUND of k2,k3.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys), OK);
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_3);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), NOT_FOUND);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_3, valueResult), NOT_FOUND);
    /**
     * @tc.steps: step5. PutBatch(k1,v2)(k2,v3)(k3,v4) then get.
     * @tc.expected: step5. Putbatch successfully, get v2 of k1, v3 of k2, v4 of k3.
     */
    vector<Entry> entries123;
    entries123.push_back(ENTRY_1_2);
    entries123.push_back(ENTRY_2_3);
    entries123.push_back(ENTRY_3_4);
    DBStatus status = DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries123);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_3);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_3, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_4);
}

/**
 * @tc.name: ComplexData 003
 * @tc.desc: Verify that can batch and single crud continuously.
 * @tc.type: FUN
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, ComplexData003, TestSize.Level0)
{
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries123;
    entries123.push_back(ENTRY_1_3);
    entries123.push_back(ENTRY_2_4);
    entries123.push_back(ENTRY_3);
    vector<Key> keys23;
    keys23.push_back(ENTRY_2.key);
    keys23.push_back(ENTRY_3.key);

    /**
     * @tc.steps: step1. PutBatch (k1,v1)(k2,v2) and then get.
     * @tc.expected: step1. PutBatch successfully get v1 of k1,v2 of k2.
     */
    DBStatus status = DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1);
    EXPECT_EQ(status, OK);
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);

    /**
     * @tc.steps: step2. delete k1 and then get.
     * @tc.expected: step2. delete successfully get return NOT_FOUND of k1, get v2 of k2.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, valueResult), NOT_FOUND);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);

    CheckBatchCrud(entries123, keys23);
}

void NbBatchCRUDThread(BatchTag tag)
{
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateFixedLenRandRecords(entriesBatch, allKeys, TWENTY_RECORDS,
        KEY_SIX_BYTE, KEY_ONE_HUNDRED_BYTE);
    if (tag == PUTBATCH) {
        DBStatus status = DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entriesBatch);
        EXPECT_EQ(status, OK);
    } else if (tag == DELETEBATCH) {
        DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, allKeys);
    } else if (tag == GETENTRIES) {
        vector<Entry> entryResults;
        DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, {'k'}, entryResults);
    }
}

/**
 * @tc.name: ComplexData 005
 * @tc.desc: test that it won't be crash when there are 20 threads to batch operation.
 * @tc.type: Pressure
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, ComplexData005, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start 20 threads to do batch operation.
     * @tc.expected: step1. process is normal and no crash.
     */
    std::vector<std::thread> threads;
    for (int threadId = 0; threadId < TWENTY_RECORDS; ++threadId) {
        int indexOption = GetRandInt(MODE_RAND_MIN, MODE_RAND_MAX);
        threads.push_back(std::thread(NbBatchCRUDThread, BatchTag(indexOption)));
    }
    for (auto& th : threads) {
        th.join();
    }
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND));
}

/**
 * @tc.name: Observer 001
 * @tc.desc: Verify that putbatch will trigger callback after register observer for specific key.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Observer001, TestSize.Level0)
{
    KvStoreObserverImpl observer1, observer2, observer3;
    vector<Entry> entries1, entries2;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries2.push_back(ENTRY_2_3);
    entries2.push_back(ENTRY_3);
    vector<DistributedDB::Entry> observerCallbackEntries;

    /**
     * @tc.steps: step1. register observer1,observer2,observer3 for k1,k2,k3 with
     * mode = OBSERVER_CHANGES_NATIVE.
     * @tc.expected: step1. register successfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observer1), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_NATIVE, &observer2), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_3, OBSERVER_CHANGES_NATIVE, &observer3), OK);

    /**
     * @tc.steps: step2. putbatch (k1,v1)(k2,v2) to db and check observer callback.
     * @tc.expected: step2. putbatch successfully and the callback of observer1 and observer2
     * are all once INSERT_LIST.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    observerCallbackEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, ListType::INSERT_LIST, observerCallbackEntries));
    observerCallbackEntries.clear();
    observerCallbackEntries.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, ListType::INSERT_LIST, observerCallbackEntries));
    observer1.Clear();
    observer2.Clear();
    observer3.Clear();

    /**
     * @tc.steps: step3. putbatch (k2,v3)(k3,v3) to db and check observer callback.
     * @tc.expected: step3. putbatch successfully and the callback of observer2 is once UPDATE_LIST,
     * the callback of observer3 is once INSERT_LIST, there is no callback of observer1.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);
    observerCallbackEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, ListType::UPDATE_LIST, observerCallbackEntries));
    observerCallbackEntries.push_back(ENTRY_2_3);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, ListType::UPDATE_LIST, observerCallbackEntries));
    observerCallbackEntries.clear();
    observerCallbackEntries.push_back(ENTRY_3);
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ONE_TIME, ListType::INSERT_LIST, observerCallbackEntries));
}

/**
 * @tc.name: Observer 002
 * @tc.desc: Verify that deletebatch will trigger callback after register observer for specific key.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Observer002, TestSize.Level0)
{
    KvStoreObserverImpl observer1, observer2, observer3;
    vector<Entry> entries1, entryResults;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    std::vector<DistributedDB::Key> keys;
    keys.push_back(KEY_3);
    keys.push_back(KEY_2);
    keys.push_back(KEY_1);
    vector<DistributedDB::Entry> observerCallbackEntries;

    /**
     * @tc.steps: step1. PutBatch (k1,v1)(k2,v2) and then get.
     * @tc.expected: step1. PutBatch successfully get v1 of k1,v2 of k2.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), OK);
    EXPECT_TRUE(CompareEntriesVector(entryResults, entries1));

    /**
     * @tc.steps: step2. register observer1,observer2,observer3 for k1,k2,k3 with
     * mode = OBSERVER_CHANGES_NATIVE.
     * @tc.expected: step2. register successfully.
     */
    DBStatus status = g_nbBatchCrudDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observer1);
    EXPECT_EQ(status, OK);
    status = g_nbBatchCrudDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_NATIVE, &observer2);
    EXPECT_EQ(status, OK);
    status = g_nbBatchCrudDelegate->RegisterObserver(KEY_3, OBSERVER_CHANGES_NATIVE, &observer3);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step3. deletebatch (k1,k2,k3) and check observer callback.
     * @tc.expected: step3. deletebatch successfully and the callback of observer1 and observer2
     * are all once DELETE_LIST, the observer3 won't receive callback.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys), OK);
    observerCallbackEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, ListType::DELETE_LIST, observerCallbackEntries));
    observerCallbackEntries.clear();
    observerCallbackEntries.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, ListType::DELETE_LIST, observerCallbackEntries));
    observerCallbackEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ZERO_TIME, ListType::DELETE_LIST, observerCallbackEntries));
}

/**
 * @tc.name: Observer 003
 * @tc.desc: Verify that putbatch will trigger callback after register observer for specific key.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Observer003, TestSize.Level0)
{
    KvStoreObserverImpl observer1, observer2, observer3;
    vector<Entry> entries1, entries2;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_1_2);
    entries1.push_back(ENTRY_2);
    entries2.push_back(ENTRY_2_3);
    entries2.push_back(ENTRY_2_4);
    entries2.push_back(ENTRY_3);
    vector<DistributedDB::Entry> observerCallbackEntries;

    /**
     * @tc.steps: step1. register observer1,observer2,observer3 for k1,k2,k3 with
     * mode = OBSERVER_CHANGES_NATIVE.
     * @tc.expected: step1. register successfully.
     */
    DBStatus status = g_nbBatchCrudDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observer1);
    EXPECT_EQ(status, OK);
    status = g_nbBatchCrudDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_NATIVE, &observer2);
    EXPECT_EQ(status, OK);
    status = g_nbBatchCrudDelegate->RegisterObserver(KEY_3, OBSERVER_CHANGES_NATIVE, &observer3);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step2. putbatch (k1,v1)(k2,v2) to db and check observer callback.
     * @tc.expected: step2. putbatch successfully and the callback of observer1 and observer2
     * are all once INSERT_LIST.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    observerCallbackEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, ListType::INSERT_LIST, observerCallbackEntries));
    observerCallbackEntries.clear();
    observerCallbackEntries.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, ListType::INSERT_LIST, observerCallbackEntries));
    observer1.Clear();
    observer2.Clear();
    observer3.Clear();

    /**
     * @tc.steps: step3. putbatch (k2,v3)(k3,v3) to db and check observer callback.
     * @tc.expected: step3. putbatch successfully and the callback of observer2 is once UPDATE_LIST,
     * the callback of observer3 is once INSERT_LIST, there is no callback of observer1.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);
    observerCallbackEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, ListType::UPDATE_LIST, observerCallbackEntries));
    observerCallbackEntries.push_back(ENTRY_2_4);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, ListType::UPDATE_LIST, observerCallbackEntries));
    observerCallbackEntries.clear();
    observerCallbackEntries.push_back(ENTRY_3);
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ONE_TIME, ListType::INSERT_LIST, observerCallbackEntries));
}

/**
 * @tc.name: Observer 004
 * @tc.desc: Verify that deletebatch will trigger callback after register observer for specific key.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Observer004, TestSize.Level0)
{
    KvStoreObserverImpl observer1, observer2;
    vector<Entry> entries1, entryResults;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    std::vector<DistributedDB::Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);
    keys.push_back(KEY_1);
    vector<DistributedDB::Entry> observerCallbackEntries;

    /**
     * @tc.steps: step1. PutBatch (k1,v1)(k2,v2) and then get.
     * @tc.expected: step1. PutBatch successfully get v1 of k1,v2 of k2.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entryResults), OK);
    EXPECT_TRUE(CompareEntriesVector(entryResults, entries1));

    /**
     * @tc.steps: step2. register observer1,observer2 for k1, k2 with mode = OBSERVER_CHANGES_NATIVE.
     * @tc.expected: step2. register successfully.
     */
    DBStatus status = g_nbBatchCrudDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observer1);
    EXPECT_EQ(status, OK);
    status = g_nbBatchCrudDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_NATIVE, &observer2);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step3. deletebatch (k1,k2,k1) and check observer callback.
     * @tc.expected: step3. deletebatch successfully and observer1 and observer2 are both triggered once.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys), OK);
    observerCallbackEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, ListType::DELETE_LIST, observerCallbackEntries));
    observerCallbackEntries.clear();
    observerCallbackEntries.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, ListType::DELETE_LIST, observerCallbackEntries));
}

/**
 * @tc.name: Observer 005
 * @tc.desc: Verify that deletebatch will trigger once callback after register observer for all key.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Observer005, TestSize.Level0)
{
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(TEN_RECORDS, DEFAULT_START, allKeys, entriesBatch);

    /**
     * @tc.steps: step1. register 8 observers for all key.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observerNative[OBSERVER_COUNT];
    for (int obsCnt = 0; obsCnt < OBSERVER_COUNT; obsCnt++) {
        EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_NATIVE,
            &observerNative[obsCnt]), OK);
    }

    /**
     * @tc.steps: step2. putbatch 10 items of <keys,values> to db and check observer callback.
     * @tc.expected: step2. putbatch successfully and the callback of all observers are once
     * INSERT_LIST with 10 items data.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entriesBatch), OK);
    for (int index = 0; index < OBSERVER_COUNT; index++) {
        EXPECT_TRUE(VerifyObserverResult(observerNative[index], CHANGED_ONE_TIME, ListType::INSERT_LIST, entriesBatch));
    }
    for (int index = 0; index < OBSERVER_COUNT; index++) {
        observerNative[index].Clear();
    }

    /**
     * @tc.steps: step3. putbatch 10 items of <keys,values> to db and check observer callback.
     * @tc.expected: step3. putbatch successfully and the callback of all observers are once
     * UPDATE_LIST with 10 items data.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entriesBatch), OK);
    for (int index = 0; index < OBSERVER_COUNT; index++) {
        EXPECT_TRUE(VerifyObserverResult(observerNative[index], CHANGED_ONE_TIME, ListType::UPDATE_LIST, entriesBatch));
    }
    for (int index = 0; index < OBSERVER_COUNT; index++) {
    observerNative[index].Clear();
    }

    /**
     * @tc.steps: step4. deletebatch 10 items of <keys> from db and check observer callback.
     * @tc.expected: step4. putbatch successfully and the callback of all observers are once
     * DELETE_LIST with 10 items data.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, allKeys), OK);
    for (int index = 0; index < OBSERVER_COUNT; index++) {
        EXPECT_TRUE(VerifyObserverResult(observerNative[index], CHANGED_ONE_TIME, ListType::DELETE_LIST, entriesBatch));
    }
}

/**
 * @tc.name: Transaction 001
 * @tc.desc: start transaction success and can only native DB support to put and local DB not.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    /**
     * @tc.steps: step2. use Put interface to put (k1, v1) and Get the value of k1;
     * @tc.expected: step2. put successfully but can't find data in db
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    Value value;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_1);
    /**
     * @tc.steps: step3. use PutLocal interface to put (k2, v2) and Get the value of k2;
     * @tc.expected: step3. PutLocal failed and returned NOT_SUPPORT and can't find data in db
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutLocal(*g_nbBatchCrudDelegate, KEY_2, VALUE_2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetLocal(*g_nbBatchCrudDelegate, KEY_2, value), OK);
    EXPECT_EQ(value, VALUE_2);

    /**
     * @tc.steps: step4. commit the transaction and check the data in db again.
     * @tc.expected: step4. commit successfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_1);
    EXPECT_EQ(DistributedDBNbTestTools::GetLocal(*g_nbBatchCrudDelegate, KEY_2, value), OK);
    EXPECT_EQ(value, VALUE_2);
}

/**
 * @tc.name: Transaction 002
 * @tc.desc: verify that the db support Rollback but can't Rollback repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    Value value;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_1);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_2, VALUE_2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, value), OK);
    EXPECT_EQ(value, VALUE_2);

    /**
     * @tc.steps: step2. use Delete interface to delete (k1, v1) and Get the value of k1.
     * @tc.expected: step2. delete successfully and can't find k1 in db.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), NOT_FOUND);

    /**
     * @tc.steps: step3. Rollback and check the data in db again.
     * @tc.expected: step3. Rollback successfully and can't find (k1, v1) and (k2, v2) in db.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), NOT_FOUND);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, value), NOT_FOUND);

    /**
     * @tc.steps: step4. Rollback again.
     * @tc.expected: step4. returned error.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), DB_ERROR);
}

/**
 * @tc.name: Transaction 003
 * @tc.desc: verify that it can't start transaction repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction003, TestSize.Level0)
{
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    /**
     * @tc.steps: step1. start transaction and update (k1, v1) to (k1, v2).
     * @tc.expected: step1. start and put successfully but the value of k1 is still v1.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_2), OK);
    Value value;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_2);
    /**
     * @tc.steps: step2. start the transaction again.
     * @tc.expected: step2. returned error.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), DB_ERROR);

    /**
     * @tc.steps: step3. commit and check the records in db.
     * @tc.expected: step3. commit successfully and the value of k1 is v2.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_2);
}

/**
 * @tc.name: Transaction 004
 * @tc.desc: verify that can't commit without start transaction and can putBatch when start transaction.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. commit the transaction without start it.
     * @tc.expected: step1. commit failed.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), DB_ERROR);
    /**
     * @tc.steps: step2. start the transaction and putBatch 128 records and check the data with GetEntries.
     * @tc.expected: step2. start success and GetEntries returns no entry.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    std::vector<DistributedDB::Entry> entries, entriesGot;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, BATCH_RECORDS);
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_EQ(entriesGot.size(), BATCH_RECORDS);

    /**
     * @tc.steps: step3. commit and GetEntries in db.
     * @tc.expected: step3. commit successfully and can find the entries in db.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries, entriesGot));
}

/**
 * @tc.name: Transaction 005
 * @tc.desc: verify that can't Rollback without start transaction and can updateBatch when start transaction.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Rollback the transaction without start it and PutBatch 10 records to DB.
     * @tc.expected: step1. Rollback failed and PutBatch success.
     */
    std::vector<DistributedDB::Entry> entries1, entries2, entriesGot;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries1, entrySize, TEN_RECORDS, {'k'}, {'a'});
    GenerateAppointPrefixAndSizeRecords(entries2, entrySize, TEN_RECORDS, {'k'}, {'b'});
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);

    EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), DB_ERROR);
    /**
     * @tc.steps: step2. start the transaction and update the 10 records Batchly and check the data with GetEntries.
     * @tc.expected: step2. start success and GetEntries return the entries is equal to entres1.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries2, entriesGot));

    /**
     * @tc.steps: step3. commit and GetEntries in db.
     * @tc.expected: step3. commit successfully and the entriesGot is equal to entries2.
     */
    entriesGot.clear();
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries2, entriesGot));
}

/**
 * @tc.name: Transaction 006
 * @tc.desc: verify that can DeleteBatch when start transaction.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction006, TestSize.Level0)
{
    std::vector<DistributedDB::Entry> entries1, entries2, entriesGot;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries1, entrySize, TEN_RECORDS, {'k'}, {'a'});
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);

    for (unsigned int index = 0; index < (entries1.size() / 2); index++) {
        entries2.push_back(entries1[index]);
    }
    std::vector<DistributedDB::Key> keys;
    for (auto const &entry : entries2) {
        keys.push_back(entry.key);
    }

    /**
     * @tc.steps: step1. start the transaction and Deleted Batchly 5 of the 10 records inserted to db.
     * @tc.expected: step1. start and delete success and GetEntries return the entries is equal to entres1.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys), OK);
    entries1.erase(entries1.begin(), entries1.begin() + FIVE_RECORDS);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, entriesGot));

    /**
     * @tc.steps: step2. commit and GetEntries in db.
     * @tc.expected: step2. commit successfully and the entriesGot is equal to (entries1 - entries2).
     */
    entriesGot.clear();
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, entriesGot));

    /**
     * @tc.steps: step3. commit again.
     * @tc.expected: step3. commit failed and returned error.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), DB_ERROR);
}

/**
 * @tc.name: Transaction 007
 * @tc.desc: verify that transaction support native db insert or delete records
 *     but don't support local db to operate db.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction007, TestSize.Level0)
{
    /**
     * @tc.steps: step1. PutLocal (k1, v1) to Local DB and start transaction
     * @tc.expected: step1. putLocal and start success.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutLocal(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);

    /**
     * @tc.steps: step2. start the transaction and Put and update and deleteLocal on DB and commit the transaction.
     * @tc.expected: step2. start and Put and commit success,
     *    but can't find records on native db, and deletelocal returned failed.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    Value value;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_1);

    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_2);

    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocal(*g_nbBatchCrudDelegate, KEY_1), OK);

    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_2);
    EXPECT_EQ(DistributedDBNbTestTools::GetLocal(*g_nbBatchCrudDelegate, KEY_1, value), NOT_FOUND);

    /**
     * @tc.steps: step3. Rollback after commit.
     * @tc.expected: step3. Rollback failed and returned error.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), DB_ERROR);
}

/**
 * @tc.name: Transaction 008
 * @tc.desc: verify that it can't commit after it was Rollbacked
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction008, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Put (k1, v1) to DB and start transaction
     * @tc.expected: step1. put and start success.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);
    /**
     * @tc.steps: step2. update (k1, v1) and Rollback.
     * @tc.expected: step2. update and Rollback successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_2), OK);

    Value value;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_2);

    EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), OK);
    /**
     * @tc.steps: step3. commit after Rollback and check the records in db.
     * @tc.expected: step3. commit failed and returned error and the value of k1 is v1.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), DB_ERROR);

    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_1);
}

/**
 * @tc.name: Transaction 009
 * @tc.desc: verify that start transaction and commit or rollback repeatedly
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction009, TestSize.Level0)
{
    /**
     * @tc.steps: step1. repeat 5 times
     * @tc.expected: step1. start loop success.
     */
    for (int index = 0; index < FIVE_TIMES; index++) {
        /**
         * @tc.steps: step2. start transaction and commit.
         * @tc.expected: step2. start and commit successfully
         */
        EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

        EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
        /**
         * @tc.steps: step3. start transaction and rollback.
         * @tc.expected: step3. start and Rollback successfully
         */
        EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

        EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), OK);
    }
}

/**
 * @tc.name: Transaction 010
 * @tc.desc: verify that can Batchly operate on transaction
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction
     * @tc.expected: step1. start success.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    std::vector<DistributedDB::Entry> entries1, entries2, entriesGot;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries1, entrySize, SIXTY_RECORDS, {'a'}, {'m'});
    GenerateAppointPrefixAndSizeRecords(entries2, entrySize, THIRTYTWO_RECORDS, {'b'}, {'n'});
    /**
     * @tc.steps: step2. PutBatch 60 records entries1 to DB.
     * @tc.expected: step2. PutBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    /**
     * @tc.steps: step3. PutBatch 32 records entries2 to DB.
     * @tc.expected: step3. PutBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);

    std::vector<DistributedDB::Key> keys;
    for (auto const &entry : entries2) {
        keys.push_back(entry.key);
    }
    /**
     * @tc.steps: step4. DeleteBatch entries2 from DB.
     * @tc.expected: step4. DeleteBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys), OK);

    /**
     * @tc.steps: step5. Delete entries1[1] and entries1[2] from DB solely.
     * @tc.expected: step5. Delete successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, entries1[1].key), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, entries1[2].key), OK);
    /**
     * @tc.steps: step6. insert entries1[1] to DB solely.
     * @tc.expected: step6. insert successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, entries1[1].key, entries1[1].value), OK);
    /**
     * @tc.steps: step7. update value of entries1[1] to v2.
     * @tc.expected: step7. update successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, entries1[1].key, VALUE_2), OK);
    /**
     * @tc.steps: step8. commit and check the records in DB.
     * @tc.expected: step8. commit success and can only find entries1 in db can't find entries2
     *    and the value of entries1[1] is v2, and also can't find entries[2].
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    for (auto const & entry : entries2) {
        auto it = std::find_if(entriesGot.begin(), entriesGot.end(), [entry](const DistributedDB::Entry &entry_) {
            return (CompareVector(entry.key, entry_.key) && CompareVector(entry.value, entry_.value));
        });
        EXPECT_EQ(it, entriesGot.end());
    }
    for (auto const & entry : entries1) {
        if (!CompareVector(entry.key, entries1[1].key) && !CompareVector(entry.key, entries1[2].key)) {
            auto it = std::find_if(entriesGot.begin(), entriesGot.end(), [entry](const DistributedDB::Entry &entry_) {
                return (CompareVector(entry.key, entry_.key) && CompareVector(entry.value, entry_.value));
            });
            EXPECT_NE(it, entriesGot.end());
        } else if (CompareVector(entry.key, entries1[1].key)) {
            auto it = std::find_if(entriesGot.begin(), entriesGot.end(), [entry](const DistributedDB::Entry &entry_) {
                return (CompareVector(entry.key, entry_.key) && CompareVector(VALUE_2, entry_.value));
            });
            EXPECT_NE(it, entriesGot.end());
        } else {
            auto it = std::find_if(entriesGot.begin(), entriesGot.end(), [entry](const DistributedDB::Entry &entry_) {
                return (CompareVector(entry.key, entry_.key) && CompareVector(entry.value, entry_.value));
            });
            EXPECT_EQ(it, entriesGot.end());
        }
    }
}

/**
 * @tc.name: Transaction 011
 * @tc.desc: verify that when rollback after logs of CRUD operate, the operate will not be effective.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction011, TestSize.Level1)
{
    std::vector<DistributedDB::Entry> entries1, entries2, entriesGot;
    entries1 = {ENTRY_1, ENTRY_2, ENTRY_3};
    entries2 = {ENTRY_4, ENTRY_5};
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    /**
     * @tc.steps: step1. start transaction
     * @tc.expected: step1. start success.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    entries1[0] = {KEY_1, VALUE_EMPTY};
    entries1[1] = {KEY_2, VALUE_EMPTY};
    entries1[2] = {KEY_3, VALUE_EMPTY};
    /**
     * @tc.steps: step2. PutBatch entries1 to DB.
     * @tc.expected: step2. PutBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    /**
     * @tc.steps: step3. PutBatch entries2 to DB.
     * @tc.expected: step3. PutBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);

    std::vector<DistributedDB::Key> keys = {KEY_4, KEY_5};
    /**
     * @tc.steps: step4. DeleteBatch entries2 from DB.
     * @tc.expected: step4. DeleteBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteBatch(*g_nbBatchCrudDelegate, keys), OK);

    /**
     * @tc.steps: step5. Delete entries1[1] and entries1[2] from DB solely.
     * @tc.expected: step5. Delete successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, entries1[1].key), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, entries1[2].key), OK);
    /**
     * @tc.steps: step6. insert entries1[1] to DB solely.
     * @tc.expected: step6. insert successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, entries1[1].key, entries1[1].value), OK);
    /**
     * @tc.steps: step7. update value of entries1[1] to v2.
     * @tc.expected: step7. update successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, entries1[1].key, VALUE_2), OK);
    /**
     * @tc.steps: step8. insert the records that key = null.
     * @tc.expected: step8. insert failed and returned DB_ERROR
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_EMPTY, VALUE_9), INVALID_ARGS);

    /**
     * @tc.steps: step9. rollback and use GetEntries interface to check.
     * @tc.expected: step9. rollback success and the data in db is entries1
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), OK);
    entries1 = {ENTRY_1, ENTRY_2, ENTRY_3};
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, entriesGot));
}

/**
 * @tc.name: Transaction 012
 * @tc.desc: verify that exception operate can't effect transaction and commit.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction012, TestSize.Level1)
{
    std::vector<DistributedDB::Entry> entries, entriesGot;
    entries = {ENTRY_1, ENTRY_2};
    /**
     * @tc.steps: step1. start transaction
     * @tc.expected: step1. start success.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);
    /**
     * @tc.steps: step2. PutBatch entries to DB.
     * @tc.expected: step2. PutBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries, entriesGot));
    /**
     * @tc.steps: step3. Put the record that key = empty to DB.
     * @tc.expected: step3. Put failed and returned error
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_EMPTY, VALUE_10), INVALID_ARGS);

    /**
     * @tc.steps: step4. commit and use GetEntries interface to check.
     * @tc.expected: step4. commit success and the data in db is entries1
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries, entriesGot));
}

/**
 * @tc.name: Transaction 013
 * @tc.desc: verify that one transaction can only operate less than MAX_BATCH_SIZE records, or it will return
 *     OVER_MAX_LIMITS.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction013, TestSize.Level1)
{
    std::vector<DistributedDB::Entry> entries, entriesGot;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, BATCH_RECORDS);
    /**
     * @tc.steps: step1. start transaction
     * @tc.expected: step1. start success.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);
    /**
     * @tc.steps: step2. PutBatch entries which contains 128 records to DB.
     * @tc.expected: step2. PutBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries, entriesGot));
    /**
     * @tc.steps: step3. Put (k1, v1) to DB.
     * @tc.expected: step3. Put failed and returned OVER_MAX_LIMITS
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OVER_MAX_LIMITS);
    /**
     * @tc.steps: step3. Delete (k1, v1) from DB.
     * @tc.expected: step3. Delete failed and returned OVER_MAX_LIMITS
     */
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_1), OVER_MAX_LIMITS);

    /**
     * @tc.steps: step4. commit and use GetEntries interface to check.
     * @tc.expected: step4. commit success and the data in db is entries1
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries, entriesGot));
}

/**
 * @tc.name: Transaction 014
 * @tc.desc: verify that one transaction can only operate less than MAX_BATCH_SIZE records, or it will return
 *     OVER_MAX_LIMITS and if one operate return OVER_MAX_LIMITS, only this operate is invalid.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction014, TestSize.Level1)
{
    std::vector<DistributedDB::Entry> entries1, entries2, entries3, entries4, entriesGot;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries1, entrySize, ONE_HUNDRED_RECORDS, {'a'}, {'m'});
    GenerateAppointPrefixAndSizeRecords(entries2, entrySize, RECORDS_SMALL_CNT, {'b'}, {'n'});
    GenerateAppointPrefixAndSizeRecords(entries3, entrySize, THIRTYTWO_RECORDS, {'c'}, {'o'});
    GenerateAppointPrefixAndSizeRecords(entries4, entrySize, TWENTYFIVE_RECORDS, {'d'}, {'p'});
    /**
     * @tc.steps: step1. start transaction
     * @tc.expected: step1. start success.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);
    /**
     * @tc.steps: step2. PutBatch 100 records to DB.
     * @tc.expected: step2. PutBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, entriesGot));
    /**
     * @tc.steps: step3. PutBatch 2 records to DB.
     * @tc.expected: step3. PutBatch successfully
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(entriesGot.size() == ONE_HUNDRED_RECORDS + RECORDS_SMALL_CNT);
    /**
     * @tc.steps: step4. Delete nonexistent key1 from DB.
     * @tc.expected: step4. Delete return OK, and the number of records is still counted in the transaction.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(entriesGot.size() == ONE_HUNDRED_RECORDS + RECORDS_SMALL_CNT);
    const int nonexistentRecordNum = 1;
    /**
     * @tc.steps: step5. PutBatch 32 records to DB.
     * @tc.expected: step5. PutBatch failed and OVER_MAX_LIMITS
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries3), OVER_MAX_LIMITS);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(entriesGot.size() == ONE_HUNDRED_RECORDS + RECORDS_SMALL_CNT);
    /**
     * @tc.steps: step5. PutBatch 26 records to DB.
     * @tc.expected: step5. PutBatch succeed.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries4), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(entriesGot.size() == (BATCH_RECORDS - nonexistentRecordNum));

    /**
     * @tc.steps: step4. commit and use GetEntries interface to check.
     * @tc.expected: step4. commit success and the data in db is entries1
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);

    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    for (auto const & entry : entries4) {
        auto it = std::find_if(entriesGot.begin(), entriesGot.end(), [entry](const DistributedDB::Entry &entry_) {
            return (CompareVector(entry.key, entry_.key) && CompareVector(entry.value, entry_.value));
        });
        EXPECT_NE(it, entriesGot.end());
    }
    for (auto const & entry : entries1) {
        auto it = std::find_if(entriesGot.begin(), entriesGot.end(), [entry](const DistributedDB::Entry &entry_) {
            return (CompareVector(entry.key, entry_.key) && CompareVector(entry.value, entry_.value));
        });
        EXPECT_NE(it, entriesGot.end());
    }
    for (auto const & entry : entries2) {
        auto it = std::find_if(entriesGot.begin(), entriesGot.end(), [entry](const DistributedDB::Entry &entry_) {
            return (CompareVector(entry.key, entry_.key) && CompareVector(entry.value, entry_.value));
        });
        EXPECT_NE(it, entriesGot.end());
    }
}

int g_threadComplete = 0;
bool g_threadSuccessFlag = true;
void ConsistencyCheck()
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(delegateManager, g_dbParameter1, g_option);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr) << "ConsistencyCheck get delegate failed.";

    while (g_threadComplete < static_cast<int>(SINGLE_THREAD_NUM)) {
        std::vector<DistributedDB::Entry> entriesGot;
        EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*delegate, KEY_EMPTY, entriesGot), OK);
        if (GetIntValue(entriesGot[0].value) + GetIntValue(entriesGot[1].value) != VALUE_SUM) {
            g_threadSuccessFlag = false;
            MST_LOG("ConsistencyCheck get sum %d,%d.", GetIntValue(entriesGot[0].value),
                GetIntValue(entriesGot[1].value));
            break;
        }
        std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));
    }

    EXPECT_EQ(delegateManager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    delete delegateManager;
    delegateManager = nullptr;
}

void ConsistencyChange(Key keyLeft, Value valueLeft, Key keyRight, Value valueRight)
{
    // wait 100 ms, let ConsistencyCheck begin
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(HUNDRED_MILLI_SECONDS));
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(delegateManager, g_dbParameter1, g_option);
    if (delegate == nullptr) {
        MST_LOG("ConsistencyChange get delegate failed.");
        g_threadComplete++;
        g_threadSuccessFlag = false;
        return;
    }

    MST_LOG("ConsistencyChange put %d,%d.", GetIntValue(valueLeft), GetIntValue(valueRight));
    DBStatus statusStart = delegate->StartTransaction();
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));

    DBStatus statusPutLeft = delegate->Put(keyLeft, valueLeft);
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));

    DBStatus statusPutRight = delegate->Put(keyRight, valueRight);
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));

    DBStatus statusCommit = delegate->Commit();
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));
    g_threadComplete++;

    if (statusStart != DBStatus::OK || statusPutLeft != DBStatus::OK ||
        statusPutRight != DBStatus::OK || statusCommit != DBStatus::OK) {
        MST_LOG("ConsistencyChange put failed.");
        g_threadComplete++;
        g_threadSuccessFlag = false;
        return;
    }
    ConsistencyCheck();

    EXPECT_EQ(delegateManager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    delete delegateManager;
    delegateManager = nullptr;
}

/**
 * @tc.name: Transaction 015
 * @tc.desc: Verify that transaction singlely action's consistency.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction015, TestSize.Level2)
{
    int baseNumer = VALUE_FIVE_HUNDRED;
    Value base = GetValueWithInt(baseNumer);

    Entry entry1, entry2;
    vector<Entry> entries1;
    entry1 = {KEY_CONS_1, base};
    entry2 = {KEY_CONS_2, base};
    entries1.push_back(entry1);
    entries1.push_back(entry2);
    /**
     * @tc.steps: step1. putBatch (k1=cons1,v1=500) (k2=cons2,v2=500) to construct exist v1+v2=1000.
     * @tc.expected: step1. putBatch successfully.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    Value value;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, entry1.key, value), OK);
    EXPECT_EQ(value, base);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, entry2.key, value), OK);
    EXPECT_EQ(value, base);

    /**
     * @tc.steps: step2. start a thread to update k1=cons1's value to 400, update k2=cons2's value to 600.
     * @tc.expected: step2. run thread successfully and k1+k2=1000 is true in sub thread.
     */
    std::vector<std::thread> suThreads;
    suThreads.push_back(std::thread(ConsistencyChange, KEY_CONS_1, GetValueWithInt(VALUE_CHANGE1_FIRST),
        KEY_CONS_2, GetValueWithInt(VALUE_CHANGE1_SECOND)));
    /**
     * @tc.steps: step3. start another thread to update k1=cons1's value to 700, update k2=cons2's value to 300.
     * @tc.expected: step3. run thread successfully and k1+k2=1000 is true in sub thread.
     */
    suThreads.push_back(std::thread(ConsistencyChange, KEY_CONS_1, GetValueWithInt(VALUE_CHANGE2_FIRST),
        KEY_CONS_2, GetValueWithInt(VALUE_CHANGE2_SECOND)));
    for (auto& th : suThreads) {
        th.detach();
    }

    ConsistencyCheck();
    std::this_thread::sleep_for(std::chrono::seconds(FIVE_SECONDS));
    ASSERT_TRUE(g_threadSuccessFlag);
}

bool g_batchThreadSuccess = true;
bool g_batchThreadComplete = false;
void ConsistencyBatchCheck(vector<Key> *keyLeft, vector<Key> *keyRight, int times)
{
    std::this_thread::sleep_for(std::chrono::seconds(TWO_SECONDS));
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(delegateManager, g_dbParameter1, g_option);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr) << "ConsistencyBatchCheck get delegate failed.";

    Value leftValue, rightValue;
    while (!g_batchThreadComplete) {
        for (int i = 0; i < times; ++i) {
            leftValue.clear();
            rightValue.clear();
            EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, keyLeft->at(i), leftValue), OK);
            EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, keyRight->at(i), rightValue), OK);

            if (GetIntValue(leftValue) + GetIntValue(rightValue) != (times + 1)) {
                g_batchThreadSuccess = false;
                MST_LOG("ConsistencyBatchCheck get leftvalue: %d, rightvalue: %d, failed.",
                    GetIntValue(leftValue), GetIntValue(rightValue));
                break;
            }
        }
    }

    MST_LOG("g_batchThreadComplete.");
    EXPECT_EQ(delegateManager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    delete delegateManager;
    delegateManager = nullptr;
}

void ConsistencyCheckTransaction(vector<Entry> &entries1, vector<Entry> &entries2)
{
    Key query1 = {'k', 'a'};
    Key query2 = {'k', 'b'};
    /**
     * @tc.steps: step1. putBatch 20 items of (keys1,values1)(key2,values2).
     * @tc.expected: step1. putBatch successfully to construct exist data in db.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);

    /**
     * @tc.steps: step3. start transaction.
     * @tc.expected: step3. start transaction successfully.
     */
    EXPECT_TRUE(g_nbBatchCrudDelegate->StartTransaction() == DBStatus::OK);
    /**
     * @tc.steps: step4. getEntries with keyprefix before updateBatch.
     * @tc.expected: step4. getEntries successfully that the size of them is 20.
     */
    vector<Entry> values1, values2;
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, query1, values1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, query2, values2), OK);
    EXPECT_EQ(static_cast<int>(values1.size()), TWENTY_RECORDS);
    EXPECT_EQ(static_cast<int>(values2.size()), TWENTY_RECORDS);
    /**
     * @tc.steps: step5. updateBatch values1+1 of keys1, values2-1 of keys2.
     * @tc.expected: step5. updateBatch successfully.
     */
    for (int i = 0; i != TWENTY_RECORDS; ++i) {
        entries1[i].value = GetValueWithInt(GetIntValue(values1[i].value) + 1);
        entries2[i].value = GetValueWithInt(GetIntValue(values2[i].value) - 1);
    }
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);
    /**
     * @tc.steps: step6. updateBatch values1+2 of keys1, values2-2 of keys2.
     * @tc.expected: step6. updateBatch successfully.
     */
    for (int i = 0; i != TWENTY_RECORDS; ++i) {
        entries1[i].value = GetValueWithInt(GetIntValue(values1[i].value) + EVEN_NUMBER);
        entries2[i].value = GetValueWithInt(GetIntValue(values2[i].value) - EVEN_NUMBER);
    }
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries2), OK);
    /**
     * @tc.steps: step7. commit transaction and stop child thread.
     * @tc.expected: step7. operate successfully.
     */
    EXPECT_TRUE(g_nbBatchCrudDelegate->Commit() == DBStatus::OK);
    MST_LOG("commit over!");
}

/**
 * @tc.name: Transaction 016
 * @tc.desc: Verify that transaction batch action's consistency.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction016, TestSize.Level2)
{
    uint8_t left = 'a';
    uint8_t right = 'b';
    vector<Entry> entries1;
    vector<Key> allKeys1;
    GenerateRecords(TWENTY_RECORDS, DEFAULT_START, allKeys1, entries1);
    vector<Entry> entries2;
    vector<Key> allKeys2;
    GenerateRecords(TWENTY_RECORDS, DEFAULT_START, allKeys2, entries2);
    for (int i = 0; i < TWENTY_RECORDS; ++i) {
        entries1[i].key.insert(entries1[i].key.begin() + 1, left);
        entries2[i].key.insert(entries2[i].key.begin() + 1, right);
        allKeys1[i].insert(allKeys1[i].begin() + 1, left);
        allKeys2[i].insert(allKeys2[i].begin() + 1, right);
        entries1[i].value = GetValueWithInt(i);
        entries2[i].value = GetValueWithInt(TWENTY_RECORDS - i);
    }

    /**
     * @tc.steps: step2. start child thread to query keys1 and keys2 continuously.
     * @tc.expected: step2. if keys1.size()+keys2.size()!=20 print info.
     */
    thread readThread = thread(ConsistencyBatchCheck, &allKeys1, &allKeys2, TWENTY_RECORDS);
    readThread.detach();
    ConsistencyCheckTransaction(entries1, entries2);
    g_batchThreadComplete = true;

    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(MILLSECONDES_PER_SECOND));
    ASSERT_TRUE(g_batchThreadSuccess);
}

void TransactionSubThread1()
{
    /**
     * @tc.steps: step2. start transanction and update v1 of k1 to v2, and put (k3, v3) to STORE_ID_1.
     * @tc.expected: step2. start and put successfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_3, VALUE_3), OK);
}
void TransactionSubThread2()
{
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(MILLSECONDES_PER_SECOND));

    /**
     * @tc.steps: step3. update v2 of k2 to v3, and delete k1, and Get (k3, v3) from DB
     * @tc.expected: step3. put and delete successfully, but can't find (k3, v3) in DB.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_2, VALUE_3), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_1), OK);
    Value value;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_3, value), OK);
    EXPECT_EQ(value, VALUE_3);
}
void TransactionSubThread3()
{
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(MILLSECONDES_PER_SECOND));

    /**
     * @tc.steps: step4. rollback
     * @tc.expected: step4. rollback successfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), OK);
}
/**
 * @tc.name: Transaction 017
 * @tc.desc: Verify that one delegate start transaction, and then the different delegate of the same storeid are
 *     already in the same transaction.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction017, TestSize.Level2)
{
    /**
     * @tc.steps: step1. put (k1, v1), (k2, v2) to STORE_ID_1.
     * @tc.expected: step1. put successfully.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_2, VALUE_2), OK);

    std::thread thread1, thread2, thread3;
    thread1 = std::thread(TransactionSubThread1);
    thread1.join();
    thread2 = std::thread(TransactionSubThread2);
    thread2.join();
    thread3 = std::thread(TransactionSubThread3);
    thread3.join();

    /**
     * @tc.steps: step5. check the records in db
     * @tc.expected: step5. the value of k1 is v1, the value of k2 is v2, but (k3, v3) is not in db.
     */
    Value value;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_1);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_2, value), OK);
    EXPECT_EQ(value, VALUE_2);
    value.clear();
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_3, value), NOT_FOUND);
    EXPECT_TRUE(value.empty());
}

/**
 * @tc.name: Transaction 018
 * @tc.desc: Verify that transaction has persistence.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction018, TestSize.Level1)
{
    KvStoreNbDelegate *delegate1 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    delegate1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager1, g_dbParameter2, g_option);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr) << "get delegate1 failed.";
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start transactionsuccessfully.
     */
    EXPECT_EQ(delegate1->StartTransaction(), OK);
    /**
     * @tc.steps: step2. putBatch (k1, v1), (k2, v2) to STORE_ID_1.
     * @tc.expected: step2. putBatch successfully.
     */
    std::vector<DistributedDB::Entry> entries = {ENTRY_1, ENTRY_2};
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*delegate1, entries), OK);

    /**
     * @tc.steps: step3. commit the transaction.
     * @tc.expected: step3. commit successfully.
     */
    EXPECT_EQ(delegate1->Commit(), OK);
    /**
     * @tc.steps: step4. close the db but don't delete the STORE, and then create a new delegate of the same STORE
     * @tc.expected: step4. close and create successfully.
     */
    EXPECT_EQ(manager1->CloseKvStore(delegate1), OK);
    delegate1 = nullptr;
    delete manager1;
    manager1 = nullptr;

    KvStoreNbDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    delegate2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager2, g_dbParameter2, g_option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr) << "get delegate2 failed.";
    /**
     * @tc.steps: step5. check the data in db STORE.
     * @tc.expected: step5. the entries is still in the db.
     */
    std::vector<DistributedDB::Entry> entriesGot;
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*delegate2, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entriesGot, entries));

    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    EXPECT_EQ(manager2->DeleteKvStore(STORE_ID_2), OK);
    delete manager2;
    manager2 = nullptr;
}

/**
 * @tc.name: Transaction 019
 * @tc.desc: Verify that close the delegate before transaction commit, and it will cause the transaction rollback.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction019, TestSize.Level1)
{
    KvStoreNbDelegate *delegate1 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    delegate1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager1, g_dbParameter2, g_option);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr) << "get delegate1 failed.";
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start transactionsuccessfully.
     */
    EXPECT_EQ(delegate1->StartTransaction(), OK);
    /**
     * @tc.steps: step2. putBatch (k1, v1), (k2, v2) to STORE_ID_1.
     * @tc.expected: step2. putBatch successfully.
     */
    std::vector<DistributedDB::Entry> entries = {ENTRY_1, ENTRY_2};
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*delegate1, entries), OK);

    /**
     * @tc.steps: step3. close the db but don't delete the STORE without committing the transaction
     * @tc.expected: step3. close successfully.
     */
    EXPECT_EQ(manager1->CloseKvStore(delegate1), OK);
    delegate1 = nullptr;
    delete manager1;
    manager1 = nullptr;

    /**
     * @tc.steps: step4. create a new delegate of the same STORE
     * @tc.expected: step4. start successfully.
     */
    KvStoreNbDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    delegate2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager2, g_dbParameter2, g_option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr) << "get delegate2 failed.";
    /**
     * @tc.steps: step5. commit the transaction on the new deleate, and check the data in db STORE.
     * @tc.expected: step5. commit failed and the entries is NULL.
     */
    EXPECT_EQ(delegate2->Commit(), DB_ERROR);
    std::vector<DistributedDB::Entry> entriesGot;
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*delegate2, KEY_EMPTY, entriesGot), NOT_FOUND);
    EXPECT_TRUE(entriesGot.empty());

    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    EXPECT_EQ(manager2->DeleteKvStore(STORE_ID_2), OK);
    delete manager2;
    manager2 = nullptr;
}

/**
 * @tc.name: Transaction 020
 * @tc.desc: Verify that close the delegate before transaction rollback, and it will cause the transaction rollback.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start transactionsuccessfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);
    /**
     * @tc.steps: step2. putBatch (k1, v1), (k2, v2) to STORE_ID_1.
     * @tc.expected: step2. putBatch successfully.
     */
    std::vector<DistributedDB::Entry> entries = {ENTRY_1, ENTRY_2};
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries), OK);

    /**
     * @tc.steps: step3. close the db but don't delete the STORE without committing the transaction
     * @tc.expected: step3. close successfully.
     */
    EXPECT_EQ(g_manager->CloseKvStore(g_nbBatchCrudDelegate), OK);
    g_nbBatchCrudDelegate = nullptr;
    delete g_manager;
    g_manager = nullptr;

    /**
     * @tc.steps: step4. create a new delegate of the same STORE
     * @tc.expected: step4. start successfully.
     */
    g_nbBatchCrudDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, g_option);
    ASSERT_TRUE(g_manager != nullptr && g_nbBatchCrudDelegate != nullptr) << "get g_nbBatchCrudDelegate failed.";
    /**
     * @tc.steps: step5. Rollback the transaction on the new deleate, and check the data in db STORE.
     * @tc.expected: step5. Rollback failed and the entries is NULL.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), DB_ERROR);
    std::vector<DistributedDB::Entry> entriesGot;
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), NOT_FOUND);
    EXPECT_TRUE(entriesGot.empty());
}

/**
 * @tc.name: Transaction 021
 * @tc.desc: Verify that it will failed to delete the store before transaction rollback.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction021, TestSize.Level0)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start transactionsuccessfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);
    /**
     * @tc.steps: step2. put (k1, v1) to STORE_ID_1.
     * @tc.expected: step2. put successfully.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    DistributedDB::Value value;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), OK);
    EXPECT_EQ(value, VALUE_1);

    /**
     * @tc.steps: step3. delete the STORE without committing or rollback
     *     the transaction
     * @tc.expected: step3. delete failed and returned BUSY.
     */
    EXPECT_EQ(g_manager->DeleteKvStore(STORE_ID_1), BUSY);

    /**
     * @tc.steps: step4. Rollback and check the db
     * @tc.expected: step4. Rollback failed.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Rollback(), OK);

    /**
     * @tc.steps: step5. check the data in db STORE.
     * @tc.expected: step5. the DB is empty.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Get(*g_nbBatchCrudDelegate, KEY_1, value), NOT_FOUND);
}

/**
 * @tc.name: Transaction 022
 * @tc.desc: Verify transaction can't support resultSet, rekey, Import, Export, RegisterObserver interface.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, Transaction022, TestSize.Level3)
{
    KvStoreResultSet *resultSet1 = nullptr;
    KvStoreResultSet *resultSet2 = nullptr;
    EXPECT_EQ(g_nbBatchCrudDelegate->GetEntries(KEY_EMPTY, resultSet1), OK);
    KvStoreObserverImpl observer1, observer2;
    vector<Entry> entries, entriesGot;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, TEN_RECORDS);

    DBStatus status = g_nbBatchCrudDelegate->RegisterObserver(KEY_K, OBSERVER_CHANGES_NATIVE, &observer1);
    EXPECT_EQ(status, OK);

    const std::string importPath = NB_DIRECTOR + "import";
    const std::string importFilePath = importPath + "/importbkpDB.bin";
    SetDir(importPath);
    EXPECT_EQ(g_nbBatchCrudDelegate->Export(importFilePath, NULL_PASSWD), OK);

    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start transaction successfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);
    /**
     * @tc.steps: step2. putBatch entries to db.
     * @tc.expected: step2. putBatch successfully.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries, entriesGot));

    vector<DistributedDB::Entry> observerCheckEntry;
    observerCheckEntry.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, ListType::INSERT_LIST, observerCheckEntry));

    /**
     * @tc.steps: step3. call rekey, import, export interface
     * @tc.expected: step3. all call failed and returned BUSY.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Rekey(g_passwd1), BUSY);

    const std::string exportPath = NB_DIRECTOR + "export";
    const std::string filePath = exportPath + "/bkpDB.bin";
    SetDir(exportPath);
    EXPECT_EQ(g_nbBatchCrudDelegate->Export(filePath, NULL_PASSWD), BUSY);
    EXPECT_EQ(g_nbBatchCrudDelegate->Import(importFilePath, NULL_PASSWD), BUSY);
    /**
     * @tc.steps: step4. call GetEntries to obtain resultSet2, and register observer2.
     * @tc.expected: step4. all call failed and returned BUSY.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->GetEntries(KEY_EMPTY, resultSet2), BUSY);
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_K, OBSERVER_CHANGES_NATIVE, &observer2), BUSY);

    /**
     * @tc.steps: step5. close resultSet1 and resultSet2, UnRegister observer1 and observer2.
     * @tc.expected: step5. close resultSet1 succeed and resultSet2 failed,
     *     UnRegister observer1 succeed and observer2 failed.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->CloseResultSet(resultSet1), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->CloseResultSet(resultSet2), INVALID_ARGS);
    EXPECT_EQ(g_nbBatchCrudDelegate->UnRegisterObserver(&observer1), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->UnRegisterObserver(&observer2), NOT_FOUND);
    /**
     * @tc.steps: step6. commit the transaction and check the data in db.
     * @tc.expected: step6. commit succeed and can find entries in DB.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
    EXPECT_EQ(DistributedDBNbTestTools::GetEntries(*g_nbBatchCrudDelegate, KEY_EMPTY, entriesGot), OK);
    EXPECT_TRUE(CompareEntriesVector(entries, entriesGot));
}

/**
 * @tc.name: TransactionObserver 001
 * @tc.desc: Verify transaction can and only trigger observer one time.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, TransactionObserver001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Register observer1 and observer2 of k1 and k2 separately and start the transaction.
     * @tc.expected: step1. Register start transaction successfully.
     */
    KvStoreObserverImpl observer1, observer2;
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observer1), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_NATIVE, &observer2), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    /**
     * @tc.steps: step2. put (k1, v1), (k3, v3) and check the callback of the observer.
     * @tc.expected: step2. put successfully and both of the observers can't receive the callback.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_3, VALUE_3), OK);
    vector<DistributedDB::Entry> observerCheckList;
    observerCheckList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, INSERT_LIST, observerCheckList));
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, INSERT_LIST, observerCheckList));
    /**
     * @tc.steps: step3. update (k1, v1) to (k1, v2) and delete (k3, v3) and check the observers.
     * @tc.expected: step3. update and delete successfully both of the observers can't receive the callback either.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_3), OK);
    observerCheckList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, INSERT_LIST, observerCheckList));
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, DELETE_LIST, observerCheckList));

    /**
     * @tc.steps: step4. commit the transaction and check the observers.
     * @tc.expected: step4. commit succeed and observer1 was triggered one time and observer2 wasn't triggered.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
    observerCheckList.clear();
    observerCheckList.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, observerCheckList));
    observerCheckList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, DELETE_LIST, observerCheckList));
}

/**
 * @tc.name: TransactionObserver 002
 * @tc.desc: Verify transaction can and only trigger observer one time.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, TransactionObserver002, TestSize.Level1)
{
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_2, VALUE_2), OK);
    /**
     * @tc.steps: step1. Register observer1, observer2, observer3, observer4 of k1, k2, k3
     *     and k4 separately and start the transaction.
     * @tc.expected: step1. Register and start transaction successfully.
     */
    KvStoreObserverImpl observer1, observer2, observer3, observer4;
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observer1), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_NATIVE, &observer2), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_3, OBSERVER_CHANGES_NATIVE, &observer3), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_4, OBSERVER_CHANGES_NATIVE, &observer4), OK);
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);

    /**
     * @tc.steps: step2. put (k1, v1), (k2, v2), (k3, v3) and (k4, v4) and then delete (k1, v1) and (k4, v4).
     * @tc.expected: step2. put and delete successfully.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_2, VALUE_3), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_3, VALUE_3), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_4, VALUE_4), OK);

    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_4), OK);

    /**
     * @tc.steps: step3. commit the transaction and check the observers.
     * @tc.expected: step3. commit succeed and observer1 received one delete notify, observer2 received one update
     *     notify, observer3 received one insert notify, and observer4 didn't receive any notify.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
    vector<DistributedDB::Entry> observerCheckList;
    observerCheckList.clear();
    observerCheckList.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, DELETE_LIST, observerCheckList));
    observerCheckList.clear();
    observerCheckList.push_back(ENTRY_2_3);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, UPDATE_LIST, observerCheckList));
    observerCheckList.clear();
    observerCheckList.push_back(ENTRY_3);
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ONE_TIME, INSERT_LIST, observerCheckList));
    observerCheckList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer4, CHANGED_ZERO_TIME, DELETE_LIST, observerCheckList));
}

/**
 * @tc.name: TransactionObserver 003
 * @tc.desc: Verify the different observer of same key will all be triggered after transaction
 *     and will only be trigger once.
 * @tc.type: FUNC
 * @tc.require: SR000DORPP
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbBatchCrudTest, TransactionObserver003, TestSize.Level1)
{
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_1, VALUE_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_2, VALUE_2), OK);
    /**
     * @tc.steps: step1. Register 8 observers of all keys.
     * @tc.expected: step1. Register successfully.
     */
    std::vector<KvStoreObserverImpl> observers(OBSERVER_NUM);
    for (unsigned long cnt = 0; cnt < OBSERVER_NUM; cnt++) {
        EXPECT_EQ(g_nbBatchCrudDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_NATIVE, &observers[cnt]), OK);
    }

    /**
     * @tc.steps: step2. start the transaction and putBatch (k3, v3), (k4, v4), ..., (k8, v8) to db.
     * @tc.expected: step2. start transaction and putBatch successfully.
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->StartTransaction(), OK);
    std::vector<DistributedDB::Entry> entries = {ENTRY_3, ENTRY_4, ENTRY_5, ENTRY_6, ENTRY_7, ENTRY_8, ENTRY_9};
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbBatchCrudDelegate, entries), OK);

    /**
     * @tc.steps: step3. delete (k1, v1), (k3, v3) and (k9, v9), update (k2, v2) to (k2, v3).
     * @tc.expected: step3. operate succeed.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*g_nbBatchCrudDelegate, KEY_2, VALUE_3), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_3), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*g_nbBatchCrudDelegate, KEY_9), OK);

    /**
     * @tc.steps: step3. commit the transaction and check the observers.
     * @tc.expected: step3. commit succeed and all of the observers received one insert notify, callbacklist of which
     *    contains (k4, v4), ..., (k8, v8), all observer received one update notify, callbacklist of which contains
     *    (k2, v3), all observers received one insert notify, callback of which contains (k1, v1), (k3, v3), (k9, v9).
     */
    EXPECT_EQ(g_nbBatchCrudDelegate->Commit(), OK);
    vector<DistributedDB::Entry> observerCheckList;
    for (unsigned long cnt = 0; cnt < OBSERVER_NUM; cnt++) {
        observerCheckList.clear();
        observerCheckList.push_back(ENTRY_4);
        observerCheckList.push_back(ENTRY_5);
        observerCheckList.push_back(ENTRY_6);
        observerCheckList.push_back(ENTRY_7);
        observerCheckList.push_back(ENTRY_8);
        EXPECT_TRUE(VerifyObserverResult(observers[cnt], CHANGED_ONE_TIME, INSERT_LIST, observerCheckList));
        observerCheckList.clear();
        observerCheckList.push_back(ENTRY_2_3);
        EXPECT_TRUE(VerifyObserverResult(observers[cnt], CHANGED_ONE_TIME, UPDATE_LIST, observerCheckList));
        observerCheckList.clear();
        observerCheckList.push_back(ENTRY_1);
        EXPECT_TRUE(VerifyObserverResult(observers[cnt], CHANGED_ONE_TIME, DELETE_LIST, observerCheckList));
    }
}
}
