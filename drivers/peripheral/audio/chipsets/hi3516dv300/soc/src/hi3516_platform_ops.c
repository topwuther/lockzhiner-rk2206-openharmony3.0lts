/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hi3516_platform_ops.h"
#ifdef __LITEOS__
#include <los_vm_iomap.h>
#else
#include <linux/dma-mapping.h>
#endif
#include "audio_platform_base.h"
#include "audio_sapm.h"
#include "hi3516_aiao_impl.h"
#include "hi3516_codec_impl.h"
#include "osal_io.h"
#include "osal_uaccess.h"

#define HDF_LOG_TAG hi3516_platform_ops

const int AIAO_BUFF_OFFSET = 128;
const int AIAO_BUFF_POINT = 320;
const int AIAO_BUFF_TRANS = 16 * 1024;
const int AIAO_BUFF_SIZE = 128 * 1024;
const int RENDER_TRAF_BUF_SIZE = 1024;
const int CAPTURE_TRAF_BUF_SIZE = 1024 * 16;

const int AUDIO_BUFF_MIN = 128;
const int AUDIO_RECORD_MIN = 1024 * 16;
const int BITSTOBYTE = 8;

static int g_captureBuffFreeCount = 0;
static int g_renderBuffFreeCount = 0;
static int g_renderBuffInitCount = 0;

const int MIN_PERIOD_SIZE = 4096;
const int MAX_PERIOD_SIZE = 1024 * 16;
const int MIN_PERIOD_COUNT = 8;
const int MAX_PERIOD_COUNT = 32;
const int MAX_AIAO_BUFF_SIZE = 128 * 1024;
const int MIN_AIAO_BUFF_SIZE = 16 * 1024;
const int MMAP_MAX_FRAME_SIZE = 4096 * 2 * 3;
const int AUDIO_CACHE_ALIGN_SIZE = 64;
const int DELAY_TIME = 5;
const int LOOP_COUNT = 100;

static struct device tmpDev = {0};
int32_t AudioRenderBuffInit(struct PlatformHost *platformHost)
{
    uint64_t buffSize;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->renderBufInfo.virtAddr != NULL) {
        return HDF_SUCCESS;
    }

    buffSize = platformHost->renderBufInfo.periodCount * platformHost->renderBufInfo.periodSize;
    if (buffSize < MIN_AIAO_BUFF_SIZE || buffSize > MAX_AIAO_BUFF_SIZE) {
        AUDIO_DRIVER_LOG_ERR("buffSize is invalid.");
        return HDF_FAILURE;
    }

    platformHost->renderBufInfo.phyAddr = 0;
#ifdef __LITEOS__
    platformHost->renderBufInfo.virtAddr = (uint32_t *)LOS_DmaMemAlloc(&platformHost->renderBufInfo.phyAddr, buffSize,
        AUDIO_CACHE_ALIGN_SIZE, DMA_NOCACHE);
#else

    tmpDev.coherent_dma_mask = 0xffffffffUL;
    platformHost->renderBufInfo.virtAddr = dma_alloc_wc(&tmpDev, buffSize,
        (dma_addr_t *)&platformHost->renderBufInfo.phyAddr, GFP_DMA | GFP_KERNEL);
#endif
    if (platformHost->renderBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("mem alloc failed.");
        return HDF_FAILURE;
    }
    platformHost->renderBufInfo.cirBufSize = buffSize;

    AUDIO_DRIVER_LOG_DEBUG("phyAddr = %x virtAddr = %x",
        platformHost->renderBufInfo.phyAddr, platformHost->renderBufInfo.virtAddr);
    AUDIO_DRIVER_LOG_DEBUG("g_renderBuffInitCount: %d", g_renderBuffInitCount++);

    return HDF_SUCCESS;
}

int32_t AudioRenderBuffFree(struct PlatformHost *platformHost)
{
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->renderBufInfo.virtAddr != NULL) {
#ifdef __LITEOS__
    LOS_DmaMemFree(platformHost->renderBufInfo.virtAddr);
#else
    dma_free_wc(&tmpDev, platformHost->renderBufInfo.cirBufSize, platformHost->renderBufInfo.virtAddr,
                platformHost->renderBufInfo.phyAddr);
#endif
    }

    AUDIO_DRIVER_LOG_DEBUG("g_renderBuffFreeCount: %d", g_renderBuffFreeCount++);

    platformHost->renderBufInfo.virtAddr = NULL;
    platformHost->renderBufInfo.phyAddr = 0;
    return HDF_SUCCESS;
}

int32_t AudioCaptureBuffInit(struct PlatformHost *platformHost)
{
    uint64_t buffSize;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }
    if (platformHost->captureBufInfo.virtAddr != NULL) {
        return HDF_SUCCESS;
    }
    buffSize = platformHost->captureBufInfo.periodCount * platformHost->captureBufInfo.periodSize;
    if (buffSize < MIN_AIAO_BUFF_SIZE || buffSize > MAX_AIAO_BUFF_SIZE) {
        AUDIO_DRIVER_LOG_ERR("buffSize is invalid.");
        return HDF_FAILURE;
    }

    platformHost->captureBufInfo.phyAddr = 0;
#ifdef __LITEOS__
    platformHost->captureBufInfo.virtAddr = (uint32_t *)LOS_DmaMemAlloc(&platformHost->captureBufInfo.phyAddr, buffSize,
        AUDIO_CACHE_ALIGN_SIZE, DMA_NOCACHE);
#else
    tmpDev.coherent_dma_mask = 0xffffffffUL;
    platformHost->captureBufInfo.virtAddr = dma_alloc_wc(&tmpDev, buffSize,
        (dma_addr_t *)&platformHost->captureBufInfo.phyAddr, GFP_DMA | GFP_KERNEL);
#endif
    if (platformHost->captureBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("mem alloc failed.");
        return HDF_FAILURE;
    }
    platformHost->captureBufInfo.cirBufSize = buffSize;

    AUDIO_DRIVER_LOG_DEBUG("phyAddr = %x virtAddr = %x",
        platformHost->captureBufInfo.phyAddr, platformHost->captureBufInfo.virtAddr);

    return HDF_SUCCESS;
}

int32_t AudioCaptureBuffFree(struct PlatformHost *platformHost)
{
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->captureBufInfo.virtAddr != NULL) {
#ifdef __LITEOS__
    LOS_DmaMemFree(platformHost->captureBufInfo.virtAddr);
#else
    dma_free_wc(&tmpDev, platformHost->captureBufInfo.cirBufSize, platformHost->captureBufInfo.virtAddr,
                platformHost->captureBufInfo.phyAddr);
#endif
    }
    AUDIO_DRIVER_LOG_DEBUG("g_captureBuffFreeCount: %d", g_captureBuffFreeCount++);

    platformHost->captureBufInfo.virtAddr = NULL;
    platformHost->captureBufInfo.phyAddr = 0;
    return HDF_SUCCESS;
}

int32_t AudioAoInit(const struct PlatformHost *platformHost)
{
    int ret;

    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->renderBufInfo.phyAddr == 0) {
        AUDIO_DRIVER_LOG_ERR("phyAddr is error");
        return HDF_FAILURE;
    }
    ret = AopHalSetBufferAddr(0, platformHost->renderBufInfo.phyAddr);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AopHalSetBufferAddr fail.");
        return HDF_FAILURE;
    }

    ret = AopHalSetBufferSize(0, platformHost->renderBufInfo.cirBufSize);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AopHalSetBufferSize: failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("AopHalSetBufferSize: %d", platformHost->renderBufInfo.cirBufSize);

    ret = AopHalSetTransSize(0, platformHost->renderBufInfo.trafBufSize);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AopHalSetTransSize fail");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioAiInit(const struct PlatformHost *platformHost)
{
    int ret;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->captureBufInfo.phyAddr == 0) {
        AUDIO_DRIVER_LOG_ERR("phyAddr is error");
        return HDF_FAILURE;
    }

    ret = AipHalSetBufferAddr(0, platformHost->captureBufInfo.phyAddr);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBufferAddr: failed.");
        return HDF_FAILURE;
    }

    ret = AipHalSetBufferSize(0, platformHost->captureBufInfo.cirBufSize);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBufferSize: failed.");
        return HDF_FAILURE;
    }

    ret = AipHalSetTransSize(0, AIAO_BUFF_POINT);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetTransSize fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static void SysWritelI2s(unsigned long addr, unsigned int value)
{
    *(volatile unsigned int *)(addr) = value;
}

int32_t AiaoSysPinMux(void)
{
    static void *regIocfg2Base = 0;
    static void *regIocfg3Base = 0;
    static void *regGpioBase = 0;

    regIocfg2Base = OsalIoRemap(IOCFG2_BASE_ADDR, BASE_ADDR_REMAP_SIZE);
    if (regIocfg2Base == NULL) {
        AUDIO_DRIVER_LOG_ERR("regIocfg2Base is NULL.");
        return HDF_FAILURE;
    }

    regIocfg3Base = OsalIoRemap(IOCFG3_BASE_ADDR, BASE_ADDR_REMAP_SIZE);
    if (regIocfg3Base == NULL) {
        AUDIO_DRIVER_LOG_ERR("regIocfg3Base is NULL.");
        return HDF_FAILURE;
    }

    regGpioBase = OsalIoRemap(GPIO_BASE_ADDR, BASE_ADDR_REMAP_SIZE);
    if (regGpioBase == NULL) {
        AUDIO_DRIVER_LOG_ERR("regGpioBase is NULL.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("I2s0PinMuxAmpUnmute i2s0_pin_mux");
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE1, I2S_IOCFG2_BASE1_VAL);
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE2, I2S_IOCFG2_BASE2_VAL);
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE3, I2S_IOCFG2_BASE3_VAL);
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE4, I2S_IOCFG2_BASE4_VAL);
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE5, I2S_IOCFG2_BASE5_VAL);

    AUDIO_DRIVER_LOG_DEBUG("I2s0PinMuxAmpUnmute AmpUnmute");
    SysWritelI2s((uintptr_t)regIocfg3Base + I2S_IOCFG3_BASE1, I2S_IOCFG3_BASE1_VAL);
    SysWritelI2s((uintptr_t)regGpioBase + GPIO_BASE1, GPIO_BASE3_VAL);
    SysWritelI2s((uintptr_t)regGpioBase + GPIO_BASE2, GPIO_BASE2_VAL);
    SysWritelI2s((uintptr_t)regGpioBase + GPIO_BASE3, GPIO_BASE3_VAL);

    OsalIoUnmap(regGpioBase);
    OsalIoUnmap(regIocfg3Base);
    OsalIoUnmap(regIocfg2Base);
    return HDF_SUCCESS;
}

int32_t AudioPlatformDeviceInit(const struct AudioCard *card, const struct PlatformDevice *platform)
{
    struct PlatformHost *platformHost = NULL;
    int ret;
    unsigned int chnId = 0;

    if (platform == NULL || platform->device == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform is NULL.");
        return HDF_FAILURE;
    }
    (void)card;

    platformHost = (struct PlatformHost *)platform->device->service;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform host is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->platformInitFlag == true) {
        AUDIO_DRIVER_LOG_DEBUG("platform init complete!");
        return HDF_SUCCESS;
    }
    platformHost->platformInitFlag = true;

    ret = AiaoHalSysInit();
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoHalSysInit:  fail.");
        return HDF_FAILURE;
    }

    /* PIN MUX */
    ret = AiaoSysPinMux();
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoSysPinMux: fail.");
        return HDF_FAILURE;
    }

    /* CLK reset */
    ret = AiaoClockReset();
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoClockReset: fail");
        return HDF_FAILURE;
    }

    /* aiao init */
    ret = AiaoDeviceInit(chnId);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoClockReset:  fail");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t RenderSetAiaoAttr(struct PlatformHost *platformHost, const struct AudioPcmHwParams *param)
{
    if (platformHost == NULL || param == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform is NULL.");
        return HDF_FAILURE;
    }
    platformHost->renderBufInfo.period = param->period;
    if (param->periodSize < MIN_PERIOD_SIZE || param->periodSize > MAX_PERIOD_SIZE) {
        AUDIO_DRIVER_LOG_ERR("periodSize is invalid.");
        return HDF_FAILURE;
    }
    platformHost->renderBufInfo.periodSize = param->periodSize;
    if (param->periodCount < MIN_PERIOD_COUNT || param->periodCount > MAX_PERIOD_COUNT) {
        AUDIO_DRIVER_LOG_ERR("periodCount is invalid.");
        return HDF_FAILURE;
    }
    platformHost->renderBufInfo.periodCount = param->periodCount;

    platformHost->renderBufInfo.trafBufSize = RENDER_TRAF_BUF_SIZE;
    return HDF_SUCCESS;
}

int32_t CaptureSetAiaoAttr(struct PlatformHost *platformHost, const struct AudioPcmHwParams *param)
{
    if (platformHost == NULL || param == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformHost or param is NULL.");
        return HDF_FAILURE;
    }

    platformHost->captureBufInfo.period = param->period;
    if (param->periodSize < MIN_PERIOD_SIZE || param->periodSize > MAX_PERIOD_SIZE) {
        AUDIO_DRIVER_LOG_ERR("periodSize is invalid %d.", param->periodSize);
        return HDF_FAILURE;
    }
    platformHost->captureBufInfo.periodSize = param->periodSize;
    if (param->periodCount < MIN_PERIOD_COUNT || param->periodCount > MAX_PERIOD_COUNT) {
        AUDIO_DRIVER_LOG_ERR("periodCount is invalid %d.", param->periodCount);
        return HDF_FAILURE;
    }
    platformHost->captureBufInfo.periodCount = param->periodCount;

    if (param->silenceThreshold < MIN_PERIOD_SIZE || param->silenceThreshold > MAX_PERIOD_SIZE) {
        AUDIO_DRIVER_LOG_ERR("silenceThreshold is invalid %d.", param->silenceThreshold);
        return HDF_FAILURE;
    }
    platformHost->captureBufInfo.trafBufSize = param->silenceThreshold;
    return HDF_SUCCESS;
}

int32_t PlatformHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param)
{
    int ret;
    const int chnlCntMin = 1;
    const int chnlCntMax = 2;
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry.");

    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL ||
        param == NULL || param->cardServiceName == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    if (param->channels < chnlCntMin || param->channels > chnlCntMax) {
        AUDIO_DRIVER_LOG_ERR("channels param is invalid.");
        return HDF_FAILURE;
    }

    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformHost is null.");
        return HDF_FAILURE;
    }

    platformHost->pcmInfo.rate     = param->rate;
    platformHost->pcmInfo.frameSize = param->channels * platformHost->pcmInfo.bitWidth / BITSTOBYTE;

    platformHost->renderBufInfo.chnId = 0;
    platformHost->captureBufInfo.chnId = 0;

    platformHost->pcmInfo.isBigEndian = param->isBigEndian;
    platformHost->pcmInfo.isSignedData = param->isSignedData;

    platformHost->pcmInfo.startThreshold = param->startThreshold;
    platformHost->pcmInfo.stopThreshold = param->stopThreshold;
    platformHost->pcmInfo.silenceThreshold = param->silenceThreshold;

    if (param->streamType == AUDIO_RENDER_STREAM) {
        ret = RenderSetAiaoAttr(platformHost, param);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AoSetClkAttr:  fail.");
            return HDF_FAILURE;
        }
    } else if (param->streamType == AUDIO_CAPTURE_STREAM) {
        ret = CaptureSetAiaoAttr(platformHost, param);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AiSetClkAttr:  fail.");
            return HDF_FAILURE;
        }
    } else {
        AUDIO_DRIVER_LOG_ERR("param streamType is invalid.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t PlatformRenderPrepare(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("PlatformPrepare: entry.");

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    if (platformHost->renderBufInfo.virtAddr == NULL) {
        ret = AudioRenderBuffInit(platformHost);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioRenderBuffInit: fail.");
            return HDF_FAILURE;
        }

        ret = AudioAoInit(platformHost);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioAoInit: fail.");
            return HDF_FAILURE;
        }
    }

    if (platformHost->renderBufInfo.virtAddr != NULL) {
        (void)memset_s(platformHost->renderBufInfo.virtAddr,
                       platformHost->renderBufInfo.cirBufSize, 0,
                       platformHost->renderBufInfo.cirBufSize);
    }
    platformHost->renderBufInfo.wbufOffSet = 0;
    platformHost->renderBufInfo.wptrOffSet = 0;
    platformHost->pcmInfo.totalStreamSize = 0;
    platformHost->renderBufInfo.framesPosition = 0;

    ret = AopHalSetBuffWptr(0, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AopHalSetBuffWptr: failed.");
        return HDF_FAILURE;
    }

    ret = AopHalSetBuffRptr(0, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AopHalSetBuffRptr: failed.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t PlatformCapturePrepare(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry.");

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    if (platformHost->captureBufInfo.virtAddr == NULL) {
        ret = AudioCaptureBuffInit(platformHost);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioCaptureBuffInit: fail.");
            return HDF_FAILURE;
        }
        ret = AudioAiInit(platformHost);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioAiInit: fail.");
            return HDF_FAILURE;
        }
    }

    if (platformHost->captureBufInfo.virtAddr != NULL) {
        (void)memset_s(platformHost->captureBufInfo.virtAddr,
                       platformHost->captureBufInfo.cirBufSize, 0,
                       platformHost->captureBufInfo.cirBufSize);
    }
    platformHost->captureBufInfo.wbufOffSet = 0;
    platformHost->captureBufInfo.wptrOffSet = 0;
    platformHost->captureBufInfo.chnId = 0;
    platformHost->pcmInfo.totalStreamSize = 0;
    platformHost->captureBufInfo.framesPosition = 0;

    ret = AipHalSetBuffWptr(0, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBuffWptr: failed.");
        return HDF_FAILURE;
    }

    ret = AipHalSetBuffRptr(0, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBuffRptr: failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

static int32_t UpdateWriteBufData(struct PlatformHost *platformHost, unsigned int wptr,
    unsigned int buffSize, unsigned int *buffOffset, const char *buf)
{
    int ret;
    unsigned int buffFirstSize;
    unsigned int buffSecordSize;
    if (platformHost == NULL || buffOffset == NULL || buf == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    if (platformHost->renderBufInfo.cirBufSize - wptr >= buffSize) {
        *buffOffset = wptr + buffSize;
        ret = memcpy_s((char *)(platformHost->renderBufInfo.virtAddr) + wptr,
                       buffSize, buf, buffSize);
        if (ret != 0) {
            AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
            return HDF_FAILURE;
        }
        if (*buffOffset >= platformHost->renderBufInfo.cirBufSize) {
            *buffOffset = 0;
        }
    } else {
        buffFirstSize = platformHost->renderBufInfo.cirBufSize - wptr;
        ret = memcpy_s((char *)(platformHost->renderBufInfo.virtAddr) + wptr,
                       buffFirstSize, buf, buffFirstSize);
        if (ret != 0) {
            AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
            return HDF_FAILURE;
        }

        buffSecordSize = buffSize - buffFirstSize;
        ret = memcpy_s((char *)platformHost->renderBufInfo.virtAddr,
                       buffSecordSize, buf + buffFirstSize,
                       buffSecordSize);
        if (ret != 0) {
            AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
            return HDF_FAILURE;
        }
        *buffOffset = buffSecordSize;
    }
    return HDF_SUCCESS;
}

static int32_t UpdateWriteBuffOffset(struct PlatformHost *platformHost,
    unsigned int buffSize, unsigned int *buffOffset, struct AudioTxData *txData)
{
    unsigned int buffAvailable;
    int ret;
    unsigned int wptr;
    unsigned int rptr;
    int devId;
    if (platformHost == NULL || buffOffset == NULL || txData == NULL || txData->buf == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    devId = platformHost->renderBufInfo.chnId;
    rptr = AiaoHalReadReg(AopBuffRptrReg(devId));
    wptr = AiaoHalReadReg(AopBuffWptrReg(devId));
    AUDIO_DRIVER_LOG_DEBUG("rptrReg = [0x%08x, wptrReg = [0x%08x], input size = [%u]",
        rptr, wptr, buffSize);

    if (wptr >= rptr) {
        // [S ... R ... W ... E]
        buffAvailable = platformHost->renderBufInfo.cirBufSize - (wptr - rptr);

        if (buffAvailable < buffSize + AUDIO_BUFF_MIN) {
            AUDIO_DRIVER_LOG_DEBUG("not available buffer.");
            txData->status = ENUM_CIR_BUFF_FULL;
            return HDF_SUCCESS;
        }

        ret = UpdateWriteBufData(platformHost, wptr, buffSize, buffOffset, txData->buf);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("UpdateWriteBufData: fail.");
            return HDF_FAILURE;
        }
    } else {
        // [S ... W ... R ... E]
        buffAvailable = rptr - wptr;

        if (buffAvailable < buffSize + AUDIO_BUFF_MIN) {
            AUDIO_DRIVER_LOG_DEBUG("not available buffer.");
            txData->status = ENUM_CIR_BUFF_FULL;
            return HDF_SUCCESS;
        }

        *buffOffset = wptr + buffSize;
        ret = memcpy_s((char *)(platformHost->renderBufInfo.virtAddr) + wptr,
                       buffSize, txData->buf, buffSize);
        if (ret != 0) {
            AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
            return HDF_FAILURE;
        }
    }
    txData->status = ENUM_CIR_BUFF_NORMAL;
    return HDF_SUCCESS;
}

static int32_t SetWriteBuffWptr(struct PlatformHost *platformHost,
    unsigned int buffSize, struct AudioTxData *txData)
{
    int ret;
    int devId;
    int buffOffset;

    if (platformHost == NULL || txData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    devId = platformHost->renderBufInfo.chnId;
    ret = UpdateWriteBuffOffset(platformHost, buffSize, &buffOffset, txData);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("UpdateWptrRptr: failed.");
        return HDF_FAILURE;
    }
    if (txData->status == ENUM_CIR_BUFF_FULL) {
        AUDIO_DRIVER_LOG_DEBUG("not available buffer wait a minute.");
        return HDF_SUCCESS;
    }

    if (platformHost->renderBufInfo.runStatus == 1) {
        platformHost->pcmInfo.totalStreamSize += buffSize;
        ret = AopHalSetBuffWptr(devId, buffOffset);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AopHalSetBuffWptr failed.");
            return HDF_FAILURE;
        }
    }
    txData->status = ENUM_CIR_BUFF_NORMAL;
    return HDF_SUCCESS;
}
int32_t PlatformWriteProcBigEndian(const struct AudioCard *card, struct AudioTxData *txData)
{
    uint64_t buffSize;
    struct PlatformHost *platformHost = NULL;
    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL ||
        txData == NULL || txData->buf == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is null.");
        return HDF_FAILURE;
    }
    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("from PlatformHostFromDevice get platformHost is NULL.");
        return HDF_FAILURE;
    }
    buffSize = txData->frames * platformHost->pcmInfo.frameSize;
    if (platformHost->pcmInfo.isBigEndian) {
        if (AudioDataBigEndianChange(txData->buf, buffSize, platformHost->pcmInfo.bitWidth) != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioDataBigEndianChange: failed.");
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

int32_t PlatformWriteExec(const struct AudioCard *card, struct AudioTxData *txData)
{
    uint64_t buffSize;
    uint64_t startThreshold;
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry.");
    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL ||
        txData == NULL || txData->buf == NULL) {
        AUDIO_DRIVER_LOG_ERR("param is null.");
        return HDF_FAILURE;
    }

    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("from PlatformHostFromDevice get platformHost is NULL.");
        return HDF_FAILURE;
    }

    OsalMutexLock(&platformHost->renderBufInfo.buffMutex);
    if (platformHost->renderBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformWrite renderBufInfo.virtAddr is nullptr.");
        OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    buffSize = txData->frames * platformHost->pcmInfo.frameSize;
    startThreshold = platformHost->pcmInfo.startThreshold * platformHost->pcmInfo.frameSize;

    if (buffSize >= platformHost->renderBufInfo.cirBufSize) {
        AUDIO_DRIVER_LOG_ERR("stream data too long.");
        OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
        return HDF_FAILURE;
    }

    if (platformHost->renderBufInfo.runStatus == 1) {
        if ((platformHost->renderBufInfo.enable == 0) &&
            (platformHost->pcmInfo.totalStreamSize > startThreshold)) {
            AopHalDevEnable(platformHost->captureBufInfo.chnId);
            platformHost->renderBufInfo.enable = 1;
        }
    }

    if (SetWriteBuffWptr(platformHost, buffSize, txData) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("SetWriteBuffWptr: failed.");
        OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);

    AUDIO_DRIVER_LOG_DEBUG("now total = %d", platformHost->pcmInfo.totalStreamSize);
    return HDF_SUCCESS;
}

int32_t PlatformWrite(const struct AudioCard *card, struct AudioTxData *txData)
{
    int32_t ret;
    ret = PlatformWriteProcBigEndian(card, txData);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    return PlatformWriteExec(card, txData);
}

static int32_t PlatformMmapWriteData(const struct AudioCard *card, struct AudioTxData *txData)
{
    int32_t ret;
    int tryCount = LOOP_COUNT;

    if (card == NULL || txData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is null!");
        return HDF_FAILURE;
    }
    ret = PlatformWriteProcBigEndian(card, txData);
    if (ret != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    while (tryCount > 0) {
        ret = PlatformWriteExec(card, txData);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("PlatformWrite failed.");
            return HDF_FAILURE;
        }
        if (txData->status != ENUM_CIR_BUFF_FULL) {
            break;
        }
        tryCount--;
        OsalMDelay(DELAY_TIME);
    }
    if (tryCount <= 0) {
        AUDIO_DRIVER_LOG_ERR("frame drop");
    }
    return HDF_SUCCESS;
}

static int32_t PlatformMmapWriteSub(const struct AudioCard *card, struct PlatformHost *platformHost,
    const struct AudioTxMmapData *txMmapData)
{
    int32_t ret;
    int32_t count = 1;
    struct AudioTxData txData;

    ret = (card == NULL || platformHost == NULL || txMmapData == NULL || txMmapData->memoryAddress == NULL
        || txMmapData->transferFrameSize <= 0 || txMmapData->totalBufferFrames <= 0);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    uint32_t offset = txMmapData->offset;
    uint32_t frameSize = platformHost->pcmInfo.channels * (platformHost->pcmInfo.bitWidth / MIN_PERIOD_COUNT);
    uint32_t totalSize = txMmapData->totalBufferFrames * frameSize;
    uint32_t buffSize = txMmapData->transferFrameSize * frameSize;
    uint32_t pageSize = ((buffSize % MIN_PERIOD_SIZE) == 0) ?
                         buffSize : (buffSize / MIN_PERIOD_SIZE + 1) * MIN_PERIOD_SIZE;
    uint32_t lastBuffSize = ((totalSize % pageSize) == 0) ? pageSize : (totalSize % pageSize);
    uint32_t loopTimes = (lastBuffSize == pageSize) ? (totalSize / pageSize) : (totalSize / pageSize + 1);
    txData.frames = pageSize / frameSize;
    txData.buf = OsalMemCalloc(pageSize);
    if (txData.buf == NULL) {
        AUDIO_DRIVER_LOG_ERR("txData.buf is null.");
        return HDF_FAILURE;
    }
    platformHost->renderBufInfo.framesPosition = 0;
    while (count <= loopTimes) {
        uint32_t copyLength = (count < loopTimes) ? pageSize : lastBuffSize;
        ret = CopyFromUser(txData.buf, txMmapData->memoryAddress + offset, copyLength);
        if (ret != EOK) {
            AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
            OsalMemFree(txData.buf);
            return HDF_FAILURE;
        }
        ret = PlatformMmapWriteData(card, &txData);
        if (ret != HDF_SUCCESS) {
            OsalMemFree(txData.buf);
            return HDF_FAILURE;
        }
        offset += copyLength;
        count++;
        platformHost->renderBufInfo.framesPosition += copyLength / frameSize;
    }
    OsalMemFree(txData.buf);
    return HDF_SUCCESS;
}

int32_t PlatformMmapWrite(const struct AudioCard *card, struct AudioTxMmapData *txMmapData)
{
    int32_t ret;
    struct PlatformHost *platformHost = NULL;

    AUDIO_DRIVER_LOG_DEBUG("entry.");
    ret = (card == NULL || card->rtd == NULL || card->rtd->platform == NULL || txMmapData == NULL
        || txMmapData->memoryAddress == NULL || txMmapData->transferFrameSize <= 0
        || txMmapData->totalBufferFrames <= 0);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformHostFromDevice fail.");
        return HDF_FAILURE;
    }
    OsalMutexLock(&platformHost->renderBufInfo.buffMutex);
    if (platformHost->renderBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("renderBufInfo.virtAddr is nullptr.");
        OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    if (!platformHost->pcmInfo.channels) {
        platformHost->pcmInfo.channels = CHANNEL_MAX_NUM;
    }
    if (!platformHost->pcmInfo.bitWidth) {
        platformHost->pcmInfo.bitWidth = DATA_BIT_WIDTH16;
    }
    OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);

    ret = PlatformMmapWriteSub(card, platformHost, txMmapData);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformMmapWriteSub failed.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("render mmap write success.");
    return HDF_SUCCESS;
}

static int32_t UpdateReadBuffData(const struct PlatformHost *platformHost,
                                  unsigned int *tranferSize, unsigned int *buffOffset,
                                  struct AudioRxData *rxData)
{
    unsigned int dataAvailable;
    unsigned int validData;
    unsigned int wptr;
    unsigned int rptr;
    int devId;
    devId = platformHost->captureBufInfo.chnId;
    // Buf/DMA offset
    rptr = AiaoHalReadReg(AipBuffRptrReg(devId));
    wptr = AiaoHalReadReg(AipBuffWptrReg(devId));
    // Buf manage
    if (wptr >= rptr) {
        // [S ... R ... W ... E]
        dataAvailable = wptr - rptr;

        if (dataAvailable >= platformHost->captureBufInfo.trafBufSize) {
            rxData->buf = (char *)(platformHost->captureBufInfo.virtAddr) + rptr;
            *tranferSize = platformHost->captureBufInfo.trafBufSize;
            *buffOffset = rptr + *tranferSize;
        } else {
            AUDIO_DRIVER_LOG_DEBUG("PlatformRead: not available data.");
            rxData->buf = (char *)(platformHost->captureBufInfo.virtAddr) + rptr;
            rxData->status = ENUM_CIR_BUFF_EMPTY;
            rxData->bufSize = 0;
            rxData->frames = 0;
            return HDF_SUCCESS;
        }

    AUDIO_DRIVER_LOG_DEBUG("tranferSize : %d  buffOffset : %d ", *tranferSize, *buffOffset);
    } else {
        // [S ... W ... R ... E]
        validData = rptr + platformHost->captureBufInfo.trafBufSize;
        if (validData < platformHost->captureBufInfo.cirBufSize) {
            rxData->buf = (char *)(platformHost->captureBufInfo.virtAddr) + rptr;
            *tranferSize = platformHost->captureBufInfo.trafBufSize;
            *buffOffset = rptr + *tranferSize;
        } else {
            rxData->buf = (char *)(platformHost->captureBufInfo.virtAddr) + rptr;
            *tranferSize = platformHost->captureBufInfo.cirBufSize - rptr;
            *buffOffset = 0;
        }
        AUDIO_DRIVER_LOG_DEBUG("tranferSize : %d  rptrReg.u32 : %d ", *tranferSize, rptr);
    }

    AUDIO_DRIVER_LOG_DEBUG("rptrReg = [0x%08x], wptrReg = [0x%08x], max size = [%u]",
        rptr, wptr, platformHost->captureBufInfo.trafBufSize);

    return HDF_SUCCESS;
}

int32_t PlatformRead(const struct AudioCard *card, struct AudioRxData *rxData)
{
    unsigned int buffOffset;
    struct PlatformHost *platformHost = NULL;
    int devId;
    unsigned int tranferSize;

    if (rxData == NULL || card == NULL || card->rtd == NULL || card->rtd->platform == NULL) {
        AUDIO_DRIVER_LOG_ERR("param is null.");
        return HDF_FAILURE;
    }

    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformHostFromDevice: fail.");
        return HDF_FAILURE;
    }
    devId = platformHost->captureBufInfo.chnId;

    OsalMutexLock(&platformHost->captureBufInfo.buffMutex);
    if (platformHost->captureBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformWrite: capture data buffer is not initialized.");
        OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);
        return HDF_FAILURE;
    }

    if (UpdateReadBuffData(platformHost, &tranferSize, &buffOffset, rxData) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("UpdateReadBuffData failed.");
        OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);

    if (rxData->status == ENUM_CIR_BUFF_EMPTY) {
        AUDIO_DRIVER_LOG_DEBUG("not available data wait a minute.");
        return HDF_SUCCESS;
    }

    if (!platformHost->pcmInfo.isBigEndian) {
        if (AudioDataBigEndianChange(rxData->buf, tranferSize, platformHost->pcmInfo.bitWidth) != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioDataBigEndianChange: failed.");
            return HDF_FAILURE;
        }
    }

    rxData->frames = tranferSize / platformHost->pcmInfo.frameSize;
    rxData->bufSize = tranferSize;
    rxData->status = ENUM_CIR_BUFF_NORMAL;

    if (buffOffset >= platformHost->captureBufInfo.cirBufSize) {
        buffOffset = 0;
    }

    if (AipHalSetBuffRptr(devId, buffOffset) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBuffRptr failed.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t PlatformMmapReadSub(const struct AudioCard *card, struct PlatformHost *platformHost,
    const struct AudioRxMmapData *rxMmapData)
{
    int32_t ret;
    struct AudioRxData rxData;
    uint32_t offset = 0;
    uint32_t frameSize;
    int totalSize;
    int count = LOOP_COUNT;

    ret = (card == NULL || platformHost == NULL || rxMmapData == NULL || rxMmapData->memoryAddress == NULL
        || rxMmapData->totalBufferFrames <= 0);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    frameSize = platformHost->pcmInfo.channels * (platformHost->pcmInfo.bitWidth / MIN_PERIOD_COUNT);
    totalSize = rxMmapData->totalBufferFrames * frameSize;
    platformHost->captureBufInfo.framesPosition = 0;
    do {
        rxData.status = ENUM_CIR_BUFF_NORMAL;
        ret = PlatformRead(card, &rxData);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("PlatformRead failed.");
            return HDF_FAILURE;
        }
        if (rxData.status == ENUM_CIR_BUFF_EMPTY) {
            if (count > 0) {
                count--;
                OsalMDelay(DELAY_TIME);
                continue;
            } else {
                AUDIO_DRIVER_LOG_ERR("Loop out of 50 times.");
                return HDF_FAILURE;
            }
        }
        count = LOOP_COUNT;
        ret = CopyToUser(rxMmapData->memoryAddress + offset, rxData.buf, rxData.bufSize);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("CopyToUser failed.");
            return HDF_FAILURE;
        }
        offset += rxData.bufSize;
        platformHost->captureBufInfo.framesPosition += rxData.bufSize / frameSize;
    } while (offset < totalSize);
    return HDF_SUCCESS;
}

int32_t PlatformMmapRead(const struct AudioCard *card, struct AudioRxMmapData *rxMmapData)
{
    int32_t ret;
    struct PlatformHost *platformHost = NULL;

    AUDIO_DRIVER_LOG_DEBUG("entry.");
    ret = (card == NULL || card->rtd == NULL || card->rtd->platform == NULL || rxMmapData == NULL
        || rxMmapData->memoryAddress == NULL || rxMmapData->totalBufferFrames <= 0);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformHostFromDevice is invalid.");
        return HDF_FAILURE;
    }
    if (!platformHost->pcmInfo.channels) {
        platformHost->pcmInfo.channels = CHANNEL_MAX_NUM;
    }
    if (!platformHost->pcmInfo.bitWidth) {
        platformHost->pcmInfo.bitWidth = DATA_BIT_WIDTH16;
    }
    ret = PlatformMmapReadSub(card, platformHost, rxMmapData);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformMmapReadSub failed.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("capture mmap read success.");
    return HDF_SUCCESS;
}

int32_t PlatformRenderStart(struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    platformHost->renderBufInfo.runStatus = 1;
    platformHost->renderBufInfo.enable = 0;

    ret = AudioSampSetPowerMonitor(card, false);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampSetPowerMonitor failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("audio render start");
    return HDF_SUCCESS;
}

int32_t PlatformCaptureStart(struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    AipHalSetRxStart(platformHost->captureBufInfo.chnId, HI_TRUE);

    ret = AudioSampSetPowerMonitor(card, false);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampSetPowerMonitor failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("audio capture start");
    return HDF_SUCCESS;
}

int32_t PlatformRenderStop(struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    OsalMutexLock(&platformHost->renderBufInfo.buffMutex);
    platformHost->renderBufInfo.runStatus = 0;
    AopHalSetTxStart(platformHost->renderBufInfo.chnId, HI_FALSE);
    ret = AudioRenderBuffFree(platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioRenderBuffFree failed.");
        OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);

    ret = AudioSampSetPowerMonitor(card, true);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampSetPowerMonitor failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("audio stream stop");
    return HDF_SUCCESS;
}

int32_t PlatformCaptureStop(struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }
    OsalMutexLock(&platformHost->captureBufInfo.buffMutex);
    AipHalSetRxStart(platformHost->captureBufInfo.chnId, HI_FALSE);
    ret = AudioCaptureBuffFree(platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioCaptureBuffFree failed.");
        OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);

    ret = AudioSampSetPowerMonitor(card, true);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampSetPowerMonitor failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("audio stream stop");
    return HDF_SUCCESS;
}

int32_t PlatformCapturePause(struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    AipHalSetRxStart(platformHost->captureBufInfo.chnId, HI_FALSE);

    ret = AudioSampSetPowerMonitor(card, true);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampSetPowerMonitor failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("audio stream pause");
    return HDF_SUCCESS;
}

int32_t PlatformRenderPause(struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    platformHost->renderBufInfo.runStatus = 0;
    AopHalSetTxStart(platformHost->renderBufInfo.chnId, HI_FALSE);

    ret = AudioSampSetPowerMonitor(card, true);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampSetPowerMonitor failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("audio stream pause");
    return HDF_SUCCESS;
}

int32_t PlatformRenderResume(struct AudioCard *card)
{
    int ret = PlatformRenderStart(card);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformRenderResume failed.");
        return HDF_FAILURE;
    }
    ret = AudioSampPowerUp(card);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampPowerUp failed.");
        return HDF_FAILURE;
    }
    ret = AudioSampSetPowerMonitor(card, false);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampSetPowerMonitor failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("audio stream resume");
    return HDF_SUCCESS;
}

int32_t PlatformCaptureResume(struct AudioCard *card)
{
    int ret;
    (void)card;
    ret = PlatformCaptureStart(card);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCaptureStart failed.");
        return HDF_FAILURE;
    }
    ret = AudioSampPowerUp(card);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampPowerUp is failure.");
        return HDF_FAILURE;
    }
    ret = AudioSampSetPowerMonitor(card, false);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioSampSetPowerMonitor is failure.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("audio stream resume");
    return HDF_SUCCESS;
}
