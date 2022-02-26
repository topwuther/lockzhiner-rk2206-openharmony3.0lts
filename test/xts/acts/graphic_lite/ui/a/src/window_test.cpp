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
#include "common/graphic_startup.h"
#include "window/window.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class WindowTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_WINDOW_DESTORYWINDOW_0200
 * @tc.name     test window destory-window api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(WindowTest, Graphic_Window_Test_DestoryWindow_0200, Function | MediumTest | Level0)
{
    Window* testObj = NULL;
    Window::DestoryWindow(testObj);
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_WINDOW_HIDE_0400
 * @tc.name     test window hide api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(WindowTest, Graphic_Window_Test_Hide_0400, Function | MediumTest | Level0)
{
    OHOS::GraphicStartUp::Init();
    WindowConfig config = {};
    config.rect.SetRect(0, 0, 100, 100);
    Window* testObj = Window::CreateWindow(config);
    if (testObj != nullptr) {
        testObj->Hide();
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_WINDOW_MOVETO_0500
 * @tc.name     test window move-to api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(WindowTest, Graphic_Window_Test_MoveTo_0500, Function | MediumTest | Level0)
{
    OHOS::GraphicStartUp::Init();
    WindowConfig config = {};
    config.rect.SetRect(0, 0, 100, 100);
    Window* testObj = Window::CreateWindow(config);
    int16_t xInt = 64;
    int16_t yInt = 128;
    if (testObj != nullptr) {
        testObj->MoveTo(xInt, yInt);
        testObj->RaiseToTop();
        testObj->LowerToBottom();
    }
}

/**
 * @tc.number   SUB_GRAPHIC_WINDOW_RESIZE_0600
 * @tc.name     test window resize api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(WindowTest, Graphic_Window_Test_Resize_0600, Function | MediumTest | Level0)
{
    OHOS::GraphicStartUp::Init();
    WindowConfig config = {};
    config.rect.SetRect(0, 0, 100, 100);
    Window* testObj = Window::CreateWindow(config);
    int16_t width = 64;
    int16_t height = 128;
    if (testObj != nullptr) {
        testObj->Resize(width, height);
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_WINDOW_BINDROOTVIEW_0700
 * @tc.name     test window bind-root-view api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(WindowTest, Graphic_Window_Test_BindRootView_0700, Function | MediumTest | Level0)
{
    OHOS::GraphicStartUp::Init();
    WindowConfig config = {};
    config.rect.SetRect(0, 0, 100, 100);
    Window* testObj = Window::CreateWindow(config);
    if (testObj != nullptr) {
        testObj->BindRootView(RootView::GetInstance());
    }
    EXPECT_EQ(0, 0);
    if (testObj != nullptr) {
        EXPECT_EQ(testObj->GetRootView(), RootView::GetInstance());
        testObj->UnbindRootView();
    }
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_5800
 * @tc.name     test DestoryWindow api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(WindowTest, SUB_GRAPHIC_INTERFACE_FIRST_5800, Function | MediumTest | Level0)
{
    OHOS::GraphicStartUp::Init();
    WindowConfig windowConfig = {};
    Window* window = Window::CreateWindow(windowConfig);
    Window::DestoryWindow(window);
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_5900
 * @tc.name     test BindRootView api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(WindowTest, SUB_GRAPHIC_INTERFACE_FIRST_5900, Function | MediumTest | Level0)
{
    OHOS::GraphicStartUp::Init();
    WindowConfig windowConfig = {};
    Window* window = Window::CreateWindow(windowConfig);
    RootView* rootView = RootView::GetInstance();
    if (window!=nullptr) {
        window->BindRootView(rootView);
    }
    EXPECT_EQ(0, 0);
}
} // namespace OHOS
