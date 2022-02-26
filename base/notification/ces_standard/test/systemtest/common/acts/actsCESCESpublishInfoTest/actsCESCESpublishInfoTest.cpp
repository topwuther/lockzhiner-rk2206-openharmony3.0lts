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
#include "common_event_manager.h"
#define private public
#define protected public
#include "common_event_manager_service.h"
#undef private
#undef protected

#include "common_event_subscribe_info.h"

#include "testConfigParser.h"
#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace EventFwk {

class ActsCommonEventServicesSystemTest : public CommonEventSubscriber {
public:
    explicit ActsCommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~ActsCommonEventServicesSystemTest(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

ActsCommonEventServicesSystemTest::ActsCommonEventServicesSystemTest(const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void ActsCommonEventServicesSystemTest::OnReceiveEvent(const CommonEventData &data)
{}

class ActsCESCESpublishInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static StressTestLevel stLevel_;
};
StressTestLevel ActsCESCESpublishInfoTest::stLevel_{};

void ActsCESCESpublishInfoTest::SetUpTestCase()
{
    TestConfigParser tcp;
    tcp.ParseFromFile4StressTest(STRESS_TEST_CONFIG_FILE_PATH, stLevel_);
    std::cout << "stress test level : "
              << "AMS : " << stLevel_.AMSLevel << " "
              << "BMS : " << stLevel_.BMSLevel << " "
              << "CES : " << stLevel_.CESLevel << std::endl;
}

void ActsCESCESpublishInfoTest::TearDownTestCase()
{}

void ActsCESCESpublishInfoTest::SetUp()
{}

void ActsCESCESpublishInfoTest::TearDown()
{}

/*
 * @tc.number: CES_PublishInfo_Sticky_0100
 * @tc.name: CommonEventPublishInfo : SetSticky and IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetSticky and IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "StickyFalseTest001";
    bool sticky = false;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(sticky);
        if (publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0100 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0100 stress level: " << stLevel_.CESLevel;
        GTEST_LOG_(INFO) << "CES_SetEventAuthority_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Sticky_0200
 * @tc.name: CommonEventPublishInfo : SetSticky and IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetSticky and IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "StickyFalseTest001";
    bool sticky = true;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(sticky);
        if (!publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0200 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Sticky_0300
 * @tc.name: CommonEventPublishInfo : SetSticky and IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetSticky and IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "StickyFalseTest003";
    bool sticky = false;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(sticky);
        if (publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0300 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Sticky_0400
 * @tc.name: CommonEventPublishInfo : SetSticky and IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetSticky and IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "StickyTrueTest004";
    bool sticky = true;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(sticky);
        if (!publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0400 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Sticky_0500
 * @tc.name: CommonEventPublishInfo : IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_0500, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "StickyFalseTest005";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        if (publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0500 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Sticky_0600
 * @tc.name: CommonEventPublishInfo : SetSticky and IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetSticky and IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_0600, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "StickyTrueTest006";
    bool sticky = true;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(sticky);
        EXPECT_TRUE(publishInfo.IsSticky());
        sticky = false;
        publishInfo.SetSticky(sticky);
        if (publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0600 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        sticky = true;
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0600 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Sticky_0700
 * @tc.name: CommonEventPublishInfo : SetSticky and IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetSticky and IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "StickyTrueTest007";
    bool sticky = false;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(sticky);
        EXPECT_FALSE(publishInfo.IsSticky());
        sticky = true;
        publishInfo.SetSticky(sticky);
        if (!publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0700 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        sticky = false;
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0700 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Sticky_0800
 * @tc.name: CommonEventPublishInfo : SetSticky and IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetSticky and IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_0800, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "StickyFalseTest008";
    bool sticky = false;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(sticky);
        EXPECT_FALSE(publishInfo.IsSticky());
        sticky = true;
        publishInfo.SetSticky(sticky);
        if (!publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0800 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        sticky = false;
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0800 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Sticky_0900
 * @tc.name: CommonEventPublishInfo : SetSticky and IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetSticky and IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_0900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "StickyFalseTest009";
    bool sticky = true;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(sticky);
        EXPECT_TRUE(publishInfo.IsSticky());
        sticky = false;
        publishInfo.SetSticky(sticky);
        if (publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0900 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        sticky = true;
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_0900 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Sticky_1000
 * @tc.name: CommonEventPublishInfo : SetSticky and IsSticky
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetSticky and IsSticky
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Sticky_1000, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "StickyFalseTest010";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        EXPECT_FALSE(publishInfo.IsSticky());
        bool sticky = true;
        publishInfo.SetSticky(sticky);
        if (!publishInfo.IsSticky()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_1000 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Sticky_1000 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_0100
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "PermissionsTest001";
    std::string eventAction = "PermissionsTest001";
    std::string permissin = "PERMISSION1";
    std::vector<std::string> permissins;
    permissins.push_back(permissin);
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (!(permissin == testPermissions[0])) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0100 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_0200
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_0200, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "PermissionsTest002";
    std::string eventAction = "PermissionsTest002";
    std::string permissin1 = "PERMISSION2";
    std::string permissin2 = "PERMISSION3";
    std::vector<std::string> permissins;
    permissins.push_back(permissin1);
    permissins.push_back(permissin2);
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (!(permissin2 == testPermissions[1])) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0200 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_0300
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_0300, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "PermissionsTest003";
    std::string eventAction = "PermissionsTest003";
    std::string permissin1 = "PERMISSION4";
    std::string permissin2 = "PERMISSION5";
    std::string permissin3 = "PERMISSION6";
    std::vector<std::string> permissins;
    permissins.push_back(permissin1);
    permissins.push_back(permissin2);
    permissins.push_back(permissin3);
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (!(permissin3 == testPermissions[2])) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0300 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_0400
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_0400, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "PermissionsTest004";
    std::string eventAction = "PermissionsTest004";
    std::string permissin = "PERMISSION1";
    std::string teststr = "test";
    std::vector<std::string> permissins;
    permissins.push_back(permissin);
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (teststr == testPermissions[0]) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0400 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_0500
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_0500, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "PermissionsTest005";
    std::string eventAction = "PermissionsTest005";
    std::string permissin1 = "PERMISSION2";
    std::string permissin2 = "PERMISSION3";
    std::vector<std::string> permissins;
    permissins.push_back(permissin1);
    permissins.push_back(permissin2);
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (permissin1 == testPermissions[1]) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0500 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_0600
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_0600, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "PermissionsTest006";
    std::string eventAction = "PermissionsTest006";
    std::string permissin1 = "PERMISSION4";
    std::string permissin2 = "PERMISSION5";
    std::string permissin3 = "PERMISSION6";
    std::vector<std::string> permissins;
    permissins.push_back(permissin1);
    permissins.push_back(permissin2);
    permissins.push_back(permissin3);
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (permissin1 == testPermissions[2]) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0600 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0600 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_0700
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_0700, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "PermissionsTest007";
    std::string eventAction = "PermissionsTest007";
    std::string permissin1 = "PERMISSION4";
    std::vector<std::string> permissins;
    permissins.push_back(permissin1);
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (!(1 == testPermissions.size())) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0700 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0700 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_0800
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_0800, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "PermissionsTest008";
    std::string eventAction = "PermissionsTest008";
    std::string permissin1 = "PERMISSION4";
    std::string permissin2 = "PERMISSION5";
    std::vector<std::string> permissins;
    permissins.push_back(permissin1);
    permissins.push_back(permissin2);
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (!(2 == testPermissions.size())) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0800 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0800 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_0900
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_0900, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "PermissionsTest009";
    std::string eventAction = "PermissionsTest009";
    std::string permissin1 = "PERMISSION4";
    std::string permissin2 = "PERMISSION5";
    std::string permissin3 = "PERMISSION6";
    std::vector<std::string> permissins;
    permissins.push_back(permissin1);
    permissins.push_back(permissin2);
    permissins.push_back(permissin3);
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (!(3 == testPermissions.size())) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0900 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_0900 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Permissions_1000
 * @tc.name: CommonEventPublishInfo : GetSubscriberPermissions
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: GetSubscriberPermissions
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Permissions_1000, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "PermissionsTest010";
    std::string eventAction = "PermissionsTest010";
    std::vector<std::string> permissins;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissins);
        std::vector<std::string> testPermissions = publishInfo.GetSubscriberPermissions();
        if (!(0 == testPermissions.size())) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_1000 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Permissions_1000 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_0100
 * @tc.name: CommonEventPublishInfo : SetOrdered and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "OrderedTest001";
    bool order = false;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(order);
        if (publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0100 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_0200
 * @tc.name: CommonEventPublishInfo : SetOrdered and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "OrderedTest002";
    bool order = true;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(order);
        if (!publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0200 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_0300
 * @tc.name: CommonEventPublishInfo : SetOrdered and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_0300, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "OrderedTest003";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        if (publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0300 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_0400
 * @tc.name: CommonEventPublishInfo : SetOrdered and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_0400, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "OrderedTest004";
    bool order = false;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(order);
        if (publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0400 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_0500
 * @tc.name: CommonEventPublishInfo : SetOrdered and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_0500, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "OrderedTest005";
    bool order = true;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(order);
        if (!publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0500 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_0600
 * @tc.name: CommonEventPublishInfo : SetOrdered and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_0600, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "OrderedTest006";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        if (publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0600 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0600 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_0700
 * @tc.name: CommonEventPublishInfo : SetOrdered SetSticky and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered SetSticky and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_0700, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "OrderedTest007";
    bool order = true;
    bool sticky = true;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(order);
        publishInfo.SetSticky(sticky);
        if (!publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0700 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0700 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_0800
 * @tc.name: CommonEventPublishInfo : SetOrdered SetSticky and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered SetSticky and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_0800, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "OrderedTest008";
    bool order = true;
    bool sticky = false;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(order);
        publishInfo.SetSticky(sticky);
        if (!publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0800 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0800 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_0900
 * @tc.name: CommonEventPublishInfo : SetOrdered SetSticky and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered SetSticky and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_0900, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "OrderedTest009";
    bool order = false;
    bool sticky = false;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(order);
        publishInfo.SetSticky(sticky);
        if (publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0900 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_0900 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_PublishInfo_Ordered_1000
 * @tc.name: CommonEventPublishInfo : SetOrdered SetSticky and IsOrdered
 * @tc.desc: 1.Set publishInfo
 *           2.Verify the function of CommonEventPublishInfo: SetOrdered SetSticky and IsOrdered
 */
HWTEST_F(ActsCESCESpublishInfoTest, CES_PublishInfo_Ordered_1000, Function | MediumTest | Level2)
{
    bool result = false;
    std::string eventName = "OrderedTest010";
    bool order = false;
    bool sticky = true;
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        CommonEventPublishInfo publishInfo;
        publishInfo.SetOrdered(order);
        publishInfo.SetSticky(sticky);
        if (publishInfo.IsOrdered()) {
            GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_1000 faild, frequency: " << i;
            result = false;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_PublishInfo_Ordered_1000 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}
}  // namespace EventFwk
}  // namespace OHOS