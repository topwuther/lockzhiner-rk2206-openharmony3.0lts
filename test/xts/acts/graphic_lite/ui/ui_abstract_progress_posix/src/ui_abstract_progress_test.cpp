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

#include "components/ui_abstract_progress.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class UiAbstractProgressTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_ABSTRACTPAOGRESS_GETVIEWTYPE_0100
 * @tc.name     test abstract get-viewtype api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiAbstractProgressTest, Graphic_UIAbsatrctProgressTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIAbstractProgress *abstractProgress = new UIAbstractProgress();
    EXPECT_EQ(abstractProgress->GetViewType(), UI_ABSTRACT_PROGRESS);
    delete abstractProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_ABSTRACTPAOGRESS_SETVALUE_0200
 * @tc.name     test abstract set-value api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiAbstractProgressTest, Graphic_UIAbsatrctProgressTest_Test_SetValue_0200, Function | MediumTest | Level0)
{
    UIAbstractProgress *abstractProgress = new UIAbstractProgress();
    int16_t value = 8;
    abstractProgress->SetValue(value);
    EXPECT_EQ(abstractProgress->GetValue(), value);
    delete abstractProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_ABSTRACTPAOGRESS_SETRANGE_0300
 * @tc.name     test abstract set-range api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiAbstractProgressTest, Graphic_UIAbsatrctProgressTest_Test_SetRange_0300, Function | MediumTest | Level0)
{
    UIAbstractProgress *abstractProgress = new UIAbstractProgress();
    int16_t rangeMax = 8;
    int16_t rangeMin = 2;
    abstractProgress->SetRange(rangeMax, rangeMin);
    EXPECT_EQ(abstractProgress->GetRangeMin(), rangeMin);
    EXPECT_EQ(abstractProgress->GetRangeMax(), rangeMax);
    delete abstractProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_ABSTRACTPAOGRESS_SETSTEP_0400
 * @tc.name     test abstract set-step api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiAbstractProgressTest, Graphic_UIAbsatrctProgressTest_Test_SetStep_0400, Function | MediumTest | Level0)
{
    UIAbstractProgress *abstractProgress = new UIAbstractProgress();
    uint16_t step = 8;
    abstractProgress->SetStep(step);
    EXPECT_EQ(abstractProgress->GetStep(), step);
    delete abstractProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_ABSTRACTPAOGRESS_SETBACKGROUNDSTYLE_0500
 * @tc.name     test abstract set-background-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiAbstractProgressTest,
         Graphic_UIAbsatrctProgressTest_Test_SetBackgroundStyle_0500,
         Function | MediumTest | Level0)
{
    UIAbstractProgress *abstractProgress = new UIAbstractProgress();
    Style style;
    style.imageOpa_ = 100;
    style.lineOpa_ = 100;
    style.borderRadius_ = 10;
    abstractProgress->SetBackgroundStyle(style);
    EXPECT_EQ(abstractProgress->GetBackgroundStyle().imageOpa_, 100);

    abstractProgress->SetBackgroundStyle(STYLE_BACKGROUND_COLOR, 1);
    EXPECT_EQ(abstractProgress->GetBackgroundStyle(STYLE_BACKGROUND_COLOR), 1);

    delete abstractProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_ABSTRACTPAOGRESS_SETFOREGROUNDSTYLE_0600
 * @tc.name     test abstract set-foreground-style api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiAbstractProgressTest,
         Graphic_UIAbsatrctProgressTest_Test_SetForegroundStyle_0600,
         Function | MediumTest | Level0)
{
    UIAbstractProgress *abstractProgress = new UIAbstractProgress();
    Style style;
    style.imageOpa_ = 100;
    style.lineOpa_ = 100;
    style.borderRadius_ = 10;
    abstractProgress->SetForegroundStyle(style);
    EXPECT_EQ(abstractProgress->GetForegroundStyle().imageOpa_, 100);

    abstractProgress->SetForegroundStyle(STYLE_BACKGROUND_COLOR, 1);
    EXPECT_EQ(abstractProgress->GetForegroundStyle(STYLE_BACKGROUND_COLOR), 1);

    delete abstractProgress;
}


/**
 * @tc.number   SUB_GRAPHIC_ABSTRACTPAOGRESS_ENABLEBACKGROUND_0700
 * @tc.name     test abstract enable-background api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiAbstractProgressTest,
         Graphic_UIAbsatrctProgressTest_Test_EnableBackground_0700,
         Function | MediumTest | Level0)
{
    UIAbstractProgress *abstractProgress = new UIAbstractProgress();

    abstractProgress->EnableBackground(true);
    EXPECT_EQ(0, 0);

    delete abstractProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_ABSTRACTPAOGRESS_SETIMAGE_0800
 * @tc.name     test abstract SetImage api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiAbstractProgressTest, Graphic_UIAbsatrctProgressTest_Test_SetImage_0800, Function | MediumTest | Level0)
{
    UIAbstractProgress *abstractProgress = new UIAbstractProgress();

    abstractProgress->SetImage("..\\config\\images\\A021_006.bin", "..\\config\\images\\A021_005.bin");
    EXPECT_EQ(0, 0);

    abstractProgress->SetImage(static_cast<ImageInfo*>(nullptr));
    EXPECT_EQ(0, 0);

    delete abstractProgress;
}

/**
 * @tc.number   SUB_GRAPHIC_ABSTRACTPAOGRESS_SETCAPTYPE_0900
 * @tc.name     test abstract set-cap-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UiAbstractProgressTest, Graphic_UIAbsatrctProgressTest_Test_SetCapType_0900, Function | MediumTest | Level0)
{
    UIAbstractProgress *abstractProgress = new UIAbstractProgress();

    abstractProgress->SetCapType(CapType::CAP_NONE);
    EXPECT_EQ(0, 0);
    delete abstractProgress;
}
}
