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

static unsigned int m_mutex_id;
static unsigned int m_data = 0;

/***************************************************************
* 函数名称: write_thread
* 说    明: 写数据线程函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void write_thread()
{
    while (1)
    {
        LOS_MuxPend(m_mutex_id, LOS_WAIT_FOREVER);

        m_data++;
        printf("write_thread write data:%u\n", m_data);

        LOS_Msleep(3000);
        LOS_MuxPost(m_mutex_id);
    }
}

/***************************************************************
* 函数名称: read_thread
* 说    明: 读数据线程函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void read_thread()
{
    /*delay 1s*/
    LOS_Msleep(1000);

    while (1)
    {
        LOS_MuxPend(m_mutex_id, LOS_WAIT_FOREVER);
        printf("read_thread read data:%u\n", m_data);
    
        LOS_Msleep(1000);
        LOS_MuxPost(m_mutex_id);
    }
}

/***************************************************************
* 函数名称: mutex_example
* 说    明: 内核互斥锁函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void mutex_example()
{
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    unsigned int ret = LOS_OK;

    ret = LOS_MuxCreate(&m_mutex_id);
    if (ret != LOS_OK)
    {
        printf("Falied to create Mutex\n");
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)write_thread;
    task1.uwStackSize = 2048;
    task1.pcName = "write_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create write_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)read_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "read_thread";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create read_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(mutex_example);

