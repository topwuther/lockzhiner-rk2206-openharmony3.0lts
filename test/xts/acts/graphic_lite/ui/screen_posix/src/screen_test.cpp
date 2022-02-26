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

#include "common/screen.h"
#include "graphic_config.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class ScreenTest : public testing::Test {
public:
    static void SetUpTestCase(void){}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_SCREEN_GETWIDTH_0100
 * @tc.name     test screen get-width api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ScreenTest, Graphic_ScreenTest_Test_GetWidth_0100, Function | MediumTest | Level0)
{
    EXPECT_EQ(Screen::GetInstance().GetWidth(), HORIZONTAL_RESOLUTION);
}

/**
 * @tc.number   SUB_GRAPHIC_IMAGE_GETHEIGHT_0200
 * @tc.name     test image get-height api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(ScreenTest, Graphic_ScreenTest_Test_GetHeight_0200, Function | MediumTest | Level0)
{
    EXPECT_EQ(Screen::GetInstance().GetHeight(), VERTICAL_RESOLUTION);
}
}