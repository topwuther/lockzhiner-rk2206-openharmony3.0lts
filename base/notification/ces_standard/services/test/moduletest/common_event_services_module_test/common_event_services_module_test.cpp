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
#include <string>

#include "common_event_listener.h"
#define private public
#define protected public
#include "common_event.h"
#include "common_event_manager_service.h"
#include "common_event_manager.h"
#undef private
#undef protected
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "datetime_ex.h"
#include "event_log_wrapper.h"
#include "event_receive_stub.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "system_ability_definition.h"

using namespace testing::ext;

namespace OHOS {
namespace EventFwk {
namespace {
std::mutex mtx_;
const time_t TIME_OUT_SECONDS_LIMIT = 3;
static OHOS::sptr<OHOS::AppExecFwk::MockBundleMgrService> bundleObject = nullptr;
std::shared_ptr<EventHandler> handlerPtr;
}  // namespace

class CommonEventServicesModuleTest : public CommonEventSubscriber {
public:
    CommonEventServicesModuleTest(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventServicesModuleTest(){};
    virtual void OnReceiveEvent(const CommonEventData &data);

public:
};
CommonEventServicesModuleTest::CommonEventServicesModuleTest(const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CommonEventServicesModuleTest::OnReceiveEvent(const CommonEventData &data)
{
    printf("CommonEventServicesModuleTest OnReceiveEvent\n");
    mtx_.unlock();
}

class cesModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void cesModuleTest::SetUpTestCase()
{
    handlerPtr = std::make_shared<EventHandler>(EventRunner::Create(true));
    auto task = []() { EventRunner::GetMainEventRunner()->Run(); };
    handlerPtr->PostTask(task);

    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStart();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject, saExtraProp);
}

void cesModuleTest::TearDownTestCase()
{
    EventRunner::GetMainEventRunner()->Stop();
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();
}

void cesModuleTest::SetUp()
{}

void cesModuleTest::TearDown()
{}

/*
 * @tc.number: CES_TC_ModuleTest_0100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_0100, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_0100 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_0100 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_0200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is number
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_0200, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_0200 start");
    std::string eventName = "1";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_0200 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_0300
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_0300, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_0300 start");
    std::string eventName = "";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_0300 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_0400
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify three subscribers subscribe to three different events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_0400, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_0400 start");
    std::string eventName1 = "TESTEVENT1";
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo1);
    sptr<CommonEventListener> commonEventListener1 = new CommonEventListener(subscriberPtr1);
    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo1, commonEventListener1));

    std::string eventName2 = "TESTEVENT2";
    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo2);
    sptr<CommonEventListener> commonEventListener2 = new CommonEventListener(subscriberPtr2);
    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo2, commonEventListener2));

    std::string eventName3 = "TESTEVENT3";
    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo3);
    sptr<CommonEventListener> commonEventListener3 = new CommonEventListener(subscriberPtr3);
    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo3, commonEventListener3));
    EVENT_LOGE("CES_TC_ModuleTest_0400 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_0500
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the ordered common event was subsribered successfully
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_0500, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_0500 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(100);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_0500 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_0600
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the ordered common event was subsribered successfully
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_0600, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_0600 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_PRIORITY";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1000);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_0600 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_0700
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the ordered common event was subsribered successfully
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_0700, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_0700 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_DEVUCEID";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetDeviceId("module_test");
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_0700 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_0800
 * @tc.name: CommonEventManagerService UnsubscribeCommonEvent
 * @tc.desc: test Unsubscribe to common event and set the priority to 1000, Verify that ordered common event was
 * unsubscribed successfully
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_0800, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_0800 start");
    std::string eventName = "UNSUBSCRIBEEVENT_MODULETEST_PRIORITY";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1000);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_0800 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_0900
 * @tc.name: CommonEventManagerService UnsubscribeCommonEvent
 * @tc.desc: test Unsubscribe to common event and set the SetDeviceId to "module_test", Verify Unsubscribe CommonEvent
 * success with setting DeviceId
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_0900, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_0900 start");
    std::string eventName = "UNSUBSCRIBEEVENT_MODULETEST_DEVUCEID";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetDeviceId("module_test");
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_0900 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1000
 * @tc.name: CommonEventManagerService UnsubscribeCommonEvent
 * @tc.desc: test UnsubscribeCommonEvent has priority, Verify that ordered common event was unsubscribed successfully
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1000, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_1000 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(100);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_1000 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1100
 * @tc.name: CommonEventManagerService UnsubscribeCommonEvent
 * @tc.desc: test UnsubscribeCommonEvent , Verify the common event was unsubscribed successfully
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1100, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_1100 start");
    std::string eventName = "UNSUBSCRIBEEVENT_MODELETEST";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_1100 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1200
 * @tc.name: CommonEventManagerService UnsubscribeCommonEvent
 * @tc.desc: test unsubscribe event when the input string is number
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1200, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_1200 start");
    std::string eventName = "2";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_1200 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1300
 * @tc.name: CommonEventManagerService UnsubscribeCommonEvent
 * @tc.desc: test unsubscribe event when the input string is empty
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1300, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_1300 start");
    std::string eventName = "";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_1300 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1400
 * @tc.name: CommonEventManagerService UnsubscribeCommonEvent
 * @tc.desc: test unsubscribe to three different events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1400, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_1400 start");
    std::string eventName4 = "TESTEVENT4";
    bool result1 = false;
    MatchingSkills matchingSkills4;
    matchingSkills4.AddEvent(eventName4);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills4);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo1);
    sptr<CommonEventListener> commonEventListener1 = new CommonEventListener(subscriberPtr1);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo1, commonEventListener1)) {
        result1 = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener1);
    }
    EXPECT_TRUE(result1);

    std::string eventName5 = "TESTEVENT5";
    bool result2 = false;
    MatchingSkills matchingSkills5;
    matchingSkills5.AddEvent(eventName5);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills5);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo2);
    sptr<CommonEventListener> commonEventListener2 = new CommonEventListener(subscriberPtr2);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo2, commonEventListener2)) {
        result2 = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener2);
    }
    EXPECT_TRUE(result2);

    std::string eventName6 = "TESTEVENT6";
    bool result3 = false;
    MatchingSkills matchingSkills6;
    matchingSkills6.AddEvent(eventName6);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills6);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo3);
    sptr<CommonEventListener> commonEventListener3 = new CommonEventListener(subscriberPtr3);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo3, commonEventListener3)) {
        result3 = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener3);
    }
    EXPECT_TRUE(result3);

    EVENT_LOGE("CES_TC_ModuleTest_1400 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1500
 * @tc.name: CommonEventManagerService PublishCommonEvent
 * @tc.desc: Verify publish event successfully
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1500, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_1500 start");
    std::string eventName = "PUBLISHEVENT_MODULETEST_ACTION";
    std::string eventAction = "PUBLISHEVENT_MODULETEST_ACTION";
    bool stickty = false;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
            commonEventData, publishInfo, commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_1500 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1600
 * @tc.name: CommonEventManagerService PublishCommonEvent
 * @tc.desc: Verify publish event with entity successfully
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1600, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_1600 start");
    std::string eventName = "PUBLISHEVENT_MODULETEST_ACTION_ENTITY";
    std::string eventAction = "PUBLISHEVENT_MODULETEST_ACTION_ENTITY";
    std::string entity = "ADDENTITY";
    bool stickty = false;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    testWant.AddEntity(entity);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
            commonEventData, publishInfo, commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_1600 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1700
 * @tc.name: CommonEventManagerService DumpState
 * @tc.desc: Verify DumpState
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1700, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_1700 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_DUMPSTATE";
    bool result = false;
    std::vector<std::string> stateTest;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        if (DelayedSingleton<CommonEvent>::GetInstance()->DumpState(eventName, stateTest)) {
            if (stateTest.size() >= 1) {
                result = true;
            }
        }
    }
    GTEST_LOG_(INFO) << " stateTest size = " << stateTest.size();
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_1700 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1800
 * @tc.name: CommonEventManagerService DumpState
 * @tc.desc: Verify DumpState another event that is not added
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1800, Function | MediumTest | Level2)
{
    EVENT_LOGE("CES_TC_ModuleTest_1800 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_DUMPSTATE_";
    std::string eventNametest = "TEST_NAME";
    bool result = false;
    std::vector<std::string> stateTest;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        if (DelayedSingleton<CommonEvent>::GetInstance()->DumpState(eventNametest, stateTest)) {
            if (stateTest.size() == 1) {
                result = true;
            } else {
                EVENT_LOGE(" CES_TC_ModuleTest_1800 stateTest.size() is null ");
            }
        }
    }

    EXPECT_FALSE(result);
    EVENT_LOGE("CES_TC_ModuleTest_1800 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_1900
 * @tc.name: CommonEventManagerService DumpState
 * @tc.desc: Verify DumpState with empty event
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_1900, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_1900 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_DUMPSTATE";
    std::string eventNameInput = "";
    bool result = false;
    std::vector<std::string> stateTest;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        if (DelayedSingleton<CommonEvent>::GetInstance()->DumpState(eventNameInput, stateTest)) {
            if (stateTest.size() >= 1) {
                result = true;
            }
        }
    }
    GTEST_LOG_(INFO) << " stateTest size = " << stateTest.size();
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_1900 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_2000
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify receive comon event
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2000, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_2000 start");
    std::string eventName = "PUBLISHEVENT_MODULETEST_ACTION2000";
    std::string eventAction = "PUBLISHEVENT_MODULETEST_ACTION2000";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        mtx_.lock();
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    mtx_.unlock();
    EVENT_LOGE("CES_TC_ModuleTest_2000 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_2100
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify receive system event
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2100, Function | MediumTest | Level2)
{
    EVENT_LOGE("CES_TC_ModuleTest_2100 start");
    std::string eventName = "PUBLISHEVENT_MODULETEST_ACTION_ENTITY";
    std::string eventAction = "PUBLISHEVENT_MODULETEST_ACTION_ENTITY";
    bool result = false;
    bool resulttime = true;
    std::string entity = "addEntity";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    testWant.AddEntity(entity);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
            commonEventData, publishInfo, commonEventListener);
        mtx_.lock();
    }
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            resulttime = false;
            break;
        }
    }
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(resulttime);
    EVENT_LOGE("CES_TC_ModuleTest_2100 end");
    mtx_.unlock();
}

/*
 * @tc.number: CES_TC_ModuleTest_2200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify Subscribe system event "COMMON_EVENT_ABILITY_ADDED"
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2200, Function | MediumTest | Level2)
{
    EVENT_LOGE("CES_TC_ModuleTest_2200 start");
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_ADDED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_ADDED;
    bool result = false;
    bool stickty = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    bundleObject->MockSetIsSystemApp(false);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        mtx_.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_FALSE(result);
    EVENT_LOGE("CES_TC_ModuleTest_2200 end");
    mtx_.unlock();
}

/*
 * @tc.number: CES_TC_ModuleTest_2300
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Subscribe system event "COMMON_EVENT_ABILITY_REMOVED"
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2300, Function | MediumTest | Level2)
{
    EVENT_LOGE("CES_TC_ModuleTest_2300 start");
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED;
    bool result = false;
    bool stickty = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    bundleObject->MockSetIsSystemApp(false);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        mtx_.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_FALSE(result);
    EVENT_LOGE("CES_TC_ModuleTest_2300 end");
    mtx_.unlock();
}

/*
 * @tc.number: CES_TC_ModuleTest_2400
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Subscribe system event "COMMON_EVENT_ABILITY_UPDATED"
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2400, Function | MediumTest | Level2)
{
    EVENT_LOGE("CES_TC_ModuleTest_2400 start");
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    bool result = false;
    bool stickty = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    bundleObject->MockSetIsSystemApp(false);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        mtx_.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_FALSE(result);
    EVENT_LOGE("CES_TC_ModuleTest_2400 end");
    mtx_.unlock();
}

/*
 * @tc.number: CES_TC_ModuleTest_2500
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Subscribe custom event "TESTEVENT_MATCHEVENTTEST"
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2500, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_2500 start");
    std::string eventName = "TESTEVENT_MATCHEVENTTEST";
    bool result = false;

    Want wantTest;
    wantTest.SetAction(eventName);
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = matchingSkills.Match(wantTest);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_2500 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_2600
 * @tc.name: PublishCommonEvent
 * @tc.desc: Publish system event "COMMON_EVENT_ABILITY_ADDED"
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2600, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_2600 start");
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_ADDED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_ADDED;
    bool stickty = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    bundleObject->MockSetIsSystemApp(true);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        mtx_.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }

    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    EVENT_LOGE("CES_TC_ModuleTest_2600 end");
    bundleObject->MockSetIsSystemApp(false);
    mtx_.unlock();
}

/*
 * @tc.number: CES_TC_ModuleTest_2700
 * @tc.name: PublishCommonEvent
 * @tc.desc: Publish system event "COMMON_EVENT_ABILITY_UPDATED"
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2700, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_2700 start");
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    bool stickty = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    bundleObject->MockSetIsSystemApp(true);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        mtx_.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }

    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    EVENT_LOGE("CES_TC_ModuleTest_2700 end");
    bundleObject->MockSetIsSystemApp(false);
    mtx_.unlock();
}

/*
 * @tc.number: CES_TC_ModuleTest_2800
 * @tc.name: GetStickyCommonEvent
 * @tc.desc: set sticky  and get action, publish common event set sticky to true and verify the action of stickyData
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2800, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_2800 start");
    std::string eventName = "MODULETEST_ACTION_STICKY";
    std::string eventAction = "MODULETEST_ACTION_STICKY";
    std::string eventActionStr = "MODULETEST_ACTION_STICKYSTR";
    bool stickty = true;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener) &&
        CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
        result = true;
    }
    EXPECT_TRUE(result);

    CommonEventData stickyData;
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->GetStickyCommonEvent(eventAction, stickyData);
    EXPECT_FALSE(eventActionStr == stickyData.GetWant().GetAction());
    EVENT_LOGE("CES_TC_ModuleTest_2800 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_2900
 * @tc.name: GetStickyCommonEvent
 * @tc.desc: set sticky  and get another action, publish common event set sticky to true and verify the action of
 * stickyData
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_2900, Function | MediumTest | Level2)
{
    EVENT_LOGE("CES_TC_ModuleTest_2900 start");
    std::string eventName = "MODULETEST_ACTION_STICKY_FALSE";
    std::string eventAction = "MODULETEST_ACTION_STICKY_FALSE";
    std::string actionTest = "CHECKTESTACTION";
    bool stickty = true;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener) &&
        OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
            commonEventData, publishInfo, commonEventListener)) {
        result = true;
    }
    EXPECT_TRUE(result);

    CommonEventData stickyData;
    bool stickyResult =
        OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->GetStickyCommonEvent(eventAction, stickyData);
    EXPECT_FALSE(stickyResult);
    EVENT_LOGE("CES_TC_ModuleTest_2900 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3000
 * @tc.name:  Set Permission
 * @tc.desc: Set permission for common event subscribers and  verify  successfully subscribe to common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3000, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3000 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_PERMISSION";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_3000 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3100
 * @tc.name:  Set Permission and Priority
 * @tc.desc: Set permission and priority for common event subscribers and  verify  successfully subscribe to
 * common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3100, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3100 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_3100 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3200
 * @tc.name:  Set Permission  Priority and DeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and  verify  successfully
 * subscribe to common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3200, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3200 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_PERMISSION_PRIORITY_DEVICEDID";
    std::string permissin = "PERMISSION";
    std::string deviceId = "DEVICEDID";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    subscribeInfo.SetDeviceId(deviceId);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_3200 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3300
 * @tc.name:  Set Permission
 * @tc.desc: Set permission for common event subscribers and verify successfully Unsubscribe to common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3300, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3300 start");
    std::string eventName = "UNSUBSCRIBEEVENT_MODULETEST_PERMISSION";
    std::string permissin = "PERMISSION";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_3300 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3400
 * @tc.name:  Set Permission and Priority
 * @tc.desc: Set permission and priority for common event subscribers and verify successfully Unsubscribe to
 * common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3400, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3400 start");
    std::string eventName = "UNSUBSCRIBEEVENT_MODULETEST_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_3400 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3500
 * @tc.name:  Set Permission  Priority and DeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and  verify  successfully
 * Unsubscribe to common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3500, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3500 start");
    std::string eventName = "UNSUBSCRIBEEVENT_MODULETEST_PERMISSION_PRIORITY_DEVICEDID";
    std::string permissin = "PERMISSION";
    std::string deviceId = "DEVICEDID";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    subscribeInfo.SetDeviceId(deviceId);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_3500 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3600
 * @tc.name:  Set Subscriber Permissions
 * @tc.desc: Set permission for common event subscribers and  verify  successfully publish common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3600, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3600 start");
    std::string eventName = "PUBLISHEVENT_MODULETEST_PERMISSION";
    std::string eventAction = "PUBLISHEVENT_MODULETEST_PERMISSION";
    std::string permissin = "PERMISSION";
    std::vector<std::string> permissions;
    permissions.emplace_back(permissin);
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    publishInfo.SetSubscriberPermissions(permissions);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
            commonEventData, publishInfo, commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_3600 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3700
 * @tc.name:  Set Subscriber Permissions
 * @tc.desc: Set permission for common event subscribers and  verify  successfully publish common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3700, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3700 start");
    std::string eventName = "PUBLISHEVENT_MODULETEST_PERMISSION";
    std::string eventAction = "PUBLISHEVENT_MODULETEST_PERMISSION";
    std::string permissin1 = "PERMISSION1";
    std::string permissin2 = "PERMISSION2";
    std::vector<std::string> permissions;
    permissions.emplace_back(permissin1);
    permissions.emplace_back(permissin2);
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    publishInfo.SetSubscriberPermissions(permissions);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
            commonEventData, publishInfo, commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_3700 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3800
 * @tc.name:  Set Subscriber Permissions
 * @tc.desc: Set permission for common event subscribers and  verify  successfully publish common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3800, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3800 start");
    std::string eventName = "PUBLISHEVENT_MODULETEST_PERMISSION";
    std::string eventAction = "PUBLISHEVENT_MODULETEST_PERMISSION";
    std::string permissin1 = "PERMISSION1";
    std::string permissin2 = "PERMISSION2";
    std::string permissin3 = "PERMISSION3";
    std::vector<std::string> permissions;
    permissions.emplace_back(permissin1);
    permissions.emplace_back(permissin2);
    permissions.emplace_back(permissin3);
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    publishInfo.SetSubscriberPermissions(permissions);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
            commonEventData, publishInfo, commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_3800 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_3900
 * @tc.name:  Set ThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully subscribe to common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_3900, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_3900 start");
    std::string eventName = "SUBSCRIBEEVENT_MODULETEST_SETTHRERADMODE";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    // set thread mode
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);

    EXPECT_TRUE(OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscribeInfo, commonEventListener));
    EVENT_LOGE("CES_TC_ModuleTest_3900 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_4000
 * @tc.name:  Set ThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully Unsubscribe to common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_4000, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_4000 start");
    std::string eventName = "UNSUBSCRIBEEVENT_MODULETEST_SETTHRERADMODE";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    // set thread mode
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(
            commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_4000 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_4100
 * @tc.name:  Set ThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully publish to common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_4100, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_4100 start");
    std::string eventName = "PUBLISHEVENT_MODULETEST_SETTHRERADMODE";
    std::string eventAction = "PUBLISHEVENT_MODULETEST_SETTHRERADMODE";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
            commonEventData, publishInfo, commonEventListener);
    }
    EXPECT_TRUE(result);
    EVENT_LOGE("CES_TC_ModuleTest_4100 end");
}

/*
 * @tc.number: CES_TC_ModuleTest_4200
 * @tc.name:  Set ThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully receive  to common events
 */
HWTEST_F(cesModuleTest, CES_TC_ModuleTest_4200, Function | MediumTest | Level1)
{
    EVENT_LOGE("CES_TC_ModuleTest_4200 start");
    std::string eventName = "PUBLISHEVENT_MODULETEST_SETTHRERADMODE_R";
    std::string eventAction = "PUBLISHEVENT_MODULETEST_SETTHRERADMODE_R";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want testWant;
    testWant.SetAction(eventAction);
    CommonEventData commonEventData(testWant);
    CommonEventPublishInfo publishInfo;
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesModuleTest>(subscribeInfo);
    sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberPtr);
    if (OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
            subscribeInfo, commonEventListener)) {
        mtx_.lock();
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    mtx_.unlock();
    EVENT_LOGE("CES_TC_ModuleTest_4200 end");
}
}  // namespace EventFwk
}  // namespace OHOS
