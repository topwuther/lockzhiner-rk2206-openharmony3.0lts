/*
 * sdio_adapter.c
 *
 * hi35xx linux sdio driver implement.
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include "plat_log.h"
#include "device_resource_if.h"
#include "mmc_corex.h"
#include "mmc_sdio.h"

#define HDF_LOG_TAG sdio_adapter_c

#define DATA_LEN_ONE_BYTE 1
#define DATA_LEN_TWO_BYTES 2
#define DATA_LEN_FOUR_BYTES 4
#define MMC_SLOT_NUM 3
#define SDIO_RESCAN_WAIT_TIME 40

struct mmc_host *himci_get_mmc_host(int slot);
void hisi_sdio_rescan(int slot);

static struct sdio_func *Hi35xxLinuxSdioGetFunc(struct SdioDevice *dev)
{
    if (dev == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioGetFunc: dev is null.");
        return NULL;
    }
    return (struct sdio_func *)dev->sd.mmc.priv;
}

static int32_t Hi35xxLinuxSdioIncrAddrReadBytes(struct SdioDevice *dev,
    uint8_t *data, uint32_t addr, uint32_t size)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);
    int32_t ret = HDF_SUCCESS;
    uint16_t *output16 = NULL;
    uint32_t *output32 = NULL;

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioIncrAddrReadBytes: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if ((data == NULL) || (size == 0)) {
        HDF_LOGE("Hi35xxLinuxSdioIncrAddrReadBytes: data or size is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    if (size == DATA_LEN_ONE_BYTE) {
        *data = sdio_readb(func, addr, &ret);
        return ret;
    }
    if (size == DATA_LEN_TWO_BYTES) {
        output16 = (uint16_t *)data;
        *output16 = sdio_readw(func, addr, &ret);
        return ret;
    }
    if (size == DATA_LEN_FOUR_BYTES) {
        output32 = (uint32_t *)data;
        *output32 = sdio_readl(func, addr, &ret);
        return ret;
    }
    return sdio_memcpy_fromio(func, data, addr, size);
}

static int32_t Hi35xxLinuxSdioIncrAddrWriteBytes(struct SdioDevice *dev,
    uint8_t *data, uint32_t addr, uint32_t size)
{
    int32_t ret = HDF_SUCCESS;
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioIncrAddrWriteBytes: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if ((data == NULL) || (size == 0)) {
        HDF_LOGE("Hi35xxLinuxSdioIncrAddrWriteBytes: data or size is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    if (size == DATA_LEN_ONE_BYTE) {
        sdio_writeb(func, *data, addr, &ret);
        return ret;
    }
    if (size == DATA_LEN_TWO_BYTES) {
        sdio_writew(func, *(uint16_t *)data, addr, &ret);
        return ret;
    }
    if (size == DATA_LEN_FOUR_BYTES) {
        sdio_writel(func, *(uint32_t *)data, addr, &ret);
        return ret;
    }
    return sdio_memcpy_toio(func, addr, data, size);
}

static int32_t Hi35xxLinuxSdioFixedAddrReadBytes(struct SdioDevice *dev,
    uint8_t *data, uint32_t addr, uint32_t size, uint32_t scatterLen)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioFixedAddrReadBytes: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if ((data == NULL) || (size == 0)) {
        HDF_LOGE("Hi35xxLinuxSdioFixedAddrReadBytes: data or size is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    if (scatterLen > 0) {
        HDF_LOGE("Hi35xxLinuxSdioFixedAddrReadBytes: not support!");
        return HDF_ERR_NOT_SUPPORT;
    }
    return sdio_readsb(func, data, addr, size);
}

static int32_t Hi35xxLinuxSdioFixedAddrWriteBytes(struct SdioDevice *dev,
    uint8_t *data, uint32_t addr, uint32_t size, uint32_t scatterLen)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioFixedAddrWriteBytes: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if ((data == NULL) || (size == 0)) {
        HDF_LOGE("Hi35xxLinuxSdioFixedAddrReadBytes: data or size is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    if (scatterLen > 0) {
        HDF_LOGE("Hi35xxLinuxSdioFixedAddrWriteBytes: not support!");
        return HDF_ERR_NOT_SUPPORT;
    }
    return sdio_writesb(func, addr, data, size);
}

static int32_t Hi35xxLinuxSdioFunc0ReadBytes(struct SdioDevice *dev,
    uint8_t *data, uint32_t addr, uint32_t size)
{
    int32_t ret = HDF_SUCCESS;
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioFunc0ReadBytes: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if ((data == NULL) || (size == 0)) {
        HDF_LOGE("Hi35xxLinuxSdioFunc0ReadBytes: data or size is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    *data = sdio_f0_readb(func, addr, &ret);
    return ret;
}

static int32_t Hi35xxLinuxSdioFunc0WriteBytes(struct SdioDevice *dev,
    uint8_t *data, uint32_t addr, uint32_t size)
{
    int32_t ret = HDF_SUCCESS;
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioFunc0WriteBytes: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if ((data == NULL) || (size == 0)) {
        HDF_LOGE("Hi35xxLinuxSdioFunc0WriteBytes: data or size is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    sdio_f0_writeb(func, *data, addr, &ret);
    return ret;
}

static int32_t Hi35xxLinuxSdioSetBlockSize(struct SdioDevice *dev, uint32_t blockSize)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioSetBlockSize, func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    return sdio_set_block_size(func, blockSize);
}

static int32_t Hi35xxLinuxSdioGetCommonInfo(struct SdioDevice *dev,
    SdioCommonInfo *info, uint32_t infoType)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioGetCommonInfo: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (info == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioGetCommonInfo: info is null.");
        return HDF_ERR_INVALID_PARAM;
    }
    if (infoType != SDIO_FUNC_INFO) {
        HDF_LOGE("Hi35xxLinuxSdioGetCommonInfo: cur type %d is not support.", infoType);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (func->card == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioGetCommonInfo fail, card is null.");
        return HDF_ERR_INVALID_PARAM;
    }
    if (func->card->host == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioGetCommonInfo fail, host is null.");
        return HDF_ERR_INVALID_PARAM;
    }
    info->funcInfo.enTimeout = func->enable_timeout;
    info->funcInfo.maxBlockNum = func->card->host->max_blk_count;
    info->funcInfo.maxBlockSize = func->card->host->max_blk_size;
    info->funcInfo.maxRequestSize = func->card->host->max_req_size;
    info->funcInfo.funcNum = func->num;
    info->funcInfo.irqCap = func->card->host->caps & MMC_CAP_SDIO_IRQ;
    info->funcInfo.data = func;
    return HDF_SUCCESS;
}

static int32_t Hi35xxLinuxSdioSetCommonInfo(struct SdioDevice *dev,
    SdioCommonInfo *info, uint32_t infoType)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioSetCommonInfo: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (info == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioSetCommonInfo: info is null.");
        return HDF_ERR_INVALID_PARAM;
    }
    if (infoType != SDIO_FUNC_INFO) {
        HDF_LOGE("Hi35xxLinuxSdioSetCommonInfo: cur type %d is not support.", infoType);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (func->card == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioSetCommonInfo fail, card is null.");
        return HDF_ERR_INVALID_PARAM;
    }
    if (func->card->host == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioSetCommonInfo fail, host is null.");
        return HDF_ERR_INVALID_PARAM;
    }
    func->enable_timeout = info->funcInfo.enTimeout;
    func->card->host->max_blk_count = info->funcInfo.maxBlockNum;
    func->card->host->max_blk_size = info->funcInfo.maxBlockSize;
    func->card->host->max_req_size = info->funcInfo.maxRequestSize;
    func->num = info->funcInfo.funcNum;
    return HDF_SUCCESS;
}

static int32_t Hi35xxLinuxSdioFlushData(struct SdioDevice *dev)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioFlushData: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (func->card == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioFlushData: card is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    return mmc_sw_reset(func->card->host);
}

static int32_t Hi35xxLinuxSdioClaimHost(struct SdioDevice *dev)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioClaimHost: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    sdio_claim_host(func);
    return HDF_SUCCESS;
}

static int32_t Hi35xxLinuxSdioReleaseHost(struct SdioDevice *dev)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioReleaseHost: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    sdio_release_host(func);
    return HDF_SUCCESS;
}

static int32_t Hi35xxLinuxSdioEnableFunc(struct SdioDevice *dev)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioEnableFunc: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    return sdio_enable_func(func);
}

static int32_t Hi35xxLinuxSdioDisableFunc(struct SdioDevice *dev)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioDisableFunc: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    return sdio_disable_func(func);
}

static int32_t Hi35xxLinuxSdioClaimIrq(struct SdioDevice *dev, SdioIrqHandler *handler)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioClaimIrq: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (handler == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioClaimIrq: handler is null.");
        return HDF_ERR_INVALID_PARAM;
    }
    return sdio_claim_irq(func, (sdio_irq_handler_t *)handler);
}

static int32_t Hi35xxLinuxSdioReleaseIrq(struct SdioDevice *dev)
{
    struct sdio_func *func = Hi35xxLinuxSdioGetFunc(dev);

    if (func == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioReleaseIrq: func is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    return sdio_release_irq(func);
}

static struct sdio_func *Hi35xxLinuxSdioSearchFunc(uint32_t funcNum, uint16_t vendorId, uint16_t deviceId)
{
    struct mmc_card *card = NULL;
    struct mmc_host *host = NULL;
    struct sdio_func *func = NULL;
    uint32_t i, j;

    for (i = 0; i < MMC_SLOT_NUM; i++) {
        host = himci_get_mmc_host(i);
        if (host == NULL) {
            continue;
        }
        card = host->card;
        if (card == NULL) {
            continue;
        }
        for (j = 0; j <= card->sdio_funcs; j++) {
            func = card->sdio_func[j];
            if ((func != NULL) &&
                (func->num == funcNum) &&
                (func->vendor == vendorId) &&
                (func->device == deviceId)) {
                HDF_LOGD("Hi35xxLinuxSdioSearchFunc: find func!");
                return func;
            }
        }
    }

    HDF_LOGE("Hi35xxLinuxSdioSearchFunc: get sdio func fail!");
    return NULL;
}

static int32_t Hi35xxLinuxSdioFindFunc(struct SdioDevice *dev, struct SdioFunctionConfig *configData)
{
    if (dev == NULL || configData == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioFindFunc: dev or configData is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    dev->sd.mmc.priv = Hi35xxLinuxSdioSearchFunc(configData->funcNr, configData->vendorId, configData->deviceId);
    if (dev->sd.mmc.priv == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioFindFunc: Hi35xxLinuxSdioSearchFunc fail.");
        return HDF_ERR_NOT_SUPPORT;
    }
    return HDF_SUCCESS;
}

static struct SdioDeviceOps g_sdioDeviceOps = {
    .incrAddrReadBytes = Hi35xxLinuxSdioIncrAddrReadBytes,
    .incrAddrWriteBytes = Hi35xxLinuxSdioIncrAddrWriteBytes,
    .fixedAddrReadBytes = Hi35xxLinuxSdioFixedAddrReadBytes,
    .fixedAddrWriteBytes = Hi35xxLinuxSdioFixedAddrWriteBytes,
    .func0ReadBytes = Hi35xxLinuxSdioFunc0ReadBytes,
    .func0WriteBytes = Hi35xxLinuxSdioFunc0WriteBytes,
    .setBlockSize = Hi35xxLinuxSdioSetBlockSize,
    .getCommonInfo = Hi35xxLinuxSdioGetCommonInfo,
    .setCommonInfo = Hi35xxLinuxSdioSetCommonInfo,
    .flushData = Hi35xxLinuxSdioFlushData,
    .enableFunc = Hi35xxLinuxSdioEnableFunc,
    .disableFunc = Hi35xxLinuxSdioDisableFunc,
    .claimIrq = Hi35xxLinuxSdioClaimIrq,
    .releaseIrq = Hi35xxLinuxSdioReleaseIrq,
    .findFunc = Hi35xxLinuxSdioFindFunc,
    .claimHost = Hi35xxLinuxSdioClaimHost,
    .releaseHost = Hi35xxLinuxSdioReleaseHost,
};

static bool Hi35xxLinuxSdioRescanFinish(struct MmcCntlr *cntlr)
{
    struct mmc_host *host = NULL;
    struct mmc_card *card = NULL;

    host = himci_get_mmc_host(cntlr->index);
    if (host == NULL) {
        return false;
    }

    card = host->card;
    if (card == NULL) {
        return false;
    }
    if (card->sdio_funcs > 0) {
        return true;
    }
    return false;
}

static int32_t Hi35xxLinuxSdioRescan(struct MmcCntlr *cntlr)
{
    bool rescanFinish = false;
    uint32_t count = 0;

    if (cntlr == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioRescan: cntlr is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    hisi_sdio_rescan(cntlr->index);
    while (rescanFinish == false && count < SDIO_RESCAN_WAIT_TIME) {
        OsalMSleep(50);
        count++;
        rescanFinish = Hi35xxLinuxSdioRescanFinish(cntlr);
    }

    if (rescanFinish == false) {
        HDF_LOGE("Hi35xxLinuxSdioRescan: fail!");
        return HDF_FAILURE;
    }

    OsalMSleep(10);
    return HDF_SUCCESS;
}

struct MmcCntlrOps g_sdioCntlrOps = {
    .rescanSdioDev = Hi35xxLinuxSdioRescan,
};

static void Hi35xxLinuxSdioDeleteCntlr(struct MmcCntlr *cntlr)
{
    if (cntlr == NULL) {
        return;
    }

    if (cntlr->curDev != NULL) {
        MmcDeviceRemove(cntlr->curDev);
        OsalMemFree(cntlr->curDev);
    }
    MmcCntlrRemove(cntlr);
    OsalMemFree(cntlr);
}

static int32_t Hi35xxLinuxSdioCntlrParse(struct MmcCntlr *cntlr, struct HdfDeviceObject *obj)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (obj == NULL || cntlr == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioCntlrParse: input para is NULL.");
        return HDF_FAILURE;
    }

    node = obj->property;
    if (node == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioCntlrParse: drs node is NULL.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetUint16 == NULL || drsOps->GetUint32 == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioCntlrParse: invalid drs ops fail!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetUint16(node, "hostId", &(cntlr->index), 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxSdioCntlrParse: read hostIndex fail!");
        return ret;
    }
    ret = drsOps->GetUint32(node, "devType", &(cntlr->devType), 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxSdioCntlrParse: read devType fail!");
        return ret;
    }
    HDF_LOGD("Hi35xxLinuxSdioCntlrParse: hostIndex = %d, devType = %d.", cntlr->index, cntlr->devType);
    return HDF_SUCCESS;
}

static int32_t Hi35xxLinuxSdioBind(struct HdfDeviceObject *obj)
{
    struct MmcCntlr *cntlr = NULL;
    int32_t ret;

    HDF_LOGE("Hi35xxLinuxSdioBind: entry!");
    if (obj == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioBind: Fail, obj is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    cntlr = (struct MmcCntlr *)OsalMemCalloc(sizeof(struct MmcCntlr));
    if (cntlr == NULL) {
        HDF_LOGE("Hi35xxLinuxSdioBind: no mem for MmcCntlr.");
        return HDF_ERR_MALLOC_FAIL;
    }

    cntlr->ops = &g_sdioCntlrOps;
    cntlr->hdfDevObj = obj;
    obj->service = &cntlr->service;
    /* init cntlr. */
    ret = Hi35xxLinuxSdioCntlrParse(cntlr, obj);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxSdioBind: cntlr parse fail.");
        goto _ERR;
    }

    ret = MmcCntlrAdd(cntlr);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxSdioBind: cntlr add fail.");
        goto _ERR;
    }

    ret = MmcCntlrAllocDev(cntlr, (enum MmcDevType)cntlr->devType);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxSdioBind: alloc dev fail.");
        goto _ERR;
    }
    MmcDeviceAddOps(cntlr->curDev, &g_sdioDeviceOps);
    HDF_LOGD("Hi35xxLinuxSdioBind: Success!");
    return HDF_SUCCESS;

_ERR:
    Hi35xxLinuxSdioDeleteCntlr(cntlr);
    HDF_LOGE("Hi35xxLinuxSdioBind: Fail!");
    return HDF_FAILURE;
}

static int32_t Hi35xxLinuxSdioInit(struct HdfDeviceObject *obj)
{
    (void)obj;
    HDF_LOGD("Hi35xxLinuxSdioInit: Success!");
    return HDF_SUCCESS;
}

static void Hi35xxLinuxSdioRelease(struct HdfDeviceObject *obj)
{
    if (obj == NULL) {
        return;
    }
    Hi35xxLinuxSdioDeleteCntlr((struct MmcCntlr *)obj->service);
}

struct HdfDriverEntry g_sdioDriverEntry = {
    .moduleVersion = 1,
    .Bind = Hi35xxLinuxSdioBind,
    .Init = Hi35xxLinuxSdioInit,
    .Release = Hi35xxLinuxSdioRelease,
    .moduleName = "HDF_PLATFORM_SDIO",
};
HDF_INIT(g_sdioDriverEntry);
