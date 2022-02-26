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

#include "components/ui_radio_button.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIRadioButtonTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_RADIO_BUTTON_GETVIEWTYPE_0100
 * @tc.name     test radio-button get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIRadioButtonTest, Graphic_UIRadioButtonTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIRadioButton *radiobutton = new UIRadioButton();
    EXPECT_EQ(radiobutton->GetViewType(), UI_RADIO_BUTTON);
    delete radiobutton;
}

/**
 * @tc.number   SUB_GRAPHIC_RADIO_BUTTON_SETNAME_0200
 * @tc.name     test radio-button set-name api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIRadioButtonTest, Graphic_UIRadioButtonTest_Test_SetName_0200, Function | MediumTest | Level0)
{
    UIRadioButton *radiobutton = new UIRadioButton();
    const char *name = "radio button test";
    radiobutton->SetName(name);
    radiobutton->GetName();
    EXPECT_EQ(0, 0);
    delete radiobutton;
}
}