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

#define LOG_TAG "DistributedKvDataManagerTest"

#include "distributed_kv_data_manager.h"
#include <cstdint>
#include <gtest/gtest.h>
#include <vector>
#include "kvstore_death_recipient.h"
#include "log_print.h"
#include "types.h"
using namespace testing::ext;
using namespace OHOS::DistributedKv;

class DistributedKvDataManagerTest : public testing::Test {
public:
    static DistributedKvDataManager manager;
    static Options create;
    static Options noCreate;

    static UserId userId;

    static AppId appId;
    static StoreId storeId64;
    static StoreId storeId65;
    static StoreId storeIdTest;
    static StoreId storeIdEmpty;

    static Entry entryA;
    static Entry entryB;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    static void RemoveAllStore(DistributedKvDataManager manager);

    void SetUp();
    void TearDown();
    DistributedKvDataManagerTest();
};

class MyDeathRecipient : public KvStoreDeathRecipient {
public:
    MyDeathRecipient() {}
    virtual ~MyDeathRecipient() {}
    virtual void OnRemoteDied() override {}
};

DistributedKvDataManager DistributedKvDataManagerTest::manager;
Options DistributedKvDataManagerTest::create;
Options DistributedKvDataManagerTest::noCreate;

UserId DistributedKvDataManagerTest::userId;

AppId DistributedKvDataManagerTest::appId;
StoreId DistributedKvDataManagerTest::storeId64;
StoreId DistributedKvDataManagerTest::storeId65;
StoreId DistributedKvDataManagerTest::storeIdTest;
StoreId DistributedKvDataManagerTest::storeIdEmpty;

Entry DistributedKvDataManagerTest::entryA;
Entry DistributedKvDataManagerTest::entryB;

void DistributedKvDataManagerTest::RemoveAllStore(DistributedKvDataManager manager)
{
    manager.CloseAllKvStore(appId);
    manager.DeleteAllKvStore(appId);
}
void DistributedKvDataManagerTest::SetUpTestCase(void)
{
    create.createIfMissing = true;
    create.encrypt = false;
    create.autoSync = true;

    noCreate.createIfMissing = false;
    noCreate.encrypt = false;
    noCreate.autoSync = true;
    noCreate.dataOwnership = true;

    userId.userId = "account0";
    appId.appId = "com.ohos.kvdatamanager.test";

    storeId64.storeId = "a000000000b000000000c000000000d000000000e000000000f000000000g000";
    storeId65.storeId = "a000000000b000000000c000000000d000000000e000000000f000000000g000"
                        "a000000000b000000000c000000000d000000000e000000000f000000000g0000";
    storeIdTest.storeId = "test";
    storeIdEmpty.storeId = "";

    entryA.key = "a";
    entryA.value = "valueA";
    entryB.key = "b";
    entryB.value = "valueB";
    RemoveAllStore(manager);
}

void DistributedKvDataManagerTest::TearDownTestCase(void)
{
    RemoveAllStore(manager);
}

void DistributedKvDataManagerTest::SetUp(void)
{}

DistributedKvDataManagerTest::DistributedKvDataManagerTest(void)
{}

void DistributedKvDataManagerTest::TearDown(void)
{
    RemoveAllStore(manager);
}

/**
* @tc.name: GetKvStore001
* @tc.desc: Get an exist KvStore
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, GetKvStore001, TestSize.Level1)
{
    ZLOGI("GetKvStore001 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(notExistKvStorePtr, nullptr);

    std::unique_ptr<KvStore> existKvStorePtr;
    manager.GetKvStore(noCreate, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        existKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(existKvStorePtr, nullptr);
}

/**
* @tc.name: GetKvStore002
* @tc.desc: Create and get a new KvStore
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, GetKvStore002, TestSize.Level1)
{
    ZLOGI("GetKvStore002 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(notExistKvStorePtr, nullptr);
    manager.CloseKvStore(appId, storeId64);
    manager.DeleteKvStore(appId, storeId64);
}

/**
* @tc.name: GetKvStore003
* @tc.desc: Get a non-existing KvStore, and the callback function should receive STORE_NOT_FOUND and
* get a nullptr.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, GetKvStore003, TestSize.Level1)
{
    ZLOGI("GetKvStore003 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(noCreate, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        EXPECT_EQ(status, Status::STORE_NOT_FOUND);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
}

/**
* @tc.name: GetKvStore004
* @tc.desc: Create a KvStore with an empty storeId, and the callback function should receive
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, GetKvStore004, TestSize.Level1)
{
    ZLOGI("GetKvStore004 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(create, appId, storeIdEmpty, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::INVALID_ARGUMENT);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
}

/**
* @tc.name: GetKvStore005
* @tc.desc: Get a KvStore with an empty storeId, and the callback function should receive INVALID_ARGUMENT
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, GetKvStore005, TestSize.Level1)
{
    ZLOGI("GetKvStore005 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(noCreate, appId, storeIdEmpty, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::INVALID_ARGUMENT);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
}

/**
* @tc.name: GetKvStore006
* @tc.desc: Create a KvStore with a 65-byte storeId, and the callback function should receive INVALID_ARGUMENT
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, GetKvStore006, TestSize.Level1)
{
    ZLOGI("GetKvStore006 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(create, appId, storeId65, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::INVALID_ARGUMENT);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
}

/**
* @tc.name: GetKvStore007
* @tc.desc: Get a KvStore with a 65-byte storeId, the callback function should receive INVALID_ARGUMENT
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, GetKvStore007, TestSize.Level1)
{
    ZLOGI("GetKvStore007 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(noCreate, appId, storeId65, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::INVALID_ARGUMENT);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
}

/**
* @tc.name: GetAllKvStore001
* @tc.desc: Get all KvStore IDs when no KvStore exists, and the callback function should receive a 0-length vector.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, GetAllKvStore001, TestSize.Level1)
{
    ZLOGI("GetAllKvStore001 begin.");
    std::vector<StoreId> idList;
    manager.GetAllKvStoreId(appId, [&](Status status, std::vector<StoreId> &idList) {
        EXPECT_EQ(status, Status::SUCCESS);
        EXPECT_EQ(idList.size(), (unsigned long)0);
    });
}

/**
* @tc.name: GetAllKvStore002
* @tc.desc: Get all KvStore IDs when no KvStore exists, and the callback function should receive a 0-length vector.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, GetAllKvStore002, TestSize.Level1)
{
    ZLOGI("GetAllKvStore002 begin.");
    StoreId id1;
    id1.storeId = "id1";
    StoreId id2;
    id2.storeId = "id2";
    StoreId id3;
    id3.storeId = "id3";
    manager.GetKvStore(create, appId, id1, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        ASSERT_NE(kvStore, nullptr);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    manager.GetKvStore(create, appId, id2, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        ASSERT_NE(kvStore, nullptr);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    manager.GetKvStore(create, appId, id3, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        ASSERT_NE(kvStore, nullptr);
        ASSERT_EQ(status, Status::SUCCESS);
    });

    std::vector<StoreId> idList;
    manager.GetAllKvStoreId(appId, [&](Status status, std::vector<StoreId> &idList) {
        EXPECT_EQ(status, Status::SUCCESS);
        bool haveId1 = false;
        bool haveId2 = false;
        bool haveId3 = false;
        for (StoreId id : idList) {
            if (id.storeId == "id1") {
                haveId1 = true;
            } else if (id.storeId == "id2") {
                haveId2 = true;
            } else if (id.storeId == "id3") {
                haveId3 = true;
            } else {
                ZLOGI("got an unknown storeId.");
                EXPECT_TRUE(false);
            }
        }
        EXPECT_TRUE(haveId1);
        EXPECT_TRUE(haveId2);
        EXPECT_TRUE(haveId3);
        EXPECT_EQ(idList.size(), (unsigned long)3);
    });
}

/**
* @tc.name: CloseKvStore001
* @tc.desc: Close an opened KVStore, and the callback function should return SUCCESS.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseKvStore001, TestSize.Level1)
{
    ZLOGI("CloseKvStore001 begin.");
    std::unique_ptr<KvStore> kvStorePtr;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr, nullptr);

    Status stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: CloseKvStore002
* @tc.desc: Close a closed KvStore, and the callback function should return SUCCESS.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseKvStore002, TestSize.Level1)
{
    ZLOGI("CloseKvStore002 begin.");
    std::unique_ptr<KvStore> kvStorePtr;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr, nullptr);

    manager.CloseKvStore(appId, storeId64);
    Status stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::STORE_NOT_OPEN);
}

/**
* @tc.name: CloseKvStore003
* @tc.desc: Close a KvStore with an empty storeId, and the callback function should return INVALID_ARGUMENT.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseKvStore003, TestSize.Level1)
{
    ZLOGI("CloseKvStore003 begin.");
    Status stat = manager.CloseKvStore(appId, storeIdEmpty);
    EXPECT_EQ(stat, Status::INVALID_ARGUMENT);
}

/**
* @tc.name: CloseKvStore004
* @tc.desc: Close a KvStore with a 65-byte storeId, and the callback function should return INVALID_ARGUMENT.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseKvStore004, TestSize.Level1)
{
    ZLOGI("CloseKvStore004 begin.");
    Status stat = manager.CloseKvStore(appId, storeId65);
    EXPECT_EQ(stat, Status::INVALID_ARGUMENT);
}

/**
* @tc.name: CloseKvStore005
* @tc.desc: Close a non-existing KvStore, and the callback function should return STORE_NOT_OPEN.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseKvStore005, TestSize.Level1)
{
    ZLOGI("CloseKvStore005 begin.");
    Status stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::STORE_NOT_OPEN);
}

/**
* @tc.name: CloseKvStoreMulti001
* @tc.desc: Open a KvStore several times and close them one by one, and the callback function should return SUCCESS.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000CSKRU
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseKvStoreMulti001, TestSize.Level1)
{
    ZLOGI("CloseKvStoreMulti001 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(notExistKvStorePtr, nullptr);

    std::unique_ptr<KvStore> existKvStorePtr;
    manager.GetKvStore(noCreate, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        existKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(existKvStorePtr, nullptr);

    Status stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);

    stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: CloseKvStoreMulti002
* @tc.desc: Open a KvStore several times and close them one by one, and the callback function should return SUCCESS.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000CSKRU
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseKvStoreMulti002, TestSize.Level1)
{
    ZLOGI("CloseKvStoreMulti002 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(notExistKvStorePtr, nullptr);

    std::unique_ptr<KvStore> existKvStorePtr1;
    manager.GetKvStore(noCreate, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        existKvStorePtr1 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(existKvStorePtr1, nullptr);

    std::unique_ptr<KvStore> existKvStorePtr2;
    manager.GetKvStore(noCreate, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        existKvStorePtr2 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(existKvStorePtr2, nullptr);

    Status stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);

    stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);

    stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);

    stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_NE(stat, Status::SUCCESS);
}

/**
* @tc.name: CloseKvStoreMulti003
* @tc.desc: Open a KvStore several times and close them one by one, and the callback function should return SUCCESS.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000CSKRU
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseKvStoreMulti003, TestSize.Level1)
{
    ZLOGI("CloseKvStoreMulti003 begin.");
    std::unique_ptr<KvStore> notExistKvStorePtr;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(notExistKvStorePtr, nullptr);

    std::unique_ptr<KvStore> existKvStorePtr1;
    manager.GetKvStore(noCreate, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        existKvStorePtr1 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(existKvStorePtr1, nullptr);

    std::unique_ptr<KvStore> existKvStorePtr2;
    manager.GetKvStore(noCreate, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        existKvStorePtr2 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(existKvStorePtr2, nullptr);

    Status stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);

    stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);

    Key keyInt = "math_score_int";
    Value valueInt = Value(TransferTypeToByteArray<int>(-100));
    Status status = existKvStorePtr2->Put(keyInt, valueInt);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;
    existKvStorePtr2->GetKvStoreSnapshot(nullptr,
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";

    std::unique_ptr<KvStore> existKvStorePtr3;
    manager.GetKvStore(noCreate, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        existKvStorePtr3 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    EXPECT_NE(existKvStorePtr3, nullptr);

    stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);

    Value valueRetInt;
    Status statusTmp = kvStoreSnapshotPtr->Get(keyInt, valueRetInt);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data return wrong status";
    EXPECT_EQ(valueInt, valueRetInt) << "valueInt and valueRetInt are not equal";

    stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: CloseAllKvStore001
* @tc.desc: Close all opened KvStores, and the callback function should return SUCCESS.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseAllKvStore001, TestSize.Level1)
{
    ZLOGI("CloseAllKvStore001 begin.");
    std::unique_ptr<KvStore> kvStorePtr1;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr1 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr1, nullptr);

    std::unique_ptr<KvStore> kvStorePtr2;
    manager.GetKvStore(create, appId, storeIdTest, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr2 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr2, nullptr);

    Status stat = manager.CloseAllKvStore(appId);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: CloseAllKvStore002
* @tc.desc: Close all KvStores which exist but are not opened, and the callback function should return STORE_NOT_OPEN.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, CloseAllKvStore002, TestSize.Level1)
{
    ZLOGI("CloseAllKvStore002 begin.");
    std::unique_ptr<KvStore> kvStorePtr1;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr1 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr1, nullptr);

    std::unique_ptr<KvStore> kvStorePtr2;
    manager.GetKvStore(create, appId, storeIdTest, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr2 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr2, nullptr);

    Status stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);

    stat = manager.CloseAllKvStore(appId);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: DeleteKvStore001
* @tc.desc: Delete a closed KvStore, and the callback function should return SUCCESS.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, DeleteKvStore001, TestSize.Level1)
{
    ZLOGI("DeleteKvStore001 begin.");
    std::unique_ptr<KvStore> kvStorePtr;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr, nullptr);

    Status stat = manager.CloseKvStore(appId, storeId64);
    ASSERT_EQ(stat, Status::SUCCESS);

    stat = manager.DeleteKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: DeleteKvStore002
* @tc.desc: Delete an opened KvStore, and the callback function should return SUCCESS.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, DeleteKvStore002, TestSize.Level1)
{
    ZLOGI("DeleteKvStore002 begin.");
    std::unique_ptr<KvStore> kvStorePtr;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr, nullptr);

    // first close it if opened, and then delete it.
    Status stat = manager.DeleteKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: DeleteKvStore003
* @tc.desc: Delete a non-existing KvStore, and the callback function should return DB_ERROR.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, DeleteKvStore003, TestSize.Level1)
{
    ZLOGI("DeleteKvStore003 begin.");
    Status stat = manager.DeleteKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::DB_ERROR);
}

/**
* @tc.name: DeleteKvStore004
* @tc.desc: Delete a KvStore with an empty storeId, and the callback function should return INVALID_ARGUMENT.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, DeleteKvStore004, TestSize.Level1)
{
    ZLOGI("DeleteKvStore004 begin.");
    Status stat = manager.DeleteKvStore(appId, storeIdEmpty);
    EXPECT_EQ(stat, Status::INVALID_ARGUMENT);
}

/**
* @tc.name: DeleteKvStore005
* @tc.desc: Delete a KvStore with 65 bytes long storeId (which exceed storeId length limit). Should
* return INVALID_ARGUMENT.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, DeleteKvStore005, TestSize.Level1)
{
    ZLOGI("DeleteKvStore005 begin.");
    Status stat = manager.DeleteKvStore(appId, storeId65);
    EXPECT_EQ(stat, Status::INVALID_ARGUMENT);
}

/**
* @tc.name: DeleteAllKvStore001
* @tc.desc: Delete all KvStores after closing all of them, and the callback function should return SUCCESS.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, DeleteAllKvStore001, TestSize.Level1)
{
    ZLOGI("DeleteAllKvStore001 begin.");
    std::unique_ptr<KvStore> kvStorePtr1;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr1 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr1, nullptr);
    std::unique_ptr<KvStore> kvStorePtr2;
    manager.GetKvStore(create, appId, storeIdTest, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr2 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr2, nullptr);
    Status stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);
    stat = manager.CloseKvStore(appId, storeIdTest);
    EXPECT_EQ(stat, Status::SUCCESS);

    stat = manager.DeleteAllKvStore(appId);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: DeleteAllKvStore002
* @tc.desc: Delete all kvstore fail when any kvstore in the appId is not closed
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, DeleteAllKvStore002, TestSize.Level1)
{
    ZLOGI("DeleteAllKvStore002 begin.");
    std::unique_ptr<KvStore> kvStorePtr1;
    manager.GetKvStore(create, appId, storeId64, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr1 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr1, nullptr);
    std::unique_ptr<KvStore> kvStorePtr2;
    manager.GetKvStore(create, appId, storeIdTest, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr2 = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr2, nullptr);
    Status stat = manager.CloseKvStore(appId, storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);

    stat = manager.DeleteAllKvStore(appId);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: DeleteAllKvStore003
* @tc.desc: Delete all KvStores even if no KvStore exists in the appId.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000BVTDM
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, DeleteAllKvStore003, TestSize.Level1)
{
    ZLOGI("DeleteAllKvStore003 begin.");
    Status stat = manager.DeleteAllKvStore(appId);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
* @tc.name: RegisterKvStoreServiceDeathRecipient001
* @tc.desc: Register a callback called when the service dies.
* @tc.type: FUNC
* @tc.require: SR000CQDU0 AR000CQDU1
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, RegisterKvStoreServiceDeathRecipient001, TestSize.Level1)
{
    ZLOGI("RegisterKvStoreServiceDeathRecipient001 begin.");
    std::shared_ptr<KvStoreDeathRecipient> kvStoreDeathRecipientPtr = std::make_shared<MyDeathRecipient>();
    manager.RegisterKvStoreServiceDeathRecipient(kvStoreDeathRecipientPtr);
    kvStoreDeathRecipientPtr->OnRemoteDied();
}

/**
* @tc.name: UnRegisterKvStoreServiceDeathRecipient001
* @tc.desc: Unregister the callback called when the service dies.
* @tc.type: FUNC
* @tc.require: AR000CQDUS AR000CQDU1
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerTest, UnRegisterKvStoreServiceDeathRecipient001, TestSize.Level1)
{
    ZLOGI("UnRegisterKvStoreServiceDeathRecipient001 begin.");
    std::shared_ptr<KvStoreDeathRecipient> kvStoreDeathRecipientPtr = std::make_shared<MyDeathRecipient>();
    manager.UnRegisterKvStoreServiceDeathRecipient(kvStoreDeathRecipientPtr);
}

class DeviceListenerImpl : public DeviceStatusChangeListener {
public:
    void OnDeviceChanged(const DeviceInfo &info, const DeviceChangeType &type) const override
    {
    }
    DeviceFilterStrategy GetFilterStrategy() const override
    {
        return DeviceFilterStrategy::NO_FILTER;
    }
};
/**
* @tc.name: GetDevice001
* @tc.desc: Get device id.
* @tc.type: FUNC
* @tc.require: SR000DOH1R AR000DPSGU
* @tc.author: hongbo
*/
HWTEST_F(DistributedKvDataManagerTest, GetDevice001, TestSize.Level1)
{
    ZLOGI("GetDevice001 begin.");
    DeviceInfo info;
    Status status = manager.GetLocalDevice(info);
    EXPECT_EQ(Status::SUCCESS, status) << "expected getLocalDevice true";
    EXPECT_TRUE(info.deviceId.size() > 0) << "expected deviceId exist";

    std::vector<DeviceInfo> infos;
    status = manager.GetDeviceList(infos, DeviceFilterStrategy::FILTER);
    // EXPECT_EQ(Status::SUCCESS, status) << "expected GetDeviceList true";
    // EXPECT_TRUE(infos.size() == 0) << "expected GetDeviceList exist";

    auto listener = std::make_shared<DeviceListenerImpl>();
    status = manager.StartWatchDeviceChange(listener);
    EXPECT_EQ(Status::SUCCESS, status) << "expected StartWatchDeviceChange true";
    status = manager.StopWatchDeviceChange(listener);
    EXPECT_EQ(Status::SUCCESS, status) << "expected StopWatchDeviceChange true";
}
