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
#include "components/ui_slider.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UISliderTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_SLIDER_GETVIEWTYPE_0100
 * @tc.name     test slider get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISliderTest, Graphic_UISliderTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UISlider* slider = new UISlider();
    EXPECT_EQ(slider->GetViewType(), UI_SLIDER);
    delete slider;
}

/**
 * @tc.number   SUB_GRAPHIC_SLIDER_SETKNOBWIDTH_0200
 * @tc.name     test slider set-knob-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISliderTest, Graphic_UISliderTest_Test_SetKnobWidth_0200, Function | MediumTest | Level0)
{
    UISlider* slider = new UISlider();
    int16_t width = 10;
    slider->SetKnobWidth(10);
    EXPECT_EQ(slider->GetKnobWidth(), width);
    delete slider;
}

/**
 * @tc.number   SUB_GRAPHIC_SLIDER_SETIMAGE_0400
 * @tc.name     test slider set-image api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UISliderTest, Graphic_UISliderTest_Test_SetImage_0400, Function | MediumTest | Level0)
{
    UISlider* slider = new UISlider();
    ImageInfo* backgroundImage = nullptr;
    ImageInfo* foregroundImage = nullptr;
    slider->SetImage(backgroundImage, foregroundImage);
    EXPECT_EQ(0, 0);
}
}
// namespace OHOS
