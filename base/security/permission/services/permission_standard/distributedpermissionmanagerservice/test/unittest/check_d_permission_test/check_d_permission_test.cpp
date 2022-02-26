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
#include "gtest/gtest.h"
#include "base_remote_command.h"
#include "constant.h"
#define private public
#include "distributed_permission_manager_service.h"
#include "subject_device_permission_manager.h"
#include "mock_bundle_mgr.h"
#include "mock_permission_mgr.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ability_manager_interface.h"

using namespace std;
using namespace OHOS::Security::Permission;
using namespace testing::ext;
namespace OHOS {

pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";

namespace Security {
namespace Permission {
class CheckDPermissionTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        cout << "SetUpTestCase()" << endl;
        OHOS::sptr<OHOS::IRemoteObject> bundleObject = NULL;
        OHOS::sptr<OHOS::IRemoteObject> permissionObject = new PermissionManagerService();
        auto sysMgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sysMgr == NULL) {
            GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
            return;
        }
        sysMgr->AddSystemAbility(Constant::ServiceId::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
        sysMgr->AddSystemAbility(Constant::ServiceId::SUBSYS_SECURITY_PERMISSION_SYS_SERVICE_ID, permissionObject);
    }

    static void TearDownTestCase()
    {
        cout << "TearDownTestCase()" << endl;
    }
    void SetUp()
    {
        cout << "SetUp() is running" << endl;
    }
    void TearDown()
    {
        cout << "TearDown()" << endl;
    }
};

HWTEST_F(CheckDPermissionTest, CheckDPermission01, Function | MediumTest | Level1)
{
    int duid = 1000;
    string permissionName = "ohos.permission.INTERNET";
    int result = Constant::PERMISSION_GRANTED;
    int ret = DistributedPermissionManagerService().CheckDPermission(duid, permissionName);
    EXPECT_EQ(result, ret);
}

HWTEST_F(CheckDPermissionTest, CheckDPermission02, Function | MediumTest | Level1)
{
    int duid = 100;
    string permissionName = "ohos.permission.INTERNET";
    int result = Constant::PERMISSION_DENIED;
    int ret = DistributedPermissionManagerService().CheckDPermission(duid, permissionName);
    EXPECT_EQ(result, ret);
}

HWTEST_F(CheckDPermissionTest, CheckDPermission03, Function | MediumTest | Level1)
{
    int duid = 100;
    string permissionName = " ";
    int result = Constant::PERMISSION_DENIED;
    int ret = DistributedPermissionManagerService().CheckDPermission(duid, permissionName);
    EXPECT_EQ(result, ret);
}

HWTEST_F(CheckDPermissionTest, CheckDPermission04, Function | MediumTest | Level1)
{
    int duid = 12619999;
    string permissionName = "ohos.permission.INTERNET";
    int result = Constant::PERMISSION_DENIED;
    int ret = DistributedPermissionManagerService().CheckDPermission(duid, permissionName);
    EXPECT_EQ(result, ret);
}

int32_t AddDistributedPermissionTest(const std::string &deviceId, UidBundleBo &ruidPackages)
{
    int duid = 12610000;
    ruidPackages.DEFAULT_SIZE = 1;
    ruidPackages.MIN_UID_PACKAGES_NUM = 1;
    ruidPackages.uid = 1;
    ruidPackages.appAttribute = Constant::INITIAL_APP_ATTRIBUTE;
    ruidPackages.uidState = 1;
    ruidPackages.remoteSensitivePermission.insert(Constant::CAMERA);
    ruidPackages.remoteSensitivePermission.insert(Constant::READ_CALENDAR);
    ruidPackages.remoteSensitivePermission.insert(Constant::MICROPHONE);
    std::vector<BundlePermissionsDto> bundleVector(2);
    ruidPackages.bundles = bundleVector;
    bundleVector[0].name = "theme";
    bundleVector[0].bundleLabel = "beta";
    SignDto signDto;
    signDto.sha256 = "ohos";
    bundleVector[0].sign.push_back(signDto);
    PermissionDto nfcTagPermission;
    nfcTagPermission.name = "ohos.permission.NFC_TAG";
    nfcTagPermission.grantMode = 000;
    nfcTagPermission.status = 1;
    bundleVector[0].permissions.push_back(nfcTagPermission);
    PermissionDto vibratePermission;
    vibratePermission.name = "ohos.permission.VIBRATE";
    vibratePermission.grantMode = 000;
    vibratePermission.status = 2;
    bundleVector[0].permissions.push_back(vibratePermission);
    PermissionDto internetPermission;
    internetPermission.name = "ohos.permission.INTERNET";
    internetPermission.grantMode = 000;
    internetPermission.status = 1;
    bundleVector[0].permissions.push_back(internetPermission);
    ruidPackages.bundles = bundleVector;
    SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.insert(
        std::pair<int32_t, UidBundleBo>(duid, ruidPackages));
    return Constant::SUCCESS;
}

HWTEST_F(CheckDPermissionTest, CheckDPermission05, Function | MediumTest | Level1)
{
    int duid = 12610000;
    string permissionName = "ohos.permission.INTERNET";
    UidBundleBo ruidPackages;
    int res = Constant::SUCCESS;
    int r = AddDistributedPermissionTest("100", ruidPackages);
    EXPECT_EQ(res, r);
    int result = Constant::PERMISSION_GRANTED;
    int ret = DistributedPermissionManagerService().CheckDPermission(duid, permissionName);
    EXPECT_EQ(result, ret);
}

HWTEST_F(CheckDPermissionTest, CheckDPermission06, Function | MediumTest | Level1)
{
    int duid = 12619999;
    string permissionName = "ohos.permission.GET_NETWORK_INFO";
    UidBundleBo ruidPackages;
    int r = AddDistributedPermissionTest("100", ruidPackages);
    EXPECT_EQ(0, r);
    int result = Constant::PERMISSION_DENIED;
    int ret = DistributedPermissionManagerService().CheckDPermission(duid, permissionName);
    EXPECT_EQ(result, ret);
}

HWTEST_F(CheckDPermissionTest, CheckDPermission07, Function | MediumTest | Level1)
{
    int duid = 12610000;
    string permissionName = Constant::CAMERA;
    UidBundleBo ruidPackages;
    int r = AddDistributedPermissionTest("100", ruidPackages);
    EXPECT_EQ(0, r);
    int result = Constant::PERMISSION_GRANTED;
    int ret = DistributedPermissionManagerService().CheckDPermission(duid, permissionName);
    EXPECT_EQ(result, ret);
}

HWTEST_F(CheckDPermissionTest, CheckDPermission08, Function | MediumTest | Level1)
{
    int duid = 12619999;
    string permissionName = Constant::ACTIVITY_MOTION;
    UidBundleBo ruidPackages;
    int r = AddDistributedPermissionTest("100", ruidPackages);
    EXPECT_EQ(0, r);
    int result = Constant::PERMISSION_DENIED;
    int ret = DistributedPermissionManagerService().CheckDPermission(duid, permissionName);
    EXPECT_EQ(result, ret);
}

HWTEST_F(CheckDPermissionTest, CheckDPermission09, Function | MediumTest | Level1)
{
    int duid = 12619999;
    string permissionName = "ohos.permission.VIBRATE";
    UidBundleBo ruidPackages;
    int res = Constant::SUCCESS;
    int r = AddDistributedPermissionTest("100", ruidPackages);
    EXPECT_EQ(res, r);
    int result = Constant::PERMISSION_DENIED;
    int ret = DistributedPermissionManagerService().CheckDPermission(duid, permissionName);
    EXPECT_EQ(result, ret);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS