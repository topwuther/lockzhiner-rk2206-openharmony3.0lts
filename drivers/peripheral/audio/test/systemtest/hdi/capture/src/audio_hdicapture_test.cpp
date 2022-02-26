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
 * @brief Defines audio-related APIs, including custom data types and functions for capture drivers funtion.
 * accessing a driver adapter, and capturing audios.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file audio_hdi_common.h
 *
 * @brief Declares APIs for operations related to the capturing audio adapter.
 *
 * @since 1.0
 * @version 1.0
 */

#include "audio_hdi_common.h"
#include "audio_hdicapture_test.h"

using namespace std;
using namespace testing::ext;
using namespace HMOS::Audio;

namespace {
const string ADAPTER_NAME_HDMI = "hdmi";
const string ADAPTER_NAME_USB = "usb";
const string ADAPTER_NAME_INTERNAL = "internal";
const int BUFFER_SIZE = 16384;
const int BUFFER_SIZE_LITTLE = 0;
const uint64_t FILESIZE = 1024;

class AudioHdiCaptureTest : public testing::Test {
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

    static int32_t GetLoadAdapterAudioPara(struct PrepareAudioPara& audiopara);
};

using THREAD_FUNC = void *(*)(void *);

TestAudioManager *(*AudioHdiCaptureTest::GetAudioManager)() = nullptr;
void *AudioHdiCaptureTest::handleSo = nullptr;
#ifdef AUDIO_MPI_SO
    int32_t (*AudioHdiCaptureTest::SdkInit)() = nullptr;
    void (*AudioHdiCaptureTest::SdkExit)() = nullptr;
    void *AudioHdiCaptureTest::sdkSo = nullptr;
#endif

void AudioHdiCaptureTest::SetUpTestCase(void)
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

void AudioHdiCaptureTest::TearDownTestCase(void)
{
#ifdef AUDIO_MPI_SO
    SdkExit();
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


void AudioHdiCaptureTest::SetUp(void) {}

void AudioHdiCaptureTest::TearDown(void) {}


int32_t AudioHdiCaptureTest::GetLoadAdapterAudioPara(struct PrepareAudioPara& audiopara)
{
    int32_t ret = -1;
    int size = 0;
    auto *inst = (AudioHdiCaptureTest *)audiopara.self;
    if (inst != nullptr && inst->GetAudioManager != nullptr) {
        audiopara.manager = inst->GetAudioManager();
    }
    if (audiopara.manager == nullptr) {
        return HDF_FAILURE;
    }
    ret = audiopara.manager->GetAllAdapters(audiopara.manager, &audiopara.descs, &size);
    if (ret < 0 || audiopara.descs == nullptr || size == 0) {
        return HDF_FAILURE;
    } else {
        int index = SwitchAdapter(audiopara.descs, audiopara.adapterName,
            audiopara.portType, audiopara.audioPort, size);
        if (index < 0) {
            return HDF_FAILURE;
        } else {
            audiopara.desc = &audiopara.descs[index];
        }
    }
    if (audiopara.desc == nullptr) {
        return HDF_FAILURE;
    } else {
        ret = audiopara.manager->LoadAdapter(audiopara.manager, audiopara.desc, &audiopara.adapter);
    }
    if (ret < 0 || audiopara.adapter == nullptr) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

/**
* @tc.name  Test AudioCaptureCaptureFrame API via legal input
* @tc.number  SUB_Audio_HDI_AudioCaptureFrame_0001
* @tc.desc  test AudioCaptureCaptureFrame interface,Returns 0 if the input data is read successfully
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureFrame_0001, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t replyBytes = 0;
    uint64_t requestBytes = BUFFER_SIZE;
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;

    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->control.Start((AudioHandle)capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    char *frame = (char *)calloc(1, BUFFER_SIZE);
    EXPECT_NE(nullptr, frame);
    ret = capture->CaptureFrame(capture, frame, requestBytes, &replyBytes);
    EXPECT_EQ(HDF_SUCCESS, ret);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
}
/**
* @tc.name  Test AudioCaptureCaptureFrame API via setting the incoming parameter frame is nullptr
* @tc.number  SUB_Audio_HDI_AudioCaptureFrame_0002
* @tc.desc  test AudioCaptureCaptureFrame interface,Returns -1 if the incoming parameter frame is nullptr
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureFrame_0002, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t replyBytes = 0;
    uint64_t requestBytes = BUFFER_SIZE;
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;
    char *frame = nullptr;

    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->control.Start((AudioHandle)capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->CaptureFrame(capture, frame, requestBytes, &replyBytes);
    EXPECT_EQ(HDF_FAILURE, ret);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test AudioCaptureCaptureFrame API via setting the incoming parameter replyBytes is nullptr
* @tc.number  SUB_Audio_HDI_AudioCaptureFrame_0003
* @tc.desc  test AudioCaptureCaptureFrame interface,Returns -1 if the incoming parameter replyBytes is nullptr
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureFrame_0003, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t requestBytes = BUFFER_SIZE;
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;
    uint64_t *replyBytes = nullptr;

    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->control.Start((AudioHandle)capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    char *frame = (char *)calloc(1, BUFFER_SIZE);
    EXPECT_NE(nullptr, frame);
    ret = capture->CaptureFrame(capture, frame, requestBytes, replyBytes);
    EXPECT_EQ(HDF_FAILURE, ret);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
}
/**
* @tc.name  Test AudioCaptureCaptureFrame API via setting the incoming parameter capture is nullptr
* @tc.number  SUB_Audio_HDI_AudioCaptureFrame_0004
* @tc.desc  test AudioCaptureCaptureFrame interface,Returns -1 if the incoming parameter capture is nullptr
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureFrame_0004, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t requestBytes = BUFFER_SIZE;
    uint64_t replyBytes = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;
    struct AudioCapture *captureNull = nullptr;

    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->control.Start((AudioHandle)capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    char *frame = (char *)calloc(1, BUFFER_SIZE);
    EXPECT_NE(nullptr, frame);
    ret = capture->CaptureFrame(captureNull, frame, requestBytes, &replyBytes);
    EXPECT_EQ(HDF_FAILURE, ret);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
}
/**
* @tc.name  Test AudioCaptureFrame API without calling interface capturestart
* @tc.number  SUB_Audio_HDI_AudioCaptureFrame_0005
* @tc.desc  Test AudioCaptureFrame interface,Returns -1 if without calling interface capturestart
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureFrame_0005, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t requestBytes = BUFFER_SIZE;
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;
    uint64_t replyBytes = 0;

    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    char *frame = (char *)calloc(1, BUFFER_SIZE);
    EXPECT_NE(nullptr, frame);
    ret = capture->CaptureFrame(capture, frame, requestBytes, &replyBytes);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
}
/**
* @tc.name  Test AudioCaptureCaptureFrame API via setting the incoming parameter requestBytes
less than interface requirements
* @tc.number  SUB_Audio_HDI_AudioCaptureFrame_0006
* @tc.desc  test AudioCaptureCaptureFrame interface,Returns -1 if the incoming parameter
requestBytes less than interface requirements
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureFrame_0006, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t requestBytes = BUFFER_SIZE_LITTLE;
    uint64_t replyBytes = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;

    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->control.Start((AudioHandle)capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    char *frame = (char *)calloc(1, BUFFER_SIZE);
    EXPECT_NE(nullptr, frame);
    ret = capture->CaptureFrame(capture, frame, requestBytes, &replyBytes);
    EXPECT_EQ(HDF_FAILURE, ret);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
}
/**
* @tc.name  Test AudioCaptureGetCapturePosition API via legal input
* @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0001
* @tc.desc  Test AudioCaptureGetCapturePosition interface,Returns 0 if get CapturePosition during playing.
* @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0001, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_IN_MIC,
        .path = AUDIO_CAPTURE_FILE.c_str(), .fileSize = FILESIZE
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);

    ret = pthread_create(&audiopara.tids, NULL, (THREAD_FUNC)RecordAudio, &audiopara);
    ASSERT_EQ(HDF_SUCCESS, ret);
    sleep(1);
    if (audiopara.capture != nullptr) {
        ret = audiopara.capture->GetCapturePosition(audiopara.capture, &frames, &time);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
        EXPECT_GT(frames, INITIAL_VALUE);
    }

    ret = ThreadRelease(audiopara);
    EXPECT_EQ(HDF_SUCCESS, ret);
}
/**
* @tc.name  Test GetCapturePosition API via get CapturePosition after the audiois Paused and resumed
* @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0002
* @tc.desc   Test GetCapturePosition interface,Returns 0 if get Position after Pause and resume during playing
* @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0002, TestSize.Level1)
{
    int32_t ret = -1;
    int64_t timeExp = 0;
    uint64_t frames = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    struct PrepareAudioPara audiopara = {
        .portType = PORT_IN, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_IN_MIC,
        .path = AUDIO_CAPTURE_FILE.c_str(), .fileSize = FILESIZE
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);

    ret = pthread_create(&audiopara.tids, NULL, (THREAD_FUNC)RecordAudio, &audiopara);
    ASSERT_EQ(HDF_SUCCESS, ret);
    sleep(1);
    if (audiopara.capture != nullptr) {
        ret = audiopara.capture->control.Pause((AudioHandle)(audiopara.capture));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = audiopara.capture->GetCapturePosition(audiopara.capture, &frames, &time);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
        EXPECT_GT(frames, INITIAL_VALUE);
        ret = audiopara.capture->control.Resume((AudioHandle)(audiopara.capture));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = audiopara.capture->GetCapturePosition(audiopara.capture, &frames, &time);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
        EXPECT_GT(frames, INITIAL_VALUE);
    }

    ret = ThreadRelease(audiopara);
    EXPECT_EQ(HDF_SUCCESS, ret);
}
/**
* @tc.name  Test GetCapturePosition API via get CapturePosition after the audio file is stopped
* @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0003
* @tc.desc  Test GetCapturePosition interface,Returns 0 if get CapturePosition after stop during playing
* @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0003, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter = {};
    struct AudioCapture *capture = nullptr;
    uint64_t frames = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    int64_t timeExp = 0;

    manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateStartCapture(manager, &capture, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->control.Stop((AudioHandle)capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(capture, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);

    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetCapturePosition API via get CapturePosition after the object is created
    * @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0004
    * @tc.desc  Test GetCapturePosition interface, return 0 if get CapturePosition after the object is created
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0004, TestSize.Level1)
{
    int32_t ret = -1;
    enum AudioPortPin pins = PIN_IN_MIC;
    struct AudioAdapter *adapter = {};
    struct AudioCapture *capture = nullptr;
    uint64_t frames = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    int64_t timeExp = 0;

    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, pins, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(capture, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);

    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetCapturePosition API via setting the parameter Capture is nullptr
    * @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0005
    * @tc.desc  Test GetCapturePosition interface, return -1 if setting the parameter Capture is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0005, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter = {};
    struct AudioCapture *capture = nullptr;
    struct AudioCapture *captureNull = nullptr;
    uint64_t frames = 0;
    struct AudioTimeStamp time = {};

    manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateStartCapture(manager, &capture, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(captureNull, &frames, &time);
    EXPECT_EQ(HDF_FAILURE, ret);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetCapturePosition API via setting the parameter frames is nullptr
    * @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0006
    * @tc.desc  Test GetCapturePosition interface, return -1 if setting the parameter frames is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0006, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter = {};
    struct AudioCapture *capture = nullptr;
    uint64_t *framesNull = nullptr;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};

    manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateStartCapture(manager, &capture, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(capture, framesNull, &time);
    EXPECT_EQ(HDF_FAILURE, ret);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetCapturePosition API via setting the parameter time is nullptr
    * @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0007
    * @tc.desc  Test GetCapturePosition interface, return -1 if setting the parameter time is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0007, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter = {};
    struct AudioCapture *capture = nullptr;
    uint64_t frames = 0;
    struct AudioTimeStamp *timeNull = nullptr;

    manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateStartCapture(manager, &capture, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(capture, &frames, timeNull);
    EXPECT_EQ(HDF_FAILURE, ret);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetCapturePosition API via get CapturePosition continuously
    * @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0008
    * @tc.desc  Test GetCapturePosition interface, return 0 if the GetCapturePosition was called twice
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0008, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter = {};
    struct AudioCapture *capture = nullptr;
    uint64_t frames = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    struct AudioTimeStamp timeSec = {.tvSec = 0, .tvNSec = 0};
    int64_t timeExp = 0;

    manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateStartCapture(manager, &capture, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(capture, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);
    ret = capture->GetCapturePosition(capture, &frames, &timeSec);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetCapturePosition API via define format to AUDIO_FORMAT_PCM_16_BIT
    * @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0009
    * @tc.desc  Test GetCapturePosition interface,return 0 if get framesize define format to AUDIO_FORMAT_PCM_16_BIT
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0009, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t channelCountExp = 2;
    uint32_t sampleRateExp = 48000;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;
    struct AudioSampleAttributes attrs = {};
    struct AudioSampleAttributes attrsValue = {};
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(attrs);
    attrs.type = AUDIO_IN_MEDIA;
    attrs.interleaved = false;
    attrs.format = AUDIO_FORMAT_PCM_16_BIT;
    attrs.sampleRate = 48000;
    attrs.channelCount = 2;
    ret = capture->attr.SetSampleAttributes(capture, &attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->attr.GetSampleAttributes(capture, &attrsValue);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(sampleRateExp, attrsValue.sampleRate);
    EXPECT_EQ(AUDIO_FORMAT_PCM_16_BIT, attrsValue.format);
    EXPECT_EQ(channelCountExp, attrsValue.channelCount);

    ret = AudioCaptureStartAndOneFrame(capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(capture, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetCapturePosition API via define format to AUDIO_FORMAT_PCM_24_BIT
    * @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0010
    * @tc.desc  Test GetCapturePosition interface,return 0 if get framesize define format to AUDIO_FORMAT_PCM_24_BIT
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0010, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioSampleAttributes attrs = {};
    struct AudioSampleAttributes attrsValue = {};
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;
    uint64_t channelCountExp = 2;
    uint32_t sampleRateExp = 48000;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(attrs);
    attrs.type = AUDIO_IN_MEDIA;
    attrs.interleaved = false;
    attrs.format = AUDIO_FORMAT_PCM_24_BIT;
    attrs.sampleRate = 48000;
    attrs.channelCount = 2;
    ret = capture->attr.SetSampleAttributes(capture, &attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->attr.GetSampleAttributes(capture, &attrsValue);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_FORMAT_PCM_24_BIT, attrsValue.format);
    EXPECT_EQ(channelCountExp, attrsValue.channelCount);
    EXPECT_EQ(sampleRateExp, attrsValue.sampleRate);

    ret = AudioCaptureStartAndOneFrame(capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(capture, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetCapturePosition API via define sampleRate and channelCount to different value
    * @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0011
    * @tc.desc  Test GetCapturePosition interface,return 0 if get framesize define channelCount  as different values
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0011, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioCapture *capture = nullptr;
    struct AudioAdapter *adapter = nullptr;
    struct AudioSampleAttributes attrs = {};
    struct AudioSampleAttributes attrsValue = {};
    uint64_t channelCountExp = 1;
    uint32_t sampleRateExp = 48000;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(attrs);
    attrs.type = AUDIO_IN_MEDIA;
    attrs.interleaved = false;
    attrs.format = AUDIO_FORMAT_PCM_16_BIT;
    attrs.sampleRate = 48000;
    attrs.channelCount = 1;
    ret = capture->attr.SetSampleAttributes(capture, &attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->attr.GetSampleAttributes(capture, &attrsValue);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_FORMAT_PCM_16_BIT, attrsValue.format);
    EXPECT_EQ(sampleRateExp, attrsValue.sampleRate);
    EXPECT_EQ(channelCountExp, attrsValue.channelCount);

    ret = AudioCaptureStartAndOneFrame(capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(capture, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetCapturePosition API via define sampleRate and channelCount to 1
    * @tc.number  SUB_Audio_HDI_AudioCaptureGetCapturePosition_0012
    * @tc.desc  Test GetCapturePosition interface,return 0 if get framesize define channelCount to 1
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiCaptureTest, SUB_Audio_HDI_AudioCaptureGetCapturePosition_0012, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t channelCountExp = 1;
    uint32_t sampleRateExp = 48000;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = nullptr;
    struct AudioSampleAttributes attrs = {};
    struct AudioSampleAttributes attrsValue = {};
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateCapture(manager, PIN_IN_MIC, ADAPTER_NAME_USB, &adapter, &capture);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(attrs);
    attrs.type = AUDIO_IN_MEDIA;
    attrs.interleaved = false;
    attrs.format = AUDIO_FORMAT_PCM_24_BIT;
    attrs.sampleRate = 48000;
    attrs.channelCount = 1;
    ret = capture->attr.SetSampleAttributes(capture, &attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->attr.GetSampleAttributes(capture, &attrsValue);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(channelCountExp, attrsValue.channelCount);
    EXPECT_EQ(AUDIO_FORMAT_PCM_24_BIT, attrsValue.format);
    EXPECT_EQ(sampleRateExp, attrsValue.sampleRate);

    ret = AudioCaptureStartAndOneFrame(capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = capture->GetCapturePosition(capture, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);

    capture->control.Stop((AudioHandle)capture);
    adapter->DestroyCapture(adapter, capture);
    manager.UnloadAdapter(&manager, adapter);
}
}