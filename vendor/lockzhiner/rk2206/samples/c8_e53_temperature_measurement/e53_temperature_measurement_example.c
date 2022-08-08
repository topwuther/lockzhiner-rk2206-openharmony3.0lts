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

#include "ohos_init.h"
#include "los_task.h"
#include "e53_temperature_measurement.h"

/***************************************************************
* 函数名称: e53_tm_thread
* 说    明: 红外测温模块例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_tm_thread()
{
    unsigned int ret = LZ_HARDWARE_SUCCESS;
    float temperature = 0;

    e53_tm_init();

    while (1)
    {
        if (SUCCESS == e53_tm_read_data(&temperature)) {
            printf("temperature:%0.1f\n", temperature);
            LOS_Msleep(1000);
        } else {
            LOS_Msleep(10);
        }
    }
}

void e53_tm_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_tm_thread;
    task.uwStackSize = 10240;
    task.pcName = "e53_tm_thread";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_tm_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(e53_tm_example);

