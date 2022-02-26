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

static int32_t AudioProxyCaptureCtrl(int cmId, const AudioHandle handle)
{
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The pointer is null");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, cmId, data, reply);
    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}

int32_t AudioProxyCaptureStart(const AudioHandle handle)
{
    LOG_FUN_INFO();
    return AudioProxyCaptureCtrl(AUDIO_HDI_CAPTURE_START, handle);
}

int32_t AudioProxyCaptureStop(const AudioHandle handle)
{
    LOG_FUN_INFO();
    return AudioProxyCaptureCtrl(AUDIO_HDI_CAPTURE_STOP, handle);
}

int32_t AudioProxyCapturePause(const AudioHandle handle)
{
    LOG_FUN_INFO();
    return AudioProxyCaptureCtrl(AUDIO_HDI_CAPTURE_PAUSE, handle);
}

int32_t AudioProxyCaptureResume(const AudioHandle handle)
{
    LOG_FUN_INFO();
    return AudioProxyCaptureCtrl(AUDIO_HDI_CAPTURE_RESUME, handle);
}

int32_t AudioProxyCaptureFlush(const AudioHandle handle)
{
    LOG_FUN_INFO();
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL) {
        return HDF_FAILURE;
    }
    return HDF_ERR_NOT_SUPPORT;
}

int32_t AudioProxyCaptureGetFrameParameter(int cmId, AudioHandle handle, uint64_t *param)
{
    if (param == NULL) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The pointer is empty");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, cmId, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioCaptureGetFrameSize FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    if (!HdfSbufReadUint64(reply, param)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}

int32_t AudioProxyCaptureGetFrameSize(AudioHandle handle, uint64_t *size)
{
    return AudioProxyCaptureGetFrameParameter(AUDIO_HDI_CAPTURE_GET_FRAME_SIZE, handle, size);
}

int32_t AudioProxyCaptureGetFrameCount(AudioHandle handle, uint64_t *count)
{
    return AudioProxyCaptureGetFrameParameter(AUDIO_HDI_CAPTURE_GET_FRAME_COUNT, handle, count);
}

int32_t AudioProxyCaptureSetSampleAttributes(const AudioHandle handle, const struct AudioSampleAttributes *attrs)
{
    if (handle == NULL || attrs == NULL) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("hwCapture or hwCapture->proxyRemoteHandle is NULL");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        LOG_FUN_ERR("AudioProxyPreprocessCapture Fail");
        return HDF_FAILURE;
    }
    if (AudioProxyWriteSampleAttributes(data, attrs) < 0) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_SET_SAMPLE_ATTR, data, reply);
    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}

int32_t AudioProxyCaptureGetSampleAttributes(const AudioHandle handle, struct AudioSampleAttributes *attrs)
{
    if (attrs == NULL) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The Invalid is pointer");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_GET_SAMPLE_ATTR, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioCaptureGetSampleAttributes FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    if (AudioProxyReadSapmleAttrbutes(reply, attrs)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyCaptureGetCurrentChannelId(const AudioHandle handle, uint32_t *channelId)
{
    if (channelId == NULL) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("hwCapture parameter is invalid");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle,
        AUDIO_HDI_CAPTURE_GET_CUR_CHANNEL_ID, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioCaptureGetFrameSize FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    if (!HdfSbufReadUint32(reply, channelId)) {
        LOG_FUN_ERR("Read reply FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyCaptureCheckSceneCapability(const AudioHandle handle,
    const struct AudioSceneDescriptor *scene, bool *supported)
{
    if (scene == NULL || supported == NULL) {
        return HDF_FAILURE;
    }
    uint32_t tempPins;
    uint32_t tempSupported = 0;
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("pointer invalid");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, scene->scene.id)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    tempPins = scene->desc.pins;
    if (!HdfSbufWriteUint32(data, tempPins)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle,
        AUDIO_HDI_CAPTURE_CHECK_SCENE_CAPABILITY, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioProxyCaptureCheckSceneCapability FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    if (!HdfSbufReadUint32(reply, &tempSupported)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    *supported = (bool)tempSupported;
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyCaptureSelectScene(const AudioHandle handle, const struct AudioSceneDescriptor *scene)
{
    if (scene == NULL) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The hwCapture pointer is null");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint32(data, (enum AudioCategory)scene->scene.id)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    uint32_t tempPins = scene->desc.pins;
    if (!HdfSbufWriteUint32(data, tempPins)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_SELECT_SCENE, data, reply);
    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}

int32_t AudioProxyCaptureSetMute(const AudioHandle handle, bool mute)
{
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The hwCapture parameter is null");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    uint32_t tempMute = (uint32_t)mute;
    if (!HdfSbufWriteUint32(data, tempMute)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_SET_MUTE, data, reply);
    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}

int32_t AudioProxyCaptureGetMute(const AudioHandle handle, bool *mute)
{
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    if (mute == NULL) {
        return HDF_FAILURE;
    }
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The hwCapture parameter is null");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_GET_MUTE, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioCaptureGetMute FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    uint32_t tempMute = 0;
    if (!HdfSbufReadUint32(reply, &tempMute)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    *mute = (bool)tempMute;
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyCaptureSetVolume(const AudioHandle handle, float volume)
{
    return AudioProxyCommonSetCaptureCtrlParam(AUDIO_HDI_CAPTURE_SET_VOLUME, handle, volume);
}

int32_t AudioProxyCaptureGetVolume(const AudioHandle handle, float *volume)
{
    return AudioProxyCommonGetCaptureCtrlParam(AUDIO_HDI_CAPTURE_GET_VOLUME, handle, volume);
}

int32_t AudioProxyCaptureGetGainThreshold(const AudioHandle handle, float *min, float *max)
{
    if (NULL == min || NULL == max) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The hwCapture pointer is invalid");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle,
        AUDIO_HDI_CAPTURE_GET_GAIN_THRESHOLD, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioCaptureGetGainThreshold FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    uint32_t temp = 0;
    if (!HdfSbufReadUint32(reply, &temp)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    *min = temp;
    if (!HdfSbufReadUint32(reply, &temp)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    *max = temp;
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyCaptureGetGain(const AudioHandle handle, float *gain)
{
    return AudioProxyCommonGetCaptureCtrlParam(AUDIO_HDI_CAPTURE_GET_GAIN, handle, gain);
}

int32_t AudioProxyCaptureSetGain(const AudioHandle handle, float gain)
{
    return AudioProxyCommonSetCaptureCtrlParam(AUDIO_HDI_CAPTURE_SET_GAIN, handle, gain);
}

int32_t AudioProxyCaptureCaptureFrame(struct AudioCapture *capture, void *frame,
                                      uint64_t requestBytes, uint64_t *replyBytes)
{
    const char *buffer = NULL;
    uint32_t length = 0;
    if (frame == NULL || replyBytes == NULL) {
        LOG_FUN_ERR("capture Frame Paras is NULL!");
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)capture;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The pointer is empty");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint64(data, requestBytes)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_CAPTURE_FRAME, data, reply);
    if (ret < 0) {
        if (ret != HDF_ERR_INVALID_OBJECT) {
            LOG_FUN_ERR("AudioCaptureCaptureFrame FAIL");
        }
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    if (!HdfSbufReadBuffer(reply, (const void **)&buffer, &length)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    if ((uint64_t)length > requestBytes) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    ret = memcpy_s(frame, requestBytes, buffer, length);
    if (ret != EOK) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint64(reply, replyBytes)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyCaptureGetCapturePosition(struct AudioCapture *capture,
    uint64_t *frames, struct AudioTimeStamp *time)
{
    if (frames == NULL || time == NULL) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)capture;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The hwCapture parameter is null");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture(hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle,
        AUDIO_HDI_CAPTURE_GET_CAPTURE_POSITION, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioCaptureGetCapturePosition FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    if (AudioProxyGetMmapPositionRead(reply, frames, time) < 0) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}
int32_t AudioProxyCaptureSetExtraParams(const AudioHandle handle, const char *keyValueList)
{
    if (NULL == handle || NULL == keyValueList) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("hwCapture or hwCapture->proxyRemoteHandle is NULL");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture((AudioHandle)hwCapture, &data, &reply) < 0) {
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteString(data, keyValueList)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_SET_EXTRA_PARAMS, data, reply);
    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}
int32_t AudioProxyCaptureGetExtraParams(const AudioHandle handle, char *keyValueList, int32_t listLenth)
{
    if (NULL == handle || NULL == keyValueList || listLenth <= 0) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The parameter is null");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture((AudioHandle)hwCapture, &data, &reply) < 0) {
        LOG_FUN_ERR("AudioProxyCaptureGetExtraParams FAIL");
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteInt32(data, listLenth)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_GET_EXTRA_PARAMS, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioProxyCaptureGetExtraParams FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    const char *strKeyValueList = NULL;
    if ((strKeyValueList = HdfSbufReadString(reply)) == NULL) {
        LOG_FUN_ERR("keyValueList Is NULL");
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    ret = strncpy_s(keyValueList, listLenth - 1, strKeyValueList, strlen(strKeyValueList) + 1);
    if (ret != 0) {
        LOG_FUN_ERR("strncpy_s failed!");
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}
int32_t AudioProxyCaptureReqMmapBuffer(const AudioHandle handle,
    int32_t reqSize, struct AudioMmapBufferDescripter *desc)
{
    if (NULL == handle || NULL == desc) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("hwCapture parameter is null");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture((AudioHandle)hwCapture, &data, &reply) < 0) {
        LOG_FUN_ERR("AudioProxyCaptureReqMmapBuffer FAIL");
        return HDF_FAILURE;
    }
    if (AudioProxyReqMmapBufferWrite(data, reqSize, desc) < 0) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_REQ_MMAP_BUFFER, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioProxyCaptureReqMmapBuffer FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }

    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}

int32_t AudioProxyCaptureGetMmapPosition(const AudioHandle handle, uint64_t *frames, struct AudioTimeStamp *time)
{
    if (NULL == handle || NULL == frames || NULL == time) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The parameter is empty");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture((AudioHandle)hwCapture, &data, &reply) < 0) {
        LOG_FUN_ERR("AudioProxyCaptureGetMmapPosition FAIL");
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle,
        AUDIO_HDI_CAPTURE_GET_MMAP_POSITION, data, reply);
    if (ret < 0) {
        AudioProxyBufReplyRecycle(data, reply);
        LOG_FUN_ERR("AudioProxyCaptureGetMmapPosition FAIL");
        return ret;
    }
    if (AudioProxyGetMmapPositionRead(reply, frames, time) < 0) {
        AudioProxyBufReplyRecycle(data, reply);
        LOG_FUN_ERR("AudioProxyGetMmapPositionRead FAIL");
        return HDF_FAILURE;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return HDF_SUCCESS;
}

int32_t AudioProxyCaptureTurnStandbyMode(const AudioHandle handle)
{
    if (NULL == handle) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("The hwCapture parameter is empty");
        return HDF_FAILURE;
    }
    if (AudioProxyPreprocessCapture((AudioHandle)hwCapture, &data, &reply) < 0) {
        LOG_FUN_ERR("AudioProxyCaptureTurnStandbyMode FAIL");
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle,
        AUDIO_HDI_CAPTURE_TURN_STAND_BY_MODE, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioProxyCaptureTurnStandbyMode FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}
int32_t AudioProxyCaptureAudioDevDump(const AudioHandle handle, int32_t range, int32_t fd)
{
    if (NULL == handle) {
        return HDF_FAILURE;
    }
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct AudioHwCapture *hwCapture = (struct AudioHwCapture *)handle;
    if (hwCapture == NULL || hwCapture->proxyRemoteHandle == NULL) {
        LOG_FUN_ERR("hwCapture parameter is empty");
        return HDF_FAILURE;
    }

    if (AudioProxyPreprocessCapture((AudioHandle)hwCapture, &data, &reply) < 0) {
        LOG_FUN_ERR("AudioProxyCaptureAudioDevDump FAIL");
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteInt32(data, range)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteInt32(data, fd)) {
        AudioProxyBufReplyRecycle(data, reply);
        return HDF_FAILURE;
    }
    int32_t ret = AudioProxyDispatchCall(hwCapture->proxyRemoteHandle, AUDIO_HDI_CAPTURE_DEV_DUMP, data, reply);
    if (ret < 0) {
        LOG_FUN_ERR("AudioProxyCaptureAudioDevDump FAIL");
        AudioProxyBufReplyRecycle(data, reply);
        return ret;
    }
    AudioProxyBufReplyRecycle(data, reply);
    return ret;
}

