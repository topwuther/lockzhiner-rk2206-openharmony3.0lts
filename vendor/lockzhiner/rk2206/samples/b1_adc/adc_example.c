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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "los_task.h"
#include "ohos_init.h"
#include "lz_hardware.h"

/* 定义ADC的通道号 */
#define ADC_CHANNEL         5
/* 定义ADC初始化的结构体 */
static DevIo m_adcKey = {
    .isr =   {.gpio = INVALID_GPIO},
    .rst =   {.gpio = INVALID_GPIO},
    .ctrl1 = {.gpio = GPIO0_PC5, .func = MUX_FUNC1, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_IN, .val = LZGPIO_LEVEL_KEEP},
    .ctrl2 = {.gpio = INVALID_GPIO},
};


/***************************************************************
* 函数名称: adc_dev_init
* 说    明: 初始化ADC
* 参    数: 无
* 返 回 值: 0为成功，反之为失败
***************************************************************/
static unsigned int adc_dev_init()
{
    unsigned int ret = 0;
    uint32_t *pGrfSocCon29 = (uint32_t *)(0x41050000U + 0x274U);
    uint32_t ulValue;

    ret = DevIoInit(m_adcKey);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %s, %d: ADC Key IO Init fail\n", __FILE__, __func__, __LINE__);
        return __LINE__;
    }
    ret = LzSaradcInit();
    if (ret != LZ_HARDWARE_SUCCESS) {
        printf("%s, %s, %d: ADC Init fail\n", __FILE__, __func__, __LINE__);
        return __LINE__;
    }

    /* 设置saradc的电压信号，选择AVDD */
    ulValue = *pGrfSocCon29;
    ulValue &= ~(0x1 << 4);
    ulValue |= ((0x1 << 4) << 16);
    *pGrfSocCon29 = ulValue;
    
    return 0;
}

/***************************************************************
* 函数名称: Get_Voltage
* 说    明: 获取ADC电压值
* 参    数: 无
* 返 回 值: 电压值
***************************************************************/
static float adc_get_voltage()
{
    unsigned int ret = LZ_HARDWARE_SUCCESS;
    unsigned int data = 0;

    ret = LzSaradcReadValue(ADC_CHANNEL, &data);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("%s, %s, %d: ADC Read Fail\n", __FILE__, __func__, __LINE__);
        return 0.0;
    }

    return (float)(data * 3.3 / 1024.0);
}

/***************************************************************
* 函数名称: adc_process
* 说    明: ADC采集循环任务
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void adc_process()
{
    float voltage;

    /* 初始化adc设备 */
    adc_dev_init();
    
    while (1)
    {
        printf("***************Adc Example*************\r\n");
        /*获取电压值*/
        voltage = adc_get_voltage();
        printf("vlt:%.3fV\n", voltage);

        /* 睡眠1秒 */
        usleep(1000000);
    }
}


/***************************************************************
* 函数名称: adc_example
* 说    明: 开机自启动调用函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void adc_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)adc_process;
    task.uwStackSize = 2048;
    task.pcName = "adc process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(adc_example);