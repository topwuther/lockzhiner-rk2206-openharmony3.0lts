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

#include "components/ui_view_group.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class UIViewTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GETRECT_0100
 * @tc.name     test ui_view GetRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test__GetRect_0100, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->GetRect();
    EXPECT_EQ(view->GetRect().GetX(), 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GetVisibleRect_0200
 * @tc.name     test UIview GetVisibleRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_GetVisibleRect_0200, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->SetVisible(true);
    view->Resize(100, 100);
    EXPECT_EQ(view->GetVisibleRect().GetHeight(), 100);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GETMASKEDRECT_0300
 * @tc.name     test UIview GetMaskedRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_GetMaskedRect_0300, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->Resize(50, 50);
    EXPECT_EQ(view->GetMaskedRect().GetHeight(), 50);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GETORIGRECT_0400
 * @tc.name     test UIview GetOrigRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_GetOrigRect_0400, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->Resize(50, 50);
    EXPECT_EQ(view->GetOrigRect().GetHeight(), 50);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GETCONTENTRECT_0500
 * @tc.name     test UIview GetContentRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_GetContentRect_0500, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->Resize(50, 50);
    EXPECT_EQ(view->GetContentRect().GetHeight(), 50);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GETRELATIVERECT_0600
 * @tc.name     test UIview GetRelativeRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_GetRelativeRect_0600, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->SetPosition(100, 50, 10, 10);
    EXPECT_EQ(view->GetRelativeRect().GetX(), 100);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETINTERCEPT_0700
 * @tc.name     test UIview set-intercept api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetIntercept_0700, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->SetIntercept(true);
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_ISVIEWGROUP_0800
 * @tc.name     test UIview IsViewGroup api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_IsViewGroup_0800, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    EXPECT_EQ(view->IsViewGroup(), false);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GetChildById_0900
 * @tc.name     test UIview GetChildById api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_GetChildById_0900, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    UIViewGroup* group = new UIViewGroup();
    view->SetViewId("hello world!");
    view->Resize(100, 100);
    group->Add(view);
    UIView* viewChild = group->GetChildById("hello world!");
    int16_t height = 0;
    if (viewChild != NULL) {
        height = viewChild->GetHeight();
    }
    EXPECT_EQ(height, 100);
    delete group;
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LAYOUTCENTEROFPARENT_1000
 * @tc.name     test UIview LayoutCenterOfParent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutCenterOfParent_1000, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->LayoutCenterOfParent();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LAYOUTCHILDREN_1100
 * @tc.name     test UIview LayoutChildren api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutChildren_1100, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->LayoutChildren(true);
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LAYOUTLEFTOFPARENT_1200
 * @tc.name     test UIview LayoutLeftOfParent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutLeftOfParent_1200, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->LayoutLeftOfParent();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LAYOUTRIGHTOFPARENT_1300
 * @tc.name     test UIview LayoutRightOfParent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutRightOfParent_1300, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->LayoutRightOfParent();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LAYOUTTOPOFPARENT_1400
 * @tc.name     test UIview LayoutTopOfParent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutTopOfParent_1400, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->LayoutTopOfParent();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LayoutBottomOfParent_1500
 * @tc.name     test UIview LayoutBottomOfParent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutBottomOfParent_1500, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->LayoutBottomOfParent();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_ALIGNLEFTTOSIBLING_1600
 * @tc.name     test UIview AlignLeftToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_AlignLeftToSibling_1600, Function | MediumTest | Level0)
{
    UIViewGroup* display = new UIViewGroup();
    UIView* refTarget = new UIView();
    UIView* view = new UIView();
    UIViewGroup* testTarget = new UIViewGroup();

    display->Add(refTarget);
    refTarget->Resize(20, 20);
    refTarget->SetViewId("refView");
    testTarget->AlignLeftToSibling("refView");

    EXPECT_EQ(0, 0);
    delete testTarget;
    delete view;
    delete refTarget;
    delete display;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_ALIGNRIGHTTOSIBLING_1700
 * @tc.name     test UIview AlignRightToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_AlignRightToSibling_1700, Function | MediumTest | Level0)
{
    UIViewGroup* display = new UIViewGroup();
    UIView* refTarget = new UIView();
    UIView* view = new UIView();
    UIViewGroup* testTarget = new UIViewGroup();

    display->Add(refTarget);
    refTarget->Resize(20, 20);
    refTarget->SetViewId("refView");
    testTarget->AlignRightToSibling("refView");

    EXPECT_EQ(0, 0);
    delete testTarget;
    delete view;
    delete refTarget;
    delete display;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_ALIGNTOPTOSIBLING_1800
 * @tc.name     test UIview AlignTopToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_AlignTopToSibling_1800, Function | MediumTest | Level0)
{
    UIViewGroup* display = new UIViewGroup();
    UIView* refTarget = new UIView();
    UIView* view = new UIView();
    UIViewGroup* testTarget = new UIViewGroup();

    display->Add(refTarget);
    refTarget->Resize(20, 20);
    refTarget->SetViewId("refView");
    testTarget->AlignTopToSibling("refView");

    EXPECT_EQ(0, 0);
    delete testTarget;
    delete view;
    delete refTarget;
    delete display;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_ALIGNBOTTOMTOSIBLING_1900
 * @tc.name     test UIview AlignBottomToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_AlignBottomToSibling_1900, Function | MediumTest | Level0)
{
    UIViewGroup* display = new UIViewGroup();
    UIView* refTarget = new UIView();
    UIView* view = new UIView();
    UIViewGroup* testTarget = new UIViewGroup();

    display->Add(refTarget);
    refTarget->Resize(20, 20);
    refTarget->SetViewId("refView");
    testTarget->AlignBottomToSibling("refView");

    EXPECT_EQ(0, 0);
    delete testTarget;
    delete view;
    delete refTarget;
    delete display;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_AlignHorCenterToSibling_2000
 * @tc.name     test UIview AlignHorCenterToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_AlignHorCenterToSibling_2000, Function | MediumTest | Level0)
{
    UIViewGroup* display = new UIViewGroup();
    UIView* refTarget = new UIView();
    UIView* view = new UIView();
    UIViewGroup* testTarget = new UIViewGroup();

    display->Add(refTarget);
    refTarget->Resize(20, 20);
    refTarget->SetViewId("refView");
    testTarget->AlignHorCenterToSibling("refView");

    EXPECT_EQ(0, 0);
    delete testTarget;
    delete view;
    delete refTarget;
    delete display;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LAYOUTLEFTTOSIBLING_2100
 * @tc.name     test UIview LayoutLeftToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutLeftToSibling_2100, Function | MediumTest | Level0)
{
    UIViewGroup* display = new UIViewGroup();
    UIView* refTarget = new UIView();
    UIView* view = new UIView();
    UIViewGroup* testTarget = new UIViewGroup();

    display->Add(refTarget);
    refTarget->Resize(20, 20);
    refTarget->SetViewId("refView");
    testTarget->LayoutLeftToSibling("refView");

    EXPECT_EQ(0, 0);
    delete testTarget;
    delete view;
    delete refTarget;
    delete display;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LAYOUTRIGHTTOSIBLING_2200
 * @tc.name     test UIview LayoutRightToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutRightToSibling_2200, Function | MediumTest | Level0)
{
    UIViewGroup* display = new UIViewGroup();
    UIView* refTarget = new UIView();
    UIView* view = new UIView();
    UIViewGroup* testTarget = new UIViewGroup();

    display->Add(refTarget);
    refTarget->Resize(20, 20);
    refTarget->SetViewId("refView");
    testTarget->LayoutRightToSibling("refView");

    EXPECT_EQ(0, 0);
    delete testTarget;
    delete view;
    delete refTarget;
    delete display;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LAYOUTTOPTOSIBLING_2300
 * @tc.name     test UIview LayoutTopToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutTopToSibling_2300, Function | MediumTest | Level0)
{
    UIViewGroup* display = new UIViewGroup();
    UIView* refTarget = new UIView();
    UIView* view = new UIView();
    UIViewGroup* testTarget = new UIViewGroup();

    display->Add(refTarget);
    refTarget->Resize(20, 20);
    refTarget->SetViewId("refView");
    testTarget->LayoutTopToSibling("refView");

    EXPECT_EQ(0, 0);
    delete testTarget;
    delete view;
    delete refTarget;
    delete display;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_LAYOUTBOTTOMTOSIBLING_2400
 * @tc.name     test UIview LayoutBottomToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_LayoutBottomToSibling_2400, Function | MediumTest | Level0)
{
    UIViewGroup* display = new UIViewGroup();
    UIView* refTarget = new UIView();
    UIView* view = new UIView();
    UIViewGroup* testTarget = new UIViewGroup();

    display->Add(refTarget);
    refTarget->Resize(20, 20);
    refTarget->SetViewId("refView");
    testTarget->LayoutBottomToSibling("refView");

    EXPECT_EQ(0, 0);
    delete testTarget;
    delete view;
    delete refTarget;
    delete display;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GETSTYLECONST_2500
 * @tc.name     test UIview get-style-const api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_GetStyleConst_2500,  Function | MediumTest | Level0)
{
    UIView* view  = new UIView();
    view->SetStyle(STYLE_BACKGROUND_OPA, 255);
    EXPECT_EQ(view->GetStyleConst().GetStyle(STYLE_BACKGROUND_OPA), 255);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_REMEASURE_2600
 * @tc.name     test UIview ReMeasure api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_ReMeasure_2600, Function | MediumTest | Level0)
{
    UIView* view = new UIView();

    view->ReMeasure();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_INVALIDATE_2700
 * @tc.name     test UIview Invalidate api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_Invalidate_2700, Function | MediumTest | Level0)
{
    UIView* view = new UIView();

    view->Invalidate();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GETTARGETVIEW_2800
 * @tc.name     test UIview GetTargetView api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_GetTargetView_2800, Function | MediumTest | Level0)
{
    Point pos = {10, 10};
    UIView* view = new UIView();
    view->SetPosition(0, 0, 454, 454);
    UIView* view2 = new UIView();
    view->GetTargetView(pos, &view2);
    EXPECT_EQ(0, 0);
    delete view;
    delete view2;
}
} // namespace OHOS