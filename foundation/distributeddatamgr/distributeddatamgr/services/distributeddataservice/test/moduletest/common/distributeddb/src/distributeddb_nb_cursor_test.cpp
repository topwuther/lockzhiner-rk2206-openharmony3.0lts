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
#include "distributeddb_nb_cursor_testcase.h"
#include "distributeddb_nb_test_tools.h"
#include "kv_store_delegate.h"
#include "kv_store_nb_delegate.h"
#include "kv_store_delegate_manager.h"

using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace std;
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;
namespace DistributeddbNbCursor {
KvStoreNbDelegate *g_nbCursorDelegate = nullptr;
KvStoreDelegateManager *g_manager = nullptr;
class DistributeddbNbCursorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbCursorTest::SetUpTestCase(void)
{
}

void DistributeddbNbCursorTest::TearDownTestCase(void)
{
}

void DistributeddbNbCursorTest::SetUp(void)
{
    RemoveDir(NB_DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    g_nbCursorDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, g_option);
    ASSERT_TRUE(g_manager != nullptr && g_nbCursorDelegate != nullptr);
}

void DistributeddbNbCursorTest::TearDown(void)
{
    MST_LOG("TearDownTestCase after case.");
    ASSERT_NE(g_manager, nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(g_manager, g_nbCursorDelegate, STORE_ID_1, g_option.isMemoryDb));
    RemoveDir(NB_DIRECTOR);
}

/*
 * @tc.name: ResultSetDb 001
 * @tc.desc: test GetCount, GetPosition, MoveToFirst, MoveToLast, IsFirst, IsLast, IsBeforeFirst and
 *    IsAfterLast interfaces with little data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb001, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb001(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 002
 * @tc.desc: test MoveToNext, MoveToPrevious interfaces with little data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb002, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb002(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 003
 * @tc.desc: test Move interface with little data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb003, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb003(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 004
 * @tc.desc: test MoveToPostions interfaces with little data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb004, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb004(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 005
 * @tc.desc: test GetCount, GetPosition, MoveToFirst, MoveToLast, IsFirst, IsLast, IsBeforeFirst and
 *    IsAfterLast interfaces with single 4M data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb005, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb005(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 006
 * @tc.desc: test MoveToNext, MoveToPrevious interface with single 4M data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb006, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb006(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 007
 * @tc.desc: test Move interface with single 4M data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb007, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb007(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 008
 * @tc.desc: test MoveToPosition interface with single 4M data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb008, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb008(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 009
 * @tc.desc: test GetCount, GetPosition, MoveToFirst, MoveToLast, IsFirst, IsLast, IsBeforeFirst and
 *    IsAfterLast interfaces with 100 100k data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb009, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb009(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 010
 * @tc.desc: test MoveToNext, MoveToPrevious interface with 100 100k data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb010, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb010(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 011
 * @tc.desc: test Move interface with 100 100k data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb011, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb011(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 012
 * @tc.desc: test MoveToPosition interface with 100 100k data.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb012, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb012(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 013
 * @tc.desc: test MoveToNext and Move interface can be called one record by one record.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb013, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb013(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 014
 * @tc.desc: test GetEntries that was not exist in db and if IsFirst, IsLast, MoveToFirst, MoveToLast, IsBeforeFirst
 *    IsAfterLast, and MoveToPrevious, MoveToNext interface can return rightly.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb014, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb014(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 015
 * @tc.desc: test GetEntries that was not exist in db and if Move interface can return rightly.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb015, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb015(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 016
 * @tc.desc: test GetEntries that was not exist in db and if MoveToPosition interface can return rightly.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb016, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb016(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 017
 * @tc.desc: call CloseResultSet with nullptr returns INVALID_ARGS .
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb017, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb017(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 018
 * @tc.desc: the count of remainder recordset don't change after close one.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb018, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb018(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 019
 * @tc.desc: append, delete, modify operation can change the count of recordset
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb019, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb019(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 020
 * @tc.desc: GetEntries returns BUSY when the count of recordsets is equal or more than 5.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb020, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb020(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 021
 * @tc.desc: CloseKvStore returns BUSY before close all resultSet(s).
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb021, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb021(g_nbCursorDelegate, g_manager, false);
}

#ifndef LOW_LEVEL_MEM_DEV
/*
 * @tc.name: ResultSetDb 022
 * @tc.desc: rekey returns BUSY when the size of recordset is larger than 4M
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb022, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb022(false);
}
#endif

/*
 * @tc.name: ResultSetDb 023
 * @tc.desc: rekey returns OK when the size of recordset is equal to 4M
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb023, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb023(false);
}

#ifndef LOW_LEVEL_MEM_DEV
/*
 * @tc.name: ResultSetDb 024
 * @tc.desc: GetEntries returns BUSY when Rekeying
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb024, TestSize.Level3)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb024(false);
}
#endif

/*
 * @tc.name: ResultSetDb 025
 * @tc.desc: Verify that multiple threads can call GetEntries() to get resultSet at the same time.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb025, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb025(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 026
 * @tc.desc: Verify that multiple threads can call GetEntries() to get the same resultSet at the same time.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb026, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb026(g_nbCursorDelegate, false);
}

/*
 * @tc.name: ResultSetDb 027
 * @tc.desc: the interfaces of different delegates opened by the same db doesn't affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000D08KS
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, ResultSetDb027, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_FULL_ENTRY.
    DistributedNbCursorTestcase::ResultSetDb027(false);
}

/*
 * @tc.name: CacheRowIdTest 001
 * @tc.desc: test GetCount, GetPosition, MoveToFirst, MoveToLast, IsFirst, IsLast, IsBeforeFirst and
 *    IsAfterLast interfaces with little data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest001, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb001(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 002
 * @tc.desc: test MoveToNext, MoveToPrevious interfaces with little data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest002, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb002(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 003
 * @tc.desc: test Move interface with little data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest003, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb003(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 004
 * @tc.desc: test MoveToPostions interfaces with little data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest004, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb004(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 005
 * @tc.desc: test GetCount, GetPosition, MoveToFirst, MoveToLast, IsFirst, IsLast, IsBeforeFirst and
 *    IsAfterLast interfaces with single 4M data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest005, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb005(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 006
 * @tc.desc: test MoveToNext, MoveToPrevious interface with single 4M data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest006, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb006(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 007
 * @tc.desc: test Move interface with single 4M data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest007, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb007(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 008
 * @tc.desc: test MoveToPosition interface with single 4M data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest008, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb008(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 009
 * @tc.desc: test GetCount, GetPosition, MoveToFirst, MoveToLast, IsFirst, IsLast, IsBeforeFirst and
 *    IsAfterLast interfaces with 100 100k data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest009, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb009(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 010
 * @tc.desc: test MoveToNext, MoveToPrevious interface with 100 100k data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest010, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb010(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 011
 * @tc.desc: test Move interface with 100 100k data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest011, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb011(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 012
 * @tc.desc: test MoveToPosition interface with 100 100k data.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest012, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb012(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 013
 * @tc.desc: test MoveToNext and Move interface can be called one record by one record.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest013, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb013(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 014
 * @tc.desc: test GetEntries that was not exist in db and if IsFirst, IsLast, MoveToFirst, MoveToLast, IsBeforeFirst
 *    IsAfterLast, and MoveToPrevious, MoveToNext interface can return rightly.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest014, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb014(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 015
 * @tc.desc: test GetEntries that was not exist in db and if Move interface can return rightly.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest015, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb015(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 016
 * @tc.desc: test GetEntries that was not exist in db and if MoveToPosition interface can return rightly.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest016, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb016(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 017
 * @tc.desc: call CloseResultSet with nullptr returns INVALID_ARGS .
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest017, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb017(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 018
 * @tc.desc: the count of remainder recordset don't change after close one.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest018, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb018(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 019
 * @tc.desc: append, delete, modify operation can change the count of recordset
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest019, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb019(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 020
 * @tc.desc: GetEntries returns BUSY when the count of recordsets is equal or more than 5.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest020, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb020(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 021
 * @tc.desc: CloseKvStore returns BUSY before close all resultSet(s).
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest021, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb021(g_nbCursorDelegate, g_manager, true);
}

#ifndef LOW_LEVEL_MEM_DEV
/*
 * @tc.name: CacheRowIdTest 022
 * @tc.desc: rekey returns BUSY when the size of recordset is larger than 4M
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest022, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb022(true);
}
#endif

/*
 * @tc.name: CacheRowIdTest 023
 * @tc.desc: rekey returns OK when the size of recordset is equal to 4M
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest023, TestSize.Level1)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb023(true);
}

#ifndef LOW_LEVEL_MEM_DEV
/*
 * @tc.name: CacheRowIdTest 024
 * @tc.desc: GetEntries returns BUSY when Rekeying
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest024, TestSize.Level3)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb024(true);
}
#endif

/*
 * @tc.name: CacheRowIdTest 025
 * @tc.desc: Verify that multiple threads can call GetEntries() to get resultSet at the same time.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest025, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb025(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 026
 * @tc.desc: Verify that multiple threads can call GetEntries() to get the same resultSet at the same time.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest026, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb026(g_nbCursorDelegate, true);
}

/*
 * @tc.name: CacheRowIdTest 027
 * @tc.desc: the interfaces of different delegates opened by the same db doesn't affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCursorTest, CacheRowIdTest027, TestSize.Level2)
{
    // test with RESULT_SET_CACHE_MODE = CACHE_ENTRY_ID_ONLY.
    DistributedNbCursorTestcase::ResultSetDb027(true);
}
}