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

#include "distributeddb_tools_unit_test.h"
#include "distributeddb_data_generate_unit_test.h"
#include "vitural_communicator_aggregator.h"
#include "vitural_device.h"
#include "virtual_single_ver_sync_db_Interface.h"
#include "platform_specific.h"
#include "process_system_api_adapter_impl.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    const string STORE_ID = "kv_stroe_sync_check_test";
    const std::string DEVICE_B = "deviceB";
    const std::string DEVICE_C = "deviceC";

    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    KvStoreConfig g_config;
    DistributedDBToolsUnitTest g_tool;
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate* g_kvDelegatePtr = nullptr;
    VirtualCommunicatorAggregator* g_communicatorAggregator = nullptr;
    VituralDevice* g_deviceB = nullptr;
    VituralDevice* g_deviceC = nullptr;
    VirtualSingleVerSyncDBInterface *g_syncInterfaceB = nullptr;
    VirtualSingleVerSyncDBInterface *g_syncInterfaceC = nullptr;

    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));
#ifndef LOW_LEVEL_MEM_DEV
    const int KEY_LEN = 20; // 20 Bytes
    const int VALUE_LEN = 4 * 1024 * 1024; // 4MB
    const int ENTRY_NUM = 6; // 6 entries
#endif
}

class DistributedDBSingleVerP2PSyncCheckTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBSingleVerP2PSyncCheckTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Init datadir and Virtual Communicator.
     */
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);

    string dir = g_testDir + "/single_ver";
    DIR* dirTmp = opendir(dir.c_str());
    if (dirTmp == nullptr) {
        OS::MakeDBDirectory(dir);
    } else {
        closedir(dirTmp);
    }

    g_communicatorAggregator = new (std::nothrow) VirtualCommunicatorAggregator();
    ASSERT_TRUE(g_communicatorAggregator != nullptr);
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(g_communicatorAggregator);

    std::shared_ptr<ProcessSystemApiAdapterImpl> g_adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    EXPECT_TRUE(g_adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(g_adapter);
}

void DistributedDBSingleVerP2PSyncCheckTest::TearDownTestCase(void)
{
    /**
     * @tc.teardown: Release virtual Communicator and clear data dir.
     */
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
}

void DistributedDBSingleVerP2PSyncCheckTest::SetUp(void)
{
    /**
     * @tc.setup: create virtual device B and C, and get a KvStoreNbDelegate as deviceA
     */
    KvStoreNbDelegate::Option option;
    option.secOption.securityLabel = SecurityLabel::S3;
    option.secOption.securityFlag = SecurityFlag::SECE;
    g_mgr.GetKvStore(STORE_ID, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegateStatus == OK);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    g_deviceB = new (std::nothrow) VituralDevice(DEVICE_B);
    ASSERT_TRUE(g_deviceB != nullptr);
    g_syncInterfaceB = new (std::nothrow) VirtualSingleVerSyncDBInterface();
    ASSERT_TRUE(g_syncInterfaceB != nullptr);
    ASSERT_EQ(g_deviceB->Initialize(g_communicatorAggregator, g_syncInterfaceB), E_OK);

    g_deviceC = new (std::nothrow) VituralDevice(DEVICE_C);
    ASSERT_TRUE(g_deviceC != nullptr);
    g_syncInterfaceC = new (std::nothrow) VirtualSingleVerSyncDBInterface();
    ASSERT_TRUE(g_syncInterfaceC != nullptr);
    ASSERT_EQ(g_deviceC->Initialize(g_communicatorAggregator, g_syncInterfaceC), E_OK);
}

