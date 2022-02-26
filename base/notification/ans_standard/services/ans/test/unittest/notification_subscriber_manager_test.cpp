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
#include <iostream>

#define private public
#include "notification_subscriber.h"
#include "notification_subscriber_manager.h"

#include "ans_inner_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace Notification {

class NotificationSubscriberManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    class TestAnsSubscriber : public NotificationSubscriber {
    public:
        void OnSubscribeResult(NotificationConstant::SubscribeResult result) override
        {}
        void OnUnsubscribeResult(NotificationConstant::SubscribeResult result) override
        {}
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
        {}
        void OnConsumed(const std::shared_ptr<Notification> &request) override
        {}
        void OnConsumed(const std::shared_ptr<Notification> &request,
            const std::shared_ptr<NotificationSortingMap> &sortingMap) override
        {}
    };

    static std::shared_ptr<NotificationSubscriberManager> notificationSubscriberManager_;
    static TestAnsSubscriber testAnsSubscriber_;
    static sptr<IAnsSubscriber> subscriber_;
};

std::shared_ptr<NotificationSubscriberManager> NotificationSubscriberManagerTest::notificationSubscriberManager_ =
    nullptr;
NotificationSubscriberManagerTest::TestAnsSubscriber NotificationSubscriberManagerTest::testAnsSubscriber_;
sptr<IAnsSubscriber> NotificationSubscriberManagerTest::subscriber_ = nullptr;

void NotificationSubscriberManagerTest::SetUpTestCase()
{
    notificationSubscriberManager_ = NotificationSubscriberManager::GetInstance();
    subscriber_ = testAnsSubscriber_.GetImpl();
}

void NotificationSubscriberManagerTest::TearDownTestCase()
{
    subscriber_ = nullptr;
    notificationSubscriberManager_ = nullptr;
}

void NotificationSubscriberManagerTest::SetUp()
{
    notificationSubscriberManager_->AddSubscriber(subscriber_, nullptr);
}

void NotificationSubscriberManagerTest::TearDown()
{
    notificationSubscriberManager_->RemoveSubscriber(subscriber_, nullptr);
}

/**
 * @tc.number    : NotificationSubscriberManagerTest_001
 * @tc.name      : AMS_ANS_NotifyUpdated_0100
 * @tc.desc      : Test NotifyUpdated function.
 */
HWTEST_F(NotificationSubscriberManagerTest, NotificationSubscriberManagerTest_001, Function | SmallTest | Level1)
{
    const std::vector<NotificationSorting> sortingList;
    sptr<NotificationSortingMap> map = new NotificationSortingMap(sortingList);
    notificationSubscriberManager_->NotifyUpdated(map);
}

/**
 * @tc.number    : NotificationSubscriberManagerTest_002
 * @tc.name      : AMS_ANS_NotifyDisturbModeChanged_0100
 * @tc.desc      : Test NotifyDisturbModeChanged function.
 */
HWTEST_F(NotificationSubscriberManagerTest, NotificationSubscriberManagerTest_002, Function | SmallTest | Level1)
{
    NotificationConstant::DisturbMode mode;
    notificationSubscriberManager_->NotifyDisturbModeChanged(mode);
}

/**
 * @tc.number    : NotificationSubscriberManagerTest_003
 * @tc.name      : AMS_ANS_NotifyConsumed_0100
 * @tc.desc      : Test NotifyConsumed function.
 */
HWTEST_F(NotificationSubscriberManagerTest, NotificationSubscriberManagerTest_003, Function | SmallTest | Level1)
{
    std::vector<NotificationSorting> sortingList;
    sptr<NotificationRequest> request = new NotificationRequest();
    sptr<Notification> notification = new Notification(request);
    sptr<NotificationSortingMap> notificationMap = new NotificationSortingMap(sortingList);
    notificationSubscriberManager_->NotifyConsumed(notification, notificationMap);
}

/**
 * @tc.number    : NotificationSubscriberManagerTest_004
 * @tc.name      : AMS_ANS_NotifyCanceled_0100
 * @tc.desc      : Test NotifyCanceled function.
 */
HWTEST_F(NotificationSubscriberManagerTest, NotificationSubscriberManagerTest_004, Function | SmallTest | Level1)
{
    std::vector<NotificationSorting> sortingList;
    sptr<NotificationRequest> request = new NotificationRequest();
    sptr<Notification> notification = new Notification(request);
    sptr<NotificationSortingMap> notificationMap = new NotificationSortingMap(sortingList);
    int deleteReason = 0;
    notificationSubscriberManager_->NotifyCanceled(notification, notificationMap, deleteReason);
}

/**
 * @tc.number    : NotificationSubscriberManagerTest_005
 * @tc.name      : AMS_ANS_AddSubscriber_0100
 * @tc.desc      : Test AddSubscriber function, return is ERR_OK.
 */
HWTEST_F(NotificationSubscriberManagerTest, NotificationSubscriberManagerTest_005, Function | SmallTest | Level1)
{
    sptr<NotificationSubscribeInfo> info = new NotificationSubscribeInfo();
    info->AddAppName("test_bundle");
    EXPECT_EQ(notificationSubscriberManager_->AddSubscriber(subscriber_, info), (int)ERR_OK);
    EXPECT_EQ(notificationSubscriberManager_->AddSubscriber(subscriber_, nullptr), (int)ERR_OK);
}

/**
 * @tc.number    : NotificationSubscriberManagerTest_006
 * @tc.name      : AMS_ANS_AddSubscriber_0100
 * @tc.desc      : Test AddSubscriber function when subscriber is nullptr, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationSubscriberManagerTest, NotificationSubscriberManagerTest_006, Function | SmallTest | Level1)
{
    sptr<NotificationSubscribeInfo> info = new NotificationSubscribeInfo();
    EXPECT_EQ(notificationSubscriberManager_->AddSubscriber(nullptr, info), (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : NotificationSubscriberManagerTest_007
 * @tc.name      : AMS_ANS_RemoveSubscriber_0100
 * @tc.desc      : Test RemoveSubscriber function, return is ERR_OK.
 */
HWTEST_F(NotificationSubscriberManagerTest, NotificationSubscriberManagerTest_007, Function | SmallTest | Level1)
{
    sptr<NotificationSubscribeInfo> info = new NotificationSubscribeInfo();
    info->AddAppName("test_bundle");
    EXPECT_EQ(notificationSubscriberManager_->RemoveSubscriber(subscriber_, info), (int)ERR_OK);
}

/**
 * @tc.number    : NotificationSubscriberManagerTest_006
 * @tc.name      : AMS_ANS_AddSubscriber_0100
 * @tc.desc      : Test RemoveSubscriber function when subscriber is nullptr, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationSubscriberManagerTest, NotificationSubscriberManagerTest_008, Function | SmallTest | Level1)
{
    sptr<NotificationSubscribeInfo> info = new NotificationSubscribeInfo();
    EXPECT_EQ(notificationSubscriberManager_->RemoveSubscriber(nullptr, info), (int)ERR_ANS_INVALID_PARAM);
}
}  // namespace Notification
}  // namespace OHOS