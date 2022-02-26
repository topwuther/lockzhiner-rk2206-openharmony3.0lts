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
#include <set>
#include <thread>
#include "message.h"
#include "log_print.h"
#include "db_errno.h"
#include "distributeddb_communicator_common.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;

namespace {
    constexpr int SEND_COUNT_GOAL = 20; // Send 20 times

    EnvHandle g_envDeviceA;
    EnvHandle g_envDeviceB;
    ICommunicator *g_commAA = nullptr;
    ICommunicator *g_commBA = nullptr;
    ICommunicator *g_commBB = nullptr;
}

class DistributedDBCommunicatorSendReceiveTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBCommunicatorSendReceiveTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Create and init CommunicatorAggregator and AdapterStub
     */
    LOGI("[UT][SendRecvTest][SetUpTestCase] Enter.");
    bool errCode = SetUpEnv(g_envDeviceA, DEVICE_NAME_A);
    ASSERT_EQ(errCode, true);
    errCode = SetUpEnv(g_envDeviceB, DEVICE_NAME_B);
    ASSERT_EQ(errCode, true);
    DoRegTransformFunction();
    CommunicatorAggregator::EnableCommunicatorNotFoundFeedback(false);
}

void DistributedDBCommunicatorSendReceiveTest::TearDownTestCase(void)
{
    /**
     * @tc.teardown: Finalize and release CommunicatorAggregator and AdapterStub
     */
    LOGI("[UT][SendRecvTest][TearDownTestCase] Enter.");
    std::this_thread::sleep_for(std::chrono::seconds(7)); // Wait 7 s to make sure all thread quiet and memory released
    TearDownEnv(g_envDeviceA);
    TearDownEnv(g_envDeviceB);
    CommunicatorAggregator::EnableCommunicatorNotFoundFeedback(true);
}

void DistributedDBCommunicatorSendReceiveTest::SetUp()
{
    /**
     * @tc.setup: Alloc communicator AA, BA, BB
     */
    int errorNo = E_OK;
    g_commAA = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    ASSERT_EQ(errorNo, E_OK);
    ASSERT_NOT_NULL_AND_ACTIVATE(g_commAA);

    errorNo = E_OK;
    g_commBA = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    ASSERT_EQ(errorNo, E_OK);
    ASSERT_NOT_NULL_AND_ACTIVATE(g_commBA);

    errorNo = E_OK;
    g_commBB = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_B, errorNo);
    ASSERT_EQ(errorNo, E_OK);
    ASSERT_NOT_NULL_AND_ACTIVATE(g_commBB);
}

void DistributedDBCommunicatorSendReceiveTest::TearDown()
{
    /**
     * @tc.teardown: Release communicator AA, BA, BB
     */
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(g_commAA);
    g_commAA = nullptr;
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(g_commBA);
    g_commBA = nullptr;
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(g_commBB);
    g_commBA = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Wait 200 ms to make sure all thread quiet
}

