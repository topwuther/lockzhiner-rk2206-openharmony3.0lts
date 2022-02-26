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

#include "types.h"
#include "sqlite_utils.h"
#include "distributeddb_tools_unit_test.h"
#include "log_print.h"
#include "sqlite_single_ver_natural_store_connection.h"
#include "distributeddb_data_generate_unit_test.h"
#include "res_finalizer.h"
#include "db_common.h"
#include "db_constant.h"
#include "platform_specific.h"
#include "sqlite_single_ver_result_set.h"
#include "kvdb_manager.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;

namespace {
    const int INSERT_NUMBER = 10;
    const Key EMPTY_KEY;
    const SQLiteSingleVerResultSet::Option OPTION = {ResultSetCacheMode::CACHE_ENTRY_ID_ONLY, 1};

    string g_testDir;
    string g_identifier;
    SQLiteSingleVerNaturalStore *g_store = nullptr;
    SQLiteSingleVerNaturalStoreConnection *g_connection = nullptr;
    KvDBProperties g_Property;
    const string STORE_ID = STORE_ID_SYNC;
}
class DistributedDBStorageResultAndJsonOptimizeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageResultAndJsonOptimizeTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    std::string origIdentifier = USER_ID + "-" + APP_ID + "-" + STORE_ID;
    std::string identifier = DBCommon::TransferHashString(origIdentifier);
    g_identifier = DBCommon::TransferStringToHex(identifier);
    std::string dir = g_testDir + g_identifier + "/" + DBConstant::SINGLE_SUB_DIR;
    DIR *dirTmp = opendir(dir.c_str());
    if (dirTmp == nullptr) {
        OS::MakeDBDirectory(dir);
    } else {
        closedir(dirTmp);
    }
    g_Property.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    g_Property.SetStringProp(KvDBProperties::STORE_ID, STORE_ID);
    g_Property.SetStringProp(KvDBProperties::IDENTIFIER_DIR, g_identifier);
    g_Property.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
}

void DistributedDBStorageResultAndJsonOptimizeTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBStorageResultAndJsonOptimizeTest::SetUp(void)
{
    /**
     * @tc.setup: 1. Create a SQLiteSingleVerNaturalStore.
     *            2. Set the ResultSet cache mode to CACHE_ENTRY_ID_ONLY.
     *            3. Put 10 records.
     */
    g_store = new (std::nothrow) SQLiteSingleVerNaturalStore;
    ASSERT_NE(g_store, nullptr);
    ASSERT_EQ(g_store->Open(g_Property), E_OK);

    int errCode = E_OK;
    g_connection = static_cast<SQLiteSingleVerNaturalStoreConnection *>(g_store->GetDBConnection(errCode));
    ASSERT_NE(g_connection, nullptr);
    g_store->DecObjRef(g_store);
    EXPECT_EQ(errCode, E_OK);

    IOption option;
    option.dataType = IOption::SYNC_DATA;
    g_connection->Clear(option);
    Key insertKey;
    ASSERT_EQ(g_connection->StartTransaction(), E_OK);
    for (int i = 1; i < INSERT_NUMBER + 1; i++) {
        insertKey.clear();
        insertKey.push_back(i);
        ASSERT_EQ(g_connection->Put(option, insertKey, VALUE_1), OK);
    }
    ASSERT_EQ(g_connection->Commit(), E_OK);
}

void DistributedDBStorageResultAndJsonOptimizeTest::TearDown(void)
{
    /**
     * @tc.teardown: Release the SQLiteSingleVerNaturalStore.
     */
    if (g_connection != nullptr) {
        g_connection->Close();
        g_connection = nullptr;
    }

    g_store = nullptr;
    KvDBManager::RemoveDatabase(g_Property);
}

/**
  * @tc.name: ResultSetOpen001
  * @tc.desc: Test the SQLiteSingleVerResultSet Open function
  * @tc.type: FUNC
  * @tc.require: AR000F3OP0
  * @tc.author: xushaohua
  */
HWTEST_F(DistributedDBStorageResultAndJsonOptimizeTest, ResultSetOpen001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a SQLiteSingleVerResultSet.
     */
    std::unique_ptr<SQLiteSingleVerResultSet> resultSet1 =
        std::make_unique<SQLiteSingleVerResultSet>(g_store, EMPTY_KEY, OPTION);

    /**
     * @tc.steps: step2. Call SQLiteSingleVerResultSet.Open with parameter true.
     * @tc.expected: step2. Expect return E_OK.
     */
    EXPECT_EQ(resultSet1->Open(true), E_OK);

    /**
     * @tc.steps: step3. Create a SQLiteSingleVerResultSet.
     */
    std::unique_ptr<SQLiteSingleVerResultSet> resultSet2 =
        std::make_unique<SQLiteSingleVerResultSet>(g_store, EMPTY_KEY, OPTION);

    /**
     * @tc.steps: step4. Call SQLiteSingleVerResultSet.Open with parameter false.
     * @tc.expected: step4. Expect return E_OK.
     */
    EXPECT_EQ(resultSet2->Open(false), E_OK);

    /**
     * @tc.steps: step5. Close all ResultSet.
     */
    resultSet1->Close();
    resultSet2->Close();
}

/**
  * @tc.name: ResultSetGetCount001
  * @tc.desc: Test the SQLiteSingleVerResultSet GetCount function.
  * @tc.type: FUNC
  * @tc.require: AR000F3OP0
  * @tc.author: xushaohua
  */
