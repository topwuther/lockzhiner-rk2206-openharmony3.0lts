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

#include "distributed_sched_proxy.h"
#include "distributed_sched_service.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "gtest/gtest.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "test_log.h"
#include "thread_pool.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace OHOS {
namespace DistributedSchedule {
using namespace AAFwk;
using namespace AppExecFwk;
namespace {
    const string LOCAL_DEVICEID = "192.168.43.100";
}

class DistributedSchedServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<IDistributedSched> GetDms();
    sptr<IDistributedSched> proxy_;

protected:
    enum class LoopTime : int32_t {
        LOOP_TIME = 10,
        LOOP_PRESSURE_TIME = 100,
    };
    void GetAbilityInfo(const std::string& package, const std::string& name,
        const std::string& bundleName, const std::string& deviceId,
        OHOS::AppExecFwk::AbilityInfo& abilityInfo);
};

void DistributedSchedServiceTest::SetUpTestCase()
{}

void DistributedSchedServiceTest::TearDownTestCase()
{}

void DistributedSchedServiceTest::SetUp()
{}

void DistributedSchedServiceTest::TearDown()
{}

sptr<IDistributedSched> DistributedSchedServiceTest::GetDms()
{
    if (proxy_ != nullptr) {
        return proxy_;
    }
    auto sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    if (sm == nullptr) {
        DTEST_LOG << "DistributedSchedServiceTest sm is nullptr" << std::endl;
        return nullptr;
    }
    DTEST_LOG << "DistributedSchedServiceTest sm is not nullptr" << std::endl;
    auto distributedObject = sm->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    EXPECT_TRUE(distributedObject != nullptr);
    proxy_ = iface_cast<IDistributedSched>(distributedObject);
    if (proxy_ == nullptr) {
        DTEST_LOG << "DistributedSchedServiceTest DistributedSched is nullptr" << std::endl;
    }
    DTEST_LOG << "DistributedSchedServiceTest DistributedSched is not nullptr" << std::endl;
    return proxy_;
}

void DistributedSchedServiceTest::GetAbilityInfo(const std::string& package, const std::string& name,
    const std::string& bundleName, const std::string& deviceId, OHOS::AppExecFwk::AbilityInfo& abilityInfo)
{
    abilityInfo.bundleName = bundleName;
    abilityInfo.deviceId = deviceId;
}

/**
 * @tc.name: StartRemoteAbility_001
 * @tc.desc: call StartRemoteAbility with illegal params
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedServiceTest, StartRemoteAbility_001, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest StartRemoteAbility_001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. StartRemoteAbility with uninitialized params
     * @tc.expected: step1. StartRemoteAbility return INVALID_PARAMETERS_ERR
     */
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    int result1 = proxy->StartRemoteAbility(want, abilityInfo, 0);
    DTEST_LOG << "result1:" << result1 << std::endl;
    /**
     * @tc.steps: step2. StartRemoteAbility with empty want's deviceId
     * @tc.expected: step2. StartRemoteAbility return INVALID_PARAMETERS_ERR
     */
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    int result2 = proxy->StartRemoteAbility(want, abilityInfo, 0);
    DTEST_LOG << "result2:" << result2 << std::endl;

    EXPECT_EQ(static_cast<int>(INVALID_PARAMETERS_ERR), result1);
    EXPECT_EQ(static_cast<int>(INVALID_PARAMETERS_ERR), result2);
    DTEST_LOG << "DistributedSchedServiceTest StartRemoteAbility_001 end" << std::endl;
}

/**
 * @tc.name: StartRemoteAbility_002
 * @tc.desc: call StartRemoteAbility with dms with wrong deviceId and local deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedServiceTest, StartRemoteAbility_002, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest StartRemoteAbility_002 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. set want with wrong deviceId
     * @tc.expected: step2. StartRemoteAbility return INVALID_PARAMETERS_ERR
     */
    AAFwk::Want want;
    AppExecFwk::ElementName element("123456", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    AppExecFwk::AbilityInfo abilityInfo;
    int result1 = DistributedSchedService::GetInstance().StartRemoteAbility(want, abilityInfo, 0);
    DTEST_LOG << "result:" << result1 << std::endl;
    std::string deviceId;
    DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(deviceId);
    AppExecFwk::ElementName element1(deviceId, "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element1);
    int result2 = DistributedSchedService::GetInstance().StartRemoteAbility(want, abilityInfo, 0);
    DTEST_LOG << "result:" << result2 << std::endl;
    EXPECT_EQ(static_cast<int>(INVALID_PARAMETERS_ERR), result1);
    EXPECT_EQ(static_cast<int>(INVALID_PARAMETERS_ERR), result2);
    DTEST_LOG << "DistributedSchedServiceTest StartRemoteAbility_002 end" << std::endl;
}

/**
 * @tc.name: StartRemoteAbility_003
 * @tc.desc: call StartRemoteAbility with dms
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedServiceTest, StartRemoteAbility_003, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest StartRemoteAbility_003 start" << std::endl;
    /**
     * @tc.steps: step1. set want with wrong deviceId
     * @tc.expected: step2. StartRemoteAbility return INVALID_PARAMETERS_ERR
     */
    AAFwk::Want want;
    AppExecFwk::ElementName element("123456", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    AppExecFwk::AbilityInfo abilityInfo;
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbility",
        "com.ohos.distributedmusicplayer", "192.168.43.101", abilityInfo);
    int result = DistributedSchedService::GetInstance().StartRemoteAbility(want, abilityInfo, 0);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(static_cast<int>(INVALID_PARAMETERS_ERR), result);
    DTEST_LOG << "DistributedSchedServiceTest StartRemoteAbility_003 end" << std::endl;
}

/**
 * @tc.name: StartRemoteAbility_004
 * @tc.desc: call StartRemoteAbility
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedServiceTest, StartRemoteAbility_004, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest StartRemoteAbility_004 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. set want and abilityInfo
     * @tc.expected: step2. StartRemoteAbility return INVALID_PARAMETERS_ERR
     */
    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    AppExecFwk::AbilityInfo abilityInfo;
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbility",
        "com.ohos.distributedmusicplayer", "192.168.43.101", abilityInfo);
    int result = proxy->StartRemoteAbility(want, abilityInfo, 0);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(static_cast<int>(INVALID_PARAMETERS_ERR), result);
    DTEST_LOG << "DistributedSchedServiceTest StartRemoteAbility_004 end" << std::endl;
}

/**
 * @tc.name: StartAbilityFromRemote_001
 * @tc.desc: call StartAbilityFromRemote with illegal param
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedServiceTest, StartAbilityFromRemote_001, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    /**
     * @tc.steps: step1. StartAbilityFromRemote with uninitialized params
     * @tc.expected: step1. StartAbilityFromRemote return INVALID_REMOTE_PARAMETERS_ERR
     */
    int result1 = proxy->StartAbilityFromRemote(want, abilityInfo, 0, callerInfo, accountInfo);
    DTEST_LOG << "result1:" << result1 << std::endl;
    /**
     * @tc.steps: step1. StartAbilityFromRemote with with empty deviceId
     * @tc.expected: step1. StartAbilityFromRemote return INVALID_REMOTE_PARAMETERS_ERR
     */
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbility",
        "com.ohos.distributedmusicplayer", "192.168.43.101", abilityInfo);
    int result2 = proxy->StartAbilityFromRemote(want, abilityInfo, 0, callerInfo, accountInfo);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(static_cast<int>(IPC_STUB_UNKNOW_TRANS_ERR), result1);
    EXPECT_EQ(static_cast<int>(IPC_STUB_UNKNOW_TRANS_ERR), result2);
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_001 end" << std::endl;
}

/**
 * @tc.name: StartAbilityFromRemote_002
 * @tc.desc: call StartAbilityFromRemote
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedServiceTest, StartAbilityFromRemote_002, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_002 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. set want and abilityInfo
     */
    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    AppExecFwk::AbilityInfo abilityInfo;
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbility",
        "com.ohos.distributedmusicplayer", "192.168.43.100", abilityInfo);
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    /**
     * @tc.steps: step2. StartAbilityFromRemote with abilityInfo which is of ACTIVITY type
     * @tc.expected: step2. StartAbilityFromRemote for result
     */
    int result1 = proxy->StartAbilityFromRemote(want, abilityInfo, 0, callerInfo, accountInfo);
    DTEST_LOG << "result1 is" << result1 << std::endl;
    /**
     * @tc.steps: step3. StartAbilityFromRemote with abilityInfo which is of SERVICE type
     * @tc.expected: step3. StartAbilityFromRemote for result
     */
    AppExecFwk::ElementName element2("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbilityService");
    want.SetElement(element2);
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbilityService",
        "com.ohos.distributedmusicplayer", "192.168.43.100", abilityInfo);
    int result2 = proxy->StartAbilityFromRemote(want, abilityInfo, 0, callerInfo, accountInfo);
    DTEST_LOG << "result2:" << result2 << std::endl;
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_002 end" << std::endl;
}

/**
 * @tc.name: StartAbilityFromRemote_003
 * @tc.desc: call StartAbilityFromRemote for pressure test
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedServiceTest, StartAbilityFromRemote_003, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_003 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. set want and abilityInfo
     */
    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    AppExecFwk::AbilityInfo abilityInfo;
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbility",
        "com.ohos.distributedmusicplayer", "192.168.43.100", abilityInfo);
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    /**
     * @tc.steps: step2. StartAbilityFromRemote for pressure test
     * @tc.expected: step2. StartAbilityFromRemote for result
     */
    for (int index = 0; index < static_cast<int32_t>(LoopTime::LOOP_TIME); index++) {
        int result = proxy->StartAbilityFromRemote(want, abilityInfo, 0, callerInfo, accountInfo);
        DTEST_LOG << "pressure" + to_string(index) + " result is " << result << std::endl;
    }
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_003 end" << std::endl;
}

/**
 * @tc.name: StartAbilityFromRemote_004
 * @tc.desc: call StartAbilityFromRemote with dms
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedServiceTest, StartAbilityFromRemote_004, TestSize.Level0)
{
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_004 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    /**
     * @tc.steps: step1. set want and abilityInfo
     */
    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    AppExecFwk::AbilityInfo abilityInfo;
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbility",
        "com.ohos.distributedmusicplayer", "192.168.43.100", abilityInfo);
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    /**
     * @tc.steps: step2. StartAbilityFromRemote with abilityInfo which is of ACTIVITY type
     * @tc.expected: step2. StartAbilityFromRemote for result
     */
    int result1 = DistributedSchedService::GetInstance().StartAbilityFromRemote(want,
        abilityInfo, 0, callerInfo, accountInfo);
    DTEST_LOG << "result1:" << result1 << std::endl;
    /**
     * @tc.steps: step3. StartAbilityFromRemote with abilityInfo which is of SERVICE type
     * @tc.expected: step3. StartAbilityFromRemote for result
     */
    AppExecFwk::ElementName element2("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbilityService");
    want.SetElement(element2);
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbilityService",
        "com.ohos.distributedmusicplayer", "192.168.43.100", abilityInfo);
    int result2 = DistributedSchedService::GetInstance().StartAbilityFromRemote(want,
        abilityInfo, 0, callerInfo, accountInfo);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(static_cast<int>(INVALID_REMOTE_PARAMETERS_ERR), result1);
    EXPECT_EQ(static_cast<int>(INVALID_REMOTE_PARAMETERS_ERR), result2);
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_004 end" << std::endl;
}

/**
 * @tc.name: StartAbilityFromRemote_005
 * @tc.desc: call StartAbilityFromRemote with dms
 * @tc.type: FUNC
 */
HWTEST_F(DistributedSchedServiceTest, StartAbilityFromRemote_005, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_005 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    /**
     * @tc.steps: step1. set want and abilityInfo
     */
    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    AppExecFwk::AbilityInfo abilityInfo;
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbility",
        "com.ohos.distributedmusicplayer", "192.168.43.100", abilityInfo);
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = 1;
    accountInfo.groupIdList.push_back("123456");
    /**
     * @tc.steps: step2. StartAbilityFromRemote with abilityInfo which is of ACTIVITY type
     * @tc.expected: step2. StartAbilityFromRemote for result
     */
    int result1 = DistributedSchedService::GetInstance().StartAbilityFromRemote(want,
        abilityInfo, 0, callerInfo, accountInfo);
    DTEST_LOG << "result1:" << result1 << std::endl;
    /**
     * @tc.steps: step3. StartAbilityFromRemote with abilityInfo which is of SERVICE type
     * @tc.expected: step3. StartAbilityFromRemote for result
     */
    AppExecFwk::ElementName element2("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbilityService");
    want.SetElement(element2);
    GetAbilityInfo("com.ohos.distributedmusicplayer", "com.ohos.distributedmusicplayer.MainAbilityService",
        "com.ohos.distributedmusicplayer", "192.168.43.100", abilityInfo);
    int result2 = DistributedSchedService::GetInstance().StartAbilityFromRemote(want,
        abilityInfo, 0, callerInfo, accountInfo);
    DTEST_LOG << "result2:" << result2 << std::endl;
    DTEST_LOG << "DistributedSchedServiceTest StartAbilityFromRemote_005 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS