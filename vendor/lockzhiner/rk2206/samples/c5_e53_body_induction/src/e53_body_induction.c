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

#include "e53_body_induction.h"

#define BI_I2C0                             0 
#define BI_PWM7                             7

static I2cBusIo m_bi_i2c0m2 = {
    .scl =  {.gpio = GPIO0_PA1, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .sda =  {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_I2C0,
    .mode = FUNC_MODE_M2,
};

static PwmBusIo m_buzzer  = {
    .pwm = {.gpio = GPIO1_PD0, .func = MUX_FUNC1, .type = PULL_DOWN, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
    .id = FUNC_ID_PWM7,
    .mode = FUNC_MODE_NONE,
};

/***************************************************************
* 函数名称: e53_bi_init
* 说    明: E53_BI初始化
* 参    数: 无
* 返 回 值: 无
***************************************************************/
void e53_bi_init()
{
    uint32_t ret = LZ_HARDWARE_SUCCESS;
#if 0
    /*初始化I2C*/
    ret = I2cIoInit(m_bi_i2c0m2);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 io fail ret:%d\n", ret);
        return;
    }
    ret = LzI2cInit(BI_I2C0, 400000);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 fail ret:%d\n", ret);
        return;
    }
#endif
    ret = PwmIoInit(m_buzzer);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("PwmIoInit failed ret:%d\n", ret);
        return;
    }

    ret = LzPwmInit(BI_PWM7);
    if (ret != LZ_HARDWARE_SUCCESS)
    {
        printf("LzPwmInit 7 failed ret:%d\n", ret);
        return;
    }

    LzGpioInit(GPIO0_PA5);
    ret = LzGpioSetDir(GPIO0_PA5, LZGPIO_DIR_IN);
    if (ret != LZ_HARDWARE_SUCCESS) {
        printf("LzGpioSetDir(GPIO0_PA5) failed ret:%d\n", ret);
        return;
    }
}

/***************************************************************
* 函数名称: buzzer_set_status
* 说    明: 蜂鸣器控制
* 参    数: OFF,蜂鸣器
*          ON,开蜂鸣器
* 返 回 值: 无
***************************************************************/
void buzzer_set_status(SWITCH_STATUS_ENUM status)
{
    if(status == ON)
    {
        LzPwmStart(BI_PWM7, 500000, 1000000);
    }
    if(status == OFF)
    {
        LzPwmStop(BI_PWM7);
    }
}

