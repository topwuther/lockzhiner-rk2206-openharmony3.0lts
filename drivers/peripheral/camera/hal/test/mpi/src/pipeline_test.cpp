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

#include "pipeline_test.h"

using namespace OHOS;
using namespace std;
using namespace testing::ext;
using namespace OHOS::Camera;

void PipelineTest::SetUpTestCase(void) {}
void PipelineTest::TearDownTestCase(void) {}
void PipelineTest::SetUp(void)
{
    Test_ = std::make_shared<OHOS::Camera::Test>();
    Test_->Init();
    Test_->Open();
}
void PipelineTest::TearDown(void)
{
    Test_->Close();

}

/**
  * @tc.name: Check ppl
  * @tc.desc: preview success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PipelineTest, Camera_Ppl_0001, TestSize.Level0)
{
    std::cout << "==========[test log]Check ppl: preview success." << std::endl;
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
  * @tc.name: Check ppl
  * @tc.desc: preview + capture success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PipelineTest, Camera_Ppl_0002, TestSize.Level1)
{
    std::cout << "==========[test log]Check ppl: preview + capture success." << std::endl;
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
  * @tc.name: Check ppl
  * @tc.desc: preview + video success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PipelineTest, Camera_Ppl_0003, TestSize.Level1)
{
    std::cout << "==========[test log]Check ppl:Check ppl: preview + video success." << std::endl;
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
  * @tc.name: Check ppl
  * @tc.desc: video mode without preview, system not support, expected return fail.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PipelineTest, Camera_Ppl_0004, TestSize.Level2)
{
    std::cout << "==========[test log]Check ppl:Video mode no preview, not support, expected fail." << std::endl;
    EXPECT_EQ(true, Test_->cameraDevice != nullptr);
    Test_->streamOperatorCallback = new StreamOperatorCallback();
    Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    if (Test_->rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]Check ppl: GetStreamOperator success." << std::endl;
    } else {
        std::cout << "==========[test log]Check ppl: GetStreamOperator fail, rc = " << Test_->rc << std::endl;
    }
    Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
    Test_->streamInfo->streamId_ = Test_->streamId_video;
    Test_->streamInfo->width_ = 640;
    Test_->streamInfo->height_ = 480;
    Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
    Test_->streamInfo->datasapce_ = 10;
    Test_->streamInfo->intent_ = Camera::VIDEO;
    Test_->streamInfo->tunneledMode_ = 5;
    std::shared_ptr<OHOS::Camera::Test::StreamConsumer> consumer =
        std::make_shared<OHOS::Camera::Test::StreamConsumer>();
    Test_->streamInfo->bufferQueue_ = consumer->CreateProducer([this](void* addr, uint32_t size) {
        Test_->SaveYUV("video", addr, size);
    });
    Test_->streamInfo->bufferQueue_->SetQueueSize(8);
    Test_->consumerMap_[Camera::PREVIEW] = consumer;
    std::vector<std::shared_ptr<Camera::StreamInfo>>().swap(Test_->streamInfos);
    Test_->streamInfos.push_back(Test_->streamInfo);
    Test_->rc = Test_->streamOperator->CreateStreams(Test_->streamInfos);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    std::cout << "==========[test log]CreateStreams rc = " << Test_->rc << std::endl;
    Test_->rc = Test_->streamOperator->CommitStreams(Camera::NORMAL, Test_->ability);
    EXPECT_EQ(Test_->rc, Camera::INVALID_ARGUMENT);
    std::cout << "==========[test log]CommitStreams rc = " << Test_->rc << std::endl;
    // 后处理
    Test_->streamIds = {Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Only Still_capture stream
  * @tc.desc: Only Still_capture stream, capture->isStreaming = false.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PipelineTest, Camera_Ppl_0005, TestSize.Level1)
{
    std::cout << "==========[test log]check ppl: Still_capture stream, capture->isStreaming = false." << std::endl;
    // 启流
    Test_->intents = {Camera::STILL_CAPTURE};
    Test_->StartStream(Test_->intents);
    // 抓拍
    Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, false);
    // 后处理
    Test_->streamIds = {Test_->streamId_capture};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Only Still_capture stream
  * @tc.desc: Only Still_capture stream, capture->isStreaming = true.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PipelineTest, Camera_Ppl_0006, TestSize.Level1)
{
    std::cout << "==========[test log]check ppl: Still_capture stream, capture->isStreaming = true." << std::endl;
    // 启流
    Test_->intents = {Camera::STILL_CAPTURE};
    Test_->StartStream(Test_->intents);
    // 抓拍
    Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, true);
    // 后处理
    Test_->captureIds = {Test_->captureId_capture};
    Test_->streamIds = {Test_->streamId_capture};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: Check ppl
  * @tc.desc: double preview streams.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PipelineTest, Camera_Ppl_0007, TestSize.Level1)
{
    std::cout << "==========[test log]Check ppl: double preview streams." << std::endl;
    // 创建并获取streamOperator信息
    Test_->streamOperatorCallback = new StreamOperatorCallback();
    Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 创建数据流 1
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

    // 创建数据流 2
    Test_->streamInfo2 = std::make_shared<StreamInfo>();
    Test_->streamInfo2->streamId_ = Test_->streamId_preview_double;
    Test_->streamInfo2->width_ = 1280;
    Test_->streamInfo2->height_ = 960;
    Test_->streamInfo2->datasapce_ = 8;
    Test_->streamInfo2->format_ = PIXEL_FMT_YCRCB_420_SP;
    Test_->streamInfo2->intent_ = Camera::PREVIEW;
    std::shared_ptr<OHOS::Camera::Test::StreamConsumer> preview_consumer2 =
        std::make_shared<OHOS::Camera::Test::StreamConsumer>();
    Test_->streamInfo2->bufferQueue_ = preview_consumer2->CreateProducer([this](void* addr, uint32_t size) {
        Test_->SaveYUV("preview2", addr, size);
    });
    Test_->streamInfo2->bufferQueue_->SetQueueSize(8);
    Test_->consumerMap_[Camera::PREVIEW] = preview_consumer2;
    Test_->streamInfo2->tunneledMode_ = 5;
    Test_->streamInfos.push_back(Test_->streamInfo2);

    Test_->rc = Test_->streamOperator->CreateStreams(Test_->streamInfos);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 配流起流
    Test_->rc = Test_->streamOperator->CommitStreams(Camera::NORMAL, Test_->ability);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 获取预览图
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    Test_->StartCapture(Test_->streamId_preview_double, Test_->captureId_preview_double, false, true);
    // 释放流
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_preview_double};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_preview_double};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}