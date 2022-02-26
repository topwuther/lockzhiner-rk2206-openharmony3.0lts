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
#include "layout/list_layout.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class ListLayoutTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_LISTLAYOUT_SETDIRECTION_0100
 * @tc.name     test list-layout set-direction api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ListLayoutTest, Graphic_ListLayout_Test_SetDirection_0100, Function | MediumTest | Level0)
{
    ListLayout *listLayout = new ListLayout();
    uint8_t direction = 10;
    listLayout->SetDirection(direction);
    EXPECT_EQ(listLayout->GetDirection(), direction);
    delete listLayout;
}
} // namespace OHOS