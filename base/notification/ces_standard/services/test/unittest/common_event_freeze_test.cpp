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
#define UNIT_TEST
#include "system_ability_definition.h"
#include "mock_bundle_manager.h"
#include "iservice_registry.h"
#include "inner_common_event_manager.h"
#include "common_event_sticky_manager.h"
#include "errors.h"
#define private public
#define protected public
#include "common_event.h"
#include "common_event_manager_service.h"
#include "common_event_manager.h"
#undef private
#undef protected
#include "datetime_ex.h"

using namespace testing::ext;
using namespace OHOS::EventFwk;
namespace {
const std::string EVENTCASE1 = "com.ces.test.event.case1";
const std::string EVENTCASE2 = "com.ces.test.event.case2";
const std::string EVENTCASE3 = "com.ces.test.event.case3";
const std::string EVENTCASE4 = "com.ces.test.event.case4";
const std::string EVENTCASE5 = "com.ces.test.event.case5";
const std::string EVENTCASE6 = "com.ces.test.event.case6";
const std::string EVENTCASE7 = "com.ces.test.event.case7";
const std::string INNITDATA = "com.ces.test.initdata";
const std::string CHANGEDATA = "com.ces.test.changedata";
const std::string CHANGEDATA2 = "com.ces.test.changedata2";
const int INNITCODE = 0;
const int CHANGECODE = 1;
const int CHANGECODE2 = 2;
const uid_t UID = 1;
const uid_t UID2 = 2;
const int PUBLISH_SLEEP = 1;
const int FREEZE_SLEEP = 10;
const int FREEZE_SLEEP2 = 120;
bool isFreeze_uid = false;
bool isFreeze_uid2 = false;
std::mutex mtx;
}  // namespace

static OHOS::sptr<OHOS::IRemoteObject> bundleObject = nullptr;
OHOS::sptr<OHOS::IRemoteObject> commonEventListener;
OHOS::sptr<OHOS::IRemoteObject> commonEventListener2;
OHOS::sptr<OHOS::IRemoteObject> commonEventListener3;
class CommonEventFreezeTest : public testing::Test {
public:
    CommonEventFreezeTest()
    {}
    ~CommonEventFreezeTest()
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
    bool Freeze(const uid_t &uid);
    bool Unfreeze(const uid_t &uid);
    void AsyncProcess();

private:
    std::shared_ptr<EventRunner> runner_;
    static std::shared_ptr<EventHandler> handler_;
    static std::shared_ptr<InnerCommonEventManager> innerCommonEventManager_;
};

class SubscriberTest : public CommonEventSubscriber {
public:
    SubscriberTest(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
        handler_ = std::make_shared<EventHandler>(EventRunner::Create());
    };

    ~SubscriberTest(){

    };

    virtual void OnReceiveEvent(const CommonEventData &data)
    {
        std::string action = data.GetWant().GetAction();
        if (action == EVENTCASE1) {
            ProcessSubscriberTestCase1(data);
        } else if (action == EVENTCASE2) {
            ProcessSubscriberTestCase2(data);
        } else {
        }
    };

private:
    void ProcessSubscriberTestCase1(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "Subscriber1: Type:  " << data.GetWant().GetType();
        if (!IsOrderedCommonEvent()) {
            return;
        }
        EXPECT_EQ(INNITCODE, data.GetCode());
        EXPECT_EQ(INNITDATA, data.GetData());
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest::AsyncProcess, this, commonEventListener);
        handler_->PostTask(asyncProcessFunc);
    }
    void ProcessSubscriberTestCase2(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "Subscriber1: Type:  " << data.GetWant().GetType();
        EXPECT_EQ(INNITCODE, data.GetCode());
        EXPECT_EQ(INNITDATA, data.GetData());
    }

    void AsyncProcess(OHOS::sptr<OHOS::IRemoteObject> commonEventListener)
    {
        bool finishReceiverResult =
            CommonEventFreezeTest::FinishReceiver(commonEventListener, CHANGECODE, CHANGEDATA, false);
        EXPECT_EQ(true, finishReceiverResult);
    }

private:
    std::shared_ptr<EventHandler> handler_;
};

class SubscriberTest2 : public CommonEventSubscriber {
public:
    SubscriberTest2(const CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp)
    {
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
        } else {
        }
    };

private:
    void ProcessSubscriberTest2Case1(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "Subscriber2: Type:  " << data.GetWant().GetType();
        if (!IsOrderedCommonEvent()) {
            return;
        }
        if (!isFreeze_uid) {
            EXPECT_EQ(CHANGECODE, data.GetCode());
            EXPECT_EQ(CHANGEDATA, data.GetData());
        } else {
            EXPECT_EQ(INNITCODE, data.GetCode());
            EXPECT_EQ(INNITDATA, data.GetData());
        }
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc = std::bind(&SubscriberTest2::AsyncProcess, this, commonEventListener2);
        handler_->PostTask(asyncProcessFunc);
    }
    void ProcessSubscriberTest2Case2(CommonEventData data)
    {
        GTEST_LOG_(INFO) << "Subscriber2: Type:  " << data.GetWant().GetType();
        EXPECT_EQ(INNITCODE, data.GetCode());
        EXPECT_EQ(INNITDATA, data.GetData());
    }

    void AsyncProcess(OHOS::sptr<OHOS::IRemoteObject> commonEventListener)
    {
        bool finishReceiverResult =
            CommonEventFreezeTest::FinishReceiver(commonEventListener, CHANGECODE2, CHANGEDATA2, false);
        EXPECT_EQ(true, finishReceiverResult);
    }

private:
    std::shared_ptr<EventHandler> handler_;
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
        GTEST_LOG_(INFO) << "SubscriberLast: Type:  " << data.GetWant().GetType();
        if (!isFreeze_uid2) {
            EXPECT_EQ(CHANGECODE2, data.GetCode());
            EXPECT_EQ(CHANGEDATA2, data.GetData());
        } else {
            if (!isFreeze_uid) {
                EXPECT_EQ(CHANGECODE, data.GetCode());
                EXPECT_EQ(CHANGEDATA, data.GetData());
            } else {
                EXPECT_EQ(INNITCODE, data.GetCode());
                EXPECT_EQ(INNITDATA, data.GetData());
            }
        }
        std::shared_ptr<AsyncCommonEventResult> result = GoAsyncCommonEvent();
        std::function<void()> asyncProcessFunc =
            std::bind(&SubscriberTestLast::AsyncProcess, this, commonEventListener3);
        handler_->PostTask(asyncProcessFunc);
    }
    void ProcessSubscriberTestLastCase2(CommonEventData data)
    {}

    void AsyncProcess(OHOS::sptr<OHOS::IRemoteObject> commonEventListener)
    {
        bool finishReceiverResult =
            CommonEventFreezeTest::FinishReceiver(commonEventListener, CHANGECODE2, CHANGEDATA2, false);
        EXPECT_EQ(true, finishReceiverResult);
    }

private:
    std::shared_ptr<EventHandler> handler_;
};

std::shared_ptr<EventHandler> CommonEventFreezeTest::handler_ = nullptr;
std::shared_ptr<InnerCommonEventManager> CommonEventFreezeTest::innerCommonEventManager_ = nullptr;

void CommonEventFreezeTest::SetUpTestCase(void)
{
    bundleObject = new OHOS::AppExecFwk::MockBundleMgrService();
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject, saExtraProp);
}

void CommonEventFreezeTest::TearDownTestCase(void)
{}

void CommonEventFreezeTest::SetUp(void)
{
    runner_ = EventRunner::Create(true);
    if (!runner_) {
        return;
    }
    handler_ = std::make_shared<EventHandler>(runner_);
    innerCommonEventManager_ = std::make_shared<InnerCommonEventManager>();
}

void CommonEventFreezeTest::TearDown(void)
{}

bool CommonEventFreezeTest::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber,
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
    pid_t callingPid = 0;

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

bool CommonEventFreezeTest::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
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
    pid_t callingPid = 0;
    uid_t callingUid = 0;
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

bool CommonEventFreezeTest::FinishReceiver(
    const OHOS::sptr<OHOS::IRemoteObject> &proxy, const int &code, const std::string &data, const bool &abortEvent)
{
    std::function<void()> FinishReceiverFunc =
        std::bind(&InnerCommonEventManager::FinishReceiver, innerCommonEventManager_, proxy, code, data, abortEvent);
    return handler_->PostTask(FinishReceiverFunc);
}

bool CommonEventFreezeTest::Freeze(const uid_t &uid)
{
    std::function<void()> FreezeFunc = std::bind(&InnerCommonEventManager::Freeze, innerCommonEventManager_, uid);
    return handler_->PostImmediateTask(FreezeFunc);
}

bool CommonEventFreezeTest::Unfreeze(const uid_t &uid)
{
    std::function<void()> UnfreezeFunc = std::bind(&InnerCommonEventManager::Unfreeze, innerCommonEventManager_, uid);
    return handler_->PostImmediateTask(UnfreezeFunc);
}

void CommonEventFreezeTest::AsyncProcess()
{
    isFreeze_uid = false;
    isFreeze_uid2 = false;
    sleep(FREEZE_SLEEP);
    GTEST_LOG_(INFO) << "Subscriber1 Freeze";
    Freeze(UID);
    GTEST_LOG_(INFO) << "Subscriber2 Freeze";
    Freeze(UID2);
    isFreeze_uid = true;
    isFreeze_uid2 = true;
    sleep(FREEZE_SLEEP);
    GTEST_LOG_(INFO) << "Subscriber1 Unfreeze";
    Unfreeze(UID);
    isFreeze_uid = false;
    sleep(FREEZE_SLEEP2);
    GTEST_LOG_(INFO) << "Subscriber2 Unfreeze";
    Unfreeze(UID2);
    isFreeze_uid2 = false;
    sleep(FREEZE_SLEEP);
    mtx.unlock();
}

HWTEST_F(CommonEventFreezeTest, CommonEventFreezeTest_001, TestSize.Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE1);

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscriberInfo);

    // subscribe a common event
    bool subscribeResult = SubscribeCommonEvent(subscriberTest, UID, commonEventListener);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE1);

    // make another subcriber info
    CommonEventSubscribeInfo subscriberInfo2(matchingSkillsAnother);

    // make another subcriber object
    std::shared_ptr<SubscriberTest2> subscriberTest2 = std::make_shared<SubscriberTest2>(subscriberInfo2);

    // subscribe another event
    bool subscribeResult2 = SubscribeCommonEvent(subscriberTest2, UID2, commonEventListener2);
    EXPECT_EQ(true, subscribeResult2);

    mtx.lock();
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    std::function<void()> asyncProcessFunc = std::bind(&CommonEventFreezeTest::AsyncProcess, this);
    handler->PostTask(asyncProcessFunc);

    std::shared_ptr<SubscriberTestLast> subscriber = std::make_shared<SubscriberTestLast>();
    /* Publish */
    int i = 0;
    while (!mtx.try_lock()) {
        // make a want
        Want want;
        want.SetAction(EVENTCASE1);
        i++;
        want.SetType(std::to_string(i));
        GTEST_LOG_(INFO) << "PublishCommonEvent: Type:  " << std::to_string(i);
        // make common event data
        CommonEventData data;
        data.SetWant(want);
        data.SetData(INNITDATA);
        data.SetCode(INNITCODE);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(true);

        sleep(PUBLISH_SLEEP);

        // publish order event
        bool publishResult = PublishCommonEvent(data, publishInfo, subscriber, commonEventListener3);
        EXPECT_EQ(true, publishResult);
    }

    sleep(PUBLISH_SLEEP);
    mtx.unlock();
}

HWTEST_F(CommonEventFreezeTest, CommonEventFreezeTest_002, TestSize.Level1)
{
    /* Subscribe */
    // make matching skills
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EVENTCASE2);

    // make subcriber info
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);

    // make a subcriber object
    std::shared_ptr<SubscriberTest> subscriberTest = std::make_shared<SubscriberTest>(subscriberInfo);

    // subscribe a common event
    bool subscribeResult = SubscribeCommonEvent(subscriberTest, UID, commonEventListener);
    EXPECT_EQ(true, subscribeResult);

    // make another matching skills
    MatchingSkills matchingSkillsAnother;
    matchingSkillsAnother.AddEvent(EVENTCASE2);

    // make another subcriber info
    CommonEventSubscribeInfo subscriberInfo2(matchingSkillsAnother);

    // make another subcriber object
    std::shared_ptr<SubscriberTest2> subscriberTest2 = std::make_shared<SubscriberTest2>(subscriberInfo2);

    // subscribe another event
    bool subscribeResult2 = SubscribeCommonEvent(subscriberTest2, UID2, commonEventListener2);
    EXPECT_EQ(true, subscribeResult2);

    mtx.lock();
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    std::function<void()> asyncProcessFunc = std::bind(&CommonEventFreezeTest::AsyncProcess, this);
    handler->PostTask(asyncProcessFunc);

    std::shared_ptr<SubscriberTestLast> subscriber = nullptr;
    /* Publish */
    int i = 0;
    while (!mtx.try_lock()) {
        // make a want
        Want want;
        want.SetAction(EVENTCASE2);
        i++;
        want.SetType(std::to_string(i));
        GTEST_LOG_(INFO) << "PublishCommonEvent: Type:  " << std::to_string(i);
        // make common event data
        CommonEventData data;
        data.SetWant(want);
        data.SetData(INNITDATA);
        data.SetCode(INNITCODE);
        CommonEventPublishInfo publishInfo;
        sleep(PUBLISH_SLEEP);

        // publish order event
        bool publishResult = PublishCommonEvent(data, publishInfo, subscriber, commonEventListener3);
        EXPECT_EQ(true, publishResult);
    }

    sleep(PUBLISH_SLEEP);
    mtx.unlock();
}
