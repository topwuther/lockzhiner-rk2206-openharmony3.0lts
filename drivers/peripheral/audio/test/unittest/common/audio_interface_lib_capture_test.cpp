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

#include "audio_interface_lib_capture.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace {
extern "C" {
struct HdfSBuf *AudioCapturebtainHdfSBuf();
int32_t AudioServiceCaptureDispatch(struct HdfIoService *service,
                                    int cmdId,
                                    struct HdfSBuf *sBuf,
                                    struct HdfSBuf *reply);
int32_t SetHwParamsCapture(const struct AudioHwCaptureParam *handleData);
int32_t ParamsSbufWriteBufferCapture(struct HdfSBuf *sBuf);
int32_t AudioCtlCaptureSetPauseBuf(struct HdfSBuf *sBuf, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureSetPauseStu(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureGetVolumeSBuf(struct HdfSBuf *sBuf, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureGetVolume(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureSetVolumeSBuf(struct HdfSBuf *sBuf, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureSetVolume(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureSetMuteSBuf(struct HdfSBuf *sBuf, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureSetMuteStu(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureGetMuteSBuf(struct HdfSBuf *sBuf, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureGetMuteStu(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureSetGainSBuf(struct HdfSBuf *sBuf, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureSetGainStu(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureGetGainSBuf(struct HdfSBuf *sBuf, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureGetGainStu(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureSceneSelectSBuf(struct HdfSBuf *sBuf,
                                       struct AudioHwCaptureParam *handleData,
                                       int32_t deviceIndex);
int32_t AudioCtlCaptureSceneSelect(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureGetGainThresholdSBuf(struct HdfSBuf *sBuf, struct AudioHwCaptureParam *handleData);
int32_t AudioCtlCaptureGetGainThreshold(struct DevHandleCapture *handle,
                                        int cmdId,
                                        struct AudioHwCaptureParam *handleData);
int32_t AudioInterfaceLibCtlCapture(struct DevHandleCapture *handle,
                                    int cmdId,
                                    struct AudioHwCaptureParam *handleData);
int32_t AudioOutputCaptureHwParams(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioOutputCaptureRead(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioOutputCaptureStop(struct DevHandleCapture *handle, int cmdId, struct AudioHwCaptureParam *handleData);
int32_t AudioInterfaceLibOutputCapture(struct DevHandleCapture *handle, int cmdId,
                                       struct AudioHwCaptureParam *handleData);
struct HdfIoService *HdfIoServiceBindName(const char *serviceName);
}

class AudioInterfaceLibCaptureTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void AudioInterfaceLibCaptureTest::SetUpTestCase()
{
}

void AudioInterfaceLibCaptureTest::TearDownTestCase()
{
}

HWTEST_F(AudioInterfaceLibCaptureTest, SetHwParamsWhenHandleIsNull, TestSize.Level0)
{
    const struct AudioHwCaptureParam *handleData = nullptr;
    int32_t ret = SetHwParamsCapture(handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
}

HWTEST_F(AudioInterfaceLibCaptureTest, SetHwParamsWhenHandleIsVaild, TestSize.Level0)
{
    const struct AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = SetHwParamsCapture(handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, ParamsSbufWriteBufferWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    int32_t ret = ParamsSbufWriteBufferCapture(sBuf);
    EXPECT_EQ(HDF_FAILURE, ret);
}

HWTEST_F(AudioInterfaceLibCaptureTest, ParamsSbufWriteBufferWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    int32_t ret = ParamsSbufWriteBufferCapture(sBuf);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetPauseBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureSetPauseBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetPauseBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = nullptr;
    int32_t ret = AudioCtlCaptureSetPauseBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetPauseBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureSetPauseBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetVolumeSBufWhenSbuffIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureGetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetVolumeSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = nullptr;
    int32_t ret = AudioCtlCaptureGetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetVolumeSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureGetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetVolumeSBufWhenSbuffIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureSetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetVolumeSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = nullptr;
    int32_t ret = AudioCtlCaptureSetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetVolumeSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureSetVolumeSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetMuteBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureSetMuteSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetMuteBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = nullptr;
    int32_t ret = AudioCtlCaptureSetMuteSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetMuteBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureSetMuteSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetMuteSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureGetMuteSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetMuteSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = nullptr;
    int32_t ret = AudioCtlCaptureGetMuteSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetMuteSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureGetMuteSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetGainSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureSetGainSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetGainSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = nullptr;
    int32_t ret = AudioCtlCaptureSetGainSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSetGainSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureSetGainSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetGainSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureGetGainSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetGainSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = nullptr;
    int32_t ret = AudioCtlCaptureGetGainSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetGainSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureGetGainSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSceneSelectSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t deviceIndex = 0;
    int32_t ret = AudioCtlCaptureSceneSelectSBuf(sBuf, handleData, deviceIndex);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSceneSelectSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = nullptr;
    int32_t deviceIndex = 0;
    int32_t ret = AudioCtlCaptureSceneSelectSBuf(sBuf, handleData, deviceIndex);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureSceneSelectSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t deviceIndex = 0;
    int32_t ret = AudioCtlCaptureSceneSelectSBuf(sBuf, handleData, deviceIndex);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetGainThresholdSBufWhenSbufIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = nullptr;
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureGetGainThresholdSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(handleData);
    handleData = nullptr;
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetGainThresholdSBufWhenHandleDataIsNull, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = nullptr;
    int32_t ret = AudioCtlCaptureGetGainThresholdSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_FAILURE, ret);
    HdfSBufRecycle(sBuf);
}

HWTEST_F(AudioInterfaceLibCaptureTest, AudioCtlCaptureGetGainThresholdSBufWhenParamIsVaild, TestSize.Level0)
{
    struct HdfSBuf *sBuf = AudioCapturebtainHdfSBuf();
    AudioHwCaptureParam *handleData = new AudioHwCaptureParam;
    int32_t ret = AudioCtlCaptureGetGainThresholdSBuf(sBuf, handleData);
    EXPECT_EQ(HDF_SUCCESS, ret);
    HdfSBufRecycle(sBuf);
    delete(handleData);
    handleData = nullptr;
}
}
