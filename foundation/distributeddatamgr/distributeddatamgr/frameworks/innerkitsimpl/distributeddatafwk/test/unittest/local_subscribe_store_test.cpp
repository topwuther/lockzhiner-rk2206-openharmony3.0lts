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

#define LOG_TAG "LocalSubscribeStoreTest"

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
namespace {
const int USLEEP_TIME = 2000000;
}
class LocalSubscribeStoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static DistributedKvDataManager manager;
    static std::unique_ptr<KvStore> kvStorePtr;  // declare kvstore instance.
    static Status statusGetKvStore;
    static AppId appId;
    static StoreId storeId;
    static int usleepTime;
};
std::unique_ptr<KvStore> LocalSubscribeStoreTest::kvStorePtr = nullptr;
Status LocalSubscribeStoreTest::statusGetKvStore = Status::ERROR;
DistributedKvDataManager LocalSubscribeStoreTest::manager;
AppId LocalSubscribeStoreTest::appId;
StoreId LocalSubscribeStoreTest::storeId;

void LocalSubscribeStoreTest::SetUpTestCase(void)
{
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;  // not supported yet.
    options.autoSync = true;  // not supported yet.
    options.kvStoreType = KvStoreType::MULTI_VERSION;

    appId.appId = "odmf";         // define app name.
    storeId.storeId = "student";  // define kvstore(database) name
    manager.DeleteKvStore(appId, storeId);
    // [create and] open and initialize kvstore instance.
    manager.GetKvStore(options, appId, storeId, [&](Status status, std::unique_ptr<KvStore> kvStore) {
        statusGetKvStore = status;
        kvStorePtr = std::move(kvStore);
    });
}

void LocalSubscribeStoreTest::TearDownTestCase(void)
{}

void LocalSubscribeStoreTest::SetUp(void)
{}

void LocalSubscribeStoreTest::TearDown(void)
{}

class KvStoreObserverUnitTest : public KvStoreObserver {
public:
    std::vector<Entry> insertEntries_;
    std::vector<Entry> updateEntries_;
    std::vector<Entry> deleteEntries_;
    bool isClear_;
    KvStoreObserverUnitTest();
    ~KvStoreObserverUnitTest()
    {}

    KvStoreObserverUnitTest(const KvStoreObserverUnitTest &) = delete;
    KvStoreObserverUnitTest &operator=(const KvStoreObserverUnitTest &) = delete;
    KvStoreObserverUnitTest(KvStoreObserverUnitTest &&) = delete;
    KvStoreObserverUnitTest &operator=(KvStoreObserverUnitTest &&) = delete;

    // callback function will be called when the db data is changed.
    void OnChange(const ChangeNotification &changeNotification, std::unique_ptr<KvStoreSnapshot> snapshot);

    void OnChange(const ChangeNotification &changeNotification);

    // reset the callCount_ to zero.
    void ResetToZero();

    unsigned long GetCallCount() const;

private:
    unsigned long callCount_;
};

KvStoreObserverUnitTest::KvStoreObserverUnitTest()
{
    callCount_ = 0;
    insertEntries_ = {};
    updateEntries_ = {};
    deleteEntries_ = {};
    isClear_ = false;
}

void KvStoreObserverUnitTest::OnChange(const ChangeNotification &changeNotification,
                                       std::unique_ptr<KvStoreSnapshot> snapshot)
{
    ZLOGD("begin.");
    callCount_++;
    const std::list<Entry> insert = changeNotification.GetInsertEntries();
    insertEntries_.clear();
    for (const auto &entry : insert) {
        insertEntries_.push_back(entry);
    }

    const std::list<Entry> update = changeNotification.GetUpdateEntries();
    updateEntries_.clear();
    for (const auto &entry : update) {
        updateEntries_.push_back(entry);
    }

    const std::list<Entry> del = changeNotification.GetDeleteEntries();
    deleteEntries_.clear();
    for (const auto &entry : del) {
        deleteEntries_.push_back(entry);
    }

    changeNotification.GetDeviceId();
    isClear_ = changeNotification.IsClear();
}

void KvStoreObserverUnitTest::OnChange(const ChangeNotification &changeNotification)
{}

void KvStoreObserverUnitTest::ResetToZero()
{
    callCount_ = 0;
}

unsigned long KvStoreObserverUnitTest::GetCallCount() const
{
    return callCount_;
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore001
* @tc.desc: Subscribe success
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore001, TestSize.Level1)
{
    ZLOGI("KvStoreDdmSubscribeKvStore001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 0);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
    observer = nullptr;
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore002
* @tc.desc: Subscribe fail, observer is null
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore002, TestSize.Level1)
{
    ZLOGI("KvStoreDdmSubscribeKvStore002 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    std::shared_ptr<KvStoreObserverUnitTest> observer = nullptr;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::INVALID_ARGUMENT, status) << "SubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore003
* @tc.desc: Subscribe success and OnChange callback after put
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore003, TestSize.Level1)
{
    ZLOGI("KvStoreDdmSubscribeKvStore003 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key = "Id1";
    Value value = "subscribe";
    status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
    observer = nullptr;
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore004
* @tc.desc: The same observer subscribe three times and OnChange callback after put
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore004, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore004 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::STORE_ALREADY_SUBSCRIBE, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::STORE_ALREADY_SUBSCRIBE, status) << "SubscribeKvStore return wrong status";

    Key key = "Id1";
    Value value = "subscribe";
    status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore005
* @tc.desc: The different observer subscribe three times and OnChange callback after put
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore005, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore005 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer1 = std::make_shared<KvStoreObserverUnitTest>();
    observer1->ResetToZero();
    std::shared_ptr<KvStoreObserverUnitTest> observer2 = std::make_shared<KvStoreObserverUnitTest>();
    observer2->ResetToZero();
    std::shared_ptr<KvStoreObserverUnitTest> observer3 = std::make_shared<KvStoreObserverUnitTest>();
    observer3->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer1);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore failed, wrong status";
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer2);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore failed, wrong status";
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer3);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore failed, wrong status";

    Key key = "Id1";
    Value value = "subscribe";
    status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "Putting data to KvStore failed, wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer1->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer2->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer3->GetCallCount()), 1);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer1);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer2);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer3);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore006
* @tc.desc: Unsubscribe an observer and subscribe again - the map should be cleared after unsubscription.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore006, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore006 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "Id1";
    Value value1 = "subscribe";
    status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";

    Key key2 = "Id2";
    Value value2 = "subscribe";
    status = kvStorePtr->Put(key2, value2);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    Key key3 = "Id3";
    Value value3 = "subscribe";
    status = kvStorePtr->Put(key3, value3);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 2);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore007
* @tc.desc: Subscribe to an observer - OnChange callback is called multiple times after the put operation.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore007, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore007 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "Id1";
    Value value1 = "subscribe";
    status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    Key key2 = "Id2";
    Value value2 = "subscribe";
    status = kvStorePtr->Put(key2, value2);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    Key key3 = "Id3";
    Value value3 = "subscribe";
    status = kvStorePtr->Put(key3, value3);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 3);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore008
* @tc.desc: Subscribe to an observer - OnChange callback is called multiple times after the put&update operations.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore008, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore008 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "Id1";
    Value value1 = "subscribe";
    status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    Key key2 = "Id2";
    Value value2 = "subscribe";
    status = kvStorePtr->Put(key2, value2);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    Key key3 = "Id1";
    Value value3 = "subscribe03";
    status = kvStorePtr->Put(key3, value3);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 3);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore009
* @tc.desc: Subscribe to an observer - OnChange callback is called multiple times after the putBatch operation.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore009, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore009 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    // before update.
    std::vector<Entry> entries1;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries1.push_back(entry1);
    entries1.push_back(entry2);
    entries1.push_back(entry3);

    std::vector<Entry> entries2;
    Entry entry4, entry5;
    entry4.key = "Id4";
    entry4.value = "subscribe";
    entry5.key = "Id5";
    entry5.value = "subscribe";
    entries2.push_back(entry4);
    entries2.push_back(entry5);

    status = kvStorePtr->PutBatch(entries1);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    status = kvStorePtr->PutBatch(entries2);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(1000000);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 2);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore010
* @tc.desc: Subscribe to an observer - OnChange callback is called multiple times after the putBatch update operation.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore010, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore010 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    // before update.
    std::vector<Entry> entries1;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries1.push_back(entry1);
    entries1.push_back(entry2);
    entries1.push_back(entry3);

    std::vector<Entry> entries2;
    Entry entry4, entry5;
    entry4.key = "Id1";
    entry4.value = "modify";
    entry5.key = "Id2";
    entry5.value = "modify";
    entries2.push_back(entry4);
    entries2.push_back(entry5);

    status = kvStorePtr->PutBatch(entries1);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    status = kvStorePtr->PutBatch(entries2);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 2);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore011
* @tc.desc: Subscribe to an observer - OnChange callback is called after successful deletion.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore011, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore011 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->Delete("Id1");
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Delete data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore012
* @tc.desc: Subscribe to an observer - OnChange callback is not called after deletion of non-existing keys.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore012, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore012 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->Delete("Id4");
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Delete data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 0);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore013
* @tc.desc: Subscribe to an observer - OnChange callback is called after KvStore is cleared.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore013, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore013 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->Clear();
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Clear data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore014
* @tc.desc: Subscribe to an observer - OnChange callback is not called after non-existing data in KvStore is cleared.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore014, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore014 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->Clear();
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Clear data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore015
* @tc.desc: Subscribe to an observer - OnChange callback is called after the deleteBatch operation.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore015, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore015 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("Id1");
    keys.push_back("Id2");

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    status = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore DeleteBatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore016
* @tc.desc: Subscribe to an observer - OnChange callback is called after deleteBatch of non-existing keys.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore016, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore016 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("Id4");
    keys.push_back("Id5");

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    status = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore DeleteBatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 0);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStore020
* @tc.desc: Unsubscribe an observer two times.
* @tc.type: FUNC
* @tc.require: AR000CQDU9 AR000CQS37
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStore020, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStore020 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::STORE_NOT_SUBSCRIBE, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification001
* @tc.desc: Subscribe to an observer successfully - callback is called with a notification after the put operation.
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification001, TestSize.Level1)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key = "Id1";
    Value value = "subscribe";
    status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    ZLOGD("kvstore_ddm_subscribekvstore_003");
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());
    ZLOGD("kvstore_ddm_subscribekvstore_003 size:%zu.", observer->insertEntries_.size());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification002
* @tc.desc: Subscribe to the same observer three times - callback is called with a notification after the put operation.
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification002, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification002 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::STORE_ALREADY_SUBSCRIBE, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::STORE_ALREADY_SUBSCRIBE, status) << "SubscribeKvStore return wrong status";

    Key key = "Id1";
    Value value = "subscribe";
    status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification003
* @tc.desc: The different observer subscribe three times and callback with notification after put
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification003, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification003 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer1 = std::make_shared<KvStoreObserverUnitTest>();
    observer1->ResetToZero();
    std::shared_ptr<KvStoreObserverUnitTest> observer2 = std::make_shared<KvStoreObserverUnitTest>();
    observer2->ResetToZero();
    std::shared_ptr<KvStoreObserverUnitTest> observer3 = std::make_shared<KvStoreObserverUnitTest>();
    observer3->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer1);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer2);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer3);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key = "Id1";
    Value value = "subscribe";
    status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer1->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer1->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer1->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer1->insertEntries_[0].value.ToString());

    EXPECT_EQ(static_cast<int>(observer2->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer2->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer2->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer2->insertEntries_[0].value.ToString());

    EXPECT_EQ(static_cast<int>(observer3->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer3->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer3->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer3->insertEntries_[0].value.ToString());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer1);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer2);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer3);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification004
* @tc.desc: Verify notification after an observer is unsubscribed and then subscribed again.
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification004, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification004 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "Id1";
    Value value1 = "subscribe";
    status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";

    Key key2 = "Id2";
    Value value2 = "subscribe";
    status = kvStorePtr->Put(key2, value2);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    Key key3 = "Id3";
    Value value3 = "subscribe";
    status = kvStorePtr->Put(key3, value3);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 2);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id3", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification005
* @tc.desc: Subscribe to an observer, callback with notification many times after put the different data
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification005, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification005 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "Id1";
    Value value1 = "subscribe";
    status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    Key key2 = "Id2";
    Value value2 = "subscribe";
    status = kvStorePtr->Put(key2, value2);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id2", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    Key key3 = "Id3";
    Value value3 = "subscribe";
    status = kvStorePtr->Put(key3, value3);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id3", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 3);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification006
* @tc.desc: Subscribe to an observer, callback with notification many times after put the same data
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification006, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification006 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "Id1";
    Value value1 = "subscribe";
    status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    Key key2 = "Id1";
    Value value2 = "subscribe";
    status = kvStorePtr->Put(key2, value2);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->updateEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->updateEntries_[0].value.ToString());

    Key key3 = "Id1";
    Value value3 = "subscribe";
    status = kvStorePtr->Put(key3, value3);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->updateEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->updateEntries_[0].value.ToString());

    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 3);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification007
* @tc.desc: Subscribe to an observer, callback with notification many times after put&update
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification007, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification007 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    Key key1 = "Id1";
    Value value1 = "subscribe";
    Status status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    Key key2 = "Id2";
    Value value2 = "subscribe";
    status = kvStorePtr->Put(key2, value2);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key3 = "Id1";
    Value value3 = "subscribe03";
    status = kvStorePtr->Put(key3, value3);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->updateEntries_[0].key.ToString());
    EXPECT_EQ("subscribe03", observer->updateEntries_[0].value.ToString());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification008
* @tc.desc: Subscribe to an observer, callback with notification one times after putbatch&update
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification008, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification008 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;

    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();
    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    entries.clear();
    entry1.key = "Id1";
    entry1.value = "subscribe_modify";
    entry2.key = "Id2";
    entry2.value = "subscribe_modify";
    entries.push_back(entry1);
    entries.push_back(entry2);
    status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 2);
    EXPECT_EQ("Id1", observer->updateEntries_[0].key.ToString());
    EXPECT_EQ("subscribe_modify", observer->updateEntries_[0].value.ToString());
    EXPECT_EQ("Id2", observer->updateEntries_[1].key.ToString());
    EXPECT_EQ("subscribe_modify", observer->updateEntries_[1].value.ToString());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification009
* @tc.desc: Subscribe to an observer, callback with notification one times after putbatch all different data
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification009, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification009 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;

    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 3);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());
    EXPECT_EQ("Id2", observer->insertEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[1].value.ToString());
    EXPECT_EQ("Id3", observer->insertEntries_[2].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[2].value.ToString());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification010
* @tc.desc: Subscribe to an observer, callback with notification one times after putbatch both different and same data
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification010, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification010 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;

    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id1";
    entry2.value = "subscribe";
    entry3.key = "Id2";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 2);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());
    EXPECT_EQ("Id2", observer->insertEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[1].value.ToString());
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 0);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 0);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification011
* @tc.desc: Subscribe to an observer, callback with notification one times after putbatch all same data
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification011, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification011 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;

    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id1";
    entry2.value = "subscribe";
    entry3.key = "Id1";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 0);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 0);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification012
* @tc.desc: Subscribe to an observer, callback with notification many times after putbatch all different data
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification012, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification012 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    std::vector<Entry> entries1;
    Entry entry1, entry2, entry3;

    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries1.push_back(entry1);
    entries1.push_back(entry2);
    entries1.push_back(entry3);

    std::vector<Entry> entries2;
    Entry entry4, entry5;
    entry4.key = "Id4";
    entry4.value = "subscribe";
    entry5.key = "Id5";
    entry5.value = "subscribe";
    entries2.push_back(entry4);
    entries2.push_back(entry5);

    status = kvStorePtr->PutBatch(entries1);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 3);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());
    EXPECT_EQ("Id2", observer->insertEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[1].value.ToString());
    EXPECT_EQ("Id3", observer->insertEntries_[2].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[2].value.ToString());

    status = kvStorePtr->PutBatch(entries2);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 2);
    EXPECT_EQ("Id4", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());
    EXPECT_EQ("Id5", observer->insertEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[1].value.ToString());

    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 2);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification013
* @tc.desc: Subscribe to an observer, callback with notification many times after putbatch both different and same data
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification013, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification013 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    std::vector<Entry> entries1;
    Entry entry1, entry2, entry3;

    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries1.push_back(entry1);
    entries1.push_back(entry2);
    entries1.push_back(entry3);

    std::vector<Entry> entries2;
    Entry entry4, entry5;
    entry4.key = "Id1";
    entry4.value = "subscribe";
    entry5.key = "Id4";
    entry5.value = "subscribe";
    entries2.push_back(entry4);
    entries2.push_back(entry5);

    status = kvStorePtr->PutBatch(entries1);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 3);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());
    EXPECT_EQ("Id2", observer->insertEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[1].value.ToString());
    EXPECT_EQ("Id3", observer->insertEntries_[2].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[2].value.ToString());

    status = kvStorePtr->PutBatch(entries2);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->updateEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->updateEntries_[0].value.ToString());
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id4", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 2);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification014
* @tc.desc: Subscribe to an observer, callback with notification many times after putbatch all same data
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification014, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification014 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    std::vector<Entry> entries1;
    Entry entry1, entry2, entry3;

    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries1.push_back(entry1);
    entries1.push_back(entry2);
    entries1.push_back(entry3);

    std::vector<Entry> entries2;
    Entry entry4, entry5;
    entry4.key = "Id1";
    entry4.value = "subscribe";
    entry5.key = "Id2";
    entry5.value = "subscribe";
    entries2.push_back(entry4);
    entries2.push_back(entry5);

    status = kvStorePtr->PutBatch(entries1);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 3);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());
    EXPECT_EQ("Id2", observer->insertEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[1].value.ToString());
    EXPECT_EQ("Id3", observer->insertEntries_[2].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[2].value.ToString());

    status = kvStorePtr->PutBatch(entries2);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 2);
    EXPECT_EQ("Id1", observer->updateEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->updateEntries_[0].value.ToString());
    EXPECT_EQ("Id2", observer->updateEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->updateEntries_[1].value.ToString());

    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 2);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification015
* @tc.desc: Subscribe to an observer, callback with notification many times after putbatch complex data
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification015, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification015 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    std::vector<Entry> entries1;
    Entry entry1, entry2, entry3;

    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id1";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries1.push_back(entry1);
    entries1.push_back(entry2);
    entries1.push_back(entry3);

    std::vector<Entry> entries2;
    Entry entry4, entry5;
    entry4.key = "Id1";
    entry4.value = "subscribe";
    entry5.key = "Id2";
    entry5.value = "subscribe";
    entries2.push_back(entry4);
    entries2.push_back(entry5);

    status = kvStorePtr->PutBatch(entries1);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 0);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 0);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 2);
    EXPECT_EQ("Id1", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());
    EXPECT_EQ("Id3", observer->insertEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[1].value.ToString());

    status = kvStorePtr->PutBatch(entries2);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->updateEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->updateEntries_[0].value.ToString());
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 1);
    EXPECT_EQ("Id2", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->insertEntries_[0].value.ToString());

    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 2);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification016
* @tc.desc: Pressure test subscribe, callback with notification many times after putbatch
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification016, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification016 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    const int ENTRIES_MAX_LEN = 100;
    std::vector<Entry> entries;
    for (int i = 0; i < ENTRIES_MAX_LEN; i++) {
        Entry entry;
        entry.key = std::to_string(i);
        entry.value = "subscribe";
        entries.push_back(entry);
    }

    status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    usleep(USLEEP_TIME);

    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 100);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification017
* @tc.desc: Subscribe to an observer, callback with notification after delete success
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification017, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification017 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->Delete("Id1");
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Delete data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->deleteEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->deleteEntries_[0].value.ToString());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification018
* @tc.desc: Subscribe to an observer, not callback after delete which key not exist
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification018, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification018 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->Delete("Id4");
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Delete data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 0);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 0);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification019
* @tc.desc: Subscribe to an observer, delete the same data many times and only first delete callback with notification
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification019, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification019 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";
    status = kvStorePtr->Delete("Id1");
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Delete data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 1);
    EXPECT_EQ("Id1", observer->deleteEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->deleteEntries_[0].value.ToString());

    status = kvStorePtr->Delete("Id1");
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Delete data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 1); // not callback so not clear

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification020
* @tc.desc: Subscribe to an observer, callback with notification after deleteBatch
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification020, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification020 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("Id1");
    keys.push_back("Id2");

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    status = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore DeleteBatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 2);
    EXPECT_EQ("Id1", observer->deleteEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->deleteEntries_[0].value.ToString());
    EXPECT_EQ("Id2", observer->deleteEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->deleteEntries_[1].value.ToString());

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification021
* @tc.desc: Subscribe to an observer, not callback after deleteBatch which all keys not exist
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification021, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification021 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("Id4");
    keys.push_back("Id5");

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    status = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore DeleteBatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 0);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 0);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification022
* @tc.desc: Subscribe to an observer, deletebatch the same data many times and only first deletebatch callback with
* notification
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification022, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification022 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id1";
    entry1.value = "subscribe";
    entry2.key = "Id2";
    entry2.value = "subscribe";
    entry3.key = "Id3";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("Id1");
    keys.push_back("Id2");

    Status status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    status = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore DeleteBatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 2);
    EXPECT_EQ("Id1", observer->deleteEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->deleteEntries_[0].value.ToString());
    EXPECT_EQ("Id2", observer->deleteEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->deleteEntries_[1].value.ToString());

    status = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore DeleteBatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 2); // not callback so not clear

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification023
* @tc.desc: Subscribe to an observer, include Clear Put PutBatch Delete DeleteBatch
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification023, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification023 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "Id1";
    Value value1 = "subscribe";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id2";
    entry1.value = "subscribe";
    entry2.key = "Id3";
    entry2.value = "subscribe";
    entry3.key = "Id4";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("Id2");
    keys.push_back("Id3");

    kvStorePtr->Clear();
    status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    status = kvStorePtr->Delete(key1);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore delete data return wrong status";
    status = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore DeleteBatch data return wrong status";
    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 5);
    // every callback will clear vector
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 2);
    EXPECT_EQ("Id2", observer->deleteEntries_[0].key.ToString());
    EXPECT_EQ("subscribe", observer->deleteEntries_[0].value.ToString());
    EXPECT_EQ("Id3", observer->deleteEntries_[1].key.ToString());
    EXPECT_EQ("subscribe", observer->deleteEntries_[1].value.ToString());
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 0);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 0);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification024
* @tc.desc: Subscribe to an observer[use transaction], include Clear Put PutBatch Delete DeleteBatch
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification024, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification024 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "Id1";
    Value value1 = "subscribe";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id2";
    entry1.value = "subscribe";
    entry2.key = "Id3";
    entry2.value = "subscribe";
    entry3.key = "Id4";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("Id2");
    keys.push_back("Id3");

    status = kvStorePtr->StartTransaction();
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore startTransaction return wrong status";
    kvStorePtr->Clear();
    status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    status = kvStorePtr->Delete(key1);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore delete data return wrong status";
    status = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore DeleteBatch data return wrong status";
    status = kvStorePtr->Commit();
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Commit return wrong status";

    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 1);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification025
* @tc.desc: Subscribe to an observer[use transaction], include Clear Put PutBatch Delete DeleteBatch
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification025, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification025 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();
    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    Key key1 = "Id1";
    Value value1 = "subscribe";

    std::vector<Entry> entries;
    Entry entry1, entry2, entry3;
    entry1.key = "Id2";
    entry1.value = "subscribe";
    entry2.key = "Id3";
    entry2.value = "subscribe";
    entry3.key = "Id4";
    entry3.value = "subscribe";
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);

    std::vector<Key> keys;
    keys.push_back("Id2");
    keys.push_back("Id3");

    status = kvStorePtr->StartTransaction();
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore startTransaction return wrong status";
    kvStorePtr->Clear();
    status = kvStorePtr->Put(key1, value1);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";
    status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";
    status = kvStorePtr->Delete(key1);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore delete data return wrong status";
    status = kvStorePtr->DeleteBatch(keys);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore DeleteBatch data return wrong status";
    status = kvStorePtr->Rollback();
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore Commit return wrong status";

    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 0);
    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 0);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 0);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 0);

    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
    observer = nullptr;
}

/**
* @tc.name: KvStoreDdmSubscribeKvStoreNotification026
* @tc.desc: Subscribe to an observer[use transaction], include bigData PutBatch  update  insert delete
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: dukaizhan
*/
HWTEST_F(LocalSubscribeStoreTest, KvStoreDdmSubscribeKvStoreNotification026, TestSize.Level2)
{
    ZLOGI("KvStoreDdmSubscribeKvStoreNotification026 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    kvStorePtr->Clear();

    std::shared_ptr<KvStoreObserverUnitTest> observer = std::make_shared<KvStoreObserverUnitTest>();
    observer->ResetToZero();

    SubscribeType subscribeType = SubscribeType::SUBSCRIBE_TYPE_ALL;
    Status status = kvStorePtr->SubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "SubscribeKvStore return wrong status";

    std::vector<Entry> entries;
    Entry entry0, entry1, entry2, entry3, entry4, entry5, entry6, entry7;

    int maxValueSize = 2 * 1024 * 1024; // max value size is 2M.
    std::vector<uint8_t> val(maxValueSize);
    for (int i = 0; i < maxValueSize; i++) {
        val[i] = static_cast<uint8_t>(i);
    }
    Value value = val;

    int maxValueSize2 = 1000 * 1024; // max value size is 1000k.
    std::vector<uint8_t> val2(maxValueSize2);
    for (int i = 0; i < maxValueSize2; i++) {
        val2[i] = static_cast<uint8_t>(i);
    }
    Value value2 = val2;

    entry0.key = "SingleKvStoreDdmPutBatch006_0";
    entry0.value = "beijing";
    entry1.key = "SingleKvStoreDdmPutBatch006_1";
    entry1.value = value;
    entry2.key = "SingleKvStoreDdmPutBatch006_2";
    entry2.value = value;
    entry3.key = "SingleKvStoreDdmPutBatch006_3";
    entry3.value = "ZuiHouBuZhiTianZaiShui";
    entry4.key = "SingleKvStoreDdmPutBatch006_4";
    entry4.value = value;

    entries.push_back(entry0);
    entries.push_back(entry1);
    entries.push_back(entry2);
    entries.push_back(entry3);
    entries.push_back(entry4);

    status = kvStorePtr->PutBatch(entries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    usleep(USLEEP_TIME);

    EXPECT_EQ(static_cast<int>(observer->insertEntries_.size()), 5);
    EXPECT_EQ("SingleKvStoreDdmPutBatch006_0", observer->insertEntries_[0].key.ToString());
    EXPECT_EQ("beijing", observer->insertEntries_[0].value.ToString());
    EXPECT_EQ("SingleKvStoreDdmPutBatch006_1", observer->insertEntries_[1].key.ToString());
    EXPECT_EQ("SingleKvStoreDdmPutBatch006_2", observer->insertEntries_[2].key.ToString());
    EXPECT_EQ("SingleKvStoreDdmPutBatch006_3", observer->insertEntries_[3].key.ToString());
    EXPECT_EQ("ZuiHouBuZhiTianZaiShui", observer->insertEntries_[3].value.ToString());

    entry5.key = "SingleKvStoreDdmPutBatch006_2";
    entry5.value = val2;
    entry6.key = "SingleKvStoreDdmPutBatch006_3";
    entry6.value = "ManChuanXingMengYaXingHe";
    entry7.key = "SingleKvStoreDdmPutBatch006_4";
    entry7.value = val2;
    std::vector<Entry> updateEntries;
    updateEntries.push_back(entry5);
    updateEntries.push_back(entry6);
    updateEntries.push_back(entry7);
    status = kvStorePtr->PutBatch(updateEntries);
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putBatch update data return wrong status";

    usleep(USLEEP_TIME);
    EXPECT_EQ(static_cast<int>(observer->updateEntries_.size()), 3);
    EXPECT_EQ("SingleKvStoreDdmPutBatch006_2", observer->updateEntries_[0].key.ToString());
    EXPECT_EQ("SingleKvStoreDdmPutBatch006_3", observer->updateEntries_[1].key.ToString());
    EXPECT_EQ("ManChuanXingMengYaXingHe", observer->updateEntries_[1].value.ToString());
    EXPECT_EQ("SingleKvStoreDdmPutBatch006_4", observer->updateEntries_[2].key.ToString());
    EXPECT_EQ(false, observer->isClear_);

    status = kvStorePtr->Delete("SingleKvStoreDdmPutBatch006_3");
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore delete data return wrong status";
    usleep(1000000);
    EXPECT_EQ(static_cast<int>(observer->deleteEntries_.size()), 1);
    EXPECT_EQ("SingleKvStoreDdmPutBatch006_3", observer->deleteEntries_[0].key.ToString());

    EXPECT_EQ(static_cast<int>(observer->GetCallCount()), 3);
    status = kvStorePtr->UnSubscribeKvStore(subscribeType, observer);
    EXPECT_EQ(Status::SUCCESS, status) << "UnSubscribeKvStore return wrong status";
}

/**
* @tc.name: ChangeNotificationMarshalling001
* @tc.desc: Test changenotification marshalling and unmarshalling function
* @tc.type: FUNC
* @tc.require: AR000CIFGM
* @tc.author: liuyuhui
*/
HWTEST_F(LocalSubscribeStoreTest, ChangeNotificationMarshalling001, TestSize.Level1)
{
    ZLOGI("ChangeNotificationMarshalling001 begin.");
    Entry insert, update, del;
    insert.key = "insert";
    update.key = "update";
    del.key = "delete";
    insert.value = "insert_value";
    update.value = "update_value";
    del.value = "delete_value";
    std::list<Entry> insertEntries, updateEntries, deleteEntries;
    insertEntries.push_back(insert);
    updateEntries.push_back(update);
    deleteEntries.push_back(del);

    ChangeNotification changeIn(insertEntries, updateEntries, deleteEntries, std::string(), false);
    OHOS::Parcel parcel;
    changeIn.Marshalling(parcel);
    ChangeNotification *changeOut = ChangeNotification::Unmarshalling(parcel);
    ASSERT_NE(changeOut, nullptr);
    ASSERT_EQ(changeOut->GetInsertEntries().size(), 1UL);
    EXPECT_EQ(changeOut->GetInsertEntries().front().key.ToString(), std::string("insert"));
    EXPECT_EQ(changeOut->GetInsertEntries().front().value.ToString(), std::string("insert_value"));
    ASSERT_EQ(changeOut->GetUpdateEntries().size(), 1UL);
    EXPECT_EQ(changeOut->GetUpdateEntries().front().key.ToString(), std::string("update"));
    EXPECT_EQ(changeOut->GetUpdateEntries().front().value.ToString(), std::string("update_value"));
    ASSERT_EQ(changeOut->GetDeleteEntries().size(), 1UL);
    EXPECT_EQ(changeOut->GetDeleteEntries().front().key.ToString(), std::string("delete"));
    EXPECT_EQ(changeOut->GetDeleteEntries().front().value.ToString(), std::string("delete_value"));
    EXPECT_EQ(changeOut->IsClear(), false);
    delete changeOut;
}

