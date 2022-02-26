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
#include "device_info_repository.h"
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
class CheckCallingPermissionTest : public testing::Test {
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

HWTEST_F(CheckCallingPermissionTest, CheckCallingPermission01, Function | MediumTest | Level1)
{
    string permissionName = "";
    int result = Constant::PERMISSION_DENIED;
    int ret = DistributedPermissionManagerService().CheckCallingPermission(permissionName);
    EXPECT_EQ(result, ret);
}

HWTEST_F(CheckCallingPermissionTest, CheckCallingPermission02, Function | MediumTest | Level1)
{
    string permissionName = "ohos.permission.INTERNET";
    int result = Constant::PERMISSION_GRANTED;
    int ret = DistributedPermissionManagerService().CheckCallingPermission(permissionName);
    EXPECT_EQ(result, ret);
}

HWTEST_F(CheckCallingPermissionTest, CheckCallingPermission03, Function | MediumTest | Level1)
{
    string permissionName = "ohos.permission.INTERNET";
    pid_t pid = getpid();
    pid_t uid = (pid_t)getuid();
    string deviceId = "9999";
    IPCSkeleton::pid_ = pid;
    IPCSkeleton::uid_ = uid;
    IPCSkeleton::localDeviceId_ = deviceId;
    IPCSkeleton::deviceId_ = deviceId;
    int result = Constant::PERMISSION_DENIED;
    int ret = DistributedPermissionManagerService().CheckCallingPermission(permissionName);
    EXPECT_EQ(result, ret);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS