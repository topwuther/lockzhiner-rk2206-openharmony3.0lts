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
#include "components/ui_button.h"
#include "components/ui_checkbox.h"
#include "components/ui_radio_button.h"
#include "components/ui_toggle_button.h"
#include "components/ui_view_group.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
enum ButtonState : uint8_t {
    RELEASED = 0,
    PRESSED,
    INACTIVE,
    BTN_STATE_NUM,
};

class UIButtonTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_BUTTON_GETVIEWTYPE_0100
 * @tc.name     test button get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIButtonTest, Graphic_UIButtonTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIButton* button = new UIButton();
    EXPECT_EQ(button->GetViewType(), UI_BUTTON);
    delete button;
}

/**
 * @tc.number   SUB_GRAPHIC_BUTTON_SETIMAGEPOSITION_0200
 * @tc.name     test button set-image-position api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIButtonTest, Graphic_UIButtonTest_Test_SetImagePosition_0200, Function | MediumTest | Level0)
{
    UIButton* button = new UIButton();
    int16_t x = 10;
    int16_t y = 12;
    button->SetImagePosition(x, y);
    EXPECT_EQ(button->GetImageX(), x);
    EXPECT_EQ(button->GetImageY(), y);
    delete button;
}

/**
 * @tc.number   SUB_GRAPHIC_BUTTON_SETSTYLE_0300
 * @tc.name     test button set-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIButtonTest, Graphic_UIButtonTest_Test_SetStyle_0300, Function | MediumTest | Level0)
{
    UIButton* button = new UIButton();
    button->SetStyle(STYLE_BACKGROUND_OPA, 10);
    EXPECT_EQ(button->GetStyle(STYLE_BACKGROUND_OPA), 10);
    delete button;
}

/**
 * @tc.number   SUB_GRAPHIC_BUTTON_ISTOUCHABLE_0400
 * @tc.name     test button is-touchable api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIButtonTest, Graphic_UIButtonTest_Test_IsTouchable_0400, Function | MediumTest | Level0)
{
    UIButton* button = new UIButton();

    button->Enable();
    EXPECT_EQ(button->IsTouchable(), true);

    button->Disable();
    EXPECT_EQ(button->IsTouchable(), false);
    delete button;
}

/**
 * @tc.number   SUB_GRAPHIC_BUTTON_SETIMAGESRC_0500
 * @tc.name     test button set-imagesrc api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIButtonTest, Graphic_UIButtonTest_SetImageSrc_0500, Function | MediumTest | Level0)
{
    UIButton* button = new UIButton();
    button->SetImageSrc("", "");
    EXPECT_EQ(0, 0);
    delete button;
}

/**
 * @tc.number   SUB_GRAPHIC_BUTTON_SETSTYLEFORSTATE_0600
 * @tc.name     test button set-imagesrc api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIButtonTest, Graphic_UIButtonTest_SetStyleForState_0600, Function | MediumTest | Level0)
{
    UIButton* button = new UIButton();
    button->SetStyleForState(STYLE_BACKGROUND_OPA, 255, UIButton::PRESSED);

    EXPECT_EQ(button->GetStyleForState(STYLE_BACKGROUND_OPA, UIButton::PRESSED), 255);
    delete button;
}
} // namespace OHOS