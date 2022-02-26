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

#include <climits>
#include <gtest/gtest.h>
#include "components/ui_arc_label.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
class UIArcLabelTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_ARCLABEL_GETVIEWTYPE_0100
 * @tc.name     test arc-label get-viewtype api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIArcLabelTest, Graphic_UIArcLabelTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIArcLabel *arcLabel = new UIArcLabel();
    EXPECT_EQ(arcLabel->GetViewType(), UI_ARC_LABEL);
    delete arcLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_ARCLABEL_GETWIDTH_0200
 * @tc.name     test arc-label get-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIArcLabelTest, Graphic_UIArcLabelTest_Test_GetWidth_0200, Function | MediumTest | Level0)
{
    UIArcLabel *arcLabel = new UIArcLabel();
    int16_t width = 100;
    arcLabel->SetWidth(width);
    EXPECT_EQ(arcLabel->GetWidth(), width);
    delete arcLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_ARCLABEL_GETHEIGHT_0300
 * @tc.name     test arc-label get-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIArcLabelTest, Graphic_UIArcLabelTest_Test_GetHeight_0300, Function | MediumTest | Level0)
{
    UIArcLabel *arcLabel = new UIArcLabel();
    int16_t height = 100;
    arcLabel->SetHeight(height);
    EXPECT_EQ(arcLabel->GetHeight(), height);
    delete arcLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_ARCLABEL_SETTEXT_0400
 * @tc.name     test arc-label set-text api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIArcLabelTest, Graphic_UIArcLabelTest_Test_SetText_0400, Function | MediumTest | Level0)
{
    UIArcLabel *arcLabel = new UIArcLabel();
    const char* text = "abc";
    arcLabel->SetText(text);
    EXPECT_EQ(strcmp(arcLabel->GetText(), text), 0);
    delete arcLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_ARCLABEL_SETALIGN_0500
 * @tc.name     test arc-label set-align api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIArcLabelTest, Graphic_UIArcLabelTest_Test_SetAlign_0500, Function | MediumTest | Level0)
{
    UIArcLabel *arcLabel = new UIArcLabel();
    UITextLanguageAlignment horizontalAlign = TEXT_ALIGNMENT_LEFT;
    arcLabel->SetAlign(horizontalAlign);
    EXPECT_EQ(arcLabel->GetHorAlign(), horizontalAlign);
    delete arcLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_ARCLABEL_SETARCTEXTCENTER_0600
 * @tc.name     test arc-label set-arc-text-canter api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIArcLabelTest, Graphic_UIArcLabelTest_Test_SetArcTextCenter_0600, Function | MediumTest | Level0)
{
    UIArcLabel *arcLabel = new UIArcLabel();
    int16_t x = 10;
    int16_t y = 15;
    arcLabel->SetArcTextCenter(x, y);
    EXPECT_EQ(arcLabel->GetArcTextCenter().x, x);
    EXPECT_EQ(arcLabel->GetArcTextCenter().y, y);
    delete arcLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_ARCLABEL_SETARCTEXTRADIO_0700
 * @tc.name     test arc-label set-arc-text-radio api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIArcLabelTest, Graphic_UIArcLabelTest_Test_SetArcTextRadius_0700, Function | MediumTest | Level0)
{
    UIArcLabel *arcLabel = new UIArcLabel();
    int16_t radius = 10;
    arcLabel->SetArcTextRadius(radius);
    EXPECT_EQ(arcLabel->GetArcTextRadius(), radius);
    delete arcLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_ARCLABEL_SETARCTEXTANGLE_0800
 * @tc.name     test arc-label set-arc-text-angle api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIArcLabelTest, Graphic_UIArcLabelTest_Test_SetArcTextAngle_0800, Function | MediumTest | Level0)
{
    UIArcLabel *arcLabel = new UIArcLabel();
    int16_t startAngle = 10;
    int16_t endAngle = 20;
    arcLabel->SetArcTextAngle(startAngle, endAngle);
    EXPECT_EQ(arcLabel->GetArcTextStartAngle(), startAngle);
    EXPECT_EQ(arcLabel->GetArcTextEndAngle(), endAngle);
    delete arcLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_ARCLABEL_SETARCTEXTORIENTATION_0900
 * @tc.name     test arc-label set-arc-text-orientation api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIArcLabelTest, Graphic_UIArcLabelTest_Test_SetArcTextOrientation_0900, Function | MediumTest | Level0)
{
    UIArcLabel *arcLabel = new UIArcLabel();
    arcLabel->SetArcTextOrientation(UIArcLabel::TextOrientation::INSIDE);
    EXPECT_EQ(arcLabel->GetArcTextOrientation(),
        UIArcLabel::TextOrientation::INSIDE);
    arcLabel->SetArcTextOrientation(UIArcLabel::TextOrientation::OUTSIDE);
    EXPECT_EQ(arcLabel->GetArcTextOrientation(),
        UIArcLabel::TextOrientation::OUTSIDE);
    delete arcLabel;
}
}