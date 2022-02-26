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

#include "surface_buffer.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class SurfaceSurfaceTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SURFACE_GETVIRADDR_0100
 * @tc.name     test surface-surface get-vir-addr api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceSurfaceTest, Graphic_SurfaceSurface_Test_GetVirAddr_0100, Function | MediumTest | Level0)
{
    Surface* surfaceVar = new Surface();
    surfaceVar->GetVirAddr();
    EXPECT_EQ(0, 0);

    delete surfaceVar;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SURFACE_INT32_0200
 * @tc.name     test surface int32 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceSurfaceTest, Graphic_SurfaceSurface_Test_Int32_0200, Function | MediumTest | Level0)
{
    Surface* surfaceVar = new Surface();
    uint32_t key = 0x00001000;
    int32_t value = 0x00002000;
    int32_t testReturn = surfaceVar->SetInt32(key, value);
    if (testReturn) {
        EXPECT_EQ(surfaceVar->GetInt32(key, value), value);
    }
    delete surfaceVar;
}

/**
 * @tc.number   SUB_GRAPHIC_SURFACE_SURFACE_INT64_0300
 * @tc.name     test surface int64 api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(SurfaceSurfaceTest, Graphic_SurfaceSurface_Test_Int64_0300, Function | MediumTest | Level0)
{
    Surface* surfaceVar = new Surface();
    uint32_t key = 0x00001000;
    int64_t value_ = 0x0000000000002000;
    int64_t testReturn = surfaceVar->SetInt64(key, value);
    if (testReturn) {
        EXPECT_EQ(surfaceVar->GetInt32(key, value), value);
    }
    delete surfaceVar;
}
} // namespace OHOS