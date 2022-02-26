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

#include "distributeddb_data_generate_unit_test.h"
#include "message.h"
#include "meta_data.h"
#include "ref_object.h"
#include "single_ver_sync_engine.h"
#include "single_ver_data_sync.h"
#include "vitural_communicator_aggregator.h"
#include "virtual_single_ver_sync_db_Interface.h"
#include "version.h"
#include "generic_single_ver_kv_entry.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    const string ANTI_DOS_STORE_ID = "anti_dos_sync_test";
    const int NUM = 108;
    const int WAIT_LONG_TIME = 26000;
    const int WAIT_SHORT_TIME = 18000;
    const int TEST_ONE = 2;
    const int TEST_TWO = 10;
    const int TEST_THREE_THREAD = 20;
    const int TEST_THREE_OUTDATA = 2048;
    const int TEST_THREE_DATATIEM = 1024;
    const int LIMIT_QUEUE_CACHE_SIZE = 1024 * 1024;
    const int DEFAULT_CACHE_SIZE = 160 * 1024 * 1024; // Initial the default cache size of queue as 160MB
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    KvStoreConfig g_config;
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate* g_kvDelegatePtr = nullptr;
    VirtualCommunicatorAggregator* g_communicatorAggregator = nullptr;
    std::shared_ptr<Metadata> g_metaData = nullptr;
    SingleVerSyncEngine *g_syncEngine = nullptr;
    VirtualCommunicator *g_communicator = nullptr;
    VirtualSingleVerSyncDBInterface *g_syncInterface = nullptr;

    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));
}

class DistributeddbAntiDosSyncTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributeddbAntiDosSyncTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Init datadir and VirtualCommunicatorAggregator.
     */
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);

    g_communicatorAggregator = new (std::nothrow) VirtualCommunicatorAggregator();
    ASSERT_TRUE(g_communicatorAggregator != nullptr);
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(g_communicatorAggregator);
}

void DistributeddbAntiDosSyncTest::TearDownTestCase(void)
{
    /**
     * @tc.teardown: Release VirtualCommunicatorAggregator and clear data dir.
     */
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
    RuntimeContext::GetInstance()->SetCommunicatorAggregator(nullptr);
}

void DistributeddbAntiDosSyncTest::SetUp(void)
{
    /**
     * @tc.setup: create VirtualCommunicator, VirtualSingleVerSyncDBInterface, SyncEngine,
     * and set maximum cache of queue.
     */
    const std::string remoteDeviceId = "real_device";
    KvStoreNbDelegate::Option option = {true};
    g_mgr.GetKvStore(ANTI_DOS_STORE_ID, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegateStatus == OK);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    g_syncInterface = new (std::nothrow) VirtualSingleVerSyncDBInterface();
    ASSERT_TRUE(g_syncInterface != nullptr);
    g_metaData = std::make_shared<Metadata>();
    int errCodeMetaData = g_metaData->Initialize(g_syncInterface);
    ASSERT_TRUE(errCodeMetaData == E_OK);
    g_syncEngine = new (std::nothrow) SingleVerSyncEngine();
    ASSERT_TRUE(g_syncEngine != nullptr);
    int errCodeSyncEngine = g_syncEngine->Initialize(g_syncInterface, g_metaData, nullptr);
    ASSERT_TRUE(errCodeSyncEngine == E_OK);
    g_communicator = static_cast<VirtualCommunicator *>(g_communicatorAggregator->GetCommunicator(remoteDeviceId));
    ASSERT_TRUE(g_communicator != nullptr);
    g_syncEngine->SetMaxQueueCacheSize(LIMIT_QUEUE_CACHE_SIZE);
}

void DistributeddbAntiDosSyncTest::TearDown(void)
{
    /**
     * @tc.teardown: Release VirtualCommunicator, VirtualSingleVerSyncDBInterface and SyncEngine.
     */
    if (g_communicator != nullptr) {
        g_communicator->KillObj();
        g_communicator = nullptr;
    }
    if (g_syncEngine != nullptr) {
        g_syncEngine->SetMaxQueueCacheSize(DEFAULT_CACHE_SIZE);
        auto syncEngine = g_syncEngine;
        g_syncEngine->OnKill([syncEngine]() { syncEngine->Close(); });
        RefObject::KillAndDecObjRef(g_syncEngine);
        g_syncEngine = nullptr;
    }
    g_metaData = nullptr;
    if (g_syncInterface != nullptr) {
        delete g_syncInterface;
        g_syncInterface = nullptr;
    }
    if (g_kvDelegatePtr != nullptr) {
        g_mgr.CloseKvStore(g_kvDelegatePtr);
        g_kvDelegatePtr = nullptr;
    }
    g_mgr.DeleteKvStore(ANTI_DOS_STORE_ID);
}

/**
 * @tc.name: Anti Dos attack Sync 001
 * @tc.desc: Whether function run normally when the amount of message is lower than the maximum of threads
 *   and the whole length of message is lower than the maximum size of queue.
 * @tc.type: FUNC
 * @tc.require: AR000D08KU
 * @tc.author: yiguang
 */
HWTEST_F(DistributeddbAntiDosSyncTest, AntiDosAttackSync001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. control MessageReceiveCallback to send messages, whose number is lower than
     *  the maximum of threads and length is lower than the maximum size of queue.
     */
    const std::string srcTarget = "001";
    std::vector<SendDataItem> outData;

    for (unsigned int index = 0; index < g_syncEngine->GetMaxExecNum() - TEST_ONE; index++) {
        DataRequestPacket *packet = new (std::nothrow) DataRequestPacket;
        ASSERT_TRUE(packet != nullptr);
        Message *message = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
        ASSERT_TRUE(message != nullptr);

        GenericSingleVerKvEntry *kvEntry = new (std::nothrow) GenericSingleVerKvEntry();
        ASSERT_TRUE(kvEntry != nullptr);
        outData.push_back(kvEntry);
        packet->SetData(outData);
        packet->SetSendCode(E_OK);
        packet->SetVersion(SOFTWARE_VERSION_CURRENT);
        uint32_t sessionId = index;
        uint32_t sequenceId = index;
        message->SetMessageType(TYPE_REQUEST);
        message->SetTarget(srcTarget);
        int errCode = message->SetExternalObject(packet);
        ASSERT_TRUE(errCode == E_OK);
        message->SetSessionId(sessionId);
        message->SetSequenceId(sequenceId);
        g_communicator->CallbackOnMessage(srcTarget, message);
    /**
     * @tc.expected: step1. no message was found to be enqueued and discarded.
     */
        EXPECT_TRUE(g_syncEngine->GetQueueCacheSize() == 0);
    }
    EXPECT_TRUE(g_syncEngine->GetDiscardMsgNum() == 0);
}

/**
 * @tc.name: Anti Dos attack Sync 002
 * @tc.desc: Check if the enqueued and dequeue are normal when the whole length of messages is lower than
 *  maximum size of queue.
 * @tc.type: FUNC
 * @tc.require: AR000D08KU
 * @tc.author: yiguang
 */
HWTEST_F(DistributeddbAntiDosSyncTest, AntiDosAttackSync002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set block in function DispatchMessage as true.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_SHORT_TIME));
    g_communicatorAggregator->SetBlockValue(true);

    /**
     * @tc.steps: step2. control MessageReceiveCallback to send suitable messages.
     */
    const std::string srcTarget = "001";

    for (unsigned int index = 0; index < g_syncEngine->GetMaxExecNum() + TEST_TWO; index++) {
        std::vector<SendDataItem> outData;
        DataRequestPacket *packet = new (std::nothrow) DataRequestPacket;
        ASSERT_TRUE(packet != nullptr);
        Message *message = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
        ASSERT_TRUE(message != nullptr);

        GenericSingleVerKvEntry *kvEntry = new (std::nothrow) GenericSingleVerKvEntry();
        ASSERT_TRUE(kvEntry != nullptr);
        outData.push_back(kvEntry);
        packet->SetData(outData);
        packet->SetSendCode(E_OK);
        packet->SetVersion(SOFTWARE_VERSION_CURRENT);

        uint32_t sessionId = index;
        uint32_t sequenceId = index;
        message->SetMessageType(TYPE_REQUEST);
        message->SetTarget(srcTarget);
        int errCode = message->SetExternalObject(packet);
        ASSERT_TRUE(errCode == E_OK);
        message->SetSessionId(sessionId);
        message->SetSequenceId(sequenceId);
        g_communicator->CallbackOnMessage(srcTarget, message);
    }

    /**
     * @tc.expected: step2. all messages enter the queue.
     */
    EXPECT_TRUE(g_syncEngine->GetDiscardMsgNum() == 0);
    EXPECT_TRUE(g_syncEngine->GetQueueCacheSize() / NUM == TEST_TWO);

    /**
     * @tc.steps: step3. set block in function DispatchMessage as false after a period of time.
     */
    g_communicator->Disable();
    g_communicatorAggregator->SetBlockValue(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_LONG_TIME));

    /**
     * @tc.expected: step3. the queue is eventually empty and no message is discarded.
     */
    EXPECT_TRUE(g_syncEngine->GetDiscardMsgNum() == 0);
    EXPECT_TRUE(g_syncEngine->GetQueueCacheSize() == 0);
}

/**
 * @tc.name: Anti Dos attack Sync 003
 * @tc.desc: Whether message enter and drop when all threads hang.
 * @tc.type: FUNC
 * @tc.require: AR000D08KU
 * @tc.author: yiguang
 */
HWTEST_F(DistributeddbAntiDosSyncTest, AntiDosAttackSync003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set block in function DispatchMessage as true.
     */
    g_communicatorAggregator->SetBlockValue(true);

    /**
     * @tc.steps: step2. control MessageReceiveCallback to send messages that are more than maximum size of queue.
     */
    const std::string srcTarget = "001";

    for (unsigned int index = 0; index < g_syncEngine->GetMaxExecNum() + TEST_THREE_THREAD; index++) {
        std::vector<SendDataItem> outData;
        DataRequestPacket *packet = new (std::nothrow) DataRequestPacket;
        ASSERT_TRUE(packet != nullptr);
        Message *message = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
        ASSERT_TRUE(message != nullptr);
        for (int outIndex = 0; outIndex < TEST_THREE_OUTDATA; outIndex++) {
            GenericSingleVerKvEntry *kvEntry = new (std::nothrow) GenericSingleVerKvEntry();
            ASSERT_TRUE(kvEntry != nullptr);
            outData.push_back(kvEntry);
        }
        packet->SetData(outData);
        packet->SetSendCode(E_OK);
        packet->SetVersion(SOFTWARE_VERSION_CURRENT);

        uint32_t sessionId = index;
        uint32_t sequenceId = index;
        message->SetMessageType(TYPE_REQUEST);
        message->SetTarget(srcTarget);
        int errCode = message->SetExternalObject(packet);
        ASSERT_TRUE(errCode == E_OK);
        message->SetSessionId(sessionId);
        message->SetSequenceId(sequenceId);
        g_communicator->CallbackOnMessage(srcTarget, message);
    }

    /**
     * @tc.expected: step2. after part of messages are enqueued, the rest of the messages are discarded.
     */
    EXPECT_TRUE(g_syncEngine->GetDiscardMsgNum() > 0);
    EXPECT_TRUE(g_syncEngine->GetQueueCacheSize() > 0);
    g_communicatorAggregator->SetBlockValue(false);
}