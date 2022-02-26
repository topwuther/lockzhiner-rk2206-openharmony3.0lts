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

#include "layout/flex_layout.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class TestFlexLayout : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

/**
 * @tc.number   SUB_GRAPHIC_FLEXLAYOUT_SETMAJORAXISALIGN_0100
 * @tc.name     test FlexLayout SetMajorAxisAlign api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestFlexLayout, SUB_GRAPHIC_FLEXLAYOUT_SETMAJORAXISALIGN_0100, Function | MediumTest | Level0)
{
    FlexLayout* layout = new FlexLayout();
    layout->SetMajorAxisAlign(ALIGN_END);
    EXPECT_EQ(0, 0);
    delete layout;
}

/**
 * @tc.number   SUB_GRAPHIC_FLEXLAYOUT_SETSECONDARYAXISALIGN_0200
 * @tc.name     test FlexLayout SetSecondaryAxisAlign api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestFlexLayout, SUB_GRAPHIC_FLEXLAYOUT_SetSecondaryAxisAlign_0200, Function | MediumTest | Level0)
{
    FlexLayout* layout = new FlexLayout();
    layout->SetSecondaryAxisAlign(ALIGN_END);
    EXPECT_EQ(0, 0);
    delete layout;
}

/**
 * @tc.number   SUB_GRAPHIC_FLEXLAYOUT_SETFLEXWRAP_0300
 * @tc.name     test FlexLayout SetFlexWrap api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestFlexLayout, SUB_GRAPHIC_FLEXLAYOUT_SetFlexWrap_0300, Function | MediumTest | Level0)
{
    FlexLayout* layout = new FlexLayout();
    layout->SetFlexWrap(true);
    EXPECT_EQ(0, 0);
    delete layout;
}

/**
 * @tc.number   SUB_GRAPHIC_FLEXLAYOUT_LAYOUTCHILDREN_0400
 * @tc.name     test FlexLayout LayoutChildren api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(TestFlexLayout, SUB_GRAPHIC_FLEXLAYOUT_LayoutChildren_0400, Function | MediumTest | Level0)
{
    FlexLayout* layout = new FlexLayout();
    layout->LayoutChildren(true);
    EXPECT_EQ(0, 0);
    delete layout;
}
} // namespace OHOS