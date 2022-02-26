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

#include "gfx_utils/color.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class ColorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static Color* color;
};
Color* ColorTest::color = nullptr;

void ColorTest::SetUpTestCase(void)
{
    return;
}

void ColorTest::TearDownTestCase(void)
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_GETCOLORFROMRGB_0100
 * @tc.name     test color get-color-from-RGB api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_GetColorFromRGB_0100, Function | MediumTest | Level0)
{
    EXPECT_EQ(Color::GetColorFromRGB(0, 0, 0).red, 0);
    EXPECT_EQ(Color::GetColorFromRGB(0, 0, 0).green, 0);
    EXPECT_EQ(Color::GetColorFromRGB(0, 0, 0).blue, 0);
    EXPECT_EQ(Color::GetColorFromRGB(0, 0, 0).alpha, 0xFF);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_GETCOLORFROMRGBA_0200
 * @tc.name     test color get-color-from-RGBA api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_GetColorFromRGBA_0200, Function | MediumTest | Level0)
{
    EXPECT_EQ(Color::GetColorFromRGBA(0, 0, 0, 0).red, 0);
    EXPECT_EQ(Color::GetColorFromRGBA(0, 0, 0, 0).green, 0);
    EXPECT_EQ(Color::GetColorFromRGBA(0, 0, 0, 0).blue, 0);
    EXPECT_EQ(Color::GetColorFromRGBA(0, 0, 0, 0).alpha, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_COLORTO32_0300
 * @tc.name     test color color-to-32 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_ColorTo32_0300, Function | MediumTest | Level0)
{
    ColorType color;
    color.red = 0;
    color.green = 0;
    color.blue = 0;
    color.alpha = 0;
    uint32_t color32 = 0;

    EXPECT_EQ(Color::ColorTo32(color), color32);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_COLORTO32_0400
 * @tc.name     test color color-to-32-2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_ColorTo32_0400, Function | MediumTest | Level0)
{
    Color16 color;
    color.red = 0;
    color.green = 0;
    color.blue = 0;

    EXPECT_EQ(Color::ColorTo32(color, 0), 0);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_COLORTO16_0500
 * @tc.name     test color color-to-16 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_ColorTo16_0500, Function | MediumTest | Level0)
{
    Color32 color32;
    color32.red = 0;
    color32.green = 0;
    color32.blue = 0;

    EXPECT_EQ(Color::ColorTo16(color32), 0);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_WHITE_0600
 * @tc.name     test color white api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_White_0600, Function | MediumTest | Level0)
{
    ColorType white = Color::White();

    EXPECT_EQ(white.red, 0xFF);
    EXPECT_EQ(white.green, 0xFF);
    EXPECT_EQ(white.blue, 0xFF);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_SILVER_0700
 * @tc.name     test color silver api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Silver_0700, Function | MediumTest | Level0)
{
    ColorType silver = Color::Silver();

    EXPECT_EQ(silver.red, 0xC0);
    EXPECT_EQ(silver.green, 0xC0);
    EXPECT_EQ(silver.blue, 0xC0);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_GRAY_0800
 * @tc.name     test color gray api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Gray_0800, Function | MediumTest | Level0)
{
    ColorType gray = Color::Gray();

    EXPECT_EQ(gray.red, 0x80);
    EXPECT_EQ(gray.green, 0x80);
    EXPECT_EQ(gray.blue, 0x80);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_BLACK_0900
 * @tc.name     test color black api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Black_0900, Function | MediumTest | Level0)
{
    ColorType black = Color::Black();

    EXPECT_EQ(black.red, 0x00);
    EXPECT_EQ(black.green, 0x00);
    EXPECT_EQ(black.blue, 0x00);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_RED_1000
 * @tc.name     test color red api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Red_1000, Function | MediumTest | Level0)
{
    ColorType red = Color::Red();

    EXPECT_EQ(red.red, 0xFF);
    EXPECT_EQ(red.green, 0x00);
    EXPECT_EQ(red.blue, 0x00);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_MAROON_1100
 * @tc.name     test color maroon api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Maroon_1100, Function | MediumTest | Level0)
{
    ColorType maroon = Color::Maroon();

    EXPECT_EQ(maroon.red, 0x80);
    EXPECT_EQ(maroon.green, 0x00);
    EXPECT_EQ(maroon.blue, 0x00);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_YELLOW_1200
 * @tc.name     test color yellow api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Yellow_1200, Function | MediumTest | Level0)
{
    ColorType yellow = Color::Yellow();

    EXPECT_EQ(yellow.red, 0xFF);
    EXPECT_EQ(yellow.green, 0xFF);
    EXPECT_EQ(yellow.blue, 0x00);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_OLIVE_1300
 * @tc.name     test color olive api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Olive_1300, Function | MediumTest | Level0)
{
    ColorType olive = Color::Olive();

    EXPECT_EQ(olive.red, 0x80);
    EXPECT_EQ(olive.green, 0x80);
    EXPECT_EQ(olive.blue, 0x00);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_LIME_1400
 * @tc.name     test color lime api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Lime_1400, Function | MediumTest | Level0)
{
    ColorType lime = Color::Lime();

    EXPECT_EQ(lime.red, 0x00);
    EXPECT_EQ(lime.green, 0xFF);
    EXPECT_EQ(lime.blue, 0x00);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_GREEN_1500
 * @tc.name     test color GREEN api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Green_1500, Function | MediumTest | Level0)
{
    ColorType green = Color::Green();

    EXPECT_EQ(green.red, 0x00);
    EXPECT_EQ(green.green, 0xFF);
    EXPECT_EQ(green.blue, 0x00);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_CYAN_1600
 * @tc.name     test color CYAN api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Cyan_1600, Function | MediumTest | Level0)
{
    ColorType cyan = Color::Cyan();

    EXPECT_EQ(cyan.red, 0x00);
    EXPECT_EQ(cyan.green, 0xFF);
    EXPECT_EQ(cyan.blue, 0xFF);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_AQUA_1700
 * @tc.name     test color aqua api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Aqua_1700, Function | MediumTest | Level0)
{
    ColorType aqua = Color::Aqua();

    EXPECT_EQ(aqua.red, 0x00);
    EXPECT_EQ(aqua.green, 0xFF);
    EXPECT_EQ(aqua.blue, 0xFF);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_TEAL_1800
 * @tc.name     test color teal api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Teal_1800, Function | MediumTest | Level0)
{
    ColorType teal = Color::Teal();

    EXPECT_EQ(teal.red, 0x00);
    EXPECT_EQ(teal.green, 0x80);
    EXPECT_EQ(teal.blue, 0x80);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_BLUE_1900
 * @tc.name     test color blue api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Blue_1900, Function | MediumTest | Level0)
{
    ColorType blue = Color::Blue();

    EXPECT_EQ(blue.red, 0x00);
    EXPECT_EQ(blue.green, 0x00);
    EXPECT_EQ(blue.blue, 0xFF);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_NAVY_2000
 * @tc.name     test color navy api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Navy_2000, Function | MediumTest | Level0)
{
    ColorType navy = Color::Navy();

    EXPECT_EQ(navy.red, 0x00);
    EXPECT_EQ(navy.green, 0x00);
    EXPECT_EQ(navy.blue, 0x80);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_MAGENTA_2100
 * @tc.name     test color magenta api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Magenta_2100, Function | MediumTest | Level0)
{
    ColorType magenta = Color::Magenta();

    EXPECT_EQ(magenta.red, 0xFF);
    EXPECT_EQ(magenta.green, 0x00);
    EXPECT_EQ(magenta.blue, 0xFF);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_PURPLE_2200
 * @tc.name     test color purple api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Purple_2200, Function | MediumTest | Level0)
{
    ColorType purple = Color::Purple();

    EXPECT_EQ(purple.red, 0x80);
    EXPECT_EQ(purple.green, 0x00);
    EXPECT_EQ(purple.blue, 0x80);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_ORANGE_2300
 * @tc.name     test color orange api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_Orange_2300, Function | MediumTest | Level0)
{
    ColorType orange = Color::Orange();

    EXPECT_EQ(orange.red, 0xFF);
    EXPECT_EQ(orange.green, 0xA5);
    EXPECT_EQ(orange.blue, 0x00);
}

/**
 * @tc.number   SUB_GRAPHIC_COLOR_GETMIXCOLOR_2400
 * @tc.name     test color getmixcolor api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ColorTest, Graphic_Color_Test_GetMixColor_2400, Function | MediumTest | Level0)
{
    ColorType c1;
    c1.red = 0;
    c1.green = 0;
    c1.blue = 0;
    ColorType c2;
    c2.red = 0;
    c2.green = 0;
    c2.blue = 0;
    uint8_t mix = 0;
    ColorType ret = Color::GetMixColor(c1, c2, mix);

    EXPECT_EQ(ret.red, 0);
    EXPECT_EQ(ret.green, 0);
    EXPECT_EQ(ret.blue, 0);
    EXPECT_EQ(ret.alpha, 0xFF); 
}
} // namespace OHOS