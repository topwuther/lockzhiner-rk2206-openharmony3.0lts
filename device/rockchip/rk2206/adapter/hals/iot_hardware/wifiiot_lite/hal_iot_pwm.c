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

/* 定义PWM设备数量 */
enum EnumPwmDev {
    EPWMDEV_PWM0_M1 = 0,    /* GPIO_PB4 */
    EPWMDEV_PWM1_M1,        /* GPIO_PB5 */
    EPWMDEV_PWM2_M1,        /* GPIO_PB6 */
    EPWMDEV_PWM0_M0,        /* GPIO_PC0 */
    EPWMDEV_PWM1_M0,        /* GPIO_PC1 */
    EPWMDEV_PWM2_M0,        /* GPIO_PC2 */
    EPWMDEV_PWM3_M0,        /* GPIO_PC3 */
    EPWMDEV_PWM4_M0,        /* GPIO_PC4 */
    EPWMDEV_PWM5_M0,        /* GPIO_PC5 */
    EPWMDEV_PWM6_M0,        /* GPIO_PC6 */
    EPWMDEV_PWM7_M0,        /* GPIO_PC7 */
    EPWMDEV_MAX
};

struct PwmBusInfo {
    unsigned int port;
    PwmBusIo pwm_bus;
};

static struct PwmBusInfo m_pwm_bus_info[EPWMDEV_MAX] = {
    [EPWMDEV_PWM0_M1] = {
        .port = 0,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PB4,
                .func = MUX_FUNC1,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM0,
            .mode = FUNC_MODE_M1,
        },
    },
    [EPWMDEV_PWM1_M1] = {
        .port = 1,
        .pwm_bus = {
            .pwm = {.gpio = GPIO0_PB5,
                .func = MUX_FUNC1,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM1,
            .mode = FUNC_MODE_M1,
        },
    },
    [EPWMDEV_PWM2_M1] = {
        .port = 2,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PB6,
                .func = MUX_FUNC1,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM0,
            .mode = FUNC_MODE_M1,
        },
    },
    [EPWMDEV_PWM0_M0] = {
        .port = 0,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PC0,
                .func = MUX_FUNC2,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM0,
            .mode = FUNC_MODE_M0,
        },
    },
    [EPWMDEV_PWM1_M0] = {
        .port = 1,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PC1,
                .func = MUX_FUNC2,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM1,
            .mode = FUNC_MODE_M0,
        },
    },
    [EPWMDEV_PWM2_M0] = {
        .port = 2,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PC2,
                .func = MUX_FUNC2,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM0,
            .mode = FUNC_MODE_M0,
        },
    },
    [EPWMDEV_PWM3_M0] = {
        .port = 3,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PC3,
                .func = MUX_FUNC2,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM3,
            .mode = FUNC_MODE_NONE,
        },
    },
    [EPWMDEV_PWM4_M0] = {
        .port = 4,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PC4,
                .func = MUX_FUNC2,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM4,
            .mode = FUNC_MODE_NONE,
        },
    },
    [EPWMDEV_PWM5_M0] = {
        .port = 5,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PC5,
                .func = MUX_FUNC2,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM5,
            .mode = FUNC_MODE_NONE,
        },
    },
    [EPWMDEV_PWM6_M0] = {
        .port = 6,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PC6,
                .func = MUX_FUNC2,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM6,
            .mode = FUNC_MODE_NONE,
        },
    },
    [EPWMDEV_PWM7_M0] = {
        .port = 7,
        .pwm_bus = {
            .pwm = {
                .gpio = GPIO0_PC7,
                .func = MUX_FUNC2,
                .type = PULL_DOWN,
                .drv = DRIVE_KEEP,
                .dir = LZGPIO_DIR_KEEP,
                .val = LZGPIO_LEVEL_KEEP
            },
            .id  = FUNC_ID_PWM7,
            .mode = FUNC_MODE_NONE,
        },
    },
};

unsigned int IoTPwmInit(unsigned int port)
{
    unsigned int ret = 0;

    if (port >= EPWMDEV_MAX) {
        PRINT_ERR("port(%d) >= EPWMDEV_MAX(%d)\n", port, EPWMDEV_MAX);
        return IOT_FAILURE;
    }

    PinctrlSet(m_pwm_bus_info[port].pwm_bus.pwm.gpio,
        m_pwm_bus_info[port].pwm_bus.pwm.func,
        m_pwm_bus_info[port].pwm_bus.pwm.type,
        m_pwm_bus_info[port].pwm_bus.pwm.drv);
    PwmIoInit(m_pwm_bus_info[port].pwm_bus);
    LzPwmInit(m_pwm_bus_info[port].port);

    return IOT_SUCCESS;
}

unsigned int IoTPwmDeinit(unsigned int port)
{
    if (port >= EPWMDEV_MAX) {
        PRINT_ERR("port(%d) >= EPWMDEV_MAX(%d)\n", port, EPWMDEV_MAX);
        return IOT_FAILURE;
    }

    LzPwmDeinit(m_pwm_bus_info[port].port);
    
    return IOT_SUCCESS;
}

unsigned int IoTPwmStart(unsigned int port, unsigned short duty, unsigned int freq)
{
#define DUTY_MIN        1
#define DUTY_MAX        99
#define SEC_TO_NSEC     (1000000000UL)
    unsigned int duty_ns;
    unsigned int cycle_ns;
    
    if (port >= EPWMDEV_MAX) {
        PRINT_ERR("port(%d) >= EPWMDEV_MAX(%d)\n", port, EPWMDEV_MAX);
        return IOT_FAILURE;
    }
    if ((duty < DUTY_MIN) || (DUTY_MAX < duty)) {
        PRINT_ERR("duty(%d) out of the range(%d ~ %d)\n", duty, DUTY_MIN, DUTY_MAX);
        return IOT_FAILURE;
    }
    if (freq == 0) {
        PRINT_ERR("freq(%d) is invalid!\n", freq);
        return IOT_FAILURE;
    }
    if (freq > SEC_TO_NSEC) {
        PRINT_ERR("freq(%d) > SEC_TO_NSEC(%d)\n", freq, SEC_TO_NSEC);
        return IOT_FAILURE;
    }

    cycle_ns = SEC_TO_NSEC / freq;
    duty_ns = cycle_ns * duty / 100;

    LzPwmStart(m_pwm_bus_info[port].port, duty_ns, cycle_ns);

    return IOT_SUCCESS;
}

unsigned int IoTPwmStop(unsigned int port)
{
    if (port >= EPWMDEV_MAX) {
        PRINT_ERR("port(%d) >= EPWMDEV_MAX(%d)\n", port, EPWMDEV_MAX);
        return IOT_FAILURE;
    }

    LzPwmStop(m_pwm_bus_info[port].port);

    return IOT_SUCCESS;
}
