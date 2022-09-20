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


/***************************************************************
* 函数名称: watchdog_process
* 说    明: 看门狗喂狗任务
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void watchdog_process()
{
    uint32_t current = 0;
    
    printf("%s: start\n", __func__);
    LzWatchdogInit();
    /* 实际是1.3981013 * (2 ^ 4) = 22.3696208秒 */
    LzWatchdogSetTimeout(20);
    LzWatchdogStart(LZ_WATCHDOG_REBOOT_MODE_FIRST);

    while (1)
    {
        printf("Wathdog: current(%d)\n", ++current);
        if (current <= 10)
        {
            printf("    freedog\n");
            LzWatchdogKeepAlive();
        }
        else
        {
            printf("    not freedog\n");
        }
        
        LOS_Msleep(1000);
    }
}


/***************************************************************
* 函数名称: watchdog_example
* 说    明: 开机自启动调用函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void watchdog_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)watchdog_process;
    task.uwStackSize = 20480;
    task.pcName = "watchdog process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(watchdog_example);