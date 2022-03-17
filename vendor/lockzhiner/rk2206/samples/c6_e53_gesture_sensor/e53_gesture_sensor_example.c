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
#include "e53_gesture_sensor.h"

void e53_gs_process(void *arg)
{
    unsigned int ret = 0;
    unsigned short flag = 0;
    
    e53_gs_init();
    
    while (1)
    {
        ret = e53_gs_get_gesture_state(&flag);
        if (ret != 0)
        {
            printf("Get Gesture Statu: 0x%x\n", flag);
            if (flag & GES_UP)
            {
                printf("\tUp\n");
            }
            if (flag & GES_DOWM)
            {
                printf("\tDown\n");
            }
            if (flag & GES_LEFT)
            {
                printf("\tLeft\n");
            }
            if (flag & GES_RIGHT)
            {
                printf("\tRight\n");
            }
            if (flag & GES_FORWARD)
            {
                printf("\tForward\n");
            }
            if (flag & GES_BACKWARD)
            {
                printf("\tBackward\n");
            }
            if (flag & GES_CLOCKWISE)
            {
                printf("\tClockwise\n");
            }
            if (flag & GES_COUNT_CLOCKWISE)
            {
                printf("\tCount Clockwise\n");
            }
            if (flag & GES_WAVE)
            {
                printf("\tWave\n");
            }
            
            e53_gs_led_up_set((flag & GES_UP) ? (1) : (0));
            e53_gs_led_down_set((flag & GES_DOWM) ? (1) : (0));
            e53_gs_led_left_set((flag & GES_LEFT) ? (1) : (0));
            e53_gs_led_right_set((flag & GES_RIGHT) ? (1) : (0));
            e53_gs_led_forward_set((flag & GES_FORWARD) ? (1) : (0));
            e53_gs_led_backward_set((flag & GES_BACKWARD) ? (1) : (0));
            e53_gs_led_cw_set((flag & GES_CLOCKWISE) ? (1) : (0));
            e53_gs_led_ccw_set((flag & GES_COUNT_CLOCKWISE) ? (1) : (0));
            e53_gs_led_wave_set((flag & GES_WAVE) ? (1) : (0));
        }
        else
        {
            /* 如果没有数据，则多等待 */
            LOS_Msleep(100);
        }
    }
}

void e53_gs_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_gs_process;
    task.uwStackSize = 2048;
    task.pcName = "e53 getsture sensor process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create Task_One ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(e53_gs_example);
