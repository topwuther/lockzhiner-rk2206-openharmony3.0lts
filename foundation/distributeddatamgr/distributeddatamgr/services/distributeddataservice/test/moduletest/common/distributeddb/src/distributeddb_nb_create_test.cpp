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
#include <fstream>
#include <condition_variable>
#include <mutex>
#include <string>

#include "distributeddb_data_generator.h"
#include "distributeddb_nb_test_tools.h"
#include "kv_store_delegate_manager.h"
#include "types.h"
#include "distributed_test_tools.h"

using namespace std;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;
using namespace std::placeholders;
static std::condition_variable g_conditionNbVar;

namespace DistributedDBNbCreate {
DistributedDB::CipherPassword g_passwd1;
DistributedDB::CipherPassword g_passwd2;
class DistributeddbNbCreateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributeddbNbCreateTest::SetUpTestCase(void)
{
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    (void)g_passwd2.SetValue(PASSWD_VECTOR_2.data(), PASSWD_VECTOR_2.size());
}

void DistributeddbNbCreateTest::TearDownTestCase(void)
{
}

void DistributeddbNbCreateTest::SetUp(void)
{
    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());
}

void DistributeddbNbCreateTest::TearDown(void)
{
}

const KvStoreConfig CONFIG = {
    .dataDir = NB_DIRECTOR
};

void WaitingDeletingDB()
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    /**
     * @tc.steps: step2. delete db when kill the process manually.
     * @tc.expected: step2. delete success.
     */
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), OK);
    }
    MST_LOG("please kill the testing process...");
    std::this_thread::sleep_for(std::chrono::seconds(TWENTY_SECONDS));
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 001
 * @tc.desc: Verify that can create distributed db normally.
 * @tc.type: FUNC
 * @tc.require: SR000CQDV2
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb001, TestSize.Level1)
{
    const std::string storeId[ID_MIN_CNT] = { "STORE_ID_1", "STORE_ID_2" };
    const std::string appId[ID_MIN_CNT] = { "APP_ID_1", "APP_ID_2" };
    const std::string userId[ID_MIN_CNT] = { "USER_ID_1", "USER_ID_2" };

    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;

    /**
     * @tc.steps: step1. create NBdelegate with different normal appId, userId and create db with storeId.
     * @tc.expected: step1. each db can be created successfully.
     */
    for (unsigned int xCnt = ID_CNT_START; xCnt < ID_MIN_CNT; xCnt++) {
        for (unsigned int yCnt = ID_CNT_START; yCnt < ID_MIN_CNT; yCnt++) {
            for (unsigned int zCnt = ID_CNT_START; zCnt < ID_MIN_CNT; zCnt++) {
                const DBParameters dbParameters(storeId[xCnt], appId[yCnt], userId[zCnt]);
                result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, dbParameters, g_option);
                ASSERT_TRUE(manager != nullptr && result != nullptr);
                EXPECT_TRUE(EndCaseDeleteDB(manager, result, storeId[xCnt], g_option.isMemoryDb));
            }
        }
    }
}

/*
 * @tc.name: ManagerDb 002
 * @tc.desc: Verify that can create distributed with inexistence appid and userid.
 * @tc.type: FUNC
 * @tc.require: SR000CQDV2
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb002, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;

    /**
     * @tc.steps: step1. create NBdelegate with not exist appId, userId and create db with storeId.
     * @tc.expected: step1. create db failed.
     */
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, DB_PARAMETER_0_1, g_option);
    ASSERT_EQ(result, nullptr);
    EXPECT_EQ(manager->CloseKvStore(result), INVALID_ARGS);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID), INVALID_ARGS);
    delete manager;
    manager = nullptr;
    result = nullptr;
}

/*
 * @tc.name: ManagerDb 003
 * @tc.desc: test and verify that Set normal db path and can create distributed.
 * @tc.type: FUNC
 * @tc.require: SR000CQDV2
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb003, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;

    /**
     * @tc.steps: step1. Set normal path create db.
     * @tc.expected: step1. db can be successfully created.
     */
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(manager, result, STORE_ID_1, g_option.isMemoryDb));
}

void DefinePath(string &pathOk, string &pathError)
{
    pathOk =
        "/data/test/getstub/dddddddddddddddddddddddddddddddddddssssssssssssssfffffDddddddddssssssssssssssssssfffffd/"
        "dddddddddddddddddddddddddssssssssssssssssssfffffDdddddddddddddddddddddddddssssssssssssssssssfffffd/dddddddd"
        "dddddddddddddddddssssssssssssssssssfffffDdddddddddddddddddddddddddssssssssssssssssssfffffd/dddddddddddddddd"
        "dddddddddssssssssssssssssssfffffDdddddddddddddddddddddddddssssssssssssssssssfffffd/dddddddddddddddddddddddd"
        "dssssssssssssssssssfffffDdddddddddddddddddddddddddssssssssssssssssssfffffd/eserweiaa";
    MST_LOG("pathOk.length() = %zd", pathOk.length());
    pathError =
        "/data/test/getstub/ddddddddddddddddddddsssssssssssssfffffDdddddddddddddddddddddddssssssssssssssssssfffffd/"
        "qdddddddddddddddddddddddssssssssssssssssssfffffDdddddddddddddddddddddddddssssssssssssssssssfffffd/qddddddd"
        "dddddddddddddddddssssssssssssssssssfffffDdddddddddddddddddddddddddssssssssssssssssssfffffd/qdddddddddddddd"
        "ddddddddddssssssssssssssssssfffffDdddddddddddddddddddddddddssssssssssssssssssfffffd/qddddddddddddddddddddd"
        "dddssssssssssssssssssfffffDdddddddddddddddddddddddddssssssssssssssssssfffffd/qserweiaaaww";
    MST_LOG("pathError.length() = %zd", pathError.length());
    return;
}
/*
 * @tc.name: ManagerDb 004
 * @tc.desc: test and verify that Set abnormal db path and can't create distributeddb.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Construct different valid path, which contains, '\0', up letter， low letter, data,
     *      '_', or '\\' '//' '&' '^' '%' '#' and length equals to 512 characters. invalid path include null,
     *      longer than 512 characters and not exist path.
     * @tc.expected: step1. Construct success.
     */
    string pathOk = "", pathError = "";
    DefinePath(pathOk, pathError);
    const string nbDirectory[DIR_MAX_CNT] = {
        "", "/data/test/getstub/ddddddddddddddddddd/idata/nbstub\0fdfg/", pathOk, pathError,
        "/data/test/getstub/ddddddddddddddddddd/idata/..nbstub/",
        "/data/test/getstub/ddddddddddddddddddd/idata/…nbstub/",
        "/data/test/getstub/ddddddddddddddddddd/idata/分布式数据库/",
        "/data/test/getstub/ddddddddddddddddddd/idata/nbstubÄäÖöÜü/",
        "/data/test/getstub/ddddddddddddddddddd/idata/nbstub\\\\/",
        "/data/test/getstub/ddddddddddddddddddd/idata/nbstub///",
        "/data/test/getstub/ddddddddddddddddddd/idata/nbstub&/",
        "/data/test/getstub/ddddddddddddddddddd/idata/nbstub^/",
        "/data/test/getstub/ddddddddddddddddddd/idata/nbstub%/",
        "/data/test/getstub/ddddddddddddddddddd/idata/nbstub#/" };
    const string notExistPath = "/data/test/getstub/dddddddddrwddddddd/idqta/nbs";
    DistributedDB::DBStatus resultStatus[DIR_MAX_CNT] = {
        INVALID_ARGS, OK, OK, INVALID_ARGS, OK,
        OK, OK, OK, OK, OK,
        OK, OK, OK, OK };
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager("APP_ID_1", "USER_ID_1");
    ASSERT_NE(manager, nullptr);
    /**
     * @tc.steps: step2. Set different path construct above, and create db.
     * @tc.expected: step2. return INVALID_ARGS if path invalid, otherwise can return ok.
     */
    DBStatus status;
    for (unsigned int index = DIR_CNT_START; index < DIR_MAX_CNT; index++) {
        MST_LOG("index %d", index);
        SetDir(nbDirectory[index]);
        KvStoreConfig config = { .dataDir = nbDirectory[index] };
        status = manager->SetKvStoreConfig(config);
        MST_LOG("config.dataDir = %s", config.dataDir.c_str());
        MST_LOG("index[%d], status[%d], expect[%d]", index, status, resultStatus[index]);
        EXPECT_EQ(status, resultStatus[index]);
        chdir("/");
        MST_LOG("\n");
    }
    /**
     * @tc.steps: step3. verify that path do not exist, and create db.
     * @tc.expected: step3. return INVALID_ARGS.
     */
    KvStoreConfig config = { .dataDir = notExistPath };
    status = manager->SetKvStoreConfig(config);
    MST_LOG("config.dataDir = %s", config.dataDir.c_str());
    EXPECT_EQ(status, INVALID_ARGS);

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 006
 * @tc.desc: verify that db can created uses IS_NEED_CREATE mode.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb006, TestSize.Level0)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    SetDir(NB_DIRECTOR);
    ASSERT_EQ(manager->SetKvStoreConfig(CONFIG), DBStatus::OK);
    ASSERT_EQ(manager->DeleteKvStore(STORE_ID_1), DBStatus::NOT_FOUND);
    delete manager;
    manager = nullptr;

    /**
     * @tc.steps: step1. create db use IS_NEED_CREATE mode.
     * @tc.expected: step1. Create success.
     */
    KvStoreNbDelegate *result = nullptr;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(manager, result, STORE_ID_1, g_option.isMemoryDb));
}

/*
 * @tc.name: ManagerDb 007
 * @tc.desc: verify that db can't be created if it inexist and opened when it exist if it uses IS_NOT_NEED_CREATE mode.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb007, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;

    /**
     * @tc.steps: step1. db inexist and create it use IS_NOT_NEED_CREATE mode.
     * @tc.expected: step1. Create failed.
     */
    Option option = g_option;
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1_2_1, option);
    ASSERT_EQ(manager, nullptr);
    ASSERT_EQ(result, nullptr);
}

/*
 * @tc.name: ManagerDb 008
 * @tc.desc: verify that different storeids can create different dbs.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb008, TestSize.Level1)
{
    const std::string storeId[ID_MEDIUM_CNT] = { "STORE_ID_1", "STORE_ID_2", "STORE_ID_3" };
    const std::string appId[ID_MEDIUM_CNT] = { "APP_ID_1", "APP_ID_2", "APP_ID_3" };
    const std::string userId[ID_MEDIUM_CNT] = { "USER_ID_1", "USER_ID_2", "USER_ID_3" };
    /**
     * @tc.steps: step1. there isn't db storeId1, storeId2, storeId3, create db with different appIds and userIds.
     * @tc.expected: step1. Create db storeId1, storeId2, storeId3 success.
     */
    DelegateMgrNbCallback delegateMgrCallback;
    function<void(DBStatus, KvStoreNbDelegate*)> Nbfunction
        = bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback, _1, _2);
    SetDir(CONFIG.dataDir);
    DBStatus status;
    KvStoreNbDelegate::Option option = DistributedDBNbTestTools::TransferNbOptionType(g_option);
    for (unsigned int index = 0; index < ID_MEDIUM_CNT; index++) {
        KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(appId[index], userId[index]);
        ASSERT_NE(manager, nullptr);
        status = manager->SetKvStoreConfig(CONFIG);
        EXPECT_EQ(status, OK);
        manager->GetKvStore(storeId[index], option, Nbfunction);
        EXPECT_EQ(delegateMgrCallback.GetStatus(), OK);
        KvStoreNbDelegate *delegate = const_cast<KvStoreNbDelegate*>(delegateMgrCallback.GetKvStore());
        EXPECT_NE(delegate, nullptr);
        EXPECT_EQ(manager->CloseKvStore(delegate), OK);
        delegate = nullptr;
        EXPECT_EQ(manager->DeleteKvStore(storeId[index]), OK);
        delete manager;
        manager = nullptr;
    }
}

/*
 * @tc.name: ManagerDb 009
 * @tc.desc: verify that don't set dir create db failed.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb009, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. didn't set datadir, delegate can create success but db create failed.
     * @tc.expected: step1. Create failed.
     */
    DelegateMgrNbCallback delegateMgrCallback;
    function<void(DBStatus, KvStoreNbDelegate*)> Nbfunction
        = bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback, _1, _2);
    manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    KvStoreNbDelegate::Option option = DistributedDBNbTestTools::TransferNbOptionType(g_option);
    manager->GetKvStore(STORE_ID_1, option, Nbfunction);
    EXPECT_NE(delegateMgrCallback.GetStatus(), OK);
    KvStoreNbDelegate *delegate = const_cast<KvStoreNbDelegate*>(delegateMgrCallback.GetKvStore());
    EXPECT_EQ(delegate, nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), INVALID_ARGS);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), INVALID_ARGS);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 010
 * @tc.desc: verify that different delegate can create different storeid.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb010, TestSize.Level1)
{
    const std::string storeId[STORE_CNT] = { "STORE_ID_1", "STORE_ID_2", "STORE_ID_3", "STORE_ID_4" };
    KvStoreDelegateManager *manager[STORE_CNT] = { nullptr };
    KvStoreNbDelegate *callbackKvStore[STORE_CNT] = { nullptr };
    DelegateMgrNbCallback delegateMgrCallback[STORE_CNT];

    function<void(DBStatus, KvStoreNbDelegate*)> Nbfunction[STORE_CNT] = {
        bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback[INDEX_ZEROTH], _1, _2),
        bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback[INDEX_FIRST], _1, _2),
        bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback[INDEX_SECOND], _1, _2),
        bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback[INDEX_THIRD], _1, _2)
    };
    SetDir(NB_DIRECTOR);
    /**
     * @tc.steps: step1. create NBdelegate(APP_ID_1, USER_ID_1) and set dataDir.
     * @tc.expected: step1. Create success.
     */
    manager[INDEX_ZEROTH] = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager[INDEX_ZEROTH], nullptr);
    manager[INDEX_ZEROTH]->SetKvStoreConfig({ .dataDir = NB_DIRECTOR });

    /**
     * @tc.steps: step2. create NBdelegate(APP_ID_1, USER_ID_2) and set dataDir.
     * @tc.expected: step2. Create success.
     */
    manager[INDEX_FIRST] = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_2);
    ASSERT_NE(manager[INDEX_FIRST], nullptr);
    manager[INDEX_FIRST]->SetKvStoreConfig({ .dataDir = NB_DIRECTOR });

    /**
     * @tc.steps: step3. create NBdelegate(APP_ID_2, USER_ID_1) and set dataDir.
     * @tc.expected: step3. Create success.
     */
    manager[INDEX_SECOND] = new (std::nothrow) KvStoreDelegateManager(APP_ID_2, USER_ID_1);
    ASSERT_NE(manager[INDEX_SECOND], nullptr);
    manager[INDEX_SECOND]->SetKvStoreConfig({ .dataDir = NB_DIRECTOR });
    /**
     * @tc.steps: step4. create NBdelegate(APP_ID_2, USER_ID_2) and set dataDir.
     * @tc.expected: step4. Create success.
     */
    manager[INDEX_THIRD] = new (std::nothrow) KvStoreDelegateManager(APP_ID_2, USER_ID_2);
    ASSERT_NE(manager[INDEX_THIRD], nullptr);
    manager[INDEX_THIRD]->SetKvStoreConfig({ .dataDir = NB_DIRECTOR });
    /**
     * @tc.steps: step5. use STORE_ID_1 create db.
     * @tc.expected: step5. Create success.
     */
    KvStoreNbDelegate::Option option = DistributedDBNbTestTools::TransferNbOptionType(g_option);
    manager[INDEX_ZEROTH]->GetKvStore(STORE_ID_1, option, Nbfunction[INDEX_ZEROTH]);
    EXPECT_EQ(delegateMgrCallback[INDEX_ZEROTH].GetStatus(), OK);
    callbackKvStore[INDEX_ZEROTH] = delegateMgrCallback[INDEX_ZEROTH].GetKvStore();
    EXPECT_EQ(manager[INDEX_ZEROTH]->CloseKvStore(callbackKvStore[INDEX_ZEROTH]), OK);
    /**
     * @tc.steps: step6. use STORE_ID_2 create db.
     * @tc.expected: step6. Create success.
     */
    manager[INDEX_FIRST]->GetKvStore(STORE_ID_2, option, Nbfunction[INDEX_FIRST]);
    EXPECT_EQ(delegateMgrCallback[INDEX_FIRST].GetStatus(), OK);
    callbackKvStore[INDEX_FIRST] = delegateMgrCallback[INDEX_FIRST].GetKvStore();
    EXPECT_EQ(manager[INDEX_FIRST]->CloseKvStore(callbackKvStore[INDEX_FIRST]), OK);
    /**
     * @tc.steps: step7. use STORE_ID_3 create db.
     * @tc.expected: step7. Create success.
     */
    manager[INDEX_SECOND]->GetKvStore(STORE_ID_3, option, Nbfunction[INDEX_SECOND]);
    EXPECT_EQ(delegateMgrCallback[INDEX_SECOND].GetStatus(), OK);
    callbackKvStore[INDEX_SECOND] = delegateMgrCallback[INDEX_SECOND].GetKvStore();
    EXPECT_EQ(manager[INDEX_SECOND]->CloseKvStore(callbackKvStore[INDEX_SECOND]), OK);
    /**
     * @tc.steps: step8. use STORE_ID_4 create db.
     * @tc.expected: step8. Create success.
     */
    manager[INDEX_THIRD]->GetKvStore(STORE_ID_4, option, Nbfunction[INDEX_THIRD]);
    EXPECT_EQ(delegateMgrCallback[INDEX_THIRD].GetStatus(), OK);
    callbackKvStore[INDEX_THIRD] = delegateMgrCallback[INDEX_THIRD].GetKvStore();
    EXPECT_EQ(manager[INDEX_THIRD]->CloseKvStore(callbackKvStore[INDEX_THIRD]), OK);

    for (unsigned int index = 0; index < STORE_CNT; index++) {
        EXPECT_EQ(manager[index]->DeleteKvStore(storeId[index]), OK);
        delete manager[index];
        manager[index] = nullptr;
    }
}

/*
 * @tc.name: ManagerDb 011
 * @tc.desc: verify whether it will check if storeid is invalid.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb011, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Construct valid storeId, length equals to 128 characters, and which contains '\0',
     *    up letter and low letter, invalid storeId which include null, or contains '\\' '//' '&' '^' '%' '#',
     *    '..', '……', chinese， latins or longer than 128 characters.
     * @tc.expected: step1. Construct success.
     */
    KvStoreDelegateManager *manager = nullptr;
    const std::string STORE_OK =
        "STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE"\
        "_OK1_STORE_OK1_STORE_OK";
    const std::string STORE_ERROR =
        "STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_"\
        "ERR_STORE_ERR_STORE_ERR";
    const std::string STORE_ID[ID_MAX_CNT] = { "", "STORE_ID_1\0", STORE_OK, STORE_ERROR, "Store_ID_2",
        "STORE_ID_1\\", "STORE_ID_1//", "STORE_I&D_1", "STORE_ID_1^", "STORE_ID%_1",
        "STORE#_ID_1", "STORE_I+D_1", "STORE_拜拜", "STOREÄäÖöÜü_ID_", " b", "store~Id" };
    DistributedDB::DBStatus resultStatus[ID_MAX_CNT] = {
        INVALID_ARGS, OK, OK, INVALID_ARGS, OK,
        INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, INVALID_ARGS,
        INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, INVALID_ARGS };
    SetDir(CONFIG.dataDir);

    /**
     * @tc.steps: step2. Use different storeid create db.
     * @tc.expected: step2. return ok if storeid is valid, and return INVALID_ARGS if storeId is invalid.
     */
    DelegateMgrNbCallback delegateMgrNbCallback;
    function<void(DBStatus, KvStoreNbDelegate*)> Nbfunction
        = bind(&DelegateMgrNbCallback::Callback, &delegateMgrNbCallback, _1, _2);
    manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    manager->SetKvStoreConfig(CONFIG);
    KvStoreNbDelegate::Option option = DistributedDBNbTestTools::TransferNbOptionType(g_option);
    for (unsigned int index = 0; index < ID_MAX_CNT; index++) {
        MST_LOG("index: %d", index);
        manager->GetKvStore(STORE_ID[index], option, Nbfunction);
        EXPECT_EQ(delegateMgrNbCallback.GetStatus(), resultStatus[index]);
        KvStoreNbDelegate *delegate = const_cast<KvStoreNbDelegate*>(delegateMgrNbCallback.GetKvStore());
        EXPECT_EQ(manager->CloseKvStore(delegate), resultStatus[index]);
        delegate = nullptr;
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID[index]), resultStatus[index]);
    }
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 013
 * @tc.desc: verify that open db with IS_NEED_CREATE and IS_NOT_NEED_CREATE mode.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb013, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    /**
     * @tc.steps: step1. open db with IS_NOT_NEED_CREATE mode .
     * @tc.expected: step1. open failed.
     */
    Option option = g_option;
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_EQ(manager, nullptr);
    ASSERT_EQ(result, nullptr);

    /**
     * @tc.steps: step2. open db with IS_NEED_CREATE mode .
     * @tc.expected: step2. open success.
     */
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(manager, result, STORE_ID_1, g_option.isMemoryDb));
}

void GetAndCloseKvStore(KvStoreDelegateManager *&manager, const std::string &storeId,
    KvStoreNbDelegate::Option &option, function<void(DBStatus, KvStoreNbDelegate*)> &nbfunction,
    DelegateMgrNbCallback &delegateMgrCallback)
{
    manager->GetKvStore(storeId, option, nbfunction);
    EXPECT_EQ(delegateMgrCallback.GetStatus(), OK);
    KvStoreNbDelegate *delegate = const_cast<KvStoreNbDelegate *>(delegateMgrCallback.GetKvStore());
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;
}
/*
 * @tc.name: ManagerDb 014
 * @tc.desc: verify that open db reduplicatedly.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb014, TestSize.Level1)
{
    KvStoreDelegateManager *manager = nullptr;
    DelegateMgrNbCallback delegateMgrCallback;
    function<void(DBStatus, KvStoreNbDelegate*)> Nbfunction
        = bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback, _1, _2);
    manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    SetDir(CONFIG.dataDir);
    EXPECT_EQ(manager->SetKvStoreConfig(CONFIG), OK);

    KvStoreNbDelegate::Option option;
    for (int cnt = 0; cnt < MANYTINES; cnt++) {
        /**
         * @tc.steps: step1. open db with IS_NEED_CREATE mode.
         * @tc.expected: step1. open success.
         */
        option = DistributedDBNbTestTools::TransferNbOptionType(g_option);
        GetAndCloseKvStore(manager, STORE_ID_1, option, Nbfunction, delegateMgrCallback);

        /**
         * @tc.steps: step2. open db with IS_NOT_NEED_CREATE mode.
         * @tc.expected: step2. open success.
         */
        option.createIfNecessary = IS_NOT_NEED_CREATE;
        GetAndCloseKvStore(manager, STORE_ID_1, option, Nbfunction, delegateMgrCallback);
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    }

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 015
 * @tc.desc: verify that check storeid when open db.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Construct valid storeId, length equals to 128 characters, and which contains '\0',
     *    up letter and low letter, invalid storeId which include null, or contains '\\' '//' '&' '^' '%' '#',
     *    '..', '……', chinese， latins or longer than 128 characters.
     * @tc.expected: step1. Construct success.
     */
    const std::string STORE_OK =
        "STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1_STORE_OK1"\
        "_STORE_OK1_STORE_OK1_STORE_OK";
    const std::string STORE_ERROR =
        "STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR_STORE_ERR"\
        "_STORE_ERR_STORE_ERR_STORE_ERR";
    const std::string STORE_ID[ID_MAX_CNT] = { "", "STORE_ID_1\0", STORE_OK, STORE_ERROR, "Store_ID_2",
        "STORE_ID_1\\", "STORE_ID_1//", "STORE_ID_1&", "STORE_ID_1^", "STORE_%ID_1",
        "STORE_ID_1#", "STORE_I-D_1", "STORE_编号", "STORE_ID_ÄäÖöÜü", " b", "STORE_ID_1" };
    DistributedDB::DBStatus resultStatus[ID_MAX_CNT] = {
        INVALID_ARGS, OK, OK, INVALID_ARGS, OK,
        INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, INVALID_ARGS,
        INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, INVALID_ARGS, OK };
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_NE(delegate, nullptr);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;

    /**
     * @tc.steps: step2. Use different storeid open and create db.
     * @tc.expected: step2. return ok if storeid is valid, and return INVALID_ARGS if storeId is invalid.
     */
    DelegateMgrNbCallback delegateMgrCallback;
    function<void(DBStatus, KvStoreNbDelegate*)> Nbfunction
        = bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback, _1, _2);
    KvStoreNbDelegate::Option option = DistributedDBNbTestTools::TransferNbOptionType(g_option);
    for (unsigned int index = 0; index < ID_MAX_CNT; index++) {
        MST_LOG("ManagerDb015 open %d db.", index);
        manager->GetKvStore(STORE_ID[index], option, Nbfunction);
        EXPECT_EQ(delegateMgrCallback.GetStatus(), resultStatus[index]);
        KvStoreNbDelegate *delegate = const_cast<KvStoreNbDelegate*>(delegateMgrCallback.GetKvStore());
        EXPECT_EQ(manager->CloseKvStore(delegate), resultStatus[index]);
        delegate = nullptr;
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID[index]), resultStatus[index]);
    }
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 018
 * @tc.desc: verify that can close db normally.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb018, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;

    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    /**
     * @tc.steps: step1. close db normally.
     * @tc.expected: step1. success.
     */
    EXPECT_TRUE(EndCaseDeleteDB(manager, result, STORE_ID_1, g_option.isMemoryDb));
}

/*
 * @tc.name: ManagerDb 019
 * @tc.desc: verify that can't close db that do not exist.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb019, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;

    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    }
    /**
     * @tc.steps: step1. close the db that is not exist.
     * @tc.expected: step1. close failed.
     */
    EXPECT_EQ(manager->CloseKvStore(result), INVALID_ARGS);

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 020
 * @tc.desc: verify that can't close db that is closed.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb020, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;

    DelegateMgrNbCallback delegateMgrCallback;
    function<void(DBStatus, KvStoreNbDelegate*)> Nbfunction
        = bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback, _1, _2);
    manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    SetDir(NB_DIRECTOR);
    manager->SetKvStoreConfig(CONFIG);
    KvStoreNbDelegate::Option option = DistributedDBNbTestTools::TransferNbOptionType(g_option);
    manager->GetKvStore(STORE_ID_1, option, Nbfunction);
    KvStoreNbDelegate *delegate = const_cast<KvStoreNbDelegate*>(delegateMgrCallback.GetKvStore());
    EXPECT_NE(delegate, nullptr);
    /**
     * @tc.steps: step1. make sure that db is closed.
     * @tc.expected: step1. close success.
     */
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    /**
     * @tc.steps: step2. close it again.
     * @tc.expected: step2. close failed.
     */
    EXPECT_EQ(manager->CloseKvStore(delegate), INVALID_ARGS);
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    }
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 023
 * @tc.desc: verify that delete the handler of the db.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb023, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    /**
     * @tc.steps: step1. close db and delete db, then check the db document manually.
     * @tc.expected: step1. close and delete db successfully, and the document is not exist.
     */
    EXPECT_TRUE(EndCaseDeleteDB(manager, result, STORE_ID_1, g_option.isMemoryDb));
}

/*
 * @tc.name: ManagerDb 024
 * @tc.desc: verify that delete db that was not closed.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb024, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    Option option = g_option;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    /**
     * @tc.steps: step1. delete the db that was not closed and check db document manually.
     * @tc.expected: step1. delete failed and db document is still exist
     */
    if (!option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), BUSY);
    }

    EXPECT_TRUE(EndCaseDeleteDB(manager, result, STORE_ID_1, option.isMemoryDb));
}

/*
 * @tc.name: ManagerDb 025
 * @tc.desc: verify that can delete db that was not open.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb025, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    /**
     * @tc.steps: step1. delete the db that was not open and check db document manually.
     * @tc.expected: step1. delete success and db document is deleted.
     */
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    }
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 026
 * @tc.desc: verify that can't delete db that was not exist.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb026, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;

    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    /**
     * @tc.steps: step1. delete the storeid that is not exist and check document manually.
     * @tc.expected: step1. return failed and db document has not changes.
     */
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), NOT_FOUND);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    }
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 027
 * @tc.desc: verify that can't delete db reduplicatedly.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb027, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    /**
     * @tc.steps: step1. close the db.
     * @tc.expected: step1. close success.
     */
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    /**
     * @tc.steps: step2. delete the db first time and check db document manually.
     * @tc.expected: step2. delete success and db document was deleted.
     */
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    }
    /**
     * @tc.steps: step2. delete the db the second time.
     * @tc.expected: step2. delete failed and db document has no changes.
     */
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), NOT_FOUND);
    }
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 028
 * @tc.desc: verify that can't open db that was deleted.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb028, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;

    DelegateMgrNbCallback delegateMgrCallback;
    function<void(DBStatus, KvStoreNbDelegate*)> Nbfunction1
        = bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback, _1, _2);

    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    /**
     * @tc.steps: step1. close the db normally.
     * @tc.expected: step1. close success.
     */
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    /**
     * @tc.steps: step2. delete the db normally.
     * @tc.expected: step2. delete success.
     */
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    }
    /**
     * @tc.steps: step3. open the db that was deleted.
     * @tc.expected: step3. opened failed.
     */
    KvStoreNbDelegate::Option option = DistributedDBNbTestTools::TransferNbOptionType(g_option);
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    manager->GetKvStore(STORE_ID_1, option, Nbfunction1);
    EXPECT_NE(delegateMgrCallback.GetStatus(), OK);
    KvStoreNbDelegate *delegate = const_cast<KvStoreNbDelegate*>(delegateMgrCallback.GetKvStore());
    EXPECT_EQ(delegate, nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), INVALID_ARGS);
    result = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), NOT_FOUND);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 029
 * @tc.desc: verify that can create db again after it was deleted.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb029, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    DelegateMgrNbCallback delegateMgrCallback;
    function<void(DBStatus, KvStoreNbDelegate*)> Nbfunction
        = bind(&DelegateMgrNbCallback::Callback, &delegateMgrCallback, _1, _2);
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    /**
     * @tc.steps: step1. close db.
     * @tc.expected: step1. close success.
     */
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    /**
     * @tc.steps: step2. delete db .
     * @tc.expected: step2. delete success.
     */
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    }
    delete manager;
    manager = nullptr;
    /**
     * @tc.steps: step2. create db after it is delete.
     * @tc.expected: step2. create success.
     */
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ManagerDb 030
 * @tc.desc: verify that can delete the db success even if the deleting process is killed when it is deleting the db.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCreateTest, ManagerDb030, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start thread.
     * @tc.expected: step1. success.
     */
    std::thread th(WaitingDeletingDB);
    th.detach();
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND));
}

/*
 * @tc.name: MemoryDb 001
 * @tc.desc: verify that can create memory db successfully.
 * @tc.type: FUNC
 * @tc.require: SR000CRAD8
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, MemoryDb001, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;

    /**
     * @tc.steps: step1. create memory db with isMemoryDb=true in SetKvStoreConfig.
     * @tc.expected: step1. create successfully.
     */
    Option option;
    option.createIfNecessary = IS_NEED_CREATE;
    option.isMemoryDb = true;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    fstream dbFile;
    dbFile.open((NB_DIRECTOR + "single_ver/main/gen_natural_store.*"), ios::in);
    if (!dbFile) {
        MST_LOG("The db file is not exist!");
    } else {
        dbFile.close();
    }

    /**
     * @tc.steps: step2. PutLocal(k1,v1) to MemoryDb.
     * @tc.expected: step2. operate successfully and GetLocal(k1)=v1.
     */
    EXPECT_EQ(result->PutLocal(KEY_1, VALUE_1), OK);
    Value valueResult;
    EXPECT_EQ(result->GetLocal(KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(result->DeleteLocal(KEY_1), OK);
    EXPECT_EQ(result->GetLocal(KEY_1, valueResult), NOT_FOUND);

    /**
     * @tc.steps: step3. Put(k1,v1) to MemoryDb.
     * @tc.expected: step3. operate successfully and Get(k1)=v1.
     */
    EXPECT_EQ(result->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(result->Get(KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(result->Delete(KEY_1), OK);
    EXPECT_EQ(result->Get(KEY_1, valueResult), NOT_FOUND);

    /**
     * @tc.steps: step4. delete MemoryDb.
     * @tc.expected: step4. delete failed.
     */
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), BUSY);

    /**
     * @tc.steps: step5. close MemoryDb.
     * @tc.expected: step5. close successfully.
     */
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    delete manager;
    manager = nullptr;
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
 * @tc.name: MemoryDb 002
 * @tc.desc: verify that the differently configured databases(e.g. diskdb,memdb) won't affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000CRAD8
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, MemoryDb002, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result1 = nullptr, *result2 = nullptr;

     /**
     * @tc.steps: step1. Open diskdb and put(k1,v1),putlocal(k1,v2) to diskdb. Then create memorydb with the same 3-
     *    params of opened diskdb.
     * @tc.expected: step1. open the diskdb successfully and failed to open the corresponding memdb.
     */
    Option option;
    option.createIfNecessary = IS_NEED_CREATE;
    option.isMemoryDb = false;
    result1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result1 != nullptr);
    EXPECT_EQ(result1->PutLocal(KEY_1, VALUE_2), OK);
    EXPECT_EQ(result1->Put(KEY_1, VALUE_1), OK);
    option.isMemoryDb = true;
    KvStoreDelegateManager *managerRes = nullptr;
    ASSERT_EQ(DistributedDBNbTestTools::GetNbDelegateSuccess(managerRes, g_dbParameter1, option), nullptr);
    DistributedDBNbTestTools::CloseNbAndRelease(manager, result1);

    /**
     * @tc.steps: step2. device A put(k2,v3),putlocal(k2,v2) to memory db and open disk db then close memory db.
     * @tc.expected: step2. put successfully and open disk db failed.
     */
    option.isMemoryDb = true;
    result2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result2 != nullptr);
    EXPECT_EQ(result2->PutLocal(KEY_2, VALUE_2), OK);
    EXPECT_EQ(result2->Put(KEY_2, VALUE_3), OK);
    option.isMemoryDb = false;
    ASSERT_EQ(DistributedDBNbTestTools::GetNbDelegateSuccess(managerRes, g_dbParameter1, option), nullptr);
    DistributedDBNbTestTools::CloseNbAndRelease(manager, result2);

     /**
     * @tc.steps: step3. device A Get(k1),Get(k2),GetLocal(k1),GetLocal(k2) in disk db.
     * @tc.expected: step3. Get(k1)=v1, Get(k2)=NOT_FOUND, GetLocal(k1)=v2, GetLocal(k2)=NOT_FOUND.
     */
    option.isMemoryDb = false;
    result1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result1 != nullptr);
    Value valueResult;
    EXPECT_EQ(result1->GetLocal(KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);
    EXPECT_EQ(result1->GetLocal(KEY_2, valueResult), NOT_FOUND);
    EXPECT_EQ(result1->Get(KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(result1->Get(KEY_2, valueResult), NOT_FOUND);
    DistributedDBNbTestTools::CloseNbAndRelease(manager, result1);
    /**
     * @tc.steps: step4. device A Get(k1),Get(k2),GetLocal(k1),GetLocal(k2) in memory db.
     * @tc.expected: step4. Get(k1)=NOT_FOUND, Get(k2)=NOT_FOUND, GetLocal(k1)=NOT_FOUND, GetLocal(k2)=NOT_FOUND.
     */
    option.isMemoryDb = true;
    result2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result2 != nullptr);
    EXPECT_EQ(result2->GetLocal(KEY_1, valueResult), NOT_FOUND);
    EXPECT_EQ(result2->GetLocal(KEY_2, valueResult), NOT_FOUND);
    EXPECT_EQ(result2->Get(KEY_1, valueResult), NOT_FOUND);
    EXPECT_EQ(result2->Get(KEY_2, valueResult), NOT_FOUND);
    EXPECT_EQ(manager->CloseKvStore(result2), OK);
    result2 = nullptr;

    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    result1 = nullptr;
    ReleaseManager(manager);
}

/*
 * @tc.name: OptionParam 001
 * @tc.desc: verify that will check the option parameter when create encrypted DB.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, OptionParam001, TestSize.Level0)
{
    vector<KvStoreDelegateManager*> manager = {nullptr, nullptr, nullptr};
    vector<KvStoreNbDelegate*> result = {nullptr, nullptr, nullptr, nullptr, nullptr};
    Option option;
    vector<uint8_t> password;

    /**
     * @tc.steps: step1. isEncryptedDb=true, passwd=NULL, cipher=DEFAULT when create db.
     * @tc.expected: step1. create failed and return INVALID_ARGS.
     */
    option.isEncryptedDb = true;
    option.cipher = CipherType::DEFAULT;
    option.passwd = NULL_PASSWD_VECTOR;
    DBStatus status;
    result[INDEX_ZEROTH] = DistributedDBNbTestTools::GetNbDelegateStatus(manager[INDEX_ZEROTH],
        status, g_dbParameter1, option);
    EXPECT_EQ(result[INDEX_ZEROTH], nullptr);
    EXPECT_EQ(status, INVALID_ARGS);

    /**
     * @tc.steps: step2. isEncryptedDb=true, passwd=a......(100B) when create db.
     * @tc.expected: step2. create successfully and return OK.
     */
    password.assign(VALUE_ONE_HUNDRED_BYTE, 'a');
    option.passwd = password;
    result[INDEX_FIRST] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager[INDEX_ZEROTH], g_dbParameter1, option);
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
    result[INDEX_SECOND] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager[INDEX_FIRST], g_dbParameter2, option);
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
    result[INDEX_THIRD] = DistributedDBNbTestTools::GetNbDelegateStatus(manager[INDEX_SECOND],
        status, g_dbParameter3, option);
    EXPECT_EQ(result[INDEX_THIRD], nullptr);
    EXPECT_EQ(status, INVALID_ARGS);

    /**
     * @tc.steps: step5. isEncryptedDb=false, passwd=a......(129B), cipher=DEFAULT when create db.
     * @tc.expected: step5. create successfully and return OK.
     */
    option.isEncryptedDb = false;
    result[INDEX_FORTH] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager[INDEX_SECOND], g_dbParameter3, option);
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
 * @tc.desc: verify that will memDb can't be encrypted.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, OptionParam002, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    Option option;
    vector<uint8_t> password;

    /**
     * @tc.steps: step1. isEncryptedDb=true, passwd=a......(100B), cipher=DEFAULT when create memdb.
     * @tc.expected: step1. create failed and return NOT_SUPPORT.
     */
    option.isEncryptedDb = true;
    option.isMemoryDb = true;
    option.cipher = CipherType::DEFAULT;
    password.assign(VALUE_ONE_HUNDRED_BYTE, 'a');
    option.passwd = password;
    DBStatus status;
    result = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter1, option);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(status, NOT_SUPPORT);

    /**
     * @tc.steps: step2. isEncryptedDb=false, passwd=a......(100B) when create memdb.
     * @tc.expected: step2. create successfully and return OK.
     */
    option.isEncryptedDb = false;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);

    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: OptionParam 003
 * @tc.desc: verify that isEncryptedDb and passwd are consistent with the creation time can open an existing DB.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, OptionParam003, TestSize.Level0)
{
    vector<KvStoreDelegateManager *>manager = {nullptr, nullptr, nullptr, nullptr};
    KvStoreNbDelegate *result = nullptr;

    /**
     * @tc.steps: step1. isEncryptedDb=true, passwd=p1, cipher=DEFAULT when create db1.
     * @tc.expected: step1. create successfully and return OK.
     */
    Option option1 = {true, false, true, DistributedDB::CipherType::DEFAULT, PASSWD_VECTOR_1};
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager[INDEX_ZEROTH], g_dbParameter1, option1);
    ASSERT_TRUE(manager[INDEX_ZEROTH] != nullptr && result != nullptr);
    EXPECT_EQ(manager[INDEX_ZEROTH]->CloseKvStore(result), OK);
    result = nullptr;
    /**
     * @tc.steps: step2. isEncryptedDb=false, passwd=p1 when open db1.
     * @tc.expected: step2. open failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    Option option2 = {false, false, false, DistributedDB::CipherType::DEFAULT, PASSWD_VECTOR_1};
    DBStatus status;
    result = DistributedDBNbTestTools::GetNbDelegateStatus(manager[INDEX_FIRST], status, g_dbParameter1, option2);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step3. isEncryptedDb=true, passwd=p2 when open db1.
     * @tc.expected: step3. open failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    Option option3 = {false, false, true, DistributedDB::CipherType::DEFAULT, PASSWD_VECTOR_2};
    result = DistributedDBNbTestTools::GetNbDelegateStatus(manager[INDEX_FIRST], status, g_dbParameter1, option3);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step4. isEncryptedDb=true, passwd=p1 when open db1.
     * @tc.expected: step4. open successfully and return OK.
     */
    Option option4 = {false, false, true, DistributedDB::CipherType::DEFAULT, PASSWD_VECTOR_1};
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager[INDEX_FIRST], g_dbParameter1, option4);
    ASSERT_TRUE(manager[INDEX_FIRST] != nullptr && result != nullptr);
    EXPECT_EQ(manager[INDEX_FIRST]->CloseKvStore(result), OK);
    result = nullptr;

    /**
     * @tc.steps: step5. isEncryptedDb=false, passwd=p1, cipher=DEFAULT when create db2.
     * @tc.expected: step5. create successfully and return OK.
     */
    Option option5 = {true, false, false, DistributedDB::CipherType::DEFAULT, PASSWD_VECTOR_1};
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager[INDEX_SECOND], g_dbParameter2, option5);
    ASSERT_TRUE(manager[INDEX_SECOND] != nullptr && result != nullptr);
    EXPECT_EQ(manager[INDEX_SECOND]->CloseKvStore(result), OK);
    result = nullptr;

    /**
     * @tc.steps: step6. isEncryptedDb=true, passwd=p1 when open db2.
     * @tc.expected: step6. open failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    Option option6 = {false, false, true, DistributedDB::CipherType::DEFAULT, PASSWD_VECTOR_1};
    result = DistributedDBNbTestTools::GetNbDelegateStatus(manager[INDEX_THIRD], status, g_dbParameter2, option6);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step7. isEncryptedDb=false, passwd=p2 when open db2.
     * @tc.expected: step7. open successfully and return OK.
     */
    Option option7 = {false, false, false, DistributedDB::CipherType::DEFAULT, PASSWD_VECTOR_2};
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager[INDEX_THIRD], g_dbParameter2, option7);
    ASSERT_TRUE(manager[INDEX_THIRD] != nullptr && result != nullptr);
    EXPECT_EQ(manager[INDEX_THIRD]->CloseKvStore(result), OK);
    result = nullptr;
    EXPECT_EQ(manager[INDEX_FIRST]->DeleteKvStore(STORE_ID_1), OK);
    EXPECT_EQ(manager[INDEX_THIRD]->DeleteKvStore(STORE_ID_2), OK);
    for (auto &item : manager) {
        if (item != nullptr) {
            delete item;
            item = nullptr;
        }
    }
}

void RekeyAndClose(KvStoreDelegateManager *&manager, KvStoreNbDelegate *&delegate,
    const DistributedDB::CipherPassword &reKeyPasswd, DistributedDB::DBStatus rekeyStatus, bool isEnd)
{
    EXPECT_EQ(delegate->Rekey(reKeyPasswd), rekeyStatus);
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    if (!isEnd) {
        ReleaseManager(manager);
    }
}

/*
 * @tc.name: RekeyDb 001
 * @tc.desc: verify that can switching a non-encrypted database to an encrypted database by Rekey.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, RekeyDb001, TestSize.Level1)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    Option option;

    /**
     * @tc.steps: step1. use Rekey to update passwd to p1 after create memdb and then close memdb.
     * @tc.expected: step1. the Rekey return NOT_SUPPORT.
     */
    option.isMemoryDb = true;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    RekeyAndClose(manager, result, g_passwd1, NOT_SUPPORT, false);
    /**
     * @tc.steps: step2. create unencrypted db, use Rekey to update its passwd to NULL,
     * then close and open without passwd.
     * @tc.expected: step2. operate successfully and can open db again without passwd.
     */
    option.isMemoryDb = false;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    RekeyAndClose(manager, result, NULL_PASSWD, OK, false);
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_EQ(result->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(result->Put(KEY_1, VALUE_1), OK);

    /**
     * @tc.steps: step3. use Rekey to update db's passwd to p1=a......(100B), then close
     * and open again without passwd.
     * @tc.expected: step3. Rekey is ok, open db failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    vector<uint8_t> passwordVector(VALUE_ONE_HUNDRED_BYTE, 'a');
    CipherPassword password;
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OK);
    RekeyAndClose(manager, result, password, OK, false);
    DBStatus status;
    EXPECT_EQ(DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter1, option), nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step4. use p1 to open db and Get(k1), GetLocal(k1).
     * @tc.expected: step4. open successfully and Get(k1)=v1, GetLocal(k1)=v1.
     */
    option.isEncryptedDb = true;
    option.passwd = passwordVector;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    Value valueResult;
    EXPECT_EQ(result->GetLocal(KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(result->Get(KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);

    /**
     * @tc.steps: step5. use Rekey to update db's passwd to p2=passwordVector whose size is 128B, then close
     * and open again with p1.
     * @tc.expected: step5. Rekey is ok, open db failed and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    passwordVector.assign(BATCH_RECORDS, 'a');
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OK);
    RekeyAndClose(manager, result, password, OK, false);
    EXPECT_EQ(DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter1, option), nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step6. use p2 to open db and delete(k1), DeleteLocal(k1).
     * @tc.expected: step6. operate successfully.
     */
    option.passwd = passwordVector;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_EQ(result->Delete(KEY_1), OK);
    EXPECT_EQ(result->DeleteLocal(KEY_1), OK);

    /**
     * @tc.steps: step7. use Rekey to update db's passwd to p2=passwordVector whose size is 129B.
     * @tc.expected: step7. Rekey failed and return INVALID_ARGS.
     */
    passwordVector.assign(PASSWD_BYTE, 'a');
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OVERSIZE);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    ReleaseManager(manager);
}

/*
 * @tc.name: RekeyDb 002
 * @tc.desc: verify that can change passwd of an encrypted database by Rekey.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, RekeyDb002, TestSize.Level1)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    Option option;

    /**
     * @tc.steps: step1. create encrypted db with p1=password@123.
     * @tc.expected: step1. create successfully.
     */
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);

    /**
     * @tc.steps: step2. use Rekey to update passwd to p1 and close db then open db again with unencrypted way.
     * @tc.expected: step2. the Rekey return OK, but open db failed.
     */
    EXPECT_EQ(result->Rekey(g_passwd1), OK);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    ReleaseManager(manager);
    option.createIfNecessary = false;
    option.isEncryptedDb = false;
    DBStatus status;
    result = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter1, option);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step3. open db with passwd=p1 and put(k1,v1), putLocal(k2,v2).
     * @tc.expected: step3. operate successfully.
     */
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    EXPECT_EQ(result->PutLocal(KEY_2, VALUE_2), OK);
    EXPECT_EQ(result->Put(KEY_1, VALUE_1), OK);

    /**
     * @tc.steps: step4. use Rekey to update passwd to NULL and close db then open db with passwd=p1.
     * @tc.expected: step4. the Rekey return OK, but open db failed.
     */
    EXPECT_EQ(result->Rekey(NULL_PASSWD), OK);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    ReleaseManager(manager);
    result = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter1, option);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(status, INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step5. open db again with unencrypted way and Get(k1), GetLocal(k2).
     * @tc.expected: step5. open successfully and Get(k1)=v1, GetLocal(k2)=v2.
     */
    option.isEncryptedDb = false;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    Value valueResult;
    EXPECT_EQ(result->Get(KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_1);
    EXPECT_EQ(result->GetLocal(KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);

    /**
     * @tc.steps: step6. use Rekey to update db's passwd to p2=a......(129B).
     * @tc.expected: step6. Reksy failed and return INVALID_ARGS.
     */
    vector<uint8_t> passwordVector(PASSWD_BYTE, 'a');
    CipherPassword password;
    EXPECT_EQ(password.SetValue(passwordVector.data(), passwordVector.size()), CipherPassword::ErrorCode::OVERSIZE);

    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
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
HWTEST_F(DistributeddbNbCreateTest, RekeyDb003, TestSize.Level3)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result1 = nullptr;
    KvStoreNbDelegate *result2 = nullptr;
    Option option;

    /**
     * @tc.steps: step1. use Rekey to update passwd to p1=PASSWD_VECTOR_1.
     * @tc.expected: step1. create successfully.
     */
    result1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result1 != nullptr);
    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, NB_OPERATION_NUM, ONE_K_LONG_STRING, ONE_M_LONG_STRING);
    for (vector<Entry>::iterator iter = entriesBatch.begin(); iter != entriesBatch.end(); iter++) {
        EXPECT_EQ(result1->Put(iter->key, iter->value), OK);
    }
    bool rekeyFlag = false;
    thread subThread([&]() {
        DBStatus rekeyStatus = result1->Rekey(g_passwd1);
        EXPECT_TRUE(rekeyStatus == OK || rekeyStatus == BUSY);
        rekeyFlag = true;
        g_conditionNbVar.notify_all();
    });
    subThread.detach();

    /**
     * @tc.steps: step2. Call the GetKvstore interface when Rekey.
     * @tc.expected: step2. the GetKvstore return BUSY.
     */
    option.createIfNecessary = false;
    DBStatus status;
    KvStoreDelegateManager *managerRes = nullptr;
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_LONG_TIME));
    result2 = DistributedDBNbTestTools::GetNbDelegateStatus(managerRes, status, g_dbParameter1, option);
    EXPECT_TRUE(status == BUSY || status == OK);
    if (status == OK) {
        EXPECT_EQ(managerRes->CloseKvStore(result2), OK);
        result2 = nullptr;
        ReleaseManager(managerRes);
    }

    /**
     * @tc.steps: step3. put data to db when Rekey.
     * @tc.expected: step3. the put return BUSY.
     */
    status = result1->Put(KEY_1, VALUE_1);
    EXPECT_TRUE(status == BUSY || status == OK);
    std::mutex count;
    std::unique_lock<std::mutex> lck(count);
    g_conditionNbVar.wait(lck, [&] { return rekeyFlag; });
    EXPECT_EQ(manager->CloseKvStore(result1), OK);
    result1 = nullptr;
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
HWTEST_F(DistributeddbNbCreateTest, RekeyDb004, TestSize.Level0)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result1 = nullptr;
    KvStoreNbDelegate *result2 = nullptr;
    Option option;

    result1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result1 != nullptr);
    ReleaseManager(manager);
    /**
     * @tc.steps: step1. use GetKvstore to open another instances of the same KvStore.
     * @tc.expected: step1. open successfully.
     */
    option.createIfNecessary = false;
    result2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result2 != nullptr);

    /**
     * @tc.steps: step2. call Rekey.
     * @tc.expected: step2. Rekey returns BUSY.
     */
    EXPECT_EQ(result1->Rekey(g_passwd1), BUSY);
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
    KvStoreNbDelegate *result1 = nullptr;
    Option option;
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager1, g_dbParameter1, option);
    ASSERT_TRUE(manager1 != nullptr && result1 != nullptr);
    EXPECT_EQ(result1->Rekey(g_passwd2), OK);
    EXPECT_EQ(manager1->CloseKvStore(result1), OK);
    result1 = nullptr;
    EXPECT_EQ(manager1->DeleteKvStore(STORE_ID_1), OK);
    delete manager1;
    manager1 = nullptr;
}

void RunDbRekeyTwo()
{
    KvStoreDelegateManager *manager2 = nullptr;
    KvStoreNbDelegate *result2 = nullptr;
    Option option;
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_2;
    result2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager2, g_dbParameter2, option);
    ASSERT_TRUE(manager2 != nullptr && result2 != nullptr);
    EXPECT_EQ(result2->Rekey(g_passwd1), OK);
    EXPECT_EQ(manager2->CloseKvStore(result2), OK);
    result2 = nullptr;
    EXPECT_EQ(manager2->DeleteKvStore(STORE_ID_2), OK);
    delete manager2;
    manager2 = nullptr;
}

void RunDbRekeyThree()
{
    KvStoreDelegateManager *manager3 = nullptr;
    KvStoreNbDelegate *result3 = nullptr;
    Option option;
    option.isEncryptedDb = true;
    option.passwd = PASSWD_VECTOR_1;
    result3 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager3, g_dbParameter3, option);
    ASSERT_TRUE(manager3 != nullptr && result3 != nullptr);
    EXPECT_EQ(result3->Rekey(NULL_PASSWD), OK);
    EXPECT_EQ(manager3->CloseKvStore(result3), OK);
    result3 = nullptr;
    EXPECT_EQ(manager3->DeleteKvStore(STORE_ID_3), OK);
    delete manager3;
    manager3 = nullptr;
}

void RunDbRekeyFour()
{
    KvStoreDelegateManager *manager4 = nullptr;
    KvStoreNbDelegate *result4 = nullptr;
    Option option;
    result4 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager4, g_dbParameter4, option);
    ASSERT_TRUE(manager4 != nullptr && result4 != nullptr);
    EXPECT_EQ(result4->Rekey(g_passwd1), OK);
    EXPECT_EQ(manager4->CloseKvStore(result4), OK);
    result4 = nullptr;
    EXPECT_EQ(manager4->DeleteKvStore(STORE_ID_4), OK);
    delete manager4;
    manager4 = nullptr;
}

void RunDbRekeyFive()
{
    KvStoreDelegateManager *manager5 = nullptr;
    KvStoreNbDelegate *result5 = nullptr;
    Option option;
    result5 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager5, g_dbParameter5, option);
    ASSERT_TRUE(manager5 != nullptr && result5 != nullptr);
    EXPECT_EQ(result5->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(result5->Delete(KEY_1), OK);
    EXPECT_EQ(result5->Put(KEY_1, VALUE_2), OK);
    Value valueResult;
    EXPECT_EQ(result5->Get(KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);
    EXPECT_EQ(manager5->CloseKvStore(result5), OK);
    result5 = nullptr;
    EXPECT_EQ(manager5->DeleteKvStore(STORE_ID_5), OK);
    delete manager5;
    manager5 = nullptr;
}
/*
 * @tc.name: RekeyDb 005
 * @tc.desc: verify that calling Rekey interfaces on different DBs does not affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, RekeyDb005, TestSize.Level1)
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
    subThread4.join();
    subThread5.join();
}

/*
 * @tc.name: SpaceManger 001
 * @tc.desc: verify that StoreID is empty or does not exist, calling the GetKvStoreDiskSize() interface failed.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, SpaceManger001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create DB1.
     * @tc.expected: step1. create successfully.
     */
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, g_option);
    ASSERT_TRUE(manager != nullptr && result != nullptr);
    /**
     * @tc.steps: step2. call the GetKvStoreDiskSize() with storeId=null.
     * @tc.expected: step2. call failed and return INVALID_ARGS.
     */
    uint64_t dbSize = 0ul;
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID, dbSize), INVALID_ARGS);

    /**
     * @tc.steps: step3. call the GetKvStoreDiskSize() with storeId=store_Id_2.
     * @tc.expected: step3. call failed and return NOT_FOUND.
     */
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID_2, dbSize), NOT_FOUND);
    EXPECT_EQ(manager->CloseKvStore(result), OK);
    result = nullptr;
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    }
    ReleaseManager(manager);
}

/*
 * @tc.name: SpaceManger 002
 * @tc.desc: verify that can calculate the space size normally with the existing databaseID.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbCreateTest, SpaceManger002, TestSize.Level2)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *result = nullptr;
    KvStoreNbDelegate *result1 = nullptr;
    Option option;
    result = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
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
    ReleaseManager(manager);

    /**
     * @tc.steps: step2. put 100 (keys,values) to db that every item's size = 1K.
     * @tc.expected: step2. operate successfully.
     */
    option.createIfNecessary = false;
    result1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result1 != nullptr);

    std::vector<DistributedDB::Entry> entriesBatch;
    std::vector<DistributedDB::Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, OPER_CNT_END, ONE_K_LONG_STRING, FOUR_M_LONG_STRING);
    for (vector<Entry>::iterator iter = entriesBatch.begin(); iter != entriesBatch.end(); iter++) {
        EXPECT_EQ(result1->Put(iter->key, iter->value), OK);
    }

    /**
     * @tc.steps: step3. call the GetKvStoreDiskSize() with storeId=store_Id_1.
     * @tc.expected: step3. call successfully and return dbSize2, dbSize2 > dbsize1.
     */
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID_1, dbSize2), OK);
    EXPECT_TRUE(dbSize2 > dbSize1);

    /**
     * @tc.steps: step4. delete the 100 (keys,values) that inserted in step2.
     * @tc.expected: step4. operate successfully.
     */
    for (vector<Entry>::iterator iter = entriesBatch.begin(); iter != entriesBatch.end(); iter++) {
        EXPECT_EQ(result1->Delete(iter->key), OK);
    }

    /**
     * @tc.steps: step5. call the GetKvStoreDiskSize() with storeId=store_Id_1.
     * @tc.expected: step5. call successfully and return dbSize3, dbSize3 < dbsize2 and dbSize3 != dbsize2.
     */
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID_1, dbSize3), OK);
    EXPECT_TRUE(dbSize2 > dbSize3);
    EXPECT_NE(dbSize3, dbSize1);
    EXPECT_EQ(manager->CloseKvStore(result1), OK);
    result1 = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    ReleaseManager(manager);

    /**
     * @tc.steps: step6. create memoryDb with storeId=store_Id_2 and call the GetKvStoreDiskSize().
     * @tc.expected: step6. call successfully and return dbSize1 and dbSize1=0.
     */
    option.createIfNecessary = true;
    option.isMemoryDb = true;
    result1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && result1 != nullptr);
    uint64_t dbSize4 = 0ul;
    EXPECT_EQ(manager->GetKvStoreDiskSize(STORE_ID_1, dbSize4), OK);
    EXPECT_EQ(dbSize4, size_t(0));
    EXPECT_EQ(manager->CloseKvStore(result1), OK);
    result1 = nullptr;
    ReleaseManager(manager);
}
}