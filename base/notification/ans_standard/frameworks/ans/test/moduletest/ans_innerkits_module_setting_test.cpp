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
#include <functional>

#include "ans_inner_errors.h"
#include "ans_manager_proxy.h"
#include "advanced_notification_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "notification_helper.h"
#include "mock_bundle_manager.h"
#include "mock_ipc_skeleton.h"
#include "system_ability_definition.h"

using namespace testing::ext;
namespace OHOS {
namespace Notification {

static sptr<ISystemAbilityManager> systemAbilityManager =
    SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
const int32_t CALLING_UID = 9998;

class AnsInterfaceModuleSettingTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AnsInterfaceModuleSettingTest::SetUpTestCase()
{
    sptr<AdvancedNotificationService> service = OHOS::Notification::AdvancedNotificationService::GetInstance();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::ADVANCED_NOTIFICATION_SERVICE_ABILITY_ID, service, saExtraProp);
    IPCSkeleton::SetCallingUid(CALLING_UID);
}

void AnsInterfaceModuleSettingTest::TearDownTestCase()
{}

void AnsInterfaceModuleSettingTest::SetUp()
{}

void AnsInterfaceModuleSettingTest::TearDown()
{}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSetting_00100
 * @tc.name      : NotificationSetting_00100
 * @tc.desc      : Set a specified application to show badge, get the specified application can show badge.
 * @tc.expected  : Set a specified application to show badge success, get the specified application can show badge.
 */
HWTEST_F(AnsInterfaceModuleSettingTest, ANS_Interface_MT_NotificationSetting_00100, Function | MediumTest | Level1)
{
    NotificationBundleOption bundleOption;
    bundleOption.SetBundleName("bundlename");
    bundleOption.SetUid(CALLING_UID);
    GTEST_LOG_(INFO) << "BundleOption is:"<<bundleOption.Dump();
    EXPECT_EQ(0, NotificationHelper::SetShowBadgeEnabledForBundle(bundleOption, true));
    bool enabled = false;
    EXPECT_EQ(0, NotificationHelper::GetShowBadgeEnabledForBundle(bundleOption, enabled));
    EXPECT_EQ(true,enabled);
    EXPECT_EQ("bundlename", bundleOption.GetBundleName());
    EXPECT_EQ(CALLING_UID, bundleOption.GetUid());
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSetting_00200
 * @tc.name      : NotificationSetting_00100
 * @tc.desc      : Set a specified application do not show badge, get the specified application can not show badge.
 * @tc.expected  : Set a specified application do not show badge success, get the specified application can not show
 *                 badge.
 */
HWTEST_F(AnsInterfaceModuleSettingTest, ANS_Interface_MT_NotificationSetting_00200, Function | MediumTest | Level1)
{
    NotificationBundleOption bundleOption("bundlename", CALLING_UID);
    EXPECT_EQ(0, NotificationHelper::SetShowBadgeEnabledForBundle(bundleOption, false));
    bool enabled = false;
    EXPECT_EQ(0, NotificationHelper::GetShowBadgeEnabledForBundle(bundleOption, enabled));
    EXPECT_EQ(false,enabled);
    EXPECT_EQ("bundlename", bundleOption.GetBundleName());
    EXPECT_EQ(CALLING_UID, bundleOption.GetUid());
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSetting_00300
 * @tc.name      : NotificationSetting_00300
 * @tc.desc      : Set a specified application can publish notification, get the specified application can publish
 *                 notification.
 * @tc.expected  : Set a specified application can publish notification success, get the specified application can
 *                 publish notification.
 */
HWTEST_F(AnsInterfaceModuleSettingTest, ANS_Interface_MT_NotificationSetting_00300, Function | MediumTest | Level1)
{
    NotificationBundleOption bundleOption("bundlename", CALLING_UID);
    std::string deviceId;
    EXPECT_EQ(0, NotificationHelper::SetNotificationsEnabledForSpecifiedBundle(bundleOption, deviceId, true));
    bool enabled = false;
    EXPECT_EQ(0, NotificationHelper::IsAllowedNotify(bundleOption, enabled));
    EXPECT_EQ(true,enabled);
    EXPECT_EQ("bundlename", bundleOption.GetBundleName());
    EXPECT_EQ(CALLING_UID, bundleOption.GetUid());
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSetting_00400
 * @tc.name      : NotificationSetting_00400
 * @tc.desc      : Set a specified application do not publish notification, get the specified application can not publish
 *                 notification.
 * @tc.expected  : Set a specified application do not publish notification success, get the specified application can
 *                 not publish notification.
 */
HWTEST_F(AnsInterfaceModuleSettingTest, ANS_Interface_MT_NotificationSetting_00400, Function | MediumTest | Level1)
{
    NotificationBundleOption bundleOption("bundlename", CALLING_UID);
    std::string deviceId;
    EXPECT_EQ(0, NotificationHelper::SetNotificationsEnabledForSpecifiedBundle(bundleOption, deviceId, false));
    bool enabled = false;
    EXPECT_EQ(0, NotificationHelper::IsAllowedNotify(bundleOption, enabled));
    EXPECT_EQ(false,enabled);
    EXPECT_EQ("bundlename", bundleOption.GetBundleName());
    EXPECT_EQ(CALLING_UID, bundleOption.GetUid());
}
}  // namespace Notification
}  // namespace OHOS