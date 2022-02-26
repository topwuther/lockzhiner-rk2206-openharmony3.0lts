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

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::EventFwk;
using OHOS::Parcel;

static const int SET_COUNT = 1;
static const int MAX_COUNT = 100;

class MatchingSkillsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MatchingSkillsTest::SetUpTestCase(void)
{}

void MatchingSkillsTest::TearDownTestCase(void)
{}

void MatchingSkillsTest::SetUp(void)
{}

void MatchingSkillsTest::TearDown(void)
{}

/*
 * Feature: MatchingSkills
 * Function: AddEntity/GetEntity/CountEntities/HasEntity/RemoveEntity
 * SubFunction: NA
 * FunctionPoints: AddEntity/GetEntity
 * EnvConditions: NA
 * CaseDescription: Verify the function when after add entity can get entity
 * and count entity is right,has entity and can success remove entity
 */
HWTEST_F(MatchingSkillsTest, MatchingSkills_Entity_001, TestSize.Level1)
{
    MatchingSkills matchSkills;
    std::string empty;
    std::string entity = "event.unit.test";
    matchSkills.AddEntity(entity);
    matchSkills.AddEntity(entity);
    EXPECT_EQ(SET_COUNT, matchSkills.CountEntities());

    EXPECT_EQ(true, matchSkills.HasEntity(entity));

    EXPECT_EQ(entity, matchSkills.GetEntity(0));

    EXPECT_EQ(false, entity == matchSkills.GetEntity(MAX_COUNT));

    matchSkills.RemoveEntity(entity);

    matchSkills.RemoveEntity(entity);

    EXPECT_EQ(0, matchSkills.CountEntities());
}

/*
 * Feature: MatchingSkills
 * Function: event
 * SubFunction: NA
 * FunctionPoints: AddEvent/GetEvent/CountEvent/HasEvent/RemoveEvent
 * EnvConditions: NA
 * CaseDescription: Verify the function when after add evnet can get event
 * and count event is right,has event and can success remove event
 */
HWTEST_F(MatchingSkillsTest, MatchingSkills_Event_001, TestSize.Level1)
{
    MatchingSkills matchSkills;
    std::string empty;
    std::string event = "event.unit.test";
    matchSkills.AddEvent(event);
    matchSkills.AddEvent(event);

    EXPECT_EQ(true, matchSkills.HasEvent(event));

    EXPECT_EQ(event, matchSkills.GetEvent(0));

    EXPECT_EQ(false, event == matchSkills.GetEvent(SET_COUNT));
    matchSkills.RemoveEvent(event);

    matchSkills.RemoveEvent(event);

    EXPECT_EQ(0, matchSkills.CountEvent());
}

/*
 * Feature: MatchingSkills
 * Function: scheme
 * SubFunction: NA
 * FunctionPoints: AddScheme/GetScheme/CountScheme/HasScheme/RemoveScheme
 * EnvConditions: NA
 * CaseDescription: Verify the function when after add scheme can get scheme
 * and count scheme is right,has scheme and can success remove scheme
 */
HWTEST_F(MatchingSkillsTest, MatchingSkills_Scheme_001, TestSize.Level1)
{
    MatchingSkills matchSkills;
    std::string empty;
    std::string shceme = "event.unit.test";
    matchSkills.AddScheme(shceme);
    matchSkills.AddScheme(shceme);

    EXPECT_EQ(true, matchSkills.HasScheme(shceme));

    EXPECT_EQ(shceme, matchSkills.GetScheme(0));

    EXPECT_EQ(false, shceme == matchSkills.GetScheme(SET_COUNT));

    matchSkills.RemoveScheme(shceme);

    matchSkills.RemoveScheme(shceme);

    EXPECT_EQ(0, matchSkills.CountSchemes());
}

/*
 * Feature: MatchingSkills
 * Function: Match Event
 * SubFunction: NA
 * FunctionPoints: Match Event
 * EnvConditions: NA
 * CaseDescription: Verify match event
 */
HWTEST_F(MatchingSkillsTest, MatchingSkills_MatchEvent_001, TestSize.Level1)
{
    MatchingSkills matchSkills;
    EXPECT_EQ(false, matchSkills.MatchEvent(""));

    std::string event = "event.unit.test";
    matchSkills.AddEvent(event);
    EXPECT_EQ(true, matchSkills.MatchEvent(event));
}

/*
 * Feature: MatchingSkills
 * Function: Match entity
 * SubFunction: NA
 * FunctionPoints: Match entity
 * EnvConditions: NA
 * CaseDescription: Verify match entity
 */

HWTEST_F(MatchingSkillsTest, MatchingSkills_MatchEntity_001, TestSize.Level1)
{
    MatchingSkills matchSkills;
    std::vector<std::string> entities;
    EXPECT_EQ(true, matchSkills.MatchEntity(entities));

    std::string entity = "event.unit.test";
    matchSkills.AddEntity(entity);
    entities.emplace_back(entity);
    EXPECT_EQ(true, matchSkills.MatchEntity(entities));
    entities.emplace_back("entitydiffer");
    EXPECT_EQ(false, matchSkills.MatchEntity(entities));
}

/*
 * Feature: MatchingSkills
 * Function: Match scheme
 * SubFunction: NA
 * FunctionPoints: Match scheme
 * EnvConditions: NA
 * CaseDescription: Verify match scheme
 */

HWTEST_F(MatchingSkillsTest, MatchingSkills_MatchScheme_001, TestSize.Level1)
{
    MatchingSkills matchSkills;
    EXPECT_EQ(true, matchSkills.MatchScheme(""));
    std::string scheme = "action.system.event";
    matchSkills.AddScheme(scheme);
    EXPECT_EQ(true, matchSkills.MatchScheme(scheme));
    EXPECT_EQ(false, matchSkills.MatchScheme("schemediffer"));
}

/*
 * Feature: Match
 * Function: Match
 * SubFunction: NA
 * FunctionPoints: Match
 * EnvConditions: NA
 * CaseDescription: Verify match
 */

HWTEST_F(MatchingSkillsTest, MatchingSkills_Match_001, TestSize.Level1)
{
    MatchingSkills matchSkills;
    std::string event = "event.unit.test";
    matchSkills.AddEvent(event);
    std::string entity;
    matchSkills.AddEntity(entity);
    Want want;
    want.AddEntity(entity);
    want.SetAction(event);
    EXPECT_EQ(true, matchSkills.Match(want));
}