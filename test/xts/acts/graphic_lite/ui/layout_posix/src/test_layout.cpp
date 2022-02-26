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

#include "layout/layout.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
class LayoutTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

class LayoutIn : public Layout {
public:
    uint8_t GetLayoutDirection(void) const
    {
        return direction_;
    }
};
/**
 * @tc.number   SUB_GRAPHIC_LAYOUT_SetLayoutDirection_0100
 * @tc.name     test ui_view SetLayoutDirection api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(LayoutTest, SUB_GRAPHIC_FLEXLAOUT_SetLayoutDirection_0100, Function | MediumTest | Level0)
{
    LayoutIn* layout = new LayoutIn();
    uint8_t direction = 1;
    layout->SetLayoutDirection(direction);
    EXPECT_EQ(layout->GetLayoutDirection(), direction);
    delete layout;
}
} // namespace OHOS