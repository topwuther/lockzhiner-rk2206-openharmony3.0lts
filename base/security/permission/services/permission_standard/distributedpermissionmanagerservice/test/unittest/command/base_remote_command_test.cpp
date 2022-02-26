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

#include <map>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <memory>
#include <iostream>
#include "gtest/gtest.h"
#include "base_remote_command.h"
#define private public
#include "distributed_uid_allocator.h"
#include "delete_uid_permission_command.h"
#include "ipc_skeleton.h"

using namespace std;
using namespace OHOS::Security::Permission;
using namespace testing::ext;

namespace {}  // namespace

namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
}  // namespace OHOS

class BaseRemoteCommandTest : public testing::Test {
public:
    BaseRemoteCommandTest();
    ~BaseRemoteCommandTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    const std::shared_ptr<BaseRemoteCommand> GetClass() const;
    UidBundleBo GetDefaultInfo() const;
    UidBundleBo GetNullPermissionsInfo() const;
    UidBundleBo GetNullRemoteSensitivePermissionsInfo() const;

private:
};

BaseRemoteCommandTest::BaseRemoteCommandTest()
{}
BaseRemoteCommandTest::~BaseRemoteCommandTest()
{}
void BaseRemoteCommandTest::SetUpTestCase()
{}
void BaseRemoteCommandTest::TearDownTestCase()
{}
void BaseRemoteCommandTest::SetUp()
{}
void BaseRemoteCommandTest::TearDown()
{}

UidBundleBo BaseRemoteCommandTest::GetDefaultInfo() const
{
    BundlePermissionsDto bundles;
    bundles.name = "bundles.name";
    bundles.bundleLabel = "bundles.bundleLabel";

    SignDto signDto;
    signDto.sha256 = "bundles.sign";
    bundles.sign.push_back(signDto);

    std::vector<PermissionDto> permissions;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        PermissionDto permission;
        permission.name = "permission.permissionName" + std::to_string(i);
        permission.grantMode = i;
        permissions.push_back(permission);
    }
    bundles.permissions = permissions;

    UidBundleBo uidBundleBo;
    uidBundleBo.DEFAULT_SIZE = 10;         // default 10
    uidBundleBo.MIN_UID_PACKAGES_NUM = 1;  // default 1
    uidBundleBo.uid = 1;
    uidBundleBo.appAttribute = 1;
    uidBundleBo.bundles.push_back(bundles);
    uidBundleBo.uidState = 1;

    std::set<std::string> remoteSensitivePermission;

    for (int i = 1; i <= maxIndex; i++) {
        remoteSensitivePermission.insert("permission.permissionName" + std::to_string(i));
    }
    return uidBundleBo;
}

UidBundleBo BaseRemoteCommandTest::GetNullPermissionsInfo() const
{
    BundlePermissionsDto bundles;
    bundles.name = "bundles.name";
    bundles.bundleLabel = "bundles.bundleLabel";
    SignDto signDto;
    signDto.sha256 = "bundles.sign";
    bundles.sign.push_back(signDto);
    std::vector<PermissionDto> permissions;

    UidBundleBo uidBundleBo;
    uidBundleBo.DEFAULT_SIZE = 10;         // default 10
    uidBundleBo.MIN_UID_PACKAGES_NUM = 1;  // default 1
    uidBundleBo.uid = 1;
    uidBundleBo.appAttribute = 1;
    uidBundleBo.bundles.push_back(bundles);
    uidBundleBo.uidState = 1;

    std::set<std::string> remoteSensitivePermission;
    int maxIndex = 5;
    for (int i = 1; i <= maxIndex; i++) {
        remoteSensitivePermission.insert("permission.permissionName" + std::to_string(i));
    }
    return uidBundleBo;
}

UidBundleBo BaseRemoteCommandTest::GetNullRemoteSensitivePermissionsInfo() const
{
    BundlePermissionsDto bundles;
    bundles.name = "bundles.name";
    bundles.bundleLabel = "bundles.bundleLabel";
    SignDto signDto;
    signDto.sha256 = "bundles.sign";
    bundles.sign.push_back(signDto);
    std::vector<PermissionDto> permissions;

    UidBundleBo uidBundleBo;
    uidBundleBo.DEFAULT_SIZE = 10;         // default 10
    uidBundleBo.MIN_UID_PACKAGES_NUM = 1;  // default 1
    uidBundleBo.uid = 1;
    uidBundleBo.appAttribute = 1;
    uidBundleBo.bundles.push_back(bundles);
    uidBundleBo.uidState = 1;

    return uidBundleBo;
}

HWTEST_F(BaseRemoteCommandTest, BaseRemoteCommandTest_0001, Function | MediumTest | Level1)
{
    bool ret1 = BaseRemoteCommand::IsValid(GetDefaultInfo());
    bool ret2 = BaseRemoteCommand::IsValid(GetNullPermissionsInfo());

    EXPECT_FALSE(ret1);
    EXPECT_TRUE(ret2);
}

HWTEST_F(BaseRemoteCommandTest, BaseRemoteCommandTest_0002, Function | MediumTest | Level1)
{
    const std::string LOCATION = "ohos.permission.LOCATION";
    const std::string CAMERA = "ohos.permission.CAMERA";
    const std::string MICROPHONE = "ohos.permission.MICROPHONE";
    const std::string READ_HEALTH_DATA = "ohos.permission.READ_HEALTH_DATA";

    bool ret1 = BaseRemoteCommand::IsSensitiveResource(CAMERA);
    bool ret2 = BaseRemoteCommand::IsSensitiveResource(LOCATION);
    bool ret3 = BaseRemoteCommand::IsSensitiveResource(READ_HEALTH_DATA);
    bool ret4 = BaseRemoteCommand::IsSensitiveResource(MICROPHONE);
    EXPECT_TRUE(ret1);
    EXPECT_TRUE(ret2);
    EXPECT_TRUE(ret3);
    EXPECT_TRUE(ret4);

    const std::string fail_str = "permission.unknowpermission";
    bool ret99 = BaseRemoteCommand::IsSensitiveResource(fail_str);
    EXPECT_FALSE(ret99);
}

HWTEST_F(BaseRemoteCommandTest, BaseRemoteCommandTest_0003, Function | MediumTest | Level1)
{
    const std::string CAMERA = "ohos.permission.CAMERA";
    const std::string fail_str = "permission.unknowpermission";

    UidBundleBo info = GetNullRemoteSensitivePermissionsInfo();

    BaseRemoteCommand::GrantSensitivePermissionToRemoteApp(info, CAMERA);

    for (const std::string &x : info.remoteSensitivePermission)
        EXPECT_EQ(x, CAMERA);

    BaseRemoteCommand::GrantSensitivePermissionToRemoteApp(info, fail_str);
}

HWTEST_F(BaseRemoteCommandTest, BaseRemoteCommandTest_0004, Function | MediumTest | Level1)
{
    const std::string CAMERA = "ohos.permission.CAMERA";
    const std::string fail_str = "permission.unknowpermission";

    UidBundleBo info = GetNullRemoteSensitivePermissionsInfo();
    BaseRemoteCommand::GrantSensitivePermissionToRemoteApp(info, CAMERA);

    bool ret1 = BaseRemoteCommand::IsGrantedSensitivePermission(info, CAMERA);
    EXPECT_TRUE(ret1);

    bool ret2 = BaseRemoteCommand::IsGrantedSensitivePermission(info, fail_str);
    EXPECT_FALSE(ret2);
}