void DistributedDBSingleVerP2PSyncCheckTest::TearDown(void)
{
    /**
     * @tc.teardown: Release device A, B, C
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
}

/**
 * @tc.name: sec option check Sync 001
 * @tc.desc: if sec option not equal, forbid sync
 * @tc.type: FUNC
 * @tc.require: AR000EV1G6
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBSingleVerP2PSyncCheckTest, SecOptionCheck001, TestSize.Level1)
{
    DBStatus status = OK;
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());
    devices.push_back(g_deviceC->GetDeviceId());

    /**
     * @tc.steps: step1. deviceA put {k1, v1}
     */
    Key key = {'1'};
    Value value = {'1'};
    status = g_kvDelegatePtr->Put(key, value);
    ASSERT_TRUE(status == OK);

    ASSERT_TRUE(g_syncInterfaceB != nullptr);
    ASSERT_TRUE(g_syncInterfaceC != nullptr);
    SecurityOption secOption{SecurityLabel::S4, SecurityFlag::ECE};
    g_syncInterfaceB->SetSecurityOption(secOption);
    g_syncInterfaceC->SetSecurityOption(secOption);

    /**
     * @tc.steps: step2. deviceA call sync and wait
     * @tc.expected: step2. sync should return SECURITY_OPTION_CHECK_ERROR.
     */
    std::map<std::string, DBStatus> result;
    status = g_tool.SyncTest(g_kvDelegatePtr, devices, SYNC_MODE_PUSH_ONLY, result);
    ASSERT_TRUE(status == OK);

    ASSERT_TRUE(result.size() == devices.size());
    for (const auto &pair : result) {
        LOGD("dev %s, status %d", pair.first.c_str(), pair.second);
        EXPECT_TRUE(pair.second == SECURITY_OPTION_CHECK_ERROR);
    }
    VirtualDataItem item;
    g_deviceB->GetData(key, item);
    EXPECT_TRUE(item.value.empty());
    g_deviceC->GetData(key, item);
    EXPECT_TRUE(item.value.empty());
}

/**
 * @tc.name: sec option check Sync 002
 * @tc.desc: if sec option not equal, forbid sync
 * @tc.type: FUNC
 * @tc.require: AR000EV1G6
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBSingleVerP2PSyncCheckTest, SecOptionCheck002, TestSize.Level1)
{
    DBStatus status = OK;
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());
    devices.push_back(g_deviceC->GetDeviceId());

    /**
     * @tc.steps: step1. deviceA put {k1, v1}
     */
    Key key = {'1'};
    Value value = {'1'};
    status = g_kvDelegatePtr->Put(key, value);
    ASSERT_TRUE(status == OK);

    ASSERT_TRUE(g_syncInterfaceC != nullptr);
    SecurityOption secOption{SecurityLabel::S4, SecurityFlag::ECE};
    g_syncInterfaceC->SetSecurityOption(secOption);
    secOption.securityLabel = SecurityLabel::S3;
    secOption.securityFlag = SecurityFlag::SECE;
    g_syncInterfaceB->SetSecurityOption(secOption);

    /**
     * @tc.steps: step2. deviceA call sync and wait
     * @tc.expected: step2. sync should return SECURITY_OPTION_CHECK_ERROR.
     */
    std::map<std::string, DBStatus> result;
    status = g_tool.SyncTest(g_kvDelegatePtr, devices, SYNC_MODE_PUSH_ONLY, result);
    ASSERT_TRUE(status == OK);

    ASSERT_TRUE(result.size() == devices.size());
    for (const auto &pair : result) {
        LOGD("dev %s, status %d", pair.first.c_str(), pair.second);
        if (pair.first == DEVICE_B) {
            EXPECT_TRUE(pair.second == OK);
        } else {
            EXPECT_TRUE(pair.second == SECURITY_OPTION_CHECK_ERROR);
        }
    }
    VirtualDataItem item;
    g_deviceC->GetData(key, item);
    EXPECT_TRUE(item.value.empty());
    g_deviceB->GetData(key, item);
    EXPECT_TRUE(item.value == value);
}

#ifndef LOW_LEVEL_MEM_DEV
/**
 * @tc.name: BigDataSync001
 * @tc.desc: big data sync push mode.
 * @tc.type: FUNC
 * @tc.require: AR000F3OOU
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBSingleVerP2PSyncCheckTest, BigDataSync001, TestSize.Level1)
{
    DBStatus status = OK;
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());
    devices.push_back(g_deviceC->GetDeviceId());

    /**
     * @tc.steps: step1. deviceA put 6 bigData
     */
    std::vector<Entry> entries;
    std::vector<Key> keys;
    DistributedDBUnitTest::GenerateRecords(ENTRY_NUM, entries, keys, KEY_LEN, VALUE_LEN);
    for (const auto &entry : entries) {
        status = g_kvDelegatePtr->Put(entry.key, entry.value);
        ASSERT_TRUE(status == OK);
    }

    /**
     * @tc.steps: step2. deviceA call sync and wait
     * @tc.expected: step2. sync should return OK.
     */
    std::map<std::string, DBStatus> result;
    status = g_tool.SyncTest(g_kvDelegatePtr, devices, SYNC_MODE_PUSH_ONLY, result);
    ASSERT_TRUE(status == OK);

    /**
     * @tc.expected: step2. onComplete should be called, DeviceB,C have {k1,v1}
     */
    ASSERT_TRUE(result.size() == devices.size());
    for (const auto &pair : result) {
        LOGD("dev %s, status %d", pair.first.c_str(), pair.second);
        EXPECT_TRUE(pair.second == OK);
    }
    VirtualDataItem item;
    for (const auto &entry : entries) {
        item.value.clear();
        g_deviceB->GetData(entry.key, item);
        EXPECT_TRUE(item.value == entry.value);
        item.value.clear();
        g_deviceC->GetData(entry.key, item);
        EXPECT_TRUE(item.value == entry.value);
    }
}

/**
 * @tc.name: BigDataSync002
 * @tc.desc: big data sync pull mode.
 * @tc.type: FUNC
 * @tc.require: AR000F3OOU
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBSingleVerP2PSyncCheckTest, BigDataSync002, TestSize.Level1)
{
    DBStatus status = OK;
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());
    devices.push_back(g_deviceC->GetDeviceId());

    /**
     * @tc.steps: step1. deviceA deviceB put bigData
     */
    std::vector<Entry> entries;
    std::vector<Key> keys;
    DistributedDBUnitTest::GenerateRecords(ENTRY_NUM, entries, keys, KEY_LEN, VALUE_LEN);

    for (uint32_t i = 0; i < entries.size(); i++) {
        if (i % 2 == 0) {
            g_deviceB->PutData(entries[i].key, entries[i].value, 0, 0);
        } else {
            g_deviceC->PutData(entries[i].key, entries[i].value, 0, 0);
        }
    }

    /**
     * @tc.steps: step3. deviceA call pull sync
     * @tc.expected: step3. sync should return OK.
     */
    std::map<std::string, DBStatus> result;
    status = g_tool.SyncTest(g_kvDelegatePtr, devices, SYNC_MODE_PULL_ONLY, result);
    ASSERT_TRUE(status == OK);

    /**
     * @tc.expected: step3. onComplete should be called, DeviceA have all bigData
     */
    ASSERT_TRUE(result.size() == devices.size());
    for (const auto &pair : result) {
        LOGD("dev %s, status %d", pair.first.c_str(), pair.second);
        EXPECT_TRUE(pair.second == OK);
    }
    for (const auto &entry : entries) {
        Value value;
        EXPECT_EQ(g_kvDelegatePtr->Get(entry.key, value), OK);
        EXPECT_EQ(value, entry.value);
    }
}

/**
 * @tc.name: BigDataSync003
 * @tc.desc: big data sync pushAndPull mode.
 * @tc.type: FUNC
 * @tc.require: AR000F3OOV
 * @tc.author: wangchuanqing
 */
HWTEST_F(DistributedDBSingleVerP2PSyncCheckTest, BigDataSync003, TestSize.Level1)
{
    DBStatus status = OK;
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());
    devices.push_back(g_deviceC->GetDeviceId());

    /**
     * @tc.steps: step1. deviceA deviceB put bigData
     */
    std::vector<Entry> entries;
    std::vector<Key> keys;
    DistributedDBUnitTest::GenerateRecords(ENTRY_NUM, entries, keys, KEY_LEN, VALUE_LEN);

    for (uint32_t i = 0; i < entries.size(); i++) {
        if (i % 3 == 0) { // 0 3 for deivec B
            g_deviceB->PutData(entries[i].key, entries[i].value, 0, 0);
        } else if (i % 3 == 1) { // 1 4 for device C
            g_deviceC->PutData(entries[i].key, entries[i].value, 0, 0);
        } else { // 2 5 for device A
            status = g_kvDelegatePtr->Put(entries[i].key, entries[i].value);
            ASSERT_TRUE(status == OK);
        }
    }

    /**
     * @tc.steps: step3. deviceA call pushAndpull sync
     * @tc.expected: step3. sync should return OK.
     */
    std::map<std::string, DBStatus> result;
    status = g_tool.SyncTest(g_kvDelegatePtr, devices, SYNC_MODE_PUSH_PULL, result);
    ASSERT_TRUE(status == OK);

    /**
     * @tc.expected: step3. onComplete should be called, DeviceA have all bigData
     * deviceB and deviceC has deviceA data
     */
    ASSERT_TRUE(result.size() == devices.size());
    for (const auto &pair : result) {
        LOGD("dev %s, status %d", pair.first.c_str(), pair.second);
        EXPECT_TRUE(pair.second == OK);
    }

    VirtualDataItem item;
    for (uint32_t i = 0; i < entries.size(); i++) {
        Value value;
        EXPECT_EQ(g_kvDelegatePtr->Get(entries[i].key, value), OK);
        EXPECT_EQ(value, entries[i].value);

        if (i % 3 == 2) { // 2 5 8 11 14 for device A
        item.value.clear();
        g_deviceB->GetData(entries[i].key, item);
        EXPECT_TRUE(item.value == entries[i].value);
        item.value.clear();
        g_deviceC->GetData(entries[i].key, item);
        EXPECT_TRUE(item.value == entries[i].value);
        }
    }
}
#endif

/**
 * @tc.name: PushFinishedNotify 001
 * @tc.desc: Test remote device push finished notify function.
 * @tc.type: FUNC
 * @tc.require: AR000CQS3S
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBSingleVerP2PSyncCheckTest, PushFinishedNotify001, TestSize.Level1)
{
    std::vector<std::string> devices;
    devices.push_back(g_deviceB->GetDeviceId());

    /**
     * @tc.steps: step1. deviceA call SetRemotePushFinishedNotify
     * @tc.expected: step1. set should return OK.
     */
    int pushfinishedFlag = 0;
    DBStatus status = g_kvDelegatePtr->SetRemotePushFinishedNotify(
        [&pushfinishedFlag](const RemotePushNotifyInfo &info) {
            EXPECT_TRUE(info.deviceId == DEVICE_B);
            pushfinishedFlag = 1;
    });
    ASSERT_EQ(status, OK);

    /**
     * @tc.steps: step2. deviceB put k2, v2, and deviceA pull from deviceB
     * @tc.expected: step2. deviceA can not revice push finished notify
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_2, VALUE_2), OK);
    std::map<std::string, DBStatus> result;
    status = g_tool.SyncTest(g_kvDelegatePtr, devices, SYNC_MODE_PUSH_PULL, result);
    EXPECT_TRUE(status == OK);
    EXPECT_EQ(pushfinishedFlag, 0);
    pushfinishedFlag = 0;

    /**
     * @tc.steps: step3. deviceB put k3, v3, and deviceA push and pull to deviceB
     * @tc.expected: step3. deviceA can not revice push finished notify
     */
    EXPECT_EQ(g_kvDelegatePtr->Put(KEY_3, VALUE_3), OK);
    status = g_tool.SyncTest(g_kvDelegatePtr, devices, SYNC_MODE_PUSH_PULL, result);
    EXPECT_TRUE(status == OK);
    EXPECT_EQ(pushfinishedFlag, 0);
    pushfinishedFlag = 0;

    /**
     * @tc.steps: step4. deviceA call SetRemotePushFinishedNotify to reset notify
     * @tc.expected: step4. set should return OK.
     */
    status = g_kvDelegatePtr->SetRemotePushFinishedNotify([&pushfinishedFlag](const RemotePushNotifyInfo &info) {
        EXPECT_TRUE(info.deviceId == DEVICE_B);
        pushfinishedFlag = 2;
    });
    ASSERT_EQ(status, OK);

    /**
     * @tc.steps: step5. deviceA call SetRemotePushFinishedNotify set null to unregist
     * @tc.expected: step5. set should return OK.
     */
    status = g_kvDelegatePtr->SetRemotePushFinishedNotify(nullptr);
    ASSERT_EQ(status, OK);
}