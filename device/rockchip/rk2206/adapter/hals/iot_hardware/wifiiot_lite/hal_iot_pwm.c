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


#include "iot_errno.h"
#include "iot_pwm.h"
#include "lz_hardware.h"

#define  TEST_PORT 0
PwmBusIo pwm = {
            .pwm = {.gpio = GPIO0_PB4, .func = MUX_FUNC1, .type = PULL_DOWN, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
            .id  = FUNC_ID_PWM0,
            .mode = FUNC_MODE_M1,
        };

unsigned int IoTPwmInit(unsigned int port)
{
    unsigned int ret = 0;

    port = TEST_PORT;
    ret = PwmIoInit(pwm);
    if (ret != 0)
    {
        return ret;
    }

    return LzPwmInit(port);
}

unsigned int IoTPwmDeinit(unsigned int port)
{
    port = TEST_PORT;
    return LzPwmDeinit(port);
}

unsigned int IoTPwmStart(unsigned int port, unsigned short duty, unsigned int freq)
{
    unsigned int toyDuty;

    port = TEST_PORT;
    toyDuty = duty * 1000;
    return LzPwmStart(port, toyDuty, freq);
}

unsigned int IoTPwmStop(unsigned int port)
{
    port = TEST_PORT;
    return LzPwmStop(port);
}
