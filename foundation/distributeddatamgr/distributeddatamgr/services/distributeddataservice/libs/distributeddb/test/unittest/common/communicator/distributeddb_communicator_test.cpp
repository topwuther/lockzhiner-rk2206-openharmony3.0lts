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
#include "log_print.h"
#include "db_errno.h"
#include "endian_convert.h"
#include "distributeddb_communicator_common.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;

namespace {
    EnvHandle g_envDeviceA;
    EnvHandle g_envDeviceB;
    EnvHandle g_envDeviceC;
}

static void HandleConnectChange(OnOfflineDevice &onlines, const std::string &target, bool isConnect)
{
    if (isConnect) {
        onlines.onlineDevices.insert(target);
        onlines.latestOnlineDevice = target;
        onlines.latestOfflineDevice.clear();
    } else {
        onlines.onlineDevices.erase(target);
        onlines.latestOnlineDevice.clear();
        onlines.latestOfflineDevice = target;
    }
}

class DistributedDBCommunicatorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBCommunicatorTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Create and init CommunicatorAggregator and AdapterStub
     */
    LOGI("[UT][Test][SetUpTestCase] Enter.");
    bool errCode = SetUpEnv(g_envDeviceA, DEVICE_NAME_A);
    ASSERT_EQ(errCode, true);
    errCode = SetUpEnv(g_envDeviceB, DEVICE_NAME_B);
    ASSERT_EQ(errCode, true);
    DoRegTransformFunction();
    CommunicatorAggregator::EnableCommunicatorNotFoundFeedback(false);
}

void DistributedDBCommunicatorTest::TearDownTestCase(void)
{
    /**
     * @tc.teardown: Finalize and release CommunicatorAggregator and AdapterStub
     */
    LOGI("[UT][Test][TearDownTestCase] Enter.");
    std::this_thread::sleep_for(std::chrono::seconds(7)); // Wait 7 s to make sure all thread quiet and memory released
    TearDownEnv(g_envDeviceA);
    TearDownEnv(g_envDeviceB);
    CommunicatorAggregator::EnableCommunicatorNotFoundFeedback(true);
}

void DistributedDBCommunicatorTest::SetUp()
{
    /**
     * @tc.setup: Do nothing
     */
}

void DistributedDBCommunicatorTest::TearDown()
{
    /**
     * @tc.teardown: Wait 100 ms to make sure all thread quiet
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait 100 ms
}

/**
 * @tc.name: Communicator Management 001
 * @tc.desc: Test alloc and release communicator
 * @tc.type: FUNC
 * @tc.require: AR000BVDGG AR000CQE0L
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorTest, CommunicatorManagement001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. alloc communicator A using label A
     * @tc.expected: step1. alloc return OK.
     */
    int errorNo = E_OK;
    ICommunicator *commA = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    EXPECT_EQ(errorNo, E_OK);
    EXPECT_NE(commA, nullptr);

    /**
     * @tc.steps: step2. alloc communicator B using label B
     * @tc.expected: step2. alloc return OK.
     */
    errorNo = E_OK;
    ICommunicator *commB = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_B, errorNo);
    EXPECT_EQ(errorNo, E_OK);
    EXPECT_NE(commA, nullptr);

    /**
     * @tc.steps: step3. alloc communicator C using label A
     * @tc.expected: step3. alloc return not OK.
     */
    errorNo = E_OK;
    ICommunicator *commC = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    EXPECT_NE(errorNo, E_OK);
    EXPECT_EQ(commC, nullptr);

    /**
     * @tc.steps: step4. release communicator A and communicator B
     */
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commA);
    commA = nullptr;
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commB);
    commB = nullptr;

    /**
     * @tc.steps: step5. alloc communicator D using label A
     * @tc.expected: step5. alloc return OK.
     */
    errorNo = E_OK;
    ICommunicator *commD = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    EXPECT_EQ(errorNo, E_OK);
    EXPECT_NE(commD, nullptr);

    /**
     * @tc.steps: step6. release communicator D
     */
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commD);
    commD = nullptr;
}

