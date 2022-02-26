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

#include "distributeddb_storage_single_ver_natural_store_testcase.h"

#include "time_helper.h"
#include "generic_single_ver_kv_entry.h"

using namespace DistributedDB;
using namespace DistributedDBUnitTest;

namespace {
    const int MAX_TEST_KEY_SIZE = 1024;
    const int MAX_TEST_VAL_SIZE = 4194304;

    // select result index for the item for sync database
    const int SYNC_RES_KEY_INDEX = 0;
    const int SYNC_RES_VAL_INDEX = 1;
    const int SYNC_RES_TIME_INDEX = 2;
    const int SYNC_RES_FLAG_INDEX = 3;
    const int SYNC_RES_HASH_KEY_INDEX = 6;

    const std::string SYNC_DATA_DEFAULT_SQL = "select * from SYNC_DATA;";
}

/**
  * @tc.name: GetSyncData001
  * @tc.desc: To test the function of querying the data in the time stamp range in the database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData001(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, C) interface of the NaturalStore, where A<B<C.
     * @tc.expected: step1. GetSyncData The number of output parameter
     *  in the output parameter OK, dataItems is 1.
     */
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    TimeStamp timeBegin;
    store->GetMaxTimeStamp(timeBegin);
    Key key1;
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);

    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);
    TimeStamp timeEnd;
    store->GetMaxTimeStamp(timeEnd);
    EXPECT_GT(timeEnd, timeBegin);

    std::vector<DataItem> vect;
    ContinueToken token = nullptr;
    SyncInputArg inputArg(timeBegin, timeEnd + 1, 1024); // no more than 1024
    EXPECT_EQ(DistributedDBToolsUnitTest::GetSyncDataTest(inputArg, store, vect, token), E_OK);

    EXPECT_EQ(token, nullptr);
    DataItem item = {key1, value1, 0, 0};
    EXPECT_EQ(DistributedDBToolsUnitTest::IsItemValueExist(item, vect), true);
}

/**
  * @tc.name: GetSyncData002
  * @tc.desc: Test the function that the database does not query the data in the time stamp range.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData002(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, B) interface of the NaturalStore,
     *  where A<B<C and interference data processing are added.
     * @tc.expected: step1. GetSyncData The number of output parameters
     *  in the output parameter E_NOT_FOUND,dataItems is 0.
     */
    IOption option;
    option.dataType = IOption::SYNC_DATA;

    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);

    EXPECT_EQ(connection->Put(option, key, value), E_OK);
    TimeStamp timestamp;
    store->GetMaxTimeStamp(timestamp);

    std::vector<DataItem> vect;
    ContinueToken token = nullptr;
    SyncInputArg inputArg(timestamp + 1, timestamp + 1000, 1024); // no more than 1024
    EXPECT_EQ(DistributedDBToolsUnitTest::GetSyncDataTest(inputArg, store, vect, token), E_OK);

    EXPECT_EQ(token, nullptr);
    EXPECT_EQ(vect.size(), 0UL);
}

/**
  * @tc.name: GetSyncData003
  * @tc.desc: To test the function of querying data when the timestamp range
  *  in the data obtaining interface is invalid.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData003(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, B) interface of the NaturalStore, where A>B
     * @tc.expected: step1. The value of GetSyncData is E_INVALID_ARG.
     */
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    TimeStamp timeBegin = 1000; // random
    TimeStamp timeEnd  = 700; // random
    std::vector<DataItem> vect;
    ContinueToken token = nullptr;
    SyncInputArg inputArg1(timeBegin, timeEnd, MAX_TEST_VAL_SIZE);
    EXPECT_EQ(DistributedDBToolsUnitTest::GetSyncDataTest(inputArg1, store, vect, token), -E_INVALID_ARGS);

    timeEnd = timeBegin;
    SyncInputArg inputArg2(timeBegin, timeEnd, MAX_TEST_VAL_SIZE);
    EXPECT_EQ(DistributedDBToolsUnitTest::GetSyncDataTest(inputArg2, store, vect, token), -E_INVALID_ARGS);

    store->GetMaxTimeStamp(timeBegin);
    Key key1;
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);

    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);
    store->GetMaxTimeStamp(timeEnd);

    SyncInputArg inputArg3(timeEnd, timeBegin, MAX_TEST_VAL_SIZE);
    EXPECT_EQ(DistributedDBToolsUnitTest::GetSyncDataTest(inputArg3, store, vect, token), -E_INVALID_ARGS);

    EXPECT_EQ(token, nullptr);
}

/**
  * @tc.name: GetSyncData004
  * @tc.desc: To the test database Subcon reading, a large number of data records exist in the time stamp range.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData004(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    Key key;
    Value value;
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    // The test assumes that there are ten data records
    for (int i = 0; i < 10; i++) {
        DistributedDBToolsUnitTest::GetRandomKeyValue(key, 100 + i); // random size
        DistributedDBToolsUnitTest::GetRandomKeyValue(value, 9900 + i); // random size
        EXPECT_EQ(connection->Put(option, key, value), E_OK);
    }

    TimeStamp timestamp = 0;
    store->GetMaxTimeStamp(timestamp);

    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, B) interface of the NaturalStore.
     * @tc.expected: step1. Return E_GET_UNFINISHED.
     */
    ContinueToken token = nullptr;
    std::vector<DataItem> dataItems;
    SyncInputArg inputArg(0, timestamp + 1, 30 * 1024); // 30k per block
    EXPECT_EQ(DistributedDBToolsUnitTest::GetSyncDataTest(inputArg, store, dataItems, token), -E_UNFINISHED);

    EXPECT_NE(token, nullptr);
    std::size_t countNum = dataItems.size();
    int count = 1;
    do {
        /**
         * @tc.steps:step2. Continue to obtain data through the GetSyncDataNext() interface
         *  of the NaturalStore until the E_GET_FINISHED message is returned.
         * @tc.expected: step2. When the GetSyncDataNext returns E_GET_FINISHED,
         *  the total number of obtained data is the number of inserted data and the data is consistent.
         */
        dataItems.clear();
        int errCode = DistributedDBToolsUnitTest::GetSyncDataNextTest(store, 30 * 1024, dataItems, token); // 30k block

        countNum += dataItems.size();
        count++;
        if (errCode == -E_UNFINISHED) {
            continue;
        } else if (errCode == -E_FINISHED || errCode == E_OK) {
            break;
        } else {
            count = 0;
            break;
        }
    } while (true);
    EXPECT_EQ(token, nullptr);
    EXPECT_EQ(countNum, 10UL); // 10 entries
    EXPECT_EQ(count, 4); // 4 blocks
}

/**
  * @tc.name: GetSyncData005
  * @tc.desc: In the test database, if a large number of data records exist
  *  in the time stamp range, a packet is read successfully.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData005(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    Key key;
    Value value;
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    for (int i = 0; i < 10; i++) { // 10 entries
        DistributedDBToolsUnitTest::GetRandomKeyValue(key, 100 + i); // about 100 byte
        DistributedDBToolsUnitTest::GetRandomKeyValue(value, 9900 + i); // about 9900 byte
        EXPECT_EQ(connection->Put(option, key, value), E_OK);
    }

    TimeStamp timestamp = 0;
    store->GetMaxTimeStamp(timestamp);

    ContinueToken token = nullptr;
    std::vector<DataItem> dataItems;
    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, B) interface of the NaturalStore.
     * @tc.expected: step1. The total size of all data in OK, dataItems is 99K.
     */
    SyncInputArg inputArg(0, timestamp + 1, 100 * 1024); // for 100k
    EXPECT_EQ(DistributedDBToolsUnitTest::GetSyncDataTest(inputArg, store, dataItems, token), E_OK);

    EXPECT_EQ(token, nullptr);
    EXPECT_EQ(dataItems.size(), 10UL);
}

