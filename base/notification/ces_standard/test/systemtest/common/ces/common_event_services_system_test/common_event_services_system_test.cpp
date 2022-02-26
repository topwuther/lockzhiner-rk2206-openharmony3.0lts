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

#include <chrono>
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

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace EventFwk {
namespace {
std::mutex mtx_;
const time_t TIME_OUT_SECONDS_LIMIT = 5;
const time_t TIME_OUT_SECONDS_ = 3;
const time_t TIME_OUT_SECONDS_NINE = 9;
const time_t TIME_OUT_SECONDS_ELEVEN = 11;
const time_t TIME_OUT_SECONDS_TWENTY = 20;
const unsigned int SUBSCRIBER_MAX_SIZE = 200;
const unsigned int SUBSCRIBER_MAX_SIZE_PLUS = 201;

const std::string CompareStr = "cesComparesStrForCase";
const std::string CompareStrFalse = "cesComparesStrForCaseFalse";

const int32_t g_CODE_COMPARE1 = 1;
const int32_t g_CODE_COMPARE2 = 2;
const int32_t g_CODE_COMPARE3 = 200;
const int32_t PRIORITY_HIGH = 80;
const int32_t PRIORITY_LOW = 20;
}  // namespace

class CommonEventServicesSystemTest : public CommonEventSubscriber {
public:
    CommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventServicesSystemTest(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CommonEventServicesSystemTest::CommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CommonEventServicesSystemTest::OnReceiveEvent(const CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    if (action == CompareStrFalse) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE3);
    }
    mtx_.unlock();
}

class CommonEventServicesSystemTestSubscriber : public CommonEventSubscriber {
public:
    CommonEventServicesSystemTestSubscriber(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventServicesSystemTestSubscriber(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CommonEventServicesSystemTestSubscriber::CommonEventServicesSystemTestSubscriber(
    const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CommonEventServicesSystemTestSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    GTEST_LOG_(INFO) << " cesSystemTest:CommonEventServicesSystemTestSubscriber:OnReceiveEvent \n";
    std::string action = data.GetWant().GetAction();
    if (action == CompareStr) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE1);
    } else if (action == CompareStrFalse) {
        EXPECT_TRUE(data.GetCode() == g_CODE_COMPARE2);
    }
}

class CESPublishOrderTimeOutOne : public CommonEventSubscriber {
public:
    CESPublishOrderTimeOutOne(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CESPublishOrderTimeOutOne(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CESPublishOrderTimeOutOne::CESPublishOrderTimeOutOne(const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CESPublishOrderTimeOutOne::OnReceiveEvent(const CommonEventData &data)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));
}

class CESPublishOrderTimeOutTwo : public CommonEventSubscriber {
public:
    CESPublishOrderTimeOutTwo(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CESPublishOrderTimeOutTwo(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CESPublishOrderTimeOutTwo::CESPublishOrderTimeOutTwo(const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CESPublishOrderTimeOutTwo::OnReceiveEvent(const CommonEventData &data)
{
    EXPECT_TRUE(data.GetCode() == GetCode());
    mtx_.unlock();
}

class cesSystemTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    bool PublishCommonEventTest(const std::string &eventName);
};

void cesSystemTest::SetUpTestCase()
{}

void cesSystemTest::TearDownTestCase()
{}

void cesSystemTest::SetUp()
{}

void cesSystemTest::TearDown()
{}

/*
 * @tc.number: CES_SubscriptionEvent_0100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEvent_0200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is number
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "1";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEvent_0300
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEvent_0400
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is quotes
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = ".............";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEvent_0500
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "HELLO\0\0\0WORLD";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEvent_0600
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the ordered common event was subsribered successfully
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_SETPRIORITY";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(100);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEvent_0700
 * @tc.name: UnsubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_UNSUBSCRIBE_SETPRIORITY";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(100);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_0800
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the same input string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0800, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "TESTEVENT1";
    std::string eventName3 = "TESTEVENT1";
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);
    EXPECT_TRUE(result1);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);
    EXPECT_TRUE(result2);

    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    bool result3 = CommonEventManager::SubscribeCommonEvent(subscriberPtr3);
    EXPECT_TRUE(result3);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
}

/*
 * @tc.number: CES_SubscriptionEvent_0900
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the same input empty string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_0900, Function | MediumTest | Level1)
{
    std::string eventName1 = "";
    std::string eventName2 = "";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);
    EXPECT_TRUE(result1);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);
    EXPECT_TRUE(result2);

    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    bool result3 = CommonEventManager::SubscribeCommonEvent(subscriberPtr3);
    EXPECT_TRUE(result3);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
}

/*
 * @tc.number: CES_SubscriptionEvent_1000
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal input string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1000, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "TESTEVENT2";
    std::string eventName3 = "TESTEVENT3";
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);
    EXPECT_TRUE(result1);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);
    EXPECT_TRUE(result2);

    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    bool result3 = CommonEventManager::SubscribeCommonEvent(subscriberPtr3);
    EXPECT_TRUE(result3);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
}

/*
 * @tc.number: CES_SubscriptionEvent_1100
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify the normal input string two times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1100, Function | MediumTest | Level1)
{
    std::string eventName1 = "TEST1";
    std::string eventName2 = "TEST2";
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);
    EXPECT_TRUE(result1);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);
    EXPECT_TRUE(result2);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
}

/*
 * @tc.number: CES_SubscriptionEvent_1200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: erify the normal, number and empty input string
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1200, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTEVENT1";
    std::string eventName2 = "1";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    bool result1 = CommonEventManager::SubscribeCommonEvent(subscriberPtr1);
    EXPECT_TRUE(result1);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    bool result2 = CommonEventManager::SubscribeCommonEvent(subscriberPtr2);
    EXPECT_TRUE(result2);

    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    bool result3 = CommonEventManager::SubscribeCommonEvent(subscriberPtr3);
    EXPECT_TRUE(result3);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
}

/*
 * @tc.number: CES_SubscriptionEvent_1300
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is normal
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBE";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1400
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is number
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1400, Function | MediumTest | Level1)
{
    std::string eventName = "2";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1500
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is empty
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1500, Function | MediumTest | Level1)
{
    std::string eventName = "";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1600
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string is quotes
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1600, Function | MediumTest | Level1)
{
    std::string eventName = "..................";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1700
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the function when the input string with \0
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1700, Function | MediumTest | Level1)
{
    std::string eventName = "HELLO\0\0\0WORLD";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SubscriptionEvent_1800
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc:  Verify the normal input string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1800, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTEVENT4";
    std::string eventName2 = "TESTEVENT5";
    std::string eventName3 = "TESTEVENT6";
    bool result1 = false;
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
        result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    }
    EXPECT_TRUE(result1);

    bool result2 = false;
    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
        result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    }
    EXPECT_TRUE(result2);

    bool result3 = false;
    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr3)) {
        result3 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
    }
    EXPECT_TRUE(result3);
}

/*
 * @tc.number: CES_SubscriptionEvent_1900
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the normal input string two times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_1900, Function | MediumTest | Level1)
{
    std::string eventName1 = "TEST3";
    std::string eventName2 = "TEST4";

    bool result1 = false;
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
        result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    }
    EXPECT_TRUE(result1);

    bool result2 = false;
    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
        result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    }
    EXPECT_TRUE(result2);
}

/*
 * @tc.number: CES_SubscriptionEvent_2000
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the normal, number and empty input string
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_2000, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTEVENT7";
    std::string eventName2 = "3";
    std::string eventName3 = "";
    bool result1 = false;
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
        result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    }
    EXPECT_TRUE(result1);

    bool result2 = false;
    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
        result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    }
    EXPECT_TRUE(result2);

    bool result3 = false;
    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr3)) {
        result3 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
    }
    EXPECT_TRUE(result3);
}

/*
 * @tc.number: CES_SubscriptionEvent_2100
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc: Verify the same input string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_2100, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTEVENT4";
    std::string eventName2 = "TESTEVENT4";
    std::string eventName3 = "TESTEVENT4";
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    bool result1 = false;
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
        result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    }
    EXPECT_TRUE(result1);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    bool result2 = false;
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
        result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    }
    EXPECT_TRUE(result2);

    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    bool result3 = false;
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr3)) {
        result3 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
    }
    EXPECT_TRUE(result3);
}

/*
 * @tc.number: CES_SubscriptionEvent_2200
 * @tc.name: UnSubscribeCommonEvent
 * @tc.desc:  Verify the same input empty string three times
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEvent_2200, Function | MediumTest | Level1)
{
    std::string eventName1 = "";
    std::string eventName2 = "";
    std::string eventName3 = "";
    MatchingSkills matchingSkills1;
    matchingSkills1.AddEvent(eventName1);
    CommonEventSubscribeInfo subscribeInfo1(matchingSkills1);
    bool result1 = false;
    auto subscriberPtr1 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo1);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr1)) {
        result1 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    }
    EXPECT_TRUE(result1);

    MatchingSkills matchingSkills2;
    matchingSkills2.AddEvent(eventName2);
    CommonEventSubscribeInfo subscribeInfo2(matchingSkills2);
    bool result2 = false;
    auto subscriberPtr2 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo2);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr2)) {
        result2 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr2);
    }
    EXPECT_TRUE(result2);

    MatchingSkills matchingSkills3;
    matchingSkills3.AddEvent(eventName3);
    CommonEventSubscribeInfo subscribeInfo3(matchingSkills3);
    bool result3 = false;
    auto subscriberPtr3 = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo3);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr3)) {
        result3 = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr3);
    }
    EXPECT_TRUE(result3);
}

/*
 * @tc.number: CES_SendEvent_0100
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when only set action
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_0200
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when add entity
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ENTITY";
    std::string eventAction = "TESTEVENT_PUBLISH_ENTITY";
    std::string entity = "ADDENTITY";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_0300
 * @tc.name: PublishCommonEvent
 * @tc.desc: erify the function when set scheme
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_SCHEME";
    std::string eventAction = "TESTEVENT_PUBLISH_SCHEME";
    std::string scheme = "SETSCHEME";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_0400
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify the function when set scheme and add entity
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_SCHEME_ENTITY";
    std::string eventAction = "TESTEVENT_PUBLISH_SCHEME_ENTITY";
    std::string scheme = "SETSCHEME";
    std::string entity = "ADDENTITY";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    wantTest.AddEntity(entity);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_0500
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0500, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_INFO_FALSE";
    bool stickty = false;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_0600
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0600, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_TRUE";
    std::string entity = "ADDENTITY";
    bool stickty = true;
    bool result = false;
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
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_0700
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0700, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";
    bool stickty = false;
    bool result = false;
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
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_0800
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0800, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_INFO_TRUE";
    bool stickty = true;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_0900
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEvent_0900, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_TRUE";
    std::string scheme = "SETSCHEME";
    bool stickty = true;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1000
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1000, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SCHEME_INFO_FALSE";
    std::string scheme = "SETSCHEME";
    bool stickty = false;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1100
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme and add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    bool stickty = true;
    bool result = false;
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
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1200
 * @tc.name: PublishCommonEvent
 * @tc.desc: Verify set scheme and add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    bool stickty = false;
    bool result = false;
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
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0100
 * @tc.name: SetSticky
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICHY_ACTION_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICHY_ACTION_INFO_FALSE";
    bool stickty = false;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0200
 * @tc.name: SetSticky
 * @tc.desc: Verify add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_TRUE";
    std::string entity = "ADDENTITY";
    bool stickty = true;
    bool result = false;
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
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0300
 * @tc.name: SetSticky
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_INFO_FALSE";
    std::string entity = "ADDENTITY";
    bool stickty = false;
    bool result = false;
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
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0400
 * @tc.name: SetSticky
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_INFO_TRUE";
    bool stickty = true;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0500
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0500, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_TRUE";
    std::string scheme = "SETSCHEME";
    bool stickty = true;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0600
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0600, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_SCHEME_INFO_FALSE";
    std::string scheme = "SETSCHEME";
    bool stickty = false;
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(stickty);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0700
 * @tc.name: SetSticky
 * @tc.desc: Verify set scheme and add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0700, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_TRUE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    bool stickty = true;
    bool result = false;
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
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0800
 * @tc.name:SetSticky
 * @tc.desc: Verify set scheme and add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0800, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string eventAction = "TESTEVENT_STICKY_ACTION_ENTITY_SCHEME_INFO_FALSE";
    std::string entity = "ADDENTITY";
    std::string scheme = "SETSCHEME";
    bool stickty = false;
    bool result = false;
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
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_0900
 * @tc.name: GetStickyCommonEvent
 * @tc.desc: publish common event set sticky to true and verify the action of stickyData
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_0900, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_GETSTICKY_";
    std::string eventAction = "TESTEVENT_GETSTICKY_";
    std::string eventActionStr = "TESTEVENT_GETSTICKY_Str";
    bool stickty = true;
    bool result = false;
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
        CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
        result = true;
    }
    EXPECT_TRUE(result);

    CommonEventData stickyData;
    CommonEventManager::GetStickyCommonEvent(eventAction, stickyData);
    EXPECT_FALSE(eventActionStr == stickyData.GetWant().GetAction());
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEventSetViscosity_1000
 * @tc.name: set sticky  and get another action
 * @tc.desc: publish common event set sticky to true and verify the action of stickyData
 */
HWTEST_F(cesSystemTest, CES_SendEventSetViscosity_1000, TestSize.Level2)
{
    std::string eventName = "TESTEVENT_GETSTICKY_FALSE";
    std::string eventAction = "TESTEVENT_GETSTICKY_FALSE";
    std::string actionTest = "CHECKTESTACTION";
    bool stickty = true;
    bool result = false;
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
        CommonEventManager::PublishCommonEvent(commonEventData, publishInfo)) {
        result = true;
    }
    EXPECT_TRUE(result);

    CommonEventData stickyData;
    bool stickyResult = CommonEventManager::GetStickyCommonEvent(actionTest, stickyData);
    EXPECT_FALSE(stickyResult);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0100
 * @tc.name:OnReceiveEvent
 * @tc.desc: Verify the function when only set action
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTIONReceiveEvent_0100";
    std::string eventAction = "TESTEVENT_RECEIVE_ACTIONReceiveEvent_0100";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData))) {
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
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0200
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify the function when add entity
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ENTITYReceiveEvent_0200";
    std::string eventAction = "TESTEVENT_RECEIVE_ENTITYReceiveEvent_0200";
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
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0300
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify set action with sticky is false
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0300, Function | MediumTest | Level1)
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
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0400
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify set action with sticky is true
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_RECEIVE_ACTION_INFO_TRUEReceiveEvent_0400";
    std::string eventAction = "TESTEVENT_RECEIVE_ACTION_INFO_TRUEReceiveEvent_0400";
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
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_FALSE(result);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0500
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify add entity with sticky is false
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0500, Function | MediumTest | Level1)
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
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_ReceiveEvent_0600
 * @tc.name:OnReceiveEvent
 * @tc.desc:  Verify add entity with sticky is true
 */
HWTEST_F(cesSystemTest, CES_ReceiveEvent_0600, Function | MediumTest | Level1)
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
    // The publisher sets the Entity, the receiver must set it, otherwise the receiver will not receive the information
    EXPECT_FALSE(result);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0100
 * @tc.name: AddEvent
 * @tc.desc: Verify add an event Theme
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEventTheme_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTADDTHEME";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0200
 * @tc.name: AddEvent
 * @tc.desc: Verify add multiple enent themes
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEventTheme_0200, Function | MediumTest | Level1)
{
    std::string eventName1 = "TESTADDTHEME1";
    std::string eventName2 = "TESTADDTHEME2";

    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName1);
    matchingSkills.AddEvent(eventName2);

    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0300
 * @tc.name: MatchEvent
 * @tc.desc: Verify march a event theme
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEventTheme_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_MATCHEVENTTEST";
    bool result = false;

    Want wantTest;
    wantTest.SetAction(eventName);
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = matchingSkills.Match(wantTest);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriptionEventTheme_0400
 * @tc.name: MatchEvent
 * @tc.desc: Verify march other event theme
 */
HWTEST_F(cesSystemTest, CES_SubscriptionEventTheme_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTMATCHEVENTTOPICAL";
    std::string eventNameCompare = "TESTMATCHEVENTTOPICAL_COMPARE";
    bool result = true;

    Want wantTest;
    wantTest.SetAction(eventNameCompare);
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = matchingSkills.Match(wantTest);
    }
    EXPECT_FALSE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1300
 * @tc.name: PublishCommonEvent
 * @tc.desc: The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1300, Function | MediumTest | Level1)
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
        mtx_.lock();
    }
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_TRUE(sysResult);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1400
 * @tc.name: PublishCommonEvent
 * @tc.desc: The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1400, Function | MediumTest | Level1)
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
        mtx_.lock();
    }
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_TRUE(sysResult);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1500
 * @tc.name: PublishCommonEvent
 * @tc.desc: The publisher can send normally, but does not have permission
 * to send system events and cannot receive published system events
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1500, Function | MediumTest | Level1)
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
        mtx_.lock();
    }
    // The publisher can send normally, but does not have permission to send system events
    EXPECT_TRUE(result);
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_) {
            sysResult = true;
            break;
        }
    }
    // Unable to receive published system events, failed to send system events
    EXPECT_TRUE(sysResult);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1600
 * @tc.name: PublishCommonEvent
 * @tc.desc: publisher cannot receive published system events
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1600, Function | MediumTest | Level1)
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
    // System events published by ordinary publishers, the publication fails, and the receiver cannot receive it
    EXPECT_FALSE(result);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1700
 * @tc.name: PublishCommonEvent
 * @tc.desc: Both subscribers subscribe to the event, after the event is published, both the subscribers can receive the
 *           event
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1700, Function | MediumTest | Level1)
{
    std::string eventName = CompareStr;
    std::string eventAction = CompareStr;
    MatchingSkills matchingSkills;
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
    sleep(1);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr1);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SendEvent_1800
 * @tc.name: PublishCommonEvent
 * @tc.desc: One subscriber subscribe to the event and another subscriber does not subscribe to the event,
 *           after the event is published, subscribed subscriber can receive the event and unsubscribed
 *           subscribe can not receive the event
 */
HWTEST_F(cesSystemTest, CES_SendEvent_1800, Function | MediumTest | Level1)
{
    std::string eventName = CompareStrFalse;
    std::string eventAction = CompareStrFalse;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    commonEventData.SetCode(200);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    sleep(1);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SetEventAuthority_0100
 * @tc.name: SetPermission
 * @tc.desc: Set permission for common event subscribers and  verify  successfully subscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SetEventAuthority_0200
 * @tc.name:Set permission and priority for common event subscribers
 * @tc.desc: Set permission and priority for common event subscribers and  verify  successfully subscribe to
 * common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SetEventAuthority_0300
 * @tc.name: SetPermission SetPriority and SetDeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and  verify  successfully
 * subscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_PERMISSION_PRIORITY_D";
    std::string permissin = "PERMISSION";
    std::string deviceId = "deviceId";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    subscribeInfo.SetDeviceId(deviceId);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SetEventAuthority_0400
 * @tc.name: SetPermission
 * @tc.desc: Set permission for common event subscribers and verify successfully Unsubscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION";
    std::string permissin = "PERMISSION";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0500
 * @tc.name: SetPermission and  SetPriority
 * @tc.desc: Set permission and priority for common event subscribers and verify successfully Unsubscribe to
 * common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0500, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION_PRIORITY";
    std::string permissin = "PERMISSION";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0600
 * @tc.name: SetPermission SetPriority and SetDeviceId
 * @tc.desc: Set permission and priority and DeviceId for common event subscribers and  verify  successfully
 * Unsubscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0600, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_PERMISSION_PRIORITY_D";
    std::string permissin = "PERMISSION";
    std::string deviceId = "deviceId";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission(permissin);
    subscribeInfo.SetPriority(1);
    subscribeInfo.SetDeviceId(deviceId);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_0700
 * @tc.name: SetSubscriberPermissions
 * @tc.desc: Set permission for common event subscribers and  verify  successfully publish common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0700, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_PERMISSION";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_PERMISSION";
    std::string permissin = "PERMISSION";
    std::vector<std::string> permissins;
    permissins.emplace_back(permissin);
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSubscriberPermissions(permissins);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData, publishInfo);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SetEventAuthority_0800
 * @tc.name: SetSubscriberPermissions
 * @tc.desc: Set permission for common event subscribers and  verify  successfully receive common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0800, Function | MediumTest | Level1)
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
            result = true;
            break;
        }
    }
    EXPECT_TRUE(result);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SetEventAuthority_0900
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully subscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_0900, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_SUBSCRIBER_SETTHREADMODE";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SetEventAuthority_1000
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully Unsubscribe to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_1000, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_UNSUBSCRIBER_SETTHREADMODE";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_SetEventAuthority_1100
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully publish to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_1100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SETTHREADMODE";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SETTHREADMODE";
    bool result = false;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData);
    }
    EXPECT_TRUE(result);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SetEventAuthority_1200
 * @tc.name: SetThreadMode
 * @tc.desc: Set ThreadMode for common event subscribers and  verify  successfully receive  to common events
 */
HWTEST_F(cesSystemTest, CES_SetEventAuthority_1200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_PUBLISH_ACTION_SETHANDLER_HANDLER_STCES";
    std::string eventAction = "TESTEVENT_PUBLISH_ACTION_SETHANDLER_HANDLER_STCES";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    Want wantTest;
    wantTest.SetAction(eventAction);
    CommonEventData commonEventData(wantTest);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::POST);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr) &&
        (CommonEventManager::PublishCommonEvent(commonEventData))) {
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
    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    mtx_.unlock();
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0100
 * @tc.name: GetEvent
 * @tc.desc: check to get the added event
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0100, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_GETMATCHINGSKILLS";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    MatchingSkills testMatching = subscribeInfo.GetMatchingSkills();
    EXPECT_EQ(testMatching.GetEvent(0), eventName);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0200
 * @tc.name: GetEntity
 * @tc.desc: check to get the added entity
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0200, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_ADDENTITY_GETENTITY";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    matchingSkills.AddEntity(entity);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    EXPECT_EQ(matchingSkills.GetEntity(0), entity);
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0300
 * @tc.name: HasEntity
 * @tc.desc: verify that entity is  in MatchingSkills
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0300, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_ADDENTITY_HASENTITY";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    matchingSkills.AddEntity(entity);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    EXPECT_TRUE(matchingSkills.HasEntity(entity));
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0400
 * @tc.name: RemoveEntity
 * @tc.desc: verify that the entity was successfully removed
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0400, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_ADDENTITY_REMOVEENTITY";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    matchingSkills.AddEntity(entity);
    matchingSkills.RemoveEntity(entity);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    EXPECT_FALSE(matchingSkills.HasEntity(entity));
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_VerifyMatchingSkills_0500
 * @tc.name: CountEntities
 * @tc.desc: verify that count correct number of entities
 */
HWTEST_F(cesSystemTest, CES_VerifyMatchingSkills_0500, Function | MediumTest | Level1)
{
    std::string eventName = "TESTEVENT_ADDENTITY_ENTITYCOUNT";
    std::string entity = "entity";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    matchingSkills.AddEntity(entity);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    bool result = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
    EXPECT_TRUE(result);
    EXPECT_TRUE((matchingSkills.CountEntities() >= 1));
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
}

/*
 * @tc.number: CES_SubscriberMaxSize_0200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the maximum number of subscriptions for a single third-party application is 200
 */
HWTEST_F(cesSystemTest, CES_SubscriberMaxSize_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    std::vector<std::shared_ptr<CommonEventServicesSystemTest>> subscriberPtrs;
    for (size_t i = 0; i < SUBSCRIBER_MAX_SIZE; i++) {
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        subscriberPtrs.push_back(subscriberPtr);
        result = CommonEventManager::SubscribeCommonEvent(subscriberPtrs.at(i));
        EXPECT_TRUE(result);
    }
    for (size_t i = 0; i < SUBSCRIBER_MAX_SIZE; i++) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtrs.at(i));
        EXPECT_TRUE(result);
    }
}

/*
 * @tc.number: CES_SubscriberMaxSize_0200
 * @tc.name: SubscribeCommonEvent
 * @tc.desc: Verify that the maximum number of subscriptions for a single third-party application is 200 and the 201st
 * subscription failed
 */
HWTEST_F(cesSystemTest, CES_SubscriberMaxSize_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "TESTEVENT_SUBSCRIBER_PLUS";
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(eventName);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    std::vector<std::shared_ptr<CommonEventServicesSystemTest>> subscriberPtrs;
    for (size_t i = 0; i < SUBSCRIBER_MAX_SIZE_PLUS - 1; i++) {
        auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
        subscriberPtrs.push_back(subscriberPtr);
        result = CommonEventManager::SubscribeCommonEvent(subscriberPtrs.at(i));
        EXPECT_TRUE(result);
    }
    auto subscriberPtr = std::make_shared<CommonEventServicesSystemTest>(subscribeInfo);
    subscriberPtrs.push_back(subscriberPtr);
    EXPECT_FALSE(CommonEventManager::SubscribeCommonEvent(subscriberPtrs.at(SUBSCRIBER_MAX_SIZE_PLUS - 1)));

    for (size_t i = 0; i < SUBSCRIBER_MAX_SIZE_PLUS - 1; i++) {
        result = CommonEventManager::UnSubscribeCommonEvent(subscriberPtrs.at(i));
        EXPECT_TRUE(result);
    }
}

/*
 * @tc.number: CES_PublishOrderTimeOut_0100
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify allowing a receiver to run for ten seconds before giving up on it
 */
HWTEST_F(cesSystemTest, CES_PublishOrderTimeOut_0100, Function | MediumTest | Level1)
{
    std::string eventNameTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT";
    std::string eventActionTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT";
    MatchingSkills matchingSkillsTimeOne;
    matchingSkillsTimeOne.AddEvent(eventNameTimeOut);
    CommonEventSubscribeInfo subscribeInfoOne(matchingSkillsTimeOne);
    subscribeInfoOne.SetPriority(PRIORITY_HIGH);
    auto subscriberTimeOnePtr = std::make_shared<CESPublishOrderTimeOutOne>(subscribeInfoOne);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr));

    MatchingSkills matchingSkillsTimeTwo;
    matchingSkillsTimeTwo.AddEvent(eventNameTimeOut);
    CommonEventSubscribeInfo subscribeInfoTwo(matchingSkillsTimeOne);
    subscribeInfoTwo.SetPriority(PRIORITY_LOW);
    auto subscriberTimeTwoPtr = std::make_shared<CESPublishOrderTimeOutTwo>(subscribeInfoTwo);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr));

    mtx_.lock();
    struct tm startTime = {0};
    Want wantTimeOut;
    wantTimeOut.SetAction(eventActionTimeOut);
    CommonEventData commonEventDataTimeOut(wantTimeOut);
    CommonEventPublishInfo publishInfoTimeOut;
    publishInfoTimeOut.SetOrdered(true);
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_TRUE(CommonEventManager::PublishCommonEvent(commonEventDataTimeOut, publishInfoTimeOut));
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWENTY) {
            break;
        }
    }
    // expect the subscriber could receive the event within 11 seconds.
    std::cout << "seconds: " << seconds << std::endl;
    EXPECT_LT(TIME_OUT_SECONDS_NINE, seconds);
    EXPECT_LT(seconds, TIME_OUT_SECONDS_ELEVEN);
    CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr);
    CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr);
    mtx_.unlock();
}

/*
 * @tc.number: CES_PublishOrderTimeOut_0200
 * @tc.name: OnReceiveEvent
 * @tc.desc: Set the last subscriber and Verify allowing a receiver to run for ten seconds before giving up on it
 */
HWTEST_F(cesSystemTest, CES_PublishOrderTimeOut_0200, Function | MediumTest | Level1)
{
    std::string eventNameTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT_SUBSCRIBER";
    std::string eventActionTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT_SUBSCRIBER";
    MatchingSkills matchingSkillsTimeOne;
    matchingSkillsTimeOne.AddEvent(eventNameTimeOut);
    CommonEventSubscribeInfo subscribeInfoOne(matchingSkillsTimeOne);
    subscribeInfoOne.SetPriority(PRIORITY_HIGH);
    auto subscriberTimeOnePtr = std::make_shared<CESPublishOrderTimeOutOne>(subscribeInfoOne);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr));

    MatchingSkills matchingSkillsTimeTwo;
    matchingSkillsTimeTwo.AddEvent(eventNameTimeOut);
    CommonEventSubscribeInfo subscribeInfoTwo(matchingSkillsTimeOne);
    subscribeInfoTwo.SetPriority(PRIORITY_LOW);
    auto subscriberTimeTwoPtr = std::make_shared<CESPublishOrderTimeOutTwo>(subscribeInfoTwo);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr));

    mtx_.lock();
    struct tm startTime = {0};
    Want wantTimeOut;
    wantTimeOut.SetAction(eventActionTimeOut);
    CommonEventData commonEventDataTimeOut(wantTimeOut);
    CommonEventPublishInfo publishInfoTimeOut;
    publishInfoTimeOut.SetOrdered(true);
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_TRUE(
        CommonEventManager::PublishCommonEvent(commonEventDataTimeOut, publishInfoTimeOut, subscriberTimeTwoPtr));
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWENTY) {
            break;
        }
    }
    // expect the subscriber could receive the event within 11 seconds.
    std::cout << "seconds: " << seconds << std::endl;
    EXPECT_LT(TIME_OUT_SECONDS_NINE, seconds);
    EXPECT_LT(seconds, TIME_OUT_SECONDS_ELEVEN);
    CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr);
    CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr);
    mtx_.unlock();
}

/*
 * @tc.number: CES_PublishOrderTimeOut_0300
 * @tc.name: OnReceiveEvent
 * @tc.desc: Without setting priority for subscribers and Verify allowing a receiver to run for ten seconds before
 * giving up on it
 */
HWTEST_F(cesSystemTest, CES_PublishOrderTimeOut_0300, Function | MediumTest | Level1)
{
    std::string eventNameTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT_NOPRIORITY";
    std::string eventActionTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT_NOPRIORITY";
    MatchingSkills matchingSkillsTimeOne;
    matchingSkillsTimeOne.AddEvent(eventNameTimeOut);
    CommonEventSubscribeInfo subscribeInfoOne(matchingSkillsTimeOne);
    auto subscriberTimeOnePtr = std::make_shared<CESPublishOrderTimeOutOne>(subscribeInfoOne);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr));

    MatchingSkills matchingSkillsTimeTwo;
    matchingSkillsTimeTwo.AddEvent(eventNameTimeOut);
    CommonEventSubscribeInfo subscribeInfoTwo(matchingSkillsTimeOne);
    auto subscriberTimeTwoPtr = std::make_shared<CESPublishOrderTimeOutTwo>(subscribeInfoTwo);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr));

    mtx_.lock();
    struct tm startTime = {0};
    Want wantTimeOut;
    wantTimeOut.SetAction(eventActionTimeOut);
    CommonEventData commonEventDataTimeOut(wantTimeOut);
    CommonEventPublishInfo publishInfoTimeOut;
    publishInfoTimeOut.SetOrdered(true);
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_TRUE(CommonEventManager::PublishCommonEvent(commonEventDataTimeOut, publishInfoTimeOut));
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWENTY) {
            break;
        }
    }
    // expect the subscriber could receive the event within 11 seconds.
    std::cout << "seconds: " << seconds << std::endl;
    EXPECT_LT(TIME_OUT_SECONDS_NINE, seconds);
    EXPECT_LT(seconds, TIME_OUT_SECONDS_ELEVEN);
    CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr);
    CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr);
    mtx_.unlock();
}

/*
 * @tc.number: CES_PublishOrderTimeOut_0400
 * @tc.name: OnReceiveEvent
 * @tc.desc: Set the last subscriber and Without setting priority for subscribers, Verify allowing a receiver to run
 * for ten seconds before giving up on it
 */
HWTEST_F(cesSystemTest, CES_PublishOrderTimeOut_0400, Function | MediumTest | Level1)
{
    std::string eventNameTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT_SUBSCRIBER_NOPRIORITY";
    std::string eventActionTimeOut = "TESTEVENT_PUBLISHORDER_TIMEOUT_SUBSCRIBER_NOPRIORITY";
    MatchingSkills matchingSkillsTimeOne;
    matchingSkillsTimeOne.AddEvent(eventNameTimeOut);
    CommonEventSubscribeInfo subscribeInfoOne(matchingSkillsTimeOne);
    auto subscriberTimeOnePtr = std::make_shared<CESPublishOrderTimeOutOne>(subscribeInfoOne);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr));

    MatchingSkills matchingSkillsTimeTwo;
    matchingSkillsTimeTwo.AddEvent(eventNameTimeOut);
    CommonEventSubscribeInfo subscribeInfoTwo(matchingSkillsTimeOne);
    auto subscriberTimeTwoPtr = std::make_shared<CESPublishOrderTimeOutTwo>(subscribeInfoTwo);
    EXPECT_TRUE(CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr));

    mtx_.lock();
    struct tm startTime = {0};
    Want wantTimeOut;
    wantTimeOut.SetAction(eventActionTimeOut);
    CommonEventData commonEventDataTimeOut(wantTimeOut);
    CommonEventPublishInfo publishInfoTimeOut;
    publishInfoTimeOut.SetOrdered(true);
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);
    EXPECT_TRUE(
        CommonEventManager::PublishCommonEvent(commonEventDataTimeOut, publishInfoTimeOut, subscriberTimeTwoPtr));
    struct tm doingTime = {0};
    int64_t seconds = 0;
    while (!mtx_.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_TWENTY) {
            break;
        }
    }
    // expect the subscriber could receive the event within 11 seconds.
    std::cout << "seconds: " << seconds << std::endl;
    EXPECT_LT(TIME_OUT_SECONDS_NINE, seconds);
    EXPECT_LT(seconds, TIME_OUT_SECONDS_ELEVEN);
    CommonEventManager::SubscribeCommonEvent(subscriberTimeOnePtr);
    CommonEventManager::SubscribeCommonEvent(subscriberTimeTwoPtr);
    mtx_.unlock();
}

/*
 * @tc.number: CES_PermissionAndOrDefault_0100
 * @tc.name: OnReceiveEvent
 * @tc.desc: Verify system applications handle system public events with permissions
 */
HWTEST_F(cesSystemTest, CES_PermissionAndOrDefault_0100, Function | MediumTest | Level1)
{
    std::string eventAndN = "COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_DISCOVERED";
    std::string eventDefaultN = "COMMON_EVENT_USER_SWITCHED";

    Want wantAnd;
    wantAnd.SetAction(eventAndN);
    CommonEventData commonEventDataAnd(wantAnd);
    CommonEventPublishInfo publishInfoAnd;
    bool resultAnd = EventFwk::CommonEventManager::PublishCommonEvent(commonEventDataAnd, publishInfoAnd);
    EXPECT_TRUE(resultAnd);

    Want wantDefaultN;
    wantDefaultN.SetAction(eventDefaultN);
    CommonEventData commonEventDataDefaultN(wantDefaultN);
    CommonEventPublishInfo publishInfoDefaultN;
    bool resultDefaultN =
        EventFwk::CommonEventManager::PublishCommonEvent(commonEventDataDefaultN, publishInfoDefaultN);
    EXPECT_TRUE(resultDefaultN);
}
}  // namespace EventFwk
}  // namespace OHOS