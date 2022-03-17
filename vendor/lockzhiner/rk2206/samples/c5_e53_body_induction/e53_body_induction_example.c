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
#include "e53_body_induction.h"

/***************************************************************
* 函数名称: e53_bi_thread
* 说    明: 人体感应模块线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_bi_thread()
{
    unsigned int ret;
    LzGpioValue val = LZGPIO_LEVEL_LOW, val_last = LZGPIO_LEVEL_LOW;

    e53_bi_init();

    while (1)
    {
        ret = LzGpioGetVal(GPIO0_PA5, &val);
        if (ret != LZ_HARDWARE_SUCCESS)
        {
            printf("get gpio value failed ret:%d\n", ret);
        }
        if (val_last != val)
        {
            if (val == LZGPIO_LEVEL_HIGH)
            {
                buzzer_set_status(ON);
                printf("buzzer on\n");
                LOS_Msleep(1000);
                buzzer_set_status(OFF);
                printf("buzzer off\n");
            }
            val_last = val;
        }
        LOS_Msleep(1000);
    }
}

/***************************************************************
* 函数名称: e53_bi_example
* 说    明: 人体感应模块例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_bi_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_bi_thread;
    task.uwStackSize = 10240;
    task.pcName = "e53_bi_thread";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_bi_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(e53_bi_example);
