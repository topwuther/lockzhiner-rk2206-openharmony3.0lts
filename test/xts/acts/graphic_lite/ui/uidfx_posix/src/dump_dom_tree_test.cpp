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

#include "dfx/ui_dump_dom_tree.h"

#include "components/root_view.h"
#include "components/ui_analog_clock.h"
#include "components/ui_arc_label.h"
#include "components/ui_box_progress.h"
#include "components/ui_checkbox.h"
#include "components/ui_circle_progress.h"
#include "components/ui_digital_clock.h"
#include "components/ui_image_view.h"
#include "components/ui_label.h"
#include "components/ui_label_button.h"
#include "components/ui_list.h"
#include "components/ui_picker.h"
#include "components/ui_radio_button.h"
#include "components/ui_scroll_view.h"
#include "components/ui_slider.h"
#include "components/ui_swipe_view.h"
#include "components/ui_time_picker.h"
#include "components/ui_toggle_button.h"
#include "components/ui_view_group.h"
#include "draw/draw_image.h"
#include "draw/draw_utils.h"
#include "gfx_utils/file.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
class UIDumpDomTreeTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0100
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0100, Function | MediumTest | Level0)
{
    UILabel* label = new UILabel();
    label->SetViewId("label");
    // 5: y-coordinate, 100: width, 20: height
    label->SetPosition(0, 5, 100, 20);
    label->SetText("dump");
    RootView::GetInstance()->Add(label);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("label");
    const char standard[] = "{\n\t\"name\":\t\"UILabel\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"label\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\tfalse,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"text\":\t\"dump\"\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete label;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0200
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0200, Function | MediumTest | Level0)
{
    UIArcLabel* arcLabel = new UIArcLabel();
    arcLabel->SetViewId("arc_label");
    // 5: y-coordinate, 100: width, 20: height
    arcLabel->SetPosition(0, 5, 100, 20);
    arcLabel->SetText("dump");
    RootView::GetInstance()->Add(arcLabel);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("arc_label");
    const char standard[] = "{\n\t\"name\":\t\"UIArcLabel\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"arc_label\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\tfalse,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"text\":\t\"dump\"\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete arcLabel;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0300
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0300, Function | MediumTest | Level0)
{
    UILabelButton* labelButton = new UILabelButton();
    labelButton->SetViewId("label_button");
    // 5: y-coordinate, 100: width, 20: height
    labelButton->SetPosition(0, 5, 100, 20);
    labelButton->SetText("dump");
    RootView::GetInstance()->Add(labelButton);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("label_button");
    const char standard[] = "{\n\t\"name\":\t\"UILabelButton\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"label_button\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\ttrue,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"text\":\t\"dump\"\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete labelButton;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0400
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0400, Function | MediumTest | Level0)
{
    UICheckBox* checkBox = new UICheckBox();
    checkBox->SetViewId("checkbox");
    // 5: y-coordinate, 100: width, 20: height
    checkBox->SetPosition(0, 5, 100, 20);
    checkBox->SetState(UICheckBox::SELECTED);
    RootView::GetInstance()->Add(checkBox);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("checkbox");
    const char standard[] = "{\n\t\"name\":\t\"UICheckBox\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"checkbox\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\ttrue,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"state\":\t\"SELECTED\"\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete checkBox;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0500
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0500, Function | MediumTest | Level0)
{
    UIRadioButton* radioButton = new UIRadioButton();
    radioButton->SetViewId("radio_button");
    // 5: y-coordinate, 100: width, 20: height
    radioButton->SetPosition(0, 5, 100, 20);
    radioButton->SetState(UICheckBox::SELECTED);
    RootView::GetInstance()->Add(radioButton);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("radio_button");
    const char standard[] = "{\n\t\"name\":\t\"UIRadioButton\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"radio_button\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\ttrue,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"state\":\t\"SELECTED\"\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete radioButton;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0600
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0600, Function | MediumTest | Level0)
{
    UIToggleButton* toggleButton = new UIToggleButton();
    toggleButton->SetViewId("toggle_button");
    // 5: y-coordinate, 100: width, 20: height
    toggleButton->SetPosition(0, 5, 100, 20);
    toggleButton->SetState(false);
    RootView::GetInstance()->Add(toggleButton);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("toggle_button");
    const char standard[] = "{\n\t\"name\":\t\"UIToggleButton\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"toggle_button\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\ttrue,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"state\":\tfalse\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete toggleButton;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0700
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0700, Function | MediumTest | Level0)
{
    UIImageView* imageView = new UIImageView();
    imageView->SetViewId("image");
    imageView->SetSrc("..\\config\\images\\A020_046_rgb888.bin");
    // 5: y-coordinate, 100: width, 100: height
    imageView->SetPosition(0, 5, 100, 100);
    RootView::GetInstance()->Add(imageView);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("image");
    const char standard[] = "{\n\t\"name\":\t\"UIImageView\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t100,"\
        "\n\t\"height\":\t100,\n\t\"id\":\t\"image\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\tfalse,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"src\":\t\"..\\\\config\\\\images\\\\A020_046_rgb888.bin\"\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete imageView;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0800
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0800, Function | MediumTest | Level0)
{
    UIImageView* imageView = new UIImageView();
    imageView->SetViewId("image");
    ImageInfo* imageInfo = new ImageInfo();
    uint32_t a = 32;
    imageInfo->userData = reinterpret_cast<void*>(&a);
    imageView->SetSrc(static_cast<const ImageInfo*>(imageInfo));
    // 5: y-coordinate, 100: width, 100: height
    imageView->SetPosition(0, 5, 100, 100);
    RootView::GetInstance()->Add(imageView);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("image");
    const char standard[] = "{\n\t\"name\":\t\"UIImageView\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t100,"\
        "\n\t\"height\":\t100,\n\t\"id\":\t\"image\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\tfalse,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"src\":\t32\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete imageInfo;
    delete imageView;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0900
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_0900, Function | MediumTest | Level0)
{
    UIBoxProgress* boxProgress = new UIBoxProgress();
    boxProgress->SetViewId("box_progress");
    // 5: y-coordinate, 100: width, 20: height
    boxProgress->SetPosition(0, 5, 100, 20);
    boxProgress->SetRange(100, 0); // 100: max range
    boxProgress->SetValue(20); // 20: value
    RootView::GetInstance()->Add(boxProgress);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("box_progress");
    const char standard[] = "{\n\t\"name\":\t\"UIBoxProgress\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"box_progress\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\tfalse,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"currValue\":\t20,\n\t\"rangeMin\":\t0,\n\t\"rangeMax\":\t100\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete boxProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1000
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1000, Function | MediumTest | Level0)
{
    UISlider* slider = new UISlider();
    slider->SetViewId("slider");
    // 5: y-coordinate, 100: width, 20: height
    slider->SetPosition(0, 5, 100, 20);
    slider->SetRange(100, 0); // 100: max range
    slider->SetValue(20); // 20: value
    RootView::GetInstance()->Add(slider);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("slider");
    const char standard[] = "{\n\t\"name\":\t\"UISlider\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"slider\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\ttrue,"\
        "\n\t\"draggable\":\ttrue,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"currValue\":\t20,\n\t\"rangeMin\":\t0,\n\t\"rangeMax\":\t100\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete slider;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1100
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1100, Function | MediumTest | Level0)
{
    UICircleProgress* circleProgress = new UICircleProgress();
    circleProgress->SetViewId("circle_progress");
    // 5: y-coordinate, 100: width, 20: height
    circleProgress->SetPosition(0, 5, 100, 20);
    circleProgress->SetRange(100, 0); // 100: max range
    circleProgress->SetValue(20); // 20: value
    RootView::GetInstance()->Add(circleProgress);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("circle_progress");
    const char standard[] = "{\n\t\"name\":\t\"UICircleProgress\",\n\t\"x\":\t0,\n\t\"y\":\t5,"\
        "\n\t\"width\":\t454,\n\t\"height\":\t20,\n\t\"id\":\t\"circle_progress\",\n\t\"visiable\":\ttrue,"\
        "\n\t\"touchable\":\tfalse,\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,"\
        "\n\t\"onDragListener\":\tfalse,\n\t\"onLongPressListener\":\tfalse,\n\t\"currValue\":\t20,"\
        "\n\t\"rangeMin\":\t0,\n\t\"rangeMax\":\t100\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete circleProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1200
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1200, Function | MediumTest | Level0)
{
    UIScrollView* scrollView = new UIScrollView();
    scrollView->SetViewId("scroll_view");
    // 5: y-coordinate, 100: width, 20: height
    scrollView->SetPosition(0, 5, 100, 20);
    scrollView->SetHorizontalScrollState(false);
    scrollView->SetVerticalScrollState(true);
    RootView::GetInstance()->Add(scrollView);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("scroll_view");
    const char standard[] = "{\n\t\"name\":\t\"UIScrollView\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"scroll_view\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\ttrue,"\
        "\n\t\"draggable\":\ttrue,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"xScrollable\":\tfalse,\n\t\"yScrollable\":\ttrue\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete scrollView;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1300
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1300, Function | MediumTest | Level0)
{
    UIList* list = new UIList();
    list->SetViewId("list");
    // 5: y-coordinate, 100: width, 20: height
    list->SetPosition(0, 5, 100, 20);
    list->SetLoopState(true);
    RootView::GetInstance()->Add(list);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("list");
    const char standard[] = "{\n\t\"name\":\t\"UIList\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"list\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\ttrue,"\
        "\n\t\"draggable\":\ttrue,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"isLoopList\":\ttrue\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1400
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1400, Function | MediumTest | Level0)
{
    UIDigitalClock* digitalClock = new UIDigitalClock();
    digitalClock->SetViewId("digital_clock");
    // 5: y-coordinate, 100: width, 20: height
    digitalClock->SetPosition(0, 5, 100, 20);
    // 14: hour, 30: minute, 30: second
    digitalClock->SetTime24Hour(14, 30, 30);
    RootView::GetInstance()->Add(digitalClock);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("digital_clock");
    const char standard[] = "{\n\t\"name\":\t\"UIDigitalClock\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"digital_clock\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\tfalse,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"currentHour\":\t14,\n\t\"currentMinute\":\t30,"\
        "\n\t\"currentSecond\":\t30\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete digitalClock;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1500
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1500, Function | MediumTest | Level0)
{
    UIAnalogClock* analogClock = new UIAnalogClock();
    analogClock->SetViewId("analog_clock");
    // 5: y Indicates, 100: width, 20: height
    analogClock->SetPosition(0, 5, 100, 20);
    // 14: hour, 30: minute, 30: second
    analogClock->SetTime24Hour(14, 30, 30);
    RootView::GetInstance()->Add(analogClock);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("analog_clock");
    const char standard[] = "{\n\t\"name\":\t\"UIAnalogClock\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"analog_clock\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\ttrue,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"currentHour\":\t14,\n\t\"currentMinute\":\t30,"\
        "\n\t\"currentSecond\":\t30\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete analogClock;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1600
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1600, Function | MediumTest | Level0)
{
    UIPicker* picker = new UIPicker();
    picker->SetViewId("picker");
    // 5: y Indicates, 100: width, 20: height
    picker->SetPosition(0, 5, 100, 20);
    picker->SetItemHeight(10); // 10: height of item
    picker->SetValues(1, 100); // 1: start integer, 100: end integer
    picker->SetSelected(30); // 30: index
    RootView::GetInstance()->Add(picker);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("picker");
    const char standard[] = "{\n\t\"name\":\t\"UIPicker\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"picker\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\tfalse,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"selectedIndex\":\t30\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete picker;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1700
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1700, Function | MediumTest | Level0)
{
    UISwipeView* swipeView = new UISwipeView();
    swipeView->SetViewId("swipe_view");
    // 5: y Indicates, 100: width, 20: height
    swipeView->SetPosition(0, 5, 100, 20);
    UILabel* view1 = new UILabel();
    UILabel* view2 = new UILabel();
    UILabel* view3 = new UILabel();
    swipeView->Add(view1);
    swipeView->Add(view2);
    swipeView->Add(view3);
    swipeView->SetCurrentPage(2, false); // 2: index of a view
    swipeView->SetDirection(UISwipeView::VERTICAL);
    RootView::GetInstance()->Add(swipeView);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("swipe_view");
    const char standard[] = "{\n\t\"name\":\t\"UISwipeView\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"swipe_view\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\ttrue,"\
        "\n\t\"draggable\":\ttrue,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"currentIndex\":\t2,\n\t\"direction\":\t1\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    swipeView->RemoveAll();
    delete view1;
    delete view2;
    delete view3;
    delete swipeView;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1800
 * @tc.name     test dumpdomtree dumpdomnode api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMNODE_1800, Function | MediumTest | Level0)
{
    UITimePicker* timePicker = new UITimePicker();
    timePicker->SetViewId("time_picker");
    // 5: y Indicates, 100: width, 20: height
    timePicker->SetPosition(0, 5, 100, 20);
    timePicker->EnableSecond(true);
    RootView::GetInstance()->Add(timePicker);
    char* tmpStr = UIDumpDomTree::GetInstance()->DumpDomNode("time_picker");
    const char standard[] = "{\n\t\"name\":\t\"UITimePicker\",\n\t\"x\":\t0,\n\t\"y\":\t5,\n\t\"width\":\t454,"\
        "\n\t\"height\":\t20,\n\t\"id\":\t\"time_picker\",\n\t\"visiable\":\ttrue,\n\t\"touchable\":\tfalse,"\
        "\n\t\"draggable\":\tfalse,\n\t\"onClickListener\":\tfalse,\n\t\"onDragListener\":\tfalse,"\
        "\n\t\"onLongPressListener\":\tfalse,\n\t\"selectedHour\":\t\"\",\n\t\"selectedMinute\":\t\"\","\
        "\n\t\"selectedSecond\":\t\"\"\n}";
    EXPECT_EQ(strcmp(tmpStr, standard), 0);
    if (tmpStr != nullptr) {
        free(tmpStr);
    }
    RootView::GetInstance()->RemoveAll();
    delete timePicker;
}

/**
 * @tc.number   SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMTREE_0100
 * @tc.name     test dumpdomtree dumpdomtree api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIDumpDomTreeTest, SUB_GRAPHIC_DUMPDOMTREE_DUMPDOMTREE_0100, Function | MediumTest | Level0)
{
    UIViewGroup* viewGroup = new UIViewGroup();
    viewGroup->SetViewId("view_group");
    viewGroup->SetPosition(0, 30, 100, 100); // 30: y Indicates, 100: width, 100: height

    UILabel* label = new UILabel();
    viewGroup->Add(label);
    label->SetPosition(0, 5, 100, 20); // 5: y Indicates, 100: width, 20: height
    label->SetText("label");

    UICheckBox* checkBox = new UICheckBox();
    checkBox->SetPosition(0, 30, 100, 30); // 30: y Indicates, 100: width, 30: height
    checkBox->SetState(UICheckBox::SELECTED);
    viewGroup->Add(checkBox);

    UIImageView* imageView = new UIImageView();
    imageView->SetSrc("..\\config\\images\\A020_046_rgb888.bin");
    imageView->SetPosition(0, 100, 100, 100); // 100: y Indicates, 100: width, 100: height
    RootView::GetInstance()->Add(viewGroup);
    RootView::GetInstance()->Add(imageView);

    EXPECT_EQ(UIDumpDomTree::GetInstance()->DumpDomTree("view_group"), true);
    const char standard1[] = "{\r\n\t\"name\":\t\"UIViewGroup\",\r\n\t\"x\":\t0,\r\n\t\"y\":\t30,"\
        "\r\n\t\"width\":\t454,\r\n\t\"height\":\t100,\r\n\t\"id\":\t\"view_group\",\r\n\t\"visiable\":\ttrue,"\
        "\r\n\t\"touchable\":\tfalse,\r\n\t\"draggable\":\tfalse,\r\n\t\"onClickListener\":\tfalse,"\
        "\r\n\t\"onDragListener\":\tfalse,\r\n\t\"onLongPressListener\":\tfalse,\r\n\t\"child\":\t[{"\
        "\r\n\t\t\t\"name\":\t\"UILabel\",\r\n\t\t\t\"x\":\t0,\r\n\t\t\t\"y\":\t35,\r\n\t\t\t\"width\":\t454,"\
        "\r\n\t\t\t\"height\":\t20,\r\n\t\t\t\"visiable\":\ttrue,\r\n\t\t\t\"touchable\":\tfalse,"\
        "\r\n\t\t\t\"draggable\":\tfalse,\r\n\t\t\t\"onClickListener\":\tfalse,"\
        "\r\n\t\t\t\"onDragListener\":\tfalse,\r\n\t\t\t\"onLongPressListener\":\tfalse,"\
        "\r\n\t\t\t\"text\":\t\"label\"\r\n\t\t}, {\r\n\t\t\t\"name\":\t\"UICheckBox\",\r\n\t\t\t\"x\":\t0,"\
        "\r\n\t\t\t\"y\":\t60,\r\n\t\t\t\"width\":\t454,\r\n\t\t\t\"height\":\t30,\r\n\t\t\t\"visiable\":\ttrue,"\
        "\r\n\t\t\t\"touchable\":\ttrue,\r\n\t\t\t\"draggable\":\tfalse,\r\n\t\t\t\"onClickListener\":\tfalse,"\
        "\r\n\t\t\t\"onDragListener\":\tfalse,\r\n\t\t\t\"onLongPressListener\":\tfalse,"\
        "\r\n\t\t\t\"state\":\t\"SELECTED\"\r\n\t\t}]\r\n}";

    int32_t fd1 = FileOpen(DEFAULT_DUMP_DOM_TREE_PATH, _O_RDONLY | _O_U16TEXT);
    if (fd1 > 0) {
        char tmp1[825];
        int num1 = FileRead(fd1, tmp1, 824); // 824: count of buffer
        tmp1[824] = '\0'; // 824: array index
        EXPECT_EQ(strcmp(tmp1, standard1), 0);
        EXPECT_EQ(UIDumpDomTree::GetInstance()->DumpDomTree(nullptr), true);
    }
    RootView::GetInstance()->RemoveAll();
    delete viewGroup;
    delete label;
    delete checkBox;
    delete imageView;
}
} // OHOS