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
#include "ikvdb_factory.h"
#include "default_factory.h"
#include "db_constant.h"
#include "db_errno.h"
#include "sqlite_single_ver_natural_store.h"
#include "db_common.h"
#include "log_print.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;

    SQLiteSingleVerNaturalStore *g_singleVerNaturaStore = nullptr;
    IKvDBConnection *g_singleVerNaturaStoreConnection = nullptr;
    bool g_createFactory = false;
    const int OBSERVER_SLEEP_TIME = 80;

    Key g_emptyKey;
    string g_keyStr1 = "key_1";
    string g_keyStr2 = "key_2";
    string g_keyStr3 = "key_3";
    string g_keyStr4 = "key_4";
    string g_keyStr5 = "key_5";
    string g_keyStr6 = "key_6";

    string g_valueStr1 = "value_1";
    string g_valueStr2 = "value_2";
    string g_valueStr3 = "value_3";
    string g_valueStr4 = "value_4";
    string g_valueStr5 = "value_5";
    string g_valueStr6 = "value_6";
    string g_oldValueStr3 = "old_value_3";
    string g_oldValueStr4 = "old_value_4";

    list<Entry> g_emptyEntries;
    Entry g_entry0;
    Entry g_entry1;
    Entry g_entry2;
    Entry g_entry3;
    Entry g_entry4;
    Entry g_entry5;
    Entry g_entry6;
    Entry g_oldEntry3;
    Entry g_oldEntry4;

    bool g_testFuncCalled = false;
    list<Entry> g_insertedEntries;
    list<Entry> g_updatedEntries;
    list<Entry> g_deletedEntries;

    Entry TransferStrToKyEntry(const string &key, const string &value)
    {
        Entry entry;
        entry.key.resize(key.size());
        entry.key.assign(key.begin(), key.end());
        entry.value.resize(value.size());
        entry.value.assign(value.begin(), value.end());
        return entry;
    }

    void TestFunc(const KvDBCommitNotifyData &data)
    {
        g_testFuncCalled = true;
        int errCode;
        g_insertedEntries = data.GetInsertedEntries(errCode);
        ASSERT_EQ(errCode, E_OK);
        g_updatedEntries = data.GetUpdatedEntries(errCode);
        ASSERT_EQ(errCode, E_OK);
        g_deletedEntries = data.GetDeletedEntries(errCode);
        ASSERT_EQ(errCode, E_OK);
        LOGI("Insert:%zu, update:%zu, delete:%zu", g_insertedEntries.size(), g_updatedEntries.size(),
            g_deletedEntries.size());
        return;
    }

    bool CompairEntryList(const list<Entry> &entryList1, const list<Entry> &entryList2)
    {
        bool result = true;
        EXPECT_EQ(entryList1.size(), entryList2.size());
        if (entryList1.size() != entryList2.size()) {
            return false;
        }
        for (const auto &entry1 : entryList1) {
            result = false;
            for (const auto &entry2 : entryList2) {
                if (entry1.key != entry2.key) {
                    continue;
                }
                if (entry1.value == entry2.value) {
                    result = true;
                    break;
                }
                cout << "entry1.key: ";
                for (const auto &character : entry1.key) {
                    cout << character;
                }
                cout << endl;
                cout << "entry2.key: ";
                for (const auto &character : entry2.key) {
                    cout << character;
                }
                cout << endl;
                cout << "entry1.value: ";
                for (const auto &character : entry1.value) {
                    cout << character;
                }
                cout << endl;
                cout << "entry2.value: ";
                for (const auto &character : entry2.value) {
                    cout << character;
                }
                cout << endl;
                break;
            }
        }
        return result;
    }

    void TestAndClearCallbackResult(bool isCallbackCalled, const list<Entry> &expectedInsertedEntries,
        const list<Entry> &expectedUpdatedEntries, const list<Entry> &expectedDeletedEntries)
    {
        EXPECT_EQ(g_testFuncCalled, isCallbackCalled);
        if (g_testFuncCalled) {
            EXPECT_EQ(CompairEntryList(g_insertedEntries, expectedInsertedEntries), true);
            EXPECT_EQ(CompairEntryList(g_updatedEntries, expectedUpdatedEntries), true);
            EXPECT_EQ(CompairEntryList(g_deletedEntries, expectedDeletedEntries), true);
        }
        // clear result
        g_testFuncCalled = false;
        g_insertedEntries.resize(0);
        g_updatedEntries.resize(0);
        g_deletedEntries.resize(0);
    }

    void PreDataforOperation(const Entry &entry, bool isLocalPutRegisted, bool isPutRegisted, list<Entry> &entries)
    {
        IOption opt;
        entries.push_back(entry);
        // test local insert
        opt.dataType = IOption::LOCAL_DATA;
        g_singleVerNaturaStoreConnection->Put(opt, entry.key, entry.value);
        std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
        TestAndClearCallbackResult(isLocalPutRegisted, entries, g_emptyEntries, g_emptyEntries);
        // test local update
        g_singleVerNaturaStoreConnection->Put(opt, entry.key, entry.value);
        std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
        TestAndClearCallbackResult(isLocalPutRegisted, g_emptyEntries, entries, g_emptyEntries);
        // test local delete
        g_singleVerNaturaStoreConnection->Delete(opt, entry.key);
        std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
        TestAndClearCallbackResult(isLocalPutRegisted, g_emptyEntries, g_emptyEntries, entries);

        // test insert
        opt.dataType = IOption::SYNC_DATA;
        g_singleVerNaturaStoreConnection->Put(opt, entry.key, entry.value);
        std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
        TestAndClearCallbackResult(isPutRegisted, entries, g_emptyEntries, g_emptyEntries);

        // test update
        g_singleVerNaturaStoreConnection->Put(opt, entry.key, entry.value);
        std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
        TestAndClearCallbackResult(isPutRegisted, g_emptyEntries, entries, g_emptyEntries);
        // test delete
        g_singleVerNaturaStoreConnection->Delete(opt, entry.key);
        std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
        TestAndClearCallbackResult(isPutRegisted, g_emptyEntries, g_emptyEntries, entries);
    }

    void TestForOperation(const Entry &entry, bool isLocalPutRegisted, bool isPutRegisted, bool isSyncRegisted)
    {
        list<Entry> entries;
        entries.push_back(entry);

        // test sync insert
        TimeStamp time;
        g_singleVerNaturaStore->GetMaxTimeStamp(time);

        DataItem dataItem;
        dataItem.key = entry.key;
        dataItem.value = entry.value;
        dataItem.timeStamp = time + 1;
        dataItem.writeTimeStamp = dataItem.timeStamp;
        dataItem.flag = 0;
        vector<DataItem> insertDataItems;
        insertDataItems.push_back(dataItem);
        int result = DistributedDBToolsUnitTest::PutSyncDataTest(g_singleVerNaturaStore, insertDataItems, "deviceB");
        std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
        ASSERT_EQ(result, E_OK);
        TestAndClearCallbackResult(isSyncRegisted, entries, g_emptyEntries, g_emptyEntries);
        // test sync update
        vector<DataItem> updateDataItems;
        dataItem.timeStamp++;
        dataItem.writeTimeStamp = dataItem.timeStamp;
        updateDataItems.push_back(dataItem);
        result = DistributedDBToolsUnitTest::PutSyncDataTest(g_singleVerNaturaStore, updateDataItems, "deviceB");

        std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
        ASSERT_EQ(result, E_OK);
        TestAndClearCallbackResult(isSyncRegisted, g_emptyEntries, entries, g_emptyEntries);
        // test sync delete
        vector<DataItem> deleteDataItems;
        DataItem dataItem1 = dataItem;
        dataItem1.timeStamp++;
        dataItem1.writeTimeStamp = dataItem1.timeStamp;
        dataItem1.flag = 1;
        DistributedDBToolsUnitTest::CalcHash(dataItem.key, dataItem1.key);
        deleteDataItems.push_back(dataItem1);
        result = DistributedDBToolsUnitTest::PutSyncDataTest(g_singleVerNaturaStore, deleteDataItems, "deviceB");

        std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
        ASSERT_EQ(result, E_OK);
        TestAndClearCallbackResult(isSyncRegisted, g_emptyEntries, g_emptyEntries, entries);
    }
}

class DistributedDBStorageRegisterObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageRegisterObserverTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    if (IKvDBFactory::GetCurrent() == nullptr) {
        IKvDBFactory *factory = new (std::nothrow) DefaultFactory();
        ASSERT_NE(factory, nullptr);
        if (factory == nullptr) {
            LOGE("failed to new DefaultFactory!");
            return;
        }
        IKvDBFactory::Register(factory);
        g_createFactory = true;
    }
    // prepare test entries
    g_entry1 = TransferStrToKyEntry(g_keyStr1, g_valueStr1);
    g_entry2 = TransferStrToKyEntry(g_keyStr2, g_valueStr2);
    g_entry3 = TransferStrToKyEntry(g_keyStr3, g_valueStr3);
    g_entry4 = TransferStrToKyEntry(g_keyStr4, g_valueStr4);
    g_entry5 = TransferStrToKyEntry(g_keyStr5, g_valueStr5);
    g_entry6 = TransferStrToKyEntry(g_keyStr6, g_valueStr6);
    g_oldEntry3 = TransferStrToKyEntry(g_keyStr3, g_oldValueStr3);
    g_oldEntry4 = TransferStrToKyEntry(g_keyStr4, g_oldValueStr4);
}

void DistributedDBStorageRegisterObserverTest::TearDownTestCase(void)
{
    if (g_createFactory) {
        if (IKvDBFactory::GetCurrent() != nullptr) {
            delete IKvDBFactory::GetCurrent();
            IKvDBFactory::Register(nullptr);
        }
    }
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBStorageRegisterObserverTest::SetUp(void)
{
    IKvDBFactory *factory = IKvDBFactory::GetCurrent();
    ASSERT_NE(factory, nullptr);
    if (factory == nullptr) {
        LOGE("failed to get DefaultFactory!");
        return;
    }

    g_singleVerNaturaStore = new (std::nothrow) SQLiteSingleVerNaturalStore();
    ASSERT_NE(g_singleVerNaturaStore, nullptr);
    if (g_singleVerNaturaStore == nullptr) {
        return;
    }

    KvDBProperties property;
    property.SetStringProp(KvDBProperties::DATA_DIR, g_testDir);
    property.SetStringProp(KvDBProperties::STORE_ID, "TestGeneralNB");
    property.SetStringProp(KvDBProperties::IDENTIFIER_DIR, "TestGeneralNB");
    property.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::SINGLE_VER_TYPE);
    int errCode = g_singleVerNaturaStore->Open(property);
    ASSERT_EQ(errCode, E_OK);
    if (errCode != E_OK) {
        g_singleVerNaturaStore = nullptr;
        return;
    }

    g_singleVerNaturaStoreConnection = g_singleVerNaturaStore->GetDBConnection(errCode);
    ASSERT_EQ(errCode, E_OK);
    ASSERT_NE(g_singleVerNaturaStoreConnection, nullptr);
}

void DistributedDBStorageRegisterObserverTest::TearDown(void)
{
    if (g_singleVerNaturaStoreConnection != nullptr) {
        g_singleVerNaturaStoreConnection->Close();
    }
    std::string identifierName;
    g_singleVerNaturaStore->DecObjRef(g_singleVerNaturaStore);
    identifierName = DBCommon::TransferStringToHex("TestGeneralNB");
    DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir + "/" + identifierName + "/" + DBConstant::SINGLE_SUB_DIR);
}

/**
  * @tc.name: RegisterObserver001
  * @tc.desc: Register a NULL pointer as an observer
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver001, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Register a null pointer to subscribe to the database.
     * Check whether the registration is successful.
     * @tc.expected: step1/2. Returns INVALID_ARGS.
     */
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_PUT_EVENT), g_entry1.key, nullptr, result);
    EXPECT_EQ(result, -E_INVALID_ARGS);
    EXPECT_EQ(handle, nullptr);

    /**
     * @tc.steps: step3/4. UnRegister a null pointer to subscribe to the database.
     * Check whether the unregistration is successful.
     * @tc.expected: step3/4. Returns INVALID_ARGS.
     */
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(nullptr);
    EXPECT_EQ(result, -E_INVALID_ARGS);
    return;
}

/**
  * @tc.name: RegisterObserver002
  * @tc.desc: Register an observer for the local database change of a specified key
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver002, TestSize.Level1)
{
    int result;
    /**
     * @tc.steps: step1/2. Register a null pointer to subscribe to the database.
     * Check whether the registration is successful.
     * @tc.expected: step1/2. Returns INVALID_ARGS.
     */
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT), g_entry1.key, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    EXPECT_NE(handle, nullptr);

    /**
     * @tc.steps: step3/4/5/6. Register an observer for the local database change of a specified key
     */
    TestForOperation(g_entry1, true, false, false);
    TestForOperation(g_entry2, false, false, false);

    /**
     * @tc.steps: step7/8. UnRegister the subscribe to the database.
     * Check whether the unregistration is successful.
     * @tc.expected: step7/8. Returns E_OK.
     */
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);

    /**
     * @tc.steps: step9. Repeat step3/5
     * @tc.expected: step9. No callback.
     */
    TestForOperation(g_entry1, false, false, false);
    TestForOperation(g_entry2, false, false, false);
    return;
}

/**
  * @tc.name: RegisterObserver003
  * @tc.desc: Register an observer for the local sync database change of a specified key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver003, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Register a null pointer to subscribe to the database.
     * Check whether the registration is successful.
     * @tc.expected: step1/2. Returns INVALID_ARGS.
     */
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_PUT_EVENT), g_entry1.key, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    EXPECT_NE(handle, nullptr);

    /**
     * @tc.steps: step3/4/5/6. Register an observer for the local sync database change of a specified key.
     */
    TestForOperation(g_entry1, false, true, false);
    TestForOperation(g_entry2, false, false, false);

    /**
     * @tc.steps: step7/8. UnRegister the subscribe to the database.
     * Check whether the unregistration is successful.
     * @tc.expected: step7/8. Returns E_OK.
     */
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);

    /**
     * @tc.steps: step9. Repeat step3/5
     * @tc.expected: step9. No callback.
     */
    TestForOperation(g_entry1, false, false, false);
    TestForOperation(g_entry2, false, false, false);
    return;
}

/**
  * @tc.name: RegisterObserver004
  * @tc.desc: Register an observer for the remote sync database change of a specified key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver004, TestSize.Level1)
{
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_SYNC_EVENT), g_entry1.key, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    EXPECT_NE(handle, nullptr);
    TestForOperation(g_entry1, false, false, true);
    list<Entry> entries1;
    PreDataforOperation(g_entry1, false, false, entries1);
    TestForOperation(g_entry2, false, false, false);
    list<Entry> entries2;
    PreDataforOperation(g_entry2, false, false, entries2);
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);
    TestForOperation(g_entry1, false, false, false);
    TestForOperation(g_entry2, false, false, false);
    return;
}

/**
  * @tc.name: RegisterObserver005
  * @tc.desc: Register an observer for the sync database change of a specified key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver005, TestSize.Level1)
{
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_PUT_EVENT) |
        static_cast<unsigned int>(SQLITE_GENERAL_NS_SYNC_EVENT), g_entry1.key, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    EXPECT_NE(handle, nullptr);
    TestForOperation(g_entry1, false, true, true);
    TestForOperation(g_entry2, false, false, false);
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);
    TestForOperation(g_entry1, false, false, false);
    TestForOperation(g_entry2, false, false, false);
    return;
}

/**
  * @tc.name: RegisterObserver006
  * @tc.desc: Register an observer for the local database change of any key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver006, TestSize.Level1)
{
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT), g_emptyKey, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    EXPECT_NE(handle, nullptr);
    TestForOperation(g_entry1, true, false, false);
    TestForOperation(g_entry2, true, false, false);
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);
    TestForOperation(g_entry1, false, false, false);
    TestForOperation(g_entry2, false, false, false);
    return;
}

/**
  * @tc.name: RegisterObserver007
  * @tc.desc: Register an observer for the local sync database change of any key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver007, TestSize.Level1)
{
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_PUT_EVENT), g_emptyKey, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    EXPECT_NE(handle, nullptr);
    TestForOperation(g_entry1, false, true, false);
    TestForOperation(g_entry2, false, true, false);
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);
    TestForOperation(g_entry1, false, false, false);
    TestForOperation(g_entry2, false, false, false);
    return;
}

/**
  * @tc.name: RegisterObserver008
  * @tc.desc: Register an observer for the remote sync database change of any key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver008, TestSize.Level1)
{
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_SYNC_EVENT), g_emptyKey, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    EXPECT_NE(handle, nullptr);
    TestForOperation(g_entry1, false, false, true);
    TestForOperation(g_entry2, false, false, true);
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);
    TestForOperation(g_entry1, false, false, false);
    TestForOperation(g_entry2, false, false, false);
    return;
}

/**
  * @tc.name: RegisterObserver009
  * @tc.desc: Register an observer for the sync database change of any key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver009, TestSize.Level1)
{
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_PUT_EVENT) |
        static_cast<unsigned int>(SQLITE_GENERAL_NS_SYNC_EVENT), g_emptyKey, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    EXPECT_NE(handle, nullptr);
    TestForOperation(g_entry1, false, true, true);
    TestForOperation(g_entry2, false, true, true);
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);
    TestForOperation(g_entry1, false, false, false);
    TestForOperation(g_entry2, false, false, false);
    return;
}

/**
  * @tc.name: RegisterObserver010
  * @tc.desc: Register an observer for the local sync database change and the local database change of a specified key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver010, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Register an observer for the local sync database change
     *  and the local database change of a specified key. Check register result.
     * @tc.expected: step1/2. Returns E_NOT_SUPPORT.
     */
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_PUT_EVENT) |
        static_cast<unsigned int>(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT), g_entry1.key, TestFunc, result);
    EXPECT_EQ(result, -E_NOT_SUPPORT);
    EXPECT_EQ(handle, nullptr);
    return;
}

/**
  * @tc.name: RegisterObserver011
  * @tc.desc: Register an observer for the remote sync database change and the local database change of a specified key
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver011, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Register an observer for the remote sync database change
     *  and the local database change of a specified key. Check register result.
     * @tc.expected: step1/2. Returns E_NOT_SUPPORT.
     */
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_SYNC_EVENT) |
        static_cast<unsigned int>(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT), g_entry1.key, TestFunc, result);
    EXPECT_EQ(result, -E_NOT_SUPPORT);
    EXPECT_EQ(handle, nullptr);
    return;
}

/**
  * @tc.name: RegisterObserver012
  * @tc.desc: Register an observer for the local sync database change and the local database change of any key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver012, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Register an observer for the local sync database change
     * and the local database change of any key. Check register result.
     * @tc.expected: step1/2. Returns E_NOT_SUPPORT.
     */
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_PUT_EVENT) |
        static_cast<unsigned int>(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT), g_emptyKey, TestFunc, result);
    EXPECT_EQ(result, -E_NOT_SUPPORT);
    EXPECT_EQ(handle, nullptr);
    return;
}

/**
  * @tc.name: RegisterObserver013
  * @tc.desc: Register an observer for the remote sync database change and the local database change of any key.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver013, TestSize.Level1)
{
    /**
     * @tc.steps: step1/2. Register an observer for the remote sync database change
     *  and the local database change of any key. Check register result.
     * @tc.expected: step1/2. Returns E_NOT_SUPPORT.
     */
    int result;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_SYNC_EVENT) |
        static_cast<unsigned int>(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT), g_emptyKey, TestFunc, result);
    EXPECT_EQ(result, -E_NOT_SUPPORT);
    EXPECT_EQ(handle, nullptr);
    return;
}

static void PreSyncDataForRegisterObserver014(TimeStamp time, vector<DataItem> &dataItems)
{
    // sync data
    DataItem dataItem = {g_entry1.key, g_entry1.value, .timeStamp = ++time, .flag = 1};
    dataItem.writeTimeStamp = dataItem.timeStamp;
    DistributedDBToolsUnitTest::CalcHash(g_entry1.key, dataItem.key);
    dataItems.push_back(dataItem);

    DistributedDBToolsUnitTest::CalcHash(g_entry2.key, dataItem.key);
    dataItem.value = g_entry2.value;
    dataItems.push_back(dataItem);

    dataItem.key = g_entry3.key;
    dataItem.value = g_entry3.value;
    dataItem.flag = 0;
    dataItems.push_back(dataItem);

    dataItem.key = g_entry4.key;
    dataItem.value = g_entry4.value;
    dataItems.push_back(dataItem);

    dataItem.key = g_entry5.key;
    dataItem.value = g_entry5.value;
    dataItems.push_back(dataItem);

    dataItem.key = g_entry6.key;
    dataItem.value = g_entry6.value;
    dataItems.push_back(dataItem);
}

/**
  * @tc.name: RegisterObserver014
  * @tc.desc: Sync multiple records to the sync database
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver014, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Write the per data records to the synchronization database by Put.
     */
    IOption opt = {.dataType = IOption::SYNC_DATA};
    g_singleVerNaturaStoreConnection->Put(opt, g_entry1.key, g_entry1.value);
    g_singleVerNaturaStoreConnection->Put(opt, g_entry2.key, g_entry2.value);
    g_singleVerNaturaStoreConnection->Put(opt, g_oldEntry3.key, g_oldEntry3.value);
    g_singleVerNaturaStoreConnection->Put(opt, g_oldEntry4.key, g_oldEntry4.value);
    // get max time
    TimeStamp time;
    g_singleVerNaturaStore->GetMaxTimeStamp(time);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps: step2. Register the observer to the sync database
     *  from the remote end without specifying the key.
     */
    int result = E_OK;
    KvDBObserverHandle* handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_SYNC_EVENT), g_emptyKey, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    EXPECT_NE(handle, nullptr);

    // sync data
    vector<DataItem> dataItems;
    PreSyncDataForRegisterObserver014(time, dataItems);

    /**
     * @tc.steps: step3. A batch write operation by PutSyncData.
     *  The key1 and key2 records are deleted, and the key3 and key4 records are recorded.
     */
    result = DistributedDBToolsUnitTest::PutSyncDataTest(g_singleVerNaturaStore, dataItems, "deviceB");

    ASSERT_EQ(result, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    // test result
    list<Entry> deletedEntries;
    deletedEntries.push_back(g_entry1);
    deletedEntries.push_back(g_entry2);
    list<Entry> updatedEntries;
    updatedEntries.push_back(g_entry3);
    updatedEntries.push_back(g_entry4);
    list<Entry> insertedEntries;
    insertedEntries.push_back(g_entry5);
    insertedEntries.push_back(g_entry6);

    /**
     * @tc.steps: step4. Callback is triggered, the Put and Delete data is obtained from the observer.
     * @tc.expected: step4. The data is consistent with the data to be written.
     */
    TestAndClearCallbackResult(true, insertedEntries, updatedEntries, deletedEntries);

    /**
     * @tc.steps: step3. unregister observer
     */
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);
    return;
}
/**
  * @tc.name: RegisterObserver015
  * @tc.desc: Sync multiple records to the sync database, and remove them.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOM
  * @tc.author: liujialei
  */
HWTEST_F(DistributedDBStorageRegisterObserverTest, RegisterObserver015, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Generate the random entry.
     */
    vector<DataItem> dataItems;
    static const unsigned long number = 2; // 2 entries
    for (unsigned long i = 0; i < number; i++) {
        DataItem item;
        DistributedDBToolsUnitTest::GetRandomKeyValue(item.key, DBConstant::MAX_KEY_SIZE);
        DistributedDBToolsUnitTest::GetRandomKeyValue(item.value, DBConstant::MAX_VALUE_SIZE);
        dataItems.push_back(std::move(item));
    }
    /**
     * @tc.steps: step2. Put the entries through the syncer interface.
     */
    int result = DistributedDBToolsUnitTest::PutSyncDataTest(g_singleVerNaturaStore, dataItems, "deviceB");
    dataItems.clear();
    dataItems.shrink_to_fit();
    ASSERT_EQ(result, E_OK);
    /**
     * @tc.steps: step3. Register the observer.
     */
    KvDBObserverHandle *handle = g_singleVerNaturaStoreConnection->RegisterObserver(
        static_cast<unsigned int>(SQLITE_GENERAL_NS_SYNC_EVENT), g_emptyKey, TestFunc, result);
    EXPECT_EQ(result, E_OK);
    ASSERT_NE(handle, nullptr);
    /**
     * @tc.steps: step4. Remove the data from "deviceB".
     * @tc.expected: step4. Return E_OK and the observer data has delete entries.
     */
    g_deletedEntries.clear();
    result = g_singleVerNaturaStore->RemoveDeviceData("deviceB", true);
    ASSERT_EQ(result, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME * number));
    ASSERT_NE(g_deletedEntries.empty(), true);
    /**
     * @tc.steps: step5. unregister observer
     */
    result = g_singleVerNaturaStoreConnection->UnRegisterObserver(handle);
    EXPECT_EQ(result, E_OK);
    return;
}