/**
  * @tc.name: GetSyncData006
  * @tc.desc: To test the function of reading data when the time stamp range in the database
  *  is greater than the value of blockSize.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData006(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, MAX_TEST_KEY_SIZE);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, MAX_TEST_VAL_SIZE);

    IOption option;
    option.dataType = IOption::SYNC_DATA;
    EXPECT_EQ(connection->Put(option, key, value), E_OK);
    TimeStamp timestamp = 0;
    store->GetMaxTimeStamp(timestamp);

    ContinueToken token = nullptr;
    std::vector<DataItem> dataItems;

    /**
     * @tc.steps:step1. Use the GetSyncData(A, B) interface of the NaturalStore
     *  and set blockSize to 50 kb to obtain the data within the time stamp range.
     * @tc.expected: step1. The system returns E_GET_FINISHED. The size of the obtained data is 1 kb.
     */
    SyncInputArg inputArg(0, timestamp + 1, 1000); // get size for 1k
    EXPECT_EQ(DistributedDBToolsUnitTest::GetSyncDataTest(inputArg, store, dataItems, token), E_OK);

    EXPECT_EQ(token, nullptr);
    DataItem item = {key, value, 0, 0};
    EXPECT_EQ(DistributedDBToolsUnitTest::IsItemValueExist(item, dataItems), true);
}

/**
  * @tc.name: PutSyncData001
  * @tc.desc: To test the function of synchronizing the new data of the remote device that synchronizes the database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::PutSyncData001(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    TimeStamp timeBegin;
    store->GetMaxTimeStamp(timeBegin);
    Key key1;
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1, 13); // random size
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, 20); // random size

    /**
     * @tc.steps:step1/2. Set Ioption to synchronous data and insert a (key1, value1) data record by put interface.
     */
    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);
    TimeStamp timeEnd;
    store->GetMaxTimeStamp(timeEnd);
    EXPECT_GT(timeEnd, timeBegin);

    DataItem item1;
    std::vector<DataItem> vect;
    item1.key = key1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(item1.value, 18); // random size
    item1.timeStamp = timeBegin;
    item1.writeTimeStamp = item1.timeStamp;
    item1.flag = 0;
    vect.push_back(item1);

    /**
     * @tc.steps:step3. Insert a (key1, value2!=value1, timeStamp, false) data record
     *  through the PutSyncData interface. The value of timeStamp is less than or equal
     *  to the value of timeStamp. For Compare the timestamp to determine whether to synchronization data.
     * @tc.expected: step3. Return OK.
     */
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps:step4. The Ioption is set to synchronize data
     *  through the Get interface to obtain the value data of the key1.
     * @tc.expected: step4. Return OK.The obtained value is value1.
     */
    Value valueRead;
    EXPECT_EQ(connection->Get(option, key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value1), true);

    item1.timeStamp = timeEnd + 1;
    item1.writeTimeStamp = item1.timeStamp;
    vect.clear();
    vect.push_back(item1);

    /**
     * @tc.steps:step5. Insert a (key1, value3!=value1, timeStamp, false) data record
     *  through the PutSyncData interface of the NaturalStore. The value of timeStamp
     *  is greater than that of timeStamp inserted in 2.
     * @tc.expected: step5. Return OK.
     */
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps:step6. The Ioption is set to synchronize data through the Get interface
     *  to obtain the value data of the key1.
     * @tc.expected: step6. Return OK.
     */
    EXPECT_EQ(connection->Get(option, key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(item1.value, valueRead), true);

    DistributedDBToolsUnitTest::GetRandomKeyValue(item1.key, 35); // random size
    DistributedDBToolsUnitTest::GetRandomKeyValue(item1.value, 47); // random size
    vect.clear();
    vect.push_back(item1);

    /**
     * @tc.steps:step7. Insert a (key2, value4) data record through the PutSyncData interface.
     * @tc.expected: step7. Return OK.
     */
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps:step8. The Ioption is set to synchronize data
     *  through the Get interface to obtain the value data of the key2.
     * @tc.expected: step8. Returns OK, and the obtained data is value4.
     */
    EXPECT_EQ(connection->Get(option, item1.key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(item1.value, valueRead), true);
}

/**
  * @tc.name: PutSyncData002
  * @tc.desc: To test the function of synchronizing data from the remote device
  *  to the local device after the data is deleted from the remote device.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::PutSyncData002(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    TimeStamp timeBegin;
    store->GetMaxTimeStamp(timeBegin);
    Key key1;
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1, 37); // random size
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, 19); // random size

    /**
     * @tc.steps:step1/2. Set Ioption to synchronous data and insert a (key1, value1) data record by put interface.
     */
    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);
    TimeStamp timeEnd;
    store->GetMaxTimeStamp(timeEnd);
    EXPECT_GT(timeEnd, timeBegin);

    DataItem item1;
    std::vector<DataItem> vect;
    item1.key = key1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(item1.value, 18); // random size
    item1.timeStamp = timeBegin;
    item1.writeTimeStamp = item1.timeStamp;
    item1.flag = 1;
    DistributedDBToolsUnitTest::CalcHash(key1, item1.key);
    vect.push_back(item1);
    /**
     * @tc.steps:step3. Insert a (key1, value2!=value1, timeStamp, false) data record
     *  through the PutSyncData interface. The value of timeStamp is less than or equal
     *  to the value of timeStamp. For Compare the timestamp to determine whether delete data.
     * @tc.expected: step3. Return OK.
     */
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps:step4. The Ioption is set to synchronize data
     *  through the Get interface to obtain the value data of the key1.
     * @tc.expected: step4. Return OK.The obtained value is value1.
     */
    Value valueRead;
    EXPECT_EQ(connection->Get(option, key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value1), true);

    item1.timeStamp = timeEnd + 1;
    item1.writeTimeStamp = item1.timeStamp;
    vect.clear();
    vect.push_back(item1);

    /**
     * @tc.steps:step5. Insert a (key1, value3!=value1, timeStamp, false) data record
     *  through the PutSyncData interfac. The value of timeStamp
     *  is greater than that of timeStamp inserted in step2.
     * @tc.expected: step5. Return OK.
     */
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps:step6. The Ioption is set to synchronize data through the Get interface
     *  to obtain the value data of the key1.
     * @tc.expected: step6. Return E_NOT_FOUND.
     */
    EXPECT_EQ(connection->Get(option, key1, valueRead), -E_NOT_FOUND);

    // put remote deleted data which not existed locally.
    DistributedDBToolsUnitTest::GetRandomKeyValue(item1.key, 35); // random size
    DistributedDBToolsUnitTest::GetRandomKeyValue(item1.value, 47); // random size
    vect.clear();
    vect.push_back(item1);
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    EXPECT_EQ(connection->Get(option, item1.key, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: PutSyncData003
  * @tc.desc: To test the function of synchronizing the mixed data of the added
  *  and deleted data from the remote device to the local device.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::PutSyncData003(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    TimeStamp timeBegin;
    store->GetMaxTimeStamp(timeBegin);
    DataItem dataItem1;
    DataItem dataItem2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem1.key, 23); // random size
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem2.key, 15); // random size
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem1.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem2.value);
    dataItem1.timeStamp = timeBegin + 1; // ensure bigger timeStamp
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    dataItem2.timeStamp = timeBegin + 2; // ensure bigger timeStamp
    dataItem2.writeTimeStamp = dataItem2.timeStamp;
    dataItem1.flag = dataItem2.flag = 0;

    /**
     * @tc.steps:step1. Insert a data record (key1,value1 is not null) and (key2, value2 is not null)
     *  through the PutSyncData interface.
     * @tc.expected: step1. Return OK.
     */
    std::vector<DataItem> vect = {dataItem1, dataItem2};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps:step2. Set Ioption as the synchronization data to obtain the data of key1 and key2.
     * @tc.expected: step2. The Get interface returns OK. The value of key1 is value1,
     *  and the value of key2 is value2.
     */
    Value valueRead1, valueRead2;
    EXPECT_EQ(connection->Get(option, dataItem1.key, valueRead1), E_OK);
    EXPECT_EQ(connection->Get(option, dataItem2.key, valueRead2), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead1, dataItem1.value), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead2, dataItem2.value), true);

    /**
     * @tc.steps:step3. Insert a (key3, value3) and delete the data of the (key1, value1).
     * @tc.expected: step3. The PutSyncData returns OK.
     */
    DataItem dataItem3 = dataItem1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem3.key, 38); // random size
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem3.value, 27); // random size

    DataItem dataItem4 = dataItem1;
    dataItem4.flag = 1;
    dataItem4.timeStamp += 1;
    dataItem4.writeTimeStamp = dataItem4.timeStamp;
    DistributedDBToolsUnitTest::CalcHash(dataItem1.key, dataItem4.key);
    vect = {dataItem4, dataItem3};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps:step4. Set Ioption to the synchronization data and obtain the data of key1, key2, and key3.
     * @tc.expected: step4. Get key1 returns E_NOT_FOUND,Get key2.
     *  The value of OK,value is value2, the value of Get key3 is OK,
     *  and the value of value is value3.
     */
    valueRead1.clear();
    valueRead2.clear();
    Value valueRead3;
    EXPECT_EQ(connection->Get(option, dataItem1.key, valueRead1), -E_NOT_FOUND);
    EXPECT_EQ(connection->Get(option, dataItem2.key, valueRead2), E_OK);
    EXPECT_EQ(connection->Get(option, dataItem3.key, valueRead3), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead2, dataItem2.value), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead3, dataItem3.value), true);
}

/**
  * @tc.name: PutMetaData001
  * @tc.desc: Test metadata insertion and modification.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::PutMetaData001(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    TestMetaDataPutAndGet(store, connection);
}

/**
  * @tc.name: GetMetaData001
  * @tc.desc: To test the function of reading the metadata of a key in the database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::GetMetaData001(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    /**
     * @tc.steps:step1. Use GetMetaData in NaturalStore to obtain the value of key1.
     *  Check whether the value is the same as the value of value1.
     * @tc.expected: step1. Return OK, and the value is the same as the value of value1.
     */
    TestMetaDataPutAndGet(store, connection);
}

/**
  * @tc.name: GetCurrentMaxTimeStamp001
  * @tc.desc: To test the function of obtaining the maximum timestamp when a record exists in the database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::GetCurrentMaxTimeStamp001(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    Key key1;
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);
    TimeStamp timeBegin = 0;
    TimeStamp timeMiddle = 0;
    TimeStamp timeEnd = 0;

    /**
     * @tc.steps:step1/2. Insert a data record into the synchronization database.
     */
    store->GetMaxTimeStamp(timeBegin);
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);

    /**
     * @tc.steps:step3. The current maximum timestamp is A.
     */
    store->GetMaxTimeStamp(timeMiddle);
    EXPECT_GT(timeMiddle, timeBegin);

    /**
     * @tc.steps:step4. Insert a data record into the synchronization database.
     */
    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);

    /**
     * @tc.steps:step5. Obtain the maximum timestamp B and check whether B>=A exists.
     * @tc.expected: step5. The obtained timestamp is B>=A.
     */
    store->GetMaxTimeStamp(timeEnd);
    EXPECT_GT(timeEnd, timeMiddle);
}

/**
  * @tc.name: GetCurrentMaxTimeStamp002
  * @tc.desc: Obtain the maximum timestamp when no record exists in the test record library.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::GetCurrentMaxTimeStamp002(SQLiteSingleVerNaturalStore *&store)
{
    /**
     * @tc.steps:step1. Obtains the maximum timestamp in the current database record.
     * @tc.expected: step1. Return timestamp is 0.
     */
    TimeStamp timestamp = 10; // non-zero
    store->GetMaxTimeStamp(timestamp);
    EXPECT_EQ(timestamp, 0UL);
}

/**
  * @tc.name: LocalDatabaseOperate001
  * @tc.desc: Test the function of inserting data in the local database of the NaturalStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::LocalDatabaseOperate001(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::LOCAL_DATA;
    DataBaseCommonPutOperate(store, connection, option);
}

/**
  * @tc.name: LocalDatabaseOperate002
  * @tc.desc: Test the function of deleting data from the local database of the NaturalStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::LocalDatabaseOperate002(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::LOCAL_DATA;
    DataBaseCommonDeleteOperate(store, connection, option);
}

/**
  * @tc.name: LocalDatabaseOperate003
  * @tc.desc: To test the function of reading data from the local database of the NaturalStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::LocalDatabaseOperate003(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::LOCAL_DATA;
    DataBaseCommonGetOperate(store, connection, option);
}

/**
  * @tc.name: SyncDatabaseOperate001
  * @tc.desc: To test the function of inserting data of the local device in the synchronization database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate001(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    DataBaseCommonPutOperate(store, connection, option);
}

/**
  * @tc.name: SyncDatabaseOperate002
  * @tc.desc: test the put operation after data synced from other devices.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate002(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    DataItem dataItem1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem1.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem1.value);
    dataItem1.timeStamp = 1001; // random timestamp
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    dataItem1.flag = 0;

    /**
     * @tc.steps: step1/2. Add a remote synchronization data record. (key1, value1).
     */
    std::vector<DataItem> vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps: step3. Ioption is set to synchronous data. Obtains the value data of the key1.
     * @tc.expected: step3. Return OK. The value is the same as the value of value1.
     */
    Value valueRead;
    EXPECT_EQ(connection->Get(option, dataItem1.key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, dataItem1.value), true);

    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, dataItem1.value.size() + 1);

    /**
     * @tc.steps: step4. Ioption Set the data to be synchronized and insert the data of key1,value2.
     * @tc.expected: step4. Return OK.
     */
    EXPECT_EQ(connection->Put(option, dataItem1.key, value2), E_OK);
    EXPECT_EQ(connection->Get(option, dataItem1.key, valueRead), E_OK);

    /**
     * @tc.steps: step3. Ioption is set to synchronous data. Obtains the value data of the key1.
     * @tc.expected: step3. Return OK. The value is the same as the value of value2.
     */
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value2), true);
}

