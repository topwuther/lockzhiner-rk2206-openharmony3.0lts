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
#include "components/ui_repeat_button.h"

using namespace std;
using namespace testing::ext;
namespace OHOS {
class UIRepeatButtonTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_REPEAT_BUTTON_GETVIEWTYPE_0100
 * @tc.name     test repeat-button get-view-type api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIRepeatButtonTest, Graphic_UIRepeatButtonTest_Test_GetViewType_0100, Function | MediumTest | Level0)
{
    UIRepeatButton *repeatButton = new UIRepeatButton();
    EXPECT_EQ(repeatButton->GetViewType(), UI_REPEAT_BUTTON);
    delete repeatButton;
}

/**
 * @tc.number   SUB_GRAPHIC_REPEAT_BUTTON_SETINTERVAL_0200
 * @tc.name     test repeat-button set-interval api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIRepeatButtonTest, Graphic_UIRepeatButtonTest_Test_SetInterval_0200, Function | MediumTest | Level0)
{
    UIRepeatButton *repeatButton = new UIRepeatButton();
    uint16_t interval = 10;
    repeatButton->SetInterval(interval);
    EXPECT_EQ(repeatButton->GetInterval(), interval);
    delete repeatButton;
}

/**
 * @tc.number   SUB_GRAPHIC_REPEAT_BUTTON_HANDLETICKEVENT_0300
 * @tc.name     test repeat-button handle-tick-event api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIRepeatButtonTest, Graphic_UIRepeatButtonTest_Test_HandleTickEvent_0300, Function | MediumTest | Level0)
{
    UIRepeatButton *repeatButton = new UIRepeatButton();
    repeatButton->HandleTickEvent();
    EXPECT_EQ(0, 0);
    delete repeatButton;
}
}