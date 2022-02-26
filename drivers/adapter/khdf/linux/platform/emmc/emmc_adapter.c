/*
 * emmc_adapter.c
 *
 * hi35xx linux emmc driver implement.
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
#include <linux/mmc/host.h>
#include <securec.h>
#include "mmc_corex.h"
#include "mmc_emmc.h"
#include "device_resource_if.h"
#include "emmc_if.h"
#include "hdf_log.h"

#define HDF_LOG_TAG emmc_adapter_c

struct mmc_host *himci_get_mmc_host(int slot);

int32_t Hi35xxLinuxEmmcGetCid(struct EmmcDevice *dev, uint8_t *cid, uint32_t size)
{
    struct mmc_host *mmcHost = NULL;
    struct MmcCntlr *cntlr = NULL;

    if (dev == NULL || dev->mmc.cntlr == NULL) {
        HDF_LOGE("Hi35xxLinuxEmmcGetCid: dev or cntlr is null.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cid == NULL || size < EMMC_CID_LEN) {
        HDF_LOGE("Hi35xxLinuxEmmcGetCid: cid is null or size is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    cntlr = dev->mmc.cntlr;
    mmcHost = (struct mmc_host *)cntlr->priv;
    if (mmcHost == NULL) {
        HDF_LOGE("Hi35xxLinuxEmmcGetCid: mmcHost is NULL!");
        return HDF_ERR_NOT_SUPPORT;
    }
    if (mmcHost->card == NULL) {
        HDF_LOGE("Hi35xxLinuxEmmcGetCid: card is null.");
        return HDF_ERR_NOT_SUPPORT;
    }
    if (memcpy_s(cid, sizeof(uint8_t) * size, (uint8_t *)(mmcHost->card->raw_cid),
        sizeof(mmcHost->card->raw_cid)) != EOK) {
        HDF_LOGE("Hi35xxLinuxEmmcGetCid: memcpy_s fail!");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static struct EmmcDeviceOps g_emmcMethod = {
    .getCid = Hi35xxLinuxEmmcGetCid,
};

static void Hi35xxLinuxEmmcDeleteCntlr(struct MmcCntlr *cntlr)
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

static int32_t Hi35xxLinuxEmmcCntlrParse(struct MmcCntlr *cntlr, struct HdfDeviceObject *obj)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (obj == NULL || cntlr == NULL) {
        HDF_LOGE("Hi35xxLinuxEmmcCntlrParse: input para is NULL.");
        return HDF_FAILURE;
    }

    node = obj->property;
    if (node == NULL) {
        HDF_LOGE("Hi35xxLinuxEmmcCntlrParse: drs node is NULL.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetUint16 == NULL || drsOps->GetUint32 == NULL) {
        HDF_LOGE("Hi35xxLinuxEmmcCntlrParse: invalid drs ops fail!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetUint16(node, "hostId", &(cntlr->index), 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxEmmcCntlrParse: read hostIndex fail!");
        return ret;
    }
    ret = drsOps->GetUint32(node, "devType", &(cntlr->devType), 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxEmmcCntlrParse: read devType fail!");
        return ret;
    }
    return HDF_SUCCESS;
}

static int32_t Hi35xxLinuxEmmcBind(struct HdfDeviceObject *obj)
{
    struct MmcCntlr *cntlr = NULL;
    int32_t ret;

    HDF_LOGE("Hi35xxLinuxEmmcBind: entry!");
    if (obj == NULL) {
        HDF_LOGE("Hi35xxLinuxEmmcBind: Fail, obj is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    cntlr = (struct MmcCntlr *)OsalMemCalloc(sizeof(struct MmcCntlr));
    if (cntlr == NULL) {
        HDF_LOGE("Hi35xxLinuxEmmcBind: no mem for MmcCntlr.");
        return HDF_ERR_MALLOC_FAIL;
    }

    cntlr->ops = NULL;
    cntlr->hdfDevObj = obj;
    obj->service = &cntlr->service;
    ret = Hi35xxLinuxEmmcCntlrParse(cntlr, obj);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxEmmcBind: cntlr parse fail.");
        goto _ERR;
    }
    cntlr->priv = (void *)himci_get_mmc_host((int32_t)cntlr->index);

    ret = MmcCntlrAdd(cntlr);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxEmmcBind: cntlr add fail.");
        goto _ERR;
    }

    ret = MmcCntlrAllocDev(cntlr, (enum MmcDevType)cntlr->devType);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Hi35xxLinuxEmmcBind: alloc dev fail.");
        goto _ERR;
    }
    MmcDeviceAddOps(cntlr->curDev, &g_emmcMethod);
    HDF_LOGD("Hi35xxLinuxEmmcBind: Success!");
    return HDF_SUCCESS;

_ERR:
    Hi35xxLinuxEmmcDeleteCntlr(cntlr);
    HDF_LOGE("Hi35xxLinuxEmmcBind: Fail!");
    return HDF_FAILURE;
}

static int32_t Hi35xxLinuxEmmcInit(struct HdfDeviceObject *obj)
{
    (void)obj;

    HDF_LOGD("Hi35xxLinuxEmmcInit: Success!");
    return HDF_SUCCESS;
}

static void Hi35xxLinuxEmmcRelease(struct HdfDeviceObject *obj)
{
    if (obj == NULL) {
        return;
    }
    Hi35xxLinuxEmmcDeleteCntlr((struct MmcCntlr *)obj->service);
}

struct HdfDriverEntry g_emmcDriverEntry = {
    .moduleVersion = 1,
    .Bind = Hi35xxLinuxEmmcBind,
    .Init = Hi35xxLinuxEmmcInit,
    .Release = Hi35xxLinuxEmmcRelease,
    .moduleName = "HDF_PLATFORM_EMMC",
};
HDF_INIT(g_emmcDriverEntry);
