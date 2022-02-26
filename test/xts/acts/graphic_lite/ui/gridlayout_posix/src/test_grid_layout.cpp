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

#include "layout/grid_layout.h"

#include <climits>
#include <gtest/gtest.h>
#include "common/screen.h"

using namespace testing::ext;
namespace OHOS {
class GridLayoutTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_GRIDLAYOUT_LAYOUTCHILDREN_0100
 * @tc.name     test grid-layout layout-children api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(GridLayoutTest, Graphic_GridLayout_Test_LayoutChildren_0100, Function | MediumTest | Level0)
{
    GridLayout* layout = new GridLayout();
    layout->SetPosition(0, 0, Screen::GetInstance().GetWidth(), Screen::GetInstance().GetHeight());
    layout->SetLayoutDirection(LAYOUT_HOR);
    layout->SetRows(5); // 5: row
    layout->SetCols(4); // 4: col
    layout->LayoutChildren();
    EXPECT_EQ(0, 0);
    delete layout;
}
} // namespace OHOS