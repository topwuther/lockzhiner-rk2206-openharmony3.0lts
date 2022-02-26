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

#include <unistd.h>
#include "camera_kit.h"
#include "recorder.h"
#include "gtest/gtest.h"
#include "ActsTestMediaUtils.h"

static int32_t g_recoderSourceMaxCount = 4; // max recorder source setting

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;

/* *
 * Media Recorder test
 */
class ActsMediaRecorderNDKTest : public testing::Test {
protected:
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
};

/* *
 * @tc.setup:This is before test class
 */
void ActsMediaRecorderNDKTest::SetUpTestCase(void)
{
    cout << "SetUpTestCase" << endl;
}

/* *
 * @tc.setup:This is after test class
 */
void ActsMediaRecorderNDKTest::TearDownTestCase(void)
{
    cout << "TearDownTestCase" << endl;
}

/* *
 * @tc.setup:This is before test case
 */
void ActsMediaRecorderNDKTest::SetUp()
{
    cout << "SetUp" << endl;
}

/* *
 * @tc.setup:This is after test case
 */
void ActsMediaRecorderNDKTest::TearDown()
{
    sleep(1);
    cout << "TearDown" << endl;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0100
 * @tc.name      : Set Video Source 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSource01, Function | MediumTest | Level0)
{
    cout << "Test_SetVideoSource01 starting..." << endl;
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    cout << "Test_SetVideoSource01 ending..." << endl;
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0110
 * @tc.name      : Set Video Source 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSource02, Function | MediumTest | Level0)
{
    cout << "Test_SetVideoSource01 starting..." << endl;
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_YUV, sourceId);
    EXPECT_EQ(SUCCESS, ret);
    cout << "Test_SetVideoSource01 ending..." << endl;
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0200
 * @tc.name      : Set Video Source 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSource03, Function | MediumTest | Level0)
{
    cout << "Test_SetVideoSource03 starting..." << endl;
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_RGB, sourceId);
    EXPECT_EQ(RET_OK, ret);
    cout << "Test_SetVideoSource03 ending..." << endl;
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0300
 * @tc.name      : Set Video Source 04
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSource04, Function | MediumTest | Level0)
{
    cout << "Test_SetVideoSource01 starting..." << endl;
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_BUTT, sourceId);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    cout << "Test_SetVideoSource01 ending..." << endl;
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0400
 * @tc.name      : Set Video Source 05
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSource05, Function | MediumTest | Level0)
{
    cout << "Test_SetVideoSource01 starting..." << endl;
    Recorder *recorder = new Recorder();
    for (int32_t i = 0; i < g_recoderSourceMaxCount; i++) {
        int32_t temp = i;
        int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, temp);
        EXPECT_EQ(RET_OK, ret);
    }
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, g_recoderSourceMaxCount);
    EXPECT_EQ(ERR_NOFREE_CHANNEL, ret);
    cout << "Test_SetVideoSource01 ending..." << endl;
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0500
 * @tc.name      : Set Video Encoder 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoEncoder01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetVideoEncoder(sourceId, HEVC);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0600
 * @tc.name      : Set Video Encoder 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoEncoder02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 10000;
    int32_t ret = recorder->SetVideoEncoder(sourceId, HEVC);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0700
 * @tc.name      : Set Video Encoder 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoEncoder03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetVideoEncoder(sourceId, VIDEO_DEFAULT);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0800
 * @tc.name      : Set Video Encoder 04
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoEncoder04, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetVideoEncoder(sourceId, H264);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_0900
 * @tc.name      : Set Video Encoder 05
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoEncoder05, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_BUTT, sourceId);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    ret = recorder->SetVideoEncoder(sourceId, HEVC);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1000
 * @tc.name      : Set Video Size 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSize01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t width = 1920;
    int32_t height = 1080;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetVideoSize(sourceId, width, height);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1100
 * @tc.name      : Set Video Size 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSize02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t width = -1920;
    int32_t height = -1080;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetVideoSize(sourceId, width, height);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1200
 * @tc.name      : Set Video Size 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSize03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 10000;
    int32_t width = 1920;
    int32_t height = 1080;
    int32_t ret = recorder->SetVideoSize(sourceId, width, height);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1300
 * @tc.name      : Set Video Size 04
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSize04, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t width = 1280;
    int32_t height = 720;
    int32_t ret = recorder->SetVideoSize(sourceId, width, height);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1400
 * @tc.name      : Set Video Size 05
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoSize05, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t ret = recorder->SetVideoSize(sourceId, width, height);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1500
 * @tc.name      : Set Video Frame Rate 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoFrameRate01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t frameRate = 30;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetVideoFrameRate(sourceId, frameRate);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1600
 * @tc.name      : Set Video Frame Rate 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoFrameRate02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t frameRate = -1;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetVideoFrameRate(sourceId, frameRate);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1700
 * @tc.name      : Set Video Frame Rate 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoFrameRate03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 10000;
    int32_t frameRate = 30;
    int32_t ret = recorder->SetVideoFrameRate(sourceId, frameRate);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1800
 * @tc.name      : Set Video Encoding BitRate 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoEncodingBitRate01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t rate = 4096;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetVideoEncodingBitRate(sourceId, rate);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_1900
 * @tc.name      : Set Video Encoding BitRate 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoEncodingBitRate02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t rate = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetVideoEncodingBitRate(sourceId, rate);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2000
 * @tc.name      : Set Video Encoding BitRate 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetVideoEncodingBitRate03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 10000;
    int32_t rate = 4096;
    int32_t ret = recorder->SetVideoEncodingBitRate(sourceId, rate);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2100
 * @tc.name      : Set Capture Rate 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetCaptureRate01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    double fps = 30.0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetCaptureRate(sourceId, fps);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2200
 * @tc.name      : Set Capture Rate 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetCaptureRate02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 10000;
    double fps = 30.0;
    int32_t ret = recorder->SetCaptureRate(sourceId, fps);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2300
 * @tc.name      : Set Capture Rate 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetCaptureRate03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    double fps = -30.0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetCaptureRate(sourceId, fps);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2400
 * @tc.name      : Set Audio Source 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioSource01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2500
 * @tc.name      : Set Audio Source 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioSource02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t ret = recorder->SetAudioSource(AUDIO_SOURCE_INVALID, audioSourceId);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2600
 * @tc.name      : Set Audio Source 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioSource03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    for (int32_t i = 0; i < g_recoderSourceMaxCount; i++) {
        int32_t tempSourceId = i;
        int32_t ret = recorder->SetAudioSource(AUDIO_MIC, tempSourceId);
        cout << i << endl;
        EXPECT_EQ(RET_OK, ret);
    }
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, g_recoderSourceMaxCount) ;
    EXPECT_EQ(ERR_NOFREE_CHANNEL, ret)  ;
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2700
 * @tc.name      : Set Audio Encoder 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncoder01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioEncoder(audioSourceId, AAC_LC);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2800
 * @tc.name      : Set Audio Encoder 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncoder02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioEncoder(audioSourceId, AUDIO_DEFAULT);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_2900
 * @tc.name      : Set Audio Encoder 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncoder03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 10000;
    int32_t ret = recorder->SetAudioEncoder(audioSourceId, AAC_LC);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3000
 * @tc.name      : Set Audio Encoder 04
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncoder04, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioEncoder(audioSourceId, AAC_HE_V1);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3100
 * @tc.name      : Set Audio Encoder 05
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncoder05, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioEncoder(audioSourceId, AAC_HE_V2);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3200
 * @tc.name      : Set Audio Encoder 06
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncoder06, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioEncoder(audioSourceId, AAC_LD);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3300
 * @tc.name      : Set Audio Encoder 07
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncoder07, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioEncoder(audioSourceId, AAC_ELD);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3400
 * @tc.name      : Set Audio Sample Rate 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioSampleRate01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t sampleRate = 48000;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioSampleRate(audioSourceId, sampleRate);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3500
 * @tc.name      : Set Audio Sample Rate 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioSamplingRate02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 10000;
    int32_t sampleRate = 48000;
    int32_t ret = recorder->SetAudioSampleRate(audioSourceId, sampleRate);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3600
 * @tc.name      : Set Audio Sample Rate 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioSampleRate03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t sampleRate = -48000;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioSampleRate(audioSourceId, sampleRate);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3700
 * @tc.name      : Set Audio Channels 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioChannels01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t channelCount = 1;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioChannels(audioSourceId, channelCount);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3800
 * @tc.name      : Set Audio Channels 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioChannels02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t channelCount = 2;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioChannels(audioSourceId, channelCount);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_3900
 * @tc.name      : Set Audio Channels 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioChannels03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t channelCount = 0;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioChannels(audioSourceId, channelCount);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4000
 * @tc.name      : Set Audio Channels 04
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioChannels04, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t channelCount = 1000;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioChannels(audioSourceId, channelCount);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4100
 * @tc.name      : Set Audio Channels 05
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioChannels05, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 100000;
    int32_t channelCount = 1;
    int32_t ret = recorder->SetAudioChannels(audioSourceId, channelCount);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4200
 * @tc.name      : Set Audio Encoding BitRate 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncodingBitRate01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t audioEncodingBitRate = 48000;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioEncodingBitRate(audioSourceId, audioEncodingBitRate);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4300
 * @tc.name      : Set Audio Encoding BitRate 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncodingBitRate02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 0;
    int32_t audioEncodingBitRate = -48000;
    int32_t ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioEncodingBitRate(audioSourceId, audioEncodingBitRate);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4400
 * @tc.name      : Set Audio Encoding BitRate 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetAudioEncodingBitRate03, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t audioSourceId = 10000;
    int32_t audioEncodingBitRate = 48000;
    int32_t ret = recorder->SetAudioEncodingBitRate(audioSourceId, audioEncodingBitRate);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4500
 * @tc.name      : Test Recorder Prepare 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_Prepare01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t audioSourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->SetAudioSource(AUDIO_MIC, audioSourceId);
    EXPECT_EQ(RET_OK, ret);
    ret = recorder->Prepare();
    EXPECT_NE(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4600
 * @tc.name      : Test Recorder GetSurface 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_GetSurface01, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 0;
    int32_t ret = recorder->SetVideoSource(VIDEO_SOURCE_SURFACE_ES, sourceId);
    EXPECT_EQ(RET_OK, ret);
    Surface *surface = recorder->GetSurface(sourceId).get();
    ret = (int32_t)(surface == nullptr);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4700
 * @tc.name      : Test Recorder GetSurface 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_GetSurface02, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t sourceId = 10000;
    Surface *surface = recorder->GetSurface(sourceId).get();
    EXPECT_EQ(nullptr, surface);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4800
 * @tc.name      : Test Recorder SetMaxDuration 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetMaxDuration, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetMaxDuration(1000);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_4900
 * @tc.name      : Test Recorder SetMaxDuration 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetMaxDuration2, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetMaxDuration(0);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5000
 * @tc.name      : Test Recorder SetMaxDuration 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetMaxDuration3, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetMaxDuration(-1);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5100
 * @tc.name      : Test Recorder SetMaxDuration 04
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetMaxDuration4, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetMaxDuration(4294967295);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5200
 * @tc.name      : Test Recorder SetMaxFileSize 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetMaxFileSize1, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetMaxFileSize(0);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5300
 * @tc.name      : Test Recorder SetMaxFileSize 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetMaxFileSize2, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetMaxFileSize(2000);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5400
 * @tc.name      : Test Recorder SetOutputFile 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetOutputFile1, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetOutputFile(-1);
    EXPECT_EQ(-5, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5500
 * @tc.name      : Test Recorder SetNextOutputFile 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetNextOutputFile1, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetNextOutputFile(-1);
    EXPECT_EQ(-5, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5600
 * @tc.name      : Test Recorder SetOutputPath 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetOutputPath1, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    const string path = "";
    int32_t ret = recorder->SetOutputPath(path);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5700
 * @tc.name      : Test Recorder SetOutputFormat 01
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetOutputFormat1, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetOutputFormat(FORMAT_DEFAULT);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5800
 * @tc.name      : Test Recorder SetOutputFormat 02
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetOutputFormat2, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetOutputFormat(FORMAT_MPEG_4);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}

/* *
 * @tc.number    : SUB_MEDIA_REC_PARAM_5900
 * @tc.name      : Test Recorder SetOutputFormat 03
 * @tc.desc      : [C- SOFTWARE -0200]
 */
HWTEST_F(ActsMediaRecorderNDKTest, Test_SetOutputFormat3, Function | MediumTest | Level0)
{
    Recorder *recorder = new Recorder();
    int32_t ret = recorder->SetOutputFormat(FORMAT_TS);
    EXPECT_EQ(RET_OK, ret);
    delete recorder;
    recorder = NULL;
}