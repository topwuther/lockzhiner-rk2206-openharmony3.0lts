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

#include "distributed_permission_manager_client.h"
#include "distributed_permission_death_recipient.h"
#include "permission_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ipc_skeleton.h"
#include "distributed_permission_kit.h"
#include "parameter.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "DistributedPermissionManagerClient"};
}
const int32_t ERROR = -1;

DistributedPermissionManagerClient &DistributedPermissionManagerClient::GetInstance()
{
    static DistributedPermissionManagerClient instance;
    return instance;
}

int32_t DistributedPermissionManagerClient::AllocateDuid(const std::string &nodeId, const int32_t rUid)
{
    PERMISSION_LOG_INFO(LABEL, "nodeId = %{public}s, rUid = %{public}d", Constant::EncryptDevId(nodeId).c_str(), rUid);
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->AllocateDuid(nodeId, rUid);
}

int32_t DistributedPermissionManagerClient::QueryDuid(const std::string &deviceId, int32_t rUid)
{
    PERMISSION_LOG_INFO(
        LABEL, "deviceId = %{public}s, rUid = %{public}d", Constant::EncryptDevId(deviceId).c_str(), rUid);
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->QueryDuid(deviceId, rUid);
}

int32_t DistributedPermissionManagerClient::CheckDPermission(int32_t dUid, const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "dUid = %{public}d, permissionName = %{public}s", dUid, permissionName.c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->CheckDPermission(dUid, permissionName);
}

int32_t DistributedPermissionManagerClient::CheckPermission(
    const std::string &permissionName, const std::string &nodeId, int32_t pid, int32_t uid)
{
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, nodeId = %{public}s, nodeId = %{public}d, nodeId = %{public}d",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str(),
        pid,
        uid);
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->CheckPermission(permissionName, nodeId, pid, uid);
}

int32_t DistributedPermissionManagerClient::CheckPermission(
    const std::string &permissionName, const std::string &appIdInfo)
{
    PERMISSION_LOG_INFO(
        LABEL, "permissionName = %{public}s, appIdInfo = %{public}s", permissionName.c_str(), appIdInfo.c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }
    AppIdInfo appIdInfoObj;
    if (!DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj)) {
        PERMISSION_LOG_INFO(LABEL, "appIdInfo data invalid");
        return ERROR;
    }
    return distributedPermissionProxy_->CheckPermission(
        permissionName, appIdInfoObj.deviceId, appIdInfoObj.pid, appIdInfoObj.uid);
}

int32_t DistributedPermissionManagerClient::CheckSelfPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->CheckSelfPermission(permissionName);
}

int32_t DistributedPermissionManagerClient::CheckCallingPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->CheckCallingPermission(permissionName);
}

int32_t DistributedPermissionManagerClient::CheckCallingOrSelfPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->CheckCallingOrSelfPermission(permissionName);
}

int32_t DistributedPermissionManagerClient::CheckCallerPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->CheckCallerPermission(permissionName);
}

bool DistributedPermissionManagerClient::IsRestrictedPermission(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "permissionName = %{public}s", permissionName.c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->IsRestrictedPermission(permissionName);
}

int32_t DistributedPermissionManagerClient::VerifyPermissionFromRemote(
    const std::string &permission, const std::string &nodeId, const std::string &appIdInfo)
{
    PERMISSION_LOG_INFO(
        LABEL, "permission = %{public}s, appIdInfo = %{public}s", permission.c_str(), appIdInfo.c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }
    AppIdInfo appIdInfoObj;
    if (!DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj)) {
        PERMISSION_LOG_INFO(LABEL, "appIdInfo data invalid");
        return ERROR;
    }
    return distributedPermissionProxy_->VerifyPermissionFromRemote(
        permission, nodeId, appIdInfoObj.pid, appIdInfoObj.uid);
}

int32_t DistributedPermissionManagerClient::VerifySelfPermissionFromRemote(
    const std::string &permissionName, const std::string &nodeId)
{
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, nodeId = %{public}s",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->VerifySelfPermissionFromRemote(permissionName, nodeId);
}

bool DistributedPermissionManagerClient::CanRequestPermissionFromRemote(
    const std::string &permissionName, const std::string &nodeId)
{
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, nodeId = %{public}s",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str());
    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->CanRequestPermissionFromRemote(permissionName, nodeId);
}

void DistributedPermissionManagerClient::RequestPermissionsFromRemote(const std::vector<std::string> permissions,
    const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
    int32_t reasonResId)
{
    PERMISSION_LOG_INFO(LABEL,
        "bundleName = %{public}s, nodeId = %{public}s",
        bundleName.c_str(),
        Constant::EncryptDevId(nodeId).c_str());
    if (!GetDistributedPermissionProxy()) {
        return;
    }

    return distributedPermissionProxy_->RequestPermissionsFromRemote(
        permissions, callback, nodeId, bundleName, reasonResId);
}

void DistributedPermissionManagerClient::GrantSensitivePermissionToRemoteApp(
    const std::string &permissionName, const std::string &nodeId, int32_t ruid)
{
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, nodeId = %{public}s, ruid = %{public}d",
        permissionName.c_str(),
        Constant::EncryptDevId(nodeId).c_str(),
        ruid);
    if (!GetDistributedPermissionProxy()) {
        return;
    }

    return distributedPermissionProxy_->GrantSensitivePermissionToRemoteApp(permissionName, nodeId, ruid);
}

int32_t DistributedPermissionManagerClient::RegisterUsingPermissionReminder(
    const sptr<OnUsingPermissionReminder> &callback)
{
    PERMISSION_LOG_INFO(LABEL, "enter RegisterUsingPermissionReminder");

    if (callback == nullptr) {
        return ERROR;
    }

    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->RegisterUsingPermissionReminder(callback);
}

int32_t DistributedPermissionManagerClient::UnregisterUsingPermissionReminder(
    const sptr<OnUsingPermissionReminder> &callback)
{
    PERMISSION_LOG_INFO(LABEL, "enter UnregisterUsingPermissionReminder");

    if (callback == nullptr) {
        return ERROR;
    }

    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    return distributedPermissionProxy_->UnregisterUsingPermissionReminder(callback);
}

int32_t DistributedPermissionManagerClient::CheckPermissionAndStartUsing(
    const std::string &permissionName, const std::string &appIdInfo)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    if (permissionName.empty() || appIdInfo.empty()) {
        return ERROR;
    }

    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    AppIdInfo appIdInfoObj;
    if (!DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj)) {
        PERMISSION_LOG_INFO(LABEL, "appIdInfo data invalid");
        return ERROR;
    }

    return distributedPermissionProxy_->CheckPermissionAndStartUsing(
        permissionName, appIdInfoObj.pid, appIdInfoObj.uid, appIdInfoObj.deviceId);
}

int32_t DistributedPermissionManagerClient::CheckCallerPermissionAndStartUsing(const std::string &permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    if (permissionName.empty()) {
        return ERROR;
    }

    if (!GetDistributedPermissionProxy()) {
        return ERROR;
    }

    pid_t pid = IPCSkeleton::GetCallingPid();
    uid_t uid = IPCSkeleton::GetCallingUid();

    char deviceId[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(deviceId, Constant::DEVICE_UUID_LENGTH);

    std::string appIdInfo = DistributedPermissionKit::AppIdInfoHelper::CreateAppIdInfo(pid, uid, deviceId);

    PERMISSION_LOG_INFO(LABEL,
        "CheckCallerPermissionAndStartUsing::calling pid: %{public}d, uid: %{public}d, deviceId: %{private}s",
        pid,
        uid,
        Constant::EncryptDevId(deviceId).c_str());

    return CheckPermissionAndStartUsing(permissionName, appIdInfo);
}

void DistributedPermissionManagerClient::StartUsingPermission(const std::string &permName, const std::string &appIdInfo)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    if (permName.empty() || appIdInfo.empty()) {
        return;
    }

    if (!GetDistributedPermissionProxy()) {
        return;
    }

    AppIdInfo appIdInfoObj;
    if (!DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj)) {
        PERMISSION_LOG_INFO(LABEL, "appIdInfo data invalid");
        return;
    }

    return distributedPermissionProxy_->StartUsingPermission(
        permName, appIdInfoObj.pid, appIdInfoObj.uid, appIdInfoObj.deviceId);
}

void DistributedPermissionManagerClient::StopUsingPermission(const std::string &permName, const std::string &appIdInfo)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s: called!", __func__);

    if (permName.empty() || appIdInfo.empty()) {
        return;
    }

    if (!GetDistributedPermissionProxy()) {
        return;
    }

    AppIdInfo appIdInfoObj;
    if (!DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj)) {
        PERMISSION_LOG_INFO(LABEL, "appIdInfo data invalid");
        return;
    }

    return distributedPermissionProxy_->StopUsingPermission(
        permName, appIdInfoObj.pid, appIdInfoObj.uid, appIdInfoObj.deviceId);
}

void DistributedPermissionManagerClient::ResetDistributedPermissionProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((distributedPermissionProxy_ != nullptr) && (distributedPermissionProxy_->AsObject() != nullptr)) {
        distributedPermissionProxy_->AsObject()->RemoveDeathRecipient(recipient_);
    }
    distributedPermissionProxy_ = nullptr;
}

void DistributedPermissionManagerClient::AddPermissionUsedRecord(
    const std::string &permissionName, const std::string &appIdInfo, const int32_t sucCount, const int32_t failCount)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    PERMISSION_LOG_INFO(LABEL,
        "permissionName = %{public}s, appIdInfo = %{public}s, sucCount = "
        "%{public}d, failCount = %{public}d",
        permissionName.c_str(),
        appIdInfo.c_str(),
        sucCount,
        failCount);

    if (permissionName.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s permissionName invalid", __func__);
        return;
    }
    Permission::AppIdInfo appIdInfoObj;
    if (!DistributedPermissionKit::AppIdInfoHelper::ParseAppIdInfo(appIdInfo, appIdInfoObj)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s appIdInfo data invalid", __func__);
        return;
    }

    if (appIdInfoObj.deviceId.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s deviceId invalid", __func__);
        return;
    }

    if (!GetDistributedPermissionProxy()) {
        return;
    }

    distributedPermissionProxy_->AddPermissionsRecord(
        permissionName, appIdInfoObj.deviceId, appIdInfoObj.uid, sucCount, failCount);
}

int32_t DistributedPermissionManagerClient::GetPermissionUsedRecords(
    const QueryPermissionUsedRequest &request, QueryPermissionUsedResult &result)
{
    if (!GetDistributedPermissionProxy()) {
        return Constant::FAILURE_DPMS;
    }
    nlohmann::json jsonObj = request.to_json(request);
    std::string queryJsonStr = jsonObj.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);
    unsigned long zipLen = queryJsonStr.length();
    unsigned long len = compressBound(zipLen);
    if (len <= 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: compress length less than 0!", __func__);
        return Constant::FAILURE;
    }
    unsigned char *buf = (unsigned char *)malloc(len + 1);

    if (!ZipUtil::ZipCompress(queryJsonStr, zipLen, buf, len)) {
        return Constant::FAILURE;
    }
    std::string queryGzipStr;
    Base64Util::Encode(buf, len, queryGzipStr);

    if (buf) {
        free(buf);
        buf = NULL;
    }
    std::string resultGzipStr;
    int32_t ret = distributedPermissionProxy_->GetPermissionRecords(queryGzipStr, len, zipLen, resultGzipStr);
    if (len <= 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: compress length less than 0!", __func__);
        return Constant::FAILURE;
    }
    unsigned char *pOut = (unsigned char *)malloc(len + 1);
    Base64Util::Decode(resultGzipStr, pOut, len);
    std::string resultJsonStr;
    if (!ZipUtil::ZipUnCompress(pOut, len, resultJsonStr, zipLen)) {
        return Constant::FAILURE;
    }
    if (pOut) {
        free(pOut);
        pOut = NULL;
    }
    nlohmann::json jsonRes = nlohmann::json::parse(resultJsonStr, nullptr, false);
    result.from_json(jsonRes, result);
    return ret;
}

int32_t DistributedPermissionManagerClient::GetPermissionUsedRecords(
    const QueryPermissionUsedRequest &request, const sptr<OnPermissionUsedRecord> &callback)
{
    if (!GetDistributedPermissionProxy()) {
        return Constant::FAILURE_DPMS;
    }
    nlohmann::json jsonObj = request.to_json(request);
    std::string queryJsonStr = jsonObj.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);

    unsigned long zipLen = queryJsonStr.length();
    unsigned long len = compressBound(zipLen);
    if (len <= 0) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: compress length less than 0!", __func__);
        return Constant::FAILURE;
    }
    unsigned char *buf = (unsigned char *)malloc(len + 1);
    if (!ZipUtil::ZipCompress(queryJsonStr, zipLen, buf, len)) {
        return Constant::FAILURE;
    }
    std::string queryGzipStr;
    Base64Util::Encode(buf, len, queryGzipStr);

    if (buf) {
        free(buf);
        buf = NULL;
    }
    int32_t ret = distributedPermissionProxy_->GetPermissionRecords(queryGzipStr, len, zipLen, callback);
    return ret;
}

bool DistributedPermissionManagerClient::GetDistributedPermissionProxy()
{
    if (!distributedPermissionProxy_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!distributedPermissionProxy_) {
            sptr<ISystemAbilityManager> systemAbilityManager =
                SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (!systemAbilityManager) {
                PERMISSION_LOG_ERROR(LABEL, "failed to get systemAbilityManager.");
                return false;
            }

            sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(
                IDistributedPermission::SA_ID_DISTRIBUTED_PERMISSION_MANAGER_SERVICE);
            if (!remoteObject) {
                PERMISSION_LOG_ERROR(LABEL, "failed to get remoteObject.");
                return false;
            }

            distributedPermissionProxy_ = iface_cast<IDistributedPermission>(remoteObject);
            if ((!distributedPermissionProxy_) || (!distributedPermissionProxy_->AsObject())) {
                PERMISSION_LOG_ERROR(LABEL, "failed to get distributedPermissionProxy_.");
                return false;
            }

            recipient_ = new DistributedPermissionDeathRecipient();
            if (!recipient_) {
                PERMISSION_LOG_ERROR(LABEL, "failed to new recipient_.");
                return false;
            }
            distributedPermissionProxy_->AsObject()->AddDeathRecipient(recipient_);
        }
    }

    return true;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
