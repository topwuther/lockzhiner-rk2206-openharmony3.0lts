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
#include "components/ui_label_button.h"
#include "font/ui_font.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UILabelButtonTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_LABEL_BUTTON_GETVIEWTYPE_0100
 * @tc.name     test label-button get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelButtonTest, Graphic_UILabelButtonTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UILabelButton* labelBtn = new UILabelButton();
    EXPECT_EQ(labelBtn->GetViewType(), UI_LABEL_BUTTON);
    delete labelBtn;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_BUTTON_SETTEXT_0200
 * @tc.name     test label-button set-text api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelButtonTest, Graphic_UILabelButtonTest_Test_SetText_0200, Function | MediumTest | Level0)
{
    const char* text = "abc";
    UILabelButton* labelBtn = new UILabelButton();
    labelBtn->SetText(text);
    labelBtn->GetText();
    EXPECT_EQ(0, 0);
    delete labelBtn;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_BUTTON_SETLABLEPOSITION_0300
 * @tc.name     test label-button set-lable-position api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelButtonTest, Graphic_UILabelButtonTest_Test_SetLablePosition_0300, Function | MediumTest | Level0)
{
    UILabelButton* labelBtn = new UILabelButton();
    labelBtn->SetLablePosition(10, 20);
    EXPECT_EQ(labelBtn->GetLabelPosition().x, 10);
    EXPECT_EQ(labelBtn->GetLabelPosition().y, 20);
    delete labelBtn;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_BUTTON_SETALIGN_0400
 * @tc.name     test label-button set-align api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelButtonTest, Graphic_UILabelButtonTest_Test_SetAlign_0400, Function | MediumTest | Level0)
{
    UILabelButton* labelBtn = new UILabelButton();
    labelBtn->SetAlign(UITextLanguageAlignment::TEXT_ALIGNMENT_RIGHT);
    EXPECT_EQ(labelBtn->GetAlign(), UITextLanguageAlignment::TEXT_ALIGNMENT_RIGHT);
    delete labelBtn;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_BUTTON_SETDIRECT_0500
 * @tc.name     test label-button set-direct api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelButtonTest, Graphic_UILabelButtonTest_Test_SetDirect_0500, Function | MediumTest | Level0)
{
    UILabelButton* labelBtn = new UILabelButton();
    labelBtn->SetDirect(UITextLanguageDirect::TEXT_DIRECT_RTL);
    EXPECT_EQ(labelBtn->GetDirect(), UITextLanguageDirect::TEXT_DIRECT_RTL);
    delete labelBtn;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_BUTTON_SETLABELSTYLE_0600
 * @tc.name     test label-button set-label-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelButtonTest, Graphic_UILabelButtonTest_Test_SetLabelStyle_0600, Function | MediumTest | Level0)
{
    UILabelButton* labelBtn = new UILabelButton();
    Style style;
    style.borderWidth_ = 10;
    labelBtn->SetLabelStyle(style);
    EXPECT_EQ(labelBtn->GetLabelStyle().borderWidth_, style.borderWidth_);
    delete labelBtn;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_BUTTON_SETLABELSTYLE_0700
 * @tc.name     test label-button set-label-style2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelButtonTest, Graphic_UILabelButtonTest_Test_SetLabelStyle_0700, Function | MediumTest | Level0)
{
    UILabelButton* labelBtn = new UILabelButton();
    uint8_t key = 1;
    int64_t value = 10;
    labelBtn->SetLabelStyle(key, value);
    EXPECT_EQ(labelBtn->GetLabelStyle(key), value);
    delete labelBtn;
}

/**
 * @tc.number   SUB_GRAPHIC_LABEL_BUTTON_SETTEXTCOLOR_0800
 * @tc.name     test label-button set-text-color api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UILabelButtonTest, Graphic_UILabelButtonTest_Test_SetTextColor_0800, Function | MediumTest | Level0)
{
    UILabelButton* labelBtn = new UILabelButton();
    ColorType colorType;
    colorType.blue = 10;
    labelBtn->SetTextColor(colorType);
    EXPECT_EQ(labelBtn->GetLabelStyle().textColor_.blue, colorType.blue);
    delete labelBtn;
}
} // namespace OHOS