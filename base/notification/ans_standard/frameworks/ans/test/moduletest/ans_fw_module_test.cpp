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
#include <condition_variable>
#include <functional>
#include <gtest/gtest.h>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "advanced_notification_service.h"
#include "ans_const_define.h"
#include "ans_inner_errors.h"
#include "ans_manager_proxy.h"
#include "ans_mt_constant.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "notification_content.h"
#include "notification_helper.h"
#include "notification_long_text_content.h"
#include "notification_subscriber.h"
#include "system_ability_definition.h"

using namespace testing::ext;
using namespace OHOS::Media;

namespace OHOS {
namespace Notification {

enum class SubscriberEventType {
    ON_SUBSCRIBERESULT,
    ON_UNSUBSCRIBERESULT,
    ON_DIED,
    ON_UPDATE,
    ON_DISTURBMODECHANGED,
    ON_CANCELED,
    ON_CANCELED_WITH_SORTINGMAP_AND_DELETEREASON,
    ON_CONSUMED,
    ON_CONSUMED_WITH_SORTINGMAP,
    UNKNOWN,
};

class SubscriberEvent {
public:
    virtual ~SubscriberEvent()
    {}
    SubscriberEventType GetType()
    {
        return type_;
    }

protected:
    SubscriberEvent(SubscriberEventType type) : type_(type)
    {}

    SubscriberEventType type_;
};

class OnSubscribeResultEvent : public SubscriberEvent {
public:
    ~OnSubscribeResultEvent() override
    {}
    OnSubscribeResultEvent(NotificationConstant::SubscribeResult result)
        : SubscriberEvent(SubscriberEventType::ON_SUBSCRIBERESULT), subscribeResult_(result)
    {}
    NotificationConstant::SubscribeResult GetSubscribeResult()
    {
        return subscribeResult_;
    }

private:
    NotificationConstant::SubscribeResult subscribeResult_;
};

class OnUnSubscribeResultEvent : public SubscriberEvent {
public:
    OnUnSubscribeResultEvent(NotificationConstant::SubscribeResult result)
        : SubscriberEvent(SubscriberEventType::ON_UNSUBSCRIBERESULT), unSubscribeResult_(result)
    {}

    ~OnUnSubscribeResultEvent() override
    {}

    NotificationConstant::SubscribeResult GetUnSubscribeResult()
    {
        return unSubscribeResult_;
    }

private:
    NotificationConstant::SubscribeResult unSubscribeResult_;
};

class OnDiedEvent : public SubscriberEvent {
public:
    OnDiedEvent() : SubscriberEvent(SubscriberEventType::ON_DIED)
    {}

    ~OnDiedEvent() override
    {}
};

class OnUpdatedEvent : public SubscriberEvent {
public:
    OnUpdatedEvent(const std::shared_ptr<NotificationSortingMap> &sortingMap)
        : SubscriberEvent(SubscriberEventType::ON_UPDATE), sortingMap_(sortingMap)
    {}

    ~OnUpdatedEvent() override
    {}

    std::shared_ptr<NotificationSortingMap> GetNotificationSortingMap()
    {
        return sortingMap_;
    }

private:
    std::shared_ptr<NotificationSortingMap> sortingMap_;
};

class OnDisturbModeChangedEvent : public SubscriberEvent {
public:
    OnDisturbModeChangedEvent(int disturbMode)
        : SubscriberEvent(SubscriberEventType::ON_DISTURBMODECHANGED), disturbMode_(disturbMode)
    {}

    ~OnDisturbModeChangedEvent() override
    {}

    int GetDisturbModeChanged()
    {
        return disturbMode_;
    }

private:
    int disturbMode_;
};

class OnOnCanceledEvent : public SubscriberEvent {
public:
    OnOnCanceledEvent(const std::shared_ptr<Notification> &request)
        : SubscriberEvent(SubscriberEventType::ON_CANCELED), request_(request)
    {}

    ~OnOnCanceledEvent() override
    {}

    std::shared_ptr<Notification> GetRequest()
    {
        return request_;
    }

private:
    std::shared_ptr<Notification> request_;
};

class OnOnCanceledWithSortingMapAndDeleteReasonEvent : public SubscriberEvent {
public:
    OnOnCanceledWithSortingMapAndDeleteReasonEvent(const std::shared_ptr<Notification> &request,
        const std::shared_ptr<NotificationSortingMap> &sortingMap, int deleteReason)
        : SubscriberEvent(SubscriberEventType::ON_CANCELED_WITH_SORTINGMAP_AND_DELETEREASON),
          request_(request),
          sortingMap_(sortingMap),
          deleteReason_(deleteReason)
    {}

    ~OnOnCanceledWithSortingMapAndDeleteReasonEvent() override
    {}

    std::shared_ptr<Notification> GetRequest()
    {
        return request_;
    }
    std::shared_ptr<NotificationSortingMap> GetSortingMap()
    {
        return sortingMap_;
    }
    int GetDeleteReason()
    {
        return deleteReason_;
    }

private:
    std::shared_ptr<Notification> request_;
    std::shared_ptr<NotificationSortingMap> sortingMap_;
    int deleteReason_;
};

class OnConsumedEvent : public SubscriberEvent {
public:
    OnConsumedEvent(const std::shared_ptr<Notification> &request)
        : SubscriberEvent(SubscriberEventType::ON_CONSUMED), request_(request)
    {}

    ~OnConsumedEvent() override
    {}

    std::shared_ptr<Notification> GetRequest()
    {
        return request_;
    }

private:
    std::shared_ptr<Notification> request_;
};

class OnConsumedWithSortingMapEvent : public SubscriberEvent {
public:
    OnConsumedWithSortingMapEvent(
        const std::shared_ptr<Notification> &request, const std::shared_ptr<NotificationSortingMap> &sortingMap)
        : SubscriberEvent(SubscriberEventType::ON_CONSUMED_WITH_SORTINGMAP), request_(request), sortingMap_(sortingMap)
    {
        type_ = SubscriberEventType::ON_CONSUMED_WITH_SORTINGMAP;
    }

