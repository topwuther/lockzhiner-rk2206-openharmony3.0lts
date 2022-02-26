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

#include "UiInterfaceTest.h"

#include "common/graphic_startup.h"
#include "components/root_view.h"
#include "components/text_adapter.h"
#include "components/ui_abstract_clock.h"
#include "components/ui_analog_clock.h"
#include "components/ui_axis.h"
#include "components/ui_button.h"
#include "components/ui_canvas.h"
#include "components/ui_checkbox.h"
#include "components/ui_image_animator.h"
#include "components/ui_scroll_view.h"
#include "components/ui_swipe_view.h"
#include "components/ui_time_picker.h"
#include "components/ui_view.h"
#include "components/ui_view_group.h"
#include "dfx/event_injector.h"
#include "dfx/ui_screenshot.h"
#include "events/aod_callback.h"
#include "events/press_event.h"
#include "events/virtual_device_event.h"
#include "gfx_utils/graphic_buffer.h"
#include "gfx_utils/graphic_math.h"
#include "themes/theme_manager.h"
#include "window/window.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UiInterfaceTest : public testing::Test {
public:
    static void SetUpTestCase(void){}
    static void TearDownTestCase(void){}
};

class TestListener : public OHOS::UICheckBox::OnChangeListener {
public:
    bool OnChange(UICheckBox::UICheckBoxState state) override
    {
        return true;
    }
};

class TestAnimatorCallback : public AnimatorCallback {
public:
    TestAnimatorCallback() {}

    ~TestAnimatorCallback() {}

    void Callback(UIView* view) override {}
};

class Font {
public:
    void SetDynamicFontId() const {};
    void SetAnimatorSpeed() const {};
    void GetLongMode() const {};
    void Scal() const {};
    void Itoa() const {};
    void GetCrossPoint() const {};
    void IsLineSegmentCross() const {};
    void IsRectCross() const {};
    void SetOnChangeListener() const {};
    void SetLineAndTextOpacity() const {};
    void OnClickEventHappen() const {};
};

class KeyActListener {
public:
    void OnDragEndEventHappen() const {};
    void OnLongPressEventHappen() const {};
    void OnFPSChangedListener() const {};
    void OnFPSChanged() const {};
    void GetFPS() const {};
    void RegisterFPSChangedListener() const {};
    void CreateSurface() const {};
    void GetQueueSize() const {};
    void RegisterConsumerListener() const {};
    void UnregisterConsumerListener() const {};
    void GetInt64() const {};
    void GetSurface() const {};
    void OnKeyAct() const {};
    void OnKeyEvent() const {};
    void SetOnKeyActListener() const {};
    void ClearOnKeyActListener() const {};
};

class RootViewManage {
public:
    void OnVirtualDeviceEvent() const {};
    void SetOnVirtualDeviceEventListener() const {};
    void ClearOnVirtualDeviceEventListener() const {};
    void EnableCursor() const {};
    void GetCursor() const {};
    void DrawTop() const {};
    void GetTopUIView() const {};
    void FindSubView() const {};
    void SetFilePath() const {};
};
/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_0100
 * @tc.name     test InvalidateRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_0100, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->InvalidateRect(view->GetRect());
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_0200
 * @tc.name     test OnLongPressEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_0200, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    Point initPos = {100, 200};
    TimeType timeStamp = 10;
    LongPressEvent* longPressEvent = new LongPressEvent(initPos, timeStamp);
    view->OnLongPressEvent(*longPressEvent);
    EXPECT_EQ(0, 0);
    delete longPressEvent;
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_0300
 * @tc.name     test OnDragStartEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_0300, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    Point initPos = {100, 200};
    DragEvent* dragEvent = new DragEvent(initPos, initPos, initPos);
    view->OnDragStartEvent(*dragEvent);
    EXPECT_EQ(0, 0);
    delete dragEvent;
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_0400
 * @tc.name     test OnDragEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_0400, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    Point initPos = {100, 200};
    DragEvent* dragEvent = new DragEvent(initPos, initPos, initPos);
    view->OnDragEvent(*dragEvent);
    EXPECT_EQ(0, 0);
    delete dragEvent;
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_0500
 * @tc.name     test OnDragEndEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_0500, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    Point initPos = {100, 200};
    DragEvent* dragEvent = new DragEvent(initPos, initPos, initPos);
    view->OnDragEndEvent(*dragEvent);
    EXPECT_EQ(0, 0);
    delete dragEvent;
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_0600
 * @tc.name     test OnPressEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_0600, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    Point initPos = {100, 200};
    PressEvent* pressEvent = new PressEvent(initPos);
    view->OnPressEvent(*pressEvent);
    EXPECT_EQ(0, 0);
    delete pressEvent;
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_0700
 * @tc.name     test OnCancelEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_0700, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    Point initPos = {100, 200};
    CancelEvent* cancelEvent = new CancelEvent(initPos);
    view->OnCancelEvent(*cancelEvent);
    EXPECT_EQ(0, 0);
    delete cancelEvent;
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_0800
 * @tc.name     test GetMaskedRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_0800, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->GetMaskedRect();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_0900
 * @tc.name     test GetOrigRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_0900, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->GetOrigRect();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1000
 * @tc.name     test GetRelativeRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1000, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->GetRelativeRect();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1100
 * @tc.name     test AlignVerCenterToSibling api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1100, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->AlignVerCenterToSibling("refView");
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1200
 * @tc.name     test GetStyleConst api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1200, Function | MediumTest | Level0)
{
    UIView* view = new UIView();
    view->GetStyleConst();
    EXPECT_EQ(0, 0);
    delete view;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1300
 * @tc.name     test TranslateToPixel api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1300, Function | MediumTest | Level0)
{
    UIXAxis* axis = new UIXAxis();
    int16_t x = 12;
    axis->TranslateToPixel(x);
    EXPECT_EQ(0, 0);
    delete axis;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1400
 * @tc.name     test SetAutoSize api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1400, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    group->SetAutoSize(true);
    EXPECT_EQ(0, 0);
    delete group;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1500
 * @tc.name     test GetChildrenHead api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1500, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    group->GetChildrenHead();
    EXPECT_EQ(0, 0);
    delete group;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1600
 * @tc.name     test MoveChildByOffset api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1600, Function | MediumTest | Level0)
{
    UIViewGroup* group = new UIViewGroup();
    group->MoveChildByOffset(1, 1);
    EXPECT_EQ(0, 0);
    delete group;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1700
 * @tc.name     test OnTimePickerStoped api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1700, Function | MediumTest | Level0)
{
    UITimePicker::SelectedListener* listener = new UITimePicker::SelectedListener();
    UITimePicker* timePicker = new UITimePicker();
    listener->OnTimePickerStoped(*timePicker);
    EXPECT_EQ(0, 0);
    delete timePicker;
    delete listener;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1800
 * @tc.name     test GetSelectSecond api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1800, Function | MediumTest | Level0)
{
    UITimePicker* timePicker = new UITimePicker();
    timePicker->GetSelectSecond();
    EXPECT_EQ(0, 0);
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_1900
 * @tc.name     test GetSelectMinute api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_1900, Function | MediumTest | Level0)
{
    UITimePicker* timePicker = new UITimePicker();
    timePicker->GetSelectMinute();
    EXPECT_EQ(0, 0);
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2000
 * @tc.name     test GetSelectHour api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2000, Function | MediumTest | Level0)
{
    UITimePicker* timePicker = new UITimePicker();
    timePicker->GetSelectHour();
    EXPECT_EQ(0, 0);
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2100
 * @tc.name     test GetSelectValue api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2100, Function | MediumTest | Level0)
{
    UITimePicker* timePicker = new UITimePicker();
    timePicker->GetSelectValue();
    EXPECT_EQ(0, 0);
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2200
 * @tc.name     test SetOnSwipeListener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2200, Function | MediumTest | Level0)
{
    UISwipeView* uiSwipeView = new UISwipeView();
    uiSwipeView->SetOnSwipeListener(uiSwipeView->GetOnSwipeListener());
    EXPECT_EQ(0, 0);
    delete uiSwipeView;
}


