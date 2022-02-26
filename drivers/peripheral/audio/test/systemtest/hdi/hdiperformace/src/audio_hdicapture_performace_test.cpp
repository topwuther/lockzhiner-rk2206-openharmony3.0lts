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

/**
 * @addtogroup Audio
 * @{
 *
 * @brief Test audio recording interface delayTime.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file audio_hdi_common.h
 *
 * @brief Declares APIs for operations related to the audio delayTime.
 *
 * @since 1.0
 * @version 1.0
 */

#include "audio_hdi_common.h"
#include "audio_hdicapture_performace_test.h"

using namespace std;
using namespace testing::ext;
using namespace HMOS::Audio;

namespace {
const string ADAPTER_NAME_USB = "usb";
const float COUNT = 1000;
const long LOWLATENCY = 10000;
const long NORMALLATENCY = 30000;
const long HIGHLATENCY = 60000;
const int BUFFER = 1024 * 4;

class AudioHdiCapturePerformaceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static TestAudioManager *(*GetAudioManager)();
    static void *handleSo;
#ifdef AUDIO_MPI_SO
    static int32_t (*SdkInit)();
    static void (*SdkExit)();
    static void *sdkSo;
#endif
};

TestAudioManager *(*AudioHdiCapturePerformaceTest::GetAudioManager)() = nullptr;
void *AudioHdiCapturePerformaceTest::handleSo = nullptr;
#ifdef AUDIO_MPI_SO
    int32_t (*AudioHdiCapturePerformaceTest::SdkInit)() = nullptr;
    void (*AudioHdiCapturePerformaceTest::SdkExit)() = nullptr;
    void *AudioHdiCapturePerformaceTest::sdkSo = nullptr;
#endif

void AudioHdiCapturePerformaceTest::SetUpTestCase(void)
{
#ifdef AUDIO_MPI_SO
    char sdkResolvedPath[] = "//system/lib/libhdi_audio_interface_lib_render.z.so";
    sdkSo = dlopen(sdkResolvedPath, RTLD_LAZY);
    if (sdkSo == nullptr) {
        return;
    }
    SdkInit = (int32_t (*)())(dlsym(sdkSo, "MpiSdkInit"));
    if (SdkInit == nullptr) {
        return;
    }
    SdkExit = (void (*)())(dlsym(sdkSo, "MpiSdkExit"));
    if (SdkExit == nullptr) {
        return;
    }
    SdkInit();
#endif
    handleSo = dlopen(RESOLVED_PATH.c_str(), RTLD_LAZY);
    if (handleSo == nullptr) {
        return;
    }
    GetAudioManager = (TestAudioManager *(*)())(dlsym(handleSo, FUNCTION_NAME.c_str()));
    if (GetAudioManager == nullptr) {
        return;
    }
}

void AudioHdiCapturePerformaceTest::TearDownTestCase(void)
{
#ifdef AUDIO_MPI_SO
    if (SdkExit != nullptr) {
        SdkExit();
    }
    if (sdkSo != nullptr) {
        dlclose(sdkSo);
        sdkSo = nullptr;
    }
    if (SdkInit != nullptr) {
        SdkInit = nullptr;
    }
    if (SdkExit != nullptr) {
        SdkExit = nullptr;
    }
#endif
    if (GetAudioManager != nullptr) {
        GetAudioManager = nullptr;
    }
}

void AudioHdiCapturePerformaceTest::SetUp(void) {}

void AudioHdiCapturePerformaceTest::TearDown(void) {}

/**
* @tc.name  the performace of AudioCreateCapture
* @tc.number  SUB_Audio_HDI_AudioCreateCapture_Performance_0001
* @tc.devDesc  tests the performace of AudioCreateCapture interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCreateCapture_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = GetLoadAdapter(*audiopara.manager, audiopara.portType, audiopara.adapterName,
                         &audiopara.adapter, audiopara.audioPort);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(audiopara.attrs);
    InitDevDesc(audiopara.devDesc, audiopara.audioPort.portId, audiopara.pins);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.adapter->CreateCapture(audiopara.adapter, &audiopara.devDesc, &audiopara.attrs,
                                               &audiopara.capture);
        gettimeofday(&audiopara.end, NULL);
        if (ret < 0 || audiopara.capture == nullptr) {
            audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
            ASSERT_EQ(HDF_SUCCESS, ret);
        }
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
        ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(HIGHLATENCY, audiopara.averageDelayTime);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioDestroyCapture
* @tc.number  SUB_Audio_HDI_AudioDestroyCapture_Performance_0001
* @tc.devDesc  tests the performace of AudioDestroyCapture interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioDestroyCapture_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    for (int i = 0; i < COUNT; ++i) {
        ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                                 &audiopara.capture);
        ASSERT_EQ(HDF_SUCCESS, ret);
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
        audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
}

/**
* @tc.name  the performace of AudioCaptureStart
* @tc.number  SUB_Audio_HDI_AudioCaptureStart_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureStart interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureStart_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    for (int i = 0; i < COUNT; ++i) {
        ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                                 &audiopara.capture);
        ASSERT_EQ(HDF_SUCCESS, ret);
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->control.Start((AudioHandle)audiopara.capture);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
        ret = StopAudio(audiopara);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
}
/**
* @tc.name  the performace of AudioCapturePause
* @tc.number  SUB_Audio_HDI_AudioCapturePause_Performance_0001
* @tc.devDesc  tests the performace of AudioCapturePause interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCapturePause_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = audiopara.capture->control.Start((AudioHandle)audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->control.Pause((AudioHandle)audiopara.capture);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = audiopara.capture->control.Resume((AudioHandle)audiopara.capture);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = StopAudio(audiopara);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
* @tc.name  the performace of AudioCaptureResume
* @tc.number  SUB_Audio_HDI_AudioCaptureResume_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureResume interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureResume_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = audiopara.capture->control.Start((AudioHandle)audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        ret = audiopara.capture->control.Pause((AudioHandle)audiopara.capture);
        EXPECT_EQ(HDF_SUCCESS, ret);
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->control.Resume((AudioHandle)audiopara.capture);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = StopAudio(audiopara);
    EXPECT_EQ(HDF_SUCCESS, ret);
}
/**
* @tc.name  the performace of AudioCaptureCaptureFrame
* @tc.number  SUB_Audio_HDI_AudioCaptureCaptureFrame_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureCaptureFrame interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureCaptureFrame_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0,
        .requestBytes = BUFFER_LENTH
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    audiopara.frame = (char *)calloc(1, BUFFER_LENTH);
    ASSERT_NE(nullptr, audiopara.frame);
    for (int i = 0; i < COUNT; ++i) {
        ret = GetLoadAdapter(*audiopara.manager, audiopara.portType, audiopara.adapterName, &(audiopara.adapter),
                             audiopara.audioPort);
        if (ret < 0 || audiopara.adapter == nullptr) {
            free(audiopara.frame);
            audiopara.frame = nullptr;
            ASSERT_EQ(HDF_SUCCESS, ret);
        }
        InitAttrs(audiopara.attrs);
        audiopara.attrs.silenceThreshold = BUFFER;
        InitDevDesc(audiopara.devDesc, (&audiopara.audioPort)->portId, audiopara.pins);
        ret = audiopara.adapter->CreateCapture(audiopara.adapter, &audiopara.devDesc, &audiopara.attrs,
                                               &audiopara.capture);
        if (ret < 0) {
            audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
            free(audiopara.frame);
            audiopara.frame = nullptr;
            ASSERT_EQ(HDF_SUCCESS, ret);
        }
        ret = audiopara.capture->control.Start((AudioHandle)audiopara.capture);
        EXPECT_EQ(HDF_SUCCESS, ret);
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->CaptureFrame(audiopara.capture, audiopara.frame, audiopara.requestBytes,
                                              &audiopara.replyBytes);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
        ret = StopAudio(audiopara);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(NORMALLATENCY, audiopara.averageDelayTime);
    free(audiopara.frame);
    audiopara.frame = nullptr;
}
/**
* @tc.name  the performace of AudioCaptureGetSampleAttributes
* @tc.number  SUB_Audio_HDI_AudioCaptureGetSampleAttributes_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureGetSampleAttributes interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureGetSampleAttributes_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(audiopara.attrs);
    ret = audiopara.capture->attr.SetSampleAttributes(audiopara.capture, &audiopara.attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->attr.GetSampleAttributes(audiopara.capture, &audiopara.attrsValue);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureSetMute
* @tc.number  SUB_Audio_HDI_AudioCaptureSetMute_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureSetMute interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureSetMute_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->volume.SetMute(audiopara.capture, false);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
        ret = audiopara.capture->volume.GetMute(audiopara.capture, &audiopara.character.getmute);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_FALSE(audiopara.character.getmute);
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureGetMute
* @tc.number  SUB_Audio_HDI_AudioCaptureGetMute_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureGetMute interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureGetMute_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = audiopara.capture->volume.SetMute(audiopara.capture, false);
    EXPECT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->volume.GetMute(audiopara.capture, &audiopara.character.getmute);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_FALSE(audiopara.character.getmute);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureSetVolume
* @tc.number  SUB_Audio_HDI_AudioCaptureSetVolume_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureSetVolume interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureSetVolume_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0,
        .character.setvolume = 0.7
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->volume.SetVolume(audiopara.capture, audiopara.character.setvolume);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = audiopara.capture->volume.GetVolume(audiopara.capture, &audiopara.character.getvolume);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(audiopara.character.setvolume, audiopara.character.getvolume);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureGetVolume
* @tc.number  SUB_Audio_HDI_AudioCaptureGetVolume_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureGetVolume interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureGetVolume_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0,
        .character.setvolume = 0.8
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = audiopara.capture->volume.SetVolume(audiopara.capture, audiopara.character.setvolume);
    EXPECT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->volume.GetVolume(audiopara.capture, &audiopara.character.getvolume);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(audiopara.character.setvolume, audiopara.character.getvolume);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureGetGain
* @tc.number  SUB_Audio_HDI_AudioCaptureGetGain_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureGetGain interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureGetGain_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0,
        .character.setgain = 7
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = audiopara.capture->volume.SetGain(audiopara.capture, audiopara.character.setgain);
    EXPECT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->volume.GetGain(audiopara.capture, &audiopara.character.getgain);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(audiopara.character.setgain, audiopara.character.getgain);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureSetGain
* @tc.number  SUB_Audio_HDI_AudioCaptureSetGain_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureSetGain interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureSetGain_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0,
        .character.setgain = 8
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->volume.SetGain(audiopara.capture, audiopara.character.setgain);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = audiopara.capture->volume.GetGain(audiopara.capture, &audiopara.character.getgain);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(audiopara.character.setgain, audiopara.character.getgain);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureGetCurrentChannelId
* @tc.number  SUB_Audio_HDI_AudioCaptureGetCurrentChannelId_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureGetCurrentChannelId interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureGetCurrentChannelId_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->attr.GetCurrentChannelId(audiopara.capture, &audiopara.character.getcurrentchannelId);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureGetFrameCount
* @tc.number  SUB_Audio_HDI_AudioCaptureGetFrameCount_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureGetFrameCount interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureGetFrameCount_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->attr.GetFrameCount(audiopara.capture, &audiopara.character.getframecount);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(INITIAL_VALUE, audiopara.character.getframecount);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureGetFrameSize
* @tc.number  SUB_Audio_HDI_AudioCaptureGetFrameSize_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureGetFrameSize interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureGetFrameSize_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->attr.GetFrameSize(audiopara.capture, &audiopara.character.getframesize);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_GT(audiopara.character.getframesize, INITIAL_VALUE);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureFlush
* @tc.number  SUB_Audio_HDI_AudioCaptureFlush_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureFlush interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureFlush_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    for (int i = 0; i < COUNT; ++i) {
        ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                                 &audiopara.capture);
        ASSERT_EQ(HDF_SUCCESS, ret);
        ret = audiopara.capture->control.Start((AudioHandle)audiopara.capture);
        EXPECT_EQ(HDF_SUCCESS, ret);
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->control.Flush((AudioHandle)audiopara.capture);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_ERR_NOT_SUPPORT, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
        ret = StopAudio(audiopara);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
}

/**
* @tc.name  the performace of AudioCaptureGetGainThreshold
* @tc.number  SUB_Audio_HDI_AudioCaptureGetGainThreshold_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureGetGainThreshold interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureGetGainThreshold_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->volume.GetGainThreshold(audiopara.capture, &audiopara.character.gainthresholdmin,
                &audiopara.character.gainthresholdmax);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(audiopara.character.gainthresholdmin, GAIN_MIN);
        EXPECT_EQ(audiopara.character.gainthresholdmax, GAIN_MAX);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureCheckSceneCapability
* @tc.number  SUB_Audio_HDI_AudioCaptureCheckSceneCapability_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureCheckSceneCapability interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureCheckSceneCapability_Performance_0001,
         TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    struct AudioSceneDescriptor scenes = { .scene.id = 0, .desc.pins = PIN_IN_MIC };
    bool supported = false;
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->scene.CheckSceneCapability(audiopara.capture, &scenes, &supported);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioCaptureSelectScene
* @tc.number  SUB_Audio_HDI_AudioCaptureSelectScene_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureSelectScene interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioCaptureSelectScene_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    struct AudioSceneDescriptor scenes = { .scene.id = 0, .desc.pins = PIN_IN_MIC };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->scene.SelectScene(audiopara.capture, &scenes);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = audiopara.adapter->DestroyCapture(audiopara.adapter, audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    audiopara.manager->UnloadAdapter(audiopara.manager, audiopara.adapter);
}

/**
* @tc.name  the performace of AudioGetCapturePosition
* @tc.number  SUB_Audio_HDI_AudioGetCapturePosition_Performance_0001
* @tc.devDesc  tests the performace of AudioCaptureGetCapturePosition interface by executing 1000 times,
*              and calculates the delay time and average of Delay Time.
* @tc.author: liutian
*/
HWTEST_F(AudioHdiCapturePerformaceTest, SUB_Audio_HDI_AudioGetCapturePosition_Performance_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .pins = PIN_IN_MIC, .totalTime = 0
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);
    ret = AudioCreateCapture(*audiopara.manager, audiopara.pins, audiopara.adapterName, &audiopara.adapter,
                             &audiopara.capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = audiopara.capture->control.Start((AudioHandle)audiopara.capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    for (int i = 0; i < COUNT; ++i) {
        gettimeofday(&audiopara.start, NULL);
        ret = audiopara.capture->GetCapturePosition(audiopara.capture, &audiopara.character.getframes, &audiopara.time);
        gettimeofday(&audiopara.end, NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        audiopara.delayTime = (audiopara.end.tv_sec * MICROSECOND + audiopara.end.tv_usec) -
                              (audiopara.start.tv_sec * MICROSECOND + audiopara.start.tv_usec);
        audiopara.totalTime += audiopara.delayTime;
    }
    audiopara.averageDelayTime = (float)audiopara.totalTime / COUNT;
    EXPECT_GT(LOWLATENCY, audiopara.averageDelayTime);
    ret = StopAudio(audiopara);
    EXPECT_EQ(HDF_SUCCESS, ret);
}
}
