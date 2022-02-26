/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "storage_block.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "mmc_corex.h"
#include "mmc_sd.h"
#include "osal_mem.h"
#include "securec.h"

static ssize_t MmcBlockRead(struct StorageBlock *sb, uint8_t *buf, size_t secStart, size_t nSec)
{
    if (sb == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (sb->type == MEDIA_MMC) {
        return MmcDeviceRead((struct MmcDevice *)sb->media, buf, secStart, nSec);
    }
    return HDF_ERR_NOT_SUPPORT;
}

static ssize_t MmcBlockWrite(struct StorageBlock *sb, const uint8_t *buf, size_t secStart, size_t nSec)
{
    if (sb == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (sb->type == MEDIA_MMC) {
        return MmcDeviceWrite((struct MmcDevice *)sb->media, (uint8_t *)buf, secStart, nSec);
    }
    return HDF_ERR_NOT_SUPPORT;
}

static ssize_t MmcBlockErase(struct StorageBlock *sb, size_t secStart, size_t nSec)
{
    if (sb == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (sb->type == MEDIA_MMC) {
        return MmcDeviceErase((struct MmcDevice *)sb->media, secStart, nSec);
    }
    return HDF_ERR_NOT_SUPPORT;
}

static uint32_t MmcBlockGetAuSize(struct StorageBlock *sb)
{
    struct MmcDevice *mmc = NULL;
    struct SdDevice *sd = NULL;

    if (sb == NULL || sb->media == NULL) {
        HDF_LOGE("MmcBlockGetAuSize: invalid sb or media!");
        return 0;
    }

    if (sb->type != MEDIA_MMC) {
        HDF_LOGE("MmcBlockGetAuSize: media is not mmc!");
        return 0;
    }

    mmc = (struct MmcDevice *)sb->media;
    if (mmc->type != MMC_DEV_SD) {
        HDF_LOGE("MmcBlockGetAuSize: media is not sd!");
        return 0;
    }
    sd = (struct SdDevice *)mmc;

    return sd->reg.ssr.auSize;
}

static bool MmcBlockIsPresent(struct StorageBlock *sb)
{
    return (sb != NULL && sb->type == MEDIA_MMC &&
        MmcDeviceIsPresent((struct MmcDevice *)sb->media));
}

static struct StorageBlockMethod g_mmcBlockOps = {
    .read = MmcBlockRead,
    .write = MmcBlockWrite,
    .erase = MmcBlockErase,
    .getAuSize = MmcBlockGetAuSize,
    .isPresent = MmcBlockIsPresent,
};

static int32_t MmcBlockInit(struct StorageBlock *sb, struct MmcDevice *mmc)
{
    int32_t ret;
    size_t nameSize;

    if (PlatformDeviceGet(&mmc->device) == NULL) {
        return HDF_PLT_ERR_DEV_GET;
    }
    mmc->sb = sb;
    sb->type = MEDIA_MMC;
    sb->media = mmc;
    sb->secSize = mmc->secSize;
    sb->capacity = mmc->capacity;
    sb->removeable = (mmc->state.bits.removeable == 0) ? false : true;
    sb->ops = &g_mmcBlockOps;
    nameSize = sizeof(sb->name);
    ret = snprintf_s(sb->name, nameSize, nameSize - 1, "/dev/mmcblk%0d", mmc->cntlr->index);
    if (ret <= 0) {
        PlatformDevicePut(&mmc->device);
    }

    return HDF_SUCCESS;
}

static void MmcBlockUninit(struct StorageBlock *sb)
{
    struct MmcDevice *mmc = (struct MmcDevice *)sb->media;

    mmc->sb = NULL;
    PlatformDevicePut(&mmc->device);
}

int32_t MmcBlockAdd(struct MmcDevice *mmc)
{
    int32_t ret;
    struct StorageBlock *sb = NULL;

    if (mmc == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    sb = (struct StorageBlock *)OsalMemCalloc(sizeof(*sb));
    if (sb == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = MmcBlockInit(sb, mmc);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(sb);
        return ret;
    }

    ret = StorageBlockAdd(sb);
    if (ret != HDF_SUCCESS) {
        MmcBlockUninit(sb);
        OsalMemFree(sb);
        return ret;
    }

    return HDF_SUCCESS;
}

void MmcBlockDel(struct MmcDevice *mmc)
{
    struct StorageBlock *sb = NULL;

    if (mmc == NULL) {
        return;
    }
    sb = mmc->sb;
    if (sb == NULL) {
        return;
    }

    StorageBlockDel(sb);
    MmcBlockUninit(sb);
    OsalMemFree(sb);
}
