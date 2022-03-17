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
#include "lz_hardware.h"
#include "ohos_init.h"
#include "los_task.h"
#include "e53_intelligent_vehicle_01.h"

void e53_iv01_process(void *arg)
{
    unsigned int ret = 0;
    /* 每个周期为200usec，占空比为100usec */
    unsigned int duty_ns = 500000;
    unsigned int cycle_ns = 1000000;
    float distance_cm = 0.0;
    
    e53_iv01_init();
    
    while (1)
    {
        printf("========== E53 IV Example ==========\n");

        ret = e53_iv01_get_distance(&distance_cm);
        if (ret == 1)
        {
            printf("distance cm: %f\n", distance_cm);
            
            if (distance_cm <= 20.0)
            {
                e53_iv01_buzzer_set(1, duty_ns, cycle_ns);
                e53_iv01_led_warning_set(1);
            }
            else
            {
                e53_iv01_buzzer_set(0, duty_ns, cycle_ns);
                e53_iv01_led_warning_set(0);
            }
        }
        
        LOS_Msleep(2000);
    }
}

/***************************************************************
* 函数名称: e53_iv01_example
* 说    明: 智慧农业例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_iv01_example()
{
    unsigned int ret = 0;
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_iv01_process;
    task.uwStackSize = 10240;
    task.pcName = "e53_iv01_process";
    task.usTaskPrio = 25;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_ia_thread ret:0x%x\n", ret);
        return;
    }
}


APP_FEATURE_INIT(e53_iv01_example);

