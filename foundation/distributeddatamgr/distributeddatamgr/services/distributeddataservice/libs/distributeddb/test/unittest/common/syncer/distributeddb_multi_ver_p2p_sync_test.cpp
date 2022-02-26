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
#include "kv_store_delegate.h"
#include "vitural_communicator_aggregator.h"
#include "vitural_communicator.h"
#include "vitural_device.h"
#include "isyncer.h"
#include "virtual_multi_ver_sync_db_interface.h"
#include "time_sync.h"
#include "meta_data.h"
#include "kvdb_manager.h"
#include "kvdb_pragma.h"
#include "ikvdb_connection.h"
#include "sync_types.h"
#include "commit_history_sync.h"
#include "log_print.h"
#include "multi_ver_data_sync.h"
#include "platform_specific.h"
#include "db_common.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

#ifndef LOW_LEVEL_MEM_DEV
namespace {
    string g_testDir;
    const string STORE_ID = "kv_stroe_sync_test";
    const string STORE_ID_A = "kv_stroe_sync_test_a";
    const string STORE_ID_B = "kv_stroe_sync_test_b";
    const int WAIT_TIME_1 = 1000;
    const int WAIT_TIME_2 = 2000;
    const int WAIT_LONG_TIME = 10000;
    const int WAIT_LIMIT_TIME = 30000;
    const std::string DEVICE_B = "deviceB";
    const std::string DEVICE_C = "deviceC";
    const int LIMIT_KEY_SIZE = 1024;
    constexpr int BIG_VALUE_SIZE = 1024 + 1; // > 1K
    constexpr int LIMIT_VALUE_SIZE = 4 * 1024 * 1024; // 4M
    KvStoreDelegateManager g_mgr("sync_test", "sync_test");
    KvStoreConfig g_config;
    KvStoreDelegate::Option g_option;

    // define the g_kvDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreDelegate *g_kvDelegatePtr = nullptr;
    MultiVerNaturalStoreConnection *g_connectionA;
    MultiVerNaturalStoreConnection *g_connectionB;
    VirtualCommunicatorAggregator* g_communicatorAggregator = nullptr;
    VituralDevice* g_deviceB = nullptr;
    VituralDevice* g_deviceC = nullptr;

    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));

    MultiVerNaturalStoreConnection *GetConnection(const std::string &dir, const std::string &storeId, int errCode)
    {
        KvDBProperties prop;
        prop.SetStringProp(KvDBProperties::USER_ID, "sync_test");
        prop.SetStringProp(KvDBProperties::APP_ID, "sync_test");
        prop.SetStringProp(KvDBProperties::STORE_ID, storeId);
        std::string identifier = DBCommon::TransferHashString("sync_test-sync_test-" + storeId);

        prop.SetStringProp(KvDBProperties::IDENTIFIER_DATA, identifier);
        std::string identifierDir = DBCommon::TransferStringToHex(identifier);
        prop.SetStringProp(KvDBProperties::IDENTIFIER_DIR, identifierDir);
        prop.SetStringProp(KvDBProperties::DATA_DIR, dir);
        prop.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
        prop.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
        errCode = E_OK;
        auto conn = KvDBManager::GetDatabaseConnection(prop, errCode);
        if (errCode != E_OK) {
            LOGE("[DistributeddbMultiVerP2PSyncTes] db create failed path, err %d", errCode);
            return nullptr;
        }
        return static_cast<MultiVerNaturalStoreConnection *>(conn);
    }

    int GetDataFromConnection(IKvDBConnection *conn, const Key &key, Value &value)
    {
        IKvDBSnapshot *snapshot = nullptr;
        int errCode = conn->GetSnapshot(snapshot);
        if (errCode != E_OK) {
            return errCode;
        }
        errCode = snapshot->Get(key, value);
        conn->ReleaseSnapshot(snapshot);
        return errCode;
    }
}

class DistributedDBMultiVerP2PSyncTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBMultiVerP2PSyncTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Init datadir and Virtual Communicator.
     */
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    string dir = g_testDir + "/commitstore";
    g_config.dataDir = dir;
    DIR* dirTmp = opendir(dir.c_str());
    if (dirTmp == nullptr) {
        OS::MakeDBDirectory(dir);
    } else {
        closedir(dirTmp);
    }
    g_mgr.SetKvStoreConfig(g_config);
    g_communicatorAggregator = new (std::nothrow) VirtualCommunicatorAggregator();
    ASSERT_TRUE(g_communicatorAggregator != nullptr);
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(g_communicatorAggregator);
}

void DistributedDBMultiVerP2PSyncTest::TearDownTestCase(void)
{
    /**
     * @tc.teardown: Release virtual Communicator and clear data dir.
     */
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }

    RuntimeContext::GetInstance()->SetCommunicatorAggregator(nullptr);
    g_communicatorAggregator = nullptr;
}

void DistributedDBMultiVerP2PSyncTest::SetUp(void)
{
    /**
     * @tc.setup: create virtual device B and C
     */
    g_communicatorAggregator->Disable();
    g_deviceB = new (std::nothrow) VituralDevice(DEVICE_B);
    ASSERT_TRUE(g_deviceB != nullptr);
    VirtualMultiVerSyncDBInterface *syncInterfaceB = new (std::nothrow) VirtualMultiVerSyncDBInterface;
    ASSERT_TRUE(syncInterfaceB != nullptr);
    ASSERT_EQ(syncInterfaceB->Initialize(DEVICE_B), E_OK);
    ASSERT_EQ(g_deviceB->Initialize(g_communicatorAggregator, syncInterfaceB), E_OK);

    g_deviceC = new (std::nothrow) VituralDevice(DEVICE_C);
    ASSERT_TRUE(g_deviceC != nullptr);
    VirtualMultiVerSyncDBInterface *syncInterfaceC = new (std::nothrow) VirtualMultiVerSyncDBInterface;
    ASSERT_TRUE(syncInterfaceC != nullptr);
    ASSERT_EQ(syncInterfaceC->Initialize(DEVICE_C), E_OK);
    ASSERT_EQ(g_deviceC->Initialize(g_communicatorAggregator, syncInterfaceC), E_OK);
    g_communicatorAggregator->Enable();

    auto permissionCheckCallback = [] (const std::string &userId, const std::string &appId, const std::string &storeId,
                                const std::string &deviceId, uint8_t flag) -> bool {
                                return true;};
    EXPECT_EQ(g_mgr.SetPermissionCheckCallback(permissionCheckCallback), OK);
}

void DistributedDBMultiVerP2PSyncTest::TearDown(void)
{
    /**
     * @tc.teardown: Release device A, B, C, connectionA and connectionB
     */
    if (g_kvDelegatePtr != nullptr) {
        ASSERT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
        g_kvDelegatePtr = nullptr;
        DBStatus status = g_mgr.DeleteKvStore(STORE_ID);
        LOGD("delete kv store status %d", status);
        ASSERT_TRUE(status == OK);
    }
    if (g_deviceB != nullptr) {
        delete g_deviceB;
        g_deviceB = nullptr;
    }
    if (g_deviceC != nullptr) {
        delete g_deviceC;
        g_deviceC = nullptr;
    }
    if (g_connectionA != nullptr) {
        g_connectionA->Close();
        ASSERT_EQ(g_mgr.DeleteKvStore(STORE_ID_A), OK);
        g_connectionA = nullptr;
    }
    if (g_connectionB != nullptr) {
        g_connectionB->Close();
        ASSERT_EQ(g_mgr.DeleteKvStore(STORE_ID_B), OK);
        g_connectionB = nullptr;
    }
    PermissionCheckCallbackV2 nullCallback;
    EXPECT_EQ(g_mgr.SetPermissionCheckCallback(nullCallback), OK);
}

static DBStatus GetData(KvStoreDelegate *kvStore, const Key &key, Value &value)
{
    KvStoreSnapshotDelegate *snapshotTmp = nullptr;
    DBStatus statusTmp;
    kvStore->GetKvStoreSnapshot(nullptr,
        [&statusTmp, &snapshotTmp](DBStatus status, KvStoreSnapshotDelegate *snapshot) {
        statusTmp = status;
        snapshotTmp = snapshot;
        });
    if (statusTmp != E_OK) {
        return statusTmp;
    }
    snapshotTmp->Get(key, [&statusTmp, &value](DBStatus status, const Value &outValue) {
        statusTmp = status;
        value = outValue;
    });
    if (statusTmp == OK) {
        LOGD("[DistributeddbMultiVerP2PSyncTes] GetData key %c, value = %c", key[0], value[0]);
    }
    kvStore->ReleaseKvStoreSnapshot(snapshotTmp);
    return statusTmp;
}

/**
 * @tc.name: Transaction Sync 001
 * @tc.desc: Verify put transaction sync function.
 * @tc.type: FUNC
 * @tc.require: AR000BVRO4 AR000CQE0K
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, TransactionSync001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));

    /**
     * @tc.steps: step2. deviceB put {k1, v1}, {k2,v2} in a transaction
     */
    g_deviceB->StartTransaction();
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_2, DistributedDBUnitTest::VALUE_2), E_OK);
    g_deviceB->Commit();

    /**
     * @tc.steps: step3. deviceB online and wait for sync
     */
    g_deviceB->Online();
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    /**
     * @tc.steps: step4. deviceC put {k3, v3}, {k4,v4} in a transaction
     */
    g_deviceC->StartTransaction();
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_3, DistributedDBUnitTest::VALUE_3), E_OK);
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_4, DistributedDBUnitTest::VALUE_4), E_OK);
    g_deviceC->Commit();

    /**
     * @tc.steps: step5. deviceC online for sync
     */
    g_deviceC->Online();

    /**
     * @tc.steps: step6. deviceC offline
     */
    g_deviceC->Offline();
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    /**
     * @tc.expected: step6. deviceA have {k1, v1}, {k2, v2}, not have k3, k4
     */
    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_1, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_1);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_2, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_2);

    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_3, value), NOT_FOUND);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_4, value), NOT_FOUND);
}

/**
 * @tc.name: Transaction Sync 002
 * @tc.desc: Verify delete transaction sync function.
 * @tc.type: FUNC
 * @tc.require: AR000BVRO4 AR000CQE0K
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, TransactionSync002, TestSize.Level2)
{
    /**
     * @tc.steps: step1. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));

    /**
     * @tc.steps: step2. deviceB put {k1, v1}, {k2,v2} in a transaction
     */
    g_deviceB->StartTransaction();
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_2, DistributedDBUnitTest::VALUE_2), E_OK);
    g_deviceB->Commit();

    /**
     * @tc.steps: step3. deviceB online and wait for sync
     */
    g_deviceB->Online();
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    /**
     * @tc.steps: step4. deviceC put {k3, v3}, and delete k3 in a transaction
     */
    g_deviceC->StartTransaction();
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_3, DistributedDBUnitTest::VALUE_3), E_OK);
    ASSERT_EQ(g_deviceC->DeleteData(DistributedDBUnitTest::KEY_3), E_OK);
    g_deviceC->Commit();

    /**
     * @tc.steps: step5. deviceB online for sync
     */
    g_deviceC->Online();

    /**
     * @tc.steps: step6. deviceC offline
     */
    g_deviceC->Offline();
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    /**
     * @tc.expected: step6. deviceA have {k1, v1}, {k2, v2}, not have k3, k4
     */
    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_1, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_1);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_2, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_2);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_3, value), NOT_FOUND);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_3, value), NOT_FOUND);
}

/**
 * @tc.name: Transaction Sync 003
 * @tc.desc: Verify update transaction sync function.
 * @tc.type: FUNC
 * @tc.require: AR000BVRO4 AR000CQE0K
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, TransactionSync003, TestSize.Level2)
{
    /**
     * @tc.steps: step1. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));

    /**
     * @tc.steps: step2. deviceB put {k1, v1}, {k2,v2} in a transaction
     */
    g_deviceB->StartTransaction();
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_2, DistributedDBUnitTest::VALUE_2), E_OK);
    g_deviceB->Commit();

    /**
     * @tc.steps: step3. deviceB online and wait for sync
     */
    g_deviceB->Online();
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    /**
     * @tc.steps: step4. deviceC put {k3, v3}, and update {k3, v4} in a transaction
     */
    g_deviceC->StartTransaction();
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_3, DistributedDBUnitTest::VALUE_3), E_OK);
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_3, DistributedDBUnitTest::VALUE_4), E_OK);
    g_deviceC->Commit();

    /**
     * @tc.steps: step5. deviceB online for sync
     */
    g_deviceC->Online();

    /**
     * @tc.steps: step6. deviceC offline
     */
    g_deviceC->Offline();
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    /**
     * @tc.expected: step6. deviceA have {k1, v1}, {k2, v2}, not have k3, k4
     */
    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_1, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_1);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_2, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_2);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_3, value), NOT_FOUND);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_3, value), NOT_FOUND);
}

/**
 * @tc.name: Metadata 001
 * @tc.desc: Verify metadata add and update function
 * @tc.type: FUNC
 * @tc.require: AR000CQE0P AR000CQE0S
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, Metadata001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a metadata and use VirtualMultiVerSyncDBInterface to init
     * @tc.expected: step1. metadata init ok
     */
    Metadata metadata;
    VirtualMultiVerSyncDBInterface *syncInterface = new (std::nothrow) VirtualMultiVerSyncDBInterface;
    ASSERT_TRUE(syncInterface != nullptr);
    EXPECT_EQ(syncInterface->Initialize("metadata_test"), E_OK);
    EXPECT_EQ(metadata.Initialize(syncInterface), E_OK);

    /**
     * @tc.steps: step2. call SaveTimeOffset to write t1.
     * @tc.expected: step2. SaveTimeOffset return ok
     */
    const TimeOffset timeOffsetA = 1024;
    EXPECT_EQ(metadata.SaveTimeOffset(DEVICE_B, timeOffsetA), E_OK);
    TimeOffset timeOffsetB = 0;

    /**
     * @tc.steps: step3. call GetTimeOffset to read t2.
     * @tc.expected: step3. t1 == t2
     */
    metadata.GetTimeOffset(DEVICE_B, timeOffsetB);
    EXPECT_EQ(timeOffsetA, timeOffsetB);

    /**
     * @tc.steps: step4. call SaveTimeOffset to write t3. t3 != t1
     * @tc.expected: step4. SaveTimeOffset return ok
     */
    const TimeOffset timeOffsetC = 2048;
    EXPECT_EQ(metadata.SaveTimeOffset(DEVICE_B, timeOffsetC), E_OK);

    /**
     * @tc.steps: step5. call GetTimeOffset to read t2.
     * @tc.expected: step5. t4 == t3
     */
    TimeOffset timeOffsetD = 0;
    metadata.GetTimeOffset(DEVICE_B, timeOffsetD);
    EXPECT_EQ(timeOffsetC, timeOffsetD);
    syncInterface->DeleteDatabase();
    delete syncInterface;
    syncInterface = nullptr;
}

/**
 * @tc.name: Isolation Sync 001
 * @tc.desc: Verify add sync isolation between different kvstore.
 * @tc.type: FUNC
 * @tc.require: AR000BVDGP
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, IsolationSync001, TestSize.Level2)
{
    int errCode = 0;

    /**
     * @tc.steps: step1. Get connectionA, connectionB from different kvstore,
     *     connectionB not in g_communicatorAggregator
     */
    g_communicatorAggregator->Disable();
    g_connectionB = GetConnection(g_config.dataDir, STORE_ID_B, errCode);
    ASSERT_TRUE(g_connectionB != nullptr);
    g_communicatorAggregator->Enable();
    g_connectionA = GetConnection(g_config.dataDir, STORE_ID_A, errCode);
    ASSERT_TRUE(g_connectionA != nullptr);

    /**
     * @tc.steps: step2. deviceB put {k1, v1}
     */
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);

    /**
     * @tc.steps: step3. connectionA pull from deviceB
     * @tc.expected: step3. Pragma OK, connectionA have {k1, v1} , connectionB don't have k1.
     */
    PragmaSync pragmaData(devices, SYNC_MODE_PULL_ONLY, nullptr);
    ASSERT_TRUE(g_connectionA->Pragma(PRAGMA_SYNC_DEVICES, &pragmaData) > 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));
    Value value;
    ASSERT_EQ(GetDataFromConnection(g_connectionA, DistributedDBUnitTest::KEY_1, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_1);
    EXPECT_EQ(GetDataFromConnection(g_connectionB, DistributedDBUnitTest::KEY_1, value), -E_NOT_FOUND);
}

/**
 * @tc.name: Isolation Sync 002
 * @tc.desc: Verify update sync isolation between different kvstore.
 * @tc.type: FUNC
 * @tc.require: AR000BVDGP
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, IsolationSync002, TestSize.Level2)
{
    int errCode = 0;

    /**
     * @tc.steps: step1. Get connectionA, connectionB from different kvstore,
     *     connectionB not in g_communicatorAggregator
     */
    g_communicatorAggregator->Disable();
    g_connectionB = GetConnection(g_config.dataDir, STORE_ID_B, errCode);
    ASSERT_TRUE(g_connectionB != nullptr);
    g_communicatorAggregator->Enable();
    g_connectionA = GetConnection(g_config.dataDir, STORE_ID_A, errCode);
    ASSERT_TRUE(g_connectionA != nullptr);

    /**
     * @tc.steps: step2. deviceB put {k1, v1} and update {k1, v2}
     */
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_2), E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    /**
     * @tc.steps: step3. connectionA pull from deviceB
     * @tc.expected: step3. Pragma OK, connectionA have {k1, v2} , connectionB don't have k1.
     */
    PragmaSync pragmaData(devices, SYNC_MODE_PULL_ONLY, nullptr);
    ASSERT_TRUE(g_connectionA->Pragma(PRAGMA_SYNC_DEVICES, &pragmaData) > 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    Value value;
    EXPECT_EQ(GetDataFromConnection(g_connectionA, DistributedDBUnitTest::KEY_1, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_2);
    EXPECT_EQ(GetDataFromConnection(g_connectionB, DistributedDBUnitTest::KEY_1, value), -E_NOT_FOUND);
}

/**
 * @tc.name: Isolation Sync 003
 * @tc.desc: Verify delete sync isolation between different kvstore.
 * @tc.type: FUNC
 * @tc.require: AR000BVDGP
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, IsolationSync003, TestSize.Level2)
{
    int errCode = 0;

    /**
     * @tc.steps: step1. Get connectionA, connectionB from different kvstore,
     *     connectionB not in g_communicatorAggregator, connectionB put {k1,v1}
     */
    g_communicatorAggregator->Disable();
    g_connectionB = GetConnection(g_config.dataDir, STORE_ID_B, errCode);
    ASSERT_TRUE(g_connectionB != nullptr);
    IOption option;
    ASSERT_EQ(g_connectionB->Put(option, KEY_1, VALUE_1), E_OK);
    g_communicatorAggregator->Enable();
    g_connectionA = GetConnection(g_config.dataDir, STORE_ID_A, errCode);
    ASSERT_TRUE(g_connectionA != nullptr);

    /**
     * @tc.steps: step2. deviceB put {k1, v1} and delete k1
     */
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);
    ASSERT_EQ(g_deviceB->DeleteData(DistributedDBUnitTest::KEY_1), E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    /**
     * @tc.steps: step3. connectionA pull from deviceB
     * @tc.expected: step3. Pragma OK, connectionA don't have k1, connectionB have {k1.v1}
     */
    LOGD("[DistributeddbMultiVerP2PSyncTes] start sync");
    PragmaSync pragmaData(devices, SYNC_MODE_PULL_ONLY, nullptr);
    ASSERT_TRUE(g_connectionA->Pragma(PRAGMA_SYNC_DEVICES, &pragmaData) > 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));

    Value value;
    EXPECT_EQ(GetDataFromConnection(g_connectionA, DistributedDBUnitTest::KEY_1, value), -E_NOT_FOUND);
    EXPECT_EQ(GetDataFromConnection(g_connectionB, DistributedDBUnitTest::KEY_1, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_1);
}

static void SetTimeSyncPacketField(TimeSyncPacket &inPacket, TimeStamp sourceBegin, TimeStamp sourceEnd,
    TimeStamp targetBegin, TimeStamp targetEnd, SyncId theId)
{
    inPacket.SetSourceTimeBegin(sourceBegin);
    inPacket.SetSourceTimeEnd(sourceEnd);
    inPacket.SetTargetTimeBegin(targetBegin);
    inPacket.SetTargetTimeEnd(targetEnd);
}

static bool IsTimeSyncPacketEqual(const TimeSyncPacket &inPacketA, const TimeSyncPacket &inPacketB)
{
    bool equal = true;
    equal = inPacketA.GetSourceTimeBegin() == inPacketB.GetSourceTimeBegin() ? equal : false;
    equal = inPacketA.GetSourceTimeEnd() == inPacketB.GetSourceTimeEnd() ? equal : false;
    equal = inPacketA.GetTargetTimeBegin() == inPacketB.GetTargetTimeBegin() ? equal : false;
    equal = inPacketA.GetTargetTimeEnd() == inPacketB.GetTargetTimeEnd() ? equal : false;
    return equal;
}

/**
 * @tc.name: Timesync Packet 001
 * @tc.desc: Verify TimesyncPacket Serialization and DeSerialization
 * @tc.type: FUNC
 * @tc.require: AR000BVRNU AR000CQE0J
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, TimesyncPacket001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create TimeSyncPacket packetA aand packetB
     */
    TimeSyncPacket packetA;
    TimeSyncPacket packetB;
    SetTimeSyncPacketField(packetA, 1, 2, 3, 4, 5); // 1, 2, 3, 4, 5 is five field for time sync packet
    SetTimeSyncPacketField(packetB, 5, 4, 3, 2, 1); // 1, 2, 3, 4, 5 is five field for time sync packet
    Message oriMsgA;
    Message oriMsgB;
    oriMsgA.SetCopiedObject(packetA);
    oriMsgA.SetMessageId(TIME_SYNC_MESSAGE);
    oriMsgA.SetMessageType(TYPE_REQUEST);
    oriMsgB.SetCopiedObject(packetB);
    oriMsgB.SetMessageId(TIME_SYNC_MESSAGE);
    oriMsgB.SetMessageType(TYPE_RESPONSE);

    /**
     * @tc.steps: step2. Serialization packetA to bufferA
     */
    uint32_t lenA = TimeSync::CalculateLen(&oriMsgA);
    vector<uint8_t> bufferA;
    bufferA.resize(lenA);
    int ret = TimeSync::Serialization(bufferA.data(), lenA, &oriMsgA);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step3. Serialization packetB to bufferB
     */
    uint32_t lenB = TimeSync::CalculateLen(&oriMsgB);
    vector<uint8_t> bufferB;
    bufferB.resize(lenB);
    ret = TimeSync::Serialization(bufferB.data(), lenB, &oriMsgB);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step4. DeSerialization bufferA to outPktA
     * @tc.expected: step4. packetA == outPktA
     */
    Message outMsgA;
    outMsgA.SetMessageId(TIME_SYNC_MESSAGE);
    outMsgA.SetMessageType(TYPE_REQUEST);
    ret = TimeSync::DeSerialization(bufferA.data(), lenA, &outMsgA);
    ASSERT_EQ(ret, E_OK);
    const TimeSyncPacket *outPktA = outMsgA.GetObject<TimeSyncPacket>();
    ASSERT_NE(outPktA, nullptr);
    EXPECT_EQ(IsTimeSyncPacketEqual(packetA, *outPktA), true);

    /**
     * @tc.steps: step5. DeSerialization bufferA to outPktA
     * @tc.expected: step5. packetB == outPktB  outPktB != outPktA
     */
    Message outMsgB;
    outMsgB.SetMessageId(TIME_SYNC_MESSAGE);
    outMsgB.SetMessageType(TYPE_RESPONSE);
    ret = TimeSync::DeSerialization(bufferB.data(), lenB, &outMsgB);
    ASSERT_EQ(ret, E_OK);
    const TimeSyncPacket *outPktB = outMsgB.GetObject<TimeSyncPacket>();
    ASSERT_NE(outPktB, nullptr);
    EXPECT_EQ(IsTimeSyncPacketEqual(packetB, *outPktB), true);
    EXPECT_EQ(IsTimeSyncPacketEqual(*outPktA, *outPktB), false);
}

static MultiVerCommitNode MakeMultiVerCommitA()
{
    MultiVerCommitNode outCommit;
    outCommit.commitId = vector<uint8_t>(1, 11); // 1 is length, 11 is value
    outCommit.leftParent = vector<uint8_t>(2, 22); // 2 is length, 22 is value
    outCommit.rightParent = vector<uint8_t>(3, 33); // 3 is length, 33 is value
    outCommit.timestamp = 444; // 444 is value
    outCommit.version = 5555; // 5555 is value
    outCommit.isLocal = 66666; // 66666 is value
    outCommit.deviceInfo = "AAAAAA";
    return outCommit;
}

static MultiVerCommitNode MakeMultiVerCommitB()
{
    MultiVerCommitNode outCommit;
    outCommit.commitId = vector<uint8_t>(9, 99); // 9 is length, 99 is value
    outCommit.leftParent = vector<uint8_t>(8, 88); // 8 is length, 88 is value
    outCommit.rightParent = vector<uint8_t>(7, 77); // 7 is length, 77 is value
    outCommit.timestamp = 666; // 666 is value
    outCommit.version = 5555; // 5555 is value
    outCommit.isLocal = 44444; // 44444 is value
    outCommit.deviceInfo = "BBBBBB";
    return outCommit;
}

static MultiVerCommitNode MakeMultiVerCommitC()
{
    MultiVerCommitNode outCommit;
    outCommit.commitId = vector<uint8_t>(1, 99); // 1 is length, 99 is value
    outCommit.leftParent = vector<uint8_t>(2, 88); // 2 is length, 88 is value
    outCommit.rightParent = vector<uint8_t>(3, 77); // 3 is length, 77 is value
    outCommit.timestamp = 466; // 466 is value
    outCommit.version = 5555; // 5555 is value
    outCommit.isLocal = 66444; // 66444 is value
    outCommit.deviceInfo = "CCCCCC";
    return outCommit;
}

static bool IsMultiVerCommitEqual(const MultiVerCommitNode &inCommitA, const MultiVerCommitNode &inCommitB)
{
    bool equal = true;
    equal = inCommitA.commitId == inCommitB.commitId ? equal : false;
    equal = inCommitA.leftParent == inCommitB.leftParent ? equal : false;
    equal = inCommitA.rightParent == inCommitB.rightParent ? equal : false;
    equal = inCommitA.timestamp == inCommitB.timestamp ? equal : false;
    equal = inCommitA.version == inCommitB.version ? equal : false;
    equal = inCommitA.isLocal == inCommitB.isLocal ? equal : false;
    equal = inCommitA.deviceInfo == inCommitB.deviceInfo ? equal : false;
    return equal;
}

static void MakeCommitHistorySyncRequestPacketA(CommitHistorySyncRequestPacket &inPacket)
{
    std::map<std::string, MultiVerCommitNode> commitMap;
    commitMap[string("A")] = MakeMultiVerCommitA();
    commitMap[string("C")] = MakeMultiVerCommitC();
    inPacket.SetCommitMap(commitMap);
}

static void MakeCommitHistorySyncRequestPacketB(CommitHistorySyncRequestPacket &inPacket)
{
    std::map<std::string, MultiVerCommitNode> commitMap;
    commitMap[string("B")] = MakeMultiVerCommitB();
    commitMap[string("C")] = MakeMultiVerCommitC();
    commitMap[string("BB")] = MakeMultiVerCommitB();
    inPacket.SetCommitMap(commitMap);
}

static bool IsCommitHistorySyncRequestPacketEqual(const CommitHistorySyncRequestPacket &inPacketA,
    const CommitHistorySyncRequestPacket &inPacketB)
{
    std::map<std::string, MultiVerCommitNode> commitMapA;
    std::map<std::string, MultiVerCommitNode> commitMapB;
    inPacketA.GetCommitMap(commitMapA);
    inPacketB.GetCommitMap(commitMapB);
    for (auto &entry : commitMapA) {
        if (commitMapB.count(entry.first) == 0) {
            return false;
        }
        if (!IsMultiVerCommitEqual(entry.second, commitMapB[entry.first])) {
            return false;
        }
    }
    for (auto &entry : commitMapB) {
        if (commitMapA.count(entry.first) == 0) {
            return false;
        }
        if (!IsMultiVerCommitEqual(entry.second, commitMapA[entry.first])) {
            return false;
        }
    }
    return true;
}

/**
 * @tc.name: Commit History Sync Request Packet 001
 * @tc.desc: Verify CommitHistorySyncRequestPacket Serialization and DeSerialization
 * @tc.type: FUNC
 * @tc.require: AR000BVRNU AR000CQE0J
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, CommitHistorySyncRequestPacket001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create CommitHistorySyncRequestPacket packetA aand packetB
     */
    CommitHistorySyncRequestPacket packetA;
    CommitHistorySyncRequestPacket packetB;
    MakeCommitHistorySyncRequestPacketA(packetA);
    MakeCommitHistorySyncRequestPacketB(packetB);
    Message oriMsgA;
    Message oriMsgB;
    oriMsgA.SetCopiedObject(packetA);
    oriMsgA.SetMessageId(COMMIT_HISTORY_SYNC_MESSAGE);
    oriMsgA.SetMessageType(TYPE_REQUEST);
    oriMsgB.SetCopiedObject(packetB);
    oriMsgB.SetMessageId(COMMIT_HISTORY_SYNC_MESSAGE);
    oriMsgB.SetMessageType(TYPE_REQUEST);

    /**
     * @tc.steps: step2. Serialization packetA to bufferA
     */
    uint32_t lenA = CommitHistorySync::CalculateLen(&oriMsgA);
    vector<uint8_t> bufferA;
    bufferA.resize(lenA);
    int ret = CommitHistorySync::Serialization(bufferA.data(), lenA, &oriMsgA);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step3. Serialization packetB to bufferB
     */
    uint32_t lenB = CommitHistorySync::CalculateLen(&oriMsgB);
    vector<uint8_t> bufferB;
    bufferB.resize(lenB);
    ret = CommitHistorySync::Serialization(bufferB.data(), lenB, &oriMsgB);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step4. DeSerialization bufferA to outPktA
     * @tc.expected: step4. packetA == outPktA
     */
    Message outMsgA;
    outMsgA.SetMessageId(COMMIT_HISTORY_SYNC_MESSAGE);
    outMsgA.SetMessageType(TYPE_REQUEST);
    ret = CommitHistorySync::DeSerialization(bufferA.data(), lenA, &outMsgA);
    ASSERT_EQ(ret, E_OK);
    const CommitHistorySyncRequestPacket *outPktA = outMsgA.GetObject<CommitHistorySyncRequestPacket>();
    ASSERT_NE(outPktA, nullptr);
    EXPECT_EQ(IsCommitHistorySyncRequestPacketEqual(packetA, *outPktA), true);

    /**
     * @tc.steps: step5. DeSerialization bufferB to outPktB
     * @tc.expected: step5. packetB == outPktB, outPktB != outPktA
     */
    Message outMsgB;
    outMsgB.SetMessageId(COMMIT_HISTORY_SYNC_MESSAGE);
    outMsgB.SetMessageType(TYPE_REQUEST);
    ret = CommitHistorySync::DeSerialization(bufferB.data(), lenB, &outMsgB);
    ASSERT_EQ(ret, E_OK);
    const CommitHistorySyncRequestPacket *outPktB = outMsgB.GetObject<CommitHistorySyncRequestPacket>();
    ASSERT_NE(outPktB, nullptr);
    EXPECT_EQ(IsCommitHistorySyncRequestPacketEqual(packetB, *outPktB), true);
    EXPECT_EQ(IsCommitHistorySyncRequestPacketEqual(*outPktA, *outPktB), false);
}

static void MakeCommitHistorySyncAckPacketA(CommitHistorySyncAckPacket &inPacket)
{
    std::vector<MultiVerCommitNode> commitVec;
    commitVec.push_back(MakeMultiVerCommitA());
    commitVec.push_back(MakeMultiVerCommitC());
    inPacket.SetData(commitVec);
    inPacket.SetErrorCode(10086); // 10086 is errorcode
}

static void MakeCommitHistorySyncAckPacketB(CommitHistorySyncAckPacket &inPacket)
{
    std::vector<MultiVerCommitNode> commitVec;
    commitVec.push_back(MakeMultiVerCommitB());
    commitVec.push_back(MakeMultiVerCommitC());
    commitVec.push_back(MakeMultiVerCommitB());
    inPacket.SetData(commitVec);
    inPacket.SetErrorCode(10010); // 10010 is errorcode
}

static bool IsCommitHistorySyncAckPacketEqual(const CommitHistorySyncAckPacket &inPacketA,
    const CommitHistorySyncAckPacket &inPacketB)
{
    int errCodeA;
    int errCodeB;
    std::vector<MultiVerCommitNode> commitVecA;
    std::vector<MultiVerCommitNode> commitVecB;
    inPacketA.GetData(commitVecA);
    inPacketB.GetData(commitVecB);
    inPacketA.GetErrorCode(errCodeA);
    inPacketB.GetErrorCode(errCodeB);
    if (errCodeA != errCodeB) {
        return false;
    }
    if (commitVecA.size() != commitVecB.size()) {
        return false;
    }
    int count = 0;
    for (auto &entry : commitVecA) {
        if (!IsMultiVerCommitEqual(entry, commitVecB[count++])) {
            return false;
        }
    }
    return true;
}

/**
 * @tc.name: Commit History Sync Ack Packet 001
 * @tc.desc: Verify CommitHistorySyncAckPacket Serialization and DeSerialization
 * @tc.type: FUNC
 * @tc.require: AR000BVRNU AR000CQE0J
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, CommitHistorySyncAckPacket001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create CommitHistorySyncAckPacket packetA aand packetB
     */
    CommitHistorySyncAckPacket packetA;
    CommitHistorySyncAckPacket packetB;
    MakeCommitHistorySyncAckPacketA(packetA);
    MakeCommitHistorySyncAckPacketB(packetB);
    Message oriMsgA;
    Message oriMsgB;
    oriMsgA.SetCopiedObject(packetA);
    oriMsgA.SetMessageId(COMMIT_HISTORY_SYNC_MESSAGE);
    oriMsgA.SetMessageType(TYPE_RESPONSE);
    oriMsgB.SetCopiedObject(packetB);
    oriMsgB.SetMessageId(COMMIT_HISTORY_SYNC_MESSAGE);
    oriMsgB.SetMessageType(TYPE_RESPONSE);

    /**
     * @tc.steps: step2. Serialization packetA to bufferA
     */
    uint32_t lenA = CommitHistorySync::CalculateLen(&oriMsgA);
    vector<uint8_t> bufferA;
    bufferA.resize(lenA);
    int ret = CommitHistorySync::Serialization(bufferA.data(), lenA, &oriMsgA);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step3. Serialization packetB to bufferB
     */
    uint32_t lenB = CommitHistorySync::CalculateLen(&oriMsgB);
    vector<uint8_t> bufferB;
    bufferB.resize(lenB);
    ret = CommitHistorySync::Serialization(bufferB.data(), lenB, &oriMsgB);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step4. DeSerialization bufferA to outPktA
     * @tc.expected: step4. packetA == outPktA
     */
    Message outMsgA;
    outMsgA.SetMessageId(COMMIT_HISTORY_SYNC_MESSAGE);
    outMsgA.SetMessageType(TYPE_RESPONSE);
    ret = CommitHistorySync::DeSerialization(bufferA.data(), lenA, &outMsgA);
    ASSERT_EQ(ret, E_OK);
    const CommitHistorySyncAckPacket *outPktA = outMsgA.GetObject<CommitHistorySyncAckPacket>();
    ASSERT_NE(outPktA, nullptr);
    EXPECT_EQ(IsCommitHistorySyncAckPacketEqual(packetA, *outPktA), true);

    /**
     * @tc.steps: step5. DeSerialization bufferB to outPktB
     * @tc.expected: step5. packetB == outPktB, outPktB!= outPktA
     */
    Message outMsgB;
    outMsgB.SetMessageId(COMMIT_HISTORY_SYNC_MESSAGE);
    outMsgB.SetMessageType(TYPE_RESPONSE);
    ret = CommitHistorySync::DeSerialization(bufferB.data(), lenB, &outMsgB);
    ASSERT_EQ(ret, E_OK);
    const CommitHistorySyncAckPacket *outPktB = outMsgB.GetObject<CommitHistorySyncAckPacket>();
    ASSERT_NE(outPktB, nullptr);
    EXPECT_EQ(IsCommitHistorySyncAckPacketEqual(packetB, *outPktB), true);
    EXPECT_EQ(IsCommitHistorySyncAckPacketEqual(*outPktA, *outPktB), false);
}

static bool IsMultiVerRequestPacketEqual(const MultiVerRequestPacket &inPacketA,
    const MultiVerRequestPacket &inPacketB)
{
    MultiVerCommitNode commitA;
    MultiVerCommitNode commitB;
    inPacketA.GetCommit(commitA);
    inPacketB.GetCommit(commitB);
    return IsMultiVerCommitEqual(commitA, commitB);
}

/**
 * @tc.name: MultiVerValueObject Request Packet 001
 * @tc.desc: Verify MultiVerRequestPacket Serialization and DeSerialization
 * @tc.type: FUNC
 * @tc.require: AR000BVRNU AR000CQE0J
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, MultiVerRequestPacket001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create CommitHistorySyncAckPacket packetA aand packetB
     */
    MultiVerRequestPacket packetA;
    MultiVerRequestPacket packetB;
    MultiVerCommitNode commitA = MakeMultiVerCommitA();
    MultiVerCommitNode commitB = MakeMultiVerCommitB();
    packetA.SetCommit(commitA);
    packetB.SetCommit(commitB);
    Message oriMsgA;
    Message oriMsgB;
    oriMsgA.SetCopiedObject(packetA);
    oriMsgA.SetMessageId(MULTI_VER_DATA_SYNC_MESSAGE);
    oriMsgA.SetMessageType(TYPE_REQUEST);
    oriMsgB.SetCopiedObject(packetB);
    oriMsgB.SetMessageId(MULTI_VER_DATA_SYNC_MESSAGE);
    oriMsgB.SetMessageType(TYPE_REQUEST);

    /**
     * @tc.steps: step2. Serialization packetA to bufferA
     */
    uint32_t lenA = MultiVerDataSync::CalculateLen(&oriMsgA);
    vector<uint8_t> bufferA;
    bufferA.resize(lenA);
    int ret = MultiVerDataSync::Serialization(bufferA.data(), lenA, &oriMsgA);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step3. Serialization packetB to bufferB
     */
    uint32_t lenB = MultiVerDataSync::CalculateLen(&oriMsgB);
    vector<uint8_t> bufferB;
    bufferB.resize(lenB);
    ret = MultiVerDataSync::Serialization(bufferB.data(), lenB, &oriMsgB);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step4. DeSerialization bufferA to outPktA
     * @tc.expected: step4. packetA == outPktA
     */
    Message outMsgA;
    outMsgA.SetMessageId(MULTI_VER_DATA_SYNC_MESSAGE);
    outMsgA.SetMessageType(TYPE_REQUEST);
    ret = MultiVerDataSync::DeSerialization(bufferA.data(), lenA, &outMsgA);
    ASSERT_EQ(ret, E_OK);
    const MultiVerRequestPacket *outPktA = outMsgA.GetObject<MultiVerRequestPacket>();
    ASSERT_NE(outPktA, nullptr);
    EXPECT_EQ(IsMultiVerRequestPacketEqual(packetA, *outPktA), true);

    /**
     * @tc.steps: step5. DeSerialization bufferB to outPktB
     * @tc.expected: step5. packetB == outPktB, outPktB!= outPktA
     */
    Message outMsgB;
    outMsgB.SetMessageId(MULTI_VER_DATA_SYNC_MESSAGE);
    outMsgB.SetMessageType(TYPE_REQUEST);
    ret = MultiVerDataSync::DeSerialization(bufferB.data(), lenB, &outMsgB);
    ASSERT_EQ(ret, E_OK);
    const MultiVerRequestPacket *outPktB = outMsgB.GetObject<MultiVerRequestPacket>();
    ASSERT_NE(outPktB, nullptr);
    EXPECT_EQ(IsMultiVerRequestPacketEqual(packetB, *outPktB), true);
    EXPECT_EQ(IsMultiVerRequestPacketEqual(*outPktA, *outPktB), false);
}

static void MakeMultiVerAckPacketA(MultiVerAckPacket &inPacket)
{
    std::vector<std::vector<uint8_t>> entryVec;
    entryVec.push_back(vector<uint8_t>(111, 11)); // 111 is length, 11 is value
    entryVec.push_back(vector<uint8_t>(222, 22)); // 222 is length, 22 is value
    inPacket.SetData(entryVec);
    inPacket.SetErrorCode(333); // 333 is errorcode
}

static void MakeMultiVerAckPacketB(MultiVerAckPacket &inPacket)
{
    std::vector<std::vector<uint8_t>> entryVec;
    entryVec.push_back(vector<uint8_t>(999, 99)); // 999 is length, 99 is value
    entryVec.push_back(vector<uint8_t>(888, 88)); // 888 is length, 88 is value
    inPacket.SetData(entryVec);
    inPacket.SetErrorCode(777); // 777 is errorcode
}

static bool IsMultiVerAckPacketEqual(const MultiVerAckPacket &inPacketA, const MultiVerAckPacket &inPacketB)
{
    int errCodeA;
    int errCodeB;
    std::vector<std::vector<uint8_t>> entryVecA;
    std::vector<std::vector<uint8_t>> entryVecB;
    inPacketA.GetData(entryVecA);
    inPacketB.GetData(entryVecB);
    inPacketA.GetErrorCode(errCodeA);
    inPacketB.GetErrorCode(errCodeB);
    if (errCodeA != errCodeB) {
        return false;
    }
    if (entryVecA != entryVecB) {
        return false;
    }
    return true;
}

/**
 * @tc.name: MultiVerValueObject Ack Packet 001
 * @tc.desc: Verify MultiVerAckPacket Serialization and DeSerialization
 * @tc.type: FUNC
 * @tc.require: AR000BVRNU AR000CQE0J
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, MultiVerAckPacket001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create MultiVerAckPacket packetA aand packetB
     */
    MultiVerAckPacket packetA;
    MultiVerAckPacket packetB;
    MakeMultiVerAckPacketA(packetA);
    MakeMultiVerAckPacketB(packetB);
    Message oriMsgA;
    Message oriMsgB;
    oriMsgA.SetCopiedObject(packetA);
    oriMsgA.SetMessageId(MULTI_VER_DATA_SYNC_MESSAGE);
    oriMsgA.SetMessageType(TYPE_RESPONSE);
    oriMsgB.SetCopiedObject(packetB);
    oriMsgB.SetMessageId(MULTI_VER_DATA_SYNC_MESSAGE);
    oriMsgB.SetMessageType(TYPE_RESPONSE);

    /**
     * @tc.steps: step2. Serialization packetA to bufferA
     */
    uint32_t lenA = MultiVerDataSync::CalculateLen(&oriMsgA);
    vector<uint8_t> bufferA;
    bufferA.resize(lenA);
    int ret = MultiVerDataSync::Serialization(bufferA.data(), lenA, &oriMsgA);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step3. Serialization packetB to bufferB
     */
    uint32_t lenB = MultiVerDataSync::CalculateLen(&oriMsgB);
    vector<uint8_t> bufferB;
    bufferB.resize(lenB);
    ret = MultiVerDataSync::Serialization(bufferB.data(), lenB, &oriMsgB);
    ASSERT_EQ(ret, E_OK);

    /**
     * @tc.steps: step4. DeSerialization bufferA to outPktA
     * @tc.expected: step4. packetA == outPktA
     */
    Message outMsgA;
    outMsgA.SetMessageId(MULTI_VER_DATA_SYNC_MESSAGE);
    outMsgA.SetMessageType(TYPE_RESPONSE);
    ret = MultiVerDataSync::DeSerialization(bufferA.data(), lenA, &outMsgA);
    ASSERT_EQ(ret, E_OK);
    const MultiVerAckPacket *outPktA = outMsgA.GetObject<MultiVerAckPacket>();
    ASSERT_NE(outPktA, nullptr);
    EXPECT_EQ(IsMultiVerAckPacketEqual(packetA, *outPktA), true);

    /**
     * @tc.steps: step5. DeSerialization bufferB to outPktB
     * @tc.expected: step5. packetB == outPktB, outPktB!= outPktA
     */
    Message outMsgB;
    outMsgB.SetMessageId(MULTI_VER_DATA_SYNC_MESSAGE);
    outMsgB.SetMessageType(TYPE_RESPONSE);
    ret = MultiVerDataSync::DeSerialization(bufferB.data(), lenB, &outMsgB);
    ASSERT_EQ(ret, E_OK);
    const MultiVerAckPacket *outPktB = outMsgB.GetObject<MultiVerAckPacket>();
    ASSERT_NE(outPktB, nullptr);
    EXPECT_EQ(IsMultiVerAckPacketEqual(packetB, *outPktB), true);
    EXPECT_EQ(IsMultiVerAckPacketEqual(*outPktA, *outPktB), false);
}

/**
 * @tc.name: Simple Data Sync 001
 * @tc.desc: Verify normal simple data sync function.
 * @tc.type: FUNC
 * @tc.require: AR000BVDGR
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, SimpleDataSync001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));

    /**
     * @tc.steps: step2. deviceB put {k1, v1}
     */
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);

    /**
     * @tc.steps: step4. deviceB put {k2, v2}
     */
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_2, DistributedDBUnitTest::VALUE_2), E_OK);

    /**
     * @tc.steps: step5. enable communicator and set deviceB,C online
     */
    g_deviceB->Online();
    g_deviceC->Online();

    /**
     * @tc.steps: step6. wait for sync
     * @tc.expected: step6. deviceA has {k1, v2} {k2, v2}
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));
    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_1, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_1);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_2, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_2);
}

/**
 * @tc.name: Big Data Sync 001
 * @tc.desc: Verify normal big data sync function.
 * @tc.type: FUNC
 * @tc.require: AR000BVDGR
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, BigDataSync001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));

    /**
     * @tc.steps: step2. deviceB put {k1, v1}, v1 size 1k
     */
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, BIG_VALUE_SIZE); // 1k +1
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, value1), E_OK);

    /**
     * @tc.steps: step4. deviceC put {k2, v2}, v2 size 1k
     */
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, BIG_VALUE_SIZE); // 1k +1
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_2, value2), E_OK);

    /**
     * @tc.steps: step5. set deviceB,C online
     */
    g_deviceB->Online();
    g_deviceC->Online();

    /**
     * @tc.steps: step5. wait 2s for sync
     * @tc.expected: step5. deviceA has {k1, v2} {k2, v2}
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));
    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_1, value), E_OK);
    EXPECT_EQ(value, value1);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_2, value), E_OK);
    EXPECT_EQ(value, value2);
}

/**
 * @tc.name: Limit Data Sync 001
 * @tc.desc: Verify normal limit data sync function.
 * @tc.type: FUNC
 * @tc.require: AR000BVDGR
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, LimitDataSync001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));
    /**
     * @tc.steps: step2. deviceB put {k1, v1}, k1 size 1k, v1 size 4M
     */
    Key key1;
    Value value1;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1, LIMIT_KEY_SIZE);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value1, LIMIT_VALUE_SIZE);
    ASSERT_EQ(g_deviceB->PutData(key1, value1), E_OK);

    /**
     * @tc.steps: step3. deviceC put {k2, v2}, k2 size 1k, v2 size 4M
     */
    Key key2;
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key2, LIMIT_KEY_SIZE);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, LIMIT_VALUE_SIZE);
    ASSERT_EQ(g_deviceC->PutData(key2, value2), E_OK);

    /**
     * @tc.steps: step4. set deviceB,C online
     */
    g_deviceB->Online();
    g_deviceC->Online();

    /**
     * @tc.steps: step5. wait 30 for sync
     * @tc.expected: step5. deviceA has {k1, v2} {k2, v2}
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_LIMIT_TIME));
    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, key1, value), E_OK);
    EXPECT_EQ(value, value1);
    EXPECT_EQ(GetData(g_kvDelegatePtr, key2, value), E_OK);
    EXPECT_EQ(value, value2);
}

/**
 * @tc.name: Multi Record 001
 * @tc.desc: Verify normal multi record sync function.
 * @tc.type: FUNC
 * @tc.require: AR000BVDGR
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, MultiRecord001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));

    /**
     * @tc.steps: step2. deviceB put {k1, v1}
     */
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);

    /**
     * @tc.steps: step4. deviceB put {k1, v2} v2 > 1K
     */
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, BIG_VALUE_SIZE); // 1k +1
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, value2), E_OK);

    /**
     * @tc.steps: step4. deviceB put {k2, v3}
     */
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_2, DistributedDBUnitTest::VALUE_3), E_OK);

    /**
     * @tc.steps: step5. deviceB put {k3, v3} and delete k3
     */
    ASSERT_TRUE(g_deviceB->StartTransaction() == E_OK);
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_3, DistributedDBUnitTest::VALUE_3), E_OK);
    ASSERT_EQ(g_deviceB->DeleteData(DistributedDBUnitTest::KEY_3), E_OK);
    ASSERT_TRUE(g_deviceB->Commit() == E_OK);

    /**
     * @tc.steps: step6. deviceC put {k4, v4}
     */
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_4, DistributedDBUnitTest::VALUE_4), E_OK);

    /**
     * @tc.steps: step7. deviceB put {k4, v5} v2 > 1K
     */
    Value value5;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value5, BIG_VALUE_SIZE); // 1k +1
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_4, value5), E_OK);

    /**
     * @tc.steps: step8. deviceB put {k5, v6}
     */
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_5, DistributedDBUnitTest::VALUE_6), E_OK);

    /**
     * @tc.steps: step9. deviceB put {k6, v6} and delete k6
     */
    ASSERT_TRUE(g_deviceC->StartTransaction() == E_OK);
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_6, DistributedDBUnitTest::VALUE_6), E_OK);
    ASSERT_EQ(g_deviceC->DeleteData(DistributedDBUnitTest::KEY_6), E_OK);
    ASSERT_TRUE(g_deviceC->Commit() == E_OK);

    /**
     * @tc.steps: step10. set deviceB,C online
     */
    g_deviceB->Online();
    g_deviceC->Online();

    /**
     * @tc.steps: step11. wait 5s for sync
     * @tc.expected: step11. deviceA has {k1, v2}, {k2, v3}, {k4, v5}, {k5, v6}
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_LONG_TIME));
    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_1, value), E_OK);
    EXPECT_EQ(value, value2);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_2, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_3);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_4, value), E_OK);
    EXPECT_EQ(value, value5);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_5, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_6);
}

/**
 * @tc.name: Net Disconnect Sync 001
 * @tc.desc: Test exception sync when net disconnected.
 * @tc.type: FUNC
 * @tc.require: AR000BVDGR
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, NetDisconnectSync001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));

    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());
    devices.push_back(g_deviceC->GetDeviceId());

    ASSERT_TRUE(g_deviceB->StartTransaction() == E_OK);
    /**
     * @tc.steps: step2. deviceB put {k1, v1}
     */
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);

    /**
     * @tc.steps: step4. deviceB put {k1, v2} v2 > 1K
     */
    Value value2;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value2, 1024 + 1); // 1k +1
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, value2), E_OK);

    /**
     * @tc.steps: step4. deviceB put {k2, v3}
     */
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_2, DistributedDBUnitTest::VALUE_3), E_OK);

    /**
     * @tc.steps: step5. deviceB put {k3, v3} and delete k3
     */
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_3, DistributedDBUnitTest::VALUE_3), E_OK);
    ASSERT_EQ(g_deviceB->DeleteData(DistributedDBUnitTest::KEY_3), E_OK);
    ASSERT_TRUE(g_deviceB->Commit() == E_OK);

    /**
     * @tc.steps: step6. deviceB online and enable communicator
     */
    g_deviceB->Online();

    /**
     * @tc.steps: step7. disable communicator and wait 5s
     * @tc.expected: step7. deviceA has no key1, key2
     */
    g_communicatorAggregator->Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_LONG_TIME + WAIT_LONG_TIME));

    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_1, value), NOT_FOUND);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_2, value), NOT_FOUND);

    ASSERT_TRUE(g_deviceC->StartTransaction() == E_OK);
    /**
     * @tc.steps: step8. deviceC put {k4, v4}
     */
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_4, DistributedDBUnitTest::VALUE_4), E_OK);

    /**
     * @tc.steps: step9. deviceB put {k4, v5} v2 > 1K
     */
    Value value5;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value5, BIG_VALUE_SIZE); // 1k +1
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_4, value5), E_OK);

    /**
     * @tc.steps: step10. deviceB put {k5, v6}
     */
    ASSERT_TRUE(g_deviceC->PutData(DistributedDBUnitTest::KEY_5, DistributedDBUnitTest::VALUE_6) == E_OK);

    /**
     * @tc.steps: step11. deviceB put {k6, v6} and delete k6
     */
    ASSERT_TRUE(g_deviceC->PutData(DistributedDBUnitTest::KEY_6, DistributedDBUnitTest::VALUE_6) == E_OK);
    ASSERT_TRUE(g_deviceC->DeleteData(DistributedDBUnitTest::KEY_6) == E_OK);
    ASSERT_TRUE(g_deviceC->Commit() == E_OK);

    /**
     * @tc.steps: step12. deviceC online and enable communicator
     */
    g_communicatorAggregator->Enable();
    g_deviceC->Online();

    /**
     * @tc.steps: step13. wait 5s for sync
     * @tc.expected: step13. deviceA has {k4, v5}, {k5, v6}
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_LONG_TIME)); // wait 5s
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_4, value), E_OK);
    EXPECT_EQ(value, value5);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_5, value), E_OK);
    EXPECT_EQ(value, DistributedDBUnitTest::VALUE_6);
}

/**
  * @tc.name: SyncQueue006
  * @tc.desc: multi version not surport sync queue
  * @tc.type: FUNC
  * @tc.require: AR000D4876
  * @tc.author: wangchuanqing
  */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, SyncQueue006, TestSize.Level3)
{
    /**
     * @tc.steps:step1. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);

    /**
     * @tc.steps:step2. Set PragmaCmd to be GET_QUEUED_SYNC_SIZE
     * @tc.expected: step2. Expect return NOT_SUPPORT.
     */
    int param;
    PragmaData input = static_cast<PragmaData>(&param);
    EXPECT_EQ(g_kvDelegatePtr->Pragma(GET_QUEUED_SYNC_SIZE, input), NOT_SUPPORT);
    EXPECT_EQ(g_kvDelegatePtr->Pragma(SET_QUEUED_SYNC_LIMIT, input), NOT_SUPPORT);
    EXPECT_EQ(g_kvDelegatePtr->Pragma(GET_QUEUED_SYNC_LIMIT, input), NOT_SUPPORT);
}

/**
 * @tc.name: PermissionCheck001
 * @tc.desc: deviceA permission check not pass
 * @tc.type: FUNC
 * @tc.require: AR000D4876
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, PermissionCheck001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. SetPermissionCheckCallback
     * @tc.expected: step1. return OK.
     */
    auto permissionCheckCallback = [] (const std::string &userId, const std::string &appId, const std::string &storeId,
                                        const std::string &deviceId, uint8_t flag) -> bool {
                                        if (flag & CHECK_FLAG_RECEIVE) {
                                            LOGD("in RunPermissionCheck callback func, check not pass, flag:%d", flag);
                                            return false;
                                        } else {
                                            LOGD("in RunPermissionCheck callback func, check pass, flag:%d", flag);
                                            return true;
                                        }
                                        };
    EXPECT_EQ(g_mgr.SetPermissionCheckCallback(permissionCheckCallback), OK);

    /**
     * @tc.steps: step2. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));

    /**
     * @tc.steps: step3. deviceB put {k1, v1}
     */
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);

    /**
     * @tc.steps: step4. deviceC put {k2, v2}
     */
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_2, DistributedDBUnitTest::VALUE_2), E_OK);

    /**
     * @tc.steps: step5. enable communicator and set deviceB,C online
     */
    g_deviceB->Online();
    g_deviceC->Online();

    /**
     * @tc.steps: step6. wait for sync
     * @tc.expected: step6. deviceA do not has {k1, v2} {k2, v2}
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));
    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_1, value), NOT_FOUND);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_2, value), NOT_FOUND);
    PermissionCheckCallbackV2 nullCallback;
    EXPECT_EQ(g_mgr.SetPermissionCheckCallback(nullCallback), OK);
}

/**
 * @tc.name: PermissionCheck002
 * @tc.desc: deviceB deviceC permission check not pass
 * @tc.type: FUNC
 * @tc.require: AR000D4876
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBMultiVerP2PSyncTest, PermissionCheck002, TestSize.Level2)
{
    /**
     * @tc.steps: step1. SetPermissionCheckCallback
     * @tc.expected: step1. return OK.
     */
    auto permissionCheckCallback = [] (const std::string &userId, const std::string &appId, const std::string &storeId,
                                        const std::string &deviceId, uint8_t flag) -> bool {
                                        if (flag & CHECK_FLAG_SEND) {
                                            LOGD("in RunPermissionCheck callback func, check not pass, flag:%d", flag);
                                            return false;
                                        } else {
                                            LOGD("in RunPermissionCheck callback func, check pass, flag:%d", flag);
                                            return true;
                                        }
                                        };
    EXPECT_EQ(g_mgr.SetPermissionCheckCallback(permissionCheckCallback), OK);

    /**
     * @tc.steps: step2. open a KvStoreNbDelegate as deviceA
     */
    g_mgr.GetKvStore(STORE_ID, g_option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_1));

    /**
     * @tc.steps: step3. deviceB put {k1, v1}
     */
    ASSERT_EQ(g_deviceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1), E_OK);

    /**
     * @tc.steps: step4. deviceC put {k2, v2}
     */
    ASSERT_EQ(g_deviceC->PutData(DistributedDBUnitTest::KEY_2, DistributedDBUnitTest::VALUE_2), E_OK);

    /**
     * @tc.steps: step5. enable communicator and set deviceB,C online
     */
    g_deviceB->Online();
    g_deviceC->Online();

    /**
     * @tc.steps: step6. wait for sync
     * @tc.expected: step6. deviceA do not has {k1, v2} {k2, v2}
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_2));
    Value value;
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_1, value), NOT_FOUND);
    EXPECT_EQ(GetData(g_kvDelegatePtr, DistributedDBUnitTest::KEY_2, value), NOT_FOUND);
    PermissionCheckCallbackV2 nullCallback;
    EXPECT_EQ(g_mgr.SetPermissionCheckCallback(nullCallback), OK);
}
#endif