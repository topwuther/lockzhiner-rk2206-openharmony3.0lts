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

#include "los_sem.h"
#include "ohos_init.h"

#define MAX_COUNT                   4

static unsigned int m_sem;

/***************************************************************
* 函数名称: control_thread
* 说    明: 控制线程函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void control_thread()
{
    unsigned int count = 0;

    while (1)
    {
        /*释放两次信号量，sem_one_thread和sem_two_thread同步执行;
        释放一次信号量，sem_one_thread和sem_two_thread交替执行*/
        if (count++%3)
        {
            LOS_SemPost(m_sem);
            printf("control_thread Release once Semaphore\n");
        }
        else
        {
            LOS_SemPost(m_sem);
            LOS_SemPost(m_sem);
            printf("control_thread Release twice Semaphore\n");
        }
        
        LOS_Msleep(1000);
    }
}

/***************************************************************
* 函数名称: sem_one_thread
* 说    明: 信号量线程函数1
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void sem_one_thread()
{
    while (1)
    {
        /*申请信号量*/
        LOS_SemPend(m_sem, LOS_WAIT_FOREVER);

        printf("sem_one_thread get Semaphore\n");
        LOS_Msleep(100);
    }
}

/***************************************************************
* 函数名称: sem_two_thread
* 说    明: 信号量线程函数2
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void sem_two_thread()
{
    while (1)
    {
        /*申请信号量*/
        LOS_SemPend(m_sem, LOS_WAIT_FOREVER);

        printf("sem_two_thread get Semaphore\n");
        LOS_Msleep(100);
    }
}

/***************************************************************
* 函数名称: semaphore_example
* 说    明: 内核信号量函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void semaphore_example()
{
    unsigned int thread_crtl;
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    TSK_INIT_PARAM_S task3 = {0};
    unsigned int ret = LOS_OK;

    ret = LOS_SemCreate(MAX_COUNT, &m_sem);
    if (ret != LOS_OK)
    {
        printf("Falied to create Semaphore\n");
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)control_thread;
    task1.uwStackSize = 2048;
    task1.pcName = "control_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_crtl, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create control_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)sem_one_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "sem_one_thread";
    task2.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create sem_one_thread ret:0x%x\n", ret);
        return;
    }

    task3.pfnTaskEntry = (TSK_ENTRY_FUNC)sem_two_thread;
    task3.uwStackSize = 2048;
    task3.pcName = "sem_two_thread";
    task3.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id2, &task3);
    if (ret != LOS_OK)
    {
        printf("Falied to create sem_two_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(semaphore_example);

