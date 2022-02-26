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
#include "audio_hdirender_scene_test.h"

using namespace std;
using namespace testing::ext;
using namespace HMOS::Audio;

namespace {
const string ADAPTER_NAME_USB = "usb";
const string ADAPTER_NAME_INTERNAL = "internal";

class AudioHdiRenderSceneTest : public testing::Test {
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

using THREAD_FUNC = void *(*)(void *);

TestAudioManager *(*AudioHdiRenderSceneTest::GetAudioManager)() = nullptr;
void *AudioHdiRenderSceneTest::handleSo = nullptr;
#ifdef AUDIO_MPI_SO
    int32_t (*AudioHdiRenderSceneTest::SdkInit)() = nullptr;
    void (*AudioHdiRenderSceneTest::SdkExit)() = nullptr;
    void *AudioHdiRenderSceneTest::sdkSo = nullptr;
#endif

void AudioHdiRenderSceneTest::SetUpTestCase(void)
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

void AudioHdiRenderSceneTest::TearDownTestCase(void)
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

void AudioHdiRenderSceneTest::SetUp(void) {}

void AudioHdiRenderSceneTest::TearDown(void) {}

/**
* @tc.name   Test AudioRenderCheckSceneCapability API and check scene's capability
* @tc.number  SUB_Audio_HDI_RenderCheckSceneCapability_0001
* @tc.desc  Test AudioRenderCheckSceneCapability interface,return 0 if check scene's capability successful.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_RenderCheckSceneCapability_0001, TestSize.Level1)
{
    int32_t ret = -1;
    bool supported = false;
    struct AudioSceneDescriptor scenes = {};
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    scenes.scene.id = 0;
    scenes.desc.pins = PIN_OUT_SPEAKER;
    ret = render->scene.CheckSceneCapability(render, &scenes, &supported);
    EXPECT_EQ(HDF_SUCCESS, ret);
    EXPECT_TRUE(supported);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name   Test checking scene's capability where the scene is not configed in the josn.
* @tc.number  SUB_Audio_HDI_RenderCheckSceneCapability_0002
* @tc.desc  Test RenderCheckSceneCapability interface,return -1 if the scene is not configed in the josn.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_RenderCheckSceneCapability_0002, TestSize.Level1)
{
    int32_t ret = -1;
    bool supported = true;
    struct AudioSceneDescriptor scenes = {};
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    scenes.scene.id = 5;
    scenes.desc.pins = PIN_OUT_SPEAKER;
    ret = render->scene.CheckSceneCapability(render, &scenes, &supported);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name   Test checking scene's capability where the render is empty
* @tc.number  SUB_Audio_HDI_RenderCheckSceneCapability_0003
* @tc.desc  Test AudioRenderCheckSceneCapability,return -1 if the render is empty.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_RenderCheckSceneCapability_0003, TestSize.Level1)
{
    int32_t ret = -1;
    bool supported = true;
    struct AudioSceneDescriptor scenes = {};
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    struct AudioRender *renderNull = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    scenes.scene.id = 0;
    scenes.desc.pins = PIN_OUT_SPEAKER;
    ret = render->scene.CheckSceneCapability(renderNull, &scenes, &supported);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name   Test AudioRenderCheckSceneCapability API and check scene's capability
* @tc.number  SUB_Audio_HDI_RenderCheckSceneCapability_0004
* @tc.desc  Test AudioRenderCheckSceneCapability interface,return -1 if the scene is empty.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_RenderCheckSceneCapability_0004, TestSize.Level1)
{
    int32_t ret = -1;
    bool supported = true;
    struct AudioSceneDescriptor *scenes = nullptr;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->scene.CheckSceneCapability(render, scenes, &supported);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name   Test AudioRenderCheckSceneCapability API and check scene's capability
* @tc.number  SUB_Audio_HDI_RenderCheckSceneCapability_0005
* @tc.desc  Test AudioRenderCheckSceneCapability interface,return -1 if the supported is empty.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_RenderCheckSceneCapability_0005, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioSceneDescriptor scenes = {};
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    scenes.scene.id = 0;
    scenes.desc.pins = PIN_OUT_SPEAKER;
    ret = render->scene.CheckSceneCapability(render, &scenes, nullptr);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test AudioRenderSelectScene API via legal input
* @tc.number  SUB_Audio_HDI_AudioRenderSelectScene_0001
* @tc.desc  Test AudioRenderSelectScene interface,return 0 if select Render's scene successful.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_AudioRenderSelectScene_0001, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioSceneDescriptor scenes = {};
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    scenes.scene.id = 0;
    scenes.desc.pins = PIN_OUT_SPEAKER;

    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->scene.SelectScene(render, &scenes);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = AudioRenderStartAndOneFrame(render);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test AudioRenderSelectScene API after Render start.
* @tc.number  SUB_Audio_HDI_AudioRenderSelectScene_0002
* @tc.desc  Test AudioRenderSelectScene, return 0 if select Render's scene successful after Render start.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_AudioRenderSelectScene_0002, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioSceneDescriptor scenes = {};
    TestAudioManager manager = {};
    struct AudioAdapter *adapter =nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    manager = *GetAudioManager();
    ret = AudioCreateStartRender(manager, &render, &adapter, ADAPTER_NAME_USB);
    ASSERT_EQ(HDF_SUCCESS, ret);

    scenes.scene.id = 0;
    scenes.desc.pins = PIN_OUT_SPEAKER;
    ret = render->scene.SelectScene(render, &scenes);
    EXPECT_EQ(HDF_SUCCESS, ret);

    ret = render->control.Stop((AudioHandle)render);
    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test AudioRenderSelectScene API where the parameter handle is empty.
* @tc.number  SUB_Audio_HDI_AudioRenderSelectScene_0003
* @tc.desc  Test AudioRenderSelectScene, return -1 if the parameter handle is empty.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_AudioRenderSelectScene_0003, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioSceneDescriptor scenes = {};
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    struct AudioRender *renderNull = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    scenes.scene.id = 0;
    scenes.desc.pins = PIN_IN_MIC;
    ret = render->scene.SelectScene(renderNull, &scenes);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test AudioRenderSelectScene API where the parameter scene is empty.
* @tc.number  SUB_Audio_HDI_AudioRenderSelectScene_0004
* @tc.desc  Test AudioRenderSelectScene, return -1 if the parameter scene is empty.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_AudioRenderSelectScene_0004, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioSceneDescriptor *scenes = nullptr;
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    ret = render->scene.SelectScene(render, scenes);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
/**
* @tc.name  Test AudioRenderSelectScene API where the scene is not configed in the josn.
* @tc.number  SUB_Audio_HDI_AudioRenderSelectScene_0005
* @tc.desc  Test AudioRenderSelectScene, return -1 if the scene is not configed in the josn.
* @tc.author: ZHANGHAILIN
*/
HWTEST_F(AudioHdiRenderSceneTest, SUB_Audio_HDI_AudioRenderSelectScene_0005, TestSize.Level1)
{
    int32_t ret = -1;
    struct AudioSceneDescriptor scenes = {};
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = nullptr;
    ASSERT_NE(nullptr, GetAudioManager);
    TestAudioManager manager = *GetAudioManager();
    ret = AudioCreateRender(manager, PIN_OUT_SPEAKER, ADAPTER_NAME_USB, &adapter, &render);
    ASSERT_EQ(HDF_SUCCESS, ret);

    scenes.scene.id = 5;
    scenes.desc.pins = PIN_OUT_HDMI;
    ret = render->scene.SelectScene(render, &scenes);
    EXPECT_EQ(HDF_FAILURE, ret);

    adapter->DestroyRender(adapter, render);
    manager.UnloadAdapter(&manager, adapter);
}
}