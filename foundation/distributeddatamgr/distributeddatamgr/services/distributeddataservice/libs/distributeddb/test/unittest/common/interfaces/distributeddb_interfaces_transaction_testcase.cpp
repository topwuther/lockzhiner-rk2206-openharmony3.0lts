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

#include "distributeddb_interfaces_transaction_testcase.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

void DistributedDBInterfacesTransactionTestCase::StartTransaction001(KvStoreDelegate *&kvDelegatePtr)
{
    /**
     * @tc.steps:step1. call StartTransaction interface the 1st time.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. call StartTransaction interface the 2nd time.
     * @tc.expected: step2. call failed and return ERROR.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == DB_ERROR);
    EXPECT_EQ(kvDelegatePtr->Commit(), OK);
}

void DistributedDBInterfacesTransactionTestCase::StartTransaction002(KvStoreDelegate *&kvDelegatePtr)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. call commit interface.
     * @tc.expected: step2. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == OK);
}

void DistributedDBInterfacesTransactionTestCase::StartTransaction003(KvStoreDelegate *&kvDelegatePtr)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. call rollback interface.
     * @tc.expected: step2. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == OK);
}

static void GetSnapshotUnitTest(KvStoreDelegate *&kvDelegatePtr, KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus snapshotDelegateStatus = INVALID_ARGS;
    auto snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(snapshotDelegateStatus), std::ref(snapshotDelegatePtr));

    kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallback);
    EXPECT_TRUE(snapshotDelegateStatus == OK);
    ASSERT_TRUE(snapshotDelegatePtr != nullptr);
}

void DistributedDBInterfacesTransactionTestCase::StartTransaction004(KvStoreDelegate *&kvDelegatePtr,
    const string &storeId, bool localOnly, KvStoreDelegateManager &mgr, KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus kvDelegateStatus = INVALID_ARGS;
    auto kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(kvDelegateStatus), std::ref(kvDelegatePtr));

    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));

    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. put (k1, v1) to data base.
     * @tc.expected: step2. put succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_1, VALUE_1) == OK);
    /**
     * @tc.steps:step3. close data base.
     * @tc.expected: step3. close succeed.
     */
    EXPECT_EQ(mgr.CloseKvStore(kvDelegatePtr), OK);
    kvDelegatePtr = nullptr;

    /**
     * @tc.steps:step4. use GetKvStore interface to open db.
     * @tc.expected: step4. open succeed.
     */
    KvStoreDelegate::Option option = {true, localOnly};
    mgr.GetKvStore(storeId, option, kvDelegateCallback);
    EXPECT_EQ(kvDelegateStatus, OK);
    ASSERT_TRUE(kvDelegatePtr != nullptr);

    /**
     * @tc.steps:step5. use snapshot interface to check the value of k1.
     * @tc.expected: step5. can't get the record of k1.
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == NOT_FOUND);
    EXPECT_TRUE(value.size() == 0);
}

void DistributedDBInterfacesTransactionTestCase::StartTransaction005(KvStoreDelegate *&kvDelegatePtr,
    const string &storeId, bool localOnly, KvStoreDelegateManager &mgr)
{
    DBStatus kvDelegateStatus = INVALID_ARGS;
    auto kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(kvDelegateStatus), std::ref(kvDelegatePtr));

    /**
     * @tc.steps:step1. call StartTransaction interface the 1st time.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    KvStoreDelegate *temp = kvDelegatePtr;
    temp->Put(KEY_1, VALUE_1);

    KvStoreDelegate::Option option = {true, localOnly};
    mgr.GetKvStore(storeId, option, kvDelegateCallback);
    EXPECT_TRUE(kvDelegateStatus == OK);
    ASSERT_TRUE(kvDelegatePtr != nullptr);
    /**
     * @tc.steps:step2. call StartTransaction interface the 2nd time using another .
     * @tc.expected: step2. call failed.
     */
    EXPECT_NE(kvDelegatePtr->StartTransaction(), OK);

    kvDelegatePtr->Put(KEY_2, VALUE_2);
    /**
     * @tc.steps:step4. call commit interface the 1st time.
     * @tc.expected: step4. call failed.
     */
    EXPECT_EQ(temp->Commit(), OK);
    EXPECT_EQ(mgr.CloseKvStore(temp), OK);
    temp = nullptr;
    /**
     * @tc.steps:step5. call commit interface the 2nd time.
     * @tc.expected: step5. call failed.
     */
    EXPECT_NE(kvDelegatePtr->Commit(), OK);
}

void DistributedDBInterfacesTransactionTestCase::Commit001(KvStoreDelegate *&kvDelegatePtr)
{
    /**
     * @tc.steps:step1. commit Transaction without start it.
     * @tc.expected: step1. commit failed and returned ERROR.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == DB_ERROR);
}

void DistributedDBInterfacesTransactionTestCase::Commit002(KvStoreDelegate *&kvDelegatePtr)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. call commit interface the 1st time.
     * @tc.expected: step2. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == OK);
    /**
     * @tc.steps:step3. call commit interface the 2nd time.
     * @tc.expected: step3. call failed and returned ERROR.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == DB_ERROR);
}

void DistributedDBInterfacesTransactionTestCase::Commit003(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));

    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. put (k1, v1) to db.
     * @tc.expected: step2. put succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_1, VALUE_1) == OK);
    /**
     * @tc.steps:step3. call commit interface.
     * @tc.expected: step3. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == OK);

    /**
     * @tc.steps:step4. use snapshot interface to check if (k1, v1) is put succeed.
     * @tc.expected: step4. can find (k1, v1) from db.
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == OK);
    ASSERT_TRUE(value.size() > 0);
    EXPECT_TRUE(value.front() == VALUE_1.front());
}

void DistributedDBInterfacesTransactionTestCase::Commit004(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));
    /**
     * @tc.steps:step1. put one data.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_1, VALUE_1) == OK);
    /**
     * @tc.steps:step2. call StartTransaction interface.
     * @tc.expected: step2. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step3. update the data to another value.
     * @tc.expected: step3. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_1, VALUE_2) == OK);
    /**
     * @tc.steps:step4. call commit interface.
     * @tc.expected: step4. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == OK);

    /**
     * @tc.steps:step5. use snapshot interface to check the updated data.
     * @tc.expected: step5. the value is updated.
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == OK);
    ASSERT_TRUE(value.size() > 0);
    EXPECT_TRUE(value.front() == VALUE_2.front());
}

void DistributedDBInterfacesTransactionTestCase::Commit005(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));

    EXPECT_TRUE(kvDelegatePtr->Put(KEY_1, VALUE_1) == OK);
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. delete record from db where key = k1.
     * @tc.expected: step2. delete succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Delete(KEY_1) == OK);
    /**
     * @tc.steps:step3. call commit interface.
     * @tc.expected: step3. commit succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == OK);

    /**
     * @tc.steps:step4. use snapshot interface to check if (k1, v1) is delete succeed.
     * @tc.expected: step4. can't find (k1, v1) in the db.
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == NOT_FOUND);
}

void DistributedDBInterfacesTransactionTestCase::Commit006(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus entryVectorStatus = INVALID_ARGS;
    unsigned long matchSize = 0;
    std::vector<Entry> entriesVector;
    auto entryVectorCallback = bind(&DistributedDBToolsUnitTest::EntryVectorCallback, placeholders::_1,
        placeholders::_2, std::ref(entryVectorStatus), std::ref(matchSize), std::ref(entriesVector));

    EXPECT_TRUE(kvDelegatePtr->PutBatch(ENTRY_VECTOR) == OK);
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. clear all the records from db.
     * @tc.expected: step2. clear succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Clear() == OK);
    /**
     * @tc.steps:step3. call commit interface.
     * @tc.expected: step3. commit succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == OK);

    /**
     * @tc.steps:step4. use snapshot interface to check if there are any data in db.
     * @tc.expected: step4. can't find any data in db.
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->GetEntries(NULL_KEY_1, entryVectorCallback);
    EXPECT_TRUE(entryVectorStatus == NOT_FOUND);
}

void DistributedDBInterfacesTransactionTestCase::Commit007(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));

    EXPECT_TRUE(kvDelegatePtr->PutBatch(ENTRY_VECTOR) == OK);
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. delete record from db where key = k1.
     * @tc.expected: step2. delete succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Delete(KEY_1) == OK);
    /**
     * @tc.steps:step3. put (k2, v1) to db.
     * @tc.expected: step3. put succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_2, VALUE_1) == OK);
    /**
     * @tc.steps:step4. call commit interface.
     * @tc.expected: step4. commit succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == OK);

    /**
     * @tc.steps:step5. use snapshot interface to check the data in db.
     * @tc.expected: step5. can't find (k1, v1) but can find (k2, v1) in db.
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == NOT_FOUND);
    snapshotDelegatePtr->Get(KEY_2, valueCallback);
    EXPECT_TRUE(valueStatus == OK);
    ASSERT_TRUE(value.size() > 0);
    EXPECT_TRUE(value.front() == VALUE_1.front());
}

void DistributedDBInterfacesTransactionTestCase::Commit008(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus entryVectorStatus = INVALID_ARGS;
    unsigned long matchSizeCallback = 0;
    std::vector<Entry> entriesVector;
    auto entryVectorCallback = bind(&DistributedDBToolsUnitTest::EntryVectorCallback, placeholders::_1,
        placeholders::_2, std::ref(entryVectorStatus), std::ref(matchSizeCallback), std::ref(entriesVector));

    EXPECT_TRUE(kvDelegatePtr->PutBatch(ENTRY_VECTOR) == OK);
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. clear all the records from db.
     * @tc.expected: step2. clear succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Clear() == OK);
    /**
     * @tc.steps:step3. put (k3, v3) to db.
     * @tc.expected: step3. put succeed.
     */
    Entry entry;
    GenerateEntry(1, 3, entry);
    EXPECT_TRUE(kvDelegatePtr->Put(entry.key, entry.value) == OK);
    /**
     * @tc.steps:step4. call commit interface.
     * @tc.expected: step4. commit succeed.
     */
    EXPECT_TRUE(kvDelegatePtr->Commit() == OK);

    /**
     * @tc.steps:step5. use snapshot interface to check the data in db.
     * @tc.expected: step5. can only find (k3, v3) in db.
     */
    unsigned long matchSize = 1;
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->GetEntries(NULL_KEY_1, entryVectorCallback);
    EXPECT_TRUE(entryVectorStatus == OK);
    ASSERT_TRUE(matchSizeCallback == matchSize);
}

void DistributedDBInterfacesTransactionTestCase::RollBack001(KvStoreDelegate *&kvDelegatePtr)
{
    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->Rollback
     * @tc.expected: step1. Return ERROR.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == DB_ERROR);
}

void DistributedDBInterfacesTransactionTestCase::RollBack002(KvStoreDelegate *&kvDelegatePtr)
{
    /**
     * @tc.steps:step1. start a transaction
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. rollback the transaction
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == OK);
    /**
     * @tc.steps:step3. rollback the transaction the second time
     * @tc.expected: step3. Return ERROR.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == DB_ERROR);
}

void DistributedDBInterfacesTransactionTestCase::RollBack003(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));
    /**
     * @tc.steps:step1. start a transaction
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step2. Put (k1,v1)
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_1, VALUE_1) == OK);
    /**
     * @tc.steps:step3. rollback a transaction
     * @tc.expected: step3. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == OK);

    /**
     * @tc.steps:step4. check if (k1,v1) exists
     * @tc.expected: step4. Return NOT_FOUND.
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == NOT_FOUND);
}

void DistributedDBInterfacesTransactionTestCase::RollBack004(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));
    /**
     * @tc.steps:step1. Put (k1,v1)
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_1, VALUE_1) == OK);
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step3. Update (k1,v1) to (k1,v2) in the transaction
     * @tc.expected: step3. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_1, VALUE_2) == OK);
    /**
     * @tc.steps:step4. rollback the transaction
     * @tc.expected: step4. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == OK);

    /**
     * @tc.steps:step5. check the value of k1 is v1
     * @tc.expected: step5. verification is OK .
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == OK);
    ASSERT_TRUE(value.size() > 0);
    EXPECT_TRUE(value.front() == VALUE_1.front());
}

void DistributedDBInterfacesTransactionTestCase::RollBack005(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));
    /**
     * @tc.steps:step1. Put (k1,v1)
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_1, VALUE_1) == OK);
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step3. Delete (k1,v1) in the transaction
     * @tc.expected: step3. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Delete(KEY_1) == OK);
    /**
     * @tc.steps:step4. rollback the transaction
     * @tc.expected: step4. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == OK);

    /**
     * @tc.steps:step5. check the value of k1 is v1
     * @tc.expected: step5. verification is OK .
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == OK);
    ASSERT_TRUE(value.size() > 0);
    EXPECT_TRUE(value.front() == VALUE_1.front());
}

void DistributedDBInterfacesTransactionTestCase::RollBack006(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus entryVectorStatus = INVALID_ARGS;
    unsigned long matchSizeCallback = 0;
    std::vector<Entry> entriesVector;
    auto entryVectorCallback = bind(&DistributedDBToolsUnitTest::EntryVectorCallback, placeholders::_1,
        placeholders::_2, std::ref(entryVectorStatus), std::ref(matchSizeCallback), std::ref(entriesVector));
    /**
     * @tc.steps:step1. PutBatch records: (k1,v1), (k2,v2)
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->PutBatch(ENTRY_VECTOR) == OK);
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step3. Clear all records in the transaction
     * @tc.expected: step3. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Clear() == OK);
    /**
     * @tc.steps:step4. rollback the transaction
     * @tc.expected: step4. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == OK);

    /**
     * @tc.steps:step5. check if there are 2 records in the db
     * @tc.expected: step5. verification is OK .
     */
    unsigned long matchSize = 2;
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->GetEntries(NULL_KEY_1, entryVectorCallback);
    EXPECT_TRUE(entryVectorStatus == OK);
    ASSERT_TRUE(matchSizeCallback == matchSize);
}

