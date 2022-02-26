/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "spi_core.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "spi_if.h"

#define HDF_LOG_TAG spi_core

int32_t SpiCntlrOpen(struct SpiCntlr *cntlr, uint32_t csNum)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (cntlr->method == NULL || cntlr->method->Open == NULL) {
        HDF_LOGE("%s: Open not support", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }
    (void)OsalMutexLock(&(cntlr->lock));
    cntlr->curCs = csNum;
    ret = cntlr->method->Open(cntlr);
    (void)OsalMutexUnlock(&(cntlr->lock));
    return ret;
}

int32_t SpiCntlrClose(struct SpiCntlr *cntlr, uint32_t csNum)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (cntlr->method == NULL || cntlr->method->Close == NULL) {
        HDF_LOGE("%s: Close not support", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }
    (void)OsalMutexLock(&(cntlr->lock));
    cntlr->curCs = csNum;
    ret = cntlr->method->Close(cntlr);
    (void)OsalMutexUnlock(&(cntlr->lock));
    return ret;
}

int32_t SpiCntlrTransfer(struct SpiCntlr *cntlr, uint32_t csNum, struct SpiMsg *msg, uint32_t count)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (cntlr->method == NULL || cntlr->method->Transfer == NULL) {
        HDF_LOGE("%s: transfer not support", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    (void)OsalMutexLock(&(cntlr->lock));
    cntlr->curCs = csNum;
    ret = cntlr->method->Transfer(cntlr, msg, count);
    (void)OsalMutexUnlock(&(cntlr->lock));
    return ret;
}

int32_t SpiCntlrSetCfg(struct SpiCntlr *cntlr, uint32_t csNum, struct SpiCfg *cfg)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    if (cntlr->method == NULL || cntlr->method->SetCfg == NULL) {
        HDF_LOGE("%s: not support", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    (void)OsalMutexLock(&(cntlr->lock));
    cntlr->curCs = csNum;
    ret = cntlr->method->SetCfg(cntlr, cfg);
    (void)OsalMutexUnlock(&(cntlr->lock));
    return ret;
}

int32_t SpiCntlrGetCfg(struct SpiCntlr *cntlr, uint32_t csNum, struct SpiCfg *cfg)
{
    int32_t ret;

    if (cntlr == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    if (cntlr->method == NULL || cntlr->method->GetCfg == NULL) {
        HDF_LOGE("%s: not support", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    (void)OsalMutexLock(&(cntlr->lock));
    cntlr->curCs = csNum;
    ret = cntlr->method->GetCfg(cntlr, cfg);
    (void)OsalMutexUnlock(&(cntlr->lock));
    return ret;
}

void SpiCntlrDestroy(struct SpiCntlr *cntlr)
{
    if (cntlr == NULL) {
        return;
    }
    (void)OsalMutexDestroy(&(cntlr->lock));
    OsalMemFree(cntlr);
}

struct SpiCntlr *SpiCntlrCreate(struct HdfDeviceObject *device)
{
    struct SpiCntlr *cntlr = NULL;

    if (device == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return NULL;
    }

    cntlr = (struct SpiCntlr *)OsalMemCalloc(sizeof(*cntlr));
    if (cntlr == NULL) {
        HDF_LOGE("%s: OsalMemCalloc error", __func__);
        return NULL;
    }
    cntlr->device = device;
    device->service = &(cntlr->service);
    (void)OsalMutexInit(&cntlr->lock);
    DListHeadInit(&cntlr->list);
    cntlr->priv = NULL;
    return cntlr;
}
