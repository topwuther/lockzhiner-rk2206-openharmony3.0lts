/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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
#include "dfx_test.h"
#include <fstream>
#include "parameters.h"
namespace {
    static const int TimeTransformation_us = 1000000;
    static const int Times = 1000;
    std::ofstream writeIntoFile;
}

using namespace OHOS;
using namespace std;
using namespace testing::ext;
using namespace OHOS::Camera;
float DfxTest::calTime(struct timeval start, struct timeval end)
{
    float time_use = 0;
    time_use = (end.tv_sec - start.tv_sec) * TimeTransformation_us + (end.tv_usec - start.tv_usec);
    return time_use;
}
void DfxTest::SetUpTestCase(void) {}
void DfxTest::TearDownTestCase(void) {}
void DfxTest::SetUp(void)
{
    Test_ = std::make_shared<OHOS::Camera::Test>();
    Test_->Init();
}
void DfxTest::TearDown(void)
{
    Test_->Close();

}

/**
  * @tc.name: Frame interrupt detection.
  * @tc.desc: Frame interrupt detection.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0001, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: Frame interrupt detection, mock 20s timeout" << std::endl;
    bool result = false;
    std::string property="Frame_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
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
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: GetStreamOperator timeout.
  * @tc.desc: mock Hdi_GetStreamOperator_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0010, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_GetStreamOperator_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_GetStreamOperator_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_GetStreamOperator_Timeout = " << parameter << std::endl;
        Test_->Open();
        Test_->streamOperatorCallback = new StreamOperatorCallback();
        Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }

    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: UpdateSettings timeout.
  * @tc.desc: mock Hdi_UpdateSettings_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0011, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_UpdateSettings_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_UpdateSettings_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_UpdateSettings_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 下发3A参数
        std::shared_ptr<Camera::CameraSetting> meta = std::make_shared<Camera::CameraSetting>(100, 2000);
        int32_t expo = 0xa0;
        meta->addEntry(OHOS_CONTROL_AE_EXPOSURE_COMPENSATION, &expo, 1);
        Test_->rc = Test_->cameraDevice->UpdateSettings(meta);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        std::cout << "cameraDevice->UpdateSettings's rc " << Test_->rc << std::endl;
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_GetEnabledResults_Timeout.
  * @tc.desc: mock Hdi_GetEnabledResults_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0012, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_GetEnabledResults_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_GetEnabledResults_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_GetEnabledResults_Timeout = " << parameter << std::endl;
        Test_->Open();
        EXPECT_EQ(true, Test_->cameraDevice != nullptr);
        std::vector<Camera::MetaType> enableTypes;
        Test_->rc = Test_->cameraDevice->GetEnabledResults(enableTypes);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        for (auto &type : enableTypes) {
            std::cout << "==========[test log]Check hdi_device: type = " << type << std::endl;
        }
        Test_->rc = Test_->cameraDevice->SetResultMode(Camera::PER_FRAME);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_GetEnabledResults_Timeout.
  * @tc.desc: mock Hdi_GetEnabledResults_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0013, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_GetEnabledResults_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_GetEnabledResults_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_GetEnabledResults_Timeout = " << parameter << std::endl;
        Test_->Open();
        EXPECT_EQ(true, Test_->cameraDevice != nullptr);
        std::vector<Camera::MetaType> enableTypes;
        Test_->rc = Test_->cameraDevice->GetEnabledResults(enableTypes);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        for (auto &type : enableTypes) {
            std::cout << "==========[test log]Check hdi_device: type = " << type << std::endl;
        }
        Test_->rc = Test_->cameraDevice->SetResultMode(Camera::PER_FRAME);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_DisableResult_Timeout.
  * @tc.desc: mock Hdi_DisableResult_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0014, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_DisableResult_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_DisableResult_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_DisableResult_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 获取设备当前支持的参数tag
        std::vector<Camera::MetaType> results_original;
        Test_->rc = Test_->cameraDevice->GetEnabledResults(results_original);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        std::cout << "==========[test log]GetEnabledResults, size = " << results_original.size() << std::endl;

        // 禁用一个tag
        std::vector<Camera::MetaType> disable_tag;
        disable_tag.push_back(results_original[2]);
        Test_->rc = Test_->cameraDevice->DisableResult(disable_tag);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        std::cout << "==========[test log]Check hdi_device: DisableResult the tag:" << results_original[0] << std::endl;

        // 再次获取设备当前支持的参数tag
        std::vector<Camera::MetaType> results;
        Test_->rc = Test_->cameraDevice->GetEnabledResults(results);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_Close_Timeout.
  * @tc.desc: mock Hdi_Close_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0015, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_Close_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_Close_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_Close_Timeout = " << parameter << std::endl;
        Test_->Open();
        Test_->Close();
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_IsStreamsSupported_Timeout.
  * @tc.desc: mock Hdi_IsStreamsSupported_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0020, TestSize.Level3) {
    std::cout << "==========[test log] Dfx: mock Hdi_IsStreamsSupported_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_IsStreamsSupported_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_IsStreamsSupported_Timeout = " << parameter << std::endl;
        // 打开相机
        Test_->Open();
        EXPECT_EQ(false, Test_->cameraDevice == nullptr);
        // 获取streamOperator
        Test_->streamOperatorCallback = new StreamOperatorCallback();
        Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        // 配置mode 和 modeSetting
        Camera::OperationMode mode = Camera::NORMAL;
        std::shared_ptr<CameraStandard::CameraMetadata> modeSetting = std::make_shared<CameraStandard::CameraMetadata>(2, 128);
        int64_t expoTime = 0;
        modeSetting->addEntry(OHOS_SENSOR_EXPOSURE_TIME, &expoTime, 1);
        int64_t colorGains[4] = {0};
        modeSetting->addEntry(OHOS_SENSOR_COLOR_CORRECTION_GAINS, &colorGains, 4);
        // 配置流信息
        Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
        Test_->streamInfo->streamId_ = 1001; // 1001:流id
        Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
        Test_->streamInfo->height_ = 480; // 480:流高度
        Test_->streamInfo->width_ = 640; // 640:流宽度
        Test_->streamInfo->datasapce_ = 8;
        std::shared_ptr<OHOS::Camera::Test::StreamConsumer> consumer =
        std::make_shared<OHOS::Camera::Test::StreamConsumer>();
        Test_->streamInfo->bufferQueue_ = consumer->CreateProducer([this](void* addr, uint32_t size) {
            Test_->SaveYUV("preview", addr, size);
        });
        Test_->streamInfo->bufferQueue_->SetQueueSize(8);
        Test_->consumerMap_[Test_->streamInfo->intent_] = consumer;
        Test_->streamInfo->intent_ = Camera::PREVIEW;
        Test_->streamInfo->tunneledMode_ = 5;
        Camera::StreamSupportType pType;
        Test_->rc = Test_->streamOperator->IsStreamsSupported(NORMAL, modeSetting, {Test_->streamInfo}, pType);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        EXPECT_NE(pType, NOT_SUPPORTED);
        if (Test_->rc == Camera::NO_ERROR) {
            std::cout << "==========[test log]Check hdi: IsStreamsSupported success, pType = " << pType << std::endl;
        } else {
            std::cout << "==========[test log]Check hdi: IsStreamsSupported fail, rc = " << Test_->rc << std::endl;
        }
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_CreateStreams_Timeout.
  * @tc.desc: mock Hdi_CreateStreams_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0021, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_CreateStreams_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_CreateStreams_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_CreateStreams_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 创建并获取streamOperator信息
        Test_->streamOperatorCallback = new StreamOperatorCallback();
        Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
        EXPECT_EQ(false, Test_->rc != Camera::NO_ERROR || Test_->streamOperator == nullptr);
        // 创建数据流
        Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
        Test_->streamInfo->streamId_ = 1001;
        Test_->streamInfo->width_ = 640;
        Test_->streamInfo->height_ = 480;
        Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
        Test_->streamInfo->datasapce_ = 8;
        Test_->streamInfo->intent_ = Camera::PREVIEW;
        Test_->streamInfo->tunneledMode_ = 5;
        std::shared_ptr<OHOS::Camera::Test::StreamConsumer> consumer =
        std::make_shared<OHOS::Camera::Test::StreamConsumer>();
        Test_->streamInfo->bufferQueue_ = consumer->CreateProducer([this](void* addr, uint32_t size) {
            Test_->SaveYUV("preview", addr, size);
        });
        Test_->consumerMap_[Test_->streamInfo->intent_] = consumer;
        Test_->streamInfos.push_back(Test_->streamInfo);
        Test_->rc = Test_->streamOperator->CreateStreams(Test_->streamInfos);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        // 释放流
        std::vector<int> streamIds;
        streamIds.push_back(Test_->streamInfo->streamId_);
        Test_->rc = Test_->streamOperator->ReleaseStreams(streamIds);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_ReleaseStreams_Timeout.
  * @tc.desc: mock Hdi_ReleaseStreams_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0022, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_ReleaseStreams_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_ReleaseStreams_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_ReleaseStreams_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 创建并获取streamOperator信息
        Test_->streamOperatorCallback = new StreamOperatorCallback();
        Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
        EXPECT_EQ(false, Test_->rc != Camera::NO_ERROR || Test_->streamOperator == nullptr);
        // 创建数据流
        Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
        Test_->streamInfo->streamId_ = 1001;
        Test_->streamInfo->width_ = 640;
        Test_->streamInfo->height_ = 480;
        Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
        Test_->streamInfo->datasapce_ = 8;
        Test_->streamInfo->intent_ = Camera::PREVIEW;
        Test_->streamInfo->tunneledMode_ = 5;
        std::shared_ptr<OHOS::Camera::Test::StreamConsumer> consumer =
        std::make_shared<OHOS::Camera::Test::StreamConsumer>();
        Test_->streamInfo->bufferQueue_ = consumer->CreateProducer([this](void* addr, uint32_t size) {
            Test_->SaveYUV("preview", addr, size);
        });
        Test_->consumerMap_[Test_->streamInfo->intent_] = consumer;
        Test_->streamInfos.push_back(Test_->streamInfo);
        Test_->rc = Test_->streamOperator->CreateStreams(Test_->streamInfos);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        // 释放流
        std::vector<int> streamIds;
        streamIds.push_back(Test_->streamInfo->streamId_);
        Test_->rc = Test_->streamOperator->ReleaseStreams(streamIds);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_CommitStreams_Timeout.
  * @tc.desc: mock Hdi_CommitStreams_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0023, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_CommitStreams_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_CommitStreams_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_CommitStreams_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 启动流
        Test_->intents = {Camera::PREVIEW};
        Test_->StartStream(Test_->intents);
        // 释放流
        Test_->captureIds = {};
        Test_->streamIds = {Test_->streamId_preview};
        Test_->StopStream(Test_->captureIds, Test_->streamIds);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_AttachBufferQueue_Timeout.
  * @tc.desc: mock Hdi_AttachBufferQueue_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0024, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_AttachBufferQueue_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_AttachBufferQueue_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_AttachBufferQueue_Timeout = " << parameter << std::endl;
        Test_->Open();
        Test_->streamOperatorCallback = new StreamOperatorCallback();
        Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        // 创建数据流
        Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
        Test_->streamInfo->streamId_ = 1001;
        Test_->streamInfo->height_ = 480;
        Test_->streamInfo->width_ = 640;
        Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
        Test_->streamInfo->datasapce_ = 8;
        Test_->streamInfo->intent_ = Camera::PREVIEW;
        Test_->streamInfo->tunneledMode_ = 5;
        std::vector<std::shared_ptr<Camera::StreamInfo>>().swap(Test_->streamInfos);
        Test_->streamInfos.push_back(Test_->streamInfo);
        Test_->rc = Test_->streamOperator->CreateStreams(Test_->streamInfos);
        std::cout << "==========[test log]Check hdi: streamOperator->CreateStreams's rc " << Test_->rc << std::endl;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        // 配流起流
        Test_->rc = Test_->streamOperator->CommitStreams(Camera::NORMAL, Test_->ability);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        std::cout << "==========[test log]Check hdi: streamOperator->CommitStreams's rc " << Test_->rc << std::endl;
        std::shared_ptr<OHOS::Camera::Test::StreamConsumer> consumer =
        std::make_shared<OHOS::Camera::Test::StreamConsumer>();
        OHOS::sptr<OHOS::IBufferProducer> producer = consumer->CreateProducer([this](void* addr, uint32_t size) {
            Test_->SaveYUV("preview", addr, size);
        });
        Test_->rc = Test_->streamOperator->AttachBufferQueue(Test_->streamInfo->streamId_, producer);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        if (Test_->rc == Camera::NO_ERROR) {
            std::cout << "==========[test log]Check hdi: AttachBufferQueue success. " << std::endl;
        } else {
            std::cout << "==========[test log]Check hdi: AttachBufferQueue fail, rc = " << Test_->rc << std::endl;
        }
        // 释放流
        Test_->captureIds = {};
        Test_->streamIds = {1001};
        Test_->StopStream(Test_->captureIds, Test_->streamIds);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_DetachBufferQueue_Timeout.
  * @tc.desc: mock Hdi_DetachBufferQueue_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0025, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_DetachBufferQueue_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_DetachBufferQueue_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_DetachBufferQueue_Timeout = " << parameter << std::endl;
        Test_->Open();
        Test_->streamOperatorCallback = new StreamOperatorCallback();
        Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        // 创建数据流
        Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
        Test_->streamInfo->streamId_ = 1001;
        Test_->streamInfo->width_ = 640;
        Test_->streamInfo->height_ = 480;
        Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
        Test_->streamInfo->intent_ = Camera::PREVIEW;
        Test_->streamInfo->datasapce_ = 8;
        Test_->streamInfo->tunneledMode_ = 5;
        std::vector<std::shared_ptr<Camera::StreamInfo>>().swap(Test_->streamInfos);
        Test_->streamInfos.push_back(Test_->streamInfo);
        Test_->rc = Test_->streamOperator->CreateStreams(Test_->streamInfos);
        std::cout << "==========[test log]Check hdi: streamOperator->CreateStreams's rc " << Test_->rc << std::endl;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        // 配流起流
        Test_->rc = Test_->streamOperator->CommitStreams(Camera::NORMAL, Test_->ability);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        std::cout << "==========[test log]Check hdi: streamOperator->CommitStreams's rc " << Test_->rc << std::endl;
        std::shared_ptr<OHOS::Camera::Test::StreamConsumer> consumer =
        std::make_shared<OHOS::Camera::Test::StreamConsumer>();
        OHOS::sptr<OHOS::IBufferProducer> producer = consumer->CreateProducer([this](void* addr, uint32_t size) {
            Test_->SaveYUV("preview", addr, size);
        });
        Test_->rc = Test_->streamOperator->AttachBufferQueue(Test_->streamInfo->streamId_, producer);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        if (Test_->rc == Camera::NO_ERROR) {
            std::cout << "==========[test log]Check hdi: AttachBufferQueue success. " << std::endl;
        } else {
            std::cout << "==========[test log]Check hdi: AttachBufferQueue fail, rc = " << Test_->rc << std::endl;
        }
        sleep(3);
        Test_->rc = Test_->streamOperator->DetachBufferQueue(Test_->streamInfo->streamId_);
        std::cout << "==========[test log]Check hdi: streamOperator->DetachBufferQueue's rc " << Test_->rc << std::endl;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        // 释放流
        Test_->captureIds = {};
        Test_->streamIds = {1001};
        Test_->StopStream(Test_->captureIds, Test_->streamIds);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_Capture_Timeout.
  * @tc.desc: mock Hdi_Capture_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0026, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_Capture_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_Capture_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_Capture_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 配置预览流信息
        Test_->intents = {Camera::PREVIEW};
        Test_->StartStream(Test_->intents);
        // 捕获预览流
        Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, true, true);
        // 后处理
        Test_->captureIds = {Test_->captureId_preview};
        Test_->streamIds = {Test_->streamId_preview};
        Test_->StopStream(Test_->captureIds, Test_->streamIds);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_ChangeToOfflineStream_Timeout.
  * @tc.desc: mock Hdi_ChangeToOfflineStream_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0027, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_ChangeToOfflineStream_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_ChangeToOfflineStream_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_ChangeToOfflineStream_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 1、配置两路流信息
        Test_->intents = {Camera::PREVIEW, Camera::STILL_CAPTURE};
        Test_->StartStream(Test_->intents);
        // 2、捕获预览流
        Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
        // 3、捕获拍照流，连拍
        Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, true);
        sleep(5);
        // 4、转成离线流
        Test_->offlineStreamOperatorCallback = Test_->streamOperatorCallback;
        Test_->rc = Test_->streamOperator->ChangeToOfflineStream(
            {Test_->streamId_capture}, Test_->offlineStreamOperatorCallback, Test_->offlineStreamOperator);
        ASSERT_EQ(Test_->rc, Camera::NO_ERROR);
        std::cout << "==========[test log] ChangeToOfflineStream rc = " << Test_->rc << std::endl;
        EXPECT_EQ(true, Test_->offlineStreamOperator != nullptr);
        if (Test_->rc == Camera::NO_ERROR) {
            std::cout << "==========[test log] offline StreamOperator != nullptr" << std::endl;
        } else {
            std::cout << "==========[test log] offline StreamOperator == nullptr" << std::endl;
        }
        // 5、原先流的后处理
        Test_->captureIds = {Test_->captureId_preview, Test_->captureId_capture};
        Test_->streamIds = {Test_->streamId_preview, Test_->streamId_capture};
        Test_->StopStream(Test_->captureIds, Test_->streamIds);
        // 6、离线流的后处理
        Test_->cameraDevice->Close();
        std::cout << "==========[test log] Pretend to wait 5s for callback..." << std::endl;
        sleep(5);
        Test_->StopOfflineStream(Test_->captureId_capture);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_Offline_CancelCapture_Timeout.
  * @tc.desc: mock Hdi_Offline_ReleaseStreams_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0030, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_Offline_CancelCapture_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_Offline_CancelCapture_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_Offline_CancelCapture_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 1、配置两路流信息
        Test_->intents = {Camera::PREVIEW, Camera::STILL_CAPTURE};
        Test_->StartStream(Test_->intents);
        // 2、捕获预览流
        Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
        // 3、捕获拍照流，连拍
        Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, true);
        sleep(5);
        // 4、转成离线流
        Test_->offlineStreamOperatorCallback = Test_->streamOperatorCallback;
        Test_->rc = Test_->streamOperator->ChangeToOfflineStream(
            {Test_->streamId_capture}, Test_->offlineStreamOperatorCallback, Test_->offlineStreamOperator);
        ASSERT_EQ(Test_->rc, Camera::NO_ERROR);
        std::cout << "==========[test log] ChangeToOfflineStream rc = " << Test_->rc << std::endl;
        EXPECT_EQ(true, Test_->offlineStreamOperator != nullptr);
        if (Test_->rc == Camera::NO_ERROR) {
            std::cout << "==========[test log] offline StreamOperator != nullptr" << std::endl;
        } else {
            std::cout << "==========[test log] offline StreamOperator == nullptr" << std::endl;
        }
        // 5、原先流的后处理
        Test_->captureIds = {Test_->captureId_preview, Test_->captureId_capture};
        Test_->streamIds = {Test_->streamId_preview, Test_->streamId_capture};
        Test_->StopStream(Test_->captureIds, Test_->streamIds);
        // 6、离线流的后处理
        Test_->cameraDevice->Close();
        std::cout << "==========[test log] Pretend to wait 5s for callback..." << std::endl;
        sleep(5);
        Test_->StopOfflineStream(Test_->captureId_capture);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_Offline_ReleaseStreams_Timeout.
  * @tc.desc: mock Hdi_Offline_ReleaseStreams_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0031, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_Offline_ReleaseStreams_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_Offline_ReleaseStreams_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_Offline_ReleaseStreams_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 1、配置两路流信息
        Test_->intents = {Camera::PREVIEW, Camera::STILL_CAPTURE};
        Test_->StartStream(Test_->intents);
        // 2、捕获预览流
        Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
        // 3、捕获拍照流，连拍
        Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, true);
        sleep(5);
        // 4、转成离线流
        Test_->offlineStreamOperatorCallback = Test_->streamOperatorCallback;
        Test_->rc = Test_->streamOperator->ChangeToOfflineStream(
            {Test_->streamId_capture}, Test_->offlineStreamOperatorCallback, Test_->offlineStreamOperator);
        ASSERT_EQ(Test_->rc, Camera::NO_ERROR);
        std::cout << "==========[test log] ChangeToOfflineStream rc = " << Test_->rc << std::endl;
        EXPECT_EQ(true, Test_->offlineStreamOperator != nullptr);
        if (Test_->rc == Camera::NO_ERROR) {
            std::cout << "==========[test log] offline StreamOperator != nullptr" << std::endl;
        } else {
            std::cout << "==========[test log] offline StreamOperator == nullptr" << std::endl;
        }
        // 5、原先流的后处理
        Test_->captureIds = {Test_->captureId_preview, Test_->captureId_capture};
        Test_->streamIds = {Test_->streamId_preview, Test_->streamId_capture};
        Test_->StopStream(Test_->captureIds, Test_->streamIds);
        // 6、离线流的后处理
        Test_->cameraDevice->Close();
        std::cout << "==========[test log] Pretend to wait 5s for callback..." << std::endl;
        sleep(5);
        Test_->StopOfflineStream(Test_->captureId_capture);
    }
    result = OHOS::system::SetParameter(property, "off");
}

/**
  * @tc.name: Hdi_Offline_Release_Timeout.
  * @tc.desc: mock Hdi_Offline_Release_Timeout.
  * @tc.size: MediumTest
  * @tc.type: DFX
  */
HWTEST_F(DfxTest, Camera_Dfx_0032, TestSize.Level3)
{
    std::cout << "==========[test log] Dfx: mock Hdi_Offline_Release_Timeout." << std::endl;
    bool result = false;
    std::string property="Hdi_Offline_Release_Timeout";
    std::string value = "on";
    std::string parameter;
    result = OHOS::system::SetParameter(property, value);
    if(result){
        parameter = OHOS::system::GetParameter(property, value);
        std::cout << "==========[test log] DFX: GetProperty Hdi_Offline_Release_Timeout = " << parameter << std::endl;
        Test_->Open();
        // 1、配置两路流信息
        Test_->intents = {Camera::PREVIEW, Camera::STILL_CAPTURE};
        Test_->StartStream(Test_->intents);
        // 2、捕获预览流
        Test_->StartCapture(Test_->streamId_preview, Test_->captureId_preview, false, true);
        // 3、捕获拍照流，连拍
        Test_->StartCapture(Test_->streamId_capture, Test_->captureId_capture, false, true);
        sleep(5);
        // 4、转成离线流
        Test_->offlineStreamOperatorCallback = Test_->streamOperatorCallback;
        Test_->rc = Test_->streamOperator->ChangeToOfflineStream(
            {Test_->streamId_capture}, Test_->offlineStreamOperatorCallback, Test_->offlineStreamOperator);
        ASSERT_EQ(Test_->rc, Camera::NO_ERROR);
        std::cout << "==========[test log] ChangeToOfflineStream rc = " << Test_->rc << std::endl;
        EXPECT_EQ(true, Test_->offlineStreamOperator != nullptr);
        if (Test_->rc == Camera::NO_ERROR) {
            std::cout << "==========[test log] offline StreamOperator != nullptr" << std::endl;
        } else {
            std::cout << "==========[test log] offline StreamOperator == nullptr" << std::endl;
        }
        // 5、原先流的后处理
        Test_->captureIds = {Test_->captureId_preview, Test_->captureId_capture};
        Test_->streamIds = {Test_->streamId_preview, Test_->streamId_capture};
        Test_->StopStream(Test_->captureIds, Test_->streamIds);
        // 6、离线流的后处理
        Test_->cameraDevice->Close();
        std::cout << "==========[test log] Pretend to wait 5s for callback..." << std::endl;
        sleep(5);
        Test_->StopOfflineStream(Test_->captureId_capture);
    }
    result = OHOS::system::SetParameter(property, "off");
}
