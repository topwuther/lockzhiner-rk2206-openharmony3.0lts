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

#include "crypto_utils.h"
#include "permission_validator.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;

class PermissionValidatorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PermissionValidatorTest::SetUpTestCase(void)
{}

void PermissionValidatorTest::TearDownTestCase(void)
{}

void PermissionValidatorTest::SetUp(void)
{}

void PermissionValidatorTest::TearDown(void)
{}

/**
  * @tc.name: TestPermissionValidate001
  * @tc.desc: test if CheckPermission can return correct permission.
  * @tc.type: FUNC
  * @tc.require: AR000CQDUT
  * @tc.author: liqiao
  */
HWTEST_F(PermissionValidatorTest, TestPermissionValidate001, TestSize.Level0)
{
    std::string userId = "ohos";
    std::string appId = "ohosApp";
    EXPECT_TRUE(PermissionValidator::CheckSyncPermission(userId, appId));
}

/**
  * @tc.name: TestPermissionValidate002
  * @tc.desc: test if CheckPermission can return correct permission.
  * @tc.type: FUNC
  * @tc.require:AR000DPSGU
  * @tc.author: liqiao
  */
HWTEST_F(PermissionValidatorTest, TestPermissionValidate002, TestSize.Level0)
{
    std::string userId = "ohos";
    std::string appId = "ohosApp";
    EXPECT_TRUE(PermissionValidator::CheckSyncPermission(userId, appId));
}

/**
  * @tc.name: TestPermissionValidate003
  * @tc.desc: test if account id sha256.
  * @tc.type: FUNC
  * @tc.require: AR000DPSH0 AR000DPSEC
  * @tc.author: liqiao
  */
HWTEST_F(PermissionValidatorTest, TestPermissionValidate003, TestSize.Level0)
{
    std::string userId = "ohos";
    EXPECT_NE(CryptoUtils::Sha256("ohos"), userId);
}
