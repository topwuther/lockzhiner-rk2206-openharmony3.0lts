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

#ifndef __CAMERA_MANAGER_H__
#define __CAMERA_MANAGER_H__

#include <stdio.h>
#include <sys/time.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <errno.h>

#include "camera_kit.h"
#include "recorder.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;

typedef enum {
    PHOTO_TYPE_NORMAL,
    PHOTO_TYPE_VIDEO,
    PHOTO_TYPE_TMP,

    PHOTO_TYPE_NBR
}PHOTO_TYPE;

typedef enum {
    MEDIA_STATE_IDLE = 0,
    MEDIA_STATE_START,
    MEDIA_STATE_PAUSE,
    MEDIA_STATE_STOP
} CAMERA_MEDIA_STATUS;

class TestFrameStateCallback : public FrameStateCallback {
public:
    TestFrameStateCallback() : gPhotoType_(0), gIsFinished_(false) {}
    ~TestFrameStateCallback(){}

    void OnFrameFinished(Camera &camera, FrameConfig &fc, FrameResult &result) override;
    void SetPhotoType(int type);
    bool IsFinish(void);
    void GetVideoName(char *pName, size_t length) const;
    void InitVideoName();
    void InitTimeStamp();
private:
    int gPhotoType_;
    bool gIsFinished_;
    char videoName_[256] = {0};
    char timeStamp_[256] = {0};
};

class SampleCameraStateMng : public CameraStateCallback {
public:
        SampleCameraStateMng() = delete;
        SampleCameraStateMng(EventHandler &eventHdlr) : eventHdlr_(eventHdlr)
        {
            gRecordSta_ = 0;
            gPreviewSta_ = 0;
            gRecFd_ = -1;
            cam_ = nullptr;
            recorder_ = nullptr;
            fc_ = nullptr;
        }
        ~SampleCameraStateMng();

    void OnCreated(Camera &c) override;
    void OnCreateFailed(const std::string cameraId, int32_t errorCode) override {}
    void OnReleased(Camera &c) override {}
    void StartRecord(Surface *mSurface);
    void StartPreview(Surface *surface);
    void Capture(int type);
    void SetPause();
    void SetResume(Surface *mSurface);
    void SetStop(int s);
    bool RecordState();
    bool CameraIsReady();
    bool IsCaptureOver(void);
private:

    int gRecordSta_;
    int gPreviewSta_;
    int gRecFd_;
    EventHandler &eventHdlr_;
    Camera *cam_;
    Recorder *recorder_;
    TestFrameStateCallback fsCb_;
    FrameConfig *fc_;
};

class SampleCameraManager {
public:
    SampleCameraManager()
    {
        camKit = nullptr;
        camId = "";
        CamStateMng = nullptr;
    }
    ~SampleCameraManager();

    int SampleCameraCreate();
    bool SampleCameraExist(void);
    int SampleCameraStart(Surface *surface);
    int SampleCameraStop(void);
    int SampleCameraCaptrue(int type);
    int SampleCameraStartRecord(Surface *surface);
    int SampleCameraPauseRecord(void);
    int SampleCameraResumeRecord(Surface *mSurface);
    int SampleCameraStopRecord(void);
    bool SampleCameraGetRecord(void);
    bool SampleCameraIsReady(void);
    bool SampleCameraCaptrueIsFinish(void);
private:
    CameraKit *camKit;
    string camId;
    SampleCameraStateMng *CamStateMng;
    EventHandler eventHdlr_;
};

#endif    /* __CAMERA_MANAGER_H__ */

