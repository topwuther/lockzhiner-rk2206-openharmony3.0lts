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

#ifndef HOS_CAMERA_ISYS_OBJECT_H
#define HOS_CAMERA_ISYS_OBJECT_H

#include "ibuffer.h"
#include "mpi_adapter.h"
#include "camera_metadata_info.h"

namespace OHOS::Camera {
class ISysObject {
public:
    static std::shared_ptr<ISysObject> CreateSysObject();
    virtual ~ISysObject() {};
    virtual void StartSys() = 0;
    virtual void UnInitSys() = 0;
    virtual void SetCallback(BufCallback cb) = 0;
    virtual void SetDevStatusCallback(DeviceStatusCb cb) = 0;
    virtual void VpssBindVenc(int32_t vpssChn, int32_t vencChn) = 0;
    virtual void VpssUnBindVenc(int32_t vpssChn, int32_t vencChn) = 0;
    virtual void ViBindVpss(int32_t viPipe, int32_t viChn, int32_t vpssGrp, int32_t vpssChn = 0) = 0;
    virtual void VpssBindVo(int32_t vpssGrp, int32_t vpssChn, int32_t voLayer, int32_t voChn) = 0;
    virtual void ViUnBindVpss(int32_t viPipe, int32_t viChn, int32_t vpssGrp, int32_t vpssChn = 0) = 0;
    virtual void VpssUnBindVo(int32_t vpssGrp, int32_t vpssChn, int32_t voLayer, int32_t voChn) = 0;
    virtual RetCode StopSys() = 0;
    virtual RetCode InitSys() = 0;
    virtual RetCode Flush(int32_t streamId) = 0;
    virtual RetCode Prepare() = 0;
    virtual RetCode RequestBuffer(std::shared_ptr<FrameSpec> &frameSpec) = 0;
    virtual RetCode PreConfig(const std::shared_ptr<CameraStandard::CameraMetadata>& meta,
        const std::vector<DeviceStreamSetting>& settings) = 0;
    virtual RetCode StartRecvFrame(int32_t streamId) = 0;
    virtual RetCode StopRecvFrame(int32_t streamId) = 0;
};
}
#endif // OHOS::Camera
