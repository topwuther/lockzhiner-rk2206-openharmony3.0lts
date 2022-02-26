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

namespace DistributedDBKvObserverSnap {
const bool IS_LOCAL = false;
const int CHANGED_ZERO_TIME = 0;
const int CHANGED_ONE_TIME = 1;
const int OPER_CNT_START = 0;
const int OPER_CNT_END = 5;
const int TEN_RECORDS = 10;
const int DELETE_CNT_START = 0;
const int DELETE_CNT_END = 2;

DistributedDB::CipherPassword g_passwd1;
DistributedDB::CipherPassword g_passwd2;
DistributedDB::CipherPassword g_filePasswd1;
DistributedDB::CipherPassword g_filePasswd2;

class DistributeddbKvObserverSnapTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

KvStoreDelegate *g_observerSnapDelegate = nullptr; // the delegate used in this suit.
KvStoreDelegateManager *g_observerSnapManager = nullptr;
void DistributeddbKvObserverSnapTest::SetUpTestCase(void)
{
    MST_LOG("SetUpTestCase before all cases local[%d].", IS_LOCAL);
    RemoveDir(DIRECTOR);
    g_observerSnapDelegate = DistributedTestTools::GetDelegateSuccess(g_observerSnapManager,
        g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(g_observerSnapManager != nullptr && g_observerSnapDelegate != nullptr);
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    (void)g_passwd2.SetValue(PASSWD_VECTOR_2.data(), PASSWD_VECTOR_2.size());
    (void)g_filePasswd1.SetValue(FILE_PASSWD_VECTOR_1.data(), FILE_PASSWD_VECTOR_1.size());
    (void)g_filePasswd2.SetValue(FILE_PASSWD_VECTOR_2.data(), FILE_PASSWD_VECTOR_2.size());
}

void DistributeddbKvObserverSnapTest::TearDownTestCase(void)
{
    EXPECT_EQ(g_observerSnapManager->CloseKvStore(g_observerSnapDelegate), OK);
    g_observerSnapDelegate = nullptr;
    DBStatus status = g_observerSnapManager->DeleteKvStore(STORE_ID_1);
    EXPECT_EQ(status, DistributedDB::DBStatus::OK) << "fail to delete exist kvdb";
    delete g_observerSnapManager;
    g_observerSnapManager = nullptr;
    RemoveDir(DIRECTOR);
}

void DistributeddbKvObserverSnapTest::SetUp(void)
{
    ASSERT_TRUE(g_observerSnapDelegate != nullptr);
    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());
}

void DistributeddbKvObserverSnapTest::TearDown(void)
{
}

/*
 * @tc.name: Register 001
 * @tc.desc: Verify that can register a fresh observer base on kv db snap.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Register001, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. RegisterSnapObserver base on kv db snap.
     * @tc.expected: step1. Register successfully.
     */
    KvStoreSnapshotDelegate *snap = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_NE(snap, nullptr);

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snap);
    snap = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: Register 002
 * @tc.desc: Verify that can register a null observer on kv db snap.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Register002, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    /**
     * @tc.steps: step1. Register a null observer base on kv db snap.
     * @tc.expected: step1. Register successfully.
     */
    KvStoreSnapshotDelegate *snap = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, nullptr);
    EXPECT_NE(snap, nullptr);

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snap);
    snap = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: Register 003
 * @tc.desc: Verify that can register only once with the same observer.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Register003, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. Register observer 6 times base on kv db snap.
     * @tc.expected: step1. First register is successful and 5 times later return null.
     */
    KvStoreSnapshotDelegate *snapShot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_NE(snapShot, nullptr);
    KvStoreSnapshotDelegate *snap = nullptr;
    for (int time = OPER_CNT_START; time < static_cast<int>(OPER_CNT_END); ++time) {
        snap = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
        EXPECT_EQ(snap, nullptr);
    }

    /**
     * @tc.steps: step2. Register Put (k1,v1) to db.
     * @tc.expected: step2. First put successfully and the observer return inserting data once.
     */
    DBStatus status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertList;
    insertList.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertList));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapShot);
    snapShot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: Register 004
 * @tc.desc: Verify that can register different observers.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Register004, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreObserverImpl observer1, observer2;

    /**
     * @tc.steps: step1. Register observer1 bases on kv db snap.
     * @tc.expected: step1. Register successfully.
     */
    KvStoreSnapshotDelegate *snapShot1 = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer1);
    EXPECT_NE(snapShot1, nullptr);

    /**
     * @tc.steps: step2. put (k1,v1) to db.
     * @tc.expected: step2. put successfully and the observer1 return 1 inserting data info.
     */
    DBStatus status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries1;
    insertEntries1.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries1));
    observer1.Clear();

    /**
     * @tc.steps: step3. register a null observer bases on kv db snap.
     * @tc.expected: step3. register successfully.
     */
    KvStoreSnapshotDelegate *snapShotNull = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, nullptr);
    EXPECT_NE(snapShotNull, nullptr);

    /**
     * @tc.steps: step4. put (k2,v2) to db.
     * @tc.expected: step4. register successfully.
     */
    status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_2, VALUE_2);
    EXPECT_TRUE(status == DBStatus::OK);
    insertEntries1.clear();
    insertEntries1.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries1));
    observer1.Clear();

    /**
     * @tc.steps: step5. register observer2 bases on kv db snap.
     * @tc.expected: step5. register successfully.
     */
    KvStoreSnapshotDelegate *snapShot2 = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer2);
    EXPECT_NE(snapShot2, nullptr);

    /**
     * @tc.steps: step6. put (k3,v3) to db.
     * @tc.expected: step6. put successfully and the observer1,2 return 1 inserting data info individually.
     */
    status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_3, VALUE_3);
    EXPECT_TRUE(status == DBStatus::OK);
    insertEntries1.clear();
    insertEntries1.push_back(ENTRY_3);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries1));
    observer1.Clear();
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ONE_TIME, INSERT_LIST, insertEntries1));
    observer2.Clear();

    EXPECT_EQ((g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapShot1)), DBStatus::OK);
    snapShot1 = nullptr;
    EXPECT_EQ((g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapShotNull)), DBStatus::OK);
    snapShotNull = nullptr;
    EXPECT_EQ((g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapShot2)), DBStatus::OK);
    snapShot2 = nullptr;
}

/*
 * @tc.name: Register 005
 * @tc.desc: Verify that unregister observer (release snapshot) won't return capture data info.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Register005, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. Register observer1 bases on kv db snap.
     * @tc.expected: step1. Register successfully to construct observer exist.
     */
    KvStoreSnapshotDelegate *snapShot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_NE(snapShot, nullptr);

    /**
     * @tc.steps: step2. put (k1,v1) to db.
     * @tc.expected: step2. put successfully and the observer1 return 1 inserting data info.
     */
    DBStatus status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    insertEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    /**
     * @tc.steps: step3. unregister observer1 (release snapshot).
     * @tc.expected: step3. operate successfully.
     */
    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapShot);
    snapShot = nullptr;
    EXPECT_TRUE(statusRelease == DBStatus::OK);

    /**
     * @tc.steps: step4. put (k2,v2) to db.
     * @tc.expected: step4. put successfully and has nothing changed data info returned.
     */
    status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_2, VALUE_2);
    EXPECT_TRUE(status == DBStatus::OK);
    insertEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();
}

/*
 * @tc.name: Register 006
 * @tc.desc: Verify that register observer, and then unregister it many times will success only firstly.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Register006, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. Register observer1 bases on kv db snap.
     * @tc.expected: step1. Register successfully to construct observer exist.
     */
    KvStoreSnapshotDelegate *snapShot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_NE(snapShot, nullptr);

    /**
     * @tc.steps: step2. unregister observer1 for 5 times.
     * @tc.expected: step2. Firstly register successfully and another failed.
     */
    DBStatus statusRelease;
    for (int time = OPER_CNT_START; time < static_cast<int>(OPER_CNT_END); ++time) {
        statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapShot);
        snapShot = nullptr;
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
    DBStatus status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();
}

/*
 * @tc.name: Register 007
 * @tc.desc: Verify that the DB was not register a snapshot can't release it.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Register007, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter2_1_1, g_kvOption);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(delegate != nullptr) << "fail to create exist kvdb";

    /**
     * @tc.steps: step1. Register observer1 bases on kv db1 snap and unregister it bases on kv db2.
     * @tc.expected: step1. Register successfully and unregister failed.
     */
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapShot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_NE(snapShot, nullptr);
    DBStatus statusRelease = delegate->ReleaseKvStoreSnapshot(snapShot);
    EXPECT_TRUE(statusRelease != DBStatus::OK);

    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    DBStatus status = manager->DeleteKvStore(STORE_ID_2);
    EXPECT_TRUE(status == DBStatus::OK) << "fail to delete exist kvdb";
    delete manager;
    manager = nullptr;
    statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapShot);
    snapShot = nullptr;
    EXPECT_TRUE(statusRelease == DBStatus::OK);
}

