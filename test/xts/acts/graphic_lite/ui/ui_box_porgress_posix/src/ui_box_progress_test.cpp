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

#include "components/root_view.h"
#include "components/ui_view_group.h"
#include "components/ui_box_progress.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIBoxProgressTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};
class TestUIBoxProgress : public UIBoxProgress {
public:
    TestUIBoxProgress() {}
    virtual ~TestUIBoxProgress() {}
};

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_GETVIEWTYPE_0100
 * @tc.name     test box-progress get-viewtype api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    TestUIBoxProgress* boxProgress = new TestUIBoxProgress();
    EXPECT_EQ(boxProgress->GetViewType(), UI_BOX_PROGRESS);
    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETSTEP_0200
 * @tc.name     test box-progress set-step api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetStep_0200, Function | MediumTest | Level0)
{
    TestUIBoxProgress* boxProgress = new TestUIBoxProgress();
    uint16_t step = 0;
    int16_t value = 1;
    boxProgress->SetRange(100, 0);
    boxProgress->SetValue(0);
    boxProgress->SetStep(step);
    boxProgress->SetValue(value);
    EXPECT_EQ(boxProgress->GetStep(), step);
    EXPECT_EQ(boxProgress->GetValue(), value);
    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETDIRECTION_0300
 * @tc.name     test box-progress set-direction api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetDirection_0300, Function | MediumTest | Level0)
{
    UIBoxProgress *boxProgress = new UIBoxProgress();
    UIBoxProgress::Direction direction = UIBoxProgress::Direction::DIR_BOTTOM_TO_TOP;
    boxProgress->SetDirection(direction);

    EXPECT_EQ(boxProgress->GetDirection(), direction);
    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETVALIDWIDTH_0400
 * @tc.name     test box-progress set-validwidth api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetValidWidth_0400, Function | MediumTest | Level0)
{
    UIBoxProgress *boxProgress = new UIBoxProgress();
    boxProgress->Resize(100, 100);
    int16_t validWidth = 10;
    boxProgress->SetValidWidth(validWidth);
    EXPECT_EQ(boxProgress->GetValidWidth(), validWidth);
    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETVALIDHEIGHT_0500
 * @tc.name     test box-progress set-validheight api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetValidHeight_0500, Function | MediumTest | Level0)
{
    UIBoxProgress *boxProgress = new UIBoxProgress();
    boxProgress->Resize(100, 100);
    int16_t validHeight = 10;
    boxProgress->SetValidHeight(validHeight);
    EXPECT_EQ(boxProgress->GetValidHeight(), validHeight);
    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETVALUE_0600
 * @tc.name     test box-progress set-value api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetValue_0600, Function | MediumTest | Level0)
{
    UIBoxProgress *boxProgress = new UIBoxProgress();
    int16_t value = 10;
    boxProgress->SetValue(value);
    EXPECT_EQ(boxProgress->GetValue(), value);
    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETRANGE_0700
 * @tc.name     test box-progress set-range api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetRange_0700, Function | MediumTest | Level0)
{
    UIBoxProgress *boxProgress = new UIBoxProgress();
    int16_t rangeMax = 500;
    int16_t rangeMin = 100;
    boxProgress->SetRange(rangeMax, rangeMin);
    EXPECT_EQ(boxProgress->GetRangeMax(), rangeMax);
    EXPECT_EQ(boxProgress->GetRangeMin(), rangeMin);
    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETRANGE_0800
 * @tc.name     test box-progress set-range2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetRange_0800, Function | MediumTest | Level0)
{
    UIBoxProgress *boxProgress = new UIBoxProgress();
    int16_t rangeMax = 500;
    int16_t rangeMin = 100;
    boxProgress->SetRange(rangeMax, rangeMin);

    int16_t value = rangeMin - 1;
    boxProgress->SetValue(value);
    EXPECT_EQ(boxProgress->GetValue(), rangeMin);

    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETRANGE_0900
 * @tc.name     test box-progress set-Range3 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetRange_0900, Function | MediumTest | Level0)
{
    UIBoxProgress *boxProgress = new UIBoxProgress();
    int16_t rangeMax = 500;
    int16_t rangeMin = 100;
    boxProgress->SetRange(rangeMax, rangeMin);

    int16_t value = rangeMax + 1;
    boxProgress->SetValue(value);
    EXPECT_EQ(boxProgress->GetValue(), rangeMax);

    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETWIDTH_1000
 * @tc.name     test box-progress set-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetWidth_1000, Function | MediumTest | Level0)
{
    UIBoxProgress *boxProgress = new UIBoxProgress();
    int16_t width = 100;
    boxProgress->SetWidth(width);

    EXPECT_EQ(boxProgress->GetWidth(), width);
    EXPECT_EQ(boxProgress->GetValidWidth(), width);

    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_BOXPROGRESS_SETHEIGHT_1100
 * @tc.name     test box-progress set-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIBoxProgressTest, Graphic_UIBoxProgressTest_Test_SetHeight_1100, Function | MediumTest | Level0)
{
    UIBoxProgress *boxProgress = new UIBoxProgress();
    int16_t height = 100;
    boxProgress->SetHeight(height);

    EXPECT_EQ(boxProgress->GetHeight(), height);
    EXPECT_EQ(boxProgress->GetValidHeight(), height);

    delete boxProgress;
}
} // namespace OHOS