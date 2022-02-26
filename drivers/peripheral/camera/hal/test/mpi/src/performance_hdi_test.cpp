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
#include <fstream>
#include "performance_hdi_test.h"
namespace {
    static const int Times = 1000;
    static const int TimeTransformation_us = 1000000;
    std::ofstream writeIntoFile;
}

using namespace OHOS;
using namespace std;
using namespace testing::ext;
using namespace OHOS::Camera;
float PerformanceTest::calTime(struct timeval start, struct timeval end)
{
    float time_use = 0;
    time_use = (end.tv_sec - start.tv_sec) * TimeTransformation_us + (end.tv_usec - start.tv_usec);
    return time_use;
    // return time us
}
void PerformanceTest::SetUpTestCase(void) {}
void PerformanceTest::TearDownTestCase(void) {}
void PerformanceTest::SetUp(void)
{
    Test_ = std::make_shared<OHOS::Camera::Test>();
    Test_->Init();
}
void PerformanceTest::TearDown(void)
{
    Test_->Close();

}

/**
  * @tc.name: HDI_GetCameraIds's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0010, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: HDI_GetCameraIds's time consuming."<< std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    for (int i= 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->service->GetCameraIds(Test_->cameraIds);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use/ Times;
    std::cout << "==========[test log] Performance: HDI_GetCameraIds's average time consuming: ";
    std::cout << avrg_time << "us." << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_GetCameraIds's average time consuming: ";
    writeIntoFile << avrg_time << "us." << std::endl;
}

/**
  * @tc.name: GetCameraAbility
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0020, TestSize.Level0)
{
    std::cout << "==========[test log] Performance: GetCameraAbility's average time consuming." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    if (Test_->cameraDevice == nullptr) {
        Test_->rc = Test_->service->GetCameraIds(Test_->cameraIds);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        for (int i= 0; i < Times; i++) {
            gettimeofday(&start, NULL);
            Test_->rc = Test_->service->GetCameraAbility(Test_->cameraIds.front(), Test_->ability);
            gettimeofday(&end, NULL);
            time_use = calTime(start, end);
            totle_time_use = totle_time_use + time_use;
            EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        }
        float avrg_time = totle_time_use/ Times;
        std::cout << "==========[test log] Performance: GetCameraAbility's average time consuming: ";
        std::cout << avrg_time << "us." << std::endl;
        writeIntoFile << "==========[test log] Performance: GetCameraAbility's average time consuming: ";
        writeIntoFile << avrg_time << "us." << std::endl;
    }
}

/**
  * @tc.name: HDI_OpenCamera's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0030, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: HDI_OpenCamera's time consuming."<< std::endl;
    Test_->service->GetCameraIds(Test_->cameraIds);
    std::string cameraId = Test_->cameraIds.front();
    Test_->deviceCallback = new CameraDeviceCallback();
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    for (int i= 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->service->OpenCamera(cameraId, Test_->deviceCallback, Test_->cameraDevice);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use/Times;
    std::cout << "==========[test log] Performance: HDI_OpenCamera's average time consuming: ";
    std::cout << avrg_time << "us." << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_OpenCamera's average time consuming: ";
    writeIntoFile << avrg_time << "us." << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: HDI_SetFlashlight's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0040, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: HDI_SetFlashlight's time consuming." << std::endl;
    // 打开相机
    Test_->Open();
    // 循环打开、关闭手电筒
    bool status;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    for (int i = 0; i < Times/2; i++) {
        // 打开手电筒
        status = true;
        gettimeofday(&start, NULL);
        Test_->rc = Test_->service->SetFlashlight(Test_->cameraIds.front(), status);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        // 关闭手电筒
        status = false;
        gettimeofday(&start, NULL);
        Test_->rc = Test_->service->SetFlashlight(Test_->cameraIds.front(), status);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_SetFlashlight's average time consuming: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_SetFlashlight's average time consuming: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: GetStreamOperator's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0050, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: GetStreamOperator success, 1000 times." << std::endl;
    // 获取配置的cameraId
    Test_->service->GetCameraIds(Test_->cameraIds);
    std::cout << "cameraIds.front() = " << Test_->cameraIds.front() << std::endl;
    // 打开相机
    Test_->Open();
    // 调用device的GetStreamOperator函数获取streamOperator
    Test_->streamOperatorCallback = new StreamOperatorCallback();
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    for (int i = 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
        gettimeofday(&end, NULL);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_cameraDevice->GetStreamOperator's average time: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_cameraDevice->GetStreamOperator's average time: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: HDI_UpdateSettings's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0060, TestSize.Level3)
{
    std::cout << "==========[test log] Check HDI_UpdateSettings's time consuming." << std::endl;
    Test_->Open();
    // 下发3A参数
    std::shared_ptr<Camera::CameraSetting> meta = std::make_shared<Camera::CameraSetting>(100, 2000);
    std::vector<uint8_t> awbMode = {
        OHOS_CAMERA_AWB_MODE_OFF,
        OHOS_CAMERA_AWB_MODE_TWILIGHT,
        OHOS_CAMERA_AWB_MODE_AUTO,
        OHOS_CAMERA_AWB_MODE_WARM_FLUORESCENT,
        OHOS_CAMERA_AWB_MODE_DAYLIGHT,
        OHOS_CAMERA_AWB_MODE_CLOUDY_DAYLIGHT,
        OHOS_CAMERA_AWB_MODE_INCANDESCENT,
        OHOS_CAMERA_AWB_MODE_FLUORESCENT,
        OHOS_CAMERA_AWB_MODE_SHADE
    };
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    for (int round = 0; round < Times; round ++) {
        int i = rand() % 9;
        std::cout << "round = "<< round << ", i = " << i << std::endl;
        meta->addEntry(OHOS_CONTROL_AWB_MODE, &awbMode.at(i), 1);
        gettimeofday(&start, NULL);
        Test_->rc = Test_->cameraDevice->UpdateSettings(meta);
        gettimeofday(&end, NULL);
        std::cout << "rc = "<< Test_->rc << std::endl;
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_UpdateSettings's  turn on average time : ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_UpdateSettings's  turn on average time : ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: SetResultMode's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0070, TestSize.Level3)
{
    std::cout << "==========[test log]Check Performance: HDI_cameraDevice->SetResultMode's average time" << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    Test_->Open();
    EXPECT_EQ(true, Test_->cameraDevice != nullptr);
    std::vector<Camera::MetaType> enableTypes;
    Test_->rc = Test_->cameraDevice->GetEnabledResults(enableTypes);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    for (int i = 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->cameraDevice->SetResultMode(Camera::PER_FRAME);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_cameraDevice->SetResultMode's average time: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_cameraDevice->SetResultMode's average time: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: GetEnabledResults
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0080, TestSize.Level3)
{
    std::cout << "==========[test log]Performance: HDI_cameraDevice->GetEnabledResults's average time." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    Test_->Open();
    std::vector<Camera::MetaType> results;
    for (int i = 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->cameraDevice->GetEnabledResults(results);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_cameraDevice->GetEnabledResults's average time: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_cameraDevice->GetEnabledResults's average time: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: EnableResult
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0090, TestSize.Level3)
{
    std::cout << "==========[test log]Performance: HDI_cameraDevice->EnableResult's average time." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    Test_->Open();
    // 获取设备当前支持的参数tag
    std::cout << "==========[test log]Check hdi_device: 1. Get the tags..." << std::endl;
    std::vector<Camera::MetaType> results_original;
    Test_->rc = Test_->cameraDevice->GetEnabledResults(results_original);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 新增这个tag
    std::vector<Camera::MetaType> enable_tag;
    enable_tag.push_back(results_original[1]);
    for (int i = 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->cameraDevice->EnableResult(enable_tag);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_cameraDevice->EnableResult's average time: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_cameraDevice->EnableResult's average time: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: DisableResult
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0100, TestSize.Level3)
{
    std::cout << "==========[test log]Performance: HDI_cameraDevice->DisableResult's average time." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    Test_->Open();
    // 获取设备当前支持的参数tag
    std::cout << "==========[test log]Check hdi_device: 1. Get the tags..." << std::endl;
    std::vector<Camera::MetaType> results_original;
    Test_->rc = Test_->cameraDevice->GetEnabledResults(results_original);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 禁用这个tag
    std::vector<Camera::MetaType> disable_tag;
    disable_tag.push_back(results_original[1]);
    for (int i = 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->cameraDevice->DisableResult(disable_tag);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_cameraDevice->DisableResult's average time: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_cameraDevice->DisableResult's average time: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: IsStreamsSupported
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0120, TestSize.Level0)
{
    std::cout << "==========[test log]Performance: HDI_IsStreamsSupported's average time." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
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
    int64_t colorGains[4] = {0};
    modeSetting->addEntry(OHOS_SENSOR_COLOR_CORRECTION_GAINS, &colorGains, 4);
    int64_t expoTime = 0;
    modeSetting->addEntry(OHOS_SENSOR_EXPOSURE_TIME, &expoTime, 1);
    // 配置流信息
    Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
    Test_->streamInfo->streamId_ = 1001; // 1001:流id
    Test_->streamInfo->datasapce_ = 8;
    Test_->streamInfo->intent_ = Camera::PREVIEW;
    Test_->streamInfo->width_ = 640; // 640:流宽度
    Test_->streamInfo->height_ = 480; // 480:流高度
    Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
    Test_->streamInfo->tunneledMode_ = 5;
    std::shared_ptr<OHOS::Camera::Test::StreamConsumer> consumer =
      std::make_shared<OHOS::Camera::Test::StreamConsumer>();
    Test_->streamInfo->bufferQueue_ = consumer->CreateProducer([this](void* addr, uint32_t size) {
        Test_->SaveYUV("preview", addr, size);
    });
    Test_->streamInfo->bufferQueue_->SetQueueSize(8);
    Test_->consumerMap_[Test_->streamInfo->intent_] = consumer;
    Camera::StreamSupportType pType;
    for (int i = 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->IsStreamsSupported(NORMAL, modeSetting, {Test_->streamInfo}, pType);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_IsStreamsSupported's average time: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_IsStreamsSupported's average time: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: HDI_CreateStreams's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0130, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: HDI_CreateStreams's time consuming." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    for (int i = 0; i < Times; i ++) {
        // 打开相机
        Test_->Open();
        // 创建并获取streamOperator信息
        Test_->streamOperatorCallback = new StreamOperatorCallback();
        Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
        EXPECT_EQ(false, Test_->rc != Camera::NO_ERROR || Test_->streamOperator == nullptr);
        // 创建数据流
        Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
        Test_->streamInfo->streamId_ = 1001;
        Test_->streamInfo->width_ = 1920;
        Test_->streamInfo->height_ = 1080;
        Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
        Test_->streamInfo->datasapce_ = 10;
        Test_->streamInfo->intent_ = Camera::PREVIEW;
        Test_->streamInfo->tunneledMode_ = 5;
        std::shared_ptr<OHOS::Camera::Test::StreamConsumer> preview_consumer =
            std::make_shared<OHOS::Camera::Test::StreamConsumer>();
        Test_->streamInfo->bufferQueue_ = preview_consumer->CreateProducer([this](void* addr, uint32_t size) {
            Test_->SaveYUV("preview", addr, size);
        });
        Test_->streamInfo->bufferQueue_->SetQueueSize(8);
        Test_->consumerMap_[Camera::PREVIEW] = preview_consumer;
        std::vector<std::shared_ptr<Camera::StreamInfo>>().swap(Test_->streamInfos);
        Test_->streamInfos.push_back(Test_->streamInfo);
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->CreateStreams(Test_->streamInfos);
        gettimeofday(&end, NULL);
        std::cout << "streamOperator->CreateStreams's rc " << Test_->rc << std::endl;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        // 释放流
        Test_->rc = Test_->streamOperator->ReleaseStreams({1001});
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_CreateStreams's average time consuming: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_CreateStreams's average time consuming: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: HDI_ReleaseStreams's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0160, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: HDI_ReleaseStreams's time consuming."<< std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    for (int i = 0; i < Times; i ++) {
        std::cout  << "Times =" << i << std::endl;
        // 打开相机
        Test_->Open();
        // 启动流
        Test_->intents = {Camera::PREVIEW};
        Test_->StartStream(Test_->intents);
        // 释放流
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->ReleaseStreams({Test_->streamId_preview});
        gettimeofday(&end, NULL);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_ReleaseStreams's average time consuming: ";
    std::cout  << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_ReleaseStreams's average time consuming: ";
    writeIntoFile  << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: CommitStreams's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0170, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: CommitStreams's time consuming." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    for (int i = 0; i < Times; i ++) {
        // 打开相机
        Test_->Open();
        // 创建并获取streamOperator信息
        Test_->streamOperatorCallback = new StreamOperatorCallback();
        Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
        EXPECT_EQ(false, Test_->rc != Camera::NO_ERROR || Test_->streamOperator == nullptr);
        // 创建数据流
        Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
        Test_->streamInfo->streamId_ = 1001;
        Test_->streamInfo->width_ = 1920;
        Test_->streamInfo->height_ = 1080;
        Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
        Test_->streamInfo->datasapce_ = 10;
        Test_->streamInfo->intent_ = Camera::PREVIEW;
        Test_->streamInfo->tunneledMode_ = 5;
        std::shared_ptr<OHOS::Camera::Test::StreamConsumer> preview_consumer =
        std::make_shared<OHOS::Camera::Test::StreamConsumer>();
        Test_->streamInfo->bufferQueue_ = preview_consumer->CreateProducer([this](void* addr, uint32_t size) {
            Test_->SaveYUV("preview", addr, size);
        });
        Test_->streamInfo->bufferQueue_->SetQueueSize(8);
        Test_->consumerMap_[Camera::PREVIEW] = preview_consumer;
        std::vector<std::shared_ptr<Camera::StreamInfo>>().swap(Test_->streamInfos);
        Test_->streamInfos.push_back(Test_->streamInfo);
        Test_->rc = Test_->streamOperator->CreateStreams(Test_->streamInfos);
        std::cout << "streamOperator->CreateStreams's rc " << Test_->rc << std::endl;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        // 配流起流
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->CommitStreams(Camera::NORMAL, Test_->ability);
        gettimeofday(&end, NULL);
        std::cout << "streamOperator->CommitStreams's rc " << Test_->rc << std::endl;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        // 释放流
        Test_->rc = Test_->streamOperator->ReleaseStreams({1001});
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: CommitStreams's average time consuming: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: CommitStreams's average time consuming: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}

/**
  * @tc.name: GetStreamAttributes
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0180, TestSize.Level3)
{
    std::cout << "==========[test log]Check Performance: HDI_GetStreamAttributes's average time." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    Test_->Open();
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    std::vector<std::shared_ptr<Camera::StreamAttribute>> attributes;
    for (int i = 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->GetStreamAttributes(attributes);
        gettimeofday(&end, NULL);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_GetStreamAttributes's average time: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_GetStreamAttributes's average time: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
    // 释放流
    Test_->rc = Test_->streamOperator->ReleaseStreams({Test_->streamId_preview});
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    std::cout << "streamOperator->ReleaseStreams's rc " << Test_->rc << std::endl;
}

/**
  * @tc.name: HDI_Capture's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0190, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: HDI_Capture's average time consuming." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    // 打开相机
    Test_->Open();
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 获取预览图
    int captureId = 2001;
    Test_->captureInfo = std::make_shared<Camera::CaptureInfo>();
    Test_->captureInfo->streamIds_ = {Test_->streamId_preview};
    Test_->captureInfo->captureSetting_ = Test_->ability;
    Test_->captureInfo->enableShutterCallback_ = true;
    for (int i = 0; i < Times; i++) {
        std::cout  << "Times =" << i << std::endl;
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->Capture(captureId, Test_->captureInfo, false);
        captureId++;
        gettimeofday(&end, NULL);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_Capture's average time consuming: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_Capture's average time consuming: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
    // 后处理
    Test_->captureIds = {};
    Test_->streamIds = {Test_->streamId_preview};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: HDI_CancelCapture's time consuming.
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0200, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: HDI_CancelCapture's average time consuming." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    // 打开相机
    Test_->Open();
    // 启动流
    Test_->intents = {Camera::PREVIEW};
    Test_->StartStream(Test_->intents);
    // 获取预览图
    int captureId = 2001;
    Test_->captureInfo = std::make_shared<Camera::CaptureInfo>();
    Test_->captureInfo->streamIds_ = {Test_->streamId_preview};
    Test_->captureInfo->captureSetting_ = Test_->ability;
    Test_->captureInfo->enableShutterCallback_ = true;
    for (int i = 0; i < Times; i++) {
        std::cout  << "Times =" << i << std::endl;
        Test_->rc = Test_->streamOperator->Capture(captureId, Test_->captureInfo, true);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->CancelCapture(captureId);
        gettimeofday(&end, NULL);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_CancelCapture's average time consuming: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_CancelCapture's average time consuming: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
    // 后处理
    Test_->captureIds = {};
    Test_->streamIds = {Test_->streamId_preview};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: AttachBufferQueue
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0210, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: HDI_AttachBufferQueue's average time consuming." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    // 打开相机
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
    for (int i = 0; i < Times; i++) {
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->AttachBufferQueue(Test_->streamInfo->streamId_, producer);
        gettimeofday(&end, NULL);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
        Test_->rc = Test_->streamOperator->DetachBufferQueue(Test_->streamInfo->streamId_);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_AttachBufferQueue's average time consuming: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_AttachBufferQueue's average time consuming: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
    // 释放流
    Test_->captureIds = {};
    Test_->streamIds = {1001};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: DetachBufferQueue
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0220, TestSize.Level3)
{
    std::cout << "==========[test log] Performance: HDI_DetachBufferQueue's average time consuming." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    // 打开相机
    Test_->Open();
    Test_->streamOperatorCallback = new StreamOperatorCallback();
    Test_->rc = Test_->cameraDevice->GetStreamOperator(Test_->streamOperatorCallback, Test_->streamOperator);
    EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
    // 创建数据流
    Test_->streamInfo = std::make_shared<Camera::StreamInfo>();
    Test_->streamInfo->intent_ = Camera::PREVIEW;
    Test_->streamInfo->streamId_ = 1001;
    Test_->streamInfo->width_ = 640;
    Test_->streamInfo->height_ = 480;
    Test_->streamInfo->format_ = PIXEL_FMT_YCRCB_420_SP;
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
    for (int i = 0; i < Times; i++) {
        Test_->rc = Test_->streamOperator->AttachBufferQueue(Test_->streamInfo->streamId_, producer);
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->DetachBufferQueue(Test_->streamInfo->streamId_);
        gettimeofday(&end, NULL);
        EXPECT_EQ(Test_->rc, Camera::NO_ERROR);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
    }
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_DetachBufferQueue's average time consuming: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_DetachBufferQueue's average time consuming: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
    // 释放流
    Test_->captureIds = {};
    Test_->streamIds = {1001};
    Test_->StopStream(Test_->captureIds, Test_->streamIds);
}

/**
  * @tc.name: ChangeToOfflineStream
  * @tc.desc: the average time for 1000 times.
  * @tc.size: MediumTest
  * @tc.type: Function
  */
HWTEST_F(PerformanceTest, Camera_Performance_Hdi_0230, TestSize.Level0)
{
    std::cout << "==========[test log] Performance: HDI_ChangeToOfflineStream's average time consuming." << std::endl;
    struct timeval start;
    struct timeval end;
    float time_use;
    float totle_time_use = 0;
    writeIntoFile.open("TimeConsuming.txt", ios::app);
    for (int i = 0; i < Times; i ++) {
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
        gettimeofday(&start, NULL);
        Test_->rc = Test_->streamOperator->ChangeToOfflineStream(
            {Test_->streamId_capture}, Test_->offlineStreamOperatorCallback, Test_->offlineStreamOperator);
        gettimeofday(&end, NULL);
        ASSERT_EQ(Test_->rc, Camera::NO_ERROR);
        time_use = calTime(start, end);
        totle_time_use = totle_time_use + time_use;
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
    float avrg_time = totle_time_use / Times;
    std::cout << "==========[test log] Performance: HDI_ChangeToOfflineStream's average time consuming: ";
    std::cout << avrg_time << "us. " << std::endl;
    writeIntoFile << "==========[test log] Performance: HDI_ChangeToOfflineStream's average time consuming: ";
    writeIntoFile << avrg_time << "us. " << std::endl;
    writeIntoFile.close();
}
