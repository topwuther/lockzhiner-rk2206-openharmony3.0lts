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
#include "components/ui_list.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIListTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void UIListTest::SetUpTestCase()
{
    return;
}

void UIListTest::TearDownTestCase()
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_SETSTARTINDEX_0100
 * @tc.name     test list set-start-index api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIListTest, Graphic_List_Test_SetStartIndex_0100, Function | MediumTest | Level0)
{
    UIList* list = new UIList();
    list->SetStartIndex(1);
    EXPECT_EQ(list->GetStartIndex(), 1);

    int16_t index = 0;
    list->SetStartIndex(index);
    EXPECT_EQ(list->GetStartIndex(), 0);
    delete list;
}

/**
 * @tc.number   SUB_GRAPHIC_LIST_SETLOOPSTATE_0300
 * @tc.name     test list set-loop-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIListTest, Graphic_List_Test_SetLoopState_0300, Function | MediumTest | Level0)
{
    UIList* list = new UIList();
    bool state = false;
    list->SetLoopState(state);
    EXPECT_EQ(list->GetLoopState(), false);
    delete list;
}
} // namespace OHOS