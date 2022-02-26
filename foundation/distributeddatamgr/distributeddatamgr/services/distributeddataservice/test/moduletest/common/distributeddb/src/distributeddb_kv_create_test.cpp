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
#include <thread>
#include <condition_variable>
#include <mutex>
#include <string>

#include "distributeddb_data_generator.h"
#include "distributed_test_tools.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "types.h"
#include "distributed_test_sysinfo.h"

using namespace std;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbKvCreate {
const int OPER_CNT = 4;
DistributedDB::CipherPassword g_passwd1;
DistributedDB::CipherPassword g_passwd2;
class DistributeddbKvCreateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributeddbKvCreateTest::SetUpTestCase(void)
{
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    (void)g_passwd2.SetValue(PASSWD_VECTOR_2.data(), PASSWD_VECTOR_2.size());
}

void DistributeddbKvCreateTest::TearDownTestCase(void)
{
}

void DistributeddbKvCreateTest::SetUp(void)
{
    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());
}

void DistributeddbKvCreateTest::TearDown(void)
{
}

/*
 * @tc.name: Open 001
 * @tc.desc: Verify that the kv db was created successfully.
 * @tc.type: FUNC
 * @tc.require: SR000CQS3O
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Open001, TestSize.Level1)
{
    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    KvStoreDelegateManager *manager3 = nullptr;
    KvStoreDelegateManager *manager4 = nullptr;

    /**
     * @tc.steps: step1. create kv db with params storeId1, appId1, userId1.
     * @tc.expected: step1. the kv db was created successfully.
     */
    KvStoreDelegate *result = nullptr;
    result = DistributedTestTools::GetDelegateSuccess(manager1, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(manager1 != nullptr && result != nullptr);
    EXPECT_TRUE(manager1->CloseKvStore(result) == OK);
    result = nullptr;
    /**
     * @tc.steps: step2. create kv db with params storeId2, appId1, userId1.
     * @tc.expected: step2. the kv db was created successfully.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter2_1_1, g_kvOption);
    ASSERT_TRUE(manager2 != nullptr && result != nullptr);
    EXPECT_TRUE(manager2->CloseKvStore(result) == OK);
    result = nullptr;
    /**
     * @tc.steps: step3. create kv db with params storeId1, appId2, userId1.
     * @tc.expected: step3. the kv db was created successfully.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager3, g_kvdbParameter1_2_1, g_kvOption);
    ASSERT_TRUE(manager3 != nullptr && result != nullptr);
    EXPECT_TRUE(manager3->CloseKvStore(result) == OK);
    result = nullptr;
    /**
     * @tc.steps: step4. create kv db with params storeId1, appId1, userId2.
     * @tc.expected: step4. the kv db was created successfully.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager4, g_kvdbParameter1_1_2, g_kvOption);
    ASSERT_TRUE(manager4 != nullptr && result != nullptr);
    EXPECT_EQ(manager4->CloseKvStore(result), OK);
    result = nullptr;

    EXPECT_EQ(manager1->DeleteKvStore(STORE_ID_1), OK);
    delete manager1;
    manager1 = nullptr;
    EXPECT_EQ(manager2->DeleteKvStore(STORE_ID_2), OK);
    delete manager2;
    manager2 = nullptr;
    EXPECT_EQ(manager3->DeleteKvStore(STORE_ID_1), OK);
    delete manager3;
    manager3 = nullptr;
    EXPECT_EQ(manager4->DeleteKvStore(STORE_ID_1), OK);
    delete manager4;
    manager4 = nullptr;
}

/*
 * @tc.name: Open 002
 * @tc.desc: Verify that the kv db can be reopened successfully
 * @tc.type: FUNC
 * @tc.require: SR000CQS3O
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Open002, TestSize.Level0)
{
    KvStoreDelegate *result = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create kv db.
     * @tc.expected: step1. the kv db was created successfully.
     */
    KvOption option = g_kvOption;
    option.localOnly = IS_LOCAL_ONLY;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    delete manager;
    manager = nullptr;
    /**
     * @tc.steps: step2. reopen kv db.
     * @tc.expected: step2. the kv db was reopened successfully.
     */
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);

    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Open 003
 * @tc.desc: Verify that can not reopen an absent db.
 * @tc.type: FUNC
 * @tc.require: SR000CQS3O
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Open003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. delete kv db if exist.
     * @tc.expected: step1. Construct that no database exist.
     */
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    SetDir(DIRECTOR);
    DBStatus status = manager->SetKvStoreConfig(KV_CONFIG);
    EXPECT_EQ(status, DBStatus::OK);
    status = manager->DeleteKvStore(STORE_ID_1);
    EXPECT_EQ(status, DBStatus::NOT_FOUND);
    delete manager;
    manager = nullptr;

    KvStoreDelegate *delegate = nullptr;
    /**
     * @tc.steps: step2. reopen an absent db.
     * @tc.expected: step2. reopen failed and return error.
     */
    KvOption option = g_kvOption;
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    option.localOnly = IS_LOCAL_ONLY;
    status = DistributedTestTools::GetDelegateNotGood(manager, delegate, STORE_ID_1, APP_ID_1, USER_ID_1, option);
    ASSERT_NE(status, DBStatus::OK);
    EXPECT_EQ(delegate, nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), INVALID_ARGS);

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Open 004
 * @tc.desc: Verify that transferring param IS_NOT_NEED_CREATE can not create kv db.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Open004, TestSize.Level1)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_2);
    ASSERT_NE(manager, nullptr);
    SetDir(DIRECTOR);
    EXPECT_EQ(manager->SetKvStoreConfig(KV_CONFIG), DBStatus::OK);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), DBStatus::NOT_FOUND);

    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    /**
     * @tc.steps: step1. create kv db with param IS_NOT_NEED_CREATE.
     * @tc.expected: step1. create kv db and close it failed.
     */
    KvOption option = g_kvOption;
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    KvStoreDelegate *result1 = DistributedTestTools::GetDelegateSuccess(manager1, g_kvdbParameter1_1_2, option);
    EXPECT_TRUE(manager1 == nullptr && result1 == nullptr);
    EXPECT_TRUE(manager->CloseKvStore(result1) != OK);
    delete manager;
    manager = nullptr;

    /**
     * @tc.steps: step2. create kv db with param IS_NEED_CREATE.
     * @tc.expected: step2. create kv db and close it successfully.
     */
    KvStoreDelegate *result2 = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter1_1_2,
        g_kvOption);
    ASSERT_TRUE(manager2 != nullptr && result2 != nullptr);
    EXPECT_TRUE(manager2->CloseKvStore(result2) == OK);
    result2 = nullptr;
    EXPECT_TRUE(manager2->DeleteKvStore(STORE_ID_1) == OK);
    delete manager2;
    manager2 = nullptr;
}