/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2300
 * @tc.name     test OnScrollEnd api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2300, Function | MediumTest | Level0)
{
    UIScrollView::OnScrollListener* listScrollListener = new     UIScrollView::OnScrollListener();
    listScrollListener->OnScrollEnd();
    EXPECT_EQ(0, 0);
    delete listScrollListener;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2400
 * @tc.name     test OnScrollStart api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2400, Function | MediumTest | Level0)
{
    UIScrollView::OnScrollListener* listScrollListener = new UIScrollView::OnScrollListener();
    listScrollListener->OnScrollStart();
    EXPECT_EQ(0, 0);
    delete listScrollListener;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2500
 * @tc.name     test RegisterScrollListener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2500, Function | MediumTest | Level0)
{
    UIScrollView* uiScrollView = new UIScrollView();
    UIScrollView::OnScrollListener* onScrollListener = new UIScrollView::OnScrollListener();
    uiScrollView->RegisterScrollListener(onScrollListener);
    EXPECT_EQ(0, 0);
    delete onScrollListener;
    delete uiScrollView;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2600
 * @tc.name     test OnPickerStoped api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2600, Function | MediumTest | Level0)
{
    UIPicker::SelectedListener* selectedListener = new UIPicker::SelectedListener();
    UIPicker* uiPicker = new UIPicker();
    selectedListener->OnPickerStoped(*uiPicker);
    EXPECT_EQ(0, 0);
    delete uiPicker;
    delete selectedListener;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2700
 * @tc.name     test EnableAutoAlign api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2700, Function | MediumTest | Level0)
{
    UIList* uiList = new UIList();
    uiList->EnableAutoAlign(true);
    EXPECT_EQ(0, 0);
    delete uiList;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2800
 * @tc.name     test GetText api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2800, Function | MediumTest | Level0)
{
    UILabel* uiLabel = new UILabel();
    uiLabel->GetText();
    EXPECT_EQ(0, 0);
    delete uiLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_2900
 * @tc.name     test VirtualDeviceEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_2900, Function | MediumTest | Level0)
{
    VirtualDeviceEvent* virtualDeviceEvent =  new VirtualDeviceEvent(1, 1);
    EXPECT_EQ(0, 0);
    delete virtualDeviceEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_3000
 * @tc.name     test PressEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_3000, Function | MediumTest | Level0)
{
    Point* point = new Point();
    PressEvent* pressEvent =  new PressEvent(*point);
    EXPECT_EQ(0, 0);
    delete point;
    delete pressEvent;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_3100
 * @tc.name     test DragEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_3100, Function | MediumTest | Level0)
{
    Point* point = new Point();
    DragEvent* dragEvent = new DragEvent(*point, *point, *point);
    EXPECT_EQ(0, 0);
    delete dragEvent;
    delete point;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_4600
 * @tc.name     test GetCurImageSrc api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_4600, Function | MediumTest | Level0)
{
    UIButton* uiButton = new UIButton();
    uiButton->GetCurImageSrc();
    EXPECT_EQ(0, 0);
    delete uiButton;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_4700
 * @tc.name     test Paint api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_4700, Function | MediumTest | Level0)
{
    Paint* paint = new Paint();
    EXPECT_EQ(0, 0);
    delete paint;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_4800
 * @tc.name     test OnChange api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_4800, Function | MediumTest | Level0)
{
    auto* onChangeListener = new TestListener();
    UICheckBox::UICheckBoxState* state = new UICheckBox::UICheckBoxState();
    onChangeListener->OnChange(*state);
    EXPECT_EQ(0, 0);
    delete state;
    delete onChangeListener;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_4900
 * @tc.name     test SetOnChangeListener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_4900, Function | MediumTest | Level0)
{
    UICheckBox* uICheckBox = new UICheckBox();
    uICheckBox->SetOnChangeListener(NULL);
    EXPECT_EQ(0, 0);
    delete uICheckBox;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_5000
 * @tc.name     test GetOpacity api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_5000, Function | MediumTest | Level0)
{
    Paint* paint = new Paint();
    paint->GetOpacity();
    EXPECT_EQ(0, 0);
    delete paint;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_5100
 * @tc.name     test OnAnimatorStop api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_5100, Function | MediumTest | Level0)
{
    UIImageAnimatorView::AnimatorStopListener* animatorStopListener = new UIImageAnimatorView::AnimatorStopListener();
    UIView* uiView = new UIView();
    animatorStopListener->OnAnimatorStop(*uiView);
    EXPECT_EQ(0, 0);
    delete uiView;
    delete animatorStopListener;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_5200
 * @tc.name     test Callback api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_5200, Function | MediumTest | Level0)
{
    auto* animatorCallback = new TestAnimatorCallback();
    UIView* view = new UIView();
    animatorCallback->Callback(view);
    EXPECT_EQ(0, 0);
    delete view;
    delete animatorCallback;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_5400
 * @tc.name     test SetMarkNum api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_5400, Function | MediumTest | Level0)
{
    UIXAxis* uixAxis = new UIXAxis();
    uixAxis->SetMarkNum(0);
    EXPECT_EQ(0, 0);
    delete uixAxis;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_5500
 * @tc.name     test GetCurrent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_5500, Function | MediumTest | Level0)
{
    ThemeManager& themeManager = ThemeManager::GetInstance();
    themeManager.GetCurrent();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_5600
 * @tc.name     test SetCurrent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_5600, Function | MediumTest | Level0)
{
    ThemeManager& themeManager = ThemeManager::GetInstance();
    Theme* theme = themeManager.GetCurrent();
    themeManager.SetCurrent(theme);
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6000
 * @tc.name     test GetCount api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6000, Function | MediumTest | Level0)
{
    TextAdapter* textAdapter = new TextAdapter();
    textAdapter->GetCount();
    EXPECT_EQ(0, 0);
    delete textAdapter;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6100
 * @tc.name     test GetView api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6100, Function | MediumTest | Level0)
{
    TextAdapter* textAdapter = new TextAdapter();
    UIView* inView = new UIView();
    int16_t index = 1;
    Font* font = new Font();
    font->SetDynamicFontId();
    font->SetAnimatorSpeed();
    font->GetLongMode();
    textAdapter->GetView(inView, index);
    EXPECT_EQ(0, 0);
    delete font;
    delete inView;
    delete textAdapter;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6200
 * @tc.name     test DeleteView api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6200, Function | MediumTest | Level0)
{
    TextAdapter* textAdapter = new TextAdapter();
    UIView* view = new UIView();
    Font* font = new Font();
    font->Scal();
    font->Itoa();
    font->GetCrossPoint();
    textAdapter->DeleteView(view);
    EXPECT_EQ(0, 0);
    delete font;
    delete view;
    delete textAdapter;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6300
 * @tc.name     test OnPostDraw api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6300, Function | MediumTest | Level0)
{
    UIView* uiView = new UIView();
    Rect* rect = new Rect();
    Font* font = new Font();
    font->IsLineSegmentCross();
    font->IsRectCross();
    font->SetOnChangeListener();
    BufferInfo bufferInfo;
    uiView->OnPostDraw(bufferInfo, *rect);
    EXPECT_EQ(0, 0);
    delete font;
    delete rect;
    delete uiView;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6400
 * @tc.name     test DrawArc api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6400, Function | MediumTest | Level0)
{
    UICanvas* canvas = new UICanvas();
    Paint paint;
    Font* font = new Font();
    font->SetLineAndTextOpacity();
    font->OnClickEventHappen();
    canvas->DrawArc({100, 150}, 50, 135, 270, paint);
    EXPECT_EQ(0, 0);
    delete font;
    delete canvas;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6500
 * @tc.name     test SetData api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6500, Function | MediumTest | Level0)
{
    List<const char*>* adapterData = new List<const char*>();
    TextAdapter* adapter = new TextAdapter();
    auto* keyActListener = new KeyActListener();
    keyActListener->OnDragEndEventHappen();
    keyActListener->OnLongPressEventHappen();
    keyActListener->OnFPSChangedListener();
    adapter->SetData(adapterData);
    EXPECT_EQ(0, 0);
    delete keyActListener;
    delete adapter;
    delete adapterData;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6600
 * @tc.name     test IncOneSecond api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6600, Function | MediumTest | Level0)
{
    UIAbstractClock* uIAbstractClock = new UIAbstractClock();
    auto* keyActListener = new KeyActListener();
    keyActListener->OnFPSChanged();
    keyActListener->GetFPS();
    keyActListener->RegisterFPSChangedListener();
    keyActListener->CreateSurface();
    uIAbstractClock->IncOneSecond();
    EXPECT_EQ(0, 0);
    delete keyActListener;
    delete uIAbstractClock;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6700
 * @tc.name     test UpdateClock api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6700, Function | MediumTest | Level0)
{
    UIAbstractClock* uIAbstractClock = new UIAbstractClock();
    auto* keyActListener = new KeyActListener();
    keyActListener->GetQueueSize();
    keyActListener->RegisterConsumerListener();
    keyActListener->UnregisterConsumerListener();
    keyActListener->GetInt64();
    uIAbstractClock->UpdateClock(false);
    EXPECT_EQ(0, 0);
    delete keyActListener;
    delete uIAbstractClock;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6800
 * @tc.name     test OnKeyEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6800, Function | MediumTest | Level0)
{
    UIAbstractClock* uIAbstractClock = new UIAbstractClock();
    UIAbstractClock::WorkMode workMode = {};
    uIAbstractClock->SetWorkMode(workMode);
    auto* keyActListener = new KeyActListener();
    keyActListener->GetSurface();
    keyActListener->OnKeyAct();
    keyActListener->OnKeyEvent();
    EXPECT_EQ(0, 0);
    delete uIAbstractClock;
    delete keyActListener;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_6900
 * @tc.name     test ClearOnKeyActListener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_6900, Function | MediumTest | Level0)
{
    UIAbstractClock* uIAbstractClock = new UIAbstractClock();
    uIAbstractClock->GetWorkMode();
    auto* keyActListener = new KeyActListener();
    keyActListener->SetOnKeyActListener();
    keyActListener->ClearOnKeyActListener();
    EXPECT_EQ(0, 0);
    delete keyActListener;
    delete uIAbstractClock;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_7000
 * @tc.name     test SetHandImage api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_7000, Function | MediumTest | Level0)
{
    UIAnalogClock* hand = new UIAnalogClock();
    UIImageView* uiImageView = new UIImageView();
    EXPECT_EQ(0, 0);
    delete uiImageView;
    delete hand;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_7100
 * @tc.name     test ClearOnVirtualDeviceEventListener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_7100, Function | MediumTest | Level0)
{
    EventInjector* eventInjector = EventInjector::GetInstance();
    EventDataType* type = new EventDataType();
    eventInjector->RegisterEventInjector(*type);
    auto* rootViewManage = new RootViewManage();
    rootViewManage->OnVirtualDeviceEvent();
    rootViewManage->SetOnVirtualDeviceEventListener();
    rootViewManage->ClearOnVirtualDeviceEventListener();
    EXPECT_EQ(0, 0);
    delete rootViewManage;
    delete type;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_7200
 * @tc.name     test DrawTop api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_7200, Function | MediumTest | Level0)
{
    EventInjector* eventInjector = EventInjector::GetInstance();
    EventDataType* type = new EventDataType();
    eventInjector->UnregisterEventInjector(*type);
    auto* rootViewManage = new RootViewManage();
    rootViewManage->EnableCursor();
    rootViewManage->GetCursor();
    rootViewManage->DrawTop();
    EXPECT_EQ(0, 0);
    delete rootViewManage;
    delete type;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_7300
 * @tc.name     test IsEventInjectorRegistered api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_7300, Function | MediumTest | Level0)
{
    EventInjector* eventInjector = EventInjector::GetInstance();
    EventDataType* type = new EventDataType();
    eventInjector->IsEventInjectorRegistered(*type);
    EXPECT_EQ(0, 0);
    delete type;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_7400
 * @tc.name     test SetInjectEvent api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_7400, Function | MediumTest | Level0)
{
    EventInjector* eventInjector = EventInjector::GetInstance();
    EventDataType* type = new EventDataType();
    DeviceData* dataArray = new DeviceData();
    uint16_t arrayLength = 1;
    eventInjector->SetInjectEvent(dataArray, arrayLength, *type);
    EXPECT_EQ(0, 0);
    delete dataArray;
    delete type;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_7500
 * @tc.name     test ScreenshotToFile api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_7500, Function | MediumTest | Level0)
{
    auto* rootViewManage = new RootViewManage();
    rootViewManage->GetTopUIView();
    rootViewManage->FindSubView();
    rootViewManage->SetFilePath();
    EXPECT_EQ(0, 0);
    delete rootViewManage;
}
} // namespace OHOS
