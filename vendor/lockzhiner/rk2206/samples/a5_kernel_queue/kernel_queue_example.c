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

#define MSG_QUEUE_LENGTH                                16
#define BUFFER_LEN                                      50

static unsigned int m_msg_queue;

/***************************************************************
* 函数名称: msg_write_thread
* 说    明: 队列写函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void msg_write_thread(void *arg)
{
    unsigned int data = 0;
    unsigned int ret = LOS_OK;

    while (1)
    {
        data++;
        ret = LOS_QueueWrite(m_msg_queue, (void *)&data, sizeof(data), LOS_WAIT_FOREVER);
        if (LOS_OK != ret)
        {
            printf("%s write Message Queue msg fail ret:0x%x\n", __func__, ret);
        }
        else
        {
            printf("%s write Message Queue msg:%u\n", __func__, data);
        }

        /*delay 1s*/
        LOS_Msleep(1000);
    }
}

/***************************************************************
* 函数名称: msg_read_thread
* 说    明: 队列读函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void msg_read_thread(void *arg)
{
    unsigned int addr;
    unsigned int ret = LOS_OK;
    unsigned int *pData = NULL;

    while (1)
    {
        /*wait for message*/
        ret = LOS_QueueRead(m_msg_queue, (void *)&addr, BUFFER_LEN, LOS_WAIT_FOREVER);
        if (ret == LOS_OK)
        {
            pData = addr;
            printf("%s read Message Queue msg:%u\n", __func__, *pData);
        }
        else
        {
            printf("%s read Message Queue fail ret:0x%x\n", __func__, ret);
        }
    }
}

/***************************************************************
* 函数名称: queue_example
* 说    明: 内核队列函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void queue_example()
{
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    unsigned int ret = LOS_OK;

    ret = LOS_QueueCreate("queue", MSG_QUEUE_LENGTH, &m_msg_queue, 0, BUFFER_LEN);
    if (ret != LOS_OK)
    {
        printf("Falied to create Message Queue ret:0x%x\n", ret);
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)msg_write_thread;
    task1.uwStackSize = 2048;
    task1.pcName = "msg_write_thread";
    task1.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create msg_write_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)msg_read_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "msg_read_thread";
    task2.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create msg_read_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(queue_example);

