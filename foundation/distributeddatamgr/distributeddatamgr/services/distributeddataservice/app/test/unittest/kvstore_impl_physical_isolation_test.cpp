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
#include <cstdint>
#include <thread>
#include <vector>
#include "constant.h"
#include "kvstore_data_service.h"
#include "kvstore_impl.h"
#include "refbase.h"
#include "types.h"
using namespace testing::ext;
using namespace OHOS::DistributedKv;
using namespace OHOS;

sptr<KvStoreDataService> g_kvStoreDataService;
Options g_defaultOptions;
AppId g_appId;
StoreId g_storeId;
AppId g_appId1;
StoreId g_storeId1;
AppId g_appId2;
StoreId g_storeId2;

class KvStoreImplPhysicalIsolationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void KvStoreImplPhysicalIsolationTest::SetUpTestCase(void)
{
    g_defaultOptions.createIfMissing = true;
    g_defaultOptions.encrypt = false;
    g_defaultOptions.autoSync = true;
    g_defaultOptions.kvStoreType = KvStoreType::MULTI_VERSION;

    g_appId.appId = "phy0";
    g_storeId.storeId = "store0";

    g_appId1.appId = "phy1";
    g_storeId1.storeId = "store1";

    g_appId2.appId = "phy2";
    g_storeId2.storeId = "store2";
}
void KvStoreImplPhysicalIsolationTest::TearDownTestCase(void)
{
    g_kvStoreDataService->CloseAllKvStore(g_appId);
    g_kvStoreDataService->CloseAllKvStore(g_appId1);
    g_kvStoreDataService->CloseAllKvStore(g_appId2);

    g_kvStoreDataService->DeleteAllKvStore(g_appId);
    g_kvStoreDataService->DeleteAllKvStore(g_appId1);
    g_kvStoreDataService->DeleteAllKvStore(g_appId2);
}

void KvStoreImplPhysicalIsolationTest::SetUp(void)
{
    g_kvStoreDataService = sptr<KvStoreDataService>(new KvStoreDataService());
    g_kvStoreDataService->CloseAllKvStore(g_appId);
    g_kvStoreDataService->CloseAllKvStore(g_appId1);
    g_kvStoreDataService->CloseAllKvStore(g_appId2);

    g_kvStoreDataService->DeleteAllKvStore(g_appId);
    g_kvStoreDataService->DeleteAllKvStore(g_appId1);
    g_kvStoreDataService->DeleteAllKvStore(g_appId2);
}
void KvStoreImplPhysicalIsolationTest::TearDown(void)
{
    g_kvStoreDataService->CloseAllKvStore(g_appId);
    g_kvStoreDataService->CloseAllKvStore(g_appId1);
    g_kvStoreDataService->CloseAllKvStore(g_appId2);

    g_kvStoreDataService->DeleteAllKvStore(g_appId);
    g_kvStoreDataService->DeleteAllKvStore(g_appId1);
    g_kvStoreDataService->DeleteAllKvStore(g_appId2);
}

/**
* @tc.name: PhysicalIsolation001
* @tc.desc: Verify the physical isolation function
* @tc.type: FUNC
* @tc.require: AR000BVDF8 AR000CQS3A SR000CQS38
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreImplPhysicalIsolationTest, PhysicalIsolation001, TestSize.Level1)
{
    const std::string storePath = Constant::Concatenate({Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        AccountDelegate::MAIN_DEVICE_ACCOUNT_ID, "/",
        Constant::GetDefaultHarmonyAccountName(), "/" }) +
        std::string("phy0/store0");

    sptr<IKvStoreImpl> kvStorePtr;
    Status status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                          [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(kvStorePtr, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";
    status = g_kvStoreDataService->CloseKvStore(g_appId, g_storeId);
}
/**
* @tc.name: PhysicalIsolation002
* @tc.desc: Verify the different store in the same app having different path
* @tc.type: FUNC
* @tc.require: AR000BVDF8 AR000CQS3A SR000CQS38
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreImplPhysicalIsolationTest, PhysicalIsolation002, TestSize.Level1)
{
    const std::string deviceAccountId = AccountDelegate::MAIN_DEVICE_ACCOUNT_ID;
    const std::string storePath = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy0/store0");

    sptr<IKvStoreImpl> kvStorePtr;
    Status status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                          [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(kvStorePtr, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";
    const std::string storePath1 = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy0/store1");
    sptr<IKvStoreImpl> kvStorePtr1;
    status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId1,
                                              [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr1 = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(kvStorePtr1, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr1.GetRefPtr()))->GetStorePath(), storePath1) << "StorePath EQ fail";
    status = g_kvStoreDataService->CloseKvStore(g_appId, g_storeId);
    status = g_kvStoreDataService->CloseKvStore(g_appId, g_storeId1);
}
/**
* @tc.name: PhysicalIsolation003
* @tc.desc: Verify the different app with the same store having different path
* @tc.type: FUNC
* @tc.require: AR000BVDF8 AR000CQS3A SR000CQS38
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreImplPhysicalIsolationTest, PhysicalIsolation003, TestSize.Level1)
{
    const std::string deviceAccountId = AccountDelegate::MAIN_DEVICE_ACCOUNT_ID;
    const std::string storePath = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy0/store0");

    sptr<IKvStoreImpl> kvStorePtr;
    Status status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                          [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(kvStorePtr, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";
    const std::string storePath1 = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy1/store0");
    sptr<IKvStoreImpl> kvStorePtr1;
    status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId1, g_storeId,
                                              [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr1 = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(kvStorePtr1, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr1.GetRefPtr()))->GetStorePath(), storePath1) << "StorePath EQ fail";
    status = g_kvStoreDataService->CloseKvStore(g_appId, g_storeId);
    status = g_kvStoreDataService->CloseKvStore(g_appId1, g_storeId);
}
/**
* @tc.name: PhysicalIsolation004
* @tc.desc: Verify the different app with the different store having different path
* @tc.type: FUNC
* @tc.require: AR000BVDF8 AR000CQS3A SR000CQS38
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreImplPhysicalIsolationTest, PhysicalIsolation004, TestSize.Level1)
{
    const std::string deviceAccountId = AccountDelegate::MAIN_DEVICE_ACCOUNT_ID;
    const std::string storePath = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy0/store0");

    sptr<IKvStoreImpl> kvStorePtr;
    Status status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                          [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(kvStorePtr, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";

    const std::string storePath1 = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy1/store1");
    sptr<IKvStoreImpl> kvStorePtr1;
    status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId1, g_storeId1,
                                              [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr1 = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(kvStorePtr1, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr1.GetRefPtr()))->GetStorePath(), storePath1) << "StorePath EQ fail";
    status = g_kvStoreDataService->CloseKvStore(g_appId, g_storeId);
    status = g_kvStoreDataService->CloseKvStore(g_appId1, g_storeId1);
}
/**
* @tc.name: PhysicalIsolation005
* @tc.desc: Verify the same app with the same store having the same path
* @tc.type: FUNC
* @tc.require: AR000BVDF8 AR000CQS3A SR000CQS38
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreImplPhysicalIsolationTest, PhysicalIsolation005, TestSize.Level1)
{
    const std::string storePath = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        AccountDelegate::MAIN_DEVICE_ACCOUNT_ID, "/",
        Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy0/store0");

    sptr<IKvStoreImpl> kvStorePtr;
    Status status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                          [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(kvStorePtr, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";

    sptr<IKvStoreImpl> kvStorePtr1;
    status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                              [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr1 = std::move(kvStore); });
    EXPECT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    EXPECT_NE(kvStorePtr1, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr1.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";
    status = g_kvStoreDataService->CloseKvStore(g_appId, g_storeId);
}
/**
* @tc.name: PhysicalIsolation006
* @tc.desc: multithread create the same kvstore.
* @tc.type: FUNC
* @tc.require: AR000BVDF8 AR000CQS3A
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreImplPhysicalIsolationTest, PhysicalIsolation006, TestSize.Level1)
{
    const std::string storePath = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        AccountDelegate::MAIN_DEVICE_ACCOUNT_ID, "/",
        Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy0/store0");
    sptr<IKvStoreImpl> kvStorePtr;
    sptr<IKvStoreImpl> kvStorePtr1;
    sptr<IKvStoreImpl> kvStorePtr2;
    Status status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                          [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr = std::move(kvStore); });
    ASSERT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    std::thread thread1([&]() {
        Status status1 = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                               [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr1 = std::move(kvStore); });
        ASSERT_EQ(status1, Status::SUCCESS) << "GetKvStore return wrong status";
    });
    std::thread thread2([&]() {
        Status status2 = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                               [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr2 = std::move(kvStore); });
        ASSERT_EQ(status2, Status::SUCCESS) << "GetKvStore return wrong status";
    });
    thread1.join();
    thread2.join();
    EXPECT_NE(kvStorePtr, nullptr) << "GetKvStore execute fail!!";
    EXPECT_NE(kvStorePtr1, nullptr) << "GetKvStore execute fail!!";
    EXPECT_NE(kvStorePtr2, nullptr) << "GetKvStore execute fail!!";
    EXPECT_EQ(kvStorePtr, kvStorePtr1) << "Two KvStoreImpl EQ fail";
    EXPECT_EQ(kvStorePtr, kvStorePtr2) << "Two KvStoreImpl EQ fail";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr1.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr2.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";
    status = g_kvStoreDataService->CloseKvStore(g_appId, g_storeId);
}
/**
* @tc.name: PhysicalIsolation007
* @tc.desc: multithread create different kvstore.
* @tc.type: FUNC
* @tc.require: AR000BVDF8 AR000CQS3A
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreImplPhysicalIsolationTest, PhysicalIsolation007, TestSize.Level1)
{
    const std::string deviceAccountId = AccountDelegate::MAIN_DEVICE_ACCOUNT_ID;
    const std::string storePath = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy0/store0");
    const std::string storePath1 = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy1/store1");
    const std::string storePath2 = Constant::Concatenate({ Constant::ROOT_PATH_CE, "/", Constant::SERVICE_NAME, "/",
        deviceAccountId, "/", Constant::GetDefaultHarmonyAccountName(), "/" }) + std::string("phy2/store2");

    sptr<IKvStoreImpl> kvStorePtr;
    sptr<IKvStoreImpl> kvStorePtr1;
    sptr<IKvStoreImpl> kvStorePtr2;
    Status status = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId, g_storeId,
                                          [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr = std::move(kvStore); });
    ASSERT_EQ(status, Status::SUCCESS) << "GetKvStore return wrong status";
    std::thread thread1([&]() {
        Status status1 = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId1, g_storeId1,
                                               [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr1 = std::move(kvStore); });
        ASSERT_EQ(status1, Status::SUCCESS) << "GetKvStore return wrong status";
    });
    std::thread thread2([&]() {
        Status status2 = g_kvStoreDataService->GetKvStore(g_defaultOptions, g_appId2, g_storeId2,
                                               [&](sptr<IKvStoreImpl> kvStore) { kvStorePtr2 = std::move(kvStore); });
        ASSERT_EQ(status2, Status::SUCCESS) << "GetKvStore return wrong status";
    });
    thread1.join();
    thread2.join();
    EXPECT_NE(kvStorePtr, nullptr) << "GetKvStore execute fail!!";
    EXPECT_NE(kvStorePtr1, nullptr) << "GetKvStore execute fail!!";
    EXPECT_NE(kvStorePtr2, nullptr) << "GetKvStore execute fail!!";
    EXPECT_NE(kvStorePtr, kvStorePtr1) << "Two KvStoreImpl NE fail";
    EXPECT_NE(kvStorePtr, kvStorePtr2) << "Two KvStoreImpl NE fail";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr.GetRefPtr()))->GetStorePath(), storePath) << "StorePath EQ fail";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr1.GetRefPtr()))->GetStorePath(), storePath1) << "StorePath EQ fail";
    EXPECT_EQ((static_cast<KvStoreImpl *>(kvStorePtr2.GetRefPtr()))->GetStorePath(), storePath2) << "StorePath EQ fail";
    status = g_kvStoreDataService->CloseKvStore(g_appId, g_storeId);
    status = g_kvStoreDataService->CloseKvStore(g_appId1, g_storeId1);
    status = g_kvStoreDataService->CloseKvStore(g_appId2, g_storeId2);
}
