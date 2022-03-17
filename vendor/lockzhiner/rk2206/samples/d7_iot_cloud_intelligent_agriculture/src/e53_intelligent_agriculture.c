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

#include "e53_intelligent_agriculture.h"

#define IA_I2C0                                    0
#define BH1750_ADDR                                0x23
#define SHT30_ADDR                                 0x44

static I2cBusIo m_ia_i2c0m2 = {
    .scl =  {.gpio = GPIO0_PA1, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C0,
    .mode = FUNC_MODE_M2,
};

/***************************************************************
* 函数名称: init_sht30
* 说    明: 初始化SHT30，设置测量周期
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void init_sht30()
{
    uint8_t send_data[2] = {0x22, 0x36};
    uint32_t send_len = 2;

    LzI2cWrite(IA_I2C0, SHT30_ADDR, send_data, send_len); 
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

    LzI2cWrite(IA_I2C0, BH1750_ADDR, send_data, send_len); 
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

    LzI2cWrite(IA_I2C0, BH1750_ADDR, send_data, send_len);
}

/***************************************************************
* 函数名称: sht30_calc_RH
* 说    明: 湿度计算
* 参    数: u16sRH：读取到的湿度原始数据
* 返 回 值: 计算后的湿度数据
***************************************************************/
float sht30_calc_RH(uint16_t u16sRH)
{
    float humidityRH = 0;

    /*clear bits [1..0] (status bits)*/
    u16sRH &= ~0x0003;
    /*calculate relative humidity [%RH]*/
    /*RH = rawValue / (2^16-1) * 10*/
    humidityRH = (100 * (float)u16sRH / 65535);

    return humidityRH;
}

/***************************************************************
* 函数名称: sht30_calc_temperature
* 说    明: 温度计算
* 参    数: u16sT：读取到的温度原始数据
* 返 回 值: 计算后的温度数据
***************************************************************/
float sht30_calc_temperature(uint16_t u16sT)
{
    float temperature = 0;

    /*clear bits [1..0] (status bits)*/
    u16sT &= ~0x0003;
    /*calculate temperature [℃]*/
    /*T = -45 + 175 * rawValue / (2^16-1)*/
    temperature = (175 * (float)u16sT / 65535 - 45);

    return temperature;
}

/***************************************************************
* 函数名称: sht30_check_crc
* 说    明: 检查数据正确性
* 参    数: data：读取到的数据
            nbrOfBytes：需要校验的数量
            checksum：读取到的校对比验值
* 返 回 值: 校验结果，0-成功 1-失败
***************************************************************/
uint8_t sht30_check_crc(uint8_t *data, uint8_t nbrOfBytes, uint8_t checksum)
{
    uint8_t crc = 0xFF;
    uint8_t bit = 0;
    uint8_t byteCtr ;
    const int16_t POLYNOMIAL = 0x131;

    /*calculates 8-Bit checksum with given polynomial*/
    for(byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);
        for ( bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else crc = (crc << 1);
        }
    }

    if(crc != checksum)
        return 1;
    else
        return 0;
}

/***************************************************************
* 函数名称: e53_ia_io_init
* 说    明: E53_IA模块IO初始化
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_ia_io_init()
{
    uint32_t ret = LZ_HARDWARE_FAILURE;

    /*初始化紫光灯GPIO*/
    LzGpioInit(GPIO0_PA2);
    /*初始化电机GPIO*/
    LzGpioInit(GPIO1_PD0);

    /*设置GPIO0_PA2为输出模式*/
    ret = LzGpioSetDir(GPIO0_PA2, LZGPIO_DIR_OUT);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("set GPIO0_PA2 Direction fail\n");
    }

    /*设置GPIO1_PD0为输出模式*/
    ret = LzGpioSetDir(GPIO1_PD0, LZGPIO_DIR_OUT);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("set GPIO0_PD0 Direction fail\n");
    }

    /*初始化I2C*/
    if (I2cIoInit(m_ia_i2c0m2) != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 io fail\n");
    }
    /*I2C时钟频率400K*/
    if (LzI2cInit(IA_I2C0, 100000) != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 fail\n");
    }
}

/***************************************************************
* 函数名称: e53_ia_init
* 说    明: 初始化E53_IA
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_ia_init()
{
    e53_ia_io_init();
    init_bh1750();
    init_sht30();
}

/***************************************************************
* 函数名称: e53_ia_read_data
* 说    明: 测量光照强度、温度、湿度
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_ia_read_data(e53_ia_data_t *pData)
{
    uint8_t recv_data[2] = {0};
    uint32_t receive_len = 2;
    uint8_t rc;

    start_bh1750();
    LOS_Msleep(180);

    LzI2cRead(IA_I2C0, BH1750_ADDR, recv_data, receive_len);
    pData->luminance = (float)(((recv_data[0]<<8) + recv_data[1])/1.2);
    //printf("BH1750 data:0x%x%x", recv_data[0], recv_data[1]);

    /*checksum verification*/
    uint8_t data[3];
    uint16_t tmp;
    /*byte 0,1 is temperature byte 4,5 is humidity*/
    uint8_t SHT30_Data_Buffer[6];
    memset(SHT30_Data_Buffer, 0, 6);
    uint8_t send_data[2] = {0xE0, 0x00};
    uint32_t send_len = 2;
    LzI2cWrite(IA_I2C0, SHT30_ADDR, send_data, send_len);
    receive_len = 6;
    LzI2cRead(IA_I2C0, SHT30_ADDR, SHT30_Data_Buffer, receive_len);
    //printf("SHT30 data:0x%x%x%x)", SHT30_Data_Buffer[0], SHT30_Data_Buffer[1], SHT30_Data_Buffer[2]);

    /*check temperature*/
    data[0] = SHT30_Data_Buffer[0];
    data[1] = SHT30_Data_Buffer[1];
    data[2] = SHT30_Data_Buffer[2];
    rc = sht30_check_crc(data, 2, data[2]);
    if(!rc)
    {
        tmp = ((uint16_t)data[0] << 8) | data[1];
        pData->temperature = sht30_calc_temperature(tmp);
    }
    
    /*check humidity*/
    data[0] = SHT30_Data_Buffer[3];
    data[1] = SHT30_Data_Buffer[4];
    data[2] = SHT30_Data_Buffer[5];
    rc = sht30_check_crc(data, 2, data[2]);
    if(!rc)
    {
        tmp = ((uint16_t)data[0] << 8) | data[1];
        pData->humidity = sht30_calc_RH(tmp);
    }
}

/***************************************************************
* 函数名称: light_set
* 说    明: 紫光灯控制
* 参    数: 
*          OFF,关
*          ON,开
* 返 回 值: 无
***************************************************************/
void light_set(SWITCH_STATUS_ENUM status)
{
    if(status == ON)
    {
        /*设置GPIO0_PA2输出高电平点亮灯*/
        LzGpioSetVal(GPIO0_PA2, LZGPIO_LEVEL_HIGH);
    }
    if(status == OFF)
    {
        /*设置GPIO0_PA2输出低电平关闭灯*/
        LzGpioSetVal(GPIO0_PA2, LZGPIO_LEVEL_LOW);
    }
}

/***************************************************************
* 函数名称: motor_set_status
* 说    明: 电机控制
* 参    数: 
*          OFF,关
*          ON,开
* 返 回 值: 无
***************************************************************/
void motor_set_status(SWITCH_STATUS_ENUM status)
{
    if(status == ON)
    {
        /*设置GPIO0_PD0输出高电平打开电机*/
        LzGpioSetVal(GPIO1_PD0, LZGPIO_LEVEL_HIGH);
    }
    if(status == OFF)
    {   
        /*设置GPIO0_PD0输出低电平关闭电机*/
        LzGpioSetVal(GPIO1_PD0, LZGPIO_LEVEL_LOW);
    }
}

