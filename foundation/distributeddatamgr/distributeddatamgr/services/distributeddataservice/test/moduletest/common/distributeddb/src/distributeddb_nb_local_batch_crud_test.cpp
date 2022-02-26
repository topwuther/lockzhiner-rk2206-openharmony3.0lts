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
#include "distributeddb_data_generator.h"
#include "process_communicator_test_stub.h"
#include "distributeddb_schema_test_tools.h"

using namespace std;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbNbLocalBatchCrud {
KvStoreNbDelegate *g_nbLocalBatchDelegate = nullptr;
KvStoreDelegateManager *g_manager = nullptr;

class DistributeddbNbLocalBatchCrudTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbLocalBatchCrudTest::SetUpTestCase(void)
{
    KvStoreDelegateManager manager(APP_ID_1, USER_ID_1);
    manager.SetProcessLabel("MST", "GetDevicesID");
    manager.SetProcessCommunicator(std::make_shared<ProcessCommunicatorTestStub>());
}

void DistributeddbNbLocalBatchCrudTest::TearDownTestCase(void)
{
}

void DistributeddbNbLocalBatchCrudTest::SetUp(void)
{
    RemoveDir(NB_DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    g_nbLocalBatchDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, g_option);
    ASSERT_TRUE(g_nbLocalBatchDelegate != nullptr);
    ASSERT_TRUE(g_manager != nullptr);
}

void DistributeddbNbLocalBatchCrudTest::TearDown(void)
{
    MST_LOG("TearDownTestCase after case.");
    ASSERT_NE(g_manager, nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(g_manager, g_nbLocalBatchDelegate, STORE_ID_1, g_option.isMemoryDb));
    RemoveDir(NB_DIRECTOR);
}

/**
 * @tc.name: SimpleData 001
 * @tc.desc: Verify that single-ver db can support PutLocalBatch to insert <keys,values> and update records db file,
 *    and DeleteLocalBatch from db.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, SimpleData001, TestSize.Level0)
{
    vector<Entry> entries1, entries2, gotValues;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries1.push_back(ENTRY_3);
    entries1.push_back(ENTRY_4);
    entries2.push_back(ENTRY_1_2);
    entries2.push_back(ENTRY_3_4);

    /**
     * @tc.steps: step1. call PutLocalBatch interface to Put (k1, v1), (k2, v2), (k3, v3), (k4, v4) to DB
     *    and check the data in DB.
     * @tc.expected: step1. PutLocalBatch successfully and there are (k1, v1), (k2, v2), (k3, v3), (k4, v4) in DB.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));

    /**
     * @tc.steps: step2. call PutLocalBatch interface to update (k1, v1), (k3, v3) to (k1, v2), (k3, v4)
     *    and check the data in DB.
     * @tc.expected: step2. PutLocalBatch successfully and there are (k1, v2), (k2, v2), (k3, v4), (k4, v4) in DB.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries2), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    entries1[0] = ENTRY_1_2;
    entries1[2] = ENTRY_3_4;
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));

    /**
     * @tc.steps: step3. call DeleteLocalBatch interface to delete (k3, v4), (k4, v4) from db and check the data in DB.
     * @tc.expected: step3. DeleteLocalBatch successfully and there are only (k1, v2), (k2, v2) in DB.
     */
    vector<Key> keys;
    keys.push_back(ENTRY_3.key);
    keys.push_back(ENTRY_4.key);
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    entries1.pop_back();
    entries1.pop_back();
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));
}

/**
 * @tc.name: SimpleData 002
 * @tc.desc: Verify that single-ver db can support PutLocalBatch records that value of which is null, but can't support
 *    the value of which bigger than 4M.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, SimpleData002, TestSize.Level0)
{
    vector<Entry> entries1, entries2, gotValues;
    entries1.push_back(ENTRY_1);
    entries1.push_back({.key = KEY_2, .value = {}});

    /**
     * @tc.steps: step1. call PutLocalBatch interface to Put (k1, v1), (k2, null) to DB and check the data in DB.
     * @tc.expected: step1. PutLocalBatch successfully and there are (k1, v1), (k2, null) in DB.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));

    /**
     * @tc.steps: step2. call PutLocalBatch interface put (k3, v3), (k3, v4) to db, size of v4 is 4M + 1
     *    and then check the data in DB.
     * @tc.expected: step2. PutLocalBatch failed and there are only (k1, v1), (k2, null) in DB.
     */
    entries2.push_back(ENTRY_3);
    Entry entry;
    EntrySize entrySize = {KEY_ONE_K_BYTE, FOUR_M_LONG_STRING + 1};
    unsigned long keyNo = 1;
    GenerateRandRecord(entry, entrySize, keyNo);
    entries2.push_back(entry);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries2), INVALID_ARGS);

    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));
}

