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

// redefine private and protected since testcase need to invoke and test private function
#define private public
#define protected public
#include "common_event_control_manager.h"
#include "common_event_manager_service.h"
#include "common_event_permission_manager.h"
#undef private
#undef protected

#include "common_event_manager.h"
#include "common_event_record.h"
#include "common_event_support.h"
#include "mock_bundle_manager.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "refbase.h"
#include "system_ability_definition.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::EventFwk;

namespace {
const std::string EVENT = "com.ces.test.event";
const std::string ENTITY = "com.ces.test.entity";
const int SLEEPTIEM = 1;
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;

class CommonEventPublishPermissionEventUnitTest : public testing::Test {
public:
    CommonEventPublishPermissionEventUnitTest()
    {}
    ~CommonEventPublishPermissionEventUnitTest()
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

class SubscriberTest1 : public CommonEventSubscriber {
public:
    SubscriberTest1(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest1()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {}
};

void CommonEventPublishPermissionEventUnitTest::SetUpTestCase(void)
{
    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject, saExtraProp);
}

void CommonEventPublishPermissionEventUnitTest::TearDownTestCase(void)
{}

void CommonEventPublishPermissionEventUnitTest::SetUp(void)
{}

void CommonEventPublishPermissionEventUnitTest::TearDown(void)
{}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_0100
 * @tc.name: test unordered event permission
 * @tc.desc: 1. subcriber permisssion is not empty
 *           2. publish permission is not empty
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0100, TestSize.Level1";
    InnerCommonEventManager inner;
    struct tm curTime{0};

    /* subscriber */
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("1234");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("123");
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "hello");

    /* Publish */
    Want want;
    want.SetAction("1234");
    CommonEventData data;
    data.SetWant(want);
    std::vector<std::string> subscriberPermissions;
    subscriberPermissions.emplace_back("456");
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "hello");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_0200
 * @tc.name: test ordered event permission
 * @tc.desc: 1. subcriber permisssion is not empty
 *           2. publish permission is not empty
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0200, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime{0};
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("1234");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("123");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "hello");

    /* Publish */
    Want want;
    want.SetAction("1234");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(true);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, listener, curTime, 0, 1000, "hello");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_0300
 * @tc.name: test unordered event permission
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0300,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0300, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime{0};
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("1234");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "hello");

    /* Publish */
    Want want;
    want.SetAction("1234");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(false);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "hello");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_0400
 * @tc.name: test ordered event permission
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0400,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0400, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime{0};
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("1234");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "hello");

    /* Publish */
    Want want;
    want.SetAction("1234");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(true);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, listener, curTime, 0, 1000, "hello");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_0500
 * @tc.name: test  CommonEventControlManager permission
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is empty
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0500,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0500, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    commonEventPublishInfo.SetSubscriberPermissions(std::vector<std::string>());
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("1234");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);

    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_0600
 * @tc.name: test  CommonEventControlManager permission
 * @tc.desc: 1. subcriber permisssion is not empty
 *           2. publish permission is empty
 *           3. function CheckPermission return OrderedEventRecord::SKIPPED
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0600,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0600, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("12345");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    commonEventPublishInfo.SetSubscriberPermissions(std::vector<std::string>());
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("1234");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);

    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello world";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::OrderedEventRecord::SKIPPED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_0700
 * @tc.name: test  CommonEventControlManager permission
 * @tc.desc: 1. subcriber permisssion is not empty
 *           2. publish permission is empty
 *           3. function CheckPermission return OrderedEventRecord::DELIVERED
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0700,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0700, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("12345");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;

    CommonEventPublishInfo commonEventPublishInfo;
    commonEventPublishInfo.SetSubscriberPermissions(std::vector<std::string>());
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("1234");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_0800
 * @tc.name: test  CommonEventControlManager permission
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. function CheckPermission return OrderedEventRecord::DELIVERED
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0800,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0800, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);
    Want want;
    want.SetAction("1234");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_0900
 * @tc.name: test  CommonEventControlManager permission
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. function CheckPermission return OrderedEventRecord::SKIPPED
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0900,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_0900, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello world";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);
    Want want;
    want.SetAction("1234");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::OrderedEventRecord::SKIPPED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1000
 * @tc.name: test  CommonEventControlManager permission
 * @tc.desc: 1. subcriber permisssion is not empty
 *           2. publish permission is not empty
 *           3. function subcriber CheckPermission return OrderedEventRecord::DELIVERED
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1000,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1000, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("12345");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("1234");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1100
 * @tc.name: test  CommonEventControlManager permission
 * @tc.desc: 1. subcriber permisssion is not empty
 *           2. publish permission is not empty
 *           3. function subcriber CheckPermission return OrderedEventRecord::SKIPPED
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1100, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("12345");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello world";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("1234");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::OrderedEventRecord::SKIPPED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1200
 * @tc.name: test  CommonEventControlManager permission
 * @tc.desc: 1. subcriber permisssion is not empty
 *           2. publish permission is not empty
 *           3. function subcriber CheckPermission return OrderedEventRecord::SKIPPED
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1200, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("12345");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("1234");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello world";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::OrderedEventRecord::SKIPPED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1300
 * @tc.name: test  CommonEventControlManager permission
 * @tc.desc: 1. subcriber permisssion is not empty
 *           2. publish permission is not empty
 *           3. function subcriber CheckPermission return OrderedEventRecord::SKIPPED
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1300,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1300, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("12345");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello world";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("1234");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello world";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::OrderedEventRecord::SKIPPED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1400
 * @tc.name: test  InnerCommonEventManager permission
 * @tc.desc:  function CheckPermission return true
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1400,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1400, TestSize.Level1";

    InnerCommonEventManager inner;
    struct tm curTime;

    Want want;
    want.SetAction("1234");
    CommonEventData data;
    data.SetWant(want);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    publishInfo.SetSticky(true);
    bool ret = inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "hello");
    EXPECT_EQ(true, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1500
 * @tc.name: test  InnerCommonEventManager permission
 * @tc.desc: function CheckPermission return false
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1500,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1500, TestSize.Level1";

    InnerCommonEventManager inner;
    struct tm curTime;

    Want want;
    want.SetAction("1234");
    CommonEventData data;
    data.SetWant(want);
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    publishInfo.SetSticky(true);
    bool ret = inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "hello world");
    EXPECT_EQ(false, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1600
 * @tc.name: test  InnerCommonEventManager permission
 * @tc.desc: function CheckPermission return false
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1600,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1600, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("ohos.permission.GET_WIFI_INFO");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("COMMON_EVENT_WIFI_AP_STA_JOIN");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1700
 * @tc.name: test  InnerCommonEventManager permission
 * @tc.desc: function CheckPermission return false
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1700,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1700, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("1234");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_DISCOVERED");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1800
 * @tc.name: test  InnerCommonEventManager permission
 * @tc.desc: function CheckPermission return false
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1800,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1800, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("1234");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("COMMON_EVENT_DISK_REMOVED");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_1900
 * @tc.name: test  InnerCommonEventManager permission
 * @tc.desc: function CheckPermission return false
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1900,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_1900, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("1234");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("COMMON_EVENT_DISK_REMOVED");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2000
 * @tc.name: test  InnerCommonEventManager permission
 * @tc.desc: function CheckPermission return false   null
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2000,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2000, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("1234");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("COMMON_EVENT_USB_DEVICE_ATTACHED");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2100
 * @tc.name: test  InnerCommonEventManager permission
 * @tc.desc: function CheckPermission return false   not find
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2100, TestSize.Level1";

    CommonEventSubscribeInfo commonEventSubscriberInfo;
    commonEventSubscriberInfo.SetPermission("1234");
    std::shared_ptr<CommonEventSubscribeInfo> commonEventSubscriberInfoSptr =
        std::make_shared<CommonEventSubscribeInfo>(commonEventSubscriberInfo);
    EventSubscriberRecord eventSubRecord;
    eventSubRecord.eventSubscriberInfo = commonEventSubscriberInfoSptr;
    eventSubRecord.bundleName = "hello";

    CommonEventPublishInfo commonEventPublishInfo;
    std::vector<std::string> vecstr;
    vecstr.emplace_back("456");
    commonEventPublishInfo.SetSubscriberPermissions(vecstr);
    std::shared_ptr<CommonEventPublishInfo> commonEventPublishInfoSptr =
        std::make_shared<CommonEventPublishInfo>(commonEventPublishInfo);

    Want want;
    want.SetAction("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    std::shared_ptr<CommonEventData> spCommonEventData = std::make_shared<CommonEventData>();
    spCommonEventData->SetWant(want);
    CommonEventRecord commonEventRecord;
    commonEventRecord.publishInfo = commonEventPublishInfoSptr;
    commonEventRecord.bundleName = "hello";
    commonEventRecord.commonEventData = spCommonEventData;

    CommonEventControlManager commonEventControlManager;
    int ret = commonEventControlManager.CheckPermission(eventSubRecord, commonEventRecord);
    EXPECT_EQ(OrderedEventRecord::DELIVERED, ret);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2200
 * @tc.name: test  CommonEventPermissionManager GetEventPermission
 * @tc.desc: 1. subcriber permisssion is not empty and has one permission
 *           2. function subcriber CheckPermission return name size
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2200, "
                        "TestSize.Level1";

    std::string event = "usual.event.BOOT_COMPLETED";

    int ret = DelayedSingleton<CommonEventPermissionManager>::GetInstance()->GetEventPermission(event).names.size();

    EXPECT_EQ(true, ret > 0);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2200
 * @tc.name: test  CommonEventPermissionManager GetEventPermission
 * @tc.desc: 1. subcriber permisssion is not empty and has two permission and is Logic and
 *           2. function subcriber CheckPermission return name size
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2300,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2300, "
                        "TestSize.Level1";

    std::string event = "usual.event.wifi.p2p.CONN_STATE_CHANGE";

    Permission per = DelayedSingleton<CommonEventPermissionManager>::GetInstance()->GetEventPermission(event);
    int ret = per.names.size();
    int state = per.state;

    EXPECT_EQ(true, ret > 0);
    EXPECT_EQ(true, state == 1);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2200
 * @tc.name: test  CommonEventPermissionManager GetEventPermission
 * @tc.desc: 1. subcriber permisssion is not empty and has two permission and is Logic or
 *           2. function subcriber CheckPermission return name size
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2400,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2400, "
                        "TestSize.Level1";

    std::string event = "usual.event.data.DISK_REMOVED";

    Permission per = DelayedSingleton<CommonEventPermissionManager>::GetInstance()->GetEventPermission(event);
    int ret = per.names.size();
    int state = per.state;

    EXPECT_EQ(true, ret > 0);
    EXPECT_EQ(true, state == 2);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2500
 * @tc.name: test unordered event permission and system send is yes
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has one attribute
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2500,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2500, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    // matchingSkillsObj.AddEvent("usual.event.BOOT_COMPLETED");
    matchingSkillsObj.AddEvent("usual.event.nfc.action.RF_FIELD_ON_DETECTED");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("123");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case1");

    /* Publish */
    Want want;
    // want.SetAction("usual.event.BOOT_COMPLETED");
    want.SetAction("usual.event.nfc.action.RF_FIELD_ON_DETECTED");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(false);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case1");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case1");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2600
 * @tc.name: test unordered event permission and system send is no
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has one attribute
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2600,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2600, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    // matchingSkillsObj.AddEvent("usual.event.USER_SWITCHED");
    matchingSkillsObj.AddEvent("usual.event.wifi.mplink.STATE_CHANGE");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case2");

    /* Publish */
    Want want;
    // want.SetAction("usual.event.USER_SWITCHED");
    want.SetAction("usual.event.wifi.mplink.STATE_CHANGE");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(false);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case2");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case2");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2700
 * @tc.name: test unordered event permission and system send is no
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has two attribute and logic and
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2700,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2700, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    // matchingSkillsObj.AddEvent("usual.event.wifi.p2p.CONN_STATE_CHANGE");
    matchingSkillsObj.AddEvent("usual.event.bluetooth.remotedevice.DISCOVERED");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case3");

    /* Publish */
    Want want;
    // want.SetAction("usual.event.wifi.p2p.CONN_STATE_CHANGE");
    want.SetAction("usual.event.bluetooth.remotedevice.DISCOVERED");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(false);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case3");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case7");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2800
 * @tc.name: test unordered event permission and system send is yes
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has two attribute and logic or
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2800,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2800, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    // matchingSkillsObj.AddEvent("usual.event.data.DISK_REMOVED");
    matchingSkillsObj.AddEvent("usual.event.data.DISK_MOUNTED");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case4");

    /* Publish */
    Want want;
    // want.SetAction("usual.event.data.DISK_REMOVED");
    want.SetAction("usual.event.data.DISK_MOUNTED");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(false);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case4");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case15");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_2900
 * @tc.name: test unordered event permission and system send is yes
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has no attribute
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2900,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_2900, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("common.event.IVI_TEMPERATURE_RECOVERY");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case6");

    /* Publish */
    Want want;
    want.SetAction("common.event.IVI_TEMPERATURE_RECOVERY");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(false);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case6");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case15");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_3000
 * @tc.name: test ordered event permission and system send is yes
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has one attribute
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3000,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3000, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("usual.event.BOOT_COMPLETED");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("123");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case1");

    /* Publish */
    Want want;
    want.SetAction("usual.event.BOOT_COMPLETED");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(true);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, listener, curTime, 0, 1000, "case1");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case1");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_3100
 * @tc.name: test unordered event permission and system send is no
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has one attribute
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3100, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("usual.event.USER_SWITCHED");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case2");

    /* Publish */
    Want want;
    want.SetAction("usual.event.USER_SWITCHED");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(true);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, listener, curTime, 0, 1000, "case2");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case2");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_3200
 * @tc.name: test unordered event permission and system send is no
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has two attribute and logic and
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3200, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("usual.event.wifi.p2p.CONN_STATE_CHANGE");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case3");

    /* Publish */
    Want want;
    want.SetAction("usual.event.wifi.p2p.CONN_STATE_CHANGE");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(true);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, listener, curTime, 0, 1000, "case3");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case7");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_3300
 * @tc.name: test unordered event permission and system send is yes
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has two attribute and logic or
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3300,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3300, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("usual.event.data.DISK_REMOVED");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case4");

    /* Publish */
    Want want;
    want.SetAction("usual.event.data.DISK_REMOVED");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(true);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, listener, curTime, 0, 1000, "case4");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case15");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}

/*
 * @tc.number: CommonEventPublishPermissionEventUnitTest_3400
 * @tc.name: test unordered event permission and system send is yes
 * @tc.desc: 1. subcriber permisssion is empty
 *           2. publish permission is not empty
 *           3. event has no attribute
 */
HWTEST_F(CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3400,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO)
        << "CommonEventPublishPermissionEventUnitTest, CommonEventPublishPermissionEventUnitTest_3400, TestSize.Level1";
    /* subscriber */
    InnerCommonEventManager inner;
    struct tm curTime {
        0
    };
    MatchingSkills matchingSkillsObj;
    matchingSkillsObj.AddEvent("common.event.IVI_TEMPERATURE_RECOVERY");
    CommonEventSubscribeInfo subscriberInfo(matchingSkillsObj);
    subscriberInfo.SetPermission("");
    subscriberInfo.SetPriority(1);
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    auto listener = sptr<IRemoteObject>(new CommonEventListener(subscriber));
    inner.SubscribeCommonEvent(subscriberInfo, listener, curTime, 0, 1000, "case6");

    /* Publish */
    Want want;
    want.SetAction("common.event.IVI_TEMPERATURE_RECOVERY");
    CommonEventData data;
    data.SetWant(want);

    CommonEventPublishInfo publishInfo;
    std::vector<std::string> subscriberPermissions;
    publishInfo.SetOrdered(true);
    subscriberPermissions.emplace_back("456");
    publishInfo.SetSubscriberPermissions(subscriberPermissions);
    bool result = inner.PublishCommonEvent(data, publishInfo, listener, curTime, 0, 1000, "case6");
    // inner.PublishCommonEvent(data, publishInfo, nullptr, curTime, 0, 1000, "case15");
    sleep(SLEEPTIEM);
    EXPECT_EQ(true, result);
    inner.UnsubscribeCommonEvent(listener);
}
