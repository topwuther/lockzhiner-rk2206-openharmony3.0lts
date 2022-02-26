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

#include "gfx_utils/graphic_math.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class MathTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void MathTest::SetUpTestCase()
{
    return;
}

void MathTest::TearDownTestCase()
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_MATH_FASTATAN2_0200
 * @tc.name     test math fast-atan2 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(MathTest, Graphic_Math_Test_FastAtan2_0100, Function | MediumTest | Level0)
{
    EXPECT_EQ(FastAtan2(0, 1), 0);
    EXPECT_EQ(FastAtan2(1, 0), 90);
    EXPECT_EQ(FastAtan2(0, -1), 180);
    EXPECT_EQ(FastAtan2(-1, 0), 270);
}
}