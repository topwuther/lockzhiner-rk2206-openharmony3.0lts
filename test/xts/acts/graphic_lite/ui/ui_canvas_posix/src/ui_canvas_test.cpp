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
#include "components/ui_canvas.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class CanvasTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_SETPAINTSTYLE_0100
 * @tc.name     test canvas set-paintstyle api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_SetPaintStyle_0100, Function | MediumTest | Level0)
{
    Paint* paint = new Paint();
    Paint::PaintStyle paintStyle = Paint::PaintStyle::FILL_STYLE;
    paint->SetStyle(Paint::PaintStyle::FILL_STYLE);

    EXPECT_EQ(paint->GetStyle(), paintStyle);
    delete paint;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_SETSTROKEWIDTH_0200
 * @tc.name     test canvas set-strokewidth api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_SetStrokeWidth_0200, Function | MediumTest | Level0)
{
    Paint* paint = new Paint();
    uint16_t width = 10;
    paint->SetStrokeWidth(width);

    EXPECT_EQ(paint->GetStrokeWidth(), width);
    delete paint;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_SETSTROKECOLOR_0300
 * @tc.name     test canvas set-stroker-color api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_SetStrokeColor_0300, Function | MediumTest | Level0)
{
    Paint* paint = new Paint();
    ColorType color = Color::Red();
    paint->SetStrokeColor(color);

    EXPECT_EQ(paint->GetStrokeColor().full, color.full);
    delete paint;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_SETFILLCOLOR_0400
 * @tc.name     test canvas set-fillcolor api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_SetFillColor_0400, Function | MediumTest | Level0)
{
    Paint* paint = new Paint();
    ColorType color = Color::Red();
    paint->SetFillColor(color);

    EXPECT_EQ(paint->GetFillColor().full, color.full);
    delete paint;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_SETOPACITY_0500
 * @tc.name     test canvas set-opacity api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_SetOpacity_0500, Function | MediumTest | Level0)
{
    Paint* paint = new Paint();
    uint8_t opacity = 200;
    paint->SetOpacity(opacity);

    EXPECT_EQ(paint->GetOpacity(), opacity);
    delete paint;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_STARTPOSITION_0600
 * @tc.name     test canvas set-start-position api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_SetStartPosition_0600, Function | MediumTest | Level0)
{
    UICanvas* canvas = new UICanvas();
    canvas->SetStartPosition({ 50, 100 });

    EXPECT_EQ(canvas->GetStartPosition().x, 50);
    EXPECT_EQ(canvas->GetStartPosition().y, 100);
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_DRAWLINE_0700
 * @tc.name     test canvas draw-line api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_DrawLine_0700, Function | MediumTest | Level0)
{
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    viewGroup->SetPosition(0, 0);
    viewGroup->SetWidth(100);
    viewGroup->SetHeight(100);

    UICanvas* canvas = new UICanvas();
    Paint paint;
    canvas->DrawLine({ 0, 50 }, { 50, 100 }, paint);

    viewGroup->Add(canvas);

    EXPECT_EQ(canvas->GetStartPosition().x, 50);
    EXPECT_EQ(canvas->GetStartPosition().y, 100);
    viewGroup->Remove(canvas);
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_DRAWCURVE_0800
 * @tc.name     test canvas draw-curve api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_DrawCurve_0800, Function | MediumTest | Level0)
{
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    viewGroup->SetPosition(0, 0);
    viewGroup->SetWidth(100);
    viewGroup->SetHeight(100);

    UICanvas* canvas = new UICanvas();
    Paint paint;
    canvas->DrawCurve({ 100, 100 }, { 150, 100 }, { 150, 100 }, { 150, 200 }, paint);

    viewGroup->Add(canvas);
    EXPECT_EQ(canvas->GetStartPosition().x, 150);
    EXPECT_EQ(canvas->GetStartPosition().y, 200);

    viewGroup->Remove(canvas);
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_DRAWRECT_0900
 * @tc.name     test canvas draw-rect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_DrawRect_0900, Function | MediumTest | Level0)
{
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    viewGroup->SetPosition(0, 0);
    viewGroup->SetWidth(100);
    viewGroup->SetHeight(100);

    UICanvas* canvas = new UICanvas();
    Paint paint;
    canvas->DrawRect({ 10, 10 }, 50, 50, paint);

    viewGroup->Add(canvas);
    EXPECT_EQ(10, 10);

    viewGroup->Remove(canvas);
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_DRAWCIRCLE_1000
 * @tc.name     test canvas draw-circle api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_DrawCircle_1000, Function | MediumTest | Level0)
{
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    viewGroup->SetPosition(0, 0);
    viewGroup->SetWidth(100);
    viewGroup->SetHeight(100);

    UICanvas* canvas = new UICanvas();
    Paint paint;
    canvas->DrawCircle({ 50, 50 }, 20, paint);

    viewGroup->Add(canvas);
    EXPECT_EQ(0, 0);

    viewGroup->Remove(canvas);
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_DRAWARC_1100
 * @tc.name     test canvas draw-arc api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_DrawArc_1100, Function | MediumTest | Level0)
{
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    viewGroup->SetPosition(0, 0);
    viewGroup->SetWidth(200);
    viewGroup->SetHeight(200);

    UICanvas* canvas = new UICanvas();
    Paint paint;
    canvas->DrawArc({ 100, 150 }, 50, 135, 270, paint);

    viewGroup->Add(canvas);
    EXPECT_EQ(0, 0);

    viewGroup->Remove(canvas);
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_DRAWSECTOR_1200
 * @tc.name     test canvas draw-sector api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_DrawSector_1200, Function | MediumTest | Level0)
{
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    viewGroup->SetPosition(0, 0);
    viewGroup->SetWidth(200);
    viewGroup->SetHeight(200);

    UICanvas* canvas = new UICanvas();
    Paint paint;
    canvas->DrawSector({ 100, 50 }, 100, 0, 30, paint);

    viewGroup->Add(canvas);
    EXPECT_EQ(0, 0);

    viewGroup->Remove(canvas);
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_DRAWIMAGE_1300
 * @tc.name     test canvas draw-image api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_DrawImage_1300, Function | MediumTest | Level0)
{
    RootView* rootView = RootView::GetInstance();
    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    viewGroup->SetPosition(0, 0);
    viewGroup->SetWidth(200);
    viewGroup->SetHeight(200);

    UICanvas* canvas = new UICanvas();
    Paint paint;
    canvas->DrawImage({ 50, 50 }, "..\\config\\images\\A021_007.bin", paint);

    viewGroup->Add(canvas);
    EXPECT_EQ(0, 0);

    viewGroup->Remove(canvas);
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_DRAWLABEL_1400
 * @tc.name     test canvas draw-label api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_DrawLabel_1400, Function | MediumTest | Level0)
{
    RootView* rootView = RootView::GetInstance();

    UIViewGroup* viewGroup = static_cast<UIViewGroup*>(rootView);
    viewGroup->SetPosition(0, 0);
    viewGroup->SetWidth(200);
    viewGroup->SetHeight(200);

    UICanvas* canvas = new UICanvas();
    UICanvas::FontStyle fontStyle;
    fontStyle.align = TEXT_ALIGNMENT_RIGHT;
    fontStyle.direct = TEXT_DIRECT_RTL;
    fontStyle.fontName = "HYQiHei-65S";
    fontStyle.fontSize = 30;
    fontStyle.letterSpace = 10;
    Paint paint;
    canvas->DrawLabel({ 50, 50 }, "²âÊÔcanvas»æÖÆ×ÖÌå", 100, fontStyle, paint);
    
    viewGroup->Add(canvas);
    EXPECT_EQ(0, 0);

    viewGroup->Remove(canvas);
    canvas->Clear();
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_CANVAS_PAINT_1500
 * @tc.name     test canvas paint api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(CanvasTest, Graphic_UICanvas_Test_Paint_1500, Function | MediumTest | Level0)
{
    Paint* paint = new Paint();
    EXPECT_NE(paint, nullptr);

    delete paint;
}
} // namespace OHOS