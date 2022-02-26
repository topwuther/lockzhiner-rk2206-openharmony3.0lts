/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef MTD_SPI_COMMON_H
#define MTD_SPI_COMMON_H

#include "mtd_core.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct SpiFlash;

struct MtdSpiConfig {
    uint8_t ifType;
    uint8_t cmd;
    uint8_t dummy;
    uint32_t size;
    uint32_t clock;
};

struct MtdSpiOps {
    int32_t (*waitReady)(struct SpiFlash *spi);
    int32_t (*writeEnable)(struct SpiFlash *spi);
    int32_t (*qeEnable)(struct SpiFlash *spi);
    int32_t (*entry4Addr)(struct SpiFlash *spi, int enable);
};

struct SpiOpsInfo {
    uint8_t id[MTD_FLASH_ID_LEN_MAX];
    uint8_t idLen;
    struct MtdSpiOps spiOps;
};

enum SpiIfType {
    MTD_SPI_IF_STD = 0,
    MTD_SPI_IF_DUAL = 1,
    MTD_SPI_IF_DIO = 2,
    MTD_SPI_IF_QUAD = 3,
    MTD_SPI_IF_QIO = 4,
};

struct SpiFlash {
    struct MtdDevice mtd;
    uint8_t cs;
    uint8_t qeEnable;
    uint8_t qeSupport;
    uint32_t addrCycle;
    struct MtdSpiConfig eraseCfg;
    struct MtdSpiConfig writeCfg;
    struct MtdSpiConfig readCfg;
    struct MtdSpiOps spiOps;
};

int32_t SpiFlashWaitReady(struct SpiFlash *spi);
int32_t SpiFlashWriteEnable(struct SpiFlash *spi);
int32_t SpiFlashQeEnable(struct SpiFlash *spi);
int32_t SpiFlashEntry4Addr(struct SpiFlash *spi, int enable);

int32_t SpiFlashAdd(struct SpiFlash *spi);
void SpiFlashDel(struct SpiFlash *spi);

/****************************Spi Common Command Set **************************************/
#define MTD_SPI_CMD_WRSR            0x01        /* Write Status Register */
#define MTD_SPI_CMD_WRITE_STD       0x02        /* Standard page program */
#define MTD_SPI_CMD_READ_STD        0x03        /* Standard page cache */
#define MTD_SPI_CMD_WRDI            0x04        /* Write Disable */

#define MTD_SPI_CMD_RDSR            0x05        /* Read Status Register */
#define MTD_SPI_SR_WEL_MASK         (1 << 1)
#define MTD_SPI_SR_WIP_MASK         (1 << 0)
#define MTD_SPI_SR_QE_MASK          (1 << 6)

#define MTD_SPI_CMD_WREN            0x06        /* Write Enable */

#define MTD_SPI_CMD_GET_FEATURE     0x0F
#define MTD_SPI_CMD_SET_FEATURE     0x1F

#define MTD_SPI_CMD_RDSR3           0x15        /* Read Status Register 3 */
#define MTD_SPI_SR3_4BYTE_SHIFT     5
#define MTD_SPI_SR3_4BYTE_MASK      (1 << MTD_SPI_SR3_4BYTE_SHIFT)
#define MTD_SPI_SR3_IS_4BYTE(sr3)   (((sr3) & MTD_SPI_SR3_4BYTE_MASK) >> MTD_SPI_SR3_4BYTE_SHIFT)


#define MTD_SPI_CMD_ERASE_4K        0x20        /* 4KB sector erase */
#define MTD_SPI_CMD_ERASE_64K       0xD8        /* 64KB sector erase */

#define MTD_SPI_CMD_WRSR2           0x31        /* Write Status Register 2 */

#define MTD_SPI_CMD_RDSR2           0x35        /* Read Status Register 2 */
#define MTD_SPI_CMD_RDCR            0x35        /* Read Config Register */
#define MTD_SPI_CR_QE_MASK          (1 << 1)

#define MTD_SPI_CMD_RDID            0x9F        /* Read Identification */

#define MTD_SPI_CMD_EN4B            0xB7        /* enter 4 bytes mode and set 4 byte bit */

#define MTD_SPI_CMD_EX4B            0xE7        /* exit 4 bytes mode and clear 4 byte bit */


#define MTD_SPI_CMD_RESET           0xFF

/**************************** Ohters **************************************/
#define MTD_SPI_CS_MAX              2
#define MTD_SPI_STD_OP_ADDR_NUM     3
#define MTD_SPI_FEATURE_ECC_ENABLE  (1 << 4)
#define MTD_SPI_FEATURE_QE_ENABLE   (1 << 0)


#ifdef __cplusplus
#if __cplusplus

}
#endif
#endif /* __cplusplus */

#endif /* MTD_SPI_COMMON_H */
