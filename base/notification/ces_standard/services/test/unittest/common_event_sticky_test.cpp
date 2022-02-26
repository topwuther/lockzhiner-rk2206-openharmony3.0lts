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

#define UNIT_TEST

#define private public
#define protected public
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_manager_service.h"
#include "common_event_sticky_manager.h"
#undef private
#undef protected

#include "errors.h"
#include "inner_common_event_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "system_ability_definition.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;

namespace {
const std::string EVENT = "com.ces.test.event";
const std::string EVENT2 = "com.ces.test.event2";
const std::string EVENT3 = "com.ces.test.event3";
const std::string EVENT4 = "com.ces.test.event4";
const std::string EVENT5 = "com.ces.test.event5";
const std::string EVENT6 = "com.ces.test.event6";
const std::string TYPE = "com.ces.test.type";
const std::string TYPE2 = "com.ces.test.type2";
const std::string PERMISSION = "com.ces.test.permission";
const std::string STRING_EVENT = "com.ces.event";
const std::string STRING_DATA = "data";
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;

class CommonEventStickyTest : public testing::Test {
public:
    CommonEventStickyTest()
    {}
    ~CommonEventStickyTest()
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

void CommonEventStickyTest::SetUpTestCase(void)
{
    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject, saExtraProp);
}

void CommonEventStickyTest::TearDownTestCase(void)
{}

void CommonEventStickyTest::SetUp(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStart();
}

void CommonEventStickyTest::TearDown(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();
}

/*
 * @tc.number: CommonEventStickyTest_0100
 * @tc.name: test StickyCommonEvent
 * @tc.desc: Gets the published sticky event
 */
HWTEST_F(CommonEventStickyTest, CommonEventStickyTest_0100, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    want.SetType(TYPE);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    bool publishResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
        data, publishInfo, nullptr);
    EXPECT_EQ(true, publishResult);

    sleep(1);

    CommonEventData Stickydata;
    bool StickyResult =
        OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->GetStickyCommonEvent(EVENT, Stickydata);
    EXPECT_EQ(true, StickyResult);
    EXPECT_EQ(EVENT, Stickydata.GetWant().GetAction());
    EXPECT_EQ(TYPE, Stickydata.GetWant().GetType());
}

/*
 * @tc.number: CommonEventStickyTest_0200
 * @tc.name: test StickyCommonEvent
 * @tc.desc: Confirm whether the acquired sticky event data is the latest released
 */
HWTEST_F(CommonEventStickyTest, CommonEventStickyTest_0200, Function | MediumTest | Level1)
{
    /* Publish */

    // make a want
    Want want;
    want.SetAction(EVENT2);
    want.SetType(TYPE);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    bool publishResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
        data, publishInfo, nullptr);
    EXPECT_EQ(true, publishResult);

    // make a want
    Want want2;
    want2.SetAction(EVENT2);
    want2.SetType(TYPE2);
    // make common event data
    CommonEventData data2;
    data2.SetWant(want2);

    publishResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
        data2, publishInfo, nullptr);
    EXPECT_EQ(true, publishResult);

    sleep(1);

    CommonEventData Stickydata;
    bool StickyResult =
        OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->GetStickyCommonEvent(EVENT2, Stickydata);
    EXPECT_EQ(true, StickyResult);
    EXPECT_EQ(EVENT2, Stickydata.GetWant().GetAction());
    EXPECT_EQ(TYPE2, Stickydata.GetWant().GetType());
}

/*
 * @tc.number: CommonEventStickyTest_0300
 * @tc.name: test StickyCommonEvent
 * @tc.desc: The event parameter of the obtained sticky event is ""
 */
HWTEST_F(CommonEventStickyTest, CommonEventStickyTest_0300, Function | MediumTest | Level1)
{
    /* Publish */

    // make a want
    Want want;
    want.SetAction(EVENT3);
    want.SetType(TYPE);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    bool publishResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
        data, publishInfo, nullptr);
    EXPECT_EQ(true, publishResult);

    CommonEventData Stickydata;
    bool StickyResult =
        OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->GetStickyCommonEvent("", Stickydata);
    EXPECT_EQ(false, StickyResult);
}