/**
 * @tc.name: SimpleData 003
 * @tc.desc: Verify that PutLocalBatch can't operate records key of which is null or key is bigger than 1k.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, SimpleData003, TestSize.Level0)
{
    vector<Entry> entries1, entries2, gotValues;
    entries1.push_back(ENTRY_1);
    entries1.push_back({.key = {}, .value = VALUE_2});

    /**
     * @tc.steps: step1. call PutLocalBatch interface to Put (k1, v1), (null, v2) to DB.
     * @tc.expected: step1. PutLocalBatch failed.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), INVALID_ARGS);

    /**
     * @tc.steps: step2. call PutLocalBatch interface put (k3, v3), (k4, v4) to db, size of k4 is 1K + 1
     *    and then check the data in DB.
     * @tc.expected: step2. PutLocalBatch failed and there are only (k1, v1), (k2, null) in DB.
     */
    entries2.push_back(ENTRY_3);
    Entry entry;
    EntrySize entrySize = {KEY_ONE_K_BYTE + 1, FOUR_M_LONG_STRING};
    unsigned long keyNo = 1;
    GenerateRandRecord(entry, entrySize, keyNo);
    entries2.push_back(entry);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries2), INVALID_ARGS);

    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), NOT_FOUND);
}

/**
 * @tc.name: SimpleData 004
 * @tc.desc: Verify that DeleteLocalBatch can't operate records key of which is null or key is bigger than 1k.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, SimpleData004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. call PutLocalBatch interface to Put (k1, v1), (k2, v2), (k3, v3), (k4, v4) to DB, and check
     *    the data from.
     * @tc.expected: step1. PutLocalBatch succeed and can find (k1, v1), (k2, v2), (k3, v3), (k4, v4) from db.
     */
    vector<Entry> entries1, entries2, gotValues;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries1.push_back(ENTRY_3);
    entries1.push_back(ENTRY_4);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));

    vector<Key> keys1, keys2;
    keys1.push_back(ENTRY_1.key);
    Key key(KEY_ONE_K_BYTE + 1, 'k');
    keys1.push_back(key);

    /**
     * @tc.steps: step2. call DeleteLocalBatch interface to Delete k1, k5 from DB, size of k5 is 1K + 1.
     * @tc.expected: step2. DeleteLocalBatch failed and all the data is still in db.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys1), INVALID_ARGS);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));

    /**
     * @tc.steps: step3. call DeleteLocalBatch interface delete k2, k6 from db, size of k6 is null
     *    and then check the data in DB.
     * @tc.expected: step3. DeleteLocalBatch failed and all the data is still in db.
     */
    keys2.push_back(ENTRY_2.key);
    keys2.push_back({});
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys2), INVALID_ARGS);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));
}

/**
 * @tc.name: SimpleData 005
 * @tc.desc: Verify that DeleteLocalBatch can delete records that is not exist.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, SimpleData005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. call PutLocalBatch interface to Put (k1, v1), (k2, v2) to DB, and check the data in db.
     * @tc.expected: step1. PutLocalBatch succeed and can find (k1, v1), (k2, v2) from db.
     */
    vector<Entry> entries1, entries2, gotValues;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));

    vector<Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);
    keys.push_back(KEY_3);

    /**
     * @tc.steps: step2. call DeleteLocalBatch interface to Delete k1, k2, k3 from DB.
     * @tc.expected: step2. DeleteLocalBatch succeed and can't find data in db.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), NOT_FOUND);

    /**
     * @tc.steps: step3. call DeleteLocalBatch interface again to Delete k1, k2, k3 from DB.
     * @tc.expected: step3. DeleteLocalBatch succeed and can't find data in db.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), NOT_FOUND);
}

/**
 * @tc.name: SimpleData 006
 * @tc.desc: Verify that PutLocalBatch and DeleteLocalBatch can't operate 129 records one time.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, SimpleData006, TestSize.Level0)
{
    EXPECT_EQ(DistributedDBNbTestTools::PutLocal(*g_nbLocalBatchDelegate, KEY_1, VALUE_1), OK);
    /**
     * @tc.steps: step1. call PutLocalBatch interface to insert 128 records to DB, and check the data from DB.
     * @tc.expected: step1. PutLocalBatch succeed and can find 129 records in DB.
     */
    vector<Entry> entries1, entries2, gotValues;
    vector<Key> allKeys1, allKeys2;
    GenerateFixedRecords(entries1, allKeys1, BATCH_RECORDS, KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE);

    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_EQ(entries1.size() + 1, gotValues.size());

    /**
     * @tc.steps: step2. call PutLocalBatch interface to insert 129 records to DB, and check the data from DB.
     * @tc.expected: step2. PutLocalBatch failed and can only find 129 records in DB.
     */
    GenerateFixedRecords(entries2, allKeys2, BATCH_RECORDS + 1, KEY_EIGHT_BYTE, VALUE_ONE_HUNDRED_BYTE);
    EXPECT_EQ(g_nbLocalBatchDelegate->PutLocalBatch(entries2), INVALID_ARGS);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_EQ(entries1.size() + 1, gotValues.size());

    /**
     * @tc.steps: step3. call DeleteLocalBatch interface to Delete 129 records from DB which is already in DB.
     * @tc.expected: step3. DeleteLocalBatch failed and can still find the 129 records which was find upstairs in db.
     */
    allKeys1.push_back(KEY_1);
    EXPECT_EQ(g_nbLocalBatchDelegate->DeleteLocalBatch(allKeys1), INVALID_ARGS);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_EQ(entries1.size() + 1, gotValues.size());
}

