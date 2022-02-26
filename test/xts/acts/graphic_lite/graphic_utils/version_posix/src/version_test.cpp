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

#include "gfx_utils/version.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class VersionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void VersionTest::SetUpTestCase()
{
    return;
}

void VersionTest::TearDownTestCase()
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_VERSION_GETVERSION_0100
 * @tc.name     test version get-version api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(VersionTest, Graphic_VersionTest_Test_GetVersion_0100, Function | MediumTest | Level0)
{
    GetVersion();
    EXPECT_EQ(0, 0);
}

/**
 * @tc.number   SUB_GRAPHIC_VERSION_GETLIBDSC_0200
 * @tc.name     test version get-lib-dsc api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(VersionTest, Graphic_VersionTest_Test_GetLibDsc_0200, Function | MediumTest | Level0)
{
    GetLibDsc();
    EXPECT_EQ(0, 0);
}
}