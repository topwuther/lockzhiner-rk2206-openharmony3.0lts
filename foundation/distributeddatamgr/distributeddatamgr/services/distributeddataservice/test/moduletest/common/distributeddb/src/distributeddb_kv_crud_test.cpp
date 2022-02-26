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
#include <ctime>
#include <cmath>
#include <random>
#include <chrono>
#include <string>

#include "distributeddb_data_generator.h"
#include "distributed_test_tools.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "types.h"

using namespace std;
using namespace chrono;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbKvCrud {
class DistributeddbKvCrudTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

const bool IS_LOCAL = GetRandBool();
KvStoreDelegate *g_kvStoreDelegate = nullptr; // the delegate used in this suit.
KvStoreDelegateManager *g_manager = nullptr;
void DistributeddbKvCrudTest::SetUpTestCase(void)
{
}

void DistributeddbKvCrudTest::TearDownTestCase(void)
{
}

void DistributeddbKvCrudTest::SetUp(void)
{
    MST_LOG("SetUp TestCase before every case local[%d].", IS_LOCAL);
    RemoveDir(DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    KvOption option = g_kvOption;
    option.localOnly = IS_LOCAL;
    g_kvStoreDelegate = DistributedTestTools::GetDelegateSuccess(g_manager, g_kvdbParameter1, option);
    ASSERT_TRUE(g_manager != nullptr && g_kvStoreDelegate != nullptr);
}

void DistributeddbKvCrudTest::TearDown(void)
{
    MST_LOG("TearDownTestCase after all cases.");
    EXPECT_TRUE(g_manager->CloseKvStore(g_kvStoreDelegate) == OK);
    g_kvStoreDelegate = nullptr;
    DBStatus status = g_manager->DeleteKvStore(STORE_ID_1);
    EXPECT_TRUE(status == DistributedDB::DBStatus::OK) << "fail to delete exist kvdb";
    delete g_manager;
    g_manager = nullptr;
    RemoveDir(DIRECTOR);
}

/*
 * @tc.name: SimpleDataTest 001
 * @tc.desc: Verify that can put value to db and get successfully.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, SimpleDataTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create kv db and put (k1,v1) then get the value of k1 from db.
     * @tc.expected: step1. put successfully and get the value of k1 is v1.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_1, VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));

    /**
     * @tc.steps: step2. create kv db and put (k2,v2) then get the value of k2 from db.
     * @tc.expected: step2. put successfully and get the value of k2 is v2.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_2, VALUE_2);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult2 = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_2);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult2, VALUE_2));
}

/*
 * @tc.name: SimpleDataTest 002
 * @tc.desc: Verify that can delete value and get successfully.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, SimpleDataTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create kv db and put (k1,v1) then get the value of k1 from db.
     * @tc.expected: step1. put successfully and get the value of k1 is v1.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_1, VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);

    /**
     * @tc.steps: step2. delete k1 from db.
     * @tc.expected: step2. delete successfully.
     */
    status = DistributedTestTools::Delete(*g_kvStoreDelegate, KEY_1);
    ASSERT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step3. get k1 from db.
     * @tc.expected: step3. get successfully but value is null.
     */
    Value valueResult2 = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
    EXPECT_TRUE(valueResult2.size() == 0);
}

/*
 * @tc.name: SimpleDataTest 003
 * @tc.desc: Verify that can put the same key with different value and get successfully.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, SimpleDataTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create kv db and put (k1,v1) then get the value of k1 from db.
     * @tc.expected: step1. put successfully and get the value of k1 is v1.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_1, VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));

    /**
     * @tc.steps: step2. create kv db and put (k1,v2) then get the value of k1 from db.
     * @tc.expected: step2. put successfully and get the value of k1 is v2.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_1, VALUE_2);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));

    /**
     * @tc.steps: step3. create kv db and put (k1,v1) then get the value of k1 from db.
     * @tc.expected: step3. put successfully and get the value of k1 is v1.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_1, VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
}

/*
 * @tc.name: SimpleDataTest 004
 * @tc.desc: Verify that put a null value to db will get nothing.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, SimpleDataTest004, TestSize.Level1)
{
    DBStatus status = DistributedTestTools::Delete(*g_kvStoreDelegate, KEY_1);
    ASSERT_EQ(status, DBStatus::OK);

    /**
     * @tc.steps: step1. create kv db and put (k1,null).
     * @tc.expected: step1. put ok.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_1, VALUE_EMPTY);
    ASSERT_EQ(status, DBStatus::OK);
    /**
     * @tc.steps: step2. get the value of k1 from db.
     * @tc.expected: step2. get successfully and get the value of k1 is null.
     */
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: SimpleDataTest 005
 * @tc.desc: Verify that can not put a null key to db.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, SimpleDataTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create kv db and put (null,v1).
     * @tc.expected: step1. put failed.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_EMPTY, VALUE_1);
    ASSERT_TRUE(status == DBStatus::INVALID_ARGS);
    /**
     * @tc.steps: step2. get k=null from db.
     * @tc.expected: step2. get value of k is null.
     */
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_EMPTY);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: SimpleDataTest 006
 * @tc.desc: Verify that can delete the entry that not exist.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, SimpleDataTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. delete k1 from db.
     * @tc.expected: step1. Construct that no k1 exist in db.
     */
    DBStatus status = DistributedTestTools::Delete(*g_kvStoreDelegate, KEY_1);
    ASSERT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. get k1 from db.
     * @tc.expected: step2. get the value of k1 is null.
     */
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: SimpleDataTest 007
 * @tc.desc: Verify that delete absent key return OK.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, SimpleDataTest007, TestSize.Level0)
{
    /**
     * @tc.steps: step1. delete k1 from db.
     * @tc.expected: step1. Construct that no k1 exist in db.
     */
    DBStatus status = DistributedTestTools::Delete(*g_kvStoreDelegate, KEY_1);
    ASSERT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. delete nonexistent k1 from db.
     * @tc.expected: step2. delete failed but return OK.
     */
    status = DistributedTestTools::Delete(*g_kvStoreDelegate, KEY_1);
    ASSERT_TRUE(status == DBStatus::OK);
}

/*
 * @tc.name: SimpleDataTest 008
 * @tc.desc: Verify that can the Clear interface can delete all the records one time.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, SimpleDataTest008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create kv db and put (k1,v1)(k1,v2).
     * @tc.expected: step1. put successfully.cd ..
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_1, VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_2, VALUE_2);
    ASSERT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. delete all k from db then get k1 from db.
     * @tc.expected: step2. delete successfully and get the value of k1 if null.
     */
    status = DistributedTestTools::Clear(*g_kvStoreDelegate);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: SimpleDataTest 009
 * @tc.desc: Verify that can crud the same key continuously.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, SimpleDataTest009, TestSize.Level1)
{
    for (int cnt = 0; cnt < FIVE_TIMES; cnt++) {
        /**
         * @tc.steps: step1. create kv db and put (k1,v1) and check the records.
         * @tc.expected: step1. put successfully and the value of the k1 in db is v1.
         */
        DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_1, VALUE_1);
        ASSERT_TRUE(status == DBStatus::OK);
        Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
        EXPECT_TRUE(valueResult.size() != 0);
        EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
        /**
         * @tc.steps: step2. put (k1,v2) to db and get.
         * @tc.expected: step2. put successfully and get the value of k1 is v2.
         */
        status = DistributedTestTools::Put(*g_kvStoreDelegate, KEY_1, VALUE_2);
        ASSERT_TRUE(status == DBStatus::OK);
        valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
        EXPECT_TRUE(valueResult.size() != 0);
        EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));
        /**
         * @tc.steps: step3. delete k1 from db and get.
         * @tc.expected: step3. delete successfully and get the value of k1 is null.
         */
        status = DistributedTestTools::Delete(*g_kvStoreDelegate, KEY_1);
        ASSERT_TRUE(status == DBStatus::OK);
        valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, KEY_1);
        EXPECT_TRUE(valueResult.size() == 0);
    }
}

/*
 * @tc.name: ComplexDataTest 001
 * @tc.desc: Verify that can operate a long string key and key's length can not bigger than 1K.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, ComplexDataTest001, TestSize.Level0)
{
    DistributedDB::Key longKey1, longKey2;
    longKey1.assign(ONE_K_LONG_STRING, (uint8_t)'a');
    longKey2.assign(ONE_K_LONG_STRING + 1, (uint8_t)'b'); // 1 Byte bigger than 1024Byte
    /**
     * @tc.steps: step1. create kv db and put (longKey1,null) which the length of longKey1 is 1k and get.
     * @tc.expected: step1. put successfully and get the value of longKey1 is null.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, longKey1, VALUE_EMPTY);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, longKey1);
    EXPECT_TRUE(valueResult.size() == 0);
    /**
     * @tc.steps: step2. create kv db and put (longKey2,null) which the length of longKey2 is 1k+1 and get.
     * @tc.expected: step2. put successfully and get the value of longKey2 is null.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, longKey2, VALUE_EMPTY);
    ASSERT_TRUE(status != DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, longKey2);
    EXPECT_TRUE(valueResult.size() == 0);

    /**
     * @tc.steps: step3. create kv db and put (longKey1,ok_v1) which the length of longKey1 is 1k and get.
     * @tc.expected: step3. put successfully and get the size of longKey1.value is 1k.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, longKey1, OK_VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, longKey1);
    EXPECT_TRUE(valueResult.size() == OK_VALUE_1.size());
    /**
     * @tc.steps: step4. create kv db and put (longKey2,ok_v1) which the length of longKey2 is 1k+1 and get.
     * @tc.expected: step4. put failed and get the value of longKey2 is null.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, longKey2, OK_VALUE_1);
    ASSERT_TRUE(status != DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, longKey2);
    EXPECT_TRUE(valueResult.size() == 0);

    /**
     * @tc.steps: step5. delete longKey1 from db and get.
     * @tc.expected: step5. delete successfully and get the value of longKey1 is null.
     */
    status = DistributedTestTools::Delete(*g_kvStoreDelegate, longKey1);
    ASSERT_EQ(status, DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, longKey1);
    EXPECT_TRUE(valueResult.size() == 0);
    /**
     * @tc.steps: step6. delete longKey2 from db and get.
     * @tc.expected: step6. delete failed and get the value of longKey2 is null.
     */
    status = DistributedTestTools::Delete(*g_kvStoreDelegate, longKey2);
    ASSERT_NE(status, DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, longKey2);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: ComplexDataTest 002
 * @tc.desc: Verify that can operate a long string value and value's length can not bigger than 4M.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, ComplexDataTest002, TestSize.Level1)
{
    DistributedDB::Value longValue1, longValue2;
    longValue1.assign(FOUR_M_LONG_STRING, (uint8_t)'a');
    longValue2.assign(FOUR_M_LONG_STRING + 1, (uint8_t)'b');

    /**
     * @tc.steps: step1. create kv db and put (OK_K1,longValue1) which the length of longValue1 is 4M and get.
     * @tc.expected: step1. put successfully and get the value of OK_K1 is longValue1.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, OK_KEY_1, longValue1);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, OK_KEY_1);
    EXPECT_TRUE(valueResult.size() == FOUR_M_LONG_STRING);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, longValue1));

    /**
     * @tc.steps: step2. create kv db and put (OK_K1,longValue2) which the length of longValue2 is 4M+1 and get.
     * @tc.expected: step2. put failed and get the size of longValue2 is 4M.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, OK_KEY_1, longValue2);
    ASSERT_TRUE(status != DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, OK_KEY_1);
    EXPECT_TRUE(valueResult.size() == FOUR_M_LONG_STRING);

    /**
     * @tc.steps: step3. delete OK_KEY_1 and get.
     * @tc.expected: step3. delete successfully and get the value of OK_KEY_1 is null.
     */
    status = DistributedTestTools::Delete(*g_kvStoreDelegate, OK_KEY_1);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, OK_KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: ComplexDataTest 003
 * @tc.desc: Verify that can operate a alphanum string key.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, ComplexDataTest003, TestSize.Level1)
{
    DistributedDB::Key alphanumKey1;
    char chr;
    for (chr = 'a'; chr <= 'z'; ++chr) {
        alphanumKey1.push_back(chr);
    }
    for (chr = 'A'; chr <= 'Z'; ++chr) {
        alphanumKey1.push_back(chr);
    }
    for (chr = '0'; chr <= '9'; ++chr) {
        alphanumKey1.push_back(chr);
    }

    /**
     * @tc.steps: step1. create kv db and put (alphanumKey1,null) that alphanumKey1=[a-zA-Z0-9].
     * @tc.expected: step1. put successfully and get the value of alphanumKey1 is null.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, alphanumKey1, VALUE_EMPTY);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, alphanumKey1);
    EXPECT_TRUE(valueResult.size() == 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_EMPTY));

    /**
     * @tc.steps: step2. create kv db and put (alphanumKey1,OK_VALUE_1) that alphanumKey1=[a-zA-Z0-9].
     * @tc.expected: step2. put successfully and get the value of alphanumKey1 is OK_VALUE_1.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, alphanumKey1, OK_VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, alphanumKey1);
    EXPECT_TRUE(valueResult.size() == OK_VALUE_1.size());
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, OK_VALUE_1));

    /**
     * @tc.steps: step3. delete alphanumKey1 from db and get.
     * @tc.expected: step3. delete successfully and get the value of alphanumKey1 is null.
     */
    status = DistributedTestTools::Delete(*g_kvStoreDelegate, alphanumKey1);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, alphanumKey1);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: ComplexDataTest 004
 * @tc.desc: Verify that can operate a alphanum string value.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, ComplexDataTest004, TestSize.Level1)
{
    DistributedDB::Value alphanumValue1, alphanumValue2;
    int chr;
    for (chr = 'a'; chr <= 'z'; ++chr) {
        alphanumValue1.push_back(chr);
        alphanumValue2.push_back('a' + 'z' - chr);
    }
    for (chr = 'A'; chr <= 'Z'; ++chr) {
        alphanumValue1.push_back(chr);
        alphanumValue2.push_back('A' + 'Z' - chr);
    }
    for (chr = '0'; chr <= '9'; ++chr) {
        alphanumValue1.push_back(chr);
        alphanumValue2.push_back('0' + '9' - chr);
    }

    /**
     * @tc.steps: step1. create kv db and put (OK_KEY_1,alphanumValue1) that alphanumValue1=[a-zA-Z0-9].
     * @tc.expected: step1. put successfully and get the value of OK_KEY_1 is alphanumValue1.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, OK_KEY_1, alphanumValue1);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, OK_KEY_1);
    EXPECT_TRUE(valueResult.size() == alphanumValue1.size());
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, alphanumValue1));

    /**
     * @tc.steps: step2. create kv db and put (OK_KEY_1,alphanumValue2) that alphanumValue1=[z-aZ-A9-0].
     * @tc.expected: step2. put successfully and get the value of OK_KEY_1 is alphanumValue2.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, OK_KEY_1, alphanumValue2);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, OK_KEY_1);
    EXPECT_TRUE(valueResult.size() == alphanumValue2.size());
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, alphanumValue2));

    /**
     * @tc.steps: step3. delete OK_KEY_1 from db and get.
     * @tc.expected: step3. delete successfully and get the value of OK_KEY_1 is null.
     */
    status = DistributedTestTools::Delete(*g_kvStoreDelegate, OK_KEY_1);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, OK_KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: ComplexDataTest 005
 * @tc.desc: Verify that can operate a full ASCII set string key.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, ComplexDataTest005, TestSize.Level1)
{
    DistributedDB::Key fullAsciiKey1;
    for (int cVal = 0; cVal <= 255; ++cVal) {
        fullAsciiKey1.push_back(cVal);
    }

    /**
     * @tc.steps: step1. create kv db and put (fullAsciiKey1,null) that fullAsciiKey1=[\0-\255].
     * @tc.expected: step1. put successfully and get the value of fullAsciiKey1 is null.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, fullAsciiKey1, VALUE_EMPTY);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, fullAsciiKey1);
    EXPECT_TRUE(valueResult.size() == 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_EMPTY));

    /**
     * @tc.steps: step2. create kv db and put (fullAsciiKey1,OK_VALUE_1) that fullAsciiKey1=[\0-\255].
     * @tc.expected: step2. put successfully and get the value of fullAsciiKey1 is OK_VALUE_1.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, fullAsciiKey1, OK_VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, fullAsciiKey1);
    EXPECT_TRUE(valueResult.size() == OK_VALUE_1.size());
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, OK_VALUE_1));

    /**
     * @tc.steps: step3. delete fullAsciiKey1 from db and get.
     * @tc.expected: step3. delete successfully and get the value of fullAsciiKey1 is null.
     */
    status = DistributedTestTools::Delete(*g_kvStoreDelegate, fullAsciiKey1);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, fullAsciiKey1);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: ComplexDataTest 006
 * @tc.desc: Verify that can operate a full ASCII set string value.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, ComplexDataTest006, TestSize.Level1)
{
    DistributedDB::Value fullAsciiValue1, fullAsciiValue2;
    for (int cVal = 0; cVal <= 255; ++cVal) {
        fullAsciiValue1.push_back(cVal);
        fullAsciiValue2.push_back(255 - cVal);
    }

    /**
     * @tc.steps: step1. create kv db and put (OK_KEY_1,fullAsciiValue1) that fullAsciiValue1=[\0-\255].
     * @tc.expected: step1. put successfully and get the value of OK_KEY_1 is fullAsciiValue1.
     */
    DBStatus status = DistributedTestTools::Put(*g_kvStoreDelegate, OK_KEY_1, fullAsciiValue1);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, OK_KEY_1);
    EXPECT_TRUE(valueResult.size() == fullAsciiValue1.size());
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, fullAsciiValue1));

    /**
     * @tc.steps: step2. create kv db and put (OK_KEY_1,fullAsciiValue2) that fullAsciiValue2=[\255-\0].
     * @tc.expected: step2. put successfully and get the value of OK_KEY_1 is fullAsciiValue2.
     */
    status = DistributedTestTools::Put(*g_kvStoreDelegate, OK_KEY_1, fullAsciiValue2);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, OK_KEY_1);
    EXPECT_TRUE(valueResult.size() == fullAsciiValue2.size());
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, fullAsciiValue2));

    /**
     * @tc.steps: step3. delete OK_KEY_1 from db and get.
     * @tc.expected: step3. delete successfully and get the value of OK_KEY_1 is null.
     */
    status = DistributedTestTools::Delete(*g_kvStoreDelegate, OK_KEY_1);
    ASSERT_TRUE(status == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_kvStoreDelegate, OK_KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: ComplexDataTest 007
 * @tc.desc: Verify that can operate chinese string key and value.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, ComplexDataTest007, TestSize.Level1)
{
    /*
     * @tc.steps: step1. create kv db and put(k,OK_VALUE_1) that k="中文"and get.
     * @tc.expected: step1. put successfully and get the value of k is OK_VALUE_1.
     */
}

