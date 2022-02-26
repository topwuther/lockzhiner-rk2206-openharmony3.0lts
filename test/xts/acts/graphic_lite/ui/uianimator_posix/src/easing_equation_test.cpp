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
class EasingEquationTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        if (easingEquationVar == nullptr) {
            easingEquationVar = new EasingEquation();
        }
    }
    static void TearDownTestCase(void)
    {
        if (easingEquationVar != nullptr) {
            delete easingEquationVar;
            easingEquationVar = nullptr;
        }
    }
    static EasingEquation* easingEquationVar;
};
EasingEquation* EasingEquationTest::easingEquationVar = nullptr;

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_SETBACKOVERSHOOT_0100
 * @tc.name     test easing-equation set-backovershoot api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(EasingEquationTest, Graphic_UIEasingEquationTest_Test_SetBackOvershoot_0100, Function | MediumTest | Level0)
{
    double overshoot = 2.8;
    easingEquationVar->SetBackOvershoot(overshoot);
    EXPECT_EQ(EasingEquation::BackEaseIn(1, 1, 1, 2), 1);
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_BACKEASEIN_0200
 * @tc.name     test easing-equation back-ease-in api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(EasingEquationTest, Graphic_UIEasingEquationTest_Test_BackEaseIn_0200, Function | MediumTest | Level0)
{
    EXPECT_EQ(EasingEquation::BackEaseIn(1, 1, 1, 2), 1);
}

/**
 * @tc.number   SUB_GRAPHIC_EASING_EQUATION_BACKEASEOUT_0300
 * @tc.name     test easing-equation back-ease-out api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(EasingEquationTest, Graphic_UIEasingEquationTest_Test_BackEaseOut_0300, Function | MediumTest | Level0)
{
    EXPECT_EQ(EasingEquation::BackEaseOut(1, 1, 1, 2), 1);
}
}