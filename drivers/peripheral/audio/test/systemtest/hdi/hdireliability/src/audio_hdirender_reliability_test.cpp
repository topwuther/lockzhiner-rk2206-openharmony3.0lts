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

#include "audio_hdi_common.h"
#include <pthread.h>
#include "audio_hdirender_reliability_test.h"

using namespace std;
using namespace testing::ext;
using namespace HMOS::Audio;

namespace {
const string ADAPTER_NAME_HDMI = "hdmi";
const string ADAPTER_NAME_USB = "usb";
const string ADAPTER_NAME_INTERNAL = "internal";
const int PTHREAD_SAMEADA_COUNT = 10;
const int PTHREAD_DIFFADA_COUNT = 3;
const int PTHREAD_DIFFADA_SIZE = 2;

class AudioHdiRenderReliabilityTest : public testing::Test {
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
    static int32_t RelGetAllAdapter(struct PrepareAudioPara& ptr);
    static int32_t RelLoadAdapter(struct PrepareAudioPara& ptr);
    static int32_t RelUnloadAdapter(struct PrepareAudioPara& ptr);
    static int32_t RelAudioCreateRender(struct PrepareAudioPara& ptr);
    static int32_t RelAudioDestroyRender(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderStartAndFrame(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderProcedure(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderSetGain(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetGain(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetGainThreshold(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderSetMute(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetMute(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderSetVolume(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetVolume(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetFrameSize(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetFrameCount(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetCurrentChannelId(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderSetChannelMode(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetChannelMode(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderSetSampleAttributes(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetSampleAttributes(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderSelectScene(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderCheckSceneCapability(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetLatency(struct PrepareAudioPara& ptr);
};

using THREAD_FUNC = void *(*)(void *);

TestAudioManager *(*AudioHdiRenderReliabilityTest::GetAudioManager)() = nullptr;
void *AudioHdiRenderReliabilityTest::handleSo = nullptr;
#ifdef AUDIO_MPI_SO
    int32_t (*AudioHdiRenderReliabilityTest::SdkInit)() = nullptr;
    void (*AudioHdiRenderReliabilityTest::SdkExit)() = nullptr;
    void *AudioHdiRenderReliabilityTest::sdkSo = nullptr;
#endif

void AudioHdiRenderReliabilityTest::SetUpTestCase(void)
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

void AudioHdiRenderReliabilityTest::TearDownTestCase(void)
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

void AudioHdiRenderReliabilityTest::SetUp(void) {}

void AudioHdiRenderReliabilityTest::TearDown(void) {}

int32_t AudioHdiRenderReliabilityTest::RelGetAllAdapter(struct PrepareAudioPara& ptr)
{
    int size = 0;
    auto *inst = (AudioHdiRenderReliabilityTest *)ptr.self;
    if (inst != nullptr && inst->GetAudioManager != nullptr) {
        ptr.manager = inst->GetAudioManager();
    }
    if (ptr.manager == nullptr) {
        return HDF_FAILURE;
    }
    ptr.manager->GetAllAdapters(ptr.manager, &ptr.descs, &size);
    if (ptr.descs == nullptr || size == 0) {
        return HDF_FAILURE;
    } else {
        int index = SwitchAdapter(ptr.descs, ptr.adapterName, ptr.portType, ptr.audioPort, size);
        if (index < 0) {
            return HDF_FAILURE;
        } else {
            ptr.desc = &ptr.descs[index];
        }
    }
    if (ptr.desc == nullptr) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelLoadAdapter(struct PrepareAudioPara& ptr)
{
    ptr.manager->LoadAdapter(ptr.manager, ptr.desc, &ptr.adapter);

    if (ptr.adapter == nullptr) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioCreateRender(struct PrepareAudioPara& ptr)
{
    int32_t ret = -1;
    struct AudioSampleAttributes attrs = {};
    struct AudioDeviceDescriptor devDesc = {};
    if (ptr.adapter == nullptr  || ptr.manager == nullptr) {
        return HDF_FAILURE;
    }
    InitAttrs(attrs);
    InitDevDesc(devDesc, (&ptr.audioPort)->portId, ptr.pins);
    ret = ptr.adapter->CreateRender(ptr.adapter, &devDesc, &attrs, &ptr.render);
    if (ret < 0 || ptr.render == nullptr) {
        ptr.manager->UnloadAdapter(ptr.manager, ptr.adapter);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioDestroyRender(struct PrepareAudioPara& ptr)
{
    int32_t ret = -1;
    if (ptr.adapter == nullptr  || ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    ret = ptr.adapter->DestroyRender(ptr.adapter, ptr.render);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderStartAndFrame(struct PrepareAudioPara& ptr)
{
    int32_t ret = -1;
    struct AudioSampleAttributes attrs = {};
    char absPath[PATH_MAX] = {0};
    if (realpath(ptr.path, absPath) == nullptr) {
        return HDF_FAILURE;
    }
    FILE *file = fopen(absPath, "rb");
    if (file == nullptr) {
        return HDF_FAILURE;
    }
    InitAttrs(attrs);

    if (HMOS::Audio::WavHeadAnalysis(ptr.headInfo, file, attrs) < 0) {
        fclose(file);
        return HDF_FAILURE;
    } else {
        ret = HMOS::Audio::FrameStart(ptr.headInfo, ptr.render, file, attrs);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    if (ret == 0) {
        fclose(file);
    } else {
        fclose(file);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetGainThreshold(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->volume.GetGainThreshold(ptr.render, &(ptr.character.gainthresholdmin),
                                              &(ptr.character.gainthresholdmax));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderSetGain(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->volume.SetGain(ptr.render, ptr.character.setgain);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetGain(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->volume.GetGain(ptr.render, &(ptr.character.getgain));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderProcedure(struct PrepareAudioPara& ptr)
{
    int32_t ret = -1;
    ret = RelGetAllAdapter(ptr);
    if (ret < 0) {
        return HDF_FAILURE;
    }

    ret = RelLoadAdapter(ptr);
    if (ret < 0) {
        return HDF_FAILURE;
    }

    ret = RelAudioCreateRender(ptr);
    if (ret < 0) {
        return HDF_FAILURE;
    }

    ret = RelAudioRenderStartAndFrame(ptr);
    if (ret < 0) {
        ptr.adapter->DestroyRender(ptr.adapter, ptr.render);
        ptr.manager->UnloadAdapter(ptr.manager, ptr.adapter);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderSetMute(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->volume.SetMute(ptr.render, ptr.character.setmute);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetMute(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->volume.GetMute(ptr.render, &(ptr.character.getmute));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderSetVolume(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->volume.SetVolume(ptr.render, ptr.character.setvolume);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetVolume(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->volume.GetVolume(ptr.render, &(ptr.character.getvolume));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetFrameSize(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->attr.GetFrameSize(ptr.render, &(ptr.character.getframesize));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetFrameCount(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->attr.GetFrameCount(ptr.render, &(ptr.character.getframecount));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetCurrentChannelId(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->attr.GetCurrentChannelId(ptr.render, &(ptr.character.getcurrentchannelId));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderSetSampleAttributes(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->attr.SetSampleAttributes(ptr.render, &(ptr.attrs));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetSampleAttributes(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->attr.GetSampleAttributes(ptr.render, &(ptr.attrsValue));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderSelectScene(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->scene.SelectScene(ptr.render, &(ptr.scenes));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderCheckSceneCapability(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->scene.CheckSceneCapability(ptr.render, &ptr.scenes, &(ptr.character.supported));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderSetChannelMode(struct PrepareAudioPara &ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->SetChannelMode(ptr.render, ptr.character.setmode);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetChannelMode(struct PrepareAudioPara &ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->GetChannelMode(ptr.render, &(ptr.character.getmode));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderReliabilityTest::RelAudioRenderGetLatency(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->GetLatency(ptr.render, &(ptr.character.latencyTime));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

/**
* @tc.name  test AudioCreateRender API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioCreateRender_Reliability_0001
* @tc.desc  test CreateRender interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioCreateRender_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ret = RelGetAllAdapter(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = RelLoadAdapter(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioCreateRender, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        ret = (intptr_t)result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}

/**
* @tc.name  test AudioDestroyRender API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioDestroyRender_Reliability_0002
* @tc.desc  test DestroyRender interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioDestroyRender_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ret = RelGetAllAdapter(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = RelLoadAdapter(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = RelAudioCreateRender(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioDestroyRender, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}

/**
* @tc.name  test AudioRenderGetFrameSize API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderGetVolume_Reliability_0001
* @tc.desc  test GetFrameSize interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetFrameSize_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetFrameSize, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_LT(INITIAL_VALUE, arrpara[i].character.getframesize);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}

/**
* @tc.name  test AudioRenderGetFrameSize API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetFrameSize_Reliability_0002
* @tc.desc  test GetFrameSize interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetFrameSize_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetFrameSize, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }

    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_LT(INITIAL_VALUE, para[i].character.getframesize);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}

/**
* @tc.name  test AudioRenderGetFrameCount API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderGetVolume_Reliability_0001
* @tc.desc  test GetFrameCount interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetFrameCount_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ret = RelAudioRenderProcedure(para);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetFrameCount, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_LT(INITIAL_VALUE, arrpara[i].character.getframecount);
    }
    ret = StopAudio(para);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
* @tc.name  test AudioRenderGetFrameCount API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetFrameCount_Reliability_0002
* @tc.desc  test GetFrameCount interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetFrameCount_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ret = RelAudioRenderProcedure(para[i]);
        ASSERT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetFrameCount, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }

    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        ret = ThreadRelease(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_LT(INITIAL_VALUE, para[i].character.getframecount);
    }
}

/**
* @tc.name  test AudioRenderGetVolume API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderGetVolume_Reliability_0001
* @tc.desc  test GetCurrentChannelId interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetCurrentChannelId_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t channelIdValue = 2;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter, &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].character.getcurrentchannelId = 0;
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetCurrentChannelId, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_EQ(channelIdValue, arrpara[i].character.getcurrentchannelId);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}

/**
* @tc.name  test AudioRenderGetCurrentChannelId API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetCurrentChannelId_Reliability_0002
* @tc.desc test GetCurrentChannelId interface Reliability pass through pthread_create fun and adapterName is different
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetCurrentChannelId_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t channelIdValue = 2;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetCurrentChannelId, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }

    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(channelIdValue, para[i].character.getcurrentchannelId);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}

/**
* @tc.name  test AudioRenderSetMute API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderSetMute_Reliability_0001
* @tc.desc  test AudioRenderSetMute interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudiorenderSetMute_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ret = RelAudioRenderProcedure(para);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = arrpara[i].render->volume.GetMute(arrpara[i].render, &(arrpara[i].character.getmute));
        EXPECT_EQ(HDF_SUCCESS, ret);
        if (arrpara[i].character.getmute == false) {
            arrpara[i].character.setmute = true;
        } else {
            arrpara[i].character.setmute = false;
        }
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetMute, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
        ret = arrpara[i].render->volume.GetMute(arrpara[i].render, &(arrpara[i].character.getmute));
        EXPECT_EQ(HDF_SUCCESS, ret);
        if (arrpara[i].character.setmute == true) {
            EXPECT_TRUE(arrpara[i].character.getmute);
        } else {
            EXPECT_FALSE(arrpara[i].character.getmute);
        }
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
    }
    ret = StopAudio(para);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
* @tc.name  test AudioRenderSetMute API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderSetMute_Reliability_0002
* @tc.desc  test AudioRenderSetMute interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudiorenderSetMute_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ret = RelAudioRenderProcedure(para[i]);
        ASSERT_EQ(HDF_SUCCESS, ret);
        ret = para[i].render->volume.GetMute(para[i].render, &(para[i].character.getmute));
        EXPECT_EQ(HDF_SUCCESS, ret);
        if (para[i].character.getmute == false) {
            para[i].character.setmute = true;
        } else {
            para[i].character.setmute = false;
        }
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetMute, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
        ret = para[i].render->volume.GetMute(para[i].render, &(para[i].character.getmute));
        EXPECT_EQ(HDF_SUCCESS, ret);
        if (para[i].character.setmute == true) {
            EXPECT_TRUE(para[i].character.getmute);
        } else {
            EXPECT_FALSE(para[i].character.getmute);
        }
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        ret = ThreadRelease(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
* @tc.name  test AudioRenderGetMute API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderGetMute_Reliability_0001
* @tc.desc  test AudioRenderGetMute interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudiorenderGetMute_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].character.setmute = true;
        ret = arrpara[i].render->volume.SetMute(arrpara[i].render, false);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetMute, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_FALSE(arrpara[i].character.getmute);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}

/**
* @tc.name  test AudioRenderGetMute API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderGetMute_Reliability_0002
* @tc.desc  test AudioRenderGetMute interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudiorenderGetMute_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };

    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        para[i].character.setmute = true;
        ret = para[i].render->volume.SetMute(para[i].render, false);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetMute, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_FALSE(para[i].character.getmute);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}
/**
* @tc.name  test AudioRenderSetVolume API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderSetVolume_Reliability_0001
* @tc.desc  test SetVolume interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudiorenderSetVolume_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    float volumeHighExpc = 0.70;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ret = RelAudioRenderProcedure(para);
    ASSERT_EQ(HDF_SUCCESS, ret);
    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = arrpara[i].render->volume.GetVolume(arrpara[i].render, &(arrpara[i].character.getvolume));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].character.setvolume = 0.70;
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetVolume, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        ret = arrpara[i].render->volume.GetVolume(arrpara[i].render, &(arrpara[i].character.getvolume));
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(volumeHighExpc, arrpara[i].character.getvolume);
    }
    ret = StopAudio(para);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
* @tc.name  test AudioRenderSetVolume API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderSetVolume_Reliability_0002
* @tc.desc  test SetVolume interface Reliability pass through pthread_create fun and adapterName is different
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudiorenderSetVolume_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    float volumeHighExpc = 0.6;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ret = RelAudioRenderProcedure(para[i]);
        ASSERT_EQ(HDF_SUCCESS, ret);
        para[i].character.setvolume = 0.6;
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetVolume, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        ret = para[i].render->volume.GetVolume(para[i].render, &(para[i].character.getvolume));
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(volumeHighExpc, para[i].character.getvolume);
        ret = StopAudio(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
* @tc.name  test AudioRenderGetVolume API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderGetVolume_Reliability_0001
* @tc.desc  test GetVolume interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudiorenderGetVolume_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    float volumeHighExpc = 0.7;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].character.setvolume = 0.7;
        ret = arrpara[i].render->volume.SetVolume(arrpara[i].render, arrpara[i].character.setvolume);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetVolume, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_EQ(volumeHighExpc, arrpara[i].character.getvolume);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}

/**
* @tc.name  test AudioRenderGetVolume API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderGetVolume_Reliability_0002
* @tc.desc  test GetVolume interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudiorenderGetVolume_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    float volumeHighExpc = 0.6;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };

    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        para[i].character.setvolume = 0.6;
        ret = para[i].render->volume.SetVolume(para[i].render, para[i].character.setvolume);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetVolume, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(volumeHighExpc, para[i].character.getvolume);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}

/**
* @tc.name  test AudioRenderSetSampleAttributes API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderSetSampleAttributes_Reliability_0001
* @tc.desc  test AudioRenderSetSampleAttributes interface Reliability pass through pthread_create fun and adapterName
            is same.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderSetSampleAttributes_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t count = 2;
    uint32_t rateExpc = 48000;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(para.attrs);
    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetSampleAttributes, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
        ret = arrpara[i].render->attr.GetSampleAttributes(arrpara[i].render, &(arrpara[i].attrsValue));
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(AUDIO_IN_MEDIA, arrpara[i].attrsValue.type);
        EXPECT_EQ(AUDIO_FORMAT_PCM_16_BIT, arrpara[i].attrsValue.format);
        EXPECT_EQ(rateExpc, arrpara[i].attrsValue.sampleRate);
        EXPECT_EQ(count, arrpara[i].attrsValue.channelCount);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}

/**
* @tc.name  test AudioRenderSetSampleAttributes API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderSetSampleAttributes_Reliability_0002
* @tc.desc  test AudioRenderSetSampleAttributes interface Reliability pass through pthread_create fun and adapterName
            is different.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderSetSampleAttributes_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t count = 2;
    uint32_t rateExpc = 48000;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };

    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        InitAttrs(para[i].attrs);
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetSampleAttributes, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
        ret = para[i].render->attr.GetSampleAttributes(para[i].render, &(para[i].attrsValue));
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(AUDIO_IN_MEDIA, para[i].attrsValue.type);
        EXPECT_EQ(AUDIO_FORMAT_PCM_16_BIT, para[i].attrsValue.format);
        EXPECT_EQ(rateExpc, para[i].attrsValue.sampleRate);
        EXPECT_EQ(count, para[i].attrsValue.channelCount);
    }

    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}

/**
* @tc.name  test AudioRenderGetSampleAttributes API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetSampleAttributes_Reliability_0001
* @tc.desc  test AudioRenderGetSampleAttributes interface Reliability pass through pthread_create fun and adapterName
            is same.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetSampleAttributes_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t count = 2;
    uint32_t rateExpc = 48000;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(para.attrs);
    ret = para.render->attr.SetSampleAttributes(para.render, &(para.attrs));
    EXPECT_EQ(HDF_SUCCESS, ret);
    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetSampleAttributes, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_EQ(AUDIO_IN_MEDIA, arrpara[i].attrsValue.type);
        EXPECT_EQ(AUDIO_FORMAT_PCM_16_BIT, arrpara[i].attrsValue.format);
        EXPECT_EQ(rateExpc, arrpara[i].attrsValue.sampleRate);
        EXPECT_EQ(count, arrpara[i].attrsValue.channelCount);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}

/**
* @tc.name  test AudioRenderGetSampleAttributes API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetSampleAttributes_Reliability_0002
* @tc.desc  test AudioRenderGetSampleAttributes interface Reliability pass through pthread_create fun and adapterName
            is different.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetSampleAttributes_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t count = 2;
    uint32_t rateExpc = 48000;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };

    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        InitAttrs(para[i].attrs);
        ret = para[i].render->attr.SetSampleAttributes(para[i].render, &(para[i].attrs));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetSampleAttributes, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(AUDIO_IN_MEDIA, para[i].attrsValue.type);
        EXPECT_EQ(AUDIO_FORMAT_PCM_16_BIT, para[i].attrsValue.format);
        EXPECT_EQ(rateExpc, para[i].attrsValue.sampleRate);
        EXPECT_EQ(count, para[i].attrsValue.channelCount);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}

/**
* @tc.name  test AudioRenderSelectScene API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderSelectScene_Reliability_0001
* @tc.desc  test AudioRenderSelectScene interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderSelectScene_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].scenes.scene.id = 0;
        arrpara[i].scenes.desc.pins = PIN_OUT_SPEAKER;
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSelectScene, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}

/**
* @tc.name  test AudioRenderSelectScene API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderSelectScene_Reliability_0002
* @tc.desc  test AudioRenderSelectScene interface Reliability pass through pthread_create fun and adapterName
            is different.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderSelectScene_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };

    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT - 1; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        para[i].scenes.scene.id = 0;
        para[i].scenes.desc.pins = PIN_OUT_SPEAKER;
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSelectScene, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT - 1; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}

/**
* @tc.name  test AudioRenderCheckSceneCapability API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderCheckSceneCapability_Reliability_0001
* @tc.desc  test AudioRenderCheckSceneCapability interface Reliability pass through pthread_create fun and adapterName
            is same.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderCheckSceneCapability_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].scenes.scene.id = 0;
        arrpara[i].scenes.desc.pins = PIN_OUT_SPEAKER;
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderCheckSceneCapability, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}

/**
* @tc.name  test AudioRenderCheckSceneCapability API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderCheckSceneCapability_Reliability_0002
* @tc.desc  test AudioRenderCheckSceneCapability interface Reliability pass through pthread_create fun and adapterName
            is different.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderCheckSceneCapability_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };

    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        para[i].scenes.scene.id = 0;
        para[i].scenes.desc.pins = PIN_OUT_SPEAKER;
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderCheckSceneCapability, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(10000);
    }

    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}
/**
* @tc.name  test AudioRenderSetGain API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderSetGain_Reliability_0001
* @tc.desc  test AudioRenderSetGain interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderSetGain_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].character.setgain = 15;
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetGain, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_LT(GAIN_MIN, arrpara[i].character.setgain);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}
/**
* @tc.name  test AudioRenderSetGain API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderSetGain_Reliability_0002
* @tc.desc  test SetGain interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderSetGain_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_SIZE] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_SIZE];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_SIZE; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        para[i].character.setgain = 15;
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetGain, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_SIZE; ++i) {
        void *result = nullptr;
        int32_t ret = -1;
        pthread_join(tids[i], &result);
        ret = (intptr_t)result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = para[i].render->volume.GetGain(para[i].render, &(para[i].character.getgain));
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_LT(GAIN_MIN, para[i].character.getgain);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}
/**
* @tc.name  test AudioRenderGetGain API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetGain_Reliability_0001
* @tc.desc  test GetGain interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetGain_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].character.setgain = 15;
        ret = arrpara[i].render->volume.SetGain(arrpara[i].render, arrpara[i].character.setgain);
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetGain, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_LT(GAIN_MIN, arrpara[i].character.setgain);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}
/**
* @tc.name  test AudioRenderGetGain API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetGain_Reliability_0002
* @tc.desc  test GetGain interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetGain_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_SIZE] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_SIZE];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_SIZE; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        para[i].character.setgain = 15;
        ret = para[i].render->volume.SetGain(para[i].render, para[i].character.setgain);
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetGain, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_SIZE; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_LT(GAIN_MIN, para[i].character.getgain);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}
/**
* @tc.name  test AudioRenderGetGainThreshold API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetGainThreshold_Reliability_0001
* @tc.desc  test GetGainThreshold interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetGainThreshold_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetGainThreshold, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_EQ(0, arrpara[i].character.gainthresholdmin);
        EXPECT_EQ(15, arrpara[i].character.gainthresholdmax);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}
/**
* @tc.name  test AudioRenderGetGainThreshold API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetGainThreshold_Reliability_0002
* @tc.desc  test GetGainThreshold interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetGainThreshold_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_SIZE] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };

    pthread_t tids[PTHREAD_DIFFADA_SIZE];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_SIZE; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetGainThreshold, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_SIZE; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(0, para[i].character.gainthresholdmin);
        EXPECT_EQ(15, para[i].character.gainthresholdmax);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}
/**
* @tc.name  test AudioRenderSetChannelMode API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderSetChannelMode_Reliability_0001
* @tc.desc  test SetChannelMode interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderSetChannelMode_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];
    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].character.setmode = AUDIO_CHANNEL_NORMAL;
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetChannelMode, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_EQ(AUDIO_CHANNEL_NORMAL, arrpara[i].character.getmode);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}
/**
* @tc.name  test AudioRenderSetChannelMode API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderSetChannelMode_Reliability_0002
* @tc.desc  test SetChannelMode interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderSetChannelMode_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        para[i].character.setmode = AUDIO_CHANNEL_NORMAL;
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetChannelMode, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        void *result = nullptr;
        int32_t ret = -1;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        ret = para[i].render->GetChannelMode(para[i].render, &(para[i].character.getmode));
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(AUDIO_CHANNEL_NORMAL, para[i].character.getmode);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}
/**
* @tc.name  test AudioRenderGetChannelMode API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetChannelMode_Reliability_0001
* @tc.desc  test GetChannelMode interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetChannelMode_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        arrpara[i].character.setmode = AUDIO_CHANNEL_NORMAL;
        ret = arrpara[i].render->SetChannelMode(arrpara[i].render, arrpara[i].character.setmode);
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderSetChannelMode, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_EQ(AUDIO_CHANNEL_NORMAL, arrpara[i].character.getmode);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}
/**
* @tc.name  test AudioRenderGetChannelMode API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetChannelMode_Reliability_0002
* @tc.desc  test GetChannelMode interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderGetChannelMode_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        para[i].character.setmode = AUDIO_CHANNEL_NORMAL;
        ret = para[i].render->SetChannelMode(para[i].render, para[i].character.setmode);
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetChannelMode, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_EQ(AUDIO_CHANNEL_NORMAL, para[i].character.getmode);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}
/**
* @tc.name  test AudioRenderRenderGetLatency API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderRenderGetLatency_Reliability_0001
* @tc.desc  test GetLatency interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: wangkang
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderRenderGetLatency_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t expectLatency = 0;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ASSERT_NE(nullptr, GetAudioManager);
    para.manager = GetAudioManager();
    ASSERT_NE(nullptr, para.manager);
    ret = AudioCreateRender(*para.manager, para.pins, para.adapterName, &para.adapter,
                            &para.render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetLatency, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_LT(expectLatency, arrpara[i].character.latencyTime);
    }
    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
}
/**
* @tc.name  test AudioRenderRenderGetLatency API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderRenderGetLatency_Reliability_0002
* @tc.desc  test GetLatency interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: wangkang
*/
HWTEST_F(AudioHdiRenderReliabilityTest, SUB_Audio_HDI_AudioRenderRenderGetLatency_Reliability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t expectLatency = 0;
    struct PrepareAudioPara para[PTHREAD_DIFFADA_COUNT] = {
        {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }, {
            .portType = PORT_OUT, .adapterName = ADAPTER_NAME_INTERNAL.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
            .path = AUDIO_FILE.c_str()
        }
    };
    pthread_t tids[PTHREAD_DIFFADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        ASSERT_NE(nullptr, GetAudioManager);
        para[i].manager = GetAudioManager();
        ASSERT_NE(nullptr, para[i].manager);
        ret = AudioCreateRender(*para[i].manager, para[i].pins, para[i].adapterName, &para[i].adapter,
                                &para[i].render);
        ASSERT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetLatency, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        pthread_join(para[i].tids, &para[i].result);
        ret = (intptr_t)para[i].result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_LT(expectLatency, para[i].character.latencyTime);
        para[i].adapter->DestroyRender(para[i].adapter, para[i].render);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}
}