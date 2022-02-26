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

#include "distributeddb_tools_unit_test.h"
#include "sqlite_local_kvdb_connection.h"
#include "sqlite_multi_ver_data_storage.h"
#include "sqlite_utils.h"
#include "db_constant.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    string g_storeDir;
    SQLiteMultiVerTransaction *g_transaction = nullptr;
    const std::string CREATE_TABLE =
        "CREATE TABLE IF NOT EXISTS version_data(key BLOB, value BLOB, oper_flag INTEGER, version INTEGER, " \
        "timestamp INTEGER, ori_timestamp INTEGER, hash_key BLOB, " \
        "PRIMARY key(hash_key, version));";
}

class DistributedDBStorageTransactionRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageTransactionRecordTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_storeDir = g_testDir + "/test_multi_version.db";
    LOGI("read directory :%s", g_storeDir.c_str());
}

void DistributedDBStorageTransactionRecordTest::TearDownTestCase(void)
{
    remove(g_testDir.c_str());
}

void DistributedDBStorageTransactionRecordTest::SetUp(void)
{
    g_transaction = new (std::nothrow) SQLiteMultiVerTransaction();
    ASSERT_NE(g_transaction, nullptr);
    CipherPassword passwd;
    int errCode = g_transaction->Initialize(g_storeDir, false, CipherType::AES_256_GCM, passwd);
    ASSERT_EQ(errCode, E_OK);
}

void DistributedDBStorageTransactionRecordTest::TearDown(void)
{
    if (g_transaction != nullptr) {
        delete g_transaction;
        g_transaction = nullptr;
    }

    remove(g_storeDir.c_str());
}

/**
  * @tc.name: MultiverStorage001
  * @tc.desc: test the putting non empty data with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the current version.
     */
    Version originVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, originVer), E_OK);

    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);

    /**
     * @tc.steps: step2. Put the new data into the database.
     * @tc.expected: step2. Put returns E_OK.
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);

    /**
     * @tc.steps: step3. Get the new data and check the value.
     * @tc.expected: step3. Get returns E_OK and the read value is equal to the put value.
     */
    Value valueRead;
    EXPECT_EQ(g_transaction->Get(key, valueRead), E_OK);

    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value), true);

    /**
     * @tc.steps: step4. Get the current max version.
     * @tc.expected: step4. The current max version is greater than the max version before put.
     */
    Version currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    ASSERT_GT(currentVer, originVer);
}

/**
  * @tc.name: MultiverStorage002
  * @tc.desc: test the putting data(empty key) with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the current version.
     */
    Version originVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, originVer), E_OK);

    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    /**
     * @tc.steps: step2. Put the new data whose key is empty and value is not empty into the database.
     * @tc.expected: step2. Put returns not E_OK
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_NE(g_transaction->Put(key, value), E_OK);
     /**
     * @tc.steps: step3. Get the current max version.
     * @tc.expected: step3. The current max version is equal to the max version before put
     */
    Version currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_EQ(currentVer, originVer);
}

/**
  * @tc.name: MultiverStorage003
  * @tc.desc: test the putting data(empty value) with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the current version.
     */
    Version originVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, originVer), E_OK);

    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    Value value;
    /**
     * @tc.steps: step2. Put the new data whose key is not empty and value is empty into the database.
     * @tc.expected: step2. Put returns E_OK
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);

    Value valueRead;
    /**
     * @tc.steps: step3. Get the new data and check the value.
     * @tc.expected: step3. Get returns E_OK and the read value is equal to the put value.
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value), true);
    /**
     * @tc.steps: step4. Get the current max version.
     * @tc.expected: step4. The current max version is greater than the max version before put.
     */
    Version currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    ASSERT_GT(currentVer, originVer);
}

/**
  * @tc.name: MultiverStorage004
  * @tc.desc: Update the data value to non-empty with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the current version.
     */
    Version originVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, originVer), E_OK);

    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);

    /**
     * @tc.steps: step2. Put the data whose key is not empty and value is empty into the database.
     * @tc.expected: step2. Put returns E_OK
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    Version currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_GT(currentVer, originVer);
    originVer = currentVer;
    g_transaction->ResetVersion();

    Value valueChanged;

    /**
     * @tc.steps: step3. Update the data with another non-empty value.
     */
    DistributedDBToolsUnitTest::GetRandomKeyValue(valueChanged);
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key, valueChanged), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step4. Get the data according the key and check the value.
     * @tc.steps: step5. Get the current max version.
     * @tc.expected: step4. Get returns E_OK and the value is equal to the new put value.
     * @tc.expected: step5. The current max version is greater than the max version before update.
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, valueChanged), true);

    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    ASSERT_GT(currentVer, originVer);
}

/**
  * @tc.name: MultiverStorage005
  * @tc.desc: Update the data value to empty with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the current version.
     */
    Version originVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, originVer), E_OK);

    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    /**
     * @tc.steps: step2. Put the data whose key is not empty and value is not empty into the database.
     * @tc.expected: step2. Put returns E_OK
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    Version currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_GT(currentVer, originVer);
    originVer = currentVer;
    g_transaction->ResetVersion();

    Value valueChanged;
    /**
     * @tc.steps: step3. Update the data with empty value.
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key, valueChanged), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step4. Get the data according the key and check the value.
     * @tc.steps: step5. Get the current max version.
     * @tc.expected: step4. Get returns E_OK and the value is empty.
     * @tc.expected: step5. The current max version is greater than the max version before update.
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, valueChanged), true);
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    ASSERT_GT(currentVer, originVer);
}

/**
  * @tc.name: MultiverStorage006
  * @tc.desc: Delete the existed data with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the current version.
     */
    Version originVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, originVer), E_OK);

    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    /**
     * @tc.steps: step2. Put the data whose key is not empty and value is not empty into the database.
     * @tc.expected: step2. Put returns E_OK
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    Version currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_GT(currentVer, originVer);
    originVer = currentVer;

    EXPECT_EQ(g_transaction->Get(key, value), E_OK);
    Value valueChanged;
    /**
     * @tc.steps: step3. Delete the data according the key.
     * @tc.expected: step3. Delete returns E_OK.
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Delete(key), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step4. Get the value according the key.
     * @tc.expected: step4. Get returns -E_NOT_FOUND.
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), -E_NOT_FOUND);
    /**
     * @tc.steps: step5. Get the current max version.
     * @tc.expected: step5. The current max version is greater than the max version before delete.
     */
    currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    ASSERT_GT(currentVer, originVer);
}

/**
  * @tc.name: MultiverStorage007
  * @tc.desc: Delete the non-existed data with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the current version.
     */
    Version originVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, originVer), E_OK);

    Key key = {12, 57, 89};
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    Version currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_GT(currentVer, originVer);
    originVer = currentVer;
    g_transaction->ResetVersion();

    Key newKey = {87, 68, 78};
    /**
     * @tc.steps: step2. Delete the non-existed data according the key.
     * @tc.expected: step2. Delete returns not E_OK.
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_NE(g_transaction->Delete(newKey), E_OK);
    /**
     * @tc.steps: step3. Get the current max version.
     * @tc.expected: step2. The current max version is equal to the max version before delete.
     */
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_EQ(currentVer, originVer);
}

/**
  * @tc.name: MultiverStorage008
  * @tc.desc: Delete an empty key with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the current version.
     */
    Version originVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, originVer), E_OK);

    Key key;
    /**
     * @tc.steps: step2. Delete the data whose key is empty from the empty database.
     * @tc.steps: step3. Get the current max version.
     * @tc.expected: step2. Delete returns not E_OK
     * @tc.expected: step3. The current max version is equal to the max version before delete.
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_NE(g_transaction->Delete(key), E_OK);
    Version currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_EQ(currentVer, originVer);
    originVer = currentVer;
    g_transaction->ResetVersion();

    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    /**
     * @tc.steps: step4. Put the non-empty key and non-empty value into the database.
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_NE(currentVer, originVer);
    originVer = currentVer;
    g_transaction->ResetVersion();
    key.clear();
    /**
     * @tc.steps: step5. Delete the data whose key is empty from the non-empty database.
     * @tc.steps: step6. Get the current max version.
     * @tc.expected: step5. Delete returns not E_OK
     * @tc.expected: step6. The current max version is equal to the max version before delete.
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_NE(g_transaction->Delete(key), E_OK);
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_EQ(currentVer, originVer);
}

/**
  * @tc.name: MultiverStorage009
  * @tc.desc: Clear the existed data with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the current version.
     */
    Version originVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, originVer), E_OK);
    Key key1, key2;
    Value value1, value2;

    DistributedDBToolsUnitTest::GetRandomKeyValue(key1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(key2);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2);

    /**
     * @tc.steps: step2. Put 2 entries into the database.
     * @tc.expected: step2. Put returns E_OK
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Put(key1, value1), E_OK);
    EXPECT_EQ(g_transaction->Put(key2, value2), E_OK);
    Version currentVer = 0;
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_NE(currentVer, originVer);
    originVer = currentVer;
    g_transaction->ResetVersion();
    /**
     * @tc.steps: step3. Clear data from the database.
     * @tc.expected: step3. Clear returns E_OK
     */
    g_transaction->SetVersion(originVer + 1);
    EXPECT_EQ(g_transaction->Clear(), E_OK);
    Value value1Read, value2Read;
    /**
     * @tc.steps: step4. Get the current max version.
     * @tc.expected: step4. The current max version is greater than the max version before clear.
     */
    EXPECT_EQ(g_transaction->GetMaxVersion(MultiVerDataType::ALL_TYPE, currentVer), E_OK);
    EXPECT_GT(currentVer, originVer);
    /**
     * @tc.steps: step5. Get the put data before clear.
     * @tc.expected: step5. Cannot get the data after clear.
     */
    EXPECT_NE(g_transaction->Get(key1, value1Read), E_OK);
    EXPECT_NE(g_transaction->Get(key2, value2Read), E_OK);
}

/**
  * @tc.name: MultiverStorage010
  * @tc.desc: Get the existed data with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Generate the random data.
     */
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);

    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    Value valueRead;
    EXPECT_EQ(g_transaction->Get(key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value), true);
}

/**
  * @tc.name: MultiverStorage011
  * @tc.desc: Get the non-existed data with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Generate the random data.
     */
    Key key;
    Value value, valueRead;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    EXPECT_EQ(g_transaction->Get(key, valueRead), -E_NOT_FOUND);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);

    /**
     * @tc.steps: step2. Put the data whose key is not empty and value is not empty into the database.
     * @tc.expected: step2. Put returns E_OK
     */
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);

    key.push_back('7');
    /**
     * @tc.steps: step3. Get the non-existed key.
     * @tc.expected: step3. Get returns E_OK
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: MultiverStorage012
  * @tc.desc: Get the empty-key data with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage012, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Generate the random data.
     */
    Key key;
    Value value;
    /**
     * @tc.steps: step2. Get the value according the empty key from the empty database.
     * @tc.expected: step2. Get returns not E_OK.
     */
    EXPECT_NE(g_transaction->Get(key, value), E_OK);
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    /**
     * @tc.steps: step3. Put the non-empty data into the database.
     */
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    Key keyEmpty;
    /**
     * @tc.steps: step4. Get the value according the empty key from the non-empty database.
     * @tc.expected: step4. Get returns not E_OK.
     */
    EXPECT_NE(g_transaction->Get(keyEmpty, value), E_OK);
}

/**
  * @tc.name: MultiverStorage013
  * @tc.desc: Get the deleted data with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage013, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Generate the random data.
     */
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    /**
     * @tc.steps: step2. put the non-empty data into the database.
     */
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    EXPECT_EQ(g_transaction->Get(key, value), E_OK);
    /**
     * @tc.steps: step3. delete the data from the database.
     */
    EXPECT_EQ(g_transaction->Delete(key), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step4. Get the value according the key.
     * @tc.expected: step4. Get returns -E_NOT_FOUND.
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: MultiverStorage014
  * @tc.desc: Get the modified data with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Generate the random data.
     */
    Key key;
    Value value;

    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);
    /**
     * @tc.steps: step2. put the non-empty data into the database.
     */
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    Value valueChanged = value;
    valueChanged.push_back('H');
    /**
     * @tc.steps: step3. update the data into the database.
     */
    EXPECT_EQ(g_transaction->Put(key, valueChanged), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step4. Get the value according the key and check the value.
     * @tc.expected: step4. Get returns E_OK and the read value is equal to the put value.
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, valueChanged), true);
}

/**
  * @tc.name: MultiverStorage015
  * @tc.desc: Get the data after clear with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Generate the random data.
     */
    Key key;
    Value value;

    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);

    /**
     * @tc.steps: step2. put the non-empty data into the database.
     */
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    /**
     * @tc.steps: step3. clear the database.
     */
    EXPECT_EQ(g_transaction->Clear(), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step4. get the data from the database after clear.
     * @tc.expected: step4. Get returns -E_NOT_FOUND.
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), -E_NOT_FOUND);
}

/**
  * @tc.name: MultiverStorage016
  * @tc.desc: Get the new inserted data after clear with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage016, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Generate the random data.
     */
    Key key1;
    Value value1;

    DistributedDBToolsUnitTest::GetRandomKeyValue(key1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);

    /**
     * @tc.steps: step2. put the non-empty data into the database.
     */
    EXPECT_EQ(g_transaction->Put(key1, value1), E_OK);
    /**
     * @tc.steps: step3. clear the database.
     */
    EXPECT_EQ(g_transaction->Clear(), E_OK);
    Value valueRead;
    EXPECT_EQ(g_transaction->Get(key1, valueRead), -E_NOT_FOUND);
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2);
    /**
     * @tc.steps: step4. re-put the data into the database using another value.
     */
    EXPECT_EQ(g_transaction->Put(key1, value2), E_OK);
    /**
     * @tc.steps: step5. Get the value according the key and check the value.
     * @tc.expected: step5. Get returns E_OK and the read value is equal to the new put value.
     */
    EXPECT_EQ(g_transaction->Get(key1, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, value2), true);
}

/**
  * @tc.name: MultiverStorage017
  * @tc.desc: Get the new inserted data after delete with the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage017, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get the random data.
     */
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, 79);

    /**
     * @tc.steps: step2. Put one data into the database.
     */
    EXPECT_EQ(g_transaction->Put(key, value), E_OK);
    /**
     * @tc.steps: step3. Delete the data from the database.
     */
    EXPECT_EQ(g_transaction->Delete(key), E_OK);
    Value valueRead;
    /**
     * @tc.steps: step4. Get the data from the database according the key.
     * @tc.expected: step4. Get returns -E_NOT_FOUND.
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), -E_NOT_FOUND);
    Value valueChanged;
    /**
     * @tc.steps: step5. Put the same key, different value into the database.
     */
    DistributedDBToolsUnitTest::GetRandomKeyValue(valueChanged, 178);
    EXPECT_EQ(g_transaction->Put(key, valueChanged), E_OK);
    /**
     * @tc.steps: step6. Get the data from the database according the key.
     * @tc.expected: step6. Get returns E_OK and the get value is equal to the value put int the step5.
     */
    EXPECT_EQ(g_transaction->Get(key, valueRead), E_OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(valueRead, valueChanged), true);
}

/**
  * @tc.name: MultiverStorage018
  * @tc.desc: Get the batch inserted data through the non-empty prefix key.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage018, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the batch data into the database(3 entries have the same prefix key,
     *  and another has different prefix key).
     */
    Entry entry1, entry2, entry3, entry4;
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry1.key, 97);
    entry2.key = entry1.key;
    entry2.key.push_back('W');
    entry3.key = entry1.key;
    entry3.key.push_back('C');
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry4.key, 67);

    DistributedDBToolsUnitTest::GetRandomKeyValue(entry1.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry2.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry3.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry4.value);

    EXPECT_EQ(g_transaction->Put(entry1.key, entry1.value), E_OK);
    EXPECT_EQ(g_transaction->Put(entry2.key, entry2.value), E_OK);
    EXPECT_EQ(g_transaction->Put(entry3.key, entry3.value), E_OK);
    EXPECT_EQ(g_transaction->Put(entry4.key, entry4.value), E_OK);
    /**
     * @tc.steps: step2. Get the batch data using the prefix key.
     * @tc.expected: step2. GetEntries returns E_OK and the number of the result entries is E_OK.
     */
    std::vector<Entry> entriesRead;
    EXPECT_EQ(g_transaction->GetEntries(entry1.key, entriesRead), E_OK);
    EXPECT_EQ(entriesRead.size(), 3UL);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry1, entriesRead), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry2, entriesRead), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry3, entriesRead), true);
}

/**
  * @tc.name: MultiverStorage019
  * @tc.desc: Get the non-existed data through the non-empty prefix key.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage019, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the batch data into the database.
     */
    Entry entry1, entry2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry1.key, 97);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry2.key, 204);

    DistributedDBToolsUnitTest::GetRandomKeyValue(entry1.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry2.value);

    EXPECT_EQ(g_transaction->Put(entry1.key, entry1.value), E_OK);
    EXPECT_EQ(g_transaction->Put(entry2.key, entry2.value), E_OK);
    /**
     * @tc.steps: step2. Get the batch data from the database using the prefix key different from the data put before.
     * @tc.expected: step2. GetEntries returns -E_NOT_FOUND.
     */
    std::vector<Entry> entriesRead;
    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, 210);
    EXPECT_EQ(g_transaction->GetEntries(key, entriesRead), -E_NOT_FOUND);
}

/**
  * @tc.name: MultiverStorage020
  * @tc.desc: Get all the data through the empty prefix key.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage020, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the data.
     */
    Entry entry1, entry2, entry3;
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry1.key, 134);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry2.key, 204);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry3.key, 43);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry1.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry2.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry3.value);

    EXPECT_EQ(g_transaction->Put(entry1.key, entry1.value), E_OK);
    EXPECT_EQ(g_transaction->Put(entry2.key, entry2.value), E_OK);
    EXPECT_EQ(g_transaction->Put(entry3.key, entry3.value), E_OK);
    /**
     * @tc.steps: step2. Get the batch data from the database using the empty prefix key.
     * @tc.expected: step2. GetEntries returns E_OK and .
     */
    std::vector<Entry> entriesRead;
    Key keyEmpty;
    EXPECT_EQ(g_transaction->GetEntries(keyEmpty, entriesRead), E_OK);
    EXPECT_EQ(entriesRead.size(), 3UL);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry1, entriesRead), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry2, entriesRead), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry3, entriesRead), true);
}

/**
  * @tc.name: MultiverStorage021
  * @tc.desc: Get the data through the empty prefix key for multiple put the same key data.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage021, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the same key, different value for 3 times into the database.
     */
    Key key;
    Value value1, value2, value3;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);

    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, 46);
    EXPECT_EQ(g_transaction->Put(key, value1), E_OK);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, 28);
    EXPECT_EQ(g_transaction->Put(key, value2), E_OK);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value3, 157);
    EXPECT_EQ(g_transaction->Put(key, value3), E_OK);
    /**
     * @tc.steps: step2. Get the batch data from the database using the empty prefix key.
     * @tc.expected: step2. GetEntries returns E_OK and the entries size is 1.
     */
    std::vector<Entry> entriesRead;
    EXPECT_EQ(g_transaction->GetEntries(key, entriesRead), E_OK);
    EXPECT_EQ(entriesRead.size(), 1UL);
    Entry entry = {key, value3};

    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry, entriesRead), true);
}

/**
  * @tc.name: MultiverStorage022
  * @tc.desc: Get the data through the empty prefix key for deleted data.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage022, TestSize.Level1)
{
    std::vector<Entry> entries;
    Entry entry;
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry.value);
    Entry entry1 = entry;
    entry1.key.push_back('o');
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry1.value);
    entries.push_back(entry);
    entries.push_back(entry1);
    /**
     * @tc.steps: step1. Put the batch data.
     */
    EXPECT_EQ(g_transaction->PutBatch(entries), E_OK);
    std::vector<Key> keys = {entry.key, entry1.key};
    /**
     * @tc.steps: step2. Delete the batch data.
     */
    EXPECT_EQ(g_transaction->Delete(entry.key), E_OK);
    EXPECT_EQ(g_transaction->Delete(entry1.key), E_OK);

    /**
     * @tc.steps: step3. Get all the data.
     * @tc.expected: step3. Returns -E_NOT_FOUND.
     */
    Key keyEmpty;
    std::vector<Entry> entriesRead;
    EXPECT_EQ(g_transaction->GetEntries(keyEmpty, entriesRead), -E_NOT_FOUND);
}