HWTEST_F(DistributedDBStorageResultAndJsonOptimizeTest, ResultSetGetCount001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a SQLiteSingleVerResultSet.
     */
    std::unique_ptr<SQLiteSingleVerResultSet> resultSet =
        std::make_unique<SQLiteSingleVerResultSet>(g_store, EMPTY_KEY, OPTION);

    /**
     * @tc.steps: step2. Call SQLiteSingleVerResultSet.Open
     * @tc.expected: step2. Expect return E_OK.Gits
     */
    EXPECT_EQ(resultSet->Open(false), E_OK);

    /**
     * @tc.steps: step2. Call SQLiteSingleVerResultSet.GetCount
     * @tc.expected: step2. Expect return INSERT_NUMBER.
     */
    EXPECT_EQ(resultSet->GetCount(), INSERT_NUMBER);

    /**
     * @tc.steps: step3. Close the ResultSet.
     */
    resultSet->Close();
}

/**
  * @tc.name: ResultSetMoveTo001
  * @tc.desc: Test the SQLiteSingleVerResultSet MoveTo And GetPosition function.
  * @tc.type: FUNC
  * @tc.require: AR000F3OP0
  * @tc.author: xushaohua
  */
HWTEST_F(DistributedDBStorageResultAndJsonOptimizeTest, ResultSetMoveTo001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a SQLiteSingleVerResultSet.
     */
    std::unique_ptr<SQLiteSingleVerResultSet> resultSet =
        std::make_unique<SQLiteSingleVerResultSet>(g_store, EMPTY_KEY, OPTION);

    /**
     * @tc.steps: step2. Call SQLiteSingleVerResultSet.Open.
     * @tc.expected: step2. Expect return E_OK.
     */
    EXPECT_EQ(resultSet->Open(false), E_OK);

    /**
     * @tc.steps: step3. Call SQLiteSingleVerResultSet MoveTo INSERT_NUMBER - 1
     * @tc.expected: step3. Expect return E_OK.
     */
    EXPECT_EQ(resultSet->MoveTo(INSERT_NUMBER - 1), E_OK);

    /**
     * @tc.steps: step4. Call SQLiteSingleVerResultSet GetPosition
     * @tc.expected: step5. Expect return INSERT_NUMBER - 1.
     */
    EXPECT_EQ(resultSet->GetPosition(), INSERT_NUMBER - 1);

    /**
     * @tc.steps: step5. Call SQLiteSingleVerResultSet MoveTo INSERT_NUMBER
     * @tc.expected: step5. Expect return -E_INVALID_ARGS.
     */
    EXPECT_EQ(resultSet->MoveTo(INSERT_NUMBER), -E_INVALID_ARGS);

    /**
     * @tc.steps: step6. Call SQLiteSingleVerResultSet GetPosition
     * @tc.expected: step6. Expect return INSERT_NUMBER.
     */
    EXPECT_EQ(resultSet->GetPosition(), INSERT_NUMBER);

    /**
     * @tc.steps: step7. Call SQLiteSingleVerResultSet MoveTo -1
     * @tc.expected: step7. Expect return E_INVALID_ARGS.
     */
    EXPECT_EQ(resultSet->MoveTo(-1), -E_INVALID_ARGS);

    /**
     * @tc.steps: step8. Call SQLiteSingleVerResultSet GetPosition
     * @tc.expected: step8. Expect return 0.
     */
    EXPECT_EQ(resultSet->GetPosition(), -1);

    /**
     * @tc.steps: step9. Call SQLiteSingleVerResultSet MoveTo 0
     * @tc.expected: step9. Expect return E_OK.
     */
    EXPECT_EQ(resultSet->MoveTo(0), E_OK);

    /**
     * @tc.steps: step10. Call SQLiteSingleVerResultSet GetPosition
     * @tc.expected: step10. Expect return 0.
     */
    EXPECT_EQ(resultSet->GetPosition(), 0);

    /**
     * @tc.steps: step11. Close the ResultSet.
     */
    resultSet->Close();
}

/**
  * @tc.name: ResultSetGetEntry001
  * @tc.desc: Test the SQLiteSingleVerResultSet GetEntry function.
  * @tc.type: FUNC
  * @tc.require: AR000F3OP0
  * @tc.author: xushaohua
  */
HWTEST_F(DistributedDBStorageResultAndJsonOptimizeTest, ResultSetGetEntry001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a SQLiteSingleVerResultSet.
     */
    std::unique_ptr<SQLiteSingleVerResultSet> resultSet =
        std::make_unique<SQLiteSingleVerResultSet>(g_store, EMPTY_KEY, OPTION);

    /**
     * @tc.steps: step2. Call SQLiteSingleVerResultSet.Open
     * @tc.expected: step2. Expect return E_OK.
     */
    EXPECT_EQ(resultSet->Open(false), E_OK);

    /**
     * @tc.steps: step2. Call SQLiteSingleVerResultSet MoveTo 0 And GetEntry
     * @tc.expected: step2. Expect return E_OK.
     */
    Entry entry;
    ASSERT_EQ(resultSet->MoveTo(0), E_OK);
    EXPECT_EQ(resultSet->GetEntry(entry), E_OK);

    /**
     * @tc.expected: step2. Expect return Key == { 1 }, value == VALUE_1.
     */
    const Key key = { 1 };
    EXPECT_EQ(entry.key, key);
    EXPECT_EQ(entry.value, VALUE_1);

    /**
     * @tc.steps: step3. Close the ResultSet.
     */
    resultSet->Close();
}
