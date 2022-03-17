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
#include "lz_hardware.h"

#define EEPROM_I2C_BUS          0
#define EEPROM_I2C_ADDRESS      0x51

/* EEPROM型号：K24C02，2Kbit（256Byte），32页，每页8个字节（Byte） */
#define EEPROM_ADDRESS_MAX      256
#define EEPROM_PAGE             8

static I2cBusIo m_i2cBus = {
    .scl =  {.gpio = GPIO0_PA1, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C0,
    .mode = FUNC_MODE_M2,
};

static unsigned int m_i2c_freq = 100000;

////////////////////////////////////////////////////////

/***************************************************************
* 函数名称: eeprog_delay_usec
* 说    明: 忙等待usec
* 参    数: 
*       @usec：等待时间，单位：usec
* 返 回 值: 无
***************************************************************/
static inline void eeprog_delay_usec(unsigned int usec)
{
    for (unsigned int i = 0; i < usec; i++)
        HAL_DelayUs(1);
}

/***************************************************************
* 函数名称: eeprom_init
* 说    明: EEPROM初始化
* 参    数: 无
* 返 回 值: 0为成功，反之为失败
***************************************************************/
unsigned int eeprom_init()
{
    if (I2cIoInit(m_i2cBus) != LZ_HARDWARE_SUCCESS) {
        printf("%s, %d: I2cIoInit failed!\n", __FILE__, __LINE__);
        return __LINE__;
    }
    if (LzI2cInit(EEPROM_I2C_BUS, m_i2c_freq) != LZ_HARDWARE_SUCCESS) {
        printf("%s, %d: I2cInit failed!\n", __FILE__, __LINE__);
        return __LINE__;
    }

    /* GPIO0_A0 => I2C1_SDA_M1 */
    PinctrlSet(GPIO0_PA0, MUX_FUNC3, PULL_NONE, DRIVE_KEEP);
    /* GPIO0_A1 => I2C1_SCL_M1 */
    PinctrlSet(GPIO0_PA1, MUX_FUNC3, PULL_NONE, DRIVE_KEEP);

    return 0;
}

/***************************************************************
* 函数名称: eeprom_deinit
* 说    明: EEPROM退出
* 参    数: 无
* 返 回 值: 0为成功，反之失败
***************************************************************/
unsigned int eeprom_deinit()
{
    LzI2cDeinit(EEPROM_I2C_BUS);
    LzGpioDeinit(m_i2cBus.scl.gpio);
    LzGpioDeinit(m_i2cBus.sda.gpio);
    return 0;
}

/***************************************************************
* 函数名称: eeprom_get_blocksize
* 说    明: EEPROM获取页大小
* 参    数: 无
* 返 回 值: 返回页大小
***************************************************************/
unsigned int eeprom_get_blocksize()
{
    return EEPROM_PAGE;
}

/***************************************************************
* 函数名称: eeprom_readbyte
* 说    明: EEPROM读一个字节
* 参    数: 
*           @addr: EEPROM存储地址
*           @data: 存放EERPOM的数据指针
* 返 回 值: 返回读取字节的长度，反之为错误
***************************************************************/
unsigned int eeprom_readbyte(unsigned int addr, unsigned char *data)
{
    unsigned int ret = 0;
    unsigned char buffer[1];
    LzI2cMsg msgs[2];

    /* K24C02的存储地址是0~255 */
    if (addr >= EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr(0x%x) >= EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_ADDRESS_MAX);
        return 0;
    }

    buffer[0] = (unsigned char)addr;

    msgs[0].addr = EEPROM_I2C_ADDRESS;
    msgs[0].flags = 0;
    msgs[0].buf = &buffer[0];
    msgs[0].len = 1;

    msgs[1].addr = EEPROM_I2C_ADDRESS;
    msgs[1].flags = I2C_M_RD;
    msgs[1].buf = data;
    msgs[1].len = 1;

   ret = LzI2cTransfer(EEPROM_I2C_BUS, msgs, 2);
   if (ret != LZ_HARDWARE_SUCCESS) {
       printf("%s, %s, %d: LzI2cTransfer failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
       return 0;
   }

    return 1;
}

/***************************************************************
* 函数名称: eeprom_writebyte
* 说    明: EEPROM写一个字节
* 参    数: 
*           @addr: EEPROM存储地址
*           @data: 写EERPOM的数据
* 返 回 值: 返回写入数据的长度，反之为错误
***************************************************************/
unsigned int eeprom_writebyte(unsigned int addr, unsigned char data)
{
    unsigned int ret = 0;
    LzI2cMsg msgs[1];
    unsigned char buffer[2];

    /* K24C02的存储地址是0~255 */
    if (addr >= EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr(0x%x) >= EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_ADDRESS_MAX);
        return 0;
    }

    buffer[0] = (unsigned char)(addr & 0xFF);
    buffer[1] = data;

    msgs[0].addr = EEPROM_I2C_ADDRESS;
    msgs[0].flags = 0;
    msgs[0].buf = &buffer[0];
    msgs[0].len = 2;

    ret = LzI2cTransfer(EEPROM_I2C_BUS, msgs, 1);
    if (ret != LZ_HARDWARE_SUCCESS) {
       printf("%s, %s, %d: LzI2cTransfer failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
       return 0;
    }

    /* K24C02芯片需要时间完成写操作，在此之前不响应其他操作*/
    eeprog_delay_usec(1000);

    return 1;
}

/***************************************************************
* 函数名称: eeprom_writepage
* 说    明: EEPROM写1个页字节
* 参    数: 
*           @addr: EEPROM存储地址，必须是页地址
*           @data: 写EERPOM的数据指针
*           @data_len: 写EEPROM数据的长度，必须是小于1个页大小
* 返 回 值: 返回写入数据的长度，反之为错误
***************************************************************/
unsigned int eeprom_writepage(unsigned int addr, unsigned char *data, unsigned int data_len)
{
    unsigned int ret = 0;
    LzI2cMsg msgs[1];
    unsigned char buffer[EEPROM_PAGE + 1];
    
    /* K24C02的存储地址是0~255 */
    if (addr >= EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr(0x%x) >= EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_ADDRESS_MAX);
        return 0;
    }

    if ((addr % EEPROM_PAGE) != 0) {
        printf("%s, %s, %d: addr(0x%x) is not page addr(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_PAGE);
        return 0;
    }

    if ((addr + data_len) > EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr + data_len(0x%x) > EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr + data_len, EEPROM_ADDRESS_MAX);
        return 0;
    }

    if (data_len > EEPROM_PAGE) {
        printf("%s, %s, %d: data_len(%d) > EEPROM_PAGE(%d)\n", __FILE__, __func__, __LINE__, data_len, EEPROM_PAGE);
        return 0;
    }

    buffer[0] = addr;
    memcpy(&buffer[1], data, data_len);
    
    msgs[0].addr = EEPROM_I2C_ADDRESS;
    msgs[0].flags = 0;
    msgs[0].buf = &buffer[0];
    msgs[0].len = 1 + data_len;

    ret = LzI2cTransfer(EEPROM_I2C_BUS, msgs, 1);
    if (ret != LZ_HARDWARE_SUCCESS) {
       printf("%s, %s, %d: LzI2cTransfer failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
       return 0;
    }

    /* K24C02芯片需要时间完成写操作，在此之前不响应其他操作*/
    eeprog_delay_usec(1000);

    return data_len;
}

/***************************************************************
* 函数名称: eeprom_read
* 说    明: EEPROM读多个字节
* 参    数: 
*           @addr:      EERPOM存储地址
*           @data:      存放EERPOM的数据指针
*           @data_len:  读取EERPOM数据的长度
* 返 回 值: 返回读取字节的长度，反之为错误
***************************************************************/
unsigned int eeprom_read(unsigned int addr, unsigned char *data, unsigned int data_len) 
{
    unsigned int ret = 0;

    if (addr >= EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr(0x%x) >= EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_ADDRESS_MAX);
        return 0;
    }

    if ((addr + data_len) > EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr + len(0x%x) > EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr + data_len, EEPROM_ADDRESS_MAX);
        return 0;
    }   

    ret = eeprom_readbyte(addr, data);
    if (ret != 1) {
        printf("%s, %s, %d: EepromReadByte failed(%d)\n", __FILE__, __func__, __LINE__, ret);
        return 0;
    }

    if (data_len > 1) {
        ret = LzI2cRead(EEPROM_I2C_BUS, EEPROM_I2C_ADDRESS, &data[1], data_len - 1);
        if (ret < 0) {
            printf("%s, %s, %d: LzI2cRead failed(%d)!\n", __FILE__, __func__, __LINE__, ret);
            return 0;
        }
    }

    return data_len;
}

/***************************************************************
* 函数名称: eeprom_write
* 说    明: EEPROM写多个字节
* 参    数: 
*           @addr: EEPROM存储地址
*           @data: 写EERPOM的数据指针
*           @data_len: 写EEPROM数据的长度
* 返 回 值: 返回写入数据的长度，反之为错误
***************************************************************/
unsigned int eeprom_write(unsigned int addr, unsigned char *data, unsigned int data_len)
{
    unsigned int ret = 0;
    unsigned int offset_current = 0;
    unsigned int page_start, page_end;
    unsigned char is_data_front = 0;
    unsigned char is_data_back = 0;
    unsigned int len;

    if (addr >= EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr(0x%x) >= EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr, EEPROM_ADDRESS_MAX);
        return 0;
    }

    if ((addr + data_len) > EEPROM_ADDRESS_MAX) {
        printf("%s, %s, %d: addr + len(0x%x) > EEPROM_ADDRESS_MAX(0x%x)\n", __FILE__, __func__, __LINE__, addr + data_len, EEPROM_ADDRESS_MAX);
        return 0;
    }

    /* 判断addr是否是页地址 */
    page_start = addr / EEPROM_PAGE;
    if ((addr % EEPROM_PAGE) != 0) {
        page_start += 1;
        is_data_front = 1;
    }

    /* 判断addr + data_len是否是页地址 */
    page_end = (addr + data_len) / EEPROM_PAGE;
    if ((addr + data_len) % EEPROM_PAGE != 0) {
        page_end += 1;
        is_data_back = 1;
    }

    offset_current = 0;
    
    /* 处理前面非页地址的数据，如果是页地址则不执行 */
    for (unsigned int i = addr; i < (page_start * EEPROM_PAGE); i++) {
        ret = eeprom_writebyte(i, data[offset_current]);
        if (ret != 1) {
            printf("%s, %s, %d: EepromWriteByte failed(%d)\n", __FILE__, __func__, __LINE__, ret);
            return offset_current;
        }
        offset_current++;
    }

    /* 处理后续的数据，如果数据长度不足一个Page，则不执行 */
    for (unsigned int page = page_start; page < page_end; page++) {
        len = EEPROM_PAGE;
        if ((page == (page_end - 1)) && (is_data_back)) {
            len = (addr + data_len) % EEPROM_PAGE;
        }
    
        ret = eeprom_writepage(page * EEPROM_PAGE, &data[offset_current], len);
        if (ret != len) {
            printf("%s, %s, %d: EepromWritePage failed(%d)\n", __FILE__, __func__, __LINE__, ret);
            return offset_current;
        }
        offset_current += EEPROM_PAGE;
    }

    return data_len;
}
