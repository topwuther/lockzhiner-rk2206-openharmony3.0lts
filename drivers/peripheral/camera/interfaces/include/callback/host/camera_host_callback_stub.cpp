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

#include "camera_host_callback_stub.h"
#include <hdf_log.h>
#include <hdf_base.h>
#include <hdf_sbuf_ipc.h>
#include "camera_host_callback.h"

namespace OHOS::Camera {
int32_t CameraHostCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    HDF_LOGE("%s: CameraHostCallbackStub::OnRemoteRequest entry!", __func__);
    switch (code) {
        case CMD_CAMERA_HOST_CALLBACK_ON_STATUS: {
            std::string cameraId = data.ReadString();
            CameraStatus status = static_cast<CameraStatus>(data.ReadInt32());
            OnCameraStatus(cameraId, status);
            break;
        }
        case CMD_CAMERA_HOST_CALLBACK_ON_FLASHLIGHT_STATUS: {
            std::string cameraId = data.ReadString();
            FlashlightStatus status =  static_cast<FlashlightStatus>(data.ReadInt32());
            OnFlashlightStatus(cameraId, status);
            break;
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return 0;
}
}