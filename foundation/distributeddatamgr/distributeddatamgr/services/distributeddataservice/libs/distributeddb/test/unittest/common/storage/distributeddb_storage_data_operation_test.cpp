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

#include "db_common.h"
#include "db_constant.h"
#include "distributeddb_tools_unit_test.h"
#include "kvdb_manager.h"
#include "sqlite_local_kvdb_connection.h"
#include "distributeddb_data_generate_unit_test.h"
#include "multi_ver_natural_store_transfer_data.h"
#include "multi_ver_natural_store_connection.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    SQLiteLocalKvDBConnection *g_connection = nullptr;
}

class DistributedDBStorageDataOperationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageDataOperationTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
}

void DistributedDBStorageDataOperationTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBStorageDataOperationTest::SetUp(void)
{
    KvDBProperties properties;
    properties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    properties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    properties.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    properties.SetStringProp(KvDBProperties::STORE_ID, "test");
    properties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, "test");

    int errCode = E_OK;
    g_connection = static_cast<SQLiteLocalKvDBConnection *>(KvDBManager::GetDatabaseConnection(properties, errCode));
    EXPECT_EQ(errCode, E_OK);
}

void DistributedDBStorageDataOperationTest::TearDown(void)
{
    if (g_connection != nullptr) {
        g_connection->Close();
        g_connection = nullptr;
    }
    return;
}

/**
  * @tc.name: Insert001
  * @tc.desc: Insert a record into a distributed db
  * @tc.type: FUNC
  * @tc.require: AR000CQDV8 AR000CQDVB
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageDataOperationTest, Insert001, TestSize.Level1)
{
    EXPECT_NE(g_connection, nullptr);
    if (g_connection == nullptr) {
        return;
    }

    Key key(3, 'w');
    Value value;
    value.assign(8, 87);
    IOption option;

    /**
     * @tc.steps:step1. Put a kv into database
     * @tc.expected: step1. Return OK.
     */
    int errCode = g_connection->Put(option, key, value);
    EXPECT_EQ(errCode, E_OK);

    Value valueRead;
    valueRead.clear();

    /**
     * @tc.steps:step2. Get k from database
     * @tc.expected: step2. Return OK. The size is right.
     */
    errCode = g_connection->Get(option, key, valueRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(valueRead.size(), 8UL);

    for (auto iter = valueRead.begin(); iter != valueRead.end(); iter++) {
        EXPECT_EQ(*iter, 87);
    }
}

/**
  * @tc.name: InsertBatch001
  * @tc.desc: Insert some records into a distributed db
  * @tc.type: FUNC
  * @tc.require: AR000CQDV9 AR000CQDVE
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageDataOperationTest, InsertBatch001, TestSize.Level1)
{
    EXPECT_NE(g_connection, nullptr);
    if (g_connection == nullptr) {
        return;
    }

    Key key(3, 'w');
    Value value;
    value.assign(8, 87);
    IOption option;

    Entry entry;
    entry.key = key;
    entry.value = value;

    std::vector<Entry> entries;
    entries.push_back(entry);

    entry.key.push_back('q');
    entry.value.assign(6, 76);
    entries.push_back(entry);

    /**
     * @tc.steps:step1. PutBatch series kv into database
     * @tc.expected: step1. Return OK.
     */
    int errCode = g_connection->PutBatch(option, entries);
    EXPECT_EQ(errCode, E_OK);

    std::vector<Entry> entriesRead;
    Key keyRead(3, 'w');
    entriesRead.clear();

    /**
     * @tc.steps:step2. Get k from database by GetEntries
     * @tc.expected: step2. Return OK. The size is right.
     */
    errCode = g_connection->GetEntries(option, keyRead, entriesRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(entriesRead.size(), 2UL);

    if (entriesRead.size() > 2) {
        EXPECT_EQ(entriesRead[0].value.size(), 8UL);
        EXPECT_EQ(entriesRead[1].value.size(), 6UL);
    }
}

/**
  * @tc.name: Clear001
  * @tc.desc: Clear some records from a distributed db
  * @tc.type: FUNC
  * @tc.require: AR000BVTO6 AR000CQDVA
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageDataOperationTest, Clear001, TestSize.Level1)
{
    EXPECT_NE(g_connection, nullptr);
    if (g_connection == nullptr) {
        return;
    }

    Key key(3, 'w');
    Value value;
    value.assign(8, 87);
    IOption option;

    Entry entry;
    entry.key = key;
    entry.value = value;

    std::vector<Entry> entries;
    entries.push_back(entry);

    entry.key.push_back('q');
    entry.value.assign(6, 76);
    entries.push_back(entry);

    /**
     * @tc.steps:step1. PutBatch series kv into database
     * @tc.expected: step1. Return OK.
     */
    int errCode = g_connection->PutBatch(option, entries);
    EXPECT_EQ(errCode, E_OK);

    std::vector<Entry> entriesRead;
    Key keyRead(3, 'w');
    entriesRead.clear();

    /**
     * @tc.steps:step2. Get k from database by GetEntries
     * @tc.expected: step2. Return OK. The size is right.
     */
    errCode = g_connection->GetEntries(option, keyRead, entriesRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(entriesRead.size(), 2UL);

    if (entriesRead.size() > 2) {
        EXPECT_EQ(entriesRead[0].value.size(), 8UL);
        EXPECT_EQ(entriesRead[1].value.size(), 6UL);
    }

    /**
     * @tc.steps:step3. Clear all data from database
     * @tc.expected: step3. Return OK.
     */
    errCode = g_connection->Clear(option);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps:step2. Get k from database by GetEntries
     * @tc.expected: step2. Return E_NOT_FOUND. The result size is 0.
     */
    entriesRead.clear();
    errCode = g_connection->GetEntries(option, keyRead, entriesRead);
    EXPECT_EQ(errCode, -E_NOT_FOUND);
    EXPECT_EQ(entriesRead.size(), 0UL);
}

/**
  * @tc.name: Delete001
  * @tc.desc: Delete a record from a distributed db
  * @tc.type: FUNC
  * @tc.require: AR000CQDVF AR000CQDVB
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageDataOperationTest, Delete001, TestSize.Level1)
{
    EXPECT_NE(g_connection, nullptr);
    if (g_connection == nullptr) {
        return;
    }

    Key key(3, 'w');
    Value value;
    value.assign(8, 87);
    IOption option;

    Entry entry;
    entry.key = key;
    entry.value = value;

    std::vector<Entry> entries;
    entries.push_back(entry);

    entry.key.push_back('q');
    entry.value.assign(6, 76);
    entries.push_back(entry);

    /**
     * @tc.steps:step1. PutBatch series kv into database
     * @tc.expected: step1. Return OK.
     */
    int errCode = g_connection->PutBatch(option, entries);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps:step2. Get k from database by GetEntries
     * @tc.expected: step2. Return OK. The size is right.
     */
    Value valueRead;
    errCode = g_connection->Get(option, entry.key, valueRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(valueRead.size(), 6UL);

    std::vector<Entry> entriesRead;
    Key keyRead(3, 'w');
    entriesRead.clear();

    /**
     * @tc.steps:step3. Get k from database by GetEntries
     * @tc.expected: step3. Return E_OK. The result size is right.
     */
    errCode = g_connection->GetEntries(option, keyRead, entriesRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(entriesRead.size(), 2UL);

    if (entriesRead.size() > 2) {
        EXPECT_EQ(entriesRead[0].value.size(), 8UL);
        EXPECT_EQ(entriesRead[1].value.size(), 6UL);
    }

    /**
     * @tc.steps:step3. Delete k from database
     * @tc.expected: step3. Return E_OK.
     */
    errCode = g_connection->Delete(option, key);
    EXPECT_EQ(errCode, E_OK);

    entriesRead.clear();

    /**
     * @tc.steps:step3. Get k from database by GetEntries
     * @tc.expected: step3. Return E_OK. The result size is reduction 1.
     */
    errCode = g_connection->GetEntries(option, keyRead, entriesRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(entriesRead.size(), 1UL);
}

/**
  * @tc.name: DeleteBatch001
  * @tc.desc: Delete some records from a distributed db
  * @tc.type: FUNC
  * @tc.require: AR000CQDVG AR000CQDVB
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBStorageDataOperationTest, DeleteBatch001, TestSize.Level1)
{
    EXPECT_NE(g_connection, nullptr);
    if (g_connection == nullptr) {
        return;
    }

    Key key(3, 'w');
    Value value;
    value.assign(8, 87);
    IOption option;

    Entry entry;
    entry.key = key;
    entry.value = value;

    std::vector<Entry> entries;
    entries.push_back(entry);

    entry.key.push_back('q');
    entry.value.assign(6, 76);
    entries.push_back(entry);

    /**
     * @tc.steps:step1. PutBatch series kv into database
     * @tc.expected: step1. Return OK.
     */
    int errCode = g_connection->PutBatch(option, entries);
    EXPECT_EQ(errCode, E_OK);

    std::vector<Entry> entriesRead;
    Key keyRead(3, 'w');
    entriesRead.clear();

    /**
     * @tc.steps:step2. Get k from database by GetEntries
     * @tc.expected: step2. Return E_OK. The result size is right.
     */
    errCode = g_connection->GetEntries(option, keyRead, entriesRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(entriesRead.size(), 2UL);

    if (entriesRead.size() > 2) {
        EXPECT_EQ(entriesRead[0].value.size(), 8UL);
        EXPECT_EQ(entriesRead[1].value.size(), 6UL);
    }

    std::vector<Key> keys;
    Key keyTmp = key;

    keys.push_back(keyTmp);
    keyTmp.push_back('q');
    keys.push_back(keyTmp);

    /**
     * @tc.steps:step3. DeleteBatch keys from database by DeleteBatch
     * @tc.expected: step3. Return E_OK.
     */
    errCode = g_connection->DeleteBatch(option, keys);
    EXPECT_EQ(errCode, E_OK);

    entriesRead.clear();

    /**
     * @tc.steps:step3. Get k from database by GetEntries
     * @tc.expected: step3. Return E_OK. The result size is 0.
     */
    errCode = g_connection->GetEntries(option, keyRead, entriesRead);
    EXPECT_EQ(errCode, -E_NOT_FOUND);
    EXPECT_EQ(entriesRead.size(), 0UL);
}

static void CheckSplitData(const Value &oriValue, const uint32_t numBlock,
    std::map<ValueSliceHash, Value> &valueDic, Value &savedValue)
{
    MultiVerValueObject valueObject;
    MultiVerNaturalStoreTransferData transferData;
    std::vector<Value> partValues;
    int errCode = transferData.SegmentAndTransferValueToHash(oriValue, partValues);
    // Default threshold
    if (oriValue.size() <= DistributedDB::DBConstant::MAX_VALUE_SIZE) {
        valueObject.SetFlag(0);
        valueObject.SetValue(oriValue);
        valueObject.GetSerialData(savedValue);
        EXPECT_EQ(errCode, -E_UNEXPECTED_DATA);
        EXPECT_EQ(partValues.size(), numBlock);
        return;
    }
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(partValues.size(), numBlock);

    valueObject.SetFlag(MultiVerValueObject::HASH_FLAG);
    std::vector<ValueSliceHash> hashValues;
    ValueSliceHash hashValue;
    for (const auto &value : partValues) {
        errCode = DBCommon::CalcValueHash(value, hashValue);
        EXPECT_EQ(errCode, E_OK);

        // prepare for recover
        valueDic[hashValue] = value;
        hashValues.push_back(std::move(hashValue));
    }

    valueObject.SetValueHash(hashValues);
    valueObject.GetSerialData(savedValue);

    return;
}

static void CheckRecoverData(const Value &savedValue, std::map<ValueSliceHash, Value> &valueDic,
    Value &checkValue)
{
    Value value;
    MultiVerValueObject valueObject;
    EXPECT_EQ(valueObject.DeSerialData(savedValue), E_OK);
    if (!valueObject.IsHash()) {
        EXPECT_EQ(valueObject.GetValue(value), E_OK);
    }

    std::vector<ValueSliceHash> sliceHashVect;
    EXPECT_EQ(valueObject.GetValueHash(sliceHashVect), E_OK);

    value.reserve(valueObject.GetDataLength());
    for (const auto &item : sliceHashVect) {
        Value itemValue = valueDic[item];
        value.insert(value.end(), itemValue.begin(), itemValue.end());
    }

    EXPECT_EQ(value, checkValue);
    return;
}

/**
  * @tc.name: BlockDataIndex001
  * @tc.desc: Determine the block threshold of the database.
  * @tc.type: FUNC
  * @tc.require: AR000CQDTT SR000CQDTR
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBStorageDataOperationTest, BlockDataIndex001, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2/3. Put 100B 1K 100k size of unique value into database
     */
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, 100); // 100B
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, 1024); // 1K
    Value value3;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value3, 1024 * 100); // 100K

    IOption option;
    option.dataType = IOption::SYNC_DATA;
    int errCode = g_connection->Put(option, KEY_1, value1);
    EXPECT_EQ(errCode, E_OK);
    errCode = g_connection->Put(option, KEY_2, value2);
    EXPECT_EQ(errCode, E_OK);
    errCode = g_connection->Put(option, KEY_3, value3);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps:step4. Check split status
     * @tc.expected: step4. Value1 not cut, value2 cut into 1 block, value3 cut into 2 blocks.
     */
    std::map<ValueSliceHash, Value> valueDic;
    Value savedValue1;
    CheckSplitData(value1, 0ul, valueDic, savedValue1);
    Value savedValue2;
    CheckSplitData(value2, 0ul, valueDic, savedValue2);
    Value savedValue3;
    CheckSplitData(value3, 0ul, valueDic, savedValue3);

    /**
     * @tc.steps:step5. Get the original before key
     * @tc.expected: step5. Return the right original value.
     */
    Value valueRead;
    valueRead.clear();
    errCode = g_connection->Get(option, KEY_1, valueRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(value1, valueRead);
    valueRead.clear();
    errCode = g_connection->Get(option, KEY_2, valueRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(value2, valueRead);
    valueRead.clear();
    errCode = g_connection->Get(option, KEY_3, valueRead);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(value3, valueRead);
}

/**
  * @tc.name: CutValueIntoBlock001
  * @tc.desc: Database block size test
  * @tc.type: FUNC
  * @tc.require: AR000CQDTS AR000CQDTU
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBStorageDataOperationTest, CutValueIntoBlock001, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2/3/4. Put 100B 1K 100k 64k size of unique value into database
     */
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, 100); // 100B
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, 1024); // 1K
    Value value3;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value3, 1024 * 100); // 100k
    Value value4;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value4, 1024 * 64); // 64K

    /**
     * @tc.steps:step4. Split and check repeat value block.
     * @tc.expected: step4. No repeat block.
     */
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    int errCode = g_connection->Put(option, KEY_1, value1);
    EXPECT_EQ(errCode, E_OK);
    errCode = g_connection->Put(option, KEY_2, value2);
    EXPECT_EQ(errCode, E_OK);
    errCode = g_connection->Put(option, KEY_3, value3);
    EXPECT_EQ(errCode, E_OK);
    errCode = g_connection->Put(option, KEY_4, value4);
    EXPECT_EQ(errCode, E_OK);

    std::map<ValueSliceHash, Value> valueDic;
    Value savedValue;
    CheckSplitData(value1, 0ul, valueDic, savedValue);
    CheckRecoverData(savedValue, valueDic, value1);

    valueDic.clear();
    savedValue.clear();
    CheckSplitData(value2, 0ul, valueDic, savedValue);
    CheckRecoverData(savedValue, valueDic, value2);

    valueDic.clear();
    savedValue.clear();
    CheckSplitData(value3, 0ul, valueDic, savedValue);
    CheckRecoverData(savedValue, valueDic, value3);
    EXPECT_EQ(valueDic.size(), 0ul);

    valueDic.clear();
    savedValue.clear();
    CheckSplitData(value4, 0ul, valueDic, savedValue);
    CheckRecoverData(savedValue, valueDic, value4);
}

/**
  * @tc.name: CutValueIntoBlock002
  * @tc.desc: Block data index
  * @tc.type: FUNC
  * @tc.require: AR000CQDTT AR000CQDTV
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBStorageDataOperationTest, CutValueIntoBlock002, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2/3. Put 64k 100k 200k size of value into database(some blocks are repeated).
     */
    Value valueTemp;
    DistributedDBToolsUnitTest::GetRandomKeyValue(valueTemp, 1024 * 36); // 36K add 64K equal 100K

    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, 1024 * 64); // 64K
    Value value2;
    value2.insert(value2.end(), value1.begin(), value1.end());
    value2.insert(value2.end(), valueTemp.begin(), valueTemp.end());

    Value value3;
    // repeat twice value1 in front of value3
    for (int i = 0; i < 2; i++) {
        value3.insert(value3.end(), value1.begin(), value1.end());
    }
    value3.insert(value3.end(), valueTemp.begin(), valueTemp.end());
    value3.insert(value3.end(), valueTemp.begin(), valueTemp.end());

    IOption option;
    option.dataType = IOption::SYNC_DATA;
    int errCode = g_connection->Put(option, KEY_1, value1);
    EXPECT_EQ(errCode, E_OK);
    errCode = g_connection->Put(option, KEY_2, value2);
    EXPECT_EQ(errCode, E_OK);
    errCode = g_connection->Put(option, KEY_3, value3);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps:step4. Split and check repeat value block.
     * @tc.expected: step4. Duplicate blocks are eliminated
     */
    std::map<ValueSliceHash, Value> valueDic;
    Value savedValue;
    CheckSplitData(value3, 0ul, valueDic, savedValue);
    CheckRecoverData(savedValue, valueDic, value3);
    EXPECT_EQ(valueDic.size(), 0ul);

    savedValue.clear();
    CheckSplitData(value1, 0ul, valueDic, savedValue);
    CheckRecoverData(savedValue, valueDic, value1);
    EXPECT_EQ(valueDic.size(), 0ul);

    savedValue.clear();
    CheckSplitData(value2, 0ul, valueDic, savedValue);
    CheckRecoverData(savedValue, valueDic, value2);
    EXPECT_EQ(valueDic.size(), 0ul);
}