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

#define LOG_TAG "DistributedKvDataManagerEncryptTest"

#include "distributed_kv_data_manager.h"
#include "kvstore_death_recipient.h"
#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include "types.h"
#include "log_print.h"
using namespace testing::ext;
using namespace OHOS::DistributedKv;

class DistributedKvDataManagerEncryptTest : public testing::Test {
public:
    static DistributedKvDataManager manager;
    static Options createEnc;

    static UserId userId;

    static AppId appId;
    static StoreId storeId;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    static void RemoveAllStore(DistributedKvDataManager manager);

    void SetUp();
    void TearDown();
    DistributedKvDataManagerEncryptTest();
    virtual ~DistributedKvDataManagerEncryptTest();
};

class MyDeathRecipient : public KvStoreDeathRecipient {
public:
    MyDeathRecipient() {}
    virtual ~MyDeathRecipient() {}
    void OnRemoteDied() override {}
};

DistributedKvDataManager DistributedKvDataManagerEncryptTest::manager;
Options DistributedKvDataManagerEncryptTest::createEnc;

UserId DistributedKvDataManagerEncryptTest::userId;

AppId DistributedKvDataManagerEncryptTest::appId;
StoreId DistributedKvDataManagerEncryptTest::storeId;

void DistributedKvDataManagerEncryptTest::RemoveAllStore(DistributedKvDataManager manager)
{
    manager.CloseAllKvStore(appId);
    manager.DeleteKvStore(appId, storeId);
    manager.DeleteAllKvStore(appId);
}
void DistributedKvDataManagerEncryptTest::SetUpTestCase(void)
{
    createEnc.createIfMissing = true;
    createEnc.encrypt = true;
    createEnc.autoSync = true;

    userId.userId = "account0";
    appId.appId = "com.ohos.nb.service";

    storeId.storeId = "EncryptStoreId";
}

void DistributedKvDataManagerEncryptTest::TearDownTestCase(void)
{
    RemoveAllStore(manager);
}

void DistributedKvDataManagerEncryptTest::SetUp(void)
{}

DistributedKvDataManagerEncryptTest::DistributedKvDataManagerEncryptTest(void)
{}

DistributedKvDataManagerEncryptTest::~DistributedKvDataManagerEncryptTest(void)
{}

void DistributedKvDataManagerEncryptTest::TearDown(void)
{}

/**
* @tc.name: kvstore_ddm_createEncryptedStore_001
* @tc.desc: Create an encrypted KvStore.
* @tc.type: FUNC
* @tc.require: SR000D08K4 AR000D08KQ
* @tc.author: liqiao
*/
HWTEST_F(DistributedKvDataManagerEncryptTest, kvstore_ddm_createEncryptedStore_001, TestSize.Level1)
{
    ZLOGI("kvstore_ddm_createEncryptedStore_001 begin.");
    std::unique_ptr<KvStore> kvStorePtr;
    manager.GetKvStore(createEnc, appId, storeId, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        kvStorePtr = std::move(kvStore);
        ASSERT_EQ(status, Status::SUCCESS);
    });
    ASSERT_NE(kvStorePtr, nullptr);

    Key key = "age";
    Value value = "18";
    Status status = kvStorePtr->Put(key, value);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;
    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr,
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    ASSERT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get value from kvstore.
    Value valueRet;
    Status statusRet = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::SUCCESS, statusRet) << "KvStoreSnapshot get data return wrong status";

    EXPECT_EQ(value, valueRet) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}