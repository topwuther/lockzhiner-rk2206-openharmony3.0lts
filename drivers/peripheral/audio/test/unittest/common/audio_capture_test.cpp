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

#include "audio_internal.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing::ext;
namespace {
class AudioCaptureTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void AudioCaptureTest::SetUpTestCase()
{
}

void AudioCaptureTest::TearDownTestCase()
{
}

HWTEST_F(AudioCaptureTest, AudioCaptureStopWhenHandleIsNull, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = nullptr;
    AudioHandle handle = (AudioHandle)hwCapture;
    int32_t ret = AudioCaptureStop(handle);
    EXPECT_EQ(HDF_FAILURE, ret);
}

HWTEST_F(AudioCaptureTest, AudioCaptureStopWhenParamIsVaild, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = new AudioHwCapture;
    AudioHandle handle = (AudioHandle)hwCapture;
    int32_t ret = AudioCaptureStop(handle);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(hwCapture);
    hwCapture = nullptr;
}

HWTEST_F(AudioCaptureTest, AudioCaptureResumeWhenHandleIsNull, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = nullptr;
    AudioHandle handle = (AudioHandle)hwCapture;
    int32_t ret = AudioCaptureResume(handle);
    EXPECT_EQ(HDF_FAILURE, ret);
}

HWTEST_F(AudioCaptureTest, AudioCaptureResumeWhenParamIsVaild, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = new AudioHwCapture;
    AudioHandle handle = (AudioHandle)hwCapture;
    int32_t ret = AudioCaptureResume(handle);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(hwCapture);
    hwCapture = nullptr;
}

HWTEST_F(AudioCaptureTest, AudioCaptureSetSampleAttributesWhenHandleIsNull, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = nullptr;
    AudioHandle handle = (AudioHandle)hwCapture;
    AudioSampleAttributes *attrs = new AudioSampleAttributes;
    int32_t ret = AudioCaptureSetSampleAttributes(handle, attrs);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioCaptureTest, AudioCaptureSetSampleAttributesWhenAttrsIsNull, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = new AudioHwCapture;
    AudioHandle handle = (AudioHandle)hwCapture;
    AudioSampleAttributes *attrs = nullptr;
    int32_t ret = AudioCaptureSetSampleAttributes(handle, attrs);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(hwCapture);
    hwCapture = nullptr;
}

HWTEST_F(AudioCaptureTest, AudioCaptureSetSampleAttributesWhenParamIsVaild, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = new AudioHwCapture;
    AudioHandle handle = (AudioHandle)hwCapture;
    AudioSampleAttributes *attrs = new AudioSampleAttributes;
    int32_t ret = AudioCaptureSetSampleAttributes(handle, attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(hwCapture);
    hwCapture = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioCaptureTest, AudioCaptureGetSampleAttributesWhenHandleIsNull, TestSize.Level0)
{
    AudioSampleAttributes *attrs = new AudioSampleAttributes;
    int32_t ret = AudioCaptureGetSampleAttributes(nullptr, attrs);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioCaptureTest, AudioCaptureGetSampleAttributesWhenAttrsIsNull, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = new AudioHwCapture;
    AudioHandle handle = (AudioHandle)hwCapture;
    int32_t ret = AudioCaptureGetSampleAttributes(handle, nullptr);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(hwCapture);
    hwCapture = nullptr;
}

HWTEST_F(AudioCaptureTest, AudioCaptureGetSampleAttributesWhenParamIsVaild, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = new AudioHwCapture;
    AudioHandle handle = (AudioHandle)hwCapture;
    AudioSampleAttributes *attrs = new AudioSampleAttributes;
    int32_t ret = AudioCaptureGetSampleAttributes(handle, attrs);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(hwCapture);
    hwCapture = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioCaptureTest, AudioCaptureSetMuteWhenHandleIsNull, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = nullptr;
    AudioHandle handle = (AudioHandle)hwCapture;
    bool mute = true;
    int32_t ret = AudioCaptureSetMute(handle, mute);
    EXPECT_EQ(HDF_FAILURE, ret);
}

HWTEST_F(AudioCaptureTest, AudioCaptureSetMuteParamIsVaild, TestSize.Level0)
{
    struct AudioHwCapture *hwCapture = new AudioHwCapture;
    AudioHandle handle = (AudioHandle)hwCapture;
    bool mute = false;
    int32_t ret = AudioCaptureSetMute(handle, mute);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(hwCapture);
    hwCapture = nullptr;
}
}
