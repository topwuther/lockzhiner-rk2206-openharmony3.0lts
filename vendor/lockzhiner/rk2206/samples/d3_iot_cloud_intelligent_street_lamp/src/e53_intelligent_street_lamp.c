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

#include "e53_intelligent_street_lamp.h"

#define ISL_I2C0                                    0
#define BH1750_ADDR                                 0x23

static I2cBusIo m_isl_i2c0m2 = {
    .scl =  {.gpio = GPIO0_PA1, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C0,
    .mode = FUNC_MODE_M2,
};

/***************************************************************
* 函数名称: e53_isl_io_init
* 说    明: E53_ISL初始化
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_isl_io_init(void)
{
    LzGpioInit(GPIO0_PA5);
    LzGpioSetDir(GPIO0_PA5, LZGPIO_DIR_OUT);

    if (I2cIoInit(m_isl_i2c0m2) != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 io failed\n");
    }
    if (LzI2cInit(ISL_I2C0, 400000) != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 failed\n");
    }
}

/***************************************************************
* 函数名称: init_bh1750
* 说    明: 写命令初始化BH1750
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void init_bh1750()
{
    uint8_t send_data[1] = {0x01};
    uint32_t send_len = 1;

    LzI2cWrite(ISL_I2C0, BH1750_ADDR, send_data, send_len); 
}

/***************************************************************
* 函数名称: start_bh1750
* 说    明: 启动BH1750
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void start_bh1750()
{
    uint8_t send_data[1] = {0x10};
    uint32_t send_len = 1;

    LzI2cWrite(ISL_I2C0, BH1750_ADDR, send_data, send_len);
}

/***************************************************************
* 函数名称: e53_isl_init
* 说    明: 初始化E53_IS1
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_isl_init(void)
{
    e53_isl_io_init();
    init_bh1750();
}

/***************************************************************
* 函数名称: e53_isl_read_data
* 说    明: 读取数据
* 参    数: 无
* 返 回 值: 无
***************************************************************/
float e53_isl_read_data()
{
    float lum = 0;

    start_bh1750();
    LOS_Msleep(180);

    uint8_t recv_data[2] = {0};
    uint32_t receive_len = 2;
    LzI2cRead(ISL_I2C0, BH1750_ADDR, recv_data, receive_len);
    lum = (float)(((recv_data[0]<<8) + recv_data[1])/1.2);

    //printf("data %x %x\n", recv_data[0], recv_data[1]);

    return lum;
}

/***************************************************************
* 函数名称: isl_light_set_status
* 说    明: 紫光灯控制
* 参    数: 
*          OFF,关
*          ON,开
* 返 回 值: 无
***************************************************************/
void isl_light_set_status(SWITCH_STATUS_ENUM status)
{
    if(status == ON)
    {
        /*设置GPIO0_PA5输出高电平点亮灯*/
        LzGpioSetVal(GPIO0_PA5, LZGPIO_LEVEL_HIGH);
    }
    if(status == OFF)
    {
        /*设置GPIO0_PA5输出低电平关闭灯*/
        LzGpioSetVal(GPIO0_PA5, LZGPIO_LEVEL_LOW);
    }
}