#ifndef LOW_LEVEL_MEM_DEV
/**
 * @tc.name: SimpleData 007
 * @tc.desc: Verify that rekey will return busy when it is PutLocalBatch-ing.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, SimpleData007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. start one new thread to use PutLocalBatch interface to insert 128 (1k, 4M) records to DB.
     * @tc.expected: step1. PutLocalBatch succeed.
     */
    vector<Entry> entries;
    EntrySize entrySize = {KEY_ONE_K_BYTE, FOUR_M_LONG_STRING};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, BATCH_RECORDS);

    std::condition_variable batchCondition;
    std::mutex batchMtx;
    bool putBatchFinish = false;
    thread subThread([&]() {
        DBStatus inserStatus = DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries);
        EXPECT_TRUE(inserStatus == OK || inserStatus == BUSY);
        {
            std::lock_guard<std::mutex> lock(batchMtx);
            putBatchFinish = true;
        }
        batchCondition.notify_one();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. rekey the DB when it is PutLocalBatch-ing.
     * @tc.expected: step2. rekey failed and it will return busy.
     */
    DistributedDB::CipherPassword passwd;
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_FIFTY_MS));
    DBStatus rekeyStatus = g_nbLocalBatchDelegate->Rekey(passwd);
    EXPECT_TRUE(rekeyStatus == BUSY || rekeyStatus == OK);

    std::unique_lock<std::mutex> lck(batchMtx);
    batchCondition.wait(lck, [&] { return putBatchFinish; });
}

/**
 * @tc.name: SimpleData 008
 * @tc.desc: Verify that import will return busy when it is PutLocalBatch-ing.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, SimpleData008, TestSize.Level3)
{
    EXPECT_EQ(g_nbLocalBatchDelegate->Put(KEY_1, VALUE_1), OK);

    std::string exportPath = NB_DIRECTOR + "export";
    SetDir(exportPath);
    std::string filePath = exportPath + "/bkpDB.bin";
    EXPECT_TRUE(g_nbLocalBatchDelegate->Export(filePath, NULL_PASSWD) == OK);
    /**
     * @tc.steps: step1. call PutLocalBatch interface to insert 128 records to DB.
     * @tc.expected: step1. PutLocalBatch succeed.
     */
    vector<Entry> entries;
    EntrySize entrySize = {KEY_ONE_K_BYTE, FOUR_M_LONG_STRING};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, BATCH_RECORDS);

    std::condition_variable batchCondition;
    std::mutex batchMtx;
    bool putBatchFinish = false;
    thread subThread([&]() {
        DBStatus insertStatus = DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries);
        EXPECT_TRUE(insertStatus == OK || insertStatus == BUSY);
        {
            std::lock_guard<std::mutex> lock(batchMtx);
            putBatchFinish = true;
        }
        batchCondition.notify_one();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. import the backup file of the db.
     * @tc.expected: step2. import failed and return busy.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_FIFTY_MS));
    DBStatus importStatus = g_nbLocalBatchDelegate->Import(filePath, NULL_PASSWD);
    EXPECT_TRUE(importStatus == BUSY || importStatus == OK);

    std::unique_lock<std::mutex> lck(batchMtx);
    batchCondition.wait(lck, [&] { return putBatchFinish; });
}
#endif
#ifndef OMIT_JSON
/**
 * @tc.name: SimpleData 009
 * @tc.desc: Verify that open the schema db use non-schema mode, it can open success on readOnly mode and can be CRUD.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, SimpleData009, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option1 = g_option;
    string schemaDefine = "{\"field0\":\"INTEGER,NOT NULL,DEFAULT 10\",\"field1\":[]}";
    option1.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, schemaDefine, VALID_INDEX_1);
    option1.isMemoryDb = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option1);
    ASSERT_NE(delegate, nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delete manager;
    manager = nullptr;

    /**
     * @tc.steps: step1. open the schema db with non-schema mode, and then do PutLocal, GetLocal, DeleteLocal.
     * @tc.expected: step1. open successfully and all operate successfully.
     */
    Option option2 = g_option;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option2);
    ASSERT_NE(delegate, nullptr);

    EXPECT_EQ(DistributedDBNbTestTools::PutLocal(*delegate, KEY_1, VALUE_1), OK);
    Value gotValue;
    EXPECT_EQ(delegate->GetLocal(KEY_1, gotValue), OK);
    EXPECT_TRUE(CompareVector(gotValue, VALUE_1));
    EXPECT_EQ(DistributedDBNbTestTools::PutLocal(*delegate, KEY_1, VALUE_2), OK);
    EXPECT_EQ(delegate->DeleteLocal(KEY_1), OK);
    /**
     * @tc.steps: step2. PutLocalBatch 128 records, GetLocalEntries PutLocalBatch to update it
     *    and then DeleteLocalBatch them.
     * @tc.expected: step2. operate successfully.
     */
    vector<Entry> entries1, entries2, gotEntries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries1, entrySize, BATCH_RECORDS);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*delegate, entries1), OK);
    EXPECT_EQ(delegate->GetLocalEntries(KEY_EMPTY, gotEntries), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotEntries));

    GenerateAppointPrefixAndSizeRecords(entries2, entrySize, BATCH_RECORDS, {'k'}, {'w'});
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*delegate, entries2), OK);

    vector<Key> keys;
    for (auto iter : entries2) {
        keys.push_back(iter.key);
    }
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*delegate, keys), OK);

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option2.isMemoryDb));
}
#endif
/**
 * @tc.name: ComplexData 001
 * @tc.desc: Verify that it can continuously insert update and delete
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, ComplexData001, TestSize.Level1)
{
    vector<Entry> entries1, entries2, gotValues;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries2.push_back(ENTRY_1_2);
    entries2.push_back(ENTRY_2_3);
    vector<Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);

    for (int index = 0; index < HUNDRED_TIMES; index++) {
        /**
         * @tc.steps: step1. call PutLocalBatch interface to insert entries1 to DB.
         * @tc.expected: step1. PutLocalBatch succeed.
         */
        EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);
        EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
        EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));

        /**
         * @tc.steps: step2. call PutLocalBatch interface to update entries1 to entries2.
         * @tc.expected: step2. update successfully.
         */
        EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries2), OK);
        EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
        EXPECT_TRUE(CompareEntriesVector(entries2, gotValues));

        /**
         * @tc.steps: step3. call DeleteLocalBatch interface to delete the records.
         * @tc.expected: step3. delete successfully.
         */
        EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys), OK);
        EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), NOT_FOUND);
    }
}

