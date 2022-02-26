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
#include <thread>

#include "db_errno.h"
#include "db_constant.h"
#include "log_print.h"
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "sqlite_single_ver_natural_store.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    KvStoreConfig g_config;
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;

    const int OBSERVER_SLEEP_TIME = 100;
    const int BATCH_PRESET_SIZE_TEST = 10;
    const int DIVIDE_BATCH_PRESET_SIZE = 5;
    const int VALUE_OFFSET = 5;
    const int DEFAULT_KEY_VALUE_SIZE = 10;

    const Key KEY1{'k', 'e', 'y', '1'};
    const Key KEY2{'k', 'e', 'y', '2'};
    const Value VALUE1{'v', 'a', 'l', 'u', 'e', '1'};
    const Value VALUE2{'v', 'a', 'l', 'u', 'e', '2'};

    const std::string VALID_SCHEMA_STRICT_DEFINE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\","
            "\"field_name2\":\"INTEGER, NOT NULL\""
        "},"
        "\"SCHEMA_INDEXES\":[\"$.field_name1\"]}";

    CipherPassword g_passwd;
    KvStoreNbDelegate::Option g_strictOpt = {
        true, false, false, CipherType::DEFAULT, g_passwd,
        VALID_SCHEMA_STRICT_DEFINE
    };

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));

    static void CreatEntrys(int recordSize, vector<Key> &keys, vector<Value> &values, vector<Entry> &entries)
    {
        keys.clear();
        values.clear();
        entries.clear();
        for (int i = 0; i < recordSize; i++) {
            string temp = to_string(i);
            Entry entry;
            Key keyTemp;
            Value valueTemp;
            for (auto &iter : temp) {
                entry.key.push_back(iter);
                entry.value.push_back(iter);
                keyTemp.push_back(iter);
                valueTemp.push_back(iter);
            }
            keys.push_back(keyTemp);
            values.push_back(valueTemp);
            entries.push_back(entry);
        }
    }
}

class DistributedDBInterfacesNBDelegateLocalBatchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesNBDelegateLocalBatchTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesNBDelegateLocalBatchTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesNBDelegateLocalBatchTest::SetUp(void)
{
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvNbDelegatePtr = nullptr;
}

void DistributedDBInterfacesNBDelegateLocalBatchTest::TearDown(void)
{
    if (g_kvNbDelegatePtr != nullptr) {
        g_mgr.CloseKvStore(g_kvNbDelegatePtr);
        g_kvNbDelegatePtr = nullptr;
    }
}

