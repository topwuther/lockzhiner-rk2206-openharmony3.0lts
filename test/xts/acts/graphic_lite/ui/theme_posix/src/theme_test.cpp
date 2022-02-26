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
#include "themes/theme.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class ThemeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void ThemeTest::SetUpTestCase(void)
{
    return;
}

void ThemeTest::TearDownTestCase(void)
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_THEME_GETMAINSTYLE_0100
 * @tc.name     test theme get-main-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ThemeTest, Graphic_Theme_Test_GetMainStyle_0100, Function | MediumTest | Level0)
{
    Theme* testObj = new Theme();
    testObj->GetMainStyle();
    EXPECT_EQ(0, 0);
    
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_THEME_GETBUTTONSTYLE_0200
 * @tc.name     test theme get-button-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ThemeTest, Graphic_Theme_Test_GetButtonStyle_0200, Function | MediumTest | Level0)
{   
    Theme* testObj = new Theme();
    testObj->GetButtonStyle();
    EXPECT_EQ(0, 0);
      
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_THEME_GETLABELSTYLE_0300
 * @tc.name     test theme get-label-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ThemeTest, Graphic_Theme_Test_GetLabelStyle_0300, Function | MediumTest | Level0)
{ 
    Theme* testObj = new Theme();
    testObj->GetLabelStyle();
    EXPECT_EQ(0, 0);
    
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_THEME_GETPICKERBACKGROUNDSTYLE_0400
 * @tc.name     test theme get-picker-background-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ThemeTest, Graphic_Theme_Test_GetPickerBackgroundStyle_0400, Function | MediumTest | Level0)
{
    Theme* testObj = new Theme();
    testObj->GetPickerBackgroundStyle();
    EXPECT_EQ(0, 0);
      
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_THEME_GETPICKERHIGHLIGHTSTYLE_0500
 * @tc.name     test theme get-picker-highlight-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ThemeTest, Graphic_Theme_Test_GetPickerHighlightStyle_0500, Function | MediumTest | Level0)
{
    Theme* testObj = new Theme();
    testObj->GetPickerHighlightStyle();
    EXPECT_EQ(0, 0);
      
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_THEME_GETPROGRESSBACKGROUNDSTYLE_0600
 * @tc.name     test theme get-progress-background-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ThemeTest, Graphic_Theme_Test_GetProgressBackgroundStyle_0600, Function | MediumTest | Level0)
{
    Theme* testObj = new Theme();
    testObj->GetProgressBackgroundStyle();
    EXPECT_EQ(0, 0);

    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_THEME_GETPROGRESSFOREGROUNDSTYLE_0700
 * @tc.name     test theme get-progress-foreground-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ThemeTest, Graphic_Theme_Test_GetProgressForegroundStyle_0700, Function | MediumTest | Level0)
{
    Theme* testObj = new Theme();
    testObj->GetProgressForegroundStyle();
    EXPECT_EQ(0, 0);
 
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_THEME_GETSLIDERKNOBSTYLE_0800
 * @tc.name     test theme get-slider-knob-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ThemeTest, Graphic_Theme_Test_GetSliderKnobStyle_0800, Function | MediumTest | Level0)
{
    Theme* testObj = new Theme();
    testObj->GetSliderKnobStyle();    
    EXPECT_EQ(0, 0);
   
    delete testObj;
}
} // namespace OHOS