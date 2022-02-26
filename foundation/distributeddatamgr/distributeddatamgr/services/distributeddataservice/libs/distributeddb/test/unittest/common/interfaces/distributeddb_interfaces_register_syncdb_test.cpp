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
#include <thread>

#include "distributeddb_tools_unit_test.h"
#include "distributeddb_data_generate_unit_test.h"
#include "kv_store_observer.h"
#include "securec.h"
#include "platform_specific.h"
#include "db_common.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    const int OBSERVER_SLEEP_TIME = 100;
    const bool LOCAL_ONLY = false;
    const string STORE_ID = STORE_ID_SYNC;
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    KvStoreConfig g_config;
    KvStoreObserverUnitTest *g_observer = nullptr;

    // define the g_kvDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreDelegate *g_kvDelegatePtr = nullptr;
    DBStatus g_snapshotDelegateStatus = INVALID_ARGS;
    KvStoreSnapshotDelegate *g_snapshotDelegatePtr = nullptr;
    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));

    // the type of g_snapshotDelegateCallback is function<void(DBStatus, KvStoreSnapshotDelegate*)>
    auto g_snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_snapshotDelegateStatus), std::ref(g_snapshotDelegatePtr));

    vector<uint8_t> TransStrToVector(const string &input)
    {
        vector<uint8_t> output(input.begin(), input.end());
        return output;
    }

    void PrintfEntryList(std::list<Entry> inEntryList)
    {
        LOGI("begin print entry list! EntryList size [%zu]", inEntryList.size());
        for (const auto &entry : inEntryList) {
            string temp = DBCommon::VectorToHexString(entry.value);

            LOGI("key[%s]", DBCommon::VectorToHexString(entry.key).c_str());
            LOGI("value[%s]", temp.c_str());
            LOGI("value size[%zu]", temp.size());
        }
    }

    bool TestEntryList(const list<Entry> &entries, const list<Entry> &expectEntries)
    {
        bool checkRes = true;
        EXPECT_EQ(entries.size(), expectEntries.size());
        bool findEntry = false;
        for (const auto &entry : entries) {
            findEntry = false;
            for (const auto &expectEntry : expectEntries) {
                if (entry.key != expectEntry.key) {
                    continue;
                }
                if (entry.value != expectEntry.value) {
                    LOGE("entry[%s]:[%s]", DBCommon::VectorToHexString(entry.value).c_str(),
                        DBCommon::VectorToHexString(expectEntry.value).c_str());
                    checkRes = false;
                    goto END;
                } else {
                    findEntry = true;
                    break;
                }
            }
            if (!findEntry) {
                LOGE("No value can matches!");
                checkRes = false;
                goto END;
            }
        }
    END:
        if (!checkRes) {
            PrintfEntryList(entries);
            PrintfEntryList(expectEntries);
        }
        return checkRes;
    }

    Entry GetEntry(const string &keyStr, const string &valueStr)
    {
        Entry entry;
        entry.key = TransStrToVector(keyStr);
        entry.value = TransStrToVector(valueStr);
        return entry;
    }
}

class DistributedDBInterfacesRegisterSyncDBTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesRegisterSyncDBTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);

    string dir = g_testDir + "/multi_ver";
    DIR *dirTmp = opendir(dir.c_str());
    if (dirTmp == nullptr) {
        OS::MakeDBDirectory(dir);
    } else {
        closedir(dirTmp);
    }
}

void DistributedDBInterfacesRegisterSyncDBTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesRegisterSyncDBTest::SetUp(void)
{
    /*
     * Here, we create STORE_ID before test,
     * and it will be closed in TearDown().
     */
    KvStoreDelegate::Option option = {true, LOCAL_ONLY};
    g_mgr.GetKvStore(STORE_ID, option, g_kvDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);

    g_observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(g_observer != nullptr);
    g_observer->ResetToZero();
}

void DistributedDBInterfacesRegisterSyncDBTest::TearDown(void)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    if (g_kvDelegatePtr != nullptr) {
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
        g_kvDelegatePtr = nullptr;
        EXPECT_TRUE(g_mgr.DeleteKvStore(STORE_ID) == OK);
    }

    if (g_observer != nullptr) {
        delete g_observer;
        g_observer = nullptr;
    }
}

/**
  * @tc.name: RegisterObserver001
  * @tc.desc: normal register observer success.
  * @tc.type: FUNC
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver001, TestSize.Level1)
{
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver002
  * @tc.desc: register(null object) observer success
  * @tc.type: FUNC
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver002, TestSize.Level1)
{
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(nullptr) == INVALID_ARGS);
}

/**
  * @tc.name: RegisterObserver003
  * @tc.desc: Test the new data and check the processing result of the callback function.
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);
    Key key;
    Value value;
    key.push_back('a');
    value.push_back('a');

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->Put
     * @tc.expected: step2. Return OK.
     */
    DBStatus status = g_kvDelegatePtr->Put(key, value);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver004
  * @tc.desc: register observer success and putbach callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);
    vector<Entry> entries;
    for (int i = 1; i < 11; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
    }

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->PutBatch
     * @tc.expected: step2. Return OK.
     */
    DBStatus status = g_kvDelegatePtr->PutBatch(entries);

    EXPECT_TRUE(status == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    LOGI("observer count:%lu", g_observer->GetCallCount());
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver005
  * @tc.desc: register observer success and putbach callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver005, TestSize.Level1)
{
    vector<Entry> entries;
    for (int i = 1; i < 6; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
    }
    DBStatus status = g_kvDelegatePtr->PutBatch(entries);
    EXPECT_TRUE(status == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);
    entries.clear();
    for (int i = 1; i < 11; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
    }

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->PutBatch
     * @tc.expected: step2. Return OK.
     */
    status = g_kvDelegatePtr->PutBatch(entries);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver006
  * @tc.desc: register observer success and update callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver006, TestSize.Level1)
{
    Key key;
    Value value1;
    Value value2;
    key.push_back(1);
    value1.push_back(8);
    value2.push_back(10);
    DBStatus status = g_kvDelegatePtr->Put(key, value1);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->Put(k1,v2)
     * @tc.expected: step2. Return OK.
     */
    status = g_kvDelegatePtr->Put(key, value2);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver007
  * @tc.desc: register observer success and delete callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver007, TestSize.Level1)
{
    Key key;
    Value value;
    key.push_back(1);
    value.push_back(8);
    DBStatus status = g_kvDelegatePtr->Put(key, value);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->Delete
     * @tc.expected: step2. Return OK.
     */
    status = g_kvDelegatePtr->Delete(key);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver008
  * @tc.desc: register observer success and delete callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver008, TestSize.Level1)
{
    Key key;
    key.push_back(1);

    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->Delete with no value
     * @tc.expected: step2. Return OK.
     */
    DBStatus status = g_kvDelegatePtr->Delete(key);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Do not print log.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 0);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver009
  * @tc.desc: register observer success and deletebatch callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver009, TestSize.Level1)
{
    vector<Entry> entries;
    vector<Key> keys;
    for (int i = 1; i < 6; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
        keys.push_back(entry.key);
    }
    DBStatus status = g_kvDelegatePtr->PutBatch(entries);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->DeleteBatch
     * @tc.expected: step2. Return OK.
     */
    status = g_kvDelegatePtr->DeleteBatch(keys);
    EXPECT_TRUE(status == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver010
  * @tc.desc: register observer success and delete callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver010, TestSize.Level1)
{
    vector<Entry> entries;
    vector<Key> keys;
    for (int i = 1; i < 6; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
    }
    for (int i = 1; i < 11; i++) {
        Key key;
        key.push_back(i);
        keys.push_back(key);
    }
    DBStatus status = g_kvDelegatePtr->PutBatch(entries);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->DeleteBatch
     * @tc.expected: step2. Return OK.
     */
    status = g_kvDelegatePtr->DeleteBatch(keys);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver011
  * @tc.desc: register observer success and DeleteBatch callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver011, TestSize.Level1)
{
    vector<Key> keys;
    for (int i = 1; i < 11; i++) {
        Key key;
        key.push_back(i);
        keys.push_back(key);
    }

    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->DeleteBatch with no value
     * @tc.expected: step2. Return OK.
     */
    DBStatus status = g_kvDelegatePtr->DeleteBatch(keys);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Do not print logy.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 0);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver012
  * @tc.desc: register observer success and clear callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver012, TestSize.Level1)
{
    vector<Entry> entries;
    vector<Key> keys;
    for (int i = 1; i < 20; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
        keys.push_back(entry.key);
    }
    DBStatus status = g_kvDelegatePtr->PutBatch(entries);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->Clear
     * @tc.expected: step2. Return OK.
     */
    status = g_kvDelegatePtr->Clear();
    EXPECT_TRUE(status == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver013
  * @tc.desc: register observer success and clear callback
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver013, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);

    /**
     * @tc.steps:step2. Test g_kvDelegatePtr->Clear with no key and value
     * @tc.expected: step2. Return OK.
     */
    DBStatus status = g_kvDelegatePtr->Clear();
    EXPECT_TRUE(status == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: RegisterObserver014
  * @tc.desc: Test the function of modifying a record, adding a record,
  *  deleting a record, and checking the processing result of the callback function.
  * @tc.type: FUNC
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, RegisterObserver014, TestSize.Level1)
{
    Key key;
    Value value;
    key.push_back(1);
    value.push_back(1);
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);

    key.clear();
    value.clear();
    key.push_back(2);
    value.push_back(2);
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test KvStoreDelegate.RegisterObserver
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);
    key.clear();
    value.clear();
    key.push_back(1);
    value.push_back(4);

    /**
     * @tc.steps:step2. Put(k1,v4)
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    key.clear();
    value.clear();
    key.push_back(3);
    value.push_back(3);

    /**
     * @tc.steps:step4. Put(k3,v3)
     * @tc.expected: step4. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step5. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step5. Print log normally.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 2);
    value.push_back(4);

    key.clear();
    key.push_back(2);

    /**
     * @tc.steps:step6. Delete(k2)
     * @tc.expected: step6. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Delete(key) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step7. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step7. Print log normally.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 3);

    key.clear();
    key.push_back(4);

    /**
     * @tc.steps:step8. Delete(k4)
     * @tc.expected: step8. Return OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Delete(key), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step9. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step9. Print log normally.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 3);

    /**
     * @tc.steps:step10. Clear data
     * @tc.expected: step10. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step11. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step11. Print log normally.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 4);

    /**
     * @tc.steps:step12. Clear data repeat
     * @tc.expected: step12. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step13. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step13. Print log normally.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 5);

    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: SnapshotRegisterObserver001
  * @tc.desc: register a normal observer for snapshot
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver002
  * @tc.desc:  register a null observer for snapshot register a null observer for snapshot
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver003
  * @tc.desc: register observer success and put callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot with
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    Key key;
    Value value;
    key.push_back(1);
    value.push_back(1);

    /**
     * @tc.steps:step2. Clear data repeat
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

static void CreatEntrysData(size_t size, uint8_t value, vector<Entry> &entries)
{
    for (size_t i = 0; i < size; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back(value);
        entries.push_back(entry);
    }
}

/**
  * @tc.name: SnapshotRegisterObserver004
  * @tc.desc: register observer success and putBatch callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot with
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    vector<Entry> entries;
    CreatEntrysData(10, '8', entries);

    /**
     * @tc.steps:step2. Put data by PutBatch
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver005
  * @tc.desc: register observer success and putBatch callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver005, TestSize.Level1)
{
    vector<Entry> entries;
    CreatEntrysData(6, '8', entries);

    DBStatus status = g_kvDelegatePtr->PutBatch(entries);
    EXPECT_TRUE(status == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    entries.clear();
    CreatEntrysData(10, '8', entries);

    /**
     * @tc.steps:step2. Put data by PutBatch
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver006
  * @tc.desc: register observer success and update callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver006, TestSize.Level1)
{
    Key key;
    Value value;
    key.push_back(1);
    value.push_back(1);

    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    value.clear();
    value.push_back(2);

    /**
     * @tc.steps:step2. Put data(k1,v2)
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver007
  * @tc.desc: register observer success and Delete callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver007, TestSize.Level1)
{
    Key key;
    Value value;
    key.push_back(1);
    value.push_back(1);
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Delete the k1,v1
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Delete(key) == OK);

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver008
  * @tc.desc: register observer success and Delete null value callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver008, TestSize.Level1)
{
    Key key;
    key.push_back(1);

    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Delete the k1, null value
     * @tc.expected: step2. Return OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Delete(key), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 0);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver009
  * @tc.desc: register observer success and DeleteBatch callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver009, TestSize.Level1)
{
    vector<Entry> entries;
    vector<Key> keys;
    for (int i = 1; i < 11; i++) {
        Entry entry;
        Key key;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
        keys.push_back(entry.key);
    }

    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Delete with no put
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->DeleteBatch(keys) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    LOGI("observer count:%lu", g_observer->GetCallCount());

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 0);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver010
  * @tc.desc: register observer success and DeleteBatch callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver010, TestSize.Level1)
{
    vector<Entry> entries;
    for (int i = 1; i < 6; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
    }

    DBStatus status = g_kvDelegatePtr->PutBatch(entries);
    EXPECT_TRUE(status == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    vector<Key> keys;
    for (int i = 1; i < 11; i++) {
        Key key;
        key.push_back(i);
        keys.push_back(key);
    }

    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Delete the keys and value normally
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->DeleteBatch(keys) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver011
  * @tc.desc: register observer success and DeleteBatch callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver011, TestSize.Level1)
{
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    vector<Key> keys;
    for (int i = 1; i < 11; i++) {
        Key key;
        key.push_back(i);
        keys.push_back(key);
    }

    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Delete with no put
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->DeleteBatch(keys) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 0);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver012
  * @tc.desc: register observer success and Clear callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver012, TestSize.Level1)
{
    vector<Entry> entries;
    for (int i = 1; i < 20; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
    }
    DBStatus status = g_kvDelegatePtr->PutBatch(entries);
    EXPECT_TRUE(status == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Clear data
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver013
  * @tc.desc: register observer success and Clear callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver013, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Clear data
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Do not print log normally.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotRegisterObserver014
  * @tc.desc: register observer success and operate callback
  * @tc.require: AR000BVDFQ AR000CQDVJ
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotRegisterObserver014, TestSize.Level1)
{
    Key key;
    Value value;
    key.push_back(1);
    value.push_back(1);
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    key.clear();
    value.clear();
    key.push_back(2);
    value.push_back(2);
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    key.clear();
    value.clear();
    key.push_back(1);
    value.push_back(4);

    /**
     * @tc.steps:step2. Put data(k1, v4)
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step3. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step3. Print log normally.
     */
    LOGI("observer count:%lu", g_observer->GetCallCount());
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    key.clear();
    value.clear();
    key.push_back(3);
    value.push_back(3);

    /**
     * @tc.steps:step4. Put data(k3, v3)
     * @tc.expected: step4. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step5. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step5. Print log normally.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 2);

    key.clear();
    key.push_back(2);

    /**
     * @tc.steps:step6. Delete(k2)
     * @tc.expected: step6. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Delete(key) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step7. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step7. Print log normally.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 3);

    key.clear();
    key.push_back(4);

    /**
     * @tc.steps:step8.Delete(k4)
     * @tc.expected: step8. Return OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Delete(key), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step9. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step9. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 3);

    /**
     * @tc.steps:step10. Clear data
     * @tc.expected: step10. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step11. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step11. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 4);

    /**
     * @tc.steps:step12. Clear data repeat
     * @tc.expected: step12. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step13. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step13. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 5);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: UnRegisterObserver001
  * @tc.desc: Unregister a normal observer
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, UnRegisterObserver001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->GetKvStoreSnapshot
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);

    /**
     * @tc.steps:step2. Test UnRegister Observer
     * @tc.expected: step2. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
}

/**
  * @tc.name: UnRegisterObserver002
  * @tc.desc: Unregister a null observer
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, UnRegisterObserver002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test UnRegister Observer with an null observer
     * @tc.expected: step1. Return INVALID_ARGS.
     */
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(nullptr) == INVALID_ARGS);
}

/**
  * @tc.name: UnRegisterObserver003
  * @tc.desc: Unregister a unregister observer
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, UnRegisterObserver003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test UnRegister Observer with observer do not have been registered
     * @tc.expected: step1. Return NOT_FOUND.
     */
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == NOT_FOUND);
}

/**
  * @tc.name: UnRegisterObserver004
  * @tc.desc: Unregister a and check callback
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, UnRegisterObserver004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Register Observer
     * @tc.expected: step1. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->RegisterObserver(g_observer) == OK);
    Key key;
    Value value;
    key.push_back(1);
    value.push_back(1);

    /**
     * @tc.steps:step2. Put data(k1,v1), Check the result of KvStoreObserver.OnChange
     * @tc.expected: step2. Return OK. Print log normally.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step3. Unregister Observer
     * @tc.expected: step3. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->UnRegisterObserver(g_observer) == OK);
    key.clear();
    value.clear();
    key.push_back(2);
    value.push_back(2);

    /**
     * @tc.steps:step4. Put data(k2,v2)
     * @tc.expected: step4. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);

    /**
     * @tc.steps:step5. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step5. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    vector<Entry> entries;
    vector<Key> keys;
    for (int i = 11; i < 21; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
        keys.push_back(entry.key);
    }

    /**
     * @tc.steps:step6. PutBatch 10 data
     * @tc.expected: step6. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);

    /**
     * @tc.steps:step7. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step7. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    key.clear();
    value.clear();
    key.push_back(1);
    value.push_back(2);

    /**
     * @tc.steps:step8. Put data(k1, v2)
     * @tc.expected: step8. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);

    /**
     * @tc.steps:step9. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step9. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step10. Delete data(k1)
     * @tc.expected: step10. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Delete(key) == OK);

    /**
     * @tc.steps:step11. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step11. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step12. Delete data(k1)
     * @tc.expected: step12. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Delete(key) == OK);

    /**
     * @tc.steps:step11. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step11. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step12. DeleteBatch data
     * @tc.expected: step12. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->DeleteBatch(keys) == OK);

    /**
     * @tc.steps:step13. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step13. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step14. Clear all data
     * @tc.expected: step14. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);

    /**
     * @tc.steps:step15. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step15. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step16. Clear all data repeat
     * @tc.expected: step16. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);

    /**
     * @tc.steps:step17. Check the result of KvStoreObserver.OnChange
     * @tc.expected: step17. Do not print log.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
}

/**
  * @tc.name: SnapshotUnRegisterObserver001
  * @tc.desc: Unregister a snapshot observer
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotUnRegisterObserver001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get KvStore Snapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotUnRegisterObserver002
  * @tc.desc: Unregister a null snaphot observer
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotUnRegisterObserver002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get KvStore Snapshot with snapshot is null
     * @tc.expected: step1. Return ERROR.
     */
    KvStoreSnapshotDelegate *snapshot = nullptr;
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshot) == DB_ERROR);
    g_snapshotDelegatePtr = nullptr;
}

/**
  * @tc.name: SnapshotUnRegisterObserver003
  * @tc.desc: Unregister a unregister snaphot observer
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotUnRegisterObserver003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get KvStore Snapshot with snapshot is not registered
     * @tc.expected: step1. Return OK.
     */
    KvStoreSnapshotDelegate *snapshot = new (std::nothrow) KvStoreSnapshotDelegateImpl(nullptr, nullptr);
    ASSERT_TRUE(snapshot != nullptr);
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshot) == OK);
    g_snapshotDelegatePtr = nullptr;
}

static void SnapshotUnRegisterObserver004Inner()
{
    /**
     * @tc.steps:step1. Get KvStore Snapshot
     * @tc.expected: step1. Return OK.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    Key key;
    Value value;
    key.push_back(1); // random key
    value.push_back(1); // random value

    /**
     * @tc.steps:step2. Put data(k1,v1), Check the result of KvStoreObserver.OnChange
     * @tc.expected: step2. Return OK. Print log normally.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step3. Release KvStore Snapshot
     * @tc.expected: step3. Return OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
    g_snapshotDelegatePtr = nullptr;
    key.clear();
    value.clear();
    key.push_back(2); // random key
    value.push_back(2); // random value

    /**
     * @tc.steps:step4/5. Put data(k2,v2), Check the result of KvStoreObserver.OnChange
     * @tc.expected: step4/5. Return OK. Do not print log.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
}

/**
  * @tc.name: SnapshotUnRegisterObserver004
  * @tc.desc: Check a unregister snaphot observer
  * @tc.require: AR000BVDFP AR000CQDVI
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, SnapshotUnRegisterObserver004, TestSize.Level1)
{
    SnapshotUnRegisterObserver004Inner();

    vector<Entry> entries;
    vector<Key> keys;
    for (int i = 11; i < 21; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
        keys.push_back(entry.key);
    }

    /**
     * @tc.steps:step6/7. PutBatch 10 data, Check the result of KvStoreObserver.OnChange
     * @tc.expected: step6/7. Return OK. Do not print log.
     */
    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    Key key;
    Value value;
    key.push_back(1);
    value.push_back(2);

    /**
     * @tc.steps:step8/9. Put data(k1,v2), Check the result of KvStoreObserver.OnChange
     * @tc.expected: step8/9. Return OK. Do not print log.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(key, value) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step10/11. Delete(k1), Check the result of KvStoreObserver.OnChange
     * @tc.expected: step10/11. Return OK. Do not print log.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Delete(key) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step12/13. Delete a not exist key, Check the result of KvStoreObserver.OnChange
     * @tc.expected: step12/13. Return OK. Do not print log.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Delete(key) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step14/15. DeleteBatch, Check the result of KvStoreObserver.OnChange
     * @tc.expected: step14/15. Return OK. Do not print log.
     */
    EXPECT_TRUE(g_kvDelegatePtr->DeleteBatch(keys) == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step14/15. Clear all data, Check the result of KvStoreObserver.OnChange
     * @tc.expected: step14/15. Return OK. Do not print log.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(g_observer->GetCallCount() == 1);

    /**
     * @tc.steps:step14/15. Clear all data repeat, Check the result of KvStoreObserver.OnChange
     * @tc.expected: step14/15. Return OK. Do not print log.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
}

static void CheckObserverCallback(const Entry &entryB)
{
    /**
     * @tc.steps: step3. Start a transaction. Insert [keyA, valueD], [keyC, valueC]
     *  through the Put interface of the delegate, and delete the keyB data
     *  through the Delete interface of the delegate. Ending a transaction
     * @tc.expected: step3. Obtain [keyC, valueC]
     *  from the GetEntriesInserted of the callback data,
     *  obtain [keyA, valueD] from the GetEntriesUpdated,
     *  and obtain [keyB, valueB] through the GetEntriesDeleted.
     */
    EXPECT_EQ(g_kvDelegatePtr->StartTransaction(), OK);
    Entry entryC = GetEntry("key_C", "");
    DistributedDBToolsUnitTest::GetRandomKeyValue(entryC.value, 10 * 1024); // 30K
    Entry entryE = GetEntry("key_E", "");
    DistributedDBToolsUnitTest::GetRandomKeyValue(entryE.value, 200 * 1024); // 200K, over the slice threshold.
    Entry entryF = GetEntry("key_F", "");
    DistributedDBToolsUnitTest::GetRandomKeyValue(entryF.value, 100); // 100
    EXPECT_EQ(g_kvDelegatePtr->Put(entryE.key, entryE.value), OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(entryF.key, entryF.value), OK);
    EXPECT_EQ(g_kvDelegatePtr->Put(entryC.key, entryC.value), OK);
    Entry entryD = GetEntry("key_A", "");
    DistributedDBToolsUnitTest::GetRandomKeyValue(entryD.value, 100 * 1024); // 100k
    EXPECT_EQ(g_kvDelegatePtr->Put(entryD.key, entryD.value), OK);
    EXPECT_EQ(g_kvDelegatePtr->Delete(entryB.key), OK);
    EXPECT_EQ(g_kvDelegatePtr->Commit(), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME * 10));

    /**
     * @tc.steps: step4. Check whether the observer callback is triggered
     *  and check the data obtained from the survey.
     */
    EXPECT_TRUE(g_observer->GetCallCount() == 1);
    std::list<Entry> inserted = g_observer->GetEntriesInserted();
    std::list<Entry> updated = g_observer->GetEntriesUpdated();
    std::list<Entry> deleted = g_observer->GetEntriesDeleted();
    LOGI("insert size[%zu], updated size[%zu], deleted size[%zu]", inserted.size(), updated.size(), deleted.size());

    std::list<Entry> expectedInserted;
    std::list<Entry> expectedUpdated;
    std::list<Entry> expectedDeleted;
    expectedInserted.push_back(entryC);
    expectedInserted.push_back(entryE);
    expectedInserted.push_back(entryF);
    expectedUpdated.push_back(entryD);
    expectedDeleted.push_back(entryB);
    EXPECT_TRUE(TestEntryList(inserted, expectedInserted));
    EXPECT_TRUE(TestEntryList(updated, expectedUpdated));
    EXPECT_TRUE(TestEntryList(deleted, expectedDeleted));
}

/**
  * @tc.name: GetObserverData001
  * @tc.desc: Test whether the data change notification can obtain these changes
  *  when the database is added, deleted, or modified.
  * @tc.type: FUNC
  * @tc.require: AR000BVDFR AR000CQDVK
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, GetObserverData001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Insert the data of [keyA, valueA], [keyB, valueB] through the Put interface of the delegate.
     */
    Entry entryA = GetEntry("key_A", "value_A");
    EXPECT_EQ(g_kvDelegatePtr->Put(entryA.key, entryA.value), OK);
    Entry entryB = GetEntry("key_B", "value_B");
    EXPECT_EQ(g_kvDelegatePtr->Put(entryB.key, entryB.value), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps: step2. Register an observer through the RegisterObserver interface of the delegate.
     * @tc.expected: step2. Returns a non-empty snapshot.
     */
    EXPECT_EQ(g_kvDelegatePtr->RegisterObserver(g_observer), OK);

    CheckObserverCallback(entryB);

    EXPECT_EQ(g_kvDelegatePtr->UnRegisterObserver(g_observer), OK);
}

/**
  * @tc.name: GetSnapshotObserverData001
  * @tc.desc: Test whether a data notification is sent
  *  when the value of observer is not empty
  *  when a snapshot is obtained and the database data changes.
  * @tc.type: FUNC
  * @tc.require: AR000C06UT AR000CQDTG
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesRegisterSyncDBTest, GetSnapshotObserverData001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Insert the data of [keyA, valueA], [keyB, valueB] through the Put interface of the delegate.
     */
    Entry entryA = GetEntry("key_A", "value_A");
    EXPECT_EQ(g_kvDelegatePtr->Put(entryA.key, entryA.value), OK);
    Entry entryB = GetEntry("key_B", "value_B");
    EXPECT_EQ(g_kvDelegatePtr->Put(entryB.key, entryB.value), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps: step2. Obtain the snapshot object snapshotA
     *  through the GetKvStoreSnapshot interface of the delegate and transfer the non-null observer.
     * @tc.expected: step2. Returns a non-empty snapshot.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(g_observer, g_snapshotDelegateCallback);
    EXPECT_EQ(g_kvDelegateStatus, OK);

    CheckObserverCallback(entryB);

    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr), OK);
    g_snapshotDelegatePtr = nullptr;
}

