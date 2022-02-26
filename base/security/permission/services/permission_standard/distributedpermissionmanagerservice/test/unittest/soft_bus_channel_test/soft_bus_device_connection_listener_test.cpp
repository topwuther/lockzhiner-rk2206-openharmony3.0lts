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
#include <string>
#include "constant.h"

#define private public
#include "rpc_channel.h"
#define private public
#include "soft_bus_device_connection_listener.h"
#include "soft_bus_manager.h"

#include "soft_bus_channel.h"

#include "zip_util.h"
#include "device_info_manager.h"
#include "remote_command_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusDeviceConnectionListenerTest"};
static const std::string TARGET_DEVICE_("test-device-id-001");
static const std::string TARGET_NETWORK_ID_(TARGET_DEVICE_ + ":network-id-001");
static const std::string TARGET_UUID_(TARGET_NETWORK_ID_ + ":uuid-001");
static const std::string TARGET_UDID_(TARGET_NETWORK_ID_ + ":udid-001");
static const std::string TARGET_DEVICE_ID_(TARGET_UDID_);

static int SLEEP_200 = 200;
static int SLEEP_100 = 100;
}  // namespace
class SoftBusDeviceConnectionListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        // simulator register
        ::RegNodeDeviceStateCb("", nullptr);
        SoftBusManager::GetInstance().Initialize();
        int count = 1;
        int countLimit = 10;
        while (SoftBusManager::GetInstance().isSoftBusServiceBindSuccess_ == false && count++ < countLimit) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_200));
            PERMISSION_LOG_DEBUG(LABEL, "init...");
        }
    }
    static void TearDownTestCase(void)
    {
        SoftBusManager::GetInstance().Destroy();
        // simulator unregister
        ::UnregNodeDeviceStateCb(nullptr);
        int sessionLimit = 20;
        for (int i = 0; i < sessionLimit + 1; i++) {
            // 1: mock session id
            ::CloseSession(1);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_200));

        DeviceInfoRepository::GetInstance().Clear();
        DeviceInfoManager::GetInstance().RemoveAllRemoteDeviceInfo();
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_200));

        RemoteCommandManager::GetInstance().Clear();

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_200));
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * Feature: DPMS
 * Function: SoftBusDeviceConnectionListener
 * SubFunction: OnDeviceOnline
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify device online callback
 */
HWTEST_F(SoftBusDeviceConnectionListenerTest, SoftBusDeviceConnectionListener_OnDeviceOnline_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusDeviceConnectionListener_OnDeviceOnline_001");
    SoftBusDeviceConnectionListener instance;
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusDeviceConnectionListener_OnDeviceOnline_001-2");

        DeviceInfoRepository::GetInstance().Clear();

        // cannot get uuid/udid: invalid networkId
        DmDeviceInfo info;
        std::strcpy(info.deviceId, "");
        std::strcpy(info.deviceName, "test-device-id-001");
        info.deviceTypeId = DistributedHardware::DMDeviceType::DEVICE_TYPE_PHONE;
        instance.OnDeviceOnline(info);
        {
            // check: local device cache
            auto list = DeviceInfoRepository::GetInstance().ListDeviceInfo();
            for (auto it = list.begin(); it != list.end(); it++) {
                PERMISSION_LOG_DEBUG(LABEL, "device network id: %{public}s ", it->deviceId.networkId.c_str());
            }
            int size = list.size();
            EXPECT_EQ(size, 0);
        }
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusDeviceConnectionListener_OnDeviceOnline_001-3");

        {
            DeviceInfoRepository::GetInstance().Clear();
            auto list = DeviceInfoRepository::GetInstance().ListDeviceInfo();
            int size = list.size();
            EXPECT_EQ(size, 0);
        }

        // success: cache device
        {
            DeviceInfoManager::GetInstance().AddDeviceInfo(TARGET_UUID_, TARGET_UUID_, "1", "2", std::to_string(1));

            auto list = DeviceInfoRepository::GetInstance().ListDeviceInfo();
            for (auto it = list.begin(); it != list.end(); it++) {
                PERMISSION_LOG_DEBUG(LABEL, "it1: device network id: %{public}s ", it->deviceId.networkId.c_str());
            }
            DeviceInfo info;
            int result = DeviceInfoManager::GetInstance().GetDeviceInfo(TARGET_UUID_, DeviceIdType::UNKNOWN, info);
            EXPECT_EQ(result, true);
            EXPECT_EQ(info.deviceId.networkId, TARGET_UUID_);
        }

        DmDeviceInfo info;
        std::strcpy(info.deviceId, "test-device-id-001:network-id-001");  // old : networkId
        std::strcpy(info.deviceName, "test-device-id-001");
        info.deviceTypeId = DistributedHardware::DMDeviceType::DEVICE_TYPE_PHONE;
        instance.OnDeviceOnline(info);
        // wait notify callback to action
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));

        {
            // check: local device cache
            auto list = DeviceInfoRepository::GetInstance().ListDeviceInfo();
            for (auto it = list.begin(); it != list.end(); it++) {
                PERMISSION_LOG_DEBUG(LABEL, "it2: device network id: %{public}s ", it->deviceId.networkId.c_str());
            }
            int size = list.size();
            EXPECT_EQ(size, 1);
            DeviceInfo info;
            int result = DeviceInfoManager::GetInstance().GetDeviceInfo(TARGET_UUID_, DeviceIdType::UNKNOWN, info);
            EXPECT_EQ(result, true);
            EXPECT_EQ(info.deviceId.networkId, TARGET_NETWORK_ID_);
        }
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusDeviceConnectionListener
 * SubFunction: OnDeviceOffline
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify device offline callback
 */
HWTEST_F(SoftBusDeviceConnectionListenerTest, SoftBusDeviceConnectionListener_OnDeviceOffline_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusDeviceConnectionListener_OnDeviceOffline_001");
    SoftBusDeviceConnectionListener instance;
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusDeviceConnectionListener_OnDeviceOffline_001-2");

        DeviceInfoRepository::GetInstance().Clear();

        // cannot get uuid/udid
        DmDeviceInfo info;
        std::strcpy(info.deviceId, "test-device-id-001:network-id-001");
        std::strcpy(info.deviceName, "test-device-id-001");
        info.deviceTypeId = DistributedHardware::DMDeviceType::DEVICE_TYPE_PHONE;
        instance.OnDeviceOffline(info);

        // check: local device cache
        auto list = DeviceInfoRepository::GetInstance().ListDeviceInfo();
        for (auto it = list.begin(); it != list.end(); it++) {
            PERMISSION_LOG_DEBUG(LABEL, "device network id: %{public}s ", it->deviceId.networkId.c_str());
        }
        int size = list.size();
        EXPECT_EQ(size, 0);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusDeviceConnectionListener_OnDeviceOffline_001-3");
        {
            DeviceInfoRepository::GetInstance().Clear();

            // cache device
            DeviceInfoManager::GetInstance().AddDeviceInfo(TARGET_UUID_, TARGET_UUID_, "1", "2", std::to_string(1));

            auto list = DeviceInfoRepository::GetInstance().ListDeviceInfo();
            for (auto it = list.begin(); it != list.end(); it++) {
                PERMISSION_LOG_DEBUG(LABEL, "it1: device network id: %{public}s ", it->deviceId.networkId.c_str());
            }
            DeviceInfo info;
            int result = DeviceInfoManager::GetInstance().GetDeviceInfo(TARGET_UUID_, DeviceIdType::UNKNOWN, info);
            EXPECT_EQ(result, true);
            EXPECT_EQ(info.deviceId.networkId, TARGET_UUID_);
        }

        // success: cache device
        DmDeviceInfo info;
        std::strcpy(info.deviceId, "test-device-id-001:network-id-001");
        std::strcpy(info.deviceName, "test-device-id-001");
        info.deviceTypeId = DistributedHardware::DMDeviceType::DEVICE_TYPE_PHONE;
        {
            // online
            instance.OnDeviceOnline(info);
            // wait notify callback to action
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            auto list = DeviceInfoRepository::GetInstance().ListDeviceInfo();
            for (auto it = list.begin(); it != list.end(); it++) {
                PERMISSION_LOG_DEBUG(LABEL, "it2: device network id: %{public}s ", it->deviceId.networkId.c_str());
            }
            int size = list.size();
            EXPECT_EQ(size, 1);
        }

        instance.OnDeviceOffline(info);
        // wait notify callback to action
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));

        {
            // check: local device cache
            auto list = DeviceInfoRepository::GetInstance().ListDeviceInfo();
            for (auto it = list.begin(); it != list.end(); it++) {
                PERMISSION_LOG_DEBUG(LABEL, "it3: device network id: %{public}s ", it->deviceId.networkId.c_str());
            }
            int size = list.size();
            EXPECT_EQ(size, 0);
        }
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusDeviceConnectionListener
 * SubFunction: OnDeviceInfoChanged
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify device info changed callback
 */
HWTEST_F(SoftBusDeviceConnectionListenerTest, SoftBusDeviceConnectionListener_OnDeviceInfoChanged_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusDeviceConnectionListener_OnDeviceInfoChanged_001");
    SoftBusDeviceConnectionListener instance;
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusDeviceConnectionListener_OnDeviceOnline_001-2");
        // cannot get uuid/udid
        DmDeviceInfo info;
        std::strcpy(info.deviceId, "test-device-id-001:network-id-001");
        std::strcpy(info.deviceName, "test-device-id-001");
        info.deviceTypeId = DistributedHardware::DMDeviceType::DEVICE_TYPE_PHONE;

        // check log
        instance.OnDeviceChanged(info);
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS