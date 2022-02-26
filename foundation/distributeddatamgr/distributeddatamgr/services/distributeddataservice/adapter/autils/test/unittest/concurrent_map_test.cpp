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

#include <gtest/gtest.h>
#include "concurrent_map.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;

class ConcurrentMapTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ConcurrentMapTest::SetUpTestCase(void)
{}

void ConcurrentMapTest::TearDownTestCase(void)
{}

void ConcurrentMapTest::SetUp(void)
{}

void ConcurrentMapTest::TearDown(void)
{}

/**
  * @tc.name: Test001
  * @tc.desc: test concurrent map CURD operation.
  * @tc.type: FUNC
  * @tc.require: AR000CQS31
  * @tc.author: hongbo
  */
HWTEST_F(ConcurrentMapTest, Test001, TestSize.Level0)
{
    ConcurrentMap<std::string, std::string> cmap;

    int size = cmap.Size();
    ASSERT_TRUE(size == 0);

    std::string temp = "abc";
    cmap.Put(temp, temp);

    std::string value;
    auto val = cmap.Get(temp, value);
    ASSERT_TRUE(val);
    ASSERT_STREQ(temp.c_str(), value.c_str());

    auto isContained = cmap.ContainsKey(temp);
    ASSERT_TRUE(isContained);

    ASSERT_TRUE(cmap.Delete(temp));

    ASSERT_TRUE(cmap.Empty());

    cmap.Clear();
}