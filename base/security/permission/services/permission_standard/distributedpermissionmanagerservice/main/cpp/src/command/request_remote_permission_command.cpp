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

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "base_remote_command.h"
#include "remote_command_factory.h"
#include "remote_command_manager.h"
#include "permission_log.h"
#include "permission_fetcher.h"
#include "subject_device_permission_manager.h"
#include "external_deps.h"
#include "ipc_skeleton.h"
#include "distributed_permission_manager_service.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "RequestRemotePermissionCommand"};
}
RequestRemotePermissionCommand::RequestRemotePermissionCommand(
    const std::string &srcDeviceId, const std::string &dstDeviceId, const std::string &requestId)
    : requestId_(requestId)
{
    remoteProtocol_.commandName = COMMAND_NAME;
    remoteProtocol_.uniqueId = COMMAND_NAME + "-" + requestId;
    remoteProtocol_.srcDeviceId = srcDeviceId;
    remoteProtocol_.dstDeviceId = dstDeviceId;
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;
    remoteProtocol_.requestVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;
}

RequestRemotePermissionCommand::RequestRemotePermissionCommand(const std::string &json)
{
    nlohmann::json jsonObject = nlohmann::json::parse(json, nullptr, false);
    BaseRemoteCommand::FromRemoteProtocolJson(jsonObject);
    if (jsonObject.find("uid") != jsonObject.end() && jsonObject.at("uid").is_number()) {
        uid_ = jsonObject.at("uid").get<int32_t>();
    }
    if (jsonObject.find("requestId") != jsonObject.end() && jsonObject.at("requestId").is_string()) {
        jsonObject.at("requestId").get_to<std::string>(requestId_);
    }
    if (jsonObject.find("bundleName") != jsonObject.end() && jsonObject.at("bundleName").is_string()) {
        jsonObject.at("bundleName").get_to<std::string>(bundleName_);
    }
    if (jsonObject.find("reason") != jsonObject.end() && jsonObject.at("reason").is_string()) {
        jsonObject.at("reason").get_to<std::string>(reason_);
    }
    jsonObject.at("permissions").get_to<std::vector<std::string>>(permissions_);
}

std::string RequestRemotePermissionCommand::ToJsonPayload()
{
    nlohmann::json j = BaseRemoteCommand::ToRemoteProtocolJson();
    j["uid"] = uid_;
    j["requestId"] = requestId_;
    j["bundleName"] = bundleName_;
    j["reason"] = reason_;
    nlohmann::json permissionsJson;
    for (auto permission : permissions_) {
        permissionsJson.emplace_back(permission);
    }
    j["permissions"] = permissionsJson;
    return j.dump();
}

/**
 * Set the more request message when the command is new
 *
 * @param uid the request uid
 * @param permissions the request permissions
 * @param bundleName the request package name
 * @param reason the request reason string
 */
void RequestRemotePermissionCommand::SetRequestPermissionInfo(
    int32_t uid, const std::vector<std::string> &permissions, const std::string &bundleName, const std::string &reason)
{
    uid_ = uid;
    permissions_ = permissions;
    bundleName_ = bundleName;
    reason_ = reason;
}

/**
 * Current the UI is not design, So use the property item to mock the activity operation。<br>
 * if "request." + permission is true, DPMS will grant the sensitive permission
 *
 * @param permissions the permissions that will grant to remote
 */
void RequestRemotePermissionCommand::StartActivityForRequestPermission(std::vector<std::string> &permissions)
{
    if (permissions.empty()) {
        return;
    }
    std::unique_ptr<ExternalDeps> externalDeps = std::make_unique<ExternalDeps>();
    if (externalDeps == nullptr) {
        return;
    }
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    for (auto permission : permissions) {
        if (iBundleManager_->CheckPermission(bundleName_, permission) == 0) {
            PERMISSION_LOG_DEBUG(LABEL,
                "mock Activity DO GRANT permission{uid = %{public}d, requestId = %{public}s }",
                uid_,
                requestId_.c_str());
            SubjectDevicePermissionManager::GetInstance().GrantSensitivePermissionToRemoteApp(
                permission, remoteProtocol_.srcDeviceId, uid_);
        } else {
            PERMISSION_LOG_DEBUG(LABEL,
                "mock Activity DO NOT GRANT permission{uid = %{public}d, requestId = %{public}s }",
                uid_,
                requestId_.c_str());
        }
    }
}

/**
 * Format the permissions to get the permissions that need to use UI to request the permission
 *
 * @param permissions the input permission from subject
 * @param deviceId the deviceId of the subject device
 * @param ruid the uid of the subject
 * @return the permission that need to request by UI
 */
std::vector<std::string> RequestRemotePermissionCommand::FormatRequestPermissions(
    std::vector<std::string> &permissions, const std::string &deviceId, int32_t ruid)
{
    int32_t duid = SubjectDevicePermissionManager::GetInstance().GetDistributedUid(deviceId, ruid);
    std::vector<std::string> needRequestPermissions;
    for (auto permission : permissions) {
        if (permission.empty()) {
            continue;
        }
        PERMISSION_LOG_DEBUG(LABEL, "get sensitive permission = %{public}s from subject request ", permission.c_str());
        if (SubjectDevicePermissionManager::GetInstance().CheckDistributedPermission(duid, permission) ==
            Constant::PERMISSION_GRANTED) {
            PERMISSION_LOG_DEBUG(LABEL,
                "sensitive permission = %{public}s from subject request have been granted before ",
                permission.c_str());
            continue;
        }
        needRequestPermissions.push_back(permission);
    }
    return needRequestPermissions;
}

void RequestRemotePermissionCommand::Prepare()
{
    PERMISSION_LOG_INFO(LABEL,
        "prepare: start as: RequestRemotePermissionCommand{uid = %{public}d, requestId = %{public}s }",
        uid_,
        requestId_.c_str());

    remoteProtocol_.statusCode = Constant::SUCCESS;
    remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;

    PERMISSION_LOG_INFO(LABEL, "prepare: end as: RequestRemotePermissionCommand.");
}

void RequestRemotePermissionCommand::Finish()
{
    remoteProtocol_.statusCode = Constant::SUCCESS;
    PERMISSION_LOG_INFO(LABEL,
        "finish: end as: RequestRemotePermissionCommand{uid = %{public}d, requestId = %{public}s }",
        uid_,
        requestId_.c_str());
}

void RequestRemotePermissionCommand::Execute()
{
    PERMISSION_LOG_INFO(LABEL,
        "execute: start as: RequestRemotePermissionCommand{uid = %{public}d, requestId = %{public}s }",
        uid_,
        requestId_.c_str());

    remoteProtocol_.responseDeviceId = Constant::GetLocalDeviceId();
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;

    std::vector<std::string> needRequestPermissions =
        FormatRequestPermissions(permissions_, remoteProtocol_.srcDeviceId, uid_);

    if (needRequestPermissions.size() > 0) {
        PERMISSION_LOG_INFO(LABEL, "start startActivityForRequestPermission");
        // start active and the the result of the active
        StartActivityForRequestPermission(needRequestPermissions);
        PERMISSION_LOG_INFO(LABEL, "end startActivityForRequestPermission");
    }

    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "fail to get EventHandler");
    } else {
        std::string taskName("RequestRemotePermissionCallbackCommand");
        // auto task = [this, &remoteProtocol_, uid_, requestId_, &bundleName_]() {
        auto task = [this]() {
            // new a command to notice subject the request permission result
            std::shared_ptr<RequestRemotePermissionCallbackCommand> callbackCommand_ =
                RemoteCommandFactory::GetInstance().NewRequestRemotePermissionCallbackCommand(
                    remoteProtocol_.responseDeviceId, remoteProtocol_.srcDeviceId, requestId_, uid_, bundleName_);

            RemoteCommandManager::GetInstance().AddCommand(remoteProtocol_.srcDeviceId, callbackCommand_);
            RemoteCommandManager::GetInstance().ProcessDeviceCommandImmediately(remoteProtocol_.srcDeviceId);
        };
        handler->PostTask(task, taskName);
    }

    remoteProtocol_.statusCode = Constant::SUCCESS;
    remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;

    PERMISSION_LOG_DEBUG(LABEL, "execute: end as: RequestRemotePermissionCommand");
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS