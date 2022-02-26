/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "hdf_base.h"
#include "hdf_log.h"
#include "platform_core.h"
#include "storage_block.h"

#define HDF_LOG_TAG storage_block_c

#define MAX_BLOCK_COUNT 3

static struct StorageBlock *g_blocks[MAX_BLOCK_COUNT];

ssize_t StorageBlockErase(struct StorageBlock *sb, size_t secStart, size_t secNr)
{
    if (sb == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (sb->ops == NULL || sb->ops->erase == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return sb->ops->erase(sb, secStart, secNr);
}

ssize_t StorageBlockWrite(struct StorageBlock *sb, const uint8_t *buf, size_t secStart, size_t secNr)
{
    if (sb == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (sb->ops == NULL || sb->ops->write == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return sb->ops->write(sb, buf, secStart, secNr);
}

ssize_t StorageBlockRead(struct StorageBlock *sb, uint8_t *buf, size_t secStart, size_t secNr)
{
    if (sb == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (sb->ops == NULL || sb->ops->read == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return sb->ops->read(sb, buf, secStart, secNr);
}

int32_t StorageBlockGetAuSize(struct StorageBlock *sb, uint32_t *auSize)
{
    if (sb == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (sb->ops == NULL || sb->ops->getAuSize == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    *auSize = sb->ops->getAuSize(sb);
    return HDF_SUCCESS;
}

size_t StorageBlockGetCapacity(struct StorageBlock *sb)
{
    return (sb == NULL) ? 0 : sb->capacity;
}

bool StorageBlockIsPresent(struct StorageBlock *sb)
{
    return (sb != NULL && sb->ops != NULL &&
        sb->ops->isPresent != NULL && sb->ops->isPresent(sb));
}

int32_t StorageBlockAdd(struct StorageBlock *sb)
{
    int i;
    int32_t ret;

    if (sb == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    for (i = 0; i < MAX_BLOCK_COUNT; i++) {
        if (g_blocks[i] == NULL) {
            g_blocks[i] = sb;
            sb->index = i;
            break;
        }
    }
    if (i >= MAX_BLOCK_COUNT) {
        return HDF_PLT_ERR_DEV_FULL;
    }

    if (sb->secSize == 0) {
        sb->secSize = STORAGE_SEC_SIZE;
    }

    ret = StorageBlockOsInit(sb);
    if (ret != HDF_SUCCESS) {
        g_blocks[i] = NULL;
        return ret;
    }

    return HDF_SUCCESS;
}

void StorageBlockDel(struct StorageBlock *sb)
{
    int i;

    if (sb == NULL) {
        return;
    }

    for (i = 0; i < MAX_BLOCK_COUNT; i++) {
        if (g_blocks[i] == sb) {
            g_blocks[i] = NULL;
        }
    }

    StorageBlockOsUninit(sb);
}

struct StorageBlock *StorageBlockFromNumber(uint32_t number)
{
    if (number >= MAX_BLOCK_COUNT) {
        HDF_LOGE("StorageBlockFromNumber: invalid number:%d", number);
        return NULL;
    }
    return g_blocks[number];
}
