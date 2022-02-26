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

#include "app_distributed_kv_data_manager.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "app_kvstore.h"
#include "app_types.h"
#include "app_kvstore_corruption_observer.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AppDistributedKvDataManagerTest"

using namespace testing::ext;
using namespace OHOS::AppDistributedKv;

class AppDistributedKvDataManagerTest : public testing::Test {
public:
    static std::shared_ptr<AppDistributedKvDataManager> manager;
    static Options create;
    static Options noCreate;

    static std::string appId;
    static std::string storeId64;
    static std::string storeId65;
    static std::string storeIdEmpty;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    static void RemoveAllStore(AppDistributedKvDataManager &manager);

    void SetUp();
    void TearDown();
    AppDistributedKvDataManagerTest();
};

std::shared_ptr<AppDistributedKvDataManager> AppDistributedKvDataManagerTest::manager;
Options AppDistributedKvDataManagerTest::create;
Options AppDistributedKvDataManagerTest::noCreate;

std::string AppDistributedKvDataManagerTest::appId;
std::string AppDistributedKvDataManagerTest::storeId64;
std::string AppDistributedKvDataManagerTest::storeId65;
std::string AppDistributedKvDataManagerTest::storeIdEmpty;

void AppDistributedKvDataManagerTest::SetUpTestCase(void)
{
    create.createIfMissing = true;
    create.encrypt = false;
    create.persistant = true;

    noCreate.createIfMissing = false;
    noCreate.encrypt = false;
    noCreate.persistant = true;

    appId = "com.ohos.nb.service";
    std::string dataDir = "data/misc_ce/0/com.ohos.nb.service";
    storeId64 = "a000000000b000000000c000000000d000000000e000000000f000000000g000";
    storeId65 = "a000000000b000000000c000000000d000000000e000000000f000000000g0000"
                "a000000000b000000000c000000000d000000000e000000000f000000000g000";
    storeIdEmpty = "";
    std::string userId = "ohosAnonymousUid";

    manager = AppDistributedKvDataManager::GetInstance(appId, dataDir, userId);
}

void AppDistributedKvDataManagerTest::TearDownTestCase(void)
{
    manager->DeleteKvStore(storeId64);
}

void AppDistributedKvDataManagerTest::SetUp(void)
{
    manager->DeleteKvStore(storeId64);
}

AppDistributedKvDataManagerTest::AppDistributedKvDataManagerTest(void)
{}

void AppDistributedKvDataManagerTest::TearDown(void)
{
    manager->DeleteKvStore(storeId64);
}

/**
  * @tc.name: AppManagerGetKvstore001
  * @tc.desc: Get an exist KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerGetKvstore001, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> notExistKvStorePtr;
    Status status;
    status = manager->GetKvStore(create, storeId64, [&](std::unique_ptr<AppKvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
    });
    EXPECT_NE(notExistKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);

    std::unique_ptr<AppKvStore> existKvStorePtr;
    status = manager->GetKvStore(noCreate, storeId64, [&](std::unique_ptr<AppKvStore> kvStore) {
        existKvStorePtr = std::move(kvStore);
    });
    EXPECT_NE(existKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->CloseKvStore(std::move(notExistKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->CloseKvStore(std::move(existKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppManagerGetKvstore002
  * @tc.desc: Create and get a new KvStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerGetKvstore002, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> notExistKvStorePtr;
    Status status;
    status = manager->GetKvStore(create, storeId64, [&](std::unique_ptr<AppKvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
    });
    EXPECT_NE(notExistKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->CloseKvStore(std::move(notExistKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppManagerGetKvstore003
  * @tc.desc: Get a non-existing KvStore, and the callback function should receive STORE_NOT_FOUND and
  * get a nullptr.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerGetKvstore003, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> notExistKvStorePtr;
    Status status;
    status = manager->GetKvStore(noCreate, storeId64, [&](std::unique_ptr<AppKvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::STORE_NOT_FOUND);
}

/**
  * @tc.name: AppManagerGetKvstore004
  * @tc.desc: Create a KvStore with an empty storeId, and the callback function should receive
  * INVALID_ARGUMENT and got a nullptr.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerGetKvstore004, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> notExistKvStorePtr;
    Status status;
    status = manager->GetKvStore(create, storeIdEmpty, [&](std::unique_ptr<AppKvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);
}

/**
  * @tc.name: AppManagerGetKvstore005
  * @tc.desc: Get a KvStore with an empty storeId, the callback function should receive INVALID_ARGUMENT
  * and got a nullptr.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerGetKvstore005, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> notExistKvStorePtr;
    Status status;
    status = manager->GetKvStore(noCreate, storeIdEmpty, [&](std::unique_ptr<AppKvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);
}

/**
  * @tc.name: AppManagerGetKvstore006
  * @tc.desc: Create a KvStore with 65-byte storeId, and the callback function should receive
  * INVALID_ARGUMENT and got a nullptr.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerGetKvstore006, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> notExistKvStorePtr;
    Status status;
    status = manager->GetKvStore(create, storeId65, [&](std::unique_ptr<AppKvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);
}

/**
  * @tc.name: AppManagerGetKvstore007
  * @tc.desc: Get a KvStore with 65-byte storeId, and the callback function should receive
  * INVALID_ARGUMENT and got a nullptr.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerGetKvstore007, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> notExistKvStorePtr;
    Status status;
    status = manager->GetKvStore(noCreate, storeId65, [&](std::unique_ptr<AppKvStore> kvStore) {
        notExistKvStorePtr = std::move(kvStore);
    });
    EXPECT_EQ(notExistKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);
}

/**
  * @tc.name: AppManagerCloseKvstore001
  * @tc.desc: Close an opened KVStore, and the callback should return SUCCESS.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerCloseKvstore001, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> kvStorePtr;
    Status status;
    status = manager->GetKvStore(create, storeId64, [&](std::unique_ptr<AppKvStore> kvStore) {
        kvStorePtr = std::move(kvStore);
    });
    ASSERT_NE(kvStorePtr, nullptr);
    ASSERT_EQ(status, Status::SUCCESS);

    Status stat = manager->CloseKvStore(std::move(kvStorePtr));
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
  * @tc.name: AppManagerCloseKvstore002
  * @tc.desc: Close a closed KvStore, and the callback should return INVALID_ARGUMENT.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerCloseKvstore002, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> kvStorePtr;
    Status status;
    status = manager->GetKvStore(create, storeId64, [&](std::unique_ptr<AppKvStore> kvStore) {
        kvStorePtr = std::move(kvStore);
    });
    ASSERT_NE(kvStorePtr, nullptr);
    ASSERT_EQ(status, Status::SUCCESS);

    manager->CloseKvStore(std::move(kvStorePtr));
    Status stat = manager->CloseKvStore(std::move(kvStorePtr));
    EXPECT_EQ(stat, Status::INVALID_ARGUMENT);
}

/**
  * @tc.name: AppManagerCloseKvstore003
  * @tc.desc: Close a KvStore with empty storeId, and the callback should return INVALID_ARGUMENT.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerCloseKvstore003, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> kvStorePtr = nullptr;
    Status stat = manager->CloseKvStore(nullptr);
    EXPECT_EQ(stat, Status::INVALID_ARGUMENT);
}

/**
  * @tc.name: AppManagerDeleteKvStore001
  * @tc.desc: Delete a closed KvStore, and the callback should return SUCCESS.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerDeleteKvStore001, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> kvStorePtr;
    Status status;
    status = manager->GetKvStore(create, storeId64, [&](std::unique_ptr<AppKvStore> kvStore) {
        kvStorePtr = std::move(kvStore);
    });
    ASSERT_NE(kvStorePtr, nullptr);
    ASSERT_EQ(status, Status::SUCCESS);

    Status stat = manager->CloseKvStore(std::move(kvStorePtr));
    ASSERT_EQ(stat, Status::SUCCESS);

    stat = manager->DeleteKvStore(storeId64);
    EXPECT_EQ(stat, Status::SUCCESS);
}

/**
  * @tc.name: AppManagerDeleteKvStore002
  * @tc.desc: Delete a opened KvStore, and the callback should return ILLEGAL_STATE.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerDeleteKvStore002, TestSize.Level1)
{
    std::unique_ptr<AppKvStore> kvStorePtr;
    Status status;
    status = manager->GetKvStore(create, storeId64, [&](std::unique_ptr<AppKvStore> kvStore) {
        kvStorePtr = std::move(kvStore);
    });
    ASSERT_NE(kvStorePtr, nullptr);
    ASSERT_EQ(status, Status::SUCCESS);

    // first close it if opened, and then delete it.
    status = manager->DeleteKvStore(storeId64);
    EXPECT_EQ(status, Status::ERROR);
    manager->CloseKvStore(std::move(kvStorePtr));
}

/**
  * @tc.name: AppManagerDeleteKvStore003
  * @tc.desc: Delete a non-existing KvStore, and the callback should return ERROR.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerDeleteKvStore003, TestSize.Level1)
{
    Status stat = manager->DeleteKvStore(storeId64);
    EXPECT_EQ(stat, Status::ERROR);
}

/**
  * @tc.name: AppManagerDeleteKvStore004
  * @tc.desc: Delete a KvStore with an empty storeId, and the callback should return INVALID_ARGUMENT.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerDeleteKvStore004, TestSize.Level1)
{
    Status stat = manager->DeleteKvStore(storeIdEmpty);
    EXPECT_EQ(stat, Status::INVALID_ARGUMENT);
}

/**
  * @tc.name: AppManagerDeleteKvStore005
  * @tc.desc: Delete a KvStore with a 65-byte storeId (which exceeds storeId length limit), and the calback should
  * return INVALID_ARGUMENT.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOJ
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvDataManagerTest, AppManagerDeleteKvStore005, TestSize.Level1)
{
    Status stat = manager->DeleteKvStore(storeId65);
    EXPECT_EQ(stat, Status::INVALID_ARGUMENT);
}

/**
  * @tc.name: GetKvStoreDiskSize001
  * @tc.desc: Get the kvStore disk size.
  * @tc.type: FUNC
  * @tc.require: AR000CQDTD
  * @tc.author: hongbo
  */
HWTEST_F(AppDistributedKvDataManagerTest, GetKvStoreDiskSize001, TestSize.Level1)
{
    uint64_t size;
    Status stat = manager->GetKvStoreDiskSize(storeId65, size);
    if (stat == Status::SUCCESS) {
        uint64_t expect = 0;
        EXPECT_GE(size, expect);
    }
}

class CorruptionObserverImpl : public AppKvStoreCorruptionObserver {
public:
    ~CorruptionObserverImpl() {}
    void OnCorruption(const std::string &appId, const std::string &userId, const std::string &storeId) override;
};

void CorruptionObserverImpl::OnCorruption(const std::string &appId, const std::string &userId,
                                          const std::string &storeId) {}
/**
  * @tc.name: RegisterKvStoreCorruptionObserver
  * @tc.desc: Register the database corruption observer.
  * @tc.type: FUNC
  * @tc.require: AR000D487D
  * @tc.author: hongbo
  */
HWTEST_F(AppDistributedKvDataManagerTest, RegisterKvStoreCorruptionObserver001, TestSize.Level1)
{
    auto observer = std::make_shared<CorruptionObserverImpl>();
    Status stat = manager->RegisterKvStoreCorruptionObserver(observer);
    EXPECT_EQ(stat, Status::SUCCESS);
}