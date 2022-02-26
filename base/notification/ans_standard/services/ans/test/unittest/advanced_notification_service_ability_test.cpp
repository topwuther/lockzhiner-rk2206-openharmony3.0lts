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

#include "advanced_notification_service_ability.h"

using namespace testing::ext;

namespace OHOS {
namespace Notification {

class AdvancedNotificationServiceAbilityTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};
};

/**
 * @tc.number    : AdvancedNotificationServiceAbilityTest_00100
 * @tc.name      : AMS_ANS_AdvancedNotificationServiceAbility_0100
 * @tc.desc      : Structure AdvancedNotificationServiceAbility with systemAbilityId and runOnCreate
 */
HWTEST_F(
    AdvancedNotificationServiceAbilityTest, AdvancedNotificationServiceAbilityTest_00100, Function | SmallTest | Level1)
{
    int32_t systemAbilityId = 1;
    bool runOnCreate = true;
    AdvancedNotificationServiceAbility(systemAbilityId, runOnCreate);
}

}  // namespace Notification
}  // namespace OHOS
