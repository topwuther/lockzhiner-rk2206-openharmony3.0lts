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

#ifndef HDF_AUDIO_SERVER_RENDER_H
#define HDF_AUDIO_SERVER_RENDER_H

#include "hdf_device_desc.h"

int32_t HdiServiceCreatRender(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderDestory(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderStart(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderStop(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderPause(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderResume(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderFlush(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetFrameSize(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetFrameCount(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderSetSampleAttr(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetSampleAttr(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetCurChannelId(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderCheckSceneCapability(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderSelectScene(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetMute(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderSetMute(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderSetVolume(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetVolume(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetGainThreshold(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetGain(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderSetGain(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetLatency(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderRenderFrame(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetRenderPosition(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetSpeed(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderSetSpeed(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderSetChannelMode(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetChannelMode(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderSetExtraParams(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetExtraParams(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderReqMmapBuffer(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderGetMmapPosition(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderTurnStandbyMode(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderDevDump(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderRegCallback(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceRenderDrainBuffer(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);

#endif

