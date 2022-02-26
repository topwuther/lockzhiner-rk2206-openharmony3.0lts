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
#include "ActsAudioRecoederTest.h"
#include "mediatest_log.h"

using namespace std;
using namespace OHOS::Media;
using namespace testing::ext;

static AudioRecorderConfig g_audioRecorderConfig = {
    0,
    48000,
    2,
    60,
    "/data/recorder",
    48000,
    AAC_LC,
    AUDIO_MIC,
    FORMAT_M4A,
};

void TestAudioRecorderCallback::OnError(int32_t errorType, int32_t errorCode)
{
    MEDIA_INFO_LOG("OnError errorType is %d, errorCode is %d", errorType, errorCode);
}

void TestAudioRecorderCallback::OnInfo(int32_t type, int32_t extra)
{
    MEDIA_INFO_LOG("OnInfo type is %d, extra is %d", type, extra);
}

void ActsAudioRecorderTest::SetUpTestCase(void)
{
    MEDIA_INFO_LOG("ActsAudioRecorderTest::SetUpTestCase");
}

void ActsAudioRecorderTest::TearDownTestCase(void)
{
    MEDIA_INFO_LOG("ActsAudioRecorderTest::TearDownTestCase");
}

void ActsAudioRecorderTest::SetUp(void)
{
    MEDIA_INFO_LOG("ActsAudioRecorderTest::SetUp");
}

void ActsAudioRecorderTest::TearDown(void)
{
    MEDIA_INFO_LOG("ActsAudioRecorderTest::TearDown");
}

void ActsAudioRecorderTest::ConfigAudioRecorder(std::shared_ptr<Recorder> recorder, AudioRecorderConfig audioConfig)
{
    std::shared_ptr<TestAudioRecorderCallback> testCallback = std::make_shared<TestAudioRecorderCallback>();
    ASSERT_NE(nullptr, testCallback);
    EXPECT_EQ(0, recorder->SetAudioSource(audioConfig.inputSource, audioConfig.audioSourceId));
    EXPECT_EQ(0, recorder->SetOutputFormat(audioConfig.outPutFormat));
    EXPECT_EQ(0, recorder->SetAudioEncoder(audioConfig.audioSourceId, audioConfig.audioFormat));
    EXPECT_EQ(0, recorder->SetAudioSampleRate(audioConfig.audioSourceId, audioConfig.sampleRate));
    EXPECT_EQ(0, recorder->SetAudioChannels(audioConfig.audioSourceId, audioConfig.channelCount));
    EXPECT_EQ(0, recorder->SetAudioEncodingBitRate(audioConfig.audioSourceId, audioConfig.audioEncodingBitRate));
    EXPECT_EQ(0, recorder->SetMaxDuration(audioConfig.duration));
    EXPECT_EQ(0, recorder->SetOutputPath(audioConfig.outPath));
    EXPECT_EQ(0, recorder->SetRecorderCallback(testCallback));
}

void ActsAudioRecorderTest::AudioRecorderStart(std::shared_ptr<Recorder> recorder)
{
    EXPECT_EQ(0, recorder->Prepare());
    EXPECT_EQ(0, recorder->Start());
    sleep(2);
    EXPECT_EQ(0, recorder->Stop(false));
    EXPECT_EQ(0, recorder->Reset());
    EXPECT_EQ(0, recorder->Release());
}

/* *
    * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0100
    * @tc.name      : 01.set audio format to AAC
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsAudioRecorderTest, SUB_MEDIA_RECORDER_Codec_AAC_Function_0100, Function | MediumTest | Level0)
{
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsAudioRecorderTest::ConfigAudioRecorder(recorder, g_audioRecorderConfig);
    ActsAudioRecorderTest::AudioRecorderStart(recorder);
}

/* *
    * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0300
    * @tc.name      : 03.Set different samplerate 8-96KHz
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsAudioRecorderTest, SUB_MEDIA_RECORDER_Codec_AAC_Function_0300, Function | MediumTest | Level0)
{
    for (int i = 8000; i < 96000; i = i + 10000) {
        g_audioRecorderConfig.sampleRate = i;
        g_audioRecorderConfig.outPath = "/data/recorder";
        std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
        ASSERT_NE(nullptr, recorder);
        ActsAudioRecorderTest::ConfigAudioRecorder(recorder, g_audioRecorderConfig);
        ActsAudioRecorderTest::AudioRecorderStart(recorder);
    }
}

/* *
    * @tc.number    : SUB_MEDIA_RECORDER_Format_MP4_Function_05_0200
    * @tc.name      : 02.Set outPutFormat to FORMAT_MPEG_4
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsAudioRecorderTest, SUB_MEDIA_RECORDER_Format_MP4_Function_05_0200, Function | MediumTest | Level0)
{
    g_audioRecorderConfig.outPutFormat = FORMAT_MPEG_4;
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsAudioRecorderTest::ConfigAudioRecorder(recorder, g_audioRecorderConfig);
    ActsAudioRecorderTest::AudioRecorderStart(recorder);
}

/* *
    * @tc.number    : SUB_MEDIA_RECORDER_Function_09_01_0100
    * @tc.name      : 001.Stopping Recording in DRAIN Mode When Recording Pure Audio Files
    * @tc.desc      : The test case is successfully executed.
                      The video file can be played properly.
*/
HWTEST_F(ActsAudioRecorderTest, SUB_MEDIA_RECORDER_Function_09_01_0100, Function | MediumTest | Level0)
{
    std::shared_ptr<Recorder> recorder = RecorderFactory::CreateRecorder();
    ASSERT_NE(nullptr, recorder);
    ActsAudioRecorderTest::ConfigAudioRecorder(recorder, g_audioRecorderConfig);
    EXPECT_EQ(0, recorder->Prepare());
    EXPECT_EQ(0, recorder->Start());
    sleep(5);
    EXPECT_EQ(0, recorder->Stop(true));
    EXPECT_EQ(0, recorder->Release());
}