void DistributedDBInterfacesTransactionTestCase::RollBack007(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));

    DBStatus entryVectorStatus = INVALID_ARGS;
    unsigned long matchSizeCallback = 0;
    std::vector<Entry> entriesVector;
    auto entryVectorCallback = bind(&DistributedDBToolsUnitTest::EntryVectorCallback, placeholders::_1,
        placeholders::_2, std::ref(entryVectorStatus), std::ref(matchSizeCallback), std::ref(entriesVector));

    /**
     * @tc.steps:step1. PutBatch records: (k1,v1), (k2,v2)
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->PutBatch(ENTRY_VECTOR) == OK);
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step3. Delete (k1,v1) in the transaction
     * @tc.expected: step3. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Delete(KEY_1) == OK);
    /**
     * @tc.steps:step4. Update (k2,v2) to (k2,v1) in the transaction
     * @tc.expected: step4. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Put(KEY_2, VALUE_1) == OK);
    /**
     * @tc.steps:step5. rollback the transaction
     * @tc.expected: step5. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == OK);

    /**
     * @tc.steps:step6. check if (k1,v1),(k2,v2) exist and no more records in the db
     * @tc.expected: step6. verification is OK .
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == OK);
    ASSERT_TRUE(value.size() > 0);
    EXPECT_TRUE(value.front() == VALUE_1.front());
    snapshotDelegatePtr->Get(KEY_2, valueCallback);
    EXPECT_TRUE(valueStatus == OK);
    ASSERT_TRUE(value.size() > 0);
    EXPECT_TRUE(value.front() == VALUE_2.front());

    unsigned long matchSize = 2;
    snapshotDelegatePtr->GetEntries(NULL_KEY_1, entryVectorCallback);
    EXPECT_TRUE(entryVectorStatus == OK);
    ASSERT_TRUE(matchSizeCallback == matchSize);
}

void DistributedDBInterfacesTransactionTestCase::RollBack008(KvStoreDelegate *&kvDelegatePtr,
    KvStoreSnapshotDelegate *&snapshotDelegatePtr)
{
    DBStatus valueStatus = INVALID_ARGS;
    Value value;
    auto valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(valueStatus), std::ref(value));
    DBStatus entryVectorStatus = INVALID_ARGS;
    unsigned long matchSizeCallback = 0;
    std::vector<Entry> entriesVector;
    auto entryVectorCallback = bind(&DistributedDBToolsUnitTest::EntryVectorCallback, placeholders::_1,
        placeholders::_2, std::ref(entryVectorStatus), std::ref(matchSizeCallback), std::ref(entriesVector));

    /**
     * @tc.steps:step1. PutBatch records: (k1,v1), (k2,v2)
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->PutBatch(ENTRY_VECTOR) == OK);
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->StartTransaction() == OK);
    /**
     * @tc.steps:step3. Clear all records in the transaction
     * @tc.expected: step3. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Clear() == OK);
    /**
     * @tc.steps:step4. Put (012, ABC) in the transaction
     * @tc.expected: step4. Return OK.
     */
    Entry entry;
    GenerateEntry(1, 3, entry);
    EXPECT_TRUE(kvDelegatePtr->Put(entry.key, entry.value) == OK);
    /**
     * @tc.steps:step5. rollback the transaction
     * @tc.expected: step5. Return OK.
     */
    EXPECT_TRUE(kvDelegatePtr->Rollback() == OK);

    /**
     * @tc.steps:step6. check if (k1,v1),(k2,v2) exist and no more records in the db
     * @tc.expected: step6. verification is OK .
     */
    GetSnapshotUnitTest(kvDelegatePtr, snapshotDelegatePtr);
    snapshotDelegatePtr->Get(KEY_1, valueCallback);
    EXPECT_TRUE(valueStatus == OK);
    ASSERT_TRUE(value.size() > 0);
    EXPECT_TRUE(value.front() == VALUE_1.front());
    snapshotDelegatePtr->Get(KEY_2, valueCallback);
    EXPECT_TRUE(valueStatus == OK);
    ASSERT_TRUE(value.size() > 0);
    EXPECT_TRUE(value.front() == VALUE_2.front());

    unsigned long matchSize = 2;
    snapshotDelegatePtr->GetEntries(NULL_KEY_1, entryVectorCallback);
    EXPECT_TRUE(entryVectorStatus == OK);
    ASSERT_TRUE(matchSizeCallback == matchSize);
}