static void ConnectWaitDisconnect()
{
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Online And Offline 001
 * @tc.desc: Test functionality triggered by physical devices online and offline
 * @tc.type: FUNC
 * @tc.require: AR000BVRNS AR000CQE0H
 * @tc.author: wudongxing
 */
HWTEST_F(DistributedDBCommunicatorTest, OnlineAndOffline001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device A alloc communicator AA using label A and register callback
     * @tc.expected: step1. no callback.
     */
    int errorNo = E_OK;
    ICommunicator *commAA = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(commAA);
    OnOfflineDevice onlineForAA;
    commAA->RegOnConnectCallback([&onlineForAA](const std::string &target, bool isConnect){
        HandleConnectChange(onlineForAA, target, isConnect);}, nullptr);
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(0));

    /**
     * @tc.steps: step2. connect device A with device B and then disconnect
     * @tc.expected: step2. no callback.
     */
    ConnectWaitDisconnect();
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(0));

    /**
     * @tc.steps: step3. device B alloc communicator BB using label B and register callback
     * @tc.expected: step3. no callback.
     */
    ICommunicator *commBB = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_B, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(commBB);
    OnOfflineDevice onlineForBB;
    commBB->RegOnConnectCallback([&onlineForBB](const std::string &target, bool isConnect){
        HandleConnectChange(onlineForBB, target, isConnect);}, nullptr);
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(0));

    /**
     * @tc.steps: step4. connect device A with device B and then disconnect
     * @tc.expected: step4. no callback.
     */
    ConnectWaitDisconnect();
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(0));

    /**
     * @tc.steps: step5. device B alloc communicator BA using label A and register callback
     * @tc.expected: step5. no callback.
     */
    ICommunicator *commBA = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(commBA);
    OnOfflineDevice onlineForBA;
    commBA->RegOnConnectCallback([&onlineForBA](const std::string &target, bool isConnect){
        HandleConnectChange(onlineForBA, target, isConnect);}, nullptr);
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForBA.onlineDevices.size(), static_cast<size_t>(0));

    /**
     * @tc.steps: step6. connect device A with device B
     * @tc.expected: step6. communicator AA has callback of device B online;
     *                      communicator BA has callback of device A online;
     *                      communicator BB no callback
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForBA.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForAA.latestOnlineDevice, DEVICE_NAME_B);
    EXPECT_EQ(onlineForBA.latestOnlineDevice, DEVICE_NAME_A);

    /**
     * @tc.steps: step7. disconnect device A with device B
     * @tc.expected: step7. communicator AA has callback of device B offline;
     *                      communicator BA has callback of device A offline;
     *                      communicator BB no callback
     */
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForBA.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForAA.latestOfflineDevice, DEVICE_NAME_B);
    EXPECT_EQ(onlineForBA.latestOfflineDevice, DEVICE_NAME_A);

    // Clean up
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAA);
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(commBB);
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(commBA);
}

#define REG_CONNECT_CALLBACK(communicator, online) \
{ \
    communicator->RegOnConnectCallback([&online](const std::string &target, bool isConnect) { \
        HandleConnectChange(online, target, isConnect); \
    }, nullptr); \
}

#define CONNECT_AND_WAIT(waitTime) \
{ \
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle); \
    std::this_thread::sleep_for(std::chrono::milliseconds(waitTime)); \
}

/**
 * @tc.name: Online And Offline 002
 * @tc.desc: Test functionality triggered by alloc and release communicator
 * @tc.type: FUNC
 * @tc.require: AR000BVRNT AR000CQE0I
 * @tc.author: wudongxing
 */
HWTEST_F(DistributedDBCommunicatorTest, OnlineAndOffline002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. connect device A with device B
     */
    CONNECT_AND_WAIT(200); // Sleep 200 ms

    /**
     * @tc.steps: step2. device A alloc communicator AA using label A and register callback
     * @tc.expected: step2. no callback.
     */
    int errorNo = E_OK;
    ICommunicator *commAA = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(commAA);
    OnOfflineDevice onlineForAA;
    REG_CONNECT_CALLBACK(commAA, onlineForAA);
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(0));

    /**
     * @tc.steps: step3. device B alloc communicator BB using label B and register callback
     * @tc.expected: step3. no callback.
     */
    ICommunicator *commBB = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_B, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(commBB);
    OnOfflineDevice onlineForBB;
    REG_CONNECT_CALLBACK(commBB, onlineForBB);
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(0));

    /**
     * @tc.steps: step4. device B alloc communicator BA using label A and register callback
     * @tc.expected: step4. communicator AA has callback of device B online;
     *                      communicator BA has callback of device A online;
     *                      communicator BB no callback.
     */
    ICommunicator *commBA = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_A, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(commBA);
    OnOfflineDevice onlineForBA;
    REG_CONNECT_CALLBACK(commBA, onlineForBA);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(onlineForAA.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForBA.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForAA.latestOnlineDevice, DEVICE_NAME_B);
    EXPECT_EQ(onlineForBA.latestOnlineDevice, DEVICE_NAME_A);

    /**
     * @tc.steps: step5. device A alloc communicator AB using label B and register callback
     * @tc.expected: step5. communicator AB has callback of device B online;
     *                      communicator BB has callback of device A online;
     */
    ICommunicator *commAB = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_B, errorNo);
    ASSERT_NOT_NULL_AND_ACTIVATE(commAB);
    OnOfflineDevice onlineForAB;
    REG_CONNECT_CALLBACK(commAB, onlineForAB);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(onlineForAB.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForAB.latestOnlineDevice, DEVICE_NAME_B);
    EXPECT_EQ(onlineForBB.latestOnlineDevice, DEVICE_NAME_A);

    /**
     * @tc.steps: step6. device A release communicator AA
     * @tc.expected: step6. communicator BA has callback of device A offline;
     *                      communicator AB and BB no callback;
     */
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAA);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(onlineForBA.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForAB.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForBA.latestOfflineDevice, DEVICE_NAME_A);

    /**
     * @tc.steps: step7. device B release communicator BA
     * @tc.expected: step7. communicator AB and BB no callback;
     */
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(commBA);
    EXPECT_EQ(onlineForAB.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForBB.onlineDevices.size(), static_cast<size_t>(1));

    /**
     * @tc.steps: step8. device B release communicator BB
     * @tc.expected: step8. communicator AB has callback of device B offline;
     */
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(commBB);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(onlineForAB.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForAB.latestOfflineDevice, DEVICE_NAME_B);

    // Clean up
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAB);
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: Report Device Connect Change 001
 * @tc.desc: Test CommunicatorAggregator support report device connect change event
 * @tc.type: FUNC
 * @tc.require: AR000DR9KV
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorTest, ReportDeviceConnectChange001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device A and device B register connect callback to CommunicatorAggregator
     */
    OnOfflineDevice onlineForA;
    int errCode = g_envDeviceA.commAggrHandle->RegOnConnectCallback(
        [&onlineForA](const std::string &target, bool isConnect) {
            HandleConnectChange(onlineForA, target, isConnect);
        }, nullptr);
    EXPECT_EQ(errCode, E_OK);
    OnOfflineDevice onlineForB;
    errCode = g_envDeviceB.commAggrHandle->RegOnConnectCallback(
        [&onlineForB](const std::string &target, bool isConnect) {
            HandleConnectChange(onlineForB, target, isConnect);
        }, nullptr);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step2. connect device A with device B
     * @tc.expected: step2. device A callback B online; device B callback A online;
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    EXPECT_EQ(onlineForA.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForB.onlineDevices.size(), static_cast<size_t>(1));
    EXPECT_EQ(onlineForA.latestOnlineDevice, DEVICE_NAME_B);
    EXPECT_EQ(onlineForB.latestOnlineDevice, DEVICE_NAME_A);

    /**
     * @tc.steps: step3. connect device A with device B
     * @tc.expected: step3. device A callback B offline; device B callback A offline;
     */
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    EXPECT_EQ(onlineForA.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForB.onlineDevices.size(), static_cast<size_t>(0));
    EXPECT_EQ(onlineForA.latestOfflineDevice, DEVICE_NAME_B);
    EXPECT_EQ(onlineForB.latestOfflineDevice, DEVICE_NAME_A);

    // Clean up
    g_envDeviceA.commAggrHandle->RegOnConnectCallback(nullptr, nullptr);
    g_envDeviceB.commAggrHandle->RegOnConnectCallback(nullptr, nullptr);
}

namespace {
LabelType ToLabelType(uint64_t commLabel)
{
    uint64_t netOrderLabel = HostToNet(commLabel);
    uint8_t *eachByte = reinterpret_cast<uint8_t *>(&netOrderLabel);
    std::vector<uint8_t> realLabel(COMM_LABEL_LENGTH, 0);
    for (int i = 0; i < static_cast<int>(sizeof(uint64_t)); i++) {
        realLabel[i] = eachByte[i];
    }
    return realLabel;
}
}

/**
 * @tc.name: Report Communicator Not Found 001
 * @tc.desc: Test CommunicatorAggregator support report communicator not found event
 * @tc.type: FUNC
 * @tc.require: AR000DR9KV
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorTest, ReportCommunicatorNotFound001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device B register communicator not found callback to CommunicatorAggregator
     */
    std::vector<LabelType> lackLabels;
    int errCode = g_envDeviceB.commAggrHandle->RegCommunicatorLackCallback(
        [&lackLabels](const LabelType &commLabel)->int {
            lackLabels.push_back(commLabel);
            return -E_NOT_FOUND;
        }, nullptr);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step2. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms

    /**
     * @tc.steps: step3. device A alloc communicator AA using label A and send message to B
     * @tc.expected: step3. device B callback that label A not found.
     */
    ICommunicator *commAA = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_A, errCode);
    ASSERT_NOT_NULL_AND_ACTIVATE(commAA);
    Message *msgForAA = BuildRegedTinyMessage();
    ASSERT_NE(msgForAA, nullptr);
    errCode = commAA->SendMessage(DEVICE_NAME_B, msgForAA, true, 0);
    EXPECT_EQ(errCode, E_OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    ASSERT_EQ(lackLabels.size(), static_cast<size_t>(1));
    EXPECT_EQ(lackLabels[0], ToLabelType(LABEL_A));

    /**
     * @tc.steps: step4. device B alloc communicator BA using label A and register message callback
     * @tc.expected: step4. communicator BA will not receive message.
     */
    ICommunicator *commBA = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_A, errCode);
    ASSERT_NE(commBA, nullptr);
    Message *recvMsgForBA = nullptr;
    commBA->RegOnMessageCallback([&recvMsgForBA](const std::string &srcTarget, Message *inMsg) {
        recvMsgForBA = inMsg;
    }, nullptr);
    commBA->Activate();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    EXPECT_EQ(recvMsgForBA, nullptr);

    // Clean up
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAA);
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(commBA);
    g_envDeviceB.commAggrHandle->RegCommunicatorLackCallback(nullptr, nullptr);
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

#define DO_SEND_MESSAGE(src, dst, label, session) \
{ \
    Message *msgFor##src##label = BuildRegedTinyMessage(); \
    ASSERT_NE(msgFor##src##label, nullptr); \
    msgFor##src##label->SetSessionId(session); \
    errCode = comm##src##label->SendMessage(DEVICE_NAME_##dst, msgFor##src##label, true, 0); \
    EXPECT_EQ(errCode, E_OK); \
}

#define DO_SEND_GIANT_MESSAGE(src, dst, label, size) \
{ \
    Message *msgFor##src##label = BuildRegedGiantMessage(size); \
    ASSERT_NE(msgFor##src##label, nullptr); \
    errCode = comm##src##label->SendMessage(DEVICE_NAME_##dst, msgFor##src##label, false, 0); \
    EXPECT_EQ(errCode, E_OK); \
}

#define ALLOC_AND_SEND_MESSAGE(src, dst, label, session) \
    ICommunicator *comm##src##label = g_envDevice##src.commAggrHandle->AllocCommunicator(LABEL_##label, errCode); \
    ASSERT_NOT_NULL_AND_ACTIVATE(comm##src##label); \
    DO_SEND_MESSAGE(src, dst, label, session)

#define REG_MESSAGE_CALLBACK(src, label) \
    string srcTargetFor##src##label; \
    Message *recvMsgFor##src##label = nullptr; \
    comm##src##label->RegOnMessageCallback( \
        [&srcTargetFor##src##label, &recvMsgFor##src##label](const std::string &srcTarget, Message *inMsg) { \
        srcTargetFor##src##label = srcTarget; \
        recvMsgFor##src##label = inMsg; \
    }, nullptr);

/**
 * @tc.name: ReDeliver Message 001
 * @tc.desc: Test CommunicatorAggregator support redeliver message
 * @tc.type: FUNC
 * @tc.require: AR000DR9KV
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorTest, ReDeliverMessage001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device B register communicator not found callback to CommunicatorAggregator
     */
    std::vector<LabelType> lackLabels;
    int errCode = g_envDeviceB.commAggrHandle->RegCommunicatorLackCallback(
        [&lackLabels](const LabelType &commLabel)->int {
            lackLabels.push_back(commLabel);
            return E_OK;
        }, nullptr);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step2. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms

    /**
     * @tc.steps: step3. device A alloc communicator AA using label A and send message to B
     * @tc.expected: step3. device B callback that label A not found.
     */
    ALLOC_AND_SEND_MESSAGE(A, B, A, 100); // session id 100
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    ASSERT_EQ(lackLabels.size(), static_cast<size_t>(1));
    EXPECT_EQ(lackLabels[0], ToLabelType(LABEL_A));

    /**
     * @tc.steps: step4. device A alloc communicator AB using label B and send message to B
     * @tc.expected: step4. device B callback that label B not found.
     */
    ALLOC_AND_SEND_MESSAGE(A, B, B, 200); // session id 200
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    ASSERT_EQ(lackLabels.size(), static_cast<size_t>(2));
    EXPECT_EQ(lackLabels[1], ToLabelType(LABEL_B)); // 1 for second element

    /**
     * @tc.steps: step5. device B alloc communicator BA using label A and register message callback
     * @tc.expected: step5. communicator BA will receive message.
     */
    ICommunicator *commBA = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_A, errCode);
    ASSERT_NE(commBA, nullptr);
    REG_MESSAGE_CALLBACK(B, A);
    commBA->Activate();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    EXPECT_EQ(srcTargetForBA, DEVICE_NAME_A);
    ASSERT_NE(recvMsgForBA, nullptr);
    EXPECT_EQ(recvMsgForBA->GetSessionId(), 100U); // session id 100
    delete recvMsgForBA;
    recvMsgForBA = nullptr;

    /**
     * @tc.steps: step6. device B alloc communicator BB using label B and register message callback
     * @tc.expected: step6. communicator BB will receive message.
     */
    ICommunicator *commBB = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_B, errCode);
    ASSERT_NE(commBB, nullptr);
    REG_MESSAGE_CALLBACK(B, B);
    commBB->Activate();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    EXPECT_EQ(srcTargetForBB, DEVICE_NAME_A);
    ASSERT_NE(recvMsgForBB, nullptr);
    EXPECT_EQ(recvMsgForBB->GetSessionId(), 200U); // session id 200
    delete recvMsgForBB;
    recvMsgForBB = nullptr;

    // Clean up
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAA);
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAB);
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(commBA);
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(commBB);
    g_envDeviceB.commAggrHandle->RegCommunicatorLackCallback(nullptr, nullptr);
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}

