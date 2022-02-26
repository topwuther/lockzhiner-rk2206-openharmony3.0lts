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

#include "remote_command_executor.h"

#include "device_info_manager.h"
#include "soft_bus_channel.h"

#include "distributed_permission_event_handler.h"
#include "distributed_permission_manager_service.h"
#include "singleton.h"
#include "parameter.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "RemoteCommandExecutor"};
static const std::string TASK_NAME = "RemoteCommandExecutor::ProcessBufferedCommandsWithThread";
}  // namespace
RemoteCommandExecutor::RemoteCommandExecutor(const std::string &targetNodeId)
    : targetNodeId_(targetNodeId), ptrChannel_(nullptr), mutex_(), commands_(), running_(false)
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandExecutor()");
}

RemoteCommandExecutor::~RemoteCommandExecutor()
{
    PERMISSION_LOG_DEBUG(LABEL, "~RemoteCommandExecutor() begin");
    running_ = false;
    PERMISSION_LOG_DEBUG(LABEL, "~RemoteCommandExecutor() end");
}

const std::shared_ptr<RpcChannel> RemoteCommandExecutor::CreateChannel(const std::string &targetNodeId)
{
    PERMISSION_LOG_DEBUG(LABEL, "CreateChannel: targetNodeId=%{public}s", targetNodeId.c_str());
    // only consider SoftBusChannel
    std::shared_ptr<RpcChannel> ptrChannel = std::make_shared<SoftBusChannel>(targetNodeId);
    if (ptrChannel == nullptr) {
        PERMISSION_LOG_INFO(
            LABEL, "CreateChannel: create channel failed, targetNodeId=%{public}s", targetNodeId.c_str());
    }
    return ptrChannel;
}

/*
 * called by RemoteCommandExecutor, RemoteCommandManager
 */
int RemoteCommandExecutor::ProcessOneCommand(const std::shared_ptr<BaseRemoteCommand> &ptrCommand)
{
    if (ptrCommand == nullptr) {
        PERMISSION_LOG_WARN(
            LABEL, "targetNodeId %{public}s, attempt to process on null command.", targetNodeId_.c_str());
        return Constant::SUCCESS;
    }
    const std::string uniqueId = ptrCommand->remoteProtocol_.uniqueId;
    PERMISSION_LOG_INFO(LABEL,
        "targetNodeId %{public}s, process one command start, uniqueId: %{public}s",
        targetNodeId_.c_str(),
        uniqueId.c_str());

    ptrCommand->Prepare();
    int status = ptrCommand->remoteProtocol_.statusCode;
    if (status != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL,
            "targetNodeId %{public}s, process one command error, uniqueId: %{public}s, message: "
            "prepare failure code %{public}d",
            targetNodeId_.c_str(),
            uniqueId.c_str(),
            status);
        return status;
    }

    char localUdid[Constant::DEVICE_UUID_LENGTH] = {0};
    ::GetDevUdid(localUdid, Constant::DEVICE_UUID_LENGTH);
    if (targetNodeId_ == localUdid) {
        return ExecuteRemoteCommand(ptrCommand, false);
    }

    // otherwise a remote device
    CreateChannelIfNeeded();
    if (ptrChannel_ == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "targetNodeId %{public}s, channel is null.", targetNodeId_.c_str());
        return Constant::FAILURE;
    }
    if (ptrChannel_->BuildConnection() != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "targetNodeId %{public}s, channel is not ready.", targetNodeId_.c_str());
        return Constant::FAILURE;
    }

    return ExecuteRemoteCommand(ptrCommand, true);
}

/*
 * called by RemoteCommandManager
 */
int RemoteCommandExecutor::AddCommand(const std::shared_ptr<BaseRemoteCommand> &ptrCommand)
{
    if (ptrCommand == nullptr) {
        PERMISSION_LOG_DEBUG(LABEL, "targetNodeId %{public}s, attempt to add an empty command.", targetNodeId_.c_str());
        return Constant::INVALID_COMMAND;
    }

    const std::string uniqueId = ptrCommand->remoteProtocol_.uniqueId;
    PERMISSION_LOG_DEBUG(
        LABEL, "targetNodeId %{public}s, add uniqueId %{public}s", targetNodeId_.c_str(), uniqueId.c_str());

    std::unique_lock<std::recursive_mutex> lock(mutex_);

    // make sure do not have the same command in the command buffer
    for (auto bufferedCommand : commands_) {
        if (bufferedCommand->remoteProtocol_.uniqueId == uniqueId) {
            PERMISSION_LOG_WARN(LABEL,
                "targetNodeId %{public}s, add uniqueId %{public}s, already exist in the buffer, skip",
                targetNodeId_.c_str(),
                uniqueId.c_str());
            return Constant::SUCCESS;
        }
    }

    commands_.push_back(ptrCommand);
    return Constant::SUCCESS;
}

/*
 * called by RemoteCommandExecutor.ProcessCommandThread, RemoteCommandManager
 */
int RemoteCommandExecutor::ProcessBufferedCommands(bool standalone)
{
    PERMISSION_LOG_INFO(
        LABEL, "begin, targetNodeId: %{public}s, standalone: %{public}d", targetNodeId_.c_str(), standalone);

    std::unique_lock<std::recursive_mutex> lock(mutex_);

    if (commands_.empty()) {
        PERMISSION_LOG_WARN(LABEL, "no command, targetNodeId %{public}s", targetNodeId_.c_str());
        running_ = false;
        return Constant::SUCCESS;
    }

    running_ = true;
    while (true) {
        // interrupt
        if (running_ == false) {
            PERMISSION_LOG_INFO(
                LABEL, "end with running flag == false, targetNodeId: %{public}s", targetNodeId_.c_str());
            return Constant::FAILURE;
        }
        // end
        if (commands_.empty()) {
            running_ = false;
            PERMISSION_LOG_INFO(LABEL, "end, no command left, targetNodeId: %{public}s", targetNodeId_.c_str());
            return Constant::SUCCESS;
        }

        // consume queue to execute
        const std::shared_ptr<BaseRemoteCommand> bufferedCommand = commands_.front();
        int status = ProcessOneCommand(bufferedCommand);
        if (status == Constant::SUCCESS) {
            commands_.pop_front();
            continue;
        } else if (status == Constant::FAILURE_BUT_CAN_RETRY) {
            PERMISSION_LOG_WARN(LABEL,
                "execute failed and wait to retry, targetNodeId: %{public}s, message: %{public}s, and will retry ",
                targetNodeId_.c_str(),
                bufferedCommand->remoteProtocol_.message.c_str());

            // now, the retry at once will have no effective because the network problem
            // so if the before the step, one command is added, and run this function
            // it should also not need to restart to process the commands buffer at once.
            running_ = false;
            return Constant::FAILURE;
        } else {
            // this command failed, move on to execute next command
            commands_.pop_front();
            PERMISSION_LOG_ERROR(LABEL,
                "execute failed, targetNodeId: %{public}s, commandName: %{public}s, message: %{public}s",
                targetNodeId_.c_str(),
                bufferedCommand->remoteProtocol_.commandName.c_str(),
                bufferedCommand->remoteProtocol_.message.c_str());
        }
    }
}

/*
 * called by RemoteCommandManager
 */
void RemoteCommandExecutor::ProcessBufferedCommandsWithThread()
{
    PERMISSION_LOG_INFO(LABEL, "begin, targetNodeId: %{public}s", targetNodeId_.c_str());

    std::unique_lock<std::recursive_mutex> lock(mutex_);

    if (commands_.empty()) {
        PERMISSION_LOG_INFO(LABEL, "No buffered commands. targetNodeId: %{public}s", targetNodeId_.c_str());
        return;
    }
    if (running_) {
        // task is running, do not need to start one more
        PERMISSION_LOG_WARN(LABEL, "task busy. targetNodeId: %{public}s", targetNodeId_.c_str());
        return;
    }

    running_ = true;
    const std::function<void()> runner = std::bind(&RemoteCommandExecutor::ProcessBufferedCommands, this, true);

    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "fail to get EventHandler");
        return;
    }
    bool result = handler->ProxyPostTask(runner, TASK_NAME);
    if (result == false) {
        PERMISSION_LOG_ERROR(LABEL, "post task failed, targetNodeId: %{public}s", targetNodeId_.c_str());
    }
    PERMISSION_LOG_INFO(LABEL,
        "post task succeed, targetNodeId: %{public}s, taskName: %{public}s",
        targetNodeId_.c_str(),
        TASK_NAME.c_str());
}

int RemoteCommandExecutor::ExecuteRemoteCommand(
    const std::shared_ptr<BaseRemoteCommand> &ptrCommand, const bool isRemote)
{
    std::string uniqueId = ptrCommand->remoteProtocol_.uniqueId;
    PERMISSION_LOG_INFO(LABEL,
        "targetNodeId %{public}s, uniqueId %{public}s, remote %{public}d: start to execute",
        targetNodeId_.c_str(),
        uniqueId.c_str(),
        isRemote);

    ptrCommand->remoteProtocol_.statusCode = Constant::STATUS_CODE_BEFORE_RPC;
    if (!isRemote) {
        // Local device, play myself.
        ptrCommand->Execute();
        int code = ClientProcessResult(ptrCommand);
        PERMISSION_LOG_DEBUG(LABEL,
            "command finished with status: %{public}d, message: %{public}s",
            ptrCommand->remoteProtocol_.statusCode,
            ptrCommand->remoteProtocol_.message.c_str());
        return code;
    }

    std::string responseString =
        ptrChannel_->ExecuteCommand(ptrCommand->remoteProtocol_.commandName, ptrCommand->ToJsonPayload());
    PERMISSION_LOG_INFO(LABEL, "command executed uniqueId %{public}s", uniqueId.c_str());
    if (responseString.empty()) {
        PERMISSION_LOG_WARN(LABEL,
            "targetNodeId %{public}s, uniqueId %{public}s, execute remote command error, response is empty.",
            targetNodeId_.c_str(),
            uniqueId.c_str());
        return Constant::FAILURE;
    }

    std::shared_ptr<BaseRemoteCommand> ptrResponseCommand =
        RemoteCommandFactory::GetInstance().NewRemoteCommandFromJson(
            ptrCommand->remoteProtocol_.commandName, responseString);
    if (ptrResponseCommand == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "targetNodeId %{public}s, get null response command!", targetNodeId_.c_str());
        return Constant::FAILURE;
    }
    int32_t result = ClientProcessResult(ptrResponseCommand);
    if (commands_.empty()) {
        ptrChannel_->CloseConnection();
    }
    PERMISSION_LOG_DEBUG(LABEL,
        "command finished with status: %{public}d, message: %{public}s",
        ptrResponseCommand->remoteProtocol_.statusCode,
        ptrResponseCommand->remoteProtocol_.message.c_str());
    return result;
}

void RemoteCommandExecutor::CreateChannelIfNeeded()
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    if (ptrChannel_ != nullptr) {
        PERMISSION_LOG_INFO(LABEL, "targetNodeId %{public}s, channel is exist.", targetNodeId_.c_str());
        return;
    }

    ptrChannel_ = CreateChannel(targetNodeId_);
}

int RemoteCommandExecutor::ClientProcessResult(const std::shared_ptr<BaseRemoteCommand> &ptrCommand)
{
    std::string uniqueId = ptrCommand->remoteProtocol_.uniqueId;
    if (ptrCommand->remoteProtocol_.statusCode == Constant::STATUS_CODE_BEFORE_RPC) {
        PERMISSION_LOG_ERROR(LABEL,
            "targetNodeId %{public}s, uniqueId %{public}s, status code after RPC is same as before, the remote side "
            "may not "
            "support this command",
            targetNodeId_.c_str(),
            uniqueId.c_str());
        return Constant::FAILURE;
    }

    ptrCommand->Finish();
    int status = ptrCommand->remoteProtocol_.statusCode;
    if (status != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL,
            "targetNodeId %{public}s, uniqueId %{public}s, execute failed, message: %{public}s",
            targetNodeId_.c_str(),
            uniqueId.c_str(),
            ptrCommand->remoteProtocol_.message.c_str());
    } else {
        PERMISSION_LOG_INFO(LABEL,
            "targetNodeId %{public}s, uniqueId %{public}s, execute succeed.",
            targetNodeId_.c_str(),
            uniqueId.c_str());
    }
    return status;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
