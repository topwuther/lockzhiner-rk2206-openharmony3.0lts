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
class AudioAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void AudioAdapterTest::SetUpTestCase()
{
}

void AudioAdapterTest::TearDownTestCase()
{
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateRenderWhenAdapterIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = nullptr;
    const struct AudioDeviceDescriptor *desc = new AudioDeviceDescriptor;
    const struct AudioSampleAttributes *attrs = new AudioSampleAttributes;
    struct AudioRender *render;
    int32_t ret = AudioAdapterCreateRender(adapter, desc, attrs, &render);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(desc);
    desc = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateRenderWhenDescIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    const struct AudioDeviceDescriptor *desc = nullptr;
    const struct AudioSampleAttributes *attrs = new AudioSampleAttributes;
    struct AudioRender *render;
    int32_t ret = AudioAdapterCreateRender(adapter, desc, attrs, &render);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(adapter);
    adapter = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateRenderWhenAttrsIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    const struct AudioDeviceDescriptor *desc = new AudioDeviceDescriptor;
    const struct AudioSampleAttributes *attrs = nullptr;
    struct AudioRender *render;
    int32_t ret = AudioAdapterCreateRender(adapter, desc, attrs, &render);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(adapter);
    adapter = nullptr;
    delete(desc);
    desc = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateRenderWhenRenderIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    const struct AudioDeviceDescriptor *desc = new AudioDeviceDescriptor;
    const struct AudioSampleAttributes *attrs = new AudioSampleAttributes;
    struct AudioRender **render = nullptr;
    int32_t ret = AudioAdapterCreateRender(adapter, desc, attrs, render);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(adapter);
    adapter = nullptr;
    delete(desc);
    desc = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateRenderWhenParamIsVaild, TestSize.Level0)
{
    struct AudioManager *managerFuncs = GetAudioManagerFuncs();
    struct AudioAdapterDescriptor *desc = new AudioAdapterDescriptor;
    desc->adapterName = "usb";
    struct AudioAdapter *adapter;
    managerFuncs->LoadAdapter(managerFuncs, desc, &adapter);
    struct AudioDeviceDescriptor *desc1 = new AudioDeviceDescriptor;
    desc1->pins = PIN_OUT_SPEAKER;
    const struct AudioSampleAttributes *attrs = new AudioSampleAttributes;
    struct AudioRender *render;
    int32_t ret = AudioAdapterCreateRender(adapter, desc1, attrs, &render);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(desc);
    desc = nullptr;
    delete(desc1);
    desc1 = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterDestroyRenderWhenAdapterIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = nullptr;
    struct AudioRender *render = new AudioRender;
    int32_t ret = AudioAdapterDestroyRender(adapter, render);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(render);
    render = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterDestroyRenderWhenRenderIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    struct AudioRender *render = nullptr;
    int32_t ret = AudioAdapterDestroyRender(adapter, render);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(adapter);
    adapter = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterDestroyRenderWhenParamIsVaild, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    struct AudioRender *render = new AudioRender;
    int32_t ret = AudioAdapterDestroyRender(adapter, render);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(adapter);
    adapter = nullptr;
    delete(render);
    render = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateCaptureWhenAdapterIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = nullptr;
    const struct AudioDeviceDescriptor *desc = new AudioDeviceDescriptor;
    const struct AudioSampleAttributes *attrs = new AudioSampleAttributes;
    struct AudioCapture *capture;
    int32_t ret = AudioAdapterCreateCapture(adapter, desc, attrs, &capture);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(desc);
    desc = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateCaptureWhenDescIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    const struct AudioDeviceDescriptor *desc = nullptr;
    const struct AudioSampleAttributes *attrs = new AudioSampleAttributes;
    struct AudioCapture *capture;
    int32_t ret = AudioAdapterCreateCapture(adapter, desc, attrs, &capture);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(adapter);
    adapter = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateCaptureWhenAttrsIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    const struct AudioDeviceDescriptor *desc = new AudioDeviceDescriptor;
    const struct AudioSampleAttributes *attrs = nullptr;
    struct AudioCapture *capture;
    int32_t ret = AudioAdapterCreateCapture(adapter, desc, attrs, &capture);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(adapter);
    adapter = nullptr;
    delete(desc);
    desc = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateCaptureWhenCaptureIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    const struct AudioDeviceDescriptor *desc = new AudioDeviceDescriptor;
    const struct AudioSampleAttributes *attrs = new AudioSampleAttributes;
    struct AudioCapture **capture = nullptr;
    int32_t ret = AudioAdapterCreateCapture(adapter, desc, attrs, capture);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(adapter);
    adapter = nullptr;
    delete(desc);
    desc = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterCreateCaptureWhenParamIsVaild, TestSize.Level0)
{
    struct AudioManager *managerFuncs = GetAudioManagerFuncs();
    struct AudioAdapterDescriptor *desc = new AudioAdapterDescriptor;
    desc->adapterName = "usb";
    struct AudioAdapter *adapter;
    managerFuncs->LoadAdapter(managerFuncs, desc, &adapter);
    struct AudioDeviceDescriptor *desc1 = new AudioDeviceDescriptor;
    desc1->pins = PIN_IN_MIC;
    const struct AudioSampleAttributes *attrs = new AudioSampleAttributes;
    struct AudioCapture *capture;
    int32_t ret = AudioAdapterCreateCapture(adapter, desc1, attrs, &capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(desc);
    desc = nullptr;
    delete(desc1);
    desc1 = nullptr;
    delete(attrs);
    attrs = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterDestroyCaptureWhenAdapterIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = nullptr;
    struct AudioCapture *capture = new AudioCapture;
    int32_t ret = AudioAdapterDestroyCapture(adapter, capture);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(capture);
    capture = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterDestroyCaptureWhenCaptureIsNull, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    struct AudioCapture *capture = nullptr;
    int32_t ret = AudioAdapterDestroyCapture(adapter, capture);
    EXPECT_EQ(HDF_FAILURE, ret);
    delete(adapter);
    adapter = nullptr;
}

HWTEST_F(AudioAdapterTest, AudioAdapterDestroyCaptureWhenParamIsVaild, TestSize.Level0)
{
    struct AudioAdapter *adapter = new AudioAdapter;
    struct AudioCapture *capture = new AudioCapture;
    int32_t ret = AudioAdapterDestroyCapture(adapter, capture);
    EXPECT_EQ(HDF_SUCCESS, ret);
    delete(adapter);
    adapter = nullptr;
    delete(capture);
    capture = nullptr;
}
}
