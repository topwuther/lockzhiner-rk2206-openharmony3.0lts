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

#define GPIO_TEST       GPIO0_PA0

/* 记录中断触发次数 */
static unsigned int m_gpio_interrupt_count = 0;

/***************************************************************
* 函数名称: gpio_isr_func
* 说    明: gpio中断响应处理函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void gpio_isr_func(void *args)
{
    m_gpio_interrupt_count++;
}

/***************************************************************
* 函数名称: gpio_process
* 说    明: gpio任务
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void gpio_process()
{
    unsigned int ret;

    /* 初始化引脚为GPIO */
    LzGpioInit(GPIO_TEST);
    /* 引脚复用配置为GPIO */
    PinctrlSet(GPIO_TEST, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);
    /* 引脚配置为输入 */
    LzGpioSetDir(GPIO_TEST, LZGPIO_DIR_IN);
    /* 设置中断触发方式为上升沿和中断处理函数，
     * 详见//device/rockchip/rk2206/adapter/include/lz_hardware/gpio.h
     */
    ret = LzGpioRegisterIsrFunc(GPIO_TEST, LZGPIO_INT_EDGE_RISING, gpio_isr_func, NULL);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("LzGpioRegisterIsrFunc failed(%d)\n", ret);
        return;
    }

    /* 初始化中断触发次数 */
    m_gpio_interrupt_count = 0;
    /* 开启中断使能，
     * 注意：如果手动将GPIO接到接地线，则容易出现抖动，
     * 导致GPIO中断次数一下就变得很大
     */
    LzGpioEnableIsr(GPIO_TEST);
    
    while (1)
    {
        printf("***************GPIO Interrupt Example*************\n");
        printf("gpio interrupt count = %d\n", m_gpio_interrupt_count);
        printf("\n");
        
        /* 睡眠1秒 */
        sleep(1);
    }
}


/***************************************************************
* 函数名称: gpio_example
* 说    明: 开机自启动调用函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void gpio_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)gpio_process;
    task.uwStackSize = 2048;
    task.pcName = "gpio process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(gpio_example);