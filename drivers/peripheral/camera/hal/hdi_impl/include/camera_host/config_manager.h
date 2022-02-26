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

#ifndef CAMERA_HOST_CONFIG_MANAGER_H
#define CAMERA_HOST_CONFIG_MANAGER_H

#include <vector>
#include <map>
#include <list>
#include "utils.h"

namespace OHOS::Camera {
class ConfigManager {
public:
    CamRetCode ReadConfigFile();
    CamRetCode GetCameraIds(std::vector<std::string> &cameraIds);
    CamRetCode GetPhysicCameraIds(const std::string &lCameraId, std::vector<std::string> &pCameraIds);
    CamRetCode GetCameraAbility(const std::string &cameraId, std::shared_ptr<CameraAbility> &ability);

public:
    ConfigManager();
    virtual ~ConfigManager();
    ConfigManager(const ConfigManager &other) = delete;
    ConfigManager(ConfigManager &&other) = delete;
    ConfigManager& operator=(const ConfigManager &other) = delete;
    ConfigManager& operator=(ConfigManager &&other) = delete;

public:
    // key: config cameraId; value: DeviceManager cameraId enum
    static std::map<std::string, CameraId> enumCameraIdMap_;

private:
    // key: cameraId, value: CameraAbility
    using CameraAbilityMap = std::map<std::string, std::shared_ptr<CameraAbility>>;
    CameraAbilityMap cameraAbilityMap_;
    // key: logicCameraId, value: physicsCameraIds
    using CameraIdMap = std::map<std::string, std::vector<std::string>>;
    CameraIdMap cameraIdMap_;
};
} // end namespace OHOS::Camera
#endif // CAMERA_HOST_CONFIG_MANAGER_H