    ~OnConsumedWithSortingMapEvent() override
    {}

    std::shared_ptr<Notification> GetRequest()
    {
        return request_;
    }

    std::shared_ptr<NotificationSortingMap> GetSortingMap()
    {
        return sortingMap_;
    }

private:
    std::shared_ptr<Notification> request_;
    std::shared_ptr<NotificationSortingMap> sortingMap_;
};

class TestAnsSubscriber : public NotificationSubscriber {
public:
    void OnSubscribeResult(NotificationConstant::SubscribeResult result) override
    {
        std::shared_ptr<OnSubscribeResultEvent> event = std::make_shared<OnSubscribeResultEvent>(result);
        std::unique_lock<std::mutex> lck(mtx_);
        events_.push_back(event);
    }
    void OnUnsubscribeResult(NotificationConstant::SubscribeResult result) override
    {
        std::shared_ptr<OnUnSubscribeResultEvent> event = std::make_shared<OnUnSubscribeResultEvent>(result);
        std::unique_lock<std::mutex> lck(mtx_);
        events_.push_back(event);
    }
    void OnDied() override
    {}
    void OnUpdate(const std::shared_ptr<NotificationSortingMap> &sortingMap) override
    {
        std::shared_ptr<OnUpdatedEvent> event = std::make_shared<OnUpdatedEvent>(sortingMap);
        std::unique_lock<std::mutex> lck(mtx_);
        events_.push_back(event);
    }
    void OnDisturbModeChanged(int disturbMode) override
    {
        std::shared_ptr<OnDisturbModeChangedEvent> event = std::make_shared<OnDisturbModeChangedEvent>(disturbMode);
        std::unique_lock<std::mutex> lck(mtx_);
        events_.push_back(event);
    }
    void OnCanceled(const std::shared_ptr<Notification> &request) override
    {
        std::shared_ptr<OnOnCanceledEvent> event = std::make_shared<OnOnCanceledEvent>(request);
        std::unique_lock<std::mutex> lck(mtx_);
        events_.push_back(event);
    }
    void OnCanceled(const std::shared_ptr<Notification> &request,
        const std::shared_ptr<NotificationSortingMap> &sortingMap, int deleteReason) override
    {
        std::shared_ptr<OnOnCanceledWithSortingMapAndDeleteReasonEvent> event =
            std::make_shared<OnOnCanceledWithSortingMapAndDeleteReasonEvent>(request, sortingMap, deleteReason);
        std::unique_lock<std::mutex> lck(mtx_);
        events_.push_back(event);
    }
    void OnConsumed(const std::shared_ptr<Notification> &request) override
    {
        std::shared_ptr<OnConsumedEvent> event = std::make_shared<OnConsumedEvent>(request);
        std::unique_lock<std::mutex> lck(mtx_);
        events_.push_back(event);
    }

    void OnConsumed(const std::shared_ptr<Notification> &request,
        const std::shared_ptr<NotificationSortingMap> &sortingMap) override
    {
        std::shared_ptr<OnConsumedWithSortingMapEvent> event =
            std::make_shared<OnConsumedWithSortingMapEvent>(request, sortingMap);
        std::unique_lock<std::mutex> lck(mtx_);
        events_.push_back(event);
    }

    std::list<std::shared_ptr<SubscriberEvent>> GetEvents()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        return events_;
    }

    void ClearEvents()
    {
        std::unique_lock<std::mutex> lck(mtx_);
        events_.clear();
    }

private:
    std::mutex mtx_;
    std::list<std::shared_ptr<SubscriberEvent>> events_;
};

class AnsFWModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    inline void SleepForFC()
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
};

class EventParser {
public:
    EventParser()
    {
        waitOnSubscriber_ = 0;
        waitOnUnSubscriber_ = 0;
        waitOnConsumed_ = false;
        onConsumedReq_.clear();
        onConsumedWithSortingMapReq_.clear();
        onConsumedWithSortingMapSor_.clear();
        waitOnConsumedWithSortingMap_ = false;
        waitOnCanceled_ = false;
        onCanceledReq_.clear();
        onCanceledWithSortingMapReq_.clear();
        onCanceledWithSortingMapSor_.clear();
        onCanceledWithSortingMapDelRea_.clear();
        waitOnCanceledWithSortingMapAndDeleteReason_ = false;
    }

    ~EventParser()
    {}

    void parse(std::list<std::shared_ptr<SubscriberEvent>> events)
    {
        for (auto event : events) {
            if (event->GetType() == SubscriberEventType::ON_SUBSCRIBERESULT) {
                std::shared_ptr<OnSubscribeResultEvent> ev = std::static_pointer_cast<OnSubscribeResultEvent>(event);
                waitOnSubscriber_ = ev->GetSubscribeResult();
            } else if (event->GetType() == SubscriberEventType::ON_CONSUMED) {
                std::shared_ptr<OnConsumedEvent> ev = std::static_pointer_cast<OnConsumedEvent>(event);
                waitOnConsumed_ = true;
                onConsumedReq_.push_back(ev->GetRequest());
            } else if (event->GetType() == SubscriberEventType::ON_CONSUMED_WITH_SORTINGMAP) {
                std::shared_ptr<OnConsumedWithSortingMapEvent> ev =
                    std::static_pointer_cast<OnConsumedWithSortingMapEvent>(event);
                waitOnConsumedWithSortingMap_ = true;
                onConsumedWithSortingMapReq_.push_back(ev->GetRequest());
                onConsumedWithSortingMapSor_.push_back(ev->GetSortingMap());
            } else if (event->GetType() == SubscriberEventType::ON_CANCELED) {
                std::shared_ptr<OnOnCanceledEvent> ev = std::static_pointer_cast<OnOnCanceledEvent>(event);
                waitOnCanceled_ = true;
                onCanceledReq_.push_back(ev->GetRequest());
            } else if (event->GetType() == SubscriberEventType::ON_CANCELED_WITH_SORTINGMAP_AND_DELETEREASON) {
                std::shared_ptr<OnOnCanceledWithSortingMapAndDeleteReasonEvent> ev =
                    std::static_pointer_cast<OnOnCanceledWithSortingMapAndDeleteReasonEvent>(event);
                waitOnCanceledWithSortingMapAndDeleteReason_ = true;
                onCanceledWithSortingMapReq_.push_back(ev->GetRequest());
                onCanceledWithSortingMapSor_.push_back(ev->GetSortingMap());
                onCanceledWithSortingMapDelRea_.push_back(ev->GetDeleteReason());
            } else if (event->GetType() == SubscriberEventType::ON_UNSUBSCRIBERESULT) {
                std::shared_ptr<OnUnSubscribeResultEvent> ev =
                    std::static_pointer_cast<OnUnSubscribeResultEvent>(event);
                waitOnUnSubscriber_ = ev->GetUnSubscribeResult();
            }
        }
    }

    void setWaitOnConsumed(bool bl)
    {
        waitOnConsumed_ = bl;
    }

    void setWaitOnCanceled(bool bl)
    {
        waitOnCanceled_ = bl;
    }

    void setWaitOnCanceledWithSortingMapAndDeleteReason(bool bl)
    {
        waitOnCanceledWithSortingMapAndDeleteReason_ = bl;
    }

    void setWaitOnUnSubscriber()
    {
        waitOnUnSubscriber_ = NotificationConstant::SubscribeResult::RESOURCES_FAIL;
    }

    uint32_t getWaitOnSubscriber()
    {
        return waitOnSubscriber_;
    }

    uint32_t getWaitOnUnSubscriber()
    {
        return waitOnUnSubscriber_;
    }

    bool getwaitOnConsumed()
    {
        return waitOnConsumed_;
    }

    std::vector<std::shared_ptr<Notification>> getOnConsumedReq()
    {
        return onConsumedReq_;
    }

    std::vector<std::shared_ptr<Notification>> getOnConsumedWithSortingMapReq()
    {
        return onConsumedWithSortingMapReq_;
    }

    std::vector<std::shared_ptr<NotificationSortingMap>> getOnConsumedWithSortingMapSor()
    {
        return onConsumedWithSortingMapSor_;
    }

    bool getWaitOnConsumedWithSortingMap()
    {
        return waitOnConsumedWithSortingMap_;
    }

    bool getWaitOnCanceled()
    {
        return waitOnCanceled_;
    }

    bool getWaitOnCanceledWithSortingMapAndDeleteReason()
    {
        return waitOnCanceledWithSortingMapAndDeleteReason_;
    }

    std::vector<std::shared_ptr<Notification>> getOnCanceledReq()
    {
        return onCanceledReq_;
    }

    std::vector<std::shared_ptr<Notification>> getOnCanceledWithSortingMapReq()
    {
        return onCanceledWithSortingMapReq_;
    }

    std::vector<std::shared_ptr<NotificationSortingMap>> getOnCanceledWithSortingMapSor()
    {
        return onCanceledWithSortingMapSor_;
    }

    std::vector<int> getOnCanceledWithSortingMapDelRea()
    {
        return onCanceledWithSortingMapDelRea_;
    }

private:
    uint32_t waitOnSubscriber_;
    uint32_t waitOnUnSubscriber_;
    bool waitOnConsumed_ = false;
    std::vector<std::shared_ptr<Notification>> onConsumedReq_;
    std::vector<std::shared_ptr<Notification>> onConsumedWithSortingMapReq_;
    std::vector<std::shared_ptr<NotificationSortingMap>> onConsumedWithSortingMapSor_;
    bool waitOnConsumedWithSortingMap_ = false;
    bool waitOnCanceled_ = false;
    std::vector<std::shared_ptr<Notification>> onCanceledReq_;
    std::vector<std::shared_ptr<Notification>> onCanceledWithSortingMapReq_;
    std::vector<std::shared_ptr<NotificationSortingMap>> onCanceledWithSortingMapSor_;
    std::vector<int> onCanceledWithSortingMapDelRea_;
    bool waitOnCanceledWithSortingMapAndDeleteReason_ = false;
};

sptr<ISystemAbilityManager> g_systemAbilityManager =
    SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
void AnsFWModuleTest::SetUpTestCase()
{
    sptr<AdvancedNotificationService> service = OHOS::Notification::AdvancedNotificationService::GetInstance();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    g_systemAbilityManager->AddSystemAbility(OHOS::ADVANCED_NOTIFICATION_SERVICE_ABILITY_ID, service, saExtraProp);
}

void AnsFWModuleTest::TearDownTestCase()
{}

void AnsFWModuleTest::SetUp()
{}

void AnsFWModuleTest::TearDown()
{
    NotificationHelper::CancelAllNotifications();
    NotificationHelper::RemoveAllSlots();
    NotificationHelper::RemoveNotifications();
    std::vector<sptr<NotificationSlotGroup>> groups;
    NotificationHelper::GetNotificationSlotGroups(groups);
    for (auto group : groups) {
        NotificationHelper::RemoveNotificationSlotGroup(group->GetId().c_str());
    }
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_FlowControl_00100
 * @tc.name      : FlowControl_00100
 * @tc.desc      : Test notification's flow control.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_FlowControl_00100, Function | MediumTest | Level1)
{
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName(APP_NAME);
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);
    std::shared_ptr<NotificationNormalContent> implContent = std::make_shared<NotificationNormalContent>();
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(implContent);
    for (uint32_t i = 0; i <= MAX_ACTIVE_NUM_PERSECOND; i++) {
        std::string label = std::to_string(i);
        NotificationRequest req(i);
        req.SetLabel(label);
        req.SetContent(content);
        if (i < MAX_ACTIVE_NUM_PERSECOND) {
            EXPECT_EQ(NotificationHelper::PublishNotification(req), ERR_OK);
        } else {
            EXPECT_EQ(NotificationHelper::PublishNotification(req), (int)ERR_ANS_OVER_MAX_ACITVE_PERSECOND);
        }
    }
    SleepForFC();
    EXPECT_EQ(NotificationHelper::RemoveNotifications(), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);
    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    EventParser eventParser;
    eventParser.parse(events);
    for (uint32_t i = 0; i <= MAX_ACTIVE_NUM_PERSECOND; i++) {
        std::string notificationLabel = std::to_string(i);
        std::string notificationIdStr = std::to_string(i);
        int32_t notificationIdInt = i;
        if (i < MAX_ACTIVE_NUM_PERSECOND) {
            std::stringstream stream;
            stream << UID << KEY_SPLITER << notificationLabel << KEY_SPLITER << notificationIdInt;
            std::string notificationKey = stream.str();
            NotificationSorting sorting;
            EXPECT_EQ(eventParser.getOnConsumedReq()[i]->GetLabel().c_str(), notificationLabel);
            EXPECT_EQ(eventParser.getOnConsumedReq()[i]->GetId(), notificationIdInt);
            EXPECT_EQ(eventParser.getOnConsumedReq()[i]->GetKey(), notificationKey);
            EXPECT_EQ(eventParser.getOnConsumedWithSortingMapReq()[i]->GetLabel().c_str(), notificationLabel);
            EXPECT_EQ(eventParser.getOnConsumedWithSortingMapReq()[i]->GetId(), notificationIdInt);
            EXPECT_EQ(eventParser.getOnConsumedWithSortingMapReq()[i]->GetKey(), notificationKey);
            EXPECT_TRUE(
                eventParser.getOnConsumedWithSortingMapSor()[i]->GetNotificationSorting(notificationKey, sorting));
            EXPECT_EQ(sorting.GetKey().c_str(), notificationKey);
        }
    }
    EXPECT_EQ((uint32_t)eventParser.getOnConsumedReq().size(), MAX_ACTIVE_NUM_PERSECOND);
    EXPECT_EQ((uint32_t)eventParser.getOnConsumedWithSortingMapReq().size(), MAX_ACTIVE_NUM_PERSECOND);
    EXPECT_EQ(eventParser.getWaitOnSubscriber(), NotificationConstant::SubscribeResult::SUCCESS);
    EXPECT_EQ(eventParser.getWaitOnUnSubscriber(), NotificationConstant::SubscribeResult::SUCCESS);
    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_RemoveNotificaitonsByKey_00100
 * @tc.name      : RemoveNotificaitonsByKey_00100
 * @tc.desc      : Remove Notification by key.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_RemoveNotificaitonsByKey_00100, Function | MediumTest | Level1)
{
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("bundleName");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);
    std::shared_ptr<NotificationNormalContent> implContent = std::make_shared<NotificationNormalContent>();
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(implContent);
    NotificationRequest req(0);
    req.SetLabel(NOTIFICATION_LABEL_0);
    req.SetContent(content);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), ERR_OK);
    std::vector<sptr<Notification>> notifications;
    EXPECT_EQ(NotificationHelper::GetAllActiveNotifications(notifications), ERR_OK);
    std::string key = notifications[0]->GetKey().c_str();
    EXPECT_EQ(NotificationHelper::RemoveNotification(key), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);
    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    EventParser eventParser;
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getwaitOnConsumed());
    EXPECT_TRUE(eventParser.getWaitOnConsumedWithSortingMap());
    EXPECT_TRUE(eventParser.getWaitOnCanceled());
    EXPECT_EQ(eventParser.getOnConsumedReq()[0]->GetLabel().c_str(), NOTIFICATION_LABEL_0);
    EXPECT_EQ(eventParser.getOnConsumedReq()[0]->GetId(), 0);
    std::stringstream stream;
    stream << UID << KEY_SPLITER << NOTIFICATION_LABEL_0 << KEY_SPLITER << 0;
    std::string notificationKey = stream.str();
    NotificationSorting sorting;
    EXPECT_EQ(eventParser.getOnCanceledReq()[0]->GetKey(), notificationKey);
    EXPECT_EQ(eventParser.getOnCanceledWithSortingMapReq()[0]->GetKey(), notificationKey);
    EXPECT_TRUE(eventParser.getOnConsumedWithSortingMapSor()[0]->GetNotificationSorting(notificationKey, sorting));
    EXPECT_EQ(sorting.GetKey().c_str(), notificationKey);
    EXPECT_EQ(eventParser.getOnCanceledWithSortingMapDelRea()[0], CANCEL_REASON_DELETE);
    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_RemoveNotificaitonsByKey_00200
 * @tc.name      : RemoveNotificaitonsByKey_00200
 * @tc.desc      : Remove Notification by a nonexistent key.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_RemoveNotificaitonsByKey_00200, Function | MediumTest | Level1)
{
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("bundleName");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);
    std::shared_ptr<NotificationNormalContent> implContent = std::make_shared<NotificationNormalContent>();
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(implContent);
    NotificationRequest req(0);
    req.SetLabel(NOTIFICATION_LABEL_0);
    req.SetContent(content);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), ERR_OK);
    std::vector<sptr<Notification>> notifications;
    EXPECT_EQ(NotificationHelper::GetAllActiveNotifications(notifications), ERR_OK);
    EXPECT_EQ(NotificationHelper::RemoveNotification(AN_NOT_EXIST_KEY), (int)ERR_ANS_NOTIFICATION_NOT_EXISTS);
    std::string key = notifications[0]->GetKey().c_str();
    EXPECT_EQ(NotificationHelper::RemoveNotification(key), (int)ERR_OK);
    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    EventParser eventParser;
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getWaitOnCanceled());
    eventParser.setWaitOnCanceled(false);
    EXPECT_EQ(NotificationHelper::RemoveNotification(key), (int)ERR_ANS_NOTIFICATION_NOT_EXISTS);
    events = subscriber.GetEvents();
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getWaitOnCanceled());
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);
    SleepForFC();
    events = subscriber.GetEvents();
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getWaitOnConsumedWithSortingMap());
    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_RemoveNotificaitons_00100
 * @tc.name      : RemoveNotificaitons_00100
 * @tc.desc      : Remove all Notifications.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_RemoveNotificaitons_00100, Function | MediumTest | Level1)
{
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("bundleName");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);

    std::shared_ptr<NotificationNormalContent> implContent = std::make_shared<NotificationNormalContent>();
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(implContent);
    NotificationRequest req(0);
    req.SetLabel(NOTIFICATION_LABEL_0);
    req.SetContent(content);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), ERR_OK);
    SleepForFC();
    EventParser eventParser;
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getwaitOnConsumed());
    eventParser.setWaitOnConsumed(false);

    NotificationRequest req1(1);
    req1.SetLabel(NOTIFICATION_LABEL_0);
    req1.SetContent(content);
    events = subscriber.GetEvents();
    EXPECT_EQ(NotificationHelper::PublishNotification(req1), ERR_OK);
    SleepForFC();
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getwaitOnConsumed());
    SleepForFC();
    EXPECT_EQ(NotificationHelper::RemoveNotifications(), ERR_OK);
    std::vector<sptr<Notification>> notifications;
    EXPECT_EQ(NotificationHelper::GetAllActiveNotifications(notifications), ERR_OK);
    EXPECT_EQ((int)notifications.size(), (int)0);

    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);
    events = subscriber.GetEvents();
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getWaitOnCanceled());
    EXPECT_TRUE(eventParser.getWaitOnCanceledWithSortingMapAndDeleteReason());
    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_RemoveNotificaitons_00200
 * @tc.name      : RemoveNotificaitons_00200
 * @tc.desc      : Remove Notifications when no Notification.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_RemoveNotificaitons_00200, Function | MediumTest | Level1)
{
    EventParser eventParser;
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("bundleName");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);
    EXPECT_EQ(NotificationHelper::RemoveNotifications(), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    eventParser.parse(events);
    EXPECT_FALSE(eventParser.getwaitOnConsumed());
    EXPECT_FALSE(eventParser.getWaitOnConsumedWithSortingMap());
    EXPECT_FALSE(eventParser.getWaitOnCanceled());
    EXPECT_FALSE(eventParser.getWaitOnCanceledWithSortingMapAndDeleteReason());
    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_UnSubscriber_00100
 * @tc.name      : UnSubscriber_00100
 * @tc.desc      : Remove Subscriber.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_UnSubscriber_00100, Function | MediumTest | Level1)
{
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("ANS_FW_MT_UnSubscriber_00100");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);
    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    uint32_t waitOnSubscriber_;
    uint32_t waitOnUnSubscriber_;

    for (auto event : events) {
        if (event->GetType() == SubscriberEventType::ON_SUBSCRIBERESULT) {
            std::shared_ptr<OnSubscribeResultEvent> ev = std::static_pointer_cast<OnSubscribeResultEvent>(event);
            waitOnSubscriber_ = ev->GetSubscribeResult();
        } else if (event->GetType() == SubscriberEventType::ON_UNSUBSCRIBERESULT) {
            std::shared_ptr<OnUnSubscribeResultEvent> ev = std::static_pointer_cast<OnUnSubscribeResultEvent>(event);
            waitOnUnSubscriber_ = ev->GetUnSubscribeResult();
        }
    }
    EXPECT_EQ(waitOnSubscriber_, NotificationConstant::SubscribeResult::SUCCESS);

    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_UnSubscriber_00200
 * @tc.name      : UnSubscriber_00200
 * @tc.desc      : Remove Subscriber.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_UnSubscriber_00200, Function | MediumTest | Level1)
{
    EventParser eventParser;
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("ANS_FW_MT_UnSubscriber_00100");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), (int)ERR_OK);
    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    uint32_t waitOnSubscriber_;
    uint32_t waitOnUnSubscriber_;
    for (auto event : events) {
        if (event->GetType() == SubscriberEventType::ON_SUBSCRIBERESULT) {
            std::shared_ptr<OnSubscribeResultEvent> ev = std::static_pointer_cast<OnSubscribeResultEvent>(event);
            waitOnSubscriber_ = ev->GetSubscribeResult();
        } else if (event->GetType() == SubscriberEventType::ON_UNSUBSCRIBERESULT) {
            std::shared_ptr<OnUnSubscribeResultEvent> ev = std::static_pointer_cast<OnUnSubscribeResultEvent>(event);
            waitOnUnSubscriber_ = ev->GetUnSubscribeResult();
        }
    }
    EXPECT_EQ(waitOnSubscriber_, NotificationConstant::SubscribeResult::SUCCESS);
    waitOnSubscriber_ = NotificationConstant::SubscribeResult::PREMISSION_FAIL;
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), (int)ERR_OK);
    events = subscriber.GetEvents();
    for (auto event : events) {
        if (event->GetType() == SubscriberEventType::ON_UNSUBSCRIBERESULT) {
            std::shared_ptr<OnUnSubscribeResultEvent> ev = std::static_pointer_cast<OnUnSubscribeResultEvent>(event);
            waitOnUnSubscriber_ = ev->GetUnSubscribeResult();
        }
    }
    EXPECT_EQ(waitOnUnSubscriber_, NotificationConstant::SubscribeResult::SUCCESS);
    subscriber.ClearEvents();
    SleepForFC();
}
/**
 *
 * @tc.number    : ANS_FW_MT_Subscriber_00100
 * @tc.name      : Subscriber_00100
 * @tc.desc      : Subscriber Notifications.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_Subscriber_00100, Function | MediumTest | Level1)
{
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("ANS_FW_MT_UnSubscriber_00100");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);
    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    uint32_t waitOnSubscriber_;
    uint32_t waitOnUnSubscriber_;
    for (auto event : events) {
        if (event->GetType() == SubscriberEventType::ON_SUBSCRIBERESULT) {
            std::shared_ptr<OnSubscribeResultEvent> ev = std::static_pointer_cast<OnSubscribeResultEvent>(event);
            waitOnSubscriber_ = ev->GetSubscribeResult();
        } else if (event->GetType() == SubscriberEventType::ON_UNSUBSCRIBERESULT) {
            std::shared_ptr<OnUnSubscribeResultEvent> ev = std::static_pointer_cast<OnUnSubscribeResultEvent>(event);
            waitOnUnSubscriber_ = ev->GetUnSubscribeResult();
        }
    }
    EXPECT_EQ(waitOnSubscriber_, NotificationConstant::SubscribeResult::SUCCESS);
    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_CancelNotificationById_00100
 * @tc.name      : CancelNotificationById_00100
 * @tc.desc      : Cancel Notification By Id.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_CancelNotificationById_00100, Function | MediumTest | Level1)
{
    EventParser eventParser;
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("bundleName");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);
    std::shared_ptr<NotificationNormalContent> implContent = std::make_shared<NotificationNormalContent>();
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(implContent);
    NotificationRequest req(1);
    req.SetLabel(NOTIFICATION_LABEL_0);
    req.SetContent(content);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), ERR_OK);
    SleepForFC();
    std::vector<sptr<Notification>> notifications;
    EXPECT_EQ(NotificationHelper::GetAllActiveNotifications(notifications), ERR_OK);
    int32_t id = notifications[0]->GetId();
    SleepForFC();
    EXPECT_EQ(NotificationHelper::CancelNotification(NOTIFICATION_LABEL_0, id), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);

    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();

    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getwaitOnConsumed());
    EXPECT_TRUE(eventParser.getWaitOnConsumedWithSortingMap());
    EXPECT_TRUE(eventParser.getWaitOnCanceled());
    EXPECT_EQ(eventParser.getOnConsumedReq()[0]->GetLabel().c_str(), NOTIFICATION_LABEL_0);
    EXPECT_EQ(eventParser.getOnConsumedReq()[0]->GetId(), 1);
    std::stringstream stream;
    stream << UID << KEY_SPLITER << NOTIFICATION_LABEL_0 << KEY_SPLITER << 1;
    std::string notificationKey = stream.str();
    NotificationSorting sorting;
    EXPECT_EQ(eventParser.getOnCanceledReq()[0]->GetKey(), notificationKey);
    EXPECT_EQ(eventParser.getOnCanceledWithSortingMapReq()[0]->GetKey(), notificationKey);
    EXPECT_TRUE(eventParser.getOnConsumedWithSortingMapSor()[0]->GetNotificationSorting(notificationKey, sorting));
    EXPECT_EQ(sorting.GetKey().c_str(), notificationKey);
    EXPECT_EQ(eventParser.getOnCanceledWithSortingMapDelRea()[0], APP_CANCEL_REASON_DELETE);
    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_CancelNotificationById_00200
 * @tc.name      : CancelNotificationById_00200
 * @tc.desc      : Cancel Notification By Id when Id is not exist.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_CancelNotificationById_00200, Function | MediumTest | Level1)
{
    EventParser eventParser;
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("bundleName");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);

    std::shared_ptr<NotificationNormalContent> implContent = std::make_shared<NotificationNormalContent>();
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(implContent);
    NotificationRequest req(1);
    req.SetLabel(NOTIFICATION_LABEL_0);
    req.SetContent(content);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), ERR_OK);
    SleepForFC();
    std::vector<sptr<Notification>> notifications;
    EXPECT_EQ(NotificationHelper::GetAllActiveNotifications(notifications), ERR_OK);
    int32_t id = 0;
    SleepForFC();

    eventParser.setWaitOnCanceled(false);
    eventParser.setWaitOnCanceledWithSortingMapAndDeleteReason(false);
    EXPECT_EQ(NotificationHelper::CancelNotification(NOTIFICATION_LABEL_0, id), (int)ERR_ANS_NOTIFICATION_NOT_EXISTS);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);

    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getwaitOnConsumed());
    EXPECT_TRUE(eventParser.getWaitOnConsumedWithSortingMap());
    EXPECT_FALSE(eventParser.getWaitOnCanceled());
    EXPECT_FALSE(eventParser.getWaitOnCanceledWithSortingMapAndDeleteReason());
    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_CancelAllNotifications_00100
 * @tc.name      :
 * @tc.desc      : Cancel all notifications.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_CancelAllNotifications_00100, Function | MediumTest | Level1)
{
    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("bundleName");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);

    std::shared_ptr<NotificationNormalContent> implContent = std::make_shared<NotificationNormalContent>();
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(implContent);
    NotificationRequest req0(0);
    req0.SetLabel(NOTIFICATION_LABEL_0);
    req0.SetContent(content);
    EXPECT_EQ(NotificationHelper::PublishNotification(req0), ERR_OK);

    NotificationRequest req1(1);
    req1.SetLabel(NOTIFICATION_LABEL_1);
    req1.SetContent(content);
    EXPECT_EQ(NotificationHelper::PublishNotification(req1), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::CancelAllNotifications(), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);

    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();

    EventParser eventParser;
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getwaitOnConsumed());
    EXPECT_TRUE(eventParser.getWaitOnConsumedWithSortingMap());
    EXPECT_TRUE(eventParser.getWaitOnCanceled());
    EXPECT_EQ(eventParser.getOnConsumedReq()[0]->GetLabel().c_str(), NOTIFICATION_LABEL_0);
    EXPECT_EQ(eventParser.getOnConsumedReq()[0]->GetId(), 0);
    std::stringstream stream0;
    stream0 << UID << KEY_SPLITER << NOTIFICATION_LABEL_0 << KEY_SPLITER << 0;
    std::string notificationKey0 = stream0.str();
    NotificationSorting sorting0;
    EXPECT_EQ(eventParser.getOnCanceledReq()[0]->GetKey(), notificationKey0);
    EXPECT_EQ(eventParser.getOnCanceledWithSortingMapReq()[0]->GetKey(), notificationKey0);
    EXPECT_TRUE(eventParser.getOnConsumedWithSortingMapSor()[0]->GetNotificationSorting(notificationKey0, sorting0));
    EXPECT_EQ(sorting0.GetKey().c_str(), notificationKey0);
    EXPECT_EQ(eventParser.getOnCanceledWithSortingMapDelRea()[0], APP_CANCEL_ALL_REASON_DELETE);

    EXPECT_EQ(eventParser.getOnConsumedReq()[1]->GetLabel().c_str(), NOTIFICATION_LABEL_1);
    EXPECT_EQ(eventParser.getOnConsumedReq()[1]->GetId(), 1);
    std::stringstream stream1;
    stream1 << UID << KEY_SPLITER << NOTIFICATION_LABEL_1 << KEY_SPLITER << 1;
    std::string notificationKey1 = stream1.str();
    NotificationSorting sorting1;
    EXPECT_EQ(eventParser.getOnCanceledReq()[1]->GetKey(), notificationKey1);
    EXPECT_EQ(eventParser.getOnCanceledWithSortingMapReq()[1]->GetKey(), notificationKey1);
    EXPECT_TRUE(eventParser.getOnConsumedWithSortingMapSor()[1]->GetNotificationSorting(notificationKey1, sorting1));
    EXPECT_EQ(sorting1.GetKey().c_str(), notificationKey1);
    EXPECT_EQ(eventParser.getOnCanceledWithSortingMapDelRea()[1], APP_CANCEL_ALL_REASON_DELETE);

    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_PublishSoundNotification_00100
 * @tc.name      :
 * @tc.desc      : Publish a sound notification.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_PublishSoundNotification_00100, Function | MediumTest | Level1)
{
    std::vector<sptr<NotificationSlot>> slots;
    NotificationSlot slot1;
    slot1.SetType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slot1.SetSound(Uri("."));
    slot1.SetVibrationStyle(std::vector<int64_t>(1, 1));
    EXPECT_EQ(NotificationHelper::AddNotificationSlot(slot1), (int)ERR_OK);

    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("bundleName");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);

    std::shared_ptr<NotificationNormalContent> implContent = std::make_shared<NotificationNormalContent>();
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(implContent);
    NotificationRequest req(0);
    req.SetLabel(NOTIFICATION_LABEL_0);
    req.SetContent(content);
    req.SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::CancelAllNotifications(), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);
    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();

    EventParser eventParser;
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getwaitOnConsumed());
    EXPECT_TRUE(eventParser.getWaitOnConsumedWithSortingMap());
    EXPECT_TRUE(eventParser.getWaitOnCanceled());
    EXPECT_TRUE(eventParser.getWaitOnCanceledWithSortingMapAndDeleteReason());
    subscriber.ClearEvents();
    SleepForFC();
}

/**
 *
 * @tc.number    : ANS_FW_MT_PublishVibrationNotification_00100
 * @tc.name      :
 * @tc.desc      : Publish a vibration notification.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_PublishVibrationNotification_00100, Function | MediumTest | Level1)
{
    std::vector<sptr<NotificationSlot>> slots;
    NotificationSlot slot1;
    slot1.SetType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slot1.SetEnableVibration(true);
    slot1.SetVibrationStyle(std::vector<int64_t>(1, 1));
    EXPECT_EQ(NotificationHelper::AddNotificationSlot(slot1), (int)ERR_OK);

    TestAnsSubscriber subscriber;
    NotificationSubscribeInfo info;
    info.AddAppName("bundleName");
    EXPECT_EQ(NotificationHelper::SubscribeNotification(subscriber, info), ERR_OK);

    std::shared_ptr<NotificationNormalContent> implContent = std::make_shared<NotificationNormalContent>();
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(implContent);
    NotificationRequest req(0);
    req.SetLabel(NOTIFICATION_LABEL_0);
    req.SetContent(content);
    req.SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::CancelAllNotifications(), ERR_OK);
    SleepForFC();
    EXPECT_EQ(NotificationHelper::UnSubscribeNotification(subscriber, info), ERR_OK);
    SleepForFC();
    std::list<std::shared_ptr<SubscriberEvent>> events = subscriber.GetEvents();
    EventParser eventParser;
    eventParser.parse(events);
    EXPECT_TRUE(eventParser.getwaitOnConsumed());
    EXPECT_TRUE(eventParser.getWaitOnConsumedWithSortingMap());
    EXPECT_TRUE(eventParser.getWaitOnCanceled());
    EXPECT_TRUE(eventParser.getWaitOnCanceledWithSortingMapAndDeleteReason());
    subscriber.ClearEvents();
    SleepForFC();
}

inline std::shared_ptr<PixelMap> MakePixelMap(int32_t width, int32_t height)
{
    const int32_t PIXEL_BYTES = 4;
    std::shared_ptr<PixelMap> pixelMap = std::make_shared<PixelMap>();
    if (pixelMap == nullptr) {
        return pixelMap;
    }
    ImageInfo info;
    info.size.width = width;
    info.size.height = height;
    info.pixelFormat = PixelFormat::ARGB_8888;
    info.colorSpace = ColorSpace::SRGB;
    pixelMap->SetImageInfo(info);
    int32_t rowDataSize = width * PIXEL_BYTES;
    uint32_t bufferSize = rowDataSize * height;
    void *buffer = malloc(bufferSize);
    if (buffer != nullptr) {
        pixelMap->SetPixelsAddr(buffer, nullptr, bufferSize, AllocatorType::HEAP_ALLOC, nullptr);
    }
    EXPECT_NE(buffer, nullptr);
    return pixelMap;
}

/**
 *
 * @tc.number    : ANS_FW_MT_PublishNotificationWithPixelMap_00100
 * @tc.name      :
 * @tc.desc      : Publish a notification with pixelMap.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_PublishNotificationWithPixelMap_00100, Function | MediumTest | Level1)
{
    const int BIG_PICTURE_WIDTH = 400;
    const int BIG_PICTURE_HEIGHT = 300;
    const int ICON_SIZE = 36;

    NotificationRequest req;
    req.SetSlotType(NotificationConstant::SlotType::OTHER);
    req.SetLabel("label");
    std::shared_ptr<NotificationPictureContent> pictureContent = std::make_shared<NotificationPictureContent>();
    EXPECT_NE(pictureContent, nullptr);
    pictureContent->SetText("notification text");
    pictureContent->SetTitle("notification title");
    std::shared_ptr<PixelMap> bigPicture = MakePixelMap(BIG_PICTURE_WIDTH, BIG_PICTURE_HEIGHT);
    EXPECT_NE(bigPicture, nullptr);
    pictureContent->SetBigPicture(bigPicture);
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(pictureContent);
    EXPECT_NE(content, nullptr);
    req.SetContent(content);
    std::shared_ptr<PixelMap> littleIcon = MakePixelMap(ICON_SIZE, ICON_SIZE);
    req.SetLittleIcon(littleIcon);
    std::shared_ptr<PixelMap> bigIcon = MakePixelMap(ICON_SIZE, ICON_SIZE);
    req.SetBigIcon(bigIcon);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), ERR_OK);
}

/**
 *
 * @tc.number    : ANS_FW_MT_PublishNotificationWithPixelMap_00200
 * @tc.name      :
 * @tc.desc      : Publish a notification with oversize pixelMap.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_PublishNotificationWithPixelMap_00200, Function | MediumTest | Level1)
{
    const int BIG_PICTURE_WIDTH = 1024;
    const int BIG_PICTURE_HEIGHT = 1024;
    const int ICON_SIZE = 36;

    NotificationRequest req;
    req.SetSlotType(NotificationConstant::SlotType::OTHER);
    req.SetLabel("label");
    std::shared_ptr<NotificationPictureContent> pictureContent = std::make_shared<NotificationPictureContent>();
    EXPECT_NE(pictureContent, nullptr);
    pictureContent->SetText("notification text");
    pictureContent->SetTitle("notification title");
    std::shared_ptr<PixelMap> bigPicture = MakePixelMap(BIG_PICTURE_WIDTH, BIG_PICTURE_HEIGHT);
    EXPECT_NE(bigPicture, nullptr);
    pictureContent->SetBigPicture(bigPicture);
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(pictureContent);
    EXPECT_NE(content, nullptr);
    req.SetContent(content);
    std::shared_ptr<PixelMap> littleIcon = MakePixelMap(ICON_SIZE, ICON_SIZE);
    req.SetLittleIcon(littleIcon);
    std::shared_ptr<PixelMap> bigIcon = MakePixelMap(ICON_SIZE, ICON_SIZE);
    req.SetBigIcon(bigIcon);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), (int)ERR_ANS_PICTURE_OVER_SIZE);
}

/**
 *
 * @tc.number    : ANS_FW_MT_PublishNotificationWithPixelMap_00300
 * @tc.name      :
 * @tc.desc      : Publish a notification with oversize pixelMap.
 */
