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
#include "lz_hardware.h"

void oled_process(void *arg)
{
    unsigned char buffer[32];
    int i = 0;
    
    oled_init();
    oled_clear();
    
    while (1)
    {
        printf("========= Oled Process =============\n");
        oled_show_string(6, 0, "0.96' OLED TEST", 16);
        oled_show_string(0, 3, "ASCII:", 16);
        oled_show_string(64, 3, "CODE:", 16);
        
        snprintf(buffer, sizeof(buffer), "%d Sec!", i++);
        oled_show_string(40, 6, buffer, 16);
        
        printf("\n\n");
        LOS_Msleep(1000);
    }
}


/***************************************************************
* 函数名称: oled_example
* 说    明: 开机自启动调用函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void oled_example()
{
    unsigned int thread_id;
    TSK_INIT_PARAM_S task = {0};
    unsigned int ret = LOS_OK;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)oled_process;
    task.uwStackSize = 2048;
    task.pcName = "oled process";
    task.usTaskPrio = 24;
    ret = LOS_TaskCreate(&thread_id, &task);
    if (ret != LOS_OK)
    {
        printf("Falied to create task ret:0x%x\n", ret);
        return;
    }
}


APP_FEATURE_INIT(oled_example);
