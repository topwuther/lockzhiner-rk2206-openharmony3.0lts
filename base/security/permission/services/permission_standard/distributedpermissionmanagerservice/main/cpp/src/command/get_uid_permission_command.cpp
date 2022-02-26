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
#include "get_uid_permission_command.h"
#include "permission_log.h"
#include "subject_device_permission_manager.h"
#include "object_device_permission_manager.h"
#include "external_deps.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "GetUidPermissionCommand"};
}
GetUidPermissionCommand::GetUidPermissionCommand(
    int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId)
    : uid_(uid)
{
    remoteProtocol_.commandName = COMMAND_NAME;
    remoteProtocol_.uniqueId = COMMAND_NAME + "-" + std::to_string(uid);
    remoteProtocol_.srcDeviceId = srcDeviceId;
    remoteProtocol_.dstDeviceId = dstDeviceId;
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;
    remoteProtocol_.requestVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;
}

GetUidPermissionCommand::GetUidPermissionCommand(const std::string &json)
{
    nlohmann::json jsonObject = nlohmann::json::parse(json, nullptr, false);
    BaseRemoteCommand::FromRemoteProtocolJson(jsonObject);
    if (jsonObject.find("uid") != jsonObject.end() && jsonObject.at("uid").is_number()) {
        uid_ = jsonObject.at("uid").get<int32_t>();
    }
    nlohmann::json bundlePermissionsJson = jsonObject.at("uidPermission").get<nlohmann::json>();
    BaseRemoteCommand::FromUidBundlePermissionsJson(bundlePermissionsJson, uidPermission_);
}

std::string GetUidPermissionCommand::ToJsonPayload()
{
    nlohmann::json j = BaseRemoteCommand::ToRemoteProtocolJson();
    j["uid"] = uid_;
    j["uidPermission"] = BaseRemoteCommand::ToUidBundlePermissionsJson(uidPermission_);
    return j.dump();
}

void GetUidPermissionCommand::Prepare()
{
    remoteProtocol_.statusCode = Constant::SUCCESS;
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: end as: GetUidPermissionCommand", __func__);
}

void GetUidPermissionCommand::Execute()
{
    PERMISSION_LOG_INFO(LABEL, "execute: start as: GetUidPermissionCommand{uid = %{public}d }", uid_);

    std::unique_ptr<ExternalDeps> externalDeps = std::make_unique<ExternalDeps>();
    if (externalDeps == nullptr) {
        return;
    }
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    iPermissionManager_ = externalDeps->GetPermissionManager(iPermissionManager_);
    permissionFetcher_ = std::make_shared<PermissionFetcher>(iBundleManager_, iPermissionManager_);

    remoteProtocol_.responseDeviceId = Constant::GetLocalDeviceId();
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;

    // Use this uid to get permission from PMS. Then save the publisher information, which is deviceId and uid.
    if (permissionFetcher_->GetPermissions(uid_, uidPermission_) < 0) {
        PERMISSION_LOG_DEBUG(LABEL, "%{public}s: resultWrapper code is not SUCCESS.", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_GET_PERMISSIONS_FAILED;
        return;
    }

    int32_t count = 0;

    for (auto bundle : uidPermission_.bundles) {
        count = bundle.permissions.size();
        if (count != 0) {
            break;
        }
    }

    // Check limit.
    if (count > Constant::MAX_UID_PERMISSIONS_COUNT) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: permissions exceed MAX_UID_PERMISSIONS_COUNT.", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_PERMISSIONS_COUNT_FAILED;
        return;
    }

    PERMISSION_LOG_DEBUG(LABEL,
        "GetUidPermissionCommand: ObjectDevicePermissionManager Save the query deviceId and uid uid:  %{public}d, "
        "deviceId: %{public}s }",
        uid_,
        remoteProtocol_.srcDeviceId.c_str());

    // Save the query deviceId and uid.
    int32_t addResult =
        ObjectDevicePermissionManager::GetInstance().AddNotifyPermissionMonitorUid(remoteProtocol_.srcDeviceId, uid_);

    PERMISSION_LOG_DEBUG(LABEL,
        "GetUidPermissionCommand: ObjectDevicePermissionManager AddNotifyPermissionMonitorUid result:  %{public}d }",
        addResult);

    if (addResult == Constant::SUCCESS) {
        remoteProtocol_.statusCode = Constant::SUCCESS;
        remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;
    } else {
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_RESULT_FAILED;
    }
    PERMISSION_LOG_INFO(LABEL, "execute: end as: GetUidPermissionCommand");
}

void GetUidPermissionCommand::Finish()
{
    PERMISSION_LOG_INFO(LABEL, "finish: start as: GetUidPermissionCommand{uid = %{public}d }", uid_);

    if (remoteProtocol_.statusCode != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: failed to GetUidPermissionCommand finish.", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_RESULT_FAILED;
        return;
    }

    std::unique_ptr<ExternalDeps> externalDeps = std::make_unique<ExternalDeps>();
    if (externalDeps == nullptr) {
        return;
    }
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    iPermissionManager_ = externalDeps->GetPermissionManager(iPermissionManager_);
    permissionFetcher_ = std::make_shared<PermissionFetcher>(iBundleManager_, iPermissionManager_);

    // Query PMS to filter uidPermissions and get the granted ones.
    UidBundleBo rUidPermission;
    if (permissionFetcher_->GetRegrantedPermissions(uidPermission_, rUidPermission) < 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: failed to get regranted permissions.", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_GET_REGRANTED_PERMISSIONS_FAILED;
        return;
    }

    int32_t addResult = SubjectDevicePermissionManager::GetInstance().AddDistributedPermission(
        remoteProtocol_.responseDeviceId, rUidPermission);
    PERMISSION_LOG_DEBUG(
        LABEL, "finish: SubjectDevicePermissionManager addSubjectPermission result: %{public}d }", addResult);

    if (addResult == Constant::SUCCESS) {
        remoteProtocol_.statusCode = Constant::SUCCESS;
        remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;
    } else {
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_RESULT_FAILED;
    }
    PERMISSION_LOG_INFO(LABEL, "finish: end as: GetUidPermissionCommand");
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS