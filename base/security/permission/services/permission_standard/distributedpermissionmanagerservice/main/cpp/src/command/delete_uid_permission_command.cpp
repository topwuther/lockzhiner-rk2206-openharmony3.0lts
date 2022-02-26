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
#include "delete_uid_permission_command.h"
#include "permission_fetcher.h"
#include "permission_log.h"
#include "subject_device_permission_manager.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "DeleteUidPermissionCommand"};
}
DeleteUidPermissionCommand::DeleteUidPermissionCommand(
    const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId)
    : uid_(uid)
{
    remoteProtocol_.commandName = COMMAND_NAME;
    remoteProtocol_.uniqueId = COMMAND_NAME + "-" + std::to_string(uid);
    remoteProtocol_.srcDeviceId = srcDeviceId;
    remoteProtocol_.dstDeviceId = dstDeviceId;
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;
    remoteProtocol_.requestVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;
}

DeleteUidPermissionCommand::DeleteUidPermissionCommand(const std::string &json)
{
    nlohmann::json jsonObject = nlohmann::json::parse(json, nullptr, false);
    BaseRemoteCommand::FromRemoteProtocolJson(jsonObject);
    if (jsonObject.find("uid") != jsonObject.end() && jsonObject.at("uid").is_number()) {
        uid_ = jsonObject.at("uid").get<int32_t>();
    }
}

std::string DeleteUidPermissionCommand::ToJsonPayload()
{
    nlohmann::json j = BaseRemoteCommand::ToRemoteProtocolJson();
    j["uid"] = uid_;
    return j.dump();
}

void DeleteUidPermissionCommand::Prepare()
{
    remoteProtocol_.statusCode = Constant::SUCCESS;
    remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: end as: DeleteUidPermissionCommand", __func__);
}

void DeleteUidPermissionCommand::Execute()
{
    PERMISSION_LOG_INFO(LABEL, "execute: start as: DeleteUidPermissionCommand{uid = %{public}d }", uid_);

    remoteProtocol_.responseDeviceId = Constant::GetLocalDeviceId();
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;

    // Call manager to remove device id to uid pair.
    int32_t removeResult =
        SubjectDevicePermissionManager::GetInstance().RemoveDistributedPermission(remoteProtocol_.srcDeviceId, uid_);
    PERMISSION_LOG_DEBUG(LABEL,
        "execute: SubjectDevicePermissionManager removeDistributedPermission result: %{public}d }",
        removeResult);

    if (removeResult == Constant::SUCCESS) {
        remoteProtocol_.statusCode = Constant::SUCCESS;
        remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;
    } else {
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_RESULT_FAILED;
    }
    PERMISSION_LOG_INFO(LABEL, "execute: end as: DeleteUidPermissionCommand");
}

void DeleteUidPermissionCommand::Finish()
{
    remoteProtocol_.statusCode = Constant::SUCCESS;
    PERMISSION_LOG_DEBUG(LABEL, "clientProcessResult: start as: DeleteUidPermissionCommand{uid = %{public}d }", uid_);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS