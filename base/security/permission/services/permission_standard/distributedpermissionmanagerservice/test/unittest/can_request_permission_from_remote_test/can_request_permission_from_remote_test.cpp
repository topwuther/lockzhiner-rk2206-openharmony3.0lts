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
#include "resource_switch.h"
#include "resource_switch_cache.h"
#include "resource_switch_local.h"
#include "resource_switch_remote.h"
#include "sensitive_resource_switch_setting.h"
using namespace testing::ext;
namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
namespace Security {
namespace Permission {
namespace {}  // namespace
class CanRequestPermissionFromRemoteTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        OHOS::DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStart();
    }
    static void TearDownTestCase(void)
    {
        OHOS::DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStop();
    }
    void SetUp()
    {}
    void TearDown()
    {}
};
/*
 * Feature: DPMS
 * Function: CanRequestPermissionFromRemoteTest
 * SubFunction: Construct
 * FunctionPoints: construct
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(CanRequestPermissionFromRemoteTest, can_request_permission_from_remote_test_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "can_request_permission_from_remote_test_001";
    std::shared_ptr<DistributedPermissionManagerService> service =
        std::make_shared<DistributedPermissionManagerService>();
    std::string permission("");
    std::string nodeId("networkId");
    std::string deviceId("deviceId");
    // case deviceId =""
    EXPECT_TRUE(!service->CanRequestPermissionFromRemote(permission, nodeId));
    // case permission =""
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    EXPECT_TRUE(!service->CanRequestPermissionFromRemote(permission, nodeId));
    // case permission is not SensitiveResource
    permission = "notSensitiveResource";
    EXPECT_TRUE(!service->CanRequestPermissionFromRemote(permission, nodeId));
    permission = Constant::LOCATION;
    EXPECT_TRUE(service->CanRequestPermissionFromRemote(permission, nodeId));
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS