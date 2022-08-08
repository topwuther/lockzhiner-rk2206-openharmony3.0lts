/*
 * Copyright (c) 2021 FuZhou LOCKZHINER Electronic Co., Ltd.
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

#include "e53_temperature_measurement.h"

#define TM_I2C0                                         0
#define TM_I2C_ADDRESS                                  0x5a

static I2cBusIo m_tm_i2c0m2 = {
    .scl =  {.gpio = GPIO0_PA1, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C0,
    .mode = FUNC_MODE_M2,
};

/***************************************************************
* 函数名称: e53_tm_init
* 说    明: E53_TM初始化
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_tm_init()
{
    /*初始化I2C*/
    if (I2cIoInit(m_tm_i2c0m2) != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 io fail\n");
    }
    /*I2C时钟频率100K*/
    if (LzI2cInit(TM_I2C0, 100000) != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 fail\n");
    }
}

unsigned int i2c_write_read(unsigned int id, unsigned short slaveAddr, const unsigned char *cmd, unsigned char *data, unsigned int len)
{
    LzI2cMsg msg[2] = {0};

    msg[0].addr = slaveAddr;
    msg[0].flags = 0;
    msg[0].buf = (unsigned char *)cmd;
    msg[0].len = 1;

    msg[1].addr = slaveAddr;
    msg[1].flags = I2C_M_RD;
    msg[1].buf = (unsigned char *)data;
    msg[1].len = len;

    return LzI2cTransfer(id, msg, 2);
}

#define TA_LEVEL 25
float calculate_temperature(float ta, float to)
{
    float temperature = 0;
    float to_low = 0, to_high = 0;

    if (ta <= TA_LEVEL) {
        to_low = 32.66 + 0.186*(ta - TA_LEVEL);
        to_high = 34.84 + 0.148*(ta - TA_LEVEL);
    } else {
        to_low = 32.66 + 0.086*(ta - TA_LEVEL);
        to_high = 34.84 + 0.1*(ta - TA_LEVEL);
    }

    if ((to_low <= to) && (to <= to_high)) {
        temperature = 36.3 + 0.5/(to_high - to_low)*(to - to_low);
    } else if (to > to_high) {
        temperature = 36.8 + (0.829321 + 0.002364*to)*(to - to_high);
    } else if (to < to_low) {
        temperature = 36.3 + (0.551658 + 0.021525*to)*(to - to_low);
    }
    return temperature;
}

void smbus_read_memory(unsigned char *cmd, unsigned int *temp)
{
    unsigned char data[3];

    i2c_write_read(TM_I2C0, TM_I2C_ADDRESS, cmd, data, 3);
    *temp = data[1] << 8 | data[0];
    //printf("data:0x%x %x %u\n", data[0], data[1], *temp);
}

/***************************************************************
* 函数名称: e53_tm_read_data
* 说    明: 测量温度
* 参    数: 无
* 返 回 值: 0:SUCCESS;1:FAIL
***************************************************************/
unsigned int e53_tm_read_data(float *temperature)
{
    unsigned char ta_cmd[] = {0x6};
    unsigned char to_cmd[] = {0x7};
    unsigned int data_ta, data_to;
    float ta, to;

    smbus_read_memory(ta_cmd, &data_ta);
    /*延时5ms，如果两次操作间隔太短，传感器无响应*/
    LOS_Msleep(5);
    smbus_read_memory(to_cmd, &data_to);

    ta = (float)data_ta * 0.02 - 273.15;
    to = (float)data_to * 0.02 - 273.15;

    if ((ta < 0) || (to < 0)) {
        return FAIL;
    }

    *temperature = calculate_temperature(ta, to);
    //printf("ta:%f to:%f temperature:%f\n", ta, to, *temperature);

    if (*temperature < 0) {
        return FAIL;
    }
    return SUCCESS;
}