/**
 * @tc.name: ComplexData 002
 * @tc.desc: Verify that it can continuously insert update and delete
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, ComplexData002, TestSize.Level0)
{
    vector<Entry> entries1, entries2, gotValues;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries2.push_back(ENTRY_1_3);
    entries2.push_back(ENTRY_2_4);
    entries2.push_back(ENTRY_3);
    vector<Key> keys;
    keys.push_back(KEY_2);
    keys.push_back(KEY_3);

    /**
     * @tc.steps: step1. call PutLocalBatch interface to insert entries1 to DB.
     * @tc.expected: step1. PutLocalBatch succeed.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));

    /**
     * @tc.steps: step2. call PutLocalBatch interface to update entries1 to entries2.
     * @tc.expected: step2. update successfully.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries2), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries2, gotValues));

    /**
     * @tc.steps: step3. call DeleteLocalBatch interface to delete the records.
     * @tc.expected: step3. delete successfully.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys), OK);
    entries2.pop_back();
    entries2.pop_back();
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries2, gotValues));
}

/**
 * @tc.name: Exception 001
 * @tc.desc: Verify that local operate will calculate the total items in transantion.
 * @tc.type: EXCEPTION
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, Exception001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start the transantion.
     * @tc.expected: step1. start success
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->StartTransaction(), OK);
    /**
     * @tc.steps: step2. call PutBatch interface to insert 128 records to sync DB, and call PutLocalBatch interface to
     *    insert (k1, v1) (k2, v2) to local DB.
     * @tc.expected: step2. PutBatch succeed but PutLocalBatch will return OVER_MAX_LIMITS.
     */
    vector<Entry> entries1, entries2, gotValues;
    EntrySize entrySize = {KEY_SIX_BYTE, ONE_K_LONG_STRING};
    GenerateAppointPrefixAndSizeRecords(entries1, entrySize, BATCH_RECORDS);
    entries2.push_back(ENTRY_1);
    entries2.push_back(ENTRY_2);
    EXPECT_EQ(DistributedDBNbTestTools::PutBatch(*g_nbLocalBatchDelegate, entries1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries2), OVER_MAX_LIMITS);

    /**
     * @tc.steps: step3. call DeleteLocalBatch interface to Delete the 128 records from local DB.
     * @tc.expected: step3. DeleteLocalBatch failed and returned OVER_MAX_LIMITS.
     */
    vector<Key> keys1, keys2;
    for (vector<Entry>::iterator it = entries1.begin(); it != entries1.end(); it++) {
        keys1.push_back(it->key);
    }
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys1), OVER_MAX_LIMITS);

    /**
     * @tc.steps: step4. call Delete interface to Delete the (k1, v1) from local DB.
     * @tc.expected: step4. Delete failed and returned OVER_MAX_LIMITS.
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->Delete(KEY_1), OVER_MAX_LIMITS);

    /**
     * @tc.steps: step5. call DeleteLocalBatch interface to Delete (k1, v1)(k2, v2) from sync DB
     *     and then commit the transantion.
     * @tc.expected: step5. Delete failed and returned OVER_MAX_LIMITS and the transantion commit success.
     */
    keys2.push_back(KEY_1);
    keys2.push_back(KEY_2);
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys2), OVER_MAX_LIMITS);
    EXPECT_EQ(g_nbLocalBatchDelegate->Commit(), OK);

    /**
     * @tc.steps: step6. call GetEntries and GetLocalEntries interface to check the data on sync and local DB.
     * @tc.expected: step6. there are 128 records in sync DB but local DB is empty.
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->GetEntries(KEY_EMPTY, gotValues), OK);
    EXPECT_TRUE(CompareEntriesVector(entries1, gotValues));
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotValues), NOT_FOUND);
}

/**
 * @tc.name: Observer 001
 * @tc.desc: Verify that different observer for different key will be triggered when PutLocalBatch.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, Observer001, TestSize.Level0)
{
    KvStoreObserverImpl observer1, observer2, observer3;
    /**
     * @tc.steps: step1. register observer of k1, k2, k3 of OBSERVER_CHANGES_LOCAL_ONLY mode.
     * @tc.expected: step1. register success.
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observer1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_LOCAL_ONLY, &observer2), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_3, OBSERVER_CHANGES_LOCAL_ONLY, &observer3), OK);

    /**
     * @tc.steps: step2. PutLocalBatch (k1, v1), (k2, v2) to DB and check the 3 observers.
     * @tc.expected: step2. PutLocalBatch and observer1, and observer2 was triggered but observer3 was not triggered.
     */
    vector<Entry> entries1, entries2;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);
    list<DistributedDB::Entry> changeList;
    changeList.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, changeList));
    changeList.clear();
    changeList.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, INSERT_LIST, changeList));
    changeList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ZERO_TIME, INSERT_LIST, changeList));
    /**
     * @tc.steps: step3. PutLocalBatch (k2, v3) (k3, v3) to insert (k3, v3) and update (k2, v3) and check the observers
     * @tc.expected: step3. PutLocalBatch success and observer1 was not triggered, observer2 was triggered update mode,
     *    and observer3 was triggered by insert mode.
     */
    observer1.Clear();
    observer2.Clear();
    entries2.push_back(ENTRY_2_3);
    entries2.push_back(ENTRY_3);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries2), OK);
    changeList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, INSERT_LIST, changeList));
    changeList.clear();
    changeList.push_back(ENTRY_2_3);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, UPDATE_LIST, changeList));
    changeList.clear();
    changeList.push_back(ENTRY_3);
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ONE_TIME, INSERT_LIST, changeList));
}

