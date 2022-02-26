
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
#include "offline_stream_test.h"

using namespace OHOS;
using namespace std;
using namespace testing::ext;
using namespace OHOS::Camera;

void OfflineStreamTest::SetUpTestCase(void) {}
void OfflineStreamTest::TearDownTestCase(void) {}
void OfflineStreamTest::SetUp(void)
{
    Test_ = std::make_shared<OHOS::Camera::Test>();
    Test_->Init();
    Test_->Open();
}
void OfflineStreamTest::TearDown(void)
{
    Test_->Close();

}

/**
  * @tc.name: OfflineStream
  * @tc.desc: Preview and still_capture streams, Change still_capture To OfflineStream, excepted success.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(OfflineStreamTest, Camera_Offline_0001, TestSize.Level0)
{
    std::cout << "==========[test log]Preview and still_capture streams, ";
    std::cout << " Change still_capture To OfflineStream, excepted success." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::STILL_CAPTURE};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获拍照流，连拍
    Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, true);
    sleep(5);
    // 转成离线流
    Test_->offlineStreamOperatorCallback = Test_->streamOperatorCallback;
    Test_->rc = Test_->streamOperator->ChangeToOfflineStream(
        {Test_->streamId_capture}, Test_->offlineStreamOperatorCallback, Test_->offlineStreamOperator);
    ASSERT_EQ(Test_->rc, Camera::NO_ERROR);
    std::cout << "==========[test log]Check offline: ChangeToOfflineStream rc = " << Test_->rc << std::endl;
    EXPECT_EQ(true, Test_->offlineStreamOperator != nullptr);
    if (Test_->rc == Camera::NO_ERROR) {
        std::cout << "==========[test log]Check offline stream: offline StreamOperator != nullptr" << std::endl;
    } else {
        std::cout << "==========[test log]Check offline stream: offline StreamOperator == nullptr" << std::endl;
    }
    // 原先流的后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_capture};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_capture};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
    // 离线流的后处理
    Test_->cameraDevice->Close();
    std::cout << "==========[test log]Check offline stream: Pretend to wait 5s for callback..." << std::endl;
    sleep(5);
    Test_->StopOfflineStream(Test_->captureId_capture);
}

/**
  * @tc.name: OfflineStream
  * @tc.desc: Preview and still_capture streams, Change Preview To OfflineStream, excepted fail.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(OfflineStreamTest, Camera_Offline_0002, TestSize.Level2)
{
    std::cout << "==========[test log]Check offline stream: Preview and still_capture streams, ";
    std::cout << " Change Preview To OfflineStream, excepted fail." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::STILL_CAPTURE};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获拍照流，连拍
    Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, true);
    sleep(5);
    // 转成离线流
    Test_->offlineStreamOperatorCallback = Test_->streamOperatorCallback;
    Test_->rc = Test_->streamOperator->ChangeToOfflineStream(
        {Test_->streamId_preview}, Test_->offlineStreamOperatorCallback, Test_->offlineStreamOperator);
    ASSERT_EQ(Test_->rc, Camera::METHOD_NOT_SUPPORTED);
    std::cout << "==========[test log]Check offline: ChangeToOfflineStream rc = " << Test_->rc << std::endl;
    ASSERT_EQ(true, Test_->offlineStreamOperator == nullptr);
    std::cout << "==========[test log]Check offline stream: offline StreamOperator == nullptr." << std::endl;
    // 原先流的后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_capture};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_capture};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);

}

/**
  * @tc.name: OfflineStream
  * @tc.desc: Preview and video streams, Change video To OfflineStream, excepted fail.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(OfflineStreamTest, Camera_Offline_0003, TestSize.Level2)
{
    std::cout << "==========[test log]Check offline stream: Preview and video streams, ";
    std::cout << "Change video To OfflineStream, excepted fail." << std::endl;
    // 配置两路流信息
    Test_->intents = {Camera::PREVIEW, Camera::VIDEO};
    Test_->StartStream(Test_->intents);
    // 捕获预览流
    Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
    // 捕获拍照流，连拍
    Test_->StartCapture(Test_->streamId_video, Test_->captureId_video, false, true);
    sleep(5);
    // 转成离线流
    Test_->offlineStreamOperatorCallback = Test_->streamOperatorCallback;
    Test_->rc = Test_->streamOperator->ChangeToOfflineStream(
        {Test_->streamId_video}, Test_->offlineStreamOperatorCallback, Test_->offlineStreamOperator);
    ASSERT_EQ(Test_->rc, Camera::METHOD_NOT_SUPPORTED);
    std::cout << "==========[test log]Check offline: ChangeToOfflineStream rc = " << Test_->rc << std::endl;
    ASSERT_EQ(Test_->offlineStreamOperator, nullptr);
    std::cout << "==========[test log]Check offline stream: offline StreamOperator == nullptr." << std::endl;
    // 原先流的后处理
    Test_->captureIds = {Test_->captureId_preview, Test_->captureId_video};
    Test_->streamIds = {Test_->streamId_preview, Test_->streamId_video};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}