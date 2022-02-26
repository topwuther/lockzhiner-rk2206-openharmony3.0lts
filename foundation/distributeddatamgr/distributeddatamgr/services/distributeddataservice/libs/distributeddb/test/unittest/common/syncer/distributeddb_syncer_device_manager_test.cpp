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

#include "vitural_communicator_aggregator.h"
#include "vitural_communicator.h"
#include "vitural_device.h"
#include "device_manager.h"
#include "virtual_single_ver_sync_db_Interface.h"
#include "log_print.h"
#include "parcel.h"
#include "sync_types.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace std;

namespace {
    const std::string DEVICE_B = "deviceB";
    const std::string DEVICE_C = "deviceC";
    VirtualCommunicatorAggregator* g_communicatorAggregator = nullptr;
    VirtualCommunicator* g_virtualCommunicator = nullptr;
    VituralDevice* g_deviceB = nullptr;
    VituralDevice* g_deviceC = nullptr;
    DeviceManager *g_deviceManager = nullptr;
    const int WAIT_TIME = 1000;
}

class DistributedDBSyncerDeviceManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBSyncerDeviceManagerTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Virtual Communicator.
     */
    g_communicatorAggregator = new (std::nothrow) VirtualCommunicatorAggregator();
    ASSERT_TRUE(g_communicatorAggregator != nullptr);
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(g_communicatorAggregator);
    int errCode;
    g_virtualCommunicator = static_cast<VirtualCommunicator*>(g_communicatorAggregator->AllocCommunicator(0, errCode));
    ASSERT_TRUE(g_virtualCommunicator != nullptr);
}

void DistributedDBSyncerDeviceManagerTest::TearDownTestCase(void)
{
    /**
     * @tc.setup: Release Virtual CommunicatorAggregator.
     */
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(nullptr);
    g_communicatorAggregator = nullptr;
}

void DistributedDBSyncerDeviceManagerTest::SetUp(void)
{
    /**
     * @tc.setup: Init a DeviceManager and DeviceB, C
     */
    g_deviceManager = new (std::nothrow) DeviceManager;
    ASSERT_TRUE(g_deviceManager != nullptr);
    g_deviceManager->Initialize(g_virtualCommunicator, nullptr);
    g_virtualCommunicator->RegOnConnectCallback(
        std::bind(&DeviceManager::OnDeviceConnectCallback, g_deviceManager,
            std::placeholders::_1, std::placeholders::_2), nullptr);

    g_deviceB = new (std::nothrow) VituralDevice(DEVICE_B);
    ASSERT_TRUE(g_deviceB != nullptr);
    VirtualSingleVerSyncDBInterface *syncInterfaceB = new (std::nothrow) VirtualSingleVerSyncDBInterface();
    ASSERT_TRUE(syncInterfaceB != nullptr);
    ASSERT_EQ(g_deviceB->Initialize(g_communicatorAggregator, syncInterfaceB), E_OK);

    g_deviceC = new (std::nothrow) VituralDevice(DEVICE_C);
    ASSERT_TRUE(g_deviceC != nullptr);
    VirtualSingleVerSyncDBInterface *syncInterfaceC = new (std::nothrow) VirtualSingleVerSyncDBInterface();
    ASSERT_TRUE(syncInterfaceC != nullptr);
    ASSERT_EQ(g_deviceC->Initialize(g_communicatorAggregator, syncInterfaceC), E_OK);
}

void DistributedDBSyncerDeviceManagerTest::TearDown(void)
{
    /**
     * @tc.setup: Release a DeviceManager and DeviceB, C
     */
    if (g_deviceManager != nullptr) {
        g_virtualCommunicator->RegOnConnectCallback(nullptr, nullptr);
        delete g_deviceManager;
        g_deviceManager = nullptr;
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
 * @tc.name: Online Callback 001
 * @tc.desc: Test DeviceManager device online callback function.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTD AR000CQE0E
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBSyncerDeviceManagerTest, OnlineCallback001, TestSize.Level0)
{
    bool onlineCalled = false;

    /**
     * @tc.steps: step1. set device online callback
     */
    g_deviceManager->RegDeviceOnLineCallBack([&onlineCalled](const std::string &targetDev) {
        LOGD("DeviceManageTest online called, dev %s", targetDev.c_str());
        if (targetDev == g_deviceB->GetDeviceId()) {
            LOGD("DEVICE TEST CALL ONLINE CALLBACK");
            onlineCalled = true;
        }
    });

    /**
     * @tc.steps: step2. deviceB online
     * @tc.expected: step2, the online callback should be called.
     */
    g_communicatorAggregator->OnlineDevice(g_deviceB->GetDeviceId());
    EXPECT_TRUE(onlineCalled);
}

/**
 * @tc.name: Offline Callback 001
 * @tc.desc: Test DeviceManager device offline callback function.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTD AR000CQE0E
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBSyncerDeviceManagerTest, OfflineCallback001, TestSize.Level0)
{
    bool offlineCalled = false;
    g_communicatorAggregator->OnlineDevice(g_deviceB->GetDeviceId());

    /**
     * @tc.steps: step1. set device offline callback
     */
    g_deviceManager->RegDeviceOffLineCallBack([&offlineCalled](const std::string &targetDev) {
        LOGD("DeviceManageTest offline called, dev %s", targetDev.c_str());
        if (targetDev == g_deviceB->GetDeviceId()) {
            offlineCalled = true;
        }
    });

    /**
     * @tc.steps: step2. deviceB offline
     * @tc.expected: step2, the offline callback should be called.
     */
    g_communicatorAggregator->OfflineDevice(g_deviceB->GetDeviceId());
    EXPECT_TRUE(offlineCalled);
}

/**
 * @tc.name: Get Devices 001
 * @tc.desc: Test DeviceManager GetDevices function.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTD AR000CQE0E
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBSyncerDeviceManagerTest, GetDevices001, TestSize.Level0)
{
    std::vector<std::string> deviceList;

    /**
     * @tc.steps: step1. call GetDevices
     * @tc.expected: step1, GetDevices return deviceB,C
     */
    g_deviceManager->GetOnlineDevices(deviceList);
    int size = deviceList.size();
    ASSERT_EQ(size, 2);
    EXPECT_TRUE(deviceList[0] == g_deviceB->GetDeviceId());
    EXPECT_TRUE(deviceList[1] == g_deviceC->GetDeviceId());
    g_communicatorAggregator->OfflineDevice(g_deviceC->GetDeviceId());

    /**
     * @tc.steps: step2. deiceC offline and call GetDevices
     * @tc.expected: step2, GetDevices return deviceB
     */
    g_deviceManager->GetOnlineDevices(deviceList);
    ASSERT_TRUE(deviceList.size() == 1);
    EXPECT_TRUE(deviceList[0] == g_deviceB->GetDeviceId());
}

/**
 * @tc.name: Send BroadCast 001
 * @tc.desc: Test DeviceManager SendBroadCast function.
 * @tc.type: FUNC
 * @tc.require: AR000CKRTD AR000CQE0E
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBSyncerDeviceManagerTest, SendBroadCast001, TestSize.Level1)
{
    bool deviceBReviced = false;
bool deviceCReviced = false;

    /**
     * @tc.steps: step1. deviceB, C set OnRemoteDataChanged callback
     */
    g_deviceB->OnRemoteDataChanged([&deviceBReviced](const std::string &deviceId){
        deviceBReviced = true;
    });
    g_deviceC->OnRemoteDataChanged([&deviceCReviced](const std::string &deviceId){
        deviceCReviced = true;
    });

    /**
     * @tc.steps: step2. call SendBroadCast.
     * @tc.expected: step2, deviceB,C OnRemoteDataChanged should be called
     */
    int errCode = g_deviceManager->SendBroadCast(LOCAL_DATA_CHANGED);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    ASSERT_TRUE(errCode == E_OK);
    EXPECT_TRUE(deviceBReviced);
    EXPECT_TRUE(deviceCReviced);
}