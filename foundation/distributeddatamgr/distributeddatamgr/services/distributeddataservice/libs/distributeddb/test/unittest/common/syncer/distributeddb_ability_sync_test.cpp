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

#include "ability_sync.h"
#include "version.h"
#include "sync_types.h"
#include "vitural_communicator_aggregator.h"
#include "vitural_communicator.h"
#include "virtual_single_ver_sync_db_Interface.h"
#include "single_ver_sync_task_context.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;

namespace {
    const std::string DEVICE_A = "deviceA";
    const std::string DEVICE_B = "deviceB";
    const std::string TEST_SCHEMA = "{\"SCHEMA_DEFINE\":{\"value\":\"LONG\"},\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_VERSION\":\"1.0\"}";

    VirtualSingleVerSyncDBInterface *g_syncInterface = nullptr;
    VirtualCommunicatorAggregator *g_communicatorAggregator = nullptr;
    ICommunicator *g_communicatorA = nullptr;
    ICommunicator *g_communicatorB = nullptr;
}

class DistributedDBAbilitySyncTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBAbilitySyncTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: NA
     */
}

void DistributedDBAbilitySyncTest::TearDownTestCase(void)
{
    /**
     * @tc.teardown: NA
     */
}

void DistributedDBAbilitySyncTest::SetUp(void)
{
    /**
     * @tc.setup: create the instance for virtual communicator, virtual storage
     */
    g_syncInterface = new (std::nothrow) VirtualSingleVerSyncDBInterface();
    ASSERT_TRUE(g_syncInterface != nullptr);
    g_syncInterface->SetSchemaInfo(TEST_SCHEMA);
    g_communicatorAggregator = new (std::nothrow) VirtualCommunicatorAggregator;
    ASSERT_TRUE(g_communicatorAggregator != nullptr);
    int errCode = E_OK;
    g_communicatorA = g_communicatorAggregator->AllocCommunicator(DEVICE_A, errCode);
    ASSERT_TRUE(g_communicatorA != nullptr);
    g_communicatorB = g_communicatorAggregator->AllocCommunicator(DEVICE_B, errCode);
    ASSERT_TRUE(g_communicatorB != nullptr);
}

void DistributedDBAbilitySyncTest::TearDown(void)
{
    /**
     * @tc.teardown: delete the ptr for testing
     */
    if (g_communicatorA != nullptr && g_communicatorAggregator != nullptr) {
        g_communicatorAggregator->ReleaseCommunicator(g_communicatorA);
        g_communicatorA = nullptr;
    }
    if (g_communicatorB != nullptr && g_communicatorAggregator != nullptr) {
        g_communicatorAggregator->ReleaseCommunicator(g_communicatorB);
        g_communicatorB = nullptr;
    }
    if (g_communicatorAggregator != nullptr) {
        RefObject::KillAndDecObjRef(g_communicatorAggregator);
        g_communicatorAggregator = nullptr;
    }
    if (g_syncInterface != nullptr) {
        delete g_syncInterface;
        g_syncInterface = nullptr;
    }
}

/**
 * @tc.name: RequestPacketTest001
 * @tc.desc: Verify RequestPacketSerialization and RequestPacketDeSerialization function.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBAbilitySyncTest, RequestPacketTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a AbilityRequestPacket packet1
     * @tc.steps: step2. set version = ABILITY_SYNC_VERSION_V1. schema = TEST_SCHEMA.
     */
    AbilitySyncRequestPacket packet1;
    packet1.SetProtocolVersion(ABILITY_SYNC_VERSION_V1);
    packet1.SetSoftwareVersion(SOFTWARE_VERSION_BASE);
    packet1.SetSchema(TEST_SCHEMA);
    packet1.SetSendCode(E_OK);
    Message msg1(ABILITY_SYNC_MESSAGE);
    msg1.SetMessageType(TYPE_REQUEST);
    msg1.SetCopiedObject(packet1);

    /**
     * @tc.steps: step3. call Serialization to Serialization the msg
     * @tc.expected: step3. Serialization return E_OK
     */
    uint32_t bufflen = packet1.CalculateLen();
    ASSERT_TRUE(bufflen != 0);
    std::vector<uint8_t> buff(bufflen, 0);
    ASSERT_TRUE(AbilitySync::Serialization(buff.data(), bufflen, &msg1) == E_OK);

    /**
     * @tc.steps: step4. call DeSerialization to DeSerialization the buff
     * @tc.expected: step4. DeSerialization return E_OK
     */
    Message msg2(ABILITY_SYNC_MESSAGE);
    msg2.SetMessageType(TYPE_REQUEST);
    ASSERT_TRUE(AbilitySync::DeSerialization(buff.data(), bufflen, &msg2) == E_OK);
    const AbilitySyncRequestPacket *packet2 = msg2.GetObject<AbilitySyncRequestPacket>();
    ASSERT_TRUE(packet2 != nullptr);

    /**
     * @tc.expected: step5. packet1 == packet2
     */
    EXPECT_TRUE(packet2->GetProtocolVersion() == ABILITY_SYNC_VERSION_V1);
    EXPECT_TRUE(packet2->GetSoftwareVersion() == SOFTWARE_VERSION_BASE);
    EXPECT_TRUE(packet2->GetSendCode() == E_OK);
    std::string schema;
    packet2->GetSchema(schema);
    EXPECT_EQ(schema, TEST_SCHEMA);
}

/**
 * @tc.name: RequestPacketTest002
 * @tc.desc: Verify RequestPacketSerialization and RequestPacketDeSerialization function when version not support.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBAbilitySyncTest, RequestPacketTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a AbilityRequestPacket packet1
     * @tc.steps: step2. set version = ABILITY_SYNC_VERSION_V1 + 1. schema = TEST_SCHEMA.
     */
    AbilitySyncRequestPacket packet1;
    packet1.SetProtocolVersion(ABILITY_SYNC_VERSION_V1 + 1);
    packet1.SetSoftwareVersion(SOFTWARE_VERSION_CURRENT);
    packet1.SetSchema("");
    Message msg1(ABILITY_SYNC_MESSAGE);
    msg1.SetMessageType(TYPE_REQUEST);
    msg1.SetCopiedObject(packet1);

    /**
     * @tc.steps: step3. call Serialization to Serialization the msg
     * @tc.expected: step3. Serialization return E_OK
     */
    uint32_t bufflen = packet1.CalculateLen();
    ASSERT_TRUE(bufflen != 0);
    std::vector<uint8_t> buff(bufflen, 0);
    ASSERT_TRUE(AbilitySync::Serialization(buff.data(), bufflen, &msg1) == E_OK);

    /**
     * @tc.steps: step4. call DeSerialization to DeSerialization the buff
     * @tc.expected: step4. DeSerialization return E_OK
     */
    Message msg2(ABILITY_SYNC_MESSAGE);
    msg2.SetMessageType(TYPE_REQUEST);
    ASSERT_TRUE(AbilitySync::DeSerialization(buff.data(), bufflen, &msg2) == E_OK);
    const AbilitySyncRequestPacket *packet2 = msg2.GetObject<AbilitySyncRequestPacket>();
    ASSERT_TRUE(packet2 != nullptr);

    /**
     * @tc.expected: step5. packet2->GetSendCode() == -E_VERSION_NOT_SUPPORT
     */
    EXPECT_TRUE(packet2->GetSendCode() == -E_VERSION_NOT_SUPPORT);
}

/**
 * @tc.name: RequestPacketTest003
 * @tc.desc: Verify RequestPacketSerialization and RequestPacketDeSerialization function.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBAbilitySyncTest, RequestPacketTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a AbilityRequestPacket packet1
     * @tc.steps: step2. set version = ABILITY_SYNC_VERSION_V1. schema = TEST_SCHEMA.
     */
    AbilitySyncRequestPacket packet1;
    packet1.SetProtocolVersion(ABILITY_SYNC_VERSION_V1);
    packet1.SetSoftwareVersion(SOFTWARE_VERSION_RELEASE_3_0);
    packet1.SetSchema(TEST_SCHEMA);
    packet1.SetSendCode(E_OK);
    int secLabel = 3; // label 3
    int secFlag = 1; // flag 1
    packet1.SetSecLabel(secLabel);
    packet1.SetSecFlag(secFlag);
    Message msg1(ABILITY_SYNC_MESSAGE);
    msg1.SetMessageType(TYPE_REQUEST);
    msg1.SetCopiedObject(packet1);

    /**
     * @tc.steps: step3. call Serialization to Serialization the msg
     * @tc.expected: step3. Serialization return E_OK
     */
    uint32_t bufflen = packet1.CalculateLen();
    ASSERT_TRUE(bufflen != 0);
    std::vector<uint8_t> buff(bufflen, 0);
    ASSERT_TRUE(AbilitySync::Serialization(buff.data(), bufflen, &msg1) == E_OK);

    /**
     * @tc.steps: step4. call DeSerialization to DeSerialization the buff
     * @tc.expected: step4. DeSerialization return E_OK
     */
    Message msg2(ABILITY_SYNC_MESSAGE);
    msg2.SetMessageType(TYPE_REQUEST);
    ASSERT_TRUE(AbilitySync::DeSerialization(buff.data(), bufflen, &msg2) == E_OK);
    const AbilitySyncRequestPacket *packet2 = msg2.GetObject<AbilitySyncRequestPacket>();
    ASSERT_TRUE(packet2 != nullptr);

    /**
     * @tc.expected: step5. packet1 == packet2
     */
    EXPECT_TRUE(packet2->GetProtocolVersion() == ABILITY_SYNC_VERSION_V1);
    EXPECT_TRUE(packet2->GetSoftwareVersion() == SOFTWARE_VERSION_RELEASE_3_0);
    EXPECT_TRUE(packet2->GetSendCode() == E_OK);
    std::string schema;
    packet2->GetSchema(schema);
    EXPECT_EQ(schema, TEST_SCHEMA);
    EXPECT_TRUE(packet2->GetSecFlag() == secFlag);
    EXPECT_TRUE(packet2->GetSecLabel() == secLabel);
}

/**
 * @tc.name: RequestPacketTest004
 * @tc.desc: Verify RequestPacketSerialization and RequestPacketDeSerialization function.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBAbilitySyncTest, RequestPacketTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a AbilityRequestPacket packet1
     * @tc.steps: step2. set version = ABILITY_SYNC_VERSION_V1. schema = TEST_SCHEMA.
     */
    AbilitySyncRequestPacket packet1;
    packet1.SetProtocolVersion(ABILITY_SYNC_VERSION_V1);
    packet1.SetSoftwareVersion(SOFTWARE_VERSION_RELEASE_2_0);
    packet1.SetSchema(TEST_SCHEMA);
    packet1.SetSendCode(E_OK);
    int secLabel = 3; // label 3
    int secFlag = 1; // flag 1
    packet1.SetSecLabel(secLabel);
    packet1.SetSecFlag(secFlag);
    Message msg1(ABILITY_SYNC_MESSAGE);
    msg1.SetMessageType(TYPE_REQUEST);
    msg1.SetCopiedObject(packet1);

    /**
     * @tc.steps: step3. call Serialization to Serialization the msg
     * @tc.expected: step3. Serialization return E_OK
     */
    uint32_t bufflen = packet1.CalculateLen();
    ASSERT_TRUE(bufflen != 0);
    std::vector<uint8_t> buff(bufflen, 0);
    ASSERT_TRUE(AbilitySync::Serialization(buff.data(), bufflen, &msg1) == E_OK);

    /**
     * @tc.steps: step4. call DeSerialization to DeSerialization the buff
     * @tc.expected: step4. DeSerialization return E_OK
     */
    Message msg2(ABILITY_SYNC_MESSAGE);
    msg2.SetMessageType(TYPE_REQUEST);
    ASSERT_TRUE(AbilitySync::DeSerialization(buff.data(), bufflen, &msg2) == E_OK);
    const AbilitySyncRequestPacket *packet2 = msg2.GetObject<AbilitySyncRequestPacket>();
    ASSERT_TRUE(packet2 != nullptr);

    /**
     * @tc.expected: step5. packet1 == packet2
     */
    EXPECT_TRUE(packet2->GetProtocolVersion() == ABILITY_SYNC_VERSION_V1);
    EXPECT_TRUE(packet2->GetSoftwareVersion() == SOFTWARE_VERSION_RELEASE_2_0);
    EXPECT_TRUE(packet2->GetSendCode() == E_OK);
    std::string schema;
    packet2->GetSchema(schema);
    EXPECT_EQ(schema, TEST_SCHEMA);
    EXPECT_TRUE(packet2->GetSecFlag() == 0);
    EXPECT_TRUE(packet2->GetSecLabel() == 0);
}

/**
 * @tc.name: AckPacketTest001
 * @tc.desc: Verify AckPacketSerialization and AckPacketDeSerialization function.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBAbilitySyncTest, AckPacketTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a AbilityAckPacket packet1
     * @tc.steps: step2. set version = ABILITY_SYNC_VERSION_V1. schema = TEST_SCHEMA.
     */
    AbilitySyncAckPacket packet1;
    packet1.SetProtocolVersion(ABILITY_SYNC_VERSION_V1);
    packet1.SetSoftwareVersion(SOFTWARE_VERSION_CURRENT);
    packet1.SetSchema(TEST_SCHEMA);
    packet1.SetAckCode(E_VERSION_NOT_SUPPORT);
    Message msg1(ABILITY_SYNC_MESSAGE);
    msg1.SetMessageType(TYPE_RESPONSE);
    msg1.SetCopiedObject(packet1);

    /**
     * @tc.steps: step3. call Serialization to Serialization the msg
     * @tc.expected: step3. Serialization return E_OK
     */
    uint32_t bufflen = packet1.CalculateLen();
    ASSERT_TRUE(bufflen != 0);
    std::vector<uint8_t> buff(bufflen, 0);
    ASSERT_EQ(AbilitySync::Serialization(buff.data(), bufflen, &msg1), E_OK);

    /**
     * @tc.steps: step4. call DeSerialization to DeSerialization the buff
     * @tc.expected: step4. DeSerialization return E_OK
     */
    Message msg2(ABILITY_SYNC_MESSAGE);
    msg2.SetMessageType(TYPE_RESPONSE);
    ASSERT_TRUE(AbilitySync::DeSerialization(buff.data(), bufflen, &msg2) == E_OK);
    const AbilitySyncAckPacket *packet2 = msg2.GetObject<AbilitySyncAckPacket>();
    ASSERT_TRUE(packet2 != nullptr);

    /**
     * @tc.expected: step5. packet1 == packet2
     */
    EXPECT_TRUE(packet2->GetProtocolVersion() == ABILITY_SYNC_VERSION_V1);
    EXPECT_TRUE(packet2->GetSoftwareVersion() == SOFTWARE_VERSION_CURRENT);
    EXPECT_TRUE(packet2->GetAckCode() == E_VERSION_NOT_SUPPORT);
    std::string schema;
    packet2->GetSchema(schema);
    ASSERT_TRUE(schema == TEST_SCHEMA);
}

/**
 * @tc.name: SyncStartTest001
 * @tc.desc: Verify Ability sync SyncStart function.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBAbilitySyncTest, SyncStart001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a AbilitySync
     */
    AbilitySync async;
    async.Initialize(g_communicatorB, g_syncInterface, DEVICE_A);

    /**
     * @tc.steps: step2. call SyncStart
     * @tc.expected: step2. SyncStart return E_OK
     */
    EXPECT_EQ(async.SyncStart(1, 1, 1), E_OK);

    /**
     * @tc.steps: step3. disable the communicator
     */
    static_cast<VirtualCommunicator *>(g_communicatorB)->Disable();

    /**
     * @tc.steps: step4. call SyncStart
     * @tc.expected: step4. SyncStart return -E_PERIPHERAL_INTERFACE_FAIL
     */
    EXPECT_TRUE(async.SyncStart(1, 1, 1) == -E_PERIPHERAL_INTERFACE_FAIL);
}
#ifndef OMIT_JSON
/**
 * @tc.name: RequestReceiveTest001
 * @tc.desc: Verify Ability RequestReceive callback.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBAbilitySyncTest, RequestReceiveTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a AbilitySync
     */
    AbilitySync async;
    async.Initialize(g_communicatorB, g_syncInterface, DEVICE_A);

    /**
     * @tc.steps: step2. call RequestRecv, set inMsg nullptr  or set context nullptr
     * @tc.expected: step2. RequestRecv return -E_INVALID_ARGS
     */
    Message msg1(ABILITY_SYNC_MESSAGE);
    msg1.SetMessageType(TYPE_REQUEST);
    SingleVerSyncTaskContext *context = new (std::nothrow) SingleVerSyncTaskContext();
    ASSERT_TRUE(context != nullptr);
    EXPECT_EQ(async.RequestRecv(nullptr, context), -E_INVALID_ARGS);
    EXPECT_EQ(async.RequestRecv(&msg1, nullptr), -E_INVALID_ARGS);

    /**
     * @tc.steps: step3. call RequestRecv, set inMsg with no packet
     * @tc.expected: step3. RequestRecv return -E_INVALID_ARGS
     */
    EXPECT_EQ(async.RequestRecv(&msg1, context), -E_INVALID_ARGS);

    /**
     * @tc.steps: step4. create a AbilityRequestkPacket packet1
     */
    AbilitySyncRequestPacket packet1;
    packet1.SetProtocolVersion(ABILITY_SYNC_VERSION_V1);
    packet1.SetSoftwareVersion(SOFTWARE_VERSION_CURRENT);
    packet1.SetSchema(TEST_SCHEMA);
    msg1.SetCopiedObject(packet1);

    /**
     * @tc.steps: step5. call RequestRecv, set inMsg with packet
     * @tc.expected: step5. RequestRecv return ok, GetRemoteSoftwareVersion is SOFTWARE_VERSION_CURRENT
     *     IsSchemaCompatible true
     *
     */
    EXPECT_EQ(async.RequestRecv(&msg1, context), OK);
    EXPECT_TRUE(context->GetRemoteSoftwareVersion() == SOFTWARE_VERSION_CURRENT);
    EXPECT_TRUE(context->GetTaskErrCode() != -E_SCHEMA_MISMATCH);

    /**
     * @tc.steps: step6. call RequestRecv, set inMsg sendCode -E_VERSION_NOT_SUPPORT
     * @tc.expected: step6. RequestRecv return E_VERSION_NOT_SUPPORT
     */
    packet1.SetSendCode(-E_VERSION_NOT_SUPPORT);
    msg1.SetCopiedObject(packet1);
    EXPECT_EQ(async.RequestRecv(&msg1, context), -E_VERSION_NOT_SUPPORT);

    /**
     * @tc.steps: step7. call RequestRecv, SetSchema ""
     * @tc.expected: step7. IsSchemaCompatible false
     */
    packet1.SetSchema("");
    packet1.SetSendCode(E_OK);
    msg1.SetCopiedObject(packet1);
    EXPECT_EQ(async.RequestRecv(&msg1, context), E_OK);
    EXPECT_FALSE(context->GetTaskErrCode() != -E_SCHEMA_MISMATCH);
    RefObject::KillAndDecObjRef(context);
}
#endif
/**
 * @tc.name: AckReceiveTest001
 * @tc.desc: Verify Ability AckReceive callback.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBAbilitySyncTest, AckReceiveTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a AbilitySync
     */
    AbilitySync async;
    async.Initialize(g_communicatorB, g_syncInterface, DEVICE_A);

    /**
     * @tc.steps: step2. call AckRecv, set inMsg nullptr or set context nullptr
     * @tc.expected: step2. AckRecv return -E_INVALID_ARGS
     */
    SingleVerSyncTaskContext *context = new (std::nothrow) SingleVerSyncTaskContext();
    ASSERT_TRUE(context != nullptr);
    Message msg1(ABILITY_SYNC_MESSAGE);
    msg1.SetMessageType(TYPE_RESPONSE);
    EXPECT_EQ(async.AckRecv(nullptr, context), -E_INVALID_ARGS);
    EXPECT_EQ(async.AckRecv(&msg1, nullptr), -E_INVALID_ARGS);

    /**
     * @tc.steps: step3. call AckRecv, set inMsg with no packet
     * @tc.expected: step3. AckRecv return -E_INVALID_ARGS
     */
    EXPECT_EQ(async.AckRecv(&msg1, context), -E_INVALID_ARGS);
    ASSERT_TRUE(context != nullptr);

    /**
     * @tc.steps: step4. create a AbilityAckPacket packet1
     */
    AbilitySyncAckPacket packet1;
    packet1.SetProtocolVersion(ABILITY_SYNC_VERSION_V1);
    packet1.SetSoftwareVersion(SOFTWARE_VERSION_CURRENT);
    packet1.SetAckCode(E_OK);
    packet1.SetSchema(TEST_SCHEMA);
    msg1.SetCopiedObject(packet1);

    /**
     * @tc.steps: step5. call AckRecv, set inMsg with packet
     * @tc.expected: step5. AckRecv return ok GetRemoteSoftwareVersion is SOFTWARE_VERSION_CURRENT
     *     IsSchemaCompatible true;
     */
    EXPECT_EQ(async.AckRecv(&msg1, context), OK);
    EXPECT_TRUE(context->GetRemoteSoftwareVersion() == SOFTWARE_VERSION_CURRENT);
    EXPECT_TRUE(context->GetTaskErrCode() != -E_SCHEMA_MISMATCH);

    /**
     * @tc.steps: step6. call RequestRecv, SetSchema ""
     * @tc.expected: step6. IsSchemaCompatible false
     */
    packet1.SetSchema("");
    msg1.SetCopiedObject(packet1);
    EXPECT_EQ(async.AckRecv(&msg1, context), E_OK);

    /**
     * @tc.steps: step7. call AckRecv, set inMsg sendCode -E_VERSION_NOT_SUPPORT
     * @tc.expected: step7. return -E_VERSION_NOT_SUPPORT
     */
    packet1.SetSchema(TEST_SCHEMA);
    packet1.SetAckCode(-E_VERSION_NOT_SUPPORT);
    msg1.SetCopiedObject(packet1);
    EXPECT_EQ(async.AckRecv(&msg1, context), -E_VERSION_NOT_SUPPORT);
    RefObject::KillAndDecObjRef(context);
}