/**
  * @tc.name: PutLocalBatch001
  * @tc.desc: This test case use to verify the PutLocalBatch interface function
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, PutLocalBatch001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get singleVer kvStore by GetKvStore.
     * @tc.expected: step1. Get database success.
     */
    const KvStoreNbDelegate::Option option = {true, true};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_PutLocalBatch_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps: step2. Insert 10 records into database.
     * @tc.expected: step2. Insert successfully.
     */
    vector<Entry> entries;
    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i);
        entries.push_back(entry);
    }

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entries), OK);

    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Key key;
        key.push_back(i);
        Value value;
        g_kvNbDelegatePtr->GetLocal(key, value);
        EXPECT_EQ(key, value);
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutLocalBatch001
  * @tc.desc: Check for illegal parameters
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerPutLocalBatch001, TestSize.Level1)
{
    /**
     * @tc.steps: step1.
     *  Create and construct three sets of vector <Entry>, each set of three data contains records:
     *  (K1, V1) It is illegal for K1 to be greater than 1K, and V1 is 1K in size
     *  (K2, V2) K2 is legal, V2 is greater than 4M
     *  (K3, V3) are not legal.
     */
    Key illegalKey;
    DistributedDBToolsUnitTest::GetRandomKeyValue(illegalKey, DBConstant::MAX_KEY_SIZE + 1); // 1K + 1
    Value illegalValue;
    DistributedDBToolsUnitTest::GetRandomKeyValue(illegalValue, DBConstant::MAX_VALUE_SIZE + 1); // 4M + 1
    vector<Entry> entrysKeyIllegal = {KV_ENTRY_1, KV_ENTRY_2, {illegalKey, VALUE_3}};
    vector<Entry> entrysValueIllegal = {KV_ENTRY_1, KV_ENTRY_2, {KEY_3, illegalValue}};
    vector<Entry> entrysIllegal = {KV_ENTRY_1, KV_ENTRY_2, {illegalKey, illegalValue}};

    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutLocalBatch_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step2. PutBatch operates on three sets of data.
     * @tc.expected: step2. All three operations return INVALID_ARGS.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysKeyIllegal), INVALID_ARGS);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysValueIllegal), INVALID_ARGS);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysIllegal), INVALID_ARGS);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutLocalBatch_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutLocalBatch002
  * @tc.desc: PutLocalBatch normal insert function test.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerPutLocalBatch002, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutLocalBatch_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step1.
     *  Create and build 4 groups of vector <Entry>, which are:
     *  Vect of empty objects;
     *  Vect1 of a legal Entry record;
     *  128 legal Entry records Vect2;
     *  129 legal Entry records Vect3;
     */
    vector<Entry> entrysMaxNumber;
    for (size_t i = 0; i < DBConstant::MAX_BATCH_SIZE; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i);
        entrysMaxNumber.push_back(entry);
    }
    Key keyTemp = {'1', '1'};
    Value valueTemp;
    Entry entryTemp = {keyTemp, VALUE_1};
    vector<Entry> entrysOneRecord = {entryTemp};
    vector<Entry> entrysOverSize = entrysMaxNumber;
    entrysOverSize.push_back(entryTemp);
    /**
     * @tc.steps: step2. PutBatch operates on four sets of data. and use get check the result of Vect3.
     * @tc.expected: step2. Returns INVALID_ARGS for 129 records, and returns OK for the rest. all get return NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysOverSize), INVALID_ARGS);
    for (size_t i = 0; i < entrysOverSize.size(); i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysOverSize[i].key, valueTemp), NOT_FOUND);
    }
    /**
     * @tc.steps: step3. Use get check the result of Vect2.
     * @tc.expected: step3. Return OK and get the correct value.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysOneRecord), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keyTemp, valueTemp), OK);
    EXPECT_EQ(valueTemp, VALUE_1);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysMaxNumber), OK);
     /**
     * @tc.steps: step4. Use get check the result of Vect3.
     * @tc.expected: step4. Return OK and get the correct value.
     */
    for (size_t i = 0; i < entrysMaxNumber.size(); i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysMaxNumber[i].key, valueTemp), OK);
        EXPECT_EQ(valueTemp, entrysMaxNumber[i].value);
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutLocalBatch_002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutLocalBatch003
  * @tc.desc: Check interface atomicity
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerPutLocalBatch003, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutLocalBatch_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step1. Create and construct a set of vector <Entry> with a total of 128 data,
     * including one illegal data. And call PutBatch interface to insert.
     */
    vector<Entry> entrysMaxNumber;
    for (size_t i = 0; i < DBConstant::MAX_BATCH_SIZE; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i);
        entrysMaxNumber.push_back(entry);
    }
    Key illegalKey;
    Value valueTemp;
    DistributedDBToolsUnitTest::GetRandomKeyValue(illegalKey, DBConstant::MAX_KEY_SIZE + 1); // 1K + 1
    entrysMaxNumber[0].key = illegalKey;

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysMaxNumber), INVALID_ARGS);
    /**
     * @tc.steps: step2. Use Get interface to query 128 corresponding key values.
     * @tc.expected: step2. All Get interface return NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysMaxNumber[0].key, valueTemp), INVALID_ARGS);
    for (size_t i = 1; i < entrysMaxNumber.size(); i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysMaxNumber[i].key, valueTemp), NOT_FOUND);
    }
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutLocalBatch_003"), OK);
    g_kvNbDelegatePtr = nullptr;
}

static void PreparePutLocalBatch004(vector<Entry> &entrys1, vector<Entry> &entrys2, vector<Entry> &entrys3)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerPutLocalBatch_004", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i);
        entrys1.push_back(entry);
    }

    for (int i = 0; i < DIVIDE_BATCH_PRESET_SIZE; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i + VALUE_OFFSET);
        entrys2.push_back(entry);
    }

    for (int i = DIVIDE_BATCH_PRESET_SIZE; i < BATCH_PRESET_SIZE_TEST; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(i - VALUE_OFFSET);
        entrys3.push_back(entry);
    }
}

/**
  * @tc.name: SingleVerPutLocalBatch004
  * @tc.desc: Check interface data insertion and update functions.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerPutLocalBatch004, TestSize.Level1)
{
    /**
     * @tc.steps: step1.
     *  Construct three groups of three vector <Entry>:
     *  (1) entrys1: key1 ~ 10, corresponding to Value1 ~ 10;
     *  (2) entrys2: key1 ~ 5, corresponding to Value6 ~ 10;
     *  (3) entrys3: key6 ~ 10, corresponding to Value1 ~ 5;
     */
    vector<Entry> entrys1;
    vector<Entry> entrys2;
    vector<Entry> entrys3;
    PreparePutLocalBatch004(entrys1, entrys2, entrys3);
    /**
     * @tc.steps: step2. PutBatch entrys2.
     * @tc.expected: step2. PutBatch return OK.
     */
    Value valueRead;
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrys2), OK);
    /**
     * @tc.steps: step3. Check PutBatch result.
     * @tc.expected: step3. Get correct value of key1~5. Key6~10 return NOT_FOUND.
     */
    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Key keyTemp;
        keyTemp.push_back(i);
        if (i < DIVIDE_BATCH_PRESET_SIZE) {
            Value valueTemp;
            valueTemp.push_back(i + VALUE_OFFSET);
            EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keyTemp, valueRead), OK);
            EXPECT_EQ(valueRead, valueTemp);
            continue;
        }
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keyTemp, valueRead), NOT_FOUND);
    }
    /**
     * @tc.steps: step4. PutBatch entrys1.
     * @tc.expected: step4. PutBatch return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrys1), OK);
    /**
     * @tc.steps: step5. Check PutBatch result.
     * @tc.expected: step5. Update and insert value of key1~10 to value1~10.
     */
    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Key keyTemp;
        keyTemp.push_back(i);
        if (i < DIVIDE_BATCH_PRESET_SIZE) {
            EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keyTemp, valueRead), OK);
            EXPECT_EQ(valueRead, keyTemp);
            continue;
        }
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keyTemp, valueRead), OK);
        EXPECT_EQ(valueRead, keyTemp);
    }
    /**
     * @tc.steps: step6. PutBatch entrys3.
     * @tc.expected: step6. PutBatch return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrys3), OK);
    /**
     * @tc.steps: step7. Check PutBatch result of key1~10.
     * @tc.expected: step7. Update value of key5~10 to value1~5.
     */
    for (int i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        Key keyTemp;
        keyTemp.push_back(i);
        if (i < DIVIDE_BATCH_PRESET_SIZE) {
            EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keyTemp, valueRead), OK);
            EXPECT_EQ(valueRead, keyTemp);
            continue;
        }
        Value valueTemp;
        valueTemp.push_back(i - VALUE_OFFSET);
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keyTemp, valueRead), OK);
        EXPECT_EQ(valueRead, valueTemp);
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutLocalBatch_004"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerDeleteLocalBatch001
  * @tc.desc: Check for illegal parameters.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerDeleteLocalBatch001, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerDeleteLocalBatch_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step1. Create and construct a set of vector <Entry>, containing a total of 10 data keys1 ~ 10,
     *  Value1 ~ 10, and call Putbatch interface to insert data.
     * @tc.expected: step1. PutBatch successfully.
     */
    vector<Entry> entries;
    vector<Key> keys;
    vector<Value> values;
    Value valueRead;
    CreatEntrys(BATCH_PRESET_SIZE_TEST, keys, values, entries);
    vector<Entry> entrysBase = entries;
    vector<Key> keysBase = keys;
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysBase), OK);
    /**
     * @tc.steps: step2. Use Get to check data in database.
     * @tc.expected: step2. Get value1~10 by key1~10 successfully.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysBase[i].key, valueRead), OK);
    }
    /**
     * @tc.steps: step3. Use DeleteBatch interface to transfer 10 + 119 extra keys (total 129).
     * @tc.expected: step3. Return INVALID_ARGS.
     */
    CreatEntrys(DBConstant::MAX_BATCH_SIZE + 1, keys, values, entries);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keys), INVALID_ARGS);
    /**
     * @tc.steps: step4. Use Get to check data in database.
     * @tc.expected: step4. Key1~10 still in database.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysBase[i].key, valueRead), OK);
    }
    /**
     * @tc.steps: step5. Use the DeleteBatch interface to pass in 10 included
     *  keys6 ~ 10 + 123 additional key values ​​(128 in total).
     * @tc.expected: step5. DeleteBatch OK.
     */
    CreatEntrys(DBConstant::MAX_BATCH_SIZE + DIVIDE_BATCH_PRESET_SIZE, keys, values, entries);
    keys.erase(keys.begin(), keys.begin() + DIVIDE_BATCH_PRESET_SIZE);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keys), OK);
    /**
     * @tc.steps: step6. Use Get to check key1~10 in database.
     * @tc.expected: step6. Key1~5 in database, key6~10 have been deleted.
     */
    for (size_t i = 0; i < DIVIDE_BATCH_PRESET_SIZE; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysBase[i].key, valueRead), OK);
    }
    for (size_t i = DIVIDE_BATCH_PRESET_SIZE; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysBase[i].key, valueRead), NOT_FOUND);
    }
    /**
     * @tc.steps: step7. Repeat Putbatch key1~10, value1~10.
     * @tc.expected: step7. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysBase), OK);

    Key illegalKey;
    DistributedDBToolsUnitTest::GetRandomKeyValue(illegalKey, DBConstant::MAX_KEY_SIZE + 1); // 1K + 1
    keysBase.push_back(illegalKey);
    /**
     * @tc.steps: step8. Use DeleteBatch interface to pass in 10 + 1(larger than 1K) keys.
     * @tc.expected: step8. Return INVALID_ARGS.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keysBase), INVALID_ARGS);
    /**
     * @tc.steps: step9. Use Get to check key1~10 in database.
     * @tc.expected: step9. Delete those data failed.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysBase[i].key, valueRead), OK);
    }
    /**
     * @tc.steps: step10. Use DeleteBatch interface to pass in 10(in database) + 1 valid keys.
     * @tc.expected: step10. Delete those data successfully.
     */
    keysBase.back().erase(keysBase.back().begin(), keysBase.back().begin() + 1);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keysBase), OK);
    /**
     * @tc.steps: step11. Check data.
     * @tc.expected: step11. DeleteBatch successfully.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(entrysBase[i].key, valueRead), NOT_FOUND);
    }

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerDeleteLocalBatch_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerDeleteLocalBatch002
  * @tc.desc: Check normal delete batch ability.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerDeleteLocalBatch002, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.SetKvStoreConfig(g_config);
    g_mgr.GetKvStore("distributed_SingleVerDeleteLocalBatch_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps: step1. Create a group of vector <Entry>, containing a total of 10 data keys1 ~ 10, Value1 ~ 10,
     *  call the Putbatch interface to insert data.
     * @tc.expected: step1. Insert to database successfully.
     */
    vector<Entry> entries;
    vector<Key> keysBase;
    vector<Value> values;
    CreatEntrys(BATCH_PRESET_SIZE_TEST, keysBase, values, entries);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entries), OK);
    /**
     * @tc.steps: step2. Check data.
     * @tc.expected: step2. Get key1~10 successfully.
     */
    Value valueRead;
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keysBase[i], valueRead), OK);
    }
    /**
     * @tc.steps: step3. DeleteBatch key1~5.
     * @tc.expected: step3. Return OK.
     */
    vector<Key> keys(keysBase.begin(), keysBase.begin() + DIVIDE_BATCH_PRESET_SIZE);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keys), OK);
    /**
     * @tc.steps: step4. Check key1~10.
     * @tc.expected: step4. Key1~5 deleted, key6~10 existed.
     */
    for (size_t i = 0; i < DIVIDE_BATCH_PRESET_SIZE; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keysBase[i], valueRead), NOT_FOUND);
    }
    for (size_t i = DIVIDE_BATCH_PRESET_SIZE; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keysBase[i], valueRead), OK);
    }
    /**
     * @tc.steps: step5. DeleteBatch key1~10.
     * @tc.expected: step5. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keysBase), OK);
    /**
     * @tc.steps: step6. Check key1~10.
     * @tc.expected: step6. Key1~10 deleted successfully.
     */
    for (size_t i = 0; i < BATCH_PRESET_SIZE_TEST; i++) {
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(keysBase[i], valueRead), NOT_FOUND);
    }
    /**
     * @tc.steps: step7. DeleteBatch key1~10 once again.
     * @tc.expected: step7. Return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keysBase), OK);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerDeleteLocalBatch_002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutLocalBatchObserver001
  * @tc.desc: Test the observer function of PutLocalBatch() interface.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerPutLocalBatchObserver001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerPutLocalBatchObserver_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, observer), OK);
    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    vector<Entry> entrysBase;
    vector<Key> keysBase;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST + 1, entrysBase, keysBase);

    vector<Entry> entries(entrysBase.begin(), entrysBase.end() - 1);
    EXPECT_EQ(entries.size(), 10UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entries), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entries, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step4. Delete the batch data.
     * @tc.expected: step4. Returns OK.
     */
    vector<Key> keys(keysBase.begin() + 5, keysBase.end());
    EXPECT_EQ(keys.size(), 6UL);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keys), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    vector<Entry> entrysDel(entrysBase.begin() + 5, entrysBase.end() - 1);
    EXPECT_EQ(entrysDel.size(), 5UL);
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrysDel, observer->GetEntriesDeleted()));
    /**
     * @tc.steps:step5. UnRegister the observer.
     * @tc.expected: step5. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;

    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutLocalBatchObserver_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutLocalBatchObserver002
  * @tc.desc: Test the observer function of PutLocalBatch() for invalid input.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerPutLocalBatchObserver002, TestSize.Level4)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerPutLocalBatchObserver_002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, observer), OK);
    /**
     * @tc.steps:step3. Put 129 batch data.
     * @tc.expected: step3. Returns INVALID_ARGS.
     */
    vector<Entry> entrys1;
    vector<Key> keys1;
    DistributedDBUnitTest::GenerateRecords(DBConstant::MAX_BATCH_SIZE + 1, entrys1, keys1);

    EXPECT_EQ(entrys1.size(), 129UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrys1), INVALID_ARGS);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(observer->GetEntriesInserted().empty());
    /**
     * @tc.steps:step4. Put invalid batch data.
     * @tc.expected: step4. Returns INVALID_ARGS.
     */
    vector<Entry> entrys2;
    vector<Key> keys2;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys2, keys2);
    EXPECT_EQ(entrys2.size(), 10UL);

    vector<Entry> entrysInvalid;
    vector<Key> keysInvalid;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrysInvalid, keysInvalid,
        DBConstant::MAX_KEY_SIZE + 10);
    EXPECT_EQ(entrysInvalid.size(), 10UL);
    entrys2[0].key = entrysInvalid[0].key;

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrys2), INVALID_ARGS);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(observer->GetEntriesInserted().empty());
    /**
     * @tc.steps:step5. Put MAX valid value batch data.
     * @tc.expected: step5. Returns OK.
     */
    vector<Entry> entrys3;
    vector<Key> keys3;

    DistributedDBUnitTest::GenerateRecords(DBConstant::MAX_BATCH_SIZE, entrys3, keys3);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrys3), OK);
    LOGD("sleep begin");
    // sleep 20 seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME * 10));
    LOGD("sleep end");
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrys3, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step6. UnRegister the observer.
     * @tc.expected: step6. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;

    /**
     * @tc.steps:step7. Close the kv store.
     * @tc.expected: step7. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutLocalBatchObserver_002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutLocalBatchObserver003
  * @tc.desc: Test the observer function of PutLocalBatch() update function.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerPutLocalBatchObserver003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerPutLocalBatchObserver_003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, observer), OK);
    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    vector<Entry> entrysAdd;
    vector<Key> keysAdd;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrysAdd, keysAdd);

    EXPECT_EQ(entrysAdd.size(), 10UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysAdd), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrysAdd, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step4. Update the batch data.
     * @tc.expected: step4. Returns OK.
     */
    vector<Entry> entrysUpdate;
    vector<Key> keysUpdate;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrysUpdate, keysUpdate, DEFAULT_KEY_VALUE_SIZE,
        DEFAULT_KEY_VALUE_SIZE + 10);

    EXPECT_EQ(entrysUpdate.size(), 10UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrysUpdate), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrysUpdate, observer->GetEntriesUpdated()));
    /**
     * @tc.steps:step5. UnRegister the observer.
     * @tc.expected: step5. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;

    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutLocalBatchObserver_003"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPutLocalBatchObserver004
  * @tc.desc: Test the observer function of PutLocalBatch(), same keys handle.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerPutLocalBatchObserver004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerPutLocalBatchObserver_004", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, observer), OK);
    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    vector<Entry> entrys1;
    vector<Key> keys1;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys1, keys1);
    vector<Entry> entrys2;
    vector<Key> keys2;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys2, keys2, DEFAULT_KEY_VALUE_SIZE,
        DEFAULT_KEY_VALUE_SIZE + 10);
    entrys1.insert(entrys1.end(), entrys2.begin(), entrys2.end());

    EXPECT_EQ(entrys1.size(), 20UL);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrys1), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrys2, observer->GetEntriesInserted()));
    EXPECT_EQ(observer->GetEntriesUpdated().size(), 0UL);

    vector<Entry> entrys3;
    vector<Key> keys3;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys3, keys3, DEFAULT_KEY_VALUE_SIZE,
        DEFAULT_KEY_VALUE_SIZE + 20);
    vector<Entry> entrys4;
    vector<Key> keys4;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entrys4, keys4, DEFAULT_KEY_VALUE_SIZE,
        DEFAULT_KEY_VALUE_SIZE + 30);
    entrys3.insert(entrys3.end(), entrys4.begin(), entrys4.end());
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entrys3), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entrys4, observer->GetEntriesUpdated()));
    EXPECT_EQ(observer->GetEntriesInserted().size(), 0UL);

    /**
     * @tc.steps:step4. UnRegister the observer.
     * @tc.expected: step4. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;

    /**
     * @tc.steps:step5. Close the kv store.
     * @tc.expected: step5. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerPutLocalBatchObserver_004"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerDeleteLocalBatchObserver001
  * @tc.desc: Test the observer function of DeleteLocalBatch() interface.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, SingleVerDeleteLocalBatchObserver001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("distributed_SingleVerDeleteLocalBatchObserver_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, observer), OK);
    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    vector<Entry> entries;
    vector<Key> keys;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entries, keys);
    EXPECT_EQ(entries.size(), 10UL);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entries), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entries, observer->GetEntriesInserted()));
    /**
     * @tc.steps:step4. Delete the batch data.
     * @tc.expected: step4. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keys), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entries, observer->GetEntriesDeleted()));
    /**
     * @tc.steps:step5. UnRegister the observer.
     * @tc.expected: step5. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;

    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_SingleVerDeleteLocalBatchObserver_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}
#ifndef OMIT_JSON
/**
  * @tc.name: LocalDataBatchNotCheckSchema001
  * @tc.desc: Local data does not check schema.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, LocalDataBatchNotCheckSchema001, TestSize.Level1)
{
    g_mgr.GetKvStore("distributed_LocalDataBatchNotCheckSchema_001", g_strictOpt, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step1. Put one data whose value has more fields than the schema.
     * @tc.expected: step1. Return OK, because PutLocal does not verify the validity of the schema.
     */
    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    std::string moreData = "{\"field_name1\":true,\"field_name2\":10,\"field_name3\":10}";
    Value value(moreData.begin(), moreData.end());
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(key, value), OK);
    Value getValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(key, getValue), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsValueEqual(getValue, value));

    /**
     * @tc.steps:step2. Delete local data
     * @tc.expected: step2. DeleteLocal return OK, GetLocal return NOT_FOUND
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocal(key), OK);
    getValue.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(key, getValue), NOT_FOUND);

    /**
     * @tc.steps:step3. PutLocalBatch local data whose value is mismatch with the schema.
     * @tc.expected: step3. return OK.
     */
    key.clear();
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    std::string invalidData = "{\"field_name1\":true, \"field_name2\":null}";
    value.assign(invalidData.begin(), invalidData.end());
    std::vector<Key> keys;
    std::vector<Entry> entries;
    entries.push_back({key, value});
    keys.push_back(key);

    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    std::string validData = "{\"field_name1\":true, \"field_name2\":0}";
    value.assign(validData.begin(), validData.end());
    entries.push_back({key, value});
    keys.push_back(key);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entries), OK);
    std::vector<Entry> getEntries;
    Key keyPrefix;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries(keyPrefix, getEntries), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(entries, getEntries, true));

    /**
     * @tc.steps:step4. Delete local data
     * @tc.expected: step4. DeleteLocal return OK, GetLocal return NOT_FOUND
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keys), OK);
    getEntries.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries(keyPrefix, getEntries), NOT_FOUND);
    EXPECT_TRUE(getEntries.empty());

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_LocalDataBatchNotCheckSchema_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: LocalDataBatchNotCheckReadOnly001
  * @tc.desc: Local data does not check readOnly.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesNBDelegateLocalBatchTest, LocalDataBatchNotCheckReadOnly001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Open the kv store with valid schema, and close it.
     * @tc.expected: step1. opened & closeed successfully - return OK.
     */
    g_mgr.GetKvStore("distributed_LocalDataBatchNotCheckReadOnly_001", g_strictOpt, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    /**
     * @tc.steps:step2. Open the kv store with no schema.
     * @tc.expected: step2. return OK.
     */
    DistributedDB::KvStoreNbDelegate::Option option = g_strictOpt;
    option.schema.clear();
    g_mgr.GetKvStore("distributed_LocalDataBatchNotCheckReadOnly_001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step3. CRUD single local the data.
     * @tc.expected: step3. return OK.
     */
    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    std::string valueData = "{\"field_name1\":true,\"field_name2\":20}";
    Value value(valueData.begin(), valueData.end());
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(key, value), OK);

    Value getValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(key, getValue), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsValueEqual(getValue, value));

    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocal(key), OK);
    getValue.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(key, getValue), NOT_FOUND);

    /**
     * @tc.steps:step3. CRUD batch local the data.
     * @tc.expected: step3. return OK.
     */
    key.clear();
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    std::string invalidData = "{\"field_name1\":true, \"field_name2\":null}";
    value.assign(invalidData.begin(), invalidData.end());
    std::vector<Key> keys;
    std::vector<Entry> entries;
    entries.push_back({key, value});
    keys.push_back(key);

    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    std::string validData = "{\"field_name1\":true, \"field_name2\":0}";
    value.assign(validData.begin(), validData.end());
    entries.push_back({key, value});
    keys.push_back(key);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(entries), OK);
    std::vector<Entry> getEntries;
    Key keyPrefix;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries(keyPrefix, getEntries), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(entries, getEntries, true));

    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(keys), OK);
    getEntries.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries(keyPrefix, getEntries), NOT_FOUND);
    EXPECT_TRUE(getEntries.empty());

    /**
     * @tc.steps:step4. Close the kv store.
     * @tc.expected: step4. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_LocalDataBatchNotCheckReadOnly_001"), OK);
    g_kvNbDelegatePtr = nullptr;
}
#endif