/**
  * @tc.name: MultiverStorage023
  * @tc.desc: Get the data through the empty prefix key for updated data.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage023, TestSize.Level1)
{
    Key key1, key2;
    Value value1, value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1, 10);
    key2 = key1;
    key2.push_back('S');
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, 46);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, 28);
    /**
     * @tc.steps: step1. Put the batch data.
     */
    EXPECT_EQ(g_transaction->Put(key1, value1), E_OK);
    EXPECT_EQ(g_transaction->Put(key2, value2), E_OK);
    Value value1Changed, value2Changed;
    /**
     * @tc.steps: step2. Update the batch data.
     */
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1Changed, 86);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2Changed, 149);
    EXPECT_EQ(g_transaction->Put(key1, value1Changed), E_OK);
    EXPECT_EQ(g_transaction->Put(key2, value2Changed), E_OK);
    Key keyEmpty;
    /**
     * @tc.steps: step3. Get all the data.
     * @tc.expected: step3. the data are equal to the updated data.
     */
    std::vector<Entry> entriesRead;
    EXPECT_EQ(g_transaction->GetEntries(keyEmpty, entriesRead), E_OK);
    ASSERT_EQ(entriesRead.size(), 2UL);

    Entry entry1 = {key1, value1Changed};
    Entry entry2 = {key2, value2Changed};
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry1, entriesRead), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry2, entriesRead), true);
}

/**
  * @tc.name: MultiverStorage024
  * @tc.desc: Get the data through the empty prefix key for cleared data.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage024, TestSize.Level1)
{
    Key key1, key2;
    Value value1, value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1, 10);
    DistributedDBToolsUnitTest::GetRandomKeyValue(key2, 20);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2);
    /**
     * @tc.steps: step1. Put the batch data.
     */
    EXPECT_EQ(g_transaction->Put(key1, value1), E_OK);
    EXPECT_EQ(g_transaction->Put(key2, value2), E_OK);
    Key keyEmpty;
    std::vector<Entry> entriesRead;
    /**
     * @tc.steps: step2. Get all the data.
     * @tc.expected: step2. the data are equal to the data put before.
     */
    EXPECT_EQ(g_transaction->GetEntries(keyEmpty, entriesRead), E_OK);
    ASSERT_EQ(entriesRead.size(), 2UL);
    /**
     * @tc.steps: step3. Clear the data and get all the data.
     * @tc.expected: step3. Get returns -E_NOT_FOUND.
     */
    EXPECT_EQ(g_transaction->Clear(), E_OK);
    EXPECT_EQ(g_transaction->GetEntries(keyEmpty, entriesRead), -E_NOT_FOUND);
}

/**
  * @tc.name: MultiverStorage025
  * @tc.desc: Get the data through the put, delete, re-put operation.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBStorageTransactionRecordTest, MultiverStorage025, TestSize.Level1)
{
    std::vector<Entry> entries;
    Entry entry;
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry.value);
    Entry entry1 = entry;
    entry1.key.push_back('q');
    DistributedDBToolsUnitTest::GetRandomKeyValue(entry1.value);
    entries.push_back(entry);
    entries.push_back(entry1);
    /**
     * @tc.steps: step1. Put the batch data.
     */
    EXPECT_EQ(g_transaction->PutBatch(entries), E_OK);
    std::vector<Key> keys = {entry.key, entry1.key};
    /**
     * @tc.steps: step2. Delete the batch data.
     */
    EXPECT_EQ(g_transaction->Delete(entry.key), E_OK);
    EXPECT_EQ(g_transaction->Delete(entry1.key), E_OK);
    /**
     * @tc.steps: step3. Get all the data.
     * @tc.expected: step3. Get results -E_NOT_FOUND.
     */
    Key keyEmpty;
    std::vector<Entry> entriesRead;
    EXPECT_EQ(g_transaction->GetEntries(keyEmpty, entriesRead), -E_NOT_FOUND);
    entry.value.push_back('q');
    entry1.value.push_back('s');
    Value valueRead, valueRead1;
    entriesRead.clear();
    /**
     * @tc.steps: step5. Re-put the different value into the database.
     */
    EXPECT_EQ(g_transaction->Put(entry.key, valueRead), E_OK);
    EXPECT_EQ(g_transaction->Put(entry1.key, valueRead1), E_OK);
    /**
     * @tc.steps: step6. Get all the data.
     * @tc.expected: step6. Get results E_OK and the data are equal to the inserted data after deleted operation.
     */
    EXPECT_EQ(g_transaction->GetEntries(keyEmpty, entriesRead), E_OK);
    ASSERT_EQ(entriesRead.size(), 2UL);

    entry.value.clear();
    entry1.value.clear();
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry, entriesRead), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsKvEntryExist(entry1, entriesRead), true);
}

