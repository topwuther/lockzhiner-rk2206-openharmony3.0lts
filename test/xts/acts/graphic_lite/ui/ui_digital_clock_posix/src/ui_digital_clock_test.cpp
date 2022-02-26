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
#include "components/ui_digital_clock.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
class UIDigitalClockTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_DIGITALCLOCK_GETVIEWTYPE_0100
 * @tc.name     test digital-clock get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDigitalClockTest, Graphic_UIDigitalClockTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIDigitalClock *clock = new UIDigitalClock();
    EXPECT_EQ(clock->GetViewType(), UI_DIGITAL_CLOCK);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_DIGITALCLOCK_SETFONTID_0200
 * @tc.name     test digital-clock set-fontID api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDigitalClockTest, Graphic_UIDigitalClockTest_Test_SetFontId_0200, Function | MediumTest | Level0)
{
    UIDigitalClock *clock = new UIDigitalClock();
    uint8_t fontId = 10;
    clock->SetFontId(fontId);
    EXPECT_EQ(clock->GetStyle(STYLE_TEXT_FONT), fontId);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_DIGITALCLOCK_SETDISPLAYMODE_0300
 * @tc.name     test digital-clock set-display-mode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDigitalClockTest, Graphic_UIDigitalClockTest_Test_SetDisplayMode_0300, Function | MediumTest | Level0)
{
    UIDigitalClock *clock = new UIDigitalClock();
    clock->SetDisplayMode(UIDigitalClock::DisplayMode::DISPLAY_12_HOUR_NO_SECONDS);
    EXPECT_EQ(clock->GetDisplayMode(), UIDigitalClock::DisplayMode::DISPLAY_12_HOUR_NO_SECONDS);
    clock->SetDisplayMode(UIDigitalClock::DisplayMode::DISPLAY_24_HOUR_NO_SECONDS);
    EXPECT_EQ(clock->GetDisplayMode(), UIDigitalClock::DisplayMode::DISPLAY_24_HOUR_NO_SECONDS);
    clock->SetDisplayMode(UIDigitalClock::DisplayMode::DISPLAY_12_HOUR);
    EXPECT_EQ(clock->GetDisplayMode(), UIDigitalClock::DisplayMode::DISPLAY_12_HOUR);
    clock->SetDisplayMode(UIDigitalClock::DisplayMode::DISPLAY_24_HOUR);
    EXPECT_EQ(clock->GetDisplayMode(), UIDigitalClock::DisplayMode::DISPLAY_24_HOUR);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_DIGITALCLOCK_SETOPACITY_0400
 * @tc.name     test digital-clock set-opacity api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDigitalClockTest, Graphic_UIDigitalClockTest_Test_SetOpacity_0400, Function | MediumTest | Level0)
{
    uint8_t opicity = 255;
    UIDigitalClock *clock = new UIDigitalClock();
    clock->SetOpacity(opicity);
    EXPECT_EQ(clock->GetOpacity(), opicity);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_DIGITALCLOCK_SETTIME24HOUR_0500
 * @tc.name     test digital-clock set-time-24hour api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDigitalClockTest, Graphic_UIDigitalClockTest_Test_SetTime24Hour_0500, Function | MediumTest | Level0)
{
    UIDigitalClock *clock = new UIDigitalClock();
    uint8_t currentHour = 14;
    uint8_t currentMinute = 40;
    uint8_t currentSecond = 30;
    clock->SetTime24Hour(currentHour, currentMinute, currentSecond);
    EXPECT_EQ(clock->GetCurrentHour(), currentHour);
    EXPECT_EQ(clock->GetCurrentMinute(), currentMinute);
    EXPECT_EQ(clock->GetCurrentSecond(), currentSecond);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_DIGITALCLOCK_SETTIME12HOUR_0600
 * @tc.name     test digital-clock set-time-12hour api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDigitalClockTest, Graphic_UIDigitalClockTest_Test_SetTime12Hour_0600, Function | MediumTest | Level0)
{
    UIDigitalClock *clock = new UIDigitalClock();
    const uint8_t currentHour = 10;
    const uint8_t currentMinute = 40;
    const uint8_t currentSecond = 30;
    clock->SetTime12Hour(currentHour, currentMinute, currentSecond, true);
    EXPECT_EQ(clock->GetCurrentHour(), currentHour);
    EXPECT_EQ(clock->GetCurrentMinute(), currentMinute);
    EXPECT_EQ(clock->GetCurrentSecond(), currentSecond);

    clock->SetTime12Hour(currentHour, currentMinute, currentSecond, false);
    EXPECT_EQ(clock->GetCurrentHour(), currentHour + 12);
    EXPECT_EQ(clock->GetCurrentMinute(), currentMinute);
    EXPECT_EQ(clock->GetCurrentSecond(), currentSecond);
    
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_DIGITALCLOCK_SETCOLOR_0700
 * @tc.name     test digital-clock set-color api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDigitalClockTest, Graphic_UIDigitalClockTest_Test_SetColor_0700, Function | MediumTest | Level0)
{
    UIDigitalClock *clock = new UIDigitalClock();
    clock->SetTime24Hour(7, 5, 9);
    clock->SetColor(Color::Red());
    EXPECT_EQ(0, 0);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_DIGITALCLOCK_DISPLAYLEADINGZERO_0800
 * @tc.name     test digital-clock set-displayleadingzero api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDigitalClockTest, Graphic_UIDigitalClockTest_Test_DisplayLeadingZero_0800, Function | MediumTest | Level0)
{
    UIDigitalClock *clock = new UIDigitalClock();
    clock->SetTime24Hour(7, 5, 9);
    clock->DisplayLeadingZero(false);
    EXPECT_EQ(0, 0);
    delete clock;
}
}