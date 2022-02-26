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
#include "components/ui_image_animator.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIImageAnimatorViewTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};
static ImageAnimatorInfo g_imageAnimatorInfo[4] = {
    { "..\\config\\images\\A021_001.bin", {50, 50},  116, 116, IMG_SRC_FILE_PATH},
    { "..\\config\\images\\A021_002.bin", {50, 50},  116, 116, IMG_SRC_FILE_PATH },
    { "..\\config\\images\\A021_003.bin", {50, 50},  116, 116, IMG_SRC_FILE_PATH },
    { "..\\config\\images\\A021_004.bin", {50, 50},  116, 116, IMG_SRC_FILE_PATH },
};

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_GETVIEWTYPE_0100
 * @tc.name     test image-animator get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */

HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_0100,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    EXPECT_EQ(imageAnimator->GetViewType(), UI_IMAGE_ANIMATOR_VIEW);
    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SETTIMEOFPAUSE_0200
 * @tc.name     test image-animator set-time-of-pause api
 * @tc.desc     [C- SOFTWARE -0200]
 */

HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_0200,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    uint16_t timeOfPause = 1;
    imageAnimator->SetTimeOfPause(timeOfPause);
    EXPECT_EQ(imageAnimator->GetTimeOfPause(), timeOfPause);
    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SETTIMEOFUPDATE_0300
 * @tc.name     test image-animator set-time-of-update api
 * @tc.desc     [C- SOFTWARE -0200]
 */

HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_0300,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    uint16_t time = 300;
    imageAnimator->SetTimeOfUpdate(time);

    EXPECT_EQ(imageAnimator->GetTimeOfUpdate(), time);
    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SETIMAGEANIMATORSRC_0400
 * @tc.name     test image-animator set-image-animator-src api
 * @tc.desc     [C- SOFTWARE -0200]
 */

HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_0400,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    uint8_t imageNum = 4;
    imageAnimator->SetImageAnimatorSrc(g_imageAnimatorInfo, imageNum);

    EXPECT_EQ(imageAnimator->GetImageAnimatorImageNum(), imageNum);
    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SETIMAGEANIMATORSRC_0500
 * @tc.name     test image-animator set-image-animator-src2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */

HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_0500,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    ImageAnimatorInfo* info = &g_imageAnimatorInfo[0];
    imageAnimator->SetImageAnimatorSrc(g_imageAnimatorInfo, 4);

    EXPECT_EQ(imageAnimator->GetImageAnimatorSrc(), info);
    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SETSIZEFIXED_0600
 * @tc.name     test image-animator set-size-fixed api
 * @tc.desc     [C- SOFTWARE -0200]
 */

HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_0600,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    bool fixed = true;
    imageAnimator->SetSizeFixed(fixed);

    EXPECT_EQ(imageAnimator->IsSizeFixed(), fixed);
    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SETREPEAT_0700
 * @tc.name     test image-animator set-repeat api
 * @tc.desc     [C- SOFTWARE -0200]
 */

HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_0700,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    bool repeat = true;
    imageAnimator->SetRepeat(repeat);

    EXPECT_EQ(imageAnimator->IsRepeat(), repeat);
    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SETREVERSE_0800
 * @tc.name     test image-animator set-reverse api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_0800,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    bool reverse = true;
    imageAnimator->SetReverse(reverse);

    EXPECT_EQ(imageAnimator->IsReverse(), reverse);
    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SETSTARTPOSITION_0900
 * @tc.name     test image-animator set-start-position api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_0900,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    uint8_t state = Animator::STOP;
    EXPECT_EQ(imageAnimator->GetState(), state);

    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_START_1000
 * @tc.name     test image-animator start api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_1000,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    imageAnimator->Start();
    EXPECT_EQ(imageAnimator->GetState(), Animator::START);

    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_STOP_1100
 * @tc.name     test image-animator stop api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_1100,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    imageAnimator->Stop();
    EXPECT_EQ(imageAnimator->GetState(), Animator::STOP);

    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_PAUSE_1200
 * @tc.name     test image-animator pause api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_1200,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    imageAnimator->Pause();
    EXPECT_EQ(imageAnimator->GetState(), Animator::PAUSE);

    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_RESUME_1300
 * @tc.name     test image-animator resume api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_1300,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    imageAnimator->Resume();
    EXPECT_EQ(imageAnimator->GetState(), Animator::START);

    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SETREPEATTIMES_1400
 * @tc.name     test image-animator setrepeattimes api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_1400,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    uint32_t times = 5;
    imageAnimator->SetRepeatTimes(times);
    EXPECT_EQ(imageAnimator->GetRepeatTimes(), times);

    delete imageAnimator;
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGEANIMATOR_SetAnimatorStopListener_1500
 * @tc.name     test image-animator SetAnimatorStopListener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIImageAnimatorViewTest,
    Graphic_UIImageAnimatorViewTest_Test_SetAnimatorStopListener_1500,
    Function | MediumTest | Level0)
{
    UIImageAnimatorView* imageAnimator = new UIImageAnimatorView();
    imageAnimator->SetAnimatorStopListener(nullptr);
    EXPECT_EQ(0, 0);

    delete imageAnimator;
}
} // namespace OHOS