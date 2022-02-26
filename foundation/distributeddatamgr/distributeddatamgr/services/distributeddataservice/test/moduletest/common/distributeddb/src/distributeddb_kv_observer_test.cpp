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
#include <thread>
#include <cstdio>
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

namespace DistributedDBKvObserver {
const bool IS_LOCAL = false;
const long CHANGED_ZERO_TIME = 0;
const long CHANGED_ONE_TIME = 1;
const unsigned long OPER_CNT_START = 0;
const unsigned long OPER_CNT_END = 5;
const unsigned long TEN_RECORDS = 10;
const unsigned long DELETE_CNT_START = 0;
const unsigned long DELETE_CNT_END = 2;

DistributedDB::CipherPassword g_passwd1;
DistributedDB::CipherPassword g_passwd2;
DistributedDB::CipherPassword g_filePasswd1;
DistributedDB::CipherPassword g_filePasswd2;

class DistributeddbKvObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

KvStoreDelegate *g_observerDelegate = nullptr; // the delegate used in this suit.
KvStoreDelegateManager *g_observerManager = nullptr;
void DistributeddbKvObserverTest::SetUpTestCase(void)
{
    MST_LOG("SetUpTestCase before all cases local[%d].", IS_LOCAL);
    RemoveDir(DIRECTOR);
    g_observerDelegate = DistributedTestTools::GetDelegateSuccess(g_observerManager,
        g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(g_observerManager != nullptr && g_observerDelegate != nullptr);
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    (void)g_passwd2.SetValue(PASSWD_VECTOR_2.data(), PASSWD_VECTOR_2.size());
    (void)g_filePasswd1.SetValue(FILE_PASSWD_VECTOR_1.data(), FILE_PASSWD_VECTOR_1.size());
    (void)g_filePasswd2.SetValue(FILE_PASSWD_VECTOR_2.data(), FILE_PASSWD_VECTOR_2.size());
}

void DistributeddbKvObserverTest::TearDownTestCase(void)
{
    EXPECT_TRUE(g_observerManager->CloseKvStore(g_observerDelegate) == OK);
    g_observerDelegate = nullptr;
    DBStatus status;
    status = g_observerManager->DeleteKvStore(STORE_ID_1);
    EXPECT_EQ(status, DistributedDB::DBStatus::OK) << "fail to delete exist kvdb";
    delete g_observerManager;
    g_observerManager = nullptr;
    RemoveDir(DIRECTOR);
}

void DistributeddbKvObserverTest::SetUp(void)
{
    EXPECT_TRUE(g_observerDelegate != nullptr);
    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());
}

void DistributeddbKvObserverTest::TearDown(void)
{
}

/*
 * @tc.name: ObserverRegister 001
 * @tc.desc: Verify that can register a fresh observer base on kv db.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRegister001, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. Register observer base on kv db.
     * @tc.expected: step1. Register successfully.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: ObserverRegister 002
 * @tc.desc: Verify that can register a null observer on kv db.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRegister002, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    /**
     * @tc.steps: step1. Register a null observer base on kv db.
     * @tc.expected: step1. Register failed.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, nullptr);
    EXPECT_TRUE(status != DBStatus::OK);

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, nullptr);
    EXPECT_TRUE(statusRelease != DBStatus::OK);
}

/*
 * @tc.name: ObserverRegister 003
 * @tc.desc: Verify that can register only once with the same observer.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRegister003, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. Register observer 6 times base on kv db.
     * @tc.expected: step1. First register is successful and 5 times later return null.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);
    for (int time = OPER_CNT_START; time < static_cast<int>(OPER_CNT_END); ++time) {
        status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
        EXPECT_TRUE(status != DBStatus::OK);
    }

    /**
     * @tc.steps: step2. Register Put (k1,v1) to db.
     * @tc.expected: step2. First put successfully and the observer return inserting data once.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    insertEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: ObserverRegister 004
 * @tc.desc: Verify that can register different observers.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRegister004, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer1, observer2;
    /**
     * @tc.steps: step1. Register observer1 bases on kv db.
     * @tc.expected: step1. Register successfully.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer1);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. put (k1,v1) to db.
     * @tc.expected: step2. put successfully and the observer1 return 1 inserting data info.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    insertEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer1.Clear();

    /**
     * @tc.steps: step3. register a null observer bases on kv db.
     * @tc.expected: step3. register successfully.
     */
    status = DistributedTestTools::RegisterObserver(g_observerDelegate, nullptr);
    EXPECT_TRUE(status != DBStatus::OK);
    /**
     * @tc.steps: step4. put (k2,v2) to db.
     * @tc.expected: step4. register successfully.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_2, VALUE_2);
    EXPECT_TRUE(status == DBStatus::OK);
    insertEntries.clear();
    DistributedDB::Entry entry2 = { KEY_2, VALUE_2 };
    insertEntries.push_back(entry2);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer1.Clear();

    /**
     * @tc.steps: step5. register observer2 bases on kv db.
     * @tc.expected: step5. register successfully.
     */
    status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer2);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step6. put (k3,v3) to db.
     * @tc.expected: step6. put successfully and the observer1,2 return 1 inserting data info individually.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_3, VALUE_3);
    EXPECT_TRUE(status == DBStatus::OK);
    insertEntries.clear();
    DistributedDB::Entry entry3 = { KEY_3, VALUE_3 };
    insertEntries.push_back(entry3);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer1.Clear();
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer2.Clear();
    // step5: clear the preset of step1(repair the config and stub)
    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer1);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
    statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer2);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: ObserverRegister 005
 * @tc.desc: Verify that unregister observer (release snapshot) won't return changed data info.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRegister005, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. Register observer1 bases on kv db.
     * @tc.expected: step1. Register successfully to construct observer exist.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. put (k1,v1) to db.
     * @tc.expected: step2. put successfully and the observer1 return 1 inserting data info.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    insertEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    /**
     * @tc.steps: step3. unregister observer1 (release snapshot).
     * @tc.expected: step3. operate successfully.
     */
    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);

    /**
     * @tc.steps: step4. put (k2,v2) to db.
     * @tc.expected: step4. put successfully and has nothing changed data info returned.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_2, VALUE_2);
    EXPECT_TRUE(status == DBStatus::OK);
    insertEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();
}

/*
 * @tc.name: ObserverRegister 006
 * @tc.desc: Verify that register observer, and then unregister it many times will success only firstly.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRegister006, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. Register observer1 bases on kv db.
     * @tc.expected: step1. Register successfully to construct observer exist.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. unregister observer1 for 5 times.
     * @tc.expected: step2. Firstly register successfully and another failed.
     */
    for (int time = OPER_CNT_START; time < static_cast<int>(OPER_CNT_END); ++time) {
        DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
        if (time == OPER_CNT_START) {
            EXPECT_TRUE(statusRelease == DBStatus::OK);
        }
        else {
            EXPECT_TRUE(statusRelease != DBStatus::OK);
        }
    }

    /**
     * @tc.steps: step3. put (k1,v1) to db.
     * @tc.expected: step3. put successfully and has nothing changed data info returned.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();
}

/*
 * @tc.name: ObserverRegister 007
 * @tc.desc: Verify that unregister an unregistered observer will fail.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRegister007, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(manager,
        g_kvdbParameter2_1_1, g_kvOption);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr) << "fail to create exist kvdb";

    /**
     * @tc.steps: step1. Register observer1 bases on kv db1 snap and unregister it bases on kv db2.
     * @tc.expected: step1. Register successfully and unregister failed.
     */
    KvStoreObserverImpl observer;
    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(delegate, &observer);

    EXPECT_TRUE(statusRelease != DBStatus::OK);

    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    DBStatus status = manager->DeleteKvStore(STORE_ID_2);
    EXPECT_TRUE(status == DBStatus::OK) << "fail to delete exist kvdb";
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: ObserverRegister 008
 * @tc.desc: Verify that observer will not be called after unregister.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRegister008, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;

    /**
     * @tc.steps: step1. open delegate1 and delegate2 from hv db2.
     * @tc.expected: step1. open successfully.
     */
    delegate1 = DistributedTestTools::GetDelegateSuccess(manager1, g_kvdbParameter2_1_1, g_kvOption);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr);
    KvOption option = g_kvOption;
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    EXPECT_TRUE(delegate1 != nullptr) << "fail to create exist kvdb";
    delegate2 = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter2_1_1, option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);

    /**
     * @tc.steps: step2. Register observer1 bases on kv db2 and close delegate1.
     * @tc.expected: step2. Register and close successfully.
     */
    KvStoreObserverImpl observer1;
    DBStatus status = DistributedTestTools::RegisterObserver(delegate1, &observer1);
    EXPECT_TRUE(status == DBStatus::OK);
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) == OK);
    delegate1 = nullptr;

    /**
     * @tc.steps: step3. put (k1,v1) to delegate2.
     * @tc.expected: step3. observer1 return nothing.
     */
    status = DistributedTestTools::Put(*delegate2, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer1.Clear();

    EXPECT_TRUE(manager2->CloseKvStore(delegate2) == OK);
    delegate2 = nullptr;
    status = manager2->DeleteKvStore(STORE_ID_2);
    EXPECT_TRUE(status == DBStatus::OK) << "fail to delete exist kvdb";
    delete manager1;
    manager1 = nullptr;
    delete manager2;
    manager2 = nullptr;
}

