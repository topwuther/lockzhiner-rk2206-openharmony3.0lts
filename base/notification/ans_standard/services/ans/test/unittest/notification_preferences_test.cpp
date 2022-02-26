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

#include "ans_inner_errors.h"
#include "ans_ut_constant.h"
#include "notification_preferences.h"

using namespace testing::ext;
namespace OHOS {
namespace Notification {

class NotificationPreferencesTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown();

    void TestAddNotificationSlot();
    void TestAddNotificationSlotGroup();

    static sptr<NotificationBundleOption> bundleOption_;
    static sptr<NotificationBundleOption> noExsitbundleOption_;
    static sptr<NotificationBundleOption> bundleEmptyOption_;
};

sptr<NotificationBundleOption> NotificationPreferencesTest::bundleOption_ =
    new NotificationBundleOption(TEST_DEFUALT_BUNDLE, NON_SYSTEM_APP_UID);
sptr<NotificationBundleOption> NotificationPreferencesTest::noExsitbundleOption_ =
    new NotificationBundleOption(std::string("notExsitBundleName"), NON_SYSTEM_APP_UID);
sptr<NotificationBundleOption> NotificationPreferencesTest::bundleEmptyOption_ =
    new NotificationBundleOption(std::string(), NON_SYSTEM_APP_UID);

void NotificationPreferencesTest::TearDown()
{
    NotificationPreferences::GetInstance().ClearNotificationInRestoreFactorySettings();
}

void NotificationPreferencesTest::TestAddNotificationSlot()
{
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption_, slots);
}

void NotificationPreferencesTest::TestAddNotificationSlotGroup()
{
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(group);
    NotificationPreferences::GetInstance().AddNotificationSlotGroups(bundleOption_, groups);
}

/**
 * @tc.number    : AddNotificationSlots_00100
 * @tc.name      :
 * @tc.desc      : Add a notification slot into distrube DB , return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlots_00100, Function | SmallTest | Level1)
{
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption_, slots), (int)ERR_OK);
}

/**
 * @tc.number    : AddNotificationSlots_00200
 * @tc.name      :
 * @tc.desc      : Add a notification slot into distrube DB when bundleName is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlots_00200, Function | SmallTest | Level1)
{
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlots(bundleEmptyOption_, slots),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : AddNotificationSlots_00300
 * @tc.name      :
 * @tc.desc      : Add a notification slot into distrube DB when slots is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlots_00300, Function | SmallTest | Level1)
{
    std::vector<sptr<NotificationSlot>> slots;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption_, slots),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : AddNotificationSlots_00400
 * @tc.name      :
 * @tc.desc      : Add a notification slot into distrube DB when slot is nullptr in vector, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_NOT_EXIST.
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlots_00400, Function | SmallTest | Level1)
{
    sptr<NotificationSlot> slot = nullptr;
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption_, slots),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_NOT_EXIST);
}

/**
 * @tc.number    : AddNotificationSlots_00500
 * @tc.name      :
 * @tc.desc      : Add a notification slot into distrube DB when slots is same, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlots_00500, Function | SmallTest | Level1)
{

    sptr<NotificationSlot> slot1 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    sptr<NotificationSlot> slot2 = new NotificationSlot(NotificationConstant::SlotType::SOCIAL_COMMUNICATION);

    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot1);
    slots.push_back(slot2);

    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption_, slots), (int)ERR_OK);
}

/**
 * @tc.number    : AddNotificationSlotGroups_00100
 * @tc.name      :
 * @tc.desc      : Add a notification groups into disturbe DB , return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlotGroups_00100, Function | SmallTest | Level1)
{
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(group);
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().AddNotificationSlotGroups(bundleOption_, groups), (int)ERR_OK);
}

/**
 * @tc.number    : AddNotificationSlotGroups_00200
 * @tc.name      :
 * @tc.desc      : Add a notification groups into disturbe DB when bundle name is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlotGroups_00200, Function | SmallTest | Level1)
{
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(group);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlotGroups(bundleEmptyOption_, groups),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : AddNotificationSlotGroups_00300
 * @tc.name      :
 * @tc.desc      : Add a notification groups into disturbe DB when groups is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlotGroups_00300, Function | SmallTest | Level1)
{
    std::vector<sptr<NotificationSlotGroup>> groups;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlotGroups(bundleOption_, groups),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : AddNotificationSlotGroups_00400
 * @tc.name      :
 * @tc.desc      : Add a notification groups into disturbe DB when group id is null, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_ID_INVALID
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlotGroups_00400, Function | SmallTest | Level1)
{
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("", "name");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(group);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlotGroups(bundleOption_, groups),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_ID_INVALID);
}

/**
 * @tc.number    : AddNotificationSlotGroups_00500
 * @tc.name      :
 * @tc.desc      : Add a notification groups into disturbe DB when add group exceed max num, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_EXCEED_MAX_NUM
 */
HWTEST_F(NotificationPreferencesTest, AddNotificationSlotGroups_00500, Function | SmallTest | Level1)
{
    sptr<NotificationSlotGroup> group1 = new NotificationSlotGroup("id1", "name1");
    sptr<NotificationSlotGroup> group2 = new NotificationSlotGroup("id2", "name1");
    sptr<NotificationSlotGroup> group3 = new NotificationSlotGroup("id3", "name1");
    sptr<NotificationSlotGroup> group4 = new NotificationSlotGroup("id4", "name1");
    sptr<NotificationSlotGroup> group5 = new NotificationSlotGroup("id5", "name1");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(group1);
    groups.push_back(group2);
    groups.push_back(group3);
    groups.push_back(group4);
    groups.push_back(group5);

    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlotGroups(bundleOption_, groups),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_EXCEED_MAX_NUM);
}

/**
 * @tc.number    : RemoveNotificationSlot_00100
 * @tc.name      :
 * @tc.desc      : Remove a notification slot from disturbe DB , return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationSlot_00100, Function | SmallTest | Level1)
{
    TestAddNotificationSlot();
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationSlot(
                  bundleOption_, NotificationConstant::SlotType::OTHER),
        (int)ERR_OK);
}

/**
 * @tc.number    : RemoveNotificationSlot_00200
 * @tc.name      :
 * @tc.desc      : Remove a notification slot from disturbe DB when bundle name is null, return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationSlot_00200, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationSlot(
                  bundleEmptyOption_, NotificationConstant::SlotType::OTHER),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : RemoveNotificationSlot_00300
 * @tc.name      :
 * @tc.desc      : Remove a notification slot from disturbe DB when bundle name does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationSlot_00300, Function | SmallTest | Level1)
{
    TestAddNotificationSlot();
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationSlot(
                  noExsitbundleOption_, NotificationConstant::SlotType::OTHER),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
}

/**
 * @tc.number    : RemoveNotificationSlot_00400
 * @tc.name      :
 * @tc.desc      : Remove a notification slot from disturbe DB when slot type does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationSlot_00400, Function | SmallTest | Level1)
{
    TestAddNotificationSlot();
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationSlot(
                  bundleOption_, NotificationConstant::SlotType::SERVICE_REMINDER),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST);
}

/**
 * @tc.number    : RemoveNotificationSlotGroups_00100
 * @tc.name      :
 * @tc.desc      : Remove a notification group from disturbe DB , return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationSlotGroups_00100, Function | SmallTest | Level1)
{
    TestAddNotificationSlotGroup();
    std::string groupId = "id";
    std::vector<std::string> groupIds;
    groupIds.push_back(groupId);
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().RemoveNotificationSlotGroups(bundleOption_, groupIds), (int)ERR_OK);
}

/**
 * @tc.number    : RemoveNotificationSlotGroups_00200
 * @tc.name      :
 * @tc.desc      : Remove a notification group from disturbe DB when bundleName is null, return is ERR_ANS_INVALID_PARAM
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationSlotGroups_00200, Function | SmallTest | Level1)
{
    TestAddNotificationSlotGroup();
    std::string groupId("id");
    std::vector<std::string> groupIds;
    groupIds.push_back(groupId);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationSlotGroups(bundleEmptyOption_, groupIds),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : RemoveNotificationSlotGroups_00300
 * @tc.name      :
 * @tc.desc      : Remove a notification group from disturbe DB when group id is null, return is ERR_ANS_INVALID_PARAM
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationSlotGroups_00300, Function | SmallTest | Level1)
{
    TestAddNotificationSlotGroup();
    std::vector<std::string> groupIds;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationSlotGroups(bundleOption_, groupIds),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : RemoveNotificationSlotGroups_00400
 * @tc.name      :
 * @tc.desc      : Remove a notification group from disturbe DB when bundle name does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationSlotGroups_00400, Function | SmallTest | Level1)
{
    TestAddNotificationSlotGroup();
    std::vector<std::string> groupIds;
    groupIds.push_back("id");
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationSlotGroups(noExsitbundleOption_, groupIds),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
}

/**
 * @tc.number    : RemoveNotificationSlotGroups_00500
 * @tc.name      :
 * @tc.desc      : Remove a notification group from disturbe DB when group id does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_ID_INVALID
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationSlotGroups_00500, Function | SmallTest | Level1)
{
    TestAddNotificationSlotGroup();
    std::string groupId = "id1";
    std::vector<std::string> groupIds;
    groupIds.push_back(groupId);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationSlotGroups(bundleOption_, groupIds),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_ID_INVALID);
}
/**
 * @tc.number    : RemoveNotificationForBundle_00100
 * @tc.name      :
 * @tc.desc      : Remove notification for bundle from disturbe DB, return is ERR_OK;
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationForBundle_00100, Function | SmallTest | Level1)
{
    TestAddNotificationSlot();
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationForBundle(bundleOption_), (int)ERR_OK);
}

/**
 * @tc.number    : RemoveNotificationForBundle_00200
 * @tc.name      :
 * @tc.desc      :  Remove notification for bundle from disturbe DB when bundle name is null, return is
 * ERR_ANS_INVALID_PARAM;
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationForBundle_00200, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationForBundle(bundleEmptyOption_),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : RemoveNotificationForBundle_00300
 * @tc.name      :
 * @tc.desc      :  Remove notification for bundle from disturbe DB when bundle name is null, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST;
 */
HWTEST_F(NotificationPreferencesTest, RemoveNotificationForBundle_00300, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().RemoveNotificationForBundle(noExsitbundleOption_),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
}

/**
 * @tc.number    : UpdateNotificationSlots_00100
 * @tc.name      :
 * @tc.desc      : Update notification slot into disturbe DB, return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlots_00100, Function | SmallTest | Level1)
{
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption_, slots), (int)ERR_OK);
    std::string des("This is a description.");
    slot->SetDescription(des);
    slots.clear();
    slots.push_back(slot);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().UpdateNotificationSlots(bundleOption_, slots), (int)ERR_OK);
}

/**
 * @tc.number    : UpdateNotificationSlots_00200
 * @tc.name      :
 * @tc.desc      : Update notification slot into disturbe DB when bundleName is null, return is ERR_ANS_INVALID_PARAM
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlots_00200, Function | SmallTest | Level1)
{
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().UpdateNotificationSlots(bundleEmptyOption_, slots),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : UpdateNotificationSlots_00300
 * @tc.name      :
 * @tc.desc      : Update notification slot into disturbe DB when slots is null, return is ERR_ANS_INVALID_PARAM
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlots_00300, Function | SmallTest | Level1)
{
    std::vector<sptr<NotificationSlot>> slots;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().UpdateNotificationSlots(bundleOption_, slots),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : UpdateNotificationSlots_00400
 * @tc.name      :
 * @tc.desc      : Update notification slot into disturbe DB when bundle does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlots_00400, Function | SmallTest | Level1)
{
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().UpdateNotificationSlots(noExsitbundleOption_, slots),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
}

/**
 * @tc.number    : UpdateNotificationSlots_00500
 * @tc.name      :
 * @tc.desc      : Update notification slot into disturbe DB when slot type does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlots_00500, Function | SmallTest | Level1)
{
    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().UpdateNotificationSlots(noExsitbundleOption_, slots),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
}

/**
 * @tc.number    : UpdateNotificationSlotGroups_00100
 * @tc.name      :
 * @tc.desc      : Update notification slot group into disturbe DB  return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlotGroups_00100, Function | SmallTest | Level1)
{
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(group);
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().AddNotificationSlotGroups(bundleOption_, groups), (int)ERR_OK);
    std::string des("This is a group description.");
    group->SetDescription(des);
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().UpdateNotificationSlotGroups(bundleOption_, groups), (int)ERR_OK);
}

/**
 * @tc.number    : UpdateNotificationSlotGroups_00200
 * @tc.name      :
 * @tc.desc      : Update notification slot group into disturbe DB when bundle name is null, return is
 * ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlotGroups_00200, Function | SmallTest | Level1)
{
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(group);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().UpdateNotificationSlotGroups(bundleEmptyOption_, groups),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : UpdateNotificationSlotGroups_00300
 * @tc.name      :
 * @tc.desc      : Update notification slot group into disturbe DB when groups is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlotGroups_00300, Function | SmallTest | Level1)
{
    std::vector<sptr<NotificationSlotGroup>> groups;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().UpdateNotificationSlotGroups(bundleOption_, groups),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : UpdateNotificationSlotGroups_00400
 * @tc.name      :
 * @tc.desc      : Update notification slot group into disturbe DB when bundle does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST.
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlotGroups_00400, Function | SmallTest | Level1)
{
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("id", "name");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(group);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().UpdateNotificationSlotGroups(noExsitbundleOption_, groups),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
}

/**
 * @tc.number    : UpdateNotificationSlotGroups_00500
 * @tc.name      :
 * @tc.desc      : Update notification slot group into disturbe DB when groupid is null, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_NOT_EXIST
 */
HWTEST_F(NotificationPreferencesTest, UpdateNotificationSlotGroups_00500, Function | SmallTest | Level1)
{
    TestAddNotificationSlotGroup();
    sptr<NotificationSlotGroup> spGroup = new NotificationSlotGroup("", "name");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(spGroup);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().UpdateNotificationSlotGroups(bundleOption_, groups),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_NOT_EXIST);
}

/**
 * @tc.number    : GetNotificationSlot_00100
 * @tc.name      :
 * @tc.desc      : Update notification slot group into disturbe DB, return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationSlot_00100, Function | SmallTest | Level1)
{
    TestAddNotificationSlot();
    sptr<NotificationSlot> slot;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationSlot(
                  bundleOption_, NotificationConstant::SlotType::OTHER, slot),
        (int)ERR_OK);
}

/**
 * @tc.number    : GetNotificationSlot_00200
 * @tc.name      :
 * @tc.desc      : Update notification slot group into disturbe DB when bundle name is null, return is
 * ERR_ANS_INVALID_PARAM
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationSlot_00200, Function | SmallTest | Level1)
{
    sptr<NotificationSlot> slot;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationSlot(
                  bundleEmptyOption_, NotificationConstant::SlotType::OTHER, slot),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : GetNotificationSlot_00300
 * @tc.name      :
 * @tc.desc      : Update notification slot group into disturbe DB when slot type does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationSlot_00300, Function | SmallTest | Level1)
{
    TestAddNotificationSlot();
    sptr<NotificationSlot> slot;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationSlot(
                  bundleOption_, NotificationConstant::SlotType::CONTENT_INFORMATION, slot),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST);
}

/**
 * @tc.number    : GetNotificationSlot_00400
 * @tc.name      :
 * @tc.desc      : Update notification slot group into disturbe DB when bundle name does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationSlot_00400, Function | SmallTest | Level1)
{
    TestAddNotificationSlot();
    sptr<NotificationSlot> slot;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationSlot(
                  noExsitbundleOption_, NotificationConstant::SlotType::OTHER, slot),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
}

/**
 * @tc.number    : GetNotificationAllSlots_00100
 * @tc.name      :
 * @tc.desc      : Get all notification slots from disturbe DB after add a notification slot, return is ERR_OK, get all
 * notifications size is 1.
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlots_00100, Function | SmallTest | Level1)
{
    TestAddNotificationSlot();
    std::vector<sptr<NotificationSlot>> slotsResult;
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().GetNotificationAllSlots(bundleOption_, slotsResult), (int)ERR_OK);
    EXPECT_EQ((int)slotsResult.size(), 1);
}

/**
 * @tc.number    : GetNotificationAllSlots_00200
 * @tc.name      :
 * @tc.desc      : Get all notification slots from disturbe DB after add some notification slot, return is ERR_OK, get
 * all notifications size is the same of adding notifications size.
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlots_00200, Function | SmallTest | Level1)
{
    sptr<NotificationSlot> slot1 = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    sptr<NotificationSlot> slot2 = new NotificationSlot(NotificationConstant::SlotType::CONTENT_INFORMATION);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot1);
    slots.push_back(slot2);
    NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption_, slots);

    std::vector<sptr<NotificationSlot>> slotsResult;
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().GetNotificationAllSlots(bundleOption_, slotsResult), (int)ERR_OK);
    EXPECT_EQ((int)slotsResult.size(), 2);
}

/**
 * @tc.number    : GetNotificationAllSlots_00300
 * @tc.name      :
 * @tc.desc      : Get all notification slots from disturbe DB when bundle name is null, return is
 * ERR_ANS_INVALID_PARAM
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlots_00300, Function | SmallTest | Level1)
{
    std::vector<sptr<NotificationSlot>> slotsResult;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationAllSlots(bundleEmptyOption_, slotsResult),
        (int)ERR_ANS_INVALID_PARAM);
    EXPECT_EQ((int)slotsResult.size(), 0);
}

/**
 * @tc.number    : GetNotificationAllSlots_00400
 * @tc.name      :
 * @tc.desc      : Get all notification slots from disturbe DB when bundle name does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST.
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlots_00400, Function | SmallTest | Level1)
{
    std::vector<sptr<NotificationSlot>> slotsResult;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationAllSlots(noExsitbundleOption_, slotsResult),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
    EXPECT_EQ((int)slotsResult.size(), 0);
}

/**
 * @tc.number    : GetNotificationAllSlotGroups_00100
 * @tc.name      :
 * @tc.desc      : Get all notification slots from disturbe DB, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlotGroups_00100, Function | SmallTest | Level1)
{
    TestAddNotificationSlotGroup();
    std::vector<sptr<NotificationSlotGroup>> groups;
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().GetNotificationAllSlotGroups(bundleOption_, groups), (int)ERR_OK);
    EXPECT_EQ((int)groups.size(), 1);
}

/**
 * @tc.number    : GetNotificationAllSlotGroups_00200
 * @tc.name      :
 * @tc.desc      : Get all notification slots from disturbe DB when bundle name is null, return is
 * ERR_ANS_INVALID_PARAM
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlotGroups_00200, Function | SmallTest | Level1)
{
    std::vector<sptr<NotificationSlotGroup>> groups;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationAllSlotGroups(bundleEmptyOption_, groups),
        (int)ERR_ANS_INVALID_PARAM);
    EXPECT_EQ((int)groups.size(), 0);
}

/**
 * @tc.number    : GetNotificationAllSlotGroups_00300
 * @tc.name      :
 * @tc.desc      : Get all notification slots from disturbe DB when bundle name does not exsit, return is
 * ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlotGroups_00300, Function | SmallTest | Level1)
{
    std::vector<sptr<NotificationSlotGroup>> groups;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationAllSlotGroups(noExsitbundleOption_, groups),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
    EXPECT_EQ((int)groups.size(), 0);
}

/**
 * @tc.number    : GetNotificationAllSlotInSlotGroup_00100
 * @tc.name      :
 * @tc.desc      : Get all notification slots in the same group from disturbe DB , return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlotInSlotGroup_00100, Function | SmallTest | Level1)
{
    std::string groupId = "groupId";
    sptr<NotificationSlotGroup> group = new NotificationSlotGroup("groupId", "name");
    std::vector<sptr<NotificationSlotGroup>> groups;
    groups.push_back(group);
    NotificationPreferences::GetInstance().AddNotificationSlotGroups(bundleOption_, groups);

    sptr<NotificationSlot> slot = new NotificationSlot(NotificationConstant::SlotType::OTHER);
    slot->SetSlotGroup(groupId);
    std::vector<sptr<NotificationSlot>> slots;
    slots.push_back(slot);
    NotificationPreferences::GetInstance().AddNotificationSlots(bundleOption_, slots);
    std::vector<sptr<NotificationSlot>> slotsReuslt;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationAllSlotInSlotGroup(
                  bundleOption_, groupId, slotsReuslt),
        (int)ERR_OK);
    EXPECT_EQ((int)slotsReuslt.size(), 1);
}

/**
 * @tc.number    : GetNotificationAllSlotInSlotGroup_00200
 * @tc.name      :
 * @tc.desc      : Get all notification slots in the same group from disturbe DB when bundle name is null, return is
 * ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlotInSlotGroup_00200, Function | SmallTest | Level1)
{
    std::string groupId = "groupId";
    std::vector<sptr<NotificationSlot>> slotsReuslt;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationAllSlotInSlotGroup(
                  bundleEmptyOption_, groupId, slotsReuslt),
        (int)ERR_ANS_INVALID_PARAM);
    EXPECT_EQ((int)slotsReuslt.size(), 0);
}

/**
 * @tc.number    : GetNotificationAllSlotInSlotGroup_00300
 * @tc.name      :
 * @tc.desc      : Get all notification slots in the same group from disturbe DB when group id is null, return is
 * ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlotInSlotGroup_00300, Function | SmallTest | Level1)
{
    std::vector<sptr<NotificationSlot>> slotsReuslt;
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().GetNotificationAllSlotInSlotGroup(bundleOption_, "", slotsReuslt),
        (int)ERR_ANS_INVALID_PARAM);
    EXPECT_EQ((int)slotsReuslt.size(), 0);
}

/**
 * @tc.number    : GetNotificationAllSlotInSlotGroup_00400
 * @tc.name      :
 * @tc.desc      : Get all notification slots in the same group from disturbe DB when bundle name does not exsit, return
 * is ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST.
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationAllSlotInSlotGroup_00400, Function | SmallTest | Level1)
{
    std::string groupId = "groupId";
    std::vector<sptr<NotificationSlot>> slotsReuslt;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationAllSlotInSlotGroup(
                  noExsitbundleOption_, groupId, slotsReuslt),
        (int)ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST);
    EXPECT_EQ((int)slotsReuslt.size(), 0);
}

/**
 * @tc.number    : SetShowBadge_00100
 * @tc.name      :
 * @tc.desc      : Set bundle show badge into disturbe DB, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, SetShowBadge_00100, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetShowBadge(bundleOption_, true), (int)ERR_OK);
}

/**
 * @tc.number    : SetShowBadge_00200
 * @tc.name      :
 * @tc.desc      : Set bundle show badge into disturbe DB when bundle name is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, SetShowBadge_00200, Function | SmallTest | Level1)
{
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().SetShowBadge(bundleEmptyOption_, true), (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : IsShowBadge_00100
 * @tc.name      :
 * @tc.desc      : Get bunlde show badge from disturbe DB , return is ERR_OK and show badge is true.
 */
HWTEST_F(NotificationPreferencesTest, IsShowBadge_00100, Function | SmallTest | Level1)
{
    bool enable = false;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetShowBadge(bundleOption_, true), (int)ERR_OK);
    EXPECT_EQ((int)NotificationPreferences::GetInstance().IsShowBadge(bundleOption_, enable), (int)ERR_OK);
    EXPECT_TRUE(enable);
}

/**
 * @tc.number    : IsShowBadge_00200
 * @tc.name      :
 * @tc.desc      : Get bunlde show badge from disturbe DB when bundle name is null, return is ERR_OK and show badge is
 * true.
 */
HWTEST_F(NotificationPreferencesTest, IsShowBadge_00200, Function | SmallTest | Level1)
{
    bool enable = false;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().IsShowBadge(bundleEmptyOption_, enable),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : SetImportance_00100
 * @tc.name      :
 * @tc.desc      : Set bundle importance into disturbe DB, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, SetImportance_00100, Function | SmallTest | Level1)
{
    int importance = 1;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetImportance(bundleOption_, importance), (int)ERR_OK);
}

/**
 * @tc.number    : SetImportance_00200
 * @tc.name      :
 * @tc.desc      : Set bundle importance into disturbe DB when bundle name is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, SetImportance_00200, Function | SmallTest | Level1)
{
    int importance = 1;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetImportance(bundleEmptyOption_, importance),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : GetImportance_00100
 * @tc.name      :
 * @tc.desc      : Get bundle importance from disturbe DB, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, GetImportance_00100, Function | SmallTest | Level1)
{
    int importance = 1;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetImportance(bundleOption_, importance), (int)ERR_OK);
    int getImportance = 0;

    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetImportance(bundleOption_, getImportance), (int)ERR_OK);
    EXPECT_EQ(getImportance, 1);
}

/**
 * @tc.number    : GetImportance_00200
 * @tc.name      :
 * @tc.desc      : Get bundle importance from disturbe DB when bundle name is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, GetImportance_00200, Function | SmallTest | Level1)
{
    int getImportance = 0;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetImportance(bundleEmptyOption_, getImportance),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : SetTotalBadgeNums_00100
 * @tc.name      :
 * @tc.desc      : Set total badge nums into disturbe DB, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, SetTotalBadgeNums_00100, Function | SmallTest | Level1)
{
    int num = 1;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetTotalBadgeNums(bundleOption_, num), (int)ERR_OK);
}

/**
 * @tc.number    : SetTotalBadgeNums_00200
 * @tc.name      :
 * @tc.desc      : Set total badge nums into disturbe DB when bundle name is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, SetTotalBadgeNums_00200, Function | SmallTest | Level1)
{
    int num = 1;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetTotalBadgeNums(bundleEmptyOption_, num),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : GetTotalBadgeNums_00100
 * @tc.name      :
 * @tc.desc      : Get total badge nums from disturbe DB, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, GetTotalBadgeNums_00100, Function | SmallTest | Level1)
{
    int num = 1;
    NotificationPreferences::GetInstance().SetTotalBadgeNums(bundleOption_, num);
    int totalBadgeNum = 0;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetTotalBadgeNums(bundleOption_, totalBadgeNum), (int)ERR_OK);
    EXPECT_EQ(totalBadgeNum, num);
}

/**
 * @tc.number    : GetTotalBadgeNums_00200
 * @tc.name      :
 * @tc.desc      : Get total badge nums from disturbe DB when bundle name is null, return is ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, GetTotalBadgeNums_00200, Function | SmallTest | Level1)
{
    int totalBadgeNum = 0;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetTotalBadgeNums(bundleEmptyOption_, totalBadgeNum),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : SetPrivateNotificationsAllowed_00100
 * @tc.name      :
 * @tc.desc      : Set private notification allowed badge nums into disturbe DB , return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, SetPrivateNotificationsAllowed_00100, Function | SmallTest | Level1)
{
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().SetPrivateNotificationsAllowed(bundleOption_, true), (int)ERR_OK);
}

/**
 * @tc.number    : SetPrivateNotificationsAllowed_00200
 * @tc.name      :
 * @tc.desc      : Set private notification allowed badge nums into disturbe DB when bundle name is null, return is
 * ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, SetPrivateNotificationsAllowed_00200, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetPrivateNotificationsAllowed(bundleEmptyOption_, true),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : GetPrivateNotificationsAllowed_00100
 * @tc.name      :
 * @tc.desc      : Get private notification allowed badge nums from disturbe DB, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, GetPrivateNotificationsAllowed_00100, Function | SmallTest | Level1)
{
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().SetPrivateNotificationsAllowed(bundleOption_, true), (int)ERR_OK);
    bool allow = false;
    EXPECT_EQ(
        (int)NotificationPreferences::GetInstance().GetPrivateNotificationsAllowed(bundleOption_, allow), (int)ERR_OK);
    EXPECT_EQ(allow, true);
}

/**
 * @tc.number    : GetPrivateNotificationsAllowed_00200
 * @tc.name      :
 * @tc.desc      : Get private notification allowed badge nums from disturbe DB when bundle is null, return is
 * ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, GetPrivateNotificationsAllowed_00200, Function | SmallTest | Level1)
{
    bool allow = false;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetPrivateNotificationsAllowed(bundleEmptyOption_, allow),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : SetNotificationsEnabledForBundle_00100
 * @tc.name      :
 * @tc.desc      : Set notification enable for bundle into disturbe DB, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, SetNotificationsEnabledForBundle_00100, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetNotificationsEnabledForBundle(bundleOption_, false),
        (int)ERR_OK);
}

/**
 * @tc.number    : SetNotificationsEnabledForBundle_00200
 * @tc.name      :
 * @tc.desc      : Set notification enable for bundle into disturbe DB when bundle name is null, return is
 * ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, SetNotificationsEnabledForBundle_00200, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetNotificationsEnabledForBundle(bundleEmptyOption_, false),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : GetNotificationsEnabledForBundle_00100
 * @tc.name      :
 * @tc.desc      : Get notification enable for bundle from disturbe DB, return is ERR_OK.
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationsEnabledForBundle_02900, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetNotificationsEnabledForBundle(bundleOption_, false),
        (int)ERR_OK);
    bool enabled = false;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationsEnabledForBundle(bundleOption_, enabled),
        (int)ERR_OK);
    EXPECT_FALSE(enabled);
}

/**
 * @tc.number    : GetNotificationsEnabledForBundle_00100
 * @tc.name      :
 * @tc.desc      : Get notification enable for bundle from disturbe DB when bundle name is null, return is
 * ERR_ANS_INVALID_PARAM.
 */
HWTEST_F(NotificationPreferencesTest, NotificationPreferencesTest_02900, Function | SmallTest | Level1)
{
    bool enabled = false;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationsEnabledForBundle(bundleEmptyOption_, enabled),
        (int)ERR_ANS_INVALID_PARAM);
}

/**
 * @tc.number    : SetNotificationsEnabled_00100
 * @tc.name      :
 * @tc.desc      : Set enable notification into disturbe DB, return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, SetNotificationsEnabled_00100, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetNotificationsEnabled(true), (int)ERR_OK);
}

/**
 * @tc.number    : GetNotificationsEnabled
 * @tc.name      :
 * @tc.desc      : Get enable notification from disturbe DB, return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, GetNotificationsEnabled_00100, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetNotificationsEnabled(true), (int)ERR_OK);
    bool enable = false;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetNotificationsEnabled(enable), (int)ERR_OK);
    EXPECT_TRUE(enable);
}

/**
 * @tc.number    : SetDisturbMode_00100
 * @tc.name      :
 * @tc.desc      : Set disturbe mode into disturbe DB, return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, SetDisturbMode_00100, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetDisturbMode(NotificationConstant::DisturbMode::ALLOW_ALL),
        (int)ERR_OK);
}

/**
 * @tc.number    : GetDisturbMode_00100
 * @tc.name      :
 * @tc.desc      : Get disturbe mode from disturbe DB, return is ERR_OK
 */
HWTEST_F(NotificationPreferencesTest, GetDisturbMode_00100, Function | SmallTest | Level1)
{
    EXPECT_EQ((int)NotificationPreferences::GetInstance().SetDisturbMode(NotificationConstant::DisturbMode::ALLOW_ALL),
        (int)ERR_OK);
    NotificationConstant::DisturbMode mode;
    EXPECT_EQ((int)NotificationPreferences::GetInstance().GetDisturbMode(mode), (int)ERR_OK);
    EXPECT_EQ((int)NotificationConstant::DisturbMode::ALLOW_ALL, mode);
}

}  // namespace Notification
}  // namespace OHOS
