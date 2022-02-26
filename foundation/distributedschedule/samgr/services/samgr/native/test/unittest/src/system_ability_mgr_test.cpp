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

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "itest_transaction_service.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "system_ability_mgr_test.h"
#include "test_log.h"

using namespace std;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace {
constexpr int32_t TEST_VALUE = 2021;
constexpr int32_t TEST_REVERSE_VALUE = 1202;
constexpr int32_t REPEAT = 10;
constexpr int32_t TEST_EXCEPTION_HIGH_SA_ID = LAST_SYS_ABILITY_ID + 1;
constexpr int32_t TEST_EXCEPTION_LOW_SA_ID = FIRST_SYS_ABILITY_ID - 1;
}
void SystemAbilityMgrTest::SetUpTestCase()
{
    DTEST_LOG << "SetUpTestCase" << std::endl;
}

void SystemAbilityMgrTest::TearDownTestCase()
{
    DTEST_LOG << "TearDownTestCase" << std::endl;
}

void SystemAbilityMgrTest::SetUp()
{
    DTEST_LOG << "SetUp" << std::endl;
}

void SystemAbilityMgrTest::TearDown()
{
    DTEST_LOG << "TearDown" << std::endl;
}

/*
 * Feature: SAMGR
 * Function: AddSystemAbility
 * SubFunction: AddSystemAbility
 * FunctionPoints: add system ability, input invalid parameter
 * CaseDescription: test AddSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, nullptr);
    DTEST_LOG << "add TestTransactionService result = " << result << std::endl;
    EXPECT_TRUE(result != ERR_OK);
}

/*
 * Feature: SAMGR
 * Function: AddSystemAbility
 * SubFunction: AddSystemAbility
 * FunctionPoints: add system ability
 * CaseDescription: test AddSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    int32_t result = sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    DTEST_LOG << "add TestTransactionService result = " << result << std::endl;
    EXPECT_EQ(result, ERR_OK);
    sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/*
 * Feature: SAMGR
 * Function: AddSystemAbility
 * SubFunction: AddSystemAbility
 * FunctionPoints: add system ability saId exception
 * CaseDescription: test AddSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    int32_t result = sm->AddSystemAbility(TEST_EXCEPTION_HIGH_SA_ID, testAbility);
    EXPECT_TRUE(result != ERR_OK);
    result = sm->AddSystemAbility(TEST_EXCEPTION_LOW_SA_ID, testAbility);
    EXPECT_TRUE(result != ERR_OK);
    sm->RemoveSystemAbility(TEST_EXCEPTION_HIGH_SA_ID);
    sm->RemoveSystemAbility(TEST_EXCEPTION_LOW_SA_ID);
}

/*
 * Feature: SAMGR
 * Function: AddSystemAbility
 * SubFunction: AddSystemAbility
 * FunctionPoints: add system ability with illegal capability
 * CaseDescription: test AddSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility004, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    std::u16string capability = u"illegal capability";
    ISystemAbilityManager::SAExtraProp saExtraProp(false, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        capability, u"");
    int32_t ret = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_TRUE(ret != ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/*
 * Feature: SAMGR
 * Function: AddSystemAbility
 * SubFunction: AddSystemAbility
 * FunctionPoints: add system ability with validated capability
 * CaseDescription: test AddSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, AddSystemAbility005, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_SO_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    std::u16string capability = u"{\"Capabilities\":{\"aaa\":\"[10.4, 20.5]\",\"bbb\":\"[11, 55]\",\
        \"ccc\":\"this is string\", \"ddd\":\"[aa, bb, cc, dd]\", \"eee\":5.60, \"fff\":4545, \"ggg\":true}}";
    ISystemAbilityManager::SAExtraProp saExtraProp(true, ISystemAbilityManager::DUMP_FLAG_PRIORITY_DEFAULT,
        capability, u"");
    int32_t ret = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(ret, ERR_OK);
    sm->RemoveSystemAbility(systemAbilityId);
}

/*
 * Feature: SAMGR
 * Function: RemoveSystemAbility
 * SubFunction: RemoveSystemAbility
 * FunctionPoints: remove not exist system ability
 * CaseDescription: test RemoveSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    int32_t result = sm->RemoveSystemAbility(0);
    EXPECT_TRUE(result != ERR_OK);
}

/*
 * Feature: SAMGR
 * Function: RemoveSystemAbility
 * SubFunction: RemoveSystemAbility
 * FunctionPoints: remove system ability
 * CaseDescription: test RemoveSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, RemoveSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    int32_t result = sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: SAMGR
 * Function: GetSystemAbility
 * SubFunction: GetSystemAbility
 * FunctionPoints: get not exist system ability
 * CaseDescription: test GetSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    auto ability = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(ability, nullptr);
}

/*
 * Feature: SAMGR
 * Function: GetSystemAbility
 * SubFunction: GetSystemAbility
 * FunctionPoints: get system ability
 * CaseDescription: test GetSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    auto ability = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_TRUE(ability != nullptr);
    sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/*
 * Feature: SAMGR
 * Function: GetSystemAbility
 * SubFunction: GetSystemAbility
 * FunctionPoints: get system ability and then transaction
 * CaseDescription: test GetSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility003, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, testAbility);
    auto ability = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_TRUE(ability != nullptr);
    sptr<ITestTransactionService> targetAblility = iface_cast<ITestTransactionService>(ability);
    EXPECT_TRUE(targetAblility != nullptr);
    int32_t rep = 0;
    int32_t result = targetAblility->ReverseInt(TEST_VALUE, rep);
    DTEST_LOG << "testAbility ReverseInt result = " << result << ", get reply = " << rep << std::endl;
    EXPECT_EQ(rep, TEST_REVERSE_VALUE);
    sm->RemoveSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID);
}

/*
 * Feature: SAMGR
 * Function: GetSystemAbility
 * SubFunction: GetSystemAbility
 * FunctionPoints: get system ability
 * CaseDescription: test GetSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility004, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    for (int32_t i = 0; i < REPEAT; ++i) {
        auto result = sm->AddSystemAbility((DISTRIBUTED_SCHED_TEST_SO_ID + i), new TestTransactionService());
        EXPECT_EQ(result, ERR_OK);
    }
    for (int32_t i = 0; i < REPEAT; ++i) {
        int32_t saId = DISTRIBUTED_SCHED_TEST_SO_ID + i;
        auto saObject = sm->GetSystemAbility(saId);
        EXPECT_TRUE(saObject != nullptr);
        sm->RemoveSystemAbility(saId);
    }
}

/*
 * Feature: SAMGR
 * Function: GetSystemAbility
 * SubFunction: GetSystemAbility
 * FunctionPoints: get remote device system ability
 * CaseDescription: test GetSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbility005, TestSize.Level2)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    string fakeDeviceId = "fake_dev";
    auto abilityObj = sm->GetSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, fakeDeviceId);
    EXPECT_EQ(abilityObj, nullptr);
    sm->SubscribeSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, u"test.listener.service");
    sm->UnSubscribeSystemAbility(DISTRIBUTED_SCHED_TEST_TT_ID, u"test.listener.service");
    sm->CheckLocalAbilityManager(u"test2.service");
}

/*
 * Feature: SAMGR
 * Function: GetSystemAbilityInfoList
 * SubFunction: GetSystemAbilityInfoList
 * FunctionPoints: get system ability info list
 * CaseDescription: test GetSystemAbilityInfoList interface with invalid capability
 */
HWTEST_F(SystemAbilityMgrTest, GetSystemAbilityInfoList001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    std::u16string capability = u"   ";
    std::list<std::shared_ptr<SystemAbilityInfo>> saInfoList;
    bool ret = sm->GetSystemAbilityInfoList(systemAbilityId, capability, saInfoList);
    EXPECT_FALSE(ret);
    capability = u"illegal capability";
    ret = sm->GetSystemAbilityInfoList(systemAbilityId, capability, saInfoList);
    EXPECT_FALSE(ret);
}

/*
 * Feature: SAMGR
 * Function: SystemAbilityInfo
 * SubFunction: SystemAbilityInfo
 * FunctionPoints: SystemAbilityInfo parcel read and write
 * CaseDescription: test SystemAbilityInfo parcel read and write
 */
HWTEST_F(SystemAbilityMgrTest, SystemAbilityInfo001, TestSize.Level1)
{
    unique_ptr<SystemAbilityInfo> saInfoIn = make_unique<SystemAbilityInfo>();
    saInfoIn->systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    saInfoIn->deviceId = "deviceId";
    MessageParcel in;
    bool ret = saInfoIn->Marshalling(in);
    EXPECT_TRUE(ret);
    unique_ptr<SystemAbilityInfo> spSaInfoOut(SystemAbilityInfo::Unmarshalling(in));
    if (spSaInfoOut == nullptr) {
        return;
    }
    EXPECT_EQ(spSaInfoOut->systemAbilityId, DISTRIBUTED_SCHED_TEST_TT_ID);
    EXPECT_EQ(spSaInfoOut->deviceId, "deviceId");
}

/*
 * Feature: SAMGR
 * Function: CheckSystemAbility
 * SubFunction: CheckSystemAbility
 * FunctionPoints: check system ability
 * CaseDescription: test CheckSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, CheckSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sptr<IRemoteObject> testAbility = new TestTransactionService();
    sm->AddSystemAbility(systemAbilityId, testAbility);
    sptr<IRemoteObject> abilityObj = sm->CheckSystemAbility(systemAbilityId);
    EXPECT_TRUE(abilityObj != nullptr);
    sm->RemoveSystemAbility(systemAbilityId);
}

/*
 * Feature: SAMGR
 * Function: CheckOnDemandSystemAbility
 * SubFunction: CheckOnDemandSystemAbility
 * FunctionPoints: check on demand system ability
 * CaseDescription: test AddOnDemandSystemAbilityInfo and CheckOnDemandSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, CheckOnDemandSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sm->AddSystemAbility(systemAbilityId, new TestTransactionService());
    int32_t ret = sm->AddOnDemandSystemAbilityInfo(systemAbilityId, u"test_localmanagername");
    EXPECT_EQ(ret, ERR_OK);
    std::u16string localAbilityName = sm->CheckOnDemandSystemAbility(systemAbilityId);
    EXPECT_TRUE(!localAbilityName.empty());
    sm->RemoveSystemAbility(systemAbilityId);
}

/*
 * Feature: SAMGR
 * Function: CheckOnDemandSystemAbility
 * SubFunction: CheckOnDemandSystemAbility
 * FunctionPoints: check on demand system ability
 * CaseDescription: test AddOnDemandSystemAbilityInfo and CheckOnDemandSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, CheckOnDemandSystemAbility002, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    ISystemAbilityManager::SAExtraProp saExtraProp(false, 0, u"", u"");
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    int32_t result = sm->AddSystemAbility(systemAbilityId, new TestTransactionService(), saExtraProp);
    EXPECT_EQ(result, ERR_OK);
    sptr<IRemoteObject> saObject = sm->CheckSystemAbility(systemAbilityId);
    result = sm->AddOnDemandSystemAbilityInfo(systemAbilityId, u"just_test");
    EXPECT_EQ(result, ERR_OK);
    auto localAbilityManager = sm->CheckOnDemandSystemAbility(systemAbilityId);
    EXPECT_TRUE(!localAbilityManager.empty());
    sm->RemoveSystemAbility(systemAbilityId);
}

/*
 * Feature: SAMGR
 * Function: ListSystemAbility
 * SubFunction: ListSystemAbility
 * FunctionPoints: list all system abilities
 * CaseDescription: test ListSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, ListSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    sm->AddSystemAbility(systemAbilityId, new TestTransactionService());
    auto saList = sm->ListSystemAbilities();
    EXPECT_TRUE(!saList.empty());
    auto iter = std::find(saList.begin(), saList.end(), to_utf16(std::to_string(systemAbilityId)));
    EXPECT_TRUE(iter != saList.end());
    sm->RemoveSystemAbility(systemAbilityId);
}

/*
 * Feature: SAMGR
 * Function: GetLocalAbilityManager
 * SubFunction: GetLocalAbilityManager
 * FunctionPoints: get local ability manager
 * CaseDescription: test GetLocalAbilityManager interface
 */
HWTEST_F(SystemAbilityMgrTest, GetLocalAbilityManager001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    u16string locAblilityName(u"123");
    sptr<IRemoteObject> localAbilityMgr = sm->CheckLocalAbilityManager(locAblilityName);
    EXPECT_TRUE(localAbilityMgr == nullptr);
    sm->RemoveLocalAbilityManager(locAblilityName);
}

/*
 * Feature: SAMGR
 * Function: ConnectSystemAbility
 * SubFunction: ConnectSystemAbility
 * FunctionPoints: connect system ability
 * CaseDescription: test ConnectSystemAbility interface
 */
HWTEST_F(SystemAbilityMgrTest, ConnectSystemAbility001, TestSize.Level1)
{
    int32_t systemAbilityId = DISTRIBUTED_SCHED_TEST_TT_ID;
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    auto result = sm->ConnectSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
    result = sm->DisConnectSystemAbility(systemAbilityId, nullptr);
    EXPECT_TRUE(result != ERR_OK);
}
} // namespace OHOS
