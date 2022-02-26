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
#include "components/ui_view.h"
#include "components/ui_view_group.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIViewTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static UIView* view;
};
UIView* UIViewTest::view = nullptr;

void UIViewTest::SetUpTestCase(void)
{
    view = new UIView();
    if (view == nullptr) {
        printf("Failed to apply for memory.\n");
        return;
    }
}

void UIViewTest::TearDownTestCase(void)
{
    if (view == nullptr) {
        delete view;
        view = nullptr;
    }
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETONDRAGLISTENER_0100
 * @tc.name     test UIview set-on-drag-listener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetOnDragListener_0100, Function | MediumTest | Level0)
{
    UIView::OnDragListener* listener = new UIView::OnDragListener();
    view->SetOnDragListener(listener);
    EXPECT_EQ(view->GetOnDragListener(), listener);
    delete listener;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETONCLICKLISTENER_0200
 * @tc.name     test UIview set-on-click-listener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetOnClickListener_0200, Function | MediumTest | Level0)
{
    UIView::OnClickListener* listener = new UIView::OnClickListener();
    view->SetOnClickListener(listener);
    EXPECT_EQ(view->GetOnClickListener(), listener);
    delete listener;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETONLONGPRESSLISTENER_0300
 * @tc.name     test UIview set-on-long-press-listener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetOnLongPressListener_0300, Function | MediumTest | Level0)
{
    UIView::OnLongPressListener* listener = new UIView::OnLongPressListener();
    view->SetOnLongPressListener(listener);
    EXPECT_EQ(view->GetOnLongPressListener(), listener);
    delete listener;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETONTOUCHLISTENER_0400
 * @tc.name     test UIview set-on-touch-listener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetOnTouchListener_0400, Function | MediumTest | Level0)
{
    UIView::OnTouchListener* listener = new UIView::OnTouchListener();
    view->SetOnTouchListener(listener);
    EXPECT_EQ(view->GetTouchListener(), listener);
    delete listener;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETPARENT_0500
 * @tc.name     test UIview set-parent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetParent_0500, Function | MediumTest | Level0)
{
    UIViewGroup* viewGroup = new UIViewGroup();
    view->SetParent(viewGroup);
    EXPECT_EQ(view->GetParent(), viewGroup);
    delete viewGroup;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETNEXTSIBLING_0600
 * @tc.name     test UIview set-next-sibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetNextSibling_0600, Function | MediumTest | Level0)
{
    UIView* tempView = new UIView();
    view->SetNextSibling(tempView);
    EXPECT_EQ(view->GetNextSibling(), tempView);
    delete tempView;
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETVISIBLE_0700
 * @tc.name     test UIview set-visible api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetVisible_0700, Function | MediumTest | Level0)
{
    view->SetVisible(true);
    EXPECT_EQ(view->IsVisible(), true);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETTOUCHABLE_0800
 * @tc.name     test UIview set-touchable api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetTouchable_0800, Function | MediumTest | Level0)
{
    view->SetTouchable(true);
    EXPECT_EQ(view->IsTouchable(), true);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETTOUCHABLE_0900
 * @tc.name     test UIview set-touchable2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetTouchable_0900, Function | MediumTest | Level0)
{
    view->SetDraggable(true);
    EXPECT_EQ(view->IsDraggable(), true);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETDRAGPARENTINSTEAD_1000
 * @tc.name     test UIview set-drag-parent-instead api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetDragParentInstead_1000, Function | MediumTest | Level0)
{
    view->SetDragParentInstead(true);
    EXPECT_EQ(view->IsDragParentInstead(), true);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_RESIZEVISIBLEAREA_1100
 * @tc.name     test UIview resize-visible-area api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_ResizeVisibleArea_1100, Function | MediumTest | Level0)
{
    int16_t x = 10;
    int16_t y = 20;
    int16_t width = 30;
    int16_t height = 60;
    view->ResizeVisibleArea(x, y, width, height);
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETWIDTH_1200
 * @tc.name     test UIview set-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetWidth_1200, Function | MediumTest | Level0)
{
    int16_t width = 50;
    view->SetWidth(width);
    EXPECT_EQ(view->GetWidth(), width);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETHEIGHT_1300
 * @tc.name     test UIview set-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetHeight_1300, Function | MediumTest | Level0)
{
    int16_t height = 30;
    view->SetHeight(height);
    EXPECT_EQ(view->GetHeight(), height);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_RESIZE_1400
 * @tc.name     test UIview resize api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_Resize_1400, Function | MediumTest | Level0)
{
    int16_t height = 30;
    int16_t width = 50;
    view->Resize(width, height);
    EXPECT_EQ(view->GetHeight(), height);
    EXPECT_EQ(view->GetWidth(), width);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETX_1500
 * @tc.name     test UIview setX api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetX_1500, Function | MediumTest | Level0)
{
    int16_t x = 30;
    view->SetX(x);
    EXPECT_EQ(view->GetX(), x);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETY_1600
 * @tc.name     test UIview setY api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetX_1600, Function | MediumTest | Level0)
{
    int16_t y = 30;
    view->SetY(y);
    EXPECT_EQ(view->GetY(), y);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETPOSITION_1700
 * @tc.name     test UIview set-position api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetPosition_1700, Function | MediumTest | Level0)
{
    int16_t x = 30;
    int16_t y = 60;
    view->SetPosition(x, y);
    EXPECT_EQ(view->GetX(), x);
    EXPECT_EQ(view->GetY(), y);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETPOSITION_1800
 * @tc.name     test UIview set-position2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetPosition_1800, Function | MediumTest | Level0)
{
    int16_t x = 30;
    int16_t y = 60;
    int16_t height = 50;
    int16_t width = 100;
    view->SetPosition(x, y, width, height);
    EXPECT_EQ(view->GetHeight(), height);
    EXPECT_EQ(view->GetWidth(), width);
    EXPECT_EQ(view->GetX(), x);
    EXPECT_EQ(view->GetY(), y);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETVIEWID_1900
 * @tc.name     test UIview set-viewID api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetViewId_1900, Function | MediumTest | Level0)
{
    const char* id = "graphic_unittest_view";
    view->SetViewId(id);
    EXPECT_EQ(memcmp(view->GetViewId(), id, strlen(id)), 0);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETVIEWINDEX_2000
 * @tc.name     test UIview set-view-index api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetViewIndex_2000, Function | MediumTest | Level0)
{
    int16_t index = 1;
    view->SetViewIndex(index);
    EXPECT_EQ(view->GetViewIndex(), index);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_GETVIEWTYPE_2100
 * @tc.name     test UIview get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_GetViewType_2100, Function | MediumTest | Level0)
{
    EXPECT_EQ(view->GetViewType(), UI_NUMBER_MAX);
}

/**
 * @tc.number   SUB_GRAPHIC_UIVIEW_SETOPACALE_2200
 * @tc.name     test UIview set-opa-scale api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIViewTest, Graphic_UIView_Test_SetOpaScale_2200, Function | MediumTest | Level0)
{
    uint8_t opaScale = 16;
    view->SetOpaScale(opaScale);
    EXPECT_EQ(view->GetOpaScale(), opaScale);
}
} // namespace OHOS