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

#include "notification_slot_filter.h"

using namespace testing::ext;
namespace OHOS {
namespace Notification {

class NotificationSlotFilterTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};
};

/**
 * @tc.number    : NotificationSlotFilterTest_00100
 * @tc.name      : AMS_ANS_OnStart_0100
 * @tc.desc      : Test OnStart function
 */
HWTEST_F(NotificationSlotFilterTest, NotificationSlotFilterTest_00100, Function | SmallTest | Level1)
{
    NotificationSlotFilter notificationSlotFilter;
    notificationSlotFilter.OnStart();
}

/**
 * @tc.number    : NotificationSlotFilterTest_00200
 * @tc.name      : AMS_ANS_OnStop_0100
 * @tc.desc      : Test OnStop function
 */
HWTEST_F(NotificationSlotFilterTest, NotificationSlotFilterTest_00200, Function | SmallTest | Level1)
{
    NotificationSlotFilter notificationSlotFilter;
    notificationSlotFilter.OnStop();
}

/**
 * @tc.number    : NotificationSlotFilterTest_00300
 * @tc.name      : AMS_ANS_OnPublish_0100
 * @tc.desc      : Test OnPublish function
 */
HWTEST_F(NotificationSlotFilterTest, NotificationSlotFilterTest_00300, Function | SmallTest | Level1)
{
    NotificationSlotFilter notificationSlotFilter;
    std::shared_ptr<NotificationRecord> record = std::make_shared<NotificationRecord>();
    record->request = new NotificationRequest();
    record->notification = new Notification(record->request);
    record->slot = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    notificationSlotFilter.OnPublish(record);
}
}  // namespace Notification
}  // namespace OHOS