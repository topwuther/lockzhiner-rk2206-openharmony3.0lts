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
#include <new>
#include <set>
#include <thread>
#include "message.h"
#include "db_errno.h"
#include "log_print.h"
#include "serial_buffer.h"
#include "distributeddb_communicator_common.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;

namespace {
    EnvHandle g_envDeviceA;
    EnvHandle g_envDeviceB;
    EnvHandle g_envDeviceC;
    ICommunicator *g_commAA = nullptr;
    ICommunicator *g_commAB = nullptr;
    ICommunicator *g_commBB = nullptr;
    ICommunicator *g_commBC = nullptr;
    ICommunicator *g_commCC = nullptr;
    ICommunicator *g_commCA = nullptr;
}

class DistributedDBCommunicatorDeepTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBCommunicatorDeepTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Create and init CommunicatorAggregator and AdapterStub
     */
    LOGI("[UT][DeepTest][SetUpTestCase] Enter.");
    bool errCode = SetUpEnv(g_envDeviceA, DEVICE_NAME_A);
    ASSERT_EQ(errCode, true);
    errCode = SetUpEnv(g_envDeviceB, DEVICE_NAME_B);
    ASSERT_EQ(errCode, true);
    errCode = SetUpEnv(g_envDeviceC, DEVICE_NAME_C);
    ASSERT_EQ(errCode, true);
    DoRegTransformFunction();
    CommunicatorAggregator::EnableCommunicatorNotFoundFeedback(false);
}

void DistributedDBCommunicatorDeepTest::TearDownTestCase(void)
{
    /**
     * @tc.teardown: Finalize and release CommunicatorAggregator and AdapterStub
     */
    LOGI("[UT][DeepTest][TearDownTestCase] Enter.");
    std::this_thread::sleep_for(std::chrono::seconds(7)); // Wait 7 s to make sure all thread quiet and memory released
    TearDownEnv(g_envDeviceA);
    TearDownEnv(g_envDeviceB);
    TearDownEnv(g_envDeviceC);
    CommunicatorAggregator::EnableCommunicatorNotFoundFeedback(true);
}

namespace {
void AllocAllCommunicator()
{
    int errorNo = E_OK;
    g_commAA = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(g_commAA);
    g_commAB = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_B, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(g_commAB);
    g_commBB = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_B, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(g_commBB);
    g_commBC = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_C, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(g_commBC);
    g_commCC = g_envDeviceC.commAggrHandle->AllocCommunicator(LABEL_C, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(g_commCC);
    g_commCA = g_envDeviceC.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(g_commCA);
}

void ReleaseAllCommunicator()
{
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(g_commAA);
    g_commAA = nullptr;
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(g_commAB);
    g_commAB = nullptr;
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(g_commBB);
    g_commBB = nullptr;
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(g_commBC);
    g_commBC = nullptr;
    g_envDeviceC.commAggrHandle->ReleaseCommunicator(g_commCC);
    g_commCC = nullptr;
    g_envDeviceC.commAggrHandle->ReleaseCommunicator(g_commCA);
    g_commCA = nullptr;
}
}

void DistributedDBCommunicatorDeepTest::SetUp()
{
    /**
     * @tc.setup: Alloc communicator AA, AB, BB, BC, CC, CA
     */
    AllocAllCommunicator();
}

void DistributedDBCommunicatorDeepTest::TearDown()
{
    /**
     * @tc.teardown: Release communicator AA, AB, BB, BC, CC, CA
     */
    ReleaseAllCommunicator();
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Wait 200 ms to make sure all thread quiet
}

/**
 * @tc.name: WaitAndRetrySend 001
 * @tc.desc: Test send retry semantic
 * @tc.type: FUNC
 * @tc.require: AR000BVDGI AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorDeepTest, WaitAndRetrySend001, TestSize.Level2)
{
    // Preset
    Message *msgForBB = nullptr;
    g_commBB->RegOnMessageCallback([&msgForBB](const std::string &srcTarget, Message *inMsg){
        msgForBB = inMsg;
    }, nullptr);

    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Wait 200 ms to make sure quiet

    /**
     * @tc.steps: step2. device A simulate send retry
     */
    g_envDeviceA.adapterHandle->SimulateSendRetry(DEVICE_NAME_B);

    /**
     * @tc.steps: step3. device A send message to device B using communicator AB
     * @tc.expected: step3. communicator BB received no message
     */
    Message *msgForAB = BuildRegedTinyMessage();
    ASSERT_NE(msgForAB, nullptr);
    int errCode = g_commAB->SendMessage(DEVICE_NAME_B, msgForAB, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait 100 ms
    EXPECT_EQ(msgForBB, nullptr);

    /**
     * @tc.steps: step4. device A simulate sendable feedback
     * @tc.expected: step4. communicator BB received the message
     */
    g_envDeviceA.adapterHandle->SimulateSendRetryClear(DEVICE_NAME_B);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait 100 ms
    EXPECT_NE(msgForBB, nullptr);
    delete msgForBB;
    msgForBB = nullptr;

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

static int CreateBufferThenAddIntoScheduler(SendTaskScheduler &scheduler, const std::string &dstTarget, Priority inPrio)
{
    SerialBuffer *eachBuff = new (std::nothrow) SerialBuffer();
    if (eachBuff == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    int errCode = eachBuff->AllocBufferByTotalLength(100, 0); // 100 totallen without header
    if (errCode != E_OK) {
        delete eachBuff;
        eachBuff = nullptr;
        return errCode;
    }
    SendTask task{eachBuff, dstTarget};
    errCode = scheduler.AddSendTaskIntoSchedule(task, inPrio);
    if (errCode != E_OK) {
        delete eachBuff;
        eachBuff = nullptr;
        return errCode;
    }
    return E_OK;
}

/**
 * @tc.name: SendSchedule 001
 * @tc.desc: Test schedule in Priority order than in send order
 * @tc.type: FUNC
 * @tc.require: AR000BVDGI AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorDeepTest, SendSchedule001, TestSize.Level2)
{
    // Preset
    SendTaskScheduler scheduler;
    scheduler.Initialize();

    /**
     * @tc.steps: step1. Add low priority target A buffer to schecduler
     */
    int errCode = CreateBufferThenAddIntoScheduler(scheduler, DEVICE_NAME_A, Priority::LOW);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step2. Add low priority target B buffer to schecduler
     */
    errCode = CreateBufferThenAddIntoScheduler(scheduler, DEVICE_NAME_B, Priority::LOW);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step3. Add normal priority target B buffer to schecduler
     */
    errCode = CreateBufferThenAddIntoScheduler(scheduler, DEVICE_NAME_B, Priority::NORMAL);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step4. Add normal priority target C buffer to schecduler
     */
    errCode = CreateBufferThenAddIntoScheduler(scheduler, DEVICE_NAME_C, Priority::NORMAL);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step5. Add high priority target C buffer to schecduler
     */
    errCode = CreateBufferThenAddIntoScheduler(scheduler, DEVICE_NAME_C, Priority::HIGH);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step6. Add high priority target A buffer to schecduler
     */
    errCode = CreateBufferThenAddIntoScheduler(scheduler, DEVICE_NAME_A, Priority::HIGH);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step7. schedule out buffers one by one
     * @tc.expected: step7. the order is: high priority target C
     *                                    high priority target A
     *                                    normal priority target B
     *                                    normal priority target C
     *                                    low priority target A
     *                                    low priority target B
     */
    SendTask outTask;
    SendTaskInfo outTaskInfo;
    // high priority target C
    errCode = scheduler.ScheduleOutSendTask(outTask, outTaskInfo);
    ASSERT_EQ(errCode, E_OK);
    EXPECT_EQ(outTask.dstTarget, DEVICE_NAME_C);
    EXPECT_EQ(outTaskInfo.taskPrio, Priority::HIGH);
    scheduler.FinalizeLastScheduleTask();
    // high priority target A
    errCode = scheduler.ScheduleOutSendTask(outTask, outTaskInfo);
    ASSERT_EQ(errCode, E_OK);
    EXPECT_EQ(outTask.dstTarget, DEVICE_NAME_A);
    EXPECT_EQ(outTaskInfo.taskPrio, Priority::HIGH);
    scheduler.FinalizeLastScheduleTask();
    // normal priority target B
    errCode = scheduler.ScheduleOutSendTask(outTask, outTaskInfo);
    ASSERT_EQ(errCode, E_OK);
    EXPECT_EQ(outTask.dstTarget, DEVICE_NAME_B);
    EXPECT_EQ(outTaskInfo.taskPrio, Priority::NORMAL);
    scheduler.FinalizeLastScheduleTask();
    // normal priority target C
    errCode = scheduler.ScheduleOutSendTask(outTask, outTaskInfo);
    ASSERT_EQ(errCode, E_OK);
    EXPECT_EQ(outTask.dstTarget, DEVICE_NAME_C);
    EXPECT_EQ(outTaskInfo.taskPrio, Priority::NORMAL);
    scheduler.FinalizeLastScheduleTask();
    // low priority target A
    errCode = scheduler.ScheduleOutSendTask(outTask, outTaskInfo);
    ASSERT_EQ(errCode, E_OK);
    EXPECT_EQ(outTask.dstTarget, DEVICE_NAME_A);
    EXPECT_EQ(outTaskInfo.taskPrio, Priority::LOW);
    scheduler.FinalizeLastScheduleTask();
    // low priority target B
    errCode = scheduler.ScheduleOutSendTask(outTask, outTaskInfo);
    ASSERT_EQ(errCode, E_OK);
    EXPECT_EQ(outTask.dstTarget, DEVICE_NAME_B);
    EXPECT_EQ(outTaskInfo.taskPrio, Priority::LOW);
    scheduler.FinalizeLastScheduleTask();
}

/**
 * @tc.name: Fragment 001
 * @tc.desc: Test fragmentation in send and receive
 * @tc.type: FUNC
 * @tc.require: AR000BVDGI AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorDeepTest, Fragment001, TestSize.Level2)
{
    // Preset
    Message *recvMsgForBB = nullptr;
    g_commBB->RegOnMessageCallback([&recvMsgForBB](const std::string &srcTarget, Message *inMsg){
        recvMsgForBB = inMsg;
    }, nullptr);

    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);

    /**
     * @tc.steps: step2. device A send message(registered and giant) to device B using communicator AB
     * @tc.expected: step2. communicator BB received the message
     */
    const uint32_t dataLength = 13 * 1024 * 1024; // 13 MB, 1024 is scale
    Message *sendMsgForAB = BuildRegedGiantMessage(dataLength);
    ASSERT_NE(sendMsgForAB, nullptr);
    int errCode = g_commAB->SendMessage(DEVICE_NAME_B, sendMsgForAB, false, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(2600)); // Wait 2600 ms to make sure send done
    ASSERT_NE(recvMsgForBB, nullptr);
    ASSERT_EQ(recvMsgForBB->GetMessageId(), REGED_GIANT_MSG_ID);

    /**
     * @tc.steps: step3. Compare received data with send data
     * @tc.expected: step3. equal
     */
    Message *oriMsgForAB = BuildRegedGiantMessage(dataLength);
    ASSERT_NE(oriMsgForAB, nullptr);
    const RegedGiantObject *oriObjForAB = oriMsgForAB->GetObject<RegedGiantObject>();
    ASSERT_NE(oriObjForAB, nullptr);
    const RegedGiantObject *recvObjForBB = recvMsgForBB->GetObject<RegedGiantObject>();
    ASSERT_NE(recvObjForBB, nullptr);
    bool isEqual = RegedGiantObject::CheckEqual(*oriObjForAB, *recvObjForBB);
    EXPECT_EQ(isEqual, true);

    // CleanUp
    delete oriMsgForAB;
    oriMsgForAB = nullptr;
    delete recvMsgForBB;
    recvMsgForBB = nullptr;
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Fragment 002
 * @tc.desc: Test fragmentation in partial loss
 * @tc.type: FUNC
 * @tc.require: AR000BVDGI AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorDeepTest, Fragment002, TestSize.Level2)
{
    // Preset
    Message *recvMsgForCC = nullptr;
    g_commCC->RegOnMessageCallback([&recvMsgForCC](const std::string &srcTarget, Message *inMsg){
        recvMsgForCC = inMsg;
    }, nullptr);

    /**
     * @tc.steps: step1. connect device B with device C
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Wait 200 ms to make sure quiet

    /**
     * @tc.steps: step2. device B simulate partial loss
     */
    g_envDeviceB.adapterHandle->SimulateSendPartialLoss();

    /**
     * @tc.steps: step3. device B send message(registered and giant) to device C using communicator BC
     * @tc.expected: step3. communicator CC not receive the message
     */
    uint32_t dataLength = 13 * 1024 * 1024; // 13 MB, 1024 is scale
    Message *sendMsgForBC = BuildRegedGiantMessage(dataLength);
    ASSERT_NE(sendMsgForBC, nullptr);
    int errCode = g_commBC->SendMessage(DEVICE_NAME_C, sendMsgForBC, false, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(2600)); // Wait 2600 ms to make sure send done
    EXPECT_EQ(recvMsgForCC, nullptr);

    /**
     * @tc.steps: step4. device B not simulate partial loss
     */
    g_envDeviceB.adapterHandle->SimulateSendPartialLossClear();

    /**
     * @tc.steps: step5. device B send message(registered and giant) to device C using communicator BC
     * @tc.expected: step5. communicator CC received the message, the length equal to the one that is second send
     */
    dataLength = 17 * 1024 * 1024; // 17 MB, 1024 is scale
    Message *resendMsgForBC = BuildRegedGiantMessage(dataLength);
    ASSERT_NE(resendMsgForBC, nullptr);
    errCode = g_commBC->SendMessage(DEVICE_NAME_C, resendMsgForBC, false, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(3400)); // Wait 3400 ms to make sure send done
    ASSERT_NE(recvMsgForCC, nullptr);
    ASSERT_EQ(recvMsgForCC->GetMessageId(), REGED_GIANT_MSG_ID);
    const RegedGiantObject *recvObjForCC = recvMsgForCC->GetObject<RegedGiantObject>();
    ASSERT_NE(recvObjForCC, nullptr);
    EXPECT_EQ(dataLength, recvObjForCC->rawData_.size());

    // CleanUp
    delete recvMsgForCC;
    recvMsgForCC = nullptr;
    AdapterStub::DisconnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
}

/**
 * @tc.name: Fragment 003
 * @tc.desc: Test fragmentation simultaneously
 * @tc.type: FUNC
 * @tc.require: AR000BVDGI AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorDeepTest, Fragment003, TestSize.Level3)
{
    // Preset
    std::atomic<int> count{0};
    OnMessageCallback callback = [&count](const std::string &srcTarget, Message *inMsg) {
        delete inMsg;
        inMsg = nullptr;
        count.fetch_add(1, std::memory_order_seq_cst);
    };
    g_commBB->RegOnMessageCallback(callback, nullptr);
    g_commBC->RegOnMessageCallback(callback, nullptr);

    /**
     * @tc.steps: step1. connect device A with device B, then device B with device C
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    AdapterStub::ConnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(400)); // Wait 400 ms to make sure quiet

    /**
     * @tc.steps: step2. device A and device C simulate send block
     */
    g_envDeviceA.adapterHandle->SimulateSendBlock();
    g_envDeviceC.adapterHandle->SimulateSendBlock();

    /**
     * @tc.steps: step3. device A send message(registered and giant) to device B using communicator AB
     */
    uint32_t dataLength = 23 * 1024 * 1024; // 23 MB, 1024 is scale
    Message *sendMsgForAB = BuildRegedGiantMessage(dataLength);
    ASSERT_NE(sendMsgForAB, nullptr);
    int errCode = g_commAB->SendMessage(DEVICE_NAME_B, sendMsgForAB, false, 0);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step4. device C send message(registered and giant) to device B using communicator CC
     */
    Message *sendMsgForCC = BuildRegedGiantMessage(dataLength);
    ASSERT_NE(sendMsgForCC, nullptr);
    errCode = g_commCC->SendMessage(DEVICE_NAME_B, sendMsgForCC, false, 0);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step5. device A and device C not simulate send block
     * @tc.expected: step5. communicator BB and BV received the message
     */
    g_envDeviceA.adapterHandle->SimulateSendBlockClear();
    g_envDeviceC.adapterHandle->SimulateSendBlockClear();
    std::this_thread::sleep_for(std::chrono::milliseconds(9200)); // Wait 9200 ms to make sure send done
    EXPECT_EQ(count, 2); // 2 combined message received

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    AdapterStub::DisconnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
}

namespace {
void ClearPreviousTestCaseInfluence()
{
    ReleaseAllCommunicator();
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    AdapterStub::ConnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
    AdapterStub::ConnectAdapterStub(g_envDeviceC.adapterHandle, g_envDeviceA.adapterHandle);
    std::this_thread::sleep_for(std::chrono::seconds(10)); // Wait 10 s to make sure all thread quiet
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    AdapterStub::DisconnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
    AdapterStub::DisconnectAdapterStub(g_envDeviceC.adapterHandle, g_envDeviceA.adapterHandle);
    AllocAllCommunicator();
}
}

/**
 * @tc.name: ReliableOnline 001
 * @tc.desc: Test device online reliability
 * @tc.type: FUNC
 * @tc.require: AR000BVDGJ AR000CQE0N
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorDeepTest, ReliableOnline001, TestSize.Level2)
{
    // Preset
    ClearPreviousTestCaseInfluence();
    std::atomic<int> count{0};
    OnConnectCallback callback = [&count](const std::string &target, bool isConnect) {
        if (isConnect) {
            count.fetch_add(1, std::memory_order_seq_cst);
        }
    };
    g_commAA->RegOnConnectCallback(callback, nullptr);
    g_commAB->RegOnConnectCallback(callback, nullptr);
    g_commBB->RegOnConnectCallback(callback, nullptr);
    g_commBC->RegOnConnectCallback(callback, nullptr);
    g_commCC->RegOnConnectCallback(callback, nullptr);
    g_commCA->RegOnConnectCallback(callback, nullptr);

    /**
     * @tc.steps: step1. device A and device B and device C simulate send total loss
     */
    g_envDeviceA.adapterHandle->SimulateSendTotalLoss();
    g_envDeviceB.adapterHandle->SimulateSendTotalLoss();
    g_envDeviceC.adapterHandle->SimulateSendTotalLoss();

    /**
     * @tc.steps: step2. connect device A with device B, device B with device C, device C with device A
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    AdapterStub::ConnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
    AdapterStub::ConnectAdapterStub(g_envDeviceC.adapterHandle, g_envDeviceA.adapterHandle);

    /**
     * @tc.steps: step3. wait a long time
     * @tc.expected: step3. no communicator received the online callback
     */
    std::this_thread::sleep_for(std::chrono::seconds(7)); // Wait 7 s to make sure quiet
    EXPECT_EQ(count, 0); // no online callback received

    /**
     * @tc.steps: step4. device A and device B and device C not simulate send total loss
     */
    g_envDeviceA.adapterHandle->SimulateSendTotalLossClear();
    g_envDeviceB.adapterHandle->SimulateSendTotalLossClear();
    g_envDeviceC.adapterHandle->SimulateSendTotalLossClear();
    std::this_thread::sleep_for(std::chrono::seconds(7)); // Wait 7 s to make sure send done
    EXPECT_EQ(count, 6); // 6 online callback received in total

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    AdapterStub::DisconnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
    AdapterStub::DisconnectAdapterStub(g_envDeviceC.adapterHandle, g_envDeviceA.adapterHandle);
}