/**
 * @tc.name: Send And Receive 001
 * @tc.desc: Test send and receive based on equipment communicator
 * @tc.type: FUNC
 * @tc.require: AR000BVDGI AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, SendAndReceive001, TestSize.Level1)
{
    // Preset
    string srcTargetForAA;
    Message *recvMsgForAA = nullptr;
    string srcTargetForBA;
    Message *recvMsgForBA = nullptr;
    string srcTargetForBB;
    Message *recvMsgForBB = nullptr;
    g_commAA->RegOnMessageCallback([&srcTargetForAA, &recvMsgForAA](const std::string &srcTarget, Message *inMsg){
        srcTargetForAA = srcTarget;
        recvMsgForAA = inMsg;
    }, nullptr);
    g_commBA->RegOnMessageCallback([&srcTargetForBA, &recvMsgForBA](const std::string &srcTarget, Message *inMsg){
        srcTargetForBA = srcTarget;
        recvMsgForBA = inMsg;
    }, nullptr);
    g_commBB->RegOnMessageCallback([&srcTargetForBB, &recvMsgForBB](const std::string &srcTarget, Message *inMsg){
        srcTargetForBB = srcTarget;
        recvMsgForBB = inMsg;
    }, nullptr);

    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);

    /**
     * @tc.steps: step2. device A send message(registered and tiny) to device B using communicator AA
     * @tc.expected: step2. communicator BA received the message
     */
    Message *msgForAA = BuildRegedTinyMessage();
    ASSERT_NE(msgForAA, nullptr);
    int errCode = g_commAA->SendMessage(DEVICE_NAME_B, msgForAA, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // sleep 200 ms
    EXPECT_EQ(recvMsgForBB, nullptr);
    EXPECT_EQ(srcTargetForBA, DEVICE_NAME_A);
    ASSERT_NE(recvMsgForBA, nullptr);
    EXPECT_EQ(recvMsgForBA->GetMessageId(), REGED_TINY_MSG_ID);
    EXPECT_EQ(recvMsgForBA->GetMessageType(), TYPE_REQUEST);
    EXPECT_EQ(recvMsgForBA->GetSessionId(), FIXED_SESSIONID);
    EXPECT_EQ(recvMsgForBA->GetSequenceId(), FIXED_SEQUENCEID);
    EXPECT_EQ(recvMsgForBA->GetErrorNo(), NO_ERROR);
    delete recvMsgForBA;
    recvMsgForBA = nullptr;

    /**
     * @tc.steps: step3. device B send message(registered and tiny) to device A using communicator BB
     * @tc.expected: step3. communicator AA did not receive the message
     */
    Message *msgForBB = BuildRegedTinyMessage();
    ASSERT_NE(msgForBB, nullptr);
    errCode = g_commBB->SendMessage(DEVICE_NAME_A, msgForBB, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(srcTargetForAA, "");

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Send And Receive 002
 * @tc.desc: Test send oversize message will fail
 * @tc.type: FUNC
 * @tc.require: AR000BVDGK AR000CQE0O
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, SendAndReceive002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);

    /**
     * @tc.steps: step2. device A send message(registered and oversize) to device B using communicator AA
     * @tc.expected: step2. send fail
     */
    Message *msgForAA = BuildRegedOverSizeMessage();
    ASSERT_NE(msgForAA, nullptr);
    int errCode = g_commAA->SendMessage(DEVICE_NAME_B, msgForAA, true, 0);
    EXPECT_NE(errCode, E_OK);
    delete msgForAA;
    msgForAA = nullptr;

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Send And Receive 003
 * @tc.desc: Test send unregistered message will fail
 * @tc.type: FUNC
 * @tc.require: AR000BVDGK AR000CQE0O
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, SendAndReceive003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);

    /**
     * @tc.steps: step2. device A send message(unregistered and tiny) to device B using communicator AA
     * @tc.expected: step2. send fail
     */
    Message *msgForAA = BuildUnRegedTinyMessage();
    ASSERT_NE(msgForAA, nullptr);
    int errCode = g_commAA->SendMessage(DEVICE_NAME_B, msgForAA, true, 0);
    EXPECT_NE(errCode, E_OK);
    delete msgForAA;
    msgForAA = nullptr;

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Send Flow Control 001
 * @tc.desc: Test send in nonblock way
 * @tc.type: FUNC
 * @tc.require: AR000BVDGI AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, SendFlowControl001, TestSize.Level1)
{
    // Preset
    int countForBA = 0;
    int countForBB = 0;
    g_commBA->RegOnSendableCallback([&countForBA](){countForBA++;}, nullptr);
    g_commBB->RegOnSendableCallback([&countForBB](){countForBB++;}, nullptr);

    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait 100 ms to make sure send cause by online done
    countForBA = 0;
    countForBB = 0;

    /**
     * @tc.steps: step2. device B simulates send block
     */
    g_envDeviceB.adapterHandle->SimulateSendBlock();

    /**
     * @tc.steps: step3. device B send as much as possible message(unregistered and huge) in nonblock way
     *                   to device A using communicator BA until send fail;
     * @tc.expected: step3. send fail will happen.
     */
    int sendCount = 0;
    while (true) {
        Message *msgForBA = BuildRegedHugeMessage();
        ASSERT_NE(msgForBA, nullptr);
        int errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, true, 0);
        if (errCode == E_OK) {
            sendCount++;
        } else {
            delete msgForBA;
            msgForBA = nullptr;
            break;
        }
    }

    /**
     * @tc.steps: step4. device B simulates send block terminate
     * @tc.expected: step4. send count before fail is equal as expected. sendable callback happened.
     */
    g_envDeviceB.adapterHandle->SimulateSendBlockClear();
    int expectSendCount = MAX_CAPACITY / (HUGE_SIZE + HEADER_SIZE) +
        (MAX_CAPACITY % (HUGE_SIZE + HEADER_SIZE) == 0 ? 0 : 1);
    EXPECT_EQ(sendCount, expectSendCount);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EXPECT_GE(countForBA, 1);
    EXPECT_GE(countForBB, 1);

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Send Flow Control 002
 * @tc.desc: Test send in block(without timeout) way
 * @tc.type: FUNC
 * @tc.require: AR000BVDGI AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, SendFlowControl002, TestSize.Level1)
{
    // Preset
    int cntForBA = 0;
    int cntForBB = 0;
    g_commBA->RegOnSendableCallback([&cntForBA](){cntForBA++;}, nullptr);
    g_commBB->RegOnSendableCallback([&cntForBB](){cntForBB++;}, nullptr);

    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait 100 ms to make sure send cause by online done
    cntForBA = 0;
    cntForBB = 0;

    /**
     * @tc.steps: step2. device B simulates send block
     */
    g_envDeviceB.adapterHandle->SimulateSendBlock();

    /**
     * @tc.steps: step3. device B send a certain message(unregistered and huge) in block way
     *                   without timeout to device A using communicator BA;
     */
    int sendCount = 0;
    int sendFailCount = 0;
    std::thread sendThread([&sendCount, &sendFailCount](){
        while (sendCount < SEND_COUNT_GOAL) {
            Message *msgForBA = BuildRegedHugeMessage();
            ASSERT_NE(msgForBA, nullptr);
            int errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, false, 0);
            if (errCode != E_OK) {
                delete msgForBA;
                msgForBA = nullptr;
                sendFailCount++;
            }
            sendCount++;
        }
    });

    /**
     * @tc.steps: step4. device B simulates send block terminate
     * @tc.expected: step4. send fail count is zero. sendable callback happened.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    g_envDeviceB.adapterHandle->SimulateSendBlockClear();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    sendThread.join();
    EXPECT_EQ(sendCount, SEND_COUNT_GOAL);
    EXPECT_EQ(sendFailCount, 0);
    EXPECT_GE(cntForBA, 1);
    EXPECT_GE(cntForBB, 1);

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Send Flow Control 003
 * @tc.desc: Test send in block(with timeout) way
 * @tc.type: FUNC
 * @tc.require: AR000BVDGI AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, SendFlowControl003, TestSize.Level1)
{
    // Preset
    int cntsForBA = 0;
    int cntsForBB = 0;
    g_commBA->RegOnSendableCallback([&cntsForBA](){cntsForBA++;}, nullptr);
    g_commBB->RegOnSendableCallback([&cntsForBB](){cntsForBB++;}, nullptr);

    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cntsForBA = 0;
    cntsForBB = 0;

    /**
     * @tc.steps: step2. device B simulates send block
     */
    g_envDeviceB.adapterHandle->SimulateSendBlock();

     /**
     * @tc.steps: step3. device B send a certain message(unregistered and huge) in block way
     *                   with timeout to device A using communicator BA;
     */
    int sendCnt = 0;
    int sendFailCnt = 0;
    std::thread sendThread([&sendCnt, &sendFailCnt](){
        while (sendCnt < SEND_COUNT_GOAL) {
            Message *msgForBA = BuildRegedHugeMessage();
            ASSERT_NE(msgForBA, nullptr);
            int errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, false, 100); // 100 ms timeout
            if (errCode != E_OK) {
                delete msgForBA;
                msgForBA = nullptr;
                sendFailCnt++;
            }
            sendCnt++;
        }
    });

    /**
     * @tc.steps: step4. device B simulates send block terminate
     * @tc.expected: step4. send fail count is no more than expected. sendable callback happened.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // wait 300 ms
    g_envDeviceB.adapterHandle->SimulateSendBlockClear();
    std::this_thread::sleep_for(std::chrono::milliseconds(1200)); // wait 1200 ms
    sendThread.join();
    EXPECT_EQ(sendCnt, SEND_COUNT_GOAL);
    EXPECT_LE(sendFailCnt, 4);
    EXPECT_GE(cntsForBA, 1);
    EXPECT_GE(cntsForBB, 1);

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Receive Check 001
 * @tc.desc: Receive packet field check
 * @tc.type: FUNC
 * @tc.require: AR000BVRNU AR000CQE0J
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, ReceiveCheck001, TestSize.Level1)
{
    // Preset
    int recvCount = 0;
    g_commAA->RegOnMessageCallback([&recvCount](const std::string &srcTarget, Message *inMsg){
        recvCount++;
        if (inMsg != nullptr) {
            delete inMsg;
            inMsg = nullptr;
        }
    }, nullptr);
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);

    /**
     * @tc.steps: step1. create packet with magic field error
     * @tc.expected: step1. no message callback
     */
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInMagicField(true, 0xFFFF);
    Message *msgForBA = BuildRegedTinyMessage();
    int errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(recvCount, 0);
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInMagicField(false, 0);

    /**
     * @tc.steps: step2. create packet with version field error
     * @tc.expected: step2. no message callback
     */
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInVersionField(true, 0xFFFF);
    msgForBA = BuildRegedTinyMessage();
    errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(recvCount, 0);
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInVersionField(false, 0);

    /**
     * @tc.steps: step3. create packet with checksum field error
     * @tc.expected: step3. no message callback
     */
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInCheckSumField(true, 0xFFFF);
    msgForBA = BuildRegedTinyMessage();
    errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(recvCount, 0);
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInCheckSumField(false, 0);

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Receive Check 002
 * @tc.desc: Receive packet field check
 * @tc.type: FUNC
 * @tc.require: AR000BVRNU AR000CQE0J
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, ReceiveCheck002, TestSize.Level1)
{
    // Preset
    int recvCount = 0;
    g_commAA->RegOnMessageCallback([&recvCount](const std::string &srcTarget, Message *inMsg){
        recvCount++;
        if (inMsg != nullptr) {
            delete inMsg;
            inMsg = nullptr;
        }
    }, nullptr);
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);

    /**
     * @tc.steps: step1. create packet with packetLen field error
     * @tc.expected: step1. no message callback
     */
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInPacketLenField(true, 0xFFFF);
    Message *msgForBA = BuildRegedTinyMessage();
    int errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(recvCount, 0);
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInPacketLenField(false, 0);

    /**
     * @tc.steps: step1. create packet with packetType field error
     * @tc.expected: step1. no message callback
     */
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInPacketTypeField(true, 0xFF);
    msgForBA = BuildRegedTinyMessage();
    errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(recvCount, 0);
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInPacketTypeField(false, 0);

    /**
     * @tc.steps: step1. create packet with paddingLen field error
     * @tc.expected: step1. no message callback
     */
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInPaddingLenField(true, 0xFF);
    msgForBA = BuildRegedTinyMessage();
    errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(recvCount, 0);
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInPaddingLenField(false, 0);

    // CleanUp
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Send Result Notify 001
 * @tc.desc: Test send result notify
 * @tc.type: FUNC
 * @tc.require: AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, SendResultNotify001, TestSize.Level1)
{
    // preset
    std::vector<int> sendResult;
    auto sendResultNotifier = [&sendResult](int result) {
        sendResult.push_back(result);
    };

    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);

    /**
     * @tc.steps: step2. device A send message to device B using communicator AA
     * @tc.expected: step2. notify send done and success
     */
    Message *msgForAA = BuildRegedTinyMessage();
    ASSERT_NE(msgForAA, nullptr);
    int errCode = g_commAA->SendMessage(DEVICE_NAME_B, msgForAA, false, 0, sendResultNotifier);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    ASSERT_EQ(sendResult.size(), static_cast<size_t>(1)); // 1 notify
    EXPECT_EQ(sendResult[0], E_OK);

    /**
     * @tc.steps: step3. disconnect device A with device B
     */
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);

    /**
     * @tc.steps: step4. device A send message to device B using communicator AA
     * @tc.expected: step2. notify send done and fail
     */
    msgForAA = BuildRegedTinyMessage();
    ASSERT_NE(msgForAA, nullptr);
    errCode = g_commAA->SendMessage(DEVICE_NAME_B, msgForAA, false, 0, sendResultNotifier);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    ASSERT_EQ(sendResult.size(), static_cast<size_t>(2)); // 2 notify
    EXPECT_NE(sendResult[1], E_OK); // 1 for second element
}

#define REG_MESSAGE_CALLBACK(src, label) \
    string srcTargetFor##src##label; \
    Message *recvMsgFor##src##label = nullptr; \
    g_comm##src##label->RegOnMessageCallback( \
        [&srcTargetFor##src##label, &recvMsgFor##src##label](const std::string &srcTarget, Message *inMsg) { \
        srcTargetFor##src##label = srcTarget; \
        recvMsgFor##src##label = inMsg; \
    }, nullptr);

/**
 * @tc.name: Message Feedback 001
 * @tc.desc: Test feedback not support messageid and communicator not found
 * @tc.type: FUNC
 * @tc.require: AR000CQE0M
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorSendReceiveTest, MessageFeedback001, TestSize.Level1)
{
    CommunicatorAggregator::EnableCommunicatorNotFoundFeedback(true);
    // preset
    REG_MESSAGE_CALLBACK(A, A);
    REG_MESSAGE_CALLBACK(B, A);
    REG_MESSAGE_CALLBACK(B, B);

    /**
     * @tc.steps: step1. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);

    /**
     * @tc.steps: step2. device B send message to device A using communicator BB
     * @tc.expected: step2. communicator BB receive communicator not found feedback
     */
    Message *msgForBB = BuildRegedTinyMessage();
    ASSERT_NE(msgForBB, nullptr);
    int errCode = g_commBB->SendMessage(DEVICE_NAME_A, msgForBB, false, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    ASSERT_NE(recvMsgForBB, nullptr);
    EXPECT_EQ(srcTargetForBB, DEVICE_NAME_A);
    EXPECT_EQ(recvMsgForBB->GetMessageId(), REGED_TINY_MSG_ID);
    EXPECT_EQ(recvMsgForBB->GetMessageType(), TYPE_RESPONSE);
    EXPECT_EQ(recvMsgForBB->GetSessionId(), FIXED_SESSIONID);
    EXPECT_EQ(recvMsgForBB->GetSequenceId(), FIXED_SEQUENCEID);
    EXPECT_EQ(recvMsgForBB->GetErrorNo(), static_cast<uint32_t>(E_FEEDBACK_COMMUNICATOR_NOT_FOUND));
    EXPECT_EQ(recvMsgForBB->GetObject<RegedTinyObject>(), nullptr);
    delete recvMsgForBB;
    recvMsgForBB = nullptr;

    /**
     * @tc.steps: step3. simulate messageid not registered
     */
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInMessageIdField(true, UNREGED_TINY_MSG_ID);

    /**
     * @tc.steps: step4. device B send message to device A using communicator BA
     * @tc.expected: step4. communicator BA receive messageid not register feedback
     */
    Message *msgForBA = BuildRegedTinyMessage();
    ASSERT_NE(msgForBA, nullptr);
    errCode = g_commBA->SendMessage(DEVICE_NAME_A, msgForBA, false, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    ASSERT_NE(recvMsgForBA, nullptr);
    EXPECT_EQ(srcTargetForBA, DEVICE_NAME_A);
    EXPECT_EQ(recvMsgForBA->GetMessageId(), UNREGED_TINY_MSG_ID);
    EXPECT_EQ(recvMsgForBA->GetMessageType(), TYPE_RESPONSE);
    EXPECT_EQ(recvMsgForBA->GetSessionId(), FIXED_SESSIONID);
    EXPECT_EQ(recvMsgForBA->GetSequenceId(), FIXED_SEQUENCEID);
    EXPECT_EQ(recvMsgForBA->GetErrorNo(), static_cast<uint32_t>(E_FEEDBACK_UNKNOWN_MESSAGE));
    EXPECT_EQ(recvMsgForBA->GetObject<RegedTinyObject>(), nullptr);
    delete recvMsgForBA;
    recvMsgForBA = nullptr;

    // CleanUp
    g_envDeviceB.adapterHandle->SimulateSendBitErrorInMessageIdField(false, 0);
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    CommunicatorAggregator::EnableCommunicatorNotFoundFeedback(false);
}