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

#include "ans_slotgroup_test.h"
#include "notification_slot_group.h"

namespace OHOS {
namespace AppExecFwk {
void AnsSlotGroupSystemTest::OnStart(const Want &want)
{
    APP_LOGI("====> ans SlotGroup test start====>");
    Notification::NotificationSlotGroup notificationSlotGroupTest("id_test", "name_test");

    if (notificationSlotGroupTest.GetId().empty()) {
        ANS_LOGI("====> Notification slot group id is invalid====>");
    } else {
        ANS_LOGI("====> Notification slot group id is====> %{public}s", notificationSlotGroupTest.GetId().c_str());
    }

    ErrCode errcodeAdd = Notification::NotificationHelper::AddNotificationSlotGroup(notificationSlotGroupTest);
    APP_LOGI("====> ansSlotGroupTest add:====> %{public}d", errcodeAdd);

    Notification::NotificationSlot notificationSlotFirst(
        Notification::NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    Notification::NotificationSlot notificationSlotSecond(
        Notification::NotificationConstant::SlotType::SERVICE_REMINDER);

    notificationSlotFirst.SetSlotGroup("id_test");
    notificationSlotSecond.SetSlotGroup("id_test");

    ErrCode errcodeAddSlotFirst = Notification::NotificationHelper::AddNotificationSlot(notificationSlotFirst);
    ErrCode errcodeAddSlotSecond = Notification::NotificationHelper::AddNotificationSlot(notificationSlotSecond);

    APP_LOGI("====> ansSlotGroupTest errcodeAddSlotFirst:====> %{public}d", errcodeAddSlotFirst);
    APP_LOGI("====> ansSlotGroupTest errcodeAddSlotSecond:====> %{public}d", errcodeAddSlotSecond);

    sptr<Notification::NotificationSlotGroup> group;
    ErrCode errcodeGet = Notification::NotificationHelper::GetNotificationSlotGroup("id_test", group);
    APP_LOGI("====> ansSlotGroupTest get:====> %{public}d", errcodeGet);

    if (group != nullptr) {
        std::string dumpGroupStr = group->Dump();
        APP_LOGI("====> ansSlotGroupTest dump:====> %{public}s", dumpGroupStr.c_str());
        APP_LOGI("====> ansSlotGroupTest the size of slots:====> %{public}zu", group->GetSlots().size());
    } else {
        APP_LOGI("====>group is null====>");
    }

    ErrCode errcodeRem = Notification::NotificationHelper::RemoveNotificationSlotGroup("id_test");
    APP_LOGI("====> ansSlotGroupTest remove:====> %{public}d", errcodeRem);
    sptr<Notification::NotificationSlotGroup> groupTwo;
    ErrCode errcodeGetTwo = Notification::NotificationHelper::GetNotificationSlotGroup("id_test", groupTwo);
    APP_LOGI("====> ansSlotGroupTest get:====> %{public}d", errcodeGetTwo);
    Ability::OnStart(want);
}

void AnsSlotGroupSystemTest::OnNewWant(const Want &want)
{
    GetWantInfo(want);
    Ability::OnNewWant(want);
}

void AnsSlotGroupSystemTest::OnForeground(const Want &want)
{
    GetWantInfo(want);
    Ability::OnForeground(want);
}

void AnsSlotGroupSystemTest::OnStop()
{
    Ability::OnStop();
}

void AnsSlotGroupSystemTest::OnActive()
{
    Ability::OnActive();
    if (std::string::npos != shouldReturn.find(GetAbilityName())) {
        TerminateAbility();
    }
    Clear();
}

void AnsSlotGroupSystemTest::OnInactive()
{
    Ability::OnInactive();
}

void AnsSlotGroupSystemTest::OnBackground()
{
    Ability::OnBackground();
}

void AnsSlotGroupSystemTest::Clear()
{
    shouldReturn = "";
    targetBundle = "";
    targetAbility = "";
}

void AnsSlotGroupSystemTest::GetWantInfo(const Want &want)
{
    Want mWant(want);
    shouldReturn = mWant.GetStringParam("shouldReturn");
    targetBundle = mWant.GetStringParam("targetBundle");
    targetAbility = mWant.GetStringParam("targetAbility");
}

REGISTER_AA(AnsSlotGroupSystemTest)
}  // namespace AppExecFwk
}  // namespace OHOS