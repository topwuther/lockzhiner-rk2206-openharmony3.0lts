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

#include "db_constant.h"
#include "db_common.h"
#include "distributeddb_storage_single_ver_natural_store_testcase.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    DistributedDB::KvStoreConfig g_config;

    std::string g_testDir;
    std::string g_databaseName;
    std::string g_identifier;

    DistributedDB::SQLiteSingleVerNaturalStore *g_store = nullptr;
    DistributedDB::SQLiteSingleVerNaturalStoreConnection *g_connection = nullptr;
}

class DistributedDBStorageSQLiteSingleVerNaturalStoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageSQLiteSingleVerNaturalStoreTest::SetUpTestCase(void)
{}

void DistributedDBStorageSQLiteSingleVerNaturalStoreTest::TearDownTestCase(void) {}

void DistributedDBStorageSQLiteSingleVerNaturalStoreTest::SetUp(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    LOGD("DistributedDBStorageSQLiteSingleVerNaturalStoreTest dir is %s", g_testDir.c_str());
    std::string oriIdentifier = APP_ID + "-" + USER_ID + "-" + "TestGeneralNB";
    std::string identifier = DBCommon::TransferHashString(oriIdentifier);
    std::string g_identifier = DBCommon::TransferStringToHex(identifier);

    g_databaseName = "/" + g_identifier + "/" + DBConstant::SINGLE_SUB_DIR + "/" + DBConstant::MAINDB_DIR + "/" +
        DBConstant::SINGLE_VER_DATA_STORE + ".db";
    DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir + "/" + g_identifier + "/" + DBConstant::SINGLE_SUB_DIR);
    KvDBProperties property;
    property.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    property.SetStringProp(KvDBProperties::STORE_ID, "TestGeneralNB");
    property.SetStringProp(KvDBProperties::IDENTIFIER_DIR, g_identifier);
    property.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);

    g_store = new (std::nothrow) SQLiteSingleVerNaturalStore;
    ASSERT_NE(g_store, nullptr);
    ASSERT_EQ(g_store->Open(property), E_OK);

    int erroCode = E_OK;
    g_connection = static_cast<SQLiteSingleVerNaturalStoreConnection *>(g_store->GetDBConnection(erroCode));
    ASSERT_NE(g_connection, nullptr);
    g_store->DecObjRef(g_store);
    EXPECT_EQ(erroCode, E_OK);
}

void DistributedDBStorageSQLiteSingleVerNaturalStoreTest::TearDown(void)
{
    if (g_connection != nullptr) {
        g_connection->Close();
    }

    g_store = nullptr;
    DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir + "/" + g_identifier + "/" +
        DBConstant::SINGLE_SUB_DIR);
}

/**
  * @tc.name: GetSyncData001
  * @tc.desc: To test the function of querying the data in the time stamp range in the database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, GetSyncData001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, C) interface of the NaturalStore, where A<B<C.
     * @tc.expected: step1. GetSyncData The number of output parameter
     *  in the output parameter OK, dataItems is 1.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData001(g_store, g_connection);
}

/**
  * @tc.name: GetSyncData002
  * @tc.desc: Test the function that the database does not query the data in the time stamp range.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, GetSyncData002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, B) interface of the NaturalStore,
     *  where A<B<C and interference data processing are added.
     * @tc.expected: step1. GetSyncData The number of output parameters
     *  in the output parameter E_NOT_FOUND,dataItems is 0.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData002(g_store, g_connection);
}

/**
  * @tc.name: GetSyncData003
  * @tc.desc: To test the function of querying data when the timestamp range
  *  in the data obtaining interface is invalid.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, GetSyncData003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, B) interface of the NaturalStore, where A>B
     * @tc.expected: step1. The value of GetSyncData is E_INVALID_ARG.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData003(g_store, g_connection);
}

/**
  * @tc.name: GetSyncData004
  * @tc.desc: To the test database Subcon reading, a large number of data records exist in the time stamp range.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, GetSyncData004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, B) interface of the NaturalStore.
     * @tc.expected: step1. Return E_GET_UNFINISHED.
     */
    /**
     * @tc.steps:step2. Continue to obtain data through the GetSyncDataNext() interface
     *  of the NaturalStore until the E_GET_FINISHED message is returned.
     * @tc.expected: step2. When the GetSyncDataNext returns E_GET_FINISHED,
     *  the total number of obtained data is the number of inserted data and the data is consistent.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData004(g_store, g_connection);
}

/**
  * @tc.name: GetSyncData005
  * @tc.desc: In the test database, if a large number of data records exist
  *  in the time stamp range, a packet is read successfully.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, GetSyncData005, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Obtain the data within the time stamp range
     *  through the GetSyncData(A, B) interface of the NaturalStore.
     * @tc.expected: step1. The total size of all data in OK, dataItems is 99K.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData005(g_store, g_connection);
}

/**
  * @tc.name: GetSyncData006
  * @tc.desc: To test the function of reading data when the time stamp range in the database
  *  is greater than the value of blockSize.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, GetSyncData006, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Use the GetSyncData(A, B) interface of the NaturalStore
     *  and set blockSize to 50 kb to obtain the data within the time stamp range.
     * @tc.expected: step1. The system returns E_GET_FINISHED. The size of the obtained data is 1 kb.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::GetSyncData006(g_store, g_connection);
}

/**
  * @tc.name: PutSyncData001
  * @tc.desc: To test the function of synchronizing the new data of the remote device that synchronizes the database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, PutSyncData001, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Set Ioption to synchronous data and insert a (key1, value1) data record by put interface.
     */
    /**
     * @tc.steps:step3. Insert a (key1, value2!=value1, timeStamp, false) data record
     *  through the PutSyncData interface. The value of timeStamp is less than or equal
     *  to the value of timeStamp. For Compare the timestamp to determine whether to synchronization data.
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. The Ioption is set to synchronize data
     *  through the Get interface to obtain the value data of the key1.
     * @tc.expected: step4. Return OK.The obtained value is value1.
     */
    /**
     * @tc.steps:step5. Insert a (key1, value3!=value1, timeStamp, false) data record
     *  through the PutSyncData interface of the NaturalStore. The value of timeStamp
     *  is greater than that of timeStamp inserted in 2.
     * @tc.expected: step5. Return OK.
     */
    /**
     * @tc.steps:step6. The Ioption is set to synchronize data through the Get interface
     *  to obtain the value data of the key1.
     * @tc.expected: step6. Return OK.
     */
    /**
     * @tc.steps:step7. Insert a (key2, value4) data record through the PutSyncData interface.
     * @tc.expected: step7. Return OK.
     */
    /**
     * @tc.steps:step8. The Ioption is set to synchronize data
     *  through the Get interface to obtain the value data of the key2.
     * @tc.expected: step8. Returns OK, and the obtained data is value4.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::PutSyncData001(g_store, g_connection);
}

/**
  * @tc.name: PutSyncData002
  * @tc.desc: To test the function of synchronizing data from the remote device
  *  to the local device after the data is deleted from the remote device.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, PutSyncData002, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Set Ioption to synchronous data and insert a (key1, value1) data record by put interface.
     */
    /**
     * @tc.steps:step3. Insert a (key1, value2!=value1, timeStamp, false) data record
     *  through the PutSyncData interface. The value of timeStamp is less than or equal
     *  to the value of timeStamp. For Compare the timestamp to determine whether delete data.
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. The Ioption is set to synchronize data
     *  through the Get interface to obtain the value data of the key1.
     * @tc.expected: step4. Return OK.The obtained value is value1.
     */
    /**
     * @tc.steps:step5. Insert a (key1, value3!=value1, timeStamp, false) data record
     *  through the PutSyncData interfac. The value of timeStamp
     *  is greater than that of timeStamp inserted in step2.
     * @tc.expected: step5. Return OK.
     */
    /**
     * @tc.steps:step6. The Ioption is set to synchronize data through the Get interface
     *  to obtain the value data of the key1.
     * @tc.expected: step6. Return E_NOT_FOUND.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::PutSyncData002(g_store, g_connection);
}

/**
  * @tc.name: PutSyncData003
  * @tc.desc: To test the function of synchronizing the mixed data of the added
  *  and deleted data from the remote device to the local device.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, PutSyncData003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Insert a data record (key1,value1 is not null) and (key2, value2 is not null)
     *  through the PutSyncData interface.
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. Set Ioption as the synchronization data to obtain the data of key1 and key2.
     * @tc.expected: step2. The Get interface returns OK. The value of key1 is value1,
     *  and the value of key2 is value2.
     */
    /**
     * @tc.steps:step3. Insert a (key3, value3) and delete the data of the (key1, value1).
     * @tc.expected: step3. The PutSyncData returns OK.
     */
    /**
     * @tc.steps:step4. Set Ioption to the synchronization data and obtain the data of key1, key2, and key3.
     * @tc.expected: step4. Get key1 returns E_NOT_FOUND,Get key2.
     *  The value of OK,value is value2, the value of Get key3 is OK,
     *  and the value of value is value3.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::PutSyncData003(g_store, g_connection);
}

/**
  * @tc.name: PutMetaData001
  * @tc.desc: Test metadata insertion and modification.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, PutMetaData001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Run the PutMetaData command to insert a non-empty key1 non-empty value1 data record.
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. Run the PutMetaData command to insert a non-empty key1 non-empty value1 data record.
     * @tc.expected: step2. The obtained value is the same as the value of value1.
     */
    /**
     * @tc.steps:step3. The key value is key1, the value is not empty,
     *  and the value of value2 is different from the value of value1 through the PutMetaData interface.
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. Run the GetMetaData command to obtain the value of key1
     *  and check whether the value is the same as the value of value2.
     * @tc.expected: step4. The obtained value is the same as the value of value2.
     */
    /**
     * @tc.steps:step5. Use PutMetaData to insert a record whose key is empty and value is not empty.
     * @tc.expected: step5. Return E_INVALID_ARGS.
     */
    /**
     * @tc.steps:step6. Use PutMetaData in NaturalStore to insert data whose key2(!=key1)
     *  is not empty and value is empty.
     * @tc.expected: step6. Return OK.
     */
    /**
     * @tc.steps:step7. Obtain the value of key2 and check whether the value is empty.
     * @tc.expected: step7. The obtained value is empty.
     */
    /**
     * @tc.steps:step8. Insert the data whose key size is 1024 and value size is 4Mb
     *  through PutMetaData of NaturalStore.
     * @tc.expected: step8. Return OK.
     */
    /**
     * @tc.steps:step9/10. Insert data items whose key size is greater than 1 kb
     *  or value size greater than 4Mb through PutMetaData of NaturalStore.
     * @tc.expected: step9/10. Return E_INVALID_ARGS.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::PutMetaData001(g_store, g_connection);
}

/**
  * @tc.name: GetMetaData001
  * @tc.desc: To test the function of reading the metadata of a key in the database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, GetMetaData001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Run the PutMetaData command to insert a non-empty key1 non-empty value1 data record.
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. Run the PutMetaData command to insert a non-empty key1 non-empty value1 data record.
     * @tc.expected: step2. The obtained value is the same as the value of value1.
     */
    /**
     * @tc.steps:step3. The key value is key1, the value is not empty,
     *  and the value of value2 is different from the value of value1 through the PutMetaData interface.
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. Run the GetMetaData command to obtain the value of key1
     *  and check whether the value is the same as the value of value2.
     * @tc.expected: step4. The obtained value is the same as the value of value2.
     */
    /**
     * @tc.steps:step5. Use PutMetaData to insert a record whose key is empty and value is not empty.
     * @tc.expected: step5. Return E_INVALID_ARGS.
     */
    /**
     * @tc.steps:step6. Use PutMetaData in NaturalStore to insert data whose key2(!=key1)
     *  is not empty and value is empty.
     * @tc.expected: step6. Return OK.
     */
    /**
     * @tc.steps:step7. Obtain the value of key2 and check whether the value is empty.
     * @tc.expected: step7. The obtained value is empty.
     */
    /**
     * @tc.steps:step8. Insert the data whose key size is 1024 and value size is 4Mb
     *  through PutMetaData of NaturalStore.
     * @tc.expected: step8. Return OK.
     */
    /**
     * @tc.steps:step9/10. Insert data items whose key size is greater than 1 kb
     *  or value size greater than 4Mb through PutMetaData of NaturalStore.
     * @tc.expected: step9/10. Return E_INVALID_ARGS.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::GetMetaData001(g_store, g_connection);
}

/**
  * @tc.name: GetCurrentMaxTimeStamp001
  * @tc.desc: To test the function of obtaining the maximum timestamp when a record exists in the database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, GetCurrentMaxTimeStamp001, TestSize.Level1)
{
    /**
     * @tc.steps:step1/2. Insert a data record into the synchronization database.
     */
    /**
     * @tc.steps:step3. The current maximum timestamp is A.
     */
    /**
     * @tc.steps:step4. Insert a data record into the synchronization database.
     */
    /**
     * @tc.steps:step5. Obtain the maximum timestamp B and check whether B>=A exists.
     * @tc.expected: step5. The obtained timestamp is B>=A.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::GetCurrentMaxTimeStamp001(g_store, g_connection);
}

/**
  * @tc.name: GetCurrentMaxTimeStamp002
  * @tc.desc: Obtain the maximum timestamp when no record exists in the test record library.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, GetCurrentMaxTimeStamp002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Obtains the maximum timestamp in the current database record.
     * @tc.expected: step1. Return timestamp is 0.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::GetCurrentMaxTimeStamp002(g_store);
}

/**
  * @tc.name: LocalDatabaseOperate001
  * @tc.desc: Test the function of inserting data in the local database of the NaturalStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, LocalDatabaseOperate001, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Set Ioption to the local data and insert a record of key1 and value1.
     * @tc.expected: step1/2. Return OK.
     */
    /**
     * @tc.steps: step3. Set Ioption to the local data and obtain the value of key1.
     *  Check whether the value is the same as the value of value1.
     * @tc.expected: step3. The obtained value and value2 are the same.
     */
    /**
     * @tc.steps: step4. Ioption Set this parameter to the local data. Insert key1.
     *  The value cannot be empty. value2(!=value1)
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps: step5. Set Ioption to the local data, GetMetaData to obtain the value of key1,
     *  and check whether the value is the same as the value of value2.
     * @tc.expected: step5. The obtained value and value2 are the same.
     */
    /**
     * @tc.steps: step6. The Ioption parameter is set to the local data.
     *  The data record whose key is empty and value is not empty is inserted.
     * @tc.expected: step6. Return E_INVALID_DATA.
     */
    /**
     * @tc.steps: step7. Set Ioption to the local data, insert data
     *  whose key2(!=key1) is not empty, and value is empty.
     * @tc.expected: step7. Return OK.
     */
    /**
     * @tc.steps: step8. Set option to local data, obtain the value of key2,
     *  and check whether the value is empty.
     * @tc.expected: step8. Return OK, value is empty.
     */
    /**
     * @tc.steps: step9. Ioption Set the local data.
     *  Insert the data whose key size is 1024 and value size is 4Mb.
     * @tc.expected: step9. Return OK.
     */
    /**
     * @tc.steps: step10/11. Set Ioption to the local data and insert data items
     *  whose value is greater than 4Mb or key is bigger than 1Kb
     * @tc.expected: step10/11. Return E_INVALID_ARGS.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::LocalDatabaseOperate001(g_store, g_connection);
}

/**
  * @tc.name: LocalDatabaseOperate002
  * @tc.desc: Test the function of deleting data from the local database of the NaturalStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, LocalDatabaseOperate002, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Set Ioption to the local data and insert a record of key1 and value1.
     * @tc.expected: step1/2. Return OK.
     */
    /**
     * @tc.steps: step3. Set Ioption to the local data and obtain the value of key1.
     *  Check whether the value is the same as the value of value1.
     * @tc.expected: step3. The obtained value and value2 are the same.
     */
    /**
     * @tc.steps: step4. Ioption Set this parameter to the local data. Insert key1.
     *  The value cannot be empty. value2(!=value1)
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps: step5. Set Ioption to the local data, GetMetaData to obtain the value of key1,
     *  and check whether the value is the same as the value of value2.
     * @tc.expected: step5. The obtained value and value2 are the same.
     */
    /**
     * @tc.steps: step6. The Ioption parameter is set to the local data.
     *  The data record whose key is empty and value is not empty is inserted.
     * @tc.expected: step6. Return E_INVALID_DATA.
     */
    /**
     * @tc.steps: step7. Set Ioption to the local data, insert data
     *  whose key2(!=key1) is not empty, and value is empty.
     * @tc.expected: step7. Return OK.
     */
    /**
     * @tc.steps: step8. Set option to local data, obtain the value of key2,
     *  and check whether the value is empty.
     * @tc.expected: step8. Return OK, value is empty.
     */
    /**
     * @tc.steps: step9. Ioption Set the local data.
     *  Insert the data whose key size is 1024 and value size is 4Mb.
     * @tc.expected: step9. Return OK.
     */
    /**
     * @tc.steps: step10/11. Set Ioption to the local data and insert data items
     *  whose value is greater than 4Mb or key is bigger than 1Kb
     * @tc.expected: step10/11. Return E_INVALID_ARGS.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::LocalDatabaseOperate002(g_store, g_connection);
}

/**
  * @tc.name: LocalDatabaseOperate003
  * @tc.desc: To test the function of reading data from the local database of the NaturalStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, LocalDatabaseOperate003, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Set Ioption to the local data and insert a record of key1 and value1.
     * @tc.expected: step1/2. Return OK.
     */
    /**
     * @tc.steps: step3. Set Ioption to the local data and obtain the value of key1.
     *  Check whether the value is the same as the value of value1.
     * @tc.expected: step3. The obtained value and value2 are the same.
     */
    /**
     * @tc.steps: step4. Ioption Set this parameter to the local data. Insert key1.
     *  The value cannot be empty. value2(!=value1)
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps: step5. Set Ioption to the local data, GetMetaData to obtain the value of key1,
     *  and check whether the value is the same as the value of value2.
     * @tc.expected: step5. The obtained value and value2 are the same.
     */
    /**
     * @tc.steps: step6. The Ioption parameter is set to the local data.
     *  The data record whose key is empty and value is not empty is inserted.
     * @tc.expected: step6. Return E_INVALID_DATA.
     */
    /**
     * @tc.steps: step7. Set Ioption to the local data, insert data
     *  whose key2(!=key1) is not empty, and value is empty.
     * @tc.expected: step7. Return OK.
     */
    /**
     * @tc.steps: step8. Set option to local data, obtain the value of key2,
     *  and check whether the value is empty.
     * @tc.expected: step8. Return OK, value is empty.
     */
    /**
     * @tc.steps: step9. Ioption Set the local data.
     *  Insert the data whose key size is 1024 and value size is 4Mb.
     * @tc.expected: step9. Return OK.
     */
    /**
     * @tc.steps: step10/11. Set Ioption to the local data and insert data items
     *  whose value is greater than 4Mb or key is bigger than 1Kb
     * @tc.expected: step10/11. Return E_INVALID_ARGS.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::LocalDatabaseOperate003(g_store, g_connection);
}

/**
  * @tc.name: SyncDatabaseOperate001
  * @tc.desc: To test the function of inserting data of the local device in the synchronization database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, SyncDatabaseOperate001, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Set Ioption to the local data and insert a record of key1 and value1.
     * @tc.expected: step1/2. Return OK.
     */
    /**
     * @tc.steps: step3. Set Ioption to the local data and obtain the value of key1.
     *  Check whether the value is the same as the value of value1.
     * @tc.expected: step3. The obtained value and value2 are the same.
     */
    /**
     * @tc.steps: step4. Ioption Set this parameter to the local data. Insert key1.
     *  The value cannot be empty. value2(!=value1)
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps: step5. Set Ioption to the local data, GetMetaData to obtain the value of key1,
     *  and check whether the value is the same as the value of value2.
     * @tc.expected: step5. The obtained  and value2 are the same.
     */
    /**
     * @tc.steps: step6. The Ioption parameter is set to the local data.
     *  The data record whose key is empty and value is not empty is inserted.
     * @tc.expected: step6. Return E_INVALID_DATA.
     */
    /**
     * @tc.steps: step7. Set Ioption to the local data, insert data
     *  whose key2(!=key1) is not empty, and value is empty.
     * @tc.expected: step7. Return OK.
     */
    /**
     * @tc.steps: step8. Set option to local data, obtain the value of key2,
     *  and check whether the value is empty.
     * @tc.expected: step8. Return OK, value is empty.
     */
    /**
     * @tc.steps: step9. Ioption Set the local data.
     *  Insert the data whose key size is 1024 and value size is 4Mb.
     * @tc.expected: step9. Return OK.
     */
    /**
     * @tc.steps: step10/11. Set Ioption to the local data and insert data items
     *  whose value is greater than 4Mb or key is bigger than 1Kb
     * @tc.expected: step10/11. Return E_INVALID_ARGS.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate001(g_store, g_connection);
}

/**
  * @tc.name: SyncDatabaseOperate002
  * @tc.desc: test the put operation after data synced from other devices.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, SyncDatabaseOperate002, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Add a remote synchronization data record. (key1, value1).
     */
    /**
     * @tc.steps: step3. Ioption is set to synchronous data. Obtains the value data of the key1.
     * @tc.expected: step3. Return OK. The value is the same as the value of value1.
     */
    /**
     * @tc.steps: step4. Ioption Set the data to be synchronized and insert the data of key1,value2.
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps: step3. Ioption is set to synchronous data. Obtains the value data of the key1.
     * @tc.expected: step3. Return OK. The value is the same as the value of value2.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate002(g_store, g_connection);
}

/**
  * @tc.name: SyncDatabaseOperate003
  * @tc.desc: test the delete operation in sync database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, SyncDatabaseOperate003, TestSize.Level1)
{
    /**
     * @tc.steps: step2. Set Ioption to the local data and delete the data whose key is key1 (empty).
     * @tc.expected: step2. Return E_INVALID_ARGS.
     */
    /**
     * @tc.steps: step3. Set Ioption to the local data, insert non-null key1, and non-null value1 data.
     * @tc.expected: step3. Return E_OK.
     */
    /**
     * @tc.steps: step4. Set Ioption to the local data, obtain the value of key1,
     *  and check whether the value is the same as that of value1.
     * @tc.expected: step4. Return E_OK. The obtained value is the same as the value of value1.
     */
    /**
     * @tc.steps: step5. Set Ioption to the local data and delete the data whose key is key1.
     * @tc.expected: step5. Return E_OK.
     */
    /**
     * @tc.steps: step5. Set Ioption to the local data and obtain the value of Key1.
     * @tc.expected: step5. Return E_NOT_FOUND.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate003(g_store, g_connection);
}

/**
  * @tc.name: SyncDatabaseOperate004
  * @tc.desc: test the delete for the data from other devices in sync database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, SyncDatabaseOperate004, TestSize.Level1)
{
    /**
     * @tc.steps: step2. The Ioption parameter is set to synchronize data to obtain the value data of the key1.
     * @tc.expected: step2. Return OK. The value is the same as the value of value1.
     */
    /**
     * @tc.steps: step3. The Ioption parameter is set to synchronize data, and the key1 data is deleted.
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps: step4. The Ioption parameter is set to synchronize data to obtain the value data of the key1.
     * @tc.expected: step4. Return E_NOT_FOUND.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate004(g_store, g_connection);
}

/**
  * @tc.name: SyncDatabaseOperate005
  * @tc.desc: test the reading for sync database.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, SyncDatabaseOperate005, TestSize.Level1)
{
    /**
     * @tc.steps: step2. Set Ioption to the local data and delete the data whose key is key1 (empty).
     * @tc.expected: step2. Return E_INVALID_ARGS.
     */
    /**
     * @tc.steps: step3. Set Ioption to the local data, insert non-null key1, and non-null value1 data.
     * @tc.expected: step3. Return E_OK.
     */
    /**
     * @tc.steps: step4. Set Ioption to the local data, obtain the value of key1,
     *  and check whether the value is the same as that of value1.
     * @tc.expected: step4. Return E_OK. The obtained value is the same as the value of value1.
     */
    /**
     * @tc.steps: step5. Set Ioption to the local data and obtain the value data of Key1.
     *  Check whether the value is the same as the value of value2.
     * @tc.expected: step4. Return E_OK, and the value is the same as the value of value2.
     */
    /**
     * @tc.steps: step5. The Ioption is set to the local.
     *  The data of the key1 and value2(!=value1) is inserted.
     * @tc.expected: step4. Return E_OK.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate005(g_store, g_connection);
}

/**
  * @tc.name: SyncDatabaseOperate006
  * @tc.desc: test the get entries for sync database
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, SyncDatabaseOperate006, TestSize.Level1)
{
    /**
     * @tc.steps: step2/3/4. Set Ioption to synchronous data.
     * Insert the data of key=keyPrefix + 'a', value1.
     * Insert the data of key=keyPrefix + 'c', value2.
     * Insert the data of key length=keyPrefix length - 1, value3.
     * @tc.expected: step2/3/4. Return E_NOT_FOUND.
     */
    /**
     * @tc.steps: step5. Obtain all data whose prefixKey is keyPrefix.
     * @tc.expected: step5. Return OK. The number of obtained data records is 2.
     */
    /**
     * @tc.steps: step6. Obtain all data whose prefixKey is empty.
     * @tc.expected: step6. Return OK. The number of obtained data records is 3.
     */
    /**
     * @tc.steps: step7. Obtain all data whose prefixKey is keyPrefix.
     * @tc.expected: step7. Return E_NOT_SUPPORT.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::SyncDatabaseOperate006(g_store, g_connection);
}

/**
  * @tc.name: ClearRemoteData001
  * @tc.desc: test the clear data synced from the remote by device.
  * @tc.type: FUNC
  * @tc.require: AR000CIFDA AR000CQS3T
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, ClearRemoteData001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. New data is inserted to the B end of the device. [keyB, valueB].
     */
    /**
     * @tc.steps: step2. The device pulls the data of the device B, and the device inserts the [keyA, valueA].
     */
    /**
     * @tc.steps: step3. The device obtains the data of keyA and valueB.
     * @tc.expected: step3. Obtain [keyA, valueA] and [keyB, valueB].
     */
    /**
     * @tc.steps: step4.Invoke the interface for clearing the synchronization data of the B device.
     */
    /**
     * @tc.steps: step5. The device obtains the data of keyA and valueB.
     * @tc.expected: step5. The value of [keyA, valueA] is obtained,
     *  and the value of NOT_FOUND is obtained by querying keyB.
     */
    DistributedDBStorageSingleVerNaturalStoreTestCase::ClearRemoteData001(g_store, g_connection);
}

/**
 * @tc.name: DeleteUserKeyValue001
 * @tc.desc: When a user deletes a data record, the system clears the user record.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, DeleteUserKeyValue001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. delete K1.
     * @tc.expected: step1. delete K1 successfully.
     */
    /**
     * @tc.steps: step2. Real query by sqlite3.
     * @tc.expected: step2. Find KEY_1, not find K2.
     */
    const std::string url = g_testDir + g_databaseName;
    DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue001(g_store, g_connection, url);
}

/**
 * @tc.name: DeleteUserKeyValue002
 * @tc.desc: After the synchronization library data is deleted locally, add the same key data locally.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, DeleteUserKeyValue002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Delete key1 data via Delete interface.
     * @tc.expected: step1. Delete successfully.
     */
    /**
     * @tc.steps: step2. New data from key1, value3 via Put interface.
     * @tc.expected: step2. New data from key1, value3 via Put interface successfully.
     */
    /**
     * @tc.steps: step3. Query key1 data via Get interface.
     * @tc.expected: step3. Query key1 data via Get interface successfully, get value3 by key1.
     */
    /**
     * @tc.steps: step4. Query key1 real data by sqlite3.
     * @tc.expected: step4. Two records were found.
     */
    const std::string url = g_testDir + g_databaseName;
    DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue002(g_store, g_connection, url);
}

