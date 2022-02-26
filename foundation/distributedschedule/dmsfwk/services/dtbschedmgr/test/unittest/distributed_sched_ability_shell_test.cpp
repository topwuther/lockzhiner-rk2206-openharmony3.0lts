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

#include "distributed_sched_ability_shell.h"
#include "distributed_sched_service.h"
#include "dtbschedmgr_log.h"
#include "gtest/gtest.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_distributed_sched.h"
#include "ohos/aafwk/content/want.h"
#include "system_ability_definition.h"
#include "test_log.h"

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace AAFwk;
using namespace AppExecFwk;

namespace OHOS {
namespace DistributedSchedule {
namespace {
constexpr int32_t LOOP_TIMES = 100;
}

class DistributedSchedAbilityShellTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<IDistributedSched> proxy_;
protected:
    sptr<IDistributedSched> GetDms();
};

void DistributedSchedAbilityShellTest::SetUpTestCase()
{
}

void DistributedSchedAbilityShellTest::TearDownTestCase()
{
}

void DistributedSchedAbilityShellTest::SetUp()
{
}

void DistributedSchedAbilityShellTest::TearDown()
{
}

sptr<IDistributedSched> DistributedSchedAbilityShellTest::GetDms()
{
    if (proxy_ != nullptr) {
        return proxy_;
    }
    auto sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    if (sm == nullptr) {
        DTEST_LOG << "DistributedSchedAbilityShellTest sm is nullptr" << std::endl;
        return nullptr;
    }
    auto distributedObject = sm->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    EXPECT_TRUE(distributedObject != nullptr);
    proxy_ = iface_cast<IDistributedSched>(distributedObject);
    if (proxy_ == nullptr) {
        DTEST_LOG << "DistributedSchedAbilityShellTest distributedObject is nullptr" << std::endl;
    }
    return proxy_;
}

/**
 * @tc.name: RegisterAbilityToken001
 * @tc.desc: register ability token with null token.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RegisterAbilityToken001, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken001 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token with null token
     * @tc.expected: step1. return ERR_NULL_OBJECT.
     */
    int result = proxy->RegisterAbilityToken(nullptr, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    EXPECT_EQ(static_cast<int>(ERR_NULL_OBJECT), result);
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken001 end" << std::endl;
}

/**
 * @tc.name: RegisterAbilityToken002
 * @tc.desc: register ability token with null callback.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RegisterAbilityToken002, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken002 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token with null callback
     * @tc.expected: step1. return ERR_NULL_OBJECT.
     */
    int result = proxy->RegisterAbilityToken(token, nullptr);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    EXPECT_EQ(static_cast<int>(ERR_NULL_OBJECT), result);
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken002 end" << std::endl;
}

/**
 * @tc.name: RegisterAbilityToken003
 * @tc.desc: register ability token with callback and token.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RegisterAbilityToken003, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken003 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = proxy->RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    EXPECT_EQ(static_cast<int>(ERR_OK), result);
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken003 end" << std::endl;
}

/**
 * @tc.name: RegisterAbilityToken004
 * @tc.desc: register ability token with callback and two tokens.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RegisterAbilityToken004, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken004 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> token1 = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = proxy->RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    /**
     * @tc.steps: step2. register ability token with callback and token1
     * @tc.expected: step2. return ERR_OK.
     */
    int result1 = proxy->RegisterAbilityToken(token1, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result1: "<< result1 << std::endl;

    EXPECT_EQ(static_cast<int>(ERR_OK), result);
    EXPECT_EQ(static_cast<int>(ERR_OK), result1);
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken004 end" << std::endl;
}

/**
 * @tc.name: RegisterAbilityToken005
 * @tc.desc: register ability token with two callbacks and two tokens.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RegisterAbilityToken005, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken005 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> token1 = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback1 = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = proxy->RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    /**
     * @tc.steps: step2. register ability token with callback1 and token1
     * @tc.expected: step2. return ERR_OK.
     */
    int result1 = proxy->RegisterAbilityToken(token1, continuationCallback1);
    DTEST_LOG << "DistributedSchedAbilityShellTest result1: "<< result1 << std::endl;

    EXPECT_EQ(static_cast<int>(ERR_OK), result);
    EXPECT_EQ(static_cast<int>(ERR_OK), result1);
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken005 end" << std::endl;
}

/**
 * @tc.name: RegisterAbilityToken006
 * @tc.desc: register ability token with callback and two repeat tokens.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RegisterAbilityToken006, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken006 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = proxy->RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    /**
     * @tc.steps: step2. register ability token with callback and repeat token
     * @tc.expected: step2. return REG_REPEAT_ABILITY_TOKEN_ERR.
     */
    int result1 = proxy->RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result1: "<< result1 << std::endl;

    EXPECT_EQ(static_cast<int>(REG_REPEAT_ABILITY_TOKEN_ERR), result1);
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken006 end" << std::endl;
}

/**
 * @tc.name: RegisterAbilityToken007
 * @tc.desc: register ability token with two callbacks and two tokens with dms.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RegisterAbilityToken007, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken007 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> token1 = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback1 = new MockDistributedSched();

    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = DistributedSchedService::GetInstance().RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    /**
     * @tc.steps: step2. register ability token with callback1 and token1
     * @tc.expected: step2. return ERR_OK.
     */
    int result1 = DistributedSchedService::GetInstance().RegisterAbilityToken(token1, continuationCallback1);
    DTEST_LOG << "DistributedSchedAbilityShellTest result1: "<< result1 << std::endl;

    EXPECT_EQ(static_cast<int>(ERR_OK), result);
    EXPECT_EQ(static_cast<int>(ERR_OK), result1);
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken007 end" << std::endl;
}

/**
 * @tc.name: RegisterAbilityToken008
 * @tc.desc: register ability token with callback and two repeat tokens.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RegisterAbilityToken008, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken008 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = DistributedSchedService::GetInstance().RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    /**
     * @tc.steps: step2. register ability token with callback and repeat token
     * @tc.expected: step2. return REG_REPEAT_ABILITY_TOKEN_ERR.
     */
    int result1 = DistributedSchedService::GetInstance().RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result1: "<< result1 << std::endl;

    EXPECT_EQ(static_cast<int>(REG_REPEAT_ABILITY_TOKEN_ERR), result1);
    sptr<MockDistributedSched> continuationCallback1 = new MockDistributedSched();
    DistributedSchedService::GetInstance().UnregisterAbilityToken(token, continuationCallback1);
    DistributedSchedService::GetInstance().UnregisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterAbilityToken008 end" << std::endl;
}

/**
 * @tc.name: UnregisterAbilityToken001
 * @tc.desc: unregister ability token with null token.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, UnregisterAbilityToken001, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken001 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. unregister ability token with null token
     * @tc.expected: step1. return ERR_NULL_OBJECT.
     */
    int result = proxy->UnregisterAbilityToken(nullptr, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    EXPECT_EQ(static_cast<int>(ERR_NULL_OBJECT), result);
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken001 end" << std::endl;
}

/**
 * @tc.name: UnregisterAbilityToken002
 * @tc.desc: unregister ability token with null token.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, UnregisterAbilityToken002, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken002 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. unregister ability token with null callback
     * @tc.expected: step1. return ERR_NULL_OBJECT.
     */
    int result = proxy->UnregisterAbilityToken(token, nullptr);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    EXPECT_EQ(static_cast<int>(ERR_NULL_OBJECT), result);
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken002 end" << std::endl;
}

/**
 * @tc.name: UnregisterAbilityToken003
 * @tc.desc: unregister ability token with callback and token.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, UnregisterAbilityToken003, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken003 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = proxy->RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;
    EXPECT_EQ(static_cast<int>(ERR_OK), result);
    /**
     * @tc.steps: step2. unregister ability token with token and callback
     * @tc.expected: step2. return ERR_OK.
     */
    int result1 = proxy->UnregisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result1: "<< result1 << std::endl;
    EXPECT_EQ(static_cast<int>(ERR_OK), result1);
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken003 end" << std::endl;
}