/**
 * @tc.name: Observer 002
 * @tc.desc: Verify that different observer for different key will be triggered when DeleteLocalBatch.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, Observer002, TestSize.Level0)
{
    KvStoreObserverImpl observer1, observer2, observer3;
    vector<Entry> entries, gotEntries;
    entries.push_back(ENTRY_1);
    entries.push_back(ENTRY_2);
    entries.push_back(ENTRY_3);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries), OK);
    /**
     * @tc.steps: step1. register observer of k1, k2, k3 of OBSERVER_CHANGES_LOCAL_ONLY mode.
     * @tc.expected: step1. register success.
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observer1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_LOCAL_ONLY, &observer2), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_3, OBSERVER_CHANGES_LOCAL_ONLY, &observer3), OK);

    /**
     * @tc.steps: step2. DeleteLocalBatch (k1, v1), (k2, v2) from DB and check the 3 observers.
     * @tc.expected: step2. DeleteLocalBatch success and observer1, and observer2 was triggered by delete mode,
     *    but observer3 was not triggered.
     */
    vector<Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys), OK);
    list<DistributedDB::Entry> changeList;
    changeList.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, DELETE_LIST, changeList));
    changeList.clear();
    changeList.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, DELETE_LIST, changeList));
    changeList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ZERO_TIME, DELETE_LIST, changeList));

    /**
     * @tc.steps: step3. use GetLocalEntries to check the data in DB.
     * @tc.expected: step3. there is only (k3, v3) in DB.
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->GetLocalEntries(KEY_EMPTY, gotEntries), OK);
    entries.erase(entries.begin(), entries.begin() + 2);
    EXPECT_TRUE(CompareEntriesVector(entries, gotEntries));
}

/**
 * @tc.name: Observer 003
 * @tc.desc: Verify that PutLocalBatch operator one key many times, the observer can still be triggered but will be
 *    triggered one time.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, Observer003, TestSize.Level0)
{
    KvStoreObserverImpl observer1, observer2, observer3;
    /**
     * @tc.steps: step1. register observer of k1, k2, k3 of OBSERVER_CHANGES_LOCAL_ONLY mode.
     * @tc.expected: step1. register success.
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observer1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_LOCAL_ONLY, &observer2), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_3, OBSERVER_CHANGES_LOCAL_ONLY, &observer3), OK);

    /**
     * @tc.steps: step2. PutLocalBatch (k1, v1), (k1, v2), (k2, v2) to DB and check the 3 observers.
     * @tc.expected: step2. PutLocalBatch success and observer1 was triggered once time by insert mode,
     *    and observer2 was triggered by insert mode, but observer3 was not triggered.
     */
    vector<Entry> entries1, entries2;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_1_2);
    entries1.push_back(ENTRY_2);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);

    list<DistributedDB::Entry> changeList;
    changeList.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, changeList));
    changeList.clear();
    changeList.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, INSERT_LIST, changeList));
    changeList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ZERO_TIME, INSERT_LIST, changeList));

    /**
     * @tc.steps: step3. PutLocalBatch (k2, v3), (k2, v4), (k3, v3) to DB and check the 3 observers.
     * @tc.expected: step3. PutLocalBatch success and observer1 was not triggered, and observer2 was triggered by
     *    update mode, and observer3 was triggered once by insert mode.
     */
    observer1.Clear();
    observer2.Clear();
    entries2.push_back(ENTRY_2_3);
    entries2.push_back(ENTRY_2_4);
    entries2.push_back(ENTRY_3);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries2), OK);

    changeList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, UPDATE_LIST, changeList));
    changeList.clear();
    changeList.push_back(ENTRY_2_4);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, UPDATE_LIST, changeList));
    changeList.clear();
    changeList.push_back(ENTRY_3);
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ONE_TIME, INSERT_LIST, changeList));
}

/**
 * @tc.name: Observer 004
 * @tc.desc: Verify that DeleteLocalBatch operator one key many times, the observer can still be triggered but will be
 *    triggered one time.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, Observer004, TestSize.Level0)
{
    KvStoreObserverImpl observer1, observer2, observer3;
    vector<Entry> entries1, entries2;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries1.push_back(ENTRY_3);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries1), OK);
    /**
     * @tc.steps: step1. register observer of k1, k2, k3 of OBSERVER_CHANGES_LOCAL_ONLY mode.
     * @tc.expected: step1. register success.
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observer1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_LOCAL_ONLY, &observer2), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_3, OBSERVER_CHANGES_LOCAL_ONLY, &observer3), OK);

    /**
     * @tc.steps: step2. DeleteLocalBatch (k1, k2, k1) from DB and check the 3 observers.
     * @tc.expected: step2. DeleteLocalBatch success and observer1 and observer2 was triggered once time by delete mode,
     *    but observer3 was not triggered.
     */
    vector<Key> keys;
    keys.push_back(KEY_1);
    keys.push_back(KEY_2);
    keys.push_back(KEY_1);
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys), OK);
    list<DistributedDB::Entry> changeList;
    changeList.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, DELETE_LIST, changeList));
    changeList.clear();
    changeList.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, DELETE_LIST, changeList));
    changeList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer3, CHANGED_ZERO_TIME, DELETE_LIST, changeList));
}