/**
 * @tc.name: ReDeliver Message 002
 * @tc.desc: Test CommunicatorAggregator support redeliver message by order
 * @tc.type: FUNC
 * @tc.require: AR000DR9KV
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorTest, ReDeliverMessage002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. device C create CommunicatorAggregator and initialize
     */
    bool step1 = SetUpEnv(g_envDeviceC, DEVICE_NAME_C);
    ASSERT_EQ(step1, true);

    /**
     * @tc.steps: step2. device B register communicator not found callback to CommunicatorAggregator
     */
    int errCode = g_envDeviceB.commAggrHandle->RegCommunicatorLackCallback([](const LabelType &commLabel)->int {
        return E_OK;
    }, nullptr);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step3. connect device A with device B, then device B with device C
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    AdapterStub::ConnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms

    /**
     * @tc.steps: step4. device A alloc communicator AA using label A and send message to B
     */
    ALLOC_AND_SEND_MESSAGE(A, B, A, 100); // session id 100
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms

    /**
     * @tc.steps: step5. device C alloc communicator CA using label A and send message to B
     */
    ALLOC_AND_SEND_MESSAGE(C, B, A, 200); // session id 200
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    DO_SEND_MESSAGE(A, B, A, 300); // session id 300
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    DO_SEND_MESSAGE(C, B, A, 400); // session id 400
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms

    /**
     * @tc.steps: step6. device B alloc communicator BA using label A and register message callback
     * @tc.expected: step6. communicator BA will receive message in order of sessionid 100, 200, 300, 400.
     */
    ICommunicator *commBA = g_envDeviceB.commAggrHandle->AllocCommunicator(LABEL_A, errCode);
    ASSERT_NE(commBA, nullptr);
    std::vector<std::pair<std::string, Message *>> msgCallbackForBA;
    commBA->RegOnMessageCallback([&msgCallbackForBA](const std::string &srcTarget, Message *inMsg) {
        msgCallbackForBA.push_back({srcTarget, inMsg});
    }, nullptr);
    commBA->Activate();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms
    ASSERT_EQ(msgCallbackForBA.size(), static_cast<size_t>(4)); // total 4 callback
    EXPECT_EQ(msgCallbackForBA[0].first, DEVICE_NAME_A); // the 0 order element
    EXPECT_EQ(msgCallbackForBA[1].first, DEVICE_NAME_C); // the 1 order element
    EXPECT_EQ(msgCallbackForBA[2].first, DEVICE_NAME_A); // the 2 order element
    EXPECT_EQ(msgCallbackForBA[3].first, DEVICE_NAME_C); // the 3 order element
    for (uint32_t i = 0; i < msgCallbackForBA.size(); i++) {
        EXPECT_EQ(msgCallbackForBA[i].second->GetSessionId(), static_cast<uint32_t>((i + 1) * 100)); // 1 sessionid 100
        delete msgCallbackForBA[i].second;
        msgCallbackForBA[i].second = nullptr;
    }

    // Clean up
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAA);
    g_envDeviceC.commAggrHandle->ReleaseCommunicator(commCA);
    g_envDeviceB.commAggrHandle->ReleaseCommunicator(commBA);
    g_envDeviceB.commAggrHandle->RegCommunicatorLackCallback(nullptr, nullptr);
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    AdapterStub::DisconnectAdapterStub(g_envDeviceB.adapterHandle, g_envDeviceC.adapterHandle);
    TearDownEnv(g_envDeviceC);
}

/**
 * @tc.name: ReDeliver Message 003
 * @tc.desc: For observe memory in unusual scenario
 * @tc.type: FUNC
 * @tc.require: AR000DR9KV
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBCommunicatorTest, ReDeliverMessage003, TestSize.Level2)
{
    /**
     * @tc.steps: step1. device B register communicator not found callback to CommunicatorAggregator
     */
    int errCode = g_envDeviceB.commAggrHandle->RegCommunicatorLackCallback([](const LabelType &commLabel)->int {
        return E_OK;
    }, nullptr);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step2. connect device A with device B
     */
    AdapterStub::ConnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms

    /**
     * @tc.steps: step3. device A alloc communicator AA,AB,AC using label A,B,C
     */
    ICommunicator *commAA = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_A, errCode);
    ASSERT_NOT_NULL_AND_ACTIVATE(commAA);
    ICommunicator *commAB = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_B, errCode);
    ASSERT_NOT_NULL_AND_ACTIVATE(commAB);
    ICommunicator *commAC = g_envDeviceA.commAggrHandle->AllocCommunicator(LABEL_C, errCode);
    ASSERT_NOT_NULL_AND_ACTIVATE(commAC);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep 100 ms

    /**
     * @tc.steps: step4. device A Continuously send tiny message to B using communicator AA,AB,AC
     */
    for (int turn = 0; turn < 11; turn++) { // Total 11 turns
        DO_SEND_MESSAGE(A, B, A, 0);
        DO_SEND_MESSAGE(A, B, B, 0);
        DO_SEND_MESSAGE(A, B, C, 0);
    }

    /**
     * @tc.steps: step5. device A Continuously send giant message to B using communicator AA,AB,AC
     */
    for (int turn = 0; turn < 5; turn++) { // Total 5 turns
        DO_SEND_GIANT_MESSAGE(A, B, A, (3 * 1024 * 1024)); // 3 MBytes, 1024 is scale
        DO_SEND_GIANT_MESSAGE(A, B, B, (6 * 1024 * 1024)); // 6 MBytes, 1024 is scale
        DO_SEND_GIANT_MESSAGE(A, B, C, (7 * 1024 * 1024)); // 7 MBytes, 1024 is scale
    }
    DO_SEND_GIANT_MESSAGE(A, B, A, (30 * 1024 * 1024)); // 30 MBytes, 1024 is scale

    /**
     * @tc.steps: step6. wait a long time then send last frame
     */
    for (int sec = 0; sec < 15; sec++) { // Total 15 s
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep 1 s
        LOGI("[UT][Test][ReDeliverMessage003] Sleep and wait=%d.", sec);
    }
    DO_SEND_MESSAGE(A, B, A, 0);
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep 1 s

    // Clean up
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAA);
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAB);
    g_envDeviceA.commAggrHandle->ReleaseCommunicator(commAC);
    g_envDeviceB.commAggrHandle->RegCommunicatorLackCallback(nullptr, nullptr);
    AdapterStub::DisconnectAdapterStub(g_envDeviceA.adapterHandle, g_envDeviceB.adapterHandle);
}