/*
 * @tc.number: CommonEventStickyTest_0400
 * @tc.name: test StickyCommonEvent
 * @tc.desc: Get an unpublished sticky event
 */
HWTEST_F(CommonEventStickyTest, CommonEventStickyTest_0400, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT4);
    want.SetType(TYPE);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    bool publishResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
        data, publishInfo, nullptr);
    EXPECT_EQ(true, publishResult);

    CommonEventData Stickydata;
    bool StickyResult =
        OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->GetStickyCommonEvent(EVENT6, Stickydata);
    EXPECT_EQ(false, StickyResult);
}

/*
 * @tc.number: CommonEventStickyTest_0500
 * @tc.name: test StickyCommonEvent
 * @tc.desc: Gets a non sticky event
 */
HWTEST_F(CommonEventStickyTest, CommonEventStickyTest_0500, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT5);
    want.SetType(TYPE);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(false);

    bool publishResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
        data, publishInfo, nullptr);
    EXPECT_EQ(true, publishResult);

    CommonEventData Stickydata;
    bool StickyResult =
        OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->GetStickyCommonEvent(EVENT5, Stickydata);
    EXPECT_EQ(false, StickyResult);
}

/*
 * @tc.number: FindStickyEvents_0600
 * @tc.name: Find events of no subscriber info
 * @tc.desc: Get invalid return value.
 */
HWTEST_F(CommonEventStickyTest, FindStickyEvents_0600, TestSize.Level0)
{
    // make a vector of records
    std::vector<std::shared_ptr<CommonEventRecord>> records;

    // get common event sticky manager
    auto stickyManagerPtr = OHOS::DelayedSingleton<CommonEventStickyManager>::GetInstance();
    // find sticky events
    int result = stickyManagerPtr->FindStickyEvents(nullptr, records);

    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
}

/*
 * @tc.number: FindStickyEvents_0700
 * @tc.name: Fail to find events of subscriber info
 * @tc.desc: No event would be pushed into common event records.
 */
HWTEST_F(CommonEventStickyTest, FindStickyEvents_0700, TestSize.Level0)
{
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto subscriberInfoPtr = std::make_shared<CommonEventSubscribeInfo>(subscriberInfo);

    // make a vector of records
    std::vector<std::shared_ptr<CommonEventRecord>> records;

    // get common event sticky manager
    auto stickyManagerPtr = OHOS::DelayedSingleton<CommonEventStickyManager>::GetInstance();
    // find sticky events
    int result = stickyManagerPtr->FindStickyEvents(subscriberInfoPtr, records);
    // check result of finding sticky events
    EXPECT_EQ(result, OHOS::ERR_OK);
}

/*
 * @tc.number: FindStickyEvents_0800
 * @tc.name: Succeed to find events of subscriber info
 * @tc.desc: Found events would be pushed into common event records.
 */
HWTEST_F(CommonEventStickyTest, FindStickyEvents_0800, TestSize.Level0)
{
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(STRING_EVENT);

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto subscriberInfoPtr = std::make_shared<CommonEventSubscribeInfo>(subscriberInfo);

    // make a vector of records
    std::vector<std::shared_ptr<CommonEventRecord>> records;
    // check size of the records
    EXPECT_EQ((int)records.size(), 0);

    // make a want
    Want want;
    want.SetAction(STRING_EVENT);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make a publish info and set it unordered
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);

    // make a record
    auto recordPtr = std::make_shared<CommonEventRecord>();
    recordPtr->commonEventData = std::make_shared<CommonEventData>(data);
    recordPtr->publishInfo = std::make_shared<CommonEventPublishInfo>(publishInfo);

    // get common event sticky manager
    auto stickyManagerPtr = OHOS::DelayedSingleton<CommonEventStickyManager>::GetInstance();
    // add a record in common event sticky manager
    stickyManagerPtr->commonEventRecords_[STRING_EVENT] = recordPtr;

    // find sticky events
    int result = stickyManagerPtr->FindStickyEvents(subscriberInfoPtr, records);
    // check result of finding sticky events
    EXPECT_EQ(result, OHOS::ERR_OK);
    // check size of the records
    EXPECT_EQ((int)records.size(), 1);

    // check the action
    auto action = records.front()->commonEventData->GetWant().GetAction();
    EXPECT_EQ(action, STRING_EVENT);
}

/*
 * @tc.number: UpdateStickyEvent_0900
 * @tc.name: Update an unexisting record
 * @tc.desc: A new record would be added.
 */
HWTEST_F(CommonEventStickyTest, UpdateStickyEvent_0900, TestSize.Level0)
{
    // make a want
    Want want;
    want.SetAction(STRING_EVENT);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    auto dataPtr = std::make_shared<CommonEventData>(data);

    // make a publish info and set it unordered
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    auto publishInfoPtr = std::make_shared<CommonEventPublishInfo>(publishInfo);

    // get common event sticky manager
    auto stickyManagerPtr = OHOS::DelayedSingleton<CommonEventStickyManager>::GetInstance();

    // update sticky event
    CommonEventRecord eventRecord;
    eventRecord.publishInfo = publishInfoPtr;
    eventRecord.commonEventData = dataPtr;
    int result = stickyManagerPtr->UpdateStickyEvent(eventRecord);
    // check result of updating sticky event
    EXPECT_EQ(result, OHOS::ERR_OK);

    // get record the event
    auto recordPtr = stickyManagerPtr->commonEventRecords_[STRING_EVENT];
    // check record of the event
    EXPECT_NE(recordPtr, nullptr);

    // check the action
    auto action = recordPtr->commonEventData->GetWant().GetAction();
    EXPECT_EQ(action, STRING_EVENT);
}

/*
 * @tc.number: UpdateStickyEvent_1000
 * @tc.name: Update an existing record
 * @tc.desc: The existing record would be updated.
 */
HWTEST_F(CommonEventStickyTest, UpdateStickyEvent_1000, TestSize.Level0)
{
    // get common event sticky manager
    auto stickyManagerPtr = OHOS::DelayedSingleton<CommonEventStickyManager>::GetInstance();

    // get record of the event
    auto recordPtr = stickyManagerPtr->commonEventRecords_[STRING_EVENT];

    // make a want
    Want want;
    want.SetAction(STRING_EVENT);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    recordPtr->commonEventData = std::make_shared<CommonEventData>(data);
    // check the data
    EXPECT_EQ(recordPtr->commonEventData->GetData(), "");

    // make a publish info and set it unordered
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    recordPtr->publishInfo = std::make_shared<CommonEventPublishInfo>(publishInfo);

    // make a new want
    Want newWant;
    newWant.SetAction(STRING_EVENT);

    // make common event data
    CommonEventData newData;
    newData.SetWant(newWant);
    newData.SetData(STRING_DATA);
    auto newDataPtr = std::make_shared<CommonEventData>(newData);

    // make a publish info and set it unordered
    CommonEventPublishInfo newPublishInfo;
    newPublishInfo.SetOrdered(false);
    auto newPublishInfoPtr = std::make_shared<CommonEventPublishInfo>(newPublishInfo);

    // update sticky event
    CommonEventRecord eventRecord;
    eventRecord.publishInfo = newPublishInfoPtr;
    eventRecord.commonEventData = newDataPtr;
    int result = stickyManagerPtr->UpdateStickyEvent(eventRecord);
    // check result of updating sticky event
    EXPECT_EQ(result, OHOS::ERR_OK);

    // get record the event
    recordPtr = stickyManagerPtr->commonEventRecords_[STRING_EVENT];
    // check record of the event
    EXPECT_NE(recordPtr, nullptr);

    // check the action
    auto action = recordPtr->commonEventData->GetWant().GetAction();
    EXPECT_EQ(action, STRING_EVENT);

    // check the data
    auto updatedData = recordPtr->commonEventData->GetData();
    EXPECT_EQ(updatedData, STRING_DATA);
}
