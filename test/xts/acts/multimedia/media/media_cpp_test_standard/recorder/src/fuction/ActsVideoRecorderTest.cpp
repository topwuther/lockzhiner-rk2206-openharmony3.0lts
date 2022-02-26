/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ActsVideoRecorderTest.h"
#include "mediatest_log.h"

using namespace std;
using namespace OHOS::Media;
using namespace testing::ext;

static VideoRecorderConfig g_videoRecorderConfig = {
    0,
    0,
    48000,
    2,
    60,
    1920,
    1080,
    30,
    48000,
    48000,
    30,
    "/data/recorder",
    AAC_LC,
    AUDIO_MIC,
    FORMAT_MPEG_4,
    VIDEO_SOURCE_SURFACE_ES,
    H264,
};

static int g_videoWidth[3] = {1920, 1280, 720};
static int g_videoHeight[3] = {1080, 720, 480};

void TestVideoRecorderCallback::OnError(int32_t errorType, int32_t errorCode)
{
    MEDIA_INFO_LOG("OnError errorType is %d, errorCode is %d", errorType, errorCode);
}

void TestVideoRecorderCallback::OnInfo(int32_t type, int32_t extra)
{
    MEDIA_INFO_LOG("OnInfo type is %d, extra is %d", type, extra);
}

void ActsVideoRecorderTest::SetUpTestCase(void)
{
    MEDIA_INFO_LOG("ActsAudioRecorderTest::SetUpTestCase");
}

void ActsVideoRecorderTest::TearDownTestCase(void)
{
    MEDIA_INFO_LOG("ActsAudioRecorderTest::TearDownTestCase");
}

void ActsVideoRecorderTest::SetUp(void)
{
    MEDIA_INFO_LOG("ActsAudioRecorderTest::SetUp");
}

void ActsVideoRecorderTest::TearDown(void)
{
    MEDIA_INFO_LOG("ActsAudioRecorderTest::TearDown");
}

void ActsVideoRecorderTest::ConfigvideoRecorder(std::shared_ptr<Recorder> recorder,
                                                  VideoRecorderConfig videoConfig, bool isAudio)
{
    std::shared_ptr<TestVideoRecorderCallback> testCallback = std::make_shared<TestVideoRecorderCallback>();
    ASSERT_NE(nullptr, testCallback);
    EXPECT_EQ(0, recorder->SetVideoSource(videoConfig.vSource, videoConfig.videoSourceId));
    if (isAudio) {
        EXPECT_EQ(0, recorder->SetAudioSource(videoConfig.aSource, videoConfig.audioSourceId));
    }
    EXPECT_EQ(0, recorder->SetOutputFormat(videoConfig.outPutFormat));
    EXPECT_EQ(0, recorder->SetVideoEncoder(videoConfig.videoSourceId, videoConfig.videoFormat));
    EXPECT_EQ(0, recorder->SetVideoSize(videoConfig.videoSourceId, videoConfig.width, videoConfig.height));
    EXPECT_EQ(0, recorder->SetVideoFrameRate(videoConfig.videoSourceId, videoConfig.frameRate));
    EXPECT_EQ(0, recorder->SetVideoEncodingBitRate(videoConfig.videoSourceId, videoConfig.videoEncodingBitRate));
    EXPECT_EQ(0, recorder->SetCaptureRate(videoConfig.videoSourceId, videoConfig.captureFps));
    if (isAudio) {
        EXPECT_EQ(0, recorder->SetAudioEncoder(videoConfig.audioSourceId, videoConfig.audioFormat));
        EXPECT_EQ(0, recorder->SetAudioSampleRate(videoConfig.audioSourceId, videoConfig.sampleRate));
        EXPECT_EQ(0, recorder->SetAudioChannels(videoConfig.audioSourceId, videoConfig.channelCount));
        EXPECT_EQ(0, recorder->SetAudioEncodingBitRate(videoConfig.audioSourceId, videoConfig.audioEncodingBitRate));
    }
    EXPECT_EQ(0, recorder->SetMaxDuration(videoConfig.duration));
    EXPECT_EQ(0, recorder->SetOutputPath(videoConfig.outPath));
    EXPECT_EQ(0, recorder->SetRecorderCallback(testCallback));
}

void ActsVideoRecorderTest::VideoRecorderStart(std::shared_ptr<Recorder> recorder)
{
    EXPECT_EQ(0, recorder->Prepare());
    EXPECT_EQ(0, recorder->Start());
    sleep(5);
    EXPECT_EQ(0, recorder->Stop(false));
    EXPECT_EQ(0, recorder->Reset());
    EXPECT_EQ(0, recorder->Release());
}

/**
    * @tc.number    : SUB_MEDIA_RECORDER_Codec_H264_Function_03_0100
    * @tc.name      : 01.Local recording H264
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsVideoRecorderTest, SUB_MEDIA_RECORDER_Codec_H264_Function_03_0100, Function | MediumTest | Level0)
{
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsVideoRecorderTest::ConfigvideoRecorder(recorder, g_videoRecorderConfig, true);
    ActsVideoRecorderTest::VideoRecorderStart(recorder);
}

/**
    * @tc.number    : SUB_MEDIA_RECORDER_Codec_H264_Function_03_0200
    * @tc.name      : 02.Local recording H264_1080P/720P/480P
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsVideoRecorderTest, SUB_MEDIA_RECORDER_Codec_H264_Function_03_0200, Function | MediumTest | Level0)
{
    for (int i = 0; i < 3; i++) {
        g_videoRecorderConfig.width = g_videoWidth[i];
        g_videoRecorderConfig.height = g_videoHeight[i];
        std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
        ASSERT_NE(nullptr, recorder);
        ActsVideoRecorderTest::ConfigvideoRecorder(recorder, g_videoRecorderConfig, true);
        ActsVideoRecorderTest::VideoRecorderStart(recorder);
    }
}

/**
    * @tc.number    : SUB_MEDIA_RECORDER_Codec_H264_Function_03_0400
    * @tc.name      : 04.Local recording H264_BitRate
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsVideoRecorderTest, SUB_MEDIA_RECORDER_Codec_H264_Function_03_0400, Function | MediumTest | Level0)
{
    g_videoRecorderConfig.audioEncodingBitRate = 96000;
    g_videoRecorderConfig.videoEncodingBitRate = 96000;
    g_videoRecorderConfig.sampleRate = 96000;
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsVideoRecorderTest::ConfigvideoRecorder(recorder, g_videoRecorderConfig, true);
    ActsVideoRecorderTest::VideoRecorderStart(recorder);
}

/**
    * @tc.number    : SUB_MEDIA_RECORDER_Format_MP4_Function_05_0100
    * @tc.name      : 01.Local recording H264
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsVideoRecorderTest, SUB_MEDIA_RECORDER_Format_MP4_Function_05_0100, Function | MediumTest | Level0)
{
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsVideoRecorderTest::ConfigvideoRecorder(recorder, g_videoRecorderConfig, false);
    ActsVideoRecorderTest::VideoRecorderStart(recorder);
}

/**
    * @tc.number    : SUB_MEDIA_RECORDER_Format_MP4_Function_05_0300
    * @tc.name      : 03.Local recording H264 and AAC
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsVideoRecorderTest, SUB_MEDIA_RECORDER_Format_MP4_Function_05_0300, Function | MediumTest | Level0)
{
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsVideoRecorderTest::ConfigvideoRecorder(recorder, g_videoRecorderConfig, true);
    ActsVideoRecorderTest::VideoRecorderStart(recorder);
}

/**
    * @tc.number    : SUB_MEDIA_RECORDER_Function_06_0100
    * @tc.name      : 001.Start
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsVideoRecorderTest, SUB_MEDIA_RECORDER_Function_06_0100, Function | MediumTest | Level0)
{
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsVideoRecorderTest::ConfigvideoRecorder(recorder, g_videoRecorderConfig, true);
    EXPECT_EQ(0, recorder->Prepare());
    EXPECT_EQ(0, recorder->Start());
    sleep(5);
    EXPECT_EQ(0, recorder->Release());
}

/**
    * @tc.number    : SUB_MEDIA_RECORDER_Function_06_0400
    * @tc.name      : 004.Start-Stop
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsVideoRecorderTest, SUB_MEDIA_RECORDER_Function_06_0400, Function | MediumTest | Level0)
{
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsVideoRecorderTest::ConfigvideoRecorder(recorder, g_videoRecorderConfig, true);
    EXPECT_EQ(0, recorder->Prepare());
    EXPECT_EQ(0, recorder->Start());
    sleep(5);
    EXPECT_EQ(0, recorder->Stop(false));
    EXPECT_EQ(0, recorder->Release());
}

/**
    * @tc.number    : SUB_MEDIA_RECORDER_Function_06_0500
    * @tc.name      : 005.Start-Reset
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsVideoRecorderTest, SUB_MEDIA_RECORDER_Function_06_0500, Function | MediumTest | Level0)
{
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsVideoRecorderTest::ConfigvideoRecorder(recorder, g_videoRecorderConfig, true);
    EXPECT_EQ(0, recorder->Prepare());
    EXPECT_EQ(0, recorder->Start());
    sleep(5);
    EXPECT_EQ(0, recorder->Reset());
    EXPECT_EQ(0, recorder->Release());
}