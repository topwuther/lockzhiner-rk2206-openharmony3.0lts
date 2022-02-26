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

#include "ans_inner_errors.h"
#include "ans_ut_constant.h"
#include "notification_preferences.h"
#include "notification_slot.h"
#include "permission_filter.h"
#include "string_ex.h"

using namespace testing::ext;
namespace OHOS {
namespace Notification {

class PermissionFilterTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp();
    void TearDown();
};

void PermissionFilterTest::SetUp()
{
    NotificationPreferences::GetInstance().ClearNotificationInRestoreFactorySettings();
}

void PermissionFilterTest::TearDown()
{
    NotificationPreferences::GetInstance().ClearNotificationInRestoreFactorySettings();
}

/**
 * @tc.number    : PermissionFilterTest_00100
 * @tc.name      : AMS_ANS_OnStop_0100
 * @tc.desc      : Test OnStart function
 */
HWTEST_F(PermissionFilterTest, PermissionFilterTest_00100, Function | SmallTest | Level1)
{
    PermissionFilter permissionFilter;
    permissionFilter.OnStart();
}

/**
 * @tc.number    : PermissionFilterTest_00200
 * @tc.name      : AMS_ANS_OnStop_0100
 * @tc.desc      : Test OnStop function
 */
HWTEST_F(PermissionFilterTest, PermissionFilterTest_00200, Function | SmallTest | Level1)
{
    PermissionFilter permissionFilter;
    permissionFilter.OnStop();
}

/**
 * @tc.number    : PermissionFilterTest_00400
 * @tc.name      :
 * @tc.desc      : Test OnPublish function.Expect return ERR_OK
 */
HWTEST_F(PermissionFilterTest, PermissionFilterTest_00400, Function | SmallTest | Level1)
{
    PermissionFilter permissionFilter;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::OTHER);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    NotificationPreferences::GetInstance().AddNotificationSlots(
        new NotificationBundleOption(TEST_DEFUALT_BUNDLE, SYSTEM_APP_UID), slots);
    std::shared_ptr<NotificationRecord> record = std::make_shared<NotificationRecord>();
    record->bundleOption = new NotificationBundleOption(TEST_DEFUALT_BUNDLE, SYSTEM_APP_UID);
    record->request = new NotificationRequest();
    record->request->SetOwnerBundleName(TEST_DEFUALT_BUNDLE);
    record->notification = new Notification(record->request);
    record->slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    EXPECT_EQ(permissionFilter.OnPublish(record), ERR_OK);
}

/**
 * @tc.number    : PermissionFilterTest_00500
 * @tc.name      :
 * @tc.desc      : Test OnPublish function when NotificationsEnabled is false.Expect return ERR_ANS_NOT_ALLOWED
 */
HWTEST_F(PermissionFilterTest, PermissionFilterTest_00500, Function | SmallTest | Level1)
{
    PermissionFilter permissionFilter;
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::OTHER);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    NotificationPreferences::GetInstance().AddNotificationSlots(
        new NotificationBundleOption(TEST_DEFUALT_BUNDLE, SYSTEM_APP_UID), slots);
    NotificationPreferences::GetInstance().SetNotificationsEnabledForBundle(
        new NotificationBundleOption(TEST_DEFUALT_BUNDLE, SYSTEM_APP_UID), false);
    std::shared_ptr<NotificationRecord> record = std::make_shared<NotificationRecord>();
    record->bundleOption = new NotificationBundleOption(TEST_DEFUALT_BUNDLE, SYSTEM_APP_UID);
    record->request = new NotificationRequest();
    record->request->SetOwnerBundleName(TEST_DEFUALT_BUNDLE);
    record->notification = new Notification(record->request);
    record->slot = slot;

    EXPECT_EQ((int)permissionFilter.OnPublish(record), (int)ERR_ANS_NOT_ALLOWED);
}
}  // namespace Notification
}  // namespace OHOS