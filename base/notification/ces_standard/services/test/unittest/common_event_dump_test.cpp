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

#define private public
#define protected public
#include "common_event.h"
#include "common_event_manager_service.h"
#include "common_event_manager.h"
#undef private
#undef protected

#include "inner_common_event_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "system_ability_definition.h"
#include "datetime_ex.h"
#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;

namespace {
const std::string EVENT = "com.ces.test.event";
const std::string ENTITY = "com.ces.test.entity";
const std::string SCHEME = "com.ces.test.scheme";
const std::string PERMISSION = "com.ces.test.permission";
const std::string DEVICEDID = "deviceId";
const std::string TYPE = "type";
const std::string BUNDLE = "BundleName";
const std::string ABILITY = "AbilityName";
const int PRIORITY = 1;
const int FLAG = 1;
const int CODE = 1;
const std::string DATA = "DATA";

const std::string EVENT2 = "com.ces.test.event2";
const std::string ENTITY2 = "com.ces.test.entity2";
const std::string SCHEME2 = "com.ces.test.scheme2";
const std::string PERMISSION2 = "com.ces.test.permission2";
const std::string DEVICEDID2 = "deviceId2";
const std::string TYPE2 = "type2";
const std::string BUNDLE2 = "BundleName";
const std::string ABILITY2 = "AbilityName";
const int PRIORITY2 = 2;
const int FLAG2 = 2;
const int CODE2 = 2;
const std::string DATA2 = "DATA2";

const std::string EVENT3 = "com.ces.test.event3";
const int INNITCODE = 0;
const int CHANGECODE = 1;
const int CHANGECODE2 = 2;
const std::string EVENTCASE1 = "com.ces.test.event.case1";
const std::string EVENTCASE2 = "com.ces.test.event.case2";
const std::string EVENTCASE3 = "com.ces.test.event.case3";
const std::string INNITDATA = "com.ces.test.initdata";
const std::string CHANGEDATA = "com.ces.test.changedata";
const std::string CHANGEDATA2 = "com.ces.test.changedata2";
const int PUBLISH_SLEEP = 10;
const uid_t UID = 10;
const uid_t UID2 = 11;
const int STATE_INDEX1 = 1;
const int STATE_INDEX2 = 2;
const int STATE_INDEX3 = 3;
const int DUMP_INFO_COUNT = 2;
const int PUBLISH_COUNT = 60;
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;
OHOS::sptr<OHOS::IRemoteObject> commonEventListener;
OHOS::sptr<OHOS::IRemoteObject> commonEventListener2;
OHOS::sptr<OHOS::IRemoteObject> commonEventListener3;

class CommonEventDumpTest : public testing::Test {
public:
    CommonEventDumpTest()
    {}
    ~CommonEventDumpTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    bool SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber, uid_t callingUid,
        OHOS::sptr<OHOS::IRemoteObject> &commonEventListener);
    bool PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber, OHOS::sptr<OHOS::IRemoteObject> &commonEventListener);
    static bool FinishReceiver(
        const OHOS::sptr<OHOS::IRemoteObject> &proxy, const int &code, const std::string &data, const bool &abortEvent);
    void AsyncProcess();
    std::shared_ptr<InnerCommonEventManager> getInnerCommonEventManager();
    static void dumpInfoCount(const std::vector<std::string> &state, int desSubscribersCount, int desStickyCount,
        int desOrderedCount, int desHistoryCount);
    void SetPublishDataByOrdered(CommonEventData &data, CommonEventPublishInfo &publishInfo);
    void SetPublishDataByOrdered2(CommonEventData &data, CommonEventPublishInfo &publishInfo);
    void SetPublishDataByOrdered3(CommonEventData &data, CommonEventPublishInfo &publishInfo);

private:
    std::shared_ptr<EventRunner> runner_;
    static std::shared_ptr<EventHandler> handler_;
    static std::shared_ptr<InnerCommonEventManager> innerCommonEventManager_;
};

class SubscriberTest : public CommonEventSubscriber {
public:
    SubscriberTest(const CommonEventSubscribeInfo &sp,
        const std::shared_ptr<InnerCommonEventManager> &innerCommonEventManager = nullptr)
        : CommonEventSubscriber(sp)
    {
        innerCommonEventManager_ = innerCommonEventManager;
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
    }

    ~SubscriberTest()
    {}

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            ProcessSubscriberTestCase1(data);
        } else if (action == EVENTCASE2) {
            ProcessSubscriberTestCase2(data);
        } else if (action == EVENTCASE3) {
            GTEST_LOG_(INFO) << "ProcessSubscriberTestCase3:  start";
        } else {
        }
    }

private:
    void ProcessSubscriberTestCase1(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTestCase1:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest::AsyncProcess, this, commonEventListener);
        handler_->PostTask(asyncProcessFunc);
    }
    void ProcessSubscriberTestCase2(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTestCase2:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest::AsyncProcess, this, commonEventListener);
        handler_->PostTask(asyncProcessFunc);
    }

    void AsyncProcess(OHOS::sptr<OHOS::IRemoteObject> commonEventListener)
    {
        bool finishReceiverResult =
            CommonEventDumpTest::FinishReceiver(commonEventListener, CHANGECODE, CHANGEDATA, false);
        EXPECT_EQ(true, finishReceiverResult);
    }

private:
    std::shared_ptr<EventHandler> handler_;
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager_;
};

class SubscriberTest2 : public CommonEventSubscriber {
public:
    SubscriberTest2(const CommonEventSubscribeInfo &sp,
        const std::shared_ptr<InnerCommonEventManager> &innerCommonEventManager = nullptr)
        : CommonEventSubscriber(sp)
    {
        innerCommonEventManager_ = innerCommonEventManager;
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
    };

    ~SubscriberTest2(){

    };

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            ProcessSubscriberTest2Case1(data);
        } else if (action == EVENTCASE2) {
            ProcessSubscriberTest2Case2(data);
        } else if (action == EVENTCASE3) {
            GTEST_LOG_(INFO) << "ProcessSubscriberTest2Case3:  start";
        } else {
        }
    };

private:
    void ProcessSubscriberTest2Case1(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTest2Case1:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        if (innerCommonEventManager_) {
            std::vector<std::string> state;
            innerCommonEventManager_->DumpState("", state);
            CommonEventDumpTest::dumpInfoCount(state, DUMP_INFO_COUNT, DUMP_INFO_COUNT, 1, 0);
        }
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest2::AsyncProcess, this, commonEventListener2);
        handler_->PostTask(asyncProcessFunc);
    }
    void ProcessSubscriberTest2Case2(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTest2Case1:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest2::AsyncProcess, this, commonEventListener2);
        handler_->PostTask(asyncProcessFunc);
    }

    void AsyncProcess(OHOS::sptr<OHOS::IRemoteObject> commonEventListener)
    {
        bool finishReceiverResult =
            CommonEventDumpTest::FinishReceiver(commonEventListener, CHANGECODE2, CHANGEDATA2, false);
        EXPECT_EQ(true, finishReceiverResult);
    }

private:
    std::shared_ptr<EventHandler> handler_;
    std::shared_ptr<InnerCommonEventManager> innerCommonEventManager_;
};

class SubscriberTestLast : public CommonEventSubscriber {
public:
    SubscriberTestLast() : CommonEventSubscriber()
    {
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
    };

    SubscriberTestLast(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
    };

    ~SubscriberTestLast(){

    };

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            ProcessSubscriberTestLastCase1(data);
        } else if (action == EVENTCASE2) {
            ProcessSubscriberTestLastCase2(data);
        } else {
        }
    };

private:
    void ProcessSubscriberTestLastCase1(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTestLastCase1:  start";
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc =
            std::bind(&SubscriberTestLast::AsyncProcess, this, commonEventListener3);
        handler_->PostTask(asyncProcessFunc);
    }
    void ProcessSubscriberTestLastCase2(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "ProcessSubscriberTestLastCase2:  start";
    }

    void AsyncProcess(OHOS::sptr<OHOS::IRemoteObject> commonEventListener)
    {
        bool finishReceiverResult =
            CommonEventDumpTest::FinishReceiver(commonEventListener, CHANGECODE2, CHANGEDATA2, false);
        EXPECT_EQ(true, finishReceiverResult);
    }

private:
    std::shared_ptr<EventHandler> handler_;
};

std::shared_ptr<EventHandler> CommonEventDumpTest::handler_ = nullptr;
std::shared_ptr<InnerCommonEventManager> CommonEventDumpTest::innerCommonEventManager_ = nullptr;

void CommonEventDumpTest::SetUpTestCase(void)
{
    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject, saExtraProp);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStart();
}

void CommonEventDumpTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->OnStop();
}

void CommonEventDumpTest::SetUp(void)
{
    runner_ = EventRunner::Create(true);
    if (!runner_) {
        return;
    }
    handler_ = std::make_shared<EventHandler>(runner_);
    innerCommonEventManager_ = std::make_shared<InnerCommonEventManager>();
}

void CommonEventDumpTest::TearDown(void)
{}

bool CommonEventDumpTest::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber,
    uid_t callingUid, OHOS::sptr<OHOS::IRemoteObject> &commonEventListener)
{
    OHOS::sptr<IEventReceive> listener = new CommonEventListener(subscriber);
    if (!listener) {
        return false;
    }
    commonEventListener = listener->AsObject();
    struct tm recordTime = {0};
    if (!OHOS::GetSystemCurrentTime(&recordTime)) {
        return false;
    }
    pid_t callingPid = 10;

    std::string bundleName = "";
    std::function<void()> SubscribeCommonEventFunc = std::bind(&InnerCommonEventManager::SubscribeCommonEvent,
        innerCommonEventManager_,
        subscriber->GetSubscribeInfo(),
        commonEventListener,
        recordTime,
        callingPid,
        callingUid,
        bundleName);
    return handler_->PostTask(SubscribeCommonEventFunc);
}

bool CommonEventDumpTest::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, OHOS::sptr<OHOS::IRemoteObject> &commonEventListener)
{
    if (commonEventListener == nullptr && publishInfo.IsOrdered()) {
        OHOS::sptr<IEventReceive> listener = new CommonEventListener(subscriber);
        if (!listener) {
            return false;
        }
        commonEventListener = listener->AsObject();
    } else if (!publishInfo.IsOrdered()) {
        commonEventListener = nullptr;
    }

    struct tm recordTime = {0};
    if (!OHOS::GetSystemCurrentTime(&recordTime)) {
        return false;
    }
    pid_t callingPid = 20;
    uid_t callingUid = 21;
    std::string bundleName = "";

    std::function<void()> PublishCommonEventFunc = std::bind(&InnerCommonEventManager::PublishCommonEvent,
        innerCommonEventManager_,
        data,
        publishInfo,
        commonEventListener,
        recordTime,
        callingPid,
        callingUid,
        bundleName);
    return handler_->PostTask(PublishCommonEventFunc);
}

bool CommonEventDumpTest::FinishReceiver(
    const OHOS::sptr<OHOS::IRemoteObject> &proxy, const int &code, const std::string &data, const bool &abortEvent)
{
    std::function<void()> FinishReceiverFunc =
        std::bind(&InnerCommonEventManager::FinishReceiver, innerCommonEventManager_, proxy, code, data, abortEvent);
    return handler_->PostTask(FinishReceiverFunc);
}

std::shared_ptr<InnerCommonEventManager> CommonEventDumpTest::getInnerCommonEventManager()
{
    return innerCommonEventManager_;
}

void CommonEventDumpTest::dumpInfoCount(const std::vector<std::string> &state, int desSubscribersCount,
    int desStickyCount, int desPendingCount, int desHistoryCount)
{
    int subscribersNum = 0;
    int stickyNum = 0;
    int pendingCount = 0;
    int historyCount = 0;
    bool isSubscribers = false;
    bool isSticky = false;
    bool isPending = false;
    bool isHistory = false;
    for (auto vec : state) {
        GTEST_LOG_(INFO) << vec;
        auto pos = vec.find("No information");
        if (pos != string::npos) {
            auto pos2 = vec.find("\tSubscribers:\tNo information");
            if (pos2 != string::npos) {
            } else {
                continue;
            }
        }
        if (isPending == false && isHistory == false) {
            auto pos = vec.find("Subscribers:");
            if (pos != string::npos) {
                isSubscribers = true;
            }
        }
        pos = vec.find("Sticky Events:");
        if (pos != string::npos) {
            isSubscribers = false;
            isSticky = true;
        }
        pos = vec.find("Pending Events:");
        if (pos != string::npos) {
            isSubscribers = false;
            isSticky = false;
            isPending = true;
        }
        pos = vec.find("History Events:");
        if (pos != string::npos) {
            isSubscribers = false;
            isSticky = false;
            isPending = false;
            isHistory = true;
        }
        if (isSubscribers) {
            subscribersNum++;
        } else if (isSticky) {
            stickyNum++;
        } else if (isPending) {
            pendingCount++;
        } else if (isHistory) {
            historyCount++;
        }
    }

    EXPECT_EQ(subscribersNum, desSubscribersCount);
    EXPECT_EQ(stickyNum, desStickyCount);
    EXPECT_EQ(pendingCount, desPendingCount);
    EXPECT_EQ(historyCount, desHistoryCount);
}

void CommonEventDumpTest::SetPublishDataByOrdered(CommonEventData &data, CommonEventPublishInfo &publishInfo)
{
    // make a want
    Want want;
    want.SetAction(EVENTCASE1);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    data.SetWant(want);
    data.SetData(INNITDATA);
    data.SetCode(INNITCODE);

    publishInfo.SetOrdered(true);
}

void CommonEventDumpTest::SetPublishDataByOrdered2(CommonEventData &data, CommonEventPublishInfo &publishInfo)
{
    // make a want
    Want want;
    want.SetAction(EVENTCASE2);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    data.SetWant(want);
    data.SetData(INNITDATA);
    data.SetCode(INNITCODE);

    publishInfo.SetOrdered(true);
}

void CommonEventDumpTest::SetPublishDataByOrdered3(CommonEventData &data, CommonEventPublishInfo &publishInfo)
{
    // make a want
    Want want;
    want.SetAction(EVENTCASE3);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    data.SetWant(want);
    data.SetData(INNITDATA);
    data.SetCode(INNITCODE);

    publishInfo.SetOrdered(false);
}

static void SetMatchingSkills(MatchingSkills &matchingSkills)
{
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddEvent(EVENT2);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddEntity(ENTITY2);
}

static void SetSubscriberInfo1(CommonEventListener *&listener, MatchingSkills &matchingSkills)
{
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(PRIORITY);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    listener = new CommonEventListener(subscriber);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());
}

static void SetSubscriberInfo2(CommonEventListener *&listener2, MatchingSkills &matchingSkills)
{
    CommonEventSubscribeInfo subscriberInfo2(matchingSkills);
    subscriberInfo2.SetPriority(PRIORITY2);
    subscriberInfo2.SetPermission(PERMISSION2);
    subscriberInfo2.SetDeviceId(DEVICEDID2);
    std::shared_ptr<SubscriberTest> subscriber2 = std::make_shared<SubscriberTest>(subscriberInfo2);
    listener2 = new CommonEventListener(subscriber2);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo2, listener2->AsObject());
}

static void Test0100Publish1()
{
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODE);
    data.SetData(DATA);

    // make publish info
    CommonEventPublishInfo publishInfo;

    publishInfo.SetSticky(false);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);
}

static void Test0100Publish2()
{
    Want want2;
    want2.SetAction(EVENT2);
    want2.AddEntity(ENTITY);
    want2.AddEntity(ENTITY2);
    want2.SetType(TYPE2);
    want2.SetFlags(FLAG2);

    OHOS::AppExecFwk::ElementName element2;
    element2.SetBundleName(BUNDLE);
    element2.SetAbilityName(ABILITY);
    element2.SetDeviceID(DEVICEDID);
    want2.SetElement(element2);

    // make common event data
    CommonEventData data2;
    data2.SetWant(want2);
    data2.SetCode(CODE2);
    data2.SetData(DATA2);

    // make publish info
    CommonEventPublishInfo publishInfo2;

    publishInfo2.SetSticky(false);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION2);
    publishInfo2.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data2, publishInfo2, nullptr);
}

static void Test0200Publish1()
{
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);
    data.SetCode(CODE);
    data.SetData(DATA);

    // make publish info
    CommonEventPublishInfo publishInfo;

    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);
}

static void Test0200Publish2()
{
    Want want2;
    want2.SetAction(EVENT2);
    want2.AddEntity(ENTITY);
    want2.AddEntity(ENTITY2);
    want2.SetType(TYPE2);
    want2.SetFlags(FLAG2);

    OHOS::AppExecFwk::ElementName element2;
    element2.SetBundleName(BUNDLE);
    element2.SetAbilityName(ABILITY);
    element2.SetDeviceID(DEVICEDID);
    want2.SetElement(element2);

    // make common event data
    CommonEventData data2;
    data2.SetWant(want2);
    data2.SetCode(CODE2);
    data2.SetData(DATA2);

    // make publish info
    CommonEventPublishInfo publishInfo2;

    publishInfo2.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION2);
    publishInfo2.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data2, publishInfo2, nullptr);
}

/*
 * @tc.number: CommonEventDumpTest_0100
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0100, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;

    SetMatchingSkills(matchingSkills);

    SetSubscriberInfo1(listener, matchingSkills);

    SetSubscriberInfo2(listener2, matchingSkills);

    /* Publish */
    Test0100Publish1();

    Test0100Publish2();

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, DUMP_INFO_COUNT, 0, 0, 2);
    EXPECT_EQ("Sticky Events:\tNo information", state[STATE_INDEX2]);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0200
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0200, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;

    SetMatchingSkills(matchingSkills);

    SetSubscriberInfo1(listener, matchingSkills);

    SetSubscriberInfo2(listener2, matchingSkills);

    /* Publish */
    Test0200Publish1();

    Test0200Publish2();

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, DUMP_INFO_COUNT, DUMP_INFO_COUNT, 0, 4);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0300
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0300, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    // make a want
    Want want2;
    want2.SetAction(EVENT2);
    want2.AddEntity(ENTITY);
    want2.AddEntity(ENTITY2);
    want2.SetType(TYPE2);
    want2.SetFlags(FLAG2);

    OHOS::AppExecFwk::ElementName element2;
    element2.SetBundleName(BUNDLE);
    element2.SetAbilityName(ABILITY);
    element2.SetDeviceID(DEVICEDID);
    want2.SetElement(element2);

    // make common event data
    CommonEventData data2;
    data2.SetWant(want2);

    // make publish info
    CommonEventPublishInfo publishInfo2;
    publishInfo2.SetSticky(true);

    permissions.emplace_back(PERMISSION2);
    publishInfo2.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data2, publishInfo2, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 0, DUMP_INFO_COUNT, 0, 6);
    EXPECT_EQ("Subscribers:\tNo information", state[0]);
}

/*
 * @tc.number: CommonEventDumpTest_0400
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0400, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);
    matchingSkills.AddScheme(SCHEME);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(PRIORITY);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, DUMP_INFO_COUNT, 0, 6);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0500
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info  to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0500, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(PRIORITY);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, DUMP_INFO_COUNT, 0, 6);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0600
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0600, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPermission(PERMISSION);
    subscriberInfo.SetDeviceId(DEVICEDID);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, DUMP_INFO_COUNT, 0, 6);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0700
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0700, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetDeviceId(DEVICEDID);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, DUMP_INFO_COUNT, 0, 6);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0800
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0800, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 1, DUMP_INFO_COUNT, 0, 6);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_0900
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_0900, Function | MediumTest | Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENT);

    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    std::shared_ptr<SubscriberTest> subscriber = std::make_shared<SubscriberTest>(subscriberInfo);
    CommonEventListener *listener = new CommonEventListener(subscriber);
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->SubscribeCommonEvent(
        subscriberInfo, listener->AsObject());

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState(EVENT3, state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 0, 0, 0, 0);
    EXPECT_EQ("Subscribers:\tNo information", state[0]);
    EXPECT_EQ("Sticky Events:\tNo information", state[STATE_INDEX1]);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
}

void Test100Publish1()
{
    Want want;
    want.SetAction(EVENT);
    want.AddEntity(ENTITY);
    want.AddEntity(ENTITY2);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);
}

void Test100Publish2()
{
    Want want2;
    want2.SetAction(EVENT2);
    want2.AddEntity(ENTITY);
    want2.AddEntity(ENTITY2);
    want2.SetType(TYPE2);
    want2.SetFlags(FLAG2);

    OHOS::AppExecFwk::ElementName element2;
    element2.SetBundleName(BUNDLE);
    element2.SetAbilityName(ABILITY);
    element2.SetDeviceID(DEVICEDID);
    want2.SetElement(element2);

    // make common event data
    CommonEventData data2;
    data2.SetWant(want2);

    // make publish info
    CommonEventPublishInfo publishInfo2;

    publishInfo2.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION2);
    publishInfo2.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data2, publishInfo2, nullptr);
}

/*
 * @tc.number: CommonEventDumpTest_1000
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1000, TestSize.Level1)
{
    /* Subscribe */
    MatchingSkills matchingSkills;
    CommonEventListener *listener = nullptr;
    CommonEventListener *listener2 = nullptr;

    SetMatchingSkills(matchingSkills);

    SetSubscriberInfo1(listener, matchingSkills);

    SetSubscriberInfo2(listener2, matchingSkills);

    /* Publish */
    // Publish1
    Test100Publish1();

    // Publish1
    Test100Publish2();

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState(EVENT2, state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, DUMP_INFO_COUNT, 1, 0, 4);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener->AsObject());
    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->UnsubscribeCommonEvent(listener2->AsObject());
}

/*
 * @tc.number: CommonEventDumpTest_1100
 * @tc.name: test dump
 * @tc.desc:  Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1100, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    want.SetType(TYPE);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;

    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 0, DUMP_INFO_COUNT, 0, 9);
}

/*
 * @tc.number: CommonEventDumpTest_1200
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1200, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    want.SetFlags(FLAG);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 0, DUMP_INFO_COUNT, 0, 10);
}

/*
 * @tc.number: CommonEventDumpTest_1300
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1300, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);

    OHOS::AppExecFwk::ElementName element;
    element.SetBundleName(BUNDLE);
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 0, DUMP_INFO_COUNT, 0, 11);
}

/*
 * @tc.number: CommonEventDumpTest_1400
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1400, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    OHOS::AppExecFwk::ElementName element;
    element.SetAbilityName(ABILITY);
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);

    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 0, DUMP_INFO_COUNT, 0, 12);
}

/*
 * @tc.number: CommonEventDumpTest_1500
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1500, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    OHOS::AppExecFwk::ElementName element;
    element.SetDeviceID(DEVICEDID);
    want.SetElement(element);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 0, DUMP_INFO_COUNT, 0, 13);
}

/*
 * @tc.number: CommonEventDumpTest_1600
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1600, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    std::vector<std::string> permissions;
    permissions.emplace_back(PERMISSION);
    publishInfo.SetSubscriberPermissions(permissions);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 0, DUMP_INFO_COUNT, 0, 14);
}

/*
 * @tc.number: CommonEventDumpTest_1700
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1700, Function | MediumTest | Level1)
{
    /* Publish */
    // make a want
    Want want;
    want.SetAction(EVENT);
    // make common event data
    CommonEventData data;
    data.SetWant(want);

    // make publish info
    CommonEventPublishInfo publishInfo;
    publishInfo.SetSticky(true);

    OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->PublishCommonEvent(data, publishInfo, nullptr);

    sleep(1);

    std::vector<std::string> state;
    bool dumpResult = OHOS::DelayedSingleton<CommonEventManagerService>::GetInstance()->DumpState("", state);
    EXPECT_EQ(true, dumpResult);
    GTEST_LOG_(INFO) << "get state size:" << state.size();
    dumpInfoCount(state, 0, DUMP_INFO_COUNT, 0, 15);
}

