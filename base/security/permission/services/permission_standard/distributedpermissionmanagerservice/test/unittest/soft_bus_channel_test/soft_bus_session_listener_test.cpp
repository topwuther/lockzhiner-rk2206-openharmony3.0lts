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
#include "rpc_channel.h"
#define private public
#include "soft_bus_session_listener.h"

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
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusSessionListenerTest"};
static const std::string TARGET_DEVICE_("test-device-id-002");
static const std::string TARGET_NETWORK_ID_(TARGET_DEVICE_ + ":network-id-001");
static const std::string TARGET_UUID_(TARGET_NETWORK_ID_ + ":uuid-001");
static const std::string TARGET_UDID_(TARGET_NETWORK_ID_ + ":udid-001");
static const std::string TARGET_DEVICE_ID_(TARGET_UDID_);
}  // namespace

class SoftBusSessionListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        // 2.assume target device info synchroized
        DeviceInfoManager::GetInstance().AddDeviceInfo(
            TARGET_NETWORK_ID_, TARGET_UUID_, TARGET_UDID_, TARGET_DEVICE_, std::to_string(1));
        // should got it
        DeviceInfo info;
        bool result = DeviceInfoManager::GetInstance().GetDeviceInfo(TARGET_DEVICE_ID_, DeviceIdType::UNKNOWN, info);
        EXPECT_EQ(result, true);
        EXPECT_EQ(info.deviceId.networkId, TARGET_NETWORK_ID_);

        std::string id = "deviceid-1";
        DeviceInfoManager::GetInstance().AddDeviceInfo(id, id, id, id, std::to_string(1));
        result = DeviceInfoManager::GetInstance().GetDeviceInfo(id, DeviceIdType::UNKNOWN, info);
        EXPECT_EQ(result, true);
        EXPECT_EQ(info.deviceId.networkId, id);

        id = "deviceid-2";
        DeviceInfoManager::GetInstance().AddDeviceInfo(id, id, id, id, std::to_string(1));
        result = DeviceInfoManager::GetInstance().GetDeviceInfo(id, DeviceIdType::UNKNOWN, info);
        EXPECT_EQ(result, true);
        EXPECT_EQ(info.deviceId.networkId, id);
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
 * Function: SoftBusSessionListener
 * SubFunction: OnSessionOpened
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify session opened and closed
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListener_OnSessionOpened_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusSessionListener_OnSessionOpened_001");
    SoftBusSessionListener instance;
    {
        // opened with error result
        int sessionId = 1;
        int result = Constant::SUCCESS + 1;
        int code = instance.OnSessionOpened(sessionId, result);
        EXPECT_TRUE(-1 == code);
    }
    {
        // opened with error session id
        int sessionId = -1;
        int result = Constant::SUCCESS;
        int code = instance.OnSessionOpened(sessionId, result);
        EXPECT_TRUE(-1 == code);
    }
    {
        // opened with error session peer name
        int sessionId = 100;
        int result = Constant::SUCCESS;
        int code = instance.OnSessionOpened(sessionId, result);
        EXPECT_TRUE(-1 == code);
    }
    {
        // opened with error session peer name
        int sessionId = 1;
        int result = Constant::SUCCESS;
        int code = instance.OnSessionOpened(sessionId, result);
        EXPECT_TRUE(0 == code);
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusSessionListener
 * SubFunction: OnSessionClosed
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify session opened and closed
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListener_OnSessionClosed_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusSessionListener_OnSessionClosed_001");
    SoftBusSessionListener instance;
    {
        // check log
        int sessionId = 1;
        instance.OnSessionClosed(sessionId);
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusSessionListener
 * SubFunction: OnMessageReceived
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify message received
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListener_OnMessageReceived_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusSessionListener_OnMessageReceived_001");
    SoftBusSessionListener instance;
    {
        // check log
        int sessionId = 1;
        char data[] = "test-data-rece_ived_";
        int len = 21;
        instance.OnMessageReceived(sessionId, data, len);
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusSessionListener
 * SubFunction: OnBytesReceived
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify bytes received
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListener_OnBytesReceived_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusSessionListener_OnBytesReceived_001");
    SoftBusSessionListener instance;
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusSessionListener_OnBytesReceived_001-1");
        // no data, check log
        int sessionId = 1;
        char data[] = "";
        int len = 0;
        instance.OnBytesReceived(sessionId, data, len);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusSessionListener_OnBytesReceived_001-2");
        // invalid session id, check log
        int sessionId = -1;
        char data[] = "a";
        int len = 2;
        instance.OnBytesReceived(sessionId, data, len);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusSessionListener_OnBytesReceived_001-3");
        // invalid deviceid, check log
        int sessionId = 9;
        char data[] = "a";
        int len = 2;
        instance.OnBytesReceived(sessionId, data, len);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusSessionListener_OnBytesReceived_001-4");
        // create channel
        auto executor = RemoteCommandManager::GetInstance().GetOrCreateRemoteCommandExecutor("deviceid-1");
        executor->CreateChannelIfNeeded();

        // prepare compressed data
        GetUidPermissionCommand cmd("");
        const std::string type = "request";
        const std::string id = "message-unique-id-001";
        const std::string commandName = "GetUidPermissionCommand";
        const std::string jsonPayload = cmd.ToJsonPayload();
        int compressedLength = 10240;
        unsigned char compressedBytes[compressedLength];

        auto channel = (SoftBusChannel *)executor->GetChannel().get();
        channel->PrepareBytes(type, id, commandName, jsonPayload, compressedBytes, compressedLength);

        // PERMISSION_LOG_INFO(LABEL,
        //     "compressed length: %{public}d, from %{public}u, result: %{public}d",
        //     compressedLength,
        //     jsonPayload.length(),
        //     code);

        // check log
        int sessionId = 1;
        instance.OnBytesReceived(sessionId, compressedBytes, compressedLength);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS