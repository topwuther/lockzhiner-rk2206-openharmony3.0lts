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

#include "components/ui_abstract_clock.h"
#include "components/ui_analog_clock.h"
#include "components/ui_image_view.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIAnalogClockTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_ANALOGCLOCK_GETVIEWTYPE_0100
 * @tc.name     test analog-clock get-viewtype api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAnalogClockTest, Graphic_UIAnalogClockTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIAnalogClock* clock = new UIAnalogClock();
    EXPECT_EQ(clock->GetViewType(), UI_ANALOG_CLOCK);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_ANALOGCLOCK_GETVIEWTYPE_0200
 * @tc.name     test analog-clock get-viewtype2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAnalogClockTest, Graphic_UIAnalogClockTest_Test_GetViewType_0200, Function | MediumTest | Level0)
{
    UIAnalogClock* clock = new UIAnalogClock();
    clock->SetPosition(10, 20);
    EXPECT_EQ(clock->GetY(), 20);

    clock->SetPosition(100, 200, 300, 400);
    EXPECT_EQ(clock->GetY(), 200);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_ANALOGCLOCK_SETTIME24HOUR_0300
 * @tc.name     test analog-clock set-time-24hour api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAnalogClockTest, Graphic_UIAnalogClockTest_Test_SetTime24Hour_0300, Function | MediumTest | Level0)
{
    UIAnalogClock* clock = new UIAnalogClock();
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
 * @tc.number   SUB_GRAPHIC_ANALOGCLOCK_SETTIME12HOUR_0400
 * @tc.name     test analog-clock set-time-12hour-am api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAnalogClockTest, Graphic_UIAnalogClockTest_Test_SetTime12Hour_0400, Function | MediumTest | Level0)
{
    UIAnalogClock* clock = new UIAnalogClock();
    uint8_t currentHour = 10;
    uint8_t currentMinute = 40;
    uint8_t currentSecond = 30;
    clock->SetTime12Hour(currentHour, currentMinute, currentSecond, true);

    EXPECT_EQ(clock->GetCurrentHour(), currentHour);
    EXPECT_EQ(clock->GetCurrentMinute(), currentMinute);
    EXPECT_EQ(clock->GetCurrentSecond(), currentSecond);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_ANALOGCLOCK_SETTIME12HOUR_0500
 * @tc.name     test analog-clock set-time-12hour-pm api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAnalogClockTest, Graphic_UIAnalogClockTest_Test_SetTime12Hour_0500, Function | MediumTest | Level0)
{
    UIAnalogClock* clock = new UIAnalogClock();
    uint8_t currentHour = 10;
    uint8_t currentMinute = 40;
    uint8_t currentSecond = 30;
    clock->SetTime12Hour(currentHour, currentMinute, currentSecond, false);

    EXPECT_EQ(clock->GetCurrentHour(), currentHour + 12);
    EXPECT_EQ(clock->GetCurrentMinute(), currentMinute);
    EXPECT_EQ(clock->GetCurrentSecond(), currentSecond);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_ANALOGCLOCK_SETHANDIMAGE_0600
 * @tc.name     test analog-clock set-hand-image api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAnalogClockTest, Graphic_UIAnalogClockTest_Test_SetHandImage_0600, Function | MediumTest | Level0)
{
    UIAnalogClock* clock = new UIAnalogClock();
    Point position = { 214, 3 };
    Point center = { 14, 223 };
    clock->SetHandLine(UIAnalogClock::HandType::SECOND_HAND, position, center, Color::White(), 1, 223, 255);
    EXPECT_EQ(clock->GetHandPosition(UIAnalogClock::HandType::SECOND_HAND).x, position.x);
    EXPECT_EQ(clock->GetHandPosition(UIAnalogClock::HandType::SECOND_HAND).y, position.y);
    EXPECT_EQ(clock->GetHandRotateCenter(UIAnalogClock::HandType::SECOND_HAND).x, center.x);
    EXPECT_EQ(clock->GetHandRotateCenter(UIAnalogClock::HandType::SECOND_HAND).y, center.y);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_ANALOGCLOCK_SETINITTIME24HOUR_0700
 * @tc.name     test analog-clock set-init-time-24hour api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAnalogClockTest, Graphic_UIAnalogClockTest_Test_SetInitTime24Hour_0700, Function | MediumTest | Level0)
{
    uint8_t initHour = 6;
    uint8_t initMinute = 0;
    uint8_t initSecond = 0;
    UIAnalogClock* clock = new UIAnalogClock();
    clock->SetInitTime24Hour(initHour, initMinute, initSecond);
    EXPECT_EQ(clock->GetCurrentHour(), initHour);
    EXPECT_EQ(clock->GetCurrentMinute(), initMinute);
    EXPECT_EQ(clock->GetCurrentSecond(), initSecond);
    EXPECT_EQ(clock->GetHandInitAngle(UIAnalogClock::HandType::HOUR_HAND), 180);
    EXPECT_EQ(clock->GetHandCurrentAngle(UIAnalogClock::HandType::HOUR_HAND), 180);

    uint8_t currentHour = 9;
    uint8_t currentMinute = 0;
    uint8_t currentSecond = 0;
    clock->SetTime24Hour(currentHour, currentMinute, currentSecond);
    EXPECT_EQ(clock->GetHandInitAngle(UIAnalogClock::HandType::HOUR_HAND), 180);
    EXPECT_EQ(clock->GetHandCurrentAngle(UIAnalogClock::HandType::HOUR_HAND), 270);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_ANALOGCLOCK_SETINITTIME12HOUR_0800
 * @tc.name     test analog-clock set-init-time-12hour-am api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAnalogClockTest, Graphic_UIAnalogClockTest_Test_SetInitTime12Hour_0800, Function | MediumTest | Level0)
{
    uint8_t initHour = 6;
    uint8_t initMinute = 0;
    uint8_t initSecond = 0;
    UIAnalogClock* clock = new UIAnalogClock();
    clock->SetInitTime12Hour(initHour, initMinute, initSecond, true);
    EXPECT_EQ(clock->GetCurrentHour(), initHour);
    EXPECT_EQ(clock->GetCurrentMinute(), initMinute);
    EXPECT_EQ(clock->GetCurrentSecond(), initSecond);
    EXPECT_EQ(clock->GetHandInitAngle(UIAnalogClock::HandType::HOUR_HAND), 180);
    EXPECT_EQ(clock->GetHandCurrentAngle(UIAnalogClock::HandType::HOUR_HAND), 180);
    delete clock;
}

/**
 * @tc.number   SUB_GRAPHIC_ANALOGCLOCK_SETINITTIME12HOUR_0900
 * @tc.name     test analog-clock set-init-time-12hour-pm api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAnalogClockTest, Graphic_UIAnalogClockTest_Test_SetInitTime12Hour_0900, Function | MediumTest | Level0)
{
    uint8_t initHour = 6;
    uint8_t initMinute = 0;
    uint8_t initSecond = 0;
    UIAnalogClock* clock = new UIAnalogClock();
    clock->SetInitTime12Hour(initHour, initMinute, initSecond, false);
    EXPECT_EQ(clock->GetCurrentHour(), initHour + 12);
    EXPECT_EQ(clock->GetCurrentMinute(), initMinute);
    EXPECT_EQ(clock->GetCurrentSecond(), initSecond);
    EXPECT_EQ(clock->GetHandInitAngle(UIAnalogClock::HandType::HOUR_HAND), 180);
    EXPECT_EQ(clock->GetHandCurrentAngle(UIAnalogClock::HandType::HOUR_HAND), 180);
    delete clock;
}
}
