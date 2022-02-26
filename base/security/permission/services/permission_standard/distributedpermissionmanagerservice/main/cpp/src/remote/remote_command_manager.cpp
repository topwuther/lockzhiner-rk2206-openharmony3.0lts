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

#include "remote_command_manager.h"
#include "device_info_manager.h"
#include "object_device_permission_manager.h"
#include "subject_device_permission_manager.h"
#include "remind_info.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "RemoteCommandManager"};
}
RemoteCommandManager::RemoteCommandManager() : executors_(), mutex_()
{
    PERMISSION_LOG_DEBUG(LABEL, "RemoteCommandManager()");
}

RemoteCommandManager::~RemoteCommandManager()
{
    PERMISSION_LOG_DEBUG(LABEL, "~RemoteCommandManager()");
}

RemoteCommandManager &RemoteCommandManager::GetInstance()
{
    static RemoteCommandManager instance;
    return instance;
}

void RemoteCommandManager::Init()
{
    PERMISSION_LOG_DEBUG(LABEL, "Init()");
}

int RemoteCommandManager::AddCommand(const std::string &udid, const std::shared_ptr<BaseRemoteCommand> &command)
{
    if (udid.empty() || command == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "invalid udid: %{public}s, or null command", udid.c_str());
        return Constant::FAILURE;
    }
    std::string uniqueId = command->remoteProtocol_.uniqueId;
    PERMISSION_LOG_INFO(LABEL, "udid: %{public}s, add uniqueId: %{public}s", udid.c_str(), uniqueId.c_str());

    std::shared_ptr<RemoteCommandExecutor> executor = GetOrCreateRemoteCommandExecutor(udid);
    if (executor == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "cannot get or create remote command executor");
        return Constant::FAILURE;
    }

    int result = executor->AddCommand(command);
    PERMISSION_LOG_INFO(LABEL, "udid: %{public}s, add command result: %{public}d ", udid.c_str(), result);
    return result;
}

void RemoteCommandManager::RemoveCommand(const std::string &udid)
{
    PERMISSION_LOG_INFO(LABEL, "remove command, udid: %{public}s", udid.c_str());
    executors_.erase(udid);
}

int RemoteCommandManager::ExecuteCommand(const std::string &udid, const std::shared_ptr<BaseRemoteCommand> &command)
{
    if (udid.empty() || command == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "invalid udid: %{public}s, or null command", udid.c_str());
        return Constant::FAILURE;
    }
    std::string uniqueId = command->remoteProtocol_.uniqueId;
    PERMISSION_LOG_INFO(LABEL, "start with udid: %{public}s , uniqueId: %{public}s ", udid.c_str(), uniqueId.c_str());

    std::shared_ptr<RemoteCommandExecutor> executor = GetOrCreateRemoteCommandExecutor(udid);
    if (executor == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "cannot get or create remote command executor");
        return Constant::FAILURE;
    }

    int result = executor->ProcessOneCommand(command);
    PERMISSION_LOG_INFO(LABEL, "remoteCommandExecutor processOneCommand result:%{public}d ", result);
    return result;
}

int RemoteCommandManager::ProcessDeviceCommandImmediately(const std::string &udid)
{
    if (udid.empty()) {
        PERMISSION_LOG_WARN(LABEL, "invalid udid: %{public}s", udid.c_str());
        return Constant::FAILURE;
    }
    PERMISSION_LOG_INFO(LABEL, "start with udid:%{public}s ", udid.c_str());
    auto executorIt = executors_.find(udid);
    if (executorIt == executors_.end()) {
        PERMISSION_LOG_ERROR(LABEL, "no executor found, udid:%{public}s", udid.c_str());
        return Constant::FAILURE;
    }

    auto executor = executorIt->second;
    if (executor == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "RemoteCommandExecutor is null for udid %{public}s ", udid.c_str());
        return Constant::FAILURE;
    }

    int result = executor->ProcessBufferedCommands();
    PERMISSION_LOG_INFO(LABEL, "processBufferedCommands result: %{public}d", result);
    return result;
}

int RemoteCommandManager::Loop()
{
    PERMISSION_LOG_INFO(LABEL, "start");
    for (auto it = executors_.begin(); it != executors_.end(); it++) {
        PERMISSION_LOG_INFO(LABEL, "udid:%{public}s", it->first.c_str());
        (*it).second->ProcessBufferedCommandsWithThread();
    }
    return Constant::SUCCESS;
}

/**
 * caller: service connection listener
 */
void RemoteCommandManager::Clear()
{
    PERMISSION_LOG_INFO(LABEL, "remove all remote command executors.");

    std::map<std::string, std::shared_ptr<RemoteCommandExecutor>> dummy;
    executors_.swap(dummy);
    executors_.clear();
}

/**
 * caller: device listener
 */
int RemoteCommandManager::NotifyDeviceOnline(const std::string &nodeId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(nodeId)) {
        PERMISSION_LOG_INFO(LABEL, "invalid nodeId: %{public}s", nodeId.c_str());
        return Constant::FAILURE;
    }
    PERMISSION_LOG_INFO(LABEL, "operation start with nodeId:  %{public}s", nodeId.c_str());

    auto executor = GetOrCreateRemoteCommandExecutor(nodeId);
    std::unique_lock<std::mutex> lock(mutex_);
    if (executor == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "cannot get or create remote command executor");
        return Constant::FAILURE;
    }

    if (executor->GetChannel() == nullptr) {
        auto channel = RemoteCommandExecutor::CreateChannel(nodeId);
        if (channel == nullptr) {
            PERMISSION_LOG_ERROR(LABEL, "create channel failed.");
            return Constant::FAILURE;
        }
        executor->SetChannel(channel);
    }

    return Constant::SUCCESS;
}

/**
 * caller: device listener
 */
int RemoteCommandManager::NotifyDeviceOffline(const std::string &nodeId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(nodeId)) {
        PERMISSION_LOG_INFO(LABEL, "invalid nodeId: %{public}s", nodeId.c_str());
        return Constant::FAILURE;
    }
    PERMISSION_LOG_INFO(LABEL, "operation start with nodeId:  %{public}s", nodeId.c_str());

    auto channel = GetExecutorChannel(nodeId);
    if (channel != nullptr) {
        channel->Release();
    }

    std::unique_lock<std::mutex> lock(mutex_);
    RemoveCommand(nodeId);
    lock.unlock();

    int rdm = SubjectDevicePermissionManager::GetInstance().RemoveDistributedPermission(nodeId);
    if (rdm != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(
            LABEL, "remove subject device distributed permission failed, nodeId:  %{public}s", nodeId.c_str());
    }
    ObjectDevicePermissionManager::GetInstance().RemoveObject(nodeId);

    RemindInfo::GetInstance().DeviceDisConnection(nodeId);

    PERMISSION_LOG_INFO(LABEL, "complete");
    return Constant::SUCCESS;
}

std::shared_ptr<RemoteCommandExecutor> RemoteCommandManager::GetOrCreateRemoteCommandExecutor(const std::string &nodeId)
{
    PERMISSION_LOG_DEBUG(LABEL, "begin, nodeId %{public}s", nodeId.c_str());

    std::unique_lock<std::mutex> lock(mutex_);
    auto executorIter = executors_.find(nodeId);
    if (executorIter != executors_.end()) {
        return executorIter->second;
    }

    auto executor = std::make_shared<RemoteCommandExecutor>(nodeId);
    if (executor == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "cannot create remote command executor, nodeId: %{public}s", nodeId.c_str());
        return nullptr;
    }

    executors_.insert(std::pair<std::string, std::shared_ptr<RemoteCommandExecutor>>(nodeId, executor));
    PERMISSION_LOG_DEBUG(LABEL, "executor added, nodeId: %{public}s", nodeId.c_str());
    return executor;
}

/**
 * caller: session listener(onBytesReceived), device listener(offline)
 */
std::shared_ptr<RpcChannel> RemoteCommandManager::GetExecutorChannel(const std::string &nodeId)
{
    std::string udid = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    PERMISSION_LOG_DEBUG(LABEL, "convert udid, nodeId:%{public}s, udid: %{public}s", nodeId.c_str(), udid.c_str());
    if (!DistributedDataValidator::IsDeviceIdValid(udid)) {
        PERMISSION_LOG_WARN(
            LABEL, "converted udid is invalid, nodeId:%{public}s, udid: %{public}s", nodeId.c_str(), udid.c_str());
        return nullptr;
    }
    std::map<std::string, std::shared_ptr<RemoteCommandExecutor>>::iterator iter = executors_.find(udid);
    if (iter == executors_.end()) {
        PERMISSION_LOG_INFO(LABEL, "executor not found");
        return nullptr;
    }
    std::shared_ptr<RemoteCommandExecutor> executor = iter->second;
    if (executor == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "executor is null");
        return nullptr;
    }
    return executor->GetChannel();
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