/*
 * @tc.name: Open 005
 * @tc.desc: Verify that can not create or open kv db with exceptional params.
 * @tc.type: EXCEPTION
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Open005, TestSize.Level1)
{
    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    KvStoreDelegateManager *manager3 = nullptr;
    KvStoreDelegateManager *manager4 = nullptr;
    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegate *delegate3 = nullptr;
    KvStoreDelegate *delegate4 = nullptr;

    /**
     * @tc.steps: step1. create kv db with param storeId is null.
     * @tc.expected: step1. create kv db failed.
     */
    KvOption option1 = g_kvOption;
    option1.createIfNecessary = IS_NEED_CREATE;
    option1.localOnly = IS_LOCAL_ONLY;
    DBStatus status = DistributedTestTools::GetDelegateNotGood(manager1, delegate1, "", APP_ID_1, USER_ID_1, option1);
    ASSERT_EQ(status, DBStatus::INVALID_ARGS);
    EXPECT_TRUE(delegate1 == nullptr);
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) == INVALID_ARGS);

    /**
     * @tc.steps: step2. create kv db with param appId is null.
     * @tc.expected: step2. create kv db failed.
     */
    KvOption option2 = g_kvOption;
    option2.createIfNecessary = IS_NEED_CREATE;
    option2.localOnly = IS_LOCAL_ONLY;
    status = DistributedTestTools::GetDelegateNotGood(manager2, delegate2, STORE_ID_1, "", USER_ID_1, option2);
    ASSERT_EQ(status, DBStatus::INVALID_ARGS);
    EXPECT_TRUE(delegate2 == nullptr);
    EXPECT_TRUE(manager2->CloseKvStore(delegate2) == INVALID_ARGS);

    /**
     * @tc.steps: step3. create kv db with param userId is null.
     * @tc.expected: step3. create kv db failed.
     */
    KvOption option3 = g_kvOption;
    option3.createIfNecessary = IS_NEED_CREATE;
    option3.localOnly = IS_LOCAL_ONLY;
    status = DistributedTestTools::GetDelegateNotGood(manager3, delegate3, STORE_ID_1, APP_ID_1, "", option3);
    ASSERT_EQ(status, DBStatus::INVALID_ARGS);
    EXPECT_TRUE(delegate3 == nullptr);
    EXPECT_TRUE(manager3->CloseKvStore(delegate3) == INVALID_ARGS);

    /**
     * @tc.steps: step3. create kv db with param userId is null.
     * @tc.expected: step3. create kv db failed.
     */
    KvOption option4 = g_kvOption;
    option4.createIfNecessary = IS_NOT_NEED_CREATE;
    option4.localOnly = IS_LOCAL_ONLY;
    status = DistributedTestTools::GetDelegateNotGood(manager4, delegate4, STORE_ID_1, APP_ID_1, USER_ID_1, option4);
    ASSERT_NE(status, DBStatus::OK);
    EXPECT_TRUE(delegate4 == nullptr);
    EXPECT_TRUE(manager4->CloseKvStore(delegate4) == INVALID_ARGS);

    delete manager1;
    manager1 = nullptr;
    delete manager2;
    manager2 = nullptr;
    delete manager3;
    manager3 = nullptr;
    delete manager4;
    manager4 = nullptr;
}

/*
 * @tc.name: Close 001
 * @tc.desc: Verify that can close kv db successfully.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Close001, TestSize.Level1)
{
    KvStoreDelegate *result = nullptr;
    KvStoreDelegateManager *manager = nullptr;

    /**
     * @tc.steps: step1. create and open kv db.
     * @tc.expected: step1. create and open kv db successfully.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(manager != nullptr && result != nullptr);

    /**
     * @tc.steps: step2. close kv db that exist.
     * @tc.expected: step2. close kv db successfully.
     */
    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Close 002
 * @tc.desc: Verify that can not close absent db successfully.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Close002, TestSize.Level1)
{
    KvStoreDelegate *result = nullptr;
    KvStoreDelegateManager *manager = nullptr;

    /**
     * @tc.steps: step1. create and delete kv db.
     * @tc.expected: step1. Construct that no database exist.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;

    /**
     * @tc.steps: step2. close nonexistent db.
     * @tc.expected: step2. close db failed.
     */
    EXPECT_EQ(manager->CloseKvStore(result), INVALID_ARGS);

    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Delete 001
 * @tc.desc: Verify that can delete db successfully.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Delete001, TestSize.Level1)
{
    KvStoreDelegate *result = nullptr;
    KvStoreDelegateManager *manager = nullptr;

    /**
     * @tc.steps: step1. create and close kv db.
     * @tc.expected: step1. Construct that database exist.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    /**
     * @tc.steps: step2. delete db.
     * @tc.expected: step2. delete db successfully.
     */
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Delete 002
 * @tc.desc: Verify that can not delete absent db.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Delete002, TestSize.Level1)
{
    KvStoreDelegate *result = nullptr;
    KvStoreDelegateManager *manager = nullptr;

    /**
     * @tc.steps: step1. create and delete kv db.
     * @tc.expected: step1. Construct that no database exist.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);

    /**
     * @tc.steps: step2. delete absent db.
     * @tc.expected: step2. delete absent db failed.
     */
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == NOT_FOUND);

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Config 001
 * @tc.desc: Verify that can set global config of db.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Config001, TestSize.Level0)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    /**
     * @tc.steps: step1. set rightly exist global config of db.
     * @tc.expected: step1. set successfully.
     */
    SetDir(DIRECTOR);
    KvStoreConfig config;
    config.dataDir = DIRECTOR;
    EXPECT_EQ(manager->SetKvStoreConfig(config), DBStatus::OK);
    /**
     * @tc.steps: step2. set exceptionally not exist global config of db.
     * @tc.expected: step2. set failed.
     */
    config.dataDir = "/dataDED/";
    EXPECT_EQ(manager->SetKvStoreConfig(config), INVALID_ARGS);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Config 002
 * @tc.desc: Verify that can get the set storeid and check it.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Config002, TestSize.Level1)
{
    KvStoreDelegate *result = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create kv db with STORE_ID_1.
     * @tc.expected: step1. create db successfully.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(manager != nullptr && result != nullptr);

    /**
     * @tc.steps: step2. get the set storeid of db.
     * @tc.expected: step2. get the set storeid successfully and it equals STORE_ID_1.
     */
    string storeid = result->GetStoreId();
    ASSERT_STREQ(STORE_ID_1.c_str(), storeid.c_str());

    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Performance 001
 * @tc.desc: calculate the time of creating db.
 * @tc.type: Performance
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Performance001, TestSize.Level2)
{
    std::chrono::steady_clock::time_point tick, tock;
    /**
     * @tc.steps: step1. get the time1 and then create kv db.
     * @tc.expected: step1. create kv db successfully.
     */
    SetDir(DIRECTOR);
    DelegateKvMgrCallback delegateKvMgrCallback;
    function<void(DBStatus, KvStoreDelegate*)> function
        = bind(&DelegateKvMgrCallback::Callback, &delegateKvMgrCallback, std::placeholders::_1, std::placeholders::_2);

    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig(KV_CONFIG), DBStatus::OK);
    KvStoreDelegate::Option option = DistributedTestTools::TransferKvOptionType(g_kvOption);

    tick = std::chrono::steady_clock::now();
    manager->GetKvStore(STORE_ID_1, option, function);
    tock = std::chrono::steady_clock::now();

    KvStoreDelegate* delegate = const_cast<KvStoreDelegate*>(delegateKvMgrCallback.GetKvStore());
    EXPECT_NE(delegate, nullptr);
    /**
     * @tc.steps: step2. get the time2 after creating kv db.
     * @tc.expected: step2. time2-time1<100s.
     */
    std::chrono::duration<uint64_t, std::milli> dur;
    dur = std::chrono::duration_cast<std::chrono::milliseconds>(tock - tick);
    double duration = static_cast<double>(dur.count());
    MST_LOG("the time used for create DB is %f ms", duration);

    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Performance 002
 * @tc.desc: check the system or cpu storage and power when creating db.
 * @tc.type: Performance
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Performance002, TestSize.Level2)
{
    /**
     * @tc.steps: step1. get the system info before creating db.
     * @tc.expected: step1. the system or cpu storage and power is normal.
     */
    DistributedTestSysInfo si;
    MST_LOG("System info before opening db");
    si.GetSysMemOccpy(FIRST);
    si.GetSysCpuUsage(FIRST, DEFAULT_INTEVAL);
    si.GetSysCurrentPower(FIRST, DEFAULT_COUNT, DEFAULT_INTEVAL);

    /**
     * @tc.steps: step2. construct the params for interface and create db
     * @tc.expected: step2. create db successfully.
     */
    KvStoreDelegate *result = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(manager != nullptr && result != nullptr);

    /**
     * @tc.steps: step3. get the system info after creating db.
     * @tc.expected: step3. the system or cpu storage and power is normal.
     */
    MST_LOG("System info after opening db");
    si.GetSysMemOccpy(SECOND);
    si.GetSysCpuUsage(SECOND, DEFAULT_INTEVAL);
    si.GetSysCurrentPower(SECOND, DEFAULT_COUNT, DEFAULT_INTEVAL);

    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == DBStatus::OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Performance 003
 * @tc.desc: calculate the time of reopening db.
 * @tc.type: Performance
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Performance003, TestSize.Level2)
{
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create and close kv db.
     * @tc.expected: step1. Construct that database exist.
     */
    KvOption option = g_kvOption;
    option.localOnly = IS_LOCAL_ONLY;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;

    DelegateKvMgrCallback delegateKvMgrCallback;
    function<void(DBStatus, KvStoreDelegate*)> function
        = bind(&DelegateKvMgrCallback::Callback, &delegateKvMgrCallback, std::placeholders::_1, std::placeholders::_2);
    std::chrono::steady_clock::time_point tick, tock;
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    /**
     * @tc.steps: step2. get the tick and tock value of the system before and after reopen the db.
     * @tc.expected: step2. the system time of tick and tock obtained successfully.
     */
    KvStoreDelegate::Option optionUsedForGetStore = DistributedTestTools::TransferKvOptionType(option);
    tick = std::chrono::steady_clock::now();
    manager->GetKvStore(STORE_ID_1, optionUsedForGetStore, function);
    tock = std::chrono::steady_clock::now();

    /**
     * @tc.steps: step3. tock - tick and translate it to milliseconds.
     * @tc.expected: step3. tock - tick < 150ms.
     */
    std::chrono::duration<uint64_t, std::milli> dur;
    dur = std::chrono::duration_cast<std::chrono::milliseconds>(tock - tick);
    double duration = static_cast<double>(dur.count());
    MST_LOG("the time used for reopen DB is %f ms", duration);

    delegate = const_cast<KvStoreDelegate*>(delegateKvMgrCallback.GetKvStore());
    EXPECT_NE(delegate, nullptr);

    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == DBStatus::OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Performance 004
 * @tc.desc: check the system or cpu storage and power when reopening db.
 * @tc.type: System overhead
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Performance004, TestSize.Level3)
{
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create and close kv db.
     * @tc.expected: step1. Construct that database exist.
     */
    KvOption option04 = g_kvOption;
    option04.localOnly = IS_LOCAL_ONLY;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option04);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;

    /**
     * @tc.steps: step2. get the system info before reopening db and then reopen db.
     * @tc.expected: step2. the system or cpu storage and power is normal and reopen db successful .
     */
    MST_LOG("System info before reopening db");
    DistributedTestSysInfo sysInfo04;
    sysInfo04.GetSysMemOccpy(FIRST);
    sysInfo04.GetSysCpuUsage(FIRST, DEFAULT_INTEVAL);
    sysInfo04.GetSysCurrentPower(FIRST, DEFAULT_COUNT, DEFAULT_INTEVAL);
    option04.createIfNecessary = IS_NOT_NEED_CREATE;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option04);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step3. get the system info after reopening db.
     * @tc.expected: step3. the system or cpu storage and power is normal and reopen db successful.
     */
    MST_LOG("System info after reopening db");
    sysInfo04.GetSysMemOccpy(SECOND);
    sysInfo04.GetSysCpuUsage(SECOND, DEFAULT_INTEVAL);
    sysInfo04.GetSysCurrentPower(SECOND, DEFAULT_COUNT, DEFAULT_INTEVAL);

    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == DBStatus::OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Performance 005
 * @tc.desc: check the system storage when reopening by reopening db.
 * @tc.type: System overhead
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Performance005, TestSize.Level3)
{
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create and close kv db.
     * @tc.expected: step1. Construct that database exist.
     */
    KvOption option05 = g_kvOption;
    option05.localOnly = IS_LOCAL_ONLY;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option05);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;

    /**
     * @tc.steps: step2. reopen db.
     * @tc.expected: step2. reopen db successful.
     */
    option05.createIfNecessary = IS_NOT_NEED_CREATE;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option05);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step3. get the system info before close db.
     * @tc.expected: step3. reopen db successful.
     */
    MST_LOG("System info before close db again and again");
    DistributedTestSysInfo sysInfo05;
    sysInfo05.GetSysMemOccpy(FIRST);
    sysInfo05.GetSysCpuUsage(FIRST, DEFAULT_INTEVAL);
    sysInfo05.GetSysCurrentPower(FIRST, DEFAULT_COUNT, DEFAULT_INTEVAL);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;

    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option05);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step4. get the system info after reopening db.
     * @tc.expected: step4. the system has no memory leak.
     */
    MST_LOG("System info after opening db again and again");
    sysInfo05.GetSysMemOccpy(SECOND);
    sysInfo05.GetSysCpuUsage(SECOND, DEFAULT_INTEVAL);
    sysInfo05.GetSysCurrentPower(SECOND, DEFAULT_COUNT, DEFAULT_INTEVAL);

    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == DBStatus::OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Performance 006
 * @tc.desc: check the handler using when reopening by reopening db.
 * @tc.type: System overhead
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, Performance006, TestSize.Level2)
{
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create and close kv db.
     * @tc.expected: step1. Construct that database exist.
     */
    KvOption option06 = g_kvOption;
    option06.localOnly = IS_LOCAL_ONLY;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option06);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;
    /**
     * @tc.steps: step2. reopen by reopen db three times.
     * @tc.expected: step2. reopen db successful and the system has no handle overflow.
     */
    option06.createIfNecessary = IS_NOT_NEED_CREATE;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option06);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;

    option06.createIfNecessary = IS_NEED_CREATE;
    option06.localOnly = IS_NOT_LOCAL_ONLY;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option06);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;

    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option06);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;

    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == DBStatus::OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: PathException 001
 * @tc.desc: check it that can't create kv db without setting path with manager->SetKvStoreConfig(KV_CONFIG) interface.
 * @tc.type: EXCEPTION
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, PathException001, TestSize.Level2)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreDelegate *delegate = nullptr;

    SetDir(DIRECTOR);
    DelegateKvMgrCallback delegateKvMgrCallback;
    function<void(DBStatus, KvStoreDelegate*)> function
        = bind(&DelegateKvMgrCallback::Callback, &delegateKvMgrCallback, std::placeholders::_1, std::placeholders::_2);

    manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    /**
     * @tc.steps: step1. create kv db without setting path with manager->SetKvStoreConfig(KV_CONFIG) interface.
     * @tc.expected: step1. create and close db failed.
     */
    KvStoreDelegate::Option option = DistributedTestTools::TransferKvOptionType(g_kvOption);
    manager->GetKvStore(STORE_ID_1, option, function);
    EXPECT_EQ(delegateKvMgrCallback.GetStatus(), DBStatus::INVALID_ARGS);

    delegate = const_cast<KvStoreDelegate*>(delegateKvMgrCallback.GetKvStore());
    EXPECT_EQ(delegate, nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), INVALID_ARGS);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), DBStatus::INVALID_ARGS);

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: PathException 002
 * @tc.desc: create kv db after setting no existing path.
 * @tc.type: EXCEPTION
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, PathException002, TestSize.Level2)
{
    /**
     * @tc.steps: step1. create kv db when setting no existing path.
     * @tc.expected: step1. create db failed.
     */
    DelegateKvMgrCallback delegateKvMgrCallback;
    function<void(DBStatus, KvStoreDelegate*)> function
        = bind(&DelegateKvMgrCallback::Callback, &delegateKvMgrCallback, std::placeholders::_1, std::placeholders::_2);

    KvStoreConfig config;
    config.dataDir = "/NOTFOUUD/PATH";

    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    KvStoreDelegate::Option option = {true, true};

    EXPECT_EQ(manager->SetKvStoreConfig(config), DBStatus::INVALID_ARGS);

    manager->GetKvStore(STORE_ID_1, option, function);
    EXPECT_EQ(delegateKvMgrCallback.GetStatus(), INVALID_ARGS);

    KvStoreDelegate* delegate = const_cast<KvStoreDelegate*>(delegateKvMgrCallback.GetKvStore());
    EXPECT_EQ(delegate, nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), INVALID_ARGS);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), DBStatus::INVALID_ARGS);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: PathException 003
 * @tc.desc: create kv db with invalid-char param.
 * @tc.type: EXCEPTION
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, PathException003, TestSize.Level2)
{
    char chinese[5] = { static_cast<char>(0xD6), static_cast<char>(0xD0),
        static_cast<char>(0xCE), static_cast<char>(0xC4) }; // not good ascii letters:<D6><D0><CE><C4>
    std::string chStr = chinese;
    MST_LOG("%s", chStr.c_str());

    DelegateKvMgrCallback exceptionCallback;
    function<void(DBStatus, KvStoreDelegate*)> function
        = bind(&DelegateKvMgrCallback::Callback, &exceptionCallback, std::placeholders::_1, std::placeholders::_2);
    KvStoreConfig config;
    config.dataDir = "";
    /**
     * @tc.steps: step1. create kv db with the path is null.
     * @tc.expected: step1. create db failed.
     */
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    KvStoreDelegate::Option option = {true, true};
    EXPECT_EQ(manager->SetKvStoreConfig(config), DBStatus::INVALID_ARGS);
    manager->GetKvStore(STORE_ID_1, option, function);
    EXPECT_EQ(exceptionCallback.GetStatus(), INVALID_ARGS);
    delete manager;
    manager = nullptr;
    /**
     * @tc.steps: step1. create kv db with storeId="中文".
     * @tc.expected: step1. create db failed.
     */
    SetDir(DIRECTOR);
    config.dataDir = DIRECTOR;
    manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig(config), DBStatus::OK);
    manager->GetKvStore(chStr, option, function);
    EXPECT_EQ(exceptionCallback.GetStatus(), INVALID_ARGS);
    delete manager;
    manager = nullptr;

    /**
     * @tc.steps: step2. create kv db with appId="中文".
     * @tc.expected: step2. create db failed.
     */
    manager = new (std::nothrow) KvStoreDelegateManager(chStr, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig(config), DBStatus::OK);
    manager->GetKvStore(chStr, option, function);
    EXPECT_EQ(exceptionCallback.GetStatus(), INVALID_ARGS);
    delete manager;
    manager = nullptr;
    /**
     * @tc.steps: step3. create kv db with userId="中文".
     * @tc.expected: step3. create db failed.
     */
    manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, chStr);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig(config), DBStatus::OK);
    manager->GetKvStore(chStr, option, function);
    EXPECT_EQ(exceptionCallback.GetStatus(), INVALID_ARGS);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: PathException 004
 * @tc.desc: create kv db with too long param.
 * @tc.type: EXCEPTION
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, PathException004, TestSize.Level2)
{
    std::string storeTooLongID, appTooLongID, userTooLongID;
    storeTooLongID.append(TWO_M_LONG_STRING, 'a');
    appTooLongID.append(TWO_M_LONG_STRING, 'b');
    userTooLongID.append(TWO_M_LONG_STRING, 'c');

    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    KvStoreDelegateManager *manager3 = nullptr;
    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegate *delegate3 = nullptr;
    /**
     * @tc.steps: step1. create kv db with storeId=storeTooLongID.
     * @tc.expected: step1. create db failed.
     */
    DBStatus status1 = DistributedTestTools::GetDelegateNotGood(manager1, delegate1,
        storeTooLongID, APP_ID_1, USER_ID_1, g_kvOption);
    EXPECT_EQ(delegate1, nullptr);
    EXPECT_EQ(manager1->CloseKvStore(delegate1), INVALID_ARGS);
    /**
     * @tc.steps: step2. create kv db with appId=appTooLongID.
     * @tc.expected: step2. create db failed.
     */
    DBStatus status2 = DistributedTestTools::GetDelegateNotGood(manager2, delegate2,
        USER_ID_1, appTooLongID, USER_ID_1, g_kvOption);
    EXPECT_EQ(delegate2, nullptr);
    EXPECT_EQ(manager2->CloseKvStore(delegate2), INVALID_ARGS);
    /**
     * @tc.steps: step3. create kv db with userId=userTooLongID.
     * @tc.expected: step3. create db failed.
     */
    DBStatus status3 = DistributedTestTools::GetDelegateNotGood(manager3, delegate3,
        USER_ID_1, APP_ID_1, userTooLongID, g_kvOption);
    EXPECT_EQ(delegate3, nullptr);
    EXPECT_EQ(manager3->CloseKvStore(delegate3), INVALID_ARGS);

    ASSERT_EQ(status1, DBStatus::INVALID_ARGS);
    ASSERT_EQ(status2, DBStatus::INVALID_ARGS);
    ASSERT_EQ(status3, DBStatus::INVALID_ARGS);

    delete manager1;
    manager1 = nullptr;
    delete manager2;
    manager2 = nullptr;
    delete manager3;
    manager3 = nullptr;
}

/*
 * @tc.name: PathException 005
 * @tc.desc: verify that the same DB can be reopen many times.
 * @tc.type: Stability
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, PathException005, TestSize.Level2)
{
    KvStoreDelegate *result = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create and close kv db.
     * @tc.expected: step1. Construct that database exist.
     */
    KvOption option = g_kvOption;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1_1_2, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;

    /**
     * @tc.steps: step2. reopen kv db five times.
     * @tc.expected: step2. reopen successfully and no memory exception when closing db.
     */
    option.createIfNecessary = IS_NEED_CREATE;
    KvStoreDelegate::Option optionUsedForGetStore = DistributedTestTools::TransferKvOptionType(option);

    DelegateKvMgrCallback delegateCallback;
    function<void(DBStatus, KvStoreDelegate*)> function
        = bind(&DelegateKvMgrCallback::Callback, &delegateCallback, std::placeholders::_1, std::placeholders::_2);
    KvStoreDelegate* delegate = nullptr;
    for (int operCnt = 1; operCnt <= OPER_CNT; ++operCnt) {
        manager->GetKvStore(STORE_ID_1, optionUsedForGetStore, function);
        EXPECT_EQ(delegateCallback.GetStatus(), OK);
        delegate = const_cast<KvStoreDelegate*>(delegateCallback.GetKvStore());
        EXPECT_NE(delegate, nullptr);

        EXPECT_EQ(manager->CloseKvStore(delegate), OK);
        delegate = nullptr;
    }

    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), DBStatus::OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: PathException 006
 * @tc.desc: verify that one delegate can be closed only one time.
 * @tc.type: EXCEPTION
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, PathException006, TestSize.Level2)
{
    KvStoreDelegate *result = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create and close kv db.
     * @tc.expected: step1. Construct that database exist.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1_1_2, g_kvOption);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;

    /**
     * @tc.steps: step2. create and close kv db more four times.
     * @tc.expected: step2. close successfully and no memory leak.
     */
    for (int operCnt = 1; operCnt <= OPER_CNT; ++operCnt) {
        EXPECT_EQ(manager->CloseKvStore(result), INVALID_ARGS);
    }

    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: PathException 007
 * @tc.desc: open deleted kv db.
 * @tc.type: EXCEPTION
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, PathException007, TestSize.Level2)
{
    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegate *delegate2 = nullptr;
    /**
     * @tc.steps: step1. create and close kv db.
     * @tc.expected: step1. Construct that database exist.
     */
    delegate1 = DistributedTestTools::GetDelegateSuccess(manager1, g_kvdbParameter1_1_2, g_kvOption);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr);
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) == OK);
    delegate1 = nullptr;

    /**
     * @tc.steps: step2. delete the DB.
     * @tc.expected: step2. delete successfully.
     */
    EXPECT_EQ(manager1->DeleteKvStore(STORE_ID_1), OK);
    delete manager1;
    manager1 = nullptr;
    /**
     * @tc.steps: step3. open the deleted DB with the mode createIfNecessary = IS_NOT_NEED_CREATE.
     * @tc.expected: step3. open failed and return error.
     */
    KvOption option = g_kvOption;
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    option.localOnly = IS_LOCAL_ONLY;
    DBStatus status = DistributedTestTools::GetDelegateNotGood(manager2, delegate2,
        STORE_ID_1, APP_ID_1, USER_ID_2, option);
    EXPECT_TRUE(delegate2 == nullptr);
    EXPECT_EQ(manager2->CloseKvStore(delegate2), INVALID_ARGS);
    ASSERT_NE(status, OK);

    delete manager2;
    manager2 = nullptr;
}

/*
 * @tc.name: ConflictConfig 001
 * @tc.desc: check SetConflictResolutionPolicy interface with AUTO_LAST_WIN mode
 * @tc.type: LONGTIME TEST
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, ConflictConfig001, TestSize.Level1)
{
    KvStoreDelegate *conflictDelegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    conflictDelegate = DistributedTestTools::GetDelegateSuccess(manager,
        g_kvdbParameter1_1_2, g_kvOption);
    ASSERT_TRUE(manager != nullptr && conflictDelegate != nullptr);

    /**
     * @tc.steps: step1. check SetConflictResolutionPolicy interface with AUTO_LAST_WIN mode.
     * @tc.expected: step1. return ok.
     */
    EXPECT_EQ(conflictDelegate->SetConflictResolutionPolicy(AUTO_LAST_WIN, nullptr), OK);

    EXPECT_TRUE(manager->CloseKvStore(conflictDelegate) == OK);
    conflictDelegate = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ConflictConfig 002
 * @tc.desc: check SetConflictResolutionPolicy interface with CUSTOMER_RESOLUTION mode
 * @tc.type: LONGTIME TEST
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, ConflictConfig002, TestSize.Level1)
{
    KvStoreDelegate *conflictDelegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    conflictDelegate = DistributedTestTools::GetDelegateSuccess(manager,
        g_kvdbParameter1_1_2, g_kvOption);
    ASSERT_TRUE(manager != nullptr && conflictDelegate != nullptr);

    /**
     * @tc.steps: step1. check SetConflictResolutionPolicy interface with CUSTOMER_RESOLUTION mode.
     * @tc.expected: step1. return error.
     */
    EXPECT_NE(conflictDelegate->SetConflictResolutionPolicy(CUSTOMER_RESOLUTION, nullptr), OK);

    EXPECT_TRUE(manager->CloseKvStore(conflictDelegate) == OK);
    conflictDelegate = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ConflictConfig 003
 * @tc.desc: check SetConflictResolutionPolicy interface with invalid mode
 * @tc.type: LONGTIME TEST
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvCreateTest, ConflictConfig003, TestSize.Level1)
{
    KvStoreDelegate *conflictDelegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    conflictDelegate = DistributedTestTools::GetDelegateSuccess(manager,
        g_kvdbParameter1_1_2, g_kvOption);
    ASSERT_TRUE(manager != nullptr && conflictDelegate != nullptr);

    /**
     * @tc.steps: step1. check SetConflictResolutionPolicy interface with invalid mode.
     * @tc.expected: step1. return ok.
     */
    EXPECT_EQ(conflictDelegate->SetConflictResolutionPolicy(static_cast<ResolutionPolicyType>(2), nullptr), DB_ERROR);
    EXPECT_EQ(conflictDelegate->SetConflictResolutionPolicy(static_cast<ResolutionPolicyType>(-1), nullptr), DB_ERROR);

    EXPECT_TRUE(manager->CloseKvStore(conflictDelegate) == OK);
    conflictDelegate = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: OptionParam 001
 * @tc.desc: verify that will check the option parameter when create encrypted DB.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvCreateTest, OptionParam001, TestSize.Level1)
{
    vector<KvStoreDelegateManager *> manager = {nullptr, nullptr, nullptr};
    vector<KvStoreDelegate*> result = {nullptr, nullptr, nullptr, nullptr, nullptr};
    KvOption option;
    vector<uint8_t> password;

    /**
     * @tc.steps: step1. isEncryptedDb=true, passwd=NULL, cipher=DEFAULT when create db.
     * @tc.expected: step1. create failed and return INVALID_ARGS.
     */
    option.isEncryptedDb = true;
    option.cipher = CipherType::DEFAULT;
    option.passwd = NULL_PASSWD_VECTOR;
    DBStatus status;
    result[INDEX_ZEROTH] = DistributedTestTools::GetDelegateStatus(manager[INDEX_ZEROTH],
        status, g_kvdbParameter1, option);
    EXPECT_TRUE(manager[INDEX_ZEROTH] == nullptr && result[INDEX_ZEROTH] == nullptr);
    EXPECT_TRUE(status == INVALID_ARGS);

    /**
     * @tc.steps: step2. isEncryptedDb=true, passwd=a......(100B) when create db.
     * @tc.expected: step2. create successfully and return OK.
     */
    password.assign(VALUE_ONE_HUNDRED_BYTE, 'a');
    option.passwd = password;
    result[INDEX_FIRST] = DistributedTestTools::GetDelegateSuccess(manager[INDEX_ZEROTH], g_kvdbParameter1, option);
    ASSERT_TRUE(manager[INDEX_ZEROTH] != nullptr && result[INDEX_FIRST] != nullptr);
    EXPECT_EQ(manager[INDEX_ZEROTH]->CloseKvStore(result[INDEX_FIRST]), OK);
    result[INDEX_FIRST] = nullptr;

    /**
     * @tc.steps: step3. isEncryptedDb=true, passwd=a......(128B), cipher=AES_256_GCM when create db.
     * @tc.expected: step3. create successfully and return OK.
     */
    password.clear();
    password.assign(BATCH_RECORDS, 'a');
    option.cipher = CipherType::AES_256_GCM;
    option.passwd = password;
    result[INDEX_SECOND] = DistributedTestTools::GetDelegateSuccess(manager[INDEX_FIRST], g_kvdbParameter2, option);
    ASSERT_TRUE(manager[INDEX_FIRST] != nullptr && result[INDEX_SECOND] != nullptr);
    EXPECT_EQ(manager[INDEX_FIRST]->CloseKvStore(result[INDEX_SECOND]), OK);
    result[INDEX_SECOND] = nullptr;

    /**
     * @tc.steps: step4. isEncryptedDb=true, passwd=a......(129B), cipher=DEFAULT when create db.
     * @tc.expected: step4. create failed and return INVALID_ARGS.
     */
    password.clear();
    password.assign(PASSWD_BYTE, 'a');
    option.cipher = CipherType::DEFAULT;
    option.passwd = password;
    result[INDEX_THIRD] = DistributedTestTools::GetDelegateStatus(manager[INDEX_SECOND],
        status, g_kvdbParameter3, option);
    EXPECT_TRUE(manager[INDEX_SECOND] == nullptr && result[INDEX_THIRD] == nullptr);
    EXPECT_TRUE(status == INVALID_ARGS);

    /**
     * @tc.steps: step5. isEncryptedDb=false, passwd=a......(129B), cipher=DEFAULT when create db.
     * @tc.expected: step5. create successfully and return OK.
     */
    option.isEncryptedDb = false;
    result[INDEX_FORTH] = DistributedTestTools::GetDelegateSuccess(manager[INDEX_SECOND], g_kvdbParameter3, option);
    ASSERT_TRUE(manager[INDEX_SECOND] != nullptr && result[INDEX_FORTH] != nullptr);
    EXPECT_EQ(manager[INDEX_SECOND]->CloseKvStore(result[INDEX_FORTH]), OK);
    result[INDEX_FORTH] = nullptr;

    EXPECT_EQ(manager[INDEX_ZEROTH]->DeleteKvStore(STORE_ID_1), OK);
    EXPECT_EQ(manager[INDEX_FIRST]->DeleteKvStore(STORE_ID_2), OK);
    EXPECT_EQ(manager[INDEX_SECOND]->DeleteKvStore(STORE_ID_3), OK);
    for (auto &item : manager) {
        if (item != nullptr) {
            delete item;
        }
    }
}

