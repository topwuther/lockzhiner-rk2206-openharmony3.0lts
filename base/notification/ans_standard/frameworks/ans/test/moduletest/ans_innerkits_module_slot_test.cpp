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

class AnsInterfaceModuleSlotTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AnsInterfaceModuleSlotTest::SetUpTestCase()
{
    sptr<AdvancedNotificationService> service = OHOS::Notification::AdvancedNotificationService::GetInstance();
    OHOS::ISystemAbilityManager::SAExtraProp saExtraProp;
    systemAbilityManager->AddSystemAbility(OHOS::ADVANCED_NOTIFICATION_SERVICE_ABILITY_ID, service, saExtraProp);
    IPCSkeleton::SetCallingUid(CALLING_UID);
}

void AnsInterfaceModuleSlotTest::TearDownTestCase()
{}

void AnsInterfaceModuleSlotTest::SetUp()
{}

void AnsInterfaceModuleSlotTest::TearDown()
{}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSlot_00100
 * @tc.name      : NotificationSlot_00100
 * @tc.desc      : Add notification slot(type is SOCIAL_COMMUNICATION), get notification slot and remove notification
 * slot.
 * @tc.expected  : Add notification slot success, get notification slot correctly and remove notification slot success.
 */
HWTEST_F(AnsInterfaceModuleSlotTest, ANS_Interface_MT_NotificationSlot_00100, Function | MediumTest | Level1)
{
    NotificationSlot slot(NotificationConstant::SOCIAL_COMMUNICATION);
    slot.SetEnableLight(true);
    slot.SetDescription("description");
    slot.SetLedLightColor(0);
    slot.SetLevel(NotificationSlot::NotificationLevel::LEVEL_LOW);
    slot.SetSlotGroup("group");
    slot.SetSound(Uri("."));
    std::vector<int64_t> style;
    style.push_back(0);
    slot.SetVibrationStyle(style);
    slot.EnableBypassDnd(true);
    slot.EnableBadge(true);
    EXPECT_EQ(0, NotificationHelper::AddNotificationSlot(slot));

    sptr<NotificationSlot> spSlot;
    EXPECT_EQ(0, NotificationHelper::GetNotificationSlot(NotificationConstant::SOCIAL_COMMUNICATION, spSlot));
    if (spSlot == nullptr) {
        GTEST_LOG_(INFO) << "get slot is empty";
    } else {
        GTEST_LOG_(INFO) << "after get slot dump slot information:" << spSlot->Dump();
        EXPECT_EQ(true, spSlot->CanEnableLight());
        EXPECT_EQ(true, spSlot->CanVibrate());
        EXPECT_EQ("description", spSlot->GetDescription());
        EXPECT_EQ("SOCIAL_COMMUNICATION", spSlot->GetId());
        EXPECT_EQ(0, spSlot->GetLedLightColor());
        EXPECT_EQ(NotificationSlot::NotificationLevel::LEVEL_LOW, spSlot->GetLevel());
        EXPECT_EQ(NotificationConstant::SOCIAL_COMMUNICATION, spSlot->GetType());
        EXPECT_EQ(NotificationConstant::VisiblenessType::PUBLIC, spSlot->GetLockScreenVisibleness());
        EXPECT_EQ("SOCIAL_COMMUNICATION", spSlot->GetName());
        EXPECT_EQ("group", spSlot->GetSlotGroup());
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        for (auto it : spSlot->GetVibrationStyle()) {
            EXPECT_EQ(0, it);
        }
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        EXPECT_EQ(true, spSlot->IsEnableBypassDnd());
        EXPECT_EQ(true, spSlot->IsShowBadge());
    }
    EXPECT_EQ(0, NotificationHelper::RemoveNotificationSlot(NotificationConstant::SOCIAL_COMMUNICATION));
    EXPECT_EQ((int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST,
        NotificationHelper::GetNotificationSlot(NotificationConstant::SOCIAL_COMMUNICATION, spSlot));
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSlot_00200
 * @tc.name      : NotificationSlot_00200
 * @tc.desc      : Add notification slot(type is SERVICE_REMINDER), get notification slot and remove notification slot.
 * @tc.expected  : Add notification slot success, get notification slot correctly and remove notification slot success.
 */
HWTEST_F(AnsInterfaceModuleSlotTest, ANS_Interface_MT_NotificationSlot_00200, Function | MediumTest | Level1)
{
    NotificationSlot slot(NotificationConstant::SERVICE_REMINDER);
    slot.SetEnableLight(true);
    slot.SetDescription("description");
    slot.SetLedLightColor(0);
    slot.SetLevel(NotificationSlot::NotificationLevel::LEVEL_LOW);
    slot.SetSlotGroup("group");
    slot.SetSound(Uri("."));
    std::vector<int64_t> style;
    style.push_back(0);
    slot.SetVibrationStyle(style);
    slot.EnableBypassDnd(true);
    slot.EnableBadge(true);
    EXPECT_EQ(0, NotificationHelper::AddNotificationSlot(slot));
    sptr<NotificationSlot> spSlot;
    EXPECT_EQ(0, NotificationHelper::GetNotificationSlot(NotificationConstant::SERVICE_REMINDER, spSlot));

    if (spSlot == nullptr) {
        GTEST_LOG_(INFO) << "get slot is empty";
    } else {
        EXPECT_EQ(true, spSlot->CanEnableLight());
        EXPECT_EQ(true, spSlot->CanVibrate());
        EXPECT_EQ("description", spSlot->GetDescription());
        EXPECT_EQ("SERVICE_REMINDER", spSlot->GetId());
        EXPECT_EQ(0, spSlot->GetLedLightColor());
        EXPECT_EQ(NotificationSlot::NotificationLevel::LEVEL_LOW, spSlot->GetLevel());
        EXPECT_EQ(NotificationConstant::SERVICE_REMINDER, spSlot->GetType());
        EXPECT_EQ(NotificationConstant::VisiblenessType::PUBLIC, spSlot->GetLockScreenVisibleness());
        EXPECT_EQ("SERVICE_REMINDER", spSlot->GetName());
        EXPECT_EQ("group", spSlot->GetSlotGroup());
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        for (auto it : spSlot->GetVibrationStyle()) {
            EXPECT_EQ(0, it);
        }
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        EXPECT_EQ(true, spSlot->IsEnableBypassDnd());
        EXPECT_EQ(true, spSlot->IsShowBadge());
    }
    EXPECT_EQ(0, NotificationHelper::RemoveNotificationSlot(NotificationConstant::SERVICE_REMINDER));
    EXPECT_EQ((int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST,
        NotificationHelper::GetNotificationSlot(NotificationConstant::SERVICE_REMINDER, spSlot));
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSlot_00300
 * @tc.name      : NotificationSlot_00300
 * @tc.desc      : Add notification slot(type is CONTENT_INFORMATION), get notification slot and remove notification
 * slot.
 * @tc.expected  : Add notification slot success, get notification slot correctly and remove notification slot success.
 */
HWTEST_F(AnsInterfaceModuleSlotTest, ANS_Interface_MT_NotificationSlot_00300, Function | MediumTest | Level1)
{
    NotificationSlot slot(NotificationConstant::CONTENT_INFORMATION);
    slot.SetEnableLight(true);
    slot.SetDescription("description");
    slot.SetLedLightColor(0);
    slot.SetLevel(NotificationSlot::NotificationLevel::LEVEL_LOW);
    slot.SetSlotGroup("group");
    slot.SetSound(Uri("."));
    std::vector<int64_t> style;
    style.push_back(0);
    slot.SetVibrationStyle(style);
    slot.EnableBypassDnd(true);
    slot.EnableBadge(true);
    EXPECT_EQ(0, NotificationHelper::AddNotificationSlot(slot));
    sptr<NotificationSlot> spSlot;
    EXPECT_EQ(0, NotificationHelper::GetNotificationSlot(NotificationConstant::CONTENT_INFORMATION, spSlot));

    if (spSlot == nullptr) {
        GTEST_LOG_(INFO) << "get slot is empty";
    } else {
        EXPECT_EQ(true, spSlot->CanEnableLight());
        EXPECT_EQ(true, spSlot->CanVibrate());
        EXPECT_EQ("description", spSlot->GetDescription());
        EXPECT_EQ("CONTENT_INFORMATION", spSlot->GetId());
        EXPECT_EQ(0, spSlot->GetLedLightColor());
        EXPECT_EQ(NotificationSlot::NotificationLevel::LEVEL_LOW, spSlot->GetLevel());
        EXPECT_EQ(NotificationConstant::CONTENT_INFORMATION, spSlot->GetType());
        EXPECT_EQ(NotificationConstant::VisiblenessType::SECRET, spSlot->GetLockScreenVisibleness());
        EXPECT_EQ("CONTENT_INFORMATION", spSlot->GetName());
        EXPECT_EQ("group", spSlot->GetSlotGroup());
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        for (auto it : spSlot->GetVibrationStyle()) {
            EXPECT_EQ(0, it);
        }
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        EXPECT_EQ(true, spSlot->IsEnableBypassDnd());
        EXPECT_EQ(true, spSlot->IsShowBadge());
    }
    EXPECT_EQ(0, NotificationHelper::RemoveNotificationSlot(NotificationConstant::CONTENT_INFORMATION));
    EXPECT_EQ((int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST,
        NotificationHelper::GetNotificationSlot(NotificationConstant::CONTENT_INFORMATION, spSlot));
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSlot_00400
 * @tc.name      : NotificationSlot_00400
 * @tc.desc      : Add notification slot(type is OTHER), get notification slot and remove notification slot.
 * @tc.expected  : Add notification slot success, get notification slot correctly and remove notification slot success.
 */
HWTEST_F(AnsInterfaceModuleSlotTest, ANS_Interface_MT_NotificationSlot_00400, Function | MediumTest | Level1)
{
    NotificationSlot slot(NotificationConstant::OTHER);
    slot.SetEnableLight(true);
    slot.SetDescription("description");
    slot.SetLedLightColor(0);
    slot.SetLevel(NotificationSlot::NotificationLevel::LEVEL_LOW);
    slot.SetSlotGroup("group");
    slot.SetSound(Uri("."));
    std::vector<int64_t> style;
    style.push_back(0);
    slot.SetVibrationStyle(style);
    slot.EnableBypassDnd(true);
    slot.EnableBadge(true);
    EXPECT_EQ(0, NotificationHelper::AddNotificationSlot(slot));
    sptr<NotificationSlot> spSlot;
    EXPECT_EQ(0, NotificationHelper::GetNotificationSlot(NotificationConstant::OTHER, spSlot));

    if (spSlot == nullptr) {
        GTEST_LOG_(INFO) << "get slot is empty";
    } else {
        EXPECT_EQ(true, spSlot->CanEnableLight());
        EXPECT_EQ(true, spSlot->CanVibrate());
        EXPECT_EQ("description", spSlot->GetDescription());
        EXPECT_EQ("OTHER", spSlot->GetId());
        EXPECT_EQ(0, spSlot->GetLedLightColor());
        EXPECT_EQ(NotificationSlot::NotificationLevel::LEVEL_LOW, spSlot->GetLevel());
        EXPECT_EQ(NotificationConstant::OTHER, spSlot->GetType());
        EXPECT_EQ(NotificationConstant::VisiblenessType::SECRET, spSlot->GetLockScreenVisibleness());
        EXPECT_EQ("OTHER", spSlot->GetName());
        EXPECT_EQ("group", spSlot->GetSlotGroup());
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        for (auto it : spSlot->GetVibrationStyle()) {
            EXPECT_EQ(0, it);
        }
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        EXPECT_EQ(true, spSlot->IsEnableBypassDnd());
        EXPECT_EQ(true, spSlot->IsShowBadge());
    }
    EXPECT_EQ(0, NotificationHelper::RemoveNotificationSlot(NotificationConstant::OTHER));
    EXPECT_EQ((int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST,
        NotificationHelper::GetNotificationSlot(NotificationConstant::OTHER, spSlot));
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSlot_00500
 * @tc.name      : NotificationSlot_00500
 * @tc.desc      : Add notification slot(type is OTHER), slot set description character length exceed 1000
 *                 characters, get notification slot and remove notification slot.
 * @tc.expected  : Add notification slot success, get notification slot correctly and remove notification slot success.
 */
HWTEST_F(AnsInterfaceModuleSlotTest, ANS_Interface_MT_NotificationSlot_00500, Function | MediumTest | Level1)
{
    NotificationSlot slot(NotificationConstant::OTHER);
    slot.SetEnableLight(true);
    std::string description(2000, 'c');
    slot.SetDescription(description);
    slot.SetLedLightColor(0);
    slot.SetLevel(NotificationSlot::NotificationLevel::LEVEL_LOW);
    slot.SetSlotGroup("group");
    slot.SetSound(Uri("."));
    std::vector<int64_t> style;
    style.push_back(0);
    slot.SetVibrationStyle(style);
    slot.EnableBypassDnd(true);
    slot.EnableBadge(true);
    EXPECT_EQ(0, NotificationHelper::AddNotificationSlot(slot));
    sptr<NotificationSlot> spSlot;
    EXPECT_EQ(0, NotificationHelper::GetNotificationSlot(NotificationConstant::OTHER, spSlot));

    if (spSlot == nullptr) {
        GTEST_LOG_(INFO) << "get slot is empty";
    } else {
        GTEST_LOG_(INFO) << "get slot is:" << spSlot->Dump();
        EXPECT_EQ(true, spSlot->CanEnableLight());
        EXPECT_EQ(true, spSlot->CanVibrate());
        std::string expecteDescription(1000, 'c');
        EXPECT_EQ(expecteDescription, spSlot->GetDescription());
        EXPECT_EQ("OTHER", spSlot->GetId());
        EXPECT_EQ(0, spSlot->GetLedLightColor());
        EXPECT_EQ(NotificationSlot::NotificationLevel::LEVEL_LOW, spSlot->GetLevel());
        EXPECT_EQ(NotificationConstant::OTHER, spSlot->GetType());
        EXPECT_EQ(NotificationConstant::VisiblenessType::SECRET, spSlot->GetLockScreenVisibleness());
        EXPECT_EQ("OTHER", spSlot->GetName());
        EXPECT_EQ("group", spSlot->GetSlotGroup());
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        for (auto it : spSlot->GetVibrationStyle()) {
            EXPECT_EQ(0, it);
        }
        EXPECT_EQ(Uri("."), spSlot->GetSound());
        EXPECT_EQ(true, spSlot->IsEnableBypassDnd());
        EXPECT_EQ(true, spSlot->IsShowBadge());
    }
    EXPECT_EQ(0, NotificationHelper::RemoveNotificationSlot(NotificationConstant::OTHER));
    EXPECT_EQ((int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST,
        NotificationHelper::GetNotificationSlot(NotificationConstant::OTHER, spSlot));
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSlotGroup_00100
 * @tc.name      : NotificationSlotGroup_00100
 * @tc.desc      : Add notification slot group, get notification slot group and remove notification slot group.
 * @tc.expected  : Add notification slot group success, get notification slot group correctly and remove notification
 *                 slot group success.
 */
HWTEST_F(AnsInterfaceModuleSlotTest, ANS_Interface_MT_NotificationSlotGroup_00100, Function | MediumTest | Level1)
{
    NotificationSlotGroup slotGroup("id", "name");
    slotGroup.SetDescription("description");
    EXPECT_EQ(0, NotificationHelper::AddNotificationSlotGroup(slotGroup));
    sptr<NotificationSlotGroup> spSlotGroup;
    EXPECT_EQ(0, NotificationHelper::GetNotificationSlotGroup("id", spSlotGroup));

    if (spSlotGroup == nullptr) {
        GTEST_LOG_(INFO) << "get slot group is empty";
    } else {
        GTEST_LOG_(INFO) << "get slot group is:" << spSlotGroup->Dump();
        EXPECT_EQ("description", spSlotGroup->GetDescription());
        EXPECT_EQ("id", spSlotGroup->GetId());
        EXPECT_EQ("name", spSlotGroup->GetName());
        EXPECT_EQ(false, spSlotGroup->IsDisabled());
    }
    EXPECT_EQ(0, NotificationHelper::RemoveNotificationSlotGroup("id"));
    EXPECT_EQ((int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_NOT_EXIST,
        NotificationHelper::GetNotificationSlotGroup("id", spSlotGroup));
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSlotGroup_00200
 * @tc.name      : NotificationSlotGroup_00200
 * @tc.desc      : Add notification slot group, slot group set description character length exceed 1000
 *                 characters, get notification slot group and remove notification slot group.
 * @tc.expected  : Add notification slot group success, get notification slot group correctly and remove notification
 *                 slot group success.
 */
HWTEST_F(AnsInterfaceModuleSlotTest, ANS_Interface_MT_NotificationSlotGroup_00200, Function | MediumTest | Level1)
{
    NotificationSlotGroup slotGroup("id", "name");
    std::string description(2000, 'c');
    slotGroup.SetDescription(description);
    EXPECT_EQ(0, NotificationHelper::AddNotificationSlotGroup(slotGroup));
    GTEST_LOG_(INFO) << "after add group is:" << slotGroup.Dump();
    sptr<NotificationSlotGroup> spSlotGroup;
    EXPECT_EQ(0, NotificationHelper::GetNotificationSlotGroup("id", spSlotGroup));

    if (spSlotGroup == nullptr) {
        GTEST_LOG_(INFO) << "get slot group is empty";
    } else {
        GTEST_LOG_(INFO) << "get slot group is:" << spSlotGroup->Dump();
        std::string expectDescription(1000, 'c');
        EXPECT_EQ(expectDescription, spSlotGroup->GetDescription());
        EXPECT_EQ("id", spSlotGroup->GetId());
        EXPECT_EQ("name", spSlotGroup->GetName());
        EXPECT_EQ(false, spSlotGroup->IsDisabled());
    }
    EXPECT_EQ(0, NotificationHelper::RemoveNotificationSlotGroup("id"));
    EXPECT_EQ((int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_NOT_EXIST,
        NotificationHelper::GetNotificationSlotGroup("id", spSlotGroup));
}

/**
 * @tc.number    : ANS_Interface_MT_NotificationSlotGroup_00300
 * @tc.name      : NotificationSlotGroup_00300
 * @tc.desc      : Two different slots added to the same slot group.
 * @tc.expected  : Add success.
 */
HWTEST_F(AnsInterfaceModuleSlotTest, ANS_Interface_MT_NotificationSlotGroup_00300, Function | MediumTest | Level1)
{
    NotificationSlot slotA(NotificationConstant::OTHER);
    slotA.SetSlotGroup("id");

    NotificationSlot slotB(NotificationConstant::CUSTOM);
    slotA.SetSlotGroup("id");

    NotificationSlotGroup slotGroup("id", "name");
    EXPECT_EQ(0, NotificationHelper::AddNotificationSlotGroup(slotGroup));

    sptr<NotificationSlotGroup> spSlotGroup;
    EXPECT_EQ(0, NotificationHelper::GetNotificationSlotGroup("id", spSlotGroup));

    if (spSlotGroup == nullptr) {
        GTEST_LOG_(INFO) << "get slot group is empty";
    } else {
        std::vector<NotificationSlot> slots = spSlotGroup->GetSlots();
        for (auto it : slots) {
            if (NotificationConstant::OTHER == it.GetType()) {
                EXPECT_EQ("OTHER", it.GetId());
            } else if (NotificationConstant::CUSTOM == it.GetType()) {
                EXPECT_EQ("CUSTOM", it.GetId());
            } else {
                GTEST_LOG_(INFO) << "slot group has wrong slot";
            }
        }
    }
    EXPECT_EQ(0, NotificationHelper::RemoveNotificationSlotGroup("id"));
    EXPECT_EQ((int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_NOT_EXIST,
        NotificationHelper::GetNotificationSlotGroup("id", spSlotGroup));
}
}  // namespace Notification
}  // namespace OHOS