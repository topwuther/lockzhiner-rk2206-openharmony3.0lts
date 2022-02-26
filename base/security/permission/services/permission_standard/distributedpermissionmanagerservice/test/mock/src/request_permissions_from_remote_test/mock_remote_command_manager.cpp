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
#include "request_remote_permission.h"
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
int RemoteCommandManager::AddCommand(const std::string &deviceId, const std::shared_ptr<BaseRemoteCommand> &command)
{
    return Constant::SUCCESS;
}
void RemoteCommandManager::RemoveCommand(const std::string &deviceId)
{
    PERMISSION_LOG_INFO(LABEL, "remove command, deviceId: %{public}s", deviceId.c_str());
    executors_.erase(deviceId);
}
int RemoteCommandManager::ExecuteCommand(const std::string &deviceId, const std::shared_ptr<BaseRemoteCommand> &command)
{
    if (deviceId == "deviceId") {
        RequestRemotePermission::GetInstance().RequestResultForResult(deviceId);
        return 0;
    }
    PERMISSION_LOG_INFO(LABEL, "run timeout case!!!!!!!!!!!");
    int32_t timeout = 61000;
    auto time = std::chrono::milliseconds(timeout);
    std::this_thread::sleep_for(time);
    return -1;
}
int RemoteCommandManager::ProcessDeviceCommandImmediately(const std::string &deviceId)
{
    if (deviceId == "deviceId") {
        RequestRemotePermission::GetInstance().RequestResultForResult(deviceId);
        return 0;
    }
    PERMISSION_LOG_INFO(LABEL, "run timeout case!!!!!!!!!!!");
    int32_t timeout = 1100;
    auto time = std::chrono::milliseconds(timeout);
    std::this_thread::sleep_for(time);
    return Constant::FAILURE;
}
int RemoteCommandManager::Loop()
{
    PERMISSION_LOG_INFO(LABEL, "loop: start");

    return Constant::SUCCESS;
}
/**
 * caller: service connection listener
 */
void RemoteCommandManager::Clear()
{
    PERMISSION_LOG_INFO(LABEL, "clear: remove all remote command executors.");

    std::map<std::string, std::shared_ptr<RemoteCommandExecutor>> dummy;
    executors_.swap(dummy);
    executors_.clear();
}
/**
 * caller: device listener
 */
int RemoteCommandManager::NotifyDeviceOnline(const std::string &deviceId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_INFO(LABEL, "notifyDeviceOnline: invalid deviceId: %{public}s", deviceId.c_str());
        return Constant::FAILURE;
    }
    PERMISSION_LOG_INFO(LABEL, "notifyDeviceOnline: operation start with deviceId:  %{public}s", deviceId.c_str());

    auto executor = GetOrCreateRemoteCommandExecutor(deviceId);
    std::unique_lock<std::mutex> lock(mutex_);
    if (executor == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "notifyDeviceOnline: cannot get or create remote command executor");
        return Constant::FAILURE;
    }
    if (executor->GetChannel() == nullptr) {
        auto channel = RemoteCommandExecutor::CreateChannel(deviceId);
        if (channel == nullptr) {
            PERMISSION_LOG_ERROR(LABEL, "notifyDeviceOnline: create channel failed.");
            return Constant::FAILURE;
        }
        executor->SetChannel(channel);
    }
    return Constant::SUCCESS;
}
/**
 * caller: device listener
 */
int RemoteCommandManager::NotifyDeviceOffline(const std::string &deviceId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_INFO(LABEL, "notifyDeviceOffline: invalid deviceId: %{public}s", deviceId.c_str());
        return Constant::FAILURE;
    }
    PERMISSION_LOG_INFO(LABEL, "notifyDeviceOffline: operation start with deviceId:  %{public}s", deviceId.c_str());

    auto channel = GetExecutorChannel(deviceId);
    if (channel != nullptr) {
        channel->Release();
    }
    std::unique_lock<std::mutex> lock(mutex_);
    RemoveCommand(deviceId);
    lock.unlock();
    SubjectDevicePermissionManager::GetInstance().RemoveDistributedPermission(deviceId);
    ObjectDevicePermissionManager::GetInstance().RemoveObject(deviceId);
    PERMISSION_LOG_INFO(LABEL, "notifyDeviceOffline: complete");
    return Constant::SUCCESS;
}
std::shared_ptr<RemoteCommandExecutor> RemoteCommandManager::GetOrCreateRemoteCommandExecutor(
    const std::string &deviceId)
{
    PERMISSION_LOG_DEBUG(LABEL, "getOrCreateRemoteCommandExecutor with deviceId %{public}s", deviceId.c_str());

    std::unique_lock<std::mutex> lock(mutex_);
    auto executorIter = executors_.find(deviceId);

    if (executorIter != executors_.end()) {
        return executorIter->second;
    }
    auto executor = std::make_shared<RemoteCommandExecutor>(deviceId);
    if (executor == nullptr) {
        PERMISSION_LOG_INFO(LABEL,
            "GetOrCreateRemoteCommandExecutor: cannot create remote command executor, deviceId: %{public}s",
            deviceId.c_str());
        return nullptr;
    }
    executors_.insert(std::pair<std::string, std::shared_ptr<RemoteCommandExecutor>>(deviceId, executor));
    return executor;
}
/**
 * caller: session listener(onBytesReceived), device listener(offline)
 */
std::shared_ptr<RpcChannel> RemoteCommandManager::GetExecutorChannel(const std::string &nodeId)
{
    std::string deviceId = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(nodeId);
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_INFO(LABEL, "getExecutorChannel: deviceId is invalid, deviceId:%{public}s", deviceId.c_str());
        return nullptr;
    }
    std::map<std::string, std::shared_ptr<RemoteCommandExecutor>>::iterator iter = executors_.find(deviceId);
    if (iter == executors_.end()) {
        return nullptr;
    }
    std::shared_ptr<RemoteCommandExecutor> executor = iter->second;
    if (executor == nullptr) {
        return nullptr;
    }
    return executor->GetChannel();
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
