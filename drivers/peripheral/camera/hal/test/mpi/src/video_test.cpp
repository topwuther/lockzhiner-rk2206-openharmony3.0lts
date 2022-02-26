/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file expected in compliance with the License.
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

#include "video_test.h"

using namespace OHOS;
using namespace std;
using namespace testing::ext;
using namespace OHOS::Camera;

void VideoTest::SetUpTestCase(void) {}
void VideoTest::TearDownTestCase(void) {}
void VideoTest::SetUp(void)
{
    Test_ = std::make_shared<OHOS::Camera::Test>();
    Test_->Init();
    Test_->Open();
}
void VideoTest::TearDown(void)
{
    Test_->Close();

}

/**
  * @tc.name: Video
  * @tc.desc: Preview and video streams, Commit 2 streams together, capture in order.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(VideoTest, Camera_Video_0001, TestSize.Level0)
{
    std::cout << "==========[test log]Check video: Preview and video streams, ";
    std::cout << "Commit 2 streams together, capture in order." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获录像流
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Video
  * @tc.desc: Preview and video streams + 3A, Commit 2 streams together, capture in order.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(VideoTest, Camera_Video_0002, TestSize.Level1)
{
    std::cout << "==========[test log]Check video: Preview and video streams + 3A, ";
    std::cout << "Commit 2 streams together, capture in order." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 下发3A参数，增加曝光度
    std::shared_ptr<Camera::CameraSetting> meta = std::make_shared<Camera::CameraSetting>(100, 2000);
    int32_t expo = 0xb0;
    meta->addEntry(OHOS_CONTROL_AE_EXPOSURE_COMPENSATION, &expo, 1);
    Test_->rc = Test_->cameraDevice->UpdateSettings(meta);
    if (Test_->rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]check Capture: UpdateSettings success, for 10s." << std::endl;
    } else {
        std::cout << "==========[test log]check Capture: UpdateSettings fail, rc = " << Test_->rc << std::endl;
    }
    // 捕获录像流
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    sleep(5);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Video
  * @tc.desc: Preview + video, commit together, then close device, and preview + video again.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(VideoTest, Camera_Video_0003, TestSize.Level1)
{
    std::cout << "==========[test log]Check video: Preview + video, ";
    std::cout << "commit together, then close device, and preview + video again." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获录像流
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
    Test_->consumerMap_.clear();
    std::cout << "==========[test log]Check video: The 2nd time." << std::endl;
    // 第二次配置两路流信息
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获录像流
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Video
  * @tc.desc: Preview + video, commit together, then close device, and preview + capture.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(VideoTest, Camera_Video_0004, TestSize.Level1)
{
    std::cout << "==========[test log]Check video: Preview + video, ";
    std::cout << "commit together, then close device, and preview + capture." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获录像流
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
    Test_->consumerMap_.clear();

    std::cout << "==========[test log]Check video: The 2nd time." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::STILL_CAPTURE};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获拍照流，连拍
    Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, true);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_capture};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_capture};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Video
  * @tc.desc: Preview + video, commit together, success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(VideoTest, Camera_Video_0005, TestSize.Level0)
{
    std::cout << "==========[test log]Check video: Preview + video, commit together, success." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获录像流
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Video
  * @tc.desc: Dynamic Video mode, preview, success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(VideoTest, Camera_Video_0020, TestSize.Level1)
{
    std::cout << "==========[test log]Check video: Video mode, preview, success." << std::endl;
    // 启动预览流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 获取预览图
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 启流
    Test_->intents = {Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 抓拍
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    // 释放流
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Video
  * @tc.desc: Video mode, preview, set 3A, success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(VideoTest, Camera_Video_0021, TestSize.Level1)
{
    std::cout << "==========[test log]Check video: Video mode, preview, set 3A, success." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 下发3A参数，增加曝光度
    std::shared_ptr<Camera::CameraSetting> meta = std::make_shared<Camera::CameraSetting>(100, 2000);
    int32_t expo = 0xc0;
    meta->addEntry(OHOS_CONTROL_AE_EXPOSURE_COMPENSATION, &expo, 1);
    Test_->rc = Test_->cameraDevice->UpdateSettings(meta);
    if (Test_->rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]check Capture: UpdateSettings success, for 10s." << std::endl;
    } else {
        std::cout << "==========[test log]check Capture: UpdateSettings fail, rc = " << Test_->rc << std::endl;
    }
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    sleep(5);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Video
  * @tc.desc: Preview and video streams + 3A, Commit 2 streams together, capture in order.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(VideoTest, Camera_Video_0022, TestSize.Level2)
{
    std::cout << "==========[test log]Check video: Preview and video streams + 3A, ";
    std::cout << "Commit 2 streams together, capture in order." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 下发3A参数，增加曝光度
    std::shared_ptr<Camera::CameraSetting> meta = std::make_shared<Camera::CameraSetting>(100, 2000);
    int32_t expo = 0xb0;
    meta->addEntry(OHOS_CONTROL_AE_EXPOSURE_COMPENSATION, &expo, 1);
    Test_->rc = Test_->cameraDevice->UpdateSettings(meta);
    if (Test_->rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]check Capture: UpdateSettings success, for 10s." << std::endl;
    } else {
        std::cout << "==========[test log]check Capture: UpdateSettings fail, rc = " << Test_->rc << std::endl;
    }
    // 捕获录像流
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    sleep(1800);
    // 后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}
