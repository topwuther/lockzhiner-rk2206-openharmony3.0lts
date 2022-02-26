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

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "GetUidPermissionCommand"};
}
namespace {
const int UID_GetPermissions_FAILED = 1;
const int UID_MAX_UID_PERMISSIONS_COUNT_OVER = 2;
const int UID_Execute_SUCCESS_INDEX = 100;
const int UID_GetRegrantedPermissions_FAILED = 100;
const int UID_FINISH_SUCCESS_INDEX = 1000;

void setDefaultCommandProps(const int uid, RemoteProtocol &remoteProtocol_, UidBundleBo &uidPermission_)
{
    // BaseRemoteCommand::FromRemoteProtocolJson(jsonObject);
    remoteProtocol_.commandName = "GetUidPermissionCommand";
    remoteProtocol_.uniqueId = "test-device-id-001:unique-id-001";
    remoteProtocol_.requestVersion = 1;
    remoteProtocol_.srcDeviceId = "test-device-id-001";
    remoteProtocol_.dstDeviceId = "test-device-id-002";
    remoteProtocol_.statusCode = 0;
    remoteProtocol_.message = "default-message";
    remoteProtocol_.responseVersion = 2;
    remoteProtocol_.responseDeviceId = "test-device-id-002";

    // BaseRemoteCommand::FromUidBundlePermissionsJson(jsonObject, uidPermission_);
    uidPermission_.DEFAULT_SIZE = 10;
    uidPermission_.MIN_UID_PACKAGES_NUM = 1;
    uidPermission_.uid = uid;
    uidPermission_.appAttribute = Constant::INITIAL_APP_ATTRIBUTE;
    uidPermission_.uidState = 0;
    {
        BundlePermissionsDto bundles;
        bundles.name = "bundlesName";
        bundles.bundleLabel = "bundlesLabel";
        SignDto signDto;
        signDto.sha256 = "test";
        bundles.sign.push_back(signDto);
        {
            PermissionDto detail;
            detail.name = "permissionName1";
            // detail.bundleName = "bundleName1";
            detail.grantMode = 1;
            // detail.availableScope = 1;
            // detail.label = "label1";
            // detail.labelId = 1;
            // detail.description = "description1";
            // detail.descriptionId = 1;
            detail.status = 1;
            bundles.permissions.push_back(detail);
        }
        {
            PermissionDto detail;
            detail.name = "permissionName2";
            // detail.bundleName = "bundleName2";
            detail.grantMode = 2;
            // detail.availableScope = 2;
            // detail.label = "label2";
            // detail.labelId = 2;
            // detail.description = "description2";
            // detail.descriptionId = 2;
            detail.status = 2;
            bundles.permissions.push_back(detail);
        }
        uidPermission_.bundles.push_back(bundles);
        // uidPermission_.remoteSensitivePermission
    }
}
}  // namespace

GetUidPermissionCommand::GetUidPermissionCommand(
    int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId)
    : uid_(uid)
{
    PERMISSION_LOG_DEBUG(LABEL, "mock GetUidPermissionCommand");
    setDefaultCommandProps(uid_, remoteProtocol_, uidPermission_);

    PERMISSION_LOG_DEBUG(LABEL, "mock GetUidPermissionCommand: reset properties");
    remoteProtocol_.commandName = COMMAND_NAME;
    remoteProtocol_.uniqueId = COMMAND_NAME + "-" + std::to_string(uid);
    remoteProtocol_.srcDeviceId = srcDeviceId;
    remoteProtocol_.dstDeviceId = dstDeviceId;
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;
}

GetUidPermissionCommand::GetUidPermissionCommand(const std::string &json)
{
    // nlohmann::json jsonObject = nlohmann::json::parse(json, nullptr, false);
    // BaseRemoteCommand::FromRemoteProtocolJson(jsonObject);
    // if (jsonObject.find("uid") != jsonObject.end() && jsonObject.at("uid").is_number()) {
    //     uid_ = jsonObject.at("uid").get<int32_t>();
    // }
    // BaseRemoteCommand::FromUidBundlePermissionsJson(jsonObject, uidPermission_);

    PERMISSION_LOG_DEBUG(LABEL, "mock GetUidPermissionCommand2");
    if (json == "{[100]}") {
        uid_ = 100;
        setDefaultCommandProps(uid_, remoteProtocol_, uidPermission_);
    } else if (json == "{[101]}") {
        uid_ = 101;
        setDefaultCommandProps(uid_, remoteProtocol_, uidPermission_);
    } else if (json == "{[1000]}") {
        uid_ = 1000;
        setDefaultCommandProps(uid_, remoteProtocol_, uidPermission_);
    } else if (json == "{[1001]}") {
        uid_ = 1001;
        setDefaultCommandProps(uid_, remoteProtocol_, uidPermission_);
    } else if (json == "{[1002]}") {
        uid_ = 1002;
        setDefaultCommandProps(uid_, remoteProtocol_, uidPermission_);
    } else {
        uid_ = 10000;
        setDefaultCommandProps(uid_, remoteProtocol_, uidPermission_);
    }
    remoteProtocol_.uniqueId = COMMAND_NAME + "-" + std::to_string(uid_);
}

std::string GetUidPermissionCommand::ToJsonPayload()
{
    PERMISSION_LOG_DEBUG(LABEL, "mock ToJsonPayload");
    nlohmann::json j = BaseRemoteCommand::ToRemoteProtocolJson();
    PERMISSION_LOG_DEBUG(LABEL, "ToRemoteProtocolJson");
    j["uid"] = uid_;
    j["uidPermission"] = BaseRemoteCommand::ToUidBundlePermissionsJson(uidPermission_);
    PERMISSION_LOG_DEBUG(LABEL, "ToUidBundlePermissionsJson");
    return j.dump();
}

void GetUidPermissionCommand::Prepare()
{
    PERMISSION_LOG_DEBUG(LABEL, "mock Prepare");
    remoteProtocol_.statusCode = Constant::SUCCESS;
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: end as: GetUidPermissionCommand", __func__);
}

void GetUidPermissionCommand::Execute()
{
    PERMISSION_LOG_DEBUG(LABEL, "mock Execute");
    PERMISSION_LOG_INFO(LABEL, "execute start as: GetUidPermissionCommand{uid = %{public}d }", uid_);

    // std::unique_ptr<ExternalDeps> externalDeps = std::make_unique<ExternalDeps>();
    // if (externalDeps == nullptr) {
    //     return;
    // }
    // iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    // iPermissionManager_ = externalDeps->GetPermissionManager(iPermissionManager_);
    // permissionFetcher_ = std::make_shared<PermissionFetcher>(iBundleManager_, iPermissionManager_);

    // DeviceIdUtil::getDeviceId();
    // IPCSkeleton::GetLocalDeviceID()
    remoteProtocol_.responseDeviceId = 1;
    remoteProtocol_.responseVersion = Constant::DISTRIBUTED_PERMISSION_SERVICE_VERSION;

    // if (permissionFetcher_->GetPermissions(uid_, uidPermission_) <= 0) {
    //     PERMISSION_LOG_DEBUG(LABEL,"%{public}s: resultWrapper code is not SUCCESS.", __func__);
    //     remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
    //     remoteProtocol_.message = Constant::COMMAND_GET_PERMISSIONS_FAILED;
    //     return;
    // }
    if (uid_ == UID_GetPermissions_FAILED) {
        PERMISSION_LOG_DEBUG(LABEL, "%{public}s: resultWrapper code is not SUCCESS.", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_GET_PERMISSIONS_FAILED;
        return;
    }

    // int32_t count = uidPermission_.bundles.permissions.size();

    // if (count > Constant::MAX_UID_PERMISSIONS_COUNT) {
    //     PERMISSION_LOG_ERROR(LABEL,"%{public}s: permissions exceed MAX_UID_PERMISSIONS_COUNT.", __func__);
    //     remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
    //     remoteProtocol_.message = Constant::COMMAND_PERMISSIONS_COUNT_FAILED;
    //     return;
    // }
    if (uid_ == UID_MAX_UID_PERMISSIONS_COUNT_OVER) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: permissions exceed MAX_UID_PERMISSIONS_COUNT.", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_PERMISSIONS_COUNT_FAILED;
        return;
    }

    // Save the query deviceId and uid.
    // int32_t addResult =
    //     ObjectDevicePermissionManager::GetInstance().AddNotifyPermissionMonitorUid(remoteProtocol_.srcDeviceId,
    //     uid_);

    // PERMISSION_LOG_DEBUG(LABEL,
    //     "GetUidPermissionCommand: ObjectDevicePermissionManager AddNotifyPermissionMonitorUid result:  %{public}d }",
    //     addResult);

    // if (addResult == Constant::SUCCESS) {
    //     remoteProtocol_.statusCode = Constant::SUCCESS;
    //     remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;
    // } else {
    //     remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
    //     remoteProtocol_.message = Constant::COMMAND_RESULT_FAILED;
    // }
    if (uid_ >= UID_Execute_SUCCESS_INDEX) {
        remoteProtocol_.statusCode = Constant::SUCCESS;
        remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;
    } else {
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_RESULT_FAILED;
    }
    remoteProtocol_.uniqueId = "test-unique-id-001";
    PERMISSION_LOG_INFO(LABEL, "execute end as: GetUidPermissionCommand");
}

void GetUidPermissionCommand::Finish()
{
    PERMISSION_LOG_DEBUG(LABEL, "mock Finish");
    PERMISSION_LOG_INFO(LABEL, "finish: start as: GetUidPermissionCommand{uid = %{public}d }", uid_);

    // std::unique_ptr<ExternalDeps> externalDeps = std::make_unique<ExternalDeps>();
    // if (externalDeps == nullptr) {
    //     return;
    // }
    // iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    // iPermissionManager_ = externalDeps->GetPermissionManager(iPermissionManager_);
    // permissionFetcher_ = std::make_shared<PermissionFetcher>(iBundleManager_, iPermissionManager_);

    // UidBundleBo rUidPermission;
    // if (permissionFetcher_->GetRegrantedPermissions(uidPermission_, rUidPermission) <= 0) {
    //     PERMISSION_LOG_ERROR(LABEL,"%{public}s: failed to get regranted permissions.", __func__);
    //     remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
    //     remoteProtocol_.message = Constant::COMMAND_GET_REGRANTED_PERMISSIONS_FAILED;
    //     return;
    // }
    if (uid_ == UID_GetRegrantedPermissions_FAILED) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: failed to get regranted permissions.", __func__);
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_GET_REGRANTED_PERMISSIONS_FAILED;
        return;
    }

    // int32_t addResult = SubjectDevicePermissionManager::GetInstance().AddDistributedPermission(
    //     remoteProtocol_.srcDeviceId, rUidPermission);
    // PERMISSION_LOG_DEBUG(LABEL,
    //     "finish: SubjectDevicePermissionManager addSubjectPermission result: %{public}d }", addResult);

    // if (addResult == Constant::SUCCESS) {
    //     remoteProtocol_.statusCode = Constant::SUCCESS;
    //     remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;
    // } else {
    //     remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
    //     remoteProtocol_.message = Constant::COMMAND_RESULT_FAILED;
    // }
    if (uid_ >= UID_FINISH_SUCCESS_INDEX) {
        remoteProtocol_.statusCode = Constant::SUCCESS;
        remoteProtocol_.message = Constant::COMMAND_RESULT_SUCCESS;
    } else {
        remoteProtocol_.statusCode = Constant::FAILURE_BUT_CAN_RETRY;
        remoteProtocol_.message = Constant::COMMAND_RESULT_FAILED;
    }
    PERMISSION_LOG_INFO(LABEL, "finish: end as: GetUidPermissionCommand, code: %{public}d", remoteProtocol_.statusCode);
}

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS