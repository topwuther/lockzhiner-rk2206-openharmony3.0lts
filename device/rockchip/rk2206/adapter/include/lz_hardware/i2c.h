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
 * @file i2c.h
 */

#ifndef LZ_HARDWARE_I2C_H
#define LZ_HARDWARE_I2C_H

#include "lz_hardware.h"
#include "stdlib.h"
#include "string.h"

typedef struct _LzI2cMsg {
    unsigned short addr;     /* slave address                        */
    unsigned short flags;
#define I2C_M_RD                0x0001  /* read data, from slave to master */
                                        /* I2C_M_RD is guaranteed to be 0x0001! */
#define I2C_M_TEN               0x0010  /* this is a ten bit chip address */
#define I2C_M_DMA_SAFE          0x0200  /* the buffer of this message is DMA safe */
                                        /* makes only sense in kernelspace */
                                        /* userspace buffers are copied anyway */
#define I2C_M_RECV_LEN          0x0400  /* length will be first received byte */
#define I2C_M_NO_RD_ACK         0x0800  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK        0x1000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR      0x2000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART           0x4000  /* if I2C_FUNC_NOSTART */
#define I2C_M_STOP              0x8000  /* if I2C_FUNC_PROTOCOL_MANGLING */
    unsigned short len;                 /* msg length                           */
    unsigned char *buf;                 /* pointer to msg data                  */
} LzI2cMsg;

/**
 * @brief Initializes an I2C device with a specified baud rate.
 *
 *
 *
 * @param id Indicates the I2C device ID.
 * @param freq Indicates the scl frequency to set, unit is HZ.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the I2C device is initialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzI2cInit(unsigned int id, unsigned int freq);

/**
 * @brief Deinitializes an I2C device.
 *
 * @param id Indicates the I2C device ID.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the I2C device is deinitialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzI2cDeinit(unsigned int id);

/**
 * @brief Sets the scl frequency for an I2C device.
 *
 * @param id Indicates the I2C device ID.
 * @param freq Indicates the scl frequency to set, unit is HZ.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the scl frequency is set;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzI2cSetFreq(unsigned int id, unsigned int freq);

/**
 * @brief Transfer data from/to an I2C device based on flags.
 *
 * @param id Indicates the I2C device ID.
 * @param msgs Indicates the point of i2c messages.
 * @param num Indicates the number of message to transfer.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is written to the I2C device successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzI2cTransfer(unsigned id, LzI2cMsg *msgs, unsigned int num);
/**
 * @brief Write data to an I2C device.
 *
 * @param id Indicates the I2C device ID.
 * @param slaveAddr Indicates the I2C slave device address. 
 * @param data Indicates the data to write.
 * @param len Indicates the bytes of the data to write.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is written to the I2C device successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
static inline unsigned int LzI2cWrite(unsigned int id, unsigned short slaveAddr, const unsigned char *data, unsigned int len)
{
    LzI2cMsg msg;

    msg.addr = slaveAddr;
    msg.flags = 0;
    msg.buf = (unsigned char *)data;
    msg.len = len;

    return LzI2cTransfer(id, &msg, 1);
}

/**
 * @brief Read data from an I2C device.
 *
 * @param id Indicates the I2C device ID.
 * @param slaveAddr Indicates the I2C slave device address. 
 * @param data Indicates the data buffer to read.
 * @param len Indicates the bytes of the data to read.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is written to the I2C device successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
static inline unsigned int LzI2cRead(unsigned int id, unsigned short slaveAddr, unsigned char *data, unsigned int len)
{
    LzI2cMsg msg;

    msg.addr = slaveAddr;
    msg.flags = I2C_M_RD;
    msg.buf = data;
    msg.len = len;

    return LzI2cTransfer(id, &msg, 1);
}

/**
 * @brief Write the data to the register address of an I2C device.
 *
 * @param id Indicates the I2C device ID.
 * @param slaveAddr Indicates the I2C slave device address. 
 * @param regAddr Indicates the register address.
 * @param regLen Indicates the bytes of register address.
 * @param data Indicates the data buffer to write.
 * @param len Indicates the bytes of the data to write.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is written to the I2C device successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */

static inline unsigned int LzI2cWriteReg(unsigned int id, unsigned short slaveAddr,
                                       unsigned char *regAddr, unsigned int regLen,
                                       unsigned char *data, unsigned int len)
{
    unsigned int ret;
    LzI2cMsg msg;

    msg.addr = slaveAddr;
    msg.flags = I2C_M_RD;
    msg.buf = malloc(regLen + len);
    msg.len = regLen + len;

    memcpy(msg.buf, regAddr, regLen);
    memcpy(msg.buf + regLen, data, len);

    ret = LzI2cTransfer(id, &msg, 1);
    free(msg.buf);

    return ret;
}


/**
 * @brief Write register address and read the register value from an I2C device.
 *
 * @param id Indicates the I2C device ID.
 * @param slaveAddr Indicates the I2C slave device address. 
 * @param regAddr Indicates the register address.
 * @param regLen Indicates the bytes of register address.
 * @param data Indicates the data buffer to read.
 * @param len Indicates the bytes of the data to read.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the data is written to the I2C device successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */

static inline unsigned int LzI2cReadReg(unsigned int id, unsigned short slaveAddr,
                                      unsigned char *regAddr, unsigned int regLen,
                                      unsigned char *data, unsigned int len)
{
    LzI2cMsg msgs[2];

    msgs[0].addr = slaveAddr;
    msgs[0].flags = 0;
    msgs[0].buf = regAddr;
    msgs[0].len = regLen;

    msgs[1].addr = slaveAddr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].buf = data;
    msgs[1].len = len;

   return LzI2cTransfer(id, msgs, 2);
}

/**
* @brief Write register address and read the register value from an I2C device.
* 注意：i2c必须经过初始化
*
* @param id Indicates the I2C device ID.
* @param slaveAddr Indicates the I2C slave device address.
* @param slaveAddr Indicates Len.
*/
static inline unsigned int LzI2cScan(unsigned int id, unsigned short *slaveAddr, unsigned int slaveAddrLen)
{
    unsigned short address;
    unsigned int ret;
    unsigned char buffer[1];
    unsigned int offset = 0;

    for (address = 0x3; address < 0x78; address++)
    {
        ret = LzI2cWrite(id, address, buffer, 0);
        if (ret == LZ_HARDWARE_SUCCESS)
        {
            /* 该从设备地址有效 */
            if (offset < slaveAddrLen)
            {
                slaveAddr[offset] = address;
                offset++;
            }
        }
        else
        {
            /* 该从设备地址没有i2c设备 */
        }
    }

    return offset;
}

#endif
/** @} */