/*
 * @tc.name: ObserverRegister 009
 * @tc.desc: Delete kv db, and verify observer will not be activated.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRegister009, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    delegate1 = DistributedTestTools::GetDelegateSuccess(manager1,
        g_kvdbParameter2_1_1, g_kvOption);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr) << "fail to create exist kvdb";

    /**
     * @tc.steps: step1. Register observer1 bases on kv db2 and put(k1,v1).
     * @tc.expected: step1. Register successfully and observer1 return a inserting data info.
     */
    KvStoreObserverImpl observer1;
    DBStatus status = DistributedTestTools::RegisterObserver(delegate1, &observer1);
    EXPECT_TRUE(status == OK);
    DBStatus statusPut = DistributedTestTools::Put(*delegate1, KEY_1, VALUE_1);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    insertEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer1.Clear();

    /**
     * @tc.steps: step2. close db then delete db.
     * @tc.expected: step2. operate successfully and observer1 return nothing.
     */
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) == OK);
    delegate1 = nullptr;
    EXPECT_TRUE(manager1->DeleteKvStore(STORE_ID_2) == OK);
    vector<DistributedDB::Entry> deleteEntries;
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, DELETE_LIST, deleteEntries));
    observer1.Clear();

    delete manager1;
    manager1 = nullptr;
}

/*
 * @tc.name: DataChange 001
 * @tc.desc: verify that can observer for inserting a record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange001, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    /**
     * @tc.steps: step1. Register observer1 bases on kv db2.
     * @tc.expected: step1. Register successfully to construct observer exist.
     */
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. put (k1,v1) to db.
     * @tc.expected: step2. put successfully and observer1 return a insert data info.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    insertEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: DataChange 002
 * @tc.desc: verify that can observer for updating a record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange002, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2.
     * @tc.expected: step1. operate successfully to construct data and observer exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. update (k1,v1) to (k1,v2).
     * @tc.expected: step2. update successfully and observer return a updating data info.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_1, VALUE_2);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<DistributedDB::Entry> updateEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_2 };
    updateEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, updateEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: DataChange 003
 * @tc.desc: verify that can observer for deleting a record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange003, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2.
     * @tc.expected: step1. operate successfully to construct data and observer exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. delete from kb.
     * @tc.expected: step2. delete successfully and observer return a deleting data info.
     */
    status = DistributedTestTools::Delete(*g_observerDelegate, KEY_1);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<DistributedDB::Entry> deleteEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    deleteEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: DataChange 004
 * @tc.desc: verify that can observer for batch inserting.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange004, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    /**
     * @tc.steps: step1. register observer1 bases on kv db2 and putBatch (k1,v1)(k2,v2).
     * @tc.expected: step1. operate successfully.
     */
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    /**
     * @tc.steps: step2. check callback.
     * @tc.expected: step2. observer1 return a batch inserting data info.
     */
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, entries1));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: DataChange 005
 * @tc.desc: verify that can observer for batch updating.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange005, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2.
     * @tc.expected: step1. operate successfully to construct data and observer1 exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. updateBatch (k1,v1)(k2,v2) to (k1,v2)(k2,v3) and check callback.
     * @tc.expected: step2. observer1 return a batch updating data info.
     */
    vector<Entry> entries2;
    entries2.push_back(ENTRY_1_2);
    entries2.push_back(ENTRY_2_3);
    statusPut = DistributedTestTools::PutBatch(*g_observerDelegate, entries2);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, entries2));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: DataChange 006
 * @tc.desc: verify that can observer for batch deleting.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange006, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2.
     * @tc.expected: step1. operate successfully to construct data and observer1 exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. deleteBatch (k1)(k2) from db and check callback.
     * @tc.expected: step2. observer1 return a batch deleting data info.
     */
    vector<Key> keys1;
    keys1.push_back(ENTRY_1.key);
    keys1.push_back(ENTRY_2.key);
    DBStatus statusDelete = DistributedTestTools::DeleteBatch(*g_observerDelegate, keys1);
    EXPECT_TRUE(statusDelete == DBStatus::OK);

    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, entries1));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: DataChange 007
 * @tc.desc: verify that can observer for clearing.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange007, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2.
     * @tc.expected: step1. operate successfully to construct data and observer1 exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. deleteBatch (k1)(k2) from db and check callback.
     * @tc.expected: step2. observer1 return a clear data info.
     */
    DBStatus statusClear = DistributedTestTools::Clear(*g_observerDelegate);
    EXPECT_TRUE(statusClear == DBStatus::OK);

    vector<DistributedDB::Entry> deleteEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: DataChange 008
 * @tc.desc: verify that observer won't be activated if inserting null key.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange008, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    /**
     * @tc.steps: step1. register observer1 bases on kv db2 and put (k=null,v=ok).
     * @tc.expected: step1. register successfully and put failed.
     */
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    DBStatus statusPut = DistributedTestTools::Put(*g_observerDelegate, KEY_EMPTY, OK_VALUE_1);
    EXPECT_TRUE(statusPut != DBStatus::OK);

    /**
     * @tc.steps: step2. check callback.
     * @tc.expected: step2. observer1 return nothing.
     */
    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: DataChange 009
 * @tc.desc: verify that observer won't be activated if batch inserting failed.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange009, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    /**
     * @tc.steps: step1. register observer1 bases on kv db2 and putBatch (k1=null,v1)(k2,v2).
     * @tc.expected: step1. register successfully and putBatch failed.
     */
    KvStoreObserverImpl observer;

    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<Entry> entries;
    entries.push_back(ENTRY_NULL_1); // null key with VALUE_1.
    entries.push_back(ENTRY_2);
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerDelegate, entries);
    EXPECT_TRUE(statusPut != DBStatus::OK);

    /**
     * @tc.steps: step2. check callback.
     * @tc.expected: step2. observer1 return nothing.
     */
    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

vector<Entry> KvObserverVerifyInsert(KvStoreDelegate *&delegate1, KvStoreDelegate *&delegate2,
    KvStoreObserverImpl &observer1, KvStoreObserverImpl &observer2)
{
    vector<DistributedDB::Entry> insertEntries1;
    vector<DistributedDB::Entry> insertEntries2;
    DBStatus statusPut = DistributedTestTools::Put(*delegate1, KEY_A_1, VALUE_A_1);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    DistributedDB::Entry entry = { KEY_A_1, VALUE_A_1 };
    insertEntries1.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries1));
    observer1.Clear();
    insertEntries2.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, INSERT_LIST, insertEntries2));
    observer2.Clear();

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(TEN_RECORDS, DEFAULT_START, allKeys, entriesBatch);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*delegate1, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);

    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, entriesBatch));
    observer1.Clear();
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, INSERT_LIST, entriesBatch));
    observer2.Clear();

    DBStatus statusDelete = DistributedTestTools::Delete(*delegate1, KEY_A_1);
    EXPECT_TRUE(statusDelete == DBStatus::OK);

    vector<DistributedDB::Entry> deleteEntries1;
    deleteEntries1.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries1));
    observer1.Clear();
    vector<DistributedDB::Entry> deleteEntries2;
    deleteEntries2.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries2));
    observer2.Clear();

    return entriesBatch;
}

vector<DistributedDB::Entry> KvObserverVerifyUpdateAndDelete(KvStoreDelegate *&delegate1, KvStoreDelegate *&delegate2,
    KvStoreObserverImpl &observer1, KvStoreObserverImpl &observer2, vector<Entry> &entriesBatch)
{
    DBStatus statusPut = DistributedTestTools::Put(*delegate1, KEY_1, VALUE_2);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    entriesBatch.front().value = VALUE_2;
    vector<DistributedDB::Entry> updateEntries1;
    DistributedDB::Entry entry1 = { KEY_1, VALUE_2 };
    updateEntries1.push_back(entry1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, UPDATE_LIST, updateEntries1));
    observer1.Clear();
    vector<DistributedDB::Entry> updateEntries2;
    updateEntries2.push_back(entry1);
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, UPDATE_LIST, updateEntries2));
    observer2.Clear();
    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(delegate2, &observer2);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
    for (auto itEntriesBatch = entriesBatch.begin(); itEntriesBatch != entriesBatch.end(); ++itEntriesBatch) {
        itEntriesBatch->value.push_back('A');
    }
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*delegate1, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);

    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, UPDATE_LIST, entriesBatch));
    observer1.Clear();
    updateEntries2.clear();
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, UPDATE_LIST, updateEntries2));
    observer2.Clear();
    vector<Key> keys1;
    vector<DistributedDB::Entry> deleteBatch;
    for (int time = OPER_CNT_START; time < static_cast<int>(OPER_CNT_END); ++time) {
        keys1.push_back(entriesBatch.back().key);
        deleteBatch.push_back(entriesBatch.back());
        entriesBatch.pop_back();
    }
    DBStatus statusDelete = DistributedTestTools::DeleteBatch(*delegate1, keys1);
    EXPECT_TRUE(statusDelete == DBStatus::OK);
    vector<DistributedDB::Entry> deleteEntries1;
    deleteEntries1.clear();
    deleteEntries1 = deleteBatch;
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries1));
    return deleteEntries1;
}

/*
 * @tc.name: DataChange 010
 * @tc.desc: verify that can observer for complex data changing.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange010, TestSize.Level2)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    delegate1 = DistributedTestTools::GetDelegateSuccess(manager1,
        g_kvdbParameter2, g_kvOption);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr) << "fail to create exist kvdb";
    KvOption option = g_kvOption;
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    delegate2 = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter2, option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);
    KvStoreObserverImpl observer1, observer2;

    /**
     * @tc.steps: step1. register observer1 bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(delegate1, &observer1);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. register observer2 bases on kv db.
     * @tc.expected: step2. register successfully.
     */
    status = DistributedTestTools::RegisterObserver(delegate2, &observer2);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step3. put (k="abc",v="a1") and putBatch 10 items (keys,values) then delete (k=abc).
     * @tc.expected: step3. operate successfully and both of observer1,2 return changes data info.
     */
    vector<Entry> entriesBatch = KvObserverVerifyInsert(delegate1, delegate2, observer1, observer2);

    /**
     * @tc.steps: step4. update (k1,v1) to (k1,v2), release observer2 and putBatch then deleteBatch.
     * @tc.expected: step4. operate successfully and only observer1 return changed data info.
     */
    vector<DistributedDB::Entry> deleteEntries1 = KvObserverVerifyUpdateAndDelete(delegate1, delegate2,
        observer1, observer2, entriesBatch);
    observer1.Clear();

    /**
     * @tc.steps: step5. clear db.
     * @tc.expected: step5. observer1 return a delete data info.
     */
    DBStatus statusClear = DistributedTestTools::Clear(*delegate1);
    EXPECT_TRUE(statusClear == DBStatus::OK);
    deleteEntries1.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries1));
    observer1.Clear();

    /**
     * @tc.steps: step6. unregister observer1.
     * @tc.expected: step6. unregister successfully.
     */
    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(delegate1, &observer1);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) == OK);
    delegate1 = nullptr;
    EXPECT_TRUE(manager2->CloseKvStore(delegate2) == OK);
    delegate2 = nullptr;
    status = manager2->DeleteKvStore(STORE_ID_2);
    EXPECT_EQ(status, DistributedDB::DBStatus::OK);
    delete manager1;
    manager1 = nullptr;
    delete manager2;
    manager2 = nullptr;
}

void KvObserverVerifyTransactionCommit(KvStoreObserverImpl &observer1,
    KvStoreDelegate *&delegate1, KvStoreDelegateManager *&transactionManager1)
{
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(TEN_RECORDS, DEFAULT_START, allKeys, entriesBatch);
    DBStatus statusStart = delegate1->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*delegate1, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    DBStatus statusDelete = DistributedTestTools::Delete(*delegate1, KEY_1);
    EXPECT_TRUE(statusDelete == DBStatus::OK);

    entriesBatch.erase(entriesBatch.begin());
    DBStatus statusPut = DistributedTestTools::Put(*delegate1, KEY_2, VALUE_3);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    entriesBatch.front().value = VALUE_3;
    vector<Key> keys;
    for (int time = DELETE_CNT_START; time < static_cast<int>(DELETE_CNT_END); ++time) {
        keys.push_back(entriesBatch.back().key);
        entriesBatch.pop_back();
    }
    statusDelete = DistributedTestTools::DeleteBatch(*delegate1, keys);
    EXPECT_TRUE(statusDelete == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    vector<DistributedDB::Entry> updateEntries;
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, UPDATE_LIST, updateEntries));
    vector<DistributedDB::Entry> deleteEntries;
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, DELETE_LIST, deleteEntries));
    observer1.Clear();
    DBStatus statusCommit = delegate1->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);

    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, entriesBatch));
    updateEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, UPDATE_LIST, updateEntries));
    deleteEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));
    observer1.Clear();
    EXPECT_TRUE(DistributedTestTools::UnRegisterObserver(delegate1, &observer1) == OK);
    EXPECT_TRUE(transactionManager1->CloseKvStore(delegate1) == OK);
    delegate1 = nullptr;
}

void KvObserverVerifyTransactionRollback(KvStoreObserverImpl &observer2, KvStoreDelegate *&delegate2)
{
    vector<Entry> entriesBatch2;
    vector<Key> allKeys2;
    GenerateRecords(TEN_RECORDS, DEFAULT_ANOTHER_START, allKeys2, entriesBatch2);

    DBStatus statusStart = delegate2->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*delegate2, entriesBatch2);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    DBStatus statusDelete = DistributedTestTools::Delete(*delegate2, entriesBatch2[0].key);
    EXPECT_TRUE(statusDelete == DBStatus::OK);
    entriesBatch2.erase(entriesBatch2.begin());
    DBStatus statusPut = DistributedTestTools::Put(*delegate2, entriesBatch2[1].key, VALUE_3);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    entriesBatch2.front().value = VALUE_3;

    vector<Key> keys2;
    for (int time = DELETE_CNT_START; time < static_cast<int>(DELETE_CNT_END); ++time) {
        keys2.push_back(entriesBatch2.at(time).key);
    }
    statusDelete = DistributedTestTools::DeleteBatch(*delegate2, keys2);
    EXPECT_TRUE(statusDelete == DBStatus::OK);

    DBStatus statusRollback = delegate2->Rollback();
    EXPECT_TRUE(statusRollback == DBStatus::OK);
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND));

    // step4: check the result.
    vector<DistributedDB::Entry> insertEntries2;
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries2));
    observer2.Clear();
    vector<DistributedDB::Entry> updateEntries2;
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, UPDATE_LIST, updateEntries2));
    observer2.Clear();
    vector<DistributedDB::Entry> deleteEntries2;
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, DELETE_LIST, deleteEntries2));
    observer2.Clear();
}

/*
 * @tc.name: DataChange 011
 * @tc.desc: verify that can observer for transaction operating changing.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, DataChange011, TestSize.Level2)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer1;
    KvStoreObserverImpl observer2;

    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegateManager *transactionManager1 = nullptr;
    delegate1 = DistributedTestTools::GetDelegateSuccess(transactionManager1,
        g_kvdbParameter2, g_kvOption);
    ASSERT_TRUE(transactionManager1 != nullptr && delegate1 != nullptr);

    /**
     * @tc.steps: step1. register observer1 bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(delegate1, &observer1);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. start transaction, putBatch 10(keys,values), delete(k1), put(k2,v3), deleteBatch(k9)(k8).
     * @tc.expected: step2. operate successfully and observer1 return corresponding changed data info.
     */
    KvObserverVerifyTransactionCommit(observer1, delegate1, transactionManager1);

    /**
     * @tc.steps: step3. register observer2 bases on kv db.
     * @tc.expected: step3. register successfully.
     */
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *transactionManager2 = nullptr;
    delegate2 = DistributedTestTools::GetDelegateSuccess(transactionManager2, g_kvdbParameter1_2_2, g_kvOption);
    ASSERT_TRUE(transactionManager2 != nullptr && delegate2 != nullptr);
    status = DistributedTestTools::RegisterObserver(delegate2, &observer2);
    EXPECT_TRUE(delegate2 != nullptr);

    /**
     * @tc.steps: step4. repeat step2 but don'commit transaction ,rollback it.
     * @tc.expected: step4. operate successfully and observer2 return nothing.
     */
    KvObserverVerifyTransactionRollback(observer2, delegate2);

    EXPECT_TRUE(DistributedTestTools::UnRegisterObserver(delegate2, &observer2) == OK);
    EXPECT_TRUE(transactionManager2->CloseKvStore(delegate2) == OK);
    delegate2 = nullptr;

    status = transactionManager2->DeleteKvStore(STORE_ID_2);
    EXPECT_TRUE(status == DistributedDB::DBStatus::OK);
    delete transactionManager1;
    transactionManager1 = nullptr;
    delete transactionManager2;
    transactionManager2 = nullptr;
}

/*
 * @tc.name: ObserverPressure 001
 * @tc.desc: verify that support many observers for inserting a record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverPressure001, TestSize.Level2)
{
    int countPerDelegate = 8;

    DistributedTestTools::Clear(*g_observerDelegate);

    /**
     * @tc.steps: step1. register 8 observers bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observers[OBSERVER_NUM];
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *transactionManagers = nullptr;
    int obsCnt;

    for (obsCnt = OBSERVER_CNT_START; obsCnt < static_cast<int>(OBSERVER_CNT_END); ++obsCnt) {
        if (obsCnt % countPerDelegate == 0) {
            delegate = DistributedTestTools::GetDelegateSuccess(transactionManagers,
                g_kvdbParameter1, g_kvOption);
            ASSERT_TRUE(transactionManagers != nullptr && delegate != nullptr);
        }
        DBStatus status = DistributedTestTools::RegisterObserver(delegate, &observers[obsCnt]);
        EXPECT_TRUE(status == OK);
    }

    /**
     * @tc.steps: step2. put (k1,v="ok").
     * @tc.expected: step2. put successfully and all of observers can return a inserting data info.
     */
    DBStatus statusPut = DistributedTestTools::Put(*g_observerDelegate, KEY_1, OK_VALUE_1);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    DistributedDB::Entry entry = { KEY_1, OK_VALUE_1 };
    insertEntries.push_back(entry);
    for (obsCnt = OBSERVER_CNT_START; obsCnt < static_cast<int>(OBSERVER_CNT_END); ++obsCnt) {
        EXPECT_TRUE(VerifyObserverResult(observers[obsCnt], CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
        observers[obsCnt].Clear();
    }

    for (obsCnt = OBSERVER_CNT_START; obsCnt < static_cast<int>(OBSERVER_CNT_END); ++obsCnt) {
        EXPECT_EQ(DistributedTestTools::UnRegisterObserver(delegate, &observers[obsCnt]), OK);
        if (obsCnt % countPerDelegate == 7) {
            EXPECT_TRUE(transactionManagers->CloseKvStore(delegate) == OK);
            delegate = nullptr;
            delete transactionManagers;
            transactionManagers = nullptr;
        }
    }
}

/*
 * @tc.name: ObserverPressure 002
 * @tc.desc: verify that support observer for inserting a big key record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverPressure002, TestSize.Level2)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    /**
     * @tc.steps: step1. register an observer bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. put (k,v) that the size of k is 1K.
     * @tc.expected: step2. put successfully and observer can return a inserting data info.
     */
    Entry entryCurrent;
    entryCurrent.key.assign(ONE_K_LONG_STRING, 'a');
    entryCurrent.value = VALUE_1;
    status = DistributedTestTools::Put(*g_observerDelegate, entryCurrent.key, entryCurrent.value);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    insertEntries.push_back(entryCurrent);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: ObserverPressure 003
 * @tc.desc: verify that support observer for inserting a big value record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverPressure003, TestSize.Level2)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. register an observer bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. put (k,v) that the size of v is 4M.
     * @tc.expected: step2. put successfully and observer can return a inserting data info.
     */
    Entry entryCurrent;
    entryCurrent.key = KEY_1;
    entryCurrent.value.assign(FOUR_M_LONG_STRING, 'a');
    status = DistributedTestTools::Put(*g_observerDelegate, entryCurrent.key, entryCurrent.value);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    insertEntries.push_back(entryCurrent);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: ObserverPerformance 001
 * @tc.desc: test performance of subscribing for inserting, deleting, updating one record.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverPerformance001, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. register an observer bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);
    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());

    /**
     * @tc.steps: step2. put (k1,v1) to db and caclute the time of observer return info.
     * @tc.expected: step2. put successfully, printf the time of observer return info after insert data.
     */
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    insertEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer.Clear();
    auto duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick);
    MST_LOG(" Getting notice from subscribing to insert a record costs: %lldus.", (long long)duration.count());
    observer.Clear();

    /**
     * @tc.steps: step3. put (k1,v2) to db and caclute the time of observer return info.
     * @tc.expected: step3. put successfully, printf the time of observer return info after update data.
     */
    tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    status = DistributedTestTools::Put(*g_observerDelegate, KEY_1, VALUE_2);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> updateEntries;
    entry.value = VALUE_2;
    updateEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, updateEntries));
    observer.Clear();
    duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick);
    MST_LOG(" Getting notice from subscribing to update a record costs: %lldus.", (long long)duration.count());
    observer.Clear();

    /**
     * @tc.steps: step4. delete (k1) from db and caclute the time of observer return info.
     * @tc.expected: step4. delete successfully, printf the time of observer return info after delete data.
     */
    tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    status = DistributedTestTools::Delete(*g_observerDelegate, KEY_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> deleteEntries;
    deleteEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));
    observer.Clear();
    duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick);
    MST_LOG(" Getting notice from subscribing to delete a record costs: %lldus.", (long long)duration.count());
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

void KvObserverBatchPerformance(KvStoreObserverImpl &observer, vector<Entry> &entriesBatch)
{
    for (auto itEntriesBatch = entriesBatch.begin(); itEntriesBatch != entriesBatch.end(); ++itEntriesBatch) {
        itEntriesBatch->value.push_back('A');
    }
    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*g_observerDelegate, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, entriesBatch));
    observer.Clear();

    auto duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick);
    MST_LOG(" Getting notice from subscribing to update 10 records costs: %lldus.", (long long)duration.count());
}

/*
 * @tc.name: ObserverPerformance 002
 * @tc.desc: test performance of subscribing for batch inserting, deleting, updating.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverPerformance002, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. register an observer bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. putBatch 10 items (keys,values) to db and caclute the time of observer return info.
     * @tc.expected: step2. putBatch successfully,printf the time of observer return info after insert 10 data.
     */
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(TEN_RECORDS, DEFAULT_START, allKeys, entriesBatch);
    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*g_observerDelegate, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, entriesBatch));
    observer.Clear();

    auto duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick);
    MST_LOG(" Getting notice from subscribing to insert 10 records costs: %lldus.", (long long)duration.count());

    /**
     * @tc.steps: step3. updateBatch (keys,values) and caclute the time of observer return info.
     * @tc.expected: step3. updateBatch successfully,printf the time of observer return info after updateBatch data.
     */
    KvObserverBatchPerformance(observer, entriesBatch);

    /**
     * @tc.steps: step4. deleteBatch (keys) from db and caclute the time of observer return info.
     * @tc.expected: step4. deleteBatch successfully,printf the time of observer return info after deleteBatch data.
     */
    tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    status = DistributedTestTools::DeleteBatch(*g_observerDelegate, allKeys);
    EXPECT_TRUE(status == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, entriesBatch));
    observer.Clear();

    duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick);
    MST_LOG(" Getting notice from subscribing to delete 10 records costs: %lldus.", (long long)duration.count());

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

void KvObserverTransactionPerformance(KvStoreObserverImpl &observer, vector<Entry> &entriesBatch)
{
    for (auto itEntriesBatch = entriesBatch.begin(); itEntriesBatch != entriesBatch.end(); ++itEntriesBatch) {
        itEntriesBatch->value.push_back('A');
    }
    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    DBStatus statusStart = g_observerDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*g_observerDelegate, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    DBStatus statusCommit = g_observerDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, entriesBatch));
    observer.Clear();

    auto duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick);
    MST_LOG(" Getting notice from subscribing a transaction to update 10 records costs: %lldus.",
        (long long)duration.count());
}

/*
 * @tc.name: ObserverPerformance 003
 * @tc.desc: test performance of subscribing for transaction operation.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverPerformance003, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. register an observer bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. start transaction and putBatch 10 items (keys,values) to db then commit.
     * @tc.expected: step2. operate successfully.
     */
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(TEN_RECORDS, DEFAULT_START, allKeys, entriesBatch);
    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    DBStatus statusStart = g_observerDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*g_observerDelegate, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    DBStatus statusCommit = g_observerDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);

    /**
     * @tc.steps: step3. check collback and calculate the duration of step2.
     * @tc.expected: step3. observer return batch inserting data info.
     */
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, entriesBatch));
    observer.Clear();
    auto duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick);
    MST_LOG(" Getting notice from subscribing a transaction to insert 10 records costs: %lldus.",
        (long long)duration.count());

    /**
     * @tc.steps: step4. start transaction and deleteBatch 10 items (keys,values) to db then commit.
     * @tc.expected: step4. operate successfully.
     */
    KvObserverTransactionPerformance(observer, entriesBatch);
    tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    statusStart = g_observerDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    status = DistributedTestTools::DeleteBatch(*g_observerDelegate, allKeys);
    EXPECT_TRUE(status == DBStatus::OK);
    statusCommit = g_observerDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);

    /**
     * @tc.steps: step5. check collback and calculate the duration of step4.
     * @tc.expected: step5. observer return batch deleting data info.
     */
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, entriesBatch));
    observer.Clear();
    duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick);
    MST_LOG(" Getting notice from subscribing to delete a record costs: %lldus.", (long long)duration.count());

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: ObserverPerformance 004
 * @tc.desc: test system info of subscribing for inserting, deleting, updating one record.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverPerformance004, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. register an observer bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: ObserverPerformance 005
 * @tc.desc: test system info of subscribing for batch inserting, deleting, updating.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverPerformance005, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    /**
     * @tc.steps: step1. register an observer bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: ObserverPerformance 006
 * @tc.desc: test system info of subscribing for transaction operation.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverPerformance006, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerDelegate);

    /**
     * @tc.steps: step1. register an observer bases on kv db.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(g_observerDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

#ifndef LOW_LEVEL_MEM_DEV
/*
 * @tc.name: ObserverRekeyDb 001
 * @tc.desc: verify that Rekey will return busy when there are registered observer or putting data to db.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvObserverTest, ObserverRekeyDb001, TestSize.Level3)
{
    KvStoreDelegate *kvObserverDelegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    KvOption option;

    kvObserverDelegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter2, option);
    ASSERT_TRUE(manager != nullptr && kvObserverDelegate != nullptr);

    /**
     * @tc.steps: step1. register observer.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observer;
    DBStatus status = DistributedTestTools::RegisterObserver(kvObserverDelegate, &observer);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. call Rekey to update passwd to passwd_1.
     * @tc.expected: step2. Rekey returns BUSY.
     */
    EXPECT_TRUE(kvObserverDelegate->Rekey(g_passwd1) == BUSY);

    /**
     * @tc.steps: step3. unregister observer.
     * @tc.expected: step3. unregister successfully.
     */
    DBStatus statusRelease = DistributedTestTools::UnRegisterObserver(kvObserverDelegate, &observer);
    EXPECT_TRUE(statusRelease == DBStatus::OK);

    /**
     * @tc.steps: step4. put (1k,4M) of (k,v) to db.
     * @tc.expected: step4. put successfully.
     */
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, BATCH_RECORDS, ONE_K_LONG_STRING, FOUR_M_LONG_STRING);
    thread subThread([&]() {
        DBStatus rekeyStatus = DistributedTestTools::PutBatch(*kvObserverDelegate, entriesBatch);
        EXPECT_TRUE(rekeyStatus == OK || rekeyStatus == BUSY);
    });
    subThread.detach();

    /**
     * @tc.steps: step5. call Rekey to update passwd to passwd_2.
     * @tc.expected: step5. Rekey returns BUSY.
     */
    status = kvObserverDelegate->Rekey(g_passwd2);
    EXPECT_TRUE(status == OK || status == BUSY);
    std::this_thread::sleep_for(std::chrono::seconds(NB_OPERATION_CNT_END));
    EXPECT_TRUE(manager->CloseKvStore(kvObserverDelegate) == OK);
    kvObserverDelegate = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_2) == OK);
    delete manager;
    manager = nullptr;
}
#endif
}
