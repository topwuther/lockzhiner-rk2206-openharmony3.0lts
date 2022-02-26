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

#include <memory>
#include "remote_command_factory.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace Security {
namespace Permission {
RemoteCommandFactory &RemoteCommandFactory::GetInstance()
{
    static RemoteCommandFactory instance;
    return instance;
}

std::shared_ptr<SyncUidPermissionCommand> RemoteCommandFactory::NewSyncUidPermissionCommand(
    const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId)
{
    return std::make_shared<SyncUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);
}

std::shared_ptr<GetUidPermissionCommand> RemoteCommandFactory::NewGetUidPermissionCommand(
    const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId)
{
    return std::make_shared<GetUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);
}

std::shared_ptr<DeleteUidPermissionCommand> RemoteCommandFactory::NewDeleteUidPermissionCommand(
    const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId)
{
    return std::make_shared<DeleteUidPermissionCommand>(uid, srcDeviceId, dstDeviceId);
}

std::shared_ptr<RequestRemotePermissionCommand> RemoteCommandFactory::NewRequestRemotePermissionCommand(
    const std::string &srcDeviceId, const std::string &dstDeviceId, const std::string &requestId)
{
    return std::make_shared<RequestRemotePermissionCommand>(srcDeviceId, dstDeviceId, requestId);
}

std::shared_ptr<RequestRemotePermissionCallbackCommand> RemoteCommandFactory::NewRequestRemotePermissionCallbackCommand(
    const std::string &srcDeviceId, const std::string &dstDeviceId, const std::string &requestId, const int32_t uid,
    const std::string &bundleName)
{
    return std::make_shared<RequestRemotePermissionCallbackCommand>(
        srcDeviceId, dstDeviceId, requestId, uid, bundleName);
}

std::shared_ptr<SyncUidStateCommand> RemoteCommandFactory::NewSyncUidStateCommand(
    const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId)
{
    return std::make_shared<SyncUidStateCommand>(uid, srcDeviceId, dstDeviceId);
}

std::shared_ptr<BaseRemoteCommand> RemoteCommandFactory::NewRemoteCommandFromJson(
    const std::string &commandName, const std::string &commandJsonString)
{
    const std::string GET_COMMAND_NAME = "GetUidPermissionCommand";
    const std::string SYNC_COMMAND_NAME = "SyncUidPermissionCommand";
    const std::string DELETE_COMMAND_NAME = "DeleteUidPermissionCommand";
    const std::string REQUEST_COMMAND_NAME = "RequestRemotePermissionCommand";
    const std::string REQUEST_CALLBACK_COMMAND_NAME = "RequestRemotePermissionCallbackCommand";
    const std::string UID_STATE_COMMAND_NAME = "SyncUidStateCommand";

    if (commandName == GET_COMMAND_NAME) {
        return std::make_shared<GetUidPermissionCommand>(commandJsonString);
    } else if (commandName == SYNC_COMMAND_NAME) {
        return std::make_shared<SyncUidPermissionCommand>(commandJsonString);
    } else if (commandName == DELETE_COMMAND_NAME) {
        return std::make_shared<DeleteUidPermissionCommand>(commandJsonString);
    } else if (commandName == REQUEST_COMMAND_NAME) {
        return std::make_shared<RequestRemotePermissionCommand>(commandJsonString);
    } else if (commandName == REQUEST_CALLBACK_COMMAND_NAME) {
        return std::make_shared<RequestRemotePermissionCallbackCommand>(commandJsonString);
    } else if (commandName == UID_STATE_COMMAND_NAME) {
        return std::make_shared<SyncUidStateCommand>(commandJsonString);
    } else {
        return nullptr;
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
