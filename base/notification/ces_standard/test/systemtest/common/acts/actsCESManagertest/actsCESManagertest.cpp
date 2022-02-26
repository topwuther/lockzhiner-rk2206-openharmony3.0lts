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

#include <iostream>
#include <string>
#include <thread>

#include "common_event_manager.h"
#define private public
#define protected public
#include "common_event_manager_service.h"
#undef private
#undef protected
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "datetime_ex.h"
#include "event_log_wrapper.h"

#include "testConfigParser.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace EventFwk {
namespace {
std::mutex g_mtx;
const time_t g_TIME_OUT_SECONDS_LIMIT = 3;
const time_t g_TIME_OUT_SECONDS = 3;

const std::string CompareStr = "cesComparesStrForCase";
const std::string CompareStrFalse = "cesComparesStrForCaseFalse";

const int32_t g_CODE_COMPARE1 = 1;
const int32_t g_CODE_COMPARE2 = 200;
int SIGNUMFIRST = 0;
int SIGNUMSECOND = 0;
int SIGNUMTHIRD = 0;
}  // namespace

class CommonEventServicesSystemTest : public CommonEventSubscriber {
public:
    explicit CommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventServicesSystemTest(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CommonEventServicesSystemTest::CommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CommonEventServicesSystemTest::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << " ActsCESManagertest:CommonEventServicesSystemTest:OnReceiveEvent \n";
    std::string action = data.GetWant().GetAction();
    if (action == CompareStr) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE1);
        ++SIGNUMFIRST;
    }
    g_mtx.unlock();
}

class CommonEventServicesSystemTestSubscriber : public CommonEventSubscriber {
public:
    explicit CommonEventServicesSystemTestSubscriber(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventServicesSystemTestSubscriber(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CommonEventServicesSystemTestSubscriber::CommonEventServicesSystemTestSubscriber(
    const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CommonEventServicesSystemTestSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << " ActsCESManagertest:CommonEventServicesSystemTestSubscriber:OnReceiveEvent \n";
    std::string action = data.GetWant().GetAction();
    if (action == CompareStr) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE1);
        ++SIGNUMSECOND;
    } else if (action == CompareStrFalse) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE2);
        ++SIGNUMTHIRD;
    }
}

class ActsCESManagertest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static StressTestLevel stLevel_;
};
StressTestLevel ActsCESManagertest::stLevel_{};

void ActsCESManagertest::SetUpTestCase()
{
    TestConfigParser tcp;
    tcp.ParseFromFile4StressTest(STRESS_TEST_CONFIG_FILE_PATH, stLevel_);
    std::cout << "stress test level : "
              << "AMS : " << stLevel_.AMSLevel << " "
              << "BMS : " << stLevel_.BMSLevel << " "
              << "CES : " << stLevel_.CESLevel << std::endl;
}

void ActsCESManagertest::TearDownTestCase()
{}

void ActsCESManagertest::SetUp()
{}

void ActsCESManagertest::TearDown()
{}

/*
 * @tc.number: CES_SubscriptionEvent_0100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is number
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "1";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0300
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0300, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0400
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is points
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0400, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = ".............";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0500
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string with \0
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0500, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "HELLO\0\0\0WORLD";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0600
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the ordered common event was subsribered succseefully
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_SETPRIORITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(100);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0600 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0600 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0700
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: erify that the ordered common event was unsubsribered successfully
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBE_SETPRIORITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPriority(100);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0700 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0700 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0800
 * @tc.name: SubscribeCommonEvent
 * @tc.desc:  Verify the same input string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0800, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "TESTEVENT1";
    std::string eventName3 = "TESTEVENT1";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);

        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
        bool result3 = CommonEventManager::SubscribeCommonEvent(subscriberPtr3);

        if (!result1 || !result2 || !result3) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0800 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0800 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0900
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the same input empty string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_0900, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "";
    std::string eventName2 = "";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);

        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
        bool result3 = CommonEventManager::SubscribeCommonEvent(subscriberPtr3);

        if (!result1 || !result2 || !result3) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0900 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_0900 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1000
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal input string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1000, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "TESTEVENT2";
    std::string eventName3 = "TESTEVENT3";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);

        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
        bool result3 = CommonEventManager::SubscribeCommonEvent(subscriberPtr3);

        if (!result1 || !result2 || !result3) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1000 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1000 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc:  Verify the normal input string two times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TEST1";
    std::string eventName2 = "TEST2";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);
        if (!result1 || !result2) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal, number and empty input string
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1200, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "1";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);

        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);

        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
        bool result3 = CommonEventManager::SubscribeCommonEvent(subscriberPtr3);

        if (!result1 || !result2 || !result3) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1300
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1400
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is number
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "2";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CCES_SubscriptionEvent_1500
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1500, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1600
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is points
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1600, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "..................";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1600 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1600 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1700
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string with \0
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1700, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "HELLO\0\0\0WORLD";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1700 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1700 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1800
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the normal input string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1800, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT4";
    std::string eventName2 = "TESTEVENT5";
    std::string eventName3 = "TESTEVENT6";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool result1 = false;
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
            result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        }

        bool result2 = false;
        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
            result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
        }

        bool result3 = false;
        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr3)) {
            result3 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
        }

        if (!result1 || !result2 || !result3) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1800 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1800 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1900
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the normal input string two times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_1900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TEST3";
    std::string eventName2 = "TEST4";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool result1 = false;
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
            result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        }

        bool result2 = false;
        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
            result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
        }

        if (!result1 || !result2) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1900 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_1900 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_2000
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal, number and empty input string
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_2000, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT7";
    std::string eventName2 = "3";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool result1 = false;
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
            result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        }

        bool result2 = false;
        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
            result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
        }

        bool result3 = false;
        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr3)) {
            result3 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
        }

        if (!result1 || !result2 || !result3) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_2000 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_2000 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_2100
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the same input string three times
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_2100, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "TESTEVENT4";
    std::string eventName2 = "TESTEVENT4";
    std::string eventName3 = "TESTEVENT4";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool result1 = false;
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
            result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        }

        bool result2 = false;
        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
            result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
        }

        bool result3 = false;
        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr3)) {
            result3 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
        }

        if (!result1 || !result2 || !result3) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_2100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_2100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_2200
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the same input empty string three time
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEvent_2200, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName1 = "";
    std::string eventName2 = "";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    MatchingSkills matchingSkills2;
    MatchingSkills matchingSkills3;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool result1 = false;
        matchingSkills1.AddEvent(eventName1);
        CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
            result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        }

        bool result2 = false;
        matchingSkills2.AddEvent(eventName2);
        CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
        auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
            result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
        }

        bool result3 = false;
        matchingSkills3.AddEvent(eventName3);
        CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
        auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr3)) {
            result3 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
        }

        if (!result1 || !result2 || !result3) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEvent_2200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills1.RemoveEvent(eventName1);
        matchingSkills2.RemoveEvent(eventName2);
        matchingSkills3.RemoveEvent(eventName3);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEvent_2200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0100
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when only set action
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0200
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when add entity
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ENTITY";
    std::string eventAction = "TESTEVENT_PUBLISH_ENTITY";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0300
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when set scheme
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_SCHEME";
    std::string eventAction = "TESTEVENT_PUBLISH_SCHEME";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0400
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when set scheme and add entity
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_SCHEME_ENTITY";
    std::string eventAction = "TESTEVENT_PUBLISH_SCHEME_ENTITY";
    std::string scheme = "SETSCHEME";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0500
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_INFO_FALSE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0600
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_TRUE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        std::string entity = "ADDENTITY";
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_0600 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_0600 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0700
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_0700 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_0700 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0800
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0800, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_INFO_TRUE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_0800 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_0800 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_0900
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_0900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_TRUE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        std::string scheme = "SETSCHEME";
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_0900 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_0900 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_1000
 * @tc.name: SPublishCommonEvent
 * @tc.desc: Verify set scheme with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1000, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_FALSE";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_1000 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_1000 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_1100
 * @tc.name: PublishCommonEvent
 * @tc.desc: VVerify set scheme and add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_1100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_1100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_1200
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme and add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEvent_1200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEvent_1200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0100
 * @tc.name: SetSticky
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICHY_ACTION_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICHY_ACTION_INFO_FALSE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0200
 * @tc.name: SetSticky
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_TRUE";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0300
 * @tc.name: SetSticky
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0400
 * @tc.name: SetSticky
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_INFO_TRUE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CCES_SendEventSetViscosity_0500
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_TRUE";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0600
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_FALSE";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0600 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0600 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0700
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme and add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0700 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0700 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0800
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme and add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0800, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = false;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        wantTest.AddEntity(entity);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0800 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0800 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0900
 * @tc.name: GetStickyCommonEvent
 * @tc.desc: publish common event set sticky to true and verify the action of stickyData
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_0900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_GETSTICKY_";
    std::string eventAction = "TESTEVENT_GETSTICKY_";
    std::string eventActionStr = "TESTEVENT_GETSTICKY_Str";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
            CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = true;
        }
        EXPECT_TRUE(result);
        result = false;
        CommonEventData stickyData;
        CommonEventManager::GetStickyCommonEvent(eventAction, stickyData);
        if (eventActionStr == stickyData.GetWant().GetAction()) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0900 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_0900 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEventSetViscosity_1000
 * @tc.name: GetStickyCommonEvent
 * @tc.desc: publish common event set sticky to true and verify the action of stickyData
 */
HWTEST_F(ActsCESManagertest, CES_SendEventSetViscosity_1000, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "TESTEVENT_GETSTICKY_FALSE";
    std::string eventAction = "TESTEVENT_GETSTICKY_FALSE";
    std::string actionTest = "CHECKTESTACTION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        bool stickty = true;
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(stickty);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
            CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = true;
        }
        EXPECT_TRUE(result);
        result = false;
        CommonEventData stickyData;
        if (CommonEventManager::GetStickyCommonEvent(actionTest, stickyData)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_1000 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SendEventSetViscosity_1000 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_ReceiveEvent_0100
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify the function when only set action
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTION";
    std::string eventAction = "TESTEVENT_RECEIVE_ACTION";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, g_TIME_OUT_SECONDS_LIMIT);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0200
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify the function when add entity
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ENTITY";
    std::string eventAction = "TESTEVENT_RECEIVE_ENTITY";
    std::string entity = "ADDENTITY";
    bool result = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0300
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTION_INFO_FALSE";
    std::string eventAction = "TESTEVENT_RECEIVE_ACTION_INFO_FALSE";
    bool stickty = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, g_TIME_OUT_SECONDS_LIMIT);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0400
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTION_INFO_TRUE";
    std::string eventAction = "TESTEVENT_RECEIVE_ACTION_INFO_TRUE";
    bool stickty = true;
    bool result = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_FALSE(result);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0500
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0500, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ENTITY_INFO_FALSE";
    std::string eventAction = "TESTEVENT_RECEIVE_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";
    bool result = true;
    bool stickty = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0600
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(ActsCESManagertest, CES_ReceiveEvent_0600, Function | MediumTest | Level1)
{
    std::string eventName = "testEventReceiveEntityInfoFalse";
    std::string eventAction = "testEventReceiveEntityInfoFalse";
    std::string entity = "addEntity";
    bool result = true;
    bool stickty = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0100
 * @tc.name: AddEvent
 * @tc.desc: Verify add an event Theme
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEventTheme_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTADDTHEME";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEventTheme_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEventTheme_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0200
 * @tc.name: AddEvent
 * @tc.desc: Verify add multiple event themes
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEventTheme_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName1 = "TESTADDTHEME1";
    std::string eventName2 = "TESTADDTHEME2";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName1);
        matchingSkills.AddEvent(eventName2);

        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEventTheme_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName1);
        matchingSkills.RemoveEvent(eventName2);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEventTheme_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0300
 * @tc.name: MatchEvent
 * @tc.desc: Verify match a event theme
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEventTheme_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_MATCHEVENTTEST";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        Want wantTest;
        wantTest.SetAction(eventName);
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!matchingSkills.Match(wantTest)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEventTheme_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEventTheme_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0400
 * @tc.name: MatchEvent
 * @tc.desc: Verify match other event theme
 */
HWTEST_F(ActsCESManagertest, CES_SubscriptionEventTheme_0400, Function | MediumTest | Level1)
{
    bool result = true;
    std::string eventName = "TESTMATCHEVENTTOPICAL";
    std::string eventNameCompare = "TESTMATCHEVENTTOPICAL_COMPARE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        Want wantTest;
        wantTest.SetAction(eventNameCompare);
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (matchingSkills.Match(wantTest)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SubscriptionEventTheme_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SubscriptionEventTheme_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SendEvent_1300
 * @tc.name: PublishCommonEvent
 * @tc.desc: The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1300, Function | MediumTest | Level1)
{
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_ADDED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_ADDED;
    bool result = false;
    bool sysResult = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        g_mtx.lock();
    }
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_TRUE(sysResult);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1400
 * @tc.name: PublishCommonEvent
 * @tc.desc:The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1400, Function | MediumTest | Level1)
{
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_REMOVED;
    bool result = false;
    bool sysResult = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        g_mtx.lock();
    }
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_TRUE(sysResult);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1500
 * @tc.name: PublishCommonEvent
 * @tc.desc:The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1500, Function | MediumTest | Level1)
{
    std::string eventName = CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ABILITY_UPDATED;
    bool result = false;
    bool sysResult = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        g_mtx.lock();
    }
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_TRUE(sysResult);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1600
 * @tc.name: PublishCommonEvent
 * @tc.desc: publisher cannot receive published system events
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1600, Function | MediumTest | Level1)
{
    std::string eventName = CommonEventSupport::COMMON_EVENT_ACCOUNT_DELETED;
    std::string eventAction = CommonEventSupport::COMMON_EVENT_ACCOUNT_DELETED;
    bool result = true;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = false;
            break;
        }
    }
    // System events published by ordinary publishers, the publication fails, and the receiver cannot receive it
    EXPECT_FALSE(result);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1700
 * @tc.name: PublishCommonEvent
 * @tc.desc: Both subscribers subscribe to the event, after the event is published, both the subscribers can receive
 * the event
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1700, Function | MediumTest | Level1)
{
    std::string eventName = CompareStr;
    std::string eventAction = CompareStr;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        commonEventData.SetCode(1);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTestSubscriber>(subscribeInfo);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
            CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
            CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    EXPECT_TRUE(SIGNUMFIRST == stLevel_.CESLevel && SIGNUMSECOND == stLevel_.CESLevel && SIGNUMTHIRD == 0);
    GTEST_LOG_(INFO) << "SIGNUMFIRST: " << SIGNUMFIRST << "SIGNUMSECOND: " << SIGNUMSECOND
                     << "SIGNUMTHIRD: " << SIGNUMTHIRD << "stLevel_.CESLevel: " << stLevel_.CESLevel;
    SIGNUMFIRST = 0;
    SIGNUMSECOND = 0;
    SIGNUMTHIRD = 0;
}

/*
 * @tc.number: CES_SendEvent_1800
 * @tc.name: PublishCommonEvent
 * @tc.desc: One subscriber subscribe to the event and another subscriber does not subscribe to the event,
 *           after the event is published, subscribed subscriber can receive the event and unsubscribed
 *           subscribe can not receive the event
 */
HWTEST_F(ActsCESManagertest, CES_SendEvent_1800, Function | MediumTest | Level1)
{
    std::string eventName = CompareStrFalse;
    std::string eventAction = CompareStrFalse;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        commonEventData.SetCode(200);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTestSubscriber>(subscribeInfo);
        if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    EXPECT_TRUE(SIGNUMFIRST == 0 && SIGNUMSECOND == 0 && SIGNUMTHIRD == stLevel_.CESLevel);
    GTEST_LOG_(INFO) << "SIGNUMFIRST: " << SIGNUMFIRST << "SIGNUMSECOND: " << SIGNUMSECOND
                     << "SIGNUMTHIRD: " << SIGNUMTHIRD << "stLevel_.CESLevel: " << stLevel_.CESLevel;
    SIGNUMFIRST = 0;
    SIGNUMSECOND = 0;
    SIGNUMTHIRD = 0;
}

/*
 * @tc.number: CES_SetEventAuthority_0100
 * @tc.name: SetPermission
 * @tc.desc: Set permission for common event subscribers and verify successfully subscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0200
 * @tc.name: SetPermission and SetPriority
 * @tc.desc: Set permission and priority for common event subscribers and verify successfully subscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        subscribeInfo.SetPriority(1);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0300
 * @tc.name: SetPermission SetPriority and SetDeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and verify successfully
 * subscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION_PRIORITY_D";
    std::string permissin = "PERMISSION";
    std::string deviceId = "deviceId";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        subscribeInfo.SetPriority(1);
        subscribeInfo.SetDeviceId(deviceId);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0400
 * @tc.name: SetPermission
 * @tc.desc: Set permission for common event subscribers and verify successfully Unsubscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0500
 * @tc.name: SetPermission and  SetPriority
 * @tc.desc: Set permission and priority for common event subscribers and verify successfully Unsubscribe to
 * common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        subscribeInfo.SetPriority(1);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0600
 * @tc.name: SetPermission SetPriority and SetDeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and  verify  successfully
 * Unsubscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION_PRIORITY_D";
    std::string permissin = "PERMISSION";
    std::string deviceId = "deviceId";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetPermission(permissin);
        subscribeInfo.SetPriority(1);
        subscribeInfo.SetDeviceId(deviceId);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_0600 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_0600 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0700
 * @tc.name: OnReceiveEvent
 * @tc.desc: Set permission for common event subscribers and  verify  successfully publish common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_PERMISSION";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_PERMISSION";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    std::vector<std::string> permissins;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        permissins.emplace_back(permissin);
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_0700 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
        permissins.clear();
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_0700 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0800
 * @tc.name: SetSubscriberPermissions
 * @tc.desc: Set permission for common event subscribers and  verify  successfully receive common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0800, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_PERMISSION_R";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_PERMISSION_R";
    std::string permissin = "PERMISSION";
    bool result = false;
    std::vector<std::string> permissins;
    permissins.emplace_back(permissin);
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSubscriberPermissions(permissins);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData, publishInfo))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = true;
            break;
        }
    }
    EXPECT_TRUE(result);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SetEventAuthority_0900
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and verify successfully subscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_0900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_SETTHREADMODE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        if (!CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_0700 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_0700 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_1000
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and verify successfully Unsubscribe to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_1000, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_SETTHREADMODE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::UnSubscribeCommonEvent(subscriberPtr)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_1000 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_1000 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_1100
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and verify successfully publish to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_1100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SETTHREADMODE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SETTHREADMODE";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        Want wantTest;
        wantTest.SetAction(eventAction);
        CommonEventData commonEventData(wantTest);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!CommonEventManager::PublishCommonEvent(commonEventData)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_SetEventAuthority_1000 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_1000 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_1200
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and verify successfully receive to common events
 */
HWTEST_F(ActsCESManagertest, CES_SetEventAuthority_1200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SETHANDLER_HANDLER";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SETHANDLER_HANDLER";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData))) {
        g_mtx.lock();
    }
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!g_mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= g_TIME_OUT_SECONDS_LIMIT) {
            result = true;
            break;
        }
    }
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_TRUE(result);
    g_mtx.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0100
 * @tc.name: GetEvent
 * @tc.desc: check to get the added event
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_GETMATCHINGSKILLS";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        MatchingSkills testMatching = subscribeInfo.GetMatchingSkills();
        if (!(testMatching.GetEvent(0) == eventName)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0200
 * @tc.name: GetEntity
 * @tc.desc: check to get the added entity
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_ADDENTITY_GETENTITY";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!(matchingSkills.GetEntity(0) == entity)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0300
 * @tc.name: HasEntity
 * @tc.desc: verify that entity is in MatchingSkills
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_ADDENTITY_HASENTITY";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!matchingSkills.HasEntity(entity)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0400
 * @tc.name: RemoveEntity
 * @tc.desc: verify that the entity was successfully removed
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_ADDENTITY_REMOVEENTITY";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        matchingSkills.RemoveEntity(entity);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (matchingSkills.HasEntity(entity)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0500
 * @tc.name: CountEntities
 * @tc.desc: verify that count correct number of entities
 */
HWTEST_F(ActsCESManagertest, CES_VerifyMatchingSkills_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_ADDENTITY_ENTITYCOUNT";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberPtr));
        if (!(matchingSkills.CountEntities() >= 1)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_VerifyMatchingSkills_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}
}  // namespace EventFwk
}  // namespace OHOS