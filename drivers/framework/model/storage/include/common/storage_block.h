/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef STORAGE_BLOCK_H
#define STORAGE_BLOCK_H

#include "storage.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define STORAGE_SEC_SIZE 512
#define STORAGE_SEC_SHIFT 9
#define STORAGE_MAX_SEC_NR (STORAGE_MAX_BYTES >> STORAGE_SEC_SHIFT)

#define STORAGE_SEC_PARAM_INVALID(s, n) \
    (s >= STORAGE_MAX_SEC_NR || \
    n >= STORAGE_MAX_SEC_NR || \
    (STORAGE_MAX_SEC_NR - n) <= s)

#define BLOCK_NAME_LEN 32

struct StorageBlock {
    char name[BLOCK_NAME_LEN]; /* name of the block device */
    int32_t index;
    enum StorageType type;     /* indicate which type of media is used */
    bool removeable;
    void *media;               /* media device of the block */
    size_t capacity;           /* sized by sector */
    size_t secSize;            /* sized by bytes */
    uint32_t errCnt;           /* err count on io transfer */
    struct StorageBlockMethod *ops; /* storage oparations provided by specific media */
    void *bops;                /* block operations of specific os */
};

struct StorageBlockMethod {
    ssize_t (*read)(struct StorageBlock *sb, uint8_t *buf, size_t secStart, size_t secNr);
    ssize_t (*write)(struct StorageBlock *sb, const uint8_t *buf, size_t secStart, size_t secNr);
    ssize_t (*erase)(struct StorageBlock *sb, size_t secStart, size_t secNr);
    size_t (*getCapacity)(struct StorageBlock *sb);
    bool (*isPresent)(struct StorageBlock *sb);
    uint32_t (*getAuSize)(struct StorageBlock *sb);
};

ssize_t StorageBlockRead(struct StorageBlock *sb, uint8_t *buf, size_t secStart, size_t secNr);
ssize_t StorageBlockWrite(struct StorageBlock *sb, const uint8_t *buf, size_t secStart, size_t secNr);
ssize_t StorageBlockErase(struct StorageBlock *sb, size_t secStart, size_t secNr);
size_t StorageBlockGetCapacity(struct StorageBlock *sb);
bool StorageBlockIsPresent(struct StorageBlock *sb);
int32_t StorageBlockGetAuSize(struct StorageBlock *sb, uint32_t *auSize);

int32_t StorageBlockAdd(struct StorageBlock *sb);
void StorageBlockDel(struct StorageBlock *sb);

/* these two functions gona implemented by specific os */
int32_t StorageBlockOsInit(struct StorageBlock *sb);
void StorageBlockOsUninit(struct StorageBlock *sb);

ssize_t StorageBlockMmcErase(uint32_t blockId, size_t secStart, size_t secNr);
struct StorageBlock *StorageBlockFromNumber(uint32_t number);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* STORAGE_BLOCK_H */
