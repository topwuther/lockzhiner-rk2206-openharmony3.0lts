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

#include "los_task.h"
#include "ohos_init.h"

/***************************************************************
* 函数名称: task_one
* 说    明: 线程函数1
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void task_one()
{
    while (1)
    {
        printf("This is %s\n", __func__);
        LOS_Msleep(1000);
    }
}

/***************************************************************
* 函数名称: task_two
* 说    明: 线程函数2
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void task_two()
{
    while (1)
    {
        printf("This is %s\n", __func__);
        LOS_Msleep(2000);
    }
}

/***************************************************************
* 函数名称: task_example
* 说    明: 内核任务例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void task_example()
{
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    unsigned int ret = LOS_OK;

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)task_one;
    task1.uwStackSize = 2048;
    task1.pcName = "Task_One";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create Task_One ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)task_two;
    task2.uwStackSize = 2048;
    task2.pcName = "Task_Two";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create Task_Two ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(task_example);