/*
 * @tc.name: Register 008
 * @tc.desc: Verify that observer will not be triggered after released.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Register008, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    KvOption option = g_kvOption;
    delegate1 = DistributedTestTools::GetDelegateSuccess(manager1, g_kvdbParameter2, option);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr);
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    delegate2 = DistributedTestTools::GetDelegateSuccess(manager2, g_kvdbParameter2, option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);

    /**
     * @tc.steps: step1. Register observer1 bases on kv db2 snap and close db then put(k1,v1).
     * @tc.expected: step1. Register successfully and close failed, observer1 return a changed data info.
     */
    KvStoreObserverImpl observer1;
    KvStoreSnapshotDelegate *snapShot = DistributedTestTools::RegisterSnapObserver(delegate1, &observer1);
    EXPECT_NE(snapShot, nullptr);
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) != OK);
    DBStatus status = DistributedTestTools::Put(*delegate2, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertList;
    insertList.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertList));
    observer1.Clear();

    /**
     * @tc.steps: step2. unregister observer1 and close db then put.
     * @tc.expected: step2. Unregister and close successfully, observer1 return nothing.
     */
    EXPECT_TRUE(delegate1->ReleaseKvStoreSnapshot(snapShot) == DBStatus::OK);
    snapShot = nullptr;
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) == OK);
    delegate1 = nullptr;
    status = DistributedTestTools::Put(*delegate2, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    insertList.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, INSERT_LIST, insertList));
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
 * @tc.name: Register 009
 * @tc.desc: Delete kv db, and verify observer will not be activated.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Register009, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    delegate1 = DistributedTestTools::GetDelegateSuccess(manager1, g_kvdbParameter2, g_kvOption);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr);

    /**
     * @tc.steps: step1. Register observer1 bases on kv db2 snap and close db then put(k1,v1).
     * @tc.expected: step1. Register successfully and close failed, observer1 return a changed data info.
     */
    KvStoreObserverImpl observer1;
    KvStoreSnapshotDelegate *snapShot = DistributedTestTools::RegisterSnapObserver(delegate1, &observer1);
    EXPECT_NE(snapShot, nullptr);
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) != OK);

    DBStatus status = DistributedTestTools::Put(*delegate1, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    insertEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer1.Clear();

    /**
     * @tc.steps: step2. Unregister observer1 and close db then delete db.
     * @tc.expected: step2. operate successfully and observer1 return nothing.
     */
    EXPECT_TRUE(delegate1->ReleaseKvStoreSnapshot(snapShot) == DBStatus::OK);
    snapShot = nullptr;
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) == OK);
    delegate1 = nullptr;
    EXPECT_TRUE(manager1->DeleteKvStore(STORE_ID_2) == OK);

    vector<DistributedDB::Entry> deleteEntries;
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, DELETE_LIST, deleteEntries));
    observer1.Clear();

    EXPECT_TRUE(status == DBStatus::OK) << "fail to delete exist kvdb";
    delete manager1;
    manager1 = nullptr;
}

/*
 * @tc.name: DataChange 001
 * @tc.desc: verify that can observer for inserting a record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange001, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    /**
     * @tc.steps: step1. Register observer1 bases on kv db2 snap.
     * @tc.expected: step1. Register successfully to construct observer exist.
     */
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapShot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapShot != nullptr);

    /**
     * @tc.steps: step2. put (k1,v1) to db.
     * @tc.expected: step2. put successfully and observer1 return a insert data info.
     */
    DBStatus status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    insertEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    status = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapShot);
    snapShot = nullptr;
    EXPECT_TRUE(status == DBStatus::OK);
}

/*
 * @tc.name: DataChange 002
 * @tc.desc: verify that can observer for updating a record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange002, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2 snap.
     * @tc.expected: step1. operate successfully to construct data and observer exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    /**
     * @tc.steps: step2. update (k1,v1) to (k1,v2).
     * @tc.expected: step2. update successfully and observer return a updating data info.
     */
    DBStatus status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_1, VALUE_2);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> updateEntries;
    updateEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, updateEntries));
    observer.Clear();

    status = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_TRUE(status == DBStatus::OK);
}

/*
 * @tc.name: DataChange 003
 * @tc.desc: verify that can observer for deleting a record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange003, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2 snap.
     * @tc.expected: step1. operate successfully to construct data and observer exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    /**
     * @tc.steps: step2. delete from kb.
     * @tc.expected: step2. delete successfully and observer return a deleting data info.
     */
    DBStatus status = DistributedTestTools::Delete(*g_observerSnapDelegate, KEY_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> deleteEntries;
    deleteEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: DataChange 004
 * @tc.desc: verify that can observer for batch inserting.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange004, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    /**
     * @tc.steps: step1. register observer1 bases on kv db2 snap and putBatch (k1,v1)(k2,v2).
     * @tc.expected: step1. operate successfully.
     */
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    /**
     * @tc.steps: step2. check callback.
     * @tc.expected: step2. observer1 return a batch inserting data info.
     */
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, entries1));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: DataChange 005
 * @tc.desc: verify that can observer for batch updating.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange005, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2 snap.
     * @tc.expected: step1. operate successfully to construct data and observer1 exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    /**
     * @tc.steps: step2. updateBatch (k1,v1)(k2,v2) to (k1,v2)(k2,v3) and check callback.
     * @tc.expected: step2. observer1 return a batch updating data info.
     */
    vector<Entry> entries2;
    entries2.push_back(ENTRY_1_2);
    entries2.push_back(ENTRY_2_3);
    statusPut = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entries2);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, entries2));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: DataChange 006
 * @tc.desc: verify that can observer for batch deleting.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange006, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2 snap.
     * @tc.expected: step1. operate successfully to construct data and observer1 exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    /**
     * @tc.steps: step2. deleteBatch (k1)(k2) from db and check callback.
     * @tc.expected: step2. observer1 return a batch deleting data info.
     */
    DBStatus statusDelete = DistributedTestTools::DeleteBatch(*g_observerSnapDelegate, KEYS_1);
    EXPECT_TRUE(statusDelete == DBStatus::OK);

    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, entries1));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: DataChange 007
 * @tc.desc: verify that can observer for clearing.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange007, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) and register observer1 bases on kv db2 snap.
     * @tc.expected: step1. operate successfully to construct data and observer1 exist.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    /**
     * @tc.steps: step2. deleteBatch (k1)(k2) from db and check callback.
     * @tc.expected: step2. observer1 return a clear data info.
     */
    DBStatus statusClear = DistributedTestTools::Clear(*g_observerSnapDelegate);
    EXPECT_TRUE(statusClear == DBStatus::OK);

    vector<DistributedDB::Entry> deleteEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: DataChange 008
 * @tc.desc: verify that observer won't be activated if inserting null key.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange008, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    /**
     * @tc.steps: step1. register observer1 bases on kv db2 snap and put (k=null,v=ok).
     * @tc.expected: step1. register successfully and put failed.
     */
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    DBStatus statusPut = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_EMPTY, OK_VALUE_1);
    EXPECT_TRUE(statusPut != DBStatus::OK);

    /**
     * @tc.steps: step2. check callback.
     * @tc.expected: step2. observer1 return nothing.
     */
    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: DataChange 009
 * @tc.desc: verify that observer won't be activated if batch inserting failed.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange009, TestSize.Level0)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    /**
     * @tc.steps: step1. register observer1 bases on kv db2 snap and putBatch (k1=null,v1)(k2,v2).
     * @tc.expected: step1. register successfully and putBatch failed.
     */
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    vector<Entry> entries;
    entries.push_back(ENTRY_NULL_1); // null key with VALUE_1.
    entries.push_back(ENTRY_2);
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entries);
    EXPECT_TRUE(statusPut != DBStatus::OK);

    /**
     * @tc.steps: step2. check callback.
     * @tc.expected: step2. observer1 return nothing.
     */
    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

void ObserverSnapVerifyInsert(KvStoreDelegate *&delegate1,
    vector<KvStoreObserverImpl> &observers)
{
    vector<DistributedDB::Entry> insertEntries1;
    vector<DistributedDB::Entry> insertEntries2;
    DBStatus statusPut = DistributedTestTools::Put(*delegate1, KEY_A_1, VALUE_A_1);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    insertEntries1.push_back(ENTRY_A_1);
    EXPECT_TRUE(VerifyObserverResult(observers[0], CHANGED_ONE_TIME, INSERT_LIST, insertEntries1));
    observers[0].Clear();

    insertEntries2.push_back(ENTRY_A_1);
    EXPECT_TRUE(VerifyObserverResult(observers[1], CHANGED_ONE_TIME, INSERT_LIST, insertEntries2));
    observers[1].Clear();
}

vector<Entry> ObserverSnapVerifyInsertBatch(KvStoreDelegate *&delegate1, vector<KvStoreObserverImpl> &observers)
{
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(TEN_RECORDS, DEFAULT_START, allKeys, entriesBatch);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*delegate1, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);

    EXPECT_TRUE(VerifyObserverResult(observers[0], CHANGED_ONE_TIME, INSERT_LIST, entriesBatch));
    observers[0].Clear();
    EXPECT_TRUE(VerifyObserverResult(observers[1], CHANGED_ONE_TIME, INSERT_LIST, entriesBatch));
    observers[1].Clear();
    return entriesBatch;
}

void ObserverSnapVerifyDelete(KvStoreDelegate *&delegate1, vector<KvStoreObserverImpl> &observers,
    DistributedDB::Entry &entry)
{
    vector<DistributedDB::Entry> deleteEntries;
    DBStatus statusDelete = DistributedTestTools::Delete(*delegate1, KEY_A_1);
    EXPECT_TRUE(statusDelete == DBStatus::OK);

    deleteEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observers[0], CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));
    observers[0].Clear();

    vector<DistributedDB::Entry> deleteEntries2;
    deleteEntries2.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observers[1], CHANGED_ONE_TIME, DELETE_LIST, deleteEntries2));
    observers[1].Clear();
}

void ObserverSnapVerifyUpdate(KvStoreDelegate *&delegate1, KvStoreDelegate *&delegate2,
    vector<KvStoreObserverImpl> &observers, vector<Entry> &entriesBatch,
    KvStoreSnapshotDelegate *&snapshot2)
{
    DBStatus statusPut = DistributedTestTools::Put(*delegate1, KEY_1, VALUE_2);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    entriesBatch.front().value = VALUE_2;

    vector<DistributedDB::Entry> updateEntries1;
    updateEntries1.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observers[0], CHANGED_ONE_TIME, UPDATE_LIST, updateEntries1));
    observers[0].Clear();

    vector<DistributedDB::Entry> updateEntries2;
    updateEntries2.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observers[1], CHANGED_ONE_TIME, UPDATE_LIST, updateEntries2));
    observers[1].Clear();

    DBStatus statusRelease = delegate2->ReleaseKvStoreSnapshot(snapshot2);
    snapshot2 = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);

    for (auto itEntriesBatch = entriesBatch.begin(); itEntriesBatch != entriesBatch.end(); ++itEntriesBatch) {
        itEntriesBatch->value.push_back('A');
    }
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*delegate1, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observers[0], CHANGED_ONE_TIME, UPDATE_LIST, entriesBatch));
    observers[0].Clear();
}

void ObserverSnapVerifyDeleteBatch(KvStoreDelegate *&delegate1, vector<KvStoreObserverImpl> &observers,
    vector<Entry> &entriesBatch)
{
    vector<DistributedDB::Entry> deleteEntries1;
    vector<Key> keys1;
    vector<DistributedDB::Entry> deleteBatch;
    for (int time = OPER_CNT_START; time < static_cast<int>(OPER_CNT_END); ++time) {
        keys1.push_back(entriesBatch.back().key);
        deleteBatch.push_back(entriesBatch.back());
        entriesBatch.pop_back();
    }
    DBStatus statusDelete = DistributedTestTools::DeleteBatch(*delegate1, keys1);
    EXPECT_TRUE(statusDelete == DBStatus::OK);

    deleteEntries1.clear();
    deleteEntries1 = deleteBatch;
    EXPECT_TRUE(VerifyObserverResult(observers[0], CHANGED_ONE_TIME, DELETE_LIST, deleteEntries1));
    observers[0].Clear();

    DBStatus statusClear = DistributedTestTools::Clear(*delegate1);
    EXPECT_TRUE(statusClear == DBStatus::OK);

    deleteEntries1.clear();
    EXPECT_TRUE(VerifyObserverResult(observers[0], CHANGED_ONE_TIME, DELETE_LIST, deleteEntries1));
    observers[0].Clear();
}

/*
 * @tc.name: DataChange 010
 * @tc.desc: verify that can observer for complex data changing.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange010, TestSize.Level2)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    delegate1 = DistributedTestTools::GetDelegateSuccess(manager1,
        g_kvdbParameter2, g_kvOption);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr) << "fail to create exist kvdb";
    KvOption option = g_kvOption;
    option.createIfNecessary = IS_NOT_NEED_CREATE;
    delegate2 = DistributedTestTools::GetDelegateSuccess(manager2,
        g_kvdbParameter2, option);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);
    KvStoreObserverImpl observer1, observer2;
    vector<KvStoreObserverImpl> observers;
    observers.push_back(observer1);
    observers.push_back(observer2);

    /**
     * @tc.steps: step1. register observer1 bases on kv db snap1.
     * @tc.expected: step1. register successfully.
     */
    KvStoreSnapshotDelegate *snapshot1 = DistributedTestTools::RegisterSnapObserver(delegate1, &observers[0]);
    EXPECT_TRUE(snapshot1 != nullptr);

    /**
     * @tc.steps: step2. register observer2 bases on kv db snap2.
     * @tc.expected: step2. register successfully.
     */
    KvStoreSnapshotDelegate *snapshot2 = DistributedTestTools::RegisterSnapObserver(delegate2, &observers[1]);
    EXPECT_TRUE(snapshot2 != nullptr);

    /**
     * @tc.steps: step3. put (k="abc",v=a1") to db.
     * @tc.expected: step3. put successfully and both of observer1,2 return a putting data info.
     */
    ObserverSnapVerifyInsert(delegate1, observers);

    /**
     * @tc.steps: step3. putBatch 10 items of (keys,values) to db.
     * @tc.expected: step3. putBatch successfully and both of observer1,2 return a batch putting data info.
     */
    vector<Entry> entriesBatch = ObserverSnapVerifyInsertBatch(delegate1, observers);

    /**
     * @tc.steps: step4. delete (k="abc") from db.
     * @tc.expected: step4. delete successfully and both of observer1,2 return a deleting data info.
     */
    DistributedDB::Entry entry = ENTRY_A_1;
    ObserverSnapVerifyDelete(delegate1, observers, entry);

    /**
     * @tc.steps: step5. update (k1,v1) to (k1,v2).
     * @tc.expected: step5. update successfully and both of observer1,2 return a updating data info.
     */
    ObserverSnapVerifyUpdate(delegate1, delegate2, observers, entriesBatch, snapshot2);

    /**
     * @tc.steps: step6. deleteBatch (keys1) from db.
     * @tc.expected: step6. deleteBatch successfully and both of observer1,2 return a batch deleting data info.
     */
    ObserverSnapVerifyDeleteBatch(delegate1, observers, entriesBatch);

    /**
     * @tc.steps: step7. unregister observer1.
     * @tc.expected: step7. unregister successfully.
     */
    DBStatus statusRelease = delegate1->ReleaseKvStoreSnapshot(snapshot1);
    snapshot1 = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
    EXPECT_TRUE(manager1->CloseKvStore(delegate1) == OK);
    delegate1 = nullptr;
    EXPECT_TRUE(manager2->CloseKvStore(delegate2) == OK);
    delegate2 = nullptr;
    DBStatus status = manager2->DeleteKvStore(STORE_ID_2);
    EXPECT_EQ(status, DistributedDB::DBStatus::OK);
    delete manager1;
    manager1 = nullptr;
    delete manager2;
    manager2 = nullptr;
}