/**
  * @tc.name: SyncDatabaseOperate003
  * @tc.desc: test the delete operation in sync database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate003(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    DataBaseCommonDeleteOperate(store, connection, option);
}

/**
  * @tc.name: SyncDatabaseOperate004
  * @tc.desc: test the delete for the data from other devices in sync database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate004(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    DataItem dataItem1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem1.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem1.value);
    dataItem1.timeStamp = 1997; // random timestamp
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    dataItem1.flag = 0;

    std::vector<DataItem> vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps: step2. The Ioption parameter is set to synchronize data to obtain the value data of the key1.
     * @tc.expected: step2. Return OK. The value is the same as the value of value1.
     */
    Value valueRead;
    EXPECT_EQ(connection->Get(option, dataItem1.key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, dataItem1.value), true);

    Key key2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key2);
    EXPECT_EQ(connection->Delete(option, key2), E_OK);

    /**
     * @tc.steps: step3. The Ioption parameter is set to synchronize data, and the key1 data is deleted.
     * @tc.expected: step3. Return OK.
     */
    EXPECT_EQ(connection->Delete(option, dataItem1.key), E_OK);

    /**
     * @tc.steps: step4. The Ioption parameter is set to synchronize data to obtain the value data of the key1.
     * @tc.expected: step4. Return E_NOT_FOUND.
     */
    EXPECT_EQ(connection->Get(option, dataItem1.key, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: SyncDatabaseOperate005
  * @tc.desc: test the reading for sync database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate005(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    DataBaseCommonGetOperate(store, connection, option);
}

/**
  * @tc.name: SyncDatabaseOperate006
  * @tc.desc: test the get entries for sync database
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate006(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    Key key1, key2, key3;
    Value value1, value2, value3;

    /**
     * @tc.steps: step2/3/4. Set Ioption to synchronous data.
     * Insert the data of key=keyPrefix + 'a', value1.
     * Insert the data of key=keyPrefix + 'c', value2.
     * Insert the data of key length=keyPrefix length - 1, value3.
     * @tc.expected: step2/3/4. Return E_NOT_FOUND.
     */
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1, 30); // random size
    key3 = key2 = key1;
    key2.push_back('C');
    key3.pop_back();
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, 84); // random size
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, 101); // random size
    DistributedDBToolsUnitTest::GetRandomKeyValue(value3, 37); // random size
    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);
    EXPECT_EQ(connection->Put(option, key2, value2), E_OK);
    EXPECT_EQ(connection->Put(option, key3, value3), E_OK);

    /**
     * @tc.steps: step5. Obtain all data whose prefixKey is keyPrefix.
     * @tc.expected: step5. Return OK. The number of obtained data records is 2.
     */
    std::vector<Entry> entriesRead;
    EXPECT_EQ(connection->GetEntries(option, key1, entriesRead), E_OK);
    EXPECT_EQ(entriesRead.size(), 2UL);

    /**
     * @tc.steps: step6. Obtain all data whose prefixKey is empty.
     * @tc.expected: step6. Return OK. The number of obtained data records is 3.
     */
    entriesRead.clear();
    Key emptyKey;
    EXPECT_EQ(connection->GetEntries(option, emptyKey, entriesRead), E_OK);
    EXPECT_EQ(entriesRead.size(), 3UL);

    /**
     * @tc.steps: step7. Obtain all data whose prefixKey is keyPrefix.
     * @tc.expected: step7. Return E_NOT_SUPPORT.
     */
    option.dataType = IOption::LOCAL_DATA;
    EXPECT_EQ(connection->GetEntries(option, emptyKey, entriesRead), -E_NOT_FOUND);
}

/**
  * @tc.name: ClearRemoteData001
  * @tc.desc: test the clear data synced from the remote by device.
  * @tc.type: FUNC
  * @tc.require: AR000CIFDA AR000CQS3T
  * @tc.author: wangbingquan
  */
void DistributedDBStorageSingleVerNaturalStoreTestCase::ClearRemoteData001(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    DataItem dataItem1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem1.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem1.value);
    dataItem1.timeStamp = 1997; // random timestamp
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    dataItem1.flag = 0;

    DataItem dataItem2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem2.key, dataItem1.key.size() + 1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(dataItem2.value);
    dataItem2.timeStamp = 2019; // random timestamp
    dataItem2.writeTimeStamp = dataItem2.timeStamp;
    dataItem2.flag = 0;

    /**
     * @tc.steps: step1. New data is inserted to the B end of the device. [keyB, valueB].
     */
    std::vector<DataItem> vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceA"), E_OK);

    /**
     * @tc.steps: step2. The device pulls the data of the device B, and the device inserts the [keyA, valueA].
     */
    vect.clear();
    vect.push_back(dataItem2);
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps: step3. The device obtains the data of keyA and valueB.
     * @tc.expected: step3. Obtain [keyA, valueA] and [keyB, valueB].
     */
    Value valueRead;
    EXPECT_EQ(connection->Get(option, dataItem1.key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, dataItem1.value), true);
    EXPECT_EQ(connection->Get(option, dataItem2.key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, dataItem2.value), true);

    /**
     * @tc.steps: step4.Invoke the interface for clearing the synchronization data of the B device.
     */
    EXPECT_EQ(store->RemoveDeviceData("deviceA", false), E_OK);

    /**
     * @tc.steps: step5. The device obtains the data of keyA and valueB.
     * @tc.expected: step5. The value of [keyA, valueA] is obtained,
     *  and the value of NOT_FOUND is obtained by querying keyB.
     */
    EXPECT_EQ(connection->Get(option, dataItem1.key, valueRead), -E_NOT_FOUND);
    EXPECT_EQ(connection->Get(option, dataItem2.key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, dataItem2.value), true);

    EXPECT_EQ(store->RemoveDeviceData("deviceB", false), E_OK);
    EXPECT_EQ(connection->Get(option, dataItem2.key, valueRead), -E_NOT_FOUND);
}

/**
 * @tc.name: DeleteUserKeyValue001
 * @tc.desc: When a user deletes a data record, the system clears the user record.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
void DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue001(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &url)
{
    // StoreID::TestGeneralNB
    IOption option;
    option.dataType = IOption::SYNC_DATA;

    // per-set data
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);
    EXPECT_EQ(connection->Put(option, KEY_2, VALUE_2), E_OK);

    /**
     * @tc.steps: step1. delete K1.
     * @tc.expected: step1. delete K1 successfully.
     */
    EXPECT_EQ(connection->Delete(option, KEY_1), E_OK);

    // Close database
    connection->Close();
    connection = nullptr;
    store = nullptr;

    /**
     * @tc.steps: step2. Real query by sqlite3.
     * @tc.expected: step2. Find KEY_1, not find K2.
     */
    std::vector<SyncData> vecSyncData;
    int numSelect = GetRawSyncData(url, SYNC_DATA_DEFAULT_SQL, vecSyncData);

    bool isFound = false;
    EXPECT_EQ(numSelect, 2);
    isFound = IsSqlinteExistKey(vecSyncData, KEY_1);
    EXPECT_EQ(isFound, false);
    isFound = IsSqlinteExistKey(vecSyncData, KEY_2);
    EXPECT_EQ(isFound, true);
}

/**
 * @tc.name: MemoryDbDeleteUserKeyValue001
 * @tc.desc: When a user deletes a data record, the system clears the user record.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
void DistributedDBStorageSingleVerNaturalStoreTestCase::MemoryDbDeleteUserKeyValue001(
    SQLiteSingleVerNaturalStore *&store, SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &url)
{
    // StoreID::TestGeneralNB
    IOption option;
    option.dataType = IOption::SYNC_DATA;

    // per-set data
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);
    EXPECT_EQ(connection->Put(option, KEY_2, VALUE_2), E_OK);

    /**
     * @tc.steps: step1. delete K1.
     * @tc.expected: step1. delete K1 successfully.
     */
    EXPECT_EQ(connection->Delete(option, KEY_1), E_OK);

    /**
     * @tc.steps: step3. Real query by sqlite3.
     * @tc.expected: step3. Find KEY_1, not find K2.
     */
    std::vector<SyncData> vecSyncData;
    int numSelect = GetRawSyncData(url, SYNC_DATA_DEFAULT_SQL, vecSyncData);

    bool isFound = false;
    EXPECT_EQ(numSelect, 2);
    isFound = IsSqlinteExistKey(vecSyncData, KEY_1);
    EXPECT_EQ(isFound, false);
    isFound = IsSqlinteExistKey(vecSyncData, KEY_2);
    EXPECT_EQ(isFound, true);

    // Close database
    connection->Close();
    connection = nullptr;
    store = nullptr;
}

/**
 * @tc.name: DeleteUserKeyValue002
 * @tc.desc: After the synchronization library data is deleted locally, add the same key data locally.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
void DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue002(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &url)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;

    // pre-set data
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);
    EXPECT_EQ(connection->Put(option, KEY_2, VALUE_2), E_OK);

    /**
     * @tc.steps: step1. Delete key1 data via Delete interface.
     * @tc.expected: step1. Delete successfully.
     */
    EXPECT_EQ(connection->Delete(option, KEY_1), E_OK);

    /**
     * @tc.steps: step2. New data from key1, value3 via Put interface.
     * @tc.expected: step2. New data from key1, value3 via Put interface successfully.
     */
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_3), E_OK);

    /**
     * @tc.steps: step3. Query key1 data via Get interface.
     * @tc.expected: step3. Query key1 data via Get interface successfully, get value3 by key1.
     */
    Value valueRead;
    EXPECT_EQ(connection->Get(option, KEY_1, valueRead), E_OK);
    EXPECT_EQ(valueRead, VALUE_3);

    /**
     * @tc.steps: step4. Query key1 real data by sqlite3.
     * @tc.expected: step4. Two records were found.
     */
    std::vector<SyncData> vecSyncData;
    int numSelect = GetRawSyncData(url, SYNC_DATA_DEFAULT_SQL, vecSyncData);

    EXPECT_EQ(numSelect, 2);
}

/**
 * @tc.name: DeleteUserKeyValue003
 * @tc.desc: After the synchronization database data is deleted locally, the same key data is added from the remote end.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
void DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue003(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &url)
{
    IOption option;
    option.dataType = IOption::SYNC_DATA;

    // ready data
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);

    /**
     * @tc.steps: step1. Delete data by key1.
     * @tc.expected: step1. Delete successfully.
     */
    EXPECT_EQ(connection->Delete(option, KEY_1), E_OK);

    /**
     * @tc.steps: step2. Get data by key1.
     * @tc.expected: step1. Key1 not exist in database.
     */
    Value valueRead;
    EXPECT_NE(connection->Get(option, KEY_1, valueRead), E_OK);

    TimeStamp timeStamp = 0;
    store->GetMaxTimeStamp(timeStamp);

    DataItem dataItem1;
    dataItem1.key = KV_ENTRY_1.key;
    dataItem1.value = KV_ENTRY_3.value;
    dataItem1.timeStamp= timeStamp - 100UL; // less than current timeStamp
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    dataItem1.flag = 0;

    DataItem dataItem2;
    dataItem2.key = KV_ENTRY_1.key;
    dataItem2.value = KV_ENTRY_4.value;
    dataItem2.timeStamp = timeStamp + 100UL; // bigger than current timeStamp
    dataItem2.writeTimeStamp = dataItem2.timeStamp;
    dataItem2.flag = 0;
    std::vector<DataItem> vect = {dataItem1};

    /**
     * @tc.steps: step3. Get a new data from remote device B , key1, value3,
     *  with a smaller timestamp than the current timestamp.
     */
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps: step4. Get data by key1.
     * @tc.expected: step4. Key1 not exist in database.
     */
    EXPECT_NE(connection->Get(option, KEY_1, valueRead), E_OK);

    /**
     * @tc.steps: step5. Get a new data from remote device C , key1, value4,
     *  and the timestamp is larger than the current timestamp.
     */
    vect.clear();
    vect.push_back(dataItem2);
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceC"), E_OK);

    /**
     * @tc.steps: step6. Get data by key1.
     * @tc.expected: step6. Key1 not exist in database.
     */
    EXPECT_EQ(connection->Get(option, KEY_1, valueRead), E_OK);

    /**
     * @tc.steps: step7. Get real data by key1.
     * @tc.expected: step7. Get 1 record.
     */
    std::vector<SyncData> vecSyncData;
    int numSelect = GetRawSyncData(url, SYNC_DATA_DEFAULT_SQL, vecSyncData);

    EXPECT_EQ(numSelect, 1);
}

/**
 * @tc.name: DeleteUserKeyValue004
 * @tc.desc: Changes in key after remote delete data syncs to local
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
void DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue004(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &url)
{
    // pre-set data
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);

    DataItem dataItem1;
    dataItem1.key = KV_ENTRY_1.key;
    dataItem1.value = KV_ENTRY_1.value;
    store->GetMaxTimeStamp(dataItem1.timeStamp);
    dataItem1.flag = 1;
    dataItem1.timeStamp += 1;
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    /**
     * @tc.steps: step1 2 3. Synchronize data to another device B; delete key1 data from device B;
     *  pull the action of key1 to local.
     */
    DistributedDBToolsUnitTest::CalcHash(KV_ENTRY_1.key, dataItem1.key);
    std::vector<DataItem> vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps: step4. Close database.
     */
    connection->Close();
    connection = nullptr;
    store = nullptr;

    /**
     * @tc.steps: step5 6. Get real data by key1;and get the number of records.
     * @tc.expected: step5 6. Not exist key1 real data in database;Get 1 record.
     */
    std::vector<SyncData> vecSyncData;
    int numSelect = GetRawSyncData(url, SYNC_DATA_DEFAULT_SQL, vecSyncData);

    EXPECT_EQ(numSelect, 1);
    bool isFound = IsSqlinteExistKey(vecSyncData, KEY_1);
    EXPECT_EQ(isFound, false);
}

/**
 * @tc.name: MemoryDbDeleteUserKeyValue004
 * @tc.desc: Changes in key after remote delete data syncs to local
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
void DistributedDBStorageSingleVerNaturalStoreTestCase::MemoryDbDeleteUserKeyValue004(
    SQLiteSingleVerNaturalStore *&store, SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &url)
{
    // pre-set data
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);

    DataItem dataItem1;
    dataItem1.key = KV_ENTRY_1.key;
    dataItem1.value = KV_ENTRY_1.value;
    store->GetMaxTimeStamp(dataItem1.timeStamp);
    dataItem1.flag = 1;
    dataItem1.timeStamp += 1;
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    /**
     * @tc.steps: step1 2 3. Synchronize data to another device B; delete key1 data from device B;
     *  pull the action of key1 to local.
     */
    DistributedDBToolsUnitTest::CalcHash(KV_ENTRY_1.key, dataItem1.key);
    std::vector<DataItem> vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps: step4 5. Get real data by key1;and get the number of records.
     * @tc.expected: step 4 5. Not exist key1 real data in database;Get 1 record.
     */
    std::vector<SyncData> vecSyncData;
    int numSelect = GetRawSyncData(url, SYNC_DATA_DEFAULT_SQL, vecSyncData);

    EXPECT_EQ(numSelect, 1);
    bool isFound = IsSqlinteExistKey(vecSyncData, KEY_1);
    EXPECT_EQ(isFound, false);

    connection->Close();
    connection = nullptr;
    store = nullptr;
}

