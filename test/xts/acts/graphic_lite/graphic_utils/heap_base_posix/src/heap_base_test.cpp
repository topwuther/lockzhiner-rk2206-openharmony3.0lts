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

#include "gfx_utils/heap_base.h"

#include <climits>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace OHOS {
class HeapBaseTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
};

/**
 * @tc.number   SUB_GRAPHIC_HEAPBASE_OPERATOR_NEW_0100
 * @tc.name     test heapbase operator-new api
 * @tc.desc     [C- SOFTWARE -0200]
 */
HWTEST_F(HeapBaseTest, Graphic_UIHeapBase_Test_operator_new_0100, Function | MediumTest | Level0)
{
    HeapBase* heap = new HeapBase();
    EXPECT_EQ(0, 0);
    delete heap;
    EXPECT_EQ(0, 0);
}
} // namespace OHOS