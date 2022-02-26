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
#include "dfx/ui_screenshot.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;

namespace OHOS {
enum ButtonState : uint8_t {
    RELEASED = 0,
    PRESSED,
    INACTIVE,
    BTN_STATE_NUM,
};

class UIScreenshotTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_SCREENSHOT_GETINSTANCE_0100
 * @tc.name     test screenshot get-instance api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(UIScreenshotTest, Graphic_UIScreenshot_Test_GetInstance_0100, Function | MediumTest | Level0)
{
    UIScreenshot::GetInstance();
    EXPECT_EQ(0, 0);    
}
} // namespace OHOS