/*
 * @tc.number: CommonEventDumpTest_1800
 * @tc.name: test dump
 * @tc.desc: The ordered event is empty
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1800, TestSize.Level1)
{
    std::vector<std::string> state;
    CommonEventDumpTest::getInnerCommonEventManager()->DumpState("", state);
    CommonEventDumpTest::dumpInfoCount(state, 0, 2, 0, 0);
    EXPECT_EQ("Pending Events:\tNo information", state[STATE_INDEX3]);
}

/*
 * @tc.number: CommonEventDumpTest_1900
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_1900, TestSize.Level1)
{

    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE1);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddEntity(ENTITY2);
    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(1);
    subscriberInfo.SetDeviceId(DEVICEDID);
    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriberTest =
        std::make_shared<SubscriberTest>(subscriberInfo, getInnerCommonEventManager());
    // subscribe a common event
    bool subscribeResult = SubscribeCommonEvent(subscriberTest, UID, commonEventListener);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE1);
    matchingSkillsAnother.AddEntity(ENTITY);
    matchingSkillsAnother.AddEntity(ENTITY2);
    // make another subcriber info
    CommonEventSubscribeInfo subscriberInfo2(matchingSkillsAnother);
    subscriberInfo2.SetPriority(0);
    subscriberInfo2.SetDeviceId(DEVICEDID2);
    // make another subcriber object
    std::shared_ptr<SubscriberTest2> subscriberTest2 =
        std::make_shared<SubscriberTest2>(subscriberInfo2, getInnerCommonEventManager());

    // subscribe another event
    bool subscribeResult2 = SubscribeCommonEvent(subscriberTest2, UID2, commonEventListener2);
    EXPECT_EQ(true, subscribeResult2);

    std::shared_ptr<SubscriberTestLast> subscriber = std::make_shared<SubscriberTestLast>();

    /* Publish */
    CommonEventData data;
    CommonEventPublishInfo publishInfo;
    SetPublishDataByOrdered(data, publishInfo);

    sleep(PUBLISH_SLEEP);

    // publish order event
    bool publishResult = PublishCommonEvent(data, publishInfo, subscriber, commonEventListener3);
    EXPECT_EQ(true, publishResult);

    sleep(PUBLISH_SLEEP);
}
/*
 * @tc.number: CommonEventDumpTest_2000
 * @tc.name: test dump
 * @tc.desc: Verify after subscribe can get subscriber info to do uid is not right
 */
HWTEST_F(CommonEventDumpTest, CommonEventDumpTest_2000, TestSize.Level1)
{

    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE2);
    matchingSkills.AddEntity(ENTITY);
    matchingSkills.AddEntity(ENTITY2);
    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetPriority(1);
    subscriberInfo.SetDeviceId(DEVICEDID);
    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriberTest =
        std::make_shared<SubscriberTest>(subscriberInfo, getInnerCommonEventManager());
    // subscribe a common event
    bool subscribeResult = SubscribeCommonEvent(subscriberTest, UID, commonEventListener);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE2);
    matchingSkillsAnother.AddEntity(ENTITY);
    matchingSkillsAnother.AddEntity(ENTITY2);
    // make another subcriber info
    CommonEventSubscribeInfo subscriberInfo2(matchingSkillsAnother);
    subscriberInfo2.SetPriority(0);
    subscriberInfo2.SetDeviceId(DEVICEDID2);
    // make another subcriber object
    std::shared_ptr<SubscriberTest2> subscriberTest2 =
        std::make_shared<SubscriberTest2>(subscriberInfo2, getInnerCommonEventManager());

    // subscribe another event
    bool subscribeResult2 = SubscribeCommonEvent(subscriberTest2, UID2, commonEventListener2);
    EXPECT_EQ(true, subscribeResult2);

    std::shared_ptr<SubscriberTestLast> subscriber = std::make_shared<SubscriberTestLast>();

    /* Publish */
    CommonEventData data;
    CommonEventPublishInfo publishInfo;

    sleep(PUBLISH_SLEEP);
    int count = 0;
    while (count < PUBLISH_COUNT) {
        // publish order event
        SetPublishDataByOrdered2(data, publishInfo);
        bool publishResult = PublishCommonEvent(data, publishInfo, subscriber, commonEventListener3);
        EXPECT_EQ(true, publishResult);

        // publish unorder event
        SetPublishDataByOrdered3(data, publishInfo);
        publishResult = PublishCommonEvent(data, publishInfo, subscriber, commonEventListener3);
        EXPECT_EQ(true, publishResult);
        count++;
    }
    sleep(PUBLISH_SLEEP);
    std::vector<std::string> state;
    getInnerCommonEventManager()->DumpState("", state);
    CommonEventDumpTest::dumpInfoCount(state, 4, DUMP_INFO_COUNT, 0, 100);
}