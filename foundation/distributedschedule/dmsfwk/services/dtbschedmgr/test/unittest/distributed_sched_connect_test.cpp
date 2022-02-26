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
#include "gtest/gtest.h"

#define private public
#define protected public
#include "ability_connect_callback_stub.h"
#include "distributed_sched_service.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "test_log.h"
#undef private
#undef protected

namespace OHOS {
namespace DistributedSchedule {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr int32_t STDOUT_FD = 1;
}

class AbilityConnectCallbackTest : public AAFwk::AbilityConnectionStub {
public:
    AbilityConnectCallbackTest() = default;
    ~AbilityConnectCallbackTest() = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName& element,
        const sptr<IRemoteObject>& remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
        int32_t resultCode) override;
};

class DistributedSchedConnectTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    void AddSession(const sptr<IRemoteObject>& connect, const std::string& localDeviceId,
        const std::string& remoteDeviceId, const AAFwk::Want& want);
    void RemoveSession(const sptr<IRemoteObject>& connect);
};

void AbilityConnectCallbackTest::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int32_t resultCode)
{
}

void AbilityConnectCallbackTest::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int32_t resultCode)
{
}

void DistributedSchedConnectTest::SetUpTestCase()
{
}

void DistributedSchedConnectTest::TearDownTestCase()
{
}

void DistributedSchedConnectTest::SetUp()
{
}

void DistributedSchedConnectTest::TearDown()
{
}

void DistributedSchedConnectTest::AddSession(const sptr<IRemoteObject>& connect,
    const std::string& localDeviceId, const std::string& remoteDeviceId, const AAFwk::Want& want)
{
    if (connect == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> autoLock(DistributedSchedService::GetInstance().distributedLock_);
    CallerInfo callerInfo;
    callerInfo.uid = IPCSkeleton::GetCallingUid();
    callerInfo.pid = IPCSkeleton::GetCallingPid();
    callerInfo.sourceDeviceId = localDeviceId;
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    DistributedSchedService::GetInstance().RemoteConnectAbilityMappingLocked(connect, localDeviceId,
        remoteDeviceId, want.GetElement(), callerInfo, TargetComponent::HARMONY_COMPONENT);
}

void DistributedSchedConnectTest::RemoveSession(const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> autoLock(DistributedSchedService::GetInstance().distributedLock_);
    DistributedSchedService::GetInstance().distributedConnectAbilityMap_.erase(connect);
}

/**
 * @tc.name: DumpConnectInfo_001
 * @tc.desc: dump connect ability info by call Dump
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedConnectTest, DumpConnectInfo_001, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest DumpConnectInfo_001 start " << std::endl;
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DTEST_LOG << "DistributedSchedServiceTest DumpConnectInfo_001 samgr null" << std::endl;
    } else {
        DTEST_LOG << "DistributedSchedServiceTest DumpConnectInfo_001 avaiable" << std::endl;
    }

    auto dms = samgr->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    std::vector<std::u16string> args;
    args.push_back(u"-connect");
    int32_t result = dms->Dump(STDOUT_FD, args);
    DTEST_LOG << "DistributedSchedServiceTest DumpConnectInfo_001 dump result: " << result << std::endl;
}

/**
 * @tc.name: DumpConnectInfo_002
 * @tc.desc: dump connect ability info by call DumpConnectInfo
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedConnectTest, DumpConnectInfo_002, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest DumpConnectInfo_002 start" << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName("", "ohos.demo.bundleName", "abilityName");

    /**
     * @tc.steps: step1. add one session
     */
    sptr<AbilityConnectCallbackTest> connect = new AbilityConnectCallbackTest();
    AddSession(connect, "123_local_device_id", "123_remote_device_id", want);
    /**
     * @tc.steps: step2. and then dump connection info
     * @tc.expected: step2. can find the newly-added connect session
     */
    std::string dumpInfo;
    DistributedSchedService::GetInstance().DumpConnectInfo(dumpInfo);
    DTEST_LOG << "DistributedSchedServiceTest DumpConnectInfo_002 dumpInfo " << dumpInfo << std::endl;
    std::string::size_type pos = dumpInfo.find("123_remote_device_id");
    EXPECT_NE(pos, std::string::npos);

    RemoveSession(connect);
}

