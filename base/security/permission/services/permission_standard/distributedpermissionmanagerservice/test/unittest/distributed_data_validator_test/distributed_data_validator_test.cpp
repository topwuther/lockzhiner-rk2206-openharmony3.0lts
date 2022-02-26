
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

#include <thread>
#include <functional>
#include "gtest/gtest.h"
#include <map>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <memory>
#include <iostream>
#include "distributed_data_validator.h"
#include "ipc_skeleton.h"

using namespace std;
using namespace OHOS::Security::Permission;
using namespace testing::ext;

namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
}  // namespace OHOS

class DistributedDataValidatorTest : public testing::Test {
public:
    DistributedDataValidatorTest();
    ~DistributedDataValidatorTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    const std::shared_ptr<DistributedDataValidator> GetCalss() const;

private:
    std::shared_ptr<DistributedDataValidator> class_ = std::make_shared<DistributedDataValidator>();
};

DistributedDataValidatorTest::DistributedDataValidatorTest()
{}
DistributedDataValidatorTest::~DistributedDataValidatorTest()
{}
void DistributedDataValidatorTest::SetUpTestCase()
{}
void DistributedDataValidatorTest::TearDownTestCase()
{}
void DistributedDataValidatorTest::SetUp()
{}
void DistributedDataValidatorTest::TearDown()
{}
const std::shared_ptr<DistributedDataValidator> DistributedDataValidatorTest::GetCalss() const
{
    return class_;
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0012, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    int duid = 100000;
    bool ret1 = class_->IsSpecDuid(duid);
    EXPECT_TRUE(ret1);

    duid = 101000;
    bool ret4 = class_->IsSpecDuid(duid);
    EXPECT_TRUE(ret4);

    duid = 101001;
    bool ret2 = class_->IsSpecDuid(duid);
    EXPECT_FALSE(ret2);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0011, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    int uid = 12600000;
    bool ret1 = class_->IsDuid(uid);
    EXPECT_TRUE(ret1);

    uid = 12500000;
    bool ret4 = class_->IsDuid(uid);
    EXPECT_FALSE(ret4);

    uid = 12800000;
    bool ret2 = class_->IsDuid(uid);
    EXPECT_FALSE(ret2);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0010, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    int ruid = 0;
    bool ret1 = class_->IsSpecRuid(ruid);
    EXPECT_TRUE(ret1);

    ruid = 1000;
    bool ret4 = class_->IsSpecRuid(ruid);
    EXPECT_TRUE(ret4);

    ruid = 123456;
    bool ret2 = class_->IsSpecRuid(ruid);
    EXPECT_FALSE(ret2);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0009, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    int version = 0;
    bool ret1 = class_->IsCommandVersionValid(version);
    EXPECT_FALSE(ret1);

    version = 1;
    bool ret4 = class_->IsCommandVersionValid(version);
    EXPECT_TRUE(ret4);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0008, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    int duid = 12500000;
    bool ret3 = class_->IsDuidValidAndNotSpecial(duid);
    EXPECT_FALSE(ret3);

    duid = 12612345;
    bool ret4 = class_->IsDuidValidAndNotSpecial(duid);
    EXPECT_TRUE(ret4);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0007, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    int duid = 0;
    bool ret1 = class_->IsDuidValid(duid);
    EXPECT_TRUE(ret1);

    duid = 1000;
    bool ret2 = class_->IsDuidValid(duid);
    EXPECT_TRUE(ret2);

    duid = 12500000;
    bool ret3 = class_->IsDuidValid(duid);
    EXPECT_FALSE(ret3);

    duid = 12612345;
    bool ret4 = class_->IsDuidValid(duid);
    EXPECT_TRUE(ret4);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0006, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    std::string packageName;
    bool ret1 = class_->IsPackageNameValid(packageName);
    EXPECT_FALSE(ret1);

    packageName = "packageName";
    bool ret2 = class_->IsPackageNameValid(packageName);
    EXPECT_TRUE(ret2);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0005, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    int uid = -1;
    bool ret1 = class_->IsUidValid(uid);
    EXPECT_FALSE(ret1);

    uid = 0;
    bool ret2 = class_->IsUidValid(uid);
    EXPECT_TRUE(ret2);

    uid = 1;
    bool ret3 = class_->IsUidValid(uid);
    EXPECT_TRUE(ret3);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0004, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    std::string deviceId;
    bool ret1 = class_->IsDeviceIdValid(deviceId);
    EXPECT_FALSE(ret1);

    deviceId = "deviceId";
    bool ret2 = class_->IsDeviceIdValid(deviceId);
    EXPECT_TRUE(ret2);

    deviceId = "";
    for (int i = 0; i <= 64; i++) {
        deviceId += "d";
    }
    bool ret3 = class_->IsDeviceIdValid(deviceId);
    EXPECT_FALSE(ret3);

    deviceId = "";
    for (int i = 0; i <= 63; i++) {
        deviceId += "d";
    }
    bool ret4 = class_->IsDeviceIdValid(deviceId);
    EXPECT_TRUE(ret4);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0003, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    int userId = -1;
    bool ret1 = class_->IsUserIdValid(userId);
    EXPECT_FALSE(ret1);

    userId = 0;
    bool ret2 = class_->IsUserIdValid(userId);
    EXPECT_TRUE(ret2);

    userId = 1;
    bool ret3 = class_->IsUserIdValid(userId);
    EXPECT_TRUE(ret3);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0002, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    std::string permissionName;
    bool ret1 = class_->IsPermissionNameValid(permissionName);
    EXPECT_FALSE(ret1);

    permissionName = "permissionName";
    bool ret2 = class_->IsPermissionNameValid(permissionName);
    EXPECT_TRUE(ret2);

    permissionName = "";
    for (int i = 0; i <= 256; i++) {
        permissionName += "p";
    }
    bool ret3 = class_->IsPermissionNameValid(permissionName);
    EXPECT_FALSE(ret3);

    permissionName = "";
    for (int i = 0; i <= 255; i++) {
        permissionName += "p";
    }
    bool ret4 = class_->IsPermissionNameValid(permissionName);

    EXPECT_TRUE(ret4);
}

HWTEST_F(DistributedDataValidatorTest, DistributedDataValidatorTest_0001, Function | MediumTest | Level1)
{
    auto class_ = GetCalss();
    ASSERT_NE(class_, nullptr);

    std::string bundleName;
    bool ret1 = class_->IsBundleNameValid(bundleName);
    EXPECT_FALSE(ret1);

    bundleName = "bundleName";
    bool ret2 = class_->IsBundleNameValid(bundleName);
    EXPECT_TRUE(ret2);

    bundleName = "";
    for (int i = 0; i <= 256; i++) {
        bundleName += "b";
    }
    bool ret3 = class_->IsBundleNameValid(bundleName);
    EXPECT_FALSE(ret3);

    bundleName = "";
    for (int i = 0; i <= 255; i++) {
        bundleName += "b";
    }
    bool ret4 = class_->IsBundleNameValid(bundleName);

    EXPECT_TRUE(ret4);
}
