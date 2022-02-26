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
const std::string ACTION = "acion";
std::mutex mtx;
static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;
CommonEventManagerService commonEventManagerService;
InnerCommonEventManager innerCommonEventManager;
std::shared_ptr<CommonEventControlManager> commonEventControlManager = std::make_shared<CommonEventControlManager>();
std::shared_ptr<EventHandler> handler_;
}  // namespace

class CommonEventPublishOrderedEventUnitTest : public testing::Test {
public:
    CommonEventPublishOrderedEventUnitTest()
    {}
    ~CommonEventPublishOrderedEventUnitTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class SubscriberTest : public CommonEventSubscriber {
public:
    explicit SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {}

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        GTEST_LOG_(INFO) << "OnReceiveEvent receive";
        mtx.unlock();
    }
};

class LoggerTest : public OHOS::AppExecFwk::Logger {
public:
    /**
     * Processes the content of a specified string.
     * @param message the content of a specified string.
     */
    void Log(const std::string &line)
    {
        GTEST_LOG_(INFO) << line;
    }
    virtual ~LoggerTest()
    {}
};

void CommonEventPublishOrderedEventUnitTest::SetUpTestCase(void)
{
    handler_ = std::make_shared<EventHandler>(EventRunner::Create(true));
    auto task = []() {
        std::shared_ptr<LoggerTest> logtest = std::make_shared<LoggerTest>();
        EventRunner::GetMainEventRunner()->SetLogger(logtest);
        EventRunner::GetMainEventRunner()->Run();
    };
    handler_->PostTask(task);

    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject, saExtraProp);
}

void CommonEventPublishOrderedEventUnitTest::TearDownTestCase(void)
{
    EventRunner::GetMainEventRunner()->Stop();
}

void CommonEventPublishOrderedEventUnitTest::SetUp(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStart();
}

