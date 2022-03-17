/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @addtogroup Lockzhiner
 *
 * @file spi.h
 */

#ifndef LZ_HARDWARE_SPI_H
#define LZ_HARDWARE_SPI_H

#include "stdint.h"
#include "stdbool.h"
/* SPI_CONFIG mode */
#define SPI_CPHA             (1<<0)                         /* bit[0]:CPHA, clock phase */
#define SPI_CPOL             (1<<1)                         /* bit[1]:CPOL, clock polarity */
/**
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge (low->high transition)
 *    and data are propagated on a falling edge (high->low clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are propagated
 *    on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are propagated
 *    on a falling edge.
 */
#define SPI_LSB              (0<<2)                         /* bit[2]: 0-LSB */
#define SPI_MSB              (1<<2)                         /* bit[2]: 1-MSB */

#define SPI_MASTER           (0<<3)                         /* SPI master device */
#define SPI_SLAVE            (1<<3)                         /* SPI slave device */

#define SPI_CSM_SHIFT        (4)
#define SPI_CSM_MASK         (0x3 << 4)                     /* SPI master ss_n hold cycles for MOTO SPI master */

#define SPI_MODE_0           (0 | 0)                        /* CPOL = 0, CPHA = 0 */
#define SPI_MODE_1           (0 | SPI_CPHA)              /* CPOL = 0, CPHA = 1 */
#define SPI_MODE_2           (SPI_CPOL | 0)              /* CPOL = 1, CPHA = 0 */
#define SPI_MODE_3           (SPI_CPOL | SPI_CPHA)    /* CPOL = 1, CPHA = 1 */
#define SPI_MODE_MASK        (SPI_CPHA | SPI_CPOL | SPI_MSB)
#define SPI_INT_TXFIM (1 << SPI_IMR_TXFIM_SHIFT)
enum {
    SPI_PERWORD_8BITS,
    SPI_PERWORD_16BITS,
};

enum {
    SPI_CMS_ZERO_CYCLES = 0,  
    SPI_CMS_HALF_CYCLES,
    SPI_CMS_ONE_CYCLES,
};

/**
 * @ingroup LzSpiConfig
 *
 *
 *
 * @bitsPerWord Indicates select a bits_per_word for transfer.
 * @firstBit: Indicates whether data transfers start from MSB or LSB bit.
 * @mode Indicates the clock polarity(SCPOL) and clock phase(SCPH).
 * 0: SCPH=0 SCPOL=0
 * 1: SCPH=0 SCPOL=1
 * 2: SCPH=1 SCPOL=0
 * 3: SCPH=1 SCPOL=1
 * @speed Indicates the Baud Rate prescaler value which will be
    used to configure the transmit and receive SCK clock, unit Hz.
 * @nCycles: Specifies Motorola SPI Master SS_N high cycles for each frame data is transfer.
 * @isSlave Indictates Whether spi work as slave mode.
 */
typedef struct _LzSpiConfig {
    unsigned int bitsPerWord;
    unsigned int firstBit;
    unsigned int mode;
    unsigned int speed;
    unsigned int nCycles;
    unsigned int csm;
    bool isSlave;
} LzSpiConfig;

/**
 * @ingroup LzSpiMsg
 *
 *
 * @chn: Indicates the spi channel based on CSn.
 * @csTake: Indicates take the CS signal.
 * @csRelease: Indicates release the CS signal.
 * @txBuf Indicates the data to be written, or NULL.
 * @rxBuf Indicates the data to be read, or NULL.
 * @len Indicates the bytes of data to transfer.
 */
typedef struct _LzSpiMsg {
    unsigned int chn;
    bool csTake;
    bool csRelease;
    const void *txBuf;
    void *rxBuf;
    unsigned int len;
} LzSpiMsg;

/**
 * @brief Sets configure information for an SPI device.
 *
 * @param id Indicates the SPI device ID.
 * @param conf Indicates the SPI configure to set.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the configure is set;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzSpiSetConfig(unsigned int id, LzSpiConfig conf);

/**
 * @brief Initializes an SPI device with a specified transfer speed.
 *
 *
 *
 * @param id Indicates the SPI device ID.
 * @param conf Indicates the SPI configure to set.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the SPI device is initialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzSpiInit(unsigned int id, LzSpiConfig conf);

/**
 * @brief Deinitializes an SPI device.
 *
 * @param id Indicates the SPI device ID.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the SPI device is deinitialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzSpiDeinit(unsigned int id);

/**
 * @brief transfer data with an SPI device.
 *
 *
 *
 * @param id Indicates the SPI device ID.
 * @param msg Indicates the pointer to the message.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is written to the SPI device successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzSpiTransfer(unsigned int id, LzSpiMsg *msg);

static inline unsigned int LzSpiRead(unsigned int id, unsigned int chn, void *buf, unsigned int len)
{
    LzSpiMsg msg;

    msg.chn = chn;
    msg.csTake = true;
    msg.csRelease = true;
    msg.txBuf = NULL;
    msg.rxBuf = buf;
    msg.len = len;
    
    return LzSpiTransfer(id, &msg);
}

static inline unsigned int LzSpiWrite(unsigned int id, unsigned int chn, const void *buf, unsigned int len)
{
    LzSpiMsg msg;

    msg.chn = chn;
    msg.csTake = true;
    msg.csRelease = true;
    msg.txBuf = buf;
    msg.rxBuf = NULL;
    msg.len = len;

    
    return LzSpiTransfer(id, &msg);
}

static inline unsigned int LzSpiWriteAndRead(unsigned int id, unsigned int chn,
                                           const void *txBuf, void *rxBuf, unsigned int len)
{
    LzSpiMsg msg;

    msg.chn = chn;
    msg.csTake = true;
    msg.csRelease = true;
    msg.txBuf = txBuf;
    msg.rxBuf = rxBuf;
    msg.len = len;

    return LzSpiTransfer(id, &msg);
}

static inline unsigned int LzSpiWriteThenRead(unsigned int id, unsigned int chn,
                                           const void *txBuf, unsigned int txLen,
                                           void *rxBuf, unsigned int rxLen)
{
    int ret;
    LzSpiMsg msg;

    msg.chn = chn;
    msg.csTake = true;
    msg.csRelease = false;
    msg.txBuf = txBuf;
    msg.rxBuf = NULL;
    msg.len = txLen;

    ret = LzSpiTransfer(id, &msg);
    if (ret != LZ_HARDWARE_SUCCESS)
        return ret;

    msg.chn = chn;
    msg.csTake = false;
    msg.csRelease = true;
    msg.txBuf = NULL;
    msg.rxBuf = rxBuf;
    msg.len = rxLen;

    return LzSpiTransfer(id, &msg);
}

#endif
/** @} */
