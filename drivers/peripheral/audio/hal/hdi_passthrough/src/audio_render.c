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
#include "audio_interface_lib_render.h"
#include "audio_render.h"

#define CONFIG_OUT_LATENCY_MS  100 // unit: ms

/* 1 buffer: 8000(8kHz sample rate) * 2(bytes, PCM_16_BIT) * 1(channel) */
/* 1 frame: 1024(sample) * 2(bytes, PCM_16_BIT) * 1(channel) */
#define CONFIG_FRAME_SIZE      (1024 * 2 * 1)
#define FRAME_SIZE              1024
#define CONFIG_FRAME_COUNT     ((8000 * 2 * 1 + (CONFIG_FRAME_SIZE - 1)) / CONFIG_FRAME_SIZE)

#define DEEP_BUFFER_PLATFORM_DELAY (29*1000LL)
#define LOW_LATENCY_PLATFORM_DELAY (13*1000LL)

int32_t PcmBytesToFrames(const struct AudioFrameRenderMode *frameRenderMode, uint64_t bytes, uint32_t *frameCount)
{
    if (frameRenderMode == NULL || frameCount == NULL) {
        return HDF_SUCCESS;
    }
    uint32_t formatBits = 0;
    int32_t ret = FormatToBits(frameRenderMode->attrs.format, &formatBits);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    uint32_t frameSize = frameRenderMode->attrs.channelCount * (formatBits >> 3); // Bit to byte >> 3
    if (frameSize == 0) {
        return HDF_FAILURE;
    }
    *frameCount = bytes / frameSize;
    return HDF_SUCCESS;
}

int32_t AudioRenderStart(AudioHandle handle)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    if (hwRender->renderParam.frameRenderMode.buffer != NULL) {
        LOG_FUN_ERR("AudioRender already start!");
        return HDF_FAILURE;
    }
    if (hwRender->devDataHandle == NULL) {
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam,
                                             AUDIO_DRV_PCM_IOCTRL_START);
    if (ret < 0) {
        LOG_FUN_ERR("AudioRenderStart SetParams FAIL");
        return HDF_FAILURE;
    }
    char *buffer = (char *)calloc(1, FRAME_DATA);
    if (buffer == NULL) {
        LOG_FUN_ERR("Calloc Render buffer Fail!");
        return HDF_FAILURE;
    }
    hwRender->renderParam.frameRenderMode.buffer = buffer;
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Render Start");
    return HDF_SUCCESS;
}

int32_t AudioRenderStop(AudioHandle handle)
{
    LOG_FUN_INFO();
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        return HDF_FAILURE;
    }
    if (hwRender->renderParam.frameRenderMode.buffer != NULL) {
        AudioMemFree((void **)&hwRender->renderParam.frameRenderMode.buffer);
    } else {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (hwRender->devDataHandle == NULL) {
        LOG_FUN_ERR("RenderStart Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam,
                                             AUDIO_DRV_PCM_IOCTRL_STOP);
    if (ret < 0) {
        LOG_FUN_ERR("AudioRenderStart SetParams FAIL");
        return HDF_FAILURE;
    }
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Render Stop");
    return HDF_SUCCESS;
}

int32_t AudioRenderPause(AudioHandle handle)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        return HDF_FAILURE;
    }
    if (hwRender->renderParam.frameRenderMode.buffer == NULL) {
        LOG_FUN_ERR("AudioRender already stop!");
        return HDF_FAILURE;
    }
    if (hwRender->renderParam.renderMode.ctlParam.pause) {
        LOG_FUN_ERR("Audio is already pause!");
        return HDF_FAILURE;
    }
    if (hwRender->devDataHandle == NULL) {
        LOG_FUN_ERR("RenderPause Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    bool pauseStatus = hwRender->renderParam.renderMode.ctlParam.pause;
    hwRender->renderParam.renderMode.ctlParam.pause = true;
    int32_t ret = (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam,
                                             AUDIODRV_CTL_IOCTL_PAUSE_WRITE);
    if (ret < 0) {
        LOG_FUN_ERR("RenderPause FAIL!");
        hwRender->renderParam.renderMode.ctlParam.pause = pauseStatus;
        return HDF_FAILURE;
    }
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Render Pause");
    return HDF_SUCCESS;
}

int32_t AudioRenderResume(AudioHandle handle)
{
    LOG_FUN_INFO();
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        return HDF_FAILURE;
    }
    if (!hwRender->renderParam.renderMode.ctlParam.pause) {
        LOG_FUN_ERR("Audio is already Resume !");
        return HDF_FAILURE;
    }
    if (hwRender->devDataHandle == NULL) {
        LOG_FUN_ERR("RenderResume Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    bool resumeStatus = hwRender->renderParam.renderMode.ctlParam.pause;
    hwRender->renderParam.renderMode.ctlParam.pause = false;
    int32_t ret = (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam,
                                             AUDIODRV_CTL_IOCTL_PAUSE_WRITE);
    if (ret < 0) {
        LOG_FUN_ERR("RenderResume FAIL!");
        hwRender->renderParam.renderMode.ctlParam.pause = resumeStatus;
        return HDF_FAILURE;
    }
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Render Resume");
    return HDF_SUCCESS;
}

int32_t AudioRenderFlush(AudioHandle handle)
{
    LOG_FUN_INFO();
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        return HDF_FAILURE;
    }
    return HDF_ERR_NOT_SUPPORT;
}

int32_t AudioRenderGetFrameSize(AudioHandle handle, uint64_t *size)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || size == NULL) {
        return HDF_FAILURE;
    }
    uint32_t channelCount = hwRender->renderParam.frameRenderMode.attrs.channelCount;
    enum AudioFormat format = hwRender->renderParam.frameRenderMode.attrs.format;
    uint32_t formatBits = 0;
    int32_t ret = FormatToBits(format, &formatBits);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    *size = FRAME_SIZE * channelCount * (formatBits >> 3);
    return HDF_SUCCESS;
}