/**
 * @tc.name: Observer 005
 * @tc.desc: Verify that different observers of the same key will be triggered when the key is changed
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, Observer005, TestSize.Level0)
{
    vector<KvStoreObserverImpl> observer(OBSERVER_CNT_END);
    /**
     * @tc.steps: step1. register observer all the 8 observers of OBSERVER_CHANGES_LOCAL_ONLY mode.
     * @tc.expected: step1. register success.
     */
    for (unsigned long index = 0; index < OBSERVER_CNT_END; index++) {
        EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY,
            &observer[index]), OK);
    }

    /**
     * @tc.steps: step2. PutLocalBatch  10 data to DB and check the 8 observers.
     * @tc.expected: step2. PutLocalBatch success and all the observers were triggered one insert callback and the
     *    callback list contains 10 data.
     */
    vector<Entry> entries;
    vector<Key> keys;
    GenerateFixedRecords(entries, keys, TEN_RECORDS, KEY_SIX_BYTE, ONE_K_LONG_STRING);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries), OK);
    list<DistributedDB::Entry> changeList;
    for (auto it = entries.begin(); it != entries.end(); it++) {
        changeList.push_back(*it);
    }
    for (auto it = observer.begin(); it != observer.end(); it++) {
        EXPECT_TRUE(VerifyObserverResult(*it, CHANGED_ONE_TIME, INSERT_LIST, changeList));
        it->Clear();
    }
    /**
     * @tc.steps: step3. use PutLocalBatch to update the 10 data to DB and check the 8 observers.
     * @tc.expected: step3. update success and all the observers were triggered one update callback and the
     *    callback list contains 10 data.
     */
    for (auto it = entries.begin(); it != entries.end(); it++) {
        it->value = {'v', 'v', 'v', 'v', 'v'};
    }
    EXPECT_EQ(DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, entries), OK);
    changeList.clear();
    for (auto it = entries.begin(); it != entries.end(); it++) {
        it->value = {'v', 'v', 'v', 'v', 'v'};
        changeList.push_back(*it);
    }
    for (auto it = observer.begin(); it != observer.end(); it++) {
        EXPECT_TRUE(VerifyObserverResult(*it, CHANGED_ONE_TIME, UPDATE_LIST, changeList));
        it->Clear();
    }
    /**
     * @tc.steps: step4. use DeleteLocalBatch to delete the 10 data from DB and check the 8 observers.
     * @tc.expected: step4. delete success and all the observers were triggered one delete callback and the
     *    callback list contains 10 data.
     */
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, keys), OK);
    for (auto it = observer.begin(); it != observer.end(); it++) {
        EXPECT_TRUE(VerifyObserverResult(*it, CHANGED_ONE_TIME, DELETE_LIST, changeList));
    }
}

