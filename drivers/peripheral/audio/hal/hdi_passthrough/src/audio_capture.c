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

#include "audio_capture.h"
#include "audio_interface_lib_capture.h"
#include "audio_internal.h"

#define CONFIG_FRAME_SIZE      (1024 * 2 * 1)
#define FRAME_SIZE              1024

#define CONFIG_FRAME_COUNT     ((8000 * 2 * 1 + (CONFIG_FRAME_SIZE - 1)) / CONFIG_FRAME_SIZE)

/* add For Capture Bytes To Frames */
int32_t FormatToBitsCapture(enum AudioFormat format, uint32_t *formatBits)
{
    LOG_FUN_INFO();
    if (formatBits == NULL) {
        return HDF_FAILURE;
    }
    switch (format) {
        case AUDIO_FORMAT_PCM_32_BIT:
            *formatBits = BIT_NUM_32;
            return HDF_SUCCESS;
        case AUDIO_FORMAT_PCM_24_BIT:
            *formatBits = BIT_NUM_24;
            return HDF_SUCCESS;
        case AUDIO_FORMAT_PCM_16_BIT:
            *formatBits = BIT_NUM_16;
            return HDF_SUCCESS;
        case AUDIO_FORMAT_PCM_8_BIT:
            *formatBits = BIT_NUM_8;
            return HDF_SUCCESS;
        default:
            return HDF_ERR_NOT_SUPPORT;
    }
}

int32_t AudioCaptureStart(AudioHandle handle)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL) {
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    if (hwCapture->captureParam.frameCaptureMode.buffer != NULL) {
        LOG_FUN_ERR("AudioCapture already start!");
        return HDF_FAILURE;
    }
    if (hwCapture->devDataHandle == NULL) {
        LOG_FUN_ERR("CaptureStart Bind Fail!");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeCapture)(hwCapture->devDataHandle, &hwCapture->captureParam,
                                              AUDIO_DRV_PCM_IOCTRL_START_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("AudioCaptureStart SetParams FAIL");
        return HDF_FAILURE;
    }
    char *tbuffer = (char *)calloc(1, FRAME_DATA);
    if (tbuffer == NULL) {
        LOG_FUN_ERR("Calloc Capture tbuffer Fail!");
        return HDF_FAILURE;
    }
    hwCapture->captureParam.frameCaptureMode.buffer = tbuffer;
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Capture Start");
    return HDF_SUCCESS;
}

int32_t AudioCaptureStop(AudioHandle handle)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL) {
        return HDF_FAILURE;
    }
    if (hwCapture->devDataHandle == NULL) {
        LOG_FUN_ERR("CaptureStart Bind Fail!");
        return HDF_FAILURE;
    }
    if (hwCapture->captureParam.frameCaptureMode.buffer != NULL) {
        AudioMemFree((void **)&hwCapture->captureParam.frameCaptureMode.buffer);
    } else {
        return HDF_ERR_INVALID_OBJECT;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeCapture)(hwCapture->devDataHandle, &hwCapture->captureParam,
                                              AUDIO_DRV_PCM_IOCTRL_STOP_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("AudioCaptureStart SetParams FAIL");
        return HDF_FAILURE;
    }
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Capture Stop");
    return HDF_SUCCESS;
}

int32_t AudioCapturePause(AudioHandle handle)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL) {
        return HDF_FAILURE;
    }
    if (hwCapture->captureParam.frameCaptureMode.buffer == NULL) {
        LOG_FUN_ERR("AudioCapture already stop!");
        return HDF_FAILURE;
    }
    if (hwCapture->captureParam.captureMode.ctlParam.pause) {
        LOG_FUN_ERR("Audio capture is already pause!");
        return HDF_FAILURE;
    }
    if (hwCapture->devDataHandle == NULL) {
        LOG_FUN_ERR("CaptureStart Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    bool pauseStatus = hwCapture->captureParam.captureMode.ctlParam.pause;
    hwCapture->captureParam.captureMode.ctlParam.pause = true;
    int32_t ret = (*pInterfaceLibModeCapture)(hwCapture->devDataHandle, &hwCapture->captureParam,
                                              AUDIODRV_CTL_IOCTL_PAUSE_WRITE_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("Audio Capture Pause FAIL!");
        hwCapture->captureParam.captureMode.ctlParam.pause = pauseStatus;
        return HDF_FAILURE;
    }
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Capture Pause");
    return HDF_SUCCESS;
}

int32_t AudioCaptureResume(AudioHandle handle)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL) {
        return HDF_FAILURE;
    }
    if (!hwCapture->captureParam.captureMode.ctlParam.pause) {
        LOG_FUN_ERR("Audio capture is already Resume !");
        return HDF_FAILURE;
    }
    if (hwCapture->devDataHandle == NULL) {
        LOG_FUN_ERR("Capture Start Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    bool resumeStatus = hwCapture->captureParam.captureMode.ctlParam.pause;
    hwCapture->captureParam.captureMode.ctlParam.pause = false;
    int32_t ret = (*pInterfaceLibModeCapture)(hwCapture->devDataHandle, &hwCapture->captureParam,
                                              AUDIODRV_CTL_IOCTL_PAUSE_WRITE_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("Audio capture Pause FAIL!");
        hwCapture->captureParam.captureMode.ctlParam.pause = resumeStatus;
        return HDF_FAILURE;
    }
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [%s]", __FILE__, __func__, __LINE__, "Audio Capture Resume");
    return HDF_SUCCESS;
}

int32_t AudioCaptureFlush(AudioHandle handle)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL) {
        return HDF_FAILURE;
    }
    return HDF_ERR_NOT_SUPPORT;
}

int32_t AudioCaptureGetFrameSize(AudioHandle handle, uint64_t *size)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || size == NULL) {
        return HDF_FAILURE;
    }
    uint32_t channelCount = hwCapture->captureParam.frameCaptureMode.attrs.channelCount;
    enum AudioFormat format = hwCapture->captureParam.frameCaptureMode.attrs.format;
    uint32_t formatBitsCapture = 0;
    int32_t ret = FormatToBitsCapture(format, &formatBitsCapture);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    *size = FRAME_SIZE * channelCount * (formatBitsCapture >> 3);
    return HDF_SUCCESS;
}

int32_t AudioCaptureGetFrameCount(AudioHandle handle, uint64_t *count)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || count == NULL) {
        return HDF_FAILURE;
    }
    *count = hwCapture->captureParam.frameCaptureMode.frames;
    return HDF_SUCCESS;
}

int32_t AudioCaptureSetSampleAttributes(AudioHandle handle, const struct AudioSampleAttributes *attrs)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || attrs == NULL) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioCheckParaAttr(attrs);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    struct AudioSampleAttributes tempAttrs = hwCapture->captureParam.frameCaptureMode.attrs;
    hwCapture->captureParam.frameCaptureMode.attrs.format = attrs->format;
    hwCapture->captureParam.frameCaptureMode.attrs.sampleRate = attrs->sampleRate;
    hwCapture->captureParam.frameCaptureMode.attrs.channelCount = attrs->channelCount;
    hwCapture->captureParam.frameCaptureMode.attrs.interleaved = attrs->interleaved;
    hwCapture->captureParam.frameCaptureMode.attrs.type = attrs->type;
    hwCapture->captureParam.frameCaptureMode.attrs.period = attrs->period;
    hwCapture->captureParam.frameCaptureMode.attrs.frameSize = attrs->frameSize;
    hwCapture->captureParam.frameCaptureMode.attrs.isBigEndian = attrs->isBigEndian;
    hwCapture->captureParam.frameCaptureMode.attrs.isSignedData = attrs->isSignedData;
    hwCapture->captureParam.frameCaptureMode.attrs.startThreshold = attrs->startThreshold;
    hwCapture->captureParam.frameCaptureMode.attrs.stopThreshold = attrs->stopThreshold;
    hwCapture->captureParam.frameCaptureMode.attrs.silenceThreshold = attrs->silenceThreshold;
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        hwCapture->captureParam.frameCaptureMode.attrs = tempAttrs;
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    if (hwCapture->devDataHandle == NULL) {
        hwCapture->captureParam.frameCaptureMode.attrs = tempAttrs;
        return HDF_FAILURE;
    }
    ret = (*pInterfaceLibModeCapture)(hwCapture->devDataHandle, &hwCapture->captureParam,
                                              AUDIO_DRV_PCM_IOCTL_HW_PARAMS);
    if (ret < 0) {
        LOG_FUN_ERR("CaptureSetSampleAttributes FAIL");
        hwCapture->captureParam.frameCaptureMode.attrs = tempAttrs;
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureGetSampleAttributes(AudioHandle handle, struct AudioSampleAttributes *attrs)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || attrs == NULL) {
        return HDF_FAILURE;
    }
    attrs->format = hwCapture->captureParam.frameCaptureMode.attrs.format;
    attrs->sampleRate = hwCapture->captureParam.frameCaptureMode.attrs.sampleRate;
    attrs->channelCount = hwCapture->captureParam.frameCaptureMode.attrs.channelCount;
    attrs->interleaved = hwCapture->captureParam.frameCaptureMode.attrs.interleaved;
    attrs->type = hwCapture->captureParam.frameCaptureMode.attrs.type;
    attrs->period = hwCapture->captureParam.frameCaptureMode.attrs.period;
    attrs->frameSize = hwCapture->captureParam.frameCaptureMode.attrs.frameSize;
    attrs->isBigEndian = hwCapture->captureParam.frameCaptureMode.attrs.isBigEndian;
    attrs->isSignedData = hwCapture->captureParam.frameCaptureMode.attrs.isSignedData;
    attrs->startThreshold = hwCapture->captureParam.frameCaptureMode.attrs.startThreshold;
    attrs->stopThreshold = hwCapture->captureParam.frameCaptureMode.attrs.stopThreshold;
    attrs->silenceThreshold = hwCapture->captureParam.frameCaptureMode.attrs.silenceThreshold;
    return HDF_SUCCESS;
}

int32_t AudioCaptureGetCurrentChannelId(AudioHandle handle, uint32_t *channelId)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || channelId == NULL) {
        return HDF_FAILURE;
    }
    *channelId = hwCapture->captureParam.frameCaptureMode.attrs.channelCount;
    return HDF_SUCCESS;
}

int32_t AudioCaptureCheckSceneCapability(AudioHandle handle, const struct AudioSceneDescriptor *scene,
                                         bool *supported)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || scene == NULL || supported == NULL) {
        return HDF_FAILURE;
    }
#ifndef AUDIO_HAL_NOTSUPPORT_PATHSELECT
    *supported = false;
    /* Temporary storage does not save the structure */
    struct AudioHwCaptureParam captureParam;
    captureParam.frameCaptureMode.attrs.type = (enum AudioCategory)scene->scene.id;
    captureParam.captureMode.hwInfo.deviceDescript.pins = scene->desc.pins;
    PathSelAnalysisJson *pPathSelAnalysisJson = AudioSoGetPathSelAnalysisJson();
    if (pPathSelAnalysisJson == NULL || *pPathSelAnalysisJson == NULL) {
        LOG_FUN_ERR("pPathSelAnalysisJson Is NULL!");
        return HDF_ERR_NOT_SUPPORT;
    }
    int ret = (*pPathSelAnalysisJson)((void *)&captureParam, CHECKSCENE_PATH_SELECT_CAPTURE);
    if (ret < 0) {
        if (ret == HDF_ERR_NOT_SUPPORT) {
            LOG_FUN_ERR("AudioCaptureCheckSceneCapability not Support!");
            return HDF_ERR_NOT_SUPPORT;
        } else {
            LOG_FUN_ERR("AudioCaptureCheckSceneCapability fail!");
            return HDF_FAILURE;
        }
    }
    *supported = true;
    return HDF_SUCCESS;
#else
    return HDF_ERR_NOT_SUPPORT;
#endif
}

int32_t AudioCaptureSelectScene(AudioHandle handle, const struct AudioSceneDescriptor *scene)
{
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || scene == NULL) {
        return HDF_FAILURE;
    }
    if (hwCapture->devCtlHandle == NULL) {
        LOG_FUN_ERR("CaptureSelectScene Bind Fail!");
        return HDF_FAILURE;
    }
#ifndef AUDIO_HAL_NOTSUPPORT_PATHSELECT
    PathSelAnalysisJson *pPathSelAnalysisJson = AudioSoGetPathSelAnalysisJson();
    if (pPathSelAnalysisJson == NULL || *pPathSelAnalysisJson == NULL) {
        LOG_FUN_ERR("pPathSelAnalysisJson Is NULL!");
        return HDF_ERR_NOT_SUPPORT;
    }
    enum AudioCategory typeTemp = hwCapture->captureParam.frameCaptureMode.attrs.type;
    enum AudioPortPin pinsTemp = hwCapture->captureParam.captureMode.hwInfo.deviceDescript.pins;
    hwCapture->captureParam.frameCaptureMode.attrs.type = (enum AudioCategory)(scene->scene.id);
    hwCapture->captureParam.captureMode.hwInfo.deviceDescript.pins = scene->desc.pins;
    if ((*pPathSelAnalysisJson)((void *)&hwCapture->captureParam, CAPTURE_PATH_SELECT) < 0) {
        LOG_FUN_ERR("AudioCaptureSelectScene Fail!");
        hwCapture->captureParam.frameCaptureMode.attrs.type = typeTemp;
        hwCapture->captureParam.captureMode.hwInfo.deviceDescript.pins = pinsTemp;
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Is NULL");
        hwCapture->captureParam.frameCaptureMode.attrs.type = typeTemp;
        hwCapture->captureParam.captureMode.hwInfo.deviceDescript.pins = pinsTemp;
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeCapture)(hwCapture->devCtlHandle, &hwCapture->captureParam,
                                              AUDIODRV_CTL_IOCTL_SCENESELECT_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("SetSelectSceneParams FAIL!");
        hwCapture->captureParam.frameCaptureMode.attrs.type = typeTemp;
        hwCapture->captureParam.captureMode.hwInfo.deviceDescript.pins = pinsTemp;
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
#else
    return HDF_ERR_NOT_SUPPORT;
#endif
}

int32_t AudioCaptureSetMute(AudioHandle handle, bool mute)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *impl = (struct AudioHwCapture *)handle;
    if (impl == NULL) {
        return HDF_FAILURE;
    }
    if (impl->devCtlHandle == NULL) {
        LOG_FUN_ERR("CaptureSetMute Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    bool muteStatus = impl->captureParam.captureMode.ctlParam.mute;
    impl->captureParam.captureMode.ctlParam.mute = mute;
    int32_t ret = (*pInterfaceLibModeCapture)(impl->devCtlHandle, &impl->captureParam,
                                              AUDIODRV_CTL_IOCTL_MUTE_WRITE_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("SetMute SetParams FAIL");
        impl->captureParam.captureMode.ctlParam.mute = muteStatus;
        return HDF_FAILURE;
    }
    AudioLogRecord(INFO, "[%s]-[%s]-[%d] :> [Setmute = %d]", __FILE__, __func__, __LINE__, mute);
    return HDF_SUCCESS;
}

int32_t AudioCaptureGetMute(AudioHandle handle, bool *mute)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *impl = (struct AudioHwCapture *)handle;
    if (impl == NULL || mute == NULL) {
        return HDF_FAILURE;
    }
    if (impl->devCtlHandle == NULL) {
        LOG_FUN_ERR("CaptureGetMute Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeCapture)(impl->devCtlHandle, &impl->captureParam,
                                              AUDIODRV_CTL_IOCTL_MUTE_READ_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("GetMute SetParams FAIL");
        return HDF_FAILURE;
    }
    *mute = impl->captureParam.captureMode.ctlParam.mute;
    LOG_PARA_INFO("Get Mute SUCCESS!");
    return HDF_SUCCESS;
}

int32_t AudioCaptureSetVolume(AudioHandle handle, float volume)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL) {
        return HDF_FAILURE;
    }
    float volumeTemp = hwCapture->captureParam.captureMode.ctlParam.volume;
    float volMax = (float)hwCapture->captureParam.captureMode.ctlParam.volThreshold.volMax;
    float volMin = (float)hwCapture->captureParam.captureMode.ctlParam.volThreshold.volMin;
    if (hwCapture->devCtlHandle == NULL) {
        LOG_FUN_ERR("Bind Fail!");
        return HDF_FAILURE;
    }
    if (volume < 0 || volume > 1) {
        LOG_FUN_ERR("volume param Is error!");
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    volume = (volume == 0) ? 1 : (volume * VOLUME_CHANGE);
    /* change volume to db */
    float volTemp = ((volMax - volMin) / 2) * log10(volume) + volMin;
    if (volTemp < volMin || volTemp > volMax) {
        LOG_FUN_ERR("volTemp fail");
        return HDF_FAILURE;
    }
    hwCapture->captureParam.captureMode.ctlParam.volume = volTemp;
    int32_t ret = (*pInterfaceLibModeCapture)(hwCapture->devCtlHandle, &hwCapture->captureParam,
                                              AUDIODRV_CTL_IOCTL_ELEM_WRITE_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("SetParams FAIL!");
        hwCapture->captureParam.captureMode.ctlParam.volume = volumeTemp;
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureGetVolume(AudioHandle handle, float *volume)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (NULL == hwCapture || NULL == volume) {
        return HDF_FAILURE;
    }
    if (hwCapture->devCtlHandle == NULL) {
        LOG_FUN_ERR("CaptureStart Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    int ret = (*pInterfaceLibModeCapture)(hwCapture->devCtlHandle, &hwCapture->captureParam,
                                          AUDIODRV_CTL_IOCTL_ELEM_READ_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("Get Volume FAIL!");
        return HDF_FAILURE;
    }
    float volumeTemp = hwCapture->captureParam.captureMode.ctlParam.volume;
    float volMax = (float)hwCapture->captureParam.captureMode.ctlParam.volThreshold.volMax;
    float volMin = (float)hwCapture->captureParam.captureMode.ctlParam.volThreshold.volMin;
    if ((volMax - volMin) == 0) {
        LOG_FUN_ERR("Divisor cannot be zero!");
        return HDF_FAILURE;
    }
    volumeTemp = (volumeTemp - volMin) / ((volMax - volMin) / 2);
    int volumeT = (int)((pow(10, volumeTemp) + 5) / 10); // delet 0.X num
    *volume = (float)volumeT / 10;  // get volume (0-1)
    return HDF_SUCCESS;
}

int32_t AudioCaptureGetGainThreshold(AudioHandle handle, float *min, float *max)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || min == NULL || max == NULL) {
        return HDF_FAILURE;
    }
    if (hwCapture->devCtlHandle == NULL) {
        LOG_FUN_ERR("AudioCaptureGetGainThreshold Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Is NULL");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeCapture)(hwCapture->devCtlHandle, &hwCapture->captureParam,
                                              AUDIODRV_CTL_IOCTL_GAINTHRESHOLD_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("SetParams FAIL!");
        return HDF_FAILURE;
    }
    *max = hwCapture->captureParam.captureMode.ctlParam.audioGain.gainMax;
    *min = hwCapture->captureParam.captureMode.ctlParam.audioGain.gainMin;
    return HDF_SUCCESS;
}

int32_t AudioCaptureGetGain(AudioHandle handle, float *gain)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *impl = (struct AudioHwCapture *)handle;
    if (impl == NULL || gain == NULL) {
        return HDF_FAILURE;
    }
    if (impl->devCtlHandle == NULL) {
        LOG_FUN_ERR("CaptureStart Bind Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeCapture)(impl->devCtlHandle, &impl->captureParam,
                                              AUDIODRV_CTL_IOCTL_GAIN_READ_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("Get Volume FAIL!");
        return HDF_FAILURE;
    }
    *gain = impl->captureParam.captureMode.ctlParam.audioGain.gain;
    return HDF_SUCCESS;
}

int32_t AudioCaptureSetGain(AudioHandle handle, float gain)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *impl = (struct AudioHwCapture *)handle;
    if (impl == NULL) {
        return HDF_FAILURE;
    }
    if (impl->devCtlHandle == NULL) {
        LOG_FUN_ERR("CaptureSetGain Bind Fail!");
        return HDF_FAILURE;
    }
    float gainTemp = impl->captureParam.captureMode.ctlParam.audioGain.gain;
    impl->captureParam.captureMode.ctlParam.audioGain.gain = gain;
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        impl->captureParam.captureMode.ctlParam.audioGain.gain = gainTemp;
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeCapture)(impl->devCtlHandle, &impl->captureParam,
                                              AUDIODRV_CTL_IOCTL_GAIN_WRITE_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("CaptureSetGain FAIL!");
        impl->captureParam.captureMode.ctlParam.audioGain.gain = gainTemp;
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

void LogErrorCapture(AudioHandle handle, int errorCode, int reason)
{
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL) {
        return;
    }
    hwCapture->errorLog.totalErrors++;
    if (hwCapture->errorLog.iter >= ERROR_LOG_MAX_NUM) {
        hwCapture->errorLog.iter = 0;
    }
    char reasonDesc[ERROR_REASON_DESC_LEN] = {0};
    int32_t ret = GetErrorReason(reason, &reasonDesc);
    if (ret < 0) {
        LOG_FUN_ERR("Capture GetErrorReason failed!");
        return;
    }
    char time[ERROR_REASON_DESC_LEN] = {0};
    ret = GetCurrentTime(&time);
    if (ret < 0) {
        LOG_FUN_ERR("GetCurrentTime failed!");
        return;
    }
    if (errorCode == WRITE_FRAME_ERROR_CODE) {
        hwCapture->errorLog.errorDump[hwCapture->errorLog.iter].errorCode = errorCode;
        hwCapture->errorLog.errorDump[hwCapture->errorLog.iter].count = hwCapture->errorLog.iter;
        hwCapture->errorLog.errorDump[hwCapture->errorLog.iter].frames =
            hwCapture->captureParam.frameCaptureMode.frames;
        hwCapture->errorLog.errorDump[hwCapture->errorLog.iter].reason = reasonDesc;
        hwCapture->errorLog.errorDump[hwCapture->errorLog.iter].currentTime = time;
        hwCapture->errorLog.iter++;
    }
}

int32_t AudioCaptureCaptureFrame(struct AudioCapture *capture, void *frame,
                                 uint64_t requestBytes, uint64_t *replyBytes)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)capture;
    if (hwCapture == NULL || frame == NULL || replyBytes == NULL ||
        hwCapture->captureParam.frameCaptureMode.buffer == NULL) {
        LOG_FUN_ERR("Param is NULL Fail!");
        return HDF_FAILURE;
    }
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    if (hwCapture->devDataHandle == NULL) {
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeCapture)(hwCapture->devDataHandle, &hwCapture->captureParam,
                                              AUDIO_DRV_PCM_IOCTL_READ);
    if (ret < 0) {
        LOG_FUN_ERR("Capture Frame FAIL!");
        LogErrorCapture(capture, WRITE_FRAME_ERROR_CODE, ret);
        return HDF_FAILURE;
    }
    if (requestBytes < hwCapture->captureParam.frameCaptureMode.bufferSize) {
        LOG_FUN_ERR("Capture Frame requestBytes too little!");
        return HDF_FAILURE;
    }
    ret = memcpy_s(frame, requestBytes, hwCapture->captureParam.frameCaptureMode.buffer,
        hwCapture->captureParam.frameCaptureMode.bufferSize);
    if (ret != EOK) {
        LOG_FUN_ERR("memcpy_s fail");
        return HDF_FAILURE;
    }
    *replyBytes = hwCapture->captureParam.frameCaptureMode.bufferSize;
    hwCapture->captureParam.frameCaptureMode.frames += hwCapture->captureParam.frameCaptureMode.bufferFrameSize;
    if (hwCapture->captureParam.frameCaptureMode.attrs.sampleRate == 0) {
        LOG_FUN_ERR("Divisor cannot be zero!");
        return HDF_FAILURE;
    }
    if (TimeToAudioTimeStamp(hwCapture->captureParam.frameCaptureMode.bufferFrameSize,
        &hwCapture->captureParam.frameCaptureMode.time,
        hwCapture->captureParam.frameCaptureMode.attrs.sampleRate) == HDF_FAILURE) {
        LOG_FUN_ERR("Frame is NULL");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureGetCapturePosition(struct AudioCapture *capture, uint64_t *frames, struct AudioTimeStamp *time)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *impl = (struct AudioHwCapture *)capture;
    if (impl == NULL || frames == NULL || time == NULL) {
        return HDF_FAILURE;
    }
    *frames = impl->captureParam.frameCaptureMode.frames;
    *time = impl->captureParam.frameCaptureMode.time;
    return HDF_SUCCESS;
}

int32_t SetValueCapture(struct ExtraParams mExtraParams, struct AudioHwCapture *capture)
{
    if (capture == NULL) {
        return HDF_FAILURE;
    }
    if (mExtraParams.route != -1) {
        capture->captureParam.captureMode.hwInfo.pathroute = mExtraParams.route;
    }
    if (mExtraParams.format != -1) {
        capture->captureParam.frameCaptureMode.attrs.format = mExtraParams.format;
    }
    if (mExtraParams.channels != 0) {
        capture->captureParam.frameCaptureMode.attrs.channelCount = mExtraParams.channels;
    }
    if (mExtraParams.flag) {
        capture->captureParam.frameCaptureMode.frames = mExtraParams.frames;
    }
    if (mExtraParams.sampleRate != 0) {
        capture->captureParam.frameCaptureMode.attrs.sampleRate = mExtraParams.sampleRate;
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureSetExtraParams(AudioHandle handle, const char *keyValueList)
{
    struct AudioHwCapture *capture = (struct AudioHwCapture *)handle;
    if (capture == NULL || keyValueList == NULL) {
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
        SetValueCapture(mExtraParams, capture);
        return HDF_SUCCESS;
    } else {
        return HDF_FAILURE;
    }
}

int32_t AudioCaptureGetExtraParams(AudioHandle handle, char *keyValueList, int32_t listLenth)
{
    struct AudioHwCapture *capture = (struct AudioHwCapture *)handle;
    if (capture == NULL || keyValueList == NULL || listLenth <= 0) {
        return HDF_FAILURE;
    }
    int32_t bufferSize = strlen(ROUTE_SAMPLE) + strlen(FORMAT_SAMPLE) + strlen(CHANNELS_SAMPLE)
                    + strlen(FRAME_COUNT_SAMPLE) + strlen(SAMPLING_RATE_SAMPLE);
    if (listLenth < bufferSize) {
        return HDF_FAILURE;
    }
    int32_t ret = AddElementToList(keyValueList, listLenth, AUDIO_ATTR_PARAM_ROUTE,
        &capture->captureParam.captureMode.hwInfo.pathroute);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    ret = AddElementToList(keyValueList, listLenth,
        AUDIO_ATTR_PARAM_FORMAT, &capture->captureParam.frameCaptureMode.attrs.format);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    ret = AddElementToList(keyValueList, listLenth, AUDIO_ATTR_PARAM_CHANNELS,
        &capture->captureParam.frameCaptureMode.attrs.channelCount);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    ret = AddElementToList(keyValueList, listLenth, AUDIO_ATTR_PARAM_FRAME_COUNT,
        &capture->captureParam.frameCaptureMode.frames);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    ret = AddElementToList(keyValueList, listLenth, AUDIO_ATTR_PARAM_SAMPLING_RATE,
        &capture->captureParam.frameCaptureMode.attrs.sampleRate);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureReqMmapBuffer(AudioHandle handle, int32_t reqSize, struct AudioMmapBufferDescripter *desc)
{
    struct AudioHwCapture *capture = (struct AudioHwCapture *)handle;
    if (capture == NULL || capture->devDataHandle == NULL || desc == NULL) {
        return HDF_FAILURE;
    }
    int32_t flags;
    if (desc->isShareable) {
        flags = MAP_SHARED;
    } else {
        flags = MAP_PRIVATE;
    }
    uint32_t formatBits = 0;
    int32_t ret = FormatToBits(capture->captureParam.frameCaptureMode.attrs.format, &formatBits);
    if (ret < 0) {
        return ret;
    }

    desc->memoryAddress = mmap(NULL, reqSize, PROT_READ | PROT_WRITE, flags, desc->memoryFd, 0);
    if (desc->memoryAddress == NULL || desc->memoryAddress == (void *)-1) {
        LOG_FUN_ERR("AudioCaptureReqMmapBuffer mmap FAIL and errno is:%d !", errno);
        return HDF_FAILURE;
    }
    // formatBits Move right 3
    desc->totalBufferFrames =
        reqSize / (capture->captureParam.frameCaptureMode.attrs.channelCount * (formatBits >> 3));
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Is NULL");
        munmap(desc->memoryAddress, reqSize);
        return HDF_FAILURE;
    }
    capture->captureParam.frameCaptureMode.mmapBufDesc.memoryAddress = desc->memoryAddress;
    capture->captureParam.frameCaptureMode.mmapBufDesc.memoryFd = desc->memoryFd;
    capture->captureParam.frameCaptureMode.mmapBufDesc.totalBufferFrames = desc->totalBufferFrames;
    capture->captureParam.frameCaptureMode.mmapBufDesc.transferFrameSize = desc->transferFrameSize;
    capture->captureParam.frameCaptureMode.mmapBufDesc.isShareable = desc->isShareable;
    capture->captureParam.frameCaptureMode.mmapBufDesc.offset = desc->offset;
    ret = (*pInterfaceLibModeCapture)(capture->devDataHandle, &capture->captureParam,
                                      AUDIO_DRV_PCM_IOCTL_MMAP_BUFFER_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("AudioCaptureReqMmapBuffer FAIL!");
        munmap(desc->memoryAddress, reqSize);
        return HDF_FAILURE;
    }
    LOG_PARA_INFO("AudioCaptureReqMmapBuffer Success!");
    return HDF_SUCCESS;
}

int32_t AudioCaptureGetMmapPosition(AudioHandle handle, uint64_t *frames, struct AudioTimeStamp *time)
{
    struct AudioHwCapture *capture = (struct AudioHwCapture *)handle;
    if (capture == NULL || frames == NULL || time == NULL) {
        return HDF_FAILURE;
    }
#ifndef AUDIO_HAL_USER
    InterfaceLibModeCaptureSo *pInterfaceLibModeCapture = AudioSoGetInterfaceLibModeCapture();
    if (pInterfaceLibModeCapture == NULL || *pInterfaceLibModeCapture == NULL) {
        LOG_FUN_ERR("pInterfaceLibModeCapture Fail!");
        return HDF_FAILURE;
    }
    int32_t ret = (*pInterfaceLibModeCapture)(capture->devDataHandle, &capture->captureParam,
                                              AUDIO_DRV_PCM_IOCTL_MMAP_POSITION_CAPTURE);
    if (ret < 0) {
        LOG_FUN_ERR("GetMmapPosition SetParams FAIL");
        return HDF_FAILURE;
    }
    LOG_PARA_INFO("GetMmapPosition SUCCESS!");
#endif
    *frames = capture->captureParam.frameCaptureMode.frames;
    capture->captureParam.frameCaptureMode.time.tvSec = capture->captureParam.frameCaptureMode.frames /
                                       (int64_t)capture->captureParam.frameCaptureMode.attrs.sampleRate;
    int64_t lastBufFrames = capture->captureParam.frameCaptureMode.frames %
                        ((int64_t)capture->captureParam.frameCaptureMode.attrs.sampleRate);
    capture->captureParam.frameCaptureMode.time.tvNSec =
        (lastBufFrames * SEC_TO_NSEC) / ((int64_t)capture->captureParam.frameCaptureMode.attrs.sampleRate);
    *time = capture->captureParam.frameCaptureMode.time;
    return HDF_SUCCESS;
}

int32_t AudioCaptureTurnStandbyMode(AudioHandle handle)
{
    struct AudioHwCapture *capture = (struct AudioHwCapture *)handle;
    if (capture == NULL) {
        return HDF_FAILURE;
    }
    capture->captureParam.captureMode.hwInfo.deviceDescript.pins = PIN_NONE;
    int32_t ret = AudioCaptureStop((AudioHandle)capture);
    if (ret < 0) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureAudioDevDump(AudioHandle handle, int32_t range, int32_t fd)
{
    struct AudioHwCapture *capture = (struct AudioHwCapture *)handle;
    if (capture == NULL) {
        return HDF_FAILURE;
    }
    dprintf(fd, "%s%d\n", "Number of errors: ", capture->errorLog.totalErrors);
    if (range < RANGE_MIN - 1 || range > RANGE_MAX) {
        dprintf(fd, "%s%d\n", "Out of range, invalid output");
        return HDF_SUCCESS;
    }
    uint32_t mSize = capture->errorLog.iter;
    if (range < RANGE_MIN) {
        dprintf(fd, "%-5s  %-10s  %s\n", "count", "errorCode", "Time");
        for (int i = 0; i < mSize; i++) {
            dprintf(fd, FORMAT_TWO, capture->errorLog.errorDump[i].count + 1,
                    capture->errorLog.errorDump[i].errorCode,
                    capture->errorLog.errorDump[i].currentTime);
        }
    } else {
        dprintf(fd, "%-5s  %-10s  %-20s  %-15s  %s\n", "count", "errorCode", "frames", "fail reason", "Time");
        for (int i = 0; i < mSize; i++) {
            dprintf(fd, FORMAT_ONE, capture->errorLog.errorDump[i].count + 1,
                    capture->errorLog.errorDump[i].errorCode,
                    capture->errorLog.errorDump[i].frames,
                    capture->errorLog.errorDump[i].reason,
                    capture->errorLog.errorDump[i].currentTime);
        }
    }
    return HDF_SUCCESS;
}

