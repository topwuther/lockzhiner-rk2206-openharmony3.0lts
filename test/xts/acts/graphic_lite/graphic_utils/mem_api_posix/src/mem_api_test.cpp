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

#include "gfx_utils/mem_api.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class MemapiTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void MemapiTest::SetUpTestCase()
{
    return;
}

void MemapiTest::TearDownTestCase()
{
    return;
}

/**
 * @tc.number   SUB_GRAPHIC_MEMAPI_IMAGECACHEMALLOC_0100
 * @tc.name     test memapi image-cache-malloc api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(MemapiTest, Graphic_UIAnimatorTest_Test_ImageCacheMalloc_0100, Function | MediumTest | Level0)
{
    ImageInfo* info = new ImageInfo();
    free(ImageCacheMalloc(*info));
    EXPECT_EQ(0, 0);
    ImageCacheFree(*info);
    delete info;
}

/**
 * @tc.number   SUB_GRAPHIC_MEMAPI_UIMALLOC_0200
 * @tc.name     test memapi uimalloc api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(MemapiTest, Graphic_UIAnimatorTest_Test_UIMalloc_0200, Function | MediumTest | Level0)
{
    void* size = UIMalloc(3);
    EXPECT_EQ(0, 0);
    free(size);
}

/**
 * @tc.number   SUB_GRAPHIC_MEMAPI_UIFREE_0300
 * @tc.name     test memapi uifree api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(MemapiTest, Graphic_UIAnimatorTest_Test_UIFree_0300, Function | MediumTest | Level0)
{
    void* buffer = nullptr;
    UIFree(buffer);
    EXPECT_EQ(0, 0);
}
}