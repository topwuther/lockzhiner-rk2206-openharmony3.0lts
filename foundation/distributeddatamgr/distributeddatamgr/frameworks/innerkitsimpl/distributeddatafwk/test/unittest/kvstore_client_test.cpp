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

#define LOG_TAG "KvStoreClientTest"

#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <unistd.h>
#include <vector>
#include "distributed_kv_data_manager.h"
#include "log_print.h"
#include "types.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;

class KvStoreClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::unique_ptr<KvStore> kvStorePtr;                  // declare kvstore instance.
    static std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;  // declare kvstore instance.
    static Status statusGetKvStore;
    static Status statusGetSnapshot;
    static int MAX_VALUE_SIZE;
};

std::unique_ptr<KvStore> KvStoreClientTest::kvStorePtr = nullptr;
std::unique_ptr<KvStoreSnapshot> KvStoreClientTest::kvStoreSnapshotPtr = nullptr;
Status KvStoreClientTest::statusGetKvStore = Status::ERROR;
Status KvStoreClientTest::statusGetSnapshot = Status::ERROR;
int KvStoreClientTest::MAX_VALUE_SIZE = 4 * 1024 * 1024;

void KvStoreClientTest::SetUpTestCase(void)
{
    DistributedKvDataManager manager;
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;
    options.autoSync = true;
    options.kvStoreType = KvStoreType::MULTI_VERSION;

    AppId appId;
    appId.appId = "odmf";  // define app name.
    StoreId storeId;
    storeId.storeId = "student";  // define kvstore(database) name.

    manager.CloseAllKvStore(appId);
    manager.DeleteAllKvStore(appId);

    // [create and] open and initialize kvstore instance.
    manager.GetKvStore(options, appId, storeId, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        statusGetKvStore = status;
        kvStorePtr = std::move(kvStore);
    });

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       statusGetSnapshot = status;
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });
}

void KvStoreClientTest::TearDownTestCase(void)
{}

void KvStoreClientTest::SetUp(void)
{}

void KvStoreClientTest::TearDown(void)
{}

/**
* @tc.name: KvStoreDdmGetKvStoreSnapshot001
* @tc.desc: Get the KvStore snapshot.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmGetKvStoreSnapshot001, TestSize.Level2)
{
    ZLOGI("KvStoreDdmGetKvStoreSnapshot001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";

    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "name";
    Value value = "test";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get values from KvStore before putting them into KvStore.
    Value valueRet;
    Status statusRet1 = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::KEY_NOT_FOUND, statusRet1) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ("", valueRet.ToString()) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));

    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get values from KvStore after putting them into KvStore.
    Status statusRet2 = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::SUCCESS, statusRet2) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(value, valueRet) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut001
* @tc.desc: Put int values to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut001, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";

    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // store int value to kvstore.
    Key keyInt = "math_score_int";
    int scoreInt = -383468;
    Value valueInt = Value(TransferTypeToByteArray<int>(scoreInt));
    Status status = kvStorePtr->Put(keyInt, valueInt);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get int value from kvstore.
    Value valueRetInt;
    Status statusTmp = kvStoreSnapshotPtr->Get(keyInt, valueRetInt);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(valueInt, valueRetInt) << "valueInt and valueRetInt are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut002
* @tc.desc: Put Float values to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut002, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut002 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // store float value to kvstore.
    Key keyFloat = "math_score_float";
    float scoreFloat = 3.14f;
    Value valueFloat = Value(TransferTypeToByteArray<float>(scoreFloat));
    Status status = kvStorePtr->Put(keyFloat, valueFloat);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get float value from kvstore.
    Value valueRetFloat;
    Status statusTmp = kvStoreSnapshotPtr->Get(keyFloat, valueRetFloat);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(valueFloat, valueRetFloat) << "valueFloat and valueRetFloat are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut003
* @tc.desc: Put Double values to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut003, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut003 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";

    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // store double value to kvstore.
    Key keyDouble = "math_score_double";
    double scoreDouble = 28.785f;
    Value valueDouble = Value(TransferTypeToByteArray<double>(scoreDouble));
    Status status = kvStorePtr->Put(keyDouble, valueDouble);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get double value from kvstore.
    Value valueRetDouble;
    Status statusTmp = kvStoreSnapshotPtr->Get(keyDouble, valueRetDouble);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(valueDouble, valueRetDouble) << "valueDouble and valueRetDouble are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut004
* @tc.desc: put unsigned int value to kvstore
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut004, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut004 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";

    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // store unsigned int value to kvstore.
    Key keyUInt = "math_score_size_t";
    size_t scoreUInt = 28;
    Value valueUInt = Value(TransferTypeToByteArray<size_t>(scoreUInt));
    Status status = kvStorePtr->Put(keyUInt, valueUInt);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get unsigned int value from kvstore.
    Value valueRetUInt;
    Status statusTmp = kvStoreSnapshotPtr->Get(keyUInt, valueRetUInt);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(valueUInt, valueRetUInt) << "valueUInt and valueRetUInt are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut005
* @tc.desc: Put Long values to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut005, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut005 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";

    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // store long int value to kvstore.
    Key keyInt64 = "math_score_int64_t";
    std::int64_t scoreInt64 = 12345678;
    Value valueInt64 = Value(TransferTypeToByteArray<std::int64_t>(scoreInt64));
    Status status = kvStorePtr->Put(keyInt64, valueInt64);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get long int value from kvstore.
    Value valueRetint64;
    Status statusTmp = kvStoreSnapshotPtr->Get(keyInt64, valueRetint64);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(valueInt64, valueRetint64) << "valueInt64 and valueRetint64 are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut006
* @tc.desc: Put JSON values to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut006, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut006 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // store json value to kvstore.
    Key key = "student_name_zhangsan";
    Value value = "{\"class\":20, \"age\":18, \"gradle\":\"good\"}";
    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get json value from kvstore.
    Value valueRet;
    Status statusTmp = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(value, valueRet) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut007
* @tc.desc: Put strings containing '\0' to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut007, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut007 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // store normal string to kvstore.
    Key key = "teacher_name_wanger";
    std::string str = "class:20\0, age:50";
    Value value = Value(str);
    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get string value from kvstore.
    Value valueRet;
    Status statusTmp = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(value, valueRet) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut008
* @tc.desc: Put normal strings to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut008, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut008 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // store normal string to kvstore.
    Key key = "teacher_name_wanger";
    std::string str = "class:20, age:50";
    Value value = Value(str);
    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get string value from kvstore.
    Value valueRet;
    Status statusTmp = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(value, valueRet) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut009
* @tc.desc: Update strings in KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut009, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut009 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "student_name_lisi";
    std::string str1 = "age:18";
    Value value1 = Value(str1);
    Status status1 = kvStorePtr->Put(key, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore put data failed, wrong status";

    std::string str2 = "age:20";
    Value value2 = Value(str2);
    Status status2 = kvStorePtr->Put(key, value2);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status2) << "KvStore put data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get string value from kvstore.
    Value valueRet;
    Status statusTmp = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(value2, valueRet) << "value2 and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPut010
* @tc.desc: Put empty keys to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut010, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut010 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "";
    std::string str = "age:18";
    Value value = Value(str);
    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "KvStore put data failed, wrong status";
}

/**
* @tc.name: KvStoreDdmPut011
* @tc.desc: Put empty keys to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut011, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut011 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "student_name_lisi";
    std::string str = "";
    Value value = Value(str);
    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";
}

/**
* @tc.name: KvStoreDdmPut012
* @tc.desc: Generate keys with a length of more than 1024 bytes.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut012, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut012 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // Generate key and the length is more than 1024;
    std::string strKey("student");
    for (int i = 0; i < 1024; i++) {
        strKey += "a";
    }
    Key key = strKey;

    std::string str = "class:2, age:50";
    Value value = Value(str);
    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "KvStore put data failed, wrong status";
}

/**
* @tc.name: KvStoreDdmPut013
* @tc.desc: Put keys with only blank space to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut013, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut013 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "      ";
    std::string str = "class:2, age:50";
    Value value = Value(str);
    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "KvStore put data failed, wrong status";
}

/**
* @tc.name: KvStoreDdmPut014
* @tc.desc: Put values greater than MAX_VALUE_SIZE to KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liqiao
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPut014, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPut014 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    // store values greater than MAX_VALUE_SIZE to KvStore.
    Key key = "student_name_zhangsan";
    std::vector<uint8_t> val(MAX_VALUE_SIZE);
    for (int i = 0; i < MAX_VALUE_SIZE; i++) {
        val[i] = static_cast<uint8_t>(i);
    }
    Value value = val;
    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data failed, wrong status";

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get values greater than MAX_VALUE_SIZE from KvStore.
    Value valueRet;
    Status statusTmp = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStoreSnapshot get large data failed, wrong status";
    EXPECT_EQ(value, valueRet) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmDelete001
* @tc.desc: Delete data which key exists
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmDelete001, TestSize.Level2)
{
    ZLOGI("KvStoreDdmDelete001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "student_name_lisi";
    std::string str = "age:18";
    Value value = Value(str);
    Status status1 = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore put data failed, wrong status";

    Status status2 = kvStorePtr->Delete(key);  // delete data
    EXPECT_EQ(Status::SUCCESS, status2) << "KvStore delete data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get value from kvstore.
    Value valueRet;
    Status statusRet = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::KEY_NOT_FOUND, statusRet) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ("", valueRet.ToString()) << "valueRet EQ fail";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmDelete002
* @tc.desc: Delete data that contains non-existing keys.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmDelete002, TestSize.Level2)
{
    ZLOGI("KvStoreDdmDelete002 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "student_name_lisi";
    Status status1 = kvStorePtr->Delete(key);  // delete data
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore delete data failed, wrong status";
    Status status2 = kvStorePtr->Delete(key);  // delete data which not exist
    EXPECT_EQ(Status::SUCCESS, status2) << "KvStore delete data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get value from kvstore.
    Value valueRet;
    Status statusRet = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::KEY_NOT_FOUND, statusRet) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ("", valueRet.ToString()) << "valueRet EQ fail";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmDelete003
* @tc.desc: Delete data that contains invalid keys.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmDelete003, TestSize.Level2)
{
    ZLOGI("KvStoreDdmDelete003 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "";
    Status status = kvStorePtr->Delete(key);  // delete data which key is invalid
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "KvStore delete data failed, wrong status";
}

/**
* @tc.name: KvStoreDdmClear001
* @tc.desc: Clear data in KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmClear001, TestSize.Level2)
{
    ZLOGI("KvStoreDdmClear001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key1 = "age";
    Value value1 = "18";
    Status status1 = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore put data failed, wrong status";

    Key key2 = "name";
    Value value2 = "test";
    Status status2 = kvStorePtr->Put(key2, value2);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status2) << "KvStore put data failed, wrong status";

    Status status3 = kvStorePtr->Clear();  // clear data
    EXPECT_EQ(Status::SUCCESS, status3) << "KvStore clear data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    Value valueRet1;
    Status statusRet1 = kvStoreSnapshotPtr->Get(key1, valueRet1);
    EXPECT_EQ(Status::KEY_NOT_FOUND, statusRet1) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ("", valueRet1.ToString()) << "valueRet EQ fail";

    Value valueRet2;
    Status statusRet2 = kvStoreSnapshotPtr->Get(key2, valueRet2);
    EXPECT_EQ(Status::KEY_NOT_FOUND, statusRet2) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ("", valueRet2.ToString()) << "valueRet EQ fail";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmClear002
* @tc.desc: Clearn data in an empty KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmClear002, TestSize.Level2)
{
    ZLOGI("KvStoreDdmClear002 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Status status1 = kvStorePtr->Clear();  // clear data
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore clear data failed, wrong status";

    Status status2 = kvStorePtr->Clear();  // clear data which kvstore is empty
    EXPECT_EQ(Status::SUCCESS, status2) << "KvStore clear data failed, wrong status";
}

/**
* @tc.name: KvStoreDdmPutBatch001
* @tc.desc: Put data to KvStore in batch.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPutBatch001, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPutBatch001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "student_name_mali";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "age:19";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get value from kvstore.
    Value valueRet1;
    Status statusRet1 = kvStoreSnapshotPtr->Get(entry1.key, valueRet1);
    EXPECT_EQ(Status::SUCCESS, statusRet1) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(entry1.value, valueRet1) << "value and valueRet are not equal";

    Value valueRet2;
    Status statusRet2 = kvStoreSnapshotPtr->Get(entry2.key, valueRet2);
    EXPECT_EQ(Status::SUCCESS, statusRet2) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(entry2.value, valueRet2) << "value and valueRet are not equal";

    Value valueRet3;
    Status statusRet3 = kvStoreSnapshotPtr->Get(entry3.key, valueRet3);
    EXPECT_EQ(Status::SUCCESS, statusRet3) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(entry3.value, valueRet3) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPutBatch002
* @tc.desc: Update data in KvStore in batch.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPutBatch002, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPutBatch002 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // before update.
    std::vector<Entry> entriesBefore;
    Entry entry1, entry2, entry3;
    entry1.key = "student_name_mali";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "age:19";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entriesBefore.push_back(entry1);
    entriesBefore.push_back(entry2);
    entriesBefore.push_back(entry3);

    // after update.
    std::vector<Entry> entriesAfter;
    Entry entry4, entry5, entry6;
    entry4.key = "student_name_mali";
    entry4.value = "age:20, sex:girl";
    entry5.key = "student_name_caixu";
    entry5.value = "age:19, sex:boy";
    entry6.key = "student_name_liuyue";
    entry6.value = "age:23, sex:girl";
    entriesAfter.push_back(entry4);
    entriesAfter.push_back(entry5);
    entriesAfter.push_back(entry6);

    Status status = kvStorePtr->PutBatch(entriesAfter);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get value from kvstore.
    Value valueRet1;
    Status statusRet1 = kvStoreSnapshotPtr->Get(entry4.key, valueRet1);
    EXPECT_EQ(Status::SUCCESS, statusRet1) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(entry4.value, valueRet1) << "value and valueRet are not equal";

    Value valueRet2;
    Status statusRet2 = kvStoreSnapshotPtr->Get(entry5.key, valueRet2);
    EXPECT_EQ(Status::SUCCESS, statusRet2) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(entry5.value, valueRet2) << "value and valueRet are not equal";

    Value valueRet3;
    Status statusRet3 = kvStoreSnapshotPtr->Get(entry6.key, valueRet3);
    EXPECT_EQ(Status::SUCCESS, statusRet3) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(entry6.value, valueRet3) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmPutBatch003
* @tc.desc: Put data that contains invalid data to KvStore in batch.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPutBatch003, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPutBatch003 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // before update.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "         ";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "         ";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "KvStore putbatch data failed, wrong status";
}

/**
* @tc.name: KvStoreDdmPutBatch004
* @tc.desc: Put data that contains invalid data to KvStore in batch.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPutBatch004, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPutBatch004 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // before update.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "KvStore putbatch data failed, wrong status";
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
* @tc.name: KvStoreDdmPutBatch005
* @tc.desc: Put data that contains invalid data to KvStore in batch.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPutBatch005, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPutBatch005 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";

    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // before update.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = Generate1025KeyLen();
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "age:19";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "KvStore putbatch data failed, wrong status";
}

/**
* @tc.name: KvStoreDdmPutBatch006
* @tc.desc: Put large data to KvStore in batch.
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmPutBatch006, TestSize.Level2)
{
    ZLOGI("KvStoreDdmPutBatch006 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";

    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    std::vector<uint8_t> val(MAX_VALUE_SIZE);
    for (int i = 0; i < MAX_VALUE_SIZE; i++) {
        val[i] = static_cast<uint8_t>(i);
    }
    Value value = val;

    // before update.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "ruby";
    entry1.value = value;
    entry2.key = "weiss";
    entry2.value = value;
    entry3.key = "blake";
    entry3.value = value;
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data failed, wrong status";

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    // get value from kvstore.
    Value valueRet1;
    Status statusRet1 = kvStoreSnapshotPtr->Get(entry1.key, valueRet1);
    EXPECT_EQ(Status::SUCCESS, statusRet1) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(entry1.value, valueRet1) << "value and valueRet are not equal";

    Value valueRet2;
    Status statusRet2 = kvStoreSnapshotPtr->Get(entry2.key, valueRet2);
    EXPECT_EQ(Status::SUCCESS, statusRet2) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(entry2.value, valueRet2) << "value and valueRet are not equal";

    Value valueRet3;
    Status statusRet3 = kvStoreSnapshotPtr->Get(entry3.key, valueRet3);
    EXPECT_EQ(Status::SUCCESS, statusRet3) << "KvStoreSnapshot get data failed, wrong status";
    EXPECT_EQ(entry3.value, valueRet3) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmDeleteBatch001
* @tc.desc: delete batch data normally
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmDeleteBatch001, TestSize.Level2)
{
    ZLOGI("KvStoreDdmDeleteBatch001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";

    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "student_name_mali";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "age:19";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("student_name_mali");
    keys.push_back("student_name_caixu");
    keys.push_back("student_name_liuyue");

    Status status1 = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore putbatch data failed, wrong status";

    Status status2 = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status2) << "KvStore deletebatch data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    Key keyPrefixStudent = "student_name_";
    std::vector<Entry> students;
    Key token;
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            students = std::move(entries);
        });
    EXPECT_EQ(0, static_cast<int>(students.size())) << "KvStore is not empty, deletebatch fail";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmDeleteBatch002
* @tc.desc: delete batch data which some keys are not in kvstore
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmDeleteBatch002, TestSize.Level2)
{
    ZLOGI("KvStoreDdmDeleteBatch002 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";

    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "student_name_mali";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "age:19";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("student_name_mali");
    keys.push_back("student_name_caixu");
    keys.push_back("student_name_liuyue");
    keys.push_back("student_not_exist");

    Status status1 = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore putbatch data failed, wrong status";

    Status status2 = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status2) << "KvStore deletebatch data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    Key keyPrefixStudent = "student_name_";
    std::vector<Entry> students;
    Key token;
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            students = std::move(entries);
        });
    EXPECT_EQ(0, static_cast<int>(students.size())) << "KvStore is not empty, deletebatch fail";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmDeleteBatch003
* @tc.desc: delete batch data which some keys are invalid
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmDeleteBatch003, TestSize.Level2)
{
    ZLOGI("KvStoreDdmDeleteBatch003 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "student_name_mali";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "age:19";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("student_name_mali");
    keys.push_back("student_name_caixu");
    keys.push_back("");

    Status status1 = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore putbatch data failed, wrong status";

    Status status2 = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status2) << "KvStore deletebatch data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    Key keyPrefixStudent = "student_name_";
    std::vector<Entry> students;
    Key token;
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            students = std::move(entries);
        });
    EXPECT_EQ(3, static_cast<int>(students.size())) << "invalid argument, deletebatch fail";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmDeleteBatch004
* @tc.desc: delete batch data which some keys are invalid
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmDeleteBatch004, TestSize.Level2)
{
    ZLOGI("KvStoreDdmDeleteBatch004 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "student_name_mali";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "age:19";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("student_name_mali");
    keys.push_back("student_name_caixu");
    keys.push_back("          ");

    Status status1 = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore putbatch data failed, wrong status";

    Status status2 = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status2) << "KvStore deletebatch data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    Key keyPrefixStudent = "student_name_";
    std::vector<Entry> students;
    Key token;
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            students = std::move(entries);
        });
    EXPECT_EQ(3, static_cast<int>(students.size())) << "invalid argument, deletebatch fail";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmDeleteBatch005
* @tc.desc: delete batch data which some keys are invalid
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmDeleteBatch005, TestSize.Level2)
{
    ZLOGI("KvStoreDdmDeleteBatch005 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    kvStorePtr->Clear();

    // store entries to kvstore.
    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "student_name_mali";
    entry1.value = "age:20";
    entry2.key = "student_name_caixu";
    entry2.value = "age:19";
    entry3.key = "student_name_liuyue";
    entry3.value = "age:23";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("student_name_mali");
    keys.push_back("student_name_caixu");
    Key keyTmp = Generate1025KeyLen();
    keys.push_back(keyTmp);

    Status status1 = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore putbatch data failed, wrong status";

    Status status2 = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status2) << "KvStore deletebatch data failed, wrong status";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;

    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    Key keyPrefixStudent = "student_name_";
    std::vector<Entry> students;
    Key token;
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            students = std::move(entries);
        });
    EXPECT_EQ(3, static_cast<int>(students.size())) << "invalid argument, deletebatch fail";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreDdmGetStoreId001
* @tc.desc: get kvstore id
* @tc.type: FUNC
* @tc.require: AR000C6GBG AR000CQS36
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreClientTest, KvStoreDdmGetStoreId001, TestSize.Level2)
{
    ZLOGI("KvStoreDdmGetStoreId001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore failed, wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    StoreId id = kvStorePtr->GetStoreId();
    EXPECT_EQ("student", id.storeId) << "GetStoreId fail";
}
