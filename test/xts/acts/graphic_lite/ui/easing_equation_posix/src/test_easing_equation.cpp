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

#include "animator/easing_equation.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class TestEasingEquation : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_BACKEASEOUT_0100
 * @tc.name     test ui_view BackEaseOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_BackEaseOut_0100, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->BackEaseOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_BACKEASEINOUT_0200
 * @tc.name     test ui_view BackEaseInOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_BackEaseInOut_0200, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->BackEaseInOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_CIRCEASEIN_0300
 * @tc.name     test ui_view CircEaseIn api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_CircEaseIn_0300, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->CircEaseIn(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_CIRCEASEOUT_0400
 * @tc.name     test ui_view CircEaseOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_CircEaseOut_0400, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->CircEaseOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_CIRCEASEINOUT_0500
 * @tc.name     test ui_view CircEaseInOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_CircEaseInOut_0500, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->CircEaseInOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_CUBICEASEIN_0600
 * @tc.name     test ui_view CubicEaseIn api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_CubicEaseIn_0600, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->CubicEaseIn(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_CUBICEASEOUT_0700
 * @tc.name     test ui_view CubicEaseOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_CubicEaseOut_0700, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->CubicEaseOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_CUBICEASEINOUT_0800
 * @tc.name     test ui_view CubicEaseInOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_CubicEaseInOut_0800, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->CubicEaseInOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_LINEAREASENONE_0900
 * @tc.name     test ui_view LinearEaseNone api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_LinearEaseNone_0900, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->LinearEaseNone(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_QUADEASEIN_1000
 * @tc.name     test ui_view QuadEaseIn api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_QuadEaseIn_1000, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->QuadEaseIn(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_QUADEASEOUT_1100
 * @tc.name     test ui_view QuadEaseOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_QuadEaseOut_1100, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->QuadEaseOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_QUADEASEINOUT_1200
 * @tc.name     test ui_view QuadEaseInOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_QuadEaseInOut_1200, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->QuadEaseInOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_QUINTEASEIN_1300
 * @tc.name     test ui_view QuintEaseIn api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_QuintEaseIn_1300, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->QuintEaseIn(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_QUINTEASEOUT_1400
 * @tc.name     test ui_view QuintEaseOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_QuintEaseOut_1400, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->QuintEaseOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_QUINTEASEINOUT_1500
 * @tc.name     test ui_view QuintEaseInOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_QuintEaseInOut_1500, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->QuintEaseInOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_SINEEASEIN_1600
 * @tc.name     test ui_view SineEaseIn api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_SineEaseIn_1600, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->SineEaseIn(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_SINEEASEOUT_1700
 * @tc.name     test ui_view SineEaseOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_SineEaseOut_1700, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->SineEaseOut(0, 1, 10, 5), 1);
    delete animator;
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_SINEEASEINOUT_1800
 * @tc.name     test ui_view SineEaseInOut api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestEasingEquation, Graphic_EasingEquation_Test_SineEaseInOut_1800, Function | MediumTest | Level0)
{
    EasingEquation* animator = new EasingEquation();
    EXPECT_EQ(animator->SineEaseInOut(0, 1, 10, 5), 1);
    delete animator;
}
}