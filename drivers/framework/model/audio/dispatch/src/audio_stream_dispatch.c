/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_stream_dispatch.h"

#define HDF_LOG_TAG audio_stream_dispatch

int32_t HwCpuDaiDispatch(const struct AudioCard *audioCard, const struct AudioPcmHwParams *params)
{
    if ((audioCard == NULL) || (params == NULL)) {
        ADM_LOG_ERR("CpuDai input param is NULL.");
        return HDF_FAILURE;
    }

    struct AudioRuntimeDeivces *rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("CpuDai audioCard rtd is NULL.");
        return HDF_FAILURE;
    }

    struct DaiDevice *cpuDai = rtd->cpuDai;
    if (cpuDai == NULL || cpuDai->devData == NULL || cpuDai->devData->ops == NULL) {
        ADM_LOG_ERR("cpuDai param is NULL.");
        return HDF_FAILURE;
    }

    /* If there are HwParams function, it will be executed directly.
     * If not, skip the if statement and execute in sequence.
     */
    if (cpuDai->devData->ops->HwParams != NULL) {
        int ret = cpuDai->devData->ops->HwParams(audioCard, params, cpuDai);
        if (ret < 0) {
            ADM_LOG_ERR("cpuDai hardware params fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    return HDF_SUCCESS;
}

int32_t HwCodecDaiDispatch(const struct AudioCard *audioCard, const struct AudioPcmHwParams *params)
{
    if ((audioCard == NULL) || (params == NULL)) {
        ADM_LOG_ERR("CodecDai input param is NULL.");
        return HDF_FAILURE;
    }

    struct AudioRuntimeDeivces *rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("CodecDai audioCard rtd is NULL.");
        return HDF_FAILURE;
    }
    struct DaiDevice *codecDai = rtd->codecDai;
    if (codecDai == NULL) {
        codecDai = rtd->accessoryDai;
    }
    if (codecDai == NULL || codecDai->devData == NULL || codecDai->devData->ops == NULL) {
        ADM_LOG_ERR("codecDai param is NULL.");
        return HDF_FAILURE;
    }

    /* If there are HwParams function, it will be executed directly.
     * If not, skip the if statement and execute in sequence.
     */
    if (codecDai->devData->ops->HwParams != NULL) {
        int ret = codecDai->devData->ops->HwParams(audioCard, params, codecDai);
        if (ret < 0) {
            ADM_LOG_ERR("codecDai hardware params fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    return HDF_SUCCESS;
}

int32_t HwPlatfromDispatch(const struct AudioCard *audioCard, const struct AudioPcmHwParams *params)
{
    if ((audioCard == NULL) || (params == NULL)) {
        ADM_LOG_ERR("Platfrom input param is NULL.");
        return HDF_FAILURE;
    }

    struct AudioRuntimeDeivces *rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("audioCard rtd is NULL.");
        return HDF_FAILURE;
    }

    struct PlatformDevice *platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL) {
        ADM_LOG_ERR("platform param is NULL.");
        return HDF_FAILURE;
    }

    /* If there are HwParams function, it will be executed directly.
     * If not, skip the if statement and execute in sequence.
     */
    if (platform->devData->ops->HwParams != NULL) {
        int ret = platform->devData->ops->HwParams(audioCard, params);
        if (ret < 0) {
            ADM_LOG_ERR("platform hardware params fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    return HDF_SUCCESS;
}

int32_t HwParamsDispatch(const struct AudioCard *audioCard, const struct AudioPcmHwParams *params)
{
    if ((audioCard == NULL) || (params == NULL)) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    /* Traverse through each driver method; Enter if you have, if not, exectue in order */
    if (HwCodecDaiDispatch(audioCard, params) ||
        HwCpuDaiDispatch(audioCard, params) ||
        HwPlatfromDispatch(audioCard, params)) {
        ADM_LOG_ERR("hardware params fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t HwParamsDataAnalysis(struct HdfSBuf *reqData, struct AudioPcmHwParams *params)
{
    if ((reqData == NULL) || (params == NULL)) {
        ADM_LOG_ERR(" input param is NULL.");
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint32(reqData, &params->streamType)) {
        ADM_LOG_ERR("read request streamType failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint32(reqData, &params->channels)) {
        ADM_LOG_ERR("read request channels failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint32(reqData, &params->rate)) {
        ADM_LOG_ERR("read request rate failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint32(reqData, &params->periodSize)) {
        ADM_LOG_ERR("read request periodSize failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint32(reqData, &params->periodCount)) {
        ADM_LOG_ERR("read request periodCount failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint32(reqData, (uint32_t *)&params->format)) {
        ADM_LOG_ERR("read request format failed!");
        return HDF_FAILURE;
    }
    if (!(params->cardServiceName = HdfSbufReadString(reqData))) {
        ADM_LOG_ERR("read request cardServiceName failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint32(reqData, &params->period)) {
        HDF_LOGE("read request perid failed!");
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint32(reqData, &params->frameSize)) {
        HDF_LOGE("read request frameSize failed!");
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint32(reqData, (uint32_t *)&params->isBigEndian)) {
        HDF_LOGE("read request isBigEndian failed!");
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint32(reqData, (uint32_t *)&params->isSignedData)) {
        HDF_LOGE("read request isSignedData failed!");
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint32(reqData, &params->startThreshold)) {
        HDF_LOGE("read request startThreshold failed!");
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint32(reqData, &params->stopThreshold)) {
        HDF_LOGE("read request stopThreshold failed!");
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint32(reqData, &params->silenceThreshold)) {
        HDF_LOGE("read request silenceThreshold failed!");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t StreamHostHwParams(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioPcmHwParams params;
    struct StreamHost *streamHost = NULL;
    struct AudioCard *audioCard = NULL;
    char *cardName = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if ((client == NULL || client->device == NULL) || (data == NULL)) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }
    (void)reply;

    streamHost = StreamHostFromDevice(client->device);
    if (streamHost == NULL) {
        ADM_LOG_ERR("renderHost is NULL");
        return HDF_FAILURE;
    }

    cardName = (char *)OsalMemCalloc(sizeof(char) * BUFF_SIZE_MAX);
    if (cardName == NULL) {
        ADM_LOG_ERR("malloc cardServiceName fail!");
        return HDF_FAILURE;
    }
    streamHost->priv = cardName;

    (void)memset_s(&params, sizeof(struct AudioPcmHwParams), 0, sizeof(struct AudioPcmHwParams));
    ret = HwParamsDataAnalysis(data, &params);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("hwparams data analysis failed ret=%d", ret);
        return HDF_FAILURE;
    }

    if (memcpy_s(cardName, BUFF_SIZE_MAX, params.cardServiceName, BUFF_SIZE_MAX) != EOK) {
        ADM_LOG_ERR("memcpy cardName failed.");
        return HDF_FAILURE;
    }

    audioCard = GetCardInstance(params.cardServiceName);
    if (audioCard == NULL) {
        ADM_LOG_ERR("get card instance fail.");
        return HDF_FAILURE;
    }

    ret = HwParamsDispatch(audioCard, &params);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("hwparams dispatch failed ret=%d", ret);
        return HDF_FAILURE;
    }
    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

static struct AudioCard *StreamHostGetCardInstance(const struct HdfDeviceIoClient *client)
{
    char *cardName = NULL;
    struct StreamHost *streamHost = NULL;
    struct AudioCard *audioCard = NULL;

    if (client == NULL || client->device == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return NULL;
    }

    streamHost = StreamHostFromDevice(client->device);
    if (streamHost == NULL) {
        ADM_LOG_ERR("streamHost is NULL");
        return NULL;
    }

    cardName = (char *)streamHost->priv;
    if (cardName == NULL) {
        ADM_LOG_ERR("streamHost priv is NULL.");
        return NULL;
    }

    audioCard = GetCardInstance(cardName);
    if (audioCard == NULL) {
        ADM_LOG_ERR("get card instance fail.");
        return NULL;
    }

    return audioCard;
}

int32_t StreamHostCapturePrepare(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("CapturePrepare input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("CapturePrepare get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->CapturePrepare == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->CapturePrepare(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform CapturePrepare fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostRenderPrepare(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("RenderPrepare input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("RenderPrepare get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->RenderPrepare == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->RenderPrepare(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform RenderPrepare fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

static int32_t StreamTransferWrite(const struct AudioCard *audioCard, struct AudioTxData *transfer)
{
    struct PlatformDevice *platform = NULL;
    int32_t ret;

    if (audioCard == NULL || audioCard->rtd == NULL || transfer == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    platform = audioCard->rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->Write == NULL) {
        ADM_LOG_ERR("audioCard platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->Write(audioCard, transfer);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform write fail ret=%d", ret);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t StreamTransferMmapWrite(const struct AudioCard *audioCard, struct AudioTxMmapData *txMmapData)
{
    struct PlatformDevice *platform = NULL;
    int32_t ret;
    ADM_LOG_DEBUG("entry.");

    if (audioCard == NULL || audioCard->rtd == NULL || txMmapData == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    platform = audioCard->rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->Write == NULL) {
        ADM_LOG_ERR("audioCard platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->MmapWrite(audioCard, txMmapData);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform write fail ret=%d", ret);
        return HDF_FAILURE;
    }

    ADM_LOG_DEBUG("sucess.");
    return HDF_SUCCESS;
}

static int32_t StreamTransferMmapRead(const struct AudioCard *audioCard, struct AudioRxMmapData *rxMmapData)
{
    struct PlatformDevice *platform = NULL;
    int32_t ret;
    ADM_LOG_DEBUG("entry.");

    if (audioCard == NULL || audioCard->rtd == NULL || rxMmapData == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    platform = audioCard->rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->MmapRead == NULL) {
        ADM_LOG_ERR("audioCard platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->MmapRead(audioCard, rxMmapData);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform read fail ret=%d", ret);
        return HDF_FAILURE;
    }

    ADM_LOG_DEBUG("sucess.");
    return HDF_SUCCESS;
}

int32_t StreamHostWrite(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct AudioTxData transfer;
    struct AudioCard *audioCard = NULL;
    int32_t ret;
    uint32_t dataSize = 0;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL || reply == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint32(data, (uint32_t *)&(transfer.frames))) {
        ADM_LOG_ERR("read request frames failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadBuffer(data, (const void **)&(transfer.buf), &dataSize)) {
        ADM_LOG_ERR("read request buf failed!");
        return HDF_FAILURE;
    }

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("get card instance or rtd fail.");
        return HDF_FAILURE;
    }

    ret = StreamTransferWrite(audioCard, &transfer);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("write reg value fail ret=%d", ret);
        return HDF_FAILURE;
    }

    if (!HdfSbufWriteUint32(reply, (uint32_t)(transfer.status))) {
        ADM_LOG_ERR("read response status failed!");
        return HDF_FAILURE;
    }
    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostRead(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    struct AudioRxData rxData;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL || reply == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }
    (void)data;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL ||
        platform->devData->ops == NULL || platform->devData->ops->Read == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->Read(audioCard, &rxData);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform read fail ret=%d", ret);
        return HDF_FAILURE;
    }

    if (!HdfSbufWriteUint32(reply, (uint32_t)(rxData.status))) {
        ADM_LOG_ERR("write request data status failed!");
        return HDF_FAILURE;
    }

    if (!HdfSbufWriteBuffer(reply, rxData.buf, (uint32_t)(rxData.bufSize))) {
        ADM_LOG_ERR("write request data buf failed!");
        return HDF_FAILURE;
    }

    if (!HdfSbufWriteUint32(reply, (uint32_t)(rxData.frames))) {
        ADM_LOG_ERR("write frames failed!");
        return HDF_FAILURE;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostMmapWrite(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct AudioTxMmapData txMmapData;
    struct AudioCard *audioCard = NULL;
    uint64_t mAddress = 0;
    ADM_LOG_DEBUG("entry.");
    if (client == NULL || reply == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint64(data, &mAddress)) {
        ADM_LOG_ERR("render mmap read request memory address failed!");
        return HDF_FAILURE;
    }

    txMmapData.memoryAddress = (void *)((uintptr_t)mAddress);

    if (!HdfSbufReadInt32(data, (uint32_t *)&(txMmapData.memoryFd))) {
        ADM_LOG_ERR("render mmap read request memory fd failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadInt32(data, (uint32_t *)&(txMmapData.totalBufferFrames))) {
        ADM_LOG_ERR("render mmap read request total buffer frames failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadInt32(data, (uint32_t *)&(txMmapData.transferFrameSize))) {
        ADM_LOG_ERR("render mmap read request transfer frame size failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadInt32(data, (uint32_t *)&(txMmapData.isShareable))) {
        ADM_LOG_ERR("render mmap read request is share able failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint32(data, (uint32_t *)&(txMmapData.offset))) {
        ADM_LOG_ERR("render mmap read request offset failed!");
        return HDF_FAILURE;
    }
    audioCard = StreamHostGetCardInstance(client);
    if (StreamTransferMmapWrite(audioCard, &txMmapData) != HDF_SUCCESS) {
        ADM_LOG_ERR("render mmap write reg value fail!");
        return HDF_FAILURE;
    }
    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}
int32_t StreamHostMmapPositionWrite(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct AudioCard *audioCard = NULL;
    ADM_LOG_DEBUG("entry.");
    if (client == NULL || reply == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }
    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL || audioCard->rtd->platform == NULL) {
        ADM_LOG_ERR("audioCard instance is NULL.");
        return HDF_FAILURE;
    }
    struct PlatformHost *platformHost = PlatformHostFromDevice(audioCard->rtd->platform->device);
    if (platformHost == NULL) {
        ADM_LOG_ERR("platformHost instance is NULL.");
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint64(reply, platformHost->renderBufInfo.framesPosition)) {
        ADM_LOG_ERR("render mmap write position fail!");
        return HDF_FAILURE;
    }
    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostMmapRead(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    uint64_t mAddress = 0;
    struct AudioCard *audioCard = NULL;
    struct AudioRxMmapData rxMmapData;
    ADM_LOG_DEBUG("entry.");
    if (client == NULL || reply == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint64(data, &mAddress)) {
        ADM_LOG_ERR("capture mmap read request memory address failed!");
        return HDF_FAILURE;
    }
    rxMmapData.memoryAddress = (void *)((uintptr_t)mAddress);

    if (!HdfSbufReadInt32(data, (uint32_t *)&(rxMmapData.memoryFd))) {
        ADM_LOG_ERR("capture mmap read request memory fd failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadInt32(data, (uint32_t *)&(rxMmapData.totalBufferFrames))) {
        ADM_LOG_ERR("capture mmap read request total buffer frames failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadInt32(data, (uint32_t *)&(rxMmapData.transferFrameSize))) {
        ADM_LOG_ERR("capture mmap read request transfer frame size failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadInt32(data, (uint32_t *)&(rxMmapData.isShareable))) {
        ADM_LOG_ERR("capture mmap read request is share able failed!");
        return HDF_FAILURE;
    }
    if (!HdfSbufReadUint32(data, (uint32_t *)&(rxMmapData.offset))) {
        ADM_LOG_ERR("capture mmap read request offset failed!");
        return HDF_FAILURE;
    }
    audioCard = StreamHostGetCardInstance(client);
    if (StreamTransferMmapRead(audioCard, &rxMmapData) != HDF_SUCCESS) {
        ADM_LOG_ERR("capture mmap read reg value fail!");
        return HDF_FAILURE;
    }
    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostMmapPositionRead(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct AudioCard *audioCard = NULL;
    ADM_LOG_DEBUG("entry.");
    if (client == NULL || reply == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }
    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL || audioCard->rtd->platform == NULL) {
        ADM_LOG_ERR("audioCard is NULL.");
        return HDF_FAILURE;
    }
    struct PlatformHost *platformHost = PlatformHostFromDevice(audioCard->rtd->platform->device);
    if (platformHost == NULL) {
        ADM_LOG_ERR("platformHost is NULL.");
        return HDF_FAILURE;
    }
    if (!HdfSbufWriteUint64(reply, platformHost->captureBufInfo.framesPosition)) {
        ADM_LOG_ERR("render mmap write position fail!");
        return HDF_FAILURE;
    }
    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostRenderStart(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("RenderStart input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("RenderStart get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->RenderStart == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->RenderStart(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform render start fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostCaptureStart(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("CaptureStart input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("CaptureStart get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->CaptureStart == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->CaptureStart(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform capture start fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostRenderStop(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("RenderStop input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("RenderStop get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->RenderStop == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->RenderStop(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform render stop fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostCaptureStop(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("CaptureStop input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("CaptureStop get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->CaptureStop == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->CaptureStop(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform capture stop fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostRenderPause(const struct HdfDeviceIoClient *client, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("RenderPause input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("RenderPause get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->RenderPause == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->RenderPause(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform render pause fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostCapturePause(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("CapturePause input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("CapturePause get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->CapturePause == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->CapturePause(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform captur pause fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostRenderResume(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("RenderResume input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("RenderResume get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->RenderResume == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->RenderResume(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform RenderResume fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostCaptureResume(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("entry.");

    if (client == NULL) {
        ADM_LOG_ERR("CaptureResume input param is NULL.");
        return HDF_FAILURE;
    }

    (void)data;
    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("CaptureResume get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    platform = rtd->platform;
    if (platform == NULL || platform->devData == NULL || platform->devData->ops == NULL ||
        platform->devData->ops->CaptureResume == NULL) {
        ADM_LOG_ERR("audioCard rtd platform is NULL.");
        return HDF_FAILURE;
    }

    ret = platform->devData->ops->CaptureResume(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("platform CaptureResume fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t StreamHostDspDecode(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct DspDevice *dspDev = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("Dsp Decode Entry.");

    if (client == NULL) {
        ADM_LOG_ERR("DspDecode input param is NULL.");
        return HDF_FAILURE;
    }

    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("DspDecode get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    dspDev = rtd->dsp;
    if (dspDev == NULL || dspDev->devData == NULL || dspDev->devData->decode == NULL) {
        ADM_LOG_ERR("audioCard rtd dsp is NULL.");
        return HDF_FAILURE;
    }

    ret = dspDev->devData->decode(audioCard, (void*)data, dspDev);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("DeCode render pause fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("Decode Success.");
    return HDF_SUCCESS;
}

int32_t StreamHostDspEncode(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct DspDevice *dspDev = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("Dsp Encode Entry.");

    if (client == NULL) {
        ADM_LOG_ERR("DspEncode input param is NULL.");
        return HDF_FAILURE;
    }

    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("DspEncode get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    dspDev = rtd->dsp;
    if (dspDev == NULL || dspDev->devData == NULL || dspDev->devData->encode == NULL) {
        ADM_LOG_ERR("audioCard rtd dsp is NULL.");
        return HDF_FAILURE;
    }

    ret = dspDev->devData->encode(audioCard, (void*)data, dspDev);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("EnCode render pause fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("Encode Success.");
    return HDF_SUCCESS;
}

int32_t StreamHostDspEqualizer(const struct HdfDeviceIoClient *client, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct DspDevice *dspDev = NULL;
    struct AudioCard *audioCard = NULL;
    int ret;
    ADM_LOG_DEBUG("Dsp Equalizer Entry.");

    if (client == NULL) {
        ADM_LOG_ERR("DspEqualizer input param is NULL.");
        return HDF_FAILURE;
    }

    (void)reply;

    audioCard = StreamHostGetCardInstance(client);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("DspEqualizer get card instance or rtd fail.");
        return HDF_FAILURE;
    }
    rtd = audioCard->rtd;

    dspDev = rtd->dsp;
    if (dspDev == NULL || dspDev->devData == NULL || dspDev->devData->Equalizer == NULL) {
        ADM_LOG_ERR("audioCard rtd dsp is NULL.");
        return HDF_FAILURE;
    }

    ret = dspDev->devData->Equalizer(audioCard, (void*)data, dspDev);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Equalizer render pause fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("Equalizer Success.");
    return HDF_SUCCESS;
}

static struct StreamDispCmdHandleList g_streamDispCmdHandle[] = {
    {AUDIO_DRV_PCM_IOCTRL_HW_PARAMS, StreamHostHwParams},
    {AUDIO_DRV_PCM_IOCTRL_RENDER_PREPARE, StreamHostRenderPrepare},
    {AUDIO_DRV_PCM_IOCTRL_CAPTURE_PREPARE, StreamHostCapturePrepare},
    {AUDIO_DRV_PCM_IOCTRL_WRITE, StreamHostWrite},
    {AUDIO_DRV_PCM_IOCTRL_READ, StreamHostRead},
    {AUDIO_DRV_PCM_IOCTRL_RENDER_START, StreamHostRenderStart},
    {AUDIO_DRV_PCM_IOCTRL_RENDER_STOP, StreamHostRenderStop},
    {AUDIO_DRV_PCM_IOCTRL_CAPTURE_START, StreamHostCaptureStart},
    {AUDIO_DRV_PCM_IOCTRL_CAPTURE_STOP, StreamHostCaptureStop},
    {AUDIO_DRV_PCM_IOCTRL_RENDER_PAUSE, StreamHostRenderPause},
    {AUDIO_DRV_PCM_IOCTRL_CAPTURE_PAUSE, StreamHostCapturePause},
    {AUDIO_DRV_PCM_IOCTRL_RENDER_RESUME, StreamHostRenderResume},
    {AUDIO_DRV_PCM_IOCTRL_CAPTURE_RESUME, StreamHostCaptureResume},
    {AUDIO_DRV_PCM_IOCTL_MMAP_BUFFER, StreamHostMmapWrite},
    {AUDIO_DRV_PCM_IOCTL_MMAP_BUFFER_CAPTURE, StreamHostMmapRead},
    {AUDIO_DRV_PCM_IOCTL_MMAP_POSITION, StreamHostMmapPositionWrite},
    {AUDIO_DRV_PCM_IOCTL_MMAP_POSITION_CAPTURE, StreamHostMmapPositionRead},
    {AUDIO_DRV_PCM_IOCTRL_DSP_DECODE, StreamHostDspDecode},
    {AUDIO_DRV_PCM_IOCTRL_DSP_ENCODE, StreamHostDspEncode},
    {AUDIO_DRV_PCM_IOCTRL_DSP_EQUALIZER, StreamHostDspEqualizer},
};

int32_t StreamDispatch(struct HdfDeviceIoClient *client, int cmdId,
                       struct HdfSBuf *data, struct HdfSBuf *reply)
{
    unsigned int i;

    if ((client == NULL) || (data == NULL) || (reply == NULL)) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (cmdId >= AUDIO_DRV_PCM_IOCTRL_BUTT || cmdId < 0) {
        ADM_LOG_ERR("invalid [cmdId=%d]", cmdId);
        return HDF_FAILURE;
    }

    for (i = 0; i < sizeof(g_streamDispCmdHandle) / sizeof(g_streamDispCmdHandle[0]); ++i) {
        if ((cmdId == (int)(g_streamDispCmdHandle[i].cmd)) && (g_streamDispCmdHandle[i].func != NULL)) {
            return g_streamDispCmdHandle[i].func(client, data, reply);
        }
    }
    return HDF_FAILURE;
}

static struct StreamHost *StreamHostCreateAndBind(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        return NULL;
    }

    struct StreamHost *streamHost = (struct StreamHost *)OsalMemCalloc(sizeof(*streamHost));
    if (streamHost == NULL) {
        ADM_LOG_ERR("malloc host fail!");
        return NULL;
    }
    streamHost->device = device;
    device->service = &streamHost->service;
    return streamHost;
}

static int32_t AudioStreamBind(struct HdfDeviceObject *device)
{
    ADM_LOG_DEBUG("entry!");
    if (device == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    struct StreamHost *streamHost = StreamHostCreateAndBind(device);
    if (streamHost == NULL || streamHost->device == NULL) {
        ADM_LOG_ERR("StreamHostCreateAndBind failed");
        return HDF_FAILURE;
    }

    streamHost->service.Dispatch = StreamDispatch;

    ADM_LOG_INFO("success!");
    return HDF_SUCCESS;
}

static int32_t AudioStreamInit(struct HdfDeviceObject *device)
{
    struct StreamHost *streamHost = NULL;

    if (device == NULL) {
        ADM_LOG_ERR("device is NULL");
        return HDF_FAILURE;
    }
    ADM_LOG_DEBUG("entry.");

    streamHost = StreamHostFromDevice(device);
    if (streamHost == NULL) {
        ADM_LOG_ERR("renderHost is NULL");
        return HDF_FAILURE;
    }

    ADM_LOG_INFO("Success!");
    return HDF_SUCCESS;
}

static void AudioStreamRelease(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        ADM_LOG_ERR("device is NULL");
        return;
    }

    struct StreamHost *streamHost = StreamHostFromDevice(device);
    if (streamHost == NULL) {
        ADM_LOG_ERR("renderHost is NULL");
        return;
    }

    if (streamHost->priv != NULL) {
        OsalMemFree(streamHost->priv);
    }
    OsalMemFree(streamHost);
}

/* HdfDriverEntry definitions */
struct HdfDriverEntry g_audioStreamEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_AUDIO_STREAM",
    .Bind = AudioStreamBind,
    .Init = AudioStreamInit,
    .Release = AudioStreamRelease,
};
HDF_INIT(g_audioStreamEntry);
