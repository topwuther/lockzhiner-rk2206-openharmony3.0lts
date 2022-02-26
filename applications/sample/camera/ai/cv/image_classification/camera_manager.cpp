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

#include "camera_manager.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;

namespace IC {
const int QFACTOR = 90;
std::mutex g_mutex;
std::condition_variable g_cv;

static void SampleSaveCapture(const char *p, uint32_t size)
{
    printf("Start saving picture.\n");
    ofstream pic(CAMERA_SAVE_PATH, ofstream::out | ofstream::trunc);
    if (pic.is_open()) {
        printf("Write %d bytes.\n", size);
        pic.write(p, size);
        pic.close();
        printf("Saving picture end.\n");
    }
}

void SampleSurfaceListener::SetSurface(Surface *surface)
{
    surface_ = surface;
}

void SampleFrameStateCallback::OnFrameFinished(Camera &camera, FrameConfig &fc, FrameResult &result)
{
    std::unique_lock<std::mutex> lock(g_mutex);
    printf("Receive frame complete inform.\n");
    if (fc.GetFrameConfigType() == FRAME_CONFIG_CAPTURE) {
        printf("Capture frame received.\n");
        list<Surface *> surfaceList = fc.GetSurfaces();
        for (Surface *surface : surfaceList) {
            SurfaceBuffer *buffer = surface->AcquireBuffer();
            if (buffer != nullptr) {
                char *virtAddr = static_cast<char *>(buffer->GetVirAddr());
                if (virtAddr != nullptr) {
                    SampleSaveCapture(virtAddr, buffer->GetSize());
                }
                surface->ReleaseBuffer(buffer);
            }
            delete surface;
        }
    } else if (fc.GetFrameConfigType() == FRAME_CONFIG_PREVIEW) {
        printf("Preview frame received.\n");
        list<Surface *> surfaceList = fc.GetSurfaces();
        for (Surface *surface : surfaceList) {
            printf("surface release.\n");
            delete surface;
        }
    }
    delete &fc;
    g_cv.notify_all();
}

void SampleSurfaceListener::OnBufferAvailable()
{
    if (surface_ == nullptr) {
        return;
    }
    SurfaceBuffer *buffer = surface_->AcquireBuffer();
    if (buffer != nullptr) {
        surface_->ReleaseBuffer(buffer);
    }
}

SampleCameraStateMng::SampleCameraStateMng(EventHandler &eventHdlr) : eventHdlr_(eventHdlr)
{
}

SampleCameraStateMng::~SampleCameraStateMng()
{
}

void SampleCameraStateMng::OnCreated(Camera &c)
{
    std::unique_lock<std::mutex> lock(g_mutex);
    printf("Sample recv OnCreate camera.\n");
    auto config = CameraConfig::CreateCameraConfig();
    if (config != nullptr) {
        config->SetFrameStateCallback(&fsCb_, &eventHdlr_);
    }
    c.Configure(*config);
    cam_ = &c;
    g_cv.notify_all();
}

void SampleCameraStateMng::OnCreateFailed(const string cameraId, int32_t errorCode)
{
    printf("Sample camera create failed. Error code: %d.\n", errorCode);
}

void SampleCameraStateMng::OnReleased(Camera &c)
{
    printf("Sample camera OnReleased.\n");
}

void SampleCameraStateMng::Capture()
{
    if (cam_ == nullptr) {
        printf("Camera is not ready.\n");
        return;
    }
    FrameConfig *fc = new (std::nothrow) FrameConfig(FRAME_CONFIG_CAPTURE);
    if (fc == nullptr) {
        printf("Create FrameConfig failed.\n");
        return;
    }
    Surface *surface = Surface::CreateSurface();
    if (surface == nullptr) {
        delete fc;
        printf("CreateSurface failed.\n");
        return;
    }
    surface->SetWidthAndHeight(CAMERA_PIC_WIDTH, CAMERA_PIC_HEIGHT);
    surface->SetUsage(BUFFER_CONSUMER_USAGE_HARDWARE);
    fc->AddSurface(*surface);
    fc->SetParameter(PARAM_KEY_IMAGE_ENCODE_QFACTOR, QFACTOR);
    cam_->TriggerSingleCapture(*fc);
}
}  // namespace IC