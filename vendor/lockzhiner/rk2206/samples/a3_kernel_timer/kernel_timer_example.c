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

#include "los_swtmr.h"
#include "ohos_init.h"

/***************************************************************
* 函数名称: timer1_timeout
* 说    明: 定时器1超时函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void timer1_timeout(void *arg)
{
    printf("This is Timer1 Timeout function\n");
}

/***************************************************************
* 函数名称: timer2_timeout
* 说    明: 定时器2超时函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void timer2_timeout(void *arg)
{
    printf("This is Timer2 Timeout function\n");
}

/***************************************************************
* 函数名称: timer_example
* 说    明: 内核定时器函数
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void timer_example()
{
    unsigned int timer_id1, timer_id2;
    unsigned int ret;

    ret = LOS_SwtmrCreate(1000, LOS_SWTMR_MODE_PERIOD, timer1_timeout, &timer_id1, NULL);
    if (ret == LOS_OK)
    {
        ret = LOS_SwtmrStart(timer_id1);
        if (ret != LOS_OK)
        {
            printf("start timer1 fail ret:0x%x\n", ret);
            return;
        }
    }
    else
    {
        printf("create timer1 fail ret:0x%x\n", ret);
        return;
    }

    ret = LOS_SwtmrCreate(2000, LOS_SWTMR_MODE_PERIOD, timer2_timeout, &timer_id2, NULL);
    if (ret == LOS_OK)
    {
        ret = LOS_SwtmrStart(timer_id2);
        if (ret != LOS_OK)
        {
            printf("start timer2 fail ret:0x%x\n", ret);
            return;
        }
    }
    else
    {
        printf("create timer2 fail ret:0x%x\n"), ret;
        return;
    }
}

APP_FEATURE_INIT(timer_example);

