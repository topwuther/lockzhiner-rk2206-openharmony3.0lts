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

#include "common_event.h"

#define private public
#define protected public
#include "common_event_manager_service.h"
#include "common_event_subscriber_manager.h"
#undef private
#undef protected

#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "system_ability_definition.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;

namespace {
const std::string EVENT = "com.ces.test.event";
const std::string ENTITY = "com.ces.test.entity";
const std::string SCHEME = "com.ces.test.scheme";
const std::string PERMISSION = "com.ces.test.permission";
const std::string DEVICEDID = "deviceId";
const int PRIORITY = 1;
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;

class CommonEventUnSubscribeUnitTest : public testing::Test {
public:
    CommonEventUnSubscribeUnitTest()
    {}
    ~CommonEventUnSubscribeUnitTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
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

void CommonEventUnSubscribeUnitTest::SetUpTestCase(void)
{
    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject, saExtraProp);
}

void CommonEventUnSubscribeUnitTest::TearDownTestCase(void)
{}

void CommonEventUnSubscribeUnitTest::SetUp(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStart();
}

void CommonEventUnSubscribeUnitTest::TearDown(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0100
 * @tc.name: test SubscribeCommonEvent
 * @tc.desc: Verify UnSubscribeCommonEvent success
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0100, Function | MediumTest | Level1)
{
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddScheme(SCHEME);

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(PRIORITY);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make common event listner
    CommonEventListener *listener = new CommonEventListener(subscriber);

    // UnsubscribeCommonEvent
    bool result =
        OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0200
 * @tc.name: test SubscribeCommonEvent
 * @tc.desc: Verify UnSubscribeCommonEvent fail bacause service stop
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0200, Function | MediumTest | Level1)
{
    // stop
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();

    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddScheme(SCHEME);

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(PRIORITY);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make common event listner
    CommonEventListener *listener = new CommonEventListener(subscriber);

    bool result =
        OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0300
 * @tc.name: exception
 * @tc.desc: verify InnerCommonEventManager UnsubscribeCommonEvent function success
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0300, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make common event listner
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // UnsubscribeCommonEvent
    InnerCommonEventManager innerCommonEventManager;
    EXPECT_EQ(true, innerCommonEventManager.UnsubscribeCommonEvent(commonEventListenerPtr));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0400
 * @tc.name: exception
 * @tc.desc: When the CommonEventListener does not exist,
 *                      verify InnerCommonEventManager UnsubscribeCommonEvent function return value.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0400, Function | MediumTest | Level1)
{
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager = std::make_shared<InnerCommonEventManager>();
    OHOS::sptr<OHOS::IRemoteObject> sp(nullptr);
    EXPECT_EQ(false, innerCommonEventManager->UnsubscribeCommonEvent(sp));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0500
 * @tc.name: exception
 * @tc.desc: verify CommonEventSubscriberManager RemoveSubscriber success
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0500, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make subscriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make common event listner
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // RemoveSubscriber
    CommonEventSubscriberManager commonEventSubscriberManager;
    int result = commonEventSubscriberManager.RemoveSubscriber(commonEventListenerPtr);
    EXPECT_EQ(OHOS::ERR_OK, result);
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0600
 * @tc.name: exception
 * @tc.desc: When the CommonEventListener does not exist,
 *                       verify RemoveSubscriber fail.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0600, Function | MediumTest | Level1)
{
    CommonEventSubscriberManager commonEventSubscriberManager;
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, commonEventSubscriberManager.RemoveSubscriber(nullptr));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0700
 * @tc.name: exception
 * @tc.desc: When the death_ is not null, verify RemoveSubscriber fail.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0700, Function | MediumTest | Level1)
{
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddScheme(SCHEME);

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(PRIORITY);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    // make a subcriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make common event listner
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    // RemoveSubscriber
    CommonEventSubscriberManager commonEventSubscriberManager;
    commonEventSubscriberManager.death_ = nullptr;
    EXPECT_EQ(OHOS::ERR_OK, commonEventSubscriberManager.RemoveSubscriber(commonEventListener));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0800
 * @tc.name: exception
 * @tc.desc: verify RemoveSubscriberRecordLocked success.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0800, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make subcriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make commone event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // RemoveSubscriber
    CommonEventSubscriberManager commonEventSubscriberManager;
    EXPECT_EQ(OHOS::ERR_OK, commonEventSubscriberManager.RemoveSubscriberRecordLocked(commonEventListenerPtr));
}

/*
 * @tc.number: CommonEventUnSubscribeUnitTest_0900
 * @tc.name: exception
 * @tc.desc: When the commonEventListener is null, verify RemoveSubscriberRecordLocked success.
 */
HWTEST_F(CommonEventUnSubscribeUnitTest, CommonEventUnSubscribeUnitTest_0900, Function | MediumTest | Level1)
{
    // make subcriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make subcriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make commone event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    // RemoveSubscriber
    CommonEventSubscriberManager commonEventSubscriberManager;
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, commonEventSubscriberManager.RemoveSubscriberRecordLocked(nullptr));
}
