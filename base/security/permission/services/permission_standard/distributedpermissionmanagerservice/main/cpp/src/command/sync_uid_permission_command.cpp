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
#include "sync_uid_permission_command.h"
#include "permission_log.h"
#include "subject_device_permission_manager.h"
#include "external_deps.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SyncUidPermissionCommand"};
}
SyncUidPermissionCommand::SyncUidPermissionCommand(
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

SyncUidPermissionCommand::SyncUidPermissionCommand(const std::string &json)
{
    nlohmann::json jsonObject = nlohmann::json::parse(json, nullptr, false);
    BaseRemoteCommand::FromRemoteProtocolJson(jsonObject);
    if (jsonObject.find("uid") != jsonObject.end() && jsonObject.at("uid").is_number()) {
        uid_ = jsonObject.at("uid").get<int32_t>();
    }
    nlohmann::json bundlePermissionsJson = jsonObject.at("uidPermission").get<nlohmann::json>();
    BaseRemoteCommand::FromUidBundlePermissionsJson(bundlePermissionsJson, uidPermission_);
}

std::string SyncUidPermissionCommand::ToJsonPayload()
{
    nlohmann::json j = BaseRemoteCommand::ToRemoteProtocolJson();
    j["uid"] = uid_;
    j["uidPermission"] = BaseRemoteCommand::ToUidBundlePermissionsJson(uidPermission_);
    return j.dump();
}

void SyncUidPermissionCommand::Prepare()
{
    std::unique_ptr<ExternalDeps> externalDeps = std::make_unique<ExternalDeps>();
    if (externalDeps == nullptr) {
        return;
    }
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    iPermissionManager_ = externalDeps->GetPermissionManager(iPermissionManager_);
    permissionFetcher_ = std::make_shared<PermissionFetcher>(iBundleManager_, iPermissionManager_);

    PERMISSION_LOG_INFO(LABEL, "prepare: start as: SyncUidPermissionCommand{uid = %{public}d }", uid_);

    // Query PMS to get all permissions and read to send.
    if (permissionFetcher_->GetPermissions(uid_, uidPermission_) < 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: resultWrapper code is not SUCCESS.", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_GET_PERMISSIONS_FAILED;
        return;
    }

    // Check limit.
    int32_t count = 0;

    for (auto bundle : uidPermission_.bundles) {
        count = bundle.permissions.size();
        if (count != 0) {
            break;
        }
    }

    if (count > Constant::MAX_UID_PERMISSIONS_COUNT) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: permissions exceed MAX_UID_PERMISSIONS_COUNT", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_PERMISSIONS_COUNT_FAILED;
        return;
    }
    remoteProtocol_.statusCode = Constant::SUCCESS;
    remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;

    PERMISSION_LOG_INFO(LABEL, "prepare: end as: SyncUidPermissionCommand");
}

void SyncUidPermissionCommand::Execute()
{
    PERMISSION_LOG_INFO(LABEL, "execute: start as: SyncUidPermissionCommand{uid = %{public}d }", uid_);

    std::unique_ptr<ExternalDeps> externalDeps = std::make_unique<ExternalDeps>();
    if (externalDeps == nullptr) {
        return;
    }
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    iPermissionManager_ = externalDeps->GetPermissionManager(iPermissionManager_);
    permissionFetcher_ = std::make_shared<PermissionFetcher>(iBundleManager_, iPermissionManager_);

    remoteProtocol_.responseDeviceId = Constant::GetLocalDeviceId();
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;

    // Re-granted the permissions
    UidBundleBo rUidPermission;
    if (permissionFetcher_->GetRegrantedPermissions(uidPermission_, rUidPermission) < 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: failed to get regranted permissions.", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_GET_REGRANTED_PERMISSIONS_FAILED;
        return;
    }

    // Save the result.
    int32_t addResult = SubjectDevicePermissionManager::GetInstance().AddDistributedPermission(
        remoteProtocol_.srcDeviceId, rUidPermission);
    PERMISSION_LOG_INFO(
        LABEL, "execute: SubjectDevicePermissionManager addSubjectPermission result: %{public}d }", addResult);

    if (addResult == Constant::SUCCESS) {
        remoteProtocol_.statusCode = Constant::SUCCESS;
        remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;
    } else {
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_RESULT_FAILED;
    }
    PERMISSION_LOG_INFO(LABEL, "execute: end as: SyncUidPermissionCommand");
}

void SyncUidPermissionCommand::Finish()
{
    remoteProtocol_.statusCode = Constant::SUCCESS;
    PERMISSION_LOG_INFO(LABEL, "clientProcessResult: start as: SyncUidPermissionCommand{uid = %{public}d }", uid_);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
