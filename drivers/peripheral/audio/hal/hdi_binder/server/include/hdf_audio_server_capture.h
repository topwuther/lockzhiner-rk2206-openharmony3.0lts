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
#ifndef HDF_AUDIO_SERVER_CAPTURE_H
#define HDF_AUDIO_SERVER_CAPTURE_H

#include "hdf_device_desc.h"

int32_t HdiServiceCreatCapture(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureDestory(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureStart(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureStop(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCapturePause(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureResume(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureFlush(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetFrameSize(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetFrameCount(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureSetSampleAttr(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetSampleAttr(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetCurChannelId(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureCheckSceneCapability(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureSelectScene(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetMute(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureSetMute(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureSetVolume(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetVolume(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetGainThreshold(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetGain(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureSetGain(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureCaptureFrame(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetCapturePosition(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureSetExtraParams(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetExtraParams(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureReqMmapBuffer(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureGetMmapPosition(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureTurnStandbyMode(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);
int32_t HdiServiceCaptureDevDump(struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);

#endif
