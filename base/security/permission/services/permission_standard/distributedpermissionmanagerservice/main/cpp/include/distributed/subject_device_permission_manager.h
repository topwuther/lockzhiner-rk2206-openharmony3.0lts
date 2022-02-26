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

#ifndef SUBJECT_DEVICE_PERMISSION_MANAGER_H
#define SUBJECT_DEVICE_PERMISSION_MANAGER_H

#include <chrono>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

#include "constant.h"
#include "base_remote_command.h"
#include "device_info_manager.h"
#include "distributed_data_validator.h"
#include "distributed_uid_allocator.h"
#include "permission_log.h"
#include "ipc_skeleton.h"
#include "remote_command_manager.h"
#include "permission_bms_manager.h"

namespace OHOS {
namespace Security {
namespace Permission {
class SubjectDevicePermissionManager {
public:
    static SubjectDevicePermissionManager &GetInstance();

    /**
     * Query function. Query an exist duid from DuidTranslator by deviceId and uid.
     *
     * @param deviceId The query deviceId.
     * @param uid The query uid.
     * @return A duid which is a positive number; INVALID_DEVICE_ID: -3; INVALID_RUID: -4.
     */
    int32_t GetDistributedUid(const std::string &deviceId, const int32_t uid) const;

    /**
     * Convert nodeId to deviceId(UDID) if possible.
     *
     * @param deviceId The remote device id.
     * @param uid The remote uid.
     * @return A positive duid; FAILURE: -1; INVALID_DEVICE_ID: -3; INVALID_RUID: -4; INVALID_DUID: -8;
     *         REMOTE_EXECUTE_FAILED: -9; CANNOT_GET_PACKAGE_FOR_UID: -12.
     */
    int32_t AllocateDistributedUid(const std::string &deviceId, const int32_t uid) const;

    /**
     * Operation function. Using DuidAllocator to assign a temp duid for deviceId and ruid pair. Ask
     * RemoteExecutorManager to execute getUidPermissionCommand in 100 milliseconds. If remote execute succeed, return
     * this duid.
     *
     * @param deviceId The peer device's id(UDID), also can be considered as subject device's id.
     * @param rUid The app uid of the caller application.
     * @param timeout Timeout interval for waiting, which unit is millisecond.
     * @return A positive number is a duid has been allocated, -1 failure, -3 invalid deviceId, -4 invalid rUid,
     *         -5 wait time out.
     */
    int32_t WaitDuidReady(const std::string &deviceId, const int32_t uid, const int32_t timeout);

    /**
     * Operation function. Using duidTranslator to generate a duid. Then add relationship from deviceId to uid and add
     * relationship from duid to permissionItemNames.
     * <p>
     * Check input parameter first. If:
     * <ol>
     * <li>DeviceId is invalid, will return <b>INVALID_DEVICE_ID</b>, and will not add.
     * <li>ObjectUid is null, or permissionItems is null, will return <b>INVALID_UID_PERMISSION</b>, and will not
     * add.
     * </ol>
     * Secondly, ask DuidTranslator to get a duid for deviceId and ruid pair. If the allocated duid is not a duid, then
     * return <b>INVALID_DUID</b>.
     * <p>
     * Finally, save the deviceId to uid relationship, and save the duid to permissionItemNames relationship. New one
     * will over the old ones.
     *
     * @param deviceId The operation deviceId.
     * @param ruidPackages The operation uid packages structure. Require non-null permissions in it.
     * @return SUCCESS: 0; INVALID_DEVICE_ID: -3; INVALID_DUID: -8; INVALID_UID_PERMISSION: -10
     */
    int32_t AddDistributedPermission(const std::string &deviceId, UidBundleBo &ruidPackages);

    /**
     * Operation function. Remove all relationship from deviceId to ruids, and from duid to permissionItemNames.
     * <p>
     * Check input parameter if deviceId is invalid, will return <b>INVALID_DEVICE_ID</b>, and will not remove anyone.
     * <p>
     * Secondly, find all ruids and get all valid duids. The invalid duid will be ignored. Remove relationship from
     * deviceId to ruids, and from duid to permissionItemNames.
     * <p>
     * Finally, notify duidTranslator to delete deviceId.
     *
     * @param deviceId The operation deviceId.
     * @return SUCCESS: 0; INVALID_DEVICE_ID: -3.
     */
    int32_t RemoveDistributedPermission(const std::string &deviceId);

    /**
     * Operation function. Remove one relationship from deviceId to ruids, and from duid to permissionItemNames.
     * <p>
     * Check input parameter first. If:
     * <ol>
     * <li>DeviceId is invalid, will return <b>INVALID_DEVICE_ID</b>, and will not remove anyone.
     * <li>Ruid is invalid, will return <b>INVALID_RUID</b>, and will not remove anyone.
     * </ol>
     * Secondly, remove this relationship from deviceId to uid.
     * <p>
     * Thirdly, get the duid. Remove the relationship from duid to permissionItemNames, if duid exist.
     * <p>
     * Finally, notify duidTranslator to delete deviceId.
     *
     * @param deviceId The remote mapped deviceId.
     * @param uid The remote mapped uid.
     * @return SUCCESS: 0; INVALID_DEVICE_ID: -3; INVALID_RUID: -4
     */
    int32_t RemoveDistributedPermission(const std::string &deviceId, const int32_t uid);

    /**
     * Cache state of uid from subject device to object device.
     *
     * @param deviceId The deivceId of subject device.
     * @param uid The caller uid from subject device.
     * @param uidState The process state of uid.
     */
    void CacheSubjectUidState(const std::string &deviceId, const int32_t uid, const int32_t uidState);

    /**
     * Query function.Check if a duid has the specific permissionName.
     * Check input parameter first. If:
     * Duid is special, return PERMISSION_GRANTED.
     * PermissionName or Duid is invalid, return PERMISSION_DENIED.
     * Check all permissionName for this duid, and if any match the input name, return PERMISSION_GRANTED,
     * else return PERMISSION_DENIED.
     *
     * @param permissionName The query permission name.
     * @param duid The query duid.
     * @return PERMISSION_GRANTED: 0; PERMISSION_DENIED: -1.
     */
    int32_t CheckDistributedPermission(const int32_t duid, const std::string &permissionName);

    /**
     * get all the sensitive permissions that grant to the UID
     *
     * @param deviceId the device id of subject device
     * @param ruid subject uid
     * @return all the sensitive permissions that grant to the UID
     */
    void GrantSensitivePermissionToRemoteApp(
        const std::string &permission, const std::string &deviceId, const int32_t ruid);

    /**
     * get all the sensitive permissions that grant to the UID
     *
     * @param deviceId the device id of subject device
     * @param ruid subject uid
     * @return all the sensitive permissions that grant to the UID
     */
    std::set<std::string> GetGrantSensitivePermissionToRemoteApp(const std::string &deviceId, const int32_t ruid);

    /**
     * Clear subject device's caches
     */
    void Clear() const;

    /**
     * ReGrant all cached dUid-permissions when object device package has been added or replaced or removed.
     */
    void ReGrantAllDuidPermissions();

private:
    bool IsDuidReady(const int32_t duid);

    bool IsMappingReady(const int32_t duid) const;

    int32_t GetOrCreateDistributedUid(const std::string &deviceId, const int32_t ruid) const;

    std::map<int32_t, UidBundleBo> distributedPermissionMapping_;

    std::recursive_mutex stackLock_;

    void ReGrantDuidPermissionsLocked(UidBundleBo &uidBundlePermInfo);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // PERMISSION_BASE_SERVICE_H