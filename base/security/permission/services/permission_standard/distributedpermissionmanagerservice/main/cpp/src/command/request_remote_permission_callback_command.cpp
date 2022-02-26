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

#include "base_remote_command.h"
#include "request_remote_permission_callback_command.h"
#include "permission_log.h"
#include "subject_device_permission_manager.h"
#include "object_device_permission_manager.h"
#include "external_deps.h"
#include "request_remote_permission.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "RequestRemotePermissionCallbackCommand"};
}
RequestRemotePermissionCallbackCommand::RequestRemotePermissionCallbackCommand(const std::string &srcDeviceId,
    const std::string &dstDeviceId, const std::string &requestId, const int32_t uid, const std::string &bundleName)
    : requestId_(requestId), bundleName_(bundleName), uid_(uid)
{
    remoteProtocol_.commandName = COMMAND_NAME;
    remoteProtocol_.uniqueId = COMMAND_NAME + "-" + std::to_string(uid);
    remoteProtocol_.srcDeviceId = srcDeviceId;
    remoteProtocol_.dstDeviceId = dstDeviceId;
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;
    remoteProtocol_.requestVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;
}

RequestRemotePermissionCallbackCommand::RequestRemotePermissionCallbackCommand(const std::string &json)
{
    PERMISSION_LOG_INFO(LABEL, "Create RequestRemotePermissionCallbackCommand  start");
    nlohmann::json jsonObject = nlohmann::json::parse(json, nullptr, false);
    BaseRemoteCommand::FromRemoteProtocolJson(jsonObject);
    if (jsonObject.find("uid") != jsonObject.end() && jsonObject.at("uid").is_number()) {
        jsonObject.at("uid").get_to<int32_t>(uid_);
    }
    if (jsonObject.find("requestId") != jsonObject.end() && jsonObject.at("requestId").is_string()) {
        jsonObject.at("requestId").get_to<std::string>(requestId_);
    }
    if (jsonObject.find("bundleName") != jsonObject.end() && jsonObject.at("bundleName").is_string()) {
        jsonObject.at("bundleName").get_to<std::string>(bundleName_);
    }
    if (jsonObject.find("objectGrantedResult") != jsonObject.end() && jsonObject.at("objectGrantedResult") != nullptr) {
        jsonObject.at("objectGrantedResult").get_to<std::set<std::string>>(objectGrantedResult_);
    }
    PERMISSION_LOG_INFO(LABEL, "Create RequestRemotePermissionCallbackCommand  end");
}

std::string RequestRemotePermissionCallbackCommand::ToJsonPayload()
{
    nlohmann::json j = BaseRemoteCommand::ToRemoteProtocolJson();
    j["uid"] = uid_;
    j["requestId"] = requestId_;
    j["bundleName"] = bundleName_;
    nlohmann::json objectGrantedResultJson;
    for (auto result : objectGrantedResult_) {
        objectGrantedResultJson.emplace_back(result);
    }
    j["objectGrantedResult"] = objectGrantedResultJson;
    return j.dump();
}

void RequestRemotePermissionCallbackCommand::Prepare()
{
    PERMISSION_LOG_INFO(LABEL, "prepare: start as: RequestRemotePermissionCallbackCommand");

    objectGrantedResult_ = SubjectDevicePermissionManager::GetInstance().GetGrantSensitivePermissionToRemoteApp(
        remoteProtocol_.dstDeviceId, uid_);

    remoteProtocol_.statusCode = Constant::SUCCESS;
    remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;

    PERMISSION_LOG_INFO(LABEL, "prepare: end as: RequestRemotePermissionCallbackCommand");
}

void RequestRemotePermissionCallbackCommand::Finish()
{
    remoteProtocol_.statusCode = Constant::SUCCESS;
    PERMISSION_LOG_INFO(LABEL, "finish: RequestRemotePermissionCallbackCommand{uid=%{public}d }", uid_);
}

void RequestRemotePermissionCallbackCommand::Execute()
{
    PERMISSION_LOG_DEBUG(LABEL,
        "execute: start as: RequestRemotePermissionCallbackCommand{uid = %{public}d, requestId = %{public}s }",
        uid_,
        requestId_.c_str());

    remoteProtocol_.responseDeviceId = Constant::GetLocalDeviceId();
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;

    PERMISSION_LOG_INFO(LABEL, "Start process RequestRemotePermissionCallbackCommand on server");

    // use the last object data to refresh the data in subject device
    ObjectDevicePermissionManager::GetInstance().ResetGrantSensitivePermission(
        remoteProtocol_.srcDeviceId, uid_, objectGrantedResult_);
    RequestRemotePermission::GetInstance().RequestResultForResult(requestId_);

    remoteProtocol_.statusCode = Constant::SUCCESS;
    remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;

    PERMISSION_LOG_DEBUG(LABEL, "execute: end as: RequestRemotePermissionCallbackCommand");
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS