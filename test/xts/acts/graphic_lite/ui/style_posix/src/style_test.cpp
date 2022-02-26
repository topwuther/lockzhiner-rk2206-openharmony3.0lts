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

#include "gfx_utils/style.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class StyleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static Style* styleVar;
    static StyleDefault* styleDefault_;
};
Style* StyleTest::styleVar = nullptr;

void StyleTest::SetUpTestCase()
{
    styleVar = new (std::nothrow) Style();
    if (styleVar == nullptr) {
        printf("Failed to apply for memory.\n");
        return;
    }
}

void StyleTest::TearDownTestCase()
{
    if (styleVar != nullptr) {
        delete styleVar;
        styleVar = nullptr;
    }
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_0100
 * @tc.name     test style set-style01 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_0100, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_BACKGROUND_COLOR, Color::Gray().full);
    EXPECT_EQ(styleVar->GetStyle(STYLE_BACKGROUND_COLOR), Color::Gray().full);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_0200
 * @tc.name     test style set-style02 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_0200, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_BACKGROUND_OPA, OPA_OPAQUE);
    EXPECT_EQ(styleVar->GetStyle(STYLE_BACKGROUND_OPA), OPA_OPAQUE);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_0300
 * @tc.name     test style set-style03 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_0300, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_BORDER_RADIUS, 50);
    EXPECT_EQ(styleVar->GetStyle(STYLE_BORDER_RADIUS), 50);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_0400
 * @tc.name     test style set-style04 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_0400, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_BORDER_COLOR, Color::Gray().full);
    EXPECT_EQ(styleVar->GetStyle(STYLE_BORDER_COLOR), Color::Gray().full);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_0500
 * @tc.name     test style set-style05 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_0500, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_BORDER_OPA, OPA_OPAQUE);
    EXPECT_EQ(styleVar->GetStyle(STYLE_BORDER_OPA), OPA_OPAQUE);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_0600
 * @tc.name     test style set-style06 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_0600, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_BORDER_WIDTH, 100);
    EXPECT_EQ(styleVar->GetStyle(STYLE_BORDER_WIDTH), 100);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_0700
 * @tc.name     test style set-style07 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_0700, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_PADDING_LEFT, 50);
    EXPECT_EQ(styleVar->GetStyle(STYLE_PADDING_LEFT), 50);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_0800
 * @tc.name     test style set-style08 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_0800, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_PADDING_RIGHT, 100);
    EXPECT_EQ(styleVar->GetStyle(STYLE_PADDING_RIGHT), 100);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_0900
 * @tc.name     test style set-style09 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_0900, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_PADDING_TOP, 100);
    EXPECT_EQ(styleVar->GetStyle(STYLE_PADDING_TOP), 100);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1000
 * @tc.name     test style set-style10 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1000, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_PADDING_BOTTOM, 100);
    EXPECT_EQ(styleVar->GetStyle(STYLE_PADDING_BOTTOM), 100);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1100
 * @tc.name     test style set-style11 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1100, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_MARGIN_LEFT, 10);
    EXPECT_EQ(styleVar->GetStyle(STYLE_MARGIN_LEFT), 10);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1200
 * @tc.name     test style set-style12 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1200, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_MARGIN_RIGHT, 20);
    EXPECT_EQ(styleVar->GetStyle(STYLE_MARGIN_RIGHT), 20);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1300
 * @tc.name     test style set-style13 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1300, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_MARGIN_TOP, 30);
    EXPECT_EQ(styleVar->GetStyle(STYLE_MARGIN_TOP), 30);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1400
 * @tc.name     test style set-style14 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1400, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_MARGIN_BOTTOM, 40);
    EXPECT_EQ(styleVar->GetStyle(STYLE_MARGIN_BOTTOM), 40);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1500
 * @tc.name     test style set-style15 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1500, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_IMAGE_OPA, OPA_OPAQUE);
    EXPECT_EQ(styleVar->GetStyle(STYLE_IMAGE_OPA), OPA_OPAQUE);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1600
 * @tc.name     test style set-style16 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1600, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_TEXT_COLOR, Color::Gray().full);
    EXPECT_EQ(styleVar->GetStyle(STYLE_TEXT_COLOR), Color::Gray().full);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1700
 * @tc.name     test style set-style17 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1700, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_TEXT_FONT, 16);
    EXPECT_EQ(styleVar->GetStyle(STYLE_TEXT_FONT), 16);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1800
 * @tc.name     test style set-style18 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1800, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_LETTER_SPACE, 4);
    EXPECT_EQ(styleVar->GetStyle(STYLE_LETTER_SPACE), 4);
}


/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_1900
 * @tc.name     test style set-style19 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_1900, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_LINE_HEIGHT, 4);
    EXPECT_EQ(styleVar->GetStyle(STYLE_LINE_HEIGHT), 4);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_2000
 * @tc.name     test style set-style20 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_2000, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_TEXT_OPA, OPA_OPAQUE);
    EXPECT_EQ(styleVar->GetStyle(STYLE_TEXT_OPA), OPA_OPAQUE);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_2100
 * @tc.name     test style set-style21 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_2100, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_LINE_COLOR, Color::Gray().full);
    EXPECT_EQ(styleVar->GetStyle(STYLE_LINE_COLOR), Color::Gray().full);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_2200
 * @tc.name     test style set-style22 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_2200, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_LINE_WIDTH, 4);
    EXPECT_EQ(styleVar->GetStyle(STYLE_LINE_WIDTH), 4);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_2300
 * @tc.name     test style set-style23 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_2300, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_LINE_OPA, OPA_OPAQUE);
    EXPECT_EQ(styleVar->GetStyle(STYLE_LINE_OPA), OPA_OPAQUE);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_SETSTYLE_2400
 * @tc.name     test style set-style24 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_SetStyle_2400, Function | MediumTest | Level0)
{
    styleVar->SetStyle(STYLE_LINE_CAP, CapType::CAP_ROUND);
    EXPECT_EQ(styleVar->GetStyle(STYLE_LINE_CAP), CapType::CAP_ROUND);
}


/**
 * @tc.number   SUB_GRAPHIC_STYLE_INIT_2500
 * @tc.name     test style init api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_Init_2500, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->Init();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETBUTTONPRESSEDSTYLE_2600
 * @tc.name     test style get-button-pressed-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetButtonPressedStyle_2600, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetButtonPressedStyle();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETBUTTONRELEASEDSTYLE_2700
 * @tc.name     test style get-button-released-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetButtonReleasedStyle_2700, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetButtonReleasedStyle();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETBUTTONINACTIVESTYLE_2800
 * @tc.name     test style get-button-inactived-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetButtonInactiveStyle_2800, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetButtonInactiveStyle();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETLABELSTYLE_2900
 * @tc.name     test style get-label-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetLabelStyle_2900, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetButtonInactiveStyle();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETPICKERGROUNDSTYLE_3000
 * @tc.name     test style get-picker-background-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetPickerBackgroundStyle_3000, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetPickerBackgroundStyle();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETPICKERHIGHLIGHTSTYLE_3100
 * @tc.name     test style get-picker-highlight-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetPickerHighlightStyle_3100, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetPickerHighlightStyle();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETPROGRESSBACKGROUNDSTYLE_3200
 * @tc.name     test style get-progress-background-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetProgressBackgroundStyle_3200, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetProgressBackgroundStyle();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETPROGRESSFOREGROUNDSTYLE_3300
 * @tc.name     test style get-progress-foreground-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetProgressForegroundStyle_3300, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetProgressForegroundStyle();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETSLIDERKNOBSTYLE_3400
 * @tc.name     test style get-slider-knob-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetSliderKnobStyle_3400, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetSliderKnobStyle();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_STYLE_GETBACKGROUNDTRANSPARENTSTYLE_3500
 * @tc.name     test style get-background-transparent-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(StyleTest, Graphic_Style_Test_GetBackgroundTransparentStyle_3500, Function | MediumTest | Level0)
{
    // static Style* styleVar;
    styleDefault_->GetBackgroundTransparentStyle();
    EXPECT_EQ(0, 0);
}
} // namespace OHOS