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

#include "3516_demo.h"

namespace OHOS::Camera {
Hos3516Demo::Hos3516Demo() {}
Hos3516Demo::~Hos3516Demo() {}

void Hos3516Demo::SetStreamInfo(std::shared_ptr<StreamInfo>& streamInfo,
    const std::shared_ptr<StreamCustomer>& streamCustomer,
    const int streamId, const StreamIntent intent)
{
    constexpr uint32_t datasapce = 8;
    constexpr uint32_t tunneledMode = 5;

    if (intent == PREVIEW) {
        constexpr uint32_t width = 640;
        constexpr uint32_t height = 480;
        streamInfo->width_ = width;
        streamInfo->height_ = height;
    } else if (intent == STILL_CAPTURE) {
        constexpr uint32_t width = 1280;
        constexpr uint32_t height = 960;
        streamInfo->width_ = width;
        streamInfo->height_ = height;
        streamInfo->encodeType_ = ENCODE_TYPE_JPEG;
    } else {
        constexpr uint32_t width = 1280;
        constexpr uint32_t height = 960;
        streamInfo->width_ = width;
        streamInfo->height_ = height;
        streamInfo->encodeType_ = ENCODE_TYPE_H265;
    }

    streamInfo->streamId_ = streamId;
    streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
    streamInfo->datasapce_ = datasapce;
    streamInfo->intent_ = intent;
    streamInfo->tunneledMode_ = tunneledMode;
    streamInfo->bufferQueue_ = streamCustomer->CreateProducer();
    streamInfo->bufferQueue_->SetQueueSize(8); // 8:set bufferQueue size
}

void Hos3516Demo::GetStreamOpt()
{
    int rc = 0;

    if (streamOperator_ == nullptr) {
        const sptr<IStreamOperatorCallback> streamOperatorCallback = new StreamOperatorCallback();
        rc = demoCameraDevice_->GetStreamOperator(streamOperatorCallback, streamOperator_);
        if (rc != Camera::NO_ERROR) {
            CAMERA_LOGE("demo test: GetStreamOpt GetStreamOperator fail\n");
            streamOperator_ = nullptr;
        }
    }
}

RetCode Hos3516Demo::CaptureON(const int streamId, const int captureId, CaptureMode mode)
{
    CAMERA_LOGD("demo test: CaptureON enter mode == %{public}d", mode);

    std::shared_ptr<Camera::CaptureInfo> captureInfo = std::make_shared<Camera::CaptureInfo>();
    captureInfo->streamIds_ = {streamId};
    captureInfo->captureSetting_ = ability_;
    captureInfo->enableShutterCallback_ = false;

    int rc = streamOperator_->Capture(captureId, captureInfo, true);
    if (rc != Camera::NO_ERROR) {
        CAMERA_LOGE("demo test: CaptureStart Capture error\n");
        streamOperator_->ReleaseStreams(captureInfo->streamIds_);
        return RC_ERROR;
    }

    if (mode == CAPTURE_PREVIEW) {
        streamCustomerPreview_->ReceiveFrameOn(nullptr);
    } else if (mode == CAPTURE_SNAPSHOT) {
        streamCustomerCapture_->ReceiveFrameOn([this](void* addr, const uint32_t size) {
            StoreImage(addr, size);
        });
    } else if (mode == CAPTURE_VIDEO) {
        OpenVideoFile();

        streamCustomerVideo_->ReceiveFrameOn([this](void* addr, const uint32_t size) {
            StoreVideo(addr, size);
        });
    }
    CAMERA_LOGD("demo test: CaptureON exit");

    return RC_OK;
}

RetCode Hos3516Demo::CaptureOff(const int captureId, const CaptureMode mode) const
{
    int rc = 0;
    CAMERA_LOGD("demo test: CaptureOff enter mode == %{public}d", mode);

    if (streamOperator_ == nullptr) {
        CAMERA_LOGE("demo test: CaptureOff streamOperator_ is nullptr\n");
        return RC_ERROR;
    }

    if (mode == CAPTURE_PREVIEW) {
        streamCustomerPreview_->ReceiveFrameOff();
        rc = streamOperator_->CancelCapture(captureId);
    } else if (mode == CAPTURE_SNAPSHOT) {
        streamCustomerCapture_->ReceiveFrameOff();
        rc = streamOperator_->CancelCapture(captureId);
    } else if (mode == CAPTURE_VIDEO) {
        streamCustomerVideo_->ReceiveFrameOff();
        rc = streamOperator_->CancelCapture(captureId);
        close(videoFd_);
    }

    if (rc != Camera::NO_ERROR) {
        CAMERA_LOGE("demo test: CaptureOff CancelCapture error mode %{public}d rc == %{public}d\n", mode, rc);
        return RC_ERROR;
    }
    CAMERA_LOGD("demo test: CaptureOff exit");

    return RC_OK;
}

RetCode Hos3516Demo::CreatStream(const int streamId, std::shared_ptr<StreamCustomer>& streamCustomer,
    StreamIntent intent)
{
    int rc = 0;
    CAMERA_LOGD("demo test: CreatStream enter");

    GetStreamOpt();
    if (streamOperator_ == nullptr) {
        CAMERA_LOGE("demo test: CreatStream GetStreamOpt() is nullptr\n");
        return RC_ERROR;
    }

    std::shared_ptr<StreamInfo> streamInfo = std::make_shared<StreamInfo>();
    if (streamInfo == nullptr) {
        CAMERA_LOGE("demo test: std::make_shared<Camera::StreamInfo>() is nullptr\n");
        return RC_ERROR;
    }

    SetStreamInfo(streamInfo, streamCustomer, streamId, intent);
    if (streamInfo->bufferQueue_ == nullptr) {
        CAMERA_LOGE("demo test: CreatStream CreateProducer(); is nullptr\n");
        return RC_ERROR;
    }

    std::vector<std::shared_ptr<StreamInfo>> streamInfos;
    std::vector<std::shared_ptr<StreamInfo>>().swap(streamInfos);
    streamInfos.push_back(streamInfo);

    rc = streamOperator_->CreateStreams(streamInfos);
    if (rc != Camera::NO_ERROR) {
        CAMERA_LOGE("demo test: CreatStream CreateStreams error\n");
        return RC_ERROR;
    }

    rc = streamOperator_->CommitStreams(Camera::NORMAL, ability_);
    if (rc != Camera::NO_ERROR) {
        CAMERA_LOGE("demo test: CreatStream CommitStreams error\n");
        streamOperator_->ReleaseStreams({streamId});
        return RC_ERROR;
    }

    CAMERA_LOGD("demo test: CreatStream exit");

    return RC_OK;
}

RetCode Hos3516Demo::InitCameraDevice()
{
    int rc = 0;

    CAMERA_LOGD("demo test: InitCameraDevice enter");

    if (demoCameraHost_ == nullptr) {
        CAMERA_LOGE("demo test: InitCameraDevice demoCameraHost_ == nullptr");
        return RC_ERROR;
    }

    (void)demoCameraHost_->GetCameraIds(cameraIds_);
    if (cameraIds_.empty()) {
        return RC_ERROR;
    }
    const std::string cameraId = cameraIds_.front();
    demoCameraHost_->GetCameraAbility(cameraId, ability_);

    sptr<CameraDeviceCallback> callback = new CameraDeviceCallback();
    rc = demoCameraHost_->OpenCamera(cameraIds_.front(), callback, demoCameraDevice_);
    if (rc != Camera::NO_ERROR || demoCameraDevice_ == nullptr) {
        CAMERA_LOGE("demo test: InitCameraDevice OpenCamera failed");
        return RC_ERROR;
    }

    CAMERA_LOGD("demo test: InitCameraDevice exit");

    return RC_OK;
}

void Hos3516Demo::ReleaseCameraDevice()
{
    if (demoCameraDevice_ != nullptr) {
        CAMERA_LOGD("demo test: ReleaseCameraDevice close Device");
        demoCameraDevice_->Close();
        demoCameraDevice_ = nullptr;
    }
}

constexpr const char *DEMO_SERVICE_NAME = "camera_service";
RetCode Hos3516Demo::InitSensors()
{
    int rc = 0;

    CAMERA_LOGD("demo test: InitSensors enter");

    if (demoCameraHost_ != nullptr) {
        return RC_OK;
    }

    demoCameraHost_ = ICameraHost::Get(DEMO_SERVICE_NAME);
    if (demoCameraHost_ == nullptr) {
        CAMERA_LOGE("demo test: ICameraHost::Get error");
        return RC_ERROR;
    }

    hostCallback_ = new CameraHostCallback();
    rc = demoCameraHost_->SetCallback(hostCallback_);
    if (rc != Camera::NO_ERROR) {
        CAMERA_LOGE("demo test: demoCameraHost_->SetCallback(hostCallback_) error");
        return RC_ERROR;
    }

    CAMERA_LOGD("demo test: InitSensors exit");

    return RC_OK;
}

void Hos3516Demo::StoreImage(const void* bufStart, const uint32_t size) const
{
    constexpr uint32_t pathLen = 30;
    char path[pathLen] = {0};
    int imgFD = 0;
    int ret;

    struct timeval start;
    gettimeofday(&start, nullptr);
    if (sprintf_s(path, sizeof(path), "/data/3516_%ld.jpeg", start.tv_usec) < 0) {
        CAMERA_LOGE("sprintf_s error .....\n");
        return;
    }

    imgFD = open(path, O_RDWR | O_CREAT, 00766); // 00766:file operate permission
    if (imgFD == -1) {
        CAMERA_LOGE("demo test:open image file error %{public}s.....\n", strerror(errno));
        return;
    }

    CAMERA_LOGD("demo test:StoreImage %{public}s buf_start == %{public}p size == %{public}d\n", path, bufStart, size);

    ret = write(imgFD, bufStart, size);
    if (ret == -1) {
        CAMERA_LOGE("demo test:write image file error %{public}s.....\n", strerror(errno));
    }

    close(imgFD);
}

void Hos3516Demo::StoreVideo(const void* bufStart, const uint32_t size) const
{
    int ret = 0;

    ret = write(videoFd_, bufStart, size);
    if (ret == -1) {
        CAMERA_LOGE("demo test:write video file error %{public}s.....\n", strerror(errno));
    }
    CAMERA_LOGD("demo test:StoreVideo buf_start == %{public}p size == %{public}d\n", bufStart, size);
}

void Hos3516Demo::OpenVideoFile()
{
    constexpr uint32_t pathLen = 30;
    char path[pathLen] = {0};

    if (sprintf_s(path, sizeof(path), "/data/3516_video%ld.h264", time(nullptr)) < 0) {
        CAMERA_LOGE("%s: sprintf  failed", __func__);
        return;
    }
    videoFd_ = open(path, O_RDWR | O_CREAT, 00766); // 00766:file operate permission
    if (videoFd_ < 0) {
        CAMERA_LOGE("demo test: StartVideo open /data/3516.yuv %{public}s\n", strerror(errno));
    }
}

RetCode Hos3516Demo::CreatStreams(const int streamIdSecond, StreamIntent intent)
{
    int rc = 0;
    std::vector<std::shared_ptr<StreamInfo>> streamInfos;
    std::vector<std::shared_ptr<StreamInfo>>().swap(streamInfos);

    CAMERA_LOGD("demo test: CreatStreams streamIdSecond = %{public}d", streamIdSecond);
    GetStreamOpt();
    if (streamOperator_ == nullptr) {
        CAMERA_LOGE("demo test: CreatStreams GetStreamOpt() is nullptr\n");
        return RC_ERROR;
    }

    std::shared_ptr<StreamInfo> previewStreamInfo = std::make_shared<StreamInfo>();
    if (previewStreamInfo == nullptr) {
        CAMERA_LOGE("demo test: CreatStreams previewStreamInfo is nullptr\n");
        return RC_ERROR;
    }

    SetStreamInfo(previewStreamInfo, streamCustomerPreview_, STREAM_ID_PREVIEW, PREVIEW);
    if (previewStreamInfo->bufferQueue_ == nullptr) {
        CAMERA_LOGE("demo test: CreatStream CreateProducer(); is nullptr\n");
        return RC_ERROR;
    }
    streamInfos.push_back(previewStreamInfo);

    std::shared_ptr<StreamInfo> secondStreamInfo = std::make_shared<StreamInfo>();
    if (secondStreamInfo == nullptr) {
        CAMERA_LOGE("demo test: CreatStreams previewStreamInfo is nullptr\n");
        return RC_ERROR;
    }

    if (streamIdSecond == STREAM_ID_CAPTURE) {
        SetStreamInfo(secondStreamInfo, streamCustomerCapture_, STREAM_ID_CAPTURE, intent);
    } else {
        SetStreamInfo(secondStreamInfo, streamCustomerVideo_, STREAM_ID_VIDEO, intent);
    }

    if (secondStreamInfo->bufferQueue_ == nullptr) {
        CAMERA_LOGE("demo test: CreatStreams CreateProducer() secondStreamInfo is nullptr\n");
        return RC_ERROR;
    }
    streamInfos.push_back(secondStreamInfo);

    rc = streamOperator_->CreateStreams(streamInfos);
    if (rc != Camera::NO_ERROR) {
        CAMERA_LOGE("demo test: CreatStream CreateStreams error\n");
        return RC_ERROR;
    }

    rc = streamOperator_->CommitStreams(Camera::NORMAL, ability_);
    if (rc != Camera::NO_ERROR) {
        CAMERA_LOGE("demo test: CreatStream CommitStreams error\n");
        std::vector<int> streamIds = {STREAM_ID_PREVIEW, streamIdSecond};
        streamOperator_->ReleaseStreams(streamIds);
        return RC_ERROR;
    }

    return RC_OK;
}

RetCode Hos3516Demo::CaptureOnDualStreams(const int streamIdSecond)
{
    int rc = 0;
    CAMERA_LOGD("demo test: CaptuCaptureOnDualStreamsreON enter");

    std::shared_ptr<Camera::CaptureInfo> previewCaptureInfo = std::make_shared<Camera::CaptureInfo>();
    previewCaptureInfo->streamIds_ = {STREAM_ID_PREVIEW};
    previewCaptureInfo->captureSetting_ = ability_;
    previewCaptureInfo->enableShutterCallback_ = false;

    rc = streamOperator_->Capture(CAPTURE_ID_PREVIEW, previewCaptureInfo, true);
    if (rc != Camera::NO_ERROR) {
        CAMERA_LOGE("demo test: CaptureOnDualStreams preview Capture error\n");
        streamOperator_->ReleaseStreams(previewCaptureInfo->streamIds_);
        return RC_ERROR;
    }
    streamCustomerPreview_->ReceiveFrameOn(nullptr);

    std::shared_ptr<Camera::CaptureInfo> secondCaptureInfo = std::make_shared<Camera::CaptureInfo>();
    secondCaptureInfo->streamIds_ = {streamIdSecond};
    secondCaptureInfo->captureSetting_ = ability_;
    previewCaptureInfo->enableShutterCallback_ = false;

    if (streamIdSecond == STREAM_ID_CAPTURE) {
        rc = streamOperator_->Capture(CAPTURE_ID_CAPTURE, secondCaptureInfo, true);
        if (rc != Camera::NO_ERROR) {
            CAMERA_LOGE("demo test: CaptureOnDualStreams CAPTURE_ID_CAPTURE error\n");
            streamOperator_->ReleaseStreams(secondCaptureInfo->streamIds_);
            return RC_ERROR;
        }

        streamCustomerCapture_->ReceiveFrameOn([this](void* addr, const uint32_t size) {
            StoreImage(addr, size);
        });
    } else {
        rc = streamOperator_->Capture(CAPTURE_ID_VIDEO, secondCaptureInfo, true);
        if (rc != Camera::NO_ERROR) {
            CAMERA_LOGE("demo test: CaptureOnDualStreams CAPTURE_ID_VIDEO error\n");
            streamOperator_->ReleaseStreams(secondCaptureInfo->streamIds_);
            return RC_ERROR;
        }

        OpenVideoFile();
        streamCustomerVideo_->ReceiveFrameOn([this](void* addr, const uint32_t size) {
            StoreVideo(addr, size);
        });
    }

    CAMERA_LOGD("demo test: CaptuCaptureOnDualStreamsreON exit");

    return RC_OK;
}

RetCode Hos3516Demo::StartDualStreams(const int streamIdSecond)
{
    RetCode rc = RC_OK;

    CAMERA_LOGD("demo test: StartDualStreams enter");

    if (streamCustomerPreview_ == nullptr) {
        streamCustomerPreview_ = std::make_shared<StreamCustomer>();
    }

    if (isPreviewOn_ == 0) {
        isPreviewOn_ = 1;
        if (streamIdSecond == STREAM_ID_CAPTURE) {
            if (streamCustomerCapture_ == nullptr) {
                streamCustomerCapture_ = std::make_shared<StreamCustomer>();
            }

            if (isCaptureOn_ == 0) {
                isCaptureOn_ = 1;
                rc = CreatStreams(streamIdSecond, STILL_CAPTURE);
                if (rc != RC_OK) {
                    CAMERA_LOGE("demo test:StartPreviewStream CreatStreams error");
                    return RC_ERROR;
                }
            }
        } else {
            if (streamCustomerVideo_ == nullptr) {
                streamCustomerVideo_ = std::make_shared<StreamCustomer>();
            }

            if (isVideoOn_ == 0) {
                isVideoOn_ = 1;
                rc = CreatStreams(streamIdSecond, VIDEO);
                if (rc != RC_OK) {
                    CAMERA_LOGE("demo test:StartPreviewStream CreatStreams error");
                    return RC_ERROR;
                }
            }
        }
    }

    CAMERA_LOGD("demo test: StartDualStreams exit");

    return RC_OK;
}

RetCode Hos3516Demo::StartCaptureStream()
{
    RetCode rc = RC_OK;

    CAMERA_LOGD("demo test: StartCaptureStream enter");
    if (streamCustomerCapture_ == nullptr) {
        streamCustomerCapture_ = std::make_shared<StreamCustomer>();
    }

    if (isCaptureOn_ == 0) {
        isCaptureOn_ = 1;

        rc = CreatStream(STREAM_ID_CAPTURE, streamCustomerCapture_, STILL_CAPTURE);
        if (rc != RC_OK) {
            CAMERA_LOGE("demo test:StartCaptureStream CreatStream error");
            return RC_ERROR;
        }
    }

    CAMERA_LOGD("demo test: StartCaptureStream exit");

    return RC_OK;
}

RetCode Hos3516Demo::StartVideoStream()
{
    RetCode rc = RC_OK;

    CAMERA_LOGD("demo test: StartVideoStream enter");
    if (streamCustomerVideo_ == nullptr) {
        streamCustomerVideo_ = std::make_shared<StreamCustomer>();
    }

    if (isVideoOn_ == 0) {
        isVideoOn_ = 1;

        rc = CreatStream(STREAM_ID_VIDEO, streamCustomerVideo_, VIDEO);
        if (rc != RC_OK) {
            CAMERA_LOGE("demo test:StartVideoStream CreatStream error");
            return RC_ERROR;
        }
    }

    CAMERA_LOGD("demo test: StartVideoStream exit");

    return RC_OK;
}

RetCode Hos3516Demo::StartPreviewStream()
{
    RetCode rc = RC_OK;

    CAMERA_LOGD("demo test: StartPreviewStream enter");

    if (streamCustomerPreview_ == nullptr) {
        streamCustomerPreview_ = std::make_shared<StreamCustomer>();
    }

    if (isPreviewOn_ == 0) {
        isPreviewOn_ = 1;

        rc = CreatStream(STREAM_ID_PREVIEW, streamCustomerPreview_, PREVIEW);
        if (rc != RC_OK) {
            CAMERA_LOGE("demo test:StartPreviewStream CreatStream error");
            return RC_ERROR;
        }
    }

    CAMERA_LOGD("demo test: StartPreviewStream exit");

    return RC_OK;
}

RetCode Hos3516Demo::ReleaseAllStream()
{
    std::vector<int> streamIds = {};

    CAMERA_LOGD("demo test: ReleaseAllStream enter");

    if (isPreviewOn_ != 1) {
        CAMERA_LOGE("demo test: ReleaseAllStream preview is not running");
        return RC_ERROR;
    }

    if (isCaptureOn_ == 1) {
        CAMERA_LOGD("demo test: ReleaseAllStream STREAM_ID_PREVIEW STREAM_ID_CAPTURE");
        streamIds = {STREAM_ID_PREVIEW, STREAM_ID_CAPTURE};
        streamOperator_->ReleaseStreams(streamIds);
    } else {
        CAMERA_LOGD("demo test: ReleaseAllStream STREAM_ID_PREVIEW STREAM_ID_VIDEO");
        streamIds = {STREAM_ID_PREVIEW, STREAM_ID_VIDEO};
        streamOperator_->ReleaseStreams(streamIds);
    }

    isPreviewOn_ = 0;
    isCaptureOn_ = 0;
    isVideoOn_ = 0;

    CAMERA_LOGD("demo test: ReleaseAllStream exit");

    return RC_OK;
}

void Hos3516Demo::QuitDemo()
{
    ReleaseCameraDevice();
    CAMERA_LOGD("demo test: QuitDemo done\n");
}

void Hos3516Demo::SetAwbMode(const int mode) const
{
    CAMERA_LOGD("demo test: SetAwbMode enter\n");

    constexpr size_t entryCapacity = 100;
    constexpr size_t dataCapacity = 2000;

    std::shared_ptr<CameraSetting> metaData = std::make_shared<CameraSetting>(entryCapacity, dataCapacity);

    const uint8_t awbMode = mode;
    metaData->addEntry(OHOS_CONTROL_AWB_MODE, &awbMode, 1);
    demoCameraDevice_->UpdateSettings(metaData);

    CAMERA_LOGD("demo test: SetAwbMode exit\n");
}

void Hos3516Demo::SetAeExpo()
{
    int32_t expo;

    CAMERA_LOGD("demo test: SetAeExpo enter\n");

    constexpr size_t entryCapacity = 100;
    constexpr size_t dataCapacity = 2000;

    std::shared_ptr<CameraSetting> metaData = std::make_shared<CameraSetting>(entryCapacity, dataCapacity);

    if (aeStatus_) {
        expo = 0xa0;
    } else {
        expo = 0x30;
    }
    aeStatus_ = !aeStatus_;
    metaData->addEntry(OHOS_CONTROL_AE_EXPOSURE_COMPENSATION, &expo, 1);
    demoCameraDevice_->UpdateSettings(metaData);

    CAMERA_LOGD("demo test: SetAeExpo exit\n");
}

void Hos3516Demo::FlashlightOnOff(bool onOff)
{
    int rc = 0;
    CAMERA_LOGD("demo test: FlashlightOnOff enter\n");

    if (demoCameraHost_ == nullptr) {
        CAMERA_LOGE("demo test: FlashlightOnOff demoCameraHost_ == nullptr\n");
        return;
    }

    demoCameraHost_->SetFlashlight(cameraIds_.front(), onOff);

    CAMERA_LOGD("demo test: FlashlightOnOff exit \n");
}

RetCode Hos3516Demo::StreamOffline(const int streamId)
{
    int rc = 0;
    constexpr size_t offlineDelayTime = 4;
    CAMERA_LOGD("demo test: StreamOffline enter\n");

    sptr<IStreamOperatorCallback> streamOperatorCallback = new StreamOperatorCallback();
    sptr<IOfflineStreamOperator> offlineStreamOperator = nullptr;

    rc = streamOperator_->ChangeToOfflineStream({streamId}, streamOperatorCallback, offlineStreamOperator);
    if (rc != NO_ERROR) {
        CAMERA_LOGE("demo test: StreamOffline ChangeToOfflineStream error\n");
        return RC_ERROR;
    }

    CaptureOff(CAPTURE_ID_PREVIEW, CAPTURE_PREVIEW);
    CaptureOff(CAPTURE_ID_CAPTURE, CAPTURE_SNAPSHOT);
    sleep(1);
    ReleaseAllStream();
    ReleaseCameraDevice();
    sleep(offlineDelayTime);

    CAMERA_LOGD("demo test: begin to release offlne stream");
    rc = offlineStreamOperator->CancelCapture(CAPTURE_ID_CAPTURE);
    if (rc != NO_ERROR) {
        CAMERA_LOGE("demo test: StreamOffline offlineStreamOperator->CancelCapture error\n");
        return RC_ERROR;
    }

    rc = offlineStreamOperator->Release();
    if (rc != OHOS::Camera::NO_ERROR) {
        CAMERA_LOGE("demo test: StreamOffline offlineStreamOperator->Release() error\n");
        return RC_ERROR;
    }

    streamCustomerCapture_->ReceiveFrameOff();

    CAMERA_LOGD("demo test: StreamOffline exit\n");

    return RC_OK;
}
} // namespace OHOS::Camera
