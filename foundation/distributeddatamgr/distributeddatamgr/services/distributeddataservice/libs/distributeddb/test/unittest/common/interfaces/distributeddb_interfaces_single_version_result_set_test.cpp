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

#include "distributeddb_data_generate_unit_test.h"
#include "kv_store_nb_delegate_impl.h"
#include "sqlite_single_ver_natural_store.h"
#include "sqlite_single_ver_natural_store_connection.h"
#include "db_common.h"
#include "db_constant.h"
#include "db_types.h"
#include "result_entries_window.h"
#include "ikvdb_raw_cursor.h"
#include "kvdb_manager.h"
#include "sqlite_local_kvdb_connection.h"
#include "sqlite_single_ver_forward_cursor.h"
#include "platform_specific.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    string g_identifier;
    IKvDBRawCursor *g_rawCursor = nullptr;
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    KvStoreConfig g_config;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    SQLiteSingleVerNaturalStore *g_store = nullptr;
    DistributedDB::SQLiteSingleVerNaturalStoreConnection *g_connection = nullptr;
    const string STORE_ID = STORE_ID_SYNC;
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));
    const int TIME_LAG = 100;
    const int INITIAL_POSITION = 0;
    const int SECOND_POSITION = 1;
    const int TOTAL_COUNT = 3;
    const Key KEY_PREFIX = {'K'};
    const Key LOCAL_KEY_1 = {'K', '1'};
    const Key LOCAL_KEY_2 = {'K', '2'};
    const Key LOCAL_KEY_3 = {'K', '3'};
    const Key LOCAL_KEY_4 = {'K', '4'};
}

class DistributedDBSingleVersionResultSetTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBSingleVersionResultSetTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
    std::string origIdentifier = USER_ID + "-" + APP_ID + "-" + STORE_ID;
    std::string identifier = DBCommon::TransferHashString(origIdentifier);
    g_identifier = DBCommon::TransferStringToHex(identifier);
    string dir = g_testDir + g_identifier + "/" + DBConstant::SINGLE_SUB_DIR;
    DIR *dirTmp = opendir(dir.c_str());
    if (dirTmp == nullptr) {
        OS::MakeDBDirectory(dir);
    } else {
        closedir(dirTmp);
    }
}

void DistributedDBSingleVersionResultSetTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir + STORE_ID + "/" + DBConstant::SINGLE_SUB_DIR) != 0) {
        LOGE("rm test db files error!");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
}

void DistributedDBSingleVersionResultSetTest::SetUp(void)
{
    KvStoreNbDelegate::Option delegateOption = {true};
    g_mgr.GetKvStore(STORE_ID, delegateOption, g_kvNbDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);

    KvDBProperties property;
    property.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    property.SetStringProp(KvDBProperties::STORE_ID, STORE_ID);
    property.SetStringProp(KvDBProperties::IDENTIFIER_DIR, g_identifier);
    property.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);

    g_store = new (std::nothrow) SQLiteSingleVerNaturalStore;
    ASSERT_NE(g_store, nullptr);
    ASSERT_EQ(g_store->Open(property), E_OK);

    int errCode = E_OK;
    g_connection = static_cast<SQLiteSingleVerNaturalStoreConnection *>(g_store->GetDBConnection(errCode));
    ASSERT_NE(g_connection, nullptr);
    g_store->DecObjRef(g_store);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps:step1. Put 3 data items.
     * @tc.expected: step1.
     */
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    g_connection->Clear(option);
    ASSERT_EQ(g_connection->Put(option, LOCAL_KEY_1, VALUE_1), OK);
    ASSERT_EQ(g_connection->Put(option, LOCAL_KEY_2, VALUE_2), OK);
    ASSERT_EQ(g_connection->Put(option, LOCAL_KEY_3, VALUE_3), OK);

    EXPECT_EQ(errCode, E_OK);
    g_rawCursor = new (std::nothrow) SQLiteSingleVerForwardCursor(g_store, KEY_PREFIX);
    ASSERT_NE(g_rawCursor, nullptr);
}

void DistributedDBSingleVersionResultSetTest::TearDown(void)
{
    if (g_connection != nullptr) {
        g_connection->Close();
        g_connection = nullptr;
    }

    g_store = nullptr;

    if (g_kvNbDelegatePtr != nullptr) {
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
        g_kvNbDelegatePtr = nullptr;
        EXPECT_TRUE(g_mgr.DeleteKvStore(STORE_ID) == OK);
    }

    if (g_rawCursor != nullptr) {
        delete g_rawCursor;
        g_rawCursor = nullptr;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_LAG));
}

