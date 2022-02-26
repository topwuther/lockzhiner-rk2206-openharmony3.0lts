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

#include "object_device_permission_repository.h"

// #include "req_object_device.h"
// #include "req_object_device object_uid.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "ObjectDevicePermissionRepository"};
}
namespace {
const std::string JSON_KEY_DEVICE_JSON = "devicesJson_";
}  // namespace
ObjectDevicePermissionRepository::ObjectDevicePermissionRepository()
{}

ObjectDevicePermissionRepository::~ObjectDevicePermissionRepository()
{}

ObjectDevicePermissionRepository &ObjectDevicePermissionRepository::GetInstance()
{
    static ObjectDevicePermissionRepository instance;
    return instance;
}

int32_t ObjectDevicePermissionRepository::VerifyPermissionFromRemote(
    const std::string &permission, const std::string &deviceId, const int32_t pid, const int32_t uid)
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    auto iter = objectDevices_.find(deviceId);
    if (iter == objectDevices_.end()) {
        PERMISSION_LOG_ERROR(
            LABEL, "VerifyPermissionFromRemote:the deviceUids is null and it has not been initialized");
        return Constant::PERMISSION_DENIED;
    }

    if (objectDevices_.at(deviceId)->IsGrantPermission(uid, permission)) {
        return Constant::PERMISSION_GRANTED;
    }
    return Constant::PERMISSION_DENIED;
}
std::shared_ptr<ObjectDevice> ObjectDevicePermissionRepository::GetOrCreateObjectDevice(std::string &deviceId)
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    auto iter = objectDevices_.find(deviceId);
    if (iter != objectDevices_.end()) {
        std::shared_ptr<ObjectDevice> device = objectDevices_.at(deviceId);
        if (device == nullptr) {
            device = std::make_shared<ObjectDevice>(deviceId);
            objectDevices_.insert(std::pair<std::string, std::shared_ptr<ObjectDevice>>(deviceId, device));
        }
        return device;
    }
    std::shared_ptr<ObjectDevice> device = std::make_shared<ObjectDevice>(deviceId);
    objectDevices_.insert(std::pair<std::string, std::shared_ptr<ObjectDevice>>(deviceId, device));
    return device;
}
void ObjectDevicePermissionRepository::PutDeviceIdUidPair(const std::string deviceId, const int32_t uid)
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    auto iter = objectDevices_.find(deviceId);
    if (iter != objectDevices_.end()) {
        std::shared_ptr<ObjectDevice> device = objectDevices_.at(deviceId);
        if (device == nullptr) {
            device = std::make_shared<ObjectDevice>(deviceId);
            objectDevices_.insert(std::pair<std::string, std::shared_ptr<ObjectDevice>>(deviceId, device));
        }
        device->AddObjectUid(uid);
        return;
    }
    std::shared_ptr<ObjectDevice> device = std::make_shared<ObjectDevice>(deviceId);
    objectDevices_.insert(std::pair<std::string, std::shared_ptr<ObjectDevice>>(deviceId, device));
    device->AddObjectUid(uid);
}
std::vector<ObjectDeviceJson> ObjectDevicePermissionRepository::GetDevicesJsonVector()
{
    std::vector<ObjectDeviceJson> objectDeviceJson;
    std::map<std::string, std::shared_ptr<ObjectDevice>>::iterator iter;
    for (iter = objectDevices_.begin(); iter != objectDevices_.end(); iter++) {
        std::string deviceId = iter->first;
        if (iter->second == nullptr) {
            continue;
        }
        std::map<int32_t, std::shared_ptr<ObjectUid>> objectUids = iter->second->GetUidPermissions();
        std::map<int32_t, std::shared_ptr<ObjectUid>>::iterator iterU;
        for (iterU = objectUids.begin(); iterU != objectUids.end(); iterU++) {
            std::string uid = std::to_string(iterU->second->GetUid());
            std::set<std::string> grabtedPermissions = iterU->second->GetGrabtedPermission();
            if (grabtedPermissions.size() == 0) {
                std::string noGrabtedPermission("");
                ObjectDeviceJson odj = {deviceId, uid, noGrabtedPermission};
                objectDeviceJson.push_back(odj);
            }
            std::set<std::string>::iterator iterSet;
            for (iterSet = grabtedPermissions.begin(); iterSet != grabtedPermissions.end(); iterSet++) {
                std::string grabtedPermission = *iterSet;
                ObjectDeviceJson odj = {deviceId, uid, grabtedPermission};
                objectDeviceJson.push_back(odj);
            }
        }
    }
    return objectDeviceJson;
}
void ObjectDevicePermissionRepository::SaveToFile()
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    std::vector<ObjectDeviceJson> objectDeviceVector = GetDevicesJsonVector();
    DevicesJsonVector devicesJsonVector = {objectDeviceVector};
    nlohmann::json j = devicesJsonVector;
    std::string devicesJsonVectorStr = j.dump();
    std::ofstream out(repository_file_path_);
    out << devicesJsonVectorStr << std::endl;
    out.close();
}
bool ObjectDevicePermissionRepository::RecoverFromFile()
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    std::ifstream jsonStream(repository_file_path_);
    if (!jsonStream.good()) {
        return false;
    }
    nlohmann::json jsonResult;
    jsonStream >> jsonResult;
    DevicesJsonVector devicesJsonVector = jsonResult.get<DevicesJsonVector>();
    std::vector<ObjectDeviceJson> devicesJson = devicesJsonVector.devicesJson_;
    std::string lastTimeDeviceId("");
    std::string lastTimeUid("");
    std::set<std::string> lastPermissionSet;
    std::map<int32_t, std::shared_ptr<ObjectUid>> lastUniPermissions;
    for (std::vector<ObjectDeviceJson>::iterator it = devicesJson.begin(); it != devicesJson.end(); it++) {
        std::string deviceId = it->deviceId_;
        std::string uid = it->uid_;
        std::string grabtedPermission = it->grantedPermission_;
        if ((lastTimeDeviceId != deviceId || lastTimeUid != uid) && !lastTimeUid.empty()) {
            std::shared_ptr<ObjectUid> uidResult =
                std::make_shared<ObjectUid>(std::atoi(lastTimeUid.c_str()), lastPermissionSet);
            lastUniPermissions.insert(
                std::pair<int32_t, std::shared_ptr<ObjectUid>>(std::atoi(lastTimeUid.c_str()), uidResult));
            lastPermissionSet.clear();
        }
        if (lastTimeDeviceId != deviceId) {
            if (!lastTimeDeviceId.empty()) {
                std::shared_ptr<ObjectDevice> objectDeviceResult =
                    std::make_shared<ObjectDevice>(lastTimeDeviceId, lastUniPermissions);
                objectDevices_.insert(
                    std::pair<std::string, std::shared_ptr<ObjectDevice>>(lastTimeDeviceId, objectDeviceResult));
                lastUniPermissions.clear();
            }
        }
        if (!grabtedPermission.empty()) {
            lastPermissionSet.insert(grabtedPermission);
        }
        lastTimeDeviceId = deviceId;
        lastTimeUid = uid;
    }
    if (devicesJson.size() > 0) {
        std::shared_ptr<ObjectUid> uidResult =
            std::make_shared<ObjectUid>(std::atoi(lastTimeUid.c_str()), lastPermissionSet);
        lastUniPermissions.insert(
            std::pair<int32_t, std::shared_ptr<ObjectUid>>(std::atoi(lastTimeUid.c_str()), uidResult));
        std::shared_ptr<ObjectDevice> objectDeviceResult =
            std::make_shared<ObjectDevice>(lastTimeDeviceId, lastUniPermissions);
        objectDevices_.insert(
            std::pair<std::string, std::shared_ptr<ObjectDevice>>(lastTimeDeviceId, objectDeviceResult));
    }
    return true;
}

std::set<std::string> ObjectDevicePermissionRepository::ListDeviceId(const int32_t uid)
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    std::set<std::string> set;
    std::map<std::string, std::shared_ptr<ObjectDevice>>::iterator iter;
    for (iter = objectDevices_.begin(); iter != objectDevices_.end(); iter++) {
        if (iter->second->ContainUid(uid)) {
            set.insert(iter->second->GetDeviceId());
        }
    }
    return set;
}
void ObjectDevicePermissionRepository::RemoveUid(const int32_t uid)
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    std::map<std::string, std::shared_ptr<ObjectDevice>>::iterator iter;
    for (iter = objectDevices_.begin(); iter != objectDevices_.end(); iter++) {
        iter->second->RemoveObjectUid(uid);
    }
}
void ObjectDevicePermissionRepository::RemoveDeviceId(const std::string &deviceId)
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    objectDevices_.erase(deviceId);
}
std::set<int32_t> ObjectDevicePermissionRepository::ListAllUid()
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    std::set<int32_t> s;
    std::map<std::string, std::shared_ptr<ObjectDevice>>::iterator iter;
    for (iter = objectDevices_.begin(); iter != objectDevices_.end(); iter++) {
        std::map<int32_t, std::shared_ptr<ObjectUid>> m = iter->second->GetUidPermissions();
        std::map<int32_t, std::shared_ptr<ObjectUid>>::iterator it;
        for (it = m.begin(); it != m.end(); it++) {
            s.insert(it->second->GetUid());
        }
    }
    return s;
}
void ObjectDevicePermissionRepository::Clear()
{
    std::lock_guard<std::recursive_mutex> guard(object_devices_locl_);
    objectDevices_.clear();
}
void to_json(nlohmann::json &jsonObject, const DevicesJsonVector &devicesJsonVector)
{
    std::vector<std::string> deviceStrings;
    for (const ObjectDeviceJson &objectDeviceJson : devicesJsonVector.devicesJson_) {
        std::string deviceStr;
        objectDeviceJson.ToJsonString(deviceStr);
        deviceStrings.emplace_back(deviceStr);
    }
    jsonObject[JSON_KEY_DEVICE_JSON] = deviceStrings;
}
void from_json(const nlohmann::json &jsonObject, DevicesJsonVector &devicesJsonVector)
{
    if (jsonObject.find(JSON_KEY_DEVICE_JSON) != jsonObject.end()) {
        std::vector<std::string> deviceStrings = jsonObject.at(JSON_KEY_DEVICE_JSON).get<std::vector<std::string>>();
        for (std::string &deviceStr : deviceStrings) {
            ObjectDeviceJson objectDeviceJson;
            objectDeviceJson.FromJsonString(deviceStr);
            devicesJsonVector.devicesJson_.emplace_back(objectDeviceJson);
        }
    }
}
ObjectDevice::ObjectDevice(const std::string &deviceId, std::map<int32_t, std::shared_ptr<ObjectUid>> uniPermissions)
    : deviceId_(deviceId)
{
    for (auto it = uniPermissions.begin(); it != uniPermissions.end(); it++) {
        uniPermissions_.insert(*it);
    }
}
std::string ObjectDevice::GetDeviceId()
{
    return deviceId_;
}
bool ObjectDevice::ContainUid(int32_t uid)
{
    return uniPermissions_.find(uid) != uniPermissions_.end();
}
std::map<int32_t, std::shared_ptr<ObjectUid>> ObjectDevice::GetUidPermissions()
{
    return uniPermissions_;
}
void ObjectDevice::AddObjectUid(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> guard(object_device_locl_);
    uniPermissions_.insert(std::pair<int32_t, std::shared_ptr<ObjectUid>>(uid, std::make_shared<ObjectUid>(uid)));
}
void ObjectDevice::RemoveObjectUid(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> guard(object_device_locl_);
    uniPermissions_.erase(uid);
}
void ObjectDevice::ResetGrantSensitivePermission(int32_t uid, std::set<std::string> permissions)
{
    std::lock_guard<std::recursive_mutex> guard(object_device_locl_);
    if (ContainUid(uid)) {
        auto iter = uniPermissions_.find(uid);
        if (iter != uniPermissions_.end()) {
            uniPermissions_.at(uid)->ResetGrantSensitivePermission(permissions);
        }
    }
}

bool ObjectDevice::IsGrantPermission(int32_t uid, std::string permission)
{
    if (ContainUid(uid)) {
        auto iter = uniPermissions_.find(uid);
        if (iter != uniPermissions_.end()) {
            return uniPermissions_.at(uid)->IsGrantPermission(permission);
        }
    }
    return false;
}
int32_t ObjectUid::GetUid()
{
    return uid_;
}
std::set<std::string> ObjectUid::GetGrabtedPermission()
{
    return grantedPermission_;
}

void ObjectUid::ResetGrantSensitivePermission(std::set<std::string> permissions)
{
    std::lock_guard<std::recursive_mutex> guard(object_uid_locl_);
    if (permissions.size() == 0) {
        return;
    }
    grantedPermission_.clear();
    std::set<std::string>::iterator iter;
    for (iter = permissions.begin(); iter != permissions.end(); iter++) {
        if (BaseRemoteCommand::IsSensitiveResource(*iter)) {
            grantedPermission_.insert(*iter);
        }
    }
}
bool ObjectUid::IsGrantPermission(std::string permission)
{
    return grantedPermission_.count(permission) > 0;
}
ObjectUid::ObjectUid(const int32_t uid, const std::set<std::string> &grabtedPermission) : uid_(uid)
{
    std::set<std::string>::iterator it;
    for (auto it = grabtedPermission.begin(); it != grabtedPermission.end(); it++) {
        grantedPermission_.insert(*it);
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
