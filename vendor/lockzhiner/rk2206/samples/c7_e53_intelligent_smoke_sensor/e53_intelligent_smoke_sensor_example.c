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
#include "e53_intelligent_smoke_sensation.h"

/***************************************************************
* 函数名称: e53_iss_thread
* 说    明: 烟感测试
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_iss_thread(void *arg)
{
    float ppm = 0;

    e53_iss_init();
    /*传感器校准*/
    usleep(2000000); // 开机2s后进行校准
    e53_iss_mq2_ppm_calibration(); // 校准传感器
    
    while (1)
    {
        printf("=======================================\r\n");
        printf("*************e53_iss_example***********\r\n");
        printf("=======================================\r\n");
        ppm = e53_iss_get_mq2_ppm();
        printf("ppm:%.3f \n", ppm);
        /*判断是否达到报警阈值*/
        if (ppm > e53_iss_get_mq2_alarm_value())
        {
            e53_iss_led_status_set(ON);
            e53_iss_beep_status_set(ON);
        }
        else
        {
            e53_iss_led_status_set(OFF);
            e53_iss_beep_status_set(OFF);
        }
        usleep(1000000); // 延时1s
    }
}

/***************************************************************
* 函数名称: e53_iss_example
* 说    明: 智慧烟感例程
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_iss_example()
{
    unsigned int ret = LOS_OK;
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    
    task.pfnTaskEntry = (TSK_ENTRY_FUNC)e53_iss_thread;
    task.uwStackSize = 1024 * 2;
    task.pcName = "e53_iss_thread";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create e53_iss_thread ret:0x%x\n", ret);
        return;
    }
}

APP_FEATURE_INIT(e53_iss_example);
