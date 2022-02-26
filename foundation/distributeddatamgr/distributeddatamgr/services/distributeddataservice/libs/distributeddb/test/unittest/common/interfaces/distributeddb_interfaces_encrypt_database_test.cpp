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

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;

    const string STORE_ID1 = "store1";
    const string STORE_ID2 = "store2";

    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    KvStoreConfig g_config;
}

class DistributedDBInterfacesEncryptDatabaseTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static void CheckRekeyWithMultiKvStore(bool isLocal);
    static void CheckRekeyWithExistedSnapshot(bool isLocal);
    static void CheckRekeyWithExistedObserver(bool isLocal);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesEncryptDatabaseTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesEncryptDatabaseTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesEncryptDatabaseTest::SetUp(void)
{
}

void DistributedDBInterfacesEncryptDatabaseTest::TearDown(void)
{
}

void DistributedDBInterfacesEncryptDatabaseTest::CheckRekeyWithMultiKvStore(bool isLocal)
{
    DBStatus status;
    KvStoreDelegate *kvStore1 = nullptr;
    auto delegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(status), std::ref(kvStore1));
    /**
     * @tc.steps:step1. Get the delegate.
     */
    KvStoreDelegate::Option option = {true, isLocal, false};
    g_mgr.GetKvStore(STORE_ID1, option, delegateCallback);
    ASSERT_TRUE(kvStore1 != nullptr);

    KvStoreDelegate *kvStore2 = nullptr;
    auto delegateCallback2 = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(status), std::ref(kvStore2));
    /**
     * @tc.steps:step2. Get another delegate.
     */
    option.createIfNecessary = false;
    g_mgr.GetKvStore(STORE_ID1, option, delegateCallback2);
    ASSERT_TRUE(kvStore2 != nullptr);

    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    CipherPassword passwd; // random password
    vector<uint8_t> passwdBuffer(10, 45);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    EXPECT_EQ(kvStore1->Rekey(passwd), BUSY);
    /**
     * @tc.steps:step4. Close the kv store delegate.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(kvStore2), OK);
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    EXPECT_EQ(kvStore1->Rekey(passwd), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(kvStore1), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
}

void DistributedDBInterfacesEncryptDatabaseTest::CheckRekeyWithExistedSnapshot(bool isLocal)
{
    DBStatus status;
    KvStoreDelegate *kvStore = nullptr;
    auto delegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(status), std::ref(kvStore));
    /**
     * @tc.steps:step1. Get the delegate.
     */
    KvStoreDelegate::Option option = {true, isLocal, false};
    g_mgr.GetKvStore(STORE_ID1, option, delegateCallback);
    ASSERT_TRUE(kvStore != nullptr);

    KvStoreSnapshotDelegate *snapshot = nullptr;
    auto snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshot));
    /**
     * @tc.steps:step2. Get the snapshot through the delegate.
     */
    kvStore->GetKvStoreSnapshot(nullptr, snapshotDelegateCallback);
    EXPECT_NE(snapshot, nullptr);
    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    CipherPassword passwd; // random password
    vector<uint8_t> passwdBuffer(10, 45);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    EXPECT_EQ(kvStore->Rekey(passwd), BUSY);
    /**
     * @tc.steps:step4. Release the snapshot.
     */
    EXPECT_EQ(kvStore->ReleaseKvStoreSnapshot(snapshot), OK);
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    EXPECT_EQ(kvStore->Rekey(passwd), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(kvStore), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
}

void DistributedDBInterfacesEncryptDatabaseTest::CheckRekeyWithExistedObserver(bool isLocal)
{
    DBStatus status;
    KvStoreDelegate *kvStore = nullptr;
    auto delegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(status), std::ref(kvStore));
    /**
     * @tc.steps:step1. Get the delegate.
     */
    KvStoreDelegate::Option option = {true, isLocal, false};
    g_mgr.GetKvStore(STORE_ID1, option, delegateCallback);
    ASSERT_TRUE(kvStore != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer.
     */
    auto observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    ASSERT_EQ(kvStore->RegisterObserver(observer), OK);
    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    CipherPassword passwd; // random password
    vector<uint8_t> passwdBuffer(10, 45);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    EXPECT_EQ(kvStore->Rekey(passwd), BUSY);
    /**
     * @tc.steps:step4. Unregister the observer.
     */
    EXPECT_EQ(kvStore->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    EXPECT_EQ(kvStore->Rekey(passwd), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(kvStore), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID1), OK);
}

/**
  * @tc.name: LocalDatabaseRekeyCheck001
  * @tc.desc: Attempt to rekey while another delegate is existed.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDatabaseTest, LocalDatabaseRekeyCheck001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the local delegate.
     */
    /**
     * @tc.steps:step2. Get another local delegate.
     */
    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    /**
     * @tc.steps:step4. Close the kv store delegate.
     */
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    CheckRekeyWithMultiKvStore(true);
}

/**
  * @tc.name: LocalDatabaseRekeyCheck002
  * @tc.desc: Attempt to rekey while the snapshot is existed.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDatabaseTest, LocalDatabaseRekeyCheck002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the local delegate.
     */
    /**
     * @tc.steps:step2. Get the snapshot through the delegate.
     */
    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    /**
     * @tc.steps:step4. Release the snapshot.
     */
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    CheckRekeyWithExistedSnapshot(true);
}

/**
  * @tc.name: MultiVerRekeyCheck001
  * @tc.desc: Attempt to rekey while another delegate is existed.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDatabaseTest, MultiVerRekeyCheck001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the multi version delegate.
     */
    /**
     * @tc.steps:step2. Get another multi version delegate.
     */
    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    /**
     * @tc.steps:step4. Close the kv store delegate.
     */
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    CheckRekeyWithMultiKvStore(false);
}

/**
  * @tc.name: MultiVerRekeyCheck002
  * @tc.desc: Attempt to rekey while the snapshot is existed.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDatabaseTest, MultiVerRekeyCheck002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the multi version delegate.
     */
    /**
     * @tc.steps:step2. Get the snapshot through the delegate.
     */
    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    /**
     * @tc.steps:step4. Release the snapshot.
     */
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    CheckRekeyWithExistedSnapshot(false);
}

/**
  * @tc.name: MultiVerRekeyCheck003
  * @tc.desc: Attempt to rekey while the observer is existed.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDatabaseTest, MultiVerRekeyCheck003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the delegate.
     */
    /**
     * @tc.steps:step2. Register the non-null observer.
     */
    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    /**
     * @tc.steps:step4. Unregister the observer.
     */
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    CheckRekeyWithExistedObserver(false);
}

/**
  * @tc.name: SingleVerRekeyCheck001
  * @tc.desc: Attempt to rekey while another delegate is existed.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDatabaseTest, SingleVerRekeyCheck001, TestSize.Level1)
{
    DBStatus status;
    KvStoreNbDelegate *kvStore1 = nullptr;
    auto delegateCallback1 = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(status), std::ref(kvStore1));
    /**
     * @tc.steps:step1. Get the single version delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(STORE_ID2, option, delegateCallback1);
    ASSERT_TRUE(kvStore1 != nullptr);

    KvStoreNbDelegate *kvStore2 = nullptr;
    auto delegateCallback2 = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(status), std::ref(kvStore2));
    /**
     * @tc.steps:step2. Get another single version delegate.
     */
    option.createIfNecessary = false;
    g_mgr.GetKvStore(STORE_ID2, option, delegateCallback2);
    ASSERT_TRUE(kvStore2 != nullptr);

    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    CipherPassword passwd;
    vector<uint8_t> passwdBuffer(10, 45);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    EXPECT_EQ(kvStore1->Rekey(passwd), BUSY);
    /**
     * @tc.steps:step4. Close the kv store delegate.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(kvStore2), OK);
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    EXPECT_EQ(kvStore1->Rekey(passwd), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(kvStore1), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID2), OK);
}

/**
  * @tc.name: SingleVerRekeyCheck002
  * @tc.desc: Attempt to rekey when the observer exists.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDatabaseTest, SingleVerRekeyCheck002, TestSize.Level1)
{
    DBStatus status;
    KvStoreNbDelegate *kvStore = nullptr;
    auto delegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(status), std::ref(kvStore));
    /**
     * @tc.steps:step1. Get the single version delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(STORE_ID2, option, delegateCallback);
    ASSERT_TRUE(kvStore != nullptr);

    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observer != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the empty key.
     */
    Key key;
    EXPECT_EQ(kvStore->RegisterObserver(key, 4, observer), OK); // Only use the event 4.

    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    CipherPassword passwd;
    vector<uint8_t> passwdBuffer(10, 45);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    EXPECT_EQ(kvStore->Rekey(passwd), BUSY);
    /**
     * @tc.steps:step4. Unregister the observer.
     */
    EXPECT_EQ(kvStore->UnRegisterObserver(observer), OK);
    delete observer;
    observer = nullptr;
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    EXPECT_EQ(kvStore->Rekey(passwd), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(kvStore), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID2), OK);
}

static void NotifierCallback(const KvStoreNbConflictData &data)
{
}
/**
  * @tc.name: SingleVerRekeyCheck003
  * @tc.desc: Attempt to rekey while the conflict notifier is set.
  * @tc.type: FUNC
  * @tc.require: AR000CQDT7
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesEncryptDatabaseTest, SingleVerRekeyCheck003, TestSize.Level1)
{
    DBStatus status;
    KvStoreNbDelegate *kvStore = nullptr;
    auto delegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(status), std::ref(kvStore));
    /**
     * @tc.steps:step1. Get the single version delegate.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(STORE_ID2, option, delegateCallback);
    ASSERT_TRUE(kvStore != nullptr);
    /**
     * @tc.steps:step2. Set the non-null conflict notifier.
     */
    const int conflictAll = 15;
    ASSERT_EQ(kvStore->SetConflictNotifier(conflictAll, NotifierCallback), OK);
    CipherPassword passwd;
    vector<uint8_t> passwdBuffer(10, 45);
    int errCode = passwd.SetValue(passwdBuffer.data(), passwdBuffer.size());
    ASSERT_EQ(errCode, CipherPassword::ErrorCode::OK);
    /**
     * @tc.steps:step3. Execute the rekey operation.
     * @tc.expected: step3. return BUSY.
     */
    EXPECT_EQ(kvStore->Rekey(passwd), BUSY);
    /**
     * @tc.steps:step4. Set the null conflict notifier to unregister the conflict notifier.
     */
    EXPECT_EQ(kvStore->SetConflictNotifier(1, nullptr), OK);
    /**
     * @tc.steps:step5. Execute the rekey operation.
     * @tc.expected: step5. return OK.
     */
    EXPECT_EQ(kvStore->Rekey(passwd), OK);
    EXPECT_EQ(g_mgr.CloseKvStore(kvStore), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID2), OK);
}
#endif
