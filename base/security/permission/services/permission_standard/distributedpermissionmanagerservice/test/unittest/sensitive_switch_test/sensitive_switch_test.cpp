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
#include "constant.h"
#define private public
#include "distributed_permission_manager_service.h"
#include "resource_switch.h"
#include "resource_switch_cache.h"
#include "resource_switch_local.h"
#include "resource_switch_remote.h"
#include "sensitive_resource_switch_setting.h"
#include "ipc_skeleton.h"
using namespace testing::ext;
namespace OHOS {
std::string IPCSkeleton::localDeviceId_ = "1004";
namespace Security {
namespace Permission {
namespace {
static const std::string DEVICE_("test-device-id-001");
}  // namespace

class SensitiveSwitchTest : public testing::Test {
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
    {
        ResourceSwitch::GetInstance().Initialize();
        int32_t time = 5;
        sleep(time);
    }
    void TearDown()
    {
        ResourceSwitchCache::GetInstance().cache_.clear();
    }
};

HWTEST_F(SensitiveSwitchTest, SensitiveSwitch_OnLocalChange_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SensitiveSwitch_OnLocalChange_001";

    ResourceSwitch::GetInstance().Initialize();
    sleep(5);
    EXPECT_GE(ResourceSwitchCache::GetInstance().cache_.size(), std::size_t(1));

    // case 1 (0 in param)
    ResourceSwitch::GetInstance().OnLocalChange({});
    EXPECT_TRUE(ResourceSwitchCache::GetInstance().GetSwitchStatus(Constant::CAMERA));

    // case 2 (1 in param)
    ResourceSwitch::GetInstance().OnLocalChange({Constant::CAMERA});
    EXPECT_TRUE(ResourceSwitchCache::GetInstance().GetSwitchStatus(Constant::CAMERA));

    // case 3 (multiple params)
    ResourceSwitch::GetInstance().OnLocalChange({Constant::READ_HEALTH_DATA, Constant::LOCATION});
    EXPECT_TRUE(ResourceSwitchCache::GetInstance().GetSwitchStatus(Constant::READ_HEALTH_DATA));
    EXPECT_TRUE(ResourceSwitchCache::GetInstance().GetSwitchStatus(Constant::LOCATION));
}

HWTEST_F(SensitiveSwitchTest, SensitiveSwitch_OnRemoteOnlineOffLine_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SensitiveSwitch_OnRemoteOnlineOffLine_001";

    // case 1 OnLine(invalid deviceid)
    ResourceSwitch::GetInstance().OnRemoteOnline("");
    EXPECT_EQ(ResourceSwitchCache::GetInstance().GetSwitchSetting(DEVICE_), nullptr);

    // case 2 OnLine(local deviceid)
    ResourceSwitch::GetInstance().OnRemoteOnline(IPCSkeleton::GetLocalDeviceID());
    EXPECT_EQ(ResourceSwitchCache::GetInstance().GetSwitchSetting(DEVICE_), nullptr);

    // case 3 OnLine
    ResourceSwitch::GetInstance().OnRemoteOnline(DEVICE_);
    EXPECT_NE(ResourceSwitchCache::GetInstance().GetSwitchSetting(DEVICE_), nullptr);

    // case 4 OffLine(invalid deviceid)
    ResourceSwitch::GetInstance().OnRemoteOffline("");
    EXPECT_NE(ResourceSwitchCache::GetInstance().GetSwitchSetting(DEVICE_), nullptr);

    // case 5 OffLine(local deviceid)
    ResourceSwitch::GetInstance().OnRemoteOffline(IPCSkeleton::GetLocalDeviceID());
    EXPECT_NE(ResourceSwitchCache::GetInstance().GetSwitchSetting(DEVICE_), nullptr);

    // case 6 OffLine
    ResourceSwitch::GetInstance().OnRemoteOffline(DEVICE_);
    EXPECT_EQ(ResourceSwitchCache::GetInstance().GetSwitchSetting(DEVICE_), nullptr);
}

HWTEST_F(SensitiveSwitchTest, SensitiveSwitch_OnRemoteChange_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SensitiveSwitch_OnRemoteChange_001";

    // case 1 OnRemoteChange(invalid deviceid)
    ResourceSwitch::GetInstance().OnRemoteChange("");
    EXPECT_EQ(ResourceSwitchCache::GetInstance().GetSwitchSetting(DEVICE_), nullptr);

    // case 2 OnRemoteChange(local deviceid)
    ResourceSwitch::GetInstance().OnRemoteChange(IPCSkeleton::GetLocalDeviceID());
    EXPECT_EQ(ResourceSwitchCache::GetInstance().GetSwitchSetting(DEVICE_), nullptr);

    // case 3 OnRemoteChange
    ResourceSwitch::GetInstance().OnRemoteChange(DEVICE_);
    EXPECT_NE(ResourceSwitchCache::GetInstance().GetSwitchSetting(DEVICE_), nullptr);
}

HWTEST_F(SensitiveSwitchTest, SensitiveSwitch_GetLocalSensitiveResourceSwitch_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SensitiveSwitch_OnRemoteChange_001";

    // case 1 invalid permissionname, status RESOURCE_SWITCH_STATUS_DENIED
    int isAllowed = ResourceSwitch::GetInstance().GetLocalSensitiveResourceSwitch("");
    EXPECT_EQ(isAllowed, Constant::RESOURCE_SWITCH_STATUS_DENIED);

    // case 2 status RESOURCE_SWITCH_STATUS_ALLOWED
    ResourceSwitchCache::GetInstance().SetSwitchStatus(Constant::CAMERA, Constant::RESOURCE_SWITCH_STATUS_ALLOWED);
    isAllowed = ResourceSwitch::GetInstance().GetLocalSensitiveResourceSwitch(Constant::CAMERA);
    EXPECT_EQ(isAllowed, Constant::RESOURCE_SWITCH_STATUS_ALLOWED);

    // case 3
    ResourceSwitchCache::GetInstance().SetSwitchStatus(Constant::CAMERA, Constant::RESOURCE_SWITCH_STATUS_DENIED);
    isAllowed = ResourceSwitch::GetInstance().GetLocalSensitiveResourceSwitch(Constant::CAMERA);
    EXPECT_EQ(isAllowed, Constant::RESOURCE_SWITCH_STATUS_DENIED);
}

HWTEST_F(SensitiveSwitchTest, SensitiveSwitch_GetRemoteSensitiveResourceSwitch_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SensitiveSwitch_OnRemoteChange_001";

    ResourceSwitch::GetInstance().OnRemoteOnline(DEVICE_);

    // case 1 invalid deviceid, status RESOURCE_SWITCH_STATUS_DENIED
    int isAllowed = ResourceSwitch::GetInstance().GetRemoteSensitiveResourceSwitch("", Constant::CAMERA);
    EXPECT_EQ(isAllowed, Constant::RESOURCE_SWITCH_STATUS_DENIED);

    // case 2 invalid permissionname, status RESOURCE_SWITCH_STATUS_DENIED
    isAllowed = ResourceSwitch::GetInstance().GetRemoteSensitiveResourceSwitch(DEVICE_, "");
    EXPECT_EQ(isAllowed, Constant::RESOURCE_SWITCH_STATUS_DENIED);

    // case 3 status RESOURCE_SWITCH_STATUS_ALLOWED
    ResourceSwitchCache::GetInstance().SetSwitchStatus(
        DEVICE_, Constant::CAMERA, Constant::RESOURCE_SWITCH_STATUS_ALLOWED);
    isAllowed = ResourceSwitch::GetInstance().GetRemoteSensitiveResourceSwitch(DEVICE_, Constant::CAMERA);
    EXPECT_EQ(isAllowed, Constant::RESOURCE_SWITCH_STATUS_ALLOWED);

    // case 4
    ResourceSwitchCache::GetInstance().SetSwitchStatus(
        DEVICE_, Constant::CAMERA, Constant::RESOURCE_SWITCH_STATUS_DENIED);
    isAllowed = ResourceSwitch::GetInstance().GetRemoteSensitiveResourceSwitch(DEVICE_, Constant::CAMERA);
    EXPECT_EQ(isAllowed, Constant::RESOURCE_SWITCH_STATUS_DENIED);
}

HWTEST_F(SensitiveSwitchTest, SensitiveSwitch_UpdateSwitchSetting_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SensitiveSwitch_UpdateSwitchSetting_001";

    ResourceSwitchCache::GetInstance().cache_.clear();

    // case 1 switchsetting is nullptr
    ResourceSwitchCache::GetInstance().UpdateSwitchSetting(nullptr);
    EXPECT_EQ(ResourceSwitchCache::GetInstance().cache_.size(), std::size_t(0));

    // case 2 cache dont have same deviceid
    std::shared_ptr<SensitiveResourceSwitchSetting> setting = SensitiveResourceSwitchSetting::CreateBuilder()
                                                                  ->DeviceId(DEVICE_)
                                                                  .Camera(true)
                                                                  .HealthSensor(true)
                                                                  .Location(true)
                                                                  .Microphone(true)
                                                                  .Build();
    ResourceSwitchCache::GetInstance().UpdateSwitchSetting(setting);
    EXPECT_TRUE(ResourceSwitchCache::GetInstance().GetSwitchStatus(DEVICE_, Constant::CAMERA));

    // case 3 cache dont have same deviceid
    setting->SetSwitchStatus(Constant::CAMERA, false);
    ResourceSwitchCache::GetInstance().UpdateSwitchSetting(setting);
    EXPECT_FALSE(ResourceSwitchCache::GetInstance().GetSwitchStatus(DEVICE_, Constant::CAMERA));
}

HWTEST_F(SensitiveSwitchTest, SensitiveSwitch_RemoveSwitchSetting_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SensitiveSwitch_RemoveSwitchSetting_001";

    // case 1 switchsetting is nullptr
    ResourceSwitchCache::GetInstance().RemoveSwitchSetting(DEVICE_);
    EXPECT_GE(ResourceSwitchCache::GetInstance().cache_.size(), std::size_t(1));

    // case 2 cache dont have same deviceid
    std::shared_ptr<SensitiveResourceSwitchSetting> setting = SensitiveResourceSwitchSetting::CreateBuilder()
                                                                  ->DeviceId(DEVICE_)
                                                                  .Camera(true)
                                                                  .HealthSensor(true)
                                                                  .Location(true)
                                                                  .Microphone(true)
                                                                  .Build();
    ResourceSwitchCache::GetInstance().UpdateSwitchSetting(setting);
    EXPECT_GE(ResourceSwitchCache::GetInstance().cache_.size(), std::size_t(2));

    ResourceSwitchCache::GetInstance().RemoveSwitchSetting(DEVICE_);
    EXPECT_GE(ResourceSwitchCache::GetInstance().cache_.size(), std::size_t(1));
}

HWTEST_F(SensitiveSwitchTest, SensitiveSwitch_RegisterContentObserver_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SensitiveSwitch_RegisterContentObserver_001";

    // case 1 callback is nullptr
    ResourceSwitchLocal::GetInstance().hasRegistered_ = false;
    ResourceSwitchLocal::GetInstance().RegisterContentObserver(nullptr);
    EXPECT_FALSE(ResourceSwitchLocal::GetInstance().hasRegistered_);

    // case 2
    ResourceSwitchLocal::GetInstance().RegisterContentObserver(
        [](std::initializer_list<std::string> sensitiveResources) {
            GTEST_LOG_(INFO) << "SensitiveSwitch_RegisterContentObserver_001 callback";
        });
    EXPECT_TRUE(ResourceSwitchLocal::GetInstance().hasRegistered_);

    // case 3 has registed
    ResourceSwitchLocal::GetInstance().RegisterContentObserver(
        [](std::initializer_list<std::string> sensitiveResources) {
            GTEST_LOG_(INFO) << "SensitiveSwitch_RegisterContentObserver_001 callback";
        });
    EXPECT_TRUE(ResourceSwitchLocal::GetInstance().hasRegistered_);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS