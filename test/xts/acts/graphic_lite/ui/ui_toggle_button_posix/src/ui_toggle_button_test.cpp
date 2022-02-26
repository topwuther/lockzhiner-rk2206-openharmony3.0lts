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
#include "components/ui_toggle_button.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIToggleButtonTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_TOGGLEBUTTON_GETVIEWTYPE_0100
 * @tc.name     test toggle-button get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIToggleButtonTest, Graphic_UIToggleButtonTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIToggleButton* toggleBtn = new UIToggleButton();
    if (toggleBtn != nullptr) {
        EXPECT_EQ(toggleBtn->GetViewType(), UI_TOGGLE_BUTTON);
        if (toggleBtn != nullptr) {
            delete toggleBtn;
        }
    }
}

/**
 * @tc.number   SUB_GRAPHIC_TOGGLEBUTTON_SETSTATE_0200
 * @tc.name     test toggle-button set-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIToggleButtonTest, Graphic_UIToggleButtonTest_Test_SetState_0200, Function | MediumTest | Level0)
{
    UIToggleButton* toggleBtn = new UIToggleButton();
    if (toggleBtn != nullptr) {
        toggleBtn->SetState(true);
        EXPECT_EQ(toggleBtn->GetState(), true);
        if (toggleBtn != nullptr) {
            delete toggleBtn;
        }
    }
}
} // namespace OHOS