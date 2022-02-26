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
#include "audio_hdirender_control_reliability_test.h"

using namespace std;
using namespace testing::ext;
using namespace HMOS::Audio;

namespace {
const string ADAPTER_NAME_HDMI = "hdmi";
const string ADAPTER_NAME_USB = "usb";
const string ADAPTER_NAME_INTERNAL = "internal";
const int PTHREAD_SAMEADA_COUNT = 10;
const int PTHREAD_DIFFADA_COUNT = 3;

class AudioHdiRenderControlReliabilityTest : public testing::Test {
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
    static int32_t RelAudioRenderStart(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderFrame(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderStop(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderStartAndFrame(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderProcedure(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderPause(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderResume(struct PrepareAudioPara& ptr);
    static int32_t RelAudioRenderGetRenderPosition(struct PrepareAudioPara& ptr);
};

using THREAD_FUNC = void *(*)(void *);

TestAudioManager *(*AudioHdiRenderControlReliabilityTest::GetAudioManager)() = nullptr;
void *AudioHdiRenderControlReliabilityTest::handleSo = nullptr;
#ifdef AUDIO_MPI_SO
    int32_t (*AudioHdiRenderControlReliabilityTest::SdkInit)() = nullptr;
    void (*AudioHdiRenderControlReliabilityTest::SdkExit)() = nullptr;
    void *AudioHdiRenderControlReliabilityTest::sdkSo = nullptr;
#endif

void AudioHdiRenderControlReliabilityTest::SetUpTestCase(void)
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

void AudioHdiRenderControlReliabilityTest::TearDownTestCase(void)
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

void AudioHdiRenderControlReliabilityTest::SetUp(void) {}

void AudioHdiRenderControlReliabilityTest::TearDown(void) {}

int32_t AudioHdiRenderControlReliabilityTest::RelGetAllAdapter(struct PrepareAudioPara& ptr)
{
    int size = 0;
    auto *inst = (AudioHdiRenderControlReliabilityTest *)ptr.self;
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

int32_t AudioHdiRenderControlReliabilityTest::RelLoadAdapter(struct PrepareAudioPara& ptr)
{
    ptr.manager->LoadAdapter(ptr.manager, ptr.desc, &ptr.adapter);

    if (ptr.adapter == nullptr) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderControlReliabilityTest::RelAudioCreateRender(struct PrepareAudioPara& ptr)
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

int32_t AudioHdiRenderControlReliabilityTest::RelAudioRenderStartAndFrame(struct PrepareAudioPara& ptr)
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

int32_t AudioHdiRenderControlReliabilityTest::RelAudioRenderStart(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->control.Start((AudioHandle)(ptr.render));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderControlReliabilityTest::RelAudioRenderFrame(struct PrepareAudioPara& ptr)
{
    int32_t ret = -1;
    uint64_t requestBytes = 0;
    uint64_t replyBytes = 0;
    char *frame = nullptr;

    ret = HMOS::Audio::RenderFramePrepare(ptr.path, frame, requestBytes);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    ret = ptr.render->RenderFrame(ptr.render, frame, requestBytes, &replyBytes);
    if (ret < 0) {
        if (frame != nullptr) {
            free(frame);
            frame = nullptr;
        }
        return HDF_FAILURE;
    }
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderControlReliabilityTest::RelAudioRenderStop(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->control.Stop((AudioHandle)(ptr.render));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderControlReliabilityTest::RelAudioRenderPause(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->control.Pause((AudioHandle)(ptr.render));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderControlReliabilityTest::RelAudioRenderResume(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->control.Resume((AudioHandle)(ptr.render));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHdiRenderControlReliabilityTest::RelAudioRenderProcedure(struct PrepareAudioPara& ptr)
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

int32_t AudioHdiRenderControlReliabilityTest::RelAudioRenderGetRenderPosition(struct PrepareAudioPara& ptr)
{
    if (ptr.render == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = -1;
    ret = ptr.render->GetRenderPosition(ptr.render, &(ptr.character.getframes), &(ptr.time));
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

/**
* @tc.name  test GetAllAdapter API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioGetAllAdapter_Reliability_0001
* @tc.desc  test Reliability GetAllAdapters interface pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioGetAllAdapter_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelGetAllAdapter, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = 0;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
    }
}

/**
* @tc.name  test LoadAdapter API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioLoadlAdapter_Reliability_0002
* @tc.desc  test LoadAdapter interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioLoadlAdapter_Reliability_0002, TestSize.Level1)
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
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelLoadAdapter, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        ret = (intptr_t)result;
        EXPECT_EQ(HDF_SUCCESS, ret);
        para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
    }
}

/**
* @tc.name  test LoadAdapter API via Multithread call but desc is nullptr.
* @tc.number  SUB_Audio_HDI_AudioLoadlAdapter_Reliability_0003
* @tc.desc  test LoadAdapter interface Reliability pass through pthread_create fun.
*           adapterName is different and desc is nullptr.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioLoadlAdapter_Reliability_0003, TestSize.Level1)
{
    int32_t ret = -1;
    int32_t failcount = 0;
    int32_t succeedcount = 0;
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
        para[i].desc = nullptr;
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelLoadAdapter, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        ret = (intptr_t)result;
        if (ret == 0) {
            EXPECT_EQ(HDF_SUCCESS, ret);
            succeedcount = succeedcount + 1;
            para[i].manager->UnloadAdapter(para[i].manager, para[i].adapter);
        } else {
            EXPECT_EQ(HDF_FAILURE, ret);
            failcount = failcount + 1;
        }
    }
    EXPECT_EQ(failcount, PTHREAD_DIFFADA_COUNT);
    EXPECT_EQ(succeedcount, 0);
}

/**
* @tc.name  test AudioRenderStart API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderStart_Reliability_0001
* @tc.desc  test AudioRenderStart interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderStart_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    int32_t failcount = 0;
    int32_t succeedcount = 0;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ret = RelGetAllAdapter(para);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = RelLoadAdapter(para);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = RelAudioCreateRender(para);
    EXPECT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderStart, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(50000);
    }
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        ret = (intptr_t)result;
        if (ret == 0) {
            EXPECT_EQ(HDF_SUCCESS, ret);
            succeedcount = succeedcount + 1;
        } else {
            EXPECT_EQ(HDF_FAILURE, ret);
            failcount = failcount + 1;
        }
    }
    ret = StopAudio(para);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(failcount, PTHREAD_SAMEADA_COUNT - 1);
    EXPECT_EQ(succeedcount, 1);
}

/**
* @tc.name  test AudioRenderStart API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderStart_Reliability_0002
* @tc.desc  test AudioRenderStart interface Reliability pass through pthread_create fun.adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderStart_Reliability_0002, TestSize.Level1)
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
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderStart, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        ret = ThreadRelease(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
* @tc.name  test AudioRenderFrame API via Multithread call.
* @tc.number  SUB_Audio_HDI_RelAudioRenderFrame_Reliability_0002
* @tc.desc  test AudioRenderFrame interface Reliability pass through pthread_create fun.adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderFrame_Reliability_0002, TestSize.Level1)
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
        ret = RelAudioRenderStart(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderFrame, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(50000);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        ret = ThreadRelease(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
* @tc.name  test AudioRenderStop API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderStop_Reliability_0001
* @tc.desc  test AudioRenderStop interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderStop_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    int32_t failcount = 0;
    int32_t succeedcount = 0;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ret = RelAudioRenderProcedure(para);
    ASSERT_EQ(HDF_SUCCESS, ret);
    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderStop, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(50000);
    }
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        ret = (intptr_t)result;
        if (ret == 0) {
            EXPECT_EQ(HDF_SUCCESS, ret);
            succeedcount = succeedcount + 1;
        } else {
            EXPECT_EQ(HDF_FAILURE, ret);
            failcount = failcount + 1;
        }
    }

    para.adapter->DestroyRender(para.adapter, para.render);
    para.manager->UnloadAdapter(para.manager, para.adapter);
    EXPECT_EQ(failcount, PTHREAD_SAMEADA_COUNT - 1);
    EXPECT_EQ(succeedcount, 1);
}

/**
* @tc.name  test AudioRenderStop API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderStop_Reliability_0002
* @tc.desc  test AudioRenderStop interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderStop_Reliability_0002, TestSize.Level1)
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
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderStop, &para[i]);
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
* @tc.name  test AudioRenderPause API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderPause_Reliability_0001
* @tc.desc  test AudioRenderPause interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderPause_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    int32_t failcount = 0;
    int32_t succeedcount = 0;
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
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderPause, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(50000);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        ret = (intptr_t)result;
        if (ret == 0) {
            EXPECT_EQ(HDF_SUCCESS, ret);
            succeedcount = succeedcount + 1;
        } else {
            EXPECT_EQ(HDF_FAILURE, ret);
            failcount = failcount + 1;
        }
    }
    ret = StopAudio(para);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(failcount, PTHREAD_SAMEADA_COUNT - 1);
    EXPECT_EQ(succeedcount, 1);
}

/**
* @tc.name  test AudioRenderPause API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderPause_Reliability_0002
* @tc.desc  test AudioRenderPause interface Reliability pass through pthread_create fun and adapterName is different
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderPause_Reliability_0002, TestSize.Level1)
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
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderPause, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        ret = ThreadRelease(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
* @tc.name  test AudioRenderResume API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderResume_Reliability_0001
* @tc.desc  test RelAudioRenderResume interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderResume_Reliability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    int32_t failcount = 0;
    int32_t succeedcount = 0;
    struct PrepareAudioPara para = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_HDMI.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    struct PrepareAudioPara arrpara[PTHREAD_SAMEADA_COUNT];

    ret = RelAudioRenderProcedure(para);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = RelAudioRenderPause(para);
    ASSERT_EQ(HDF_SUCCESS, ret);

    pthread_t tids[PTHREAD_SAMEADA_COUNT];
    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        ret = memcpy_s(&arrpara[i], sizeof(PrepareAudioPara), &para, sizeof(PrepareAudioPara));
        EXPECT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderResume, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        usleep(50000);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        ret = (intptr_t)result;
        if (ret == 0) {
            EXPECT_EQ(HDF_SUCCESS, ret);
            succeedcount = succeedcount + 1;
        } else {
            EXPECT_EQ(HDF_FAILURE, ret);
            failcount = failcount + 1;
        }
    }
    ret = StopAudio(para);
    EXPECT_EQ(HDF_SUCCESS, ret);
}
/**
* @tc.name  test AudioRenderResume API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderResume_Reliability_0002
* @tc.desc  test AudioRenderResume interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderResume_Reliability_0002, TestSize.Level1)
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
        ret = RelAudioRenderPause(para[i]);
        ASSERT_EQ(HDF_SUCCESS, ret);
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderResume, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        ret = ThreadRelease(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
* @tc.name  test AudioRenderGetRenderPosition API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudiorenderGetVolume_Reliability_0001
* @tc.desc  test GetRenderPosition interface Reliability pass through pthread_create fun and adapterName is same.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_Reliability_0001,
         TestSize.Level1)
{
    int32_t ret = -1;
    int64_t timeExp = 0;
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
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetRenderPosition, &arrpara[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }

    for (int32_t i = 0; i < PTHREAD_SAMEADA_COUNT; ++i) {
        void *result = nullptr;
        pthread_join(tids[i], &result);
        EXPECT_EQ(HDF_SUCCESS, (intptr_t)result);
        EXPECT_LT(INITIAL_VALUE, arrpara[i].character.getframes);
        EXPECT_LT(timeExp, arrpara[i].time.tvNSec);
    }
    ret = StopAudio(para);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
* @tc.name  test AudioRenderGetRenderPosition API via Multithread call.
* @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_Reliability_0002
* @tc.desc  test GetRenderPosition interface Reliability pass through pthread_create fun and adapterName is different.
* @tc.author: zhouyongxiao
*/
HWTEST_F(AudioHdiRenderControlReliabilityTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_Reliability_0002,
         TestSize.Level1)
{
    int32_t ret = -1;
    int64_t timeExp = 0;
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
        int32_t ret = pthread_create(&tids[i], NULL, (THREAD_FUNC)RelAudioRenderGetRenderPosition, &para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }

    for (int32_t i = 0; i < PTHREAD_DIFFADA_COUNT; ++i) {
        para[i].tids = tids[i];
        ret = ThreadRelease(para[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_LT(INITIAL_VALUE, para[i].character.getframes);
        EXPECT_LT(timeExp, para[i].time.tvNSec);
    }
}
}