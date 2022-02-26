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

#ifndef OBJECT_DEVICE_PERMISSION_REPOSITORY
#define OBJECT_DEVICE_PERMISSION_REPOSITORY
#include <string>
#include <memory>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"
#include "constant.h"
#include "base_remote_command.h"
#include "object_device_json.h"
#include "permission_log.h"
#include "permission_log.h"
namespace OHOS {
namespace Security {
namespace Permission {
class ObjectUid {
public:
    ObjectUid(int32_t uid) : uid_(uid)
    {}
    int32_t GetUid();
    std::set<std::string> GetGrabtedPermission();
    void ResetGrantSensitivePermission(std::set<std::string> permissions);
    bool IsGrantPermission(std::string permission);
    ObjectUid(const int32_t uid, const std::set<std::string> &grabtedPermission);

private:
    int32_t uid_;
    std::set<std::string> grantedPermission_;
    std::recursive_mutex object_uid_locl_;
};  // end of ObjectUid
class ObjectDevice {
public:
    ObjectDevice(const std::string &deviceId) : deviceId_(deviceId)
    {}
    ObjectDevice(const std::string &deviceId, std::map<int32_t, std::shared_ptr<ObjectUid>> uniPermissions);
    std::string GetDeviceId();
    bool ContainUid(int32_t uid);
    std::map<int32_t, std::shared_ptr<ObjectUid>> GetUidPermissions();
    void AddObjectUid(int32_t uid);
    void RemoveObjectUid(int32_t uid);
    void ResetGrantSensitivePermission(int32_t uid, std::set<std::string> permissions);
    bool IsGrantPermission(int32_t uid, std::string permission);

private:
    const std::string deviceId_;
    std::map<int32_t, std::shared_ptr<ObjectUid>> uniPermissions_;
    std::recursive_mutex object_device_locl_;
};  // end of ObjectDevice
/**
 * Repository level, store the relationship from deviceId to uid, and provide the operation function CRUD.
 *
 * @since 3
 */
class ObjectDevicePermissionRepository {
public:
    virtual ~ObjectDevicePermissionRepository();
    static ObjectDevicePermissionRepository &GetInstance();
    /**
     * Recover data from file.
     *
     * @return Recover result, Success: true; Fail: false.
     */
    bool RecoverFromFile();
    /**
     * Persist and save data to file.
     */
    void SaveToFile();
    void PutDeviceIdUidPair(const std::string deviceId, const int32_t uid);
    std::set<int32_t> ListAllUid();

    std::set<std::string> ListDeviceId(const int32_t uid);
    /**
     * Operation function. Remove uid from all device to uid relationship. If none exist, then nothing happened.
     *
     * @param uid The operation uid.
     */
    void RemoveUid(const int32_t uid);
    /**
     * Verify if one Uid have sensistive permission from deviceId
     *
     * @param permission the permission to check
     * @param deviceId the deviceId who grant the permission to uid
     * @param pid the pid of the caller
     * @param uid the local uid
     * @return 0: grant the permission, otherwise reject
     */
    int32_t VerifyPermissionFromRemote(
        const std::string &permission, const std::string &deviceId, const int32_t pid, const int32_t uid);

    std::shared_ptr<ObjectDevice> GetOrCreateObjectDevice(std::string &deviceId);

    /**
     * Operation function. Remove all uid for under this device id. If none exist, then nothing happened.
     *
     * @param deviceId The operation device id.
     */
    void RemoveDeviceId(const std::string &deviceId);
    /**
     * Clear the map.
     */
    void Clear();

protected:
private:
    ObjectDevicePermissionRepository();

    static ObjectDevicePermissionRepository instance_;

    bool IsGrantPermission(int32_t uid, std::string permission);
    std::vector<ObjectDeviceJson> GetDevicesJsonVector();

private:
    std::map<std::string, std::shared_ptr<ObjectDevice>> objectDevices_;
    std::recursive_mutex object_devices_locl_;
    const std::string repository_file_path_ = "/data/distribute_permission_subscriber";
};  // end of ObjectDevicePermissionRepository

struct DevicesJsonVector {
    std::vector<ObjectDeviceJson> devicesJson_;
};
void to_json(nlohmann::json &jsonObject, const DevicesJsonVector &devicesJsonVector);
void from_json(const nlohmann::json &jsonObject, DevicesJsonVector &devicesJsonVector);
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // OBJECT_DEVICE_PERMISSON_MANAGER_H