/*
 * @tc.name: OptionParam 002
 * @tc.desc: verify that isEncryptedDb and passwd are consistent with the creation time can open an existing DB.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvCreateTest, OptionParam002, TestSize.Level1)
{
    vector<KvStoreDelegateManager *> manager = {nullptr, nullptr, nullptr, nullptr};
    KvStoreDelegate *result = nullptr;
    KvOption option;

    /**
     * @tc.steps: step1. isEncryptedDb=true, passwd=p1, cipher=DEFAULT when create db1.
     * @tc.expected: step1. create successfully and return OK.
     */
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result = DistributedTestTools::GetDelegateSuccess(manager[INDEX_ZEROTH], g_kvdbParameter1, option);
    ASSERT_TRUE(manager[INDEX_ZEROTH] != nullptr && result != nullptr);
    EXPECT_TRUE(manager[INDEX_ZEROTH]->CloseKvStore(result) == OK);
    result = nullptr;
    /**
     * @tc.steps: step2. isEncryptedDb=false, passwd=p1 when open db1.
     * @tc.expected: step2. open failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    option.isEncryptedDb = false;
    DBStatus status;
    result = DistributedTestTools::GetDelegateStatus(manager[INDEX_FIRST], status, g_kvdbParameter1, option);
    EXPECT_TRUE(manager[INDEX_FIRST] == nullptr && result == nullptr);
    EXPECT_TRUE(status == INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step3. isEncryptedDb=true, passwd=p2 when open db1.
     * @tc.expected: step3. open failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_2;
    result = DistributedTestTools::GetDelegateStatus(manager[INDEX_FIRST], status, g_kvdbParameter1, option);
    EXPECT_TRUE(manager[INDEX_FIRST] == nullptr && result == nullptr);
    EXPECT_TRUE(status == INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step4. isEncryptedDb=true, passwd=p1 when open db1.
     * @tc.expected: step4. open successfully and return OK.
     */
    option.passwd = PASSWD_VECTOR_1;
    result = DistributedTestTools::GetDelegateSuccess(manager[INDEX_FIRST], g_kvdbParameter1, option);
    ASSERT_TRUE(manager[INDEX_FIRST] != nullptr && result != nullptr);
    EXPECT_TRUE(manager[INDEX_FIRST]->CloseKvStore(result) == OK);
    result = nullptr;

    /**
     * @tc.steps: step5. isEncryptedDb=false, passwd=p1, cipher=DEFAULT when create db2.
     * @tc.expected: step5. create successfully and return OK.
     */
    option.isEncryptedDb = false;
    result = DistributedTestTools::GetDelegateSuccess(manager[INDEX_SECOND], g_kvdbParameter2, option);
    ASSERT_TRUE(manager[INDEX_SECOND] != nullptr && result != nullptr);
    EXPECT_TRUE(manager[INDEX_SECOND]->CloseKvStore(result) == OK);
    result = nullptr;

    /**
     * @tc.steps: step6. isEncryptedDb=true, passwd=p1 when open db2.
     * @tc.expected: step6. open failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    option.isEncryptedDb = true;
    result = DistributedTestTools::GetDelegateStatus(manager[INDEX_THIRD], status, g_kvdbParameter2, option);
    EXPECT_TRUE(manager[INDEX_THIRD] == nullptr && result == nullptr);
    EXPECT_TRUE(status == INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step7. isEncryptedDb=false, passwd=p2 when open db2.
     * @tc.expected: step7. open successfully and return OK.
     */
    option.isEncryptedDb = false;
    option.passwd = PASSWD_VECTOR_2;
    result = DistributedTestTools::GetDelegateSuccess(manager[INDEX_THIRD], g_kvdbParameter2, option);
    ASSERT_TRUE(manager[INDEX_THIRD] != nullptr && result != nullptr);
    EXPECT_TRUE(manager[INDEX_THIRD]->CloseKvStore(result) == OK);
    result = nullptr;
    EXPECT_TRUE(manager[INDEX_FIRST]->DeleteKvStore(STORE_ID_1) == OK);
    EXPECT_TRUE(manager[INDEX_THIRD]->DeleteKvStore(STORE_ID_2) == OK);
    for (auto &item : manager) {
        if (item != nullptr) {
            delete item;
            item = nullptr;
        }
    }
}

void ReleaseManager(KvStoreDelegateManager *&manager)
{
    if (manager != nullptr) {
        delete manager;
        manager = nullptr;
    }
    return;
}
/*
 * @tc.name: RekeyDb 001
 * @tc.desc: verify that can switching a non-encrypted database to an encrypted database by Rekey.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvCreateTest, RekeyDb001, TestSize.Level1)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreDelegate *result = nullptr;
    KvOption option;

    /**
     * @tc.steps: step1. create unencrypted db, use Rekey to update its passwd to NULL,
     * then close and open without passwd.
     * @tc.expected: step1. operate successfully and can open db again without passwd.
     */
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_TRUE(result->Rekey(NULL_PASSWD) == OK);
    DistributedTestTools::CloseAndRelease(manager, result);
    option.createIfNecessary = false;
    option.isEncryptedDb = false;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    ASSERT_TRUE(DistributedTestTools::Put(*result, KEY_1, VALUE_1) == DBStatus::OK);

    /**
     * @tc.steps: step2. use Rekey to update db's passwd to p1=a......(100B), then close
     * and open again without passwd.
     * @tc.expected: step2. Rekey is ok, open db failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    vector<uint8_t> passwordVector(VALUE_ONE_HUNDRED_BYTE, 'a');
    CipherPassword password;
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OK);
    EXPECT_TRUE(result->Rekey(password) == OK);
    DistributedTestTools::CloseAndRelease(manager, result);
    DBStatus status;
    ASSERT_TRUE(DistributedTestTools::GetDelegateStatus(manager, status, g_kvdbParameter1, option) == nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step3. use p1 to open db and Get(k1), GetLocal(k1).
     * @tc.expected: step3. open successfully and Get(k1)=v1.
     */
    option.isEncryptedDb = true;
    option.passwd = passwordVector;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    Value valueResult = DistributedTestTools::Get(*result, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));

    /**
     * @tc.steps: step4. use Rekey to update db's passwd to p2=passwordVector whose size is 128B, then close
     * and open again with p1.
     * @tc.expected: step4. Rekey is ok, open db failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    passwordVector.assign(BATCH_RECORDS, 'a');
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OK);
    EXPECT_TRUE(result->Rekey(password) == OK);
    DistributedTestTools::CloseAndRelease(manager, result);
    ASSERT_TRUE(DistributedTestTools::GetDelegateStatus(manager, status, g_kvdbParameter1, option) == nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step5. use p2 to open db and delete(k1).
     * @tc.expected: step5. operate successfully.
     */
    option.passwd = passwordVector;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    ASSERT_TRUE(DistributedTestTools::Delete(*result, KEY_1) == OK);

    /**
     * @tc.steps: step6. use Rekey to update db's passwd to p2=passwordVector whose size is 129B.
     * @tc.expected: step6. Rekey failed and return INVALID_ARGS.
     */
    passwordVector.assign(PASSWD_BYTE, 'a');
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OVERSIZE);

    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);
    ReleaseManager(manager);
}

/*
 * @tc.name: RekeyDb 002
 * @tc.desc: verify that can change passwd of an encrypted database by Rekey.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvCreateTest, RekeyDb002, TestSize.Level1)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreDelegate *result = nullptr;
    KvOption option;

    /**
     * @tc.steps: step1. create encrypted db with p1=PASSWD_VECTOR_1.
     * @tc.expected: step1. create successfully.
     */
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);

    /**
     * @tc.steps: step2. use Rekey to update passwd to p1 and close db then open db again with unencrypted way.
     * @tc.expected: step2. the Rekey return OK, but open db failed.
     */
    EXPECT_TRUE(result->Rekey(g_passwd1) == OK);
    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    ReleaseManager(manager);
    option.createIfNecessary = false;
    option.isEncryptedDb = false;
    DBStatus status;
    result = DistributedTestTools::GetDelegateStatus(manager, status, g_kvdbParameter1, option);
    ASSERT_TRUE(result == nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step3. open db with passwd=p1 and putbatch(k1,v1)(k2,v2).
     * @tc.expected: step3. operate successfully.
     */
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    EXPECT_TRUE(DistributedTestTools::PutBatch(*result, entries1) == OK);

    /**
     * @tc.steps: step4. use Rekey to update passwd to NULL and close db then open db with passwd=p1.
     * @tc.expected: step4. the Rekey return OK, but open db failed.
     */
    EXPECT_TRUE(result->Rekey(NULL_PASSWD) == OK);
    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    ReleaseManager(manager);
    result = DistributedTestTools::GetDelegateStatus(manager, status, g_kvdbParameter1, option);
    ASSERT_TRUE(result == nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step5. open db again with unencrypted way and Get(k1), GetLocal(k2).
     * @tc.expected: step5. open successfully and Get(k1)=v1, GetLocal(k2)=v2.
     */
    option.isEncryptedDb = false;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    vector<Entry> valueResult = DistributedTestTools::GetEntries(*result, KEY_EMPTY);
    ASSERT_TRUE(valueResult.size() == entries1.size());

    /**
     * @tc.steps: step6. use Rekey to update db's passwd to p2=a......(129B).
     * @tc.expected: step6. Reksy failed and return INVALID_ARGS.
     */
    vector<uint8_t> passwordVector(PASSWD_BYTE, 'a');
    CipherPassword password;
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OVERSIZE);

    EXPECT_TRUE(manager->CloseKvStore(result) == OK);
    result = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_1) == OK);
    ReleaseManager(manager);
}

#ifndef LOW_LEVEL_MEM_DEV
/*
 * @tc.name: RekeyDb 003
 * @tc.desc: verify that do other operations during Rekey execution, the operation returns busy.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvCreateTest, RekeyDb003, TestSize.Level3)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreDelegate *result1 = nullptr;
    KvStoreDelegate *result2 = nullptr;
    KvOption option;

    /**
     * @tc.steps: step1. use Rekey to update passwd to p1=PASSWD_VECTOR_1.
     * @tc.expected: step1. create successfully.
     */
    result1 = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result1 != nullptr);
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, ONE_HUNDRED_RECORDS, ONE_K_LONG_STRING, ONE_M_LONG_STRING);
    EXPECT_EQ(DistributedTestTools::PutBatch(*result1, entriesBatch), OK);
    bool rekeyFlag1 = false;
    thread subThread1([&]() {
        DBStatus status = result1->Rekey(g_passwd1);
        EXPECT_TRUE(status == OK || status == BUSY);
        rekeyFlag1 = true;
        g_conditionKvVar.notify_one();
    });
    subThread1.detach();

    /**
     * @tc.steps: step2. Call the GetKvstore interface when Rekey.
     * @tc.expected: step2. the GetKvstore return BUSY.
     */
    option.createIfNecessary = false;
    KvStoreDelegateManager *managerRes = nullptr;
    DBStatus status;
    result2 = DistributedTestTools::GetDelegateStatus(managerRes, status, g_kvdbParameter1, option);
    EXPECT_TRUE(status == BUSY || status == OK);
    if (result2 != nullptr) {
        EXPECT_EQ(managerRes->CloseKvStore(result2), OK);
        ReleaseManager(managerRes);
    }
    std::mutex count;
    {
        std::unique_lock<std::mutex> lck(count);
        g_conditionKvVar.wait(lck, [&]{return rekeyFlag1;});
    }

    /**
     * @tc.steps: step3. put data to db when Rekey.
     * @tc.expected: step3. the put return BUSY.
     */
    bool rekeyFlag2 = false;
    thread subThread2([&]() {
        DBStatus rekeyStatus = result1->Rekey(g_passwd2);
        EXPECT_TRUE(rekeyStatus == OK || rekeyStatus == BUSY);
        rekeyFlag2 = true;
        g_conditionKvVar.notify_all();
    });
    subThread2.detach();
    status = DistributedTestTools::Put(*result1, KEY_1, VALUE_1);
    EXPECT_TRUE(status == BUSY || status == OK);
    std::unique_lock<std::mutex> lck(count);
    g_conditionKvVar.wait(lck, [&]{return rekeyFlag2;});
    EXPECT_EQ(manager->CloseKvStore(result1), OK);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    ReleaseManager(manager);
}
#endif

/*
 * @tc.name: RekeyDb 004
 * @tc.desc: verify that Rekey will return busy when there are multiple instances of the same KvStore.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvCreateTest, RekeyDb004, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreDelegate *result1 = nullptr;
    KvStoreDelegate *result2 = nullptr;
    KvOption option;

    result1 = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result1 != nullptr);
    ReleaseManager(manager);

    /**
     * @tc.steps: step1. use GetKvstore to open another instances of the same KvStore.
     * @tc.expected: step1. open successfully.
     */
    option.createIfNecessary = false;
    result2 = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result2 != nullptr);

    /**
     * @tc.steps: step2. call Rekey.
     * @tc.expected: step2. Rekey returns BUSY.
     */
    EXPECT_EQ(result2->Rekey(g_passwd1), BUSY);
    EXPECT_EQ(manager->CloseKvStore(result1), OK);
    result1 = nullptr;
    EXPECT_EQ(manager->CloseKvStore(result2), OK);
    result2 = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    ReleaseManager(manager);
}

void RunDbRekeyOne()
{
    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegate *result1 = nullptr;
    KvOption option;
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result1 = DistributedTestTools::GetDelegateSuccess(manager1, g_kvdbParameter1, option);
    ASSERT_TRUE(manager1 != nullptr && result1 != nullptr);
    EXPECT_TRUE(result1->Rekey(g_passwd2) == OK);
    EXPECT_TRUE(manager1->CloseKvStore(result1) == OK);
    result1 = nullptr;
    EXPECT_TRUE(manager1->DeleteKvStore(STORE_ID_1) == OK);
    delete manager1;
    manager1 = nullptr;
}

void RunDbRekeyTwo()
{
    KvStoreDelegateManager *manager2 = nullptr;
    KvStoreDelegate *result2 = nullptr;
    KvOption option;
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_2;
    result2 = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter2, option);
    ASSERT_TRUE(manager2 != nullptr && result2 != nullptr);
    EXPECT_TRUE(result2->Rekey(g_passwd1) == OK);
    EXPECT_TRUE(manager2->CloseKvStore(result2) == OK);
    result2 = nullptr;
    EXPECT_TRUE(manager2->DeleteKvStore(STORE_ID_2) == OK);
    delete manager2;
    manager2 = nullptr;
}

void RunDbRekeyThree()
{
    KvStoreDelegateManager *manager3 = nullptr;
    KvStoreDelegate *result3 = nullptr;
    KvOption option;
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result3 = DistributedTestTools::GetDelegateSuccess(manager3, g_kvdbParameter3, option);
    ASSERT_TRUE(manager3 != nullptr && result3 != nullptr);
    EXPECT_TRUE(result3->Rekey(NULL_PASSWD) == OK);
    EXPECT_TRUE(manager3->CloseKvStore(result3) == OK);
    result3 = nullptr;
    EXPECT_TRUE(manager3->DeleteKvStore(STORE_ID_3) == OK);
    delete manager3;
    manager3 = nullptr;
}

void RunDbRekeyFour()
{
    KvStoreDelegateManager *manager4 = nullptr;
    KvStoreDelegate *result4 = nullptr;
    KvOption option;
    result4 = DistributedTestTools::GetDelegateSuccess(manager4, g_kvdbParameter4, option);
    ASSERT_TRUE(manager4 != nullptr && result4 != nullptr);
    EXPECT_TRUE(result4->Rekey(g_passwd1) == OK);
    EXPECT_TRUE(manager4->CloseKvStore(result4) == OK);
    result4 = nullptr;
    EXPECT_TRUE(manager4->DeleteKvStore(STORE_ID_4) == OK);
    delete manager4;
    manager4 = nullptr;
}

void RunDbRekeyFive()
{
    KvStoreDelegateManager *manager5 = nullptr;
    KvStoreDelegate *result5 = nullptr;
    KvOption option;
    result5 = DistributedTestTools::GetDelegateSuccess(manager5, g_kvdbParameter5, option);
    ASSERT_TRUE(manager5 != nullptr && result5 != nullptr);
    vector<Entry> entries1;
    vector<Key> allKey1, allKey2;
    GenerateRecords(BATCH_RECORDS, DEFAULT_START, allKey1, entries1, K_SEARCH_3);
    DBStatus status = DistributedTestTools::PutBatch(*result5, entries1);
    ASSERT_TRUE(status == DBStatus::OK);
    DBStatus statusDelete = DistributedTestTools::DeleteBatch(*result5, allKey1);
    ASSERT_TRUE(statusDelete == DBStatus::OK);
    status = DistributedTestTools::Put(*result5, KEY_1, VALUE_1);
    ASSERT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*result5, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
    EXPECT_TRUE(manager5->CloseKvStore(result5) == OK);
    result5 = nullptr;
    EXPECT_TRUE(manager5->DeleteKvStore(STORE_ID_5) == OK);
    delete manager5;
    manager5 = nullptr;
}
/*
 * @tc.name: RekeyDb 005
 * @tc.desc: verify that calling Rekey interfaces on different DBs does not affect each other..
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvCreateTest, RekeyDb005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create thread1 to create db1 with passwd=p1, call Rekey to update passwd to p2=PASSSWD_2.
     * @tc.expected: step1. operate successfully.
     */
    thread subThread1(RunDbRekeyOne);

    /**
     * @tc.steps: step2. create thread2 to create db2 with passwd=p2, call Rekey to update passwd to p1=PASSSWD_1.
     * @tc.expected: step2. operate successfully.
     */
    thread subThread2(RunDbRekeyTwo);

    /**
     * @tc.steps: step3. create thread3 to create db3 with passwd=p1, call Rekey to update passwd to NULL_PASSWD.
     * @tc.expected: step3. operate successfully.
     */
    thread subThread3(RunDbRekeyThree);

    /**
     * @tc.steps: step4. create thread4 to create db4 without passwd, call Rekey to make db to be encrypted.
     * @tc.expected: step4. operate successfully.
     */
    thread subThread4(RunDbRekeyFour);

    /**
     * @tc.steps: step5. create thread5 to create db5 without passwd, then CRUD data to db5.
     * @tc.expected: step5. operate successfully.
     */
    thread subThread5(RunDbRekeyFive);
    subThread1.join();
    subThread2.join();
    subThread3.join();
    subThread5.join();
    subThread4.join();
}

/*
 * @tc.name: SpaceManger 001
 * @tc.desc: verify that can calculate the space size normally with the existing databaseID.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvCreateTest, SpaceManger001, TestSize.Level1)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreDelegate *result = nullptr;
    KvOption option;
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;

    /**
     * @tc.steps: step1. call the GetKvStoreDiskSize() with storeId=store_Id_1.
     * @tc.expected: step1. call successfully and return dbSize1.
     */
    uint64_t dbSize1, dbSize2, dbSize3;
    dbSize1 = dbSize2 = dbSize3 = 0ul;
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID_1, dbSize1), OK);

    /**
     * @tc.steps: step2. put 100 (keys,values) to db that every item's size = 1K.
     * @tc.expected: step2. operate successfully.
     */
    option.createIfNecessary = false;
    ReleaseManager(manager);
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateTenThousandRecords(NB_OPERATION_NUM, DEFAULT_START, allKeys, entriesBatch);
    EXPECT_EQ(DistributedTestTools::PutBatch(*result, entriesBatch), OK);

    /**
     * @tc.steps: step3. call the GetKvStoreDiskSize() with storeId=store_Id_1.
     * @tc.expected: step3. call successfully and return dbSize2, dbSize2>dbSize1.
     */
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID_1, dbSize2), OK);
    EXPECT_GT(dbSize2, dbSize1);

    /**
     * @tc.steps: step4. delete the 100 (keys,values) that inserted in step2.
     * @tc.expected: step4. operate successfully.
     */
    EXPECT_EQ(DistributedTestTools::DeleteBatch(*result, allKeys), OK);

    /**
     * @tc.steps: step5. call the GetKvStoreDiskSize() with storeId=store_Id_1.
     * @tc.expected: step5. call successfully and return dbSize3, dbSize3>dbSize2 and dbSize3 != dbSize2.
     */
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID_1, dbSize3), OK);
    EXPECT_GT(dbSize3, dbSize2);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    ReleaseManager(manager);
}

/*
 * @tc.name: MergeRepeat 001
 * @tc.desc: verify that delete 9 items of (keys,v) that have the same value, can query remaining data's value is v.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvCreateTest, MergeRepeat001, TestSize.Level2)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreDelegate *result = nullptr;
    result = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(manager != nullptr && result != nullptr);

    /**
     * @tc.steps: step1. put 10 items of (keys,v) to db and GetKvStoreDiskSize() with storeId=store_Id_1.
     * @tc.expected: step1. put successfully and the GetKvStoreDiskSize() returns dbSize1.
     */
    uint64_t dbSize1, dbSize2;
    dbSize1 = dbSize2 = 0ul;
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID_1, dbSize1), OK);
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    DistributedDB::Entry entry;
    int putCount=0;
    entry.value.assign(TWO_M_LONG_STRING, 'v');
    GenerateTenThousandRecords(OPER_CNT_END, DEFAULT_START, allKeys, entriesBatch);
    for (vector<Entry>::iterator iter = entriesBatch.begin(); iter != entriesBatch.end(); iter++) {
        EXPECT_EQ(DistributedTestTools::Put(*result, iter->key, entry.value), OK);
        putCount++;
    }

    /**
     * @tc.steps: step2. call the GetKvStoreDiskSize() with storeId=store_Id_1.
     * @tc.expected: step2. call successfully and return dbSize2, dbSize2 > dbSize1.
     */
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID_1, dbSize2), OK);
    EXPECT_TRUE(dbSize2 > dbSize1);

    /**
     * @tc.steps: step3. delete the 10 items of (keys,v) except the sixth item.
     * @tc.expected: step3. operate successfully.
     */
    allKeys.erase(allKeys.begin() + FIVE_SECONDS);
    DBStatus statusDelete = DistributedTestTools::DeleteBatch(*result, allKeys);
    EXPECT_EQ(statusDelete, DBStatus::OK);

    /**
     * @tc.steps: step4. Get(k6).
     * @tc.expected: step4. Get(k6)=v.
     */
    Value valueResult = DistributedTestTools::Get(*result, KEY_SEARCH_6);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, entry.value));
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    delete manager;
    manager = nullptr;
}
}
