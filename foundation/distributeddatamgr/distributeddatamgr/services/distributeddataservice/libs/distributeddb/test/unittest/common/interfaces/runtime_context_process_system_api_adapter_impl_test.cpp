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

#include "db_errno.h"
#include "log_print.h"
#include "runtime_context.h"
#include "iprocess_system_api_adapter.h"
#include "process_system_api_adapter_impl.h"
#include "lock_status_observer.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace std;

namespace {
    const std::string DATA_FILE_PATH = "/data/test/";
    SecurityOption g_option = {0, 0};
    const std::string DEV_ID = "devId";
    std::shared_ptr<ProcessSystemApiAdapterImpl> g_adapter;
}

class RuntimeContextProcessSystemApiAdapterImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
};

void RuntimeContextProcessSystemApiAdapterImplTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Get a adapter
     */
    g_adapter = std::make_shared<ProcessSystemApiAdapterImpl>();
    EXPECT_TRUE(g_adapter != nullptr);
}

void RuntimeContextProcessSystemApiAdapterImplTest::TearDownTestCase(void)
{
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
}

void RuntimeContextProcessSystemApiAdapterImplTest::SetUp(void)
{
    g_adapter->ResetAdapter();
}

/**
 * @tc.name: SetSecurityOption001
 * @tc.desc: Set SecurityOption.
 * @tc.type: FUNC
 * @tc.require: AR000EV1G2
 */
HWTEST_F(RuntimeContextProcessSystemApiAdapterImplTest, SetSecurityOption001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. call SetSecurityOption to set SecurityOption before set g_adapter
     * @tc.expected: step1. function return E_NOT_SUPPORT
     */
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
    int errCode = RuntimeContext::GetInstance()->SetSecurityOption(DATA_FILE_PATH, g_option);
    EXPECT_TRUE(errCode == -E_NOT_SUPPORT);

    /**
     * @tc.steps: step2. call SetSecurityOption to set SecurityOption after set g_adapter
     * @tc.expected: step2. function return E_OK
     */
    EXPECT_TRUE(g_adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(g_adapter);
    errCode = RuntimeContext::GetInstance()->SetSecurityOption(DATA_FILE_PATH, g_option);
    EXPECT_TRUE(errCode == E_OK);
}

/**
 * @tc.name: GetSecurityOption001
 * @tc.desc: Get SecurityOption.
 * @tc.type: FUNC
 * @tc.require: AR000EV1G2
 */
HWTEST_F(RuntimeContextProcessSystemApiAdapterImplTest, GetSecurityOption001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. call GetSecurityOption to get SecurityOption before set g_adapter
     * @tc.expected: step1. function return E_NOT_SUPPORT
     */
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
    int errCode = RuntimeContext::GetInstance()->GetSecurityOption(DATA_FILE_PATH, g_option);
    EXPECT_TRUE(errCode == -E_NOT_SUPPORT);

    /**
     * @tc.steps: step2. call GetSecurityOption to get SecurityOption after set g_adapter
     * @tc.expected: step2. function return E_OK
     */
    EXPECT_TRUE(g_adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(g_adapter);
    errCode = RuntimeContext::GetInstance()->GetSecurityOption(DATA_FILE_PATH, g_option);
    EXPECT_TRUE(errCode == E_OK);
}

/**
 * @tc.name: RegisterLockStatusLister001
 * @tc.desc: Register a listener.
 * @tc.type: FUNC
 * @tc.require: AR000EV1G2
 */
HWTEST_F(RuntimeContextProcessSystemApiAdapterImplTest, RegisterLockStatusLister001, TestSize.Level0)
{
    int errCode = E_OK;
    bool lockStatus = false;
    auto onEventFunction1 = [&lockStatus](void *isLock) {
        LOGI("lock status 1 changed %d", *(static_cast<bool *>(isLock)));
        lockStatus = *(static_cast<bool *>(isLock));
    };

    auto onEventFunction2 = [&lockStatus](void *isLock) {
        LOGI("lock status 2 changed %d", *(static_cast<bool *>(isLock)));
        lockStatus = *(static_cast<bool *>(isLock));
    };
    /**
     * @tc.steps: step1. call RegisterLockStatusLister to register a listener before set adapter
     * @tc.expected: step1. function return ok
     */
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
    NotificationChain::Listener *listener =
        RuntimeContext::GetInstance()->RegisterLockStatusLister(onEventFunction1, errCode);
    EXPECT_NE(listener, nullptr);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step2. call RegisterLockStatusLister to register a listener after set g_adapter
     * @tc.expected: step2. function return a not null listener
     */
    EXPECT_TRUE(g_adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(g_adapter);

    auto listener1 = RuntimeContext::GetInstance()->RegisterLockStatusLister(onEventFunction1, errCode);
    EXPECT_TRUE(errCode == E_OK);
    EXPECT_NE(listener1, nullptr);
    listener1->Drop();

    /**
     * @tc.steps: step3. call SetLockStatus to change lock status
     * @tc.expected: step3. the listener's callback should be called
     */
    g_adapter->SetLockStatus(false);
    EXPECT_TRUE(!lockStatus);

    /**
     * @tc.steps: step4. call RegisterLockStatusLister to register another listener after set g_adapter
     * @tc.expected: step4. function return a not null listener
     */
    listener->Drop();
    listener = RuntimeContext::GetInstance()->RegisterLockStatusLister(onEventFunction2, errCode);
    EXPECT_NE(listener, nullptr);
    listener->Drop();
}

/**
 * @tc.name: IsAccessControlled001
 * @tc.desc: Get Access Lock Status.
 * @tc.type: FUNC
 * @tc.require: AR000EV1G2
 */
HWTEST_F(RuntimeContextProcessSystemApiAdapterImplTest, IsAccessControlled001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. call IsAccessControlled to get Access lock status before set g_adapter
     * @tc.expected: step1. function return true
     */
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
    bool isLocked = RuntimeContext::GetInstance()->IsAccessControlled();
    EXPECT_FALSE(isLocked);

    /**
     * @tc.steps: step2. IsAccessControlled to get Access lock status after set g_adapter
     * @tc.expected: step2. function return false
     */
    EXPECT_TRUE(g_adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(g_adapter);
    isLocked = RuntimeContext::GetInstance()->IsAccessControlled();
    EXPECT_TRUE(!isLocked);
}

/**
 * @tc.name: CheckDeviceSecurityAbility001
 * @tc.desc: Check device security ability.
 * @tc.type: FUNC
 * @tc.require: AR000EV1G2
 */
HWTEST_F(RuntimeContextProcessSystemApiAdapterImplTest, CheckDeviceSecurityAbility001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. call CheckDeviceSecurityAbility to check device security ability before set g_adapter
     * @tc.expected: step1. function return true
     */
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(nullptr);
    bool isSupported = RuntimeContext::GetInstance()->CheckDeviceSecurityAbility(DEV_ID, g_option);
    EXPECT_TRUE(isSupported);

    /**
     * @tc.steps: step2. IsAccessControlled to check device security ability after set g_adapter
     * @tc.expected: step2. function return true
     */
    EXPECT_TRUE(g_adapter != nullptr);
    RuntimeContext::GetInstance()->SetProcessSystemApiAdapter(g_adapter);
    isSupported = RuntimeContext::GetInstance()->CheckDeviceSecurityAbility(DEV_ID, g_option);
    EXPECT_TRUE(isSupported);
}