/*
 * @tc.name: ReadWritePerformance 001
 * @tc.desc: calculate the time of put-get in 16b-100b/16b-100kb, 1k times' random-put&get.
 * @tc.type: Performance
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCrudTest, ReadWritePerformance001, TestSize.Level3)
{
    const int PERFORMANCE_SIZE = 6;
    PerformanceData performanceData[PERFORMANCE_SIZE] = {
    /**
     * @tc.steps: step1. put/get 16B key,100B value with random model.
     * @tc.expected: step1. Performance of put&get is normal.
     */
        { 1, 16, 100, false, false, false, false, IS_LOCAL },
    /**
     * @tc.steps: step2. put/get 16B key,100KB value with random model.
     * @tc.expected: step2. Performance of put&get is normal.
     */
        { 1, 16, 100 * 1000, false, false, false, false, IS_LOCAL },
    /**
     * @tc.steps: step3. put/get 16B key,100B value with random model.
     * @tc.expected: step3. Performance of put&get is normal.
     */
        { 50, 16, 100, false, false, false, false, IS_LOCAL },
    /**
     * @tc.steps: step4. put/get 16B key,100KB value with random model.
     * @tc.expected: step4. Performance of put&get is normal.
     */
        { 50, 16, 100 * 1000, false, false, false, false, IS_LOCAL },
    /**
     * @tc.steps: step5. put/get 16B key,100B value with random model.
     * @tc.expected: step5. Performance of put&get is normal.
     */
        { 100, 16, 100, false, false, false, false, IS_LOCAL },
    /**
     * @tc.steps: step6. put/get 16B key,100B value with random model.
     * @tc.expected: step6. Performance of put&get is normal.
     */
        { 100, 16, 100 * 1000, false, false, false, false, IS_LOCAL }
    };

    for (int ps = 0; ps < PERFORMANCE_SIZE; ++ps) {
        DistributedTestTools::CalculateOpenPerformance(performanceData[ps]);
        DistributedTestTools::CalculateInsertPerformance(performanceData[ps]);
        DistributedTestTools::CalculateGetPutPerformance(performanceData[ps]);
        DistributedTestTools::CalculateUpdatePerformance(performanceData[ps]);
        DistributedTestTools::CalculateGetUpdatePerformance(performanceData[ps]);
        DistributedTestTools::CalculateUseClearPerformance(performanceData[ps]);
    }
}
}