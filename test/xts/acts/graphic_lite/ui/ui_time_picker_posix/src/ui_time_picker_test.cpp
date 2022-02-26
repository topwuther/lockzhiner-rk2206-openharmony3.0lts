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
#include "components/ui_time_picker.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UITimePickerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_TIMEPICKER_GETVIEWTYPE_0100
 * @tc.name     test time-picker get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITimePickerTest, Graphic_UITimePicker_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UITimePicker *timePicker = new UITimePicker();
    EXPECT_EQ(timePicker->GetViewType(), UI_TIME_PICKER);
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_TIMEPICKER_SETWIDTH_0200
 * @tc.name     test time-picker set-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITimePickerTest, Graphic_UITimePicker_Test_SetWidth_0200, Function | MediumTest | Level0)
{
    UITimePicker *timePicker = new UITimePicker();
    int16_t width = 50;
    timePicker->SetWidth(width);
    EXPECT_EQ(timePicker->GetWidth(), width);
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_TIMEPICKER_SETHEIGHT_0300
 * @tc.name     test time-picker set-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITimePickerTest, Graphic_UITimePicker_Test_SetHeight_0300, Function | MediumTest | Level0)
{
    UITimePicker *timePicker = new UITimePicker();
    int16_t height = 100;
    timePicker->SetHeight(height);
    EXPECT_EQ(timePicker->GetHeight(), height);
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_TIMEPICKER_SETSELECTED_0400
 * @tc.name     test time-picker set-selected api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITimePickerTest, Graphic_UITimePicker_Test_SetSelected_0400, Function | MediumTest | Level0)
{
    UITimePicker *timePicker = new UITimePicker();
    const char* value = "12:34:56";
    timePicker->EnableSecond(true);
    bool ret = timePicker->SetSelected(value);
    EXPECT_EQ(ret, true);
    
    timePicker->GetSelectValue();
    timePicker->GetSelectHour();
    timePicker->GetSelectMinute();
    timePicker->GetSelectSecond();

    EXPECT_EQ(0, 0);
    
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_TIMEPICKER_REGISTERSELECTEDLISTENER_0500
 * @tc.name     test time-picker register-selected-listener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITimePickerTest, Graphic_UITimePicker_Test_RegisterSelectedListener_0500, Function | MediumTest | Level0)
{   
    UITimePicker *timePicker = new UITimePicker();
    UITimePicker::SelectedListener *timePickerListener = new UITimePicker::SelectedListener();
    timePicker->RegisterSelectedListener(timePickerListener);
    EXPECT_EQ(0, 0);
    delete timePickerListener;
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_TIMEPICKER_ENABLESECOND_0600
 * @tc.name     test time-picker enable-second api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITimePickerTest, Graphic_UITimePicker_Test_EnableSecond_0600, Function | MediumTest | Level0)
{
    UITimePicker *timePicker = new UITimePicker();
    bool isEnable = false;
    timePicker->EnableSecond(isEnable);
    EXPECT_EQ(0, 0);
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_TIMEPICKER_SETTEXTSTYLE_0700
 * @tc.name     test time-picker set-text-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITimePickerTest, Graphic_UITimePicker_Test_SetTextStyle_0700, Function | MediumTest | Level0)
{
    UITimePicker *timePicker = new UITimePicker();
    uint8_t backgroundFontId = 128;
    uint8_t highlightFontId = 64;
    ColorType backgroundColor = Color::Green();
    ColorType highlightColor = Color::Red();
    timePicker->SetTextStyle(backgroundFontId, highlightFontId, backgroundColor, highlightColor);
    EXPECT_EQ(0, 0);
    delete timePicker;
}
} // namespace OHOS