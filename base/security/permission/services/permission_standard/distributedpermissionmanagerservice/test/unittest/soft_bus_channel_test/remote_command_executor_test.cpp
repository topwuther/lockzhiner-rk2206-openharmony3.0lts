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
#include "remote_command_executor.h"
#define private public
#include "soft_bus_channel.h"

#include "constant.h"
#include "distributed_permission_manager_service.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "RemoteCommandExecutorTest"};
}
// copied from get_uid_permission_command_mock.cpp
namespace {
static const int UID_GetPermissions_FAILED = 1;
static const int UID_MAX_UID_PERMISSIONS_COUNT_OVER = 2;
static const int UID_GetRegrantedPermissions_FAILED = 100;
static const int UID_FINISH_SUCCESS_INDEX = 1000;

static const int UID_MOCK_NORMAL = 10000;

static const int RETRY_TIMES = 300;

static const std::string INVALID_DEVICE_("test-device-id-invalid");
static const std::string SELF_DEVICE_("test-device-id-001");
static const std::string TARGET_DEVICE_("test-device-id-002");
static const std::string deviceIdForDeviceIdUtil = "1004";

static const std::string TARGET_NETWORK_ID_(TARGET_DEVICE_ + ":network-id-001");
static const std::string TARGET_UUID_(TARGET_NETWORK_ID_ + ":uuid-001");
static const std::string TARGET_UDID_(TARGET_NETWORK_ID_ + ":udid-001");
static const std::string TARGET_DEVICE_ID_(TARGET_UDID_);

static const int SLEEP_100 = 100;
static const int SLEEP_500 = 500;
}  // namespace

class RemoteCommandExecutorTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        PERMISSION_LOG_DEBUG(LABEL, "SetUp");

        // patch for not start error
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStop();
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStart();
        // simulator register
        ::RegNodeDeviceStateCb("", nullptr);
        // 1.assume SoftBusManager inited
        int count = 1;
        while (SoftBusManager::GetInstance().isSoftBusServiceBindSuccess_ == false && count++ < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
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

        // for DeviceIdUtil
        DeviceInfoManager::GetInstance().AddDeviceInfo(
            deviceIdForDeviceIdUtil, deviceIdForDeviceIdUtil, "1", "2", std::to_string(1));
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
    {
        // 3.assume SoftBusManager inited
        handler_ = DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
        if (handler_ == nullptr) {
            PERMISSION_LOG_ERROR(LABEL, "fail to get EventHandler");
            ASSERT_TRUE(false);
        }
    }
    void TearDown()
    {}

    std::shared_ptr<DistributedPermissionEventHandler> handler_;
    bool *executed_ = new bool;
    const long EXECUTE_COMMAND_TIME_OUT = 3;  // 3000 ms
};

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: Construct
 * FunctionPoints: Init
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_Construct_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_Construct_001");

    {
        RemoteCommandExecutor instance(TARGET_UDID_);
        EXPECT_TRUE(instance.targetNodeId_ == TARGET_UDID_);
        EXPECT_EQ(instance.commands_.empty(), true);
        EXPECT_EQ(instance.ptrChannel_, nullptr);
        EXPECT_EQ((instance.running_), false);
    }
    {
        RemoteCommandExecutor instance(TARGET_UDID_);
        EXPECT_TRUE(instance.targetNodeId_ == TARGET_UDID_);
        EXPECT_EQ(instance.commands_.empty(), true);
        EXPECT_EQ(instance.ptrChannel_, nullptr);
        EXPECT_EQ((instance.running_), false);
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: Construct
 * FunctionPoints: join task thread when destroy.
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_Construct_002, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_Construct_002");

    time_t begin = TimeUtil::GetTimestamp();
    {
        RemoteCommandExecutor instance(TARGET_UDID_);

        const int32_t uid = UID_MOCK_NORMAL;  // normal uid, see mock, 10000
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        ASSERT_TRUE(ptrCommand != nullptr);
        PERMISSION_LOG_DEBUG(LABEL, "ptrCommand created, %{public}s", (ptrCommand == nullptr) ? "null" : "not null");

        // channel: will timeout(3000ms) for no response
        instance.ProcessOneCommand(ptrCommand);
    }

    // destruct
    time_t end = TimeUtil::GetTimestamp();
    PERMISSION_LOG_DEBUG(LABEL,
        "RemoteCommandExecutor_Construct_002: ellapsed %{public}ld second(s) from %{public}ld to %{public}ld",
        (long)end - begin,
        (long)begin,
        (long)end);

    PERMISSION_LOG_DEBUG(LABEL, "start: %{public}ld", (long)begin);
    PERMISSION_LOG_DEBUG(LABEL, "end: %{public}ld", (long)end);

    EXPECT_TRUE(end - begin >= 3);
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: GetChannel
 * FunctionPoints: get and set channel
 * EnvConditions: NA
 * CaseDescription: verify get channel and set channel
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_GetChannel_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_GetChannel_001");

    {
        RemoteCommandExecutor instance(TARGET_UDID_);
        EXPECT_TRUE(instance.GetChannel() == nullptr);

        instance.SetChannel(std::make_shared<SoftBusChannel>(TARGET_UDID_));
        EXPECT_TRUE(instance.GetChannel() != nullptr);

        instance.SetChannel(nullptr);
        EXPECT_TRUE(instance.GetChannel() == nullptr);
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: CreateChannel
 * FunctionPoints: create channel
 * EnvConditions: NA
 * CaseDescription: verify create channel
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_CreateChannel_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_CreateChannel_001");

    {
        RemoteCommandExecutor instance(TARGET_UDID_);
        auto channel = instance.CreateChannel(TARGET_UDID_);
        EXPECT_TRUE(channel != nullptr);
    }
    {
        auto channel = RemoteCommandExecutor::CreateChannel(TARGET_UDID_);
        EXPECT_TRUE(channel != nullptr);
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: CreateChannelIfNeeded
 * FunctionPoints: create channel if not exists
 * EnvConditions: NA
 * CaseDescription: verify use cached channel if exists. if not exists, create and cache it.
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_CreateChannelIfNeeded_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_CreateChannelIfNeeded_001");

    {
        RemoteCommandExecutor instance(TARGET_UDID_);
        EXPECT_TRUE(instance.GetChannel() == nullptr);
        instance.CreateChannelIfNeeded();
        EXPECT_TRUE(instance.GetChannel() != nullptr);
        auto channel1 = instance.GetChannel();

        instance.CreateChannelIfNeeded();
        auto channel2 = instance.GetChannel();
        EXPECT_TRUE(channel1 == channel2);

        // recreate channel
        instance.SetChannel(nullptr);
        instance.CreateChannelIfNeeded();
        auto channel3 = instance.GetChannel();
        EXPECT_TRUE(channel1 != channel3);
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: ClientProcessResult
 * FunctionPoints: finish command
 * EnvConditions: NA
 * CaseDescription: Verify finish a command
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_ClientProcessResult_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ClientProcessResult_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ClientProcessResult_001-1");
        const int32_t uid = UID_GetRegrantedPermissions_FAILED;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);
        // simulate a STATUS_CODE_BEFORE_RPC error
        ptrCommand->remoteProtocol_.statusCode = Constant::STATUS_CODE_BEFORE_RPC;

        // check a STATUS_CODE_BEFORE_RPC error
        RemoteCommandExecutor instance(TARGET_UDID_);
        instance.ClientProcessResult(ptrCommand);
        int code = Constant::STATUS_CODE_BEFORE_RPC;
        EXPECT_EQ(ptrCommand->remoteProtocol_.statusCode, code);
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ClientProcessResult_001-2");
        const int32_t uid = UID_GetRegrantedPermissions_FAILED;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        // check a normal FAILURE_BUT_CAN_RETRY error with command-get-permissions error.
        RemoteCommandExecutor instance(TARGET_UDID_);
        instance.ClientProcessResult(ptrCommand);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.statusCode == Constant::FAILURE_BUT_CAN_RETRY);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.message == Constant::COMMAND_GET_REGRANTED_PERMISSIONS_FAILED);
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ClientProcessResult_001-3");
        const int32_t uid = UID_GetRegrantedPermissions_FAILED + 1;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        // check a FAILURE_BUT_CAN_RETRY error with command-execute error.
        RemoteCommandExecutor instance(TARGET_UDID_);
        instance.ClientProcessResult(ptrCommand);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.statusCode == Constant::FAILURE_BUT_CAN_RETRY);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.message == Constant::COMMAND_RESULT_FAILED);
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ClientProcessResult_001-4");
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        // check a success code
        RemoteCommandExecutor instance(TARGET_UDID_);
        instance.ClientProcessResult(ptrCommand);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.statusCode == Constant::SUCCESS);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.message == Constant::COMMAND_RESULT_SUCCESS);
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: ExecuteRemoteCommand
 * FunctionPoints: execute remote command
 * EnvConditions: NA
 * CaseDescription: Verify execute a remote command at local device
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_ExecuteRemoteCommand_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ExecuteRemoteCommand_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ExecuteRemoteCommand_001-1");
        // local command: execute and finish
        // will get a COMMAND_GET_PERMISSIONS_FAILED error at execute,
        // will get a COMMAND_RESULT_FAILED error at finish.
        const int32_t uid = UID_GetPermissions_FAILED;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        bool isRemote = false;
        RemoteCommandExecutor instance(TARGET_UDID_);
        int code = instance.ExecuteRemoteCommand(ptrCommand, isRemote);

        EXPECT_TRUE(ptrCommand->remoteProtocol_.statusCode == Constant::FAILURE_BUT_CAN_RETRY);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.message == Constant::COMMAND_RESULT_FAILED);
        EXPECT_TRUE(code == Constant::FAILURE_BUT_CAN_RETRY);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ExecuteRemoteCommand_001-2");
        // local command: execute and finish
        // will get a COMMAND_PERMISSIONS_COUNT_FAILED error at execute,
        // will get a COMMAND_RESULT_FAILED error at finish.
        const int32_t uid = UID_MAX_UID_PERMISSIONS_COUNT_OVER;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        // will get an error
        bool isRemote = false;
        RemoteCommandExecutor instance(TARGET_UDID_);
        int code = instance.ExecuteRemoteCommand(ptrCommand, isRemote);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.statusCode == Constant::FAILURE_BUT_CAN_RETRY);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.message == Constant::COMMAND_RESULT_FAILED);
        EXPECT_TRUE(code == Constant::FAILURE_BUT_CAN_RETRY);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ExecuteRemoteCommand_001-3");
        // local command: execute and finish
        // will get a COMMAND_RESULT_SUCCESS error at execute,
        // will get a COMMAND_RESULT_SUCCESS error at finish.
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        bool isRemote = false;
        RemoteCommandExecutor instance(TARGET_UDID_);
        int code = instance.ExecuteRemoteCommand(ptrCommand, isRemote);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.statusCode == Constant::SUCCESS);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.message == Constant::COMMAND_RESULT_SUCCESS);
        EXPECT_TRUE(code == Constant::SUCCESS);
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: ExecuteRemoteCommand
 * FunctionPoints: execute remote command
 * EnvConditions: NA
 * CaseDescription: Verify execute a remote command at remote device
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_ExecuteRemoteCommand_002, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ExecuteRemoteCommand_002");
    std::shared_ptr<SoftBusChannel> channel = std::make_shared<SoftBusChannel>(TARGET_UDID_);
    channel->BuildConnection();

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ExecuteRemoteCommand_002-1");
        // remote command: transfer to remote, wait response, finish
        // will get a COMMAND_GET_REGRANTED_PERMISSIONS_FAILED error at execute
        const int32_t uid = UID_GetRegrantedPermissions_FAILED;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        // prepare channel
        instance.SetChannel(channel);

        // simulate a response
        *executed_ = false;
        std::function<void()> runner = [&]() {
            PERMISSION_LOG_DEBUG(LABEL, "enter runner");
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_500));
            const std::string id = "message-unique-id-100";  // UID_GetRegrantedPermissions_FAILED
            std::string dummyResult = "";
            channel->HandleResponse(id, dummyResult);
            *executed_ = true;
        };
        std::thread responseThread(runner);
        EXPECT_TRUE(*executed_ == false);

        // will got an empty response
        PERMISSION_LOG_DEBUG(LABEL, "ExecuteRemoteCommand begin");
        bool isRemote = true;
        int code = instance.ExecuteRemoteCommand(ptrCommand, isRemote);
        PERMISSION_LOG_DEBUG(LABEL, "ExecuteRemoteCommand end");

        // for test
        if (responseThread.joinable()) {
            responseThread.join();
        }

        // check executed status
        EXPECT_TRUE(*executed_ == true);

        // check empty response
        EXPECT_TRUE(code == Constant::FAILURE);

        PERMISSION_LOG_DEBUG(LABEL, "check done");
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ExecuteRemoteCommand_002-2");
        // remote command: transfer to remote, wait response, finish local
        // will get a COMMAND_GET_REGRANTED_PERMISSIONS_FAILED error at execute,
        // will get a COMMAND_RESULT_FAILED error at finish.
        const int32_t uid = UID_GetRegrantedPermissions_FAILED;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        // prepare channel
        instance.SetChannel(channel);

        // simulate a response, response uid 100.
        *executed_ = false;
        std::function<void()> runner = [&]() {
            PERMISSION_LOG_DEBUG(LABEL, "enter runner");
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-100";  // UID_GetRegrantedPermissions_FAILED
            std::string dummyResult = "{[100]}";
            channel->HandleResponse(id, dummyResult);
            *executed_ = true;
        };
        std::thread responseThread(runner);
        EXPECT_TRUE(*executed_ == false);

        bool isRemote = true;
        int code = instance.ExecuteRemoteCommand(ptrCommand, isRemote);

        // for test
        if (responseThread.joinable()) {
            responseThread.join();
        }

        // check executed status
        EXPECT_TRUE(*executed_ == true);
        // check invalid response
        PERMISSION_LOG_DEBUG(LABEL, "return status: %{public}d.", code);
        EXPECT_TRUE(code != Constant::SUCCESS);
        EXPECT_TRUE(code == Constant::FAILURE_BUT_CAN_RETRY);

        // wait destruct to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ExecuteRemoteCommand_002-3");
        // remote command: transfer to remote, wait response, finish local
        // will get a success at execute,
        // will get a COMMAND_RESULT_FAILED error at finish.
        const int32_t uid = UID_GetRegrantedPermissions_FAILED;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        // prepare channel
        instance.SetChannel(channel);
        // simulate a response, response uid 101.
        *executed_ = false;
        std::function<void()> runner = [&]() {
            PERMISSION_LOG_DEBUG(LABEL, "enter runner");
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-100";  // UID_GetRegrantedPermissions_FAILED
            std::string dummyResult = "{[101]}";
            channel->HandleResponse(id, dummyResult);
            *executed_ = true;
        };
        std::thread responseThread(runner);
        EXPECT_TRUE(*executed_ == false);

        bool isRemote = true;
        int code = instance.ExecuteRemoteCommand(ptrCommand, isRemote);

        // for test
        if (responseThread.joinable()) {
            responseThread.join();
        }

        // check executed status
        EXPECT_TRUE(*executed_ == true);
        // check finish failed
        EXPECT_TRUE(code == Constant::FAILURE_BUT_CAN_RETRY);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ExecuteRemoteCommand_002-4");
        // remote command: transfer to remote, wait response, finish local
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        // prepare channel
        instance.SetChannel(channel);
        // simulate a response
        bool executed = false;
        std::function<void()> runner = [&]() {
            PERMISSION_LOG_DEBUG(LABEL, "enter runner");
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-1000";  // UID_FINISH_SUCCESS_INDEX
            std::string dummyResult = "{[1000]}";
            channel->HandleResponse(id, dummyResult);
            executed = true;
        };
        std::thread responseThread(runner);
        EXPECT_TRUE(executed == false);

        bool isRemote = true;
        int code = instance.ExecuteRemoteCommand(ptrCommand, isRemote);

        // for test
        if (responseThread.joinable()) {
            responseThread.join();
        }

        EXPECT_TRUE(executed == true);
        // mock a normal response command, finish
        EXPECT_TRUE(code == Constant::SUCCESS);
    }

    channel->CloseConnection();
    channel->Release();
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: ProcessOneCommand
 * FunctionPoints: execute remote command on local device
 * EnvConditions: NA
 * CaseDescription: Verify execute a remote command on localce device
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_ProcessOneCommand_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessOneCommand_001");
    {
        // check parameter: nullptr, no need to do anything, return success.
        RemoteCommandExecutor instance(TARGET_UDID_);
        int code = instance.ProcessOneCommand(nullptr);
        EXPECT_TRUE(Constant::SUCCESS == code);
    }

    {
        // local command: execute and finish
        // will get a COMMAND_RESULT_SUCCESS error at execute,
        // will get a COMMAND_RESULT_SUCCESS error at finish.
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(deviceIdForDeviceIdUtil);
        int code = instance.ProcessOneCommand(ptrCommand);
        EXPECT_TRUE(ptrCommand->remoteProtocol_.statusCode == Constant::SUCCESS);

        PERMISSION_LOG_DEBUG(LABEL, "result message: %{public}s", ptrCommand->remoteProtocol_.message.c_str());
        PERMISSION_LOG_DEBUG(LABEL, "expect message: %{public}s", Constant::COMMAND_RESULT_SUCCESS.c_str());
        EXPECT_TRUE(ptrCommand->remoteProtocol_.message == Constant::COMMAND_RESULT_SUCCESS);

        EXPECT_TRUE(code == Constant::SUCCESS);
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: ProcessOneCommand
 * FunctionPoints: execute remote command on remote device
 * EnvConditions: NA
 * CaseDescription: Verify execute a remote command on remote device
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_ProcessOneCommand_002, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessOneCommand_002");
    std::shared_ptr<SoftBusChannel> channel = std::make_shared<SoftBusChannel>(TARGET_UDID_);
    channel->BuildConnection();

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessOneCommand_002-1");
        // remote command: transfer to remote, wait response, finish local
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        // prepare channel
        instance.SetChannel(channel);
        // simulate a response
        std::function<void()> runner = [&]() {
            PERMISSION_LOG_DEBUG(LABEL, "enter runner");
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-1000";
            std::string dummyResult = "{[1000]}";
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        int code = instance.ProcessOneCommand(ptrCommand);

        // for test
        if (responseThread.joinable()) {
            responseThread.join();
        }

        // mock a normal response command, finish
        EXPECT_TRUE(code == Constant::SUCCESS);
    }
    channel->CloseConnection();
    channel->Release();
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: AddCommand
 * FunctionPoints: add commands to be execute.
 * EnvConditions: NA
 * CaseDescription: Verify add commands
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_AddCommand_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommands_001");
    {
        // check parameter: nullptr
        RemoteCommandExecutor instance(TARGET_UDID_);
        int code = instance.AddCommand(nullptr);
        EXPECT_TRUE(Constant::INVALID_COMMAND == code);
    }
    {
        // one command
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        int code = instance.AddCommand(ptrCommand);

        EXPECT_TRUE(code == Constant::SUCCESS);
    }
    {
        // repeated command
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        int code = instance.AddCommand(ptrCommand);
        EXPECT_TRUE(code == Constant::SUCCESS);
        EXPECT_TRUE(instance.commands_.size() == 1);

        // will skip repeated commands(uniqueId)
        code = instance.AddCommand(ptrCommand);
        EXPECT_TRUE(code == Constant::SUCCESS);
        EXPECT_TRUE(instance.commands_.size() == 1);
    }
    {
        // multi commands
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        {
            int code = instance.AddCommand(ptrCommand);
            EXPECT_TRUE(code == Constant::SUCCESS);
            EXPECT_TRUE(instance.commands_.size() == 1);
        }

        const std::shared_ptr<BaseRemoteCommand> ptrCommand2 =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid + 1, srcDeviceId, dstDeviceId);

        {
            int code = instance.AddCommand(ptrCommand2);
            EXPECT_TRUE(code == Constant::SUCCESS);
            EXPECT_TRUE(instance.commands_.size() == 2);
        }
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: ProcessBufferedCommands
 * FunctionPoints: add commands and execute them.
 * EnvConditions: NA
 * CaseDescription: Verify add commands and execute them all.
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_ProcessBufferedCommands_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommands_001");
    std::shared_ptr<SoftBusChannel> channel = std::make_shared<SoftBusChannel>(TARGET_UDID_);
    channel->BuildConnection();
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommands_001-1");
        // empty commands, no need to do anything, return success.
        RemoteCommandExecutor instance(TARGET_UDID_);
        int code = instance.ProcessBufferedCommands();
        EXPECT_TRUE(Constant::SUCCESS == code);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommands_001-2");
        // remote command: transfer to remote, wait response, finish local
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);
        const std::shared_ptr<BaseRemoteCommand> ptrCommand2 =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid + 1, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        instance.AddCommand(ptrCommand);
        instance.AddCommand(ptrCommand2);
        instance.AddCommand(ptrCommand);
        instance.AddCommand(ptrCommand2);
        EXPECT_TRUE(instance.commands_.size() == 2);

        // prepare channel
        instance.SetChannel(channel);
        // simulate a response
        {
            std::function<void()> runner = [&]() {
                PERMISSION_LOG_DEBUG(LABEL, "enter runner 1");
                std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
                const std::string id = "message-unique-id-1000";
                std::string dummyResult = "{[1000]}";
                channel->HandleResponse(id, dummyResult);
            };
            std::thread responseThread(runner);

            std::function<void()> runner2 = [&]() {
                PERMISSION_LOG_DEBUG(LABEL, "enter runner 2");
                std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_500));
                const std::string id = "message-unique-id-1000";
                std::string dummyResult = "{[1000]}";
                channel->HandleResponse(id, dummyResult);
            };
            std::thread responseThread2(runner2);

            int code = instance.ProcessBufferedCommands();

            // wait to process commands.
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_500));
            PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommands_001-2:response");
            EXPECT_TRUE(code == Constant::SUCCESS);
            EXPECT_EQ(instance.commands_.size(), (uint32_t)0);

            // for test
            if (responseThread.joinable()) {
                responseThread.join();
            }
            if (responseThread2.joinable()) {
                responseThread2.join();
            }
        }
    }
    channel->CloseConnection();
    channel->Release();
}

/*
 * Feature: DPMS
 * Function: RemoteCommandExecutor
 * SubFunction: ProcessBufferedCommandsWithThread
 * FunctionPoints: add commands and execute them in a thread
 * EnvConditions: NA
 * CaseDescription: Verify add commands and execute them all in a thread
 */
HWTEST_F(RemoteCommandExecutorTest, RemoteCommandExecutor_ProcessBufferedCommandsWithThread_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommandsWithThread_001");
    std::shared_ptr<SoftBusChannel> channel = std::make_shared<SoftBusChannel>(TARGET_UDID_);
    channel->BuildConnection();
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommandsWithThread_001-1");
        // empty commands, no need to do anything, return success.
        RemoteCommandExecutor instance(TARGET_UDID_);
        instance.ProcessBufferedCommandsWithThread();
        EXPECT_TRUE(instance.running_ == false);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommandsWithThread_001-2");
        // remote command: transfer to remote, wait response, finish local
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);
        const std::shared_ptr<BaseRemoteCommand> ptrCommand2 =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid + 1, srcDeviceId, dstDeviceId);

        RemoteCommandExecutor instance(TARGET_UDID_);
        instance.AddCommand(ptrCommand);
        instance.AddCommand(ptrCommand2);
        instance.AddCommand(ptrCommand);
        instance.AddCommand(ptrCommand2);
        EXPECT_EQ(instance.commands_.size(), (uint32_t)2);

        // prepare channel
        instance.SetChannel(channel);
        // simulate a response

        std::function<void()> runner = [&]() {
            PERMISSION_LOG_DEBUG(LABEL, "enter runner 1");
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-1000";
            std::string dummyResult = "{[1000]}";
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        std::function<void()> runner2 = [&]() {
            PERMISSION_LOG_DEBUG(LABEL, "enter runner 2");
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_500));
            const std::string id = "message-unique-id-1001";
            std::string dummyResult = "{[1000]}";
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread2(runner2);

        // run commands in new thread(post task)
        instance.ProcessBufferedCommandsWithThread();
        EXPECT_TRUE(instance.running_ != false);

        // wait to process commands.
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_500));
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommands_001-2:response1");
        EXPECT_EQ(instance.commands_.size(), (uint32_t)1);
        EXPECT_TRUE(instance.running_ != false);

        // wait to process commands.
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_500));
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommands_001-2:response2");
        // mock a normal response command, finish
        EXPECT_EQ(instance.commands_.size(), (uint32_t)0);

        // for test
        if (responseThread.joinable()) {
            responseThread.join();
        }
        if (responseThread2.joinable()) {
            responseThread2.join();
        }

        PERMISSION_LOG_DEBUG(LABEL, "check done");
    }
    // CloseConnection: delay close connection
    channel->CloseConnection();

    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor_ProcessBufferedCommandsWithThread_001 complete");
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS