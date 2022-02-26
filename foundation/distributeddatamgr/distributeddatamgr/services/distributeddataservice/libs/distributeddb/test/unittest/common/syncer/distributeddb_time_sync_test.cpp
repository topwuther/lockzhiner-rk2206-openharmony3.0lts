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
#include "virtual_time_sync_communicator.h"
#include "isyncer.h"
#include "virtual_single_ver_sync_db_Interface.h"
#include "sync_types.h"
#include "single_ver_sync_state_machine.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;

namespace {
    const std::string DEVICE_A = "deviceA";
    const std::string DEVICE_B = "deviceB";
    VirtualTimeSyncCommunicator *g_virtualCommunicator = nullptr;
    TimeSync *g_timeSyncA = nullptr;
    TimeSync *g_timeSyncB = nullptr;
    VirtualSingleVerSyncDBInterface *g_syncInterfaceA = nullptr;
    VirtualSingleVerSyncDBInterface *g_syncInterfaceB = nullptr;
    std::shared_ptr<Metadata> g_metadataA = nullptr;
    std::shared_ptr<Metadata> g_metadataB = nullptr;
    SingleVerSyncTaskContext *g_syncTaskContext = nullptr;
    const int NETWORK_DELAY = 100 * 1000; // 100ms
}

class DistributedDBTimeSyncTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBTimeSyncTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: NA
     */
}

void DistributedDBTimeSyncTest::TearDownTestCase(void)
{
    /**
     * @tc.teardown: NA
     */
}

void DistributedDBTimeSyncTest::SetUp(void)
{
    /**
     * @tc.setup: create the instance for virtual communicator, virtual storage component and time syncer
     */
    g_virtualCommunicator = new (std::nothrow) VirtualTimeSyncCommunicator();
    ASSERT_TRUE(g_virtualCommunicator != nullptr);

    g_syncInterfaceA = new (std::nothrow) VirtualSingleVerSyncDBInterface();
    ASSERT_TRUE(g_syncInterfaceA != nullptr);

    g_metadataA = std::make_shared<Metadata>();

    g_syncInterfaceB = new (std::nothrow) VirtualSingleVerSyncDBInterface;
    ASSERT_TRUE(g_syncInterfaceB != nullptr);

    g_metadataB = std::make_shared<Metadata>();

    g_timeSyncA = new (std::nothrow) TimeSync();
    ASSERT_TRUE(g_timeSyncA != nullptr);

    g_timeSyncB = new (std::nothrow) TimeSync();
    ASSERT_TRUE(g_timeSyncB != nullptr);

    g_syncTaskContext = new (std::nothrow) SingleVerSyncTaskContext();
    ASSERT_TRUE(g_syncTaskContext != nullptr);
}

void DistributedDBTimeSyncTest::TearDown(void)
{
    /**
     * @tc.teardown: delete the ptr for testing
     */
    if (g_syncTaskContext != nullptr) {
        RefObject::DecObjRef(g_syncTaskContext);
        g_syncTaskContext = nullptr;
    }
    if (g_syncInterfaceA != nullptr) {
        delete g_syncInterfaceA;
        g_syncInterfaceA = nullptr;
    }
    if (g_syncInterfaceB != nullptr) {
        delete g_syncInterfaceB;
        g_syncInterfaceB = nullptr;
    }

    g_metadataA = nullptr;
    g_metadataB = nullptr;
    if (g_timeSyncA != nullptr) {
        delete g_timeSyncA;
        g_timeSyncA = nullptr;
    }
    if (g_timeSyncB != nullptr) {
        delete g_timeSyncB;
        g_timeSyncB = nullptr;
    }
    if (g_virtualCommunicator != nullptr) {
        delete g_virtualCommunicator;
        g_virtualCommunicator = nullptr;
    }
}

/**
 * @tc.name: NormalSync001
 * @tc.desc: Verify time sync function is normal between two time sync instance with different timestamp.
 * @tc.type: FUNC
 * @tc.require: AR000C05EP AR000CQE0G
 * @tc.author: wumin
 */
HWTEST_F(DistributedDBTimeSyncTest, NormalSync001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Initialize the time sync A and B
     * @tc.steps: step2. Write the timestamp into virtual storage component
     * @tc.expected: step1. Initialize time sync A and B successfully
     * @tc.expected: step2. Write the timestamp into virtual storage component successfully.
     */
    g_metadataA->Initialize(g_syncInterfaceA);
    TimeOffset offsetA = 100 * 1000 * 1000; // 100 seconds
    // set timestamp for A virtual storage component
    g_syncInterfaceA->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1,
        TimeHelper::GetSysCurrentTime() + TimeHelper::BASE_OFFSET + offsetA, 0);
    int errCode;
    // initialize timeSyncA
    errCode = g_timeSyncA->Initialize(g_virtualCommunicator, g_metadataA, g_syncInterfaceA, DEVICE_B);
    EXPECT_TRUE(errCode == E_OK);

    g_metadataB->Initialize(g_syncInterfaceB);
    TimeOffset offsetB = 200 * 1000 * 1000; // 200 seconds
    // set timestamp for B virtual storage component
    g_syncInterfaceB->PutData(DistributedDBUnitTest::KEY_1, DistributedDBUnitTest::VALUE_1,
        TimeHelper::GetSysCurrentTime() + TimeHelper::BASE_OFFSET + offsetB, 0);
    // initialize timeSyncB
    errCode = g_timeSyncB->Initialize(g_virtualCommunicator, g_metadataB, g_syncInterfaceB, DEVICE_A);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step3. Register the OnMessageCallback to virtual communicator
     */
    g_syncTaskContext->Initialize(DEVICE_B, g_syncInterfaceA, g_metadataA, g_virtualCommunicator);
    g_virtualCommunicator->SetTimeSync(g_timeSyncA, g_timeSyncB, DEVICE_A, g_syncTaskContext);

    /**
     * @tc.steps: step4. Fetch timeOffset value
     * @tc.expected: step4. (offsetB - offsetA ) - timeOffset < 100ms.
     */
    TimeOffset timeOffset = 0;
    g_timeSyncA->GetTimeOffset(timeOffset);
    offsetB = g_metadataB->GetLocalTimeOffset();
    offsetA = g_metadataA->GetLocalTimeOffset();
    EXPECT_TRUE(abs(offsetB - offsetA - timeOffset) < NETWORK_DELAY);
}

/**
 * @tc.name: NormalSync002
 * @tc.desc: Verify time sync function is normal between two time sync instance with the same timestamp.
 * @tc.type: FUNC
 * @tc.require: AR000C05EP AR000CQE0G
 * @tc.author: wumin
 */
HWTEST_F(DistributedDBTimeSyncTest, NormalSync002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Initialize the time sync A and B
     * @tc.expected: step1. Initialize time sync A and B successfully
     */
    g_metadataA->Initialize(g_syncInterfaceA);
    int errCode;
    // initialize timeSyncA
    errCode = g_timeSyncA->Initialize(g_virtualCommunicator, g_metadataA, g_syncInterfaceA, DEVICE_B);
    EXPECT_TRUE(errCode == E_OK);

    g_metadataB->Initialize(g_syncInterfaceB);
        // initialize timeSyncB
    errCode = g_timeSyncB->Initialize(g_virtualCommunicator, g_metadataB, g_syncInterfaceB, DEVICE_A);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step2. Register the OnMessageCallback to virtual communicator
     */
    g_syncTaskContext->Initialize(DEVICE_B, g_syncInterfaceA, g_metadataA, g_virtualCommunicator);
    g_virtualCommunicator->SetTimeSync(g_timeSyncA, g_timeSyncB, DEVICE_A, g_syncTaskContext);

    /**
     * @tc.steps: step3. Fetch timeOffset value
     * @tc.expected: step3. (offsetB - offsetA ) - timeOffset < 100ms.
     */
    TimeOffset timeOffset;
    g_timeSyncA->GetTimeOffset(timeOffset);
    TimeOffset offsetB = g_metadataB->GetLocalTimeOffset();
    TimeOffset offsetA = g_metadataA->GetLocalTimeOffset();
    EXPECT_TRUE(abs(offsetB - offsetA - timeOffset) < NETWORK_DELAY);
}

/**
 * @tc.name: NormalSync003
 * @tc.desc: Verify time sync function is normal between two time sync instance with different localTimeOffset.
 * @tc.type: FUNC
 * @tc.require: AR000C05EP AR000CQE0G
 * @tc.author: wumin
 */
HWTEST_F(DistributedDBTimeSyncTest, NormalSync003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Initialize the time sync A and B
     * @tc.steps: step2. Write the timeOffset into time sync A and B
     * @tc.expected: step1. Initialize time sync A and B successfully
     * @tc.expected: step2. Write the timeOffset into time sync A and B successfully.
     */
    g_metadataA->Initialize(g_syncInterfaceA);

    // set timeOffset for timeSyncA
    TimeOffset offsetA = 1;
    g_metadataA->SaveLocalTimeOffset(offsetA);

    int errCode;
    // initialize timeSyncA
    errCode = g_timeSyncA->Initialize(g_virtualCommunicator, g_metadataA, g_syncInterfaceA, DEVICE_B);
    EXPECT_TRUE(errCode == E_OK);

    // set timeOffset for timeSyncA
    g_metadataB->Initialize(g_syncInterfaceB);
    TimeOffset offsetB = 100 * 1000 * 1000;
    g_metadataB->SaveLocalTimeOffset(offsetB);

    // initialize timeSyncB
    errCode = g_timeSyncB->Initialize(g_virtualCommunicator, g_metadataB, g_syncInterfaceB, DEVICE_A);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step3. Register the OnMessageCallback to virtual communicator
     */
    g_syncTaskContext->Initialize(DEVICE_B, g_syncInterfaceA, g_metadataA, g_virtualCommunicator);
    g_virtualCommunicator->SetTimeSync(g_timeSyncA, g_timeSyncB, DEVICE_A, g_syncTaskContext);

    /**
     * @tc.steps: step4. Fetch timeOffset value
     * @tc.expected: step4. (offsetB - offsetA ) - timeOffset < 100ms.
     */
    TimeOffset timeOffset = 0;
    g_timeSyncA->GetTimeOffset(timeOffset);

    TimeOffset absTimeOffset = abs(timeOffset);
    EXPECT_TRUE(abs(offsetB - offsetA - absTimeOffset) < NETWORK_DELAY);
}

/**
 * @tc.name: NetDisconnetSyncTest001
 * @tc.desc: Verify time sync function return failed when the virtual communicator disabled.
 * @tc.type: FUNC
 * @tc.require: AR000C05EP AR000CQE0G
 * @tc.author: wumin
 */
HWTEST_F(DistributedDBTimeSyncTest, NetDisconnetSyncTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Initialize the time sync A and B
     * @tc.expected: step1. Initialize time sync A and B successfully
     */
    g_metadataA->Initialize(g_syncInterfaceA);
    int errCode;
    // initialize timeSyncA
    errCode = g_timeSyncA->Initialize(g_virtualCommunicator, g_metadataA, g_syncInterfaceA, DEVICE_B);
    EXPECT_TRUE(errCode == E_OK);

    g_metadataB->Initialize(g_syncInterfaceB);
    // initialize timeSyncB
    errCode = g_timeSyncB->Initialize(g_virtualCommunicator, g_metadataB, g_syncInterfaceB, DEVICE_A);
    EXPECT_TRUE(errCode == E_OK);

    g_syncTaskContext->Initialize(DEVICE_B, g_syncInterfaceA, g_metadataA, g_virtualCommunicator);
    g_virtualCommunicator->SetTimeSync(g_timeSyncA, g_timeSyncB, DEVICE_A, g_syncTaskContext);

    /**
     * @tc.steps: step2. Disable the virtual communicator
     */
    g_virtualCommunicator->Disable();

    /**
     * @tc.steps: step3. Start time sync function
     * @tc.expected: step3. time sync return -E_PERIPHERAL_INTERFACE_FAIL
     */
    errCode = g_timeSyncA->SyncStart();
    EXPECT_TRUE(errCode == -E_PERIPHERAL_INTERFACE_FAIL);
}

