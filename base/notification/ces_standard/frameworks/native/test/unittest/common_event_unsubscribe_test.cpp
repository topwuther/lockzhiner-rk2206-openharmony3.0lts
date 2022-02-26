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

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_manager_service.h"
#undef private
#undef protected

#include "errors.h"
#include "sa_mgr_client.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;
using OHOS::Parcel;

class CommonEventUnSubscribeTest : public testing::Test {
public:
    CommonEventUnSubscribeTest()
    {}
    ~CommonEventUnSubscribeTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    MatchingSkills matchingSkills_;
    void SetMatchingSkillsWithEvent(const std::string &event);
    void SetMatchingSkillsWithEntity(const std::string &entity);
    void SetMatchingSkillsWithScheme(const std::string &scheme);
};

class DreivedSubscriber : public CommonEventSubscriber {
public:
    DreivedSubscriber(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~DreivedSubscriber()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
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

class SubscriberTest : public CommonEventSubscriber {
public:
    SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

void CommonEventUnSubscribeTest::SetUpTestCase(void)
{}

void CommonEventUnSubscribeTest::TearDownTestCase(void)
{}

void CommonEventUnSubscribeTest::SetUp(void)
{}

void CommonEventUnSubscribeTest::TearDown(void)
{}

void CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent(const std::string &event)
{
    matchingSkills_.AddEvent(event);
}

void CommonEventUnSubscribeTest::SetMatchingSkillsWithEntity(const std::string &entity)
{
    matchingSkills_.AddEntity(entity);
}

void CommonEventUnSubscribeTest::SetMatchingSkillsWithScheme(const std::string &scheme)
{
    matchingSkills_.AddScheme(scheme);
}

/*
 * Feature: CommonEventManager
 * Function: UnSubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Unsubscribe normally and verify UnSubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_001, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscriberInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscriberInfo);
    std::shared_ptr<CommonEventManager> helper;
    EXPECT_EQ(true, helper->UnSubscribeCommonEvent(subscriber));
}

/*
 * Feature: CommonEvent
 * Function: UnSubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Verify UnSubscribeCommonEvent function return value with eventListener not exist.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_002, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscriberInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscriberInfo);
    EXPECT_EQ(true, DelayedSingleton<CommonEvent>::GetInstance()->UnSubscribeCommonEvent(subscriber));
}

/*
 * Feature: CommonEvent
 * Function: UnSubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Verify UnSubscribeCommonEvent function return value with eventListener existed.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_003, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscriberInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscriberInfo);
    DelayedSingleton<CommonEvent>::GetInstance()->SubscribeCommonEvent(subscriber);
    EXPECT_EQ(true, DelayedSingleton<CommonEvent>::GetInstance()->UnSubscribeCommonEvent(subscriber));
}

/*
 * Feature: InnerCommonEventManager
 * Function: UnsubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Verify UnSubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_004, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscriberInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscriberInfo);
    sptr<IRemoteObject> commonEventListener = new CommonEventListener(subscriber);
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    EXPECT_EQ(true, innerCommonEventManager->UnsubscribeCommonEvent(commonEventListener));
}

/*
 * Feature: CommonEventSubscriberManager
 * Function: RemoveSubscriber
 * SubFunction: NA
 * FunctionPoints: normal
 * EnvConditions: system running normally
 * CaseDescription: Verify RemoveSubscriber function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_005, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscriberInfo(matchingSkills_);
    std::shared_ptr<DreivedSubscriber> subscriber = std::make_shared<DreivedSubscriber>(subscriberInfo);
    sptr<IRemoteObject> commonEventListener = new CommonEventListener(subscriber);
    EXPECT_EQ(
        ERR_OK, DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->RemoveSubscriber(commonEventListener));
}

/*
 * Feature: CommonEvent
 * Function: UnSubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the subscriber does not exist, verify UnSubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_006, TestSize.Level1)
{
    std::shared_ptr<DreivedSubscriber> subscriber = nullptr;
    EXPECT_EQ(false, DelayedSingleton<CommonEvent>::GetInstance()->UnSubscribeCommonEvent(subscriber));
}

/*
 * Feature: CommonEventStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the CommonEventListener does not exist, verify IPC return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_007, TestSize.Level1)
{
    CommonEventStubTest CommonEventStubTest;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    sptr<CommonEventListener> commonEventListener = nullptr;
    data.WriteParcelable(commonEventListener);

    int32_t ret = CommonEventStubTest.OnRemoteRequest(
        static_cast<int32_t>(ICommonEvent::Message::CES_UNSUBSCRIBE_COMMON_EVENT), data, reply, option);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

/*
 * Feature: CommonEventManagerService
 * Function: UnsubscribeCommonEvent
 * SubFunction: IsReady
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the CommonEventManagerService is not ready,
 *                  verify UnsubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_008, TestSize.Level1)
{
    DelayedSingleton<CommonEventManagerService>::GetInstance()->innerCommonEventManager_ = nullptr;
    sptr<IRemoteObject> sp;
    EXPECT_EQ(false, DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(sp));
}

/*
 * Feature: InnerCommonEventManager
 * Function: UnsubscribeCommonEvent
 * SubFunction: NA
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the CommonEventListener does not exist,
 *                  verify UnsubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_009, TestSize.Level1)
{
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    sptr<IRemoteObject> sp(nullptr);
    EXPECT_EQ(false, innerCommonEventManager->UnsubscribeCommonEvent(sp));
}

/*
 * Feature: CommonEventSubscriberManager
 * Function: RemoveSubscriber
 * SubFunction: NA
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the CommonEventListener does not exist,
 *                  verify RemoveSubscriber function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_010, TestSize.Level1)
{
    EXPECT_EQ(
        ERR_INVALID_VALUE, DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->RemoveSubscriber(nullptr));
}

/*
 * Feature: CommonEventSubscriberManager
 * Function: RemoveSubscriber
 * SubFunction: NA
 * FunctionPoints: exception
 * EnvConditions: system running normally
 * CaseDescription: When the death_ is not null,
 *                  verify RemoveSubscriber function return value.
 */
HWTEST_F(CommonEventUnSubscribeTest, UnSubscribe_011, TestSize.Level1)
{
    CommonEventUnSubscribeTest::SetMatchingSkillsWithEvent("event");
    CommonEventSubscribeInfo subscriberInfo(matchingSkills_);
    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscriberInfo);
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriberTest);
    DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->death_ = nullptr;
    EXPECT_EQ(
        ERR_OK, DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->RemoveSubscriber(commonEventListener));
}