void CommonEventPublishOrderedEventUnitTest::TearDown(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0100
 * @tc.name: test CommonEventPublishOrderedEventUnitTest
 * @tc.desc: system run normally CaseDescription: Verify PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0100, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish infoPRIORITY
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
        data, publishInfo, nullptr);
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0200
 * @tc.name: test CommonEventPublishOrderedEventUnitTest
 * @tc.desc: Verify PublishCommonEvent false because servie stop
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0200, Function | MediumTest | Level1)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish infoPRIORITY
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(
        data, publishInfo, nullptr);
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0300
 * @tc.name: test FinishReceiver
 * @tc.desc: Verify FinishReceiver return false bacause service stop
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0300, Function | MediumTest | Level1)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *proxy = new CommonEventListener(subscriber);
    bool result = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->FinishReceiver(
        proxy, 0, "receiverdata", true);
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0400
 * @tc.name: test Init
 * @tc.desc: Verify Init fail because innerCommonEventManager_ is null
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0400, Function | MediumTest | Level1)
{
    commonEventManagerService.innerCommonEventManager_ = nullptr;
    OHOS::ErrCode code = commonEventManagerService.Init();
    EXPECT_EQ(OHOS::ERR_INVALID_OPERATION, code);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0500
 * @tc.name: test IsReady
 * @tc.desc: Verify IsReady fail because innerCommonEventManager_ is null
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0500, Function | MediumTest | Level1)
{
    commonEventManagerService.innerCommonEventManager_ = nullptr;
    bool result = commonEventManagerService.IsReady();
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0600
 * @tc.name: test IsReady
 * @tc.desc: Verify IsReady fail because handler_ is null
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0600, Function | MediumTest | Level1)
{
    commonEventManagerService.handler_ = nullptr;
    bool result = commonEventManagerService.IsReady();
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0700
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0700, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    mtx.lock();

    struct tm curTime {
        0
    };
    // publish ordered event
    bool result =
        innerCommonEventManager.PublishCommonEvent(data, publishInfo, commonEventListener, curTime, 0, 0, "bundlename");

    EXPECT_EQ(true, result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0800
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify InnerCommonEventManager PublishCommonEvent fail because data has no aciton
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0800, Function | MediumTest | Level1)
{
    // make common event data
    CommonEventData data;

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make commonEventListener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    struct tm curTime {
        0
    };
    // publish ordered event
    bool result =
        innerCommonEventManager.PublishCommonEvent(data, publishInfo, commonEventListener, curTime, 0, 0, "bundlename");
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_0900
 * @tc.name: test PublishCommonEvent
 * @tc.desc: Verify CommonEventControlManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_0900, Function | MediumTest | Level1)
{
    // make a commonEventRecord
    Want want;
    want.SetAction(ACTION);

    CommonEventData eventRef;
    eventRef.SetWant(want);

    CommonEventPublishInfo publishinfoRef;

    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>(eventRef);
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>(publishinfoRef);
    publishInfo->SetOrdered(true);

    CommonEventRecord commonEventRecord;
    commonEventRecord.commonEventData = commonEventData;
    commonEventRecord.publishInfo = publishInfo;
    commonEventRecord.pid = 0;
    commonEventRecord.uid = 0;
    commonEventRecord.bundleName = "bundleName";
    commonEventRecord.recordTime = {0};

    // make subcriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make subcriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make commone event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);

    mtx.lock();

    bool result = false;
    result = commonEventControlManager->PublishCommonEvent(commonEventRecord, commonEventListenerPtr);
    EXPECT_EQ(true, result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }
    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1000
 * @tc.name: test ProcessOrderedEvent
 * @tc.desc: Verify CommonEventControlManager ProcessOrderedEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1000, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();
    publishInfo->SetOrdered(true);

    CommonEventRecord commonEventRecord;
    commonEventRecord.commonEventData = commonEventData;
    commonEventRecord.publishInfo = publishInfo;
    commonEventRecord.pid = 0;
    commonEventRecord.uid = 0;
    commonEventRecord.bundleName = "bundleName";

    // make subcriber info
    MatchingSkills matchingSkills;
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make subcriber
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    // make commone event listener
    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);
    OHOS::sptr<OHOS::IRemoteObject> commonEventListenerPtr(commonEventListener);
    mtx.lock();
    bool result = commonEventControlManager->ProcessOrderedEvent(commonEventRecord, commonEventListenerPtr);
    EXPECT_EQ(true, result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }
    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1100
 * @tc.name: test EnqueueOrderedRecord eventRecordPtr is null
 * @tc.desc: Verify EnqueueOrderedRecord eventRecordPtr is null orderedEventQueue_ size is 0
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1100, Function | MediumTest | Level1)
{
    bool result = commonEventControlManager->EnqueueOrderedRecord(nullptr);
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1200
 * @tc.name: test EnqueueOrderedRecord
 * @tc.desc: Verify EnqueueOrderedRecord success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1200, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    // make ordered event record
    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 0;

    bool result = commonEventControlManager->EnqueueOrderedRecord(eventRecord);
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1300
 * @tc.name: test ScheduleOrderedCommonEvent
 * @tc.desc: Verify ScheduleOrderedCommonEvent success when scheduled is true
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1300, Function | MediumTest | Level1)
{
    commonEventControlManager->scheduled_ = true;
    bool result = commonEventControlManager->ScheduleOrderedCommonEvent();
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1400
 * @tc.name: test ScheduleOrderedCommonEvent
 * @tc.desc: Verify ScheduleOrderedCommonEvent success when scheduled is false
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1400, Function | MediumTest | Level1)
{
    commonEventControlManager->scheduled_ = false;

    bool result = commonEventControlManager->ScheduleOrderedCommonEvent();
    EXPECT_EQ(true, result);
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1500
 * @tc.name: test FinishReceiver
 * @tc.desc: Verify FinishReceiver return true because eventRecord state is received
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1500, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    // make ordered event record
    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::RECEIVED;
    eventRecord->nextReceiver = 0;

    std::string receiverData = "receiverData";
    bool result = commonEventControlManager->FinishReceiver(eventRecord, 0, receiverData, false);
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1600
 * @tc.name: test FinishReceiver
 * @tc.desc: Verify FinishReceiver return false eventRecord state is idle
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1600, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    // make ordered event record
    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 0;

    std::string receiverData = "receiverData";
    bool result = commonEventControlManager->FinishReceiver(eventRecord, 0, receiverData, false);
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1700
 * @tc.name: test FinishReceiver recordPtr is null
 * @tc.desc: Verify FinishReceiver recordPtr is null return false
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1700, Function | MediumTest | Level1)
{
    std::string receiverData = "receiverData";
    bool result = commonEventControlManager->FinishReceiver(nullptr, 0, receiverData, false);
    EXPECT_EQ(false, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1800
 * @tc.name: test GetOrderedEventHandler handlerOrdered_ is not null
 * @tc.desc: Verify GetOrderedEventHandler handlerOrdered_ is not return true
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1800, Function | MediumTest | Level1)
{
    commonEventControlManager->handlerOrdered_ =
        std::make_shared<OrderedEventHandler>(EventRunner::Create(), commonEventControlManager);
    bool result = commonEventControlManager->GetOrderedEventHandler();
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_1900
 * @tc.name: test CurrentOrderedEventTimeout orderedEventQueue_ is null
 * @tc.desc: Verify CurrentOrderedEventTimeout fail because orderedEventQueue_ is null
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_1900, Function | MediumTest | Level1)
{
    commonEventControlManager->orderedEventQueue_.clear();
    commonEventControlManager->CurrentOrderedEventTimeout(true);

    bool result = false;
    if (commonEventControlManager->orderedEventQueue_.size() == 0) {
        result = true;
    }
    EXPECT_EQ(true, result);
    commonEventControlManager->orderedEventQueue_.clear();
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2000
 * @tc.name: test CurrentOrderedEventTimeout
 * @tc.desc: Verify CurrentOrderedEventTimeout success with eventRecord->nextReceiver = 0;
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2000, Function | MediumTest | Level1)
{
    // make common event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    // make ordered event record
    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 0;

    // enqueue ordered record
    commonEventControlManager->scheduled_ = true;
    commonEventControlManager->EnqueueOrderedRecord(eventRecord);
    commonEventControlManager->CurrentOrderedEventTimeout(true);

    bool result = false;
    if (commonEventControlManager->orderedEventQueue_.size() > 0) {
        result = true;
    }
    EXPECT_EQ(true, result);
    commonEventControlManager->orderedEventQueue_.clear();
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2100
 * @tc.name: test CurrentOrderedEventTimeout
 * @tc.desc: Verify CurrentOrderedEventTimeout success with eventRecord->nextReceiver = 1;
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2100, Function | MediumTest | Level1)
{
    // make event record
    std::shared_ptr<CommonEventData> commonEventData = std::make_shared<CommonEventData>();
    std::shared_ptr<CommonEventPublishInfo> publishInfo = std::make_shared<CommonEventPublishInfo>();

    std::shared_ptr<EventSubscriberRecord> subscriberRecord = std::make_shared<EventSubscriberRecord>();

    std::shared_ptr<OrderedEventRecord> eventRecord = std::make_shared<OrderedEventRecord>();
    eventRecord->commonEventData = commonEventData;
    eventRecord->publishInfo = publishInfo;
    eventRecord->resultTo = nullptr;
    eventRecord->state = OrderedEventRecord::IDLE;
    eventRecord->nextReceiver = 1;
    eventRecord->deliveryState.emplace_back(OrderedEventRecord::PENDING);
    eventRecord->receivers.emplace_back(subscriberRecord);

    commonEventControlManager->scheduled_ = true;
    bool ret = commonEventControlManager->EnqueueOrderedRecord(eventRecord);
    EXPECT_EQ(true, ret);
    commonEventControlManager->CurrentOrderedEventTimeout(true);

    bool result = false;
    if (commonEventControlManager->orderedEventQueue_.front()->nextReceiver > 0) {
        GTEST_LOG_(INFO) << std::to_string(commonEventControlManager->orderedEventQueue_.front()->nextReceiver);
        result = true;
    }
    EXPECT_EQ(true, result);
    commonEventControlManager->orderedEventQueue_.clear();
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2200
 * @tc.name: test CancelTimeout
 * @tc.desc: Verify CancelTimeout return true when pendingTimeoutMessage is true
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2200, Function | MediumTest | Level1)
{
    commonEventControlManager->pendingTimeoutMessage_ = true;
    bool result = commonEventControlManager->CancelTimeout();
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2300
 * @tc.name: test CancelTimeout
 * @tc.desc: Verify CancelTimeout return true when pendingTimeoutMessage is false
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2300, Function | MediumTest | Level1)
{
    bool result = false;
    commonEventControlManager->pendingTimeoutMessage_ = false;
    result = commonEventControlManager->CancelTimeout();
    EXPECT_EQ(true, result);
}

/*
 * @tc.number: CommonEventPublishOrderedUnitTest_2400
 * @tc.name: test PublishCommonEvent
 * @tc.desc: 1.Set thread mode handler
 *           2.Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2400, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::HANDLER);

    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    mtx.lock();

    struct tm curTime {
        0
    };
    // publish ordered event
    bool result =
        innerCommonEventManager.PublishCommonEvent(data, publishInfo, commonEventListener, curTime, 0, 0, "bundlename");

    EXPECT_EQ(true, result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedEventUnitTest_2500
 * @tc.name: test PublishCommonEvent
 * @tc.desc: 1.Set thread mode POST
 *           2.Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2500, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::POST);

    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    mtx.lock();

    struct tm curTime {
        0
    };
    // publish ordered event
    bool result =
        innerCommonEventManager.PublishCommonEvent(data, publishInfo, commonEventListener, curTime, 0, 0, "bundlename");

    EXPECT_EQ(true, result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedEventUnitTest_2600
 * @tc.name: test PublishCommonEvent
 * @tc.desc: 1.Set thread mode ASYNC
 *           2.Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2600, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::ASYNC);

    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    mtx.lock();

    struct tm curTime {
        0
    };
    // publish ordered event
    bool result =
        innerCommonEventManager.PublishCommonEvent(data, publishInfo, commonEventListener, curTime, 0, 0, "bundlename");

    EXPECT_EQ(true, result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}

/*
 * @tc.number: CommonEventPublishOrderedEventUnitTest_2700
 * @tc.name: test PublishCommonEvent
 * @tc.desc: 1.Set thread mode BACKGROUND
 *           2.Verify InnerCommonEventManager PublishCommonEvent success
 */
HWTEST_F(CommonEventPublishOrderedEventUnitTest, CommonEventPublishOrderedUnitTest_2700, Function | MediumTest | Level1)
{
    // make a want
    Want want;
    want.SetAction(ACTION);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);

    MatchingSkills matchingSkills;

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(CommonEventSubscribeInfo::ThreadMode::BACKGROUND);

    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);

    OHOS::sptr<CommonEventListener> commonEventListener = new CommonEventListener(subscriber);

    mtx.lock();

    struct tm curTime {
        0
    };
    // publish ordered event
    bool result =
        innerCommonEventManager.PublishCommonEvent(data, publishInfo, commonEventListener, curTime, 0, 0, "bundlename");

    EXPECT_EQ(true, result);

    int count = 0;
    while (!mtx.try_lock()) {
        if (count == 0) {
            GTEST_LOG_(INFO) << "Wait OnReceive callback function process";
            count = 1;
        } else {
            usleep(100 * 1000);
        }
    }

    mtx.unlock();
    GTEST_LOG_(INFO) << "Testcase finished";
}
