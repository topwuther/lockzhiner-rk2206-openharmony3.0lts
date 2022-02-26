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
#define private public
#include "mock_bundle_mgr.h"
#include "mock_permission_mgr.h"
#include "distributed_permission_manager_service.h"
#include "base_remote_command.h"
#include "get_uid_permission_command.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ability_manager_interface.h"
using namespace testing::ext;
namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
namespace Security {
namespace Permission {
namespace {}  // namespace
class VerifyPermissionFromRemoteTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
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
    static void TearDownTestCase(void)
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};
/*
 * Feature: DPMS
 * Function: VerifyPermissionFromRemoteTest
 * SubFunction: Construct
 * FunctionPoints: construct
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(VerifyPermissionFromRemoteTest, verify_permission_from_remote_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "verify_permission_from_remote_001";
    std::shared_ptr<DistributedPermissionManagerService> service =
        std::make_shared<DistributedPermissionManagerService>();
    std::string permission("");
    std::string nodeId("networkId");
    std::string deviceId("deviceId");
    std::string deviceName("deviceName");
    std::string deviceType("deviceType");
    // std::string existDeviceInfo("ExistDeviceInfo");
    int32_t pid = 1;
    int32_t uid = -100000;
    // case deviceId =""
    EXPECT_TRUE(service->VerifyPermissionFromRemote(permission, nodeId, pid, uid) == Constant::PERMISSION_DENIED);
    // case permission =""
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    EXPECT_TRUE(service->VerifyPermissionFromRemote(permission, nodeId, pid, uid) == Constant::PERMISSION_DENIED);
    // case permission is not SensitiveResource
    permission = "notSensitiveResource";
    EXPECT_TRUE(service->VerifyPermissionFromRemote(permission, nodeId, pid, uid) == Constant::PERMISSION_DENIED);
    // case  func IsValidVerificationParams  IsDeviceIdValid can not test
    // case IsUidValid
    permission = Constant::LOCATION;
    EXPECT_TRUE(service->VerifyPermissionFromRemote(permission, nodeId, pid, uid) == Constant::PERMISSION_DENIED);
    // case  is  ExistDeviceInfo
    uid = 100000;
    // now ,if ConvertToUniqueDisabilityIdOrFetch success,ExistDeviceInfo  can not fail
    // case IsRootOrSystemUid
    // isRoot
    EXPECT_TRUE(service->VerifyPermissionFromRemote(permission, nodeId, pid, uid) == Constant::PERMISSION_GRANTED);
    // is system
    uid = 101000;
    EXPECT_TRUE(service->VerifyPermissionFromRemote(permission, nodeId, pid, uid) == Constant::PERMISSION_GRANTED);
    //  IsSystemSignatureUid
    uid = 100111;
    EXPECT_TRUE(service->VerifyPermissionFromRemote(permission, nodeId, pid, uid) == Constant::PERMISSION_GRANTED);
    //  Is  not SystemSignatureUid  objectDevices_  no permission cache
    uid = 122111;
    EXPECT_TRUE(service->VerifyPermissionFromRemote(permission, nodeId, pid, uid) == Constant::PERMISSION_DENIED);
    std::set<std::string> permissionSet;
    permissionSet.insert(permission);
    ObjectDevicePermissionRepository::GetInstance().GetOrCreateObjectDevice(deviceId)->AddObjectUid(uid);
    ObjectDevicePermissionRepository::GetInstance().GetOrCreateObjectDevice(deviceId)->ResetGrantSensitivePermission(
        uid, permissionSet);
    EXPECT_TRUE(service->VerifyPermissionFromRemote(permission, nodeId, pid, uid) == Constant::PERMISSION_GRANTED);
}
HWTEST_F(VerifyPermissionFromRemoteTest, verify_self_permission_from_remote_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "verify_self_permission_from_remote_001";
    std::shared_ptr<DistributedPermissionManagerService> service =
        std::make_shared<DistributedPermissionManagerService>();
    std::string permission("");
    std::string nodeId("networkId");
    std::string deviceId("deviceId");
    std::string deviceName("deviceName");
    std::string deviceType("deviceType");
    // case permission =""
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    EXPECT_TRUE(service->VerifySelfPermissionFromRemote(permission, nodeId) == Constant::PERMISSION_DENIED);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS