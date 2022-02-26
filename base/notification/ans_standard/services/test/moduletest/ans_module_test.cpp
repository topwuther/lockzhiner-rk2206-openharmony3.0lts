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
#include <functional>
#include <gtest/gtest.h>

#include "notification_preferences.h"
#define private public
#include "advanced_notification_service.h"
#include "notification_subscriber.h"

using namespace testing::ext;

namespace OHOS {
namespace Notification {
typedef std::function<void(const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>)>
    ConsumedFunc;
typedef std::function<void(const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>, int)>
    CanceledFunc;

sptr<AdvancedNotificationService> g_advancedNotificationService;
bool passed = false;
class TestAnsSubscriber : public NotificationSubscriber {
public:
    ~TestAnsSubscriber(){};

    void OnSubscribeResult(NotificationConstant::SubscribeResult result) override
    {
        if (subscriberCb_ != nullptr) {
            subscriberCb_(result);
        }
    }
    void OnUnsubscribeResult(NotificationConstant::SubscribeResult result) override
    {
        if (unSubscriberCb_ != nullptr) {
            unSubscriberCb_(result);
        }
    }
    void OnDied() override
    {}
    void OnUpdate(const std::shared_ptr<NotificationSortingMap> &sortingMap) override
    {}
    void OnDisturbModeChanged(int disturbMode) override
    {}
    void OnCanceled(const std::shared_ptr<Notification> &request) override
    {}
    void OnCanceled(const std::shared_ptr<Notification> &request,
        const std::shared_ptr<NotificationSortingMap> &sortingMap, int deleteReason) override
    {
        if (canceledCb_ != nullptr) {
            canceledCb_(request, sortingMap, deleteReason);
        }
    }
    void OnConsumed(const std::shared_ptr<Notification> &request) override
    {}
    void OnConsumed(const std::shared_ptr<Notification> &request,
        const std::shared_ptr<NotificationSortingMap> &sortingMap) override
    {
        if (consumedCb_ != nullptr) {
            consumedCb_(request, sortingMap);
        }
    }

    ConsumedFunc consumedCb_ = nullptr;
    CanceledFunc canceledCb_ = nullptr;
    std::function<void(NotificationConstant::SubscribeResult)> unSubscriberCb_ = nullptr;
    std::function<void(NotificationConstant::SubscribeResult)> subscriberCb_ = nullptr;
};

class AnsModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AnsModuleTest::SetUpTestCase()
{
    passed = false;
    g_advancedNotificationService = new AdvancedNotificationService();
    NotificationPreferences::GetInstance().ClearNotificationInRestoreFactorySettings();
}

void AnsModuleTest::TearDownTestCase()
{
    passed = false;
    g_advancedNotificationService = new AdvancedNotificationService();
    NotificationPreferences::GetInstance().ClearNotificationInRestoreFactorySettings();
}

void AnsModuleTest::SetUp()
{
    passed = false;
    g_advancedNotificationService = new AdvancedNotificationService();
    NotificationPreferences::GetInstance().ClearNotificationInRestoreFactorySettings();
}

void AnsModuleTest::TearDown()
{
    sptr<AdvancedNotificationService> g_advancedNotificationService = nullptr;
    NotificationPreferences::GetInstance().ClearNotificationInRestoreFactorySettings();
    passed = false;
}

void TestAddSlots()
{
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);
}

/**
 * @tc.number    : AnsModuleTest_001
 * @tc.name      : AMS_ANS_Subscribe_Publish_0100
 * @tc.desc      : Test the functions of subscribing, publishing, and canceling
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_001, Function | SmallTest | Level1)
{
    TestAddSlots();
    std::string label = "testLabel";
    auto subscriber = new TestAnsSubscriber();
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };
    sptr<NotificationRequest> req = new NotificationRequest(2);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req1->SetLabel(label);
    sptr<NotificationSubscribeInfo> info = new NotificationSubscribeInfo();
    info->AddAppName("bundleName");
    EXPECT_EQ((int)g_advancedNotificationService->Subscribe(subscriber->GetImpl(), info), (int)ERR_OK);
    EXPECT_EQ((int)g_advancedNotificationService->Publish(label, req), (int)ERR_OK);
    EXPECT_EQ((int)g_advancedNotificationService->Publish(label, req1), (int)ERR_OK);
    EXPECT_EQ((int)g_advancedNotificationService->Cancel(1, label), (int)ERR_OK);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->CancelAll();
    EXPECT_EQ((int)g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), info), (int)ERR_OK);
    passed = false;
}

/**
 * @tc.number    : AnsModuleTest_002
 * @tc.name      : AMS_ANS_GetActiveNotifications_0200
 * @tc.desc      : Test the function of getting notifications and getting all notifications
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_002, Function | SmallTest | Level1)
{
    TestAddSlots();
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req1->SetLabel(label);
    sptr<NotificationRequest> req2 = new NotificationRequest(2);
    req2->SetLabel("testLabel1");
    sptr<NotificationSubscribeInfo> info = new NotificationSubscribeInfo();
    info->AddAppName("bundleName");
    std::vector<sptr<NotificationRequest>> notificationsReqs;
    std::vector<sptr<Notification>> notifications;
    EXPECT_EQ((int)g_advancedNotificationService->Publish(label, req), (int)ERR_OK);
    EXPECT_EQ((int)g_advancedNotificationService->Publish(label, req1), (int)ERR_OK);
    EXPECT_EQ((int)g_advancedNotificationService->Publish("testLabel1", req2), (int)ERR_OK);
    EXPECT_EQ((int)g_advancedNotificationService->GetActiveNotifications(notificationsReqs), (int)ERR_OK);
    int num;
    g_advancedNotificationService->GetActiveNotificationNums(num);
    EXPECT_EQ((int)num, (int)3);
    EXPECT_EQ((int)g_advancedNotificationService->Cancel(2, "testLabel1"), (int)ERR_OK);
    EXPECT_EQ((int)g_advancedNotificationService->GetAllActiveNotifications(notifications), (int)ERR_OK);
    EXPECT_EQ((int)notifications.size(), (int)2);
    EXPECT_EQ((int)g_advancedNotificationService->CancelAll(), (int)ERR_OK);
}

/**
 * @tc.number    : AnsModuleTest_003
 * @tc.name      : AMS_ANS_GetActiveNotifications_0300
 * @tc.desc      : Test publish notifications when slot are not allowed publish.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_003, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetStatusBarText("text");

    g_advancedNotificationService->SetNotificationsEnabledForBundle("", false);

    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(false, passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_004
 * @tc.name      : AMS_ANS_GetActiveNotifications_0400
 * @tc.desc      : Test publish notifications when Disturb are not allowed publish.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_004, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetStatusBarText("text");

    g_advancedNotificationService->SetDisturbMode(NotificationConstant::DisturbMode::ALLOW_NONE);

    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(false, passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_005
 * @tc.name      : AMS_ANS_GetActiveNotifications_0500
 * @tc.desc      : Test publish notifications when Disturb are not allowed publish.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_005, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_006
 * @tc.name      : AMS_ANS_GetActiveNotifications_0600
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_006, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_007
 * @tc.name      : AMS_ANS_GetActiveNotifications_0700
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_007, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0013
 * @tc.name      : AMS_ANS_GetActiveNotifications_01300
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0013, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0014
 * @tc.name      : AMS_ANS_GetActiveNotifications_01400
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0014, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationNormalContent> normalContent = std::make_shared<NotificationNormalContent>();
    normalContent->SetText("1");
    normalContent->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(normalContent);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0015
 * @tc.name      : AMS_ANS_GetActiveNotifications_01500
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0015, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationNormalContent> normalContent = std::make_shared<NotificationNormalContent>();
    normalContent->SetText("1");
    normalContent->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(normalContent);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0017
 * @tc.name      : AMS_ANS_GetActiveNotifications_01700
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0017, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationNormalContent> normalContent = std::make_shared<NotificationNormalContent>();
    normalContent->SetText("1");
    normalContent->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(normalContent);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0019
 * @tc.name      : AMS_ANS_GetActiveNotifications_01900
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0019, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationLongTextContent> longTextContent = std::make_shared<NotificationLongTextContent>();
    longTextContent->SetText("1");
    longTextContent->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(longTextContent);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0021
 * @tc.name      : AMS_ANS_GetActiveNotifications_02100
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0021, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);
    std::shared_ptr<NotificationPictureContent> pictureContent = std::make_shared<NotificationPictureContent>();
    pictureContent->SetText("1");
    pictureContent->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(pictureContent);
    req->SetContent(content);
    req1->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0023
 * @tc.name      : AMS_ANS_GetActiveNotifications_02300
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0023, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationMultiLineContent> contentImpl = std::make_shared<NotificationMultiLineContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0031
 * @tc.name      : AMS_ANS_GetActiveNotifications_03100
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0031, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);
    std::shared_ptr<NotificationMediaContent> contentImpl = std::make_shared<NotificationMediaContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req1->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0033
 * @tc.name      : AMS_ANS_GetActiveNotifications_03300
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0033, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>) {
        if (notification->EnableVibrate()) {
            passed = true;
        }
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slot->SetEnableVibration(true);
    slot->SetVibrationStyle(std::vector<int64_t>(1, 1));
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationNormalContent> contentImpl = std::make_shared<NotificationNormalContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::CONTENT_INFORMATION);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_FALSE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0034
 * @tc.name      : AMS_ANS_GetActiveNotifications_03400
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0034, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>) {
        if (notification->EnableSound()) {
            passed = true;
        }
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slot->SetEnableVibration(true);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationNormalContent> contentImpl = std::make_shared<NotificationNormalContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0035
 * @tc.name      : AMS_ANS_GetActiveNotifications_03500
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0035, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->canceledCb_ = [](const std::shared_ptr<Notification> &request,
                                  const std::shared_ptr<NotificationSortingMap> &sortingMap,
                                  int deleteReason) { passed = true; };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slot->SetEnableVibration(true);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationMediaContent> contentImpl = std::make_shared<NotificationMediaContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Cancel(0, label);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0036
 * @tc.name      : AMS_ANS_GetActiveNotifications_03600
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0036, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->canceledCb_ = [](const std::shared_ptr<Notification> &request,
                                  const std::shared_ptr<NotificationSortingMap> &sortingMap,
                                  int deleteReason) { passed = true; };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slot->SetEnableVibration(true);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationMediaContent> contentImpl = std::make_shared<NotificationMediaContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->CancelAll();
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0039
 * @tc.name      : AMS_ANS_GetActiveNotifications_03900
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0039, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slot->SetEnableVibration(true);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);
    std::shared_ptr<NotificationMediaContent> contentImpl = std::make_shared<NotificationMediaContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req1->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0040
 * @tc.name      : AMS_ANS_GetActiveNotifications_04000
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0040, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>) { passed = true; };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER);
    slot->SetEnableVibration(true);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationMediaContent> contentImpl = std::make_shared<NotificationMediaContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0041
 * @tc.name      : AMS_ANS_GetActiveNotifications_04100
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0041, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>) { passed = true; };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slot->SetEnableVibration(true);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationMediaContent> contentImpl = std::make_shared<NotificationMediaContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0042
 * @tc.name      : AMS_ANS_GetActiveNotifications_04200
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0042, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>) { passed = true; };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slot->SetEnableVibration(true);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationMediaContent> contentImpl = std::make_shared<NotificationMediaContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0043
 * @tc.name      : AMS_ANS_GetActiveNotifications_04300
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0043, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>) { passed = true; };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::CUSTOM);
    slot->SetEnableVibration(true);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationMediaContent> contentImpl = std::make_shared<NotificationMediaContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_TRUE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0049
 * @tc.name      : AMS_ANS_GetActiveNotifications_04900
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0049, Function | SmallTest | Level1)
{
    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> socialSlot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    sptr<NotificationSlot> reminderSlot = new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER);
    sptr<NotificationSlot> contentSlot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    sptr<NotificationSlot> otherSlot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slots.push_back(socialSlot);
    slots.push_back(reminderSlot);
    slots.push_back(contentSlot);
    slots.push_back(otherSlot);

    g_advancedNotificationService->AddSlots(slots);
}

/**
 * @tc.number    : AnsModuleTest_0051
 * @tc.name      : AMS_ANS_GetActiveNotifications_05100
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0051, Function | SmallTest | Level1)
{
    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    std::string slotId = slot->GetId();
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    std::vector<sptr<NotificationSlot>> slotsRef{};
    g_advancedNotificationService->GetSlots(slotsRef);
    EXPECT_EQ(5, static_cast<int>(slotsRef.size()));
    std::vector<std::string> slotsId{};
    for (const auto &i : slotsRef) {
        slotsId.push_back(i->GetId());
    }
    g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    g_advancedNotificationService->GetSlots(slotsRef);
    EXPECT_EQ(9, static_cast<int>(slotsRef.size()));
}

/**
 * @tc.number    : AnsModuleTest_0052
 * @tc.name      : AMS_ANS_GetActiveNotifications_05200
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0052, Function | SmallTest | Level1)
{
    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    std::vector<sptr<NotificationSlot>> slotsRef{};
    g_advancedNotificationService->GetSlots(slotsRef);
    std::vector<std::string> slotsId{};
    for (const auto &i : slotsRef) {
        slotsId.push_back(i->GetId());
    }
    g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    g_advancedNotificationService->GetSlots(slotsRef);
    EXPECT_EQ(9, static_cast<int>(slotsRef.size()));
}

/**
 * @tc.number    : AnsModuleTest_0054
 * @tc.name      : AMS_ANS_GetActiveNotifications_05400
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0054, Function | SmallTest | Level1)
{
    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    sptr<NotificationSlot> socialSlot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    sptr<NotificationSlot> reminderSlot = new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER);
    sptr<NotificationSlot> contentSlot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    sptr<NotificationSlot> otherSlot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    socialSlot->SetSlotGroup(group->GetId());
    reminderSlot->SetSlotGroup(group->GetId());
    contentSlot->SetSlotGroup(group->GetId());
    otherSlot->SetSlotGroup(group->GetId());

    // add slot group
    std::vector<sptr<NotificationSlotGroup>> slotGroups;
    slotGroups.push_back(group);
    g_advancedNotificationService->AddSlotGroups(slotGroups);

    slots.push_back(socialSlot);
    slots.push_back(reminderSlot);
    slots.push_back(contentSlot);
    slots.push_back(otherSlot);

    EXPECT_EQ(g_advancedNotificationService->AddSlots(slots), 0);
}

/**
 * @tc.number    : AnsModuleTest_0055
 * @tc.name      : AMS_ANS_GetActiveNotifications_05500
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0055, Function | SmallTest | Level1)
{
    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    sptr<NotificationSlot> socialSlot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    socialSlot->SetSlotGroup(group->GetId());
    slots.push_back(socialSlot);
    EXPECT_EQ(g_advancedNotificationService->AddSlots(slots), 0);

    EXPECT_EQ(g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION), 0);
}

/**
 * @tc.number    : AnsModuleTest_0056
 * @tc.name      : AMS_ANS_GetActiveNotifications_05600
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0056, Function | SmallTest | Level1)
{
    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    sptr<NotificationSlot> socialSlot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    socialSlot->SetSlotGroup(group->GetId());
    slots.push_back(socialSlot);
    EXPECT_EQ(g_advancedNotificationService->AddSlots(slots), 0);

    // get slot group by groupId
    group = nullptr;
    g_advancedNotificationService->GetSlotGroup("id", group);
    EXPECT_FALSE(group != nullptr);

    // remove slot group
    EXPECT_EQ(g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION), 0);
}

/**
 * @tc.number    : AnsModuleTest_0058
 * @tc.name      : AMS_ANS_GetActiveNotifications_05800
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0058, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification> r, const std::shared_ptr<NotificationSortingMap>) {
        if (r->GetNotificationRequest().GetBadgeNumber() == 1) {
            passed = true;
        }
    };
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot();
    slot->EnableBadge(true);
    slots.push_back(slot);
    EXPECT_EQ(g_advancedNotificationService->AddSlots(slots), 0);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetStatusBarText("text");
    req->SetBadgeNumber(1);

    // SetShowBadgeEnabledForBundle true
    sptr<NotificationBundleOption> bundleOption;
    bundleOption->SetBundleName("bundleName");
    g_advancedNotificationService->SetShowBadgeEnabledForBundle(bundleOption, true);

    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(true, passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0060
 * @tc.name      : AMS_ANS_GetActiveNotifications_06000
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0060, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification> r, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot();
    slot->EnableBadge(true);
    slots.push_back(slot);
    EXPECT_EQ(g_advancedNotificationService->AddSlots(slots), 0);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetStatusBarText("text");
    req->SetBadgeNumber(1);

    // not allow publish notification
    g_advancedNotificationService->SetNotificationsEnabledForBundle("bundleName", false);
    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(false, passed);

    // allow publish
    g_advancedNotificationService->SetNotificationsEnabledForBundle("bundleName", true);
    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(true, passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0061
 * @tc.name      : AMS_ANS_GetActiveNotifications_06100
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0061, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification> r, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot();
    slot->EnableBadge(true);
    slots.push_back(slot);
    EXPECT_EQ(g_advancedNotificationService->AddSlots(slots), 0);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetStatusBarText("text");
    req->SetBadgeNumber(1);

    // allow publish notification
    g_advancedNotificationService->SetNotificationsEnabledForBundle("bundleName", true);
    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(true, passed);

    // not allow publish
    passed = false;
    g_advancedNotificationService->SetNotificationsEnabledForBundle("bundleName", false);
    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(false, passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_062
 * @tc.name      : AMS_ANS_GetActiveNotifications_06200
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0062, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    EXPECT_FALSE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_063
 * @tc.name      : AMS_ANS_GetActiveNotifications_06300
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0063, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    EXPECT_FALSE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
}

/**
 * @tc.number    : AnsModuleTest_064
 * @tc.name      : AMS_ANS_GetActiveNotifications_06400
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0064, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->unSubscriberCb_ = [](NotificationConstant::SubscribeResult) { passed = true; };
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
    EXPECT_TRUE(passed);
}

/**
 * @tc.number    : AnsModuleTest_065
 * @tc.name      : AMS_ANS_GetActiveNotifications_06500
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0065, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->unSubscriberCb_ = [](NotificationConstant::SubscribeResult) { passed = true; };
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
    EXPECT_TRUE(passed);
}

/**
 * @tc.number    : AnsModuleTest_066
 * @tc.name      : AMS_ANS_GetActiveNotifications_06600
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0066, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->canceledCb_ = [](const std::shared_ptr<Notification> &request,
                                  const std::shared_ptr<NotificationSortingMap> &sortingMap,
                                  int deleteReason) { passed = true; };

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);

    // publish request
    g_advancedNotificationService->Publish(label, req);

    // remove request
    g_advancedNotificationService->Delete("1_testLabel_0");
    EXPECT_FALSE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
}

/**
 * @tc.number    : AnsModuleTest_100
 * @tc.name      : AMS_ANS_GetActiveNotifications_10000
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0100, Function | SmallTest | Level1)
{
    // create wantagent
    std::shared_ptr<WantAgent::WantAgent> agent = std::make_shared<WantAgent::WantAgent>();

    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [&agent](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>
                                      sortingMap) { passed = true; };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);

    // set content
    std::shared_ptr<NotificationNormalContent> normalContent = std::make_shared<NotificationNormalContent>();
    normalContent->SetText("1");
    normalContent->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(normalContent);
    req->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(passed, false);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_101
 * @tc.name      : AMS_ANS_GetActiveNotifications_10100
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0101, Function | SmallTest | Level1)
{
    // create wantagent
    std::shared_ptr<WantAgent::WantAgent> agent = std::make_shared<WantAgent::WantAgent>();

    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [&agent](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>
                                      sortingMap) { passed = true; };

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetWantAgent(agent);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(passed, false);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_102
 * @tc.name      : AMS_ANS_GetActiveNotifications_10200
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0102, Function | SmallTest | Level1)
{
    // create wantagent
    std::shared_ptr<WantAgent::WantAgent> agent = std::make_shared<WantAgent::WantAgent>();

    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [&agent](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>
                                      sortingMap) { passed = true; };

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetWantAgent(agent);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(passed, false);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_103
 * @tc.name      : AMS_ANS_GetActiveNotifications_10300
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0103, Function | SmallTest | Level1)
{
    // create wantagent
    std::shared_ptr<WantAgent::WantAgent> agent = std::make_shared<WantAgent::WantAgent>();

    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("a");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [&agent](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>
                                      sortingMap) { passed = true; };

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetWantAgent(agent);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(passed, false);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_104
 * @tc.name      : AMS_ANS_GetActiveNotifications_10400
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0104, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetStatusBarText("text");
    req->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);

    g_advancedNotificationService->SetDisturbMode(NotificationConstant::DisturbMode::ALLOW_NONE);

    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(false, passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_105
 * @tc.name      : AMS_ANS_GetActiveNotifications_10500
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0105, Function | SmallTest | Level1)
{

    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> socialSlot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    sptr<NotificationSlot> reminderSlot = new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER);
    sptr<NotificationSlot> contentSlot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    sptr<NotificationSlot> otherSlot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    sptr<NotificationSlot> customSlot = new NotificationSlot(NotificationConstant::SlotType::CUSTOM);

    slots.push_back(socialSlot);
    slots.push_back(reminderSlot);
    slots.push_back(contentSlot);
    slots.push_back(otherSlot);
    slots.push_back(customSlot);

    g_advancedNotificationService->AddSlots(slots);
    EXPECT_EQ(0, g_advancedNotificationService->AddSlots(slots));
}

/**
 * @tc.number    : AnsModuleTest_106
 * @tc.name      : AMS_ANS_GetActiveNotifications_10600
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0106, Function | SmallTest | Level1)
{
    // create wantagent
    std::shared_ptr<WantAgent::WantAgent> agent = std::make_shared<WantAgent::WantAgent>();

    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            passed = true;
        };

    // set disturb mode
    g_advancedNotificationService->SetNotificationsEnabledForBundle("bundleName", false);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetWantAgent(agent);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(passed, false);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_107
 * @tc.name      : AMS_ANS_GetActiveNotifications_10700
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0107, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);

    // set content
    std::shared_ptr<NotificationNormalContent> normalContent = std::make_shared<NotificationNormalContent>();
    normalContent->SetText("1");
    normalContent->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(normalContent);
    req->SetContent(content);
    req1->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);

    // remove request
    g_advancedNotificationService->Delete("1_testLabel_0");
    g_advancedNotificationService->Delete("1_testLabel_1");
    int nums = -1;
    g_advancedNotificationService->GetActiveNotificationNums(nums);
    EXPECT_EQ(nums, 0);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_108
 * @tc.name      : AMS_ANS_GetActiveNotifications_10800
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0108, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);

    // set content
    std::shared_ptr<NotificationNormalContent> normalContent = std::make_shared<NotificationNormalContent>();
    normalContent->SetText("1");
    normalContent->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(normalContent);
    req->SetContent(content);
    req1->SetContent(content);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);

    // remove request
    g_advancedNotificationService->DeleteAll();
    int nums = -1;
    g_advancedNotificationService->GetActiveNotificationNums(nums);
    EXPECT_EQ(nums, 0);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_110
 * @tc.name      : AMS_ANS_GetActiveNotifications_11000
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0110, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    subscriber->unSubscriberCb_ = [](NotificationConstant::SubscribeResult) { passed = true; };
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);

    // unsubscriber
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
    EXPECT_EQ(passed, true);
}

/**
 * @tc.number    : AnsModuleTest_111
 * @tc.name      : AMS_ANS_GetActiveNotifications_11100
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0111, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    subscriberInfo->AddAppName("bundleName");
    subscriber->subscriberCb_ = [](NotificationConstant::SubscribeResult) { passed = true; };
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
    EXPECT_EQ(passed, true);
}

/**
 * @tc.number    : AnsModuleTest_112
 * @tc.name      : AMS_ANS_GetActiveNotifications_11200
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0112, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking() && notification->EnableLight() &&
                notification->EnableSound() && notification->EnableVibrate()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slot->SetSound(Uri("."));
    slot->SetEnableLight(true);
    slot->SetEnableVibration(true);
    slot->SetVibrationStyle(std::vector<int64_t>(1, 1));
    slot->SetLedLightColor(1);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);
    std::shared_ptr<NotificationMultiLineContent> contentImpl = std::make_shared<NotificationMultiLineContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req1->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    req1->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);

    // publish request
    EXPECT_FALSE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
}

/**
 * @tc.number    : AnsModuleTest_113
 * @tc.name      : AMS_ANS_GetActiveNotifications_11300
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0113, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking() && notification->EnableLight() &&
                notification->EnableSound() && notification->EnableVibrate()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slot->SetSound(Uri("."));
    slot->SetEnableLight(true);
    slot->SetEnableVibration(true);
    slot->SetVibrationStyle(std::vector<int64_t>(1, 1));
    slot->SetLedLightColor(1);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);
    std::shared_ptr<NotificationLongTextContent> contentImpl = std::make_shared<NotificationLongTextContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req1->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::OTHER);
    req1->SetSlotType(NotificationConstant::SlotType::OTHER);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
    EXPECT_FALSE(passed);
}

/**
 * @tc.number    : AnsModuleTest_114
 * @tc.name      : AMS_ANS_GetActiveNotifications_11400
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0114, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking() && notification->EnableLight() &&
                notification->EnableSound() && notification->EnableVibrate()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slot->SetSound(Uri("."));
    slot->SetEnableLight(true);
    slot->SetEnableVibration(true);
    slot->SetLockscreenVisibleness(NotificationConstant::VisiblenessType::PUBLIC);
    slot->SetLedLightColor(1);
    slot->SetVibrationStyle(std::vector<int64_t>(1, 1));
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);
    std::shared_ptr<NotificationNormalContent> contentImpl = std::make_shared<NotificationNormalContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req1->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::CONTENT_INFORMATION);
    req1->SetSlotType(NotificationConstant::SlotType::CONTENT_INFORMATION);
    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
    EXPECT_FALSE(passed);
}

/**
 * @tc.number    : AnsModuleTest_116
 * @tc.name      : AMS_ANS_GetActiveNotifications_11600
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0116, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking() && notification->EnableLight() &&
                notification->EnableSound() && notification->EnableVibrate()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER);
    slot->SetSound(Uri("."));
    slot->SetEnableLight(true);
    slot->SetEnableVibration(true);
    slot->SetLockscreenVisibleness(NotificationConstant::VisiblenessType::PUBLIC);
    slot->SetLedLightColor(1);
    slot->SetVibrationStyle(std::vector<int64_t>(1, 1));
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);
    std::shared_ptr<NotificationNormalContent> contentImpl = std::make_shared<NotificationNormalContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req1->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::SERVICE_REMINDER);
    req1->SetSlotType(NotificationConstant::SlotType::SERVICE_REMINDER);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    EXPECT_FALSE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
}

/**
 * @tc.number    : AnsModuleTest_117
 * @tc.name      : AMS_ANS_GetActiveNotifications_11700
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0117, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            std::vector<std::string> sortingKey = sortingMap->GetKey();

            NotificationSorting sorting1;
            NotificationSorting sorting2;
            if (sortingKey.size() == 2) {
                sortingMap->GetNotificationSorting("1_testLabel_0", sorting1);
                sortingMap->GetNotificationSorting("1_testLabel_1", sorting2);
            }
            if (sorting1.GetRanking() < sorting2.GetRanking() && notification->EnableLight() &&
                notification->EnableSound() && notification->EnableVibrate()) {
                passed = true;
            }
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slot->SetSound(Uri("."));
    slot->SetEnableLight(true);
    slot->SetEnableVibration(true);
    slot->SetLockscreenVisibleness(NotificationConstant::VisiblenessType::PUBLIC);
    slot->SetLedLightColor(1);
    slot->SetVibrationStyle(std::vector<int64_t>(1, 1));
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);
    std::shared_ptr<NotificationNormalContent> contentImpl = std::make_shared<NotificationNormalContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req1->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    req1->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Publish(label, req1);
    EXPECT_FALSE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
}

/**
 * @tc.number    : AnsModuleTest_118
 * @tc.name      : AMS_ANS_GetActiveNotifications_11800
 * @tc.desc      : Test publish notifications when Disturb are not allowed publish.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0118, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetStatusBarText("text");
    std::shared_ptr<NotificationNormalContent> contentImpl = std::make_shared<NotificationNormalContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);

    g_advancedNotificationService->SetDisturbMode(NotificationConstant::DisturbMode::ALLOW_NONE);

    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(false, passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_120
 * @tc.name      : AMS_ANS_GetActiveNotifications_12000
 * @tc.desc      : Test publish notifications when Disturb are not allowed publish.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0120, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    sptr<NotificationSubscribeInfo> subscriberInfo = new NotificationSubscribeInfo();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), subscriberInfo);
    subscriber->consumedCb_ = [](const std::shared_ptr<Notification>, const std::shared_ptr<NotificationSortingMap>) {
        passed = true;
    };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot0 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot0);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetStatusBarText("text");
    std::shared_ptr<NotificationNormalContent> contentImpl = std::make_shared<NotificationNormalContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);

    g_advancedNotificationService->SetNotificationsEnabledForBundle("bundleName", false);

    g_advancedNotificationService->Publish(label, req);
    EXPECT_EQ(false, passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), subscriberInfo);
}

/**
 * @tc.number    : AnsModuleTest_0121
 * @tc.name      : AMS_ANS_GetActiveNotifications_12100
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0121, Function | SmallTest | Level1)
{
    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> socialSlot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    sptr<NotificationSlot> reminderSlot = new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER);
    sptr<NotificationSlot> contentSlot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    sptr<NotificationSlot> otherSlot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    sptr<NotificationSlot> customSlot = new NotificationSlot(NotificationConstant::SlotType::CUSTOM);

    slots.push_back(socialSlot);
    slots.push_back(reminderSlot);
    slots.push_back(contentSlot);
    slots.push_back(otherSlot);
    slots.push_back(customSlot);

    EXPECT_EQ(g_advancedNotificationService->AddSlots(slots), 0);
    EXPECT_EQ(g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION), 0);
    EXPECT_EQ(g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::SERVICE_REMINDER), 0);
    EXPECT_EQ(g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::CONTENT_INFORMATION), 0);
    EXPECT_EQ(g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::OTHER), 0);
    EXPECT_EQ(g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::CUSTOM), 0);
}

/**
 * @tc.number    : AnsModuleTest_0122
 * @tc.name      : AMS_ANS_GetActiveNotifications_12200
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0122, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            passed = true;
        };

    subscriber->canceledCb_ = [](const std::shared_ptr<Notification> &request,
                                  const std::shared_ptr<NotificationSortingMap> &sortingMap,
                                  int deleteReason) { passed = true; };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req1->SetLabel(label);
    req->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    req1->SetSlotType(NotificationConstant::SlotType::OTHER);

    // publish request

    // remove social slot
    EXPECT_EQ(0, g_advancedNotificationService->RemoveSlotByType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION));

    // add slot
    std::vector<sptr<NotificationSlot>> otherSlots;
    slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    otherSlots.push_back(slot);
    EXPECT_EQ(0, g_advancedNotificationService->AddSlots(otherSlots));

    EXPECT_FALSE(passed);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
}

/**
 * @tc.number    : AnsModuleTest_0123
 * @tc.name      : AMS_ANS_GetActiveNotifications_12300
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0123, Function | SmallTest | Level1)
{
    int ret = 0;
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ = [&ret](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>
                                      sortingMap) { ret++; };
    subscriber->canceledCb_ = [](const std::shared_ptr<Notification> &request,
                                  const std::shared_ptr<NotificationSortingMap> &sortingMap,
                                  int deleteReason) { passed = true; };
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION));
    slots.push_back(new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER));
    slots.push_back(new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION));
    slots.push_back(new NotificationSlot(NotificationConstant::SlotType::OTHER));
    slots.push_back(new NotificationSlot(NotificationConstant::SlotType::CUSTOM));
    EXPECT_EQ(g_advancedNotificationService->AddSlots(slots), 0);
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    sptr<NotificationRequest> req2 = new NotificationRequest(2);
    sptr<NotificationRequest> req3 = new NotificationRequest(3);
    sptr<NotificationRequest> req4 = new NotificationRequest(4);
    req->SetLabel("testLabel");
    req1->SetLabel("testLabel");
    req2->SetLabel("testLabel");
    req3->SetLabel("testLabel");
    req4->SetLabel("testLabel");

    req->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    req1->SetSlotType(NotificationConstant::SlotType::SERVICE_REMINDER);
    req2->SetSlotType(NotificationConstant::SlotType::CONTENT_INFORMATION);
    req3->SetSlotType(NotificationConstant::SlotType::OTHER);
    req4->SetSlotType(NotificationConstant::SlotType::CUSTOM);

    g_advancedNotificationService->Publish("testLabel", req);
    EXPECT_EQ(ret, 0);
    g_advancedNotificationService->Publish("testLabel", req1);
    EXPECT_EQ(ret, 0);
    g_advancedNotificationService->Publish("testLabel", req2);
    EXPECT_EQ(ret, 0);
    g_advancedNotificationService->Publish("testLabel", req3);
    EXPECT_EQ(ret, 0);
    g_advancedNotificationService->Publish("testLabel", req4);
    EXPECT_EQ(ret, 0);
    g_advancedNotificationService->DeleteAll();
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
    EXPECT_FALSE(passed);
}

/**
 * @tc.number    : AnsModuleTest_0124
 * @tc.name      : AMS_ANS_GetActiveNotifications_12400
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0124, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            passed = true;
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationMediaContent> contentImpl = std::make_shared<NotificationMediaContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::CONTENT_INFORMATION);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
    EXPECT_FALSE(passed);
}

/**
 * @tc.number    : AnsModuleTest_0125
 * @tc.name      : AMS_ANS_GetActiveNotifications_12500
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0125, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            passed = true;
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    req->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
    EXPECT_FALSE(passed);
}

/**
 * @tc.number    : AnsModuleTest_0126
 * @tc.name      : AMS_ANS_GetActiveNotifications_12600
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0126, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            passed = true;
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER);
    slots.push_back(slot);
    g_advancedNotificationService->AddSlots(slots);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetLabel(label);
    std::shared_ptr<NotificationPictureContent> contentImpl = std::make_shared<NotificationPictureContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> content = std::make_shared<NotificationContent>(contentImpl);
    req->SetContent(content);
    req->SetSlotType(NotificationConstant::SlotType::SERVICE_REMINDER);

    // publish request
    g_advancedNotificationService->Publish(label, req);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
    EXPECT_FALSE(passed);
}

/**
 * @tc.number    : AnsModuleTest_0127
 * @tc.name      : AMS_ANS_GetActiveNotifications_12700
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0127, Function | SmallTest | Level1)
{
    int ret = 0;
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ = [&ret](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>
                                      sortingMap) { ret++; };

    // add slot
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    sptr<NotificationSlot> slot1 = new NotificationSlot(NotificationConstant::SlotType::SERVICE_REMINDER);
    slots.push_back(slot);
    slots.push_back(slot1);
    slot->SetSlotGroup(group->GetId());
    g_advancedNotificationService->AddSlots(slots);

    // add slot group
    std::vector<sptr<NotificationSlotGroup>> slotGroups;
    slotGroups.push_back(group);
    g_advancedNotificationService->AddSlotGroups(slotGroups);

    // create content
    std::shared_ptr<NotificationPictureContent> contentImpl = std::make_shared<NotificationPictureContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> pictureContent = std::make_shared<NotificationContent>(contentImpl);

    std::shared_ptr<NotificationLongTextContent> contentImpl1 = std::make_shared<NotificationLongTextContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> longTextContent = std::make_shared<NotificationContent>(contentImpl);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req->SetContent(pictureContent);
    req1->SetLabel(label);
    req1->SetContent(longTextContent);
    req->SetSlotType(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    req1->SetSlotType(NotificationConstant::SlotType::SERVICE_REMINDER);

    // publish
    EXPECT_EQ(0, ret);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
}

/**
 * @tc.number    : AnsModuleTest_0128
 * @tc.name      : AMS_ANS_GetActiveNotifications_12800
 * @tc.desc      : Test publish notification when slot type is CONTENT_INFORMATION.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0128, Function | SmallTest | Level1)
{
    int ret = 0;
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ = [&ret](const std::shared_ptr<Notification> notification,
                                  const std::shared_ptr<NotificationSortingMap>
                                      sortingMap) { ret++; };

    // add slot
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    sptr<NotificationSlot> slot1 = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slots.push_back(slot);
    slots.push_back(slot1);
    slot->SetSlotGroup(group->GetId());
    g_advancedNotificationService->AddSlots(slots);

    // add slot group
    std::vector<sptr<NotificationSlotGroup>> slotGroups;
    slotGroups.push_back(group);
    g_advancedNotificationService->AddSlotGroups(slotGroups);

    // create content
    std::shared_ptr<NotificationPictureContent> contentImpl = std::make_shared<NotificationPictureContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> pictureContent = std::make_shared<NotificationContent>(contentImpl);

    std::shared_ptr<NotificationLongTextContent> contentImpl1 = std::make_shared<NotificationLongTextContent>();
    contentImpl->SetText("1");
    contentImpl->SetTitle("1");
    std::shared_ptr<NotificationContent> longTextContent = std::make_shared<NotificationContent>(contentImpl);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    sptr<NotificationRequest> req1 = new NotificationRequest(1);
    req->SetLabel(label);
    req->SetContent(pictureContent);
    req1->SetLabel(label);
    req1->SetContent(longTextContent);
    req->SetSlotType(NotificationConstant::SlotType::CONTENT_INFORMATION);
    req1->SetSlotType(NotificationConstant::SlotType::OTHER);

    // publish
    EXPECT_EQ(0, ret);
    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
}

/**
 * @tc.number    : AnsModuleTest_0130
 * @tc.name      : AMS_ANS_GetActiveNotifications_13000
 * @tc.desc      : Test publish notification when slot type is OTHER.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0130, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->consumedCb_ =
        [](const std::shared_ptr<Notification> notification, const std::shared_ptr<NotificationSortingMap> sortingMap) {
            EXPECT_EQ(false, notification->EnableVibrate());
            EXPECT_EQ(NotificationConstant::VisiblenessType::PUBLIC, notification->GetLockscreenVisibleness());
        };

    // add slot
    std::vector<sptr<NotificationSlot>> slots;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slots.push_back(slot);
    slot->SetLockscreenVisibleness(NotificationConstant::VisiblenessType::PUBLIC);
    slot->SetEnableVibration(true);

    // create request
    std::string label = "testLabel";
    sptr<NotificationRequest> req = new NotificationRequest(0);
    req->SetSlotType(NotificationConstant::SlotType::OTHER);
    req->SetLabel(label);
    // publish

    g_advancedNotificationService->Unsubscribe(subscriber->GetImpl(), nullptr);
}

/**
 * @tc.number    : AnsModuleTest_0131
 * @tc.name      : AMS_ANS_GetActiveNotifications_13100
 * @tc.desc      : Test publish notification when cancel a  notification.
 */
HWTEST_F(AnsModuleTest, AnsModuleTest_0131, Function | SmallTest | Level1)
{
    // subscriber
    auto subscriber = new TestAnsSubscriber();
    g_advancedNotificationService->Subscribe(subscriber->GetImpl(), nullptr);
    subscriber->canceledCb_ = [](const std::shared_ptr<Notification> &request,
                                  const std::shared_ptr<NotificationSortingMap> &sortingMap,
                                  int deleteReason) { passed = true; };
    //
    g_advancedNotificationService->Cancel(1, "1");
    EXPECT_EQ(false, passed);
}

}  // namespace Notification
}  // namespace OHOS
