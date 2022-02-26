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
#include <iostream>
#include "distributed_permission_manager_service.h"
#include "on_request_permissions_result_stub.h"
#include "mock_bundle_mgr.h"
#include "mock_permission_mgr.h"
#include "base_remote_command.h"
#include "get_uid_permission_command.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ability_manager_interface.h"
#include "permission_log.h"
#include "request_remote_permission.h"
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
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionBmsManager"};
}
namespace {}  // namespace
class RequestPermissionsFromRemoteTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {}
    static void TearDownTestCase(void)
    {}
    void SetUp()
    {
        service = DelayedSingleton<DistributedPermissionManagerService>::GetInstance();
        service->OnStart();
    }
    void TearDown()
    {
        service->OnStop();
    }

public:
    std::shared_ptr<DistributedPermissionManagerService> service;
};
class TestCallbackA : public OnRequestPermissionsResultStub {
public:
    TestCallbackA() = default;
    virtual ~TestCallbackA() = default;

    void OnResult(const std::string nodeId, std::vector<std::string> permissions, std::vector<int32_t> grantResults)
    {
        onResult_--;
        PERMISSION_LOG_INFO(LABEL, "RequestPermissionsFromRemoteTest :OnResult  is run!!!");
    }

    void OnCancel(const std::string nodeId, std::vector<std::string> permissions)
    {
        onCancel_--;
        PERMISSION_LOG_INFO(LABEL, "RequestPermissionsFromRemoteTest :OnCancel  is run!!!");
    }
    void OnTimeOut(const std::string nodeId, std::vector<std::string> permissions)
    {
        onTimeOut_--;
        PERMISSION_LOG_INFO(LABEL, "RequestPermissionsFromRemoteTest :OnTimeOut  is run!!!");
    }
    int onResult_ = 100;
    int onCancel_ = 100;
    int onTimeOut_ = 100;
};
HWTEST_F(RequestPermissionsFromRemoteTest, request_permissionsservice_from_remote_test_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_permissionsservice_from_remote_test_001";
    auto timeFast = std::chrono::milliseconds(200);
    std::vector<std::string> permissions(0);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("");
    std::string deviceId("deviceId");
    int32_t reasonResId = -1;
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    // case NeedStopProcess fail
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    PERMISSION_LOG_INFO(LABEL, "onResult_ : %{public}d,", callback->onResult_);
    EXPECT_EQ(callback->onResult_, 100);
    EXPECT_EQ(callback->onCancel_, 100);
    EXPECT_EQ(callback->onTimeOut_, 100);
}
HWTEST_F(RequestPermissionsFromRemoteTest, request_permissionsservice_from_remote_test_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_permissionsservice_from_remote_test_002";
    auto timeFast = std::chrono::milliseconds(200);
    std::vector<std::string> permissions;
    permissions.push_back(Constant::LOCATION);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("");
    std::string deviceId("deviceId");
    int32_t reasonResId = -1;
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    // requestingDevices_.count(request->deviceId_) != 0
    RequestRemotePermission::GetInstance().requestingDevices_.insert(
        std::pair<std::string, std::string>(deviceId, deviceId));
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    PERMISSION_LOG_INFO(LABEL, "onResult_ : %{public}d,", callback->onResult_);
    EXPECT_EQ(callback->onResult_, 99);
    EXPECT_EQ(callback->onCancel_, 100);
    EXPECT_EQ(callback->onTimeOut_, 100);
}
// 3and 4same ，deviceid can not empty
HWTEST_F(RequestPermissionsFromRemoteTest, request_permissionsservice_from_remote_test_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_permissionsservice_from_remote_test_003";
    auto timeFast = std::chrono::milliseconds(200);
    std::vector<std::string> permissions;
    permissions.push_back(Constant::LOCATION);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("");
    std::string deviceId("deviceId");
    int32_t reasonResId = -1;
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    RequestRemotePermission::GetInstance().requestingDevices_.erase(deviceId);
    // case   RequestPermissionsInfo::CanRequestPermissions IsDeviceIdValid fail
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    PERMISSION_LOG_INFO(LABEL, "onResult_ : %{public}d,", callback->onResult_);

    EXPECT_EQ(callback->onResult_, 99);
    EXPECT_EQ(callback->onCancel_, 100);
    EXPECT_EQ(callback->onTimeOut_, 100);
}
HWTEST_F(RequestPermissionsFromRemoteTest, request_permissionsservice_from_remote_test_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_permissionsservice_from_remote_test_005";
    auto timeFast = std::chrono::milliseconds(200);
    std::vector<std::string> permissions;
    permissions.push_back(Constant::LOCATION);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("");
    std::string deviceId("deviceId");
    int32_t reasonResId = -1;
    // case  InitObjectLocaleReason bundleName=""
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    PERMISSION_LOG_INFO(LABEL, "onResult_ : %{public}d,", callback->onResult_);

    EXPECT_EQ(callback->onResult_, 99);
    EXPECT_EQ(callback->onCancel_, 100);
    EXPECT_EQ(callback->onTimeOut_, 100);
}
HWTEST_F(RequestPermissionsFromRemoteTest, request_permissionsservice_from_remote_test_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_permissionsservice_from_remote_test_006";
    auto timeFast = std::chrono::milliseconds(200);
    std::vector<std::string> permissions;
    permissions.push_back(Constant::LOCATION);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("bundleName");
    std::string deviceId("deviceId");
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    // case  InitObjectLocaleReason reasonResId_ < 0)
    int32_t reasonResId = -1;
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    PERMISSION_LOG_INFO(LABEL, "onResult_ : %{public}d,", callback->onResult_);
    EXPECT_EQ(callback->onResult_, 99);
    EXPECT_EQ(callback->onCancel_, 100);
    EXPECT_EQ(callback->onTimeOut_, 100);
}

HWTEST_F(RequestPermissionsFromRemoteTest, request_permissionsservice_from_remote_test_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_permissionsservice_from_remote_test_007";
    auto timeFast = std::chrono::milliseconds(2000);
    std::vector<std::string> permissions;
    permissions.push_back(Constant::LOCATION);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("bundleName");
    std::string deviceId("deviceId");
    int32_t reasonResId = -1;
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    //  case now  InitObjectLocaleReason reasonString_ is  not empty  ,will be request command
    reasonResId = 1;
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    PERMISSION_LOG_INFO(LABEL, "onResult_ : %{public}d,", callback->onResult_);

    EXPECT_EQ(callback->onResult_, 99);
    EXPECT_EQ(callback->onCancel_, 100);
    EXPECT_EQ(callback->onTimeOut_, 100);
}
// timeout run,but  already handle
HWTEST_F(RequestPermissionsFromRemoteTest, request_permissionsservice_from_remote_test_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_permissionsservice_from_remote_test_008";
    auto timeFast = std::chrono::milliseconds(64000);
    std::vector<std::string> permissions;
    permissions.push_back(Constant::LOCATION);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("bundleName");
    std::string deviceId("deviceId");
    int32_t reasonResId = 1;
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    //  case now  InitObjectLocaleReason reasonString_ is  not empty  ,will be request command
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    PERMISSION_LOG_INFO(LABEL, "onResult_ : %{public}d,", callback->onResult_);

    EXPECT_EQ(callback->onResult_, 99);
    EXPECT_EQ(callback->onCancel_, 100);
    EXPECT_EQ(callback->onTimeOut_, 100);
}
// timeout
HWTEST_F(RequestPermissionsFromRemoteTest, request_permissionsservice_from_remote_test_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_permissionsservice_from_remote_test_009";
    auto timeFast = std::chrono::milliseconds(64000);
    std::vector<std::string> permissions;
    permissions.push_back(Constant::LOCATION);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("bundleName");
    std::string deviceId("deviceIdTimeout");
    ResourceSwitch::GetInstance().OnRemoteOnline(deviceId);
    ResourceSwitchCache::GetInstance().SetSwitchStatus(
        deviceId, Constant::LOCATION, Constant::RESOURCE_SWITCH_STATUS_ALLOWED);
    int32_t reasonResId = 1;
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    PERMISSION_LOG_INFO(LABEL, "onResult_ : %{public}d,", callback->onResult_);

    EXPECT_EQ(callback->onResult_, 100);
    EXPECT_EQ(callback->onCancel_, 100);
    EXPECT_EQ(callback->onTimeOut_, 99);
}
// requestResultForCancel requestId  is not exist and have been processed
HWTEST_F(RequestPermissionsFromRemoteTest, request_result_for_cancel_test_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_result_for_cancel_test_001";
    auto timeFast = std::chrono::milliseconds(2000);
    std::vector<std::string> permissions;
    permissions.push_back(Constant::LOCATION);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("bundleName");
    std::string deviceId("deviceIdCancelExit");
    int32_t reasonResId = 1;
    ResourceSwitch::GetInstance().OnRemoteOnline(deviceId);
    ResourceSwitchCache::GetInstance().SetSwitchStatus(
        deviceId, Constant::LOCATION, Constant::RESOURCE_SWITCH_STATUS_ALLOWED);
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    // will sleep 60s
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    std::string requestId = "requestId";
    RequestRemotePermission::GetInstance().RequestResultForCancel(requestId);

    EXPECT_EQ(callback->onResult_, 100);
    EXPECT_EQ(callback->onCancel_, 100);
    EXPECT_EQ(callback->onTimeOut_, 100);
}
// requestResultForCancel
HWTEST_F(RequestPermissionsFromRemoteTest, request_result_for_cancel_test_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "request_result_for_cancel_test_002";
    auto timeFast = std::chrono::milliseconds(2000);
    std::vector<std::string> permissions;
    permissions.push_back(Constant::LOCATION);
    OHOS::sptr<TestCallbackA> callback(new TestCallbackA());
    std::string nodeId("networkId");
    std::string bundleName("bundleName");
    std::string deviceId("deviceIdCancel ");
    int32_t reasonResId = 1;
    ResourceSwitch::GetInstance().OnRemoteOnline(deviceId);
    ResourceSwitchCache::GetInstance().SetSwitchStatus(
        deviceId, Constant::LOCATION, Constant::RESOURCE_SWITCH_STATUS_ALLOWED);
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        nodeId, "universallyUniqueId", deviceId, "deviceName", "deviceType");
    // will sleep 60s
    service->RequestPermissionsFromRemote(permissions, callback, nodeId, bundleName, reasonResId);
    std::this_thread::sleep_for(timeFast);
    // cancel task
    RequestRemotePermission::GetInstance().RequestResultForCancel(deviceId);
    std::this_thread::sleep_for(timeFast);

    EXPECT_EQ(callback->onResult_, 100);
    EXPECT_EQ(callback->onCancel_, 99);
    EXPECT_EQ(callback->onTimeOut_, 100);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS