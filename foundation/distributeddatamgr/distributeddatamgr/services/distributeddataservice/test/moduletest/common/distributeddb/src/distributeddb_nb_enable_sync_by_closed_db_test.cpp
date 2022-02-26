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
#include <chrono>
#include <string>

#include "types.h"
#include "kv_store_delegate.h"
#include "kv_store_nb_delegate.h"
#include "kv_store_delegate_manager.h"
#include "distributed_test_tools.h"
#include "distributeddb_nb_test_tools.h"
#include "distributeddb_data_generator.h"
#include "distributeddb_schema_test_tools.h"
#include "process_communicator_test_stub.h"

using namespace std;
using namespace chrono;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace std::placeholders;
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbNbEnableSyncByClosedDb {
const int PARAM_CRITICAL_LENGTH = 128;
const int STRING_ONE_BYTE = 1;
const int STRING_TEN_BYTE = 10;
const int STRING_ONE_TWO_EIGHT_BYTE = 128;
const int STRING_ONE_TWO_NINE_BYTE = 129;

class DistributeddbNbEnableSyncByClosedDbTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbEnableSyncByClosedDbTest::SetUpTestCase(void)
{
    SetDir(NB_DIRECTOR);
}

void DistributeddbNbEnableSyncByClosedDbTest::TearDownTestCase(void)
{
}

void DistributeddbNbEnableSyncByClosedDbTest::SetUp(void)
{
    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());
}

void DistributeddbNbEnableSyncByClosedDbTest::TearDown(void)
{
}
#ifndef OMIT_JSON
/*
 * @tc.name: ParamCheck 001
 * @tc.desc: test EnableKvStoreAutoLaunch and DisableKvStoreAutoLaunch interface checking 3 elements function.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JR
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, ParamCheck001, TestSize.Level0)
{
    string schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_DEFINE_1, VALID_INDEX_1);
    AutoLaunchOption option = {true, false, CipherType::DEFAULT, NULL_PASSWD, schema, true, NB_DIRECTOR, nullptr};
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig({ .dataDir = NB_DIRECTOR }), OK);
    /**
     * @tc.steps: step1. call EnableKvStoreAutoLaunch and DisableKvStoreAutoLaunch interface using the storeId such as
     *    "", or string with "\0", or length of the storeId is 128\129, or that has Uppercase letters, Lowercase
     *    letters, digit "_", or some Special characters.
     * @tc.expected: step1. only the storeId has "\0", or length is 128, or has Upper or lower letters or digit or "_"
     *    can enable success, otherwise it will return INVALID_ARGS.
     */
    string storeId1(PARAM_CRITICAL_LENGTH, 'a');
    vector<string> storeIdValid = {storeId1, "abc\0", "Abc576_"};
    for (unsigned long index = 0; index < storeIdValid.size(); index++) {
        EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, storeIdValid[index], option, nullptr), OK);
        EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, storeIdValid[index]), OK);
        EXPECT_EQ(manager->DeleteKvStore(storeIdValid[index]), OK);
    }

    string storeId2(PARAM_CRITICAL_LENGTH + 1, 'a');
    vector<string> storeIdInValid = {"", "6\\", "6//", "6&", "6^", "6%", "6#", "6-", "6中文", "6Ã„Ã¤", storeId2};
    for (unsigned long index = 0; index < storeIdInValid.size(); index++) {
        EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, storeIdInValid[index], option, nullptr),
            INVALID_ARGS);
        EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, storeIdInValid[index]), NOT_FOUND);
    }

    /**
     * @tc.steps: step2. call EnableKvStoreAutoLaunch and DisableKvStoreAutoLaunch interface using the appId such as
     *    "", or length of the storeId is 128\129.
     * @tc.expected: step2. only the storeId has the length as 128
     *    can enable success, otherwise it will return INVALID_ARGS.
     */
    string appId1(PARAM_CRITICAL_LENGTH, 'b');
    string appId2(PARAM_CRITICAL_LENGTH + 1, 'b');
    vector<string> appId = {"", appId1, appId2};
    DBStatus result[] = {INVALID_ARGS, OK, INVALID_ARGS};
    DBStatus resultDisable[] = {NOT_FOUND, OK, NOT_FOUND};
    for (unsigned long index = 0; index < appId.size(); index++) {
        EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, appId[index], STORE_ID_1, option, nullptr),
            result[index]);
        EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, appId[index], STORE_ID_1), resultDisable[index]);
    }
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);

    /**
     * @tc.steps: step3. call EnableKvStoreAutoLaunch and DisableKvStoreAutoLaunch interface using the userId such as
     *    "", or length of the storeId is 128\129.
     * @tc.expected: step3. only the storeId has the length as 128
     *    can enable success, otherwise it will return INVALID_ARGS.
     */
    string userId1(PARAM_CRITICAL_LENGTH, 'c');
    string userId2(PARAM_CRITICAL_LENGTH + 1, 'c');
    vector<string> userId = {"", appId1, appId2};
    for (unsigned long index = 0; index < userId.size(); index++) {
        EXPECT_EQ(manager->EnableKvStoreAutoLaunch(userId[index], APP_ID_1, STORE_ID_1, option, nullptr),
            result[index]);
        EXPECT_EQ(manager->DisableKvStoreAutoLaunch(userId[index], APP_ID_1, STORE_ID_1), resultDisable[index]);
    }
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);

    delete manager;
    manager = nullptr;
}
#endif
/*
 * @tc.name: ParamCheck 002
 * @tc.desc: test EnableKvStoreAutoLaunch can only effect when option.createIfNecessary=true if the DB is not exist.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JR
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, ParamCheck002, TestSize.Level0)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig({ .dataDir = NB_DIRECTOR }), OK);
    /**
     * @tc.steps: step1. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = false,
     *    dataDir = NB_DIRECTOR, observer = TheAppoitObserver;
     * @tc.expected: step1. enable failed, and return DB_ERROR.
     */
    KvStoreObserverImpl *observer = nullptr;
    AutoLaunchOption option;
    option.createIfNecessary = false;
    option.dataDir = NB_DIRECTOR;
    option.observer = observer;
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr), DB_ERROR);
    /**
     * @tc.steps: step2. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    and other settings don't change;
     * @tc.expected: step2. enable succeed, and return OK.
     */
    option.createIfNecessary = true;
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr), OK);

    /**
     * @tc.steps: step3. create db use option.createIfNecessary = false, and put (k1, v1), (k2, v2);
     * @tc.expected: step3. create and put succeed.
     */
    Option dbOption;
    dbOption.createIfNecessary = false;
    dbOption.isMemoryDb = false;
    KvStoreNbDelegate *delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, dbOption);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    EXPECT_EQ(delegate->Put(KEY_1, VALUE_1), OK);
    EXPECT_EQ(delegate->Put(KEY_2, VALUE_2), OK);

    /**
     * @tc.steps: step4. update (k1, v1) to (k1, v2), and check the data with Get interface, and then delete (k2, v2);
     * @tc.expected: step4. operate succeed.
     */
    EXPECT_EQ(delegate->Put(KEY_1, VALUE_2), OK);
    Value realValue;
    EXPECT_EQ(delegate->Get(KEY_1, realValue), OK);
    EXPECT_EQ(realValue, VALUE_2);
    EXPECT_EQ(delegate->Delete(KEY_2), OK);

    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1), OK);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ParamCheck 003
 * @tc.desc: verification of isEncrypted, cipher, passwd of AutoLaunchOption.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JR
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, ParamCheck003, TestSize.Level0)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig({ .dataDir = NB_DIRECTOR }), OK);
    /**
     * @tc.steps: step1. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    isEncryptedDb = true, but the passwd is "";
     * @tc.expected: step1. enable failed, and return INVALID_ARGS.
     */
    AutoLaunchOption option = {true, true, CipherType::DEFAULT, NULL_PASSWD, "", true, NB_DIRECTOR, nullptr};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr), INVALID_ARGS);
    /**
     * @tc.steps: step2. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    isEncryptedDb = true, and the passwd is 100 length;
     * @tc.expected: step2. enable succeed, and return OK.
     */
    CipherPassword passwd1, passwd2, passwd3;
    vector<uint8_t> passwordVector1(ENCRYPT_COUNT, 'a');
    passwd1.SetValue(passwordVector1.data(), ENCRYPT_COUNT);
    option.passwd = passwd1;
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr), OK);
    EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1), OK);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);

    /**
     * @tc.steps: step3. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    isEncryptedDb = true, the passwd is 128 length;
     * @tc.expected: step3. enable succeed, and return OK.
     */
    vector<uint8_t> passwordVector2(PARAM_CRITICAL_LENGTH, 'a');
    passwd2.SetValue(passwordVector2.data(), PARAM_CRITICAL_LENGTH);
    option.passwd = passwd2;
    option.cipher = CipherType::AES_256_GCM;
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr), OK);
    EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1), OK);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    /**
     * @tc.steps: step4. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    isEncryptedDb = true, the passwd is 129 length;
     * @tc.expected: step4. enable failed, and return INVALID_ARGS.
     */
    vector<uint8_t> passwordVector3(PASSWD_BYTE, 'a');
    passwd3.SetValue(passwordVector3.data(), PASSWD_BYTE);
    option.passwd = passwd3;
    option.cipher = CipherType::DEFAULT;
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr), INVALID_ARGS);

    /**
     * @tc.steps: step5. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    isEncryptedDb = false, the passwd is 129 length;
     * @tc.expected: step5. enable success, and return OK.
     */
    option.isEncryptedDb = false;
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr), OK);
    EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1), OK);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    delete manager;
    manager = nullptr;
}
#ifndef OMIT_JSON
/*
 * @tc.name: ParamCheck 004
 * @tc.desc: verify that isEncrypted, passwd of AutoLaunchOption must be the same to params of the db created already.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JR
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, ParamCheck004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create db with the option isEncryptedDb = true, passwd = PASSWD_VECTOR_1;
     * @tc.expected: step1. create successfully.
     */
    string schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_DEFINE_1, VALID_INDEX_1);
    KvStoreDelegateManager *manager = nullptr;
    Option dbOption;
    dbOption.isEncryptedDb = true;
    dbOption.passwd = PASSWD_VECTOR_1;
    dbOption.schema = schema;
    dbOption.isMemoryDb = false;
    KvStoreNbDelegate *delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, dbOption);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    /**
     * @tc.steps: step2. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = false,
     *    isEncryptedDb = false, the passwd is passwd1 and a valid schema;
     * @tc.expected: step2. enable failed, and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    CipherPassword passwd1, passwd2;
    passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    AutoLaunchOption option = {false, false, CipherType::DEFAULT, passwd1, schema, false, NB_DIRECTOR, nullptr};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr),
        INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step3. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = false,
     *    isEncryptedDb = true, and the passwd is passwd2;
     * @tc.expected: step3. enable failed, and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    passwd2.SetValue(PASSWD_VECTOR_2.data(), PASSWD_VECTOR_2.size());
    option = {false, true, CipherType::DEFAULT, passwd2, schema, false, NB_DIRECTOR, nullptr};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr),
        INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step4. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = false,
     *    isEncryptedDb = true, and the passwd is passwd1;
     * @tc.expected: step4. enable success, and return OK.
     */
    option = {false, true, CipherType::DEFAULT, passwd1, schema, false, NB_DIRECTOR, nullptr};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr), OK);
    EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1), OK);

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_1, dbOption.isMemoryDb));

    /**
     * @tc.steps: step5. create unEncrypted DB2, but passwd = p1;
     * @tc.expected: step5. create success.
     */
    dbOption.isEncryptedDb = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, dbOption);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step6. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = false,
     *    isEncryptedDb = true, and the passwd is passwd2;
     * @tc.expected: step6. enable failed, and return INVALID_PASSWD_OR_CORRUPTED_DB.
     */
    passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    option = {false, true, CipherType::DEFAULT, passwd1, schema, false, NB_DIRECTOR, nullptr};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_2, APP_ID_2, STORE_ID_2, option, nullptr),
        INVALID_PASSWD_OR_CORRUPTED_DB);

    /**
     * @tc.steps: step7. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = false,
     *    isEncryptedDb = false, and the passwd is passwd2;
     * @tc.expected: step7. enable success, and return OK.
     */
    passwd2.SetValue(PASSWD_VECTOR_2.data(), PASSWD_VECTOR_2.size());
    option = {false, false, CipherType::DEFAULT, passwd2, schema, false, NB_DIRECTOR, nullptr};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_2, APP_ID_2, STORE_ID_2, option, nullptr), OK);
    EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_2, APP_ID_2, STORE_ID_2), OK);

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, dbOption.isMemoryDb));
}

/*
 * @tc.name: ParamCheck 005
 * @tc.desc: verify that EnableKvStoreAutoLaunch interface would check the schema param's legitimacy.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JR
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, ParamCheck005, TestSize.Level0)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig({ .dataDir = NB_DIRECTOR }), OK);
    /**
     * @tc.steps: step1. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    isEncryptedDb = false, and a valid schema;
     * @tc.expected: step1. enable success, and return OK.
     */
    string schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_DEFINE_1, VALID_INDEX_1);
    AutoLaunchOption option = {true, false, CipherType::DEFAULT, NULL_PASSWD, schema, false, NB_DIRECTOR, nullptr};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr), OK);
    EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1), OK);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    /**
     * @tc.steps: step2. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    isEncryptedDb = false, and a invalid schema;
     * @tc.expected: step2. enable failed, and return INVALID_SCHEMA.
     */
    string inValidSchema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, INVALID_DEFINE_2, VALID_INDEX_1);
    option = {true, false, CipherType::DEFAULT, NULL_PASSWD, inValidSchema, false, NB_DIRECTOR, nullptr};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr),
        INVALID_SCHEMA);

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ParamCheck 006
 * @tc.desc: verify that EnableKvStoreAutoLaunch interface would check the dataDir param's legitimacy.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JR
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, ParamCheck006, TestSize.Level0)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig({ .dataDir = NB_DIRECTOR }), OK);
    /**
     * @tc.steps: step1. call EnableKvStoreAutoLaunch use the option with which createIfNecessary is true,
     *    isEncryptedDb = false, and different dataDir that include many special characters;
     * @tc.expected: step1. enable success, and return OK.
     */
    string middlePath = "ddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd/"
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff/"
        "gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg/";
    // the length of the valid string is 504-102 = 402
    string tailPathValid(402 - (NB_DIRECTOR + middlePath).length(), 'e');
    // the length of the invalid string is 504-101 = 403
    string tailPathInvalid(403 - (NB_DIRECTOR + middlePath).length(), 'h');
    string validLengthDir = NB_DIRECTOR + middlePath + tailPathValid;
    string inValidLengthDir = NB_DIRECTOR + middlePath + tailPathInvalid;

    vector<string> dirs = {"", NB_DIRECTOR + "a\0", validLengthDir, inValidLengthDir,
        NB_DIRECTOR + "a..b", NB_DIRECTOR + "a/…b",
        NB_DIRECTOR + "a/中文", NB_DIRECTOR + "a\\//&^a%#",
        NB_DIRECTOR + "diehgid/"};
    vector<DBStatus> results = {INVALID_ARGS, OK, OK, INVALID_ARGS, OK, OK, OK, OK, OK, OK, INVALID_ARGS};
    string schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_DEFINE_1, VALID_INDEX_1);

    AutoLaunchOption option;
    DBStatus status;
    RemoveDir("/data/test/getstub");
    for (unsigned long index = 0; index < dirs.size(); index++) {
        if (results[index] == OK) {
            SetDir(dirs[index]);
        }
        option = {true, false, CipherType::DEFAULT, NULL_PASSWD, schema, false, dirs[index], nullptr};
        status = manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, nullptr);
        if (index == 3) { // the 3th element
            EXPECT_NE(status, OK);
        } else {
            EXPECT_EQ(status, results[index]);
        }
        if (status == OK) {
            EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1), OK);
        }
        if (results[index] == OK) {
            RemoveDir(dirs[index]);
        }
    }

    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ParamCheck 007
 * @tc.desc: verify that EnableKvStoreAutoLaunch many times, it can only be successfully when the option is same as the
 *    first time unless call DisableKvStoreAutoLaunch interface to disable it.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JR
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, ParamCheck007, TestSize.Level0)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->SetKvStoreConfig({ .dataDir = NB_DIRECTOR }), OK);
    AutoLaunchCallback callback;
    auto notifier = bind(&AutoLaunchCallback::AutoLaunchNotifier, &callback, placeholders::_1, placeholders::_2,
        placeholders::_3, placeholders::_4);
    /**
     * @tc.steps: step1. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    isEncryptedDb = false;
     * @tc.expected: step1. enable success, and return OK.
     */
    KvStoreObserverImpl observer;
    AutoLaunchOption option = {true, false, CipherType::DEFAULT, NULL_PASSWD, "", false, NB_DIRECTOR, &observer};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), OK);

    /**
     * @tc.steps: step2. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = false.
     * @tc.expected: step2. enable failed, and return ALREADY_SET.
     */
    option = {false, false, CipherType::DEFAULT, NULL_PASSWD, "", false, NB_DIRECTOR, &observer};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), ALREADY_SET);

    /**
     * @tc.steps: step3. call EnableKvStoreAutoLaunch use the option with which isEncrypt = true, passwd = p1.
     * @tc.expected: step3. enable failed, and return ALREADY_SET.
     */
    CipherPassword passwd1;
    passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    option = {true, true, CipherType::DEFAULT, passwd1, "", false, NB_DIRECTOR, &observer};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), ALREADY_SET);

    /**
     * @tc.steps: step4. call EnableKvStoreAutoLaunch use the option with which schema is a valid schema.
     * @tc.expected: step4. enable failed, and return ALREADY_SET.
     */
    string schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_DEFINE_1, VALID_INDEX_1);
    option = {true, false, CipherType::DEFAULT, NULL_PASSWD, schema, false, NB_DIRECTOR, &observer};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), ALREADY_SET);

    /**
     * @tc.steps: step5. call EnableKvStoreAutoLaunch use the option with which createDirByStoreIdOnly = true.
     * @tc.expected: step5. enable failed, and return ALREADY_SET.
     */
    option = {true, false, CipherType::DEFAULT, NULL_PASSWD, "", true, NB_DIRECTOR, &observer};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), ALREADY_SET);

    /**
     * @tc.steps: step6. call EnableKvStoreAutoLaunch use the option with which dataDir = DIRECTOR.
     * @tc.expected: step6. enable failed, and return ALREADY_SET.
     */
    SetDir(DIRECTOR);
    option = {true, false, CipherType::DEFAULT, NULL_PASSWD, "", false, DIRECTOR, &observer};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), ALREADY_SET);

    /**
     * @tc.steps: step7. call EnableKvStoreAutoLaunch use the option with which observer = observer2.
     * @tc.expected: step7. enable failed, and return ALREADY_SET.
     */
    KvStoreObserverImpl observer2;
    option = {true, false, CipherType::DEFAULT, NULL_PASSWD, "", false, NB_DIRECTOR, &observer2};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), ALREADY_SET);

    /**
     * @tc.steps: step8. call EnableKvStoreAutoLaunch use the option with which notifier is not null.
     * @tc.expected: step8. enable failed, and return ALREADY_SET.
     */
    option = {true, false, CipherType::DEFAULT, NULL_PASSWD, "", false, NB_DIRECTOR, &observer};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), ALREADY_SET);
    /**
     * @tc.steps: step9. call EnableKvStoreAutoLaunch use the option with the params is the same with the first time .
     * @tc.expected: step9. enable failed, and return ALREADY_SET.
     */
    option = {true, false, CipherType::DEFAULT, NULL_PASSWD, "", false, NB_DIRECTOR, &observer};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), ALREADY_SET);
    /**
     * @tc.steps: step10. call DisableKvStoreAutoLaunch interface to disable the function,
     *     and use the option which is different from the first time EnableKvStoreAutoLaunch used.
     * @tc.expected: step10. enable success, and return OK.
     */
    EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1), OK);
    option = {true, true, CipherType::DEFAULT, passwd1, schema, true, DIRECTOR, &observer2};
    EXPECT_EQ(manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1, option, notifier), OK);
    EXPECT_EQ(manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1), OK);

    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    delete manager;
    manager = nullptr;
}
#endif
void VerifyDisableRetStatus(KvStoreDelegateManager *&manager)
{
    DBStatus status = manager->DisableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1);
    EXPECT_TRUE(status == OK || status == NOT_FOUND || status == BUSY);
}
/*
 * @tc.name: ClosedSyncPressure 001
 * @tc.desc: concurrency open/close operation of EnableKvStoreAutoLaunch or DisableKvStoreAutoLaunch.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JR
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, ClosedSyncPressure001, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    AutoLaunchOption autoLaunchOption;
    autoLaunchOption.isEncryptedDb = option.isEncryptedDb;
    autoLaunchOption.passwd.SetValue(option.passwd.data(), option.passwd.size());
    autoLaunchOption.dataDir = NB_DIRECTOR;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    /**
     * @tc.steps: step1. call EnableKvStoreAutoLaunch use the option with which createIfNecessary = true,
     *    isEncryptedDb = false;
     * @tc.expected: step1. enable success, and return OK.
     */
    vector<thread> threads;
    const unsigned int threeThreads = 3;
    for (unsigned int threadId = 0; threadId < threeThreads; ++threadId) {
        threads.push_back(thread([&]() {
            DBStatus enableStatus = manager->EnableKvStoreAutoLaunch(USER_ID_1, APP_ID_1, STORE_ID_1,
                autoLaunchOption, nullptr);
            EXPECT_TRUE(enableStatus == OK || enableStatus == ALREADY_SET);
        }));
    }

    for (unsigned int threadId = 0; threadId < threeThreads; ++threadId) {
        threads.push_back(thread([&]() {
            VerifyDisableRetStatus(manager);
        }));
    }

    for (unsigned int threadId = 0; threadId <= threeThreads; ++threadId) {
        threads.push_back(thread([&]() {
            KvStoreNbDelegate *delegateThread = nullptr;
            KvStoreDelegateManager *managerThread = nullptr;
            option.createIfNecessary = false;
            delegateThread = DistributedDBNbTestTools::GetNbDelegateSuccess(managerThread, g_dbParameter1, option);
            ASSERT_TRUE(managerThread != nullptr && delegateThread != nullptr);
            EXPECT_TRUE(managerThread->CloseKvStore(delegateThread) == OK);
            delegateThread = nullptr;
            delete managerThread;
            managerThread = nullptr;
        }));
    }

    for (auto &th : threads) {
        th.join();
    }
    VerifyDisableRetStatus(manager);

    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: SyncCommErr001
 * @tc.desc: Test Sync return Code, when Communicator get some error.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JR
 * @tc.author: xushaohua
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, SyncCommErr001, TestSize.Level1)
{
    KvStoreDelegateManager *manager = new (std::nothrow) KvStoreDelegateManager(APP_ID_1, USER_ID_1);
    ASSERT_NE(manager, nullptr);
    manager->SetProcessLabel("MST", "GetDevicesID");
    std::shared_ptr<ProcessCommunicatorTestStub> communicator = std::make_shared<ProcessCommunicatorTestStub>();
    manager->SetProcessCommunicator(communicator);

    /**
     * @tc.steps: step1. Get a KvStoreNbDelegate delegate.
     * @tc.expected: step1. delegate is not null.
     */
    Option dbOption;
    KvStoreNbDelegate *delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter1, dbOption);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step2. Set the ProcessCommunicatorTestStub CommErr to make communicator error scene, and
     *    call sync.
     * @tc.expected: step2. Sync callback will return COMM_FAILURE.
     */
    communicator->SetCommErr(true);
    const std::string testDeviceName = "test_device";
    std::vector<std::string> deviceList;
    deviceList.push_back(testDeviceName);

    auto syncCallBack = [testDeviceName](const std::map<std::string, DBStatus> &statusMap) {
        auto iter = statusMap.find(testDeviceName);
        EXPECT_NE(iter, statusMap.end());
        EXPECT_EQ(iter->second, COMM_FAILURE);
    };
    EXPECT_EQ(delegate->Sync(deviceList, SYNC_MODE_PUSH_ONLY, syncCallBack, true), OK);

    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_1), OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: AutoLaunch 001
 * @tc.desc: GetKvStoreIdentifier returns the responding hashIdentity if the parameters are valid,
 *  or it returns empty string.
 * @tc.type: FUNC
 * @tc.require: SR000EPA24
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbEnableSyncByClosedDbTest, AutoLaunch001, TestSize.Level1)
{
    KvStoreDelegateManager manager(APP_ID_1, USER_ID_1);
    /**
     * @tc.steps: step1. call GetKvStoreIdentifier with valid useId, appId and storeId
     * @tc.expected: step1. returns valid hashIdentity.
     */
    string hashIdentity = manager.GetKvStoreIdentifier(USER_ID_1, APP_ID_1, STORE_ID_SYNC_1);
    EXPECT_NE(hashIdentity, "");
    /**
     * @tc.steps: step2. call GetKvStoreIdentifier separately with empty string, 128 or 129 characters as userId
     * @tc.expected: step2. returns empty string.
     */
    string userId = "";
    hashIdentity = manager.GetKvStoreIdentifier(APP_ID_1, userId, STORE_ID_SYNC_1);
    EXPECT_EQ(hashIdentity, "");

    GenerateFixedLenRandString(STRING_ONE_TWO_EIGHT_BYTE, RandType::ALPHA_NUM, userId);
    hashIdentity = manager.GetKvStoreIdentifier(APP_ID_1, userId, STORE_ID_SYNC_1);
    EXPECT_NE(hashIdentity, "");

    GenerateFixedLenRandString(STRING_ONE_TWO_NINE_BYTE, RandType::ALPHA_NUM, userId);
    hashIdentity = manager.GetKvStoreIdentifier(APP_ID_1, userId, STORE_ID_SYNC_1);
    EXPECT_EQ(hashIdentity, "");
    /**
     * @tc.steps: step3. call GetKvStoreIdentifier separately with empty string, 128 or 129 characters as appId
     * @tc.expected: step3. returns empty string.
     */
    string appId = "";
    hashIdentity = manager.GetKvStoreIdentifier(appId, USER_ID_1, STORE_ID_SYNC_1);
    EXPECT_EQ(hashIdentity, "");

    GenerateFixedLenRandString(STRING_ONE_TWO_EIGHT_BYTE, RandType::ALPHA_NUM, appId);
    hashIdentity = manager.GetKvStoreIdentifier(appId, USER_ID_1, STORE_ID_SYNC_1);
    EXPECT_NE(hashIdentity, "");

    GenerateFixedLenRandString(STRING_ONE_TWO_NINE_BYTE, RandType::ALPHA_NUM, appId);
    hashIdentity = manager.GetKvStoreIdentifier(appId, USER_ID_1, STORE_ID_SYNC_1);
    EXPECT_EQ(hashIdentity, "");
    /**
     * @tc.steps: step4. call GetKvStoreIdentifier separately with empty string, '\0' included characters,
     *  128 or 129 characters, upper case, lower case, number, _ randomed characters,
     *  special characters(\\ // & ^ % # -, Chinese or Latin characters) as storeId
     * @tc.expected: step4. returns empty string.
     */
    string storeId = "";
    hashIdentity = manager.GetKvStoreIdentifier(USER_ID_1, APP_ID_1, storeId);
    EXPECT_EQ(hashIdentity, "");

    string prefixStoreId, suffixStoreId;
    GenerateFixedLenRandString(GetRandInt(STRING_ONE_BYTE, STRING_TEN_BYTE), RandType::ALPHA_NUM, prefixStoreId);
    GenerateFixedLenRandString(GetRandInt(STRING_ONE_BYTE, STRING_TEN_BYTE), RandType::ALPHA_NUM, suffixStoreId);
    storeId = prefixStoreId + "\0" + suffixStoreId;
    hashIdentity = manager.GetKvStoreIdentifier(USER_ID_1, APP_ID_1, storeId);
    EXPECT_NE(hashIdentity, "");

    GenerateFixedLenRandString(STRING_ONE_TWO_EIGHT_BYTE, RandType::ALPHA_NUM, storeId);
    hashIdentity = manager.GetKvStoreIdentifier(USER_ID_1, APP_ID_1, storeId);
    EXPECT_NE(hashIdentity, "");

    GenerateFixedLenRandString(STRING_ONE_TWO_NINE_BYTE, RandType::ALPHA_NUM, storeId);
    hashIdentity = manager.GetKvStoreIdentifier(USER_ID_1, APP_ID_1, storeId);
    EXPECT_EQ(hashIdentity, "");

    GenerateFixedLenRandString(ONE_K_LONG_STRING, RandType::ALPHA_NUM_UNDERLINE, storeId);
    hashIdentity = manager.GetKvStoreIdentifier(USER_ID_1, APP_ID_1, storeId);
    EXPECT_EQ(hashIdentity, "");

    GenerateFixedLenRandString(ONE_K_LONG_STRING, RandType::SPECIAL, storeId);
    storeId += string("中文");
    storeId += string("Ã„Ã¤");
    hashIdentity = manager.GetKvStoreIdentifier(USER_ID_1, APP_ID_1, storeId);
    EXPECT_EQ(hashIdentity, "");
}
}