/**
 * @tc.name: InvalidMessgeTest001
 * @tc.desc: Verify RequestReceive() return failed with invalid input.
 * @tc.type: FUNC
 * @tc.require: AR000C05EP AR000CQE0G
 * @tc.author: wumin
 */
HWTEST_F(DistributedDBTimeSyncTest, InvalidMessgeTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Initialize the time sync A and B
     * @tc.expected: step1. Initialize time sync A and B successfully
     */
    g_metadataA->Initialize(g_syncInterfaceA);
    int errCode;
    // initialize timeSyncA
    errCode = g_timeSyncA->Initialize(g_virtualCommunicator, g_metadataA, g_syncInterfaceA, DEVICE_B);
    EXPECT_TRUE(errCode == E_OK);

    g_metadataB->Initialize(g_syncInterfaceB);
    // initialize timeSyncB
    errCode = g_timeSyncB->Initialize(g_virtualCommunicator, g_metadataB, g_syncInterfaceB, DEVICE_A);
    EXPECT_TRUE(errCode == E_OK);

    g_virtualCommunicator->SetTimeSync(g_timeSyncA, g_timeSyncB, DEVICE_A, g_syncTaskContext);

    Message *msg = new (std::nothrow) Message();
    ASSERT_TRUE(msg != nullptr);

    /**
     * @tc.steps: step2. SendMessage with id = TIME_SYNC_MESSAGE, type = TYPE_REQUEST and no data set
     * @tc.expected: step2. RequestRecv() return -E_INVALID_ARGS
     */
    msg->SetMessageId(TIME_SYNC_MESSAGE);
    msg->SetMessageType(TYPE_REQUEST);
    errCode = g_virtualCommunicator->SendMessage(DEVICE_B, msg, false, 0);
    EXPECT_TRUE(errCode == -E_INVALID_ARGS);

    TimeSyncPacket data;
    data.SetSourceTimeBegin(0);
    data.SetSourceTimeEnd(0);
    data.SetTargetTimeBegin(0);
    data.SetTargetTimeEnd(0);

    /**
     * @tc.steps: step3. SendMessage with id = DATA_SYNC_MESSAGE, type = TYPE_REQUEST
     * @tc.expected: step3. RequestRecv() return -E_INVALID_ARGS
     */
    msg = new (std::nothrow) Message();
    ASSERT_TRUE(msg != nullptr);
    msg->SetMessageId(DATA_SYNC_MESSAGE);
    msg->SetMessageType(TYPE_REQUEST);
    msg->SetCopiedObject<>(data);
    errCode = g_virtualCommunicator->SendMessage(DEVICE_B, msg, false, 0);
    EXPECT_TRUE(errCode == -E_INVALID_ARGS);

    /**
     * @tc.steps: step4. SendMessage with id = TIME_SYNC_MESSAGE, type = TYPE_RESPONSE
     * @tc.expected: step4. RequestRecv() return -E_INVALID_ARGS
     */
    msg = new (std::nothrow) Message();
    ASSERT_TRUE(msg != nullptr);
    msg->SetMessageId(TIME_SYNC_MESSAGE);
    msg->SetMessageType(TYPE_RESPONSE);
    msg->SetCopiedObject<>(data);
    errCode = g_virtualCommunicator->SendMessage(DEVICE_B, msg, false, 0);
    EXPECT_TRUE(errCode == -E_INVALID_ARGS);
}

/**
 * @tc.name: InvalidMessgeTest002
 * @tc.desc: Verify AckRec() return failed with invalid input.
 * @tc.type: FUNC
 * @tc.require: AR000C05EP AR000CQE0G
 * @tc.author: wumin
 */
