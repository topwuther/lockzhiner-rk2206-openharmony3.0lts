/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "disk.h"
#include "fs/fs.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "storage_block.h"
#include "sys/ioctl.h"
#include "user_copy.h"

#define HDF_LOG_TAG storage_block_lite_c

/* block ioctl */
#define MMC_IOC_MAGIC   'm'
#define RT_DEVICE_CTRL_BLK_GETGEOME _IOR(MMC_IOC_MAGIC, 1, struct RtDeviceBlkGeometry)
#define RT_DEVICE_CARD_STATUS       _IOR(MMC_IOC_MAGIC, 2, int)
#define RT_DEVICE_CARD_AU_SIZE      _IOR(MMC_IOC_MAGIC, 3, unsigned int)
#define RT_DEVICE_BLOCK_ERROR_COUNT _IOR(MMC_IOC_MAGIC, 4, int)

#define RT_BLOCK_SIZE 512

/**
 * block device geometry structure
 */
struct RtDeviceBlkGeometry {
    unsigned long sectorCount;        /**< count of sectors */
    unsigned long bytesPerSector;     /**< number of bytes per sector */
    unsigned long blockSize;          /**< number of bytes to erase one block */
};

struct disk_divide_info g_emmcInfo = {.sector_count = 0xffffffff};

struct disk_divide_info *StorageBlockGetEmmc(void)
{
    return &g_emmcInfo;
}

char *StorageBlockGetEmmcNodeName(void *block)
{
    struct StorageBlock *sb = (struct StorageBlock *)block;
 
    if (sb == NULL) {
        return NULL;
    }
    return sb->name;
}

static int LiteosBlockOpen(FAR struct Vnode *vnode)
{
    (void)vnode;
    return 0;
}

static int LiteosBlockClose(FAR struct Vnode *vnode)
{
    (void)vnode;
    return 0;
}

static ssize_t LiteosBlockRead(FAR struct Vnode *vnode, FAR unsigned char *buf,
    unsigned long long secStart, unsigned int nSecs)
{
    size_t max = (size_t)(-1);
    struct StorageBlock *sb = (struct StorageBlock *)((struct drv_data*)vnode->data)->priv;

    if (secStart >= max || nSecs >= max) {
        return HDF_ERR_INVALID_PARAM;
    }
    return StorageBlockRead(sb, buf, (size_t)secStart, (size_t)nSecs);
}

static ssize_t LiteosBlockWrite(FAR struct Vnode *vnode, FAR const unsigned char *buf,
    unsigned long long secStart, unsigned int nSecs)
{
    size_t max = (size_t)(-1);
    struct StorageBlock *sb = (struct StorageBlock *)((struct drv_data*)vnode->data)->priv;

    if (secStart >= max || nSecs >= max) {
        return HDF_ERR_INVALID_PARAM;
    }
    return StorageBlockWrite(sb, buf, (size_t)secStart, (size_t)nSecs);
}

static int LiteosBlockGeometry(FAR struct Vnode *vnode, FAR struct geometry *geometry)
{
    struct StorageBlock *sb = (struct StorageBlock *)((struct drv_data*)vnode->data)->priv;

    if (sb == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (geometry == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    geometry->geo_available    = true;
    geometry->geo_mediachanged = false;
    geometry->geo_writeenabled = true;
    geometry->geo_nsectors = sb->capacity; /* StorageBlock sized by sectors */
    geometry->geo_sectorsize = sb->secSize;
    return HDF_SUCCESS;
}

static int32_t LiteosBlockSaveGeometry(FAR struct Vnode *vnode, unsigned long arg)
{
    int32_t ret;
    struct geometry gm;
    struct RtDeviceBlkGeometry rtGeo = {0};

    ret = LiteosBlockGeometry(vnode, &gm);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    rtGeo.sectorCount = gm.geo_nsectors;
    rtGeo.bytesPerSector = gm.geo_sectorsize;
    rtGeo.blockSize = RT_BLOCK_SIZE;
    ret = LOS_CopyFromKernel((void *)(uintptr_t)arg, sizeof(struct RtDeviceBlkGeometry),
              &rtGeo, sizeof(struct RtDeviceBlkGeometry));
    if (ret != 0) {
        return HDF_ERR_IO;
    }
    return HDF_SUCCESS;
}

static int32_t LiteosBlockIoctl(FAR struct Vnode *vnode, int cmd, unsigned long arg)
{
    int32_t ret;
    int flag, errCnt;
    unsigned int au;
    uint32_t auSize;
    struct StorageBlock *sb = NULL;

    sb = (struct StorageBlock *)((struct drv_data*)vnode->data)->priv;

    switch (cmd) {
        case RT_DEVICE_CTRL_BLK_GETGEOME:
            ret = LiteosBlockSaveGeometry(vnode, arg);
            break;
        case RT_DEVICE_CARD_STATUS:
            flag = (StorageBlockIsPresent(sb)) ? 1 : 0;
            ret = LOS_CopyFromKernel((void *)(uintptr_t)arg, sizeof(int), &flag, sizeof(int));
            if (ret) {
                ret = HDF_ERR_IO;
            }
            break;
        case RT_DEVICE_CARD_AU_SIZE:
            ret = StorageBlockGetAuSize(sb, &auSize);
            if (ret == HDF_SUCCESS) {
                au = (unsigned int)auSize;
                ret = LOS_CopyFromKernel((void *)(uintptr_t)arg, sizeof(au), &au, sizeof(au));
                if (ret) {
                    ret = HDF_ERR_IO;
                }
            }
            break;
        case RT_DEVICE_BLOCK_ERROR_COUNT:
            errCnt = sb->errCnt;
            ret = LOS_CopyFromKernel((void *)(uintptr_t)arg, sizeof(int), &errCnt,
                      sizeof(int));
            if (ret) {
                ret = HDF_ERR_IO;
            }
            break;
        default:
            ret = HDF_ERR_NOT_SUPPORT;
            break;
    }

    return ret;
}

static struct block_operations g_blockOps = {
    .open = LiteosBlockOpen,
    .close = LiteosBlockClose,
    .read = LiteosBlockRead,
    .write = LiteosBlockWrite,
    .geometry = LiteosBlockGeometry,
    .ioctl = LiteosBlockIoctl,
};

struct block_operations *StorageBlockGetMmcOps(void)
{
    return &g_blockOps;
}

int32_t StorageBlockOsInit(struct StorageBlock *sb)
{
    int32_t ret;
    int diskId;
    struct disk_divide_info *info = NULL;

    if (sb == NULL || sb->name[0] == '\0') {
        return HDF_ERR_INVALID_OBJECT;
    }

    diskId = los_alloc_diskid_byname(sb->name);
    if (!sb->removeable) {
        info = &g_emmcInfo;
        info->sector_count = sb->capacity;
    }

    ret = los_disk_init(sb->name, &g_blockOps, (void *)sb, diskId, info);
    if (ret != ENOERR) {
        HDF_LOGE("StorageBlockRegister: los_disk_init fail:%d", ret);
        return ret;
    }

    return HDF_SUCCESS;
}

void StorageBlockOsUninit(struct StorageBlock *sb)
{
    int diskId;

    if (sb == NULL || sb->name[0] == '\0') {
        return;
    }
    diskId = los_get_diskid_byname(sb->name);
    (void)los_disk_deinit(diskId);
}

ssize_t StorageBlockMmcErase(unsigned int block_id, size_t secStart, unsigned int secNr)
{
    return StorageBlockErase(StorageBlockFromNumber(block_id), secStart, secNr);
}
