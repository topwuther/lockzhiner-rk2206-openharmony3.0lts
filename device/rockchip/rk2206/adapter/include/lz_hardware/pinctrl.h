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

/**
 * @addtogroup Lockzhiner
 */

/**
 * @file pinctrl.h
 *
 * @brief pinctrl.
 */

#ifndef LZ_HARDWARE_PINCTRL_H
#define LZ_HARDWARE_PINCTRL_H

#include "lz_hardware/gpio.h"
#include "lz_hardware/errno.h"

typedef enum {
    MUX_FUNC0 = 0,
    MUX_FUNC1,
    MUX_FUNC2,
    MUX_FUNC3,
    MUX_FUNC4,
    MUX_FUNC5,
    MUX_FUNC6,
    MUX_FUNC7,
    /** keep the last iomux function, no set */
    MUX_KEEP,
} MuxFunc;
#define MUX_GPIO MUX_FUNC0

typedef enum {
    /** high-z */
    PULL_NONE,
    /** pull up */
    PULL_UP,
    /** pull down */
    PULL_DOWN,
    /** keep the last pull type, no set */
    PULL_KEEP,
} PullType;

typedef enum {
    DRIVE_LEVEL0,
    DRIVE_LEVEL1,
    DRIVE_LEVEL2,
    DRIVE_LEVEL3,
    DRIVE_LEVEL4,
    DRIVE_LEVEL5,
    DRIVE_LEVEL6,
    DRIVE_LEVEL7,
    /** keep the last drive level, no set */
    DRIVE_KEEP,
} DriveLevel;

typedef enum {
    FUNC_ID_CIF,
    FUNC_ID_EMMC,
    FUNC_ID_FLASH,
    FUNC_ID_FSPI,
    FUNC_ID_LCDC,
    FUNC_ID_MIPICSI,
    FUNC_ID_RGMII,
    FUNC_ID_GMAC0,
    FUNC_ID_GMAC1,
    FUNC_ID_SDIO,
    FUNC_ID_SDMMC0,
    FUNC_ID_CAN0,
    FUNC_ID_CAN1,
    FUNC_ID_CAN2,
    FUNC_ID_CAN3,
    FUNC_ID_CAN4,
    FUNC_ID_CAN5,
    FUNC_ID_I2C0,
    FUNC_ID_I2C1,
    FUNC_ID_I2C2,
    FUNC_ID_I2C3,
    FUNC_ID_I2C4,
    FUNC_ID_I2C5,
    FUNC_ID_I2S0,
    FUNC_ID_I2S1,
    FUNC_ID_I2S2,
    FUNC_ID_PWM0,
    FUNC_ID_PWM1,
    FUNC_ID_PWM2,
    FUNC_ID_PWM3,
    FUNC_ID_PWM4,
    FUNC_ID_PWM5,
    FUNC_ID_PWM6,
    FUNC_ID_PWM7,
    FUNC_ID_PWM8,
    FUNC_ID_PWM9,
    FUNC_ID_PWM10,
    FUNC_ID_PWM11,
    FUNC_ID_SPI0,
    FUNC_ID_SPI1,
    FUNC_ID_SPI2,
    FUNC_ID_SPI3,
    FUNC_ID_SPI4,
    FUNC_ID_SPI5,
    FUNC_ID_UART0,
    FUNC_ID_UART1,
    FUNC_ID_UART2,
    FUNC_ID_UART3,
    FUNC_ID_UART4,
    FUNC_ID_UART5,
    FUNC_ID_UART6,
    FUNC_ID_UART7,
    FUNC_ID_UART8,
    FUNC_ID_UART9,
    FUNC_ID_UART10,
    FUNC_ID_UART11,
    FUNC_ID_NONE,
} FuncID;

typedef enum {
    FUNC_MODE_M0,
    FUNC_MODE_M1,
    FUNC_MODE_M2,
    FUNC_MODE_NONE,
} FuncMode;

typedef struct _Pinctrl {
    GpioID gpio;
    MuxFunc func;
    PullType type;
    DriveLevel drv;
    LzGpioDir dir;
    LzGpioValue val;
} Pinctrl;

/**
 * @brief pinctrl: set pinctrl.
 *
 * @param gpio Indicates gpio ID.
 * @param func Indicates iomux function.
 * @param type Indicates pull type.
 * @param drv Indicates drive level.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the pinctrl is set;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int PinctrlSet(GpioID gpio, MuxFunc func, PullType type, DriveLevel drv);

/**
 * @brief pinctrl: set pin function.
 *
 * @param id Indicates pin function id.
 * @param mode Indicates pin function mode.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the pin function is selected successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int PinfuncSel(FuncID id, FuncMode mode);

static inline unsigned int PinctrlInit(Pinctrl pin)
{
    if (pin.gpio == INVALID_GPIO)
        return LZ_HARDWARE_SUCCESS;

    if (LzGpioInit(pin.gpio) != LZ_HARDWARE_SUCCESS)
        return LZ_HARDWARE_FAILURE;

    PinctrlSet(pin.gpio, pin.func, pin.type, pin.drv);
    LzGpioSetDir(pin.gpio, pin.dir);
    LzGpioSetVal(pin.gpio, pin.val);

    return LZ_HARDWARE_SUCCESS;
}
#endif
/** @} */
