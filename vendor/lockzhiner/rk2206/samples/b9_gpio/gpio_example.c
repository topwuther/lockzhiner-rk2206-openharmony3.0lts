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

#define GPIO_TEST     GPIO0_PA0

/***************************************************************
* 函数名称: gpio_process
* 说    明: gpio任务
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void gpio_process()
{
    unsigned int cur = 0;
    LzGpioValue value = LZGPIO_LEVEL_LOW;

    /* 初始化引脚为GPIO */
    LzGpioInit(GPIO_TEST);
    /* 引脚复用配置为GPIO */
    PinctrlSet(GPIO_TEST, MUX_FUNC0, PULL_KEEP, DRIVE_LEVEL0);
    
    while (1)
    {
        printf("***************GPIO Example*************\r\n");
        printf("Write GPIO\n");
        LzGpioSetDir(GPIO_TEST, LZGPIO_DIR_OUT);
        if (cur == 0)
        {
            LzGpioSetVal(GPIO_TEST, cur);
            LzGpioGetVal(GPIO_TEST, &value);
            printf("\tgpio set %d => gpio get %d\n", cur, value);

            cur = 1;
        }
        else
        {
            LzGpioSetVal(GPIO_TEST, cur);
            LzGpioGetVal(GPIO_TEST, &value);
            printf("\tgpio set %d => gpio get %d\n", cur, value);

            cur = 0;
        }
        /* 睡眠5秒 */
        sleep(5);

        printf("Read GPIO\n");
        LzGpioSetDir(GPIO_TEST, LZGPIO_DIR_IN);
        LzGpioGetVal(GPIO_TEST, &value);
        printf("\tgpio get %d\n", value);
        /* 睡眠5秒 */
        sleep(5);

        printf("\n");
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