HWTEST_F(AnsFWModuleTest, ANS_FW_MT_PublishNotificationWithPixelMap_00300, Function | MediumTest | Level1)
{
    const int BIG_PICTURE_WIDTH = 400;
    const int BIG_PICTURE_HEIGHT = 300;
    const int ICON_SIZE = 256;

    NotificationRequest req;
    req.SetSlotType(NotificationConstant::SlotType::OTHER);
    req.SetLabel("label");
    std::shared_ptr<NotificationPictureContent> pictureContent = std::make_shared<NotificationPictureContent>();
    EXPECT_NE(pictureContent, nullptr);
    pictureContent->SetText("notification text");
    pictureContent->SetTitle("notification title");
    std::shared_ptr<PixelMap> bigPicture = MakePixelMap(BIG_PICTURE_WIDTH, BIG_PICTURE_HEIGHT);
    EXPECT_NE(bigPicture, nullptr);
    pictureContent->SetBigPicture(bigPicture);
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(pictureContent);
    EXPECT_NE(content, nullptr);
    req.SetContent(content);
    std::shared_ptr<PixelMap> littleIcon = MakePixelMap(ICON_SIZE, ICON_SIZE);
    req.SetLittleIcon(littleIcon);
    std::shared_ptr<PixelMap> bigIcon = MakePixelMap(ICON_SIZE, ICON_SIZE);
    req.SetBigIcon(bigIcon);
    EXPECT_EQ(NotificationHelper::PublishNotification(req), (int)ERR_ANS_ICON_OVER_SIZE);
}

}  // namespace Notification
}  // namespace OHOS