/**
 * @tc.name: Observer 006
 * @tc.desc: Operation data committed in a transaction will trigger batch callback in registered observer
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, Observer006, TestSize.Level0)
{
    EXPECT_EQ(g_nbLocalBatchDelegate->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->Put(KEY_2, VALUE_2), OK);
    vector<Entry> insertEntries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_HUNDRED_BYTE};
    GenerateAppointPrefixAndSizeRecords(insertEntries, entrySize, TEN_RECORDS, {'k'}, {'v', 'm'});
    DBStatus status = DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, insertEntries);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step1. register a local and a native observer separately, then start a transaction.
     * @tc.expected: step1. operate successfully.
     */
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerNative;
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_NATIVE, &observerNative), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->StartTransaction(), OK);
    /**
     * @tc.steps: step2. PutBatch 10 records, Put (k1,v1), Delete (k2).
     * @tc.expected: step2. operate successfully.
     */
    status = DistributedDBNbTestTools::PutBatch(*g_nbLocalBatchDelegate, insertEntries);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->Put(KEY_1, VALUE_2), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->Delete(KEY_2), OK);
    /**
     * @tc.steps: step3. PutLocal (k1,v1), (k2,v2), DeleteLocal (k1,v1),
     *  use PutLocalBatch to update records from k1 to k5, use DeleteLocalBatch to delete keys from k6 to k10.
     * @tc.expected: step3. operate successfully.
     */
    EXPECT_EQ(DistributedDBNbTestTools::PutLocal(*g_nbLocalBatchDelegate, KEY_1, VALUE_1), OK);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocal(*g_nbLocalBatchDelegate, KEY_2, VALUE_2), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->DeleteLocal(KEY_1), OK);
    vector<Entry> localUpdateEntries;
    GenerateAppointPrefixAndSizeRecords(localUpdateEntries, entrySize, FIVE_RECORDS, {'k'}, {'v', 'n'});
    status = DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, localUpdateEntries);
    EXPECT_EQ(status, OK);

    vector<Key> localDeleteKeys;
    vector<Entry> localDeleteEntries;
    for (size_t index = INDEX_FIFTH; index < insertEntries.size(); index++) {
        localDeleteKeys.push_back(insertEntries[index].key);
        localDeleteEntries.push_back(insertEntries[index]);
    }
    EXPECT_EQ(DistributedDBNbTestTools::DeleteLocalBatch(*g_nbLocalBatchDelegate, localDeleteKeys), OK);
    /**
     * @tc.steps: step4. commit the transaction and verify corresponding data.
     * @tc.expected: step4. delete success and all the observers were triggered one delete callback and the
     *    callback list contains 10 data.
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->Commit(), OK);
    EXPECT_TRUE(VerifyObserverResult(observerNative, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    vector<Entry> updateEntries = { ENTRY_1_2 };
    EXPECT_TRUE(VerifyObserverResult(observerNative, CHANGED_ONE_TIME, UPDATE_LIST, updateEntries));
    vector<Entry> deleteEntries = { ENTRY_2 };
    EXPECT_TRUE(VerifyObserverResult(observerNative, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));

    vector<Entry> localInsertEntries = { ENTRY_2 };
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, INSERT_LIST, localInsertEntries));
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, UPDATE_LIST, localUpdateEntries));
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, DELETE_LIST, localDeleteEntries));

    EXPECT_EQ(g_nbLocalBatchDelegate->UnRegisterObserver(&observerLocal), OK);
    EXPECT_EQ(g_nbLocalBatchDelegate->UnRegisterObserver(&observerNative), OK);
}

/**
 * @tc.name: Observer 007
 * @tc.desc: observer callback data only can be received after a transaction committed
 *  in multi-thread environment using the same delegate and observer.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbLocalBatchCrudTest, Observer007, TestSize.Level0)
{
    /**
     * @tc.steps: step1. register a local observer.
     * @tc.expected: step1. operate successfully.
     */
    KvStoreObserverImpl observerLocal;
    EXPECT_EQ(g_nbLocalBatchDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal), OK);
    /**
     * @tc.steps: step2. start a transaction, then PutBatch 10 records, PutLocal (k10,v10),
     *  and PutLocalBatch from k1 to k5.
     * @tc.expected: step2. operate successfully.
     */
    EXPECT_EQ(g_nbLocalBatchDelegate->StartTransaction(), OK);
    EXPECT_EQ(DistributedDBNbTestTools::PutLocal(*g_nbLocalBatchDelegate, KEY_10, VALUE_10), OK);
    vector<Entry> insertEntries = { ENTRY_1, ENTRY_2, ENTRY_3, ENTRY_4, ENTRY_5 };
    DBStatus status = DistributedDBNbTestTools::PutLocalBatch(*g_nbLocalBatchDelegate, insertEntries);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step3. launch a thread to verify observer callback data, commit the transaction
     *  and then verify the data again.
     * @tc.expected: step3. there is no data before committing the transaction
     *  and there are 6 records in insert list of the local observer after committing.
     */
    insertEntries.push_back(ENTRY_10);
    thread subThread([&]() {
        vector<Entry> emptyInsertEntries;
        EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, emptyInsertEntries));
        EXPECT_EQ(g_nbLocalBatchDelegate->Commit(), OK);
        EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    });
    subThread.join();
    /**
     * @tc.steps: step4. verify the data in main thread after the transaction committed in sub thread.
     * @tc.expected: step4. there are 6 records in insert list of the local observer.
     */
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    EXPECT_EQ(g_nbLocalBatchDelegate->UnRegisterObserver(&observerLocal), OK);
}
} // end of namespace DistributeddbNbLocalBatchCrud