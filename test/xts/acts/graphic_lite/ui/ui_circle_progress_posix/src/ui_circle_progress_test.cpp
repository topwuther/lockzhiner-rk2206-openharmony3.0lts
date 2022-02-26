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
#include "components/ui_circle_progress.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UICircleProgressTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_GETVIEWTYPE_0100
 * @tc.name     test circle-progress get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UICircleProgress* circleProgress = new UICircleProgress();
    EXPECT_EQ(circleProgress->GetViewType(), UI_CIRCLE_PROGRESS);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_SETSTEP_0200
 * @tc.name     test circle-progress set-step api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetStep_0200, Function | MediumTest | Level0)
{
    UICircleProgress* circleProgress = new UICircleProgress();
    uint16_t step = 10;
    circleProgress->SetStep(step);

    EXPECT_EQ(circleProgress->GetStep(), step);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_GETRADIUS_0300
 * @tc.name     test circle-progress get-radius api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetRadius_0300, Function | MediumTest | Level0)
{
    UICircleProgress *circleProgress = new UICircleProgress();
    uint16_t radius = 10;
    circleProgress->SetRadius(radius);

    EXPECT_EQ(circleProgress->GetRadius(), radius);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_SETCENTERPOSITION_0400
 * @tc.name     test circle-progress set-center-position api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetCenterPosition_0400, Function | MediumTest | Level0)
{
    UICircleProgress *circleProgress = new UICircleProgress();
    circleProgress->Resize(100, 100);
    int16_t centerX = 50;
    int16_t centerY = 50;
    circleProgress->SetCenterPosition(centerX, centerY);

    EXPECT_EQ(circleProgress->GetCenterPosition().x, centerX);
    EXPECT_EQ(circleProgress->GetCenterPosition().y, centerY);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_SETVALUE_0500
 * @tc.name     test circle-progress set-value api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetValue_0500, Function | MediumTest | Level0)
{
    UICircleProgress *circleProgress = new UICircleProgress();
    int16_t value = 10;
    circleProgress->SetValue(value);

    EXPECT_EQ(circleProgress->GetValue(), value);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_SETRANGEMAX_0600
 * @tc.name     test circle-progress set-range-max api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetRangeMax_0600, Function | MediumTest | Level0)
{
    UICircleProgress *circleProgress = new UICircleProgress();
    int16_t rangeMax = 500;
    int16_t rangeMin = 100;
    circleProgress->SetRange(rangeMax, rangeMin);

    EXPECT_EQ(circleProgress->GetRangeMax(), rangeMax);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_SETRANGEMIN_0700
 * @tc.name     test circle-progress set-range-min api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetRangeMin_0700, Function | MediumTest | Level0)
{
    UICircleProgress *circleProgress = new UICircleProgress();
    int16_t rangeMax = 500;
    int16_t rangeMin = 100;
    circleProgress->SetRange(rangeMax, rangeMin);

    EXPECT_EQ(circleProgress->GetRangeMin(), rangeMin);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_SETSTARTANGLE_0800
 * @tc.name     test circle-progress set-start-angle api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetStartAngle_0800, Function | MediumTest | Level0)
{
    UICircleProgress *circleProgress = new UICircleProgress();
    int16_t startAngle = 50;
    circleProgress->SetStartAngle(startAngle);

    EXPECT_EQ(circleProgress->GetStartAngle(), startAngle);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_SETENDANGLE_0900
 * @tc.name     test circle-progress set-end-angle api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetEndAngle_0900, Function | MediumTest | Level0)
{
    UICircleProgress *circleProgress = new UICircleProgress();
    int16_t endAngle = 50;
    circleProgress->SetEndAngle(endAngle);

    EXPECT_EQ(circleProgress->GetEndAngle(), endAngle);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_SETPROGRESSIMAGEPOSITION_1000
 * @tc.name     test circle-progress SetProgressImagePosition api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetProgressImagePosition_1000, Function | MediumTest | Level0)
{
    UICircleProgress *circleProgress = new UICircleProgress();
    static Point imgPos = { 1, 0 };
    circleProgress->SetBackgroundImagePosition(imgPos.x, imgPos.y);
    circleProgress->SetProgressImagePosition(imgPos.x, imgPos.y);

    EXPECT_EQ(0, 0);
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_CIRCLEPROGRESS_SETLINECOLOR_1100
 * @tc.name     test circle-progress SetLineColor api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICircleProgressTest, Graphic_UICircleProgressTest_Test_SetLineColor_1100, Function | MediumTest | Level0)
{
    UICircleProgress* circleProgress = new UICircleProgress();
    ColorType color = Color::Red();
    circleProgress->SetLineColor(color);
    circleProgress->GetForegroundStyle(STYLE_LINE_COLOR);
    EXPECT_EQ(0, 0);
    delete circleProgress;
}
} // namespace OHOS