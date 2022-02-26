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

#include "hdi_callback_test.h"

using namespace OHOS;
using namespace std;
using namespace testing::ext;
using namespace OHOS::Camera;

void HdiCallbackTest::SetUpTestCase(void) {}
void HdiCallbackTest::TearDownTestCase(void) {}
void HdiCallbackTest::SetUp(void)
{
    Test_ = std::make_shared<OHOS::Camera::Test>();
    Test_->Init();
}
void HdiCallbackTest::TearDown(void)
{
    Test_->Close();

}

/**
  * @tc.name: OnCameraStatus
  * @tc.desc: CameraHostCallback, OnCameraStatus
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(HdiCallbackTest, Camera_Hdi_1001, TestSize.Level0)
{
    std::cout << "==========[test log] CameraHostCallback, OnCameraStatus."<< std::endl;
}

/**
  * @tc.name: OnFlashlightStatus
  * @tc.desc: CameraHostCallback, OnFlashlightStatus
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(HdiCallbackTest, Camera_Hdi_1002, TestSize.Level0)
{
    std::cout << "==========[test log] CameraHostCallback, OnFlashlightStatus."<< std::endl;
    Test_->service->GetCameraIds(Test_->cameraIds);
    std::string cameraId = Test_->cameraIds.front();
    Test_->deviceCallback = new CameraDeviceCallback();
    Test_->rc = Test_->service->OpenCamera(cameraId, Test_->deviceCallback, Test_->cameraDevice);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
}

/**
  * @tc.name: OnError
  * @tc.desc: CameraDeviceCallback, OnError.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(HdiCallbackTest, Camera_Hdi_1010, TestSize.Level0)
{
    std::cout << "==========[test log]CameraDeviceCallback, OnError." << std::endl;
}

/**
  * @tc.name: OnResult
  * @tc.desc: CameraDeviceCallback, OnResult.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(HdiCallbackTest, Camera_Hdi_1011, TestSize.Level0)
{
    Test_->Open();
    EXPECT_EQ(true, Test_->cameraDevice != nullptr);
    std::cout << "==========[test log]CameraDeviceCallback, OnResult." << std::endl;
    std::vector<Camera::MetaType> enableTypes;
    Test_->rc = Test_->cameraDevice->GetEnabledResults(enableTypes);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    for (auto &type : enableTypes) {
        std::cout << "==========[test log] hdi_device: type = " << type << std::endl;
    }
    Test_->rc = Test_->cameraDevice->SetResultMode(Camera::PER_FRAME);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 新增这个tag
    std::vector<Camera::MetaType> enable_tag;
    enable_tag.push_back(OHOS_SENSOR_EXPOSURE_TIME);
    enable_tag.push_back(OHOS_SENSOR_COLOR_CORRECTION_GAINS);
    Test_->rc = Test_->cameraDevice->EnableResult(enable_tag);
    std::cout << "==========[test log] EnableResult rc = " << Test_->rc << std::endl;
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 获取预览图
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    std::cout << "==========[test log] Please cover the camera..." << std::endl;
    sleep(10);
    // 释放流
    Test_->captureIds = {Test_->captureId_preview};
    Test_->streamIds = {Test_->streamId_preview};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: OnCaptureStarted
  * @tc.desc: IStreamOpereatorCallback, OnCaptureStarted
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(HdiCallbackTest, Camera_Hdi_1020, TestSize.Level0)
{
    std::cout << "==========[test log] IStreamOpereatorCallback, OnCaptureStarted." << std::endl;
    Test_->Open();
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 获取预览图
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 释放流
    Test_->captureIds = {Test_->captureId_preview};
    Test_->streamIds = {Test_->streamId_preview};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: OnCaptureEnded
  * @tc.desc: IStreamOpereatorCallback, OnCaptureEnded
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(HdiCallbackTest, Camera_Hdi_1021, TestSize.Level0)
{
    std::cout << "==========[test log]IStreamOpereatorCallback, OnCaptureEnded" << std::endl;
    Test_->Open();
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::STILL_CAPTURE};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获拍照流，单拍
    Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, false);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_capture};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: OnCaptureError
  * @tc.desc: IStreamOpereatorCallback, OnCaptureError
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(HdiCallbackTest, Camera_Hdi_1022, TestSize.Level0)
{
    std::cout << "==========[test log]IStreamOpereatorCallback, OnCaptureError" << std::endl;
}

/**
  * @tc.name: OnFrameShutter
  * @tc.desc: IStreamOpereatorCallback, OnFrameShutter
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(HdiCallbackTest, Camera_Hdi_1023, TestSize.Level0)
{
    std::cout << "==========[test log] IStreamOpereatorCallback, OnFrameShutter." << std::endl;
    Test_->Open();
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::STILL_CAPTURE};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, true, true);
    // 捕获拍照流，连拍
    Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, true, true);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_capture};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_capture};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}
