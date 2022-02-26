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
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_3200
 * @tc.name     test GetBoxRect api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_3200, Function | MediumTest | Level0)
{
    TransformMap* transformMap = new TransformMap();
    transformMap->GetBoxRect();
    EXPECT_EQ(0, 0);
    delete transformMap;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_3300
 * @tc.name     test GetPolygon api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_3300, Function | MediumTest | Level0)
{
    TransformMap* transformMap = new TransformMap();
    transformMap->GetPolygon();
    EXPECT_EQ(0, 0);
    delete transformMap;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_3400
 * @tc.name     test Translate api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_3400, Function | MediumTest | Level0)
{
    Matrix3<int>* matrix3 = new Matrix3<int>();
    Vector2<int>* vector2 = new Vector2<int>();
    matrix3->Translate(*vector2);
    EXPECT_EQ(0, 0);
    delete vector2;
    delete matrix3;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_3500
 * @tc.name     test IsExtends api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_3500, Function | MediumTest | Level0)
{
    Rect* rect = new Rect();
    rect->IsExtends(*rect);
    EXPECT_EQ(0, 0);
    delete rect;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_3600
 * @tc.name     test ColorTo16 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_3600, Function | MediumTest | Level0)
{
    Color* color = new Color();
    Color32* ret = new Color32();
    color->ColorTo16(*ret);
    EXPECT_EQ(0, 0);
    delete ret;
    delete color;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_3700
 * @tc.name     test ColorTo32 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_3700, Function | MediumTest | Level0)
{
    Color* color = new Color();
    ColorType* colorType = new ColorType();
    color->ColorTo32(*colorType);
    EXPECT_EQ(0, 0);
    delete colorType;
    delete color;
}

/**
 * @tc.number   SUB_GRAPHIC_INTERFACE_FIRST_7600
 * @tc.name     test new api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiInterfaceTest, SUB_GRAPHIC_INTERFACE_FIRST_7600, Function | MediumTest | Level0)
{
    delete(operator new(10));
    EXPECT_EQ(0, 0);
}
} // namespace OHOS
