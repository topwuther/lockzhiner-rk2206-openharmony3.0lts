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
#include "components/ui_texture_mapper.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UITextureMapperTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_START_0100
 * @tc.name     test UItexture-mapper start api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_Start_0100, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    testObj->Start();
    EXPECT_EQ(0, 0);
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_SETROTATESTART_0200
 * @tc.name     test UItexture-mapper set-rotate-start api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_SetRotateStart_0200, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    int16_t start = 100;
    testObj->SetRotateStart(start);
    EXPECT_EQ(0, 0);
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_SETROTATEEND_0300
 * @tc.name     test UItexture-mapper set-rotate-end api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_SetRotateEnd_0300, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    int16_t end = 100;
    testObj->SetRotateEnd(end);
    EXPECT_EQ(0, 0);
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_SETSCALESTART_0400
 * @tc.name     test UItexture-mapper set-scale-start api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_SetScaleStart_0400, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    float start = 100.0;
    testObj->SetScaleStart(start);
    EXPECT_EQ(0, 0);
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_SETSCALEEND_0500
 * @tc.name     test UItexture-mapper set-scale-end api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_SetScaleEnd_0500, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    float end = 100.0;
    testObj->SetScaleEnd(end);
    EXPECT_EQ(0, 0);
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_SETDURATIONTIME_0600
 * @tc.name     test UItexture-mapper set-duration-time api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_SetDurationTime_0600, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    uint16_t durationTime = 100;
    testObj->SetDurationTime(durationTime);
    EXPECT_EQ(0, 0);
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_SETDELAYTIME_0700
 * @tc.name     test UItexture-mapper set-delay-time api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_SetDelayTime_0700, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    uint16_t delayTime = 100;
    testObj->SetDelayTime(delayTime);
    EXPECT_EQ(0, 0);
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_SETEASINGFUNC_0800
 * @tc.name     test UItexture-mapper set-easing-func api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_SetEasingFunc_0800, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    EasingFunc* easingFunc = new EasingFunc();
    testObj->SetEasingFunc(*easingFunc);
    EXPECT_EQ(0, 0);
    delete easingFunc;
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_CANCEL_0900
 * @tc.name     test UItexture-mapper cancel api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_Cancel_0900, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    testObj->Cancel();
    EXPECT_EQ(0, 0);
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_RESET_1000
 * @tc.name     test UItexture-mapper reset api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_Reset_1000, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    testObj->Reset();
    EXPECT_EQ(0, 0);
    delete testObj;
}

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_SETPIVOT_1100
 * @tc.name     test UItexture-mapper set-pivot api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_SetPivot_1100, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    int16_t xInt = 10;
    int16_t yInt = 20;
    testObj->SetPivot(xInt, yInt);
    EXPECT_EQ(0, 0);
    delete testObj;
}

class MyTest : public UITextureMapper::AnimatorStopListener {
public:
    MyTest(){};
    ~MyTest(){};
    void OnAnimatorStop(UIView& view) {}
};

/**
 * @tc.number   SUB_GRAPHIC_UITEXTUREMAPPER_SETANIMATORSTOPLISTENER_1200
 * @tc.name     test UItexture-mapper set-animator-stop-listener api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UITextureMapperTest, Graphic_UITextureMapper_Test_SetAnimatorStopListener_1200, Function | MediumTest | Level0)
{
    UITextureMapper* testObj = new UITextureMapper();
    auto* listener = new MyTest();
    testObj->SetAnimatorStopListener(listener);
    EXPECT_EQ(0, 0);
    delete listener;
    delete testObj;
}
} // namespace OHOS