int32_t AudioRenderGetFrameCount(AudioHandle handle, uint64_t *count)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || count == NULL) {
        return HDF_FAILURE;
    }
    *count = hwRender->renderParam.frameRenderMode.frames;
    return HDF_SUCCESS;
}

int32_t AudioRenderSetSampleAttributes(AudioHandle handle, const struct AudioSampleAttributes *attrs)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || attrs == NULL || hwRender->devDataHandle == NULL) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioCheckParaAttr(attrs);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    /* attrs temp */
    struct AudioSampleAttributes tempAttrs = hwRender->renderParam.frameRenderMode.attrs;
    hwRender->renderParam.frameRenderMode.attrs.format = attrs->format;
    hwRender->renderParam.frameRenderMode.attrs.sampleRate = attrs->sampleRate;
    hwRender->renderParam.frameRenderMode.attrs.channelCount = attrs->channelCount;
    hwRender->renderParam.frameRenderMode.attrs.interleaved = attrs->interleaved;
    hwRender->renderParam.frameRenderMode.attrs.type = attrs->type;
    hwRender->renderParam.frameRenderMode.attrs.period = attrs->period;
    hwRender->renderParam.frameRenderMode.attrs.frameSize = attrs->frameSize;
    hwRender->renderParam.frameRenderMode.attrs.isBigEndian = attrs->isBigEndian;
    hwRender->renderParam.frameRenderMode.attrs.isSignedData = attrs->isSignedData;
    hwRender->renderParam.frameRenderMode.attrs.startThreshold = attrs->startThreshold;
    hwRender->renderParam.frameRenderMode.attrs.stopThreshold = attrs->stopThreshold;
    hwRender->renderParam.frameRenderMode.attrs.silenceThreshold = attrs->silenceThreshold;
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL || hwRender->devDataHandle == NULL) {
        hwRender->renderParam.frameRenderMode.attrs = tempAttrs;
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    ret = (*pInterfaceLibModeRender)(hwRender->devDataHandle,
                                             &hwRender->renderParam,
                                             AUDIO_DRV_PCM_IOCTL_HW_PARAMS);
    if (ret < 0) {
        LOG_FUN_ERR("SetSampleAttributes FAIL");
        hwRender->renderParam.frameRenderMode.attrs = tempAttrs;
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderGetSampleAttributes(AudioHandle handle, struct AudioSampleAttributes *attrs)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || attrs == NULL) {
        return HDF_FAILURE;
    }
    attrs->format = hwRender->renderParam.frameRenderMode.attrs.format;
    attrs->sampleRate = hwRender->renderParam.frameRenderMode.attrs.sampleRate;
    attrs->channelCount = hwRender->renderParam.frameRenderMode.attrs.channelCount;
    attrs->type = hwRender->renderParam.frameRenderMode.attrs.type;
    attrs->interleaved = hwRender->renderParam.frameRenderMode.attrs.interleaved;
    attrs->period = hwRender->renderParam.frameRenderMode.attrs.period;
    attrs->frameSize = hwRender->renderParam.frameRenderMode.attrs.frameSize;
    attrs->isBigEndian = hwRender->renderParam.frameRenderMode.attrs.isBigEndian;
    attrs->isSignedData = hwRender->renderParam.frameRenderMode.attrs.isSignedData;
    attrs->startThreshold = hwRender->renderParam.frameRenderMode.attrs.startThreshold;
    attrs->stopThreshold = hwRender->renderParam.frameRenderMode.attrs.stopThreshold;
    attrs->silenceThreshold = hwRender->renderParam.frameRenderMode.attrs.silenceThreshold;
    return HDF_SUCCESS;
}

int32_t AudioRenderGetCurrentChannelId(AudioHandle handle, uint32_t *channelId)
{
    LOG_FUN_INFO();
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || channelId == NULL) {
        return HDF_FAILURE;
    }
    *channelId = hwRender->renderParam.frameRenderMode.attrs.channelCount;
    return HDF_SUCCESS;
}

int32_t AudioRenderCheckSceneCapability(AudioHandle handle, const struct AudioSceneDescriptor *scene,
                                        bool *supported)
{
    LOG_FUN_INFO();
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || scene == NULL || supported == NULL) {
        return HDF_FAILURE;
    }
#ifndef AUDIO_HAL_NOTSUPPORT_PATHSELECT
    *supported = false;
    /* Temporary storage does not save the structure */
    struct AudioHwRenderParam renderParam;
    renderParam.frameRenderMode.attrs.type = (enum AudioCategory)scene->scene.id;
    renderParam.renderMode.hwInfo.deviceDescript.pins = scene->desc.pins;
    PathSelAnalysisJson *pPathSelAnalysisJson = AudioSoGetPathSelAnalysisJson();
    if (pPathSelAnalysisJson == NULL) {
        LOG_FUN_ERR("pPathSelAnalysisJson Is NULL!");
        return HDF_ERR_NOT_SUPPORT;
    }
    int ret = (*pPathSelAnalysisJson)((void *)&renderParam, CHECKSCENE_PATH_SELECT);
    if (ret < 0) {
        if (ret == HDF_ERR_NOT_SUPPORT) {
            LOG_FUN_ERR("AudioRenderCheckSceneCapability not Support!");
            return HDF_ERR_NOT_SUPPORT;
        } else {
            LOG_FUN_ERR("AudioRenderCheckSceneCapability fail!");
            return HDF_FAILURE;
        }
    }
    *supported = true;
    return HDF_SUCCESS;
#else
    return HDF_ERR_NOT_SUPPORT;
#endif
}

int32_t AudioRenderSelectScene(AudioHandle handle, const struct AudioSceneDescriptor *scene)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL || scene == NULL) {
        return HDF_FAILURE;
    }
    if (hwRender->devCtlHandle == NULL) {
        LOG_FUN_ERR("RenderSelectScene Bind Fail!");
        return HDF_FAILURE;
    }
#ifndef AUDIO_HAL_NOTSUPPORT_PATHSELECT
    PathSelAnalysisJson *pPathSelAnalysisJson = AudioSoGetPathSelAnalysisJson();
    if (pPathSelAnalysisJson == NULL) {
        LOG_FUN_ERR("pPathSelAnalysisJson Is NULL!");
        return HDF_ERR_NOT_SUPPORT;
    }
    enum AudioCategory sceneId = hwRender->renderParam.frameRenderMode.attrs.type;
    enum AudioPortPin descPins = hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins;
    hwRender->renderParam.frameRenderMode.attrs.type = (enum AudioCategory)(scene->scene.id);
    hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins = scene->desc.pins;
    if ((*pPathSelAnalysisJson)((void *)&hwRender->renderParam, RENDER_PATH_SELECT) < 0) {
        LOG_FUN_ERR("AudioRenderSelectScene Fail!");
        hwRender->renderParam.frameRenderMode.attrs.type = sceneId;
        hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins = descPins;
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        hwRender->renderParam.frameRenderMode.attrs.type = sceneId;
        hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins = descPins;
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeRender)(hwRender->devCtlHandle, &hwRender->renderParam,
                                             AUDIODRV_CTL_IOCTL_SCENESELECT_WRITE);
    if (ret < 0) {
        LOG_FUN_ERR("SetParams FAIL!");
        hwRender->renderParam.frameRenderMode.attrs.type = sceneId;
        hwRender->renderParam.renderMode.hwInfo.deviceDescript.pins = descPins;
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
#else
    return HDF_ERR_NOT_SUPPORT;
#endif
}

int32_t AudioRenderSetMute(AudioHandle handle, bool mute)
{
    struct AudioHwRender *impl = (struct AudioHwRender *)handle;
    if (impl == NULL) {
        return HDF_FAILURE;
    }
    if (impl->devCtlHandle == NULL) {
        LOG_FUN_ERR("RenderSetMute Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    bool muteStatus = impl->renderParam.renderMode.ctlParam.mute;
    impl->renderParam.renderMode.ctlParam.mute = mute;
    int32_t ret = (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_MUTE_WRITE);
    if (ret < 0) {
        LOG_FUN_ERR("SetMute SetParams FAIL");
        impl->renderParam.renderMode.ctlParam.mute = muteStatus;
        return HDF_FAILURE;
    }
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [Setmute = %d]", __FILE__, __func__, __LINE__, mute);
    return HDF_SUCCESS;
}

int32_t AudioRenderGetMute(AudioHandle handle, bool *mute)
{
    struct AudioHwRender *impl = (struct AudioHwRender *)handle;
    if (impl == NULL || mute == NULL) {
        return HDF_FAILURE;
    }

    if (impl->devCtlHandle == NULL) {
        LOG_FUN_ERR("RenderGetMute Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_MUTE_READ);
    if (ret < 0) {
        LOG_FUN_ERR("Get Mute FAIL!");
        return HDF_FAILURE;
    }
    *mute = impl->renderParam.renderMode.ctlParam.mute;
    LOG_PARA_INFO("GetMute SUCCESS!");
    return HDF_SUCCESS;
}

int32_t AudioRenderSetVolume(AudioHandle handle, float volume)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        return HDF_FAILURE;
    }
    float volumeTemp = hwRender->renderParam.renderMode.ctlParam.volume;
    float volMax = (float)hwRender->renderParam.renderMode.ctlParam.volThreshold.volMax;
    float volMin = (float)hwRender->renderParam.renderMode.ctlParam.volThreshold.volMin;
    if (volume < 0 || volume > 1) {
        LOG_FUN_ERR("volume param Is error!");
        return HDF_FAILURE;
    }
    if (hwRender->devCtlHandle == NULL) {
        LOG_FUN_ERR("RenderSetVolume Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    volume = (volume == 0) ? 1 : (volume * VOLUME_CHANGE);
    /* change volume to db */
    float volTemp = ((volMax - volMin) / 2) * log10(volume) + volMin;
    if (volTemp < volMin || volTemp > volMax) {
        LOG_FUN_ERR("volTemp fail");
        return HDF_FAILURE;
    }
    hwRender->renderParam.renderMode.ctlParam.volume = volTemp;
    int32_t ret = (*pInterfaceLibModeRender)(hwRender->devCtlHandle, &hwRender->renderParam,
                                             AUDIODRV_CTL_IOCTL_ELEM_WRITE);
    if (ret < 0) {
        LOG_FUN_ERR("RenderSetVolume FAIL!");
        hwRender->renderParam.renderMode.ctlParam.volume = volumeTemp;
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderGetVolume(AudioHandle handle, float *volume)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (NULL == hwRender || NULL == volume) {
        return HDF_FAILURE;
    }
    if (hwRender->devCtlHandle == NULL) {
        LOG_FUN_ERR("RenderGetVolume Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    int ret = (*pInterfaceLibModeRender)(hwRender->devCtlHandle, &hwRender->renderParam, AUDIODRV_CTL_IOCTL_ELEM_READ);
    if (ret < 0) {
        LOG_FUN_ERR("RenderGetVolume FAIL!");
        return HDF_FAILURE;
    }
    float volumeTemp = hwRender->renderParam.renderMode.ctlParam.volume;
    float volMax = (float)hwRender->renderParam.renderMode.ctlParam.volThreshold.volMax;
    float volMin = (float)hwRender->renderParam.renderMode.ctlParam.volThreshold.volMin;
    if ((volMax - volMin) == 0) {
        LOG_FUN_ERR("Divisor cannot be zero!");
        return HDF_FAILURE;
    }
    volumeTemp = (volumeTemp - volMin) / ((volMax - volMin) / 2);
    int volumeT = (int)((pow(10, volumeTemp) + 5) / 10); // delet 0.X num
    *volume = (float)volumeT / 10;  // get volume (0-1)
    return HDF_SUCCESS;
}

int32_t AudioRenderGetGainThreshold(AudioHandle handle, float *min, float *max)
{
    LOG_FUN_INFO();
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (NULL == hwRender || NULL == min || NULL == max) {
        return HDF_FAILURE;
    }
    if (hwRender->devCtlHandle == NULL) {
        LOG_FUN_ERR("RenderGetGainThreshold Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeRender)(hwRender->devCtlHandle, &hwRender->renderParam,
                                             AUDIODRV_CTL_IOCTL_GAINTHRESHOLD_READ);
    if (ret < 0) {
        LOG_FUN_ERR("SetParams FAIL!");
        return HDF_FAILURE;
    }
    *max = hwRender->renderParam.renderMode.ctlParam.audioGain.gainMax;
    *min = hwRender->renderParam.renderMode.ctlParam.audioGain.gainMin;
    return HDF_SUCCESS;
}

int32_t AudioRenderGetGain(AudioHandle handle, float *gain)
{
    LOG_FUN_INFO();
    struct AudioHwRender *impl = (struct AudioHwRender *)handle;
    if (impl == NULL || gain == NULL) {
        return HDF_FAILURE;
    }
    if (impl->devCtlHandle == NULL) {
        LOG_FUN_ERR("RenderGetGain Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_GAIN_READ);
    if (ret < 0) {
        LOG_FUN_ERR("RenderGetGain FAIL");
        return HDF_FAILURE;
    }
    *gain = impl->renderParam.renderMode.ctlParam.audioGain.gain;
    LOG_PARA_INFO("RenderGetGain SUCCESS!");
    return HDF_SUCCESS;
}

int32_t AudioRenderSetGain(AudioHandle handle, float gain)
{
    LOG_FUN_INFO();
    struct AudioHwRender *impl = (struct AudioHwRender *)handle;
    if (impl == NULL) {
        return HDF_FAILURE;
    }
    float gainTemp = impl->renderParam.renderMode.ctlParam.audioGain.gain;
    impl->renderParam.renderMode.ctlParam.audioGain.gain = gain;
    if (impl->devCtlHandle == NULL) {
        LOG_FUN_ERR("RenderSetGain Bind Fail!");
        impl->renderParam.renderMode.ctlParam.audioGain.gain = gainTemp;
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        impl->renderParam.renderMode.ctlParam.audioGain.gain = gainTemp;
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_GAIN_WRITE);
    if (ret < 0) {
        LOG_FUN_ERR("RenderSetGain FAIL");
        impl->renderParam.renderMode.ctlParam.audioGain.gain = gainTemp;
        return HDF_FAILURE;
    }
    LOG_PARA_INFO("RenderSetGain SUCCESS!");
    return HDF_SUCCESS;
}

int32_t AudioRenderGetLatency(struct AudioRender *render, uint32_t *ms)
{
    struct AudioHwRender *impl = (struct AudioHwRender *)render;
    if (impl == NULL || ms == NULL) {
        return HDF_FAILURE;
    }
    uint32_t byteRate = impl->renderParam.frameRenderMode.byteRate;
    uint32_t periodSize = impl->renderParam.frameRenderMode.periodSize;
    uint32_t periodCount = impl->renderParam.frameRenderMode.periodCount;
    if (byteRate == 0) {
        return HDF_FAILURE;
    }
    uint32_t period_ms = (periodCount * periodSize * 1000) / byteRate;
    *ms = period_ms;
    return HDF_SUCCESS;
}

void LogError(AudioHandle handle, int32_t errorCode, int reason)
{
    struct AudioHwRender *hwRender = (struct AudioHwRender *)handle;
    if (hwRender == NULL) {
        return;
    }
    hwRender->errorLog.totalErrors++;
    if (hwRender->errorLog.iter >= ERROR_LOG_MAX_NUM) {
        hwRender->errorLog.iter = 0;
    }
    char reasonDesc[ERROR_REASON_DESC_LEN] = {0};
    int32_t ret = GetErrorReason(reason, &reasonDesc);
    if (ret < 0) {
        LOG_FUN_ERR("GetErrorReason failed!");
        return;
    }
    char time[ERROR_REASON_DESC_LEN] = {0};
    ret = GetCurrentTime(&time);
    if (ret < 0) {
        LOG_FUN_ERR("GetCurrentTime Fail");
        return;
    }
    if (errorCode == WRITE_FRAME_ERROR_CODE) {
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].errorCode = errorCode;
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].count = hwRender->errorLog.iter;
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].frames = hwRender->renderParam.frameRenderMode.frames;
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].reason = reasonDesc;
        hwRender->errorLog.errorDump[hwRender->errorLog.iter].currentTime = time;
        hwRender->errorLog.iter++;
    }
}

int32_t AudioRenderRenderFramSplit(struct AudioHwRender *hwRender)
{
    int32_t ret;
    if (hwRender == NULL) {
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    if (hwRender->devDataHandle == NULL) {
        return HDF_FAILURE;
    }
    ret = (*pInterfaceLibModeRender)(hwRender->devDataHandle, &hwRender->renderParam,
                                             AUDIO_DRV_PCM_IOCTL_WRITE);
    if (ret < 0) {
        LOG_FUN_ERR("Render Frame FAIL!");
        LogError((AudioHandle)hwRender, WRITE_FRAME_ERROR_CODE, ret);
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderRenderFrame(struct AudioRender *render, const void *frame,
                               uint64_t requestBytes, uint64_t *replyBytes)
{
    LOG_FUN_INFO();
    struct AudioHwRender *hwRender = (struct AudioHwRender *)render;
    if (hwRender == NULL || frame == NULL || replyBytes == NULL ||
        hwRender->renderParam.frameRenderMode.buffer == NULL) {
        LOG_FUN_ERR("Render Frame Paras is NULL!");
        return HDF_FAILURE;
    }
    if (FRAME_DATA < requestBytes) {
        LOG_FUN_ERR("Out of FRAME_DATA size!");
        return HDF_FAILURE;
    }
    int32_t ret = memcpy_s(hwRender->renderParam.frameRenderMode.buffer, FRAME_DATA, frame, (uint32_t)requestBytes);
    if (ret != EOK) {
        LOG_FUN_ERR("memcpy_s fail");
        return HDF_FAILURE;
    }
    hwRender->renderParam.frameRenderMode.bufferSize = requestBytes;
    uint32_t frameCount = 0;
    ret = PcmBytesToFrames(&hwRender->renderParam.frameRenderMode, requestBytes, &frameCount);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    hwRender->renderParam.frameRenderMode.bufferFrameSize = (uint64_t)frameCount;
    if (AudioRenderRenderFramSplit(hwRender) < 0) {
        return HDF_FAILURE;
    }
    *replyBytes = requestBytes;
    hwRender->renderParam.frameRenderMode.frames += hwRender->renderParam.frameRenderMode.bufferFrameSize;
    if (hwRender->renderParam.frameRenderMode.attrs.sampleRate == 0) {
        LOG_FUN_ERR("Divisor cannot be zero!");
        return HDF_FAILURE;
    }
    if (TimeToAudioTimeStamp(hwRender->renderParam.frameRenderMode.bufferFrameSize,
        &hwRender->renderParam.frameRenderMode.time,
        hwRender->renderParam.frameRenderMode.attrs.sampleRate) == HDF_FAILURE) {
        LOG_FUN_ERR("Frame is NULL");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderGetRenderPosition(struct AudioRender *render, uint64_t *frames, struct AudioTimeStamp *time)
{
    struct AudioHwRender *impl = (struct AudioHwRender *)render;
    if (impl == NULL || frames == NULL || time == NULL) {
        return HDF_FAILURE;
    }
    *frames = impl->renderParam.frameRenderMode.frames;
    *time = impl->renderParam.frameRenderMode.time;
    return HDF_SUCCESS;
}

int32_t AudioRenderSetRenderSpeed(struct AudioRender *render, float speed)
{
    LOG_FUN_INFO();
    struct AudioHwRender *hwRender = (struct AudioHwRender *)render;
    if (hwRender == NULL) {
        return HDF_FAILURE;
    }
    return HDF_ERR_NOT_SUPPORT;
}

int32_t AudioRenderGetRenderSpeed(struct AudioRender *render, float *speed)
{
    LOG_FUN_INFO();
    struct AudioHwRender *hwRender = (struct AudioHwRender *)render;
    if (hwRender == NULL || speed == NULL) {
        return HDF_FAILURE;
    }
    return HDF_ERR_NOT_SUPPORT;
}

int32_t AudioRenderSetChannelMode(struct AudioRender *render, enum AudioChannelMode mode)
{
    LOG_FUN_INFO();
    struct AudioHwRender *impl = (struct AudioHwRender *)render;
    if (impl == NULL) {
        return HDF_FAILURE;
    }
    if (impl->devCtlHandle == NULL) {
        LOG_FUN_ERR("Bind Fail!");
        return HDF_FAILURE;
    }
    enum AudioChannelMode tempMode = impl->renderParam.frameRenderMode.mode;
    impl->renderParam.frameRenderMode.mode = mode;
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        impl->renderParam.frameRenderMode.mode = tempMode;
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam,
                                             AUDIODRV_CTL_IOCTL_CHANNEL_MODE_WRITE);
    if (ret < 0) {
        LOG_FUN_ERR("SetParams FAIL!");
        impl->renderParam.frameRenderMode.mode = tempMode;
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderGetChannelMode(struct AudioRender *render, enum AudioChannelMode *mode)
{
    LOG_FUN_INFO();
    struct AudioHwRender *impl = (struct AudioHwRender *)render;
    if (impl == NULL || mode == NULL || impl->devCtlHandle == NULL) {
        return HDF_FAILURE;
    }
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        return HDF_FAILURE;
    }
    int ret = (*pInterfaceLibModeRender)(impl->devCtlHandle, &impl->renderParam, AUDIODRV_CTL_IOCTL_CHANNEL_MODE_READ);
    if (ret < 0) {
        LOG_FUN_ERR("Get ChannelMode FAIL!");
        return HDF_FAILURE;
    }
    *mode = impl->renderParam.frameRenderMode.mode;
    return HDF_SUCCESS;
}

int32_t SetValue(struct ExtraParams mExtraParams, struct AudioHwRender *render)
{
    if (render == NULL) {
        return HDF_FAILURE;
    }
    if (mExtraParams.route != -1) {
        render->renderParam.renderMode.hwInfo.pathroute = mExtraParams.route;
    }
    if (mExtraParams.format != -1) {
        render->renderParam.frameRenderMode.attrs.format = mExtraParams.format;
    }
    if (mExtraParams.channels != 0) {
        render->renderParam.frameRenderMode.attrs.channelCount = mExtraParams.channels;
    }
    if (mExtraParams.flag) {
        render->renderParam.frameRenderMode.frames = mExtraParams.frames;
    }
    if (mExtraParams.sampleRate != 0) {
        render->renderParam.frameRenderMode.attrs.sampleRate = mExtraParams.sampleRate;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderSetExtraParams(AudioHandle handle, const char *keyValueList)
{
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL || keyValueList == NULL) {
        return HDF_FAILURE;
    }

    struct ParamValMap mParamValMap[MAP_MAX];
    int32_t count = 0;
    int32_t ret = KeyValueListToMap(keyValueList, mParamValMap, &count);
    if (ret < 0) {
        LOG_FUN_ERR("Convert to map FAIL!");
        return HDF_FAILURE;
    }
    int index = 0;
    int32_t sumOk = 0;
    struct ExtraParams mExtraParams;
    mExtraParams.route = -1;
    mExtraParams.format = -1;
    mExtraParams.channels = 0;
    mExtraParams.frames = 0;
    mExtraParams.sampleRate = 0;
    mExtraParams.flag = false;
    while (index < count) {
        ret = SetExtParam(mParamValMap[index].key, mParamValMap[index].value, &mExtraParams);
        if (ret < 0) {
            return HDF_FAILURE;
        } else {
            sumOk++;
        }
        index++;
    }
    if (count != 0 && sumOk == count) {
        SetValue(mExtraParams, render);
        return HDF_SUCCESS;
    } else {
        return HDF_FAILURE;
    }
}

int32_t AudioRenderGetExtraParams(AudioHandle handle, char *keyValueList, int32_t listLenth)
{
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL || keyValueList == NULL || listLenth <= 0) {
        return HDF_FAILURE;
    }
    int32_t bufferSize = strlen(ROUTE_SAMPLE) + strlen(FORMAT_SAMPLE) + strlen(CHANNELS_SAMPLE)
                    + strlen(FRAME_COUNT_SAMPLE) + strlen(SAMPLING_RATE_SAMPLE);
    if (listLenth < bufferSize) {
        return HDF_FAILURE;
    }
    int32_t ret = AddElementToList(keyValueList, listLenth, AUDIO_ATTR_PARAM_ROUTE,
        &render->renderParam.renderMode.hwInfo.pathroute);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    ret = AddElementToList(keyValueList, listLenth, AUDIO_ATTR_PARAM_FORMAT,
        &render->renderParam.frameRenderMode.attrs.format);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    ret = AddElementToList(keyValueList, listLenth, AUDIO_ATTR_PARAM_CHANNELS,
        &render->renderParam.frameRenderMode.attrs.channelCount);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    ret = AddElementToList(keyValueList, listLenth, AUDIO_ATTR_PARAM_FRAME_COUNT,
        &render->renderParam.frameRenderMode.frames);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    ret = AddElementToList(keyValueList, listLenth, AUDIO_ATTR_PARAM_SAMPLING_RATE,
        &render->renderParam.frameRenderMode.attrs.sampleRate);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderReqMmapBuffer(AudioHandle handle, int32_t reqSize, struct AudioMmapBufferDescripter *desc)
{
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL || render->devDataHandle == NULL || desc == NULL) {
        return HDF_FAILURE;
    }
    int32_t flags;
    if (desc->isShareable) {
        flags = MAP_SHARED;
    } else {
        flags = MAP_PRIVATE;
    }
    uint32_t formatBits = 0;
    int32_t ret = FormatToBits(render->renderParam.frameRenderMode.attrs.format, &formatBits);
    if (ret < 0) {
        return ret;
    }
    if (reqSize <= 0) {
        return HDF_FAILURE;
    }
    desc->memoryAddress = mmap(NULL, reqSize, PROT_READ | PROT_WRITE, flags, desc->memoryFd, 0);
    if (desc->memoryAddress == NULL || desc->memoryAddress == (void *)-1) {
        LOG_FUN_ERR("AudioRenderReqMmapBuffer mmap FAIL and errno is:%d !", errno);
        return HDF_FAILURE;
    }
    desc->totalBufferFrames = reqSize / (render->renderParam.frameRenderMode.attrs.channelCount * (formatBits >> 3));
    InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
    if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
        munmap(desc->memoryAddress, reqSize);
        return HDF_FAILURE;
    }
    render->renderParam.frameRenderMode.mmapBufDesc.memoryAddress = desc->memoryAddress;
    render->renderParam.frameRenderMode.mmapBufDesc.memoryFd = desc->memoryFd;
    render->renderParam.frameRenderMode.mmapBufDesc.totalBufferFrames = desc->totalBufferFrames;
    render->renderParam.frameRenderMode.mmapBufDesc.transferFrameSize = desc->transferFrameSize;
    render->renderParam.frameRenderMode.mmapBufDesc.isShareable = desc->isShareable;
    render->renderParam.frameRenderMode.mmapBufDesc.offset = desc->offset;
    ret = (*pInterfaceLibModeRender)(render->devDataHandle, &render->renderParam, AUDIO_DRV_PCM_IOCTL_MMAP_BUFFER);
    if (ret < 0) {
        LOG_FUN_ERR("AudioRenderReqMmapBuffer FAIL!");
        munmap(desc->memoryAddress, reqSize);
        return HDF_FAILURE;
    }
    LOG_PARA_INFO("AudioRenderReqMmapBuffer Success!");
    return HDF_SUCCESS;
}

int32_t AudioRenderGetMmapPosition(AudioHandle handle, uint64_t *frames, struct AudioTimeStamp *time)
{
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL || frames == NULL || time == NULL) {
        return HDF_FAILURE;
    }
#ifndef AUDIO_HAL_USER
        InterfaceLibModeRenderSo *pInterfaceLibModeRender = AudioSoGetInterfaceLibModeRender();
        if (pInterfaceLibModeRender == NULL || *pInterfaceLibModeRender == NULL) {
            LOG_FUN_ERR("pInterfaceLibModeRender Is NULL");
            return HDF_FAILURE;
        }
        int ret = (*pInterfaceLibModeRender)(render->devDataHandle,
            &render->renderParam, AUDIO_DRV_PCM_IOCTL_MMAP_POSITION);
        if (ret < 0) {
            LOG_FUN_ERR("Get Position FAIL!");
            return HDF_FAILURE;
        }
#endif
    *frames = render->renderParam.frameRenderMode.frames;
    render->renderParam.frameRenderMode.time.tvSec = render->renderParam.frameRenderMode.frames /
                                       (int64_t)render->renderParam.frameRenderMode.attrs.sampleRate;
    int64_t lastBufFrames = render->renderParam.frameRenderMode.frames %
                        ((int64_t)render->renderParam.frameRenderMode.attrs.sampleRate);
    render->renderParam.frameRenderMode.time.tvNSec =
        (lastBufFrames * SEC_TO_NSEC) / ((int64_t)render->renderParam.frameRenderMode.attrs.sampleRate);
    *time = render->renderParam.frameRenderMode.time;
    return HDF_SUCCESS;
}

int32_t AudioRenderTurnStandbyMode(AudioHandle handle)
{
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioRenderStop((AudioHandle)render);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderAudioDevDump(AudioHandle handle, int32_t range, int32_t fd)
{
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL) {
        return HDF_FAILURE;
    }
    dprintf(fd, "%s%d\n", "Number of errors: ", render->errorLog.totalErrors);
    if (range < RANGE_MIN - 1 || range > RANGE_MAX) {
        dprintf(fd, "%s\n", "Out of range, invalid output");
        return HDF_SUCCESS;
    }
    uint32_t mSize = render->errorLog.iter;
    if (range < RANGE_MIN) {
        dprintf(fd, "%-5s  %-10s  %s\n", "count", "errorCode", "Time");
        for (int i = 0; i < mSize; i++) {
            dprintf(fd, FORMAT_TWO, render->errorLog.errorDump[i].count + 1,
                    render->errorLog.errorDump[i].errorCode,
                    render->errorLog.errorDump[i].currentTime);
        }
    } else {
        dprintf(fd, "%-5s  %-10s  %-20s  %-15s  %s\n", "count", "errorCode", "frames", "fail reason", "Time");
        for (int i = 0; i < mSize; i++) {
            dprintf(fd, FORMAT_ONE, render->errorLog.errorDump[i].count + 1,
                    render->errorLog.errorDump[i].errorCode,
                    render->errorLog.errorDump[i].frames,
                    render->errorLog.errorDump[i].reason,
                    render->errorLog.errorDump[i].currentTime);
        }
    }
    return HDF_SUCCESS;
}
int32_t CallbackProcessing(AudioHandle handle, enum AudioCallbackType callBackType)
{
    struct AudioHwRender *render = (struct AudioHwRender *)handle;
    if (render == NULL) {
        LOG_PARA_INFO("Unregistered callback.\n");
        return HDF_FAILURE;
    }
    if (render->renderParam.frameRenderMode.callback == NULL) {
        return HDF_FAILURE;
    }
    bool isCallBack = true;
    switch (callBackType) {
        case AUDIO_NONBLOCK_WRITE_COMPELETED:
        case AUDIO_DRAIN_COMPELETED:
        case AUDIO_FLUSH_COMPLETED:
        case AUDIO_RENDER_FULL:
        case AUDIO_ERROR_OCCUR:
            isCallBack = true;
            break;
        default:
            isCallBack = false;
            break;
    }
    if (!isCallBack) {
        LOG_PARA_INFO("No callback processing is required.\n");
        return HDF_ERR_NOT_SUPPORT;
    }
    render->renderParam.frameRenderMode.callback(callBackType, NULL, render->renderParam.frameRenderMode.cookie);
    return HDF_SUCCESS;
}

int32_t AudioRenderRegCallback(struct AudioRender *render, RenderCallback callback, void *cookie)
{
    struct AudioHwRender *pRender = (struct AudioHwRender *)render;
    if (pRender == NULL) {
        return HDF_FAILURE;
    }
    pRender->renderParam.frameRenderMode.callback = callback;
    pRender->renderParam.frameRenderMode.cookie = cookie;
    return HDF_SUCCESS;
}

int32_t AudioRenderDrainBuffer(struct AudioRender *render, enum AudioDrainNotifyType *type)
{
    struct AudioHwRender *pRender = (struct AudioHwRender *)render;
    if (pRender == NULL) {
        return HDF_FAILURE;
    }
    return HDF_ERR_NOT_SUPPORT;
}

