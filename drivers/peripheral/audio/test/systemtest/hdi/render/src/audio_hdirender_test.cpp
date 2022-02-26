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
 * @brief Defines audio-related APIs, including custom data types and functions for loading drivers,
 * accessing a driver adapter, and rendering audios.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file audio_hdi_common.h
 *
 * @brief Declares APIs for operations related to the audio render adapter.
 *
 * @since 1.0
 * @version 1.0
 */

#include "audio_hdi_common.h"
#include "audio_hdirender_test.h"

using namespace std;
using namespace testing::ext;
using namespace HMOS::Audio;

namespace {
const string ADAPTER_NAME_USB = "usb";
const string ADAPTER_NAME_INTERNAL = "internal";

class AudioHdiRenderTest : public testing::Test {
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

TestAudioManager *(*AudioHdiRenderTest::GetAudioManager)() = nullptr;
void *AudioHdiRenderTest::handleSo = nullptr;
#ifdef AUDIO_MPI_SO
    int32_t (*AudioHdiRenderTest::SdkInit)() = nullptr;
    void (*AudioHdiRenderTest::SdkExit)() = nullptr;
    void *AudioHdiRenderTest::sdkSo = nullptr;
#endif
void AudioHdiRenderTest::SetUpTestCase(void)
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

void AudioHdiRenderTest::TearDownTestCase(void)
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

void AudioHdiRenderTest::SetUp(void) {}

void AudioHdiRenderTest::TearDown(void) {}

int32_t AudioHdiRenderTest::GetLoadAdapterAudioPara(struct PrepareAudioPara& audiopara)
{
    int32_t ret = -1;
    int size = 0;
    auto *inst = (AudioHdiRenderTest *)audiopara.self;
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
* @tc.name  Test RenderGetLatency API via legal
* @tc.number  SUB_Audio_HDI_RenderGetLatency_0001
* @tc.desc  test RenderGetLatency interface, return 0 if GetLatency successful
* @tc.author: wangkang
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetLatency_0001, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t latencyTime = 0;
    uint32_t expectLatency = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->GetLatency(render, &latencyTime);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_LT(expectLatency, latencyTime);

    ret = render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test RenderGetLatency API via Setting parameters render is empty
* @tc.number  SUB_Audio_HDI_AudioRenderGetLatency_0002
* @tc.desc  test RenderGetLatency interface, return -1 if Setting parameters render is empty
* @tc.author: wangkang
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetLatency_0002, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t latencyTime = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    struct AudioRender *renderNull = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->GetLatency(renderNull, &latencyTime);
    EXPECT_EQ(HDF_FAILURE, ret);

    ret = render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test RenderGetLatency API via Setting parameters ms is empty
* @tc.number  SUB_Audio_HDI_AudioRenderGetLatency_0003
* @tc.desc  test RenderGetLatency interface,return -1 if Setting parameters ms is empty
* @tc.author: wangkang
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetLatency_0003, TestSize.Level1)
{
    int32_t ret = -1;
    uint32_t *latencyTime = nullptr;
    struct AudioRender *render = nullptr;
    struct AudioAdapter *adapter = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->GetLatency(render, latencyTime);
    EXPECT_EQ(HDF_FAILURE, ret);

    ret = render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test SetRenderSpeed API via legal
    * @tc.number  SUB_Audio_HDI_AudioRenderSetRenderSpeed_0001
    * @tc.desc  Test SetRenderSpeed interface,return -2 if setting RenderSpeed
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderSetRenderSpeed_0001, TestSize.Level1)
{
    int32_t ret = -1;
    float speed = 100;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->SetRenderSpeed(render, speed);
    EXPECT_EQ(HDF_ERR_NOT_SUPPORT, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test SetRenderSpeed API via setting the incoming parameter handle is nullptr
    * @tc.number  SUB_Audio_HDI_AudioRenderSetRenderSpeed_0002
    * @tc.desc  Test SetRenderSpeed interface,return -2 if the incoming parameter handle is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderSetRenderSpeed_0002, TestSize.Level1)
{
    int32_t ret = -1;
    float speed = 0;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    struct AudioRender *renderNull = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->SetRenderSpeed(renderNull, speed);
    EXPECT_EQ(HDF_FAILURE, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetRenderSpeed API via legal
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderSpeed_0001
    * @tc.desc  Test GetRenderSpeed interface,return -2 if getting RenderSpeed
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderSpeed_0001, TestSize.Level1)
{
    int32_t ret = -1;
    float speed = 0;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->GetRenderSpeed(render, &speed);
    EXPECT_EQ(HDF_ERR_NOT_SUPPORT, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetRenderSpeed API via setting the incoming parameter handle is nullptr
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderSpeed_0002
    * @tc.desc  Test GetRenderSpeed interface,return -2 if the incoming parameter handle is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderSpeed_0002, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    struct AudioRender *renderNull = nullptr;
    float speed = 0;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->GetRenderSpeed(renderNull, &speed);
    EXPECT_EQ(HDF_FAILURE, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetRenderSpeed API via setting the incoming parameter speed is nullptr
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderSpeed_0002
    * @tc.desc  Test GetRenderSpeed interface,return -2 if the incoming parameter speed is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderSpeed_0003, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    float *speedNull = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->GetRenderSpeed(render, speedNull);
    EXPECT_EQ(HDF_FAILURE, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test AudioRenderFrame API via legal input
* @tc.number  SUB_Audio_HDI_AudioRenderFrame_0001
* @tc.desc  test AudioRenderFrame interface,Returns 0 if the data is written successfully
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderFrame_0001, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t requestBytes = 0;
    uint64_t replyBytes = 0;
    enum AudioPortPin pins = PIN_OUT_SPEAKER;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    char *frame = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, pins, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->control.Start((AudioHandle)render);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = RenderFramePrepare(AUDIO_FILE, frame, requestBytes);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->RenderFrame(render, frame, requestBytes, &replyBytes);
    EXPECT_EQ(HDF_SUCCESS, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
}
/**
* @tc.name  Test AudioRenderFrame API via setting the incoming parameter render is nullptr
* @tc.number  SUB_Audio_HDI_AudioRenderFrame_0002
* @tc.desc  Test AudioRenderFrame interface,Returns -1 if the incoming parameter render is nullptr
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderFrame_0002, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t requestBytes = 0;
    uint64_t replyBytes = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    struct AudioRender *renderNull = nullptr;
    char *frame = nullptr;

    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->control.Start((AudioHandle)render);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = RenderFramePrepare(AUDIO_FILE, frame, requestBytes);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->RenderFrame(renderNull, frame, requestBytes, &replyBytes);
    EXPECT_EQ(HDF_FAILURE, ret);

    ret = render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
}
/**
* @tc.name  Test AudioRenderFrame API via setting the incoming parameter frame is nullptr
* @tc.number  SUB_Audio_HDI_AudioRenderFrame_0003
* @tc.desc  Test AudioRenderFrame interface,Returns -1 if the incoming parameter frame is nullptr
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderFrame_0003, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t requestBytes = 0;
    uint64_t replyBytes = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    char *frame = nullptr;

    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->control.Start((AudioHandle)render);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = render->RenderFrame(render, frame, requestBytes, &replyBytes);
    EXPECT_EQ(HDF_FAILURE, ret);
    ret = render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test AudioRenderFrame API via setting the incoming parameter replyBytes is nullptr
* @tc.number  SUB_Audio_HDI_AudioRenderFrame_0004
* @tc.desc  Test AudioRenderFrame interface,Returns -1 if the incoming parameter replyBytes is nullptr
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderFrame_0004, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t requestBytes = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    char *frame = nullptr;
    uint64_t *replyBytes = nullptr;

    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->control.Start((AudioHandle)render);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = RenderFramePrepare(AUDIO_FILE, frame, requestBytes);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->RenderFrame(render, frame, requestBytes, replyBytes);
    EXPECT_EQ(HDF_FAILURE, ret);

    ret = render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
}
/**
* @tc.name  Test AudioRenderFrame API without calling interface renderstart
* @tc.number  SUB_Audio_HDI_AudioRenderFrame_0005
* @tc.desc  Test AudioRenderFrame interface,Returns -1 if without calling interface renderstart
* @tc.author: liweiming
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderFrame_0005, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t replyBytes = 0;
    uint64_t requestBytes = 0;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    char *frame = nullptr;

    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = RenderFramePrepare(AUDIO_FILE, frame, requestBytes);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->RenderFrame(render, frame, requestBytes, &replyBytes);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
    if (frame != nullptr) {
        free(frame);
        frame = nullptr;
    }
}
/**
    * @tc.name  Test SetChannelMode API via setting channel mode to different enumeration values
    * @tc.number  SUB_Audio_HDI_AudioRenderSetChannelMode_0001
    * @tc.desc  Test SetChannelMode interface,return 0 if set channel mode to different enumeration values
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderSetChannelMode_0001, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    enum AudioChannelMode mode = AUDIO_CHANNEL_NORMAL;
    enum AudioChannelMode modeOne = AUDIO_CHANNEL_BOTH_LEFT;
    enum AudioChannelMode modeSec = AUDIO_CHANNEL_BOTH_RIGHT;
    enum AudioChannelMode modeTrd = AUDIO_CHANNEL_EXCHANGE;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->SetChannelMode(render, mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_NORMAL, mode);
    ret = render->SetChannelMode(render, modeOne);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &modeOne);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_BOTH_LEFT, modeOne);
    ret = render->SetChannelMode(render, modeSec);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &modeSec);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_BOTH_RIGHT, modeSec);
    ret = render->SetChannelMode(render, modeTrd);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &modeTrd);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_EXCHANGE, modeTrd);
    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test SetChannelMode API via setting channel mode to different values
    * @tc.number  SUB_Audio_HDI_AudioRenderSetChannelMode_0002
    * @tc.desc  Test SetChannelMode interface,return 0 if set channel mode to different values
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderSetChannelMode_0002, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    enum AudioChannelMode mode = AUDIO_CHANNEL_MIX;
    enum AudioChannelMode modeOne = AUDIO_CHANNEL_LEFT_MUTE;
    enum AudioChannelMode modeSec = AUDIO_CHANNEL_RIGHT_MUTE;
    enum AudioChannelMode modeTrd = AUDIO_CHANNEL_BOTH_MUTE;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->SetChannelMode(render, mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_MIX, mode);
    ret = render->SetChannelMode(render, modeOne);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &modeOne);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_LEFT_MUTE, modeOne);
    ret = render->SetChannelMode(render, modeSec);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &modeSec);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_RIGHT_MUTE, modeSec);
    ret = render->SetChannelMode(render, modeTrd);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &modeTrd);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_BOTH_MUTE, modeTrd);
    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test SetChannelMode API via setting channel mode after render object is created
    * @tc.number  SUB_Audio_HDI_AudioRenderSetChannelMode_0003
    * @tc.desc  Test SetChannelMode interface,return 0 if set channel mode after render object is created
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderSetChannelMode_0003, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    enum AudioChannelMode mode = AUDIO_CHANNEL_NORMAL;
    enum AudioPortPin pins = PIN_OUT_SPEAKER;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateRender(manager, pins, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->SetChannelMode(render, mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_NORMAL, mode);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test SetChannelMode API via setting the parameter render is nullptr
    * @tc.number  SUB_Audio_HDI_AudioRenderSetChannelMode_0004
    * @tc.desc  Test SetChannelMode interface,return -1 if set the parameter render is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderSetChannelMode_0004, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioRender *render = nullptr;
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *renderNull = nullptr;
    enum AudioChannelMode mode = AUDIO_CHANNEL_NORMAL;
    enum AudioPortPin pins = PIN_OUT_SPEAKER;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateRender(manager, pins, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->SetChannelMode(renderNull, mode);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetChannelMode API via getting the channel mode after setting
    * @tc.number  SUB_Audio_HDI_AudioRenderGetChannelMode_0001
    * @tc.desc  Test GetChannelMode interface,return 0 if getting the channel mode after setting
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetChannelMode_0001, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    enum AudioChannelMode mode = AUDIO_CHANNEL_NORMAL;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->GetChannelMode(render, &mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->SetChannelMode(render, mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_NORMAL, mode);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetChannelMode API via getting the parameter render is nullptr
    * @tc.number  SUB_Audio_HDI_AudioRenderGetChannelMode_0002
    * @tc.desc  Test GetChannelMode interface,return -1 if getting the parameter render is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetChannelMode_0002, TestSize.Level1)
{
    int32_t ret = -1;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *renderNull = nullptr;
    struct AudioRender *render = nullptr;
    enum AudioChannelMode mode = AUDIO_CHANNEL_NORMAL;

    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->GetChannelMode(renderNull, &mode);
    EXPECT_EQ(HDF_FAILURE, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetChannelMode API via getting the channel mode after the render object is created
    * @tc.number  SUB_Audio_HDI_AudioRenderGetChannelMode_0003
    * @tc.desc  Test GetChannelMode interface,return 0 if getting the channel mode after the object is created
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetChannelMode_0003, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    TestAudioManager manager = {};
    enum AudioPortPin pins = PIN_OUT_SPEAKER;
    enum AudioChannelMode mode = AUDIO_CHANNEL_NORMAL;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateRender(manager, pins, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->GetChannelMode(render, &mode);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_CHANNEL_NORMAL, mode);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test GetRenderPosition API via legal input
* @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0001
* @tc.desc  Test GetRenderPosition interface,Returns 0 if get RenderPosition during playing.
* @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0001, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    struct PrepareAudioPara audiopara = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);

    ret = pthread_create(&audiopara.tids, NULL, (THREAD_FUNC)PlayAudioFile, &audiopara);
    ASSERT_EQ(HDF_SUCCESS, ret);
    sleep(1);
    if (audiopara.render != nullptr) {
        ret = audiopara.render->GetRenderPosition(audiopara.render, &frames, &time);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
        EXPECT_GT(frames, INITIAL_VALUE);
    }

    ret = ThreadRelease(audiopara);
    EXPECT_EQ(HDF_SUCCESS, ret);
}
/**
* @tc.name  Test AudioRenderGetRenderPosition API via get RenderPosition after the audio file is Paused and resumed
* @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0002
* @tc.desc   Test GetRenderPosition interface,Returns 0 if get RenderPosition after Pause and resume during playing
* @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0002, TestSize.Level1)
{
    int32_t ret = -1;
    int64_t timeExp = 0;
    uint64_t frames = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    struct PrepareAudioPara audiopara = {
        .portType = PORT_OUT, .adapterName = ADAPTER_NAME_USB.c_str(), .self = this, .pins = PIN_OUT_SPEAKER,
        .path = AUDIO_FILE.c_str()
    };
    ASSERT_NE(nullptr, GetAudioManager);
    audiopara.manager = GetAudioManager();
    ASSERT_NE(nullptr, audiopara.manager);

    ret = pthread_create(&audiopara.tids, NULL, (THREAD_FUNC)PlayAudioFile, &audiopara);
    ASSERT_EQ(HDF_SUCCESS, ret);
    sleep(1);
    if (audiopara.render != nullptr) {
        ret = audiopara.render->control.Pause((AudioHandle)(audiopara.render));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = audiopara.render->GetRenderPosition(audiopara.render, &frames, &time);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
        EXPECT_GT(frames, INITIAL_VALUE);
        usleep(1000);
        ret = audiopara.render->control.Resume((AudioHandle)(audiopara.render));
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = audiopara.render->GetRenderPosition(audiopara.render, &frames, &time);
        EXPECT_EQ(HDF_SUCCESS, ret);
        EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
        EXPECT_GT(frames, INITIAL_VALUE);
    }

    ret = ThreadRelease(audiopara);
    EXPECT_EQ(HDF_SUCCESS, ret);
}
/**
* @tc.name  Test GetRenderPosition API via get RenderPosition after the audio file is stopped
* @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0003
* @tc.desc  Test GetRenderPosition interface,Returns 0 if get RenderPosition after stop
* @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0003, TestSize.Level1)
{
    int32_t ret = -1;
    int64_t timeExp = 0;
    uint64_t frames = 0;
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->control.Stop((AudioHandle)render);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = render->GetRenderPosition(render, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test AudioRenderGetRenderPosition API via setting the parameter render is nullptr
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0004
    * @tc.desc  Test AudioRenderGetRenderPosition interface, return -1 if setting the parameter render is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0004, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    struct AudioRender *render = nullptr;
    struct AudioAdapter *adapter =nullptr;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};

    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->GetRenderPosition(render, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test AudioRenderGetRenderPosition API via setting the parameter render is nullptr
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0005
    * @tc.desc  Test AudioRenderGetRenderPosition interface, return -1 if setting the parameter render is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0005, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t frames = 0;
    struct AudioTimeStamp time = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    struct AudioRender *renderNull = nullptr;

    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->GetRenderPosition(renderNull, &frames, &time);
    EXPECT_EQ(HDF_FAILURE, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test AudioRenderGetRenderPosition API via setting the parameter frames is nullptr
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0006
    * @tc.desc  Test AudioRenderGetRenderPosition interface, return -1 if setting the parameter frames is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0006, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t *framesNull = nullptr;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};

    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);
    ret = render->GetRenderPosition(render, framesNull, &time);
    EXPECT_EQ(HDF_FAILURE, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test AudioRenderGetRenderPosition API via setting the parameter time is nullptr
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0007
    * @tc.desc  Test AudioRenderGetRenderPosition interface, return -1 if setting the parameter time is nullptr
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0007, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t frames = 0;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    struct AudioTimeStamp *timeNull = nullptr;

    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->GetRenderPosition(render, &frames, timeNull);
    EXPECT_EQ(HDF_FAILURE, ret);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test AudioRenderGetRenderPosition API via get RenderPosition continuously
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0008
    * @tc.desc  Test AudioRenderGetRenderPosition interface, return 0 if the GetRenderPosition was called twice
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0008, TestSize.Level1)
{
    int32_t ret = -1;
    int64_t timeExp = 0;
    uint64_t frames = 0;
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->GetRenderPosition(render, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetRenderPosition(render, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);

    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetRenderPosition API via define format to AUDIO_FORMAT_PCM_16_BIT
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0009
    * @tc.desc  Test GetRenderPosition interface,return 0 if get framesize define format to AUDIO_FORMAT_PCM_16_BIT
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0009, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    enum AudioPortPin pins = PIN_OUT_SPEAKER;
    struct AudioSampleAttributes attrs = {};
    struct AudioSampleAttributes attrsValue = {};
    uint64_t channelCountExp = 2;
    uint32_t sampleRateExp = 48000;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    TestAudioManager manager = *GetAudioManager();
    ASSERT_NE(nullptr, GetAudioManager);
    ret = AudioCreateRender(manager, pins, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(attrs);
    attrs.type = AUDIO_IN_MEDIA;
    attrs.interleaved = false;
    attrs.format = AUDIO_FORMAT_PCM_16_BIT;
    attrs.sampleRate = 48000;
    attrs.channelCount = 2;
    ret = render->attr.SetSampleAttributes(render, &attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->attr.GetSampleAttributes(render, &attrsValue);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_FORMAT_PCM_16_BIT, attrsValue.format);
    EXPECT_EQ(sampleRateExp, attrsValue.sampleRate);
    EXPECT_EQ(channelCountExp, attrsValue.channelCount);
    ret = AudioRenderStartAndOneFrame(render);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetRenderPosition(render, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);
    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetRenderPosition API via define format to AUDIO_FORMAT_PCM_24_BIT
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0010
    * @tc.desc  Test GetRenderPosition interface,return 0 if get framesize define format to AUDIO_FORMAT_PCM_24_BIT
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0010, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    uint64_t channelCountExp = 2;
    uint32_t sampleRateExp = 48000;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    struct AudioSampleAttributes attrs = {};
    struct AudioSampleAttributes attrsValue = {};
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(attrs);
    attrs.type = AUDIO_IN_MEDIA;
    attrs.interleaved = false;
    attrs.format = AUDIO_FORMAT_PCM_24_BIT;
    attrs.sampleRate = 48000;
    attrs.channelCount = 2;
    ret = render->attr.SetSampleAttributes(render, &attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->attr.GetSampleAttributes(render, &attrsValue);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_FORMAT_PCM_24_BIT, attrsValue.format);
    EXPECT_EQ(sampleRateExp, attrsValue.sampleRate);
    EXPECT_EQ(channelCountExp, attrsValue.channelCount);
    ret = AudioRenderStartAndOneFrame(render);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetRenderPosition(render, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);
    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetRenderPosition API via define sampleRate and channelCount to different value
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0011
    * @tc.desc  Test GetRenderPosition interface,return 0 if get framesize define channelCount  as different values
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0011, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    uint32_t sampleRateExp = 48000;
    uint64_t channelCountExp = 1;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    struct AudioSampleAttributes attrs = {};
    struct AudioSampleAttributes attrsValue = {};
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(attrs);
    attrs.type = AUDIO_IN_MEDIA;
    attrs.interleaved = false;
    attrs.format = AUDIO_FORMAT_PCM_16_BIT;
    attrs.sampleRate = 48000;
    attrs.channelCount = 1;
    ret = render->attr.SetSampleAttributes(render, &attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->attr.GetSampleAttributes(render, &attrsValue);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(sampleRateExp, attrsValue.sampleRate);
    EXPECT_EQ(AUDIO_FORMAT_PCM_16_BIT, attrsValue.format);
    EXPECT_EQ(channelCountExp, attrsValue.channelCount);
    ret = AudioRenderStartAndOneFrame(render);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetRenderPosition(render, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);
    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
    * @tc.name  Test GetRenderPosition API via define sampleRate and channelCount to 1
    * @tc.number  SUB_Audio_HDI_AudioRenderGetRenderPosition_0012
    * @tc.desc  Test GetRenderPosition interface,return 0 if get framesize define channelCount to 1
    * @tc.author: tiansuli
*/
HWTEST_F(AudioHdiRenderTest, SUB_Audio_HDI_AudioRenderGetRenderPosition_0012, TestSize.Level1)
{
    int32_t ret = -1;
    uint64_t frames = 0;
    int64_t timeExp = 0;
    struct AudioRender *render = nullptr;
    struct AudioAdapter *adapter = nullptr;
    struct AudioSampleAttributes attrs = {};
    struct AudioSampleAttributes attrsValue = {};
    struct AudioTimeStamp time = {.tvSec = 0, .tvNSec = 0};
    uint64_t channelCountExp = 1;
    uint32_t sampleRateExp = 48000;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);
    InitAttrs(attrs);
    attrs.type = AUDIO_IN_MEDIA;
    attrs.interleaved = false;
    attrs.format = AUDIO_FORMAT_PCM_24_BIT;
    attrs.sampleRate = 48000;
    attrs.channelCount = 1;
    ret = render->attr.SetSampleAttributes(render, &attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->attr.GetSampleAttributes(render, &attrsValue);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_EQ(AUDIO_FORMAT_PCM_24_BIT, attrsValue.format);
    EXPECT_EQ(sampleRateExp, attrsValue.sampleRate);
    EXPECT_EQ(channelCountExp, attrsValue.channelCount);
    ret = AudioRenderStartAndOneFrame(render);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = render->GetRenderPosition(render, &frames, &time);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_GT((time.tvSec) * SECTONSEC + (time.tvNSec), timeExp);
    EXPECT_GT(frames, INITIAL_VALUE);
    render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
}