/**
  * @tc.name: SingleVersionResultSetTest001
  * @tc.desc: CursorWindow Class: Return error when the window size too large.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be INT_MAX, which is larger than the upper limit.
     * @tc.expected: step1. Expect return -E_INVALID_ARGS.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int64_t windoweSize = 0x100000000L; // 4G
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windoweSize, scale), -E_INVALID_ARGS);
}

/**
  * @tc.name: SingleVersionResultSetTest002
  * @tc.desc: CursorWindow Class: Return error when the window size is negative.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be -1.
     * @tc.expected: step1. Expect return -E_INVALID_ARGS.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = -1;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), -E_INVALID_ARGS);
}

/**
  * @tc.name: SingleVersionResultSetTest003
  * @tc.desc: CursorWindow Class: Return error when the window size is zero.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 0.
     * @tc.expected: step1. Expect return -E_INVALID_ARGS.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = 0;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), -E_INVALID_ARGS);
}

/**
  * @tc.name: SingleVersionResultSetTest004
  * @tc.desc: CursorWindow Class: Return OK when the window size is positive.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, which is smaller than the upper limit.
     * @tc.expected: step1. Expect return OK.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), E_OK);
}

/**
  * @tc.name: SingleVersionResultSetTest005
  * @tc.desc: CursorWindow Class: Return -E_INVALID_ARGS when the window scale is negative.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest005, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be negative (-1).
     * @tc.expected: step1. Expect return -E_INVALID_ARGS.
     */
    ResultEntriesWindow resultWindow;
    double scale = -1;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), -E_INVALID_ARGS);
}

/**
  * @tc.name: SingleVersionResultSetTest006
  * @tc.desc: CursorWindow Class: Return -E_INVALID_ARGS when the window scale is larger than 1.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest006, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 2.
     * @tc.expected: step1. Expect return -E_INVALID_ARGS.
     */
    ResultEntriesWindow resultWindow;
    double scale = 2;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), -E_INVALID_ARGS);
}

/**
  * @tc.name: SingleVersionResultSetTest007
  * @tc.desc: CursorWindow Class: Return -E_INVALID_ARGS when the window scale 0.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest007, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 0.
     * @tc.expected: step1. Expect return -E_INVALID_ARGS.
     */
    ResultEntriesWindow resultWindow;
    double scale = 0;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), -E_INVALID_ARGS);
}

/**
  * @tc.name: SingleVersionResultSetTest008
  * @tc.desc: CursorWindow Class: Return OK when the window scale is between 0 and 1.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest008, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 0.5.
     * @tc.expected: step1. Expect return OK.
     */
    ResultEntriesWindow resultWindow;
    double scale = 0.5;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), E_OK);
}

/**
  * @tc.name: SingleVersionResultSetTest009
  * @tc.desc: CursorWindow Class: Return -E_INVALID_ARGS when the g_rawCursor is nulSSSlptr.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest009, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 1 and resultWindow be null pointer.
     * @tc.expected: step1. Expect return -E_INVALID_ARGS.
     */
    IKvDBRawCursor *rawCursor = nullptr;
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(rawCursor, windowSize, scale), -E_INVALID_ARGS);
}

/**
  * @tc.name: SingleVersionResultSetTest010
  * @tc.desc: CursorWindow Class: Check if get total count is feasible.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest010, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 1 and resultWindow be null pointer.
     * @tc.expected: step1. Expect return OK.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), E_OK);

    /**
     * @tc.steps:step2. Get the total count.
     * @tc.expected: step2. Expect return 3.
     */
    EXPECT_EQ(resultWindow.GetTotalCount(), TOTAL_COUNT);
}

/**
  * @tc.name: SingleVersionResultSetTest011
  * @tc.desc: CursorWindow Class: Check if get total count is feasible and the inserted items after
  *           creating ResultEntriesWindow have not been counted.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest011, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 1 and resultWindow be null pointer.
     * @tc.expected: step1. Expect return OK.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), E_OK);

    /**
     * @tc.steps:step2. Get the total count.
     * @tc.expected: step2. Expect return 3.
     */
    EXPECT_EQ(resultWindow.GetTotalCount(), TOTAL_COUNT);

    /**
     * @tc.steps:step3. Put one more item
     * @tc.expected: step3.
     */
    IOption option;
    option.dataType = IOption::SYNC_DATA;
    ASSERT_EQ(g_connection->Put(option, LOCAL_KEY_4, VALUE_4), OK);

    /**
     * @tc.steps:step4. Get the total count.
     * @tc.expected: step4. Expect return 3.
     */
    EXPECT_EQ(resultWindow.GetTotalCount(), TOTAL_COUNT);
}

/**
  * @tc.name: SingleVersionResultSetTest012
  * @tc.desc: CursorWindow Class: Check if current position after initialization is at 0.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest012, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 1 and resultWindow be null pointer.
     * @tc.expected: step1. Expect return OK.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), E_OK);

    /**
     * @tc.steps:step2. Get initial position.
     * @tc.expected: step2. Expect return INITIAL_POSITION (which is 0).
     */
    EXPECT_EQ(resultWindow.GetCurrentPosition(), INITIAL_POSITION);

    /**
     * @tc.steps:step3. Get entry .
     * @tc.expected: step3. Expect return E_OK.
     */
    Entry entry;
    EXPECT_EQ(resultWindow.GetEntry(entry), E_OK);
    EXPECT_EQ(entry.key, LOCAL_KEY_1);
    EXPECT_EQ(entry.value, VALUE_1);
}

/**
  * @tc.name: SingleVersionResultSetTest013
  * @tc.desc: CursorWindow Class: Check if current position after move is at the right place+.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest013, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 1 and resultWindow be null pointer.
     * @tc.expected: step1. Expect return OK.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), E_OK);

    /**
     * @tc.steps:step2. move to second position.
     * @tc.expected: step2. Expect return SECOND_POSITION (which is 2).
     */
    EXPECT_EQ(resultWindow.MoveToPosition(SECOND_POSITION), true);
    EXPECT_EQ(resultWindow.GetCurrentPosition(), SECOND_POSITION);

    /**
     * @tc.steps:step3. Get entry .
     * @tc.expected: step3. Expect return OK and entry corresponds to the right item.
     */
    Entry entry;
    EXPECT_EQ(resultWindow.GetEntry(entry), E_OK);
    EXPECT_EQ(entry.key, LOCAL_KEY_2);
    EXPECT_EQ(entry.value, VALUE_2);
}

/**
  * @tc.name: SingleVersionResultSetTest014
  * @tc.desc: CursorWindow Class: Move to negative position and the position bounces back to zero.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest014, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 1 and resultWindow be null pointer.
     * @tc.expected: step1. Expect return OK.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), E_OK);

    /**
     * @tc.steps:step2. move to second position.
     * @tc.expected: step2. Expect return false and initial position.
     */
    int negativePosition = -2;
    EXPECT_EQ(resultWindow.MoveToPosition(negativePosition), false);
    EXPECT_EQ(resultWindow.GetCurrentPosition(), INITIAL_POSITION);

    /**
     * @tc.steps:step3. Get entry .
     * @tc.expected: step3. Expect return E_OK.
     */
    Entry entry;
    EXPECT_EQ(resultWindow.GetEntry(entry), E_OK);
    EXPECT_EQ(entry.key, LOCAL_KEY_1);
    EXPECT_EQ(entry.value, VALUE_1);
}

/**
  * @tc.name: SingleVersionResultSetTest015
  * @tc.desc: CursorWindow Class: Move to position larger than N and the position bounces back to original position.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: maokeheng
  */
HWTEST_F(DistributedDBSingleVersionResultSetTest, SingleVersionResultSetTest015, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Let the WindowSize be 100, and window scale to be 1 and resultWindow be null pointer.
     * @tc.expected: step1. Expect return OK.
     */
    ResultEntriesWindow resultWindow;
    double scale = 1;
    int windowSize = 100;
    EXPECT_EQ(resultWindow.Init(g_rawCursor, windowSize, scale), E_OK);

    /**
     * @tc.steps:step2. move to second position.
     * @tc.expected: step2. Expect return false and move to total count.
     */
    int largePosition = TOTAL_COUNT + 1;
    EXPECT_EQ(resultWindow.MoveToPosition(largePosition), false);
    EXPECT_EQ(resultWindow.GetCurrentPosition(), INITIAL_POSITION);

    /**
     * @tc.steps:step3. Get entry .
     * @tc.expected: step3. Expect return VALUE_1.
     */
    Entry entry;
    EXPECT_EQ(resultWindow.GetEntry(entry), E_OK);
    EXPECT_EQ(entry.key, LOCAL_KEY_1);
    EXPECT_EQ(entry.value, VALUE_1);
}