HWTEST_F(DistributedDBTimeSyncTest, InvalidMessgeTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Initialize the time sync A and B
     * @tc.expected: step1. Initialize time sync A and B successfully
     */
    g_metadataA->Initialize(g_syncInterfaceA);
    int errCode;
    // initialize timeSyncA
    errCode = g_timeSyncA->Initialize(g_virtualCommunicator, g_metadataA, g_syncInterfaceA, DEVICE_B);
    EXPECT_TRUE(errCode == E_OK);

    g_metadataB->Initialize(g_syncInterfaceB);
    // initialize timeSyncB
    errCode = g_timeSyncB->Initialize(g_virtualCommunicator, g_metadataB, g_syncInterfaceB, DEVICE_A);
    EXPECT_TRUE(errCode == E_OK);

    g_syncTaskContext->Initialize(DEVICE_B, g_syncInterfaceA, g_metadataA, g_virtualCommunicator);
    g_virtualCommunicator->SetTimeSync(g_timeSyncA, g_timeSyncB, DEVICE_A, g_syncTaskContext);

    Message *msg = new (std::nothrow) Message();
    ASSERT_TRUE(msg != nullptr);

    /**
     * @tc.steps: step2. SendMessage with id = TIME_SYNC_MESSAGE, type = TYPE_RESPONSE and no data set
     * @tc.expected: step2. AckRecv() return -E_INVALID_ARGS
     */
    msg->SetMessageId(TIME_SYNC_MESSAGE);
    msg->SetMessageType(TYPE_RESPONSE);
    errCode = g_virtualCommunicator->SendMessage(DEVICE_A, msg, false, 0);
    EXPECT_TRUE(errCode == -E_INVALID_ARGS);

    TimeSyncPacket data;
    data.SetSourceTimeBegin(0);
    data.SetSourceTimeEnd(0);
    data.SetTargetTimeBegin(0);
    data.SetTargetTimeEnd(0);

    /**
     * @tc.steps: step3. SendMessage with id = DATA_SYNC_MESSAGE, type = TYPE_RESPONSE and no data set
     * @tc.expected: step3. AckRecv() return -E_INVALID_ARGS
     */
    msg = new (std::nothrow) Message();
    ASSERT_TRUE(msg != nullptr);
    msg->SetMessageId(DATA_SYNC_MESSAGE);
    msg->SetMessageType(TYPE_RESPONSE);
    msg->SetCopiedObject<>(data);
    errCode = g_virtualCommunicator->SendMessage(DEVICE_A, msg, false, 0);
    EXPECT_TRUE(errCode == -E_INVALID_ARGS);

    /**
     * @tc.steps: step4. SendMessage with id = TIME_SYNC_MESSAGE, type = TYPE_REQUEST and no data set
     * @tc.expected: step4. AckRecv() return -E_INVALID_ARGS
     */
    msg = new (std::nothrow) Message();
    ASSERT_TRUE(msg != nullptr);
    msg->SetMessageId(TIME_SYNC_MESSAGE);
    msg->SetMessageType(TYPE_REQUEST);
    msg->SetCopiedObject<>(data);
    errCode = g_virtualCommunicator->SendMessage(DEVICE_A, msg, false, 0);
    EXPECT_TRUE(errCode == -E_INVALID_ARGS);
}

/**
 * @tc.name: SyncTimeout001
 * @tc.desc: Verify the timeout scenario for time sync.
 * @tc.type: FUNC
 * @tc.require: AR000C05EP AR000CQE0G
 * @tc.author: wumin
 */
HWTEST_F(DistributedDBTimeSyncTest, SyncTimeout001, TestSize.Level2)
{
    // initialize timeSyncA
    g_metadataA->Initialize(g_syncInterfaceA);
    int errCode;
    errCode = g_timeSyncA->Initialize(g_virtualCommunicator, g_metadataA, g_syncInterfaceA, DEVICE_B);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step1. Initialize the syncTaskContext
     * @tc.expected: step1. Initialize syncTaskContext successfully
     */
    errCode = g_syncTaskContext->Initialize(DEVICE_B, g_syncInterfaceA, g_metadataA, g_virtualCommunicator);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step2. Start the time syc task invoking StartSync() method
     * @tc.expected: step2. Start the time sync task return E_TIMEOUT
     */
    TimeOffset offset;
    errCode = g_timeSyncA->GetTimeOffset(offset);
    EXPECT_TRUE(errCode == -E_TIMEOUT);
}