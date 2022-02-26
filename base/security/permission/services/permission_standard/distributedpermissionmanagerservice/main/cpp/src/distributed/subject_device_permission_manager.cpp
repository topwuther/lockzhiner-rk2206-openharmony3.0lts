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

#include "subject_device_permission_manager.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SubjectDevicePermissionManager"};
}
SubjectDevicePermissionManager &SubjectDevicePermissionManager::GetInstance()
{
    static SubjectDevicePermissionManager instance;
    return instance;
}

int32_t SubjectDevicePermissionManager::GetDistributedUid(const std::string &deviceId, const int32_t uid) const
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        return Constant::INVALID_DEVICE_ID;
    }
    if (!DistributedDataValidator::IsUidValid(uid)) {
        return Constant::INVALID_RUID;
    }
    return DistributedUidAllocator::GetInstance().GetDuid(deviceId, uid);
}

int32_t SubjectDevicePermissionManager::AllocateDistributedUid(const std::string &deviceId, const int32_t uid) const
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, nodeId: %{public}s, uid: %{public}d",
        __func__,
        Constant::EncryptDevId(deviceId).c_str(),
        uid);
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_ERROR(
            LABEL, "allocateDuid: deviceId invalid, return INVALID_DEVICE_ID: %d", Constant::INVALID_DEVICE_ID);
        return Constant::INVALID_DEVICE_ID;
    }

    if (!DistributedDataValidator::IsUidValid(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "allocateDuid: ruid is invalid, return INVALID_RUID: %d ", Constant::INVALID_RUID);
        return Constant::INVALID_RUID;
    }

    // For system uid, only return the id and will not store the information in the data structure.
    if (DistributedDataValidator::IsSpecRuid(uid)) {
        PERMISSION_LOG_INFO(LABEL, "allocateDuid: uid is special, allocate by DuidTranslator allocateDuid instead");
        return DistributedUidAllocator::GetInstance().AllocateDuid(deviceId, uid);
    }

    // Try to get duid, check if there were duid been allocated, no need to get again if duid already exist.
    const int32_t existDuid = DistributedUidAllocator::GetInstance().GetDuid(deviceId, uid);
    if (existDuid != Constant::DISTRIBUTED_UID_NOT_EXIST) {
        if (!DistributedDataValidator::IsDuidValidAndNotSpecial(existDuid)) {
            PERMISSION_LOG_ERROR(LABEL,
                "allocateDuid: getDuid got invalid duid, return INVALID_DUID: %d ",
                Constant::INVALID_DISTRIBUTED_UID);
            return Constant::INVALID_DISTRIBUTED_UID;
        }

        if (IsMappingReady(existDuid)) {
            PERMISSION_LOG_DEBUG(LABEL,
                "%{public}s: duid: %{public}d has been allocated by subject device: %{public}s and uid: %{public}d",
                __func__,
                existDuid,
                Constant::EncryptDevId(deviceId).c_str(),
                uid);
            return existDuid;
        }
    }
    // Synchronous blocked wait for getUidPermissionCommand, until other process finished addSubjectPermission.
    char deviceIdCharArray[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(deviceIdCharArray, Constant::DEVICE_UUID_LENGTH);
    const std::shared_ptr<GetUidPermissionCommand> getUidPermissionCommand =
        RemoteCommandFactory::GetInstance().NewGetUidPermissionCommand(uid, deviceIdCharArray, deviceId);

    const int32_t resultCode = RemoteCommandManager::GetInstance().ExecuteCommand(deviceId, getUidPermissionCommand);
    if (resultCode != Constant::SUCCESS) {
        PERMISSION_LOG_ERROR(LABEL,
            "allocateDuid: RemoteExecutorManager executeCommand GetUidPermissionCommand failed, return %d",
            resultCode);
        return resultCode;
    }
    PERMISSION_LOG_INFO(LABEL, "allocateDuid: get resultCode: %d", resultCode);
    return DistributedUidAllocator::GetInstance().GetDuid(deviceId, uid);
}

int32_t SubjectDevicePermissionManager::WaitDuidReady(
    const std::string &deviceId, const int32_t uid, const int32_t timeout)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "waitDuidReady: invalid deviceId: %s", Constant::EncryptDevId(deviceId).c_str());
        return Constant::INVALID_DEVICE_ID;
    }
    if (!DistributedDataValidator::IsUidValid(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "waitDuidReady: invalid uid: %d", uid);
        return Constant::INVALID_RUID;
    }
    if (timeout < 0 || timeout > Constant::MAX_WAITING_TIME) {
        PERMISSION_LOG_INFO(LABEL, "waitDuidReady: parameter invalid, timeout: %d", timeout);
        return Constant::FAILURE;
    }
    DeviceInfo deviceInfo;
    if (DeviceInfoManager::GetInstance().GetDeviceInfo(deviceId, DeviceIdType::UNIQUE_DISABILITY_ID, deviceInfo)) {
        const std::string universallyUniqueId = deviceInfo.deviceId.universallyUniqueId;
        const std::string networkId = deviceInfo.deviceId.networkId;
        if (networkId == universallyUniqueId) {
            PERMISSION_LOG_INFO(LABEL, "waitDuidReady: peer device version is before 11.0, transfer to allocateDuid.");
            return AllocateDistributedUid(deviceId, uid);
        }
    }

    struct timeval time;
    gettimeofday(&time, NULL);
    int32_t rate = 1000;
    long startTime = time.tv_sec * rate + time.tv_usec / rate;
    while (true) {
        int32_t dUid = DistributedUidAllocator::GetInstance().GetDuid(deviceId, uid);
        if (dUid != Constant::DISTRIBUTED_UID_NOT_EXIST && IsDuidReady(dUid)) {
            PERMISSION_LOG_INFO(LABEL, "waitDuidReady: get successfully, duid: %d", dUid);
            return dUid;
        }
        // Sleep for one second to avoid frequent refresh of the database.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        gettimeofday(&time, NULL);
        long currentTime = time.tv_sec * rate + time.tv_usec / rate;
        if (currentTime - startTime > timeout) {
            PERMISSION_LOG_WARN(LABEL, "waitDuidReady: time out!");
            return Constant::WAIT_DISTRIBUTED_UID_TIME_OUT;
        }
    }
}

bool SubjectDevicePermissionManager::IsDuidReady(const int32_t duid)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    return distributedPermissionMapping_.count(duid) > 0;
}

bool SubjectDevicePermissionManager::IsMappingReady(const int32_t duid) const
{
    return distributedPermissionMapping_.count(duid) > 0;
}

int32_t SubjectDevicePermissionManager::AddDistributedPermission(const std::string &deviceId, UidBundleBo &ruidPackages)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_ERROR(
            LABEL, "addSubjectPermission: deviceId invalid, return INVALID_DEVICE_ID: %d", Constant::INVALID_DEVICE_ID);
        return Constant::INVALID_DEVICE_ID;
    }
    if (BaseRemoteCommand::IsValid(ruidPackages)) {
        PERMISSION_LOG_ERROR(LABEL,
            "addSubjectPermission: ruidPackages is null or null permissions, return INVALID_UID_PERMISSION: %d",
            Constant::INVALID_UID_PERMISSION);
        return Constant::INVALID_UID_PERMISSION;
    }

    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    int32_t duid = GetOrCreateDistributedUid(deviceId, ruidPackages.uid);
    if (!DistributedDataValidator::IsDuidValidAndNotSpecial(duid)) {
        PERMISSION_LOG_ERROR(LABEL,
            "addSubjectPermission: getOrCreateDuid got invalid duid, return INVALID_DUID: %d",
            Constant::INVALID_DISTRIBUTED_UID);
        return Constant::INVALID_DISTRIBUTED_UID;
    }
    distributedPermissionMapping_.insert(std::pair<int32_t, UidBundleBo>(duid, ruidPackages));
    return Constant::SUCCESS;
}

int32_t SubjectDevicePermissionManager::RemoveDistributedPermission(const std::string &deviceId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL,
            "removeDistributedPermission: deviceId invalid, return INVALID_DEVICE_ID: %d",
            Constant::INVALID_DEVICE_ID);
        return Constant::INVALID_DEVICE_ID;
    }
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    DistributedUidAllocator::GetInstance().DeleteDuid(deviceId);
    return Constant::SUCCESS;
}

int32_t SubjectDevicePermissionManager::RemoveDistributedPermission(const std::string &deviceId, const int32_t uid)
{
    // Parameter check.
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        return Constant::INVALID_DEVICE_ID;
    }
    if (!DistributedDataValidator::IsUidValid(uid)) {
        return Constant::INVALID_RUID;
    }
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    // Find this duid. Remove relationship from duid to permissionItemNames.
    const int32_t duid = DistributedUidAllocator::GetInstance().GetDuid(deviceId, uid);
    if (DistributedDataValidator::IsDuidValidAndNotSpecial(duid)) {
        distributedPermissionMapping_.erase(duid);
    }
    return DistributedUidAllocator::GetInstance().DeleteDuid(deviceId, uid);
}

void SubjectDevicePermissionManager::CacheSubjectUidState(
    const std::string &deviceId, const int32_t uid, const int32_t uidState)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId)) {
        PERMISSION_LOG_ERROR(LABEL, "cacheSubjectUidState: deviceId invalid.");
        return;
    }
    PERMISSION_LOG_ERROR(LABEL,
        "%{public}s: start with deviceId: %{public}s , uidState: %{public}d",
        __func__,
        Constant::EncryptDevId(deviceId).c_str(),
        uidState);
    const int32_t duid = GetDistributedUid(deviceId, uid);
    if (!DistributedDataValidator::IsDuidValidAndNotSpecial(duid)) {
        PERMISSION_LOG_ERROR(LABEL, "cacheSubjectUidState: ensure that the duid has been allocated at first!");
        return;
    }
    if (distributedPermissionMapping_.count(duid) > 0) {
        UidBundleBo rUidPackages = distributedPermissionMapping_.at(duid);
        rUidPackages.uidState = uidState;
    }
}

int32_t SubjectDevicePermissionManager::GetOrCreateDistributedUid(const std::string &deviceId, const int32_t ruid) const
{
    int32_t existDistributedUid = GetDistributedUid(deviceId, ruid);
    if (existDistributedUid != Constant::DISTRIBUTED_UID_NOT_EXIST) {
        PERMISSION_LOG_DEBUG(LABEL,
            "%{public}s: duid: %{public}d has been allocated by subject device: %{public}s and ruid %{public}d",
            __func__,
            existDistributedUid,
            Constant::EncryptDevId(deviceId).c_str(),
            ruid);
        return existDistributedUid;
    }

    int32_t newDistributedUid = DistributedUidAllocator::GetInstance().AllocateDuid(deviceId, ruid);
    if (newDistributedUid == Constant::DISTRIBUTED_UID_NOT_EXIST) {
        PERMISSION_LOG_ERROR(LABEL,
            "%{public}s: duidTranslator allocateDuid return DUID_NOT_EXIST when allocating "
            "device: %{public}s and ruid: %{public}d",
            __func__,
            Constant::EncryptDevId(deviceId).c_str(),
            ruid);
        return Constant::DISTRIBUTED_UID_NOT_EXIST;
    }
    return newDistributedUid;
}

int32_t SubjectDevicePermissionManager::CheckDistributedPermission(
    const int32_t duid, const std::string &permissionName)
{
    // for system duid, always return Granted
    if (DistributedDataValidator::IsSpecDuid(duid)) {
        PERMISSION_LOG_INFO(LABEL, "checkDistributedPermission: duid is special, return PERMISSION_GRANTED");
        return Constant::PERMISSION_GRANTED;
    }
    if (!DistributedDataValidator::IsPermissionNameValid(permissionName) ||
        !DistributedDataValidator::IsDuidValidAndNotSpecial(duid)) {
        PERMISSION_LOG_ERROR(
            LABEL, "checkDistributedPermission: permissionName or duid invalid, return PERMISSION_DENIED");
        return Constant::PERMISSION_DENIED;
    }
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    UidBundleBo ruidPackages;
    if (distributedPermissionMapping_.count(duid) == 0) {
        PERMISSION_LOG_ERROR(LABEL, "PERMISSION_DENIED");
        return Constant::PERMISSION_DENIED;
    }
    // If the permissionName is in the duid's permission list, and the status is granted, return PERMISSION_GRANTED:0.
    ruidPackages = distributedPermissionMapping_.at(duid);
    if (ruidPackages.bundles.size() > 0) {
        auto it = std::find_if(ruidPackages.bundles.at(0).permissions.begin(),
            ruidPackages.bundles.at(0).permissions.end(),
            [&permissionName](const auto &permissionDetail) {
                return (permissionDetail.name == permissionName) &&
                       (permissionDetail.status == Constant::PERMISSION_GRANTED_STATUS);
            });
        if (it != ruidPackages.bundles.at(0).permissions.end()) {
            return Constant::PERMISSION_GRANTED;
        }
    }

    if (BaseRemoteCommand::IsGrantedSensitivePermission(ruidPackages, permissionName)) {
        return Constant::PERMISSION_GRANTED;
    }
    return Constant::PERMISSION_DENIED;
}

void SubjectDevicePermissionManager::GrantSensitivePermissionToRemoteApp(
    const std::string &permission, const std::string &deviceId, const int32_t ruid)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    int32_t duid = GetOrCreateDistributedUid(deviceId, ruid);
    if (duid < 0) {
        PERMISSION_LOG_ERROR(LABEL,
            "%{public}s(permission: %{public}s, deviceId: %{public}s, ruid: %{public}d) failed to get duid: %{public}d",
            __func__,
            permission.c_str(),
            Constant::EncryptDevId(deviceId).c_str(),
            ruid,
            duid);
        return;
    }

    if (DistributedDataValidator::IsSpecDuid(duid)) {
        PERMISSION_LOG_ERROR(LABEL,
            "%{public}s(permission: %{public}s, deviceId: %{public}s, ruid: %{public}d) is special duid: %{public}d",
            __func__,
            permission.c_str(),
            Constant::EncryptDevId(deviceId).c_str(),
            ruid,
            duid);
        return;
    }

    if (distributedPermissionMapping_.count(duid) == 0) {
        PERMISSION_LOG_ERROR(LABEL,
            "%{public}s(permission: %{public}s, deviceId: %{public}s, ruid: %{public}d) map to "
            "duid: %{public}d  package is empty",
            __func__,
            permission.c_str(),
            Constant::EncryptDevId(deviceId).c_str(),
            ruid,
            duid);
        UidBundleBo info;
        info.uid = ruid;
        distributedPermissionMapping_.insert(std::pair<int32_t, UidBundleBo>(duid, info));
    }
    UidBundleBo info = distributedPermissionMapping_.at(duid);
    if (BaseRemoteCommand::IsSensitiveResource(permission)) {
        info.remoteSensitivePermission.insert(permission);
        distributedPermissionMapping_[duid] = info;
    }
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s(permission: %{public}s, deviceId: %{public}s, ruid: %{public}d) map to "
        "duid: %{public}d  operation success",
        __func__,
        permission.c_str(),
        Constant::EncryptDevId(deviceId).c_str(),
        ruid,
        duid);
}

std::set<std::string> SubjectDevicePermissionManager::GetGrantSensitivePermissionToRemoteApp(
    const std::string &deviceId, const int32_t ruid)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::set<std::string> set;
    int32_t duid = GetDistributedUid(deviceId, ruid);
    if (duid < 0) {
        PERMISSION_LOG_ERROR(LABEL,
            "%{public}s(deviceId: %{public}s, ruid: %{public}d) failed to get duid: %{public}d ",
            __func__,
            Constant::EncryptDevId(deviceId).c_str(),
            ruid,
            duid);
        return set;
    }
    if (DistributedDataValidator::IsSpecDuid(duid)) {
        PERMISSION_LOG_ERROR(LABEL,
            "%{public}s(deviceId: %{public}s, ruid: %{public}d) is special duid: %{public}d",
            __func__,
            Constant::EncryptDevId(deviceId).c_str(),
            ruid,
            duid);
        return set;
    }
    if (distributedPermissionMapping_.count(duid) == 0) {
        PERMISSION_LOG_ERROR(LABEL,
            "%{public}s(deviceId: %{public}s, ruid: %{public}d) map to duid: %{public}d package is empty",
            __func__,
            Constant::EncryptDevId(deviceId).c_str(),
            ruid,
            duid);
        return set;
    }
    UidBundleBo info = distributedPermissionMapping_.at(duid);
    return info.remoteSensitivePermission;
}

void SubjectDevicePermissionManager::Clear() const
{
    PERMISSION_LOG_INFO(LABEL, "clear: remove caches related to all subject devices.");
    DistributedUidAllocator::GetInstance().Clear();
}

void SubjectDevicePermissionManager::ReGrantAllDuidPermissions()
{
    PERMISSION_LOG_DEBUG(LABEL, "begin");
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::map<int32_t, UidBundleBo>::iterator it = distributedPermissionMapping_.begin();
    while (it != distributedPermissionMapping_.end()) {
        PERMISSION_LOG_DEBUG(LABEL, "item, duid: %{public}d", it->first);
        ReGrantDuidPermissionsLocked(it->second);
        it++;
    }
    PERMISSION_LOG_DEBUG(LABEL, "end");
}

void SubjectDevicePermissionManager::ReGrantDuidPermissionsLocked(UidBundleBo &uidBundlePermInfo)
{
    if (PermissionBmsManager::GetInstance().IsSystemSignatureUid(uidBundlePermInfo.uid)) {
        return;
    }

    PermissionBmsManager::GetInstance().ReGrantDuidPermissions(uidBundlePermInfo);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS