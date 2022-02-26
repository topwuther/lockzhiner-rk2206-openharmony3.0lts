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

#include <thread>
#include <chrono>

#include <sys/time.h>

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_manager_service.h"
#undef private
#undef protected

#include "datetime_ex.h"
#include "iremote_object.h"
#include "message_parcel.h"
#include "sa_mgr_client.h"
#include "singleton.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "want.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;
using OHOS::Parcel;

namespace {
const std::string EVENT = "com.ces.test.event";
std::mutex mtx;
const time_t TIME_OUT_SECONDS_LIMIT = 5;
}  // namespace

class SubscriberTest;

class CommonEventSubscribeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    static constexpr int TEST_WAIT_TIME = 100000;
    std::shared_ptr<CommonEventManagerService> commonEventMs_;
    CommonEventManager commonEventManager;
    MatchingSkills matchingSkills;
};

class SubscriberTest : public CommonEventSubscriber {
public:
    SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        mtx.unlock();
    }
};

class EventReceiveStubTest : public EventReceiveStub {
public:
    EventReceiveStubTest()
    {}

    ~EventReceiveStubTest()
    {}

    virtual void NotifyEvent(const CommonEventData &commonEventData, const bool &ordered, const bool &sticky)
    {}
};

class CommonEventStubTest : public CommonEventStub {
public:
    CommonEventStubTest()
    {}

    virtual bool PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
        const OHOS::sptr<IRemoteObject> &commonEventListener)
    {
        return false;
    }

    virtual bool SubscribeCommonEvent(
        const CommonEventSubscribeInfo &subscriberInfo, const OHOS::sptr<IRemoteObject> &commonEventListener)
    {
        return false;
    }

    virtual bool UnsubscribeCommonEvent(const OHOS::sptr<IRemoteObject> &commonEventListener)
    {
        return false;
    }

    virtual bool DumpState(const std::string &event, std::vector<std::string> &state)
    {
        return false;
    }

    virtual ~CommonEventStubTest()
    {}

    virtual bool FinishReceiver(const OHOS::sptr<IRemoteObject> &proxy, const int &code,
        const std::string &receiverData, const bool &abortEvent)
    {
        return false;
    }
};

void CommonEventSubscribeTest::SetUpTestCase(void)
{}

void CommonEventSubscribeTest::TearDownTestCase(void)
{}

void CommonEventSubscribeTest::SetUp(void)
{}

void CommonEventSubscribeTest::TearDown(void)
{}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEvent SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. success subscribe common event with right parameters
 */

HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_001, TestSize.Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriber);

    EXPECT_EQ(true, subscribeResult);

    /* Publish */

    // make a want
    Want want;
    want.SetAction(EVENT);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // lock the mutex
    mtx.lock();

    // publish a common event
    bool publishResult = CommonEventManager::PublishCommonEvent(data);

    EXPECT_EQ(true, publishResult);

    // record start time of publishing
    struct tm startTime = {0};
    EXPECT_EQ(OHOS::GetSystemCurrentTime(&startTime), true);

    // record current time
    struct tm doingTime = {0};
    int64_t seconds = 0;

    while (!mtx.try_lock()) {
        // get current time and compare it with the start time
        EXPECT_EQ(OHOS::GetSystemCurrentTime(&doingTime), true);
        seconds = OHOS::GetSecondsBetween(startTime, doingTime);
        if (seconds >= TIME_OUT_SECONDS_LIMIT) {
            break;
        }
    }

    // expect the subscriber could receive the event within 5 seconds.
    EXPECT_LT(seconds, TIME_OUT_SECONDS_LIMIT);
    mtx.unlock();
}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEventSubscriberManager InsertSubscriber
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. different subscriber subscribe event
 *                   3. success subscribe common event with right parameters
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_002, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscribera = std::make_shared<SubscriberTest>(subscriberInfo);
    std::shared_ptr<SubscriberTest> subscriberb = std::make_shared<SubscriberTest>(subscriberInfo);

    bool subscribeResulta = CommonEventManager::SubscribeCommonEvent(subscribera);

    EXPECT_EQ(true, subscribeResulta);

    bool subscribeResultb = CommonEventManager::SubscribeCommonEvent(subscriberb);

    EXPECT_EQ(true, subscribeResultb);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEvent SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event kit with null subscriber
 */

HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_003, TestSize.Level1)
{
    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(nullptr);

    EXPECT_EQ(false, subscribeResult);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEvent SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event with no event
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_004, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriber);

    EXPECT_EQ(false, subscribeResult);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function:CommonEvent SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event because common event listener has subsrciber
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_005, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    CommonEventManager::SubscribeCommonEvent(subscriber);

    bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriber);

    EXPECT_EQ(true, subscribeResult);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventProxy  SubscribeCommonEvent CommonEventStub OnRemoteRequest
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event kit proxy subscriber info is null to do
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_006, TestSize.Level1)
{
    CommonEventStubTest CommonEventStubTest;
    OHOS::MessageParcel data, reply;
    OHOS::MessageOption option;

    int subscribeResult = CommonEventStubTest.OnRemoteRequest(
        static_cast<uint32_t>(ICommonEvent::Message::CES_SUBSCRIBE_COMMON_EVENT), data, reply, option);

    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, subscribeResult);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventProxy  SubscribeCommonEvent CommonEventStub OnRemoteRequest
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event kit proxy common event listener is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_007, TestSize.Level1)
{
    CommonEventStubTest CommonEventStubTest;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    OHOS::MessageParcel data, reply;
    OHOS::MessageOption option;
    data.WriteParcelable(&subscriberInfo);

    int subscribeResult = CommonEventStubTest.OnRemoteRequest(
        static_cast<uint32_t>(ICommonEvent::Message::CES_SUBSCRIBE_COMMON_EVENT), data, reply, option);

    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, subscribeResult);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventProxy  SubscribeCommonEvent CommonEventStub OnRemoteRequest
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event code is error
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_008, TestSize.Level1)
{
    CommonEventStubTest CommonEventStubTest;
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    OHOS::MessageParcel data, reply;
    OHOS::MessageOption option;
    data.WriteParcelable(&subscriberInfo);

    int subscribeResult = CommonEventStubTest.OnRemoteRequest(static_cast<uint32_t>(-1), data, reply, option);

    EXPECT_EQ(OHOS::IPC_STUB_UNKNOW_TRANS_ERR, subscribeResult);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: InnerCommonEventManager SubscribeCommonEvent
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event, inner common event manager
 *                      common event listener is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_009, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> sp(nullptr);

    struct tm curTime;

    EXPECT_EQ(false, innerCommonEventManager->SubscribeCommonEvent(subscriberInfo, sp, curTime, 0, 0, ""));
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventSubscriberManager InsertSubscriber
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event , common event subscriber manager
 *                      event subscriber info is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_010, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    struct tm curTime{0};

    int result = OHOS::DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertSubscriber(
        nullptr, commonEventListenerPtr, curTime, 0, 0, "");

    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventSubscriberManager InsertSubscriber
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event , common event subscriber manager
 *                      event common event listener is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_011, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscribeInfo =
        std::make_shared<CommonEventSubscribeInfo>(subscriberInfo);

    struct tm curTime{0};

    int result = OHOS::DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertSubscriber(
        commonEventSubscribeInfo, nullptr, curTime, 0, 0, "");

    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventSubscriberManager InsertSubscriber
 * SubFunction: Subscribe common event
 * FunctionPoints: test subscribe event
 * EnvConditions: system run normally
 * CaseDescription:  1. subscribe common event
 *                   2. fail subscribe common event , common event subscriber manager
 *                      event size is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_012, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscribeInfo =
        std::make_shared<CommonEventSubscribeInfo>(subscriberInfo);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerSp(commonEventListener);

    struct tm curTime{0};

    int result = OHOS::DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->InsertSubscriber(
        commonEventSubscribeInfo, commonEventListenerSp, curTime, 0, 0, "");

    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: EventReceiveStub OnRemoteRequest
 * SubFunction: EventReceiveStub OnRemoteRequest
 * FunctionPoints: test on Remote request
 * EnvConditions: system run normally
 * CaseDescription:  1. event receive fail
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_013, TestSize.Level1)
{
    EventReceiveStubTest eventReceiveStub;
    OHOS::MessageParcel data, reply;
    OHOS::MessageOption option;

    int result = eventReceiveStub.OnRemoteRequest(0, data, reply, option);

    EXPECT_EQ(OHOS::IPC_STUB_UNKNOW_TRANS_ERR, result);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventListener IsReady
 * SubFunction: Subscribe common event
 * FunctionPoints: test is ready
 * EnvConditions: system run normally
 * CaseDescription:  1. ready fail because runner is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_014, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscribeInfo =
        std::make_shared<CommonEventSubscribeInfo>(subscriberInfo);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener commonEventListener(subscriber);
    commonEventListener.runner_ = nullptr;

    int result = commonEventListener.IsReady();

    EXPECT_EQ(false, result);
}

/*
 * Feature: CommonEventSubscribeTest
 * Function: CommonEventListener IsReady
 * SubFunction: Subscribe common event
 * FunctionPoints: test is ready
 * EnvConditions: system run normally
 * CaseDescription:  1. ready fail because handler is null
 */
HWTEST_F(CommonEventSubscribeTest, CommonEventSubscribe_015, TestSize.Level1)
{
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscribeInfo =
        std::make_shared<CommonEventSubscribeInfo>(subscriberInfo);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener commonEventListener(subscriber);
    commonEventListener.handler_ = nullptr;

    int result = commonEventListener.IsReady();

    EXPECT_EQ(false, result);
}