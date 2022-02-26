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

#include "audio_proxy_common.h"
#include "audio_proxy_internal.h"

int32_t AudioProxyCommonInitAttrs(struct HdfSBuf *data, const struct AudioSampleAttributes *attrs)
{
    if (data == NULL || attrs == NULL) {
        LOG_FUN_ERR("data == NULL || attrs == NULL");
        return HDF_FAILURE;
    }
    uint32_t tempAtrr;
    tempAtrr = (uint32_t)attrs->interleaved;
    if (!HdfSbufWriteUint32(data, tempAtrr)) {
        return HDF_FAILURE;
    }
    tempAtrr = (uint32_t)attrs->type;
    if (!HdfSbufWriteUint32(data, tempAtrr)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, attrs->period)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, attrs->frameSize)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, attrs->startThreshold)) {
        LOG_FUN_ERR("startThreshold Write Fail");
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, attrs->stopThreshold)) {
        LOG_FUN_ERR("stopThreshold Write Fail");
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, attrs->silenceThreshold)) {
        return HDF_FAILURE;
    }
    tempAtrr = (uint32_t)attrs->isBigEndian;
    if (!HdfSbufWriteUint32(data, tempAtrr)) {
        return HDF_FAILURE;
    }
    tempAtrr = (uint32_t)attrs->isSignedData;
    if (!HdfSbufWriteUint32(data, tempAtrr)) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioProxyCommonInitCreateData(struct HdfSBuf *data, struct AudioHwAdapter *adapter,
    const struct AudioDeviceDescriptor *desc, const struct AudioSampleAttributes *attrs)
{
    LOG_FUN_INFO();
    if (data == NULL || adapter == NULL || desc == NULL || attrs == NULL) {
        LOG_FUN_ERR("data == NULL || adapter == NULL || desc == NULL || attrs == NULL");
        return HDF_FAILURE;
    }
    uint32_t tempDesc;
    uint32_t tempAtrr;
    uint32_t pid = getpid();
    const char *adapterName = adapter->adapterDescriptor.adapterName;
    if (adapterName == NULL) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteString(data, adapterName)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, pid)) {
        return HDF_FAILURE;
    }
    tempAtrr = (uint32_t)attrs->format;
    if (!HdfSbufWriteUint32(data, tempAtrr)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, attrs->channelCount)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, attrs->sampleRate)) {
        return HDF_FAILURE;
    }
    if (AudioProxyCommonInitAttrs(data, attrs) < 0) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, desc->portId)) {
        return HDF_FAILURE;
    }
    tempDesc = (uint32_t)desc->pins;
    if (!HdfSbufWriteUint32(data, tempDesc)) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t GetAudioProxyRenderFunc(struct AudioHwRender *hwRender)
{
    if (hwRender == NULL) {
        return HDF_FAILURE;
    }
    hwRender->common.control.Start = AudioProxyRenderStart;
    hwRender->common.control.Stop = AudioProxyRenderStop;
    hwRender->common.control.Pause = AudioProxyRenderPause;
    hwRender->common.control.Resume = AudioProxyRenderResume;
    hwRender->common.control.Flush = AudioProxyRenderFlush;
    hwRender->common.control.TurnStandbyMode = AudioProxyRenderTurnStandbyMode;
    hwRender->common.control.AudioDevDump = AudioProxyRenderAudioDevDump;
    hwRender->common.attr.GetFrameSize = AudioProxyRenderGetFrameSize;
    hwRender->common.attr.GetFrameCount = AudioProxyRenderGetFrameCount;
    hwRender->common.attr.SetSampleAttributes = AudioProxyRenderSetSampleAttributes;
    hwRender->common.attr.GetSampleAttributes = AudioProxyRenderGetSampleAttributes;
    hwRender->common.attr.GetCurrentChannelId = AudioProxyRenderGetCurrentChannelId;
    hwRender->common.attr.SetExtraParams = AudioProxyRenderSetExtraParams;
    hwRender->common.attr.GetExtraParams = AudioProxyRenderGetExtraParams;
    hwRender->common.attr.ReqMmapBuffer = AudioProxyRenderReqMmapBuffer;
    hwRender->common.attr.GetMmapPosition = AudioProxyRenderGetMmapPosition;
    hwRender->common.scene.CheckSceneCapability = AudioProxyRenderCheckSceneCapability;
    hwRender->common.scene.SelectScene = AudioProxyRenderSelectScene;
    hwRender->common.volume.SetMute = AudioProxyRenderSetMute;
    hwRender->common.volume.GetMute = AudioProxyRenderGetMute;
    hwRender->common.volume.SetVolume = AudioProxyRenderSetVolume;
    hwRender->common.volume.GetVolume = AudioProxyRenderGetVolume;
    hwRender->common.volume.GetGainThreshold = AudioProxyRenderGetGainThreshold;
    hwRender->common.volume.GetGain = AudioProxyRenderGetGain;
    hwRender->common.volume.SetGain = AudioProxyRenderSetGain;
    hwRender->common.GetLatency = AudioProxyRenderGetLatency;
    hwRender->common.RenderFrame = AudioProxyRenderRenderFrame;
    hwRender->common.GetRenderPosition = AudioProxyRenderGetRenderPosition;
    hwRender->common.SetRenderSpeed = AudioProxyRenderSetRenderSpeed;
    hwRender->common.GetRenderSpeed = AudioProxyRenderGetRenderSpeed;
    hwRender->common.SetChannelMode = AudioProxyRenderSetChannelMode;
    hwRender->common.GetChannelMode = AudioProxyRenderGetChannelMode;
    hwRender->common.RegCallback = AudioProxyRenderRegCallback;
    hwRender->common.DrainBuffer = AudioProxyRenderDrainBuffer;
    return HDF_SUCCESS;
}

int32_t InitHwRenderParam(struct AudioHwRender *hwRender, const struct AudioDeviceDescriptor *desc,
                          const struct AudioSampleAttributes *attrs)
{
    if (hwRender == NULL || desc == NULL || attrs == NULL) {
        LOG_FUN_ERR("InitHwRenderParam param Is NULL");
        return HDF_FAILURE;
    }
    hwRender->renderParam.renderMode.hwInfo.deviceDescript = *desc;
    hwRender->renderParam.frameRenderMode.attrs = *attrs;
    return HDF_SUCCESS;
}

enum AudioFormat g_formatIdZero = AUDIO_FORMAT_PCM_16_BIT;
int32_t InitForGetPortCapability(struct AudioPort portIndex, struct AudioPortCapability *capabilityIndex)
{
    if (capabilityIndex == NULL) {
        LOG_FUN_ERR("capabilityIndex Is NULL");
        return HDF_FAILURE;
    }
    /* get capabilityIndex from driver or default */
    if (portIndex.dir != PORT_OUT) {
        capabilityIndex->hardwareMode = true;
        capabilityIndex->channelMasks = AUDIO_CHANNEL_STEREO;
        capabilityIndex->channelCount = CONFIG_CHANNEL_COUNT;
        return HDF_SUCCESS;
    }
    if (portIndex.portId == 0) {
        capabilityIndex->hardwareMode = true;
        capabilityIndex->channelMasks = AUDIO_CHANNEL_STEREO;
        capabilityIndex->channelCount = CONFIG_CHANNEL_COUNT;
        capabilityIndex->deviceType = portIndex.dir;
        capabilityIndex->deviceId = PIN_OUT_SPEAKER;
        capabilityIndex->formatNum = 1;
        capabilityIndex->formats = &g_formatIdZero;
        capabilityIndex->sampleRateMasks = AUDIO_SAMPLE_RATE_MASK_16000;
        capabilityIndex->subPortsNum = 1;
        capabilityIndex->subPorts = (struct AudioSubPortCapability *)calloc(capabilityIndex->subPortsNum,
            sizeof(struct AudioSubPortCapability));
        if (capabilityIndex->subPorts == NULL) {
            LOG_FUN_ERR("pointer is null!");
            return HDF_FAILURE;
        }
        capabilityIndex->subPorts->portId = portIndex.portId;
        capabilityIndex->subPorts->desc = portIndex.portName;
        capabilityIndex->subPorts->mask = PORT_PASSTHROUGH_LPCM;
        return HDF_SUCCESS;
    }
    if (portIndex.portId == 1) {
        capabilityIndex->hardwareMode = true;
        capabilityIndex->channelMasks = AUDIO_CHANNEL_STEREO;
        capabilityIndex->channelCount = CONFIG_CHANNEL_COUNT;
        capabilityIndex->deviceType = portIndex.dir;
        capabilityIndex->deviceId = PIN_OUT_HEADSET;
        capabilityIndex->formatNum = 1;
        capabilityIndex->formats = &g_formatIdZero;
        capabilityIndex->sampleRateMasks = AUDIO_SAMPLE_RATE_MASK_16000 | AUDIO_SAMPLE_RATE_MASK_8000;
        return HDF_SUCCESS;
    }
    if (portIndex.portId == HDMI_PORT_ID) {
        return HdmiPortInit(portIndex, capabilityIndex);
    }
    return HDF_FAILURE;
}

void AudioAdapterReleaseCapSubPorts(const struct AudioPortAndCapability *portCapabilitys, const int32_t num)
{
    int32_t i = 0;
    if (portCapabilitys == NULL) {
        return;
    }
    while (i < num) {
        if (&portCapabilitys[i] == NULL) {
            break;
        }
        AudioMemFree((void **)(&portCapabilitys[i].capability.subPorts));
        i++;
    }
    return;
}

int32_t AudioProxyAdapterInitAllPorts(struct AudioAdapter *adapter)
{
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    const char *adapterName = NULL;
    struct AudioHwAdapter *hwAdapter = (struct AudioHwAdapter *)adapter;
    if (hwAdapter == NULL || hwAdapter->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("hwAdapter Is NULL");
        return HDF_FAILURE;
    }
    /* Fake data */
    uint32_t portNum = hwAdapter->adapterDescriptor.portNum;
    struct AudioPort *ports = hwAdapter->adapterDescriptor.ports;
    if (ports == NULL || portNum == 0) {
        LOG_FUN_ERR("ports is NULL!");
        return HDF_FAILURE;
    }
    struct AudioPortAndCapability *portCapability = (struct AudioPortAndCapability *)calloc(portNum,
        sizeof(struct AudioPortAndCapability));
    if (portCapability == NULL) {
        LOG_FUN_ERR("portCapability is NULL!");
        return HDF_FAILURE;
    }
    for (int i = 0; i < portNum; i++) {
        portCapability[i].port = ports[i];
        if (InitForGetPortCapability(ports[i], &portCapability[i].capability)) {
            LOG_FUN_ERR("ports Init Invalid!");
            AudioAdapterReleaseCapSubPorts(portCapability, portNum);
            AudioMemFree((void **)&portCapability);
            return HDF_FAILURE;
        }
    }
    hwAdapter->portCapabilitys = portCapability;
    hwAdapter->portCapabilitys->mode = PORT_PASSTHROUGH_LPCM;
    if (AudioProxyPreprocessSBuf(&data, &reply) < 0) {
        return HDF_FAILURE;
    }
    adapterName = hwAdapter->adapterDescriptor.adapterName;
    if (!HdfSbufWriteString(data, adapterName)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwAdapter->proxyRemoteHandle, AUDIO_HDI_ADT_INIT_PORTS, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("Get Failed AudioAdapter!");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyAdapterCreateRenderSplit(struct AudioHwAdapter *hwAdapter, struct AudioHwRender *hwRender)
{
    if (hwAdapter == NULL || hwRender == NULL) {
        return HDF_FAILURE;
    }
    if (hwAdapter->adapterDescriptor.adapterName == NULL) {
        return HDF_FAILURE;
    }
    uint32_t adapterNameLen = strlen(hwAdapter->adapterDescriptor.adapterName);
    /* Get Adapter name */
    int32_t ret = strncpy_s(hwRender->renderParam.renderMode.hwInfo.adapterName, NAME_LEN - 1,
        hwAdapter->adapterDescriptor.adapterName, adapterNameLen);
    if (ret != EOK) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioProxyAdapterCreateRender(struct AudioAdapter *adapter, const struct AudioDeviceDescriptor *desc,
                                      const struct AudioSampleAttributes *attrs, struct AudioRender **render)
{
    LOG_FUN_INFO();
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwAdapter *hwAdapter = (struct AudioHwAdapter *)adapter;
    if (hwAdapter == NULL || hwAdapter->proxyRemoteHandle == NULL || desc == NULL || attrs == NULL || render == NULL) {
        return HDF_FAILURE;
    }
    struct AudioHwRender *hwRender = (struct AudioHwRender *)calloc(1, sizeof(*hwRender));
    if (hwRender == NULL) {
        LOG_FUN_ERR("hwRender is NULL!");
        return HDF_FAILURE;
    }
    hwRender->proxyRemoteHandle = hwAdapter->proxyRemoteHandle;
    if (GetAudioProxyRenderFunc(hwRender) < 0) {
        AudioMemFree((void **)&hwRender);
        return HDF_FAILURE;
    }
    /* Fill hwRender para */
    if (InitHwRenderParam(hwRender, desc, attrs) < 0) {
        AudioMemFree((void **)&hwRender);
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessSBuf(&data, &reply) < 0) {
        AudioMemFree((void **)&hwRender);
        return HDF_FAILURE;
    }
    if (AudioProxyCommonInitCreateData(data, hwAdapter, desc, attrs) < 0) {
        LOG_FUN_ERR("Failed to obtain reply");
        AudioProxyBufReplyRecycle(data, reply);
        AudioMemFree((void **)&hwRender);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwRender->proxyRemoteHandle, AUDIO_HDI_RENDER_CREATE_RENDER, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("Send Server fail!");
        AudioProxyBufReplyRecycle(data, reply);
        AudioMemFree((void **)&hwRender);
        return ret;
    }
    AudioProxyBufReplyRecycle(data, reply);
    if (AudioProxyAdapterCreateRenderSplit(hwAdapter, hwRender) < 0) {
        AudioMemFree((void **)&hwRender);
        return HDF_FAILURE;
    }
    *render = &hwRender->common;
    return HDF_SUCCESS;
}

int32_t AudioProxyAdapterDestroyRender(struct AudioAdapter *adapter, struct AudioRender *render)
{
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    if (adapter == NULL || render == NULL) {
        return HDF_FAILURE;
    }
    struct AudioHwRender *hwRender = (struct AudioHwRender *)render;
    if (hwRender == NULL || hwRender->proxyRemoteHandle == NULL) {
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessRender((AudioHandle)render, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwRender->proxyRemoteHandle, AUDIO_HDI_RENDER_DESTROY, data, reply);
    if (ret < 0) {
        if (ret != HDF_ERR_INVALID_OBJECT) {
            LOG_FUN_ERR("AudioRenderRenderFrame FAIL");
        }
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    AudioMemFree((void **)&hwRender->renderParam.frameRenderMode.buffer);
    AudioMemFree((void **)&render);
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t GetAudioProxyCaptureFunc(struct AudioHwCapture *hwCapture)
{
    if (hwCapture == NULL) {
        return HDF_FAILURE;
    }
    hwCapture->common.control.Start = AudioProxyCaptureStart;
    hwCapture->common.control.Stop = AudioProxyCaptureStop;
    hwCapture->common.control.Pause = AudioProxyCapturePause;
    hwCapture->common.control.Resume = AudioProxyCaptureResume;
    hwCapture->common.control.Flush = AudioProxyCaptureFlush;
    hwCapture->common.control.TurnStandbyMode = AudioProxyCaptureTurnStandbyMode;
    hwCapture->common.control.AudioDevDump = AudioProxyCaptureAudioDevDump;
    hwCapture->common.attr.GetFrameSize = AudioProxyCaptureGetFrameSize;
    hwCapture->common.attr.GetFrameCount = AudioProxyCaptureGetFrameCount;
    hwCapture->common.attr.SetSampleAttributes = AudioProxyCaptureSetSampleAttributes;
    hwCapture->common.attr.GetSampleAttributes = AudioProxyCaptureGetSampleAttributes;
    hwCapture->common.attr.GetCurrentChannelId = AudioProxyCaptureGetCurrentChannelId;
    hwCapture->common.attr.SetExtraParams = AudioProxyCaptureSetExtraParams;
    hwCapture->common.attr.GetExtraParams = AudioProxyCaptureGetExtraParams;
    hwCapture->common.attr.ReqMmapBuffer = AudioProxyCaptureReqMmapBuffer;
    hwCapture->common.attr.GetMmapPosition = AudioProxyCaptureGetMmapPosition;
    hwCapture->common.scene.CheckSceneCapability = AudioProxyCaptureCheckSceneCapability;
    hwCapture->common.scene.SelectScene = AudioProxyCaptureSelectScene;
    hwCapture->common.volume.SetMute = AudioProxyCaptureSetMute;
    hwCapture->common.volume.GetMute = AudioProxyCaptureGetMute;
    hwCapture->common.volume.SetVolume = AudioProxyCaptureSetVolume;
    hwCapture->common.volume.GetVolume = AudioProxyCaptureGetVolume;
    hwCapture->common.volume.GetGainThreshold = AudioProxyCaptureGetGainThreshold;
    hwCapture->common.volume.GetGain = AudioProxyCaptureGetGain;
    hwCapture->common.volume.SetGain = AudioProxyCaptureSetGain;
    hwCapture->common.CaptureFrame = AudioProxyCaptureCaptureFrame;
    hwCapture->common.GetCapturePosition = AudioProxyCaptureGetCapturePosition;
    return HDF_SUCCESS;
}

int32_t InitProxyHwCaptureParam(struct AudioHwCapture *hwCapture, const struct AudioDeviceDescriptor *desc,
                                const struct AudioSampleAttributes *attrs)
{
    if (NULL == hwCapture || NULL == desc || NULL == attrs) {
        LOG_FUN_ERR("InitHwCaptureParam param Is NULL");
        return HDF_FAILURE;
    }
    hwCapture->captureParam.captureMode.hwInfo.deviceDescript = *desc;
    hwCapture->captureParam.frameCaptureMode.attrs = *attrs;
    return HDF_SUCCESS;
}

int32_t AudioProxyAdapterCreateCaptureSplit(struct AudioHwAdapter *hwAdapter, struct AudioHwCapture *hwCapture)
{
    if (hwAdapter == NULL || hwCapture == NULL) {
        return HDF_FAILURE;
    }
    if (hwAdapter->adapterDescriptor.adapterName == NULL) {
        return HDF_FAILURE;
    }
    uint32_t adapterNameLen = strlen(hwAdapter->adapterDescriptor.adapterName);
    /* Get AdapterName */
    int32_t ret = strncpy_s(hwCapture->captureParam.captureMode.hwInfo.adapterName, NAME_LEN - 1,
        hwAdapter->adapterDescriptor.adapterName, adapterNameLen);
    if (ret != EOK) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioProxyAdapterCreateCapture(struct AudioAdapter *adapter, const struct AudioDeviceDescriptor *desc,
                                       const struct AudioSampleAttributes *attrs, struct AudioCapture **capture)
{
    LOG_FUN_INFO();
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwAdapter *hwAdapter = (struct AudioHwAdapter *)adapter;
    if (hwAdapter == NULL || hwAdapter->proxyRemoteHandle == NULL || desc == NULL ||
        attrs == NULL || capture == NULL) {
        return HDF_FAILURE;
    }
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)calloc(1, sizeof(struct AudioHwCapture));
    if (hwCapture == NULL) {
        LOG_FUN_ERR("hwCapture is NULL!");
        return HDF_FAILURE;
    }
    hwCapture->proxyRemoteHandle = hwAdapter->proxyRemoteHandle;
    if (GetAudioProxyCaptureFunc(hwCapture) < 0) {
        AudioMemFree((void **)&hwCapture);
        return HDF_FAILURE;
    }
    /* Fill hwRender para */
    if (InitProxyHwCaptureParam(hwCapture, desc, attrs) < 0) {
        AudioMemFree((void **)&hwCapture);
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessSBuf(&data, &reply) < 0) {
        AudioMemFree((void **)&hwCapture);
        return HDF_FAILURE;
    }
    if (AudioProxyCommonInitCreateData(data, hwAdapter, desc, attrs) < 0) {
        LOG_FUN_ERR("Failed to obtain reply");
        AudioProxyBufReplyRecycle(data, reply);
        AudioMemFree((void **)&hwCapture);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_CREATE_CAPTURE, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("Send Server fail!");
        AudioProxyBufReplyRecycle(data, reply);
        AudioMemFree((void **)&hwCapture);
        return ret;
    }
    AudioProxyBufReplyRecycle(data, reply);
    if (AudioProxyAdapterCreateCaptureSplit(hwAdapter, hwCapture) < 0) {
        AudioMemFree((void **)&hwCapture);
        return HDF_FAILURE;
    }
    *capture = &hwCapture->common;
    return HDF_SUCCESS;
}

int32_t AudioProxyAdapterDestroyCapture(struct AudioAdapter *adapter, struct AudioCapture *capture)
{
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    if (adapter == NULL || capture == NULL) {
        return HDF_FAILURE;
    }
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)capture;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture((AudioHandle)capture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_DESTROY, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("Send Server fail!");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    AudioMemFree((void **)&hwCapture->captureParam.frameCaptureMode.buffer);
    AudioMemFree((void **)&capture);
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}
int32_t AudioProxyAdapterWritePortCapability(const struct AudioHwAdapter *hwAdapter,
    const struct AudioPort *port, struct HdfSBuf *data)
{
    if (hwAdapter == NULL || port == NULL || data == NULL) {
        return HDF_FAILURE;
    }
    if (hwAdapter->adapterDescriptor.adapterName == NULL) {
        return HDF_FAILURE;
    }
    const char *adapterName = hwAdapter->adapterDescriptor.adapterName;
    if (!HdfSbufWriteString(data, adapterName)) {
        return HDF_FAILURE;
    }
    uint32_t tempDir = (uint32_t)port->dir;
    if (!HdfSbufWriteUint32(data, tempDir)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, port->portId)) {
        return HDF_FAILURE;
    }
    if (port->portName == NULL) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteString(data, port->portName)) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioProxyAdapterGetPortCapability(struct AudioAdapter *adapter,
    const struct AudioPort *port, struct AudioPortCapability *capability)
{
    LOG_FUN_INFO();
    if (adapter == NULL || port == NULL || port->portName == NULL || capability == NULL) {
        return HDF_FAILURE;
    }
    if (port->portId < 0) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    if (AudioProxyPreprocessSBuf(&data, &reply) < 0) {
        return HDF_FAILURE;
    }
    struct AudioHwAdapter *hwAdapter = (struct AudioHwAdapter *)adapter;
    if (hwAdapter == NULL || hwAdapter->proxyRemoteHandle == NULL) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    if (AudioProxyAdapterWritePortCapability(hwAdapter, port, data)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwAdapter->proxyRemoteHandle, AUDIO_HDI_ADT_GET_PORT_CAPABILITY, data, reply);
    if (ret < 0) {
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    LOG_FUN_INFO();
    AudioProxyBufReplyRecycle(data, reply);
    /* proxy must init local capability ,this capability the same of Server's */
    struct AudioPortAndCapability *hwAdapterPortCapabilitys = hwAdapter->portCapabilitys;
    if (hwAdapterPortCapabilitys == NULL) {
        LOG_FUN_ERR("hwAdapter portCapabilitys is NULL!");
        return HDF_FAILURE;
    }
    int32_t portNum = hwAdapter->adapterDescriptor.portNum;
    while (hwAdapterPortCapabilitys != NULL && (portNum > 0)) {
        if (hwAdapterPortCapabilitys->port.portId == port->portId) {
            *capability = hwAdapterPortCapabilitys->capability;
            return HDF_SUCCESS;
        }
        hwAdapterPortCapabilitys++;
        portNum--;
    }
    return HDF_FAILURE;
}

int32_t AudioProxyAdapterSetAndGetPassthroughModeSBuf(struct HdfSBuf *data,
    const struct HdfSBuf *reply, const struct AudioPort *port)
{
    if (data == NULL || port == NULL || port->portName == NULL) {
        return HDF_FAILURE;
    }
    uint32_t tempDir = port->dir;
    if (!HdfSbufWriteUint32(data, tempDir)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, port->portId)) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteString(data, port->portName)) {
        LOG_FUN_ERR("HdfSbufWriteString error");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioProxyAdapterSetPassthroughMode(struct AudioAdapter *adapter,
    const struct AudioPort *port, enum AudioPortPassthroughMode mode)
{
    LOG_FUN_INFO();
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    if (adapter == NULL || port == NULL || port->portName == NULL) {
        return HDF_FAILURE;
    }
    if (port->dir != PORT_OUT || port->portId < 0 || strcmp(port->portName, "AOP") != 0) {
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessSBuf(&data, &reply) < 0) {
        LOG_FUN_ERR("AudioProxyPreprocessSBuf Fail");
        return HDF_FAILURE;
    }
    struct AudioHwAdapter *hwAdapter = (struct AudioHwAdapter *)adapter;
    if (hwAdapter == NULL || hwAdapter->proxyRemoteHandle == NULL ||
        hwAdapter->adapterDescriptor.adapterName == NULL) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    const char *adapterName = hwAdapter->adapterDescriptor.adapterName;
    if (!HdfSbufWriteString(data, adapterName)) {
        LOG_FUN_ERR("adapterName Write Fail");
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    if (AudioProxyAdapterSetAndGetPassthroughModeSBuf(data, reply, port) < 0) {
        LOG_FUN_ERR("Failed to obtain data");
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    uint32_t tempMode = (uint32_t)mode;
    if (!HdfSbufWriteUint32(data, tempMode)) {
        LOG_FUN_ERR("Mode Write Fail");
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwAdapter->proxyRemoteHandle, AUDIO_HDI_ADT_SET_PASS_MODE, data, reply);
    if (ret < 0) {
        AudioProxyBufReplyRecycle(data, reply);
        LOG_FUN_ERR("Failed to send server");
        return ret;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyAdapterGetPassthroughMode(struct AudioAdapter *adapter,
    const struct AudioPort *port, enum AudioPortPassthroughMode *mode)
{
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    if (adapter == NULL || port == NULL || port->portName == NULL || mode == NULL) {
        return HDF_FAILURE;
    }
    if (port->dir != PORT_OUT || port->portId < 0 || strcmp(port->portName, "AOP") != 0) {
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessSBuf(&data, &reply) < 0) {
        return HDF_FAILURE;
    }
    struct AudioHwAdapter *hwAdapter = (struct AudioHwAdapter *)adapter;
    if (hwAdapter == NULL || hwAdapter->proxyRemoteHandle == NULL) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    if (hwAdapter->adapterDescriptor.adapterName == NULL) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    const char *adapterName = hwAdapter->adapterDescriptor.adapterName;
    if (!HdfSbufWriteString(data, adapterName)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    if (AudioProxyAdapterSetAndGetPassthroughModeSBuf(data, reply, port) < 0) {
        LOG_FUN_ERR("Failed to obtain data");
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwAdapter->proxyRemoteHandle, AUDIO_HDI_ADT_GET_PASS_MODE, data, reply);
    if (ret < 0) {
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    uint32_t tempMode = 0;
    if (!HdfSbufReadUint32(reply, &tempMode)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    *mode = (enum AudioPortPassthroughMode)tempMode;
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