/**
 * @tc.name: UnregisterAbilityToken004
 * @tc.desc: unregister ability token with wrong callback.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, UnregisterAbilityToken004, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken004 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = proxy->RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;
    EXPECT_EQ(static_cast<int>(ERR_OK), result);
    /**
     * @tc.steps: step2. unregister ability token with token and callback
     * @tc.expected: step2. return NO_APP_THREAD_ERR.
     */
    sptr<MockDistributedSched> continuationCallback1 = new MockDistributedSched();
    int result1 = proxy->UnregisterAbilityToken(token, continuationCallback1);
    DTEST_LOG << "DistributedSchedAbilityShellTest result1: "<< result1 << std::endl;
    EXPECT_EQ(static_cast<int>(NO_APP_THREAD_ERR), result1);
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken004 end" << std::endl;
}

/**
 * @tc.name: UnregisterAbilityToken005
 * @tc.desc: unregister ability token with wrong callback.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, UnregisterAbilityToken005, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken005 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = proxy->RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;
    EXPECT_EQ(static_cast<int>(ERR_OK), result);
    /**
     * @tc.steps: step2. unregister ability token with token and callback
     * @tc.expected: step2. return NO_ABILITY_TOKEN_ERR.
     */
    sptr<MockDistributedSched> token1 = new MockDistributedSched();
    int result1 = proxy->UnregisterAbilityToken(token1, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result1: "<< result1 << std::endl;
    EXPECT_EQ(static_cast<int>(NO_ABILITY_TOKEN_ERR), result1);
    DTEST_LOG << "DistributedSchedAbilityShellTest UnregisterAbilityToken005 end" << std::endl;
}

/**
 * @tc.name: RegisterUnregisterAbilityTokenPressure001
 * @tc.desc: register and unregister ability token pressure.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RegisterUnregisterAbilityTokenPressure001, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterUnregisterAbilityTokenPressure001 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. register ability token for 100 times
     */
    for (int index = 0; index < LOOP_TIMES; ++index) {
        proxy->RegisterAbilityToken(token, continuationCallback);
    }

    /**
     * @tc.steps: step2. unregister ability token for 100 times
     */
    for (int index = 0; index < LOOP_TIMES; ++index) {
        proxy->UnregisterAbilityToken(token, continuationCallback);
    }
    DTEST_LOG << "DistributedSchedAbilityShellTest RegisterUnregisterAbilityTokenPressure001 end" << std::endl;
}

/**
 * @tc.name: RemoveContinuationCallback001
 * @tc.desc: remove continuation callback with dms.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, RemoveContinuationCallback001, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest RemoveContinuationCallback001 start" << std::endl;
    AppExecFwk::AbilityInfo abilityInfo;
    sptr<MockDistributedSched> token = new MockDistributedSched();
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    /**
     * @tc.steps: step1. register ability token with callback and token
     * @tc.expected: step1. return ERR_OK.
     */
    int result = DistributedSchedService::GetInstance().RegisterAbilityToken(token, continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest result: "<< result << std::endl;

    /**
     * @tc.steps: step2. remove continuation callback with dms
     * @tc.expected: step2. return ERR_OK.
     */
    DistributedSchedAbilityShell::GetInstance().RemoveContinuationCallback(continuationCallback);

    EXPECT_EQ(static_cast<int>(ERR_OK), result);
    DTEST_LOG << "DistributedSchedAbilityShellTest RemoveContinuationCallback001 end" << std::endl;
}

/**
 * @tc.name: ContinuationCallbackAddDeathRecipient001
 * @tc.desc: add death recipient for continuation callback.
 * @tc.type: FUNC
 */

HWTEST_F(DistributedSchedAbilityShellTest, ContinuationCallbackAddDeathRecipient001, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedAbilityShellTest ContinuationCallbackAddDeathRecipient001 start" << std::endl;
    sptr<MockDistributedSched> continuationCallback = new MockDistributedSched();
    /**
     * @tc.steps: step1. add death recipient for continuation callback
     */
    sptr<IRemoteObject::DeathRecipient>(new ContinuationCallbackDeathRecipient())->OnRemoteDied(continuationCallback);
    DTEST_LOG << "DistributedSchedAbilityShellTest ContinuationCallbackAddDeathRecipient001 end" << std::endl;
}
} // DistributedSchedule
} // namespace OHOS