/**
 * @tc.name: DeleteUserKeyValue003
 * @tc.desc: After the synchronization database data is deleted locally, the same key data is added from the remote end.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, DeleteUserKeyValue003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Delete data by key1.
     * @tc.expected: step1. Delete successfully.
     */
    /**
     * @tc.steps: step2. Get data by key1.
     * @tc.expected: step1. Key1 not exist in database.
     */
    /**
     * @tc.steps: step3. Get a new data from remote device B , key1, value3,
     *  with a smaller timestamp than the current timestamp.
     */
    /**
     * @tc.steps: step4. Get data by key1.
     * @tc.expected: step4. Key1 not exist in database.
     */
    /**
     * @tc.steps: step5. Get a new data from remote device C , key1, value4,
     *  and the timestamp is larger than the current timestamp.
     */
    /**
     * @tc.steps: step6. Get data by key1.
     * @tc.expected: step6. Key1 not exist in database.
     */
    /**
     * @tc.steps: step7. Get real data by key1.
     * @tc.expected: step7. Get 1 record.
     */
    const std::string url = g_testDir + g_databaseName;
    DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue003(g_store, g_connection, url);
}

/**
 * @tc.name: DeleteUserKeyValue004
 * @tc.desc: Changes in key after remote delete data syncs to local
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, DeleteUserKeyValue004, TestSize.Level1)
{
    /**
     * @tc.steps: step1 2 3. Synchronize data to another device B; delete key1 data from device B;
     *  pull the action of key1 to local.
     */
    /**
     * @tc.steps: step4. Close database.
     */
    /**
     * @tc.steps: step5 6. Get real data by key1;and get the number of records.
     * @tc.expected: step5 6. Not exist key1 real data in database;Get 1 record.
     */
    const std::string url = g_testDir + g_databaseName;
    DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue004(g_store, g_connection, url);
}

/**
 * @tc.name: DeleteUserKeyValue005
 * @tc.desc: New unified key data locally after remote delete data syncs to local
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, DeleteUserKeyValue005, TestSize.Level1)
{
    /**
     * @tc.steps: step1 2 3. Synchronize data to another device B; delete key1 data from device B;
     *  pull the action of key1 to local.
     */
    /**
     * @tc.steps: step4. Put K1 V1 to database.
     * @tc.expected: step4. Put successfully.
     */
    /**
     * @tc.steps: step5. Close database.
     */
    /**
     * @tc.steps: step6 7. Get real data by key1;and get the number of records.
     * @tc.expected: step6 7. Not exist key1 real data in database;Get 2 record.
     */
    const std::string url = g_testDir + g_databaseName;
    DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue005(g_store, g_connection, url);
}

/**
 * @tc.name: DeleteUserKeyValue006
 * @tc.desc: After the remote delete data is synced to the local,
 *  the same key data is added from the remote other devices
 * @tc.type: FUNC
 * @tc.require: AR000CKRTC AR000CQE0D
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBStorageSQLiteSingleVerNaturalStoreTest, DeleteUserKeyValue006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Remote device B sync deletes data key1 and pushes to local.
     */
    /**
     * @tc.steps: step2. Get key1 from database.
     * @tc.expected: step2. Not exist key1.
     */
    /**
     * @tc.steps: step3. Remote device C syncs new data (key1, value2),
     *  timestamp is less than delete timestamp, to local.
     */
    /**
     * @tc.steps: step4. Get key1 from database.
     * @tc.expected: step4. Not exist key1.
     */
    /**
     * @tc.steps: step5. Remote device C syncs new data (key1, value2),
     *  timestamp is bigger than delete timestamp, to local.
     */
    /**
     * @tc.steps: step6. Get key1 from database.
     * @tc.expected: step6. Exist key1.
     */
    /**
     * @tc.steps: step7. Get real data from database.
     * @tc.expected: step7. Get 1 record.
     */
    const std::string url = g_testDir + g_databaseName;
    DistributedDBStorageSingleVerNaturalStoreTestCase::DeleteUserKeyValue006(g_store, g_connection, url);
}

