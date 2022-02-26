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

#include "audio_interface_lib_render.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace {
extern "C" {
struct HdfSBuf *AudioRenderObtainHdfSBuf();
int32_t IoctlWrite(const struct AudioHwRenderParam *handleData);
int32_t AudioServiceRenderDispatch(struct HdfIoService *service,
                                   int cmdId,
                                   struct HdfSBuf *sBuf,
                                   struct HdfSBuf *reply);
int32_t SetHwParams(const struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetVolumeSBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderGetVolumeSBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetVolume(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderGetVolume(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioOutputRenderSetSpeedSBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioOutputRenderSetSpeed(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetPauseBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetPauseStu(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetMuteBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetMuteStu(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderGetMuteSBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderGetMuteStu(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetGainBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetGainStu(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderGetGainSBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderGetGainStu(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSceneSelectSBuf(struct HdfSBuf *sBuf,
                                      struct AudioHwRenderParam *handleData,
                                      int32_t deviceIndex);
int32_t AudioCtlRenderSceneSelect(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSceneGetGainThresholdSBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSceneGetGainThreshold(struct DevHandle *handle,
                                            int cmdId,
                                            struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetChannelModeBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderSetChannelMode(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderGetChannelModeSBuf(struct HdfSBuf *sBuf, struct AudioHwRenderParam *handleData);
int32_t AudioCtlRenderGetChannelMode(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioInterfaceLibCtlRender(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t ParamsSbufWriteBuffer(struct HdfSBuf *sBuf);
int32_t FrameSbufWriteBuffer(struct HdfSBuf *sBuf, const struct AudioHwRenderParam *handleData);
int32_t AudioOutputRenderHwParams(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioOutputRenderWrite(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioOutputRenderStop(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
int32_t AudioInterfaceLibOutputRender(struct DevHandle *handle, int cmdId, struct AudioHwRenderParam *handleData);
struct HdfIoService *HdfIoServiceBindName(const char *serviceName);
}

class AudioInterfaceLibRenderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void AudioInterfaceLibRenderTest::SetUpTestCase()
{
}

void AudioInterfaceLibRenderTest::TearDownTestCase()
{
}

HWTEST_F(AudioInterfaceLibRenderTest, IoctlWriteWhenHandleIsNull, TestSize.Level0)
{
    const struct AudioHwRenderParam *handleData = nullptr;
    int32_t ret = IoctlWrite(handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
}

HWTEST_F(AudioInterfaceLibRenderTest, IoctlWriteWhenParamIsVaild, TestSize.Level0)
{
    const struct AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = IoctlWrite(handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, SetHwParamsWhenHandleIsNull, TestSize.Level0)
{
    const struct AudioHwRenderParam *handleData = nullptr;
    int32_t ret = SetHwParams(handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
}

HWTEST_F(AudioInterfaceLibRenderTest, SetHwParamsWhenHandleIsVaild, TestSize.Level0)
{
    const struct AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = SetHwParams(handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetVolumeSBufWhenSbuffIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetVolumeSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderSetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetVolumeSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetVolumeSBufWhenSbuffIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderGetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetVolumeSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderGetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetVolumeSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderGetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioOutputRenderSetSpeedSBufWhenSbuffIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioOutputRenderSetSpeedSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioOutputRenderSetSpeedSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioOutputRenderSetSpeedSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioOutputRenderSetSpeedSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioOutputRenderSetSpeedSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetPauseBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetPauseBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetPauseBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderSetPauseBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetPauseBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetPauseBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetMuteBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetMuteBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetMuteBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderSetMuteBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetMuteBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetMuteBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetMuteSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderGetMuteSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetMuteSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderGetMuteSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetMuteSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderGetMuteSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetGainBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetGainBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetGainBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderSetGainBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetGainBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetGainBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetGainSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderGetGainSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetGainSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderGetGainSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetGainSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderGetGainSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSceneSelectSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t deviceIndex = 0;
    int32_t ret = AudioCtlRenderSceneSelectSBuf(sBuf, handleData, deviceIndex);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSceneSelectSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t deviceIndex = 0;
    int32_t ret = AudioCtlRenderSceneSelectSBuf(sBuf, handleData, deviceIndex);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSceneSelectSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t deviceIndex = 0;
    int32_t ret = AudioCtlRenderSceneSelectSBuf(sBuf, handleData, deviceIndex);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSceneGetGainThresholdSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSceneGetGainThresholdSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSceneGetGainThresholdSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderSceneGetGainThresholdSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSceneGetGainThresholdSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSceneGetGainThresholdSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetChannelModeBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetChannelModeBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetChannelModeBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderSetChannelModeBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderSetChannelModeBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderSetChannelModeBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetChannelModeSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderGetChannelModeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetChannelModeSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = nullptr;
    int32_t ret = AudioCtlRenderGetChannelModeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibRenderTest, AudioCtlRenderGetChannelModeSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    AudioHwRenderParam *handleData = new AudioHwRenderParam;
    int32_t ret = AudioCtlRenderGetChannelModeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibRenderTest, ParamsSbufWriteBufferWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    int32_t ret = ParamsSbufWriteBuffer(sBuf);
    EXPECT_EQ(HDF_FAILURE, ret);
}

HWTEST_F(AudioInterfaceLibRenderTest, ParamsSbufWriteBufferWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioRenderObtainHdfSBuf();
    int32_t ret = ParamsSbufWriteBuffer(sBuf);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
}
}
