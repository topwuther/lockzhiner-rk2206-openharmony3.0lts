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
#include "los_task.h"
#include "ohos_init.h"

/***************************************************************
* 函数名称: hal_pwm_thread
* 说    明: 控制PWM线程函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void hal_pw_thread()
{
    unsigned int ret;
    /* PWM端口号对应于参考文件：
     * device/rockchip/rk2206/adapter/hals/iot_hardware/wifiiot_lite/hal_iot_pwm.c
     */
    unsigned int port = 0;

    while (1)
    {
        printf("===========================\n");
        printf("PWM(%d) Init\n", port);
        ret = IoTPwmInit(port);
        if (ret != 0) {
            printf("IoTPwmInit failed(%d)\n");
            continue;
        }

        printf("PWM(%d) Start\n", port);
        ret = IoTPwmStart(port, 50, 1000);
        if (ret != 0) {
            printf("IoTPwmStart failed(%d)\n");
            continue;
        }
        
        LOS_Msleep(5000);
        
        printf("PWM(%d) end\n", port);
        ret = IoTPwmStop(port);
        if (ret != 0) {
            printf("IoTPwmStop failed(%d)\n");
            continue;
        }
        
        ret = IoTPwmDeinit(port);
        if (ret != 0) {
            printf("IoTPwmInit failed(%d)\n");
            continue;
        }
        
        printf("\n");

        port++;
        if (port >= 11) {
            port = 0;
        }
    }
}

/***************************************************************
* 函数名称: pwm_example
* 说    明: pwm控制入口函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void pwm_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)hal_pw_thread;
    task.uwStackSize = 2048;
    task.pcName = "hal_pwm_thread";
    task.usTaskPrio = 20;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create hal_pw_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(pwm_example);
