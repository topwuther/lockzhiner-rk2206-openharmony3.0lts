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
#include <thread>
#include <atomic>

#define private public
#include "remote_command_manager.h"

#include "soft_bus_channel.h"
#include "distributed_permission_manager_service.h"
#include "permission_log.h"

using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "RemoteCommandManagerTest"};
}
namespace {
static const int UID_GetRegrantedPermissions_FAILED = 100;
static const int UID_FINISH_SUCCESS_INDEX = 1000;

static const std::string INVALID_DEVICE_("test-device-id-invalid");
static const std::string SELF_DEVICE_("test-device-id-001");
static const std::string TARGET_DEVICE_("test-device-id-002");

static const std::string TARGET_NETWORK_ID_(TARGET_DEVICE_ + ":network-id-001");
static const std::string TARGET_UUID_(TARGET_NETWORK_ID_ + ":uuid-001");
static const std::string TARGET_UDID_(TARGET_NETWORK_ID_ + ":udid-001");
static const std::string TARGET_DEVICE_ID_(TARGET_UDID_);

static const int SLEEP_1S = 1000;
static const int SLEEP_100 = 100;
}  // namespace
class RemoteCommandManagerTest : public testing::Test {
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
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_1S));

        ASSERT_EQ(SoftBusManager::GetInstance().isSoftBusServiceBindSuccess_, true);

        MonitorManager *instance = &MonitorManager::GetInstance();
        // remove permission_bms_manager listener for test
        instance->RemoveOnPermissionChangedListener("PERMISSION_BMS_MANAGER");
        instance->RemoveOnPermissionChangedListener("ObjectDevicePermissionManager");

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
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->OnStop();
    }
    void SetUp()
    {}
    void TearDown()
    {}
};

/*
 * Feature: DPMS
 * Function: RemoteCommandManager
 * SubFunction: Construct
 * FunctionPoints: construct
 * EnvConditions: NA
 * CaseDescription: verify constructor and destructor
 */
HWTEST_F(RemoteCommandManagerTest, RemoteCommandManager_Construct_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "RemoteCommandManager_Construct_001");
    {
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
    }
    {
        RemoteCommandManager instance;
        EXPECT_EQ(instance.executors_.size(), (const unsigned int)(0));
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandManager
 * SubFunction: GetOrCreateRemoteCommandExecutor
 * FunctionPoints: create executor or get cached one.
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(RemoteCommandManagerTest, RemoteCommandManager_GetOrCreateRemoteCommandExecutor_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "RemoteCommandManager_GetOrCreateRemoteCommandExecutor_001");
    {
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));

        // udid
        instance->GetOrCreateRemoteCommandExecutor(TARGET_UDID_);
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(1));

        instance->GetOrCreateRemoteCommandExecutor(TARGET_UDID_);
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(1));

        instance->GetOrCreateRemoteCommandExecutor(TARGET_UDID_ + "-test");
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(2));

        instance->Clear();
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandManager
 * SubFunction: ExecuteCommand
 * FunctionPoints: execute a command
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(RemoteCommandManagerTest, RemoteCommandManager_ExecuteCommand_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ExecuteCommand_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ExecuteCommand_001-1");
        // check parameter: nullptr command
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        int code = instance->ExecuteCommand("test", nullptr);
        EXPECT_TRUE(Constant::FAILURE == code);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ExecuteCommand_001-2");
        // check parameter: empty deviceId
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        int code = instance->ExecuteCommand("", ptrCommand);
        EXPECT_TRUE(Constant::FAILURE == code);
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ExecuteCommand_001-3");

        // success
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        // simulate response
        std::function<void()> runner = [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-1000";  // UID_FINISH_SUCCESS_INDEX
            std::string dummyResult = "{[1000]}";
            auto channel = (SoftBusChannel *)(instance->GetExecutorChannel(TARGET_UDID_).get());
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        int code = instance->ExecuteCommand(TARGET_UDID_, ptrCommand);
        EXPECT_TRUE(Constant::SUCCESS == code);

        // wait thread
        if (responseThread.joinable()) {
            responseThread.join();
        }
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ExecuteCommand_001-4");
        // fail
        const int UID_GetPermissions_FAILED = 1;
        const int32_t uid = UID_GetPermissions_FAILED;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        // simulate response
        std::function<void()> runner = [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-1";
            std::string dummyResult = "{[100]}";
            auto channel = (SoftBusChannel *)(instance->GetExecutorChannel(TARGET_UDID_).get());
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        int code = instance->ExecuteCommand(TARGET_UDID_, ptrCommand);
        // FAILURE_BUT_CAN_RETRY
        EXPECT_TRUE(Constant::SUCCESS != code);

        // wait thread
        if (responseThread.joinable()) {
            responseThread.join();
        }
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandManager
 * SubFunction: AddCommand
 * FunctionPoints: add command and remove
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(RemoteCommandManagerTest, RemoteCommandManager_AddCommand_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_AddCommand_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_AddCommand_001-1");
        // check parameter: nullptr
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        instance->Clear();
        int code = instance->AddCommand(TARGET_UDID_, nullptr);
        EXPECT_TRUE(Constant::FAILURE == code);
        EXPECT_TRUE(instance->executors_.size() == 0);
    }
    {
        // check parameter: empty deviceid
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        int code = instance->AddCommand("", ptrCommand);
        EXPECT_TRUE(Constant::FAILURE == code);
        EXPECT_TRUE(instance->executors_.size() == 0);
    }
    {
        // one command
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        // add repeated command
        int code = instance->AddCommand(TARGET_UDID_, ptrCommand);
        EXPECT_TRUE(code == Constant::SUCCESS);
        EXPECT_TRUE(instance->executors_.size() == 1);
        code = instance->AddCommand(TARGET_UDID_, ptrCommand);
        EXPECT_TRUE(code == Constant::SUCCESS);
        EXPECT_TRUE(instance->executors_.size() == 1);

        // remove command
        instance->RemoveCommand(TARGET_UDID_);
        EXPECT_TRUE(instance->executors_.size() == 0);
    }
    {
        // multi commands
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        const std::shared_ptr<BaseRemoteCommand> ptrCommand2 =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid + 1, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        int code = instance->AddCommand(TARGET_UDID_, ptrCommand);
        EXPECT_TRUE(instance->executors_.size() == 1);
        EXPECT_TRUE(code == Constant::SUCCESS);
        code = instance->AddCommand(TARGET_UUID_, ptrCommand2);
        EXPECT_TRUE(instance->executors_.size() == 2);
        EXPECT_TRUE(code == Constant::SUCCESS);

        instance->RemoveCommand(TARGET_UUID_);
        EXPECT_TRUE(instance->executors_.size() == 1);
        instance->RemoveCommand(TARGET_UDID_);
        EXPECT_TRUE(instance->executors_.size() == 0);
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandManager
 * SubFunction: ProcessDeviceCommandImmediately
 * FunctionPoints: execute buffered commands for a device
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(RemoteCommandManagerTest, RemoteCommandManager_ProcessDeviceCommandImmediately_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ProcessDeviceCommandImmediately_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ProcessDeviceCommandImmediately_001-1");
        // check parameter: empty deviceId
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        instance->Clear();
        if (instance->GetExecutorChannel(TARGET_UDID_) != nullptr) {
            instance->GetExecutorChannel(TARGET_UDID_)->Release();
        }
        if (instance->GetExecutorChannel(TARGET_UUID_) != nullptr) {
            instance->GetExecutorChannel(TARGET_UUID_)->Release();
        }
        int code = instance->ProcessDeviceCommandImmediately("");
        EXPECT_TRUE(Constant::FAILURE == code);
    }

    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ProcessDeviceCommandImmediately_001-2");
        // fail: no executor(command not added)
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        int code = instance->ProcessDeviceCommandImmediately(TARGET_UDID_);
        EXPECT_TRUE(Constant::FAILURE == code);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ProcessDeviceCommandImmediately_001-3");
        // fail: FAILURE_BUT_CAN_RETRY
        const int32_t uid = UID_GetRegrantedPermissions_FAILED;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();

        instance->AddCommand(TARGET_UDID_, ptrCommand);

        // simulate response
        std::function<void()> runner = [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-100";
            // fail response: FAILURE_BUT_CAN_RETRY
            std::string dummyResult = "{[100]}";
            auto channel = (SoftBusChannel *)(instance->GetExecutorChannel(TARGET_UDID_).get());
            PERMISSION_LOG_DEBUG(LABEL, "channel in callback, %{public}ld", (long)channel);
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        // ensure executor and channel in AddCommand
        int code = instance->ProcessDeviceCommandImmediately(TARGET_UDID_);
        PERMISSION_LOG_DEBUG(LABEL, "ret code: %{public}d", code);
        EXPECT_TRUE(Constant::SUCCESS != code);

        // wait thread
        if (responseThread.joinable()) {
            responseThread.join();
        }

        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ProcessDeviceCommandImmediately_001-3 end");
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ProcessDeviceCommandImmediately_001-4");
        // success
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        // simulate response
        std::function<void()> runner = [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-1000";
            std::string dummyResult = "{[1000]}";
            auto channel = (SoftBusChannel *)(instance->GetExecutorChannel(TARGET_UDID_).get());
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        instance->AddCommand(TARGET_UDID_, ptrCommand);
        int code = instance->ProcessDeviceCommandImmediately(TARGET_UDID_);
        EXPECT_TRUE(Constant::SUCCESS == code);

        // wait thread
        if (responseThread.joinable()) {
            responseThread.join();
        }
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_ProcessDeviceCommandImmediately_001-5");
        // success, multi commands
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        const std::shared_ptr<BaseRemoteCommand> ptrCommand2 =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid + 1, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        // simulate response
        std::atomic<int> count(0);
        std::function<void()> runner = [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            std::string id = "message-unique-id-1000";  // UID_FINISH_SUCCESS_INDEX
            std::string dummyResult = "{[1000]}";
            auto channel = (SoftBusChannel *)(instance->GetExecutorChannel(TARGET_UDID_).get());
            count++;
            channel->HandleResponse(id, dummyResult);

            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            count++;
            id = "message-unique-id-1001";  // UID_FINISH_SUCCESS_INDEX + 1
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        instance->AddCommand(TARGET_UDID_, ptrCommand);
        instance->AddCommand(TARGET_UDID_, ptrCommand2);
        int code = instance->ProcessDeviceCommandImmediately(TARGET_UDID_);
        EXPECT_TRUE(Constant::SUCCESS == code);
        EXPECT_EQ(count.load(), 2);

        // wait thread
        if (responseThread.joinable()) {
            responseThread.join();
        }
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandManager
 * SubFunction: Loop
 * FunctionPoints: Execute all buffered commands for all device asynchronized.
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(RemoteCommandManagerTest, RemoteCommandManager_Loop_001, TestSize.Level1)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_Loop_001");
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_Loop_001-1");
        // no commands
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        instance->Clear();
        if (instance->GetExecutorChannel(TARGET_UDID_) != nullptr) {
            instance->GetExecutorChannel(TARGET_UDID_)->Release();
        }
        if (instance->GetExecutorChannel(TARGET_UUID_) != nullptr) {
            instance->GetExecutorChannel(TARGET_UUID_)->Release();
        }
        int code = instance->Loop();
        PERMISSION_LOG_DEBUG(LABEL, "ret code: %{public}d", code);
        EXPECT_TRUE(Constant::SUCCESS == code);

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_1S));
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_Loop_001-2");
        // 1 command
        const int32_t uid = UID_GetRegrantedPermissions_FAILED;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        instance->Clear();
        instance->AddCommand(TARGET_UDID_, ptrCommand);

        // simulate response
        std::atomic<int> count(0);
        std::function<void()> runner = [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            const std::string id = "message-unique-id-100";
            // fail response: FAILURE_BUT_CAN_RETRY
            std::string dummyResult = "{[100]}";
            auto channel = (SoftBusChannel *)(instance->GetExecutorChannel(TARGET_UDID_).get());
            PERMISSION_LOG_DEBUG(LABEL, "channel in callback, %{public}ld", (long)channel);
            count++;
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        int code = instance->Loop();
        PERMISSION_LOG_DEBUG(LABEL, "ret code: %{public}d", code);
        EXPECT_TRUE(Constant::SUCCESS == code);

        // wait thread
        if (responseThread.joinable()) {
            responseThread.join();
        }
        // wait for executor thread
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
        EXPECT_EQ(count.load(), 1);
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_Loop_001-3");
        // multi commands
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);

        const std::shared_ptr<BaseRemoteCommand> ptrCommand2 =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid + 1, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        instance->Clear();
        // simulate response
        std::atomic<int> count(0);
        std::function<void()> runner = [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            std::string id = "message-unique-id-1000";  // UID_FINISH_SUCCESS_INDEX
            std::string dummyResult = "{[1000]}";
            auto channel = (SoftBusChannel *)(instance->GetExecutorChannel(TARGET_UDID_).get());
            count++;
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        int sleep200 = 200;
        std::function<void()> runner2 = [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep200));
            std::string id = "message-unique-id-1001";  // UID_FINISH_SUCCESS_INDEX+1
            std::string dummyResult = "{[1001]}";
            auto channel = (SoftBusChannel *)(instance->GetExecutorChannel(TARGET_UDID_).get());
            count++;
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread2(runner2);

        instance->AddCommand(TARGET_UDID_, ptrCommand);
        instance->AddCommand(TARGET_UDID_, ptrCommand2);

        int code = instance->Loop();
        EXPECT_TRUE(Constant::SUCCESS == code);

        PERMISSION_LOG_DEBUG(LABEL, "wait for tasks1");
        int sleep150 = 150;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep150));
        EXPECT_EQ(count.load(), 1);

        // wait thread
        if (responseThread.joinable()) {
            responseThread.join();
        }
        // wait for executor thread
        PERMISSION_LOG_DEBUG(LABEL, "wait for tasks2");
        int sleep500 = 500;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep500));
        EXPECT_EQ(count.load(), 2);

        if (responseThread2.joinable()) {
            responseThread2.join();
        }
    }
    {
        PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager_Loop_001-4");
        // multi commands & multi devices
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);
        const std::shared_ptr<BaseRemoteCommand> ptrCommand2 =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid + 1, srcDeviceId, dstDeviceId);
        const std::shared_ptr<BaseRemoteCommand> ptrCommand3 =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid + 2, srcDeviceId, dstDeviceId);

        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        instance->Clear();

        // simulate response
        std::atomic<int> count(0);
        std::function<void()> runner = [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            std::string id = "message-unique-id-1000";  // UID_FINISH_SUCCESS_INDEX
            std::string dummyResult = "{[1000]}";
            auto channel = (SoftBusChannel *)(instance->GetExecutorChannel(TARGET_UDID_).get());
            count++;
            id = "message-unique-id-1000";  // UID_FINISH_SUCCESS_INDEX
            channel->HandleResponse(id, dummyResult);

            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            count++;
            id = "message-unique-id-1001";  // UID_FINISH_SUCCESS_INDEX + 1
            channel->HandleResponse(id, dummyResult);

            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
            count++;
            id = "message-unique-id-1002";  // UID_FINISH_SUCCESS_INDEX + 2
            channel->HandleResponse(id, dummyResult);
        };
        std::thread responseThread(runner);

        // sequence related(key), maybe fail
        instance->AddCommand(TARGET_UDID_, ptrCommand);
        instance->AddCommand(TARGET_UDID_, ptrCommand2);
        instance->AddCommand(TARGET_UUID_, ptrCommand3);
        int code = instance->Loop();

        PERMISSION_LOG_DEBUG(LABEL, "wait for tasks1");
        int sleep150 = 150;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep150));
        EXPECT_EQ(count.load(), 1);

        PERMISSION_LOG_DEBUG(LABEL, "wait for tasks2");
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_100));
        EXPECT_EQ(count.load(), 2);

        EXPECT_TRUE(Constant::SUCCESS == code);

        // wait thread
        if (responseThread.joinable()) {
            responseThread.join();
        }
        PERMISSION_LOG_DEBUG(LABEL, "wait for tasks3");
        // wait for executor thread
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_1S));
        EXPECT_EQ(count.load(), 3);
        // wait for timeout thread to complete(maybe)
        PERMISSION_LOG_DEBUG(LABEL, "wait for timeout thread");
        if (instance->GetExecutorChannel(TARGET_UDID_) != nullptr) {
            instance->GetExecutorChannel(TARGET_UDID_)->Release();
        }
        if (instance->GetExecutorChannel(TARGET_UUID_) != nullptr) {
            instance->GetExecutorChannel(TARGET_UUID_)->Release();
        }
        int sleep5000 = 500;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep5000));
    }
    int sleep500 = 500;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep500));
}

/*
 * Feature: DPMS
 * Function: RemoteCommandManager
 * SubFunction: NotifyDeviceOnline
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify init channel for the online service
 */
HWTEST_F(RemoteCommandManagerTest, RemoteCommandManager_NotifyDeviceOnline_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "RemoteCommandManager_NotifyDeviceOnline_001");
    {
        // parameter check: emtpy
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        instance->Clear();
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        int code = instance->NotifyDeviceOnline("");
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        // parameter check: max length
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        std::string padding(65, 'x');
        int code = instance->NotifyDeviceOnline(padding);
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        // normal
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        {
            int code = instance->NotifyDeviceOnline(TARGET_UDID_);
            EXPECT_EQ(instance->executors_.size(), (const unsigned int)(1));
            EXPECT_TRUE(code == Constant::SUCCESS);
        }
        // repeat
        {
            int code = instance->NotifyDeviceOnline(TARGET_UDID_);
            EXPECT_EQ(instance->executors_.size(), (const unsigned int)(1));
            EXPECT_TRUE(code == Constant::SUCCESS);
        }
    }
}

/*
 * Feature: DPMS
 * Function: RemoteCommandManager
 * SubFunction: NotifyDeviceOffline
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: verify init channel for the offline service
 */
HWTEST_F(RemoteCommandManagerTest, RemoteCommandManager_NotifyDeviceOffline_001, TestSize.Level1)
{
    PERMISSION_LOG_INFO(LABEL, "RemoteCommandManager_NotifyDeviceOffline_001");
    {
        // parameter check: emtpy
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        instance->Clear();
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        int code = instance->NotifyDeviceOffline("");
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        // parameter check: max length
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        std::string padding(65, 'x');
        int code = instance->NotifyDeviceOnline(padding);
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        EXPECT_TRUE(code == Constant::FAILURE);
    }
    {
        // normal: no executor
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        int code = instance->NotifyDeviceOffline(TARGET_UDID_);
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        EXPECT_TRUE(code == Constant::SUCCESS);
    }
    {
        // normal: has executor and commands
        RemoteCommandManager *instance = &RemoteCommandManager::GetInstance();
        const int32_t uid = UID_FINISH_SUCCESS_INDEX;
        const std::string srcDeviceId = SELF_DEVICE_;
        const std::string dstDeviceId = TARGET_DEVICE_;
        const std::shared_ptr<BaseRemoteCommand> ptrCommand =
            RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, srcDeviceId, dstDeviceId);
        instance->AddCommand(TARGET_UDID_, ptrCommand);
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(1));

        int code = instance->NotifyDeviceOffline(TARGET_UDID_);
        EXPECT_EQ(instance->executors_.size(), (const unsigned int)(0));
        // channel.Release() => PostTask removed(delay closing channel): skip
        EXPECT_TRUE(code == Constant::SUCCESS);
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS