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
#include "components/ui_surface_view.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UISurfaceViewTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SETPOSITION_0100
 * @tc.name     test surface set-position api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISurfaceViewTest, Graphic_UISurfaceViewTest_Test_SetPosition_0100, Function | MediumTest | Level0)
{
    UISurfaceView* surface = new UISurfaceView();
    int16_t x = 10;
    int16_t y = 20;
    surface->SetPosition(x, y);
    EXPECT_EQ(surface->GetX(), x);
    EXPECT_EQ(surface->GetY(), y);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SETPOSITION_0200
 * @tc.name     test surface set-position2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISurfaceViewTest, Graphic_UISurfaceViewTest_Test_SetPosition_0200, Function | MediumTest | Level0)
{
    UISurfaceView* surface = new UISurfaceView();
    int16_t x = 10;
    int16_t y = 20;
    int16_t width = 50;
    int16_t height = 30;
    surface->SetPosition(x, y, width, height);
    EXPECT_EQ(surface->GetX(), x);
    EXPECT_EQ(surface->GetY(), y);
    EXPECT_EQ(surface->GetWidth(), width);
    EXPECT_EQ(surface->GetHeight(), height);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_RESIZE_0300
 * @tc.name     test surface resize api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISurfaceViewTest, Graphic_UISurfaceViewTest_Test_Resize_0300, Function | MediumTest | Level0)
{
    UISurfaceView* surface = new UISurfaceView();
    int16_t width = 50;
    int16_t height = 30;
    surface->Resize(width, height);
    EXPECT_EQ(surface->GetWidth(), width);
    EXPECT_EQ(surface->GetHeight(), height);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SETX_0400
 * @tc.name     test surface setX api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISurfaceViewTest, Graphic_UISurfaceViewTest_Test_SetX_0400, Function | MediumTest | Level0)
{
    UISurfaceView* surface = new UISurfaceView();
    int16_t x = 10;
    surface->SetX(x);
    EXPECT_EQ(surface->GetX(), x);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SETY_0500
 * @tc.name     test surface setY api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISurfaceViewTest, Graphic_UISurfaceViewTest_Test_SetY_0500, Function | MediumTest | Level0)
{
    UISurfaceView* surface = new UISurfaceView();
    int16_t y = 10;
    surface->SetY(y);
    EXPECT_EQ(surface->GetY(), y);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SETWIDTH_0600
 * @tc.name     test surface set-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISurfaceViewTest, Graphic_UISurfaceViewTest_Test_SetWidth_0600, Function | MediumTest | Level0)
{
    UISurfaceView* surface = new UISurfaceView();
    int16_t width = 50;
    surface->SetWidth(width);
    EXPECT_EQ(surface->GetWidth(), width);
    delete surface;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SETHEIGHT_0700
 * @tc.name     test surface set-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISurfaceViewTest, Graphic_UISurfaceViewTest_Test_SetHeight_0700, Function | MediumTest | Level0)
{
    UISurfaceView* surface = new UISurfaceView();
    surface->SetVisible(true);
    EXPECT_EQ(surface->IsVisible(), true);
    delete surface;
}
} // namespace OHOS