/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef MTD_CORE_H
#define MTD_CORE_H

#include "los_vm_phys.h"

#include "hdf_base.h"
#include "osal_mutex.h"
#include "platform_device.h"

#define MTD_FLASH_ID_LEN_MAX 8

// #define MTD_DEBUG /* open this macro for debug */

struct MtdDevice;
struct MtdDeviceMethod;

enum MtdDevType {
    MTD_TYPE_NOR,
    MTD_TYPE_NAND,
    MTD_TYPE_SPI_NOR,
    MTD_TYPE_SPI_NAND,
    MTD_TYPE_MAX,
};

enum MtdMsgType {
    MTD_MSG_TYPE_WRITE,
    MTD_MSG_TYPE_READ,
    MTD_MSG_TYPE_ERASE,
};

struct MtdMsg {
    enum MtdMsgType type;
    off_t addr;
    off_t faddr;
    uint8_t *buf;
    size_t len;
    bool withOob;
    bool skipBad;
};

struct MtdPage {
    enum MtdMsgType type;
    off_t addr;
    off_t failAddr;
    uint8_t *dataBuf;
    size_t dataLen;
    uint8_t *oobBuf;
    size_t oobLen;
};

struct MtdDeviceMethod {
    int32_t (*read)(struct MtdDevice *mtdDevice, off_t from, size_t len, uint8_t *buf);
    int32_t (*write)(struct MtdDevice *mtdDevice, off_t to, size_t len, const uint8_t *buf);
    int32_t (*erase)(struct MtdDevice *mtdDevice, off_t addr, size_t len, off_t *faddr);
    int32_t (*pageTransfer)(struct MtdDevice *mtdDevice, struct MtdPage *mtdPage);
    bool (*isBadBlock)(struct MtdDevice *mtdDevice, off_t addr);
    int32_t (*markBadBlock)(struct MtdDevice *mtdDevice, off_t addr);
    void (*dump)(struct MtdDevice *mtdDevice);
    int32_t (*lock)(struct MtdDevice *mtdDevice);
    void (*unlock)(struct MtdDevice *mtdDevice);
}; 

struct MtdDevice {
    struct PlatformDevice device;
    int16_t index;
    const char *name;
    const char *chipName;
    enum MtdDevType type;
    union {
        uint8_t id[MTD_FLASH_ID_LEN_MAX];
        struct {
            uint8_t mfrId; // id[0]: Manufacture ID
            uint8_t devId; // id[0]: Device ID
        };
    };
    uint16_t idLen;
    size_t capacity;              // by bytes
    size_t eraseSize;             // by bytes
    size_t writeSize;             // by bytes
    size_t readSize;              // by bytes
    size_t oobSize;               // for nand only
    unsigned int writeSizeShift;
    struct OsalMutex lock;
    struct MtdDeviceMethod *ops;
    void *osData;
    void *cntlr;
    void *priv;
};

int32_t MtdDeviceAdd(struct MtdDevice *mtdDevice);
void MtdDeviceDel(struct MtdDevice *mtdDevice);

int32_t MtdDeviceLock(struct MtdDevice *mtdDevice);
void MtdDeviceUnlock(struct MtdDevice *mtdDevice);

ssize_t MtdDeviceRead(struct MtdDevice *mtdDevice, off_t from, size_t len, uint8_t *buf);
ssize_t MtdDeviceWrite(struct MtdDevice *mtdDevice, off_t to, size_t len, const uint8_t *buf);
ssize_t MtdDeviceErase(struct MtdDevice *mtdDevice, off_t from, size_t len, off_t *failAddr);

ssize_t MtdDeviceReadWithOob(struct MtdDevice *mtdDevice, off_t from, size_t len, uint8_t *buf);
ssize_t MtdDeviceWriteWithOob(struct MtdDevice *mtdDevice, off_t to, size_t len, const uint8_t *buf);
bool MtdDeviceIsBadBlock(struct MtdDevice *mtdDevice, off_t addr);
int32_t MtdDeviceMarkBadBlock(struct MtdDevice *mtdDevice, off_t addr);

static inline bool MtdDeviceIsPageAligned(struct MtdDevice *mtdDevice, off_t addr)
{
    return ((addr & (mtdDevice->writeSize - 1)) == 0);
}

static inline size_t MtdDeviceAddrToPage(struct MtdDevice *mtdDevice, off_t addr)
{
    return (size_t)(addr >> mtdDevice->writeSizeShift);
}

#define MTD_DEVICE_DUMP(mtd) \
    do { \
        uint16_t i; \
        HDF_LOGI("%s: name = %s(%s), type = %d", __func__, mtd->name, mtd->chipName, mtd->type); \
        for (i = 0; i < mtd->idLen; i++) { \
            HDF_LOGI("%s: id[%u] = 0x%x", __func__, i, mtd->id[i]); \
        } \
        HDF_LOGI("%s: capacity: %zu", __func__, mtd->capacity); \
        HDF_LOGI("%s: eraseSize: %zu", __func__, mtd->eraseSize); \
        HDF_LOGI("%s: writeSize: %zu", __func__, mtd->writeSize); \
        HDF_LOGI("%s: readSize: %zu", __func__, mtd->readSize); \
        HDF_LOGI("%s: oobSize: %zu", __func__, mtd->oobSize); \
    } while (0)


/***************************** Other Utils *******************************************/

static inline void MtdDmaCacheClean(void *addr, size_t size)
{
    uintptr_t start = (uintptr_t)addr & ~(CACHE_ALIGNED_SIZE - 1);
    uintptr_t end = (uintptr_t)addr + (uintptr_t)size;

    end = ALIGN(end, CACHE_ALIGNED_SIZE);
    DCacheFlushRange(start, end);
    return;
}

static inline void MtdDmaCacheInv(void *addr, size_t size)
{
    uintptr_t start = (uintptr_t)addr & ~(CACHE_ALIGNED_SIZE - 1);
    uintptr_t end = (uintptr_t)addr + (uintptr_t)size;

    end = ALIGN(end, CACHE_ALIGNED_SIZE);
    DCacheInvRange(start, end);
    return;
}

int MtdFfs(int x);

#endif /* MTD_CORE_H */
