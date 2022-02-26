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
#include "preview_test.h"

using namespace OHOS;
using namespace std;
using namespace testing::ext;
using namespace OHOS::Camera;

void PreviewTest::SetUpTestCase(void) {}
void PreviewTest::TearDownTestCase(void) {}
void PreviewTest::SetUp(void)
{
    Test_ = std::make_shared<OHOS::Camera::Test>();
    Test_->Init();
    Test_->Open();
}
void PreviewTest::TearDown(void)
{
    Test_->Close();
}

/**
  * @tc.name: Preview
  * @tc.desc: Preview stream, 640*480, expected success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0001, TestSize.Level0)
{
    std::cout << "==========[test log]Preview stream, 640*480, expected success." << std::endl;
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
  * @tc.name: Preview
  * @tc.desc: Preview stream, 1280*960, expected success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0004, TestSize.Level2)
{
    std::cout << "==========[test log]Preview stream, 1280*960, expected success." << std::endl;

    // 创建并获取streamOperator信息
    Test_->streamOperatorCallback = new StreamOperatorCallback();
    Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 创建数据流
    Test_->streamInfo = std::make_shared<StreamInfo>();
    Test_->streamInfo->streamId_ = Test_->streamId_preview;
    Test_->streamInfo->width_ = 1280;
    Test_->streamInfo->height_ = 960;
    Test_->streamInfo->datasapce_ = 8;
    Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
    Test_->streamInfo->intent_ = Camera::PREVIEW;
    std::shared_ptr<OHOS::Camera::Test::StreamConsumer> preview_consumer =
        std::make_shared<OHOS::Camera::Test::StreamConsumer>();
    Test_->streamInfo->bufferQueue_ = preview_consumer->CreateProducer([this](void* addr, uint32_t size) {
        Test_->SaveYUV("preview", addr, size);
    });
    Test_->streamInfo->bufferQueue_->SetQueueSize(8);
    Test_->consumerMap_[Camera::PREVIEW] = preview_consumer;
    Test_->streamInfo->tunneledMode_ = 5;
    std::vector<std::shared_ptr<StreamInfo>>().swap(Test_->streamInfos);
    Test_->streamInfos.push_back(Test_->streamInfo);
    Test_->rc = Test_->streamOperator->CreateStreams(Test_->streamInfos);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 配流起流
    Test_->rc = Test_->streamOperator->CommitStreams(Camera::NORMAL, Test_->ability);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 获取预览图
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 释放流
    Test_->captureIds = {Test_->captureId_preview};
    Test_->streamIds = {Test_->streamId_preview};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Preview
  * @tc.desc: GetStreamOperator success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0010, TestSize.Level0)
{
    std::cout << "==========[test log]GetStreamOperator success." << std::endl;
    // 创建并获取streamOperator信息
    Test_->streamOperatorCallback = new StreamOperatorCallback();
    Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
    EXPECT_EQ(false, Test_->rc != Camera::NO_ERROR || Test_->streamOperator == nullptr);
}

/**
  * @tc.name: Preview
  * @tc.desc: GetStreamOperator, input nullptr.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0011, TestSize.Level2)
{
    std::cout << "==========[test log]GetStreamOperator, input nullptr." << std::endl;
    // 创建并获取streamOperator信息
    Test_->rc = Test_->cameraDevice->GetStreamOperator(nullptr, Test_->streamOperator);
    std::cout << "GetStreamOperator's rc " << Test_->rc << std::endl;
    EXPECT_EQ(Camera::CamRetCode::INVALID_ARGUMENT, Test_->rc);
}

/**
  * @tc.name: Preview
  * @tc.desc: Preview, CommitStreams Metadata = nullptr.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0040, TestSize.Level2)
{
    std::cout << "==========[test log]Preview, CommitStreams Metadata = nullptr." << std::endl;
    // 创建并获取streamOperator信息
    Test_->streamOperatorCallback = new StreamOperatorCallback();
    Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 配流起流
    Test_->rc = Test_->streamOperator->CommitStreams(Camera::NORMAL, nullptr);
    EXPECT_EQ(true, Test_->rc == INVALID_ARGUMENT);
}

/**
  * @tc.name: Preview
  * @tc.desc: Preview, CommitStreams without CreateStreams, expected fail.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0050, TestSize.Level2)
{
    std::cout << "==========[test log]Preview, CommitStreams no CreateStreams, expected fail." << std::endl;
    // 创建并获取streamOperator信息
    Test_->streamOperatorCallback = new StreamOperatorCallback();
    Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 配流起流
    Test_->rc = Test_->streamOperator->CommitStreams(Camera::NORMAL, Test_->ability);
    std::cout << "streamOperator->CommitStreams's rc " << Test_->rc << std::endl;
    EXPECT_EQ(true, Test_->rc == INVALID_ARGUMENT);
}

/**
  * @tc.name: Preview
  * @tc.desc: Preview  and release streams, success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0060, TestSize.Level1)
{
    std::cout << "==========[test log]Preview  and release streams, success." << std::endl;
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
  * @tc.name: Preview
  * @tc.desc: ReleaseStreams no exist streamID, expect success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0062, TestSize.Level2)
{
    std::cout << "==========[test log]ReleaseStreams no exist streamID, expect success." << std::endl;
    // 创建并获取streamOperator信息
    Test_->streamOperatorCallback = new StreamOperatorCallback();
    Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
    EXPECT_EQ(false, Test_->rc != Camera::NO_ERROR || Test_->streamOperator == nullptr);
    Test_->streamInfo = std::make_shared<StreamInfo>();
    Test_->rc = Test_->streamOperator->ReleaseStreams({9999});
    std::cout << "streamOperator->ReleaseStreams's rc " << Test_->rc << std::endl;
    EXPECT_EQ(true, Test_->rc == Camera::NO_ERROR);
}

/**
  * @tc.name: Preview
  * @tc.desc: CancelCapture without Create capture.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0090, TestSize.Level2)
{
    std::cout << "==========[test log]CancelCapture without Create capture ." << std::endl;
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 不捕获预览图，直接取消捕获
    int captureId = 100;
    Test_->rc = Test_->streamOperator->CancelCapture(captureId);
    EXPECT_EQ(true, Test_->rc == Camera::INVALID_ARGUMENT);
    if (Test_->rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]CancelCapture success." << std::endl;
    } else {
        std::cout << "==========[test log]CancelCapture fail, rc = " << Test_->rc << std::endl;
    }
    // 释放流
    Test_->rc = Test_->streamOperator->ReleaseStreams({Test_->streamId_preview});
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    if (Test_->rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]ReleaseStreams success." << std::endl;
    } else {
        std::cout << "==========[test log]ReleaseStreams fail, rc = " << Test_->rc << std::endl;
    }
}

/**
  * @tc.name: Preview
  * @tc.desc: Release streams, then creatCapture.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0091, TestSize.Level2)
{
    std::cout << "==========[test log]Create capture, then release streams." << std::endl;
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 释放流
    Test_->rc = Test_->streamOperator->ReleaseStreams({Test_->streamId_preview});
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    if (Test_->rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]ReleaseStreams success." << std::endl;
    } else {
        std::cout << "==========[test log]ReleaseStreams fail, rc = " << Test_->rc << std::endl;
    }

    // 获取预览图
    int captureId = 100;
    Test_->captureInfo = std::make_shared<CaptureInfo>();
    Test_->captureInfo->streamIds_ = {Test_->streamId_preview};
    Test_->captureInfo->captureSetting_ = Test_->ability;
    Test_->captureInfo->enableShutterCallback_ = false;
    bool isStreaming = true;
    Test_->rc = Test_->streamOperator->Capture(captureId, Test_->captureInfo, isStreaming);
    EXPECT_EQ(Test_->rc, Camera::INVALID_ARGUMENT);
    // 关闭设备
    Test_->cameraDevice->Close();
    std::cout << "cameraDevice->Close" << std::endl;
}

/**
  * @tc.name: Preview
  * @tc.desc: The same CaptureID, Create capture twice, expected success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0092, TestSize.Level2)
{
    std::cout << "==========[test log]The same CaptureID, Create capture twice, expected success." << std::endl;
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 获取预览图
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 释放流
    Test_->captureIds = {Test_->captureId_preview};
    Test_->streamIds = {Test_->streamId_preview};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Preview
  * @tc.desc: Different captureIDs, Create capture，expected success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PreviewTest, Camera_Preview_0093, TestSize.Level2)
{
    std::cout << "==========[test log]Different captureIDs, Create capture，expected success." << std::endl;
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 获取预览图
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, false);
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_capture, false, false);
    // 释放流
    Test_->captureIds = {};
    Test_->streamIds = {Test_->streamId_preview};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}
