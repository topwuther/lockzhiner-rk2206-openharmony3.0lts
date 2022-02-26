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
#include "components/ui_checkbox.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
class UICheckboxTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_CHECKBOX_SETSTATE_0100
 * @tc.name     test check-box set-state api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICheckboxTest, Graphic_UICheckboxTest_Test_SetState_0100, Function | MediumTest | Level0)
{
    UICheckBox *box = new UICheckBox();
    EXPECT_EQ(box->GetViewType(), UI_CHECK_BOX);
    delete(box);
}

/**
 * @tc.number   SUB_GRAPHIC_CHECKBOX_ONCLICKEVENT_0200
 * @tc.name     test check-box on-click-event api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICheckboxTest, Graphic_UICheckboxTest_Test_OnClickEvent_0200, Function | MediumTest | Level0)
{
    UICheckBox *box = new UICheckBox();
    ClickEvent event({0, 0});
    box->OnClickEvent(event);
    EXPECT_EQ(box->GetState(), UICheckBox::SELECTED);
    delete box;
}

/**
 * @tc.number   SUB_GRAPHIC_CHECKBOX_SETSTATE_0300
 * @tc.name     test check-box set-state2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICheckboxTest, Graphic_UICheckboxTest_Test_SetState_0300, Function | MediumTest | Level0)
{
    UICheckBox *box = new UICheckBox();
    box->SetState(UICheckBox::UICheckBoxState::SELECTED);
    EXPECT_EQ(box->GetState(), UICheckBox::UICheckBoxState::SELECTED);
    box->SetState(UICheckBox::UICheckBoxState::UNSELECTED);
    EXPECT_EQ(box->GetState(), UICheckBox::UICheckBoxState::UNSELECTED);
    delete box;
}

/**
 * @tc.number   SUB_GRAPHIC_CHECKBOX_SetImages_0400
 * @tc.name     test check-box set-images api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UICheckboxTest, Graphic_UICheckboxTest_Test_SetImages_0400, Function | MediumTest | Level0)
{
    UICheckBox *box = new UICheckBox();
    box->SetImages("..\\config\\images\\A021_006.bin", "..\\config\\images\\A021_005.bin");
    EXPECT_EQ(0, 0);
    delete box;
}
}  // namespace OHOS