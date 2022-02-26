/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#ifndef HOS_CAMERA_3516_DEMO_H
#define HOS_CAMERA_3516_DEMO_H

#include <vector>
#include <map>
#include <iostream>
#include <hdf_log.h>
#include <surface.h>
#include <time.h>
#include <sys/time.h>
#include <display_type.h>
#include "camera.h"
#include "camera_metadata_info.h"
#include "ibuffer.h"
#include "if_system_ability_manager.h"
#include "ioffline_stream_operator.h"
#include "iservice_registry.h"
#include "icamera_host.h"
#include "camera_host_proxy.h"
#include "camera_host_callback.h"
#include "camera_device_callback.h"
#include "icamera_device.h"
#include "stream_operator_callback.h"
#include "istream_operator_callback.h"
#include "stream_customer.h"
#include "securec.h"

namespace OHOS::Camera {
enum CaptureMode {
    CAPTURE_PREVIEW = 0,
    CAPTURE_SNAPSHOT,
    CAPTURE_VIDEO,
};

enum CameraId {
    STREAM_ID_PREVIEW = 1001,
    STREAM_ID_CAPTURE,
    STREAM_ID_VIDEO,
    CAPTURE_ID_PREVIEW = 2001,
    CAPTURE_ID_CAPTURE,
    CAPTURE_ID_VIDEO,
};

class Hos3516Demo {
public:
    Hos3516Demo();
    ~Hos3516Demo();

    RetCode InitCameraDevice();
    void ReleaseCameraDevice();
    RetCode InitSensors();

    RetCode StartPreviewStream();
    RetCode StartCaptureStream();
    RetCode StartVideoStream();
    RetCode StartDualStreams(const int streamIdSecond);

    RetCode ReleaseAllStream();

    RetCode CaptureOnDualStreams(const int streamIdSecond);
    RetCode CaptureON(const int streamId, const int captureId, CaptureMode mode);
    RetCode CaptureOff(const int captureId, const CaptureMode mode) const;

    void SetAwbMode(const int mode) const;
    void SetAeExpo();
    void FlashlightOnOff(bool onOff);

    RetCode StreamOffline(const int streamId);

    void QuitDemo();

private:
    void SetStreamInfo(std::shared_ptr<OHOS::Camera::StreamInfo>& streamInfo,
        const std::shared_ptr<StreamCustomer>& streamCustomer,
        const int streamId, const StreamIntent intent);
    void GetStreamOpt();

    RetCode CreatStream(const int streamId, std::shared_ptr<StreamCustomer>& streamCustomer,
        StreamIntent intent);
    RetCode CreatStreams(const int streamIdSecond, StreamIntent intent);

    void StoreImage(const void* bufStart, const unsigned int size) const;
    void StoreVideo(const void* bufStart, const unsigned int size) const;
    void OpenVideoFile();

    int aeStatus_ = 1;
    int videoFd_ = -1;
    unsigned int isPreviewOn_ = 0;
    unsigned int isCaptureOn_ = 0;
    unsigned int isVideoOn_ = 0;

    std::shared_ptr<StreamCustomer> streamCustomerPreview_ = nullptr;
    std::shared_ptr<StreamCustomer> streamCustomerCapture_ = nullptr;
    std::shared_ptr<StreamCustomer> streamCustomerVideo_ = nullptr;
    std::shared_ptr<CameraAbility> ability_ = nullptr;

    sptr<CameraHostCallback> hostCallback_ = nullptr;
    sptr<IStreamOperator> streamOperator_ = nullptr;
    sptr<ICameraHost> demoCameraHost_ = nullptr;
    sptr<ICameraDevice> demoCameraDevice_ = nullptr;
    std::vector<std::string> cameraIds_ = {};
};
} // namespace OHOS::Camera
#endif // HOS_CAMERA_3516_DEMO_H
