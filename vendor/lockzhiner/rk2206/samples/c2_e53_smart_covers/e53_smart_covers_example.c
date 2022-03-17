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
#include "e53_smart_covers.h"

/***************************************************************
* 函数名称: e53_sc_thread
* 说    明: E53智慧井盖线程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_sc_thread()
{
    e53_sc_data_t data;
    int x = 0, y = 0, z = 0;

    e53_sc_init();
    led_d1_set(OFF);
    led_d2_set(OFF);

    while (1)
    {
        e53_sc_read_data(&data);
        printf("x is %d\n", (int)data.accel[0]);
        printf("y is %d\n", (int)data.accel[1]);
        printf("z is %d\n", (int)data.accel[2]);
        printf("init x:%d y:%d z:%d\n", x, y, z);

        if (x == 0 && y == 0 && z == 0)
        {
            x = (int)data.accel[0];
            y = (int)data.accel[1];
            z = (int)data.accel[2];
        }
        else
        {
            if ((x + DELTA) < data.accel[0] || (x - DELTA) > data.accel[0] ||
                (y + DELTA) < data.accel[1] || (y - DELTA) > data.accel[1] ||
                (z + DELTA) < data.accel[2] || (z - DELTA) > data.accel[2])
            {
                /*倾斜告警*/
                led_d1_set(OFF);
                led_d2_set(ON);
                data.tilt_status = 1;
                printf("tilt warning \nLED1 OFF LED2 On\n");
            }
            else
            {
                led_d1_set(ON);
                led_d2_set(OFF);
                data.tilt_status = 0;
                printf("normal \nLED1 ON LED2 OFF\n");
            }
        }
        LOS_Msleep(2000);
    }
}

/***************************************************************
* 函数名称: e53_sc_example
* 说    明: 智慧井盖例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_sc_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_sc_thread;
    task.uwStackSize = 10240;
    task.pcName = "e53_sc_thread";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_sc_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(e53_sc_example);
