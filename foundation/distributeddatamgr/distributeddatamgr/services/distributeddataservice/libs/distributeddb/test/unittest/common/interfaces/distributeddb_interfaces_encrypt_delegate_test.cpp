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
#ifndef OMIT_ENCRYPT
#include <gtest/gtest.h>

#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "sqlite_utils.h"
#include "sqlite_single_ver_natural_store.h"
#include "db_errno.h"
#include "log_print.h"
#include "kv_store_nb_conflict_data.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    const string STORE_ID1 = "store1_singleVersionDB";
    const string STORE_ID2 = "store2_localDB";
    const string STORE_ID3 = "store3_multiVersionDB";
    CipherPassword g_passwd1; // 5 '1'
    CipherPassword g_passwd2; // 5 '2'
    CipherPassword g_passwd3; // 5 '3'
    const CipherPassword PASSWD_EMPTY;
    const int CONFLICT_ALL = 15;
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    string g_testDir;
    KvStoreConfig g_config;
    DBStatus g_errCode = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    KvStoreDelegate *g_kvDelegatePtr = nullptr;

    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_errCode), std::ref(g_kvNbDelegatePtr));

    // define the delegate call back
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_errCode), std::ref(g_kvDelegatePtr));

    DBStatus g_snapshotDelegateStatus = INVALID_ARGS;
    KvStoreSnapshotDelegate *g_snapshotDelegatePtr = nullptr;

    auto g_snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_snapshotDelegateStatus), std::ref(g_snapshotDelegatePtr));

    DBStatus g_valueStatus = INVALID_ARGS;
    Value g_value;

    auto g_valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(g_valueStatus), std::ref(g_value));

    void GetSnapshotAndValueCheck(const Key &testKey, const Value &testValue, DBStatus errCode)
    {
        g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, g_snapshotDelegateCallback);
        EXPECT_TRUE(g_snapshotDelegateStatus == OK);
        ASSERT_TRUE(g_snapshotDelegatePtr != nullptr);
        // check value and errCode
        g_snapshotDelegatePtr->Get(testKey, g_valueCallback);
        EXPECT_EQ(g_valueStatus, errCode);

        if (errCode == OK) {
            EXPECT_TRUE(g_value.size() > 0);
            if (g_value.size() > 0) {
                EXPECT_EQ(g_value.front(), testValue[0]);
            }
        }
    }

    void GetNbKvStoreAndCheckFun(const std::string &storeId, const KvStoreNbDelegate::Option &option,
        const Key &testKey, const Value &testValue)
    {
        g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
        ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
        EXPECT_EQ(g_errCode, OK);
        EXPECT_EQ(g_kvNbDelegatePtr->Put(testKey, testValue), OK);
        Value valueRead;
        EXPECT_EQ(g_kvNbDelegatePtr->Get(testKey, valueRead), OK);
        EXPECT_EQ(valueRead, testValue);
    }

    void NotifierConnectTestCallback(const KvStoreNbConflictData &data)
    {
        g_errCode = INVALID_ARGS;
    }
}

class DistributedDBInterfacesEncryptDelegateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesEncryptDelegateTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
    vector<uint8_t> passwdBuffer1(5, 1);
    int errCode = g_passwd1.SetValue(passwdBuffer1.data(), passwdBuffer1.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    vector<uint8_t> passwdBuffer2(5, 2);
    errCode = g_passwd2.SetValue(passwdBuffer2.data(), passwdBuffer2.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    vector<uint8_t> passwdBuffer3(5, 3);
    errCode = g_passwd3.SetValue(passwdBuffer3.data(), passwdBuffer3.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
}

void DistributedDBInterfacesEncryptDelegateTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesEncryptDelegateTest::SetUp(void)
{
    g_errCode = INVALID_ARGS;
    g_kvNbDelegatePtr = nullptr;
    g_kvDelegatePtr = nullptr;
}

void DistributedDBInterfacesEncryptDelegateTest::TearDown(void) {}

/**
  * @tc.name: EncryptedDbOperation001
  * @tc.desc: Test the single version db encrypted function.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT5 AR000CQDT6
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbOperation001, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    ASSERT_TRUE(g_errCode == OK);
    /**
     * @tc.steps: step1. Put [KEY_1, V1]
     * @tc.expected: step1. Get result OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps: step2. Close db and open it again ,then get the value of K1
     * @tc.expected: step2. Close and open db successfully, value of K1 is V1
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;

    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_1);
    /**
     * @tc.steps: step3. Put [KEY_1, V2]
     * @tc.expected: step3. Get result OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps: step4. Close db and open it again ,then get the value of K1
     * @tc.expected: step4. Close and open db successfully, value of K1 is V2
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    g_mgr.GetKvStore(STORE_ID1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);
    Value valueRead2;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, valueRead2), OK);
    EXPECT_EQ(valueRead2, VALUE_2);
    /**
     * @tc.steps: step5. Delete record K1
     * @tc.expected: step5. Get result OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_1), OK);
    /**
     * @tc.steps: step6. Close db and open it again ,then get the value of K1
     * @tc.expected: step6. Close and open db successfully, get value of K1 NOT_FOUND
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    g_mgr.GetKvStore(STORE_ID1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, valueRead2), NOT_FOUND);

    // additional test
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    option.passwd = g_passwd2;
    g_mgr.GetKvStore(STORE_ID1, option, g_kvNbDelegateCallback);
    EXPECT_EQ(g_kvNbDelegatePtr, nullptr);
    EXPECT_EQ(g_errCode, INVALID_PASSWD_OR_CORRUPTED_DB);

    // finilize logic
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), INVALID_ARGS);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbOperation002
  * @tc.desc: Test the local db encrypted function.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT5 AR000CQDT6
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbOperation002, TestSize.Level1)
{
    KvStoreDelegate::Option option = {true, true, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    /**
     * @tc.steps: step1. Put [KEY_1, V1]
     * @tc.expected: step1. Get result OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps: step2. Close db and open it again ,then get the value of K1
     * @tc.expected: step2. Close and open db successfully, value of K1 is V1
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_1, OK);

    /**
     * @tc.steps: step3. Put [KEY_1, V2]
     * @tc.expected: step3. Get result OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps: step4. Close db and open it again ,then get the value of K1
     * @tc.expected: step4. Close and open db successfully, value of K1 is V2
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_2, OK);
    /**
     * @tc.steps: step5. Delete record K1
     * @tc.expected: step5. Get result OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Delete(KEY_1), OK);
    /**
     * @tc.steps: step6. Close db and open it again ,then get the value of K1
     * @tc.expected: step6. Close and open db successfully, get value of K1 NOT_FOUND
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);
    GetSnapshotAndValueCheck(KEY_1, VALUE_2, NOT_FOUND);

    // additional test
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    option.passwd = g_passwd2;
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    EXPECT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_EQ(g_errCode, INVALID_PASSWD_OR_CORRUPTED_DB);

    // finilize logic
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID2), OK);
}

/**
  * @tc.name: EncryptedDbOperation003
  * @tc.desc: Test the multi version db encrypted function.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT5 AR000CQDT6
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbOperation003, TestSize.Level1)
{
    KvStoreDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    /**
     * @tc.steps: step1. Put [KEY_1, V1]
     * @tc.expected: step1. Get result OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps: step2. Close db and open it again ,then get the value of K1
     * @tc.expected: step2. Close and open db successfully, value of K1 is V1
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_1, OK);
    /**
     * @tc.steps: step3. Put [KEY_1, V2]
     * @tc.expected: step3. Get result OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps: step4. Close db and open it again ,then get the value of K1
     * @tc.expected: step4. Close and open db successfully, value of K1 is V2
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_2, OK);
    /**
     * @tc.steps: step5. Delete record K1
     * @tc.expected: step5. Get result OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Delete(KEY_1), OK);
    /**
     * @tc.steps: step6. Close db and open it again ,then get the value of K1
     * @tc.expected: step6. Close and open db successfully, get value of K1 NOT_FOUND
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_2, NOT_FOUND);

    // additional test
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    option.passwd = g_passwd2;
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    EXPECT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_EQ(g_errCode, INVALID_PASSWD_OR_CORRUPTED_DB);

    // finilize logic
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID3), OK);
}

/**
  * @tc.name: EncryptedDbSwitch001
  * @tc.desc: Test the single version db for Rekey function.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch001, TestSize.Level1)
{
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_1);
    /**
     * @tc.steps: step3. Rekey passwd to passwd2
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(g_passwd2), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step4/step5. Put and get [KEY_1, V2]
     * @tc.expected: step4/step5. Get value of KEY_1, value of K1 is V2.
     */
    option.passwd = g_passwd2;
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_2);

    /**
     * @tc.steps: step6. Rekey passwd to empty
     * @tc.expected: step6. result is OK
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(PASSWD_EMPTY), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step7/step8. Put and get [KEY_1, V3]
     * @tc.expected: step7/step8. Get value of KEY_1, value of K1 is V3.
     */
    option.isEncryptedDb = false;
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_3);
    /**
     * @tc.steps: step9. Rekey passwd to passwd3
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(g_passwd3), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step10/step11. Put and get [KEY_1, V4]
     * @tc.expected: step10/step11. Get value of KEY_1, value of K1 is V4.
     */
    option.isEncryptedDb = true;
    option.passwd = g_passwd3;
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_4);

    // finilize logic
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbSwitch002
  * @tc.desc: Test the single version db Rekey function return BUSY because of multiple instances.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch002, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);
    /**
     * @tc.steps: step1. open same database again
     * @tc.expected: step1. Get result OK
     */
    DBStatus errCode = INVALID_ARGS;
    KvStoreNbDelegate *kvNbDelegatePtr = nullptr;
    auto kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(errCode), std::ref(kvNbDelegatePtr));
    g_mgr.GetKvStore(STORE_ID1, option, kvNbDelegateCallback);
    ASSERT_TRUE(kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(errCode == OK);
    /**
     * @tc.steps: step2. invoke rekey logic
     * @tc.expected: step2. Get result BUSY
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(g_passwd2), BUSY);
    // finilize logic
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
}

/**
  * @tc.name: EncryptedDbSwitch003
  * @tc.desc: Test the single version db Rekey function return BUSY because of observer.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch003, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);
    /**
     * @tc.steps: step1. register observer
     * @tc.expected: step1. Get result OK
     */
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_NE(observer, nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, observer), OK);
    /**
     * @tc.steps: step2. invoke rekey logic
     * @tc.expected: step2. Get result BUSY
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(g_passwd2), BUSY);
    // finilize logic
    delete observer;
    observer = nullptr;
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
}

/**
  * @tc.name: EncryptedDbSwitch004
  * @tc.desc: Test the single version db Rekey function return BUSY because of conflict notifier.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch004, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);
    /**
     * @tc.steps: step1. register observer
     * @tc.expected: step1. Get result OK
     */
    EXPECT_EQ(g_kvNbDelegatePtr->SetConflictNotifier(CONFLICT_ALL, NotifierConnectTestCallback), OK);
    /**
     * @tc.steps: step2. invoke rekey logic
     * @tc.expected: step2. Get result BUSY
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(g_passwd2), BUSY);
    // finilize logic
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
}

/**
  * @tc.name: EncryptedDbSwitch008
  * @tc.desc: Test the multi version db Rekey function return BUSY because of Snapshot not close.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch008, TestSize.Level1)
{
}

/**
  * @tc.name: EncryptedDbSwitch009
  * @tc.desc: Test the single version db Rekey function from password1 to password2.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch009, TestSize.Level1)
{
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_1);
    /**
     * @tc.steps: step3. Rekey passwd to passwd2
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(g_passwd2), OK);
    Value valueRead1;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, valueRead1), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step4/step5. Put and get [KEY_1, V2]
     * @tc.expected: step4/step5. Get value of KEY_1, value of K1 is V2.
     */
    option.passwd = g_passwd2;
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_2);
    // finilize logic
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbSwitch010
  * @tc.desc: Test the single version db Rekey function from encrypted db unencrypted db .
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch010, TestSize.Level1)
{
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    KvStoreNbDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_1);
    /**
     * @tc.steps: step6. Rekey passwd to empty
     * @tc.expected: step6. result is OK
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(PASSWD_EMPTY), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step7/step8. Put and get [KEY_1, V3]
     * @tc.expected: step7/step8. Get value of KEY_1, value of K1 is V3.
     */
    option.isEncryptedDb = false;
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_3);
    // finilize logic
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbSwitch011
  * @tc.desc: Test the single version db Rekey function from unencrypted db to encrypted db.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch011, TestSize.Level1)
{
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    KvStoreNbDelegate::Option option = {true, false, false, CipherType::DEFAULT, PASSWD_EMPTY};
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_1);
    /**
     * @tc.steps: step3. Rekey passwd to passwd2
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rekey(g_passwd2), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step4/step5. Put and get [KEY_1, V2]
     * @tc.expected: step4/step5. Get value of KEY_1, value of K1 is V2.
     */
    option.passwd = g_passwd2;
    option.isEncryptedDb = true;
    GetNbKvStoreAndCheckFun(STORE_ID1, option, KEY_1, VALUE_2);
    // finilize logic
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbSwitch012
  * @tc.desc: Test the local db Rekey function from password1 to password2.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch012, TestSize.Level1)
{
    KvStoreDelegate::Option option = {true, true, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_1), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_1, OK);

    /**
     * @tc.steps: step3. Rekey passwd to passwd2
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_kvDelegatePtr->Rekey(g_passwd2), OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_3), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step4/step5. Put and get [KEY_1, V2]
     * @tc.expected: step4/step5. Get value of KEY_1, value of K1 is V2.
     */
    option.passwd = g_passwd2;
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_2), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_2, OK);
    // finilize logic
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID2), OK);
    g_kvDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbSwitch013
  * @tc.desc: Test the local db Rekey function from encrypted db unencrypted db .
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch013, TestSize.Level1)
{
    KvStoreDelegate::Option option = {true, true, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_1), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_1, OK);
    /**
     * @tc.steps: step3. Rekey passwd to empty
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_kvDelegatePtr->Rekey(PASSWD_EMPTY), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step4/step5. Put and get [KEY_1, V2]
     * @tc.expected: step4/step5. Get value of KEY_1, value of K1 is V2.
     */
    option.isEncryptedDb = false;
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_2), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_2, OK);
    // finilize logic
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID2), OK);
    g_kvDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbSwitch014
  * @tc.desc: Test the local db Rekey function from unencrypted db to encrypted db.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch014, TestSize.Level1)
{
    KvStoreDelegate::Option option = {true, true, false, CipherType::DEFAULT, PASSWD_EMPTY};
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_1), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_1, OK);
    /**
     * @tc.steps: step3. Rekey passwd to passwd2
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_kvDelegatePtr->Rekey(g_passwd2), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step4/step5. Put and get [KEY_1, V2]
     * @tc.expected: step4/step5. Get value of KEY_1, value of K1 is V2.
     */
    option.passwd = g_passwd2;
    option.isEncryptedDb = true;
    g_mgr.GetKvStore(STORE_ID2, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_2), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_2, OK);
    // finilize logic
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID2), OK);
    g_kvDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbSwitch015
  * @tc.desc: Test the multi version db Rekey function from password1 to password2.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch015, TestSize.Level1)
{
    KvStoreDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_1), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_1, OK);
    /**
     * @tc.steps: step3. Rekey passwd to passwd2
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_kvDelegatePtr->Rekey(g_passwd2), OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_3), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_3, OK);
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);

    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step4/step5. Put and get [KEY_1, V2]
     * @tc.expected: step4/step5. Get value of KEY_1, value of K1 is V2.
     */
    option.passwd = g_passwd2;
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_2), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_2, OK);
    // finilize logic
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID3), OK);
    g_kvDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbSwitch016
  * @tc.desc: Test the multi version db Rekey function from encrypted db unencrypted db .
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch016, TestSize.Level1)
{
    KvStoreDelegate::Option option = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_1), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_1, OK);
    /**
     * @tc.steps: step3. Rekey passwd to empty
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_kvDelegatePtr->Rekey(PASSWD_EMPTY), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step4/step5. Put and get [KEY_1, V2]
     * @tc.expected: step4/step5. Get value of KEY_1, value of K1 is V2.
     */
    option.isEncryptedDb = false;
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_2), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_2, OK);
    // finilize logic
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID3), OK);
    g_kvDelegatePtr = nullptr;
}

/**
  * @tc.name: EncryptedDbSwitch017
  * @tc.desc: Test the multi version db Rekey function from unencrypted db to encrypted db.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, EncryptedDbSwitch017, TestSize.Level1)
{
    KvStoreDelegate::Option option = {true, false, false, CipherType::DEFAULT, PASSWD_EMPTY};
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    /**
     * @tc.steps: step1/step2. Put and get [KEY_1, V1]
     * @tc.expected: step1/step2. Get value of KEY_1, value of K1 is V1.
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_1), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_1, OK);
    /**
     * @tc.steps: step3. Rekey passwd to passwd2
     * @tc.expected: step3. result is OK
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_kvDelegatePtr->Rekey(g_passwd2), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;
    g_errCode = INVALID_ARGS;
    /**
     * @tc.steps: step4/step5. Put and get [KEY_1, V2]
     * @tc.expected: step4/step5. Get value of KEY_1, value of K1 is V2.
     */
    option.passwd = g_passwd2;
    option.isEncryptedDb = true;
    g_mgr.GetKvStore(STORE_ID3, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_1, VALUE_2), OK);

    GetSnapshotAndValueCheck(KEY_1, VALUE_2, OK);
    // finilize logic
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID3), OK);
    g_kvDelegatePtr = nullptr;
}

/**
  * @tc.name: OpenEncryptedDb001
  * @tc.desc: Test create an encrypted database successfully.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT5 AR000CQDT6
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, OpenEncryptedDb001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create single version encrypted database
     * @tc.expected: step1. Get result OK.
     */
    KvStoreNbDelegate::Option option1 = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID1, option1, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_errCode == OK);
    /**
     * @tc.steps: step2. create multi version encrypted database
     * @tc.expected: step2. Get result OK.
     */
    KvStoreDelegate::Option option2 = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID3, option2, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    /**
     * @tc.steps: step3. Close db.
     * @tc.expected: step3. Get result ok.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID3), OK);
}

/**
  * @tc.name: OpenEncryptedDb002
  * @tc.desc: Test create an encrypted database failed.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT8
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, OpenEncryptedDb002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create single version encrypted database
     * @tc.expected: step1. Get result INVALID_ARGS.
     */
    KvStoreNbDelegate::Option option1 = {true, false, true, CipherType::DEFAULT, PASSWD_EMPTY};
    g_mgr.GetKvStore(STORE_ID1, option1, g_kvNbDelegateCallback);
    EXPECT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_EQ(g_errCode, INVALID_ARGS);
    /**
     * @tc.steps: step2. create multi version encrypted database
     * @tc.expected: step2. Get result INVALID_ARGS.
     */
    KvStoreDelegate::Option option2 = {true, false, true, CipherType::DEFAULT, PASSWD_EMPTY};
    g_mgr.GetKvStore(STORE_ID3, option2, g_kvDelegateCallback);
    EXPECT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_EQ(g_errCode, INVALID_ARGS);
    /**
     * @tc.steps: step3. Close db.
     * @tc.expected: step3. Get result ok.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), INVALID_ARGS);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), INVALID_ARGS);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), NOT_FOUND);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID3), NOT_FOUND);
}

/**
  * @tc.name: OpenEncryptedDb003
  * @tc.desc: Test reopen an encrypted database successfully.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT8
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, OpenEncryptedDb003, TestSize.Level1)
{
    KvStoreNbDelegate::Option option1 = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID1, option1, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    KvStoreDelegate::Option option2 = {true, false, true, CipherType::DEFAULT, g_passwd3};
    g_mgr.GetKvStore(STORE_ID3, option2, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    /**
     * @tc.steps: step1. create single version encrypted database
     * @tc.expected: step1. Get result INVALID_ARGS.
     */
    KvStoreNbDelegate::Option option3 = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID1, option3, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    /**
     * @tc.steps: step2. create multi version encrypted database
     * @tc.expected: step2. Get result INVALID_ARGS.
     */
    KvStoreDelegate::Option option4 = {true, false, true, CipherType::DEFAULT, g_passwd3};
    g_mgr.GetKvStore(STORE_ID3, option4, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    /**
     * @tc.steps: step3. Close db.
     * @tc.expected: step3. Get result ok.
     */
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID3), OK);
}

/**
  * @tc.name: OpenEncryptedDb004
  * @tc.desc: Test reopen an encrypted database failed.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT8
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDelegateTest, OpenEncryptedDb004, TestSize.Level1)
{
    KvStoreNbDelegate::Option option1 = {true, false, true, CipherType::DEFAULT, g_passwd1};
    g_mgr.GetKvStore(STORE_ID1, option1, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);

    KvStoreDelegate::Option option2 = {true, false, true, CipherType::DEFAULT, g_passwd3};
    g_mgr.GetKvStore(STORE_ID3, option2, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_EQ(g_errCode, OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    /**
     * @tc.steps: step1. create single version encrypted database
     * @tc.expected: step1. Get result INVALID_ARGS.
     */
    KvStoreNbDelegate::Option option3 = {true, false, true, CipherType::DEFAULT, g_passwd2};
    g_mgr.GetKvStore(STORE_ID1, option3, g_kvNbDelegateCallback);
    EXPECT_TRUE(g_kvNbDelegatePtr == nullptr);
    EXPECT_EQ(g_errCode, INVALID_PASSWD_OR_CORRUPTED_DB);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), INVALID_ARGS);
    /**
     * @tc.steps: step2. create multi version encrypted database
     * @tc.expected: step2. Get result INVALID_ARGS.
     */
    KvStoreDelegate::Option option4 = {true, false, true, CipherType::DEFAULT, g_passwd2};
    g_mgr.GetKvStore(STORE_ID3, option4, g_kvDelegateCallback);
    EXPECT_TRUE(g_kvDelegatePtr == nullptr);
    EXPECT_EQ(g_errCode, INVALID_PASSWD_OR_CORRUPTED_DB);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), INVALID_ARGS);
    /**
     * @tc.steps: step3. Close db.
     * @tc.expected: step3. Get result ok.
     */
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID3), OK);
}
#endif
