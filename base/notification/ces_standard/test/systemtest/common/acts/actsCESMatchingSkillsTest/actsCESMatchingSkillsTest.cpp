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
#define private public
#define protected public
#include "matching_skills.h"
#undef private
#undef protected

#include "testConfigParser.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace EventFwk {

class ActsCESMatchingSkillsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static StressTestLevel stLevel_;
};
StressTestLevel ActsCESMatchingSkillsTest::stLevel_{};

void ActsCESMatchingSkillsTest::SetUpTestCase()
{
    TestConfigParser tcp;
    tcp.ParseFromFile4StressTest(STRESS_TEST_CONFIG_FILE_PATH, stLevel_);
    std::cout << "stress test level : "
              << "AMS : " << stLevel_.AMSLevel << " "
              << "BMS : " << stLevel_.BMSLevel << " "
              << "CES : " << stLevel_.CESLevel << std::endl;
}

void ActsCESMatchingSkillsTest::TearDownTestCase()
{}

void ActsCESMatchingSkillsTest::SetUp()
{}

void ActsCESMatchingSkillsTest::TearDown()
{}

/*
 * @tc.number: CES_MatchingSkills_Entity_0100
 * @tc.name: AddEntity、GetEntity、CountEntities、HasEntity and RemoveEntity
 * @tc.desc: Verify the function of  MatchingSkills entity: AddEntity、GetEntity、
 *           CountEntities、HasEntity and RemoveEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Entity_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "entityTest";
    std::string entity = "entity";
    std::string entityRemove = "entityRemove";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        matchingSkills.AddEntity(entityRemove);

        EXPECT_EQ(entity, matchingSkills.GetEntity(0));
        EXPECT_TRUE(matchingSkills.HasEvent(eventName));
        EXPECT_EQ(false, entity == matchingSkills.GetEntity(1000));
        if (matchingSkills.HasEntity(entity)) {
            matchingSkills.RemoveEntity(entity);
        }

        EXPECT_EQ(1, matchingSkills.CountEntities());
        EXPECT_FALSE(matchingSkills.HasEntity(entity));
        matchingSkills.RemoveEntity(entityRemove);
        if (matchingSkills.CountEntities() != 0) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Entity_0200
 * @tc.name: AddEntity、GetEntity、CountEntities、HasEntity and RemoveEntity
 * @tc.desc: Verify the function of  MatchingSkills entity with different name of event and entity:
 *           AddEntity、GetEntity、CountEntities、HasEntity and RemoveEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Entity_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "EventEntityTest";
    std::string entity = "EventEntityTest";
    std::string entityRemove = "EventEntityTestRemove";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        matchingSkills.AddEntity(entityRemove);

        EXPECT_EQ(entity, matchingSkills.GetEntity(0));
        EXPECT_TRUE(matchingSkills.HasEvent(eventName));
        EXPECT_EQ(false, entity == matchingSkills.GetEntity(1000));
        if (matchingSkills.HasEntity(entity)) {
            matchingSkills.RemoveEntity(entity);
        }

        EXPECT_EQ(1, matchingSkills.CountEntities());
        EXPECT_FALSE(matchingSkills.HasEntity(entity));
        matchingSkills.RemoveEntity(entityRemove);
        if (matchingSkills.CountEntities() != 0) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Entity_0300
 * @tc.name: AddEntity、GetEntity、CountEntities、HasEntity and RemoveEntity
 * @tc.desc: Verify the function of  MatchingSkills entity with different name of event and entity:
 *           AddEntity、GetEntity、CountEntities、HasEntity and RemoveEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Entity_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "EventEntityTest2";
    std::string entity = "EventEntityTes2t";
    std::string entityRemove = "EventEntityTestRemove2";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEntity(entity);
        matchingSkills.AddEntity(entityRemove);

        EXPECT_EQ(entity, matchingSkills.GetEntity(0));
        EXPECT_TRUE(matchingSkills.HasEvent(eventName));
        EXPECT_EQ(false, entity == matchingSkills.GetEntity(1000));
        EXPECT_EQ(true, "" == matchingSkills.GetEntity(1000));
        if (matchingSkills.HasEntity(entity)) {
            matchingSkills.RemoveEntity(entity);
        }

        EXPECT_EQ(1, matchingSkills.CountEntities());
        EXPECT_FALSE(matchingSkills.HasEntity(entity));
        matchingSkills.RemoveEntity(entityRemove);
        if (matchingSkills.CountEntities() != 0) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Entity_0400
 * @tc.name: AddEntity、GetEntity、CountEntities、HasEntity and RemoveEntity
 * @tc.desc: Verify the function of  MatchingSkills entity with different name of event and entity:
 *           AddEntity、GetEntity、CountEntities、HasEntity and RemoveEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Entity_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "EventEntityTest3";
    std::string eventName2 = "EventEntityTest4";
    std::string eventName3 = "EventEntityTest5";
    std::string eventName4 = "EventEntityTest6";
    std::string eventName5 = "EventEntityTest7";
    std::string entity = "EventEntityTest3";
    std::string entityRemove = "EventEntityTestRemove3";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEvent(eventName2);
        matchingSkills.AddEvent(eventName3);
        matchingSkills.AddEvent(eventName4);
        matchingSkills.AddEvent(eventName5);

        matchingSkills.AddEntity(entity);
        matchingSkills.AddEntity(entityRemove);

        EXPECT_EQ(5, matchingSkills.CountEvent());
        EXPECT_EQ(eventName4, matchingSkills.GetEvent(3));
        EXPECT_EQ(entity, matchingSkills.GetEntity(0));
        EXPECT_TRUE(matchingSkills.HasEvent(eventName));
        EXPECT_EQ(false, entity == matchingSkills.GetEntity(1000));
        EXPECT_EQ(true, "" == matchingSkills.GetEntity(1000));

        if (matchingSkills.HasEntity(entity)) {
            matchingSkills.RemoveEntity(entity);
        }
        EXPECT_EQ(1, matchingSkills.CountEntities());
        EXPECT_FALSE(matchingSkills.HasEntity(entity));

        matchingSkills.RemoveEntity(entityRemove);
        if (matchingSkills.CountEntities() != 0) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        matchingSkills.RemoveEvent(eventName2);
        matchingSkills.RemoveEvent(eventName3);
        matchingSkills.RemoveEvent(eventName4);
        matchingSkills.RemoveEvent(eventName5);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Entity_0500
 * @tc.name: AddEntity、GetEntity、CountEntities、HasEntity and RemoveEntity
 * @tc.desc: Verify the function of  MatchingSkills entity with different name of event and entity:
 *           AddEntity、GetEntity、CountEntities、HasEntity and RemoveEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Entity_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "EventEntityTest3";
    std::string eventName2 = "EventEntityTest4";
    std::string eventName3 = "EventEntityTest5";
    std::string eventName4 = "EventEntityTest6";
    std::string eventName5 = "EventEntityTest7";
    std::string eventName6 = "EventEntityTest8";
    std::string eventName7 = "EventEntityTest9";
    std::string eventName8 = "EventEntityTest10";
    std::string eventName9 = "EventEntityTest11";
    std::string eventName10 = "EventEntityTest12";
    std::string entity = "EventEntityTest3";
    std::string entityRemove = "EventEntityTestRemove3";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        matchingSkills.AddEvent(eventName2);
        matchingSkills.AddEvent(eventName3);
        matchingSkills.AddEvent(eventName4);
        matchingSkills.AddEvent(eventName5);
        matchingSkills.AddEvent(eventName6);
        matchingSkills.AddEvent(eventName7);
        matchingSkills.AddEvent(eventName8);
        matchingSkills.AddEvent(eventName9);
        matchingSkills.AddEvent(eventName10);

        matchingSkills.AddEntity(entity);
        matchingSkills.AddEntity(entityRemove);

        EXPECT_EQ(10, matchingSkills.CountEvent());
        EXPECT_EQ(eventName4, matchingSkills.GetEvent(3));
        EXPECT_EQ(entityRemove, matchingSkills.GetEntity(1));
        EXPECT_TRUE(matchingSkills.HasEvent(eventName));
        EXPECT_EQ(false, entity == matchingSkills.GetEntity(1000));
        EXPECT_EQ(true, "" == matchingSkills.GetEntity(1000));
        if (matchingSkills.HasEntity(entity)) {
            matchingSkills.RemoveEntity(entity);
        }

        EXPECT_EQ(1, matchingSkills.CountEntities());
        EXPECT_FALSE(matchingSkills.HasEntity(entity));
        matchingSkills.RemoveEntity(entityRemove);
        if (matchingSkills.CountEntities() != 0) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        matchingSkills.RemoveEvent(eventName2);
        matchingSkills.RemoveEvent(eventName3);
        matchingSkills.RemoveEvent(eventName4);
        matchingSkills.RemoveEvent(eventName5);
        matchingSkills.RemoveEvent(eventName6);
        matchingSkills.RemoveEvent(eventName7);
        matchingSkills.RemoveEvent(eventName8);
        matchingSkills.RemoveEvent(eventName9);
        matchingSkills.RemoveEvent(eventName10);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Entity_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Scheme_0100
 * @tc.name: AddScheme、GetScheme、CountSchemes、HasScheme and RemoveScheme
 * @tc.desc: Verify the function of  MatchingSkills scheme : AddScheme、GetScheme、
 *           CountSchemes、HasScheme and RemoveScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Scheme_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "EventSchemeTest";
    std::string shceme = "shceme";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddScheme(shceme);
        EXPECT_TRUE(matchingSkills.HasScheme(shceme));
        EXPECT_EQ(shceme, matchingSkills.GetScheme(0));
        EXPECT_EQ(false, shceme == matchingSkills.GetScheme(1000));
        matchingSkills.RemoveScheme(shceme);
        if (matchingSkills.CountSchemes() != 0) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Scheme_0200
 * @tc.name: AddScheme、GetScheme、CountSchemes、HasScheme and RemoveScheme
 * @tc.desc: Verify the function of  MatchingSkills scheme : AddScheme、GetScheme、
 *           CountSchemes、HasScheme and RemoveScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Scheme_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "EventSchemeTest2";
    std::string shceme = "shceme2";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddScheme(shceme);
        EXPECT_TRUE(matchingSkills.HasScheme(shceme));
        EXPECT_EQ(shceme, matchingSkills.GetScheme(0));
        EXPECT_EQ(false, shceme == matchingSkills.GetScheme(1000));
        matchingSkills.RemoveScheme(shceme);
        if (matchingSkills.CountSchemes() != 0) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Scheme_0300
 * @tc.name: AddScheme、GetScheme、CountSchemes、HasScheme and RemoveScheme
 * @tc.desc: Verify the function of  MatchingSkills scheme : AddScheme、GetScheme、
 *           CountSchemes、HasScheme and RemoveScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Scheme_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "EventSchemeTest3";
    std::string shceme = "shceme3";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddScheme(shceme);
        EXPECT_TRUE(matchingSkills.HasScheme(shceme));
        EXPECT_EQ(shceme, matchingSkills.GetScheme(0));
        EXPECT_EQ(false, shceme == matchingSkills.GetScheme(1000));
        matchingSkills.RemoveScheme(shceme);
        if (matchingSkills.CountSchemes() != 0) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Scheme_0400
 * @tc.name: AddScheme、GetScheme、CountSchemes、HasScheme and RemoveScheme
 * @tc.desc: Verify the function of  MatchingSkills scheme : AddScheme、GetScheme、
 *           CountSchemes、HasScheme and RemoveScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Scheme_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "EventSchemeTest2";
    std::string shceme = "shceme2";
    std::string shceme2 = "shceme3";
    std::string shceme3 = "shceme4";
    std::string shceme4 = "shceme5";
    std::string shceme5 = "shceme6";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddScheme(shceme);
        matchingSkills.AddScheme(shceme2);
        matchingSkills.AddScheme(shceme3);
        matchingSkills.AddScheme(shceme4);
        matchingSkills.AddScheme(shceme5);

        EXPECT_TRUE(matchingSkills.HasScheme(shceme2));
        EXPECT_EQ(shceme, matchingSkills.GetScheme(0));
        EXPECT_EQ(false, shceme == matchingSkills.GetScheme(1000));
        EXPECT_EQ(true, "" == matchingSkills.GetScheme(1000));
        matchingSkills.RemoveScheme(shceme);

        if (matchingSkills.CountSchemes() != 4) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveScheme(shceme2);
        matchingSkills.RemoveScheme(shceme3);
        matchingSkills.RemoveScheme(shceme4);
        matchingSkills.RemoveScheme(shceme5);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Scheme_0500
 * @tc.name: AddScheme、GetScheme、CountSchemes、HasScheme and RemoveScheme
 * @tc.desc: Verify the function of  MatchingSkills scheme : AddScheme、GetScheme、
 *           CountSchemes、HasScheme and RemoveScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Scheme_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "EventSchemeTest2";
    std::string shceme = "shceme2";
    std::string shceme2 = "shceme3";
    std::string shceme3 = "shceme4";
    std::string shceme4 = "shceme5";
    std::string shceme5 = "shceme6";
    std::string shceme6 = "shceme7";
    std::string shceme7 = "shceme8";
    std::string shceme8 = "shceme9";
    std::string shceme9 = "shceme10";
    std::string shceme10 = "shceme11";
    MatchingSkills matchingSkills;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddScheme(shceme);
        matchingSkills.AddScheme(shceme2);
        matchingSkills.AddScheme(shceme3);
        matchingSkills.AddScheme(shceme4);
        matchingSkills.AddScheme(shceme5);
        matchingSkills.AddScheme(shceme6);
        matchingSkills.AddScheme(shceme7);
        matchingSkills.AddScheme(shceme8);
        matchingSkills.AddScheme(shceme9);
        matchingSkills.AddScheme(shceme10);

        EXPECT_TRUE(matchingSkills.HasScheme(shceme2));
        EXPECT_EQ(shceme, matchingSkills.GetScheme(0));
        EXPECT_EQ(shceme10, matchingSkills.GetScheme(9));
        EXPECT_EQ(false, shceme == matchingSkills.GetScheme(1000));
        EXPECT_EQ(true, "" == matchingSkills.GetScheme(1000));
        matchingSkills.RemoveScheme(shceme);

        if (matchingSkills.CountSchemes() != 9) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveScheme(shceme2);
        matchingSkills.RemoveScheme(shceme3);
        matchingSkills.RemoveScheme(shceme4);
        matchingSkills.RemoveScheme(shceme5);
        matchingSkills.RemoveScheme(shceme6);
        matchingSkills.RemoveScheme(shceme7);
        matchingSkills.RemoveScheme(shceme8);
        matchingSkills.RemoveScheme(shceme9);
        matchingSkills.RemoveScheme(shceme10);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Scheme_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEvent_0100
 * @tc.name: MatchEvent and AddEvent
 * @tc.desc: Verify the function of  MatchingSkills event : MatchEvent、AddEvent
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEvent_0100, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string eventName = "matchEvent";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_FALSE(matchingSkills.MatchEvent(""));
        matchingSkills.AddEvent(eventName);
        if (!matchingSkills.MatchEvent(eventName)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEvent_0200
 * @tc.name: MatchEvent and AddEvent
 * @tc.desc: Verify the function of  MatchingSkills event : MatchEvent、AddEvent
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEvent_0200, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string eventName = "matchEvent2";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_FALSE(matchingSkills.MatchEvent(""));
        matchingSkills.AddEvent(eventName);
        if (!matchingSkills.MatchEvent(eventName)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEvent_0300
 * @tc.name: MatchEvent and AddEvent
 * @tc.desc: Verify the function of  MatchingSkills event : MatchEvent、AddEvent
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEvent_0300, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string eventName = "matchEvent3";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_FALSE(matchingSkills.MatchEvent(""));
        matchingSkills.AddEvent(eventName);
        if (!matchingSkills.MatchEvent(eventName)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEvent_0400
 * @tc.name: MatchEvent and AddEvent
 * @tc.desc: Verify the function of  MatchingSkills event : MatchEvent、AddEvent
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEvent_0400, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string eventName = "matchEvent4";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_FALSE(matchingSkills.MatchEvent(""));
        matchingSkills.AddEvent(eventName);
        if (!matchingSkills.MatchEvent(eventName)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEvent_0500
 * @tc.name: MatchEvent and AddEvent
 * @tc.desc: Verify the function of  MatchingSkills event : MatchEvent、AddEvent
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEvent_0500, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string eventName = "matchEvent5";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_FALSE(matchingSkills.MatchEvent(""));
        matchingSkills.AddEvent(eventName);
        if (!matchingSkills.MatchEvent(eventName)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEvent_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEntity_0100
 * @tc.name: MatchEntity and AddEntity
 * @tc.desc: Verify the function of  MatchingSkills entity : MatchEntity、AddEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEntity_0100, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::vector<std::string> entityVector;
    std::string entity = "matchEntity";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));
        matchingSkills.AddEntity(entity);
        entityVector.emplace_back(entity);
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));
        entityVector.emplace_back("entityTestMatchEntity");
        if (matchingSkills.MatchEntity(entityVector)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        entityVector.clear();
        matchingSkills.RemoveEntity(entity);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEntity_0200
 * @tc.name: MatchEntity and AddEntity
 * @tc.desc: Verify the function of  MatchingSkills entity : MatchEntity、AddEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEntity_0200, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::vector<std::string> entityVector;
    std::string entity = "matchEntity2";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));
        matchingSkills.AddEntity(entity);
        entityVector.emplace_back(entity);
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));

        entityVector.emplace_back("entityTestMatchEntity2");
        if (matchingSkills.MatchEntity(entityVector)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEntity(entity);
        entityVector.clear();
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEntity_0300
 * @tc.name: MatchEntity and AddEntity
 * @tc.desc: Verify the function of  MatchingSkills entity : MatchEntity、AddEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEntity_0300, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::vector<std::string> entityVector;
    std::string entity = "matchEntity3";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));
        matchingSkills.AddEntity(entity);
        entityVector.emplace_back(entity);
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));

        entityVector.emplace_back("entityTestMatchEntity3");
        if (matchingSkills.MatchEntity(entityVector)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEntity(entity);
        entityVector.clear();
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEntity_0400
 * @tc.name: MatchEntity and AddEntity
 * @tc.desc: Verify the function of  MatchingSkills entity : MatchEntity、AddEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEntity_0400, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::vector<std::string> entityVector;
    std::string entity = "matchEntity4";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));
        matchingSkills.AddEntity(entity);
        entityVector.emplace_back(entity);
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));

        entityVector.emplace_back("entityTestMatchEntity4");
        if (matchingSkills.MatchEntity(entityVector)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEntity(entity);
        entityVector.clear();
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchEntity_0500
 * @tc.name: MatchEntity and AddEntity
 * @tc.desc: Verify the function of  MatchingSkills entity : MatchEntity、AddEntity
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchEntity_0500, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::vector<std::string> entityVector;
    std::string entity = "matchEntity5";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));
        matchingSkills.AddEntity(entity);
        entityVector.emplace_back(entity);
        EXPECT_TRUE(matchingSkills.MatchEntity(entityVector));

        entityVector.emplace_back("entityTestMatchEntity5");
        if (matchingSkills.MatchEntity(entityVector)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEntity(entity);
        entityVector.clear();
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchEntity_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchScheme_0100
 * @tc.name: MatchScheme and AddScheme
 * @tc.desc: Verify the function of  MatchingSkills scheme : MatchScheme、AddScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchScheme_0100, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string scheme = "schemeMatchSchemeTest";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_TRUE(matchingSkills.MatchScheme(""));
        matchingSkills.AddScheme(scheme);
        EXPECT_TRUE(matchingSkills.MatchScheme(scheme));

        if (matchingSkills.MatchScheme("schemeMatchScheme")) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveScheme(scheme);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchScheme_0200
 * @tc.name: check MatchScheme of MatchingSkills
 * @tc.desc: verify the function of  MatchingSkills: MatchScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchScheme_0200, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string scheme = "schemeMatchScheme";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        EXPECT_TRUE(matchingSkills.MatchScheme(""));
        matchingSkills.AddScheme(scheme);
        EXPECT_TRUE(matchingSkills.MatchScheme(scheme));

        if (!matchingSkills.MatchScheme("schemeMatchScheme")) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveScheme(scheme);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchScheme_0300
 * @tc.name: MatchScheme
 * @tc.desc: verify the function of  MatchingSkills: MatchScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchScheme_0300, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string scheme = "MatchScheme";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddScheme(scheme);
        EXPECT_TRUE(matchingSkills.MatchScheme(scheme));

        if (!matchingSkills.MatchScheme("MatchScheme")) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveScheme(scheme);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchScheme_0400
 * @tc.name: MatchScheme
 * @tc.desc: verify the function of  MatchingSkills: MatchScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchScheme_0400, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string scheme = "MatchScheme2";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddScheme(scheme);
        EXPECT_TRUE(matchingSkills.MatchScheme(scheme));

        if (!matchingSkills.MatchScheme("MatchScheme2")) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveScheme(scheme);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_MatchScheme_0500
 * @tc.name: MatchScheme
 * @tc.desc: verify the function of MatchingSkills: MatchScheme
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_MatchScheme_0500, Function | MediumTest | Level1)
{
    bool result = false;
    MatchingSkills matchingSkills;
    std::string scheme = "MatchScheme5";
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddScheme(scheme);
        EXPECT_TRUE(matchingSkills.MatchScheme(scheme));

        if (!matchingSkills.MatchScheme("MatchScheme5")) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveScheme(scheme);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_MatchScheme_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Match_0100
 * @tc.name: Match
 * @tc.desc: verify the function of MatchingSkills: March Want
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Match_0100, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName = "matchTest";
    MatchingSkills matchingSkills;
    Want want;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName);
        std::string entity = "entity";
        matchingSkills.AddEntity(entity);
        want.AddEntity(entity);
        want.SetAction(eventName);

        if (!matchingSkills.Match(want)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0100 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName);
        matchingSkills.RemoveEntity(entity);
        want.RemoveEntity(entity);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0100 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Match_0200
 * @tc.name: Match
 * @tc.desc: verify the function of MatchingSkills: March Want
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Match_0200, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName2 = "matchTest2";
    MatchingSkills matchingSkills;
    Want want;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName2);
        std::string entity = "entity2";
        matchingSkills.AddEntity(entity);

        want.AddEntity(entity);
        want.SetAction(eventName2);
        if (!matchingSkills.Match(want)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0200 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName2);
        matchingSkills.RemoveEntity(entity);
        want.RemoveEntity(entity);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0200 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Match_0300
 * @tc.name: Match
 * @tc.desc: verify the function of MatchingSkills: March Want
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Match_0300, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName3 = "matchTest3";
    MatchingSkills matchingSkills;
    Want want;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName3);
        std::string entity = "entity3";
        matchingSkills.AddEntity(entity);
        want.AddEntity(entity);
        want.SetAction(eventName3);
        if (!matchingSkills.Match(want)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0300 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName3);
        matchingSkills.RemoveEntity(entity);
        want.RemoveEntity(entity);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0300 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Match_0400
 * @tc.name: Match
 * @tc.desc: verify the function of MatchingSkills: March Want
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Match_0400, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName4 = "matchTest4";
    MatchingSkills matchingSkills;
    Want want;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName4);
        std::string entity = "entity4";
        matchingSkills.AddEntity(entity);
        want.AddEntity(entity);
        want.SetAction(eventName4);
        if (!matchingSkills.Match(want)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0400 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName4);
        matchingSkills.RemoveEntity(entity);
        want.RemoveEntity(entity);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0400 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

/*
 * @tc.number: CES_MatchingSkills_Match_0500
 * @tc.name: Match
 * @tc.desc: verify the function of MatchingSkills: March Want
 */
HWTEST_F(ActsCESMatchingSkillsTest, CES_MatchingSkills_Match_0500, Function | MediumTest | Level1)
{
    bool result = false;
    std::string eventName5 = "matchTest5";
    MatchingSkills matchingSkills;
    Want want;
    for (int i = 1; i <= stLevel_.CESLevel; i++) {
        matchingSkills.AddEvent(eventName5);
        std::string entity = "entity5";
        matchingSkills.AddEntity(entity);
        want.AddEntity(entity);
        want.SetAction(eventName5);
        if (!matchingSkills.Match(want)) {
            result = false;
            GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0500 faild, frequency: " << i;
            break;
        } else {
            result = true;
        }
        matchingSkills.RemoveEvent(eventName5);
        matchingSkills.RemoveEntity(entity);
        want.RemoveEntity(entity);
    }
    if (result && stLevel_.CESLevel >= 1) {
        GTEST_LOG_(INFO) << "CES_MatchingSkills_Match_0500 stress level: " << stLevel_.CESLevel;
    }
    EXPECT_TRUE(result);
}

}  // namespace EventFwk
}  // namespace OHOS