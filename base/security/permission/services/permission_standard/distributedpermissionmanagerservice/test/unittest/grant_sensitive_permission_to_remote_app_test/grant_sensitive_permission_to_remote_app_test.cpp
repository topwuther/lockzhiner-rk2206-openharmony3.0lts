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
#include "distributed_permission_manager_service.h"
using namespace testing::ext;
namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
namespace Security {
namespace Permission {
namespace {}  // namespace
class GrantSensitivePermissionToRemoteAppTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {}
    static void TearDownTestCase(void)
    {}
    void SetUp()
    {}
    void TearDown()
    {}
};
/*
 * Feature: DPMS
 * Function: GrantSensitivePermissionToRemoteApp
 * SubFunction: Construct
 * FunctionPoints: construct
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(GrantSensitivePermissionToRemoteAppTest, grant_sensitive_permission_to_remote_app_test_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "grant_sensitive_permission_to_remote_app_test_001";
    std::shared_ptr<DistributedPermissionManagerService> service =
        std::make_shared<DistributedPermissionManagerService>();
    std::string permission("");
    std::string nodeId("networkId");
    std::string deviceId("deviceId");
    int ruid = -10000;
    service->GrantSensitivePermissionToRemoteApp(permission, nodeId, ruid);
    EXPECT_TRUE(SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.size() == 0);
    EXPECT_TRUE(
        SubjectDevicePermissionManager::GetInstance().GetGrantSensitivePermissionToRemoteApp(deviceId, ruid).size() ==
        0);
    // case uid  fail
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    service->GrantSensitivePermissionToRemoteApp(permission, nodeId, ruid);
    EXPECT_TRUE(SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.size() == 0);
    EXPECT_TRUE(
        SubjectDevicePermissionManager::GetInstance().GetGrantSensitivePermissionToRemoteApp(deviceId, ruid).size() ==
        0);
    // case uid isRootOrSys  fail
    ruid = 1000;
    service->GrantSensitivePermissionToRemoteApp(permission, nodeId, ruid);
    EXPECT_TRUE(SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.size() == 0);
    EXPECT_TRUE(
        SubjectDevicePermissionManager::GetInstance().GetGrantSensitivePermissionToRemoteApp(deviceId, ruid).size() ==
        0);
    // case uid isRootOrSys  fail
    ruid = 100000;
    service->GrantSensitivePermissionToRemoteApp(permission, nodeId, ruid);
    EXPECT_TRUE(SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.size() == 0);
    EXPECT_TRUE(
        SubjectDevicePermissionManager::GetInstance().GetGrantSensitivePermissionToRemoteApp(deviceId, ruid).size() ==
        0);
    // case distributedPermissionMapping_ can not have this
    ruid = 123456;
    service->GrantSensitivePermissionToRemoteApp(permission, nodeId, ruid);
    EXPECT_TRUE(SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.size() == 1);
    EXPECT_TRUE(
        SubjectDevicePermissionManager::GetInstance().GetGrantSensitivePermissionToRemoteApp(deviceId, ruid).size() ==
        0);
    // permission is IsSensitiveResource
    permission = Constant::LOCATION;
    service->GrantSensitivePermissionToRemoteApp(permission, nodeId, ruid);
    EXPECT_TRUE(SubjectDevicePermissionManager::GetInstance().distributedPermissionMapping_.size() == 1);
    EXPECT_TRUE(
        SubjectDevicePermissionManager::GetInstance().GetGrantSensitivePermissionToRemoteApp(deviceId, ruid).size() ==
        1);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS