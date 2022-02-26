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
#include "uninstaller.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;

class UninstallerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UninstallerTest::SetUpTestCase(void)
{}

void UninstallerTest::TearDownTestCase(void)
{}

void UninstallerTest::SetUp(void)
{}

void UninstallerTest::TearDown(void)
{}

/**
  * @tc.name: Test001
  * @tc.desc: test get uninstaller instance.
  * @tc.type: FUNC
  * @tc.require: SR000DOGUN AR000DPSE9
  * @tc.author: hongbo
  */
HWTEST_F(UninstallerTest, Test001, TestSize.Level0)
{
    auto &unin = Uninstaller::GetInstance();
    unin.Init(nullptr);
}
