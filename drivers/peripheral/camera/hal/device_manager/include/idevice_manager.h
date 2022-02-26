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

#ifndef HOS_CAMERA_IDEVICED_MANAGER_H
#define HOS_CAMERA_IDEVICED_MANAGER_H

#include "icontroller.h"
#include "imanager.h"
#include "device_manager_adapter.h"
#include "isensor.h"
#include <vector>

namespace OHOS::Camera {
class IDeviceManager {
public:
    IDeviceManager();
    virtual ~IDeviceManager();
    static std::shared_ptr<IDeviceManager> GetInstance();
    virtual std::shared_ptr<IManager> GetManager(ManagerId managerId) = 0;
    virtual RetCode CreateManager() = 0;
    virtual RetCode DestroyManager() = 0;
    virtual std::shared_ptr<IController> GetController(ManagerId managerId, ControllerId controllerId)
    {
        return nullptr;
    };
    virtual std::shared_ptr<IController>
        GetController(CameraId cameraId, ManagerId managerId, ControllerId controllerId)
    {
        return nullptr;
    };
    virtual RetCode PowerUp(CameraId cameraId) = 0;
    virtual RetCode PowerDown(CameraId cameraId) = 0;
    virtual RetCode Init() = 0;
    virtual std::shared_ptr<ISensor> GetSensor(CameraId cameraId) = 0;
    virtual std::vector<CameraId> GetCameraId() = 0;
    virtual RetCode Connect(std::string controller,
        std::string portNum, std::string connectController, std::string connectPortNum)
    {
        return RC_OK;
    };
    virtual RetCode UnConnect(std::string controller,
        std::string portNum, std::string connectController, std::string connectPortNum)
    {
        return RC_OK;
    };
    virtual void BufferCallback(std::shared_ptr<FrameSpec> buffer) {};
    virtual void SetAbilityMetaDataTag(std::vector<int32_t> abilityMetaDataTag) {};
    virtual RetCode SendFrameBuffer(std::shared_ptr<FrameSpec> buffer, CameraId cameraId = CAMERA_MAX)
    {
        return RC_OK;
    };
    virtual void SetNodeCallBack(const NodeBufferCb cb, CameraId cameraId = CAMERA_MAX) {};
    virtual void SetMetaDataCallBack(const MetaDataCb cb, CameraId cameraId = CAMERA_MAX) {};
    virtual void SetDevStatusCallBack(const DeviceStatusCb cb) {};
    virtual RetCode SetFlashlight(FlashMode flashMode, bool enable, CameraId cameraId = CAMERA_MAX) = 0;
    virtual void Configure(std::shared_ptr<CameraStandard::CameraMetadata> meta) = 0;
    virtual void SetCallBack(UvcMetaDataCb cb) {};
    virtual RetCode PreConfig(const ModeMeta& meta, const std::vector<DeviceStreamSetting>& settings)
    {
        return RC_OK;
    }
    virtual RetCode Flush(int32_t streamId)
    {
        return RC_OK;
    }
    virtual RetCode StartRecvFrame(int32_t streamId)
    {
        return RC_OK;
    }
    virtual RetCode StopRecvFrame(int32_t streamId)
    {
        return RC_OK;
    }
};
} // namespace OHOS::Camera
#endif
