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
#include "components/ui_axis.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
class UIAxisTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_AXIS_GETVIEWTYPE_0100
 * @tc.name     test axis get-viewtype api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAxisTest, Graphic_UIAxisTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIXAxis *axis = new UIXAxis();
    EXPECT_EQ(axis->GetViewType(), UI_AXIS);
    delete axis;
}

/**
 * @tc.number   SUB_GRAPHIC_AXIS_SETDATARANGE_0200
 * @tc.name     test axis set-data-rangX api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAxisTest, Graphic_UIAxisTest_Test_SetDataRange_0200, Function | MediumTest | Level0)
{
    UIXAxis *axis = new UIXAxis();
    uint16_t min = 10;
    uint16_t max = 100;
    EXPECT_EQ(axis->SetDataRange(min, max), true);
    delete axis;
}

/**
 * @tc.number   SUB_GRAPHIC_AXIS_SETDATARANGE_0300
 * @tc.name     test axis set-data-rangX2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAxisTest, Graphic_UIAxisTest_Test_SetDataRange_0300, Function | MediumTest | Level0)
{
    UIYAxis *axis = new UIYAxis();
    uint16_t min = 100;
    uint16_t max = 10;
    EXPECT_EQ(axis->SetDataRange(min, max), false);
    delete axis;
}

/**
 * @tc.number   SUB_GRAPHIC_AXIS_SETDATARANGE_0400
 * @tc.name     test axis set-data-rangY api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAxisTest, Graphic_UIAxisTest_Test_SetDataRange_0400, Function | MediumTest | Level0)
{
    UIYAxis *axis = new UIYAxis();
    uint16_t min = 10;
    uint16_t max = 100;
    EXPECT_EQ(axis->SetDataRange(min, max), true);
    delete axis;
}

/**
 * @tc.number   SUB_GRAPHIC_AXIS_SETDATARANGE_0500
 * @tc.name     test axis set-data-rangY2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIAxisTest, Graphic_UIAxisTest_Test_SetDataRange_0500, Function | MediumTest | Level0)
{
    UIYAxis *axis = new UIYAxis();
    uint16_t min = 100;
    uint16_t max = 10;
    EXPECT_EQ(axis->SetDataRange(min, max), false);
    delete axis;
}
}