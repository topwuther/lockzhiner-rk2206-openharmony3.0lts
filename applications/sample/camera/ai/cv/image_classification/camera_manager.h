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

#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "camera_kit.h"
#include "recorder.h"

#include <algorithm>
#include <condition_variable>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <sstream>
#include <string>
#include <sys/io.h>
#include <sys/prctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

namespace IC {
// Settings of camera
const int32_t CAMERA_PIC_WIDTH = 1920;
const int32_t CAMERA_PIC_HEIGHT = 1080;
const std::string CAMERA_SAVE_PATH = "/storage/Capture.jpg";

// condition_variable for camera
extern std::mutex g_mutex;
extern std::condition_variable g_cv;

class SampleFrameStateCallback : public OHOS::Media::FrameStateCallback {
public:
    void OnFrameFinished(OHOS::Media::Camera &camera,
                        OHOS::Media::FrameConfig &fc,
                        OHOS::Media::FrameResult &result) override;
};

class SampleSurfaceListener : public OHOS::IBufferConsumerListener {
public:
    void SetSurface(OHOS::Media::Surface *surface);
    void OnBufferAvailable() override;

private:
    OHOS::Media::Surface *surface_ = nullptr;
};

class SampleCameraDeivceCallback : public OHOS::Media::CameraDeviceCallback {
};

class SampleCameraStateMng : public OHOS::Media::CameraStateCallback {
public:
    SampleCameraStateMng() = delete;
    explicit SampleCameraStateMng(OHOS::Media::EventHandler &eventHdlr);
    ~SampleCameraStateMng();
    void OnCreated(OHOS::Media::Camera &c) override;
    void OnCreateFailed(const std::string cameraId, int32_t errorCode) override;
    void OnReleased(OHOS::Media::Camera &c) override;
    void Capture();
private:
    OHOS::Media::EventHandler &eventHdlr_;
    OHOS::Media::Camera *cam_ = nullptr;
    SampleSurfaceListener listener_;
    SampleFrameStateCallback fsCb_;
    OHOS::Media::FrameConfig *fc_ = nullptr;
};
}  // namespace IC
#endif