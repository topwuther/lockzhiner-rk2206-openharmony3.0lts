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

#include "los_event.h"
#include "los_task.h"
#include "ohos_init.h"

#define EVENT_WAIT                                0x00000001

static EVENT_CB_S m_event;

/***************************************************************
* 函数名称: event_master_thread
* 说    明: 事件主线程函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void event_master_thread()
{
    unsigned int ret = LOS_OK;

    LOS_Msleep(1000);

    while (1)
    {
        printf("%s write event:0x%x\n", __func__, EVENT_WAIT);
        ret = LOS_EventWrite(&m_event, EVENT_WAIT);
        if (ret != LOS_OK) {
            printf("%s write event failed ret:0x%x\n", __func__, ret);
        }

        /*delay 1s*/
        LOS_Msleep(2000);
        LOS_EventClear(&m_event, ~m_event.uwEventID);
    }
}

/***************************************************************
* 函数名称: event_slave_thread
* 说    明: 事件从线程函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void event_slave_thread()
{
    unsigned int event;

    while (1)
    {
        /* 阻塞方式读事件，等待事件到达*/
        event = LOS_EventRead(&m_event, EVENT_WAIT, LOS_WAITMODE_AND, LOS_WAIT_FOREVER);
        printf("%s read event:0x%x\n", __func__, event);
        LOS_Msleep(1000);
    }
}

/***************************************************************
* 函数名称: event_example
* 说    明: 内核事件函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void event_example()
{
    unsigned int thread_id1;
    unsigned int thread_id2;
    TSK_INIT_PARAM_S task1 = {0};
    TSK_INIT_PARAM_S task2 = {0};
    unsigned int ret = LOS_OK;

    ret = LOS_EventInit(&m_event);
    if (ret != LOS_OK)
    {
        printf("Falied to create EventFlags\n");
        return;
    }

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)event_master_thread;
    task1.uwStackSize = 2048;
    task1.pcName = "event_master_thread";
    task1.usTaskPrio = 5;
    ret = LOS_TaskCreate(&thread_id1, &task1);
    if (ret != LOS_OK)
    {
        printf("Falied to create event_master_thread ret:0x%x\n", ret);
        return;
    }

    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)event_slave_thread;
    task2.uwStackSize = 2048;
    task2.pcName = "event_slave_thread";
    task2.usTaskPrio = 5;
    ret = LOS_TaskCreate(&thread_id2, &task2);
    if (ret != LOS_OK)
    {
        printf("Falied to create event_slave_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(event_example);

