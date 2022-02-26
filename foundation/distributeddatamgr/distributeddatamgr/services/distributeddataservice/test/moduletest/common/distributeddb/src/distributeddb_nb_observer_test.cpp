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
#include <chrono>
#include <thread>
#include <cstdio>
#include <random>
#include <string>

#include "types.h"
#include "kv_store_delegate.h"
#include "kv_store_nb_delegate.h"
#include "distributeddb_nb_test_tools.h"
#include "kv_store_delegate_manager.h"
#include "distributeddb_data_generator.h"
#include "distributed_test_tools.h"

using namespace std;
using namespace chrono;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbNbObserver {
const int CHANGED_ZERO_TIME = 0;
const int CHANGED_ONE_TIME = 1;
const int STORE_NUM = 2;

const unsigned int ANY_RECORDS_NUM_START = 1;
const unsigned int ANY_RECORDS_NUM_END = 1000;

const unsigned int LONG_TIME_TEST_SECONDS = 10;
const unsigned int LONG_TIME_INTERVAL_MILLSECONDS = 5;

KvStoreNbDelegate *g_nbObserverDelegate = nullptr;
KvStoreDelegateManager *g_nbObserverManager = nullptr;

struct ConcurParam {
    unsigned int threadId_;
    ReadOrWriteTag tag_;
    Entry* entryPtr_;
};

DistributedDB::CipherPassword g_passwd1;
DistributedDB::CipherPassword g_passwd2;

class DistributeddbNbObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbObserverTest::SetUpTestCase(void)
{
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    (void)g_passwd2.SetValue(PASSWD_VECTOR_2.data(), PASSWD_VECTOR_2.size());
}

void DistributeddbNbObserverTest::TearDownTestCase(void)
{
}

void DistributeddbNbObserverTest::SetUp(void)
{
    MST_LOG("SetUpTest before case local.");
    RemoveDir(NB_DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    g_nbObserverDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_nbObserverManager,
        g_dbParameter1, g_option);
    EXPECT_TRUE(g_nbObserverManager != nullptr && g_nbObserverDelegate != nullptr);
}

void DistributeddbNbObserverTest::TearDown(void)
{
    EXPECT_TRUE(EndCaseDeleteDB(g_nbObserverManager, g_nbObserverDelegate, STORE_ID_1, g_option.isMemoryDb));
    RemoveDir(NB_DIRECTOR);
}

void RegisterAndUnRegisterObserver(ConcurParam* paramsPtr)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    DBStatus status = g_nbObserverDelegate->RegisterObserver(
        paramsPtr->entryPtr_->key, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(
        paramsPtr->entryPtr_->key, OBSERVER_CHANGES_NATIVE, &observerSync);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);
}

void ConcurOperThread(ConcurParam* args)
{
    auto paramsPtr = static_cast<ConcurParam *>(args);
    DBStatus status;
    Value valueResult;

    if (paramsPtr->tag_ == ReadOrWriteTag::READ) {
        status = DistributedDBNbTestTools::Get(*g_nbObserverDelegate, paramsPtr->entryPtr_->key, valueResult);
        if (valueResult.size() != 0) {
            EXPECT_EQ(status, OK);
            EXPECT_TRUE(DistributedDBNbTestTools::isValueEquals(valueResult, paramsPtr->entryPtr_->value));
        }
    } else if (paramsPtr->tag_ == ReadOrWriteTag::WRITE) {
        status = DistributedDBNbTestTools::Put(*g_nbObserverDelegate,
            paramsPtr->entryPtr_->key, paramsPtr->entryPtr_->value);
        ASSERT_EQ(status, DBStatus::OK);

        status = DistributedDBNbTestTools::Get(*g_nbObserverDelegate, paramsPtr->entryPtr_->key, valueResult);
        EXPECT_EQ(status, OK);
        EXPECT_TRUE(DistributedDBNbTestTools::isValueEquals(valueResult, paramsPtr->entryPtr_->value));
    } else if (paramsPtr->tag_ == ReadOrWriteTag::DELETE) {
        status = DistributedDBNbTestTools::Get(*g_nbObserverDelegate, paramsPtr->entryPtr_->key, valueResult);
        if (valueResult.size() != 0) {
            status = DistributedDBNbTestTools::Delete(*g_nbObserverDelegate, paramsPtr->entryPtr_->key);
            ASSERT_EQ(status, DBStatus::OK);

            valueResult.clear();
            status = DistributedDBNbTestTools::Get(*g_nbObserverDelegate, paramsPtr->entryPtr_->key, valueResult);
            EXPECT_EQ(status, NOT_FOUND);
            EXPECT_EQ(valueResult.size(), (size_t)0);
        } else {
            status = DistributedDBNbTestTools::Delete(*g_nbObserverDelegate, paramsPtr->entryPtr_->key);
            ASSERT_EQ(status, DBStatus::OK);
        }
    } else {
        RegisterAndUnRegisterObserver(paramsPtr);
    }
}

/*
 * @tc.name: RegisterData 001
 * @tc.desc: Verify that observer of local db and sync db can't effect each other.
 * @tc.type: FUNC
 * @tc.require: SR000CQDVH
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, RegisterData001, TestSize.Level0)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    /**
     * @tc.steps: step1. register local observer1 use OBSERVER_CHANGES_LOCAL_ONLY mode.
     * @tc.expected: step1. register success.
     */
    DBStatus status = g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. register sync observer2 use OBSERVER_CHANGES_NATIVE.
     * @tc.expected: step2. register success.
     */
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observerSync);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step3. verify that if observer1 will be triggered when put (KEY_1, VALUE_1) to local db.
     * @tc.expected: step3. observer1 will be response but observer2 won't.
     */
    EXPECT_EQ((g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1)), OK);
    vector<DistributedDB::Entry> insertLocalEntries;
    insertLocalEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, INSERT_LIST, insertLocalEntries));
    vector<DistributedDB::Entry> insertNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerLocal.Clear();
    /**
     * @tc.steps: step4. verify that if observer1 will be triggered when delete (KEY_1, VALUE_1) from local db.
     * @tc.expected: step4. observer1 will be response but observer2 won't.
     */
    EXPECT_EQ((g_nbObserverDelegate->DeleteLocal(KEY_1)), OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    deleteLocalEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerLocal.Clear();
    /**
     * @tc.steps: step5. verify that if observer2 will be triggered when put (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step5. observer2 will be response but observer1 won't.
     */
    EXPECT_EQ((g_nbObserverDelegate->Put(KEY_1, VALUE_1)), OK);
    insertLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    insertNativeEntries.clear();
    insertNativeEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, INSERT_LIST, insertNativeEntries));
    observerSync.Clear();
    /**
     * @tc.steps: step6. verify that if observer2 will be triggered when delete (KEY_1, VALUE_1) from sync db.
     * @tc.expected: step6. observer1 will be response but observer2 won't.
     */
    EXPECT_EQ((g_nbObserverDelegate->Delete(KEY_1)), OK);
    deleteLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    deleteNativeEntries.clear();
    deleteNativeEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, DELETE_LIST, deleteNativeEntries));
    observerSync.Clear();
    g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    g_nbObserverDelegate->UnRegisterObserver(&observerSync);
}

void CheckObserverAllLocalValue(KvStoreObserverImpl &observerLocal)
{
    /**
     * @tc.steps: step2. put one entries (KEY_1, VALUE_1) to local db.
     * @tc.expected: step2. observerLocal will be response and the data observerLocal got is (KEY_1, VALUE_1).
     */
    DBStatus status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> insertLocalEntries;
    insertLocalEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, INSERT_LIST, insertLocalEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step3. update one entries (KEY_2, VALUE_2) of local db.
     * @tc.expected: step3. observerLocal will be response and the data observerLocal got is (KEY_2, VALUE_2).
     */
    status = g_nbObserverDelegate->PutLocal(KEY_2, VALUE_2);
    EXPECT_EQ(status, OK);
    insertLocalEntries.clear();
    insertLocalEntries.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, INSERT_LIST, insertLocalEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step4. delete one entry of local db where key = KEY_1.
     * @tc.expected: step4. observerLocal will be response and the data observerLocal got is (KEY_1, VALUE_1).
     */
    status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    deleteLocalEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, DELETE_LIST, deleteLocalEntries));
    observerLocal.Clear();
}

/*
 * @tc.name: RegisterData 002
 * @tc.desc: Verify that can observer all records of local db.
 * @tc.type: FUNC
 * @tc.require: SR000CQDVH
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, RegisterData002, TestSize.Level0)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    /**
     * @tc.steps: step1. register local observerLocal use empty key KEY_EMPTY.
     * @tc.expected: step1. register success.
     */
    DBStatus status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);

    CheckObserverAllLocalValue(observerLocal);

    /**
     * @tc.steps: step5. put one entry (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step5. observerLocal won't be response.
     */
    status = g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> insertNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerSync.Clear();

    /**
     * @tc.steps: step6. put one entry (KEY_2, VALUE_2) to sync db.
     * @tc.expected: step6. observerLocal won't be response.
     */
    status = g_nbObserverDelegate->Put(KEY_2, VALUE_2);
    EXPECT_EQ(status, OK);
    insertNativeEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerSync.Clear();

    /**
     * @tc.steps: step7. delete one entry from sync db where key = KEY_1.
     * @tc.expected: step7. observerLocal won't be response.
     */
    status = g_nbObserverDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerSync.Clear();

    g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    g_nbObserverDelegate->DeleteLocal(KEY_2);
    g_nbObserverDelegate->Delete(KEY_2);
}

void CheckObserverAllNativeValue(KvStoreObserverImpl &observerLocal, KvStoreObserverImpl &observerSync)
{
    /**
     * @tc.steps: step2. put one entries (KEY_1, VALUE_1) to local db.
     * @tc.expected: step2. observerLocal won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1), OK);
    vector<DistributedDB::Entry> insertLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step3. update one entries (KEY_2, VALUE_2) of local db.
     * @tc.expected: step3. observerLocal won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->PutLocal(KEY_2, VALUE_2), OK);
    insertLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step4. delete one entry of local db where key = KEY_1.
     * @tc.expected: step4. observerLocal won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->DeleteLocal(KEY_1), OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step5. put one entry (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step5. observerSync will be response and the data observerSync got is (KEY_1, VALUE_1).
     */
    EXPECT_EQ(g_nbObserverDelegate->Put(KEY_1, VALUE_1), OK);
    vector<DistributedDB::Entry> insertNativeEntries;
    insertNativeEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, INSERT_LIST, insertNativeEntries));
    observerSync.Clear();

    /**
     * @tc.steps: step6. update one entry (KEY_2, VALUE_3) to sync db.
     * @tc.expected: step6. observerSync will be response and the data observerSync got is (KEY_2, VALUE_3).
     */
    EXPECT_EQ(g_nbObserverDelegate->Put(KEY_2, VALUE_3), OK);
    insertNativeEntries.clear();
    insertNativeEntries.push_back(ENTRY_2_3);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, INSERT_LIST, insertNativeEntries));
    observerSync.Clear();
}

/*
 * @tc.name: RegisterData 003
 * @tc.desc: Verify that can observer all records of sync db.
 * @tc.type: FUNC
 * @tc.require: SR000CQDVH
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, RegisterData003, TestSize.Level0)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    /**
     * @tc.steps: step1. register sync observerSync use empty key KEY_EMPTY.
     * @tc.expected: step1. register success.
     */
    DBStatus status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_NATIVE, &observerSync);
    EXPECT_EQ(status, OK);

    CheckObserverAllNativeValue(observerLocal, observerSync);

    status = g_nbObserverDelegate->Put(KEY_2, VALUE_2);
    vector<DistributedDB::Entry> updateNativeEntries;
    updateNativeEntries.push_back(ENTRY_2);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, UPDATE_LIST, updateNativeEntries));
    observerSync.Clear();

    EXPECT_EQ(g_nbObserverDelegate->Put(KEY_2, VALUE_3), OK);
    updateNativeEntries.clear();
    updateNativeEntries.push_back(ENTRY_2_3);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, UPDATE_LIST, updateNativeEntries));
    observerSync.Clear();

    /**
     * @tc.steps: step7. delete one entry from sync db where key = KEY_1.
     * @tc.expected: step7. observerSync will be response and the data observerSync got is (KEY_1, VALUE_1).
     */
    EXPECT_EQ(g_nbObserverDelegate->Delete(KEY_1), OK);
    vector<DistributedDB::Entry> deleteNativeEntries;
    deleteNativeEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, DELETE_LIST, deleteNativeEntries));
    observerSync.Clear();

    g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    g_nbObserverDelegate->DeleteLocal(KEY_2);
    g_nbObserverDelegate->Delete(KEY_2);
}

/*
 * @tc.name: UnRegister 001
 * @tc.desc: Verify that the record didn't register won't trigger the observer.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, UnRegister001, TestSize.Level0)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    /**
     * @tc.steps: step1. register local observerLocal use key = KEY_1.
     * @tc.expected: step1. register success.
     */
    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. register sync observerSync use  key = KEY_2.
     * @tc.expected: step2. register success.
     */
    status = g_nbObserverDelegate->RegisterObserver(KEY_2, OBSERVER_CHANGES_NATIVE, &observerSync);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step3. put one entries (KEY_2, VALUE_2) to local db.
     * @tc.expected: step3. observerLocal won't be response and observerSync won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->PutLocal(KEY_2, VALUE_2), OK);
    vector<DistributedDB::Entry> insertLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    vector<DistributedDB::Entry> insertNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step4. delete one entries from local db where key = KEY_2.
     * @tc.expected: step4. observerLocal won't be response and observerSync won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->DeleteLocal(KEY_2), OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step5. put one entries (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step5. observerLocal won't be response and observerSync won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->Put(KEY_1, VALUE_1), OK);
    insertLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    insertNativeEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerSync.Clear();

    /**
     * @tc.steps: step6. delete one entries from sync db where key = KEY_1.
     * @tc.expected: step6. observerLocal won't be response and observerSync won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->Delete(KEY_1), OK);
    deleteLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    deleteNativeEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerSync.Clear();
    g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    g_nbObserverDelegate->UnRegisterObserver(&observerSync);
}

/*
 * @tc.name: UnRegister 002
 * @tc.desc: Verify that UnRegister local of KEY_1 observer and sync observer won't be affected.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, UnRegister002, TestSize.Level0)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observerSync);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step1. UnRegister local observer of KEY_1.
     * @tc.expected: step1. UnRegister success.
     */
    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. delete one entries from local db where key = KEY_1.
     * @tc.expected: step2. observerLocal won't be response and observerSync won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->DeleteLocal(KEY_1), OK);
    vector<DistributedDB::Entry> deleteLocalEntries, deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step3. delete one entries from sync db where key = KEY_1.
     * @tc.expected: step3. observerLocal won't be response but observerSync will be response
     *    and got record (KEY_1, VALUE_1).
     */
    EXPECT_EQ(g_nbObserverDelegate->Delete(KEY_1), OK);
    deleteNativeEntries.clear();
    deleteNativeEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, DELETE_LIST, deleteNativeEntries));
    deleteLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    observerSync.Clear();
    g_nbObserverDelegate->UnRegisterObserver(&observerSync);
}

/*
 * @tc.name: UnRegister 003
 * @tc.desc: Verify that UnRegister sync observer of KEY_1 and local observer won't be affected.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, UnRegister003, TestSize.Level0)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_TRUE(status == OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observerSync);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step1. UnRegister sync observer of KEY_1.
     * @tc.expected: step1. UnRegister success.
     */
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step2. delete one entries from local db where key = KEY_1.
     * @tc.expected: step2. observerLocal will be response and got record (KEY_1, VALUE_1)
     *    but observerSync won't be response.
     */
    EXPECT_TRUE(g_nbObserverDelegate->DeleteLocal(KEY_1) == OK);
    vector<DistributedDB::Entry> deleteLocalEntry;
    deleteLocalEntry.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, DELETE_LIST, deleteLocalEntry));
    vector<DistributedDB::Entry> deleteNativeEntry;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntry));
    observerLocal.Clear();

    /**
     * @tc.steps: step3. delete one entries from sync db where key = KEY_1.
     * @tc.expected: step3. observerLocal won't be response and observerSync won't be response.
     */
    EXPECT_TRUE(g_nbObserverDelegate->Delete(KEY_1) == OK);
    deleteLocalEntry.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntry));
    deleteNativeEntry.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntry));
    observerSync.Clear();
    g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
}

/*
 * @tc.name: UnRegister 004
 * @tc.desc: Verify that UnRegister local observer of all-key and sync observer won't be affected.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, UnRegister004, TestSize.Level0)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_NATIVE, &observerSync);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step1. UnRegister local observer of KEY_EMPTY.
     * @tc.expected: step1. UnRegister success.
     */
    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. delete one entries from local db where key = KEY_1.
     * @tc.expected: step2. observerLocal won't be response but observerSync won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->DeleteLocal(KEY_1), OK);
    vector<DistributedDB::Entry> insertLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    vector<DistributedDB::Entry> insertNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step3. delete one entries from sync db where key = KEY_1.
     * @tc.expected: step3. observerLocal won't be response but observerSync will be response
     *    and got record (KEY_1, VALUE_1).
     */
    EXPECT_EQ(g_nbObserverDelegate->Delete(KEY_1), OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    deleteNativeEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, DELETE_LIST, deleteNativeEntries));
    observerSync.Clear();
    g_nbObserverDelegate->UnRegisterObserver(&observerSync);
}

/*
 * @tc.name: UnRegister 005
 * @tc.desc: Verify that UnRegister sync observer of all-key and local observer won't be affected.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, UnRegister005, TestSize.Level0)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_NATIVE, &observerSync);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step1. UnRegister sync observer of KEY_EMPTY.
     * @tc.expected: step1. UnRegister success.
     */
    EXPECT_EQ(g_nbObserverDelegate->UnRegisterObserver(&observerSync), OK);
    /**
     * @tc.steps: step2. delete one entries from local db where key = KEY_1.
     * @tc.expected: step2. observerLocal will be response and got record (KEY_1, VALUE_1)
     *    but observerSync won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->DeleteLocal(KEY_1), OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    deleteLocalEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step3. delete one entries from sync db where key = KEY_1.
     * @tc.expected: step3. observerLocal won't be response and observerSync won't be response.
     */
    EXPECT_EQ(g_nbObserverDelegate->Delete(KEY_1), OK);
    deleteLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerSync.Clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    deleteNativeEntries.clear();
    g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
}

/*
 * @tc.name: ParamCheck 001
 * @tc.desc: Verify that can check effectiveness of params mode.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, ParamCheck001, TestSize.Level0)
{
    KvStoreObserverImpl observer1, observer2, observer3, observer4, observer;

    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observer1);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_NATIVE, &observer2);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_FOREIGN, &observer3);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1,
        OBSERVER_CHANGES_FOREIGN | OBSERVER_CHANGES_NATIVE, &observer4);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step1. Register observer with the mode is not in (1, 2, 3, 4).
     * @tc.expected: step1. Register failed and return INVALID_ARGS.
     */
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, 0, &observer); // invalid mode number 0
    EXPECT_EQ(status, INVALID_ARGS);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, 5, &observer); // invalid mode number 5
    EXPECT_EQ(status, INVALID_ARGS);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, -1, &observer); // invalid mode number -1
    EXPECT_EQ(status, INVALID_ARGS);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, 2147483647, &observer); // invalid mode number 2147483647
    EXPECT_EQ(status, INVALID_ARGS);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, -2147483648, &observer); // invalid mode number -2147483648
    EXPECT_EQ(status, INVALID_ARGS);
    status = g_nbObserverDelegate->RegisterObserver(KEY_1, 999, &observer); // invalid mode number 999
    EXPECT_EQ(status, INVALID_ARGS);
    g_nbObserverDelegate->UnRegisterObserver(&observer1);
    g_nbObserverDelegate->UnRegisterObserver(&observer2);
    g_nbObserverDelegate->UnRegisterObserver(&observer3);
    g_nbObserverDelegate->UnRegisterObserver(&observer4);
}

/*
 * @tc.name: ParamCheck 002
 * @tc.desc: Verify that can check effectiveness of params key.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, ParamCheck002, TestSize.Level0)
{
    DistributedDB::Key eKey1, eKey2, eKey3;
    eKey1.assign(ONE_K_LONG_STRING, (uint8_t)'a');
    eKey2.assign(ONE_K_LONG_STRING + 1, (uint8_t)'b');
    eKey3 = { 'a', 'b', 'c', 'D', 'E', 'F', '2', '4', '5', 199, 1, 255, 0 };
    KvStoreObserverImpl observer1, observer2, observer3, observer4, observer5, observer6;

    /**
     * @tc.steps: step1. Register local observer with the key = eKey1 size of which is 1024.
     * @tc.expected: step1. Register success.
     */
    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(eKey1, OBSERVER_CHANGES_LOCAL_ONLY, &observer1);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. Register local observer with the key = eKey2 size of which is 1025.
     * @tc.expected: step2. Register failed and return INVALID_ARGS.
     */
    status = g_nbObserverDelegate->RegisterObserver(eKey2, OBSERVER_CHANGES_LOCAL_ONLY, &observer2);
    EXPECT_EQ(status, INVALID_ARGS);
    /**
     * @tc.steps: step3. Register local observer with the key = eKey3 which contains
     *    [a-zA-Z0-9], [\0-\255], chinese and latins.
     * @tc.expected: step3. Register failed and return INVALID_ARGS.
     */
    status = g_nbObserverDelegate->RegisterObserver(eKey3, OBSERVER_CHANGES_LOCAL_ONLY, &observer3);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step4. Register sync observer with the key = eKey1 size of which is 1024.
     * @tc.expected: step4. Register success.
     */
    status = g_nbObserverDelegate->RegisterObserver(eKey1, OBSERVER_CHANGES_NATIVE, &observer4);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step5. Register local observer with the key = eKey2 size of which is 1025.
     * @tc.expected: step5. Register failed and return INVALID_ARGS.
     */
    status = g_nbObserverDelegate->RegisterObserver(eKey2, OBSERVER_CHANGES_NATIVE, &observer5);
    EXPECT_EQ(status, INVALID_ARGS);
    /**
     * @tc.steps: step6. Register local observer with the key = eKey3 which contains
     *    [a-zA-Z0-9], [\0-\255], chinese and latins.
     * @tc.expected: step6. Register failed and return INVALID_ARGS.
     */
    status = g_nbObserverDelegate->RegisterObserver(eKey3, OBSERVER_CHANGES_NATIVE, &observer6);
    EXPECT_EQ(status, OK);
    g_nbObserverDelegate->UnRegisterObserver(&observer1);
    g_nbObserverDelegate->UnRegisterObserver(&observer2);
    g_nbObserverDelegate->UnRegisterObserver(&observer3);
    g_nbObserverDelegate->UnRegisterObserver(&observer4);
    g_nbObserverDelegate->UnRegisterObserver(&observer5);
    g_nbObserverDelegate->UnRegisterObserver(&observer6);
}

void CheckPressureActionInNative(KvStoreObserverImpl &observerLocal, KvStoreObserverImpl &observerSync)
{
    /**
     * @tc.steps: step3. put one entries (KEY_1, VALUE_1) to local db.
     * @tc.expected: step3. observerLocal won't be response.
     */
    DBStatus status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> insertLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    vector<DistributedDB::Entry> insertNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step4. put one entries (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step4. observerSync will be response and the data observerSync got is (KEY_1, VALUE_1).
     */
    status = g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    insertLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    insertNativeEntries.clear();
    insertNativeEntries.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, INSERT_LIST, insertNativeEntries));
    observerSync.Clear();

    /**
     * @tc.steps: step5. update one entries (KEY_1, VALUE_2) to local db.
     * @tc.expected: step5. observerLocal won't be response.
     */
    status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> updateLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, updateLocalEntries));
    vector<DistributedDB::Entry> updateNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, updateNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step6. update one entries (KEY_1, VALUE_2) to sync db.
     * @tc.expected: step6. observerSync will be response and the data observerSync got is (KEY_1, VALUE_2).
     */
    status = g_nbObserverDelegate->Put(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);
    updateLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, updateLocalEntries));
    updateNativeEntries.clear();
    updateNativeEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, UPDATE_LIST, updateNativeEntries));
    observerSync.Clear();
}

/*
 * @tc.name: Pressure 001
 * @tc.desc: Verify that local db can observer the key that do not exist.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure001, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;

    /**
     * @tc.steps: step1. Register local observer with the key = KEY_A_1 which do not exist in db.
     * @tc.expected: step1. Register success.
     */
    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(KEY_A_1, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. Register sync observer with the key = KEY_EMPTY.
     * @tc.expected: step2. Register success.
     */
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(status, OK);

    CheckPressureActionInNative(observerLocal, observerSync);

    /**
     * @tc.steps: step7. delete one entries from local db where key = KEY_1.
     * @tc.expected: step7. observerLocal won't be response.
     */
    status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step8. delete one entries from sync db where key = KEY_1.
     * @tc.expected: step8. observerLocal won't be response but observerSync will be response and
     *     the data observerSync got is (KEY_1, VALUE_2).
     */
    status = g_nbObserverDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);
    deleteLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    deleteNativeEntries.clear();
    deleteNativeEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, DELETE_LIST, deleteNativeEntries));
    observerSync.Clear();

    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);
}

void CheckPressureActionInLocal(KvStoreObserverImpl &observerLocal, KvStoreObserverImpl &observerSync)
{
    /**
     * @tc.steps: step3. put one entries (KEY_1, VALUE_1) to local db.
     * @tc.expected: step3. observerLocal will be response and the data observerLocal got is (KEY_1, VALUE_1).
     */
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    vector<DistributedDB::Entry> insertLocalEntry;
    insertLocalEntry.push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, INSERT_LIST, insertLocalEntry));
    vector<DistributedDB::Entry> insertNativeEntry;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntry));
    observerLocal.Clear();

    /**
     * @tc.steps: step4. put one entries (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step4. observerLocal won't be response.
     */
    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    insertLocalEntry.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntry));
    insertNativeEntry.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntry));
    observerSync.Clear();

    /**
     * @tc.steps: step5. update one entries (KEY_1, VALUE_2) to local db.
     * @tc.expected: step5. observerLocal will be response and the data observerLocal got is (KEY_1, VALUE_2).
     */
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_2);
    vector<DistributedDB::Entry> updateLocalEntry;
    updateLocalEntry.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, UPDATE_LIST, updateLocalEntry));
    vector<DistributedDB::Entry> updateNativeEntry;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, updateNativeEntry));
    observerLocal.Clear();

    /**
     * @tc.steps: step6. update one entries (KEY_1, VALUE_2) to sync db.
     * @tc.expected: step6. observerSync won't be response.
     */
    g_nbObserverDelegate->Put(KEY_1, VALUE_2);
    updateLocalEntry.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, updateLocalEntry));
    updateNativeEntry.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, updateNativeEntry));
    observerSync.Clear();
}

/*
 * @tc.name: Pressure 002
 * @tc.desc: Verify that sync db can observer the key that do not exist.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure002, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;

    /**
     * @tc.steps: step1. Register local observer with the key = KEY_EMPTY.
     * @tc.expected: step1. Register success.
     */
    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. Register sync observer with the key = KEY_A_1 which do not exist in db.
     * @tc.expected: step2. Register success.
     */
    status = g_nbObserverDelegate->RegisterObserver(KEY_A_1, OBSERVER_CHANGES_NATIVE, &observerSync);
    EXPECT_EQ(status, OK);

    CheckPressureActionInLocal(observerLocal, observerSync);

    /**
     * @tc.steps: step7. delete one entries from local db where key = KEY_1.
     * @tc.expected: step7. observerLocal will be response and the data observerLocal got is (KEY_1, VALUE_2)
     *    but observerSync won't be response.
     */
    status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_2 };
    deleteLocalEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step8. delete one entries from sync db where key = KEY_1.
     * @tc.expected: step8. observerSync won't be response.
     */
    status = g_nbObserverDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);
    deleteNativeEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    deleteLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    observerSync.Clear();
    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);
}

/*
 * @tc.name: Pressure 003
 * @tc.desc: Verify that can't unregister observer that do not exist.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure003, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;

    /**
     * @tc.steps: step1. UnRegister local and sync Observer.
     * @tc.expected: step1. it will be both failed to unregister observerLocal and observerSync.
     */
    DBStatus status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, NOT_FOUND);

    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, NOT_FOUND);
}

/*
 * @tc.name: Pressure 004
 * @tc.desc: Verify that can't unregister nullptr.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. UnRegister nullpter.
     * @tc.expected: step1. it will be failed to unregister nullptr and return INVALID_ARGS.
     */
    DBStatus status = g_nbObserverDelegate->UnRegisterObserver(nullptr);

    EXPECT_EQ(status, INVALID_ARGS);
}

void CheckPressureActionAfterUnregister(KvStoreObserverImpl &observerLocal, KvStoreObserverImpl &observerSync)
{
    /**
     * @tc.steps: step4. Crud to local and sync db.
     * @tc.expected: step4. Both observerLocal and observerSync won't be response.
     */
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    vector<DistributedDB::Entry> insertLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    vector<DistributedDB::Entry> insertNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerLocal.Clear();

    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    insertLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    insertNativeEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerSync.Clear();

    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_2);
    vector<DistributedDB::Entry> updateNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, updateNativeEntries));
    vector<DistributedDB::Entry> updateLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, updateLocalEntries));
    observerLocal.Clear();

    g_nbObserverDelegate->Put(KEY_1, VALUE_2);
    updateNativeEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, updateNativeEntries));
    updateLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, updateLocalEntries));
    observerSync.Clear();

    DBStatus status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerLocal.Clear();
}

/*
 * @tc.name: Pressure 005
 * @tc.desc: Verify that can't unregister observer repeately.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure005, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;

    DBStatus status =
        g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step1. UnRegister local and sync observer the first time.
     * @tc.expected: step1. it will be both success to unregister observerLocal and observerSync.
     */
    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step2. UnRegister local and sync observer the second time.
     * @tc.expected: step2. both failed to unregister observerLocal and observerSync, and return NOT_FOUND.
     */
    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, NOT_FOUND);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, NOT_FOUND);

    /**
     * @tc.steps: step3. UnRegister local and sync observer the third time.
     * @tc.expected: step3. both failed to unregister observerLocal and observerSync, and return NOT_FOUND.
     */
    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, NOT_FOUND);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, NOT_FOUND);

    CheckPressureActionAfterUnregister(observerLocal, observerSync);

    status = g_nbObserverDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> deleteLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerSync.Clear();
}

/*
 * @tc.name: Pressure 006
 * @tc.desc: Verify that can register and unregister observer repeately.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure006, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    DBStatus status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step1. Register and UnRegister the observer repeately.
     * @tc.expected: step1. Register and unregister observerLocal and observerSync success each time.
     */
    for (unsigned int opCnt = NB_OPERATION_CNT_START; opCnt < NB_OPERATION_CNT_END; ++opCnt) {
        status = g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
        EXPECT_EQ(status, OK);
        status = g_nbObserverDelegate->RegisterObserver(KEY_1,
            OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
        EXPECT_EQ(status, OK);

        status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
        EXPECT_EQ(status, OK);
        status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
        EXPECT_EQ(status, OK);
    }

    status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);
}

void CheckPressureForRepeatAction(KvStoreObserverImpl &observerLocal, KvStoreObserverImpl &observerSync)
{
    /**
     * @tc.steps: step2. crud KEY_1 of local and sync db.
     * @tc.expected: step2. observerLocal and observerSync can response each time.
     */
    DBStatus status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);
    vector<DistributedDB::Entry> updateLocalEntries;
    updateLocalEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, UPDATE_LIST, updateLocalEntries));
    vector<DistributedDB::Entry> updateNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, updateNativeEntries));
    observerLocal.Clear();

    status = g_nbObserverDelegate->Put(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);
    updateLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, updateLocalEntries));
    updateNativeEntries.clear();
    updateNativeEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, UPDATE_LIST, updateNativeEntries));
    observerSync.Clear();

    vector<DistributedDB::Entry> deleteLocalEntries;
    vector<DistributedDB::Entry> deleteNativeEntries;
    status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    deleteLocalEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, DELETE_LIST, deleteLocalEntries));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerLocal.Clear();

    status = g_nbObserverDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);

    deleteNativeEntries.push_back(ENTRY_1_2);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, DELETE_LIST, deleteNativeEntries));
    deleteLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    observerSync.Clear();
}

/*
 * @tc.name: Pressure 007
 * @tc.desc: Verify that can't register an observer of the same key repeately without unrigister.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure007, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal, observerSync;
    DBStatus status = g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step1. Register a local and a sync observer of one same key KEY_EMPTY 5 times separately.
     * @tc.expected: step1. Register observerLocal and observerSync success first time and failed later.
     */
    for (unsigned int opCnt = NB_OPERATION_CNT_START; opCnt < NB_OPERATION_CNT_END; ++opCnt) {
        status = g_nbObserverDelegate->RegisterObserver(KEY_1, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
        if (opCnt == NB_OPERATION_CNT_START) {
            EXPECT_EQ(status, OK);
        } else {
            EXPECT_EQ(status, DB_ERROR);
        }
        status = g_nbObserverDelegate->RegisterObserver(KEY_1,
            OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
        if (opCnt == NB_OPERATION_CNT_START) {
            EXPECT_EQ(status, OK);
        } else {
            EXPECT_EQ(status, DB_ERROR);
        }
    }

    CheckPressureForRepeatAction(observerLocal, observerSync);

    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);
}

void CheckPressureForLocalRepeat(KvStoreObserverImpl &observerLocal, KvStoreObserverImpl &observerSync,
    vector< vector<DistributedDB::Entry> > &entries)
{
    /**
     * @tc.steps: step2. Crud on local db.
     * @tc.expected: step2. Each operator observerLocal response one time.
     */
    DBStatus status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);

    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    entries[INSERT_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    observerLocal.Clear();

    status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);

    entry.value = VALUE_2;
    entries[UPDATE_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));

    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_NATIVE]));
    observerLocal.Clear();

    status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    entries[DELETE_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    observerLocal.Clear();

    /**
     * @tc.steps: step3. Crud on sync db.
     * @tc.expected: step3. Each operator observerSync won't be response.
     */
    status = g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    entries[INSERT_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    entries[INSERT_NATIVE].clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    observerSync.Clear();
}

/*
 * @tc.name: Pressure 008
 * @tc.desc: Verify that can't register a local observer of all key repeately.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure008, TestSize.Level1)
{
    DBStatus status;
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;

    /**
     * @tc.steps: step1. Register a local observer of key KEY_EMPTY 5 times repeately.
     * @tc.expected: step1. Register observerLocal success first time and failed later.
     */
    for (unsigned int opCnt = NB_OPERATION_CNT_START; opCnt < NB_OPERATION_CNT_END; ++opCnt) {
        status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
        if (opCnt == NB_OPERATION_CNT_START) {
            EXPECT_EQ(status, OK);
        } else {
            EXPECT_EQ(status, DB_ERROR);
        }
    }

    vector< vector<DistributedDB::Entry> > entries(6); // 6 element
    CheckPressureForLocalRepeat(observerLocal, observerSync, entries);

    status = g_nbObserverDelegate->Put(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);
    entries[UPDATE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    entries[UPDATE_NATIVE].clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_NATIVE]));
    observerSync.Clear();

    status = g_nbObserverDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);
    entries[DELETE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    entries[DELETE_NATIVE].clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    observerSync.Clear();

    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
}

void CheckPressureForNativeRepeat(KvStoreObserverImpl &observerLocal, KvStoreObserverImpl &observerSync,
    vector< vector<DistributedDB::Entry> > &entries)
{
    /**
     * @tc.steps: step2. Crud on local db.
     * @tc.expected: step2. Each operator observerLocal won't be response.
     */
    DBStatus status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    observerLocal.Clear();

    status = g_nbObserverDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_NATIVE]));
    observerLocal.Clear();

    status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    observerLocal.Clear();

    /**
     * @tc.steps: step3. Crud on Sync db.
     * @tc.expected: step3. Each operator observerSync will be response one time.
     */
    status = g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    entries[INSERT_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    entries[INSERT_NATIVE].clear();
    entries[INSERT_NATIVE].push_back(ENTRY_1);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    observerSync.Clear();
}

/*
 * @tc.name: Pressure 009
 * @tc.desc: Verify that register a sync observer of the same key repeately.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure009, TestSize.Level1)
{
    DBStatus status;
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;

    /**
     * @tc.steps: step1. Register a sync observer of key KEY_EMPTY 5 times repeately.
     * @tc.expected: step1. Register observerSync success first time and failed later.
     */
    for (unsigned int opCnt = NB_OPERATION_CNT_START; opCnt < NB_OPERATION_CNT_END; ++opCnt) {
        status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
            OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
        if (opCnt == NB_OPERATION_CNT_START) {
            EXPECT_EQ(status, OK);
        } else {
            EXPECT_EQ(status, DB_ERROR);
        }
    }

    vector< vector<DistributedDB::Entry> > entries(6); // 6 element
    CheckPressureForNativeRepeat(observerLocal, observerSync, entries);

    status = g_nbObserverDelegate->Put(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);
    entries[UPDATE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    entries[UPDATE_NATIVE].clear();
    DistributedDB::Entry entry = { KEY_1, VALUE_2 };
    entries[UPDATE_NATIVE].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, UPDATE_LIST, entries[UPDATE_NATIVE]));
    observerSync.Clear();

    status = g_nbObserverDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);
    entries[DELETE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    entries[DELETE_NATIVE].clear();
    entries[DELETE_NATIVE].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    observerSync.Clear();

    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);
}

/*
 * @tc.name: Pressure 012
 * @tc.desc: Verify that insert unusual key-value can't trigger the observer.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure012, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;

    DBStatus status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step1. put a unusual key = KEY_EMPTY to local and sync db and check corresponding observer.
     * @tc.expected: step1. both observerLocal and observerSync won't be response.
     */
    g_nbObserverDelegate->PutLocal(KEY_EMPTY, VALUE_1);
    vector<DistributedDB::Entry> insertLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    vector<DistributedDB::Entry> insertNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerLocal.Clear();

    g_nbObserverDelegate->Put(KEY_EMPTY, VALUE_1);
    insertLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    insertNativeEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
    observerSync.Clear();

    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);
}

/*
 * @tc.name: Pressure 013
 * @tc.desc: Verify that delete the record that do not exist can't trigger the observer.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure013, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;

    DBStatus status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step1. delete a record that do not exist in the local and sync db and check corresponding observer.
     * @tc.expected: step1. both observerLocal and observerSync won't be response.
     */
    g_nbObserverDelegate->DeleteLocal(KEY_1);
    vector<DistributedDB::Entry> deleteLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerLocal.Clear();

    g_nbObserverDelegate->Delete(KEY_1);
    deleteLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    deleteNativeEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));
    observerSync.Clear();

    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);
}

/*
 * @tc.name: Pressure 014
 * @tc.desc: Verify that delete the record that do not exist can't trigger the observer.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure014, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    DBStatus status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step1. put a record (KEY_1, VALUE_1) to local db and check observerLocal.
     * @tc.expected: step1. observerLocal will be response and observerSync won't be response.
     */
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    vector<DistributedDB::Entry> updateLocalEntries;
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    updateLocalEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, UPDATE_LIST, updateLocalEntries));
    vector<DistributedDB::Entry> updateNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, updateNativeEntries));
    observerLocal.Clear();

    /**
     * @tc.steps: step2. put a record (KEY_1, VALUE_1) to sync db and check observerSync.
     * @tc.expected: step2. observerLocal won't be response and observerSync will be response.
     */
    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    updateLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, updateLocalEntries));
    updateNativeEntries.clear();
    updateNativeEntries.push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, UPDATE_LIST, updateNativeEntries));
    observerSync.Clear();

    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);
}

void CheckPressureAfterClose(KvStoreObserverImpl &observerLocal, KvStoreObserverImpl &observerSync,
    vector< vector<DistributedDB::Entry> > &entries)
{
    /**
     * @tc.steps: step3. Crud on local and sync db and check corresponding observerLocal or observerSync.
     * @tc.expected: step3. nor observerLocal neither observerSync will be response.
     */
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    observerLocal.Clear();

    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    entries[INSERT_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    entries[INSERT_NATIVE].clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    observerSync.Clear();

    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_NATIVE]));
    observerLocal.Clear();

    g_nbObserverDelegate->Put(KEY_1, VALUE_2);
    entries[UPDATE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    entries[UPDATE_NATIVE].clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_NATIVE]));
    observerSync.Clear();

    DBStatus status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    observerLocal.Clear();

    status = g_nbObserverDelegate->Delete(KEY_1);
    entries[DELETE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    entries[DELETE_NATIVE].clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    observerSync.Clear();

    g_nbObserverDelegate->DeleteLocal(KEY_1);
    g_nbObserverDelegate->Delete(KEY_1);
}
/*
 * @tc.name: Pressure 015
 * @tc.desc: Verify that close db and open it again, observer won't be triggered again.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure015, TestSize.Level1)
{
    KvStoreObserverImpl observerLocal, observerSync;
    DBStatus status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step1. close db.
     * @tc.expected: step1. closed success.
     */
    EXPECT_EQ(g_nbObserverManager->CloseKvStore(g_nbObserverDelegate), OK);
    g_nbObserverDelegate = nullptr;
    delete g_nbObserverManager;
    g_nbObserverManager = nullptr;
    /**
     * @tc.steps: step2. open db again.
     * @tc.expected: step2. opened success.
     */
    Option option = g_option;
    if (!option.isMemoryDb) {
        option.createIfNecessary = IS_NOT_NEED_CREATE;
    }
    g_nbObserverDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_nbObserverManager,
        g_dbParameter1, option);
    ASSERT_TRUE(g_nbObserverManager != nullptr && g_nbObserverDelegate != nullptr);

    vector< vector<DistributedDB::Entry> > entries(6); // 6 element
    CheckPressureAfterClose(observerLocal, observerSync, entries);

    vector<DistributedDB::Entry> insertLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, insertLocalEntries));
    vector<DistributedDB::Entry> insertNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, insertNativeEntries));
}

void CheckPressureAfterReopen(KvStoreObserverImpl &observerLocal, KvStoreObserverImpl &observerSync,
    vector< vector<DistributedDB::Entry> > &entries)
{
    /**
     * @tc.steps: step5. Crud on local and sync db and check corresponding observer.
     * @tc.expected: step5. It will be correctly triggered when crud  where key = KEY_1.
     */
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_1);
    entries[INSERT_LOCAL].clear();
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    entries[INSERT_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    entries[INSERT_NATIVE].clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    observerLocal.Clear();

    g_nbObserverDelegate->Put(KEY_1, VALUE_1);
    entries[INSERT_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    entries[INSERT_NATIVE].clear();
    entries[INSERT_NATIVE].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    observerSync.Clear();
    g_nbObserverDelegate->PutLocal(KEY_1, VALUE_2);
    entries[UPDATE_LOCAL].clear();
    entry.value = VALUE_2;
    entries[UPDATE_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    entries[UPDATE_NATIVE].clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_NATIVE]));
    observerLocal.Clear();

    g_nbObserverDelegate->Put(KEY_1, VALUE_2);
    entries[UPDATE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    entries[UPDATE_NATIVE].clear();
    entries[UPDATE_NATIVE].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, UPDATE_LIST, entries[UPDATE_NATIVE]));
    observerSync.Clear();

    DBStatus status = g_nbObserverDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    entries[DELETE_LOCAL].clear();
    entries[DELETE_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ONE_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    entries[DELETE_NATIVE].clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    observerLocal.Clear();
}

/*
 * @tc.name: Pressure 016
 * @tc.desc: Verify that close db and open it again, observer won't be triggered again,
 *    register observer again and it will response.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure016, TestSize.Level2)
{
    KvStoreObserverImpl observerSync, observerLocal;
    DBStatus status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step1. close db.
     * @tc.expected: step1. closed success.
     */
    EXPECT_EQ(g_nbObserverManager->CloseKvStore(g_nbObserverDelegate), OK);
    g_nbObserverDelegate = nullptr;
    delete g_nbObserverManager;
    g_nbObserverManager = nullptr;
    /**
     * @tc.steps: step2. open db again.
     * @tc.expected: step2. opened success.
     */
    if (g_option.isMemoryDb) {
        g_nbObserverDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_nbObserverManager,
            g_dbParameter1, g_option);
    } else {
        Option option = g_option;
        option.createIfNecessary = IS_NOT_NEED_CREATE;
        g_nbObserverDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_nbObserverManager,
            g_dbParameter1, option);
    }
    ASSERT_TRUE(g_nbObserverManager != nullptr && g_nbObserverDelegate != nullptr);
    vector< vector<DistributedDB::Entry> > entries(6); // 6 element
    CheckPressureAfterClose(observerLocal, observerSync, entries);

    /**
     * @tc.steps: step3. register observerLocal and observerSync on key = KEY_EMPTY again.
     * @tc.expected: step3. register success.
     */
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(status, OK);

    CheckPressureAfterReopen(observerLocal, observerSync, entries);

    status = g_nbObserverDelegate->Delete(KEY_1);
    entries[DELETE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    entries[DELETE_NATIVE].clear();
    DistributedDB::Entry entry = { KEY_1, VALUE_2 };
    entries[DELETE_NATIVE].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ONE_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    observerSync.Clear();

    status = g_nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = g_nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);
}

/*
 * @tc.name: Pressure 017
 * @tc.desc: Verify that close db and observer won't be triggered again,
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure017, TestSize.Level2)
{
    KvStoreObserverImpl observerSync, observerLocal;
    DBStatus registerStatus = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(registerStatus, OK);
    registerStatus = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(registerStatus, OK);

    /**
     * @tc.steps: step1. close db and check whether the observer was triggered.
     * @tc.expected: step1. closed success and the observer won't response.
     */
    EXPECT_EQ(g_nbObserverManager->CloseKvStore(g_nbObserverDelegate), OK);
    g_nbObserverDelegate = nullptr;
    delete g_nbObserverManager;
    g_nbObserverManager = nullptr;

    /**
     * @tc.steps: step2. check the local and sync observer after the data base was closed.
     * @tc.expected: step2. neither the local observer nor the sync observer was triggered.
     */
    vector<DistributedDB::Entry> deleteLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));

    if (g_option.isMemoryDb) {
        g_nbObserverDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_nbObserverManager,
            g_dbParameter1, g_option);
    } else {
        Option option = g_option;
        option.createIfNecessary = IS_NOT_NEED_CREATE;
        g_nbObserverDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_nbObserverManager,
            g_dbParameter1, option);
    }
    ASSERT_TRUE(g_nbObserverManager != nullptr && g_nbObserverDelegate != nullptr);
}

/*
 * @tc.name: Pressure 018
 * @tc.desc: Verify that close db and observer won't be triggered again,
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure018, TestSize.Level2)
{
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    DBStatus statusLocal = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(statusLocal, OK);
    DBStatus statusSync = g_nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(statusSync, OK);

    /**
     * @tc.steps: step1. close and delete db and check whether the observer was triggered.
     * @tc.expected: step1. closed and deleted success and the observer won't response.
     */
    EXPECT_EQ(g_nbObserverManager->CloseKvStore(g_nbObserverDelegate), OK);
    g_nbObserverDelegate = nullptr;
    vector<DistributedDB::Entry> deleteLocalEntries;
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    vector<DistributedDB::Entry> deleteNativeEntries;
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));

    if (!g_option.isMemoryDb) {
        DBStatus status = g_nbObserverManager->DeleteKvStore(STORE_ID_1);
        EXPECT_TRUE(status == OK);
    }
    delete g_nbObserverManager;
    g_nbObserverManager = nullptr;
    deleteLocalEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocal, CHANGED_ZERO_TIME, DELETE_LIST, deleteLocalEntries));
    deleteNativeEntries.clear();
    EXPECT_TRUE(VerifyObserverResult(observerSync, CHANGED_ZERO_TIME, DELETE_LIST, deleteNativeEntries));

    RemoveDir(DIRECTOR);
    g_nbObserverDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_nbObserverManager,
        g_dbParameter1, g_option);
    ASSERT_TRUE(g_nbObserverManager != nullptr && g_nbObserverDelegate != nullptr);
}

void CheckPressureAcrossDatabase(vector<KvStoreNbDelegate *> &nbDelegateVec, KvStoreObserverImpl *observerLocals,
    KvStoreObserverImpl *observerSyncs, vector< vector<DistributedDB::Entry> > &entries, unsigned int &opCnt)
{
    DBStatus status = nbDelegateVec[opCnt]->PutLocal(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    DistributedDB::Entry entry = { KEY_1, VALUE_1 };
    entries[INSERT_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocals[opCnt], CHANGED_ONE_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSyncs[opCnt], CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    observerLocals[opCnt].Clear();
    status = nbDelegateVec[opCnt]->Put(KEY_1, VALUE_1);
    EXPECT_EQ(status, OK);
    entries[INSERT_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocals[opCnt], CHANGED_ZERO_TIME, INSERT_LIST, entries[INSERT_LOCAL]));
    entries[INSERT_NATIVE].clear();
    entries[INSERT_NATIVE].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerSyncs[opCnt], CHANGED_ONE_TIME, INSERT_LIST, entries[INSERT_NATIVE]));
    entries[INSERT_NATIVE].clear();
    observerSyncs[opCnt].Clear();
    status = nbDelegateVec[opCnt]->PutLocal(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);
    entry.value = VALUE_2;
    entries[UPDATE_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocals[opCnt], CHANGED_ONE_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSyncs[opCnt], CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_NATIVE]));
    observerLocals[opCnt].Clear();
    status = nbDelegateVec[opCnt]->Put(KEY_1, VALUE_2);
    EXPECT_EQ(status, OK);
    entries[UPDATE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocals[opCnt], CHANGED_ZERO_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    entries[UPDATE_LOCAL].clear();
    entries[UPDATE_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerSyncs[opCnt], CHANGED_ONE_TIME, UPDATE_LIST, entries[UPDATE_LOCAL]));
    entries[UPDATE_LOCAL].clear();
    observerSyncs[opCnt].Clear();
    EXPECT_EQ((nbDelegateVec[opCnt]->DeleteLocal(KEY_1)), OK);
    entries[DELETE_LOCAL].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerLocals[opCnt], CHANGED_ONE_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    EXPECT_TRUE(VerifyObserverResult(observerSyncs[opCnt], CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    observerLocals[opCnt].Clear();
    EXPECT_EQ((nbDelegateVec[opCnt]->Delete(KEY_1)), OK);
    entries[DELETE_LOCAL].clear();
    EXPECT_TRUE(VerifyObserverResult(observerLocals[opCnt], CHANGED_ZERO_TIME, DELETE_LIST, entries[DELETE_LOCAL]));
    entries[DELETE_NATIVE].clear();
    entries[DELETE_NATIVE].push_back(entry);
    EXPECT_TRUE(VerifyObserverResult(observerSyncs[opCnt], CHANGED_ONE_TIME, DELETE_LIST, entries[DELETE_NATIVE]));
    entries[DELETE_NATIVE].clear();
    observerSyncs[opCnt].Clear();
}

/*
 * @tc.name: Pressure 019
 * @tc.desc: Verify that close db and observer won't be triggered again,
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure019, TestSize.Level2)
{
    KvStoreDelegateManager *nbObserverManager = nullptr;
    vector<string> storeIds = { STORE_ID_2, STORE_ID_3 };
    vector<KvStoreNbDelegate *> nbDelegateVec;
    DBStatus status = DistributedDBNbTestTools::GetNbDelegateStoresSuccess(nbObserverManager,
        nbDelegateVec, storeIds, APP_ID_2, USER_ID_2, g_option);
    EXPECT_EQ(status, OK);
    MST_LOG("nbDelegateVec.size() = %zu", nbDelegateVec.size());

    /**
     * @tc.steps: step1. register different observer on different stores.
     * @tc.expected: step1. each observer is registered successfully.
     */
    KvStoreObserverImpl observerLocals[STORE_NUM];
    KvStoreObserverImpl observerSyncs[STORE_NUM];
    for (unsigned int opCnt = 0; opCnt < static_cast<unsigned int>(nbDelegateVec.size()); ++opCnt) {
        MST_LOG("opCnt = %d", opCnt);
        status = nbDelegateVec[opCnt]->RegisterObserver(KEY_EMPTY, OBSERVER_CHANGES_LOCAL_ONLY, &observerLocals[opCnt]);
        EXPECT_EQ(status, OK);
        status = nbDelegateVec[opCnt]->RegisterObserver(KEY_EMPTY,
            OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSyncs[opCnt]);
        EXPECT_EQ(status, OK);
    }

    /**
     * @tc.steps: step2. crud on each db of corresponding observer on different stores.
     * @tc.expected: step2. each observer is triggered successfully.
     */
    vector< vector<DistributedDB::Entry> > entries(6); // 6 element
    for (unsigned int opCnt = 0; opCnt < static_cast<unsigned int>(nbDelegateVec.size()); ++opCnt) {
        CheckPressureAcrossDatabase(nbDelegateVec, observerLocals, observerSyncs, entries, opCnt);
    }
    for (unsigned int opCnt = 0; opCnt < static_cast<unsigned int>(nbDelegateVec.size()); ++opCnt) {
        status = nbDelegateVec[opCnt]->UnRegisterObserver(&observerLocals[opCnt]);
        EXPECT_EQ(status, OK);
        status = nbDelegateVec[opCnt]->UnRegisterObserver(&observerSyncs[opCnt]);
        EXPECT_EQ(status, OK);
    }

    unsigned int delegateVecSize = static_cast<unsigned int>(nbDelegateVec.size());
    for (unsigned int opCnt = 0; opCnt < delegateVecSize; ++opCnt) {
        EXPECT_EQ(nbObserverManager->CloseKvStore(nbDelegateVec[opCnt]), OK);
        nbDelegateVec[opCnt] = nullptr;
    }
    delete nbObserverManager;
    nbObserverManager = nullptr;
}

void CheckPressureLongCompare(vector<Entry> &entriesBatch, ConcurParam *threadParam, Entry &entryCurrent)
{
    bool isExist = false;
    if (threadParam->tag_ == ReadOrWriteTag::WRITE) {
        for (auto &entry : entriesBatch) {
            if (CompareVector(entry.key, entryCurrent.key)) {
                isExist = true;
            }
        }
        if (!isExist) {
            entriesBatch.push_back(entryCurrent);
        }
    } else if (threadParam->tag_ == ReadOrWriteTag::DELETE) {
        for (unsigned int index = 0; index < static_cast<unsigned int>(entriesBatch.size()); ++index) {
            if (CompareVector(entriesBatch[index].key, entryCurrent.key)) {
                entriesBatch.erase(entriesBatch.begin() + index);
            }
        }
    }
}

void CheckPressureLongConcurrency(vector<Entry> &entriesBatch)
{
    std::random_device randDevAnyKeyNo, randDevTag;
    std::mt19937 genRandAnyKeyNo(randDevAnyKeyNo()), genRandTag(randDevTag());
    std::uniform_int_distribution<unsigned int> disRandAnyKeyNo(ANY_RECORDS_NUM_START, ANY_RECORDS_NUM_END);
    std::uniform_int_distribution<int> disRandTag(static_cast<int>(ReadOrWriteTag::READ),
        static_cast<int>(ReadOrWriteTag::REGISTER));
    unsigned int randKeyNo;
    unsigned int threadCurId = 0;
    int randFlag;

    std::chrono::time_point<chrono::steady_clock, chrono::microseconds> start, end;
    std::chrono::duration<uint64_t, std::micro> dur;
    double operInterval = 0.0;

    start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    while (operInterval < static_cast<double>(LONG_TIME_TEST_SECONDS)) {
        auto threadParam = new (std::nothrow) ConcurParam;
        ASSERT_NE(threadParam, nullptr);
        threadParam->entryPtr_ = new (std::nothrow) DistributedDB::Entry;
        ASSERT_NE(threadParam->entryPtr_, nullptr);
        threadParam->threadId_ = threadCurId++;
        randFlag = disRandTag(genRandTag);
        threadParam->tag_ = static_cast<ReadOrWriteTag>(randFlag);
        randKeyNo = disRandAnyKeyNo(genRandAnyKeyNo);

        Entry entryCurrent;
        GenerateRecord(randKeyNo, entryCurrent);

        CheckPressureLongCompare(entriesBatch, threadParam, entryCurrent);

        threadParam->entryPtr_->key = entryCurrent.key;
        threadParam->entryPtr_->value = entryCurrent.value;
        std::thread thread = std::thread(ConcurOperThread, reinterpret_cast<ConcurParam *>(threadParam));
        thread.join();

        std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(LONG_TIME_INTERVAL_MILLSECONDS));
        end = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
        dur = std::chrono::duration_cast<chrono::microseconds>(end - start);
        operInterval = static_cast<double>(dur.count()) * chrono::microseconds::period::num
            / chrono::microseconds::period::den;
        delete threadParam->entryPtr_;
        threadParam->entryPtr_ = nullptr;
        delete threadParam;
        threadParam = nullptr;
    }
}

/*
 * @tc.name: Pressure 021
 * @tc.desc: Verify that can read and write Concurrency observer, and each observer will success
 * @tc.type: LONGTIME TEST
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbObserverTest, Pressure021, TestSize.Level3)
{
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(BATCH_RECORDS, DEFAULT_START, allKeys, entriesBatch);

    DBStatus status = DistributedDBNbTestTools::PutBatch(*g_nbObserverDelegate, entriesBatch);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<Entry> valueResult;
    status = DistributedDBNbTestTools::GetEntries(*g_nbObserverDelegate, KEY_SEARCH_4, valueResult);
    EXPECT_EQ(status, OK);
    MST_LOG("value size %zu", valueResult.size());

    /**
     * @tc.steps: step1. start 6 threads to register 6 observers to crud on db and
     *    start 4 threads to register observer at the same time.
     * @tc.expected: step1. each thread start successfully and each observer register successfully
     *    and could not effect each other.
     */
    CheckPressureLongConcurrency(entriesBatch);

    while (entriesBatch.size() > 0) {
        status = DistributedDBNbTestTools::Delete(*g_nbObserverDelegate, entriesBatch[0].key);
        EXPECT_EQ(status, OK);
        entriesBatch.erase(entriesBatch.begin());
    }
    MST_LOG("entriesBatch.size() = %zu", entriesBatch.size());
}

/*
 * @tc.name: RekeyNbDb 001
 * @tc.desc: verify that Rekey will return busy when there are registered observer or putting data to db.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbObserverTest, RekeyNbDb001, TestSize.Level2)
{
    KvStoreNbDelegate *nbObserverDelegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option;

    nbObserverDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && nbObserverDelegate != nullptr);

    /**
     * @tc.steps: step1. register observer.
     * @tc.expected: step1. register successfully.
     */
    KvStoreObserverImpl observerLocal;
    KvStoreObserverImpl observerSync;
    DBStatus status = nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_LOCAL_ONLY, &observerLocal);
    EXPECT_EQ(status, OK);
    status = nbObserverDelegate->RegisterObserver(KEY_EMPTY,
        OBSERVER_CHANGES_NATIVE | OBSERVER_CHANGES_FOREIGN, &observerSync);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step2. call Rekey to update passwd to passwd_1.
     * @tc.expected: step2. Rekey returns BUSY.
     */
    EXPECT_TRUE(nbObserverDelegate->Rekey(g_passwd1) == BUSY);

    /**
     * @tc.steps: step3. unregister observer.
     * @tc.expected: step3. unregister successfully.
     */
    status = nbObserverDelegate->UnRegisterObserver(&observerLocal);
    EXPECT_EQ(status, OK);
    status = nbObserverDelegate->UnRegisterObserver(&observerSync);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step4. put (1k,4M) of (k,v) to db.
     * @tc.expected: step4. put successfully.
     */
    std::mutex count;
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, RECORDS_NUM_START, ONE_K_LONG_STRING, FOUR_M_LONG_STRING);
    thread subThread([&]() {
        DBStatus putStatus = nbObserverDelegate->Put(entriesBatch[0].key, entriesBatch[0].value);
        EXPECT_TRUE(putStatus == OK || putStatus == BUSY);
    });
    subThread.detach();

    /**
     * @tc.steps: step5. call Rekey to update passwd to passwd_2 when put data to db.
     * @tc.expected: step5. Rekey returns BUSY.
     */
    status = nbObserverDelegate->Rekey(g_passwd2);
    EXPECT_TRUE(status == OK || status == BUSY);
    std::this_thread::sleep_for(std::chrono::seconds(FIVE_SECONDS));
    EXPECT_TRUE(manager->CloseKvStore(nbObserverDelegate) == OK);
    nbObserverDelegate = nullptr;
    EXPECT_TRUE(manager->DeleteKvStore(STORE_ID_2) == OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: RekeyNbDb 002
 * @tc.desc: verify that Rekey will return busy when there are opened conflict notifier.
 * @tc.type: FUNC
 * @tc.require: SR000CQDT4
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbObserverTest, RekeyNbDb002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. register conflict notifier.
     * @tc.expected: step1. register successfully.
     */
    ConflictNbCallback callback;
    std::vector<ConflictData> conflictData;
    auto notifier = bind(&ConflictNbCallback::NotifyCallBack, &callback, placeholders::_1, &conflictData);
    DBStatus status = g_nbObserverDelegate->SetConflictNotifier(CONFLICT_FOREIGN_KEY_ONLY, notifier);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps: step2. call Rekey to update passwd to passwd_1.
     * @tc.expected: step2. Rekey returns BUSY.
     */
    EXPECT_TRUE(g_nbObserverDelegate->Rekey(g_passwd1) == BUSY);
}
}