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

#ifndef ACTS_MEDIA_RECODER_TEST_H
#define ACTS_MEDIA_RECODER_TEST_H

#include <unistd.h>
#include "recorder.h"
#include "gtest/gtest.h"
#include "camera_kit.h"
#include "TestCaseParse.h"
#include "ActsTestMediaUtils.h"

#define TESTCASECOUNT 38

const string g_testPath;
const string g_recordFilePath;
static int32_t g_fdNull = -1; // fd is null
static int32_t g_sleepTime = 1;

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;

/* *
 * creat camera device callback
 */
class SampleCameraDeviceCallback : public CameraDeviceCallback {
public:
    SampleCameraDeviceCallback()
    {
        cout << "camera device call back created" << endl;
    }

    ~SampleCameraDeviceCallback()
    {
        cout << "camera device call back destroyed" << endl;
    }
    /* *
     * camera statu changed
     */
    void OnCameraStatus(std::string cameraId, int32_t status) override;
};

/* *
 * create FrameStateCallback
 */
class SampleFrameStateCallback : public FrameStateCallback {
    void OnFrameFinished(Camera &camera, FrameConfig &fc, FrameResult &result) override;
    void OnFrameError(Camera &camera, FrameConfig &frameConfig, int32_t errorCode, FrameResult &frameResult) override;
};

/* *
 * create CameraStateCallback
 */
class SampleCameraStateMng : public CameraStateCallback {
public:
    SampleCameraStateMng() = delete;

    SampleCameraStateMng(EventHandler &eventHandler) : eventHandler(eventHandler) {}

    ~SampleCameraStateMng()
    {
        CloseRecorder();
        if (cam_ != nullptr) {
            cam_->Release();
        }
    }

    // close Recorder
    void CloseRecorder();

    // 相机创建成功
    void OnCreated(Camera &c) override;

    // 相机创建失败
    void OnCreateFailed(const std::string cameraId, int32_t errorCode) override
    {
        cout << "CameraStateCallback OnCreateFailed\n" << endl;
    }

    void OnReleased(Camera &c) override
    {
        cout << "CameraStateCallback OnReleased\n" << endl;
    }

    void OnConfigured(Camera &c) override
    {
        cout << "CameraStateCallback OnConfigured\n" << endl;
    }

    void OnConfigureFailed(const std::string cameraId, int32_t errorCode) override
    {
        cout << "CameraStateCallback OnConfigureFailed\n" << endl;
    }

    // 创建Reorder
    int32_t SampleCreateRecorder(TestCase &ft);

    // 准备录像
    int32_t PrepareRecorder();

    // 开始录像
    int32_t StartRecord();

    /* *
     * 开始预览
     */
    int32_t StartPreview();

    /* *
     * 拍照，并保存文件
     */
    int32_t Capture();

    /* *
     * 停止录像
     */
    int32_t Stop();

private:
    enum State : int32_t {
        STATE_IDLE,
        STATE_RUNNING,
        STATE_BUTT
    };
    State previewState_ = STATE_IDLE;
    State recordState_ = STATE_IDLE;
    EventHandler &eventHandler;
    Recorder *recorder_ = nullptr;
    Camera *cam_ = nullptr;
    int32_t recordFd_ = g_fdNull;
    SampleFrameStateCallback fsCb_;
};

/* *
 * Media Recorder test
 */
class ActsMediaRecorderTest : public testing::Test {
public:
    /* *
     * @tc.setup:This is before test class
     */
    static void SetUpTestCase(void);

    /* *
     * @tc.setup:This is after test class
     */
    static void TearDownTestCase(void);

    /* *
     * @tc.setup:This is before test case
     */
    virtual void SetUp(void);

    /* *
     * @tc.setup:This is after test case
     */
    virtual void TearDown(void);

    /* *
     * Create CameraKit
     */
    virtual void CreateCameraKit(void);

    /* *
     * release Camera
     */
    virtual void ReleaseCameraKit(void);

    CameraKit *cameraKit_;
    string camId;
};

#endif // ACTS_MEDIA_RECODER_TEST_H