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
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <unistd.h>
#include <iostream>
#include "app_kvstore.h"
#include "app_types.h"
#include "log_print.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AppDistributedKvDataManagerTest"

using namespace testing::ext;
using namespace OHOS::AppDistributedKv;

class AppDistributedKvStoreTest : public testing::Test {
public:
    static std::shared_ptr<AppDistributedKvDataManager> manager;
    static WriteOptions localWrite, syncWrite;
    static ReadOptions localRead, syncRead;
    static Options options;
    static std::string appId;
    static std::string storeId;
    static std::string dataDir;
    static std::string userId;
    static SubscribeType subscribeType;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    void SetUp();
    void TearDown();
    AppDistributedKvStoreTest();
};

std::shared_ptr<AppDistributedKvDataManager> AppDistributedKvStoreTest::manager;

ReadOptions AppDistributedKvStoreTest::localRead;
ReadOptions AppDistributedKvStoreTest::syncRead;
WriteOptions AppDistributedKvStoreTest::localWrite;
WriteOptions AppDistributedKvStoreTest::syncWrite;
Options AppDistributedKvStoreTest::options;

SubscribeType AppDistributedKvStoreTest::subscribeType = SubscribeType::DEFAULT;
std::string AppDistributedKvStoreTest::dataDir = "data/misc_ce/0/appKvStoreTest";
std::string AppDistributedKvStoreTest::appId = "appKvStoreTest";
std::string AppDistributedKvStoreTest::storeId = "appKvStore0";
std::string AppDistributedKvStoreTest::userId = "domainUser0";

void AppDistributedKvStoreTest::SetUpTestCase(void)
{
    manager = AppDistributedKvDataManager::GetInstance(appId, dataDir, userId);
    localRead.local = true;
    syncRead.local = false;
    localWrite.local = true;
    syncWrite.local = false;
    options.createIfMissing = true;
    options.encrypt = false;
    options.persistant = true;
}

void AppDistributedKvStoreTest::TearDownTestCase(void)
{
}

void AppDistributedKvStoreTest::SetUp(void)
{
}

AppDistributedKvStoreTest::AppDistributedKvStoreTest(void)
{
}

void AppDistributedKvStoreTest::TearDown(void)
{
}

class ObserverImpl : public AppKvStoreObserver {
public:
    std::vector<Entry> insertEntries;
    std::vector<Entry> updateEntries;
    std::vector<Entry> deleteEntries;
    bool isClear;
    ObserverImpl()
    {
        insertEntries = {};
        updateEntries = {};
        deleteEntries = {};
        isClear = false;
        callCount_ = 0;
    }
    ~ObserverImpl() = default;

    int CallCount()
    {
        return callCount_;
    }

    void OnChange(const AppChangeNotification &appChangeNotification) override
    {
        const std::list<Entry> insert = appChangeNotification.GetInsertEntries();
        insertEntries.clear();
        for (const auto &entry : insert) {
            insertEntries.push_back(entry);
        }

        const std::list<Entry> update = appChangeNotification.GetUpdateEntries();
        updateEntries.clear();
        for (const auto &entry : update) {
            updateEntries.push_back(entry);
        }

        const std::list<Entry> del = appChangeNotification.GetDeleteEntries();
        deleteEntries.clear();
        for (const auto &entry : del) {
            deleteEntries.push_back(entry);
        }

        isClear = appChangeNotification.IsClear();
        callCount_ += 1;

        ZLOGI("AppChangeNotification OnChange GetDeviceId");
        appChangeNotification.GetDeviceId();
    }

    void Clear()
    {
        insertEntries.clear();
        updateEntries.clear();
        deleteEntries.clear();
        isClear = false;
        callCount_ = 0;
    }
private:
    int callCount_;
};

