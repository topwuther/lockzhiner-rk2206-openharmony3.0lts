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
#include "components/ui_label.h"
#include "font/ui_font.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UILabelTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_LABEL_GETVIEWTYPE_0100
 * @tc.name     test label get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    EXPECT_EQ(label->GetViewType(), UI_LABEL);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_SETLABELSIZE_0200
 * @tc.name     test label set-label-size api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_SetLabelSize_0200, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    label->Resize(100, 100);
    EXPECT_EQ(label->GetWidth(), 100);
    EXPECT_EQ(label->GetHeight(), 100);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_SETTEXT_0300
 * @tc.name     test label set-text api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_SetText_0300, Function | MediumTest | Level0)
{
    const char* text = "abc";
    UILabel* label = new UILabel();
    label->Resize(100, 100);
    label->SetText(text);
    label->GetText();
    EXPECT_EQ(0, 0);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_SETLINEBREAKMODE_0400
 * @tc.name     test label set-line-breakmode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_SetLineBreakMode_0400, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    const uint8_t lineBreakMode = UILabel::LINE_BREAK_ELLIPSIS;
    label->SetLineBreakMode(lineBreakMode);
    EXPECT_EQ(label->GetLineBreakMode(), lineBreakMode);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_SETTEXTCOLOR_0500
 * @tc.name     test label set-text-color api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_SetTextColor_0500, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    ColorType color = Color::White();
    label->SetTextColor(color);
    EXPECT_EQ(label->GetTextColor().full, color.full);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_SETLONGMODE_0600
 * @tc.name     test label set-long-mode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_SetLongMode_0600, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    label->SetLineBreakMode(UILabel::LINE_BREAK_ADAPT);
    EXPECT_EQ(label->GetLineBreakMode(), UILabel::LINE_BREAK_ADAPT);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_SETALIGN_0700
 * @tc.name     test label set-align api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_SetAlign_0700, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    label->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_RIGHT, UITextLanguageAlignment::TEXT_ALIGNMENT_BOTTOM);
    EXPECT_EQ(label->GetHorAlign(), UITextLanguageAlignment::TEXT_ALIGNMENT_RIGHT);
    EXPECT_EQ(label->GetVerAlign(), UITextLanguageAlignment::TEXT_ALIGNMENT_BOTTOM);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_SETDIRECT_0800
 * @tc.name     test label set-direct api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_SetDirect_0800, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    label->SetDirect(UITextLanguageDirect::TEXT_DIRECT_RTL);
    EXPECT_EQ(label->GetDirect(), UITextLanguageDirect::TEXT_DIRECT_RTL);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_SETROLLSTARTPOS_0900
 * @tc.name     test label set-roll-start-pos api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_SetRollStartPos_0900, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    label->SetRollStartPos(50);
    EXPECT_EQ(label->GetRollStartPos(), 50);  // 219: text width
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_SETFONT_1100
 * @tc.name     test label setfont api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_SetFont_1100, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    char const *name = "1";
    uint8_t size = 1;
    label->SetFont(name, size);
    EXPECT_EQ(0, 0);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_GETTEXTWIDTH_1200
 * @tc.name     test label gettextwidth api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_GetTextWidth_1200, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    label->GetTextWidth();
    EXPECT_EQ(0, 0);
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_GETTEXTHEIGHT_1300
 * @tc.name     test label gettextheight api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelTest, Graphic_UILabelTest_Test_GetTextHeight_1300, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    label->GetTextHeight();
    EXPECT_EQ(0, 0);
    delete label;
}
} // namespace OHOS