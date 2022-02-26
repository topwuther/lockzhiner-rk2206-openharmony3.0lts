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
#include "components/ui_swipe_view.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UISwipeViewTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_SWIPE_VIEW_GETVIEWTYPE_0100
 * @tc.name     test swipe-view get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISwipeViewTest, Graphic_UISwipeView_Test_GetViewType_010, Function | MediumTest | Level0)
{
    UISwipeView *swipeView = new UISwipeView();
    EXPECT_EQ(swipeView->GetViewType(), UI_SWIPE_VIEW);
    delete swipeView;
}

/**
 * @tc.number   SUB_GRAPHIC_SWIPE_VIEW_SETDIRECTION_0200
 * @tc.name     test swipe-view set-direction api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISwipeViewTest, Graphic_UISwipeView_Test_SetDirection_0200, Function | MediumTest | Level0)
{
    UISwipeView *swipeView = new UISwipeView();
    uint8_t direction = 20;
    swipeView->SetDirection(direction);
    EXPECT_EQ(swipeView->GetDirection(), 0);
    delete swipeView;
}

/**
 * @tc.number   SUB_GRAPHIC_SWIPE_VIEW_ADD_0300
 * @tc.name     test swipe-view add api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISwipeViewTest, Graphic_UISwipeView_Test_Add_0300, Function | MediumTest | Level0)
{
    UISwipeView *swipeView = new UISwipeView();
    UIView* view = new UIView();
    swipeView->Add(view);
    EXPECT_EQ(view, swipeView->GetChildrenHead());
    EXPECT_EQ(view->GetParent(), swipeView);
    delete swipeView;
}

/**
 * @tc.number   SUB_GRAPHIC_SWIPE_VIEW_INSERT_0400
 * @tc.name     test swipe-view insert api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISwipeViewTest, Graphic_UISwipeView_Test_Insert_0400, Function | MediumTest | Level0)
{
    UISwipeView *swipeView = new UISwipeView();
    UIView* preView = new UIView();
    UIView* view = new UIView();
    swipeView->Add(preView);
    swipeView->Insert(preView, view);
    EXPECT_EQ(view, preView->GetNextSibling());

    delete view;
    delete preView;
    delete swipeView;
}

/**
 * @tc.number   SUB_GRAPHIC_SWIPE_VIEW_REMOVE_0500
 * @tc.name     test swipe-view remove api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISwipeViewTest, Graphic_UISwipeView_Test_Remove_0500, Function | MediumTest | Level0)
{
    UISwipeView *swipeView = new UISwipeView();
    UIView* view = new UIView();
    swipeView->Add(view);
    swipeView->Remove(view);

    EXPECT_EQ(nullptr, swipeView->GetChildrenHead());
    EXPECT_EQ(nullptr, view->GetParent());
    delete view;
    delete swipeView;
}

/**
 * @tc.number   SUB_GRAPHIC_SWIPE_VIEW_SETCURRENTPAGE_0600
 * @tc.name     test swipe-view set-current-page api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISwipeViewTest, Graphic_UISwipeView_Test_SetCurrentPage_0600, Function | MediumTest | Level0)
{
    UISwipeView *swipeView = new UISwipeView(UISwipeView::VERTICAL);
    swipeView->SetPosition(0, 0, 200, 200);
    UIViewGroup *view = new UIViewGroup();
    view->SetPosition(0, 0, 200, 200);
    swipeView->Add(view);
    UIViewGroup *view2 = new UIViewGroup();
    view2->SetPosition(0, 0, 200, 200);
    swipeView->Add(view2);
    uint16_t index = 0;
    swipeView->SetCurrentPage(index);
    EXPECT_EQ(swipeView->GetCurrentPage(), index);
    delete view;
    delete view2;
    delete swipeView;
}

/**
 * @tc.number   SUB_GRAPHIC_SWIPE_VIEW_SETANIMATORTIME_0800
 * @tc.name     test swipe-view set-current-page api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISwipeViewTest, Graphic_UISwipeView_Test_SetAnimatorTime_0800, Function | MediumTest | Level0)
{
    UISwipeView *swipeView = new UISwipeView(UISwipeView::VERTICAL);
    uint16_t time = 0;
    swipeView->SetAnimatorTime(time);
    EXPECT_EQ(0, 0);
    delete swipeView;
}

/**
 * @tc.number   SUB_GRAPHIC_SWIPE_VIEW_SETLOOPSTATE_0900
 * @tc.name     test swipe-view set-current-page api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISwipeViewTest, Graphic_UISwipeView_Test_SetLoopState_0900, Function | MediumTest | Level0)
{
    UISwipeView *swipeView = new UISwipeView(UISwipeView::VERTICAL);
    bool loop = false;
    swipeView->SetLoopState(loop);
    EXPECT_EQ(0, 0);
    delete swipeView;
}
} // namespace OHOS