/**
  * @tc.name: AppKvstorePut001
  * @tc.desc: put int data to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut001, TestSize.Level1)
{
    // cover app_kvstore_put_001, app_kvstore_put_local_001,
    // app_kvstore_get_001, app_kvstore_get_local_001
    // app_kvstore_independent_001, app_kvstore_independent_002
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("math_score_int"), Value(TransferTypeToByteArray<int>(-383468)));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("math_score_int"), Value(TransferTypeToByteArray<int>(-383469)));
    EXPECT_EQ(status, Status::SUCCESS);

    Value ret;
    status = appKvStorePtr->Get(localRead, Key("math_score_int"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(TransferByteArrayToType<int>(ret.Data()), -383468);
    status = appKvStorePtr->Get(syncRead, Key("math_score_int"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(TransferByteArrayToType<int>(ret.Data()), -383469);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut002
  * @tc.desc: put float data to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut002, TestSize.Level1)
{
    // cover app_kvstore_put_002, app_kvstore_put_local_002
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("math_score_float"), Value(TransferTypeToByteArray<float>(3.14f)));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("math_score_float"), Value(TransferTypeToByteArray<float>(3.1416f)));
    EXPECT_EQ(status, Status::SUCCESS);

    Value ret;
    status = appKvStorePtr->Get(localRead, Key("math_score_float"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    float delta = TransferByteArrayToType<float>(ret.Data()) - 3.14f;
    EXPECT_LE(std::abs(delta), 0.00001);
    status = appKvStorePtr->Get(syncRead, Key("math_score_float"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    delta = TransferByteArrayToType<float>(ret.Data()) - 3.1416f;
    EXPECT_LE(std::abs(delta), 0.00001);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut003
  * @tc.desc: put double data to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut003, TestSize.Level1)
{
    // cover app_kvstore_put_003, app_kvstore_put_local_003
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("math_score_double"), Value(TransferTypeToByteArray<double>(28.785f)));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("math_score_double"), Value(TransferTypeToByteArray<double>(28.790f)));
    EXPECT_EQ(status, Status::SUCCESS);

    Value ret;
    status = appKvStorePtr->Get(localRead, Key("math_score_double"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    double delta = TransferByteArrayToType<double>(ret.Data()) - 28.785f;
    EXPECT_LE(std::abs(delta), 0.00001);
    status = appKvStorePtr->Get(syncRead, Key("math_score_double"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    delta = TransferByteArrayToType<double>(ret.Data()) - 28.790f;
    EXPECT_LE(std::abs(delta), 0.00001);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut004
  * @tc.desc: put size_t data to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut004, TestSize.Level1)
{
    // cover app_kvstore_put_004, app_kvstore_put_local_004
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("math_score_size_t"), Value(TransferTypeToByteArray<size_t>(28)));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("math_score_size_t"), Value(TransferTypeToByteArray<size_t>(29)));
    EXPECT_EQ(status, Status::SUCCESS);
    Value ret;
    status = appKvStorePtr->Get(localRead, Key("math_score_size_t"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(TransferByteArrayToType<size_t>(ret.Data()), 28u);
    status = appKvStorePtr->Get(syncRead, Key("math_score_size_t"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(TransferByteArrayToType<size_t>(ret.Data()), 29u);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut005
  * @tc.desc: put int64_t data to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut005, TestSize.Level1)
{
    // cover app_kvstore_put_005, app_kvstore_put_local_005
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status =
        appKvStorePtr->Put(localWrite, Key("math_score_int64_t"), Value(TransferTypeToByteArray<int64_t>(12345678)));
    EXPECT_EQ(status, Status::SUCCESS);
    status =
        appKvStorePtr->Put(syncWrite, Key("math_score_int64_t"), Value(TransferTypeToByteArray<int64_t>(123456789)));
    EXPECT_EQ(status, Status::SUCCESS);
    Value ret;
    status = appKvStorePtr->Get(localRead, Key("math_score_int64_t"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(TransferByteArrayToType<int64_t>(ret.Data()), 12345678u);
    status = appKvStorePtr->Get(syncRead, Key("math_score_int64_t"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(TransferByteArrayToType<int64_t>(ret.Data()), 123456789u);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut006
  * @tc.desc: put string data to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut006, TestSize.Level1)
{
    // cover app_kvstore_put_006, app_kvstore_put_local_006
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("student_name_zhangsan"),
                                Value("{\"class\":20, \"age\":18, \"gradle\":\"good\"}"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_zhangsan"),
                                Value("{\"class\":20, \"age\":19, \"gradle\":\"good\"}"));
    EXPECT_EQ(status, Status::SUCCESS);
    Value ret;
    status = appKvStorePtr->Get(localRead, Key("student_name_zhangsan"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(ret.ToString(), std::string("{\"class\":20, \"age\":18, \"gradle\":\"good\"}"));
    status = appKvStorePtr->Get(syncRead, Key("student_name_zhangsan"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(ret.ToString(), std::string("{\"class\":20, \"age\":19, \"gradle\":\"good\"}"));

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut007
  * @tc.desc: put byte array data to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut007, TestSize.Level1)
{
    // cover app_kvstore_put_007, app_kvstore_put_local_007
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);
    std::vector<uint8_t> val = {0, 1, 2, 0, 0, 5, 6, 0};
    status = appKvStorePtr->Put(localWrite, Key("teacher_name_wanger"),
                                Value(val));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("teacher_name_wanger"),
                                Value(val));
    EXPECT_EQ(status, Status::SUCCESS);
    Value ret;
    status = appKvStorePtr->Get(localRead, Key("teacher_name_wanger"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    ASSERT_EQ(ret.Size(), val.size());
    for(unsigned long i = 0; i < ret.Size(); i++) {
        EXPECT_EQ(ret.Data()[i], val[i]);
    }
    status = appKvStorePtr->Get(syncRead, Key("teacher_name_wanger"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    ASSERT_EQ(ret.Size(), val.size());
    for(unsigned long i = 0; i < ret.Size(); i++) {
        EXPECT_EQ(ret.Data()[i], val[i]);
    }

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut008
  * @tc.desc: Put data including localWrite and syncWrite to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut008, TestSize.Level1)
{
    // cover app_kvstore_put_008, app_kvstore_put_local_008
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("teacher_name_wanger"), Value("class:20, age:50"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("teacher_name_wanger"), Value("class:20, age:51"));
    EXPECT_EQ(status, Status::SUCCESS);
    Value ret;
    status = appKvStorePtr->Get(localRead, Key("teacher_name_wanger"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(ret.ToString(), std::string("class:20, age:50"));
    status = appKvStorePtr->Get(syncRead, Key("teacher_name_wanger"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(ret.ToString(), std::string("class:20, age:51"));

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut009
  * @tc.desc: Update data including localWrite and syncWrite in KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut009, TestSize.Level1)
{
    // cover app_kvstore_put_009, app_kvstore_put_local_009
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("student_name_lisi"), Value("age:18"));
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("student_name_lisi"), Value("age:20"));
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(syncWrite, Key("student_name_lisi"), Value("age:19"));
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(syncWrite, Key("student_name_lisi"), Value("age:21"));
    EXPECT_EQ(status, Status::SUCCESS);

    Value ret;
    status = appKvStorePtr->Get(localRead, Key("student_name_lisi"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(ret.ToString(), std::string("age:20"));
    status = appKvStorePtr->Get(syncRead, Key("student_name_lisi"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(ret.ToString(), std::string("age:21"));

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut010
  * @tc.desc: put invalid key data to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut010, TestSize.Level1)
{
    // cover app_kvstore_put_010, app_kvstore_put_local_010
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key(""), Value("age:18"));
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = appKvStorePtr->Put(syncWrite, Key(""), Value("age:19"));
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut011
  * @tc.desc: Put empty value data to KvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut011, TestSize.Level1)
{
    // cover app_kvstore_put_011, app_kvstore_put_local_011
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("student_name_lisi"), Value(""));
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(syncWrite, Key("student_name_lisi"), Value(""));
    EXPECT_EQ(status, Status::SUCCESS);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

std::string Generate1025KeyLen()
{
    // Generate key and the length is more than 1024;
    std::string str("prefix");
    for (int i = 0; i < 1024; i++) {
        str += "a";
    }
    return str;
}

/**
  * @tc.name: AppKvstorePut012
  * @tc.desc: Put key (greater than or equal to 256 bytes) data to KvStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut012, TestSize.Level1)
{
    // cover app_kvstore_put_012, app_kvstore_put_local_012
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;
    std::string str16 = "0123456789abcdef";
    std::string str256 = "";
    for (int i = 0; i < 16; i++) {
        str256 = str256 + str16;
    }
    std::string str257 = str256 + "g";
    ASSERT_EQ(256UL, str256.size());
    ASSERT_EQ(257UL, str257.size());

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key(str256), Value("class:2, age:50"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(localWrite, Key(Generate1025KeyLen()), Value("class:2, age:50"));
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = appKvStorePtr->Put(syncWrite, Key(str256), Value("class:2, age:51"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key(Generate1025KeyLen()), Value("class:2, age:51"));
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut013
  * @tc.desc: put invalid key data to KvStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, app_kvstore_put_013, TestSize.Level1)
{
    // cover app_kvstore_put_013, app_kvstore_put_local_013
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("     "), Value("class:2, age:50"));
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = appKvStorePtr->Put(syncWrite, Key("     "), Value("class:2, age:51"));
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstorePut014
  * @tc.desc: Put value (greater than or equal to 1 M) data to KvStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstorePut014, TestSize.Level1)
{
    // cover app_kvstore_put_014, app_kvstore_put_local_014
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    constexpr int VALUE_MAX_SIZE = 4 * 1024 * 1024;
    std::vector<uint8_t> valueData(VALUE_MAX_SIZE);
    for(int i = 0; i < VALUE_MAX_SIZE; i++) {
        valueData[i] = 'a';
    }

    status = appKvStorePtr->Put(localWrite, Key("student_name_lisi"), Value(valueData));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_lisa"), Value(valueData));
    EXPECT_EQ(status, Status::SUCCESS);

    valueData.push_back('a');
    status = appKvStorePtr->Put(localWrite, Key("student_name_lisi"), Value(valueData));
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_lisa"), Value(valueData));
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreDelete001
  * @tc.desc: Delete data from KvStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreDelete001, TestSize.Level1)
{
    // cover app_kvstore_delete_001, app_kvstore_delete_local_001
    // app_kvstore_get_002, app_kvstore_get_local_002
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("math_score_int"), Value(TransferTypeToByteArray<int>(-383468)));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(localWrite, Key("math_score_int"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("math_score_int_another"), Value(TransferTypeToByteArray<int>(-383468)));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(syncWrite, Key("math_score_int_another"));
    EXPECT_EQ(status, Status::SUCCESS);

    Value ret;
    status = appKvStorePtr->Get(localRead, Key("math_score_int"), ret);
    EXPECT_EQ(status, Status::KEY_NOT_FOUND);
    status = appKvStorePtr->Get(localRead, Key("math_score_int_another"), ret);
    EXPECT_EQ(status, Status::KEY_NOT_FOUND);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreDelete002
  * @tc.desc: Delete a key which does not exist.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreDelete002, TestSize.Level1)
{
    // cover app_kvstore_delete_002, app_kvstore_delete_local_002
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("math_score_int"), Value(TransferTypeToByteArray<int>(-383468)));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(localWrite, Key("math_score_int"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(localWrite, Key("math_score_int"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("math_score_int_another"), Value(TransferTypeToByteArray<int>(-383468)));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(syncWrite, Key("math_score_int_another"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(syncWrite, Key("math_score_int_another"));
    EXPECT_EQ(status, Status::SUCCESS);

    Value ret;
    status = appKvStorePtr->Get(localRead, Key("math_score_int"), ret);
    EXPECT_EQ(status, Status::KEY_NOT_FOUND);
    status = appKvStorePtr->Get(localRead, Key("math_score_int_another"), ret);
    EXPECT_EQ(status, Status::KEY_NOT_FOUND);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreDelete003
  * @tc.desc: Delete a key which is invalid.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreDelete003, TestSize.Level1)
{
    // cover app_kvstore_delete_003, app_kvstore_delete_local_003
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Delete(localWrite, Key(""));
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreGet001
  * @tc.desc: Get data from KvStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreGet001, TestSize.Level1)
{
    // cover app_kvstore_get_003, app_kvstore_get_local_003
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    Value ret;
    status = appKvStorePtr->Get(localRead, Key("student_name_lisi"), ret);
    EXPECT_EQ(status, Status::KEY_NOT_FOUND);
    status = appKvStorePtr->Get(syncRead, Key("student_name_lisi"), ret);
    EXPECT_EQ(status, Status::KEY_NOT_FOUND);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreGetEntries001
  * @tc.desc: Get entries with prefix from KvStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreGetEntries001, TestSize.Level1)
{
    // cover app_kvstore_getentries_001, app_kvstore_getentries_local_001
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    std::vector<Entry> ret;
    status = appKvStorePtr->Put(localWrite, Key("student_name_mali"), Value("age:20"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(localWrite, Key("student_name_caixu"), Value("age:19"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(localWrite, Key("student_name_liuyue"), Value("age:23"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_maliang"), Value("age:21"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_zhuangzhou"), Value("age:22"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_liuyuxi"), Value("age:24"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("teacher_name_libai"), Value("age:25"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->GetEntries(Key("student_name_"), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(ret.size(), 3UL);

    std::map<std::string, std::string> result;
    for (Entry entry : ret) {
        result.insert(std::pair<std::string, std::string>(entry.key.ToString(), entry.value.ToString()));
    }
    EXPECT_EQ(result["student_name_maliang"], std::string("age:21"));
    EXPECT_EQ(result["student_name_zhuangzhou"], std::string("age:22"));
    EXPECT_EQ(result["student_name_liuyuxi"], std::string("age:24"));

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreGetEntries002
  * @tc.desc: Get entries with non-existing prefix from KvStore.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreGetEntries002, TestSize.Level1)
{
    // cover app_kvstore_getentries_002, app_kvstore_getentries_local_002
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    std::vector<Entry> ret;
    status = appKvStorePtr->Put(syncWrite, Key("student_name_maliang"), Value("age:21"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_zhuangzhou"), Value("age:22"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_liuyuxi"), Value("age:24"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->GetEntries(Key("teacher_name_"), ret);
    EXPECT_EQ(status, Status::KEY_NOT_FOUND);
    EXPECT_EQ(ret.size(), 0UL);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreGetEntries003
  * @tc.desc: Get entries with prefix (empty string) from KvStore, and all data should be returned.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreGetEntries003, TestSize.Level1)
{
    // cover app_kvstore_getentries_003, app_kvstore_getentries_local_003
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    std::vector<Entry> ret;
    status = appKvStorePtr->Put(localWrite, Key("student_name_mali"), Value("age:20"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_maliang"), Value("age:21"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_zhuangzhou"), Value("age:22"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_liuyuxi"), Value("age:24"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->GetEntries(Key(""), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(ret.size(), 3UL);

    std::map<std::string, std::string> result;
    for (Entry entry : ret) {
        result.insert(std::pair<std::string, std::string>(entry.key.ToString(), entry.value.ToString()));
    }
    EXPECT_EQ(result["student_name_maliang"], std::string("age:21"));
    EXPECT_EQ(result["student_name_zhuangzhou"], std::string("age:22"));
    EXPECT_EQ(result["student_name_liuyuxi"], std::string("age:24"));

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreGetEntries004
  * @tc.desc: Get entries with prefix (empty string) from KvStore, and all data should be returned.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreGetEntries004, TestSize.Level1)
{
    // cover app_kvstore_getentries_004, app_kvstore_getentries_local_004
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    std::vector<Entry> ret;
    status = appKvStorePtr->Put(localWrite, Key("student_name_mali"), Value("age:20"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_maliang"), Value("age:21"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_zhuangzhou"), Value("age:22"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_liuyuxi"), Value("age:24"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->GetEntries(Key("          "), ret);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(ret.size(), 3UL);

    std::map<std::string, std::string> result;
    for (Entry entry : ret) {
        result.insert(std::pair<std::string, std::string>(entry.key.ToString(), entry.value.ToString()));
    }
    EXPECT_EQ(result["student_name_maliang"], std::string("age:21"));
    EXPECT_EQ(result["student_name_zhuangzhou"], std::string("age:22"));
    EXPECT_EQ(result["student_name_liuyuxi"], std::string("age:24"));

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreGetEntries005
  * @tc.desc: GetEntries
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL AR000CQS36
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreGetEntries005, TestSize.Level1)
{
    // cover app_kvstore_getentries_005, app_kvstore_getentries_local_005
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, "student", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    std::string str16 = "0123456789abcdef";
    std::string str240 = "";
    for (int i = 0; i < 15; i++) {
        str240 = str240 + str16;
    }
    std::vector<Entry> ret;
    status = appKvStorePtr->Put(localWrite, Key("student_name_mali"), Value("age:20"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_maliang"), Value("age:21"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_zhuangzhou"), Value("age:22"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("student_name_liuyuxi"), Value("age:24"));
    EXPECT_EQ(status, Status::SUCCESS);
    ASSERT_EQ((std::string("student_name_lisi") + str240).size(), 257UL);  // key have max size 256
    status = appKvStorePtr->GetEntries(Key(std::string("student_name_lisi") + Generate1025KeyLen()), ret);
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("student");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreSubscribeKvStore001
  * @tc.desc: SubscribeKvStore and revieve callback
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreSubscribeKvStore001, TestSize.Level1)
{
    // cover app_kvstore_subscribekvstore_001, app_kvstore_subscribekvstore_003,
    // app_kvstore_subscribekvstore_local_001, app_kvstore_subscribekvstore_local_003

    // this testcase fails with a small probable. please leave this log print alone.
    ZLOGI("testcase app_kvstore_subscribekvstore_001");
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, storeId, [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_NE(appKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);
    ObserverImpl *observerLocal = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::SUCCESS);
    ObserverImpl *observerSync = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::SUCCESS);

    observerLocal->Clear();
    observerSync->Clear();
    status = appKvStorePtr->Put(localWrite, Key("key_s1"), Value("value"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(observerLocal->insertEntries.size(), 1UL);
    EXPECT_EQ(observerLocal->deleteEntries.size(), 0UL);
    EXPECT_EQ(observerSync->insertEntries.size(), 0UL) << "contain " << observerSync->insertEntries[0].key.ToString();
    EXPECT_EQ(observerSync->deleteEntries.size(), 0UL) << "contain " << observerSync->deleteEntries[0].key.ToString();
    std::cout << observerLocal->CallCount() << observerSync->CallCount() << std::endl;

    observerLocal->Clear();
    observerSync->Clear();
    status = appKvStorePtr->Put(syncWrite, Key("key_s1"), Value("value"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(syncWrite, Key("key_s1"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(observerLocal->insertEntries.size(), 0UL);
    EXPECT_EQ(observerLocal->deleteEntries.size(), 0UL);
    EXPECT_EQ(observerSync->insertEntries.size(), 0UL) << "contain " << observerSync->insertEntries[0].key.ToString();
    EXPECT_EQ(observerSync->deleteEntries.size(), 1UL);
    std::cout << observerLocal->CallCount() << observerSync->CallCount() << std::endl;

    EXPECT_EQ(observerLocal->isClear, false);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);

    status = manager->DeleteKvStore(storeId);
    EXPECT_EQ(status, Status::SUCCESS);
    delete observerLocal;
    delete observerSync;
    observerLocal = nullptr;
    observerSync = nullptr;
}

/**
  * @tc.name: AppKvstoreSubscribeKvStore002
  * @tc.desc: SubscribeKvStore observer is nullptr
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreSubscribeKvStore002, TestSize.Level1)
{
    // cover app_kvstore_subscribekvstore_002, app_kvstore_subscribekvstore_local_002
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;
    status = manager->GetKvStore(
        options, storeId, [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_NE(appKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, nullptr);
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, nullptr);
    EXPECT_EQ(status, Status::INVALID_ARGUMENT);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);

    status = manager->DeleteKvStore(storeId);
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreSubscribeKvStore003
  * @tc.desc: the same observer SubscribeKvStore many times
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreSubscribeKvStore003, TestSize.Level2)
{
    // cover app_kvstore_subscribekvstore_004, app_kvstore_subscribekvstore_local_004
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, storeId, [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_NE(appKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);

    ObserverImpl *observerLocal = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::STORE_ALREADY_SUBSCRIBE);
    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::STORE_ALREADY_SUBSCRIBE);
    ObserverImpl *observerSync = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::STORE_ALREADY_SUBSCRIBE);
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::STORE_ALREADY_SUBSCRIBE);

    status = appKvStorePtr->Put(localWrite, Key("key"), Value("value"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(observerLocal->CallCount(), 1);
    EXPECT_EQ(observerSync->CallCount(), 0);

    status = appKvStorePtr->Put(syncWrite, Key("key"), Value("value"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(syncWrite, Key("key"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(observerLocal->CallCount(), 1);
    EXPECT_EQ(observerSync->CallCount(), 2);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);

    status = manager->DeleteKvStore(storeId);
    EXPECT_EQ(status, Status::SUCCESS);
    delete observerLocal;
    delete observerSync;
    observerLocal = nullptr;
    observerSync = nullptr;
}

/**
  * @tc.name: AppKvstoreSubscribeKvStore004
  * @tc.desc: the different observer SubscribeKvStore many times
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreSubscribeKvStore004, TestSize.Level2)
{
    // cover app_kvstore_subscribekvstore_004, app_kvstore_subscribekvstore_local_004
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, storeId, [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_NE(appKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);

    ObserverImpl *observerLocal1 = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, observerLocal1);
    EXPECT_EQ(status, Status::SUCCESS);
    ObserverImpl *observerLocal2 = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, observerLocal2);
    EXPECT_EQ(status, Status::SUCCESS);
    ObserverImpl *observerLocal3 = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, observerLocal3);
    EXPECT_EQ(status, Status::SUCCESS);
    ObserverImpl *observerSync1 = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, observerSync1);
    EXPECT_EQ(status, Status::SUCCESS);
    ObserverImpl *observerSync2 = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, observerSync2);
    EXPECT_EQ(status, Status::SUCCESS);
    ObserverImpl *observerSync3 = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, observerSync3);
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("key"), Value("value"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(observerLocal1->CallCount(), 1);
    EXPECT_EQ(observerLocal2->CallCount(), 1);
    EXPECT_EQ(observerLocal3->CallCount(), 1);
    EXPECT_EQ(observerSync1->CallCount(), 0);
    EXPECT_EQ(observerSync2->CallCount(), 0);
    EXPECT_EQ(observerSync3->CallCount(), 0);

    status = appKvStorePtr->Put(syncWrite, Key("key"), Value("value"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(syncWrite, Key("key"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(observerLocal1->CallCount(), 1);
    EXPECT_EQ(observerLocal2->CallCount(), 1);
    EXPECT_EQ(observerLocal3->CallCount(), 1);
    EXPECT_EQ(observerSync1->CallCount(), 2);
    EXPECT_EQ(observerSync2->CallCount(), 2);
    EXPECT_EQ(observerSync3->CallCount(), 2);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);

    status = manager->DeleteKvStore(storeId);
    EXPECT_EQ(status, Status::SUCCESS);
    delete observerLocal1;
    delete observerLocal2;
    delete observerLocal3;
    delete observerSync1;
    delete observerSync2;
    delete observerSync3;
    observerLocal1 = nullptr;
    observerLocal2 = nullptr;
    observerLocal3 = nullptr;
    observerSync1 = nullptr;
    observerSync2 = nullptr;
    observerSync3 = nullptr;
}

/**
  * @tc.name: AppKvstoreSubscribeKvStore005
  * @tc.desc: SubscribeKvStore and then unSubscribeKvStore
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreSubscribeKvStore005, TestSize.Level2)
{
    // cover app_kvstore_subscribekvstore_004, app_kvstore_subscribekvstore_local_004
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, storeId, [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_NE(appKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);

    ObserverImpl *observerLocal = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::SUCCESS);
    ObserverImpl *observerSync = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(localWrite, Key("key"), Value("value"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(observerLocal->CallCount(), 1);
    EXPECT_EQ(observerSync->CallCount(), 0);
    status = appKvStorePtr->Put(syncWrite, Key("key"), Value("value"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(observerLocal->CallCount(), 1);
    EXPECT_EQ(observerSync->CallCount(), 1);

    status = appKvStorePtr->UnSubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->UnSubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::SUCCESS);
    observerLocal->Clear();
    observerSync->Clear();


    status = appKvStorePtr->Put(syncWrite, Key("key"), Value("value"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(syncWrite, Key("key"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(observerLocal->CallCount(), 0);
    EXPECT_EQ(observerSync->CallCount(), 0);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);

    status = manager->DeleteKvStore(storeId);
    EXPECT_EQ(status, Status::SUCCESS);
    delete observerLocal;
    delete observerSync;
    observerLocal = nullptr;
    observerSync = nullptr;
}

/**
  * @tc.name: AppKvstoreSubscribeKvStore006
  * @tc.desc: SubscribeKvStore and then unSubscribeKvStore, verifying callback condition
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreSubscribeKvStore006, TestSize.Level2)
{
    // cover app_kvstore_subscribekvstore_007~011, app_kvstore_subscribekvstore_local_007~011
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, storeId, [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_NE(appKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->Put(syncWrite, Key("key"), Value("syncValue"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(localWrite, Key("key"), Value("localValue"));
    EXPECT_EQ(status, Status::SUCCESS);

    ObserverImpl *localObserver = new ObserverImpl();
    ObserverImpl *syncObserver = new ObserverImpl();
    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, localObserver);
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(localObserver->CallCount(), 0);
    EXPECT_EQ(syncObserver->CallCount(), 0);
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, syncObserver);
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(localObserver->CallCount(), 0);
    EXPECT_EQ(syncObserver->CallCount(), 0);

    status = appKvStorePtr->Put(syncWrite, Key("key1"), Value("syncValue1_"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("key1"), Value("syncValue1"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(syncWrite, Key("key2"), Value("syncValue2"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(localObserver->CallCount(), 0);
    EXPECT_EQ(syncObserver->insertEntries.size(), 1UL);
    EXPECT_EQ(syncObserver->insertEntries[0].key.ToString(), std::string("key2"));
    EXPECT_EQ(syncObserver->insertEntries[0].value.ToString(), std::string("syncValue2"));
    EXPECT_EQ(syncObserver->deleteEntries.size(), 0UL);
    EXPECT_EQ(syncObserver->CallCount(), 3);

    status = appKvStorePtr->Delete(syncWrite, Key("key1"));
    sleep(1);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(localObserver->CallCount(), 0);
    EXPECT_EQ(syncObserver->insertEntries.size(), 0UL);
    EXPECT_EQ(syncObserver->deleteEntries.size(), 1UL);
    EXPECT_EQ(syncObserver->deleteEntries[0].key.ToString(), std::string("key1"));
    EXPECT_EQ(syncObserver->deleteEntries[0].value.ToString(), std::string("syncValue1"));
    EXPECT_EQ(syncObserver->CallCount(), 4);

    status = appKvStorePtr->Put(localWrite, Key("key1"), Value("localvalue1_"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(localWrite, Key("key1"), Value("localvalue1"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(localWrite, Key("key2"), Value("localvalue2"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(syncObserver->CallCount(), 4);
    EXPECT_EQ(localObserver->insertEntries.size(), 1UL);
    EXPECT_EQ(localObserver->insertEntries[0].key.ToString(), std::string("key2"));
    EXPECT_EQ(localObserver->insertEntries[0].value.ToString(), std::string("localvalue2"));
    EXPECT_EQ(localObserver->deleteEntries.size(), 0UL);
    EXPECT_EQ(localObserver->CallCount(), 3);

    status = appKvStorePtr->Delete(localWrite, Key("key1"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(syncObserver->CallCount(), 4);
    EXPECT_EQ(localObserver->insertEntries.size(), 0UL);
    EXPECT_EQ(localObserver->deleteEntries.size(), 1UL);
    EXPECT_EQ(localObserver->deleteEntries[0].key.ToString(), std::string("key1"));
    EXPECT_EQ(localObserver->deleteEntries[0].value.ToString(), std::string("localvalue1"));
    EXPECT_EQ(localObserver->CallCount(), 4);

    status = appKvStorePtr->Delete(localWrite, Key("key10"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(syncWrite, Key("key10"));
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(syncObserver->CallCount(), 4);
    EXPECT_EQ(localObserver->CallCount(), 4);

    status = appKvStorePtr->UnSubscribeKvStore(syncRead, subscribeType, syncObserver);
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->UnSubscribeKvStore(localRead, subscribeType, localObserver);
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(syncObserver->CallCount(), 4);
    EXPECT_EQ(localObserver->CallCount(), 4);

    status = appKvStorePtr->Put(syncWrite, Key("key1"), Value("value1"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Put(localWrite, Key("key1"), Value("value1"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(syncWrite, Key("key2"));
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->Delete(localWrite, Key("key2"));
    EXPECT_EQ(status, Status::SUCCESS);
    sleep(1);
    EXPECT_EQ(syncObserver->CallCount(), 4);
    EXPECT_EQ(localObserver->CallCount(), 4);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);

    status = manager->DeleteKvStore(storeId);
    EXPECT_EQ(status, Status::SUCCESS);
    delete localObserver;
    localObserver = nullptr;
    delete syncObserver;
    syncObserver = nullptr;
}

/**
  * @tc.name: AppKvstoreSubscribeKvStore007
  * @tc.desc: unSubscribeKvStore many times
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreSubscribeKvStore007, TestSize.Level1)
{
    // cover app_kvstore_subscribekvstore_014, app_kvstore_subscribekvstore_015,
    // app_kvstore_subscribekvstore_local_014, app_kvstore_subscribekvstore_local_015
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status;

    status = manager->GetKvStore(
        options, storeId, [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_NE(appKvStorePtr, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);
    ObserverImpl *observerLocal = new ObserverImpl();
    status = appKvStorePtr->UnSubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::STORE_NOT_SUBSCRIBE);
    ObserverImpl *observerSync = new ObserverImpl();
    status = appKvStorePtr->UnSubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::STORE_NOT_SUBSCRIBE);

    status = appKvStorePtr->SubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->SubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->UnSubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::SUCCESS);
    status = appKvStorePtr->UnSubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::SUCCESS);

    status = appKvStorePtr->UnSubscribeKvStore(localRead, subscribeType, observerLocal);
    EXPECT_EQ(status, Status::STORE_NOT_SUBSCRIBE);
    status = appKvStorePtr->UnSubscribeKvStore(syncRead, subscribeType, observerSync);
    EXPECT_EQ(status, Status::STORE_NOT_SUBSCRIBE);

    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);

    status = manager->DeleteKvStore(storeId);
    EXPECT_EQ(status, Status::SUCCESS);
    delete observerLocal;
    delete observerSync;
    observerLocal = nullptr;
    observerSync = nullptr;
}

static void InitResultSetData()
{
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status = AppDistributedKvStoreTest::manager->GetKvStore(
        AppDistributedKvStoreTest::options, "school", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    WriteOptions localWrite;
    localWrite.local = false;
    const std::vector<std::string> students = {"stu_Id1", "stu_Id2", "stu_Id3", "stu_Id4", "stu_Id5"};
    for (const auto &student : students) {
        status = appKvStorePtr->Put(localWrite, Key(student), Value("result_set"));
        EXPECT_EQ(status, Status::SUCCESS);
    }
    const std::vector<std::string> teachers = {"tch_Id1", "tch_Id2", "tch_Id3", "tch_Id4", "tch_Id5", "tch_Id6"};
    for (const auto &teacher : teachers) {
        status = appKvStorePtr->Put(localWrite, Key(teacher), Value("result_set"));
        EXPECT_EQ(status, Status::SUCCESS);
    }
    AppDistributedKvStoreTest::manager->CloseKvStore(std::move(appKvStorePtr));
}

/**
  * @tc.name: AppKvstoreResultSet001
  * @tc.desc: test ResultSet GetEntries when key not exist
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: liuyuhui
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreResultSet001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. initialize kvstore.
     * @tc.expected: step1. SUCCESS.
     */
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status = manager->GetKvStore(
        options, "school", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    /**
     * @tc.steps: step2. call GetEntries when key not exist.
     * @tc.expected: step2. KEY_NOT_FOUND.
     */
    AppKvStoreResultSet *appKvStoreResultSet = nullptr;
    status = appKvStorePtr->GetEntries(Key("key_no_exist"), appKvStoreResultSet);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_NE(appKvStoreResultSet, nullptr);
    EXPECT_EQ(0, appKvStoreResultSet->GetCount());
    status = appKvStorePtr->CloseResultSet(appKvStoreResultSet);
    EXPECT_EQ(status, Status::SUCCESS);

    /**
     * @tc.steps: step3. close and delete kvstore.
     * @tc.expected: step3. SUCCESS.
     */
    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("school");
    EXPECT_EQ(status, Status::SUCCESS);
}

/**
  * @tc.name: AppKvstoreResultSet002
  * @tc.desc: test ResultSet GetEntries
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: liuyuhui
  */
HWTEST_F(AppDistributedKvStoreTest, AppKvstoreResultSet002, TestSize.Level1)
{
    InitResultSetData();

    /**
     * @tc.steps: step1. initialize kvstore.
     * @tc.expected: step1. SUCCESS.
     */
    std::unique_ptr<AppKvStore> appKvStorePtr;
    Status status = manager->GetKvStore(
        options, "school", [&](std::unique_ptr<AppKvStore> appKvStore) {
            appKvStorePtr = std::move(appKvStore);
        });
    EXPECT_EQ(status, Status::SUCCESS);

    /**
     * @tc.steps: step2. call GetEntries with param appKvStoreResultSet.
     * @tc.expected: step2. SUCCESS.
     */
    AppKvStoreResultSet *appKvStoreResultSet = nullptr;
    status = appKvStorePtr->GetEntries(Key("stu_"), appKvStoreResultSet);
    EXPECT_NE(appKvStoreResultSet, nullptr);
    EXPECT_EQ(status, Status::SUCCESS);
    EXPECT_EQ(5, static_cast<int>(appKvStoreResultSet->GetCount()));

    EXPECT_EQ(-1, static_cast<int>(appKvStoreResultSet->GetPosition()));
    EXPECT_EQ(true, appKvStoreResultSet->IsBeforeFirst());

    EXPECT_EQ(true, appKvStoreResultSet->MoveToNext());
    EXPECT_EQ(0, static_cast<int>(appKvStoreResultSet->GetPosition()));
    EXPECT_EQ(true, appKvStoreResultSet->IsFirst());

    EXPECT_EQ(true, appKvStoreResultSet->Move(1));
    EXPECT_EQ(1, static_cast<int>(appKvStoreResultSet->GetPosition()));
    EXPECT_EQ(false, appKvStoreResultSet->IsFirst());

    EXPECT_EQ(true, appKvStoreResultSet->MoveToPosition(2));
    EXPECT_EQ(2, static_cast<int>(appKvStoreResultSet->GetPosition()));

    EXPECT_EQ(true, appKvStoreResultSet->MoveToPrevious());
    EXPECT_EQ(1, static_cast<int>(appKvStoreResultSet->GetPosition()));

    EXPECT_EQ(false, appKvStoreResultSet->IsAfterLast());
    EXPECT_EQ(true, appKvStoreResultSet->MoveToLast());
    EXPECT_EQ(4, static_cast<int>(appKvStoreResultSet->GetPosition()));
    EXPECT_EQ(true, appKvStoreResultSet->IsLast());
    Entry entry;
    status = appKvStoreResultSet->GetEntry(entry);
    EXPECT_EQ(Status::SUCCESS, status);
    EXPECT_EQ(entry.key.ToString(), std::string("stu_Id5"));
    EXPECT_EQ(entry.value.ToString(), std::string("result_set"));

    EXPECT_EQ(false, appKvStoreResultSet->MoveToNext());
    EXPECT_EQ(true, appKvStoreResultSet->IsAfterLast());

    EXPECT_EQ(false, appKvStoreResultSet->MoveToNext());
    EXPECT_EQ(true, appKvStoreResultSet->IsAfterLast());

    EXPECT_EQ(false, appKvStoreResultSet->MoveToPosition(5)); // MoveToPosition more than data size
    EXPECT_EQ(false, appKvStoreResultSet->Move(10));  // Move more than data size

    status = appKvStorePtr->CloseResultSet(appKvStoreResultSet);
    EXPECT_EQ(status, Status::SUCCESS);

    /**
     * @tc.steps: step3. close and delete kvstore.
     * @tc.expected: step3. SUCCESS.
     */
    status = manager->CloseKvStore(std::move(appKvStorePtr));
    EXPECT_EQ(status, Status::SUCCESS);
    status = manager->DeleteKvStore("school");
    EXPECT_EQ(status, Status::SUCCESS);
}