/**
 * @tc.name: DeleteUserKeyValue005
 * @tc.desc: New unified key data locally after remote delete data syncs to local
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
void DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue005(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &url)
{
    // pre-set data
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);
    EXPECT_EQ(connection->Put(option, KEY_2, VALUE_2), E_OK);

    DataItem dataItem1;
    dataItem1.key = KV_ENTRY_1.key;
    dataItem1.value = KV_ENTRY_1.value;
    store->GetMaxTimeStamp(dataItem1.timeStamp);
    dataItem1.timeStamp = dataItem1.timeStamp + 10UL;
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    dataItem1.flag = 1;

    /**
     * @tc.steps: step1 2 3. Synchronize data to another device B; delete key1 data from device B;
     *  pull the action of key1 to local.
     */
    DistributedDBToolsUnitTest::CalcHash(KV_ENTRY_1.key, dataItem1.key);
    std::vector<DataItem> vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps: step4. Put K1 V1 to database.
     * @tc.expected: step4. Put successfully.
     */
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);

    /**
     * @tc.steps: step5. Close database.
     */
    connection->Close();
    connection = nullptr;
    store = nullptr;

    /**
     * @tc.steps: step6 7. Get real data by key1;and get the number of records.
     * @tc.expected: step6 7. Not exist key1 real data in database;Get 2 record.
     */
    std::vector<SyncData> vecSyncData;
    int numSelect = GetRawSyncData(url, SYNC_DATA_DEFAULT_SQL, vecSyncData);

    EXPECT_EQ(numSelect, 2);
    bool isFound = IsSqlinteExistKey(vecSyncData, KEY_1);
    EXPECT_EQ(isFound, true);
}

/**
 * @tc.name: MemoryDbDeleteUserKeyValue005
 * @tc.desc: New unified key data locally after remote delete data syncs to local
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
void DistributedDBStorageSingleVerNaturalStoreTestCase::MemoryDbDeleteUserKeyValue005(
    SQLiteSingleVerNaturalStore *&store, SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &url)
{
    // pre-set data
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);
    EXPECT_EQ(connection->Put(option, KEY_2, VALUE_2), E_OK);

    DataItem dataItem1;
    dataItem1.key = KV_ENTRY_1.key;
    dataItem1.value = KV_ENTRY_1.value;
    store->GetMaxTimeStamp(dataItem1.timeStamp);
    dataItem1.timeStamp = TimeHelper::GetSysCurrentTime();
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    dataItem1.flag = 1;

    /**
     * @tc.steps: step1 2 3. Synchronize data to another device B; delete key1 data from device B;
     *  pull the action of key1 to local.
     */
    DistributedDBToolsUnitTest::CalcHash(KV_ENTRY_1.key, dataItem1.key);
    std::vector<DataItem> vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps: step4. Put K1 V1 to database.
     * @tc.expected: step4. Put successfully.
     */
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);

    /**
     * @tc.steps: step5 6. Get real data by key1;and get the number of records.
     * @tc.expected: step5 6. Not exist key1 real data in database;Get 2 record.
     */
    std::vector<SyncData> vecSyncData;
    int numSelect = GetRawSyncData(url, SYNC_DATA_DEFAULT_SQL, vecSyncData);

    EXPECT_EQ(numSelect, 2);
    bool isFound = IsSqlinteExistKey(vecSyncData, KEY_1);
    EXPECT_EQ(isFound, true);

    connection->Close();
    connection = nullptr;
    store = nullptr;
}

/**
 * @tc.name: DeleteUserKeyValue006
 * @tc.desc: After the remote delete data is synced to the local,
 *  the same key data is added from the remote other devices
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
void DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue006(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection, const std::string &url)
{
    // pre-set data
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    EXPECT_EQ(connection->Put(option, KEY_1, VALUE_1), E_OK);

    DataItem dataItem1;
    dataItem1.key = KV_ENTRY_1.key;
    dataItem1.value = KV_ENTRY_1.value;
    store->GetMaxTimeStamp(dataItem1.timeStamp);
    dataItem1.timeStamp = dataItem1.timeStamp + 10UL;
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    dataItem1.flag = 1;

    /**
     * @tc.steps: step1. Remote device B sync deletes data key1 and pushes to local.
     */
    DistributedDBToolsUnitTest::CalcHash(KV_ENTRY_1.key, dataItem1.key);
    std::vector<DataItem> vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceB"), E_OK);

    /**
     * @tc.steps: step2. Get key1 from database.
     * @tc.expected: step2. Not exist key1.
     */
    Value valueRead;
    EXPECT_NE(connection->Get(option, KEY_1, valueRead), E_OK);

    dataItem1.key = KV_ENTRY_1.key;
    dataItem1.flag = 0;
    dataItem1.value = KV_ENTRY_2.value;
    dataItem1.timeStamp = dataItem1.timeStamp - 100UL; // less than current timeStamp
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    /**
     * @tc.steps: step3. Remote device C syncs new data (key1, value2),
     *  timestamp is less than delete timestamp, to local.
     */
    vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceC"), E_OK);

    /**
     * @tc.steps: step4. Get key1 from database.
     * @tc.expected: step4. Not exist key1.
     */
    EXPECT_NE(connection->Get(option, KEY_1, valueRead), E_OK);

    dataItem1.value = KV_ENTRY_3.value;
    dataItem1.timeStamp = dataItem1.timeStamp + 200UL; // bigger than current timeStamp
    dataItem1.writeTimeStamp = dataItem1.timeStamp;
    /**
     * @tc.steps: step5. Remote device C syncs new data (key1, value2),
     *  timestamp is bigger than delete timestamp, to local.
     */
    vect = {dataItem1};
    EXPECT_EQ(DistributedDBToolsUnitTest::PutSyncDataTest(store, vect, "deviceD"), E_OK);

    /**
     * @tc.steps: step6. Get key1 from database.
     * @tc.expected: step6. Exist key1.
     */
    EXPECT_EQ(connection->Get(option, KEY_1, valueRead), E_OK);
    EXPECT_EQ(valueRead, VALUE_3);

    /**
     * @tc.steps: step7. Get real data from database.
     * @tc.expected: step7. Get 1 record.
     */
    std::vector<SyncData> vecSyncData;
    int numSelect = GetRawSyncData(url, SYNC_DATA_DEFAULT_SQL, vecSyncData);

    EXPECT_EQ(numSelect, 1);
}

// private Begin
void DistributedDBStorageSingleVerNaturalStoreTestCase::CreateMemDb(SQLiteSingleVerNaturalStoreConnection *&connection,
    int &errCode)
{
    // pre-Set close other db
    if (connection != nullptr) {
        connection->Close();
        connection = nullptr;
    }

    KvDBProperties property;
    property.SetStringProp(KvDBProperties::STORE_ID, "TestGeneralNB");
    property.SetStringProp(KvDBProperties::IDENTIFIER_DIR, "TestGeneralNB");
    property.SetBoolProp(KvDBProperties::MEMORY_MODE, true);

    SQLiteSingleVerNaturalStore *memoryStore = nullptr;
    memoryStore = new (std::nothrow) SQLiteSingleVerNaturalStore;
    ASSERT_NE(memoryStore, nullptr);

    errCode = memoryStore->Open(property);
    if (errCode != E_OK) {
        return;
    }

    connection = static_cast<SQLiteSingleVerNaturalStoreConnection *>(memoryStore->GetDBConnection(errCode));
    ASSERT_NE(connection, nullptr);
    memoryStore->DecObjRef(memoryStore);
}

// param [in] dbName:Database name,strSql: The sql statement executed,[out],vecSyncData:SYNC_DATA table data
// Real query sync-DATA table data via sqlite. return query data row number
int DistributedDBStorageSingleVerNaturalStoreTestCase::GetRawSyncData(const std::string &dbName,
    const std::string &strSql, std::vector<SyncData> &vecSyncData)
{
    uint64_t flag = SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE;
    flag |= SQLITE_OPEN_CREATE;

    sqlite3* db = nullptr;
    int nResult = sqlite3_open_v2(dbName.c_str(), &db, flag, nullptr);
    if (nResult != SQLITE_OK) {
        return -nResult;
    }

    sqlite3_stmt *statement = nullptr;

    nResult = sqlite3_prepare(db, strSql.c_str(), -1, &statement, NULL);
    if (nResult != SQLITE_OK) {
        (void)sqlite3_close_v2(db);
        return -1;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        SyncData stuSyncData;
        const uint8_t *blobValue = static_cast<const uint8_t *>(sqlite3_column_blob(statement, SYNC_RES_KEY_INDEX));
        int valueLength = sqlite3_column_bytes(statement, SYNC_RES_KEY_INDEX);
        if (blobValue == nullptr) {
            stuSyncData.key.clear();
        } else {
            stuSyncData.key.resize(valueLength);
            stuSyncData.key.assign(blobValue, blobValue + valueLength);
        }

        blobValue = static_cast<const uint8_t *>(sqlite3_column_blob(statement, SYNC_RES_HASH_KEY_INDEX));
        valueLength = sqlite3_column_bytes(statement, SYNC_RES_HASH_KEY_INDEX);
        stuSyncData.hashKey.resize(valueLength);
        stuSyncData.hashKey.assign(blobValue, blobValue + valueLength);

        blobValue = static_cast<const uint8_t *>(sqlite3_column_blob(statement, SYNC_RES_VAL_INDEX));
        valueLength = sqlite3_column_bytes(statement, SYNC_RES_VAL_INDEX);
        if (blobValue == nullptr) {
            stuSyncData.value.clear();
        } else {
            stuSyncData.value.resize(valueLength);
            stuSyncData.value.assign(blobValue, blobValue + valueLength);
        }

        stuSyncData.timeStamp = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_TIME_INDEX));
        stuSyncData.flag = sqlite3_column_int64(statement, SYNC_RES_FLAG_INDEX);
        vecSyncData.push_back(stuSyncData);
    }

    sqlite3_finalize(statement);
    statement = nullptr;
    (void)sqlite3_close_v2(db);
    return static_cast<int>(vecSyncData.size());
}

// @Real query sync-DATA table by key, judge is exist.
bool DistributedDBStorageSingleVerNaturalStoreTestCase::IsSqlinteExistKey(const std::vector<SyncData> &vecSyncData,
    const std::vector<uint8_t> &key)
{
    for (const auto &iter : vecSyncData) {
        if (key == iter.key) {
            return true;
        }
    }
    return false;
}

void DistributedDBStorageSingleVerNaturalStoreTestCase::TestMetaDataPutAndGet(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection)
{
    Key key1;
    Value value1;
    Key emptyKey;
    Value emptyValue;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);

    /**
     * @tc.steps:step1. Run the PutMetaData command to insert a non-empty key1 non-empty value1 data record.
     * @tc.expected: step1. Return OK.
     */
    EXPECT_EQ(store->PutMetaData(key1, value1), E_OK);

    /**
     * @tc.steps:step2. Run the PutMetaData command to insert a non-empty key1 non-empty value1 data record.
     * @tc.expected: step2. The obtained value is the same as the value of value1.
     */
    Value valueRead;
    EXPECT_EQ(store->GetMetaData(key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value1), true);
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, static_cast<int>(value1.size() + 3)); // random size

    /**
     * @tc.steps:step3. The key value is key1, the value is not empty,
     *  and the value of value2 is different from the value of value1 through the PutMetaData interface.
     * @tc.expected: step3. Return OK.
     */
    EXPECT_EQ(store->PutMetaData(key1, value2), E_OK);

    /**
     * @tc.steps:step4. Run the GetMetaData command to obtain the value of key1
     *  and check whether the value is the same as the value of value2.
     * @tc.expected: step4. The obtained value is the same as the value of value2.
     */
    EXPECT_EQ(store->GetMetaData(key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value2), true);

    /**
     * @tc.steps:step5. Use PutMetaData to insert a record whose key is empty and value is not empty.
     * @tc.expected: step5. Return E_INVALID_ARGS.
     */
    EXPECT_EQ(store->PutMetaData(emptyKey, value1), -E_INVALID_ARGS);

    /**
     * @tc.steps:step6. Use PutMetaData in NaturalStore to insert data whose key2(!=key1)
     *  is not empty and value is empty.
     * @tc.expected: step6. Return OK.
     */
    Key key2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key2, static_cast<int>(key1.size() + 1));
    EXPECT_EQ(store->PutMetaData(key2, emptyValue), E_OK);

    /**
     * @tc.steps:step7. Obtain the value of key2 and check whether the value is empty.
     * @tc.expected: step7. The obtained value is empty.
     */
    EXPECT_EQ(store->GetMetaData(key2, valueRead), E_OK);
    EXPECT_EQ(valueRead.empty(), true);

    /**
     * @tc.steps:step8. Insert the data whose key size is 1024 and value size is 4Mb
     *  through PutMetaData of NaturalStore.
     * @tc.expected: step8. Return OK.
     */
    Key sizeKey;
    Value sizeValue;
    DistributedDBToolsUnitTest::GetRandomKeyValue(sizeKey, MAX_TEST_KEY_SIZE);
    DistributedDBToolsUnitTest::GetRandomKeyValue(sizeValue, MAX_TEST_VAL_SIZE);
    EXPECT_EQ(store->PutMetaData(sizeKey, sizeValue), E_OK);
    EXPECT_EQ(store->GetMetaData(sizeKey, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, sizeValue), true);

    /**
     * @tc.steps:step9/10. Insert data items whose key size is greater than 1 kb
     *  or value size greater than 4Mb through PutMetaData of NaturalStore.
     * @tc.expected: step9/10. Return E_INVALID_ARGS.
     */
    sizeKey.push_back(249); // random
    EXPECT_EQ(store->PutMetaData(sizeKey, sizeValue), -E_INVALID_ARGS);
    sizeKey.pop_back();
    sizeValue.push_back(174); // random
    EXPECT_EQ(store->PutMetaData(sizeKey, sizeValue), -E_INVALID_ARGS);
}

void DistributedDBStorageSingleVerNaturalStoreTestCase::DataBaseCommonPutOperate(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection, IOption option)
{
    Key key1;
    Value value1;

    /**
     * @tc.steps: step1/2. Set Ioption to the local data and insert a record of key1 and value1.
     * @tc.expected: step1/2. Return OK.
     */
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);
    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);

    /**
     * @tc.steps: step3. Set Ioption to the local data and obtain the value of key1.
     *  Check whether the value is the same as the value of value1.
     * @tc.expected: step3. The obtained value and value2 are the same.
     */
    Value valueRead;
    EXPECT_EQ(connection->Get(option, key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value1), true);
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, static_cast<int>(value1.size() + 3)); // 3 more for diff

    /**
     * @tc.steps: step4. Ioption Set this parameter to the local data. Insert key1.
     *  The value cannot be empty. value2(!=value1)
     * @tc.expected: step4. Return OK.
     */
    EXPECT_EQ(connection->Put(option, key1, value2), E_OK);

    /**
     * @tc.steps: step5. Set Ioption to the local data, GetMetaData to obtain the value of key1,
     *  and check whether the value is the same as the value of value2.
     * @tc.expected: step5. The obtained value and value2 are the same.
     */
    EXPECT_EQ(connection->Get(option, key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value2), true);

    /**
     * @tc.steps: step6. The Ioption parameter is set to the local data.
     *  The data record whose key is empty and value is not empty is inserted.
     * @tc.expected: step6. Return E_INVALID_DATA.
     */
    Key emptyKey;
    Value emptyValue;
    EXPECT_EQ(connection->Put(option, emptyKey, value1), -E_INVALID_ARGS);

    /**
     * @tc.steps: step7. Set Ioption to the local data, insert data
     *  whose key2(!=key1) is not empty, and value is empty.
     * @tc.expected: step7. Return OK.
     */
    Key key2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key2, static_cast<int>(key1.size() + 1));
    EXPECT_EQ(connection->Put(option, key2, emptyValue), E_OK);

    /**
     * @tc.steps: step8. Set option to local data, obtain the value of key2,
     *  and check whether the value is empty.
     * @tc.expected: step8. Return OK, value is empty.
     */
    EXPECT_EQ(connection->Get(option, key2, valueRead), E_OK);
    EXPECT_EQ(valueRead.empty(), true);

    /**
     * @tc.steps: step9. Ioption Set the local data.
     *  Insert the data whose key size is 1024 and value size is 4Mb.
     * @tc.expected: step9. Return OK.
     */
    Key sizeKey;
    Value sizeValue;
    DistributedDBToolsUnitTest::GetRandomKeyValue(sizeKey, MAX_TEST_KEY_SIZE);
    DistributedDBToolsUnitTest::GetRandomKeyValue(sizeValue, MAX_TEST_VAL_SIZE);
    EXPECT_EQ(connection->Put(option, sizeKey, sizeValue), E_OK);
    EXPECT_EQ(connection->Get(option, sizeKey, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, sizeValue), true);

    /**
     * @tc.steps: step10/11. Set Ioption to the local data and insert data items
     *  whose value is greater than 4Mb or key is bigger than 1Kb
     * @tc.expected: step10/11. Return E_INVALID_ARGS.
     */
    sizeKey.push_back(std::rand()); // random size
    EXPECT_EQ(connection->Put(option, sizeKey, sizeValue), -E_INVALID_ARGS);
    sizeKey.pop_back();
    sizeValue.push_back(174); // random size
    EXPECT_EQ(connection->Put(option, sizeKey, sizeValue), -E_INVALID_ARGS);
}

void DistributedDBStorageSingleVerNaturalStoreTestCase::DataBaseCommonDeleteOperate(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection, IOption option)
{
    /**
     * @tc.steps: step2. Set Ioption to the local data and delete the data whose key is key1 (empty).
     * @tc.expected: step2. Return E_INVALID_ARGS.
     */
    Key key1;
    EXPECT_EQ(connection->Delete(option, key1), -E_INVALID_ARGS);
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1, MAX_TEST_KEY_SIZE + 1);
    EXPECT_EQ(connection->Delete(option, key1), -E_INVALID_ARGS);
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1);
    EXPECT_EQ(connection->Delete(option, key1), E_OK);

    /**
     * @tc.steps: step3. Set Ioption to the local data, insert non-null key1, and non-null value1 data.
     * @tc.expected: step3. Return E_OK.
     */
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);
    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);

    /**
     * @tc.steps: step4. Set Ioption to the local data, obtain the value of key1,
     *  and check whether the value is the same as that of value1.
     * @tc.expected: step4. Return E_OK. The obtained value is the same as the value of value1.
     */
    Value valueRead;
    EXPECT_EQ(connection->Get(option, key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value1), true);

    /**
     * @tc.steps: step5. Set Ioption to the local data and delete the data whose key is key1.
     * @tc.expected: step5. Return E_OK.
     */
    EXPECT_EQ(connection->Delete(option, key1), E_OK);

    /**
     * @tc.steps: step5. Set Ioption to the local data and obtain the value of Key1.
     * @tc.expected: step5. Return E_NOT_FOUND.
     */
    EXPECT_EQ(connection->Get(option, key1, valueRead), -E_NOT_FOUND);
}

void DistributedDBStorageSingleVerNaturalStoreTestCase::DataBaseCommonGetOperate(SQLiteSingleVerNaturalStore *&store,
    SQLiteSingleVerNaturalStoreConnection *&connection, IOption option)
{
    /**
     * @tc.steps: step2. Set Ioption to the local data and delete the data whose key is key1 (empty).
     * @tc.expected: step2. Return E_INVALID_ARGS.
     */
    Key key1;
    Value valueRead;
    // empty key
    EXPECT_EQ(connection->Get(option, key1, valueRead), -E_INVALID_ARGS);

    // invalid key
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1, MAX_TEST_KEY_SIZE + 1);
    EXPECT_EQ(connection->Get(option, key1, valueRead), -E_INVALID_ARGS);

    // non-exist key
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1, MAX_TEST_KEY_SIZE);
    EXPECT_EQ(connection->Get(option, key1, valueRead), -E_NOT_FOUND);

    /**
     * @tc.steps: step3. Set Ioption to the local data, insert non-null key1, and non-null value1 data.
     * @tc.expected: step3. Return E_OK.
     */
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);
    EXPECT_EQ(connection->Put(option, key1, value1), E_OK);

    /**
     * @tc.steps: step4. Set Ioption to the local data, obtain the value of key1,
     *  and check whether the value is the same as that of value1.
     * @tc.expected: step4. Return E_OK. The obtained value is the same as the value of value1.
     */
    EXPECT_EQ(connection->Get(option, key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value1), true);

    Key key2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key2);
    EXPECT_EQ(connection->Get(option, key2, valueRead), -E_NOT_FOUND);

    /**
     * @tc.steps: step5. Set Ioption to the local data and obtain the value data of Key1.
     *  Check whether the value is the same as the value of value2.
     * @tc.expected: step4. Return E_OK, and the value is the same as the value of value2.
     */
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, value1.size() + 1);
    EXPECT_EQ(connection->Put(option, key1, value2), E_OK);

    /**
     * @tc.steps: step5. The Ioption is set to the local.
     *  The data of the key1 and value2(!=value1) is inserted.
     * @tc.expected: step4. Return E_OK.
     */
    EXPECT_EQ(connection->Get(option, key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value2), true);
}

