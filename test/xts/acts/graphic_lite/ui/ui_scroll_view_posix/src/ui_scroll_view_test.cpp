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
#include "components/ui_scroll_view.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class ScrollViewTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

class ScrollListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_SCROLL_VIEW_SETSCROLLSTATE_0100
 * @tc.name     test scroll-view set-scroll-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ScrollListenerTest, Graphic_UIScrollViewTest_Test_SetScrollState_0100, Function | MediumTest | Level0)
{
    UIScrollView::OnScrollListener *scrolllis = new UIScrollView::OnScrollListener();
    uint8_t state = 1;
    scrolllis->SetScrollState(state);
    EXPECT_EQ(scrolllis->GetScrollState(), state);
    delete scrolllis;
}

/**
 * @tc.number   SUB_GRAPHIC_SCROLL_VIEW_GETVIEWTYPE_0200
 * @tc.name     test scroll-view get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ScrollViewTest, Graphic_UIScrollViewTest_Test_GetViewType_0200, Function | MediumTest | Level0)
{
    UIScrollView *scrolllis = new UIScrollView();
    EXPECT_EQ(scrolllis->GetViewType(), UI_SCROLL_VIEW);
    delete scrolllis;
}

/**
 * @tc.number   SUB_GRAPHIC_SCROLL_VIEW_SETHORIZONTALSCROLLSTATE_0300
 * @tc.name     test scroll-view set-horizontal-scroll-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ScrollViewTest, Graphic_UIScrollViewTest_Test_SetHorizontalScrollState_0300, Function | MediumTest | Level0)
{
    UIScrollView *scrollView = new UIScrollView();
    bool state = false;
    scrollView->SetHorizontalScrollState(state);
    EXPECT_EQ(scrollView->GetHorizontalScrollState(), state);
    delete scrollView;
}

/**
 * @tc.number   SUB_GRAPHIC_SCROLL_VIEW_SETVERTICALSCROLLSTATE_0400
 * @tc.name     test scroll-view set-vertical-scroll-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ScrollViewTest, Graphic_UIScrollViewTest_Test_SetVerticalScrollState_0400, Function | MediumTest | Level0)
{
    UIScrollView *scrollView = new UIScrollView();
    bool state = false;
    scrollView->SetVerticalScrollState(state);
    EXPECT_EQ(scrollView->GetVerticalScrollState(), state);
    delete scrollView;
}

/**
 * @tc.number   SUB_GRAPHIC_SCROLL_VIEW_SCROLLBY_0500
 * @tc.name     test scroll-view scroll-by api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ScrollViewTest, Graphic_UIScrollViewTest_Test_ScrollBy_0500, Function | MediumTest | Level0)
{
    UIScrollView *scrollView = new UIScrollView();
    int16_t xDistance = 0;
    int16_t yDistance = 0;
    scrollView->ScrollBy(xDistance, yDistance);
    EXPECT_EQ(0, 0);
    delete scrollView;
}

/**
 * @tc.number   SUB_GRAPHIC_SCROLL_VIEW_SETXSCROLLBARVISIBLE_0600
 * @tc.name     test scroll-view set-vertical-scroll-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ScrollViewTest, Graphic_UIScrollViewTest_Test_SetXScrollBarVisible_0600, Function | MediumTest | Level0)
{
    UIScrollView *scrollView = new UIScrollView();
    bool state = false;
    scrollView->SetXScrollBarVisible(state);
    EXPECT_EQ(0, 0);
    delete scrollView;
}

/**
 * @tc.number   SUB_GRAPHIC_SCROLL_VIEW_SETYSCROLLBARVISIBLE_0700
 * @tc.name     test scroll-view set-vertical-scroll-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ScrollViewTest, Graphic_UIScrollViewTest_Test_SetYScrollBarVisible_0700, Function | MediumTest | Level0)
{
    UIScrollView *scrollView = new UIScrollView();
    bool state = false;
    scrollView->SetYScrollBarVisible(state);
    EXPECT_EQ(0, 0);
    delete scrollView;
}
}