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

#define LOG_TAG "KvStoreSnapshotClientTest"

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

class KvStoreSnapshotClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::unique_ptr<KvStore> kvStorePtr;                  // declare kv store instance.
    static Status statusGetKvStore;
    static DistributedKvDataManager manager;
};

DistributedKvDataManager KvStoreSnapshotClientTest::manager;
std::unique_ptr<KvStore> KvStoreSnapshotClientTest::kvStorePtr = nullptr;
Status KvStoreSnapshotClientTest::statusGetKvStore = Status::ERROR;

void KvStoreSnapshotClientTest::SetUpTestCase(void)
{
    ZLOGI("KvStoreSnapshotClientTest::SetUpTestCase");
    Options options;
    options.createIfMissing = true;
    options.encrypt = false;  // not supported yet.
    options.autoSync = true;  // not supported yet.
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
}

void KvStoreSnapshotClientTest::TearDownTestCase(void)
{
    ZLOGI("KvStoreSnapshotClientTest::TearDownTestCase");
    if (kvStorePtr != nullptr) {
        AppId appId;
        appId.appId = "odmf";  // define app name.
        StoreId storeId;
        storeId.storeId = "student";  // define kvstore(database) name.
        manager.CloseKvStore(appId, storeId);
    }
}

void KvStoreSnapshotClientTest::SetUp(void)
{}

void KvStoreSnapshotClientTest::TearDown(void)
{}

/**
* @tc.name: KvStoreSnapshotDdmGet001
* @tc.desc: Get values of keys in KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreSnapshotClientTest, KvStoreSnapshotDdmGet001, TestSize.Level1)
{
    ZLOGI("KvStoreSnapshotDdmGet001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "age";
    Value value = "18";
    Status status = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore put data return wrong status";

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
    EXPECT_EQ(Status::SUCCESS, statusRet) << "KvStoreSnapshot get data return wrong status";

    EXPECT_EQ(value, valueRet) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreSnapshotDdmGet002
* @tc.desc: Get key values in KvStore when keys do not exist.
* @tc.type: FUNC
* @tc.require: AR000C6GBG
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreSnapshotClientTest, KvStoreSnapshotDdmGet002, TestSize.Level1)
{
    ZLOGI("KvStoreSnapshotDdmGet002 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";

    Key key = "age";
    Value value = "18";
    Status status1 = kvStorePtr->Put(key, value);  // insert or update key-value
    EXPECT_EQ(Status::SUCCESS, status1) << "KvStore put data return wrong status";

    Status status2 = kvStorePtr->Delete(key);  // delete data
    EXPECT_EQ(Status::SUCCESS, status2) << "KvStore delete data return wrong status";

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
    EXPECT_EQ(Status::KEY_NOT_FOUND, statusRet);
    EXPECT_EQ(Status::KEY_NOT_FOUND, statusRet) << "KvStoreSnapshot get data return wrong status";
    EXPECT_EQ("", valueRet.ToString()) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreSnapshotDdmGet003
* @tc.desc: Get key values in KvStore when keys are invalid.
* @tc.type: FUNC
* @tc.require: AR000C6GBG
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreSnapshotClientTest, KvStoreSnapshotDdmGet003, TestSize.Level1)
{
    ZLOGI("KvStoreSnapshotDdmGet003 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
    EXPECT_NE(nullptr, kvStorePtr) << "kvStorePtr is nullptr";
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;
    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
                                   [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
                                       kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
                                   });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";

    // get value from kvstore.
    Key key;
    Value valueRet;
    Status statusRet = kvStoreSnapshotPtr->Get(key, valueRet);
    EXPECT_EQ(Status::INVALID_ARGUMENT, statusRet);
    EXPECT_EQ(Status::INVALID_ARGUMENT, statusRet) << "KvStoreSnapshot get data return wrong status";
    EXPECT_EQ("", valueRet.ToString()) << "value and valueRet are not equal";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreSnapshotDdmGetEntries001
* @tc.desc: Get key entries from KvStore.
* @tc.type: FUNC
* @tc.require: AR000C6GBG
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreSnapshotClientTest, KvStoreSnapshotDdmGetEntries001, TestSize.Level1)
{
    ZLOGI("KvStoreSnapshotDdmGetEntries001 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
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
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    // get entries
    Key keyPrefixStudent = "student_name_";
    std::vector<Entry> students;
    Key token;
    Status statusTmp = Status::ERROR;
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;
    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            statusTmp = std::move(status);
            students = std::move(entries);
        });
    EXPECT_EQ(3, static_cast<int>(students.size())) << "GetEntries failed";
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStore GetEntries data return wrong status";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreSnapshotDdmGetEntries002
* @tc.desc: Get key entries from KvStore when the keyPrefix does not exist.
* @tc.type: FUNC
* @tc.require: AR000C6GBG
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreSnapshotClientTest, KvStoreSnapshotDdmGetEntries002, TestSize.Level1)
{
    ZLOGI("KvStoreSnapshotDdmGetEntries002 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
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
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    // get entries
    Key keyPrefixStudent = "teacher_name_";
    std::vector<Entry> students;
    Key token;
    Status statusTmp = Status::ERROR;
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;
    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            statusTmp = std::move(status);
            students = std::move(entries);
        });
    EXPECT_EQ(0, static_cast<int>(students.size())) << "GetEntries fail";
    EXPECT_EQ(Status::KEY_NOT_FOUND, statusTmp) << "KvStore GetEntries data return wrong status";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreSnapshotDdmGetEntries003
* @tc.desc: Get all key entries from KvStore when the keys are empty.
* @tc.type: FUNC
* @tc.require: AR000C6GBG
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreSnapshotClientTest, KvStoreSnapshotDdmGetEntries003, TestSize.Level1)
{
    ZLOGI("KvStoreSnapshotDdmGetEntries003 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
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
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    // get entries
    Key keyPrefixStudent = "";
    std::vector<Entry> students;
    Key token;
    Status statusTmp = Status::ERROR;
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;
    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            statusTmp = std::move(status);
            students = std::move(entries);
        });
    EXPECT_EQ(3, static_cast<int>(students.size())) << "GetEntries fail";
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStore GetEntries data return wrong status";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: KvStoreSnapshotDdmGetEntries004
* @tc.desc: Get all key entries from KvStore when the keys contain only space blank.
* @tc.type: FUNC
* @tc.require: AR000C6GBG
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreSnapshotClientTest, KvStoreSnapshotDdmGetEntries004, TestSize.Level1)
{
    ZLOGI("KvStoreSnapshotDdmGetEntries004 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
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
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    // get entries
    Key keyPrefixStudent = "       ";
    std::vector<Entry> students;
    Key token;
    Status statusTmp = Status::ERROR;
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;
    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            statusTmp = std::move(status);
            students = std::move(entries);
        });
    EXPECT_EQ(3, static_cast<int>(students.size())) << "GetEntries fail";
    EXPECT_EQ(Status::SUCCESS, statusTmp) << "KvStore GetEntries data return wrong status";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
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
* @tc.name: KvStoreSnapshotDdmGetEntries005
* @tc.desc: Get key entries from KvStore when the keyPrefix is invalid.
* @tc.type: FUNC
* @tc.require: AR000C6GBG
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreSnapshotClientTest, KvStoreSnapshotDdmGetEntries005, TestSize.Level1)
{
    ZLOGI("KvStoreSnapshotDdmGetEntries005 begin.");
    EXPECT_EQ(Status::SUCCESS, statusGetKvStore) << "statusGetKvStore return wrong status";
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
    EXPECT_EQ(Status::SUCCESS, status) << "KvStore putbatch data return wrong status";

    // get entries
    Key keyPrefixStudent = Generate1025KeyLen();
    std::vector<Entry> students;
    Key token;
    Status statusTmp = Status::ERROR;
    std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr;
    // [create and] open and initialize kvstore snapshot instance.
    kvStorePtr->GetKvStoreSnapshot(nullptr, /* (KvStoreObserver ) */
        [&](Status status, std::unique_ptr<KvStoreSnapshot> kvStoreSnapshot) {
            kvStoreSnapshotPtr = std::move(kvStoreSnapshot);
        });

    EXPECT_NE(nullptr, kvStoreSnapshotPtr) << "kvStoreSnapshotPtr is nullptr";
    kvStoreSnapshotPtr->GetEntries(keyPrefixStudent,
        [&](Status status, std::vector<Entry> &entries) {
            statusTmp = std::move(status);
            students = std::move(entries);
        });
    EXPECT_EQ(0, static_cast<int>(students.size())) << "invalid argument, GetEntries fail";
    EXPECT_EQ(Status::INVALID_ARGUMENT, statusTmp) << "KvStore GetEntries data return wrong status";
    kvStorePtr->ReleaseKvStoreSnapshot(std::move(kvStoreSnapshotPtr));
}

/**
* @tc.name: EntryIpcInterfaceTest001
* @tc.desc: Marshal and unmarshall key entries.
* @tc.type: FUNC
* @tc.require: AR000C6GBG
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreSnapshotClientTest, EntryIpcInterfaceTest001, TestSize.Level1)
{
    ZLOGI("EntryIpcInterfaceTest001 begin.");
    Entry entryIn, *entryOut;
    entryIn.key = "student_name_mali";
    entryIn.value = "age:20";
    OHOS::Parcel parcel;
    entryIn.Marshalling(parcel);
    entryOut = Entry::Unmarshalling(parcel);
    EXPECT_NE(entryOut, nullptr);
    EXPECT_EQ(entryOut->key.ToString(), std::string("student_name_mali"));
    EXPECT_EQ(entryOut->value.ToString(), std::string("age:20"));
    delete entryOut;
}
