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

#include "ohos_init.h"
#include "los_task.h"
#include "e53_intelligent_agriculture.h"

/***************************************************************
* 函数名称: e53_ia_thread
* 说    明: 智慧农业线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_ia_thread()
{
    e53_ia_data_t data;

    e53_ia_init();

    while (1)
    {
        e53_ia_read_data(&data);

        printf("\nLuminance is %.2f\n", data.luminance);
        printf("\nHumidity is %.2f\n", data.humidity);
        printf("\nTemperature is %.2f\n", data.temperature);

        if (data.luminance < 20)
        {
            light_set(ON);
            printf("light on\n");
        }
        else
        {
            light_set(OFF);
            printf("light off\n");
        }

        if ((data.humidity > 60) || (data.temperature > 30))
        {
            motor_set_status(ON);
            printf("motor on\n");
        }
        else
        {
            motor_set_status(OFF);
            printf("motor off\n");
        }
        LOS_Msleep(2000);
    }
}

/***************************************************************
* 函数名称: e53_ia_example
* 说    明: 智慧农业例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_ia_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_ia_thread;
    task.uwStackSize = 10240;
    task.pcName = "e53_ia_thread";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_ia_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(e53_ia_example);