vector<Entry> ObserverSnapVerifyTransactionCommit(KvStoreDelegate *&delegate1, KvStoreObserverImpl &observer1)
{
    vector<Entry> entryBatches;
    vector<Key> allKeys;
    GenerateRecords(TEN_RECORDS, DEFAULT_START, allKeys, entryBatches);
    list<DistributedDB::Entry> deleteBatch;
    DBStatus statusStart = delegate1->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*delegate1, entryBatches);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    DBStatus statusDelete = DistributedTestTools::Delete(*delegate1, KEY_1);
    EXPECT_TRUE(statusDelete == DBStatus::OK);
    deleteBatch.push_front(entryBatches[0]);
    entryBatches.erase(entryBatches.begin());
    DBStatus statusPut = DistributedTestTools::Put(*delegate1, KEY_2, VALUE_3);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    entryBatches.front().value = VALUE_3;
    vector<Key> keys1;
    for (int time = DELETE_CNT_START; time < static_cast<int>(DELETE_CNT_END); ++time) {
        keys1.push_back(entryBatches.back().key);
        deleteBatch.push_front(entryBatches.back());
        entryBatches.pop_back();
    }
    statusDelete = DistributedTestTools::DeleteBatch(*delegate1, keys1);
    EXPECT_TRUE(statusDelete == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries, updateEntries, deleteEntries;
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, UPDATE_LIST, updateEntries));
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ZERO_TIME, DELETE_LIST, deleteEntries));
    observer1.Clear();
    DBStatus statusCommit = delegate1->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    insertEntries.clear();
    for (auto eachEntry : entryBatches) {
        insertEntries.push_back(eachEntry);
    }
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    updateEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, UPDATE_LIST, updateEntries));
    deleteEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer1, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));
    observer1.Clear();
    return entryBatches;
}

void ObserverSnapVerifyTransactionRollback(KvStoreDelegate *&delegate2, KvStoreObserverImpl &observer2)
{
    vector<Entry> entriesBatch;
    vector<Key> allKeys2;
    GenerateRecords(TEN_RECORDS, DEFAULT_ANOTHER_START, allKeys2, entriesBatch);

    DBStatus statusStart = delegate2->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*delegate2, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    DBStatus statusDelete = DistributedTestTools::Delete(*delegate2, entriesBatch[0].key);
    EXPECT_TRUE(statusDelete == DBStatus::OK);
    DBStatus statusPut = DistributedTestTools::Put(*delegate2, entriesBatch[1].key, VALUE_3);
    EXPECT_TRUE(statusPut == DBStatus::OK);

    vector<Key> keys2;
    for (int time = DELETE_CNT_START; time < static_cast<int>(DELETE_CNT_END); ++time) {
        keys2.push_back(entriesBatch.at(time).key);
    }
    statusDelete = DistributedTestTools::DeleteBatch(*delegate2, keys2);
    EXPECT_TRUE(statusDelete == DBStatus::OK);

    vector<DistributedDB::Entry> insertEntries;
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    vector<DistributedDB::Entry> updateEntries;
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, UPDATE_LIST, updateEntries));
    vector<DistributedDB::Entry> deleteEntries;
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, DELETE_LIST, deleteEntries));
    DBStatus statusRollback = delegate2->Rollback();
    EXPECT_TRUE(statusRollback == DBStatus::OK);

    // step4: check the result.
    insertEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    updateEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, UPDATE_LIST, updateEntries));
    deleteEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer2, CHANGED_ZERO_TIME, DELETE_LIST, deleteEntries));
    observer2.Clear();
}

/*
 * @tc.name: DataChange 011
 * @tc.desc: verify that can observer for transaction operating changing.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, DataChange011, TestSize.Level2)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreObserverImpl observer1;
    KvStoreObserverImpl observer2;

    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegateManager *transactionManager1 = nullptr;
    delegate1 = DistributedTestTools::GetDelegateSuccess(transactionManager1,
        g_kvdbParameter2, g_kvOption);
    ASSERT_TRUE(transactionManager1 != nullptr && delegate1 != nullptr);

    /**
     * @tc.steps: step1. register observer1 bases on kv db snap.
     * @tc.expected: step1. register successfully.
     */
    KvStoreSnapshotDelegate *snapshot1 = DistributedTestTools::RegisterSnapObserver(delegate1, &observer1);
    EXPECT_TRUE(snapshot1 != nullptr);

    /**
     * @tc.steps: step2. start transaction, putBatch 10(keys,values), delete(k1), put(k2,v3), deleteBatch(k9)(k8).
     * @tc.expected: step2. operate successfully and observer1 return corresponding changed data info.
     */
    vector<Entry> entriesBatch = ObserverSnapVerifyTransactionCommit(delegate1, observer1);

    /**
     * @tc.steps: step3. unregister observer1.
     * @tc.expected: step3. unregister successfully.
     */
    DBStatus statusRelease = delegate1->ReleaseKvStoreSnapshot(snapshot1);
    snapshot1 = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
    EXPECT_TRUE(transactionManager1->CloseKvStore(delegate1) == OK);
    delegate1 = nullptr;

    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *transactionManager2 = nullptr;
    delegate2 = DistributedTestTools::GetDelegateSuccess(transactionManager2,
        g_kvdbParameter1_2_2, g_kvOption);
    ASSERT_TRUE(transactionManager2 != nullptr && delegate2 != nullptr);

    /**
     * @tc.steps: step4. register observer2 bases on kv db snap.
     * @tc.expected: step4. register successfully.
     */
    KvStoreSnapshotDelegate *snapshot2 = DistributedTestTools::RegisterSnapObserver(delegate2, &observer2);
    EXPECT_TRUE(snapshot2 != nullptr);

    /**
     * @tc.steps: step5. repeat step2 but don'commit transaction ,rollback it.
     * @tc.expected: step5. operate successfully and observer2 return nothing.
     */
    ObserverSnapVerifyTransactionRollback(delegate2, observer2);

    statusRelease = delegate2->ReleaseKvStoreSnapshot(snapshot2);
    snapshot2 = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
    EXPECT_TRUE(transactionManager2->CloseKvStore(delegate2) == OK);
    delegate2 = nullptr;
    DBStatus status = transactionManager2->DeleteKvStore(STORE_ID_2);
    EXPECT_TRUE(status == DistributedDB::DBStatus::OK);
    delete transactionManager1;
    transactionManager1 = nullptr;
    delete transactionManager2;
    transactionManager2 = nullptr;
}

/*
 * @tc.name: Performance 001
 * @tc.desc: test performance of subscribing for inserting, deleting, updating one record.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Performance001, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);
    KvStoreObserverImpl observer;
    /**
     * @tc.steps: step1. register an observer bases on kv db snap.
     * @tc.expected: step1. register successfully.
     */
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);
    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());

    /**
     * @tc.steps: step2. put (k1,v1) to db and caclute the time of observer return info.
     * @tc.expected: step2. put successfully, printf the time of observer return info after insert data.
     */
    DBStatus status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> insertEntries;
    insertEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, insertEntries));
    observer.Clear();
    double duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick).count();
    MST_LOG(" Getting notice from subscribing to insert a record costs: %lldus.", (long long int)duration);

    /**
     * @tc.steps: step3. put (k1,v2) to db and caclute the time of observer return info.
     * @tc.expected: step3. put successfully, printf the time of observer return info after update data.
     */
    tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    status = DistributedTestTools::Put(*g_observerSnapDelegate, KEY_1, VALUE_2);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> updateEntries;
    updateEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, updateEntries));
    observer.Clear();
    duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick).count();
    MST_LOG(" Getting notice from subscribing to update a record costs: %lldus.", (long long int)duration);

    /**
     * @tc.steps: step4. delete (k1) from db and caclute the time of observer return info.
     * @tc.expected: step4. delete successfully, printf the time of observer return info after delete data.
     */
    tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    status = DistributedTestTools::Delete(*g_observerSnapDelegate, KEY_1);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<DistributedDB::Entry> deleteEntries;
    deleteEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, deleteEntries));
    observer.Clear();
    duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick).count();
    MST_LOG(" Getting notice from subscribing to delete a record costs: %lldus.", (long long int)duration);

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

void ObserverSnapBatchPerformance(KvStoreObserverImpl &observer, vector<Entry> &entriesBatch)
{
    for (auto itEntriesBatch = entriesBatch.begin(); itEntriesBatch != entriesBatch.end(); ++itEntriesBatch) {
        itEntriesBatch->value.push_back('A');
    }
    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, entriesBatch));
    observer.Clear();

    double duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick).count();
    MST_LOG(" Getting notice from subscribing to update 10 records costs: %lldus.", (long long int)duration);
    observer.Clear();
}

/*
 * @tc.name: Performance 002
 * @tc.desc: test performance of subscribing for batch inserting, deleting, updating.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Performance002, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. register an observer bases on kv db snap.
     * @tc.expected: step1. register successfully.
     */
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    /**
     * @tc.steps: step2. putBatch 10 items (keys,values) to db and caclute the time of observer return info.
     * @tc.expected: step2. putBatch successfully,printf the time of observer return info after insert 10 data.
     */
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(TEN_RECORDS, DEFAULT_START, allKeys, entriesBatch);

    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, entriesBatch));
    observer.Clear();
    double duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick).count();
    MST_LOG(" Getting notice from subscribing to insert 10 records costs: %lldus.", (long long int)duration);

    /**
     * @tc.steps: step3. updateBatch (keys,values) and caclute the time of observer return info.
     * @tc.expected: step3. updateBatch successfully,printf the time of observer return info after updateBatch data.
     */
    observer.Clear();
    ObserverSnapBatchPerformance(observer, entriesBatch);

    /**
     * @tc.steps: step4. deleteBatch (keys) from db and caclute the time of observer return info.
     * @tc.expected: step4. deleteBatch successfully,printf the time of observer return info after deleteBatch data.
     */
    tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    DBStatus status = DistributedTestTools::DeleteBatch(*g_observerSnapDelegate, allKeys);
    EXPECT_TRUE(status == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, entriesBatch));
    observer.Clear();

    duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick).count();
    MST_LOG(" Getting notice from subscribing to delete 10 records costs: %lldus.", (long long int)duration);

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

void ObserverSnapTransactionPerformance(vector<Entry> &entriesBatch,
    vector<Key> &allKeys, KvStoreObserverImpl &observer)
{
    for (auto itEntriesBatch = entriesBatch.begin(); itEntriesBatch != entriesBatch.end(); ++itEntriesBatch) {
        itEntriesBatch->value.push_back('A');
    }
    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    DBStatus statusStart = g_observerSnapDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    DBStatus statusCommit = g_observerSnapDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, UPDATE_LIST, entriesBatch));
    observer.Clear();

    double duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick).count();
    MST_LOG(" Getting notice from subscribing a transaction to update 10 records costs: %lldus.",
        (long long int)duration);

    tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    statusStart = g_observerSnapDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus status = DistributedTestTools::DeleteBatch(*g_observerSnapDelegate, allKeys);
    EXPECT_TRUE(status == DBStatus::OK);
    statusCommit = g_observerSnapDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, DELETE_LIST, entriesBatch));
    observer.Clear();
    duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick).count();
    MST_LOG("Getting notice from subscribing to delete a record costs: %lldus.", (long long int)duration);
}

/*
 * @tc.name: Performance 003
 * @tc.desc: test performance of subscribing for transaction operation.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Performance003, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. register an observer bases on kv db snap.
     * @tc.expected: step1. register successfully.
     */
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    /**
     * @tc.steps: step2. start transaction and putBatch 10 items (keys,values) to db then commit.
     * @tc.expected: step2. operate successfully.
     */
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(TEN_RECORDS, DEFAULT_START, allKeys, entriesBatch);
    auto tick = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    DBStatus statusStart = g_observerSnapDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*g_observerSnapDelegate, entriesBatch);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    DBStatus statusCommit = g_observerSnapDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);

    /**
     * @tc.steps: step3. check collback and calculate the duration of step2.
     * @tc.expected: step3. observer return batch inserting data info.
     */
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ONE_TIME, INSERT_LIST, entriesBatch));
    observer.Clear();

    double duration = std::chrono::duration_cast<microseconds>(observer.GetOnChangeTime() - tick).count();
    MST_LOG(" Getting notice from subscribing a transaction to insert 10 records costs: %lldus.",
        (long long int)duration);

    ObserverSnapTransactionPerformance(entriesBatch, allKeys, observer);

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: Performance 004
 * @tc.desc: test system info of subscribing for inserting, deleting, updating one record.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Performance004, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    KvStoreObserverImpl observer;

    /**
     * @tc.steps: step1. register an observer bases on kv db snap.
     * @tc.expected: step1. register successfully.
     */
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    vector<DistributedDB::Entry> insertEntries;
    insertEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: Performance 005
 * @tc.desc: test system info of subscribing for batch inserting, deleting, updating.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Performance005, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);

    /**
     * @tc.steps: step1. register an observer bases on kv db snap.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    vector<DistributedDB::Entry> insertEntries;
    insertEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: Performance 006
 * @tc.desc: test system info of subscribing for transaction operation.
 * @tc.type: Performance
 * @tc.require: SR000BUH3K,SR000BVRNE
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvObserverSnapTest, Performance006, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_observerSnapDelegate);
    /**
     * @tc.steps: step1. register an observer bases on kv db snap.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(g_observerSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    vector<DistributedDB::Entry> insertEntries;
    insertEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observer, CHANGED_ZERO_TIME, INSERT_LIST, insertEntries));
    observer.Clear();

    DBStatus statusRelease = g_observerSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);
}

/*
 * @tc.name: RekeyDb 001
 * @tc.desc: verify that Rekey will return busy when there are registered snapshot.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbKvObserverSnapTest, RekeyDb001, TestSize.Level0)
{
    KvStoreDelegate *kvObserverSnapDelegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    KvOption option;

    kvObserverSnapDelegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter2, option);
    ASSERT_TRUE(manager != nullptr && kvObserverSnapDelegate != nullptr);

    /**
     * @tc.steps: step1. register an observer bases on kv db snap.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observer;
    KvStoreSnapshotDelegate *snapshot = DistributedTestTools::RegisterSnapObserver(kvObserverSnapDelegate, &observer);
    EXPECT_TRUE(snapshot != nullptr);

    /**
     * @tc.steps: step2. call Rekey to update passwd to passwd_1.
     * @tc.expected: step2. Rekey returns BUSY.
     */
    EXPECT_TRUE(kvObserverSnapDelegate->Rekey(g_passwd1) == BUSY);

    DBStatus statusRelease = kvObserverSnapDelegate->ReleaseKvStoreSnapshot(snapshot);
    snapshot = nullptr;
    EXPECT_EQ(statusRelease, DBStatus::OK);

    EXPECT_TRUE(manager->CloseKvStore(kvObserverSnapDelegate) == OK);
    kvObserverSnapDelegate = nullptr;

    DBStatus status = manager->DeleteKvStore(STORE_ID_2);
    EXPECT_TRUE(status == DBStatus::OK) << "fail to delete exist kvdb";
    delete manager;
    manager = nullptr;
}
}