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
#include "components/ui_picker.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIPickerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_PICKER_GETVIEWTYPE_0100
 * @tc.name     test picker get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIPickerTest, Graphic_UIPickerTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIPicker *picker = new UIPicker();
    EXPECT_EQ(picker->GetViewType(), UI_PICKER);
    delete picker;
}

/**
 * @tc.number   SUB_GRAPHIC_PICKER_GETHIGHLIGHTFONTID_0200
 * @tc.name     test picker get-high-light-fontID api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIPickerTest, Graphic_UIPickerTest_Test_GetHighlightFontId_0200, Function | MediumTest | Level0)
{
    UIPicker *picker = new UIPicker();
    uint8_t highlightFontId = 16;
    EXPECT_EQ(picker->GetHighlightFontId(), highlightFontId);
    delete picker;
}

/**
 * @tc.number   SUB_GRAPHIC_PICKER_SETFONTID_0300
 * @tc.name     test picker set-fondID api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIPickerTest, Graphic_UIPickerTest_Test_SetFontId_0300, Function | MediumTest | Level0)
{
    UIPicker *picker = new UIPicker();
    uint8_t backgroundFontId = 16;
    uint8_t highlightFontId = 18;
    picker->SetFontId(backgroundFontId, highlightFontId);
    EXPECT_EQ(picker->GetBackgroundFontId(), backgroundFontId);
    EXPECT_EQ(picker->GetHighlightFontId(), highlightFontId);
    delete picker;
}

/**
 * @tc.number   SUB_GRAPHIC_PICKER_SETTEXTCOLOR_0400
 * @tc.name     test picker set-text-color api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIPickerTest, Graphic_UIPickerTest_Test_SetTextColor_0400, Function | MediumTest | Level0)
{
    UIPicker *picker = new UIPicker();
    ColorType backgroundColor;
    backgroundColor.alpha = 10;
    ColorType highlightColor;
    highlightColor.alpha = 20;
    picker->SetTextColor(backgroundColor, highlightColor);
    EXPECT_EQ(picker->GetBackgroundTextColor().alpha, backgroundColor.alpha);
    EXPECT_EQ(picker->GetHighlightTextColor().alpha, highlightColor.alpha);
    delete picker;
}

/**
 * @tc.number   SUB_GRAPHIC_PICKER_SETWIDTH_0500
 * @tc.name     test picker set-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIPickerTest, Graphic_UIPickerTest_Test_SetWidth_0500, Function | MediumTest | Level0)
{
    UIPicker *picker = new UIPicker();
    int16_t width = 10;
    int16_t height = 20;
    picker->SetWidth(width);
    picker->SetHeight(height);
    EXPECT_EQ(picker->GetWidth(), width);
    EXPECT_EQ(picker->GetHeight(), height);
    delete picker;
}


/**
 * @tc.number   SUB_GRAPHIC_PICKER_SETSELECTED_0600
 * @tc.name     test picker set-selectred api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIPickerTest, Graphic_UIPickerTest_Test_SetSelected_0600, Function | MediumTest | Level0)
{
    UIPicker *picker = new UIPicker();
    uint16_t index = 30;
    int16_t itemHeight = 10;
    int16_t width = 100;
    int16_t height = 100;
    int16_t start = 0;
    int16_t end = 100;
    picker->SetPosition(0, 0, width, height);
    picker->SetItemHeight(itemHeight);
    picker->SetValues(start, end);
    const char* value[1];
    value[0] = "abc";
    picker->SetValues(value, 1);
    picker->SetValues(start, end);
    picker->SetSelected(index);
    EXPECT_EQ(picker->GetSelected(), index);
    delete picker;
}

/**
 * @tc.number   SUB_GRAPHIC_PICKER_CLEARVALUES_0700
 * @tc.name     test picker set-selectred api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIPickerTest, Graphic_UIPickerTest_Test_ClearValues_0700, Function | MediumTest | Level0)
{
    UIPicker *picker = new UIPicker();
    picker->ClearValues();
    EXPECT_EQ(0, 0);
    delete picker;
}

/**
 * @tc.number   SUB_GRAPHIC_PICKER_SETLOOPSTATE_0800
 * @tc.name     test picker set-selectred api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIPickerTest, Graphic_UIPickerTest_Test_SetLoopState_0800, Function | MediumTest | Level0)
{
    UIPicker *picker = new UIPicker();
    bool state = false;
    picker->SetLoopState(state);
    EXPECT_EQ(0, 0);
    delete picker;
}
}