/**
 * @tc.name: ProcessConnectDied001
 * @tc.desc: process connect died
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedConnectTest, ProcessConnectDied001, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest ProcessConnectDied001 start" << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName("", "ohos.demo.bundleName", "abilityName");
    auto& connectionMap = DistributedSchedService::GetInstance().distributedConnectAbilityMap_;
    auto& distributedLock = DistributedSchedService::GetInstance().distributedLock_;

    /**
     * @tc.steps: step1. add one session and check the map
     * @tc.expected: step1. can find the newly-added connect session
     */
    sptr<AbilityConnectCallbackTest> connect = new AbilityConnectCallbackTest();
    AddSession(connect, "123_local_device_id", "123_remote_device_id", want);
    {
        std::lock_guard<std::mutex> autoLock(distributedLock);
        EXPECT_EQ(connectionMap.size(), static_cast<size_t>(1));
    }

    /**
     * @tc.steps: step2. process connect died and then check the map
     * @tc.expected: step2. the connect session is removed
     */
    DistributedSchedService::GetInstance().ProcessConnectDied(connect);
    {
        std::lock_guard<std::mutex> autoLock(distributedLock);
        EXPECT_EQ(connectionMap.size(), static_cast<size_t>(0));
    }

    RemoveSession(connect);
}

/**
 * @tc.name: ProcessConnectDied002
 * @tc.desc: process connect died which is not exist
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedConnectTest, ProcessConnectDied002, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest ProcessConnectDied002 start" << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName("", "ohos.demo.bundleName", "abilityName");
    auto& connectionMap = DistributedSchedService::GetInstance().distributedConnectAbilityMap_;
    auto& distributedLock = DistributedSchedService::GetInstance().distributedLock_;

    /**
     * @tc.steps: step1. add one session
     * @tc.expected: step1. can find the newly-added connect session
     */
    sptr<AbilityConnectCallbackTest> connect = new AbilityConnectCallbackTest();
    AddSession(connect, "123_local_device_id", "123_remote_device_id", want);
    {
        std::lock_guard<std::mutex> autoLock(distributedLock);
        EXPECT_EQ(connectionMap.size(), static_cast<size_t>(1));
    }

    /**
     * @tc.steps: step2. process connect died which is not exist
     * @tc.expected: step2. still can find the newly-added connect session
     */
    DistributedSchedService::GetInstance().ProcessConnectDied(new AbilityConnectCallbackTest());
    {
        std::lock_guard<std::mutex> autoLock(distributedLock);
        EXPECT_EQ(connectionMap.size(), static_cast<size_t>(1));
    }

    RemoveSession(connect);
}

/**
 * @tc.name: ProcessDeviceOffline001
 * @tc.desc: process device offline with only one connection
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedConnectTest, ProcessDeviceOffline001, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest ProcessDeviceOffline001 start" << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName("", "ohos.demo.bundleName", "abilityName");
    auto& connectionMap = DistributedSchedService::GetInstance().distributedConnectAbilityMap_;
    auto& distributedLock = DistributedSchedService::GetInstance().distributedLock_;

    /**
     * @tc.steps: step1. add one session
     */
    sptr<AbilityConnectCallbackTest> connect = new AbilityConnectCallbackTest();
    AddSession(connect, "123_local_device_id", "123_remote_device_id", want);
    /**
     * @tc.steps: step2. process device offline and check the map
     * @tc.expected: step2. the connect session is removed
     */
    DistributedSchedService::GetInstance().ProcessDeviceOffline("123_remote_device_id");
    {
        std::lock_guard<std::mutex> autoLock(distributedLock);
        EXPECT_EQ(connectionMap.size(), static_cast<size_t>(0));
    }

    RemoveSession(connect);
}

/**
 * @tc.name: ProcessDeviceOffline002
 * @tc.desc: process device offline with multiple connections
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedConnectTest, ProcessDeviceOffline002, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest ProcessDeviceOffline002 start" << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName("", "ohos.demo.bundleName", "abilityName");
    auto& connectionMap = DistributedSchedService::GetInstance().distributedConnectAbilityMap_;
    auto& distributedLock = DistributedSchedService::GetInstance().distributedLock_;

    /**
     * @tc.steps: step1. add one session
     * @tc.expected: step1. can find the two newly-added connect sessions
     */
    sptr<AbilityConnectCallbackTest> connect1 = new AbilityConnectCallbackTest();
    AddSession(connect1, "123_local_device_id", "123_remote_device_id", want);
    sptr<AbilityConnectCallbackTest> connect2 = new AbilityConnectCallbackTest();
    AddSession(connect2, "123_local_device_id", "123_remote_device_id", want);
    {
        std::lock_guard<std::mutex> autoLock(distributedLock);
        EXPECT_EQ(connectionMap.size(), static_cast<size_t>(2));
    }

    /**
     * @tc.steps: step2. process device offline
     * @tc.expected: step2. the connect sessions are removed
     */
    DistributedSchedService::GetInstance().ProcessDeviceOffline("123_remote_device_id");
    {
        std::lock_guard<std::mutex> autoLock(distributedLock);
        EXPECT_EQ(connectionMap.size(), static_cast<size_t>(0));
    }

    RemoveSession(connect1);
    RemoveSession(connect2);
}

/**
 * @tc.name: ProcessDeviceOffline003
 * @tc.desc: process device offline with multiple online devices
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedConnectTest, ProcessDeviceOffline003, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest ProcessDeviceOffline003 start" << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName("", "ohos.demo.bundleName", "abilityName");
    auto& connectionMap = DistributedSchedService::GetInstance().distributedConnectAbilityMap_;
    auto& distributedLock = DistributedSchedService::GetInstance().distributedLock_;

    /**
     * @tc.steps: step1. add one session
     */
    sptr<AbilityConnectCallbackTest> connect = new AbilityConnectCallbackTest();
    AddSession(connect, "123_local_device_id", "123_remote_device_id", want);
    /**
     * @tc.steps: step2. process other device offline and check the map
     * @tc.expected: step2. still can find the newly-added connect session
     */
    DistributedSchedService::GetInstance().ProcessDeviceOffline("456_remote_device_id");
    {
        std::lock_guard<std::mutex> autoLock(distributedLock);
        auto iter = connectionMap.find(connect);
        EXPECT_NE(iter, connectionMap.end());
        EXPECT_EQ(connectionMap.size(), static_cast<size_t>(1));
    }

    RemoveSession(connect);
}

/**
 * @tc.name: DisconnectRemoteAbility001
 * @tc.desc: disconnect remote ability
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedConnectTest, DisconnectRemoteAbility001, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest DisconnectRemoteAbility001 start" << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName("", "ohos.demo.bundleName", "abilityName");
    auto& connectionMap = DistributedSchedService::GetInstance().distributedConnectAbilityMap_;
    auto& distributedLock = DistributedSchedService::GetInstance().distributedLock_;

    /**
     * @tc.steps: step1. add one session
     */
    sptr<AbilityConnectCallbackTest> connect = new AbilityConnectCallbackTest();
    AddSession(connect, "123_local_device_id", "123_remote_device_id", want);
    /**
     * @tc.steps: step2. disconnect the ability and check the map
     * @tc.expected: step2. the connect session is removed
     */
    DistributedSchedService::GetInstance().DisconnectRemoteAbility(connect);
    {
        std::lock_guard<std::mutex> autoLock(distributedLock);
        EXPECT_EQ(connectionMap.size(), static_cast<size_t>(0));
    }

    RemoveSession(connect);
}
}
}