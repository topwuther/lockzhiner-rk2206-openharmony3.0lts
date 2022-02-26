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

#include "config_manager.h"

extern "C"
{
#include "camera_host_config.h"
}

extern "C"
{
const struct HdfConfigHostRoot* HdfGetHostModuleConfigRoot(void);
}

namespace OHOS::Camera {
std::map<std::string, CameraId> ConfigManager::enumCameraIdMap_ = {
    { "CAMERA_FIRST", CAMERA_FIRST },
    { "CAMERA_SECOND", CAMERA_SECOND },
    { "CAMERA_THIRD", CAMERA_THIRD },
};

ConfigManager::ConfigManager()
{
}

ConfigManager::~ConfigManager()
{
}

CamRetCode ConfigManager::ReadConfigFile()
{
    const struct HdfConfigHostRoot *hcsRoot = HdfGetHostModuleConfigRoot();
    uint16_t lcameraIdCount = hcsRoot->camera_host_config.cameraHostCfgSize;
    for (uint16_t i = 0; i < lcameraIdCount; i++) {
        const char *lcameraId = hcsRoot->camera_host_config.cameraHostCfg[i].logicCameraId;
        std::shared_ptr<CameraAbility> ability = std::make_shared<CameraAbility>(0, 0);
        cameraAbilityMap_.insert(std::make_pair(std::string(lcameraId), ability));

        std::vector<std::string> pCameraIds;
        uint16_t pcameraIdCount = hcsRoot->camera_host_config.cameraHostCfg[i].physicsCameraIdSize;
        for (uint16_t j = 0; j < pcameraIdCount; j++) {
            const char *pcameraId = hcsRoot->camera_host_config.cameraHostCfg[i].physicsCameraId[j].cameraId;
            pCameraIds.push_back(std::string(pcameraId));
            CAMERA_LOGE("phy cameraID = %{public}s.", pcameraId);
        }

        if (!pCameraIds.empty()) {
            cameraIdMap_.insert(std::make_pair(std::string(lcameraId), pCameraIds));
        }
    }

    return NO_ERROR;
}

CamRetCode ConfigManager::GetCameraIds(std::vector<std::string> &cameraIds)
{
    auto itr = cameraAbilityMap_.begin();
    for (; itr != cameraAbilityMap_.end(); itr++) {
        cameraIds.push_back(itr->first);
    }

    return NO_ERROR;
}

CamRetCode ConfigManager::GetPhysicCameraIds(const std::string &lCameraId, std::vector<std::string> &pCameraIds)
{
    auto itr = cameraIdMap_.find(lCameraId);
    if (itr != cameraIdMap_.end()) {
        pCameraIds = itr->second;
        return NO_ERROR;
    }
    return INSUFFICIENT_RESOURCES;
}

CamRetCode ConfigManager::GetCameraAbility(
    const std::string &cameraId, std::shared_ptr<CameraAbility> &ability)
{
    if (ability == nullptr) {
        CAMERA_LOGE("ability is null.");
        return INVALID_ARGUMENT;
    }

    auto itr = cameraAbilityMap_.find(cameraId);
    if (itr != cameraAbilityMap_.end()) {
        ability = itr->second;
        return NO_ERROR;
    }

    return INSUFFICIENT_RESOURCES;
}
} // end namespace OHOS::Camera
