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

#include "vi_controller.h"
#include <array>
#include <iostream>
#include "camera_metadata_info.h"
#include "create_sensor_factory.h"

namespace OHOS::Camera {
ViController::ViController() {}

ViController::ViController(std::string hardwareName) : IController(hardwareName) {}

ViController::~ViController() {}

RetCode ViController::Init()
{
    viObject_ = IViObject::CreateViObject();
    if (viObject_ == nullptr) {
        CAMERA_LOGE("%{public}s Create ViObject fail", __FUNCTION__);
        return RC_ERROR;
    }
    return RC_OK;
}

std::shared_ptr<ISensor> ViController::GetSensor(std::string sensorName)
{
    CAMERA_LOGI("%{public}s GetSensor %{public}s", __FUNCTION__, sensorName.c_str());
    return GetSensorFactory.GetSensorByName(sensorName);
}

RetCode ViController::PowerUp(CameraId cameraId)
{
    RetCode rc = RC_OK;
    if (GetPowerOnState() == false) {
        SetPowerOnState(true);
        CAMERA_LOGI("%{public}s Vi Powerup", __FUNCTION__);
        return rc;
    }
    return rc;
}

RetCode ViController::PowerDown(CameraId cameraId)
{
    RetCode rc = RC_OK;
    if (GetPowerOnState() == true) {
        SetPowerOnState(false);
        CAMERA_LOGI("%{public}s Vi PowerDown", __FUNCTION__);
        return rc;
    }
    return rc;
}

RetCode ViController::ConfigVi()
{
    std::vector<DeviceFormat> format;
    viObject_->ConfigVi(format);
    CAMERA_LOGI("%{public}s Vi ConfigVenc", __FUNCTION__);
    return RC_OK;
}

RetCode ViController::StartVi()
{
    std::lock_guard<std::mutex> l(startVilock_);
    RetCode rc = RC_OK;
    if (startViState_ == false) {
        viObject_->StartVi();
        startViState_ = true;
    }
    CAMERA_LOGI("%{public}s Vi StartVi", __FUNCTION__);
    return rc;
}

RetCode ViController::StopVi()
{
    CAMERA_LOGI("%{public}s Vi StopVi", __FUNCTION__);
    std::lock_guard<std::mutex> l(startVilock_);
    RetCode rc = RC_OK;
    if (startViState_ == true) {
        std::lock_guard<std::mutex> l(metaDataSetlock_);
        std::vector<int32_t>().swap(abilityMetaData_);
        viObject_->StopVi();
        startViState_ = false;
    }
    return rc;
}

void ViController::SetAbilityMetaDataTag(std::vector<int32_t> abilityMetaDataTag)
{
    std::lock_guard<std::mutex> lo(metaDataSetlock_);
    std::vector<int32_t>().swap(abilityMetaData_);
    for (auto iter = abilityMetaDataTag.cbegin(); iter != abilityMetaDataTag.cend(); iter++) {
        switch (*iter) {
            case OHOS_SENSOR_EXPOSURE_TIME:{
                abilityMetaData_.push_back(*iter);
                break;
            }
            case OHOS_SENSOR_COLOR_CORRECTION_GAINS:{
                abilityMetaData_.push_back((*iter));
                break;
            }
            default:
            break;
        }
    }
}

RetCode ViController::Configure(std::shared_ptr<CameraStandard::CameraMetadata> meta)
{
    return SendIspMetaData(meta);
}

RetCode ViController::SendIspMetaData(std::shared_ptr<CameraStandard::CameraMetadata> meta)
{
    common_metadata_header_t* data = meta->get();
    if (data == nullptr) {
        CAMERA_LOGE("%{public}s data is nullptr", __FUNCTION__);
        return RC_ERROR;
    }
    RetCode rc = SendAEMetaData(*data);
    if (rc == RC_ERROR) {
        CAMERA_LOGE("%{public}s SendAEMetaData fail", __FUNCTION__);
        return rc;
    }
    rc = SendAWBMetaData(*data);
    if (rc == RC_ERROR) {
        CAMERA_LOGE("%{public}s SendAWBMetaData fail", __FUNCTION__);
    }
    return rc;
}

RetCode ViController::SendAEMetaData(common_metadata_header_t& data)
{
    int32_t expo = 0;
    RetCode rc = RC_OK;
    camera_metadata_item_t entry;
    int ret = find_camera_metadata_item(&data, OHOS_CONTROL_AE_EXPOSURE_COMPENSATION, &entry);
    if (ret == 0) {
        expo = *(entry.data.i32);
        rc = viObject_->UpdateSetting(OHOS_CONTROL_AE_EXPOSURE_COMPENSATION, (char*)&expo);
        CAMERA_LOGD("%{public}s Set CMD_AE_EXPO [%{public}d]", __FUNCTION__, expo);
        if (rc == RC_ERROR) {
            CAMERA_LOGE("%{public}s Send CMD_AE_EXPO fail", __FUNCTION__);
            return rc;
        }
    }
    return rc;
}

RetCode ViController::SendAWBMetaData(common_metadata_header_t& data)
{
    uint8_t awbMode = 0;
    RetCode rc = RC_OK;
    camera_metadata_item_t entry;
    int ret = find_camera_metadata_item(&data, OHOS_CONTROL_AWB_MODE, &entry);
    if (ret == 0) {
        awbMode = *(entry.data.u8);
        rc = viObject_->UpdateSetting(OHOS_CONTROL_AWB_MODE, (char*)&awbMode);
        CAMERA_LOGD("%{public}s Set CMD_AWB_MODE [%{public}d]", __FUNCTION__, awbMode);
        if (rc == RC_ERROR) {
            CAMERA_LOGE("%{public}s Send CMD_AWB_MODE fail", __FUNCTION__);
            return rc;
        }
    }
    return rc;
}

RetCode ViController::GetAbilityMetaData(std::shared_ptr<CameraStandard::CameraMetadata> meta)
{
    return GetIspMetaData(meta);
}

RetCode ViController::GetIspMetaData(std::shared_ptr<CameraStandard::CameraMetadata> meta)
{
    RetCode rc = GetAEMetaData(meta);
    if (rc == RC_ERROR) {
        CAMERA_LOGE("%{public}s GetAEMetaData fail", __FUNCTION__);
        return rc;
    }
    rc = GetAWBMetaData(meta);
    if (rc == RC_ERROR) {
        CAMERA_LOGE("%{public}s GetAWBMetaData fail", __FUNCTION__);
    }
    return rc;
}

RetCode ViController::GetAEMetaData(std::shared_ptr<CameraStandard::CameraMetadata> meta)
{
    static int64_t oldExpoTime = 0;
    int64_t expoTime = 0;
    RetCode rc = RC_ERROR;
    std::lock_guard<std::mutex> l(metaDataSetlock_);
    for (auto iter = abilityMetaData_.cbegin(); iter != abilityMetaData_.cend(); iter++) {
        switch (*iter) {
            case OHOS_SENSOR_EXPOSURE_TIME: {
                rc = viObject_->QuerySetting(OHOS_SENSOR_EXPOSURE_TIME, (char*)&expoTime);
                if (rc == RC_ERROR) {
                    CAMERA_LOGE("%{public}s CMD_AE_EXPOTIME QuerySetting fail", __FUNCTION__);
                    return rc;
                }
                if (oldExpoTime != expoTime) {
                    std::lock_guard<std::mutex> l(metaDataFlaglock_);
                    metaDataFlag_ = true;
                    oldExpoTime = expoTime;
                }
                meta->addEntry(OHOS_SENSOR_EXPOSURE_TIME, &expoTime, 1);
                CAMERA_LOGD("%{public}s Get CMD_AE_EXPOTIME [%{public}d]", __FUNCTION__, expoTime);
                break;
            }
            default:
            break;
        }
    }
    return rc;
}

RetCode ViController::GetAWBMetaData(std::shared_ptr<CameraStandard::CameraMetadata> meta)
{
    static float oldColorGains[4] = {0};
    float colorGains[4] = {0};
    RetCode rc = RC_ERROR;
    std::lock_guard<std::mutex> l(metaDataSetlock_);
    for (auto iter = abilityMetaData_.cbegin(); iter != abilityMetaData_.cend(); iter++) {
        switch (*iter) {
            case OHOS_SENSOR_COLOR_CORRECTION_GAINS: {
                rc = viObject_->QuerySetting(OHOS_SENSOR_COLOR_CORRECTION_GAINS, (char*)colorGains);
                if (rc == RC_ERROR) {
                    CAMERA_LOGE("%{public}s CMD_AWB_COLORGAINS QuerySetting fail", __FUNCTION__);
                    return rc;
                }
                int gainsSize = 4;
                if (!CheckNumequal(oldColorGains, colorGains, gainsSize)) {
                    std::lock_guard<std::mutex> l(metaDataFlaglock_);
                    metaDataFlag_ = true;
                    memcpy_s(oldColorGains, gainsSize*sizeof(float), colorGains, gainsSize*sizeof(float));
                }
                static constexpr int DATA_COUNT = 4;
                meta->addEntry(OHOS_SENSOR_COLOR_CORRECTION_GAINS, &colorGains, DATA_COUNT);
                CAMERA_LOGD("%{public}s Get CMD_AWB_COLORGAINS [%{public}f,%{public}f,%{public}f,%{public}f]",
                    __FUNCTION__,
                    colorGains[0], // 0:数组范围
                    colorGains[1], // 1:数组范围
                    colorGains[2], // 2:数组范围
                    colorGains[3]); // 3:数组范围
                break;
            }
            default:
            break;
        }
    }
    return rc;
}

bool ViController::GetMetaDataFlag()
{
    std::lock_guard<std::mutex> l(metaDataFlaglock_);
    return metaDataFlag_;
}

void ViController::SetMetaDataFlag(bool metaDataFlag)
{
    std::lock_guard<std::mutex> l(metaDataFlaglock_);
    metaDataFlag_ = metaDataFlag;
}

RetCode ViController::SetFlashlight(FlashMode flashMode, bool enable)
{
    return viObject_->SetFlashlight(flashMode, enable);
}
} // namespace OHOS::Camera
