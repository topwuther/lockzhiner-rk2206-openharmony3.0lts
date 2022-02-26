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
#include "soft_bus_channel.h"
#include "soft_bus_manager.h"

#undef private
#include "distributed_permission_manager_service.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusChannelTest"};
}
namespace {
static const std::string INVALID_DEVICE_("test-device-id-invalid");
static const std::string SELF_DEVICE_("test-device-id-001");
static const std::string TARGET_DEVICE_("test-device-id-002");
static const std::string TARGET_NETWORK_ID_(TARGET_DEVICE_ + ":network-id-001");
static const std::string TARGET_UUID_(TARGET_NETWORK_ID_ + ":uuid-001");
static const std::string TARGET_UDID_(TARGET_NETWORK_ID_ + ":udid-001");
static const std::string TARGET_DEVICE_ID_(TARGET_UDID_);
static const int RETRY_TIMES = 30;
}  // namespace

class SoftBusChannelTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        // init service
        // PermissionBmsManager will register a permission changed listener on MonitorManager.
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStop();
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStart();

        // simulator register
        ::RegNodeDeviceStateCb("", nullptr);

        // wait service to init;

        // 1.assume SoftBusManager inited
        int count = 1;
        int timeoutInterval = 100;
        while (SoftBusManager::GetInstance().isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeoutInterval));
        }
        // should be inited.
        ASSERT_EQ(SoftBusManager::GetInstance().isSoftBusServiceBindSuccess_, true);

        // 2.assume target device info synchroized
        DeviceInfoManager::GetInstance().AddDeviceInfo(
            TARGET_NETWORK_ID_, TARGET_UUID_, TARGET_UDID_, TARGET_DEVICE_, std::to_string(1));
        // should got it
        DeviceInfo info;
        bool result = DeviceInfoManager::GetInstance().GetDeviceInfo(TARGET_DEVICE_ID_, DeviceIdType::UNKNOWN, info);
        EXPECT_EQ(result, true);
        EXPECT_EQ(info.deviceId.networkId, TARGET_NETWORK_ID_);
    }
    static void TearDownTestCase(void)
    {
        // simulator unregister
        ::UnregNodeDeviceStateCb(nullptr);
        int sessionLimit = 20;
        for (int i = 0; i < sessionLimit + 1; i++) {
            // 1: mock session id
            ::CloseSession(1);
        }
    }
    void SetUp()
    {}
    void TearDown()
    {}
    std::atomic<bool> simulatorExecuted;
};

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: Construct
 * FunctionPoints: Init
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_Construct_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_Construct_001");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: BuildConnection
 * FunctionPoints: CancelCloseConnectionIfNeeded, OpenSession
 * EnvConditions: Open session with soft bus.
 * CaseDescription: Verify open session
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_BuildConnection_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_BuildConnection_001");
    {
        auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
        int code = channel->BuildConnection();
        EXPECT_EQ(code, 0);
        channel->CloseConnection();
    }

    {
        // when open session failed, return -1.
        auto channel = std::make_shared<SoftBusChannel>(INVALID_DEVICE_);
        int code = channel->BuildConnection();
        EXPECT_EQ(code, -1);
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: BuildConnection
 * FunctionPoints: CancelCloseConnectionIfNeeded, OpenSession
 * EnvConditions: Open session with soft bus.
 * CaseDescription: verify open session when a session exist.
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_BuildConnection_002, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_BuildConnection_002");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        // open session at first call
        int code = channel->BuildConnection();
        EXPECT_EQ(code, 0);
    }
    {
        // will not open again
        int code = channel->BuildConnection();
        EXPECT_EQ(code, 0);
    }

    channel->CloseConnection();
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: BuildConnection
 * FunctionPoints: CancelCloseConnectionIfNeeded, OpenSession
 * EnvConditions: Open session with soft bus.
 * CaseDescription: verify open session just after closing.
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_BuildConnection_003, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_BuildConnection_003");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        // open session at first call
        int code = channel->BuildConnection();
        EXPECT_EQ(code, 0);
    }

    // delay close session
    // will add a delay task
    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    channel->CloseConnection();
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_BuildConnection_003: re-build");
        // will cancel delay closing, and remove the delay task, then open session
        int code = channel->BuildConnection();
        EXPECT_EQ(code, 0);
    }

    channel->CloseConnection();
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: BuildConnection
 * FunctionPoints: CancelCloseConnectionIfNeeded, OpenSession
 * EnvConditions: Open session with soft bus.
 * CaseDescription: verify open session after closed.
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_BuildConnection_004, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_BuildConnection_004");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        // open session at first call
        int code = channel->BuildConnection();
        EXPECT_EQ(code, 0);
    }

    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    {
        // delay close session
        // will add a delay task
        channel->CloseConnection();
    }

    {
        // open session
        int code = channel->BuildConnection();
        EXPECT_EQ(code, 0);
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: Compress
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify compress a json string to bytes.
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_Compress_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_Compress_001");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_Compress_001-1");
        std::string json = "abcdefg-0123456789";
        int compressedLength = 1024;
        unsigned char compressedBytes[compressedLength];
        int code = channel->Compress(json, compressedBytes, compressedLength);
        PERMISSION_LOG_DEBUG(LABEL, "compressed length: %{public}d", compressedLength);
        EXPECT_EQ(code, 0);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_Compress_001-2");
        // compressedLength too small
        std::string json = "abcdefg-0123456789";
        int compressedLength = 10;
        unsigned char compressedBytes[compressedLength];
        int code = channel->Compress(json, compressedBytes, compressedLength);
        EXPECT_EQ(code, -2);
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: Compress and Decompress
 * FunctionPoints:
 * EnvConditions:
 * CaseDescription: verify compress a json string to bytes and decompress.
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_Compress_002, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_Compress_002");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_Compress_002-1");
        std::string json = "abcdefg-0123456789";
        int compressedLength = 1024;
        unsigned char compressedBytes[compressedLength];
        int code = channel->Compress(json, compressedBytes, compressedLength);

        PERMISSION_LOG_DEBUG(LABEL, "compressed length: %{public}d", compressedLength);
        EXPECT_EQ(code, 0);

        std::string dec = channel->Decompress(compressedBytes, compressedLength);
        PERMISSION_LOG_DEBUG(LABEL, "decompressed: %{public}s", dec.c_str());
        EXPECT_EQ(json, dec);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_Compress_002-2");
        std::string json = "{\"name\":\"test\",\"number\":100,\"flag\":true,\"array\":[0,1,2]}";
        int compressedLength = 1024;
        unsigned char compressedBytes[compressedLength];
        int code = channel->Compress(json, compressedBytes, compressedLength);

        PERMISSION_LOG_DEBUG(LABEL, "compressed length: %{public}d", compressedLength);
        EXPECT_EQ(code, 0);

        std::string dec = channel->Decompress(compressedBytes, compressedLength);

        PERMISSION_LOG_DEBUG(LABEL, "decompressed: %{public}s", dec.c_str());
        EXPECT_EQ(json, dec);
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: PrepareBytes
 * FunctionPoints: Deserialize json string to command object, and compress it to bytes.
 * EnvConditions:
 * CaseDescription: verify prepare bytes。
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_PrepareBytes_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_PrepareBytes_001");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        const std::string type = "request";
        const std::string id = "message-unique-id-001";
        const std::string commandName = "GetUidPermissionCommand";
        const std::string jsonPayload = "\"{\"name\":\"test\",\"number\":100,\"flag\":true,\"array\":[0,1,2]}\"";
        int compressedLength = 1024;
        unsigned char compressedBytes[compressedLength];
        int code = channel->PrepareBytes(type, id, commandName, jsonPayload, compressedBytes, compressedLength);
        EXPECT_EQ(code, 0);

        std::string dec = channel->Decompress(compressedBytes, compressedLength);

        PERMISSION_LOG_DEBUG(LABEL, "decompressed: %{public}s", dec.c_str());
        EXPECT_TRUE(dec != "");
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: IsSessionAvailable
 * FunctionPoints: check if session is opened
 * EnvConditions:
 * CaseDescription: verify check if session is opened or closed correctly.
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_IsSessionAvailable_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_IsSessionAvailable_001");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_IsSessionAvailable_001-1");
        bool ret = channel->IsSessionAvailable();
        EXPECT_FALSE(ret);
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_IsSessionAvailable_001-2");
        channel->BuildConnection();
        bool ret = channel->IsSessionAvailable();
        EXPECT_TRUE(ret);
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_IsSessionAvailable_001: close connection");
        channel->CloseConnection();
        // will execute delayed task
        const long WAIT_SESSION_CLOSE_MILLISECONDS = 5000L;
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_SESSION_CLOSE_MILLISECONDS + 1000));
        bool ret = channel->IsSessionAvailable();
        EXPECT_FALSE(ret);
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: CheckSessionMayReopenLocked
 * FunctionPoints: check session and open it if closed.
 * EnvConditions:
 * CaseDescription: verify check if session is opened, open it if closed.
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_CheckSessionMayReopenLocked_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_CheckSessionMayReopenLocked_001");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        bool ret1 = channel->IsSessionAvailable();
        EXPECT_FALSE(ret1);

        // will open session
        int ret2 = channel->CheckSessionMayReopenLocked();
        EXPECT_TRUE(ret2 == 0);
    }
    {
        // session opened
        bool ret3 = channel->IsSessionAvailable();
        EXPECT_TRUE(ret3);
    }
    {
        // will not open session if opened.
        int ret = channel->CheckSessionMayReopenLocked();
        EXPECT_TRUE(ret == 0);
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: SendRequestBytes
 * FunctionPoints: check session, send reqeust bytes to soft bus.
 * EnvConditions:
 * CaseDescription: verify send request bytes
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_SendRequestBytes_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_SendRequestBytes_001");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        channel->BuildConnection();
        std::string bytesString = "abcdefg-0123456789";
        const int bytesLength = bytesString.length();
        int code = channel->SendRequestBytes((unsigned char *)bytesString.c_str(), bytesLength);
        EXPECT_TRUE(code == 0);

        channel->CloseConnection();
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: HandleRequest
 * FunctionPoints: create command from json, execute, response.
 * EnvConditions:
 * CaseDescription: verify handle request
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_HandleRequest_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleRequest_001");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    channel->BuildConnection();
    int session = 1;

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleRequest_001-1 begin");
        // getuid command need a uid property.
        const std::string id = "message-unique-id-001";
        const std::string commandName = "GetUidPermissionCommand";
        // for mock, will get a uid 100
        const std::string jsonPayload = "{[100]}";
        channel->HandleRequest(session, id, commandName, jsonPayload);
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleRequest_001-1 end");
        // watch logs to confirm response contents
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleRequest_001-2 begin");
        // not a valid command name, direct send back error response
        const std::string id = "message-unique-id-001";
        const std::string commandName = "GetUidPermissionCommand-XXX";
        // for mock, will get a uid 100
        // const std::string jsonPayload =
        //     "{\"name\":\"test\",\"number\":100,\"flag\":true,\"array\":[0,1,2],\"uid\":\"test-device-id-001-uid\"}";
        const std::string jsonPayload = "{[100]}";
        channel->HandleRequest(session, id, commandName, jsonPayload);
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleRequest_001-2 end");
        // watch logs to confirm response contents
    }

    channel->CloseConnection();
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: SendResponseBytes
 * FunctionPoints: check session, send response bytes to soft bus.
 * EnvConditions:
 * CaseDescription: verify send response bytes
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_SendResponseBytes_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_SendResponseBytes_001");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        channel->BuildConnection();
        std::string bytesString = "abcdefg-0123456789";
        const int bytesLength = bytesString.length();
        int sessionId = 1;
        int code = channel->SendResponseBytes(sessionId, (unsigned char *)bytesString.c_str(), bytesLength);
        EXPECT_TRUE(code == 0);
    }

    channel->CloseConnection();
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: HandleResponse
 * FunctionPoints: execute response callback and clear it from callback buffer.
 * EnvConditions:
 * CaseDescription: verify handle handle
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_HandleResponse_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleResponse_001");
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    channel->BuildConnection();
    {
        const std::string id = "message-unique-id-001";
        const std::string jsonPayload =
            "\"{\"name\":\"test\",\"number\":100,\"flag\":true,\"array\":[0,1,2],\"uid\":\"test-device-id-001-uid\"}\"";
        channel->HandleResponse(id, jsonPayload);
    }

    channel->CloseConnection();
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: ExecuteCommand
 * FunctionPoints: execute a command, timeout
 * EnvConditions:
 * CaseDescription: verify handle execute command
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_ExecuteCommand_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_ExecuteCommand_001");

    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_ExecuteCommand_001-1");
        // check invalid parameter
        const std::string id = "message-unique-id-001";
        const std::string commandName = "";
        const std::string jsonPayload =
            "\"{\"name\":\"test\",\"number\":100,\"flag\":true,\"array\":[0,1,2],\"uid\":\"test-device-id-001-uid\"}\"";
        channel->BuildConnection();
        std::string result = channel->ExecuteCommand(commandName, jsonPayload);
        EXPECT_TRUE(result == "");
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_ExecuteCommand_001-2");
        // check invalid parameter
        const std::string id = "message-unique-id-001";
        const std::string commandName = "GetUidPermissionCommand";
        const std::string jsonPayload = "";
        std::string result = channel->ExecuteCommand(commandName, jsonPayload);
        EXPECT_TRUE(result == "");
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_ExecuteCommand_001-3");
        // send request timeout
        const std::string id = "message-unique-id-001";
        const std::string commandName = "GetUidPermissionCommand";
        const std::string jsonPayload =
            "\"{\"name\":\"test\",\"number\":100,\"flag\":true,\"array\":[0,1,2],\"uid\":\"test-device-id-001-uid\"}\"";
        std::string result = channel->ExecuteCommand(commandName, jsonPayload);
        EXPECT_TRUE(result == "");
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: ExecuteCommand
 * FunctionPoints: execute a command, timeout
 * EnvConditions:
 * CaseDescription: verify handle execute command
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_ExecuteCommand_002, TestSize.Level1)
{
    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_ExecuteCommand_001-4");
        const std::string id = "message-unique-id-001";
        const std::string commandName = "GetUidPermissionCommand";
        const std::string jsonPayload =
            "\"{\"name\":\"test\",\"number\":100,\"flag\":true,\"array\":[0,1,2],\"uid\":\"test-device-id-001-uid\"}\"";
        std::string dummyResult = "{\"type\":\"request\",\"id\":\"" + id + "\",\"commandName\":\"" + commandName +
                                  "\",\"jsonPayload\":" + jsonPayload + "}";

        // simulate a response
        simulatorExecuted.store(false);
        int callbackSleepMs = 1000;
        std::function<void()> runner = [&]() {
            PERMISSION_LOG_DEBUG(LABEL, "simulator start");
            std::this_thread::sleep_for(std::chrono::milliseconds(callbackSleepMs));

            simulatorExecuted.store(true);
            PERMISSION_LOG_DEBUG(LABEL, "simulator before handle response");
            channel->HandleResponse(id, dummyResult);

            PERMISSION_LOG_DEBUG(LABEL, "simulator after handle response, stop");
        };
        std::thread responseThread(runner);

        EXPECT_TRUE(simulatorExecuted.load() == false);
        PERMISSION_LOG_DEBUG(LABEL, "start simulator thread");

        time_t begin = TimeUtil::GetTimestamp();
        // send request ok
        channel->BuildConnection();
        std::string result = channel->ExecuteCommand(commandName, jsonPayload);

        PERMISSION_LOG_DEBUG(LABEL, "execute command compelete");

        // for test
        if (responseThread.joinable()) {
            responseThread.join();
        }

        EXPECT_TRUE(simulatorExecuted.load() == true);
        EXPECT_TRUE(result == dummyResult);
        time_t end = TimeUtil::GetTimestamp();
        PERMISSION_LOG_DEBUG(LABEL,
            "time from %{public}ld to %{public}ld, with callback ellapsed %{public}d ms",
            (long)begin,
            (long)end,
            callbackSleepMs);
        // callback ellapsed 1 second.
        EXPECT_TRUE(end - begin >= 1);
        EXPECT_TRUE(end - begin <= 2);

        channel->CloseConnection();
    }
}

/*
 * Feature: DPMS
 * Function: SoftBusChannel
 * SubFunction: HandleDataReceived
 * FunctionPoints: callback for soft bus data transfer.
 * EnvConditions:
 * CaseDescription: verify handle data received events.
 */
HWTEST_F(SoftBusChannelTest, SoftBusChannel_HandleDataReceived_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleDataReceived_001 begin");

    auto channel = std::make_shared<SoftBusChannel>(TARGET_DEVICE_ID_);
    channel->BuildConnection();

    int session = 1;
    std::string json = "abcdefg-0123456789";
    int len = 100;
    char unsigned buf[len];
    memset(buf, 0, len);
    channel->Compress(json, buf, len);
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleDataReceived_001-1");
        // check invalid parameter
        channel->HandleDataReceived(0, buf, len);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleDataReceived_001-2");
        // check invalid parameter
        channel->HandleDataReceived(session, buf, 0);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleDataReceived_001-3");
        // json error
        channel->HandleDataReceived(session, buf, len);
    }

    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleDataReceived_001:check logic");

    channel->BuildConnection();

    const std::string id = "message-unique-id-001";
    const std::string commandName = "GetUidPermissionCommand";
    std::string jsonPayload = "1";

    // check received: request
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleDataReceived_001-4");
        len = 1000;
        json = "{\"type\":\"request\",\"id\":\"" + id + "\",\"commandName\":\"" + commandName +
               "\",\"jsonPayload\":\"" + jsonPayload + "\"}";
        channel->Compress(json, buf, len);
        // ok
        // check log to confirm
        channel->HandleDataReceived(session, buf, len);
    }

    // check received: response
    {
        PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleDataReceived_001-5");
        len = 1000;
        json = "{\"type\":\"response\",\"id\":\"" + id + "\",\"commandName\":\"" + commandName +
               "\",\"jsonPayload\":\"" + jsonPayload + "\"}";
        channel->Compress(json, buf, len);
        // check log to confirm
        channel->HandleDataReceived(session, buf, len);
    }

    channel->CloseConnection();
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusChannel_HandleDataReceived_001 end");
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS