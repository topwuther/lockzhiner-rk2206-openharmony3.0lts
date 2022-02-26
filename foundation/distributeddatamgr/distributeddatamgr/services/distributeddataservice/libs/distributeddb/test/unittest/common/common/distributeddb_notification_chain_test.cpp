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
#include "notification_chain.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace std;

namespace {
    const EventType COMMIT_EVENT = 1;
    const int INVALID_EVENT = 0;
    NotificationChain *g_notificationChain = nullptr;
    NotificationChain::Listener *g_listener = nullptr;
    int g_onEventTestNum = 0;
    bool g_onFinalizeCalled = false;

    auto g_onEventFunction = [](const void *arg) {
        g_onEventTestNum = *(reinterpret_cast<const int *>(arg));
        LOGI("g_onEventFunction called.");
    };

    auto g_onFinalize = []() {
        g_onFinalizeCalled = true;
        LOGI("g_onFinalize called.");
    };
}

class DistributedDBNotificationChainTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBNotificationChainTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Create a NotificationChain.
     */
    g_notificationChain = new (std::nothrow) NotificationChain();
    EXPECT_TRUE(g_notificationChain != nullptr);
}

void DistributedDBNotificationChainTest::TearDownTestCase(void)
{
    /**
     * @tc.setup: Release a NotificationChain.
     */
    g_notificationChain->OnLastRef([]() { LOGI("g_notificationChain finalize called."); });
    g_notificationChain->KillAndDecObjRef(g_notificationChain);
    g_notificationChain = nullptr;
}

void DistributedDBNotificationChainTest::SetUp(void)
{
    /**
     * @tc.setup: Register a listener to the NotificationChain
     */
    g_onEventTestNum = 0;
    g_onFinalizeCalled = false;
    int result = g_notificationChain->RegisterEventType(COMMIT_EVENT);
    EXPECT_TRUE(result == E_OK);
    int errCode = E_OK;
    g_listener = g_notificationChain->RegisterListener(COMMIT_EVENT, g_onEventFunction, g_onFinalize, errCode);
    EXPECT_TRUE(g_listener != nullptr);
}

void DistributedDBNotificationChainTest::TearDown(void)
{
    /**
     * @tc.setup: Unregister a listener to the NotificationChain
     */
    g_listener->Drop();
    g_listener = nullptr;
    EXPECT_TRUE(g_notificationChain->UnRegisterEventType(COMMIT_EVENT) == E_OK);
}

/**
 * @tc.name: RegisterEvent001
 * @tc.desc: Register an exits event.
 * @tc.type: FUNC
 * @tc.require: AR000BVDFP AR000CQDVI
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBNotificationChainTest, RegisterEvent001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. call RegisterEventType to register a exist event
     * @tc.expected: step1. function return -E_ALREADY_REGISTER
     */
    int result = g_notificationChain->RegisterEventType(COMMIT_EVENT);
    EXPECT_TRUE(result == -E_ALREADY_REGISTER);
}

/**
 * @tc.name: RegisterListener001
 * @tc.desc: Register and unregister a listener.
 * @tc.type: FUNC
 * @tc.require: AR000BVDFP AR000CQDVI
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBNotificationChainTest, RegisterListener001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. call RegisterListener to register a listener
     * @tc.expected: step1. function return a not null listener
     */
    int errCode = E_OK;
    NotificationChain::Listener *listener =
        g_notificationChain->RegisterListener(COMMIT_EVENT, g_onEventFunction, g_onFinalize, errCode);
    EXPECT_TRUE(listener != nullptr);

    /**
     * @tc.steps: step2. call Drop to unregister the listener
     * @tc.expected: step2. function return E_OK
     */
    int result = listener->Drop();
    EXPECT_TRUE(g_onFinalizeCalled);
    EXPECT_TRUE(result == E_OK);
}

/**
 * @tc.name: NotifyEvent001
 * @tc.desc: notify an event to listener.
 * @tc.type: FUNC
 * @tc.require: AR000BVDFQ AR000CQDVJ
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBNotificationChainTest, NotifyEvent001, TestSize.Level0)
{
    int errCode = E_OK;
    /**
     * @tc.steps: step1. call RegisterListener to register a listener
     * @tc.expected: step1. function return a not null listener
     */
    NotificationChain::Listener *listener =
        g_notificationChain->RegisterListener(COMMIT_EVENT, g_onEventFunction, g_onFinalize, errCode);
    EXPECT_TRUE(listener != nullptr);

    /**
     * @tc.steps: step2. call NotifyEvent to notify an event
     * @tc.expected: step2. the listener's callback should be called
     */
    int testNum = 2048;
    g_notificationChain->NotifyEvent(COMMIT_EVENT, &testNum);
    EXPECT_TRUE(g_onEventTestNum == testNum);
    listener->Drop();
}

/**
 * @tc.name: UnRegisterEvent001
 * @tc.desc: unregister a invalid event.
 * @tc.type: FUNC
 * @tc.require: AR000BVDFP AR000CQDVI
 * @tc.author: xushaohua
 */
HWTEST_F(DistributedDBNotificationChainTest, UnRegisterEvent001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. UnRegisterEventType a invalid event
     * @tc.expected: step1. function should return -E_NOT_FOUND
     */
    int result = g_notificationChain->UnRegisterEventType(INVALID_EVENT);
    EXPECT_EQ(result, -E_NOT_FOUND);
}
