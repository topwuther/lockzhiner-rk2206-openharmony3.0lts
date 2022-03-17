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
 *
 * @file gpio.h
 */
#ifndef LZ_HARDWARE_GPIO_H
#define LZ_HARDWARE_GPIO_H

typedef enum {
    GPIO0_PA0 = 0,
    GPIO0_PA1,
    GPIO0_PA2,
    GPIO0_PA3,
    GPIO0_PA4,
    GPIO0_PA5,
    GPIO0_PA6,
    GPIO0_PA7,

    GPIO0_PB0,
    GPIO0_PB1,
    GPIO0_PB2,
    GPIO0_PB3,
    GPIO0_PB4,
    GPIO0_PB5,
    GPIO0_PB6,
    GPIO0_PB7,

    GPIO0_PC0,
    GPIO0_PC1,
    GPIO0_PC2,
    GPIO0_PC3,
    GPIO0_PC4,
    GPIO0_PC5,
    GPIO0_PC6,
    GPIO0_PC7,

    GPIO0_PD0,
    GPIO0_PD1,
    GPIO0_PD2,
    GPIO0_PD3,
    GPIO0_PD4,
    GPIO0_PD5,
    GPIO0_PD6,
    GPIO0_PD7,

    GPIO1_PA0 = 32,
    GPIO1_PA1,
    GPIO1_PA2,
    GPIO1_PA3,
    GPIO1_PA4,
    GPIO1_PA5,
    GPIO1_PA6,
    GPIO1_PA7,

    GPIO1_PB0,
    GPIO1_PB1,
    GPIO1_PB2,
    GPIO1_PB3,
    GPIO1_PB4,
    GPIO1_PB5,
    GPIO1_PB6,
    GPIO1_PB7,

    GPIO1_PC0,
    GPIO1_PC1,
    GPIO1_PC2,
    GPIO1_PC3,
    GPIO1_PC4,
    GPIO1_PC5,
    GPIO1_PC6,
    GPIO1_PC7,

    GPIO1_PD0,
    GPIO1_PD1,
    GPIO1_PD2,
    GPIO1_PD3,
    GPIO1_PD4,
    GPIO1_PD5,
    GPIO1_PD6,
    GPIO1_PD7,

    GPIO2_PA0 = 64,
    GPIO2_PA1,
    GPIO2_PA2,
    GPIO2_PA3,
    GPIO2_PA4,
    GPIO2_PA5,
    GPIO2_PA6,
    GPIO2_PA7,

    GPIO2_PB0,
    GPIO2_PB1,
    GPIO2_PB2,
    GPIO2_PB3,
    GPIO2_PB4,
    GPIO2_PB5,
    GPIO2_PB6,
    GPIO2_PB7,

    GPIO2_PC0,
    GPIO2_PC1,
    GPIO2_PC2,
    GPIO2_PC3,
    GPIO2_PC4,
    GPIO2_PC5,
    GPIO2_PC6,
    GPIO2_PC7,

    GPIO2_PD0,
    GPIO2_PD1,
    GPIO2_PD2,
    GPIO2_PD3,
    GPIO2_PD4,
    GPIO2_PD5,
    GPIO2_PD6,
    GPIO2_PD7,

    GPIO3_PA0 = 96,
    GPIO3_PA1,
    GPIO3_PA2,
    GPIO3_PA3,
    GPIO3_PA4,
    GPIO3_PA5,
    GPIO3_PA6,
    GPIO3_PA7,

    GPIO3_PB0,
    GPIO3_PB1,
    GPIO3_PB2,
    GPIO3_PB3,
    GPIO3_PB4,
    GPIO3_PB5,
    GPIO3_PB6,
    GPIO3_PB7,

    GPIO3_PC0,
    GPIO3_PC1,
    GPIO3_PC2,
    GPIO3_PC3,
    GPIO3_PC4,
    GPIO3_PC5,
    GPIO3_PC6,
    GPIO3_PC7,

    GPIO3_PD0,
    GPIO3_PD1,
    GPIO3_PD2,
    GPIO3_PD3,
    GPIO3_PD4,
    GPIO3_PD5,
    GPIO3_PD6,
    GPIO3_PD7,

    GPIO4_PA0 = 128,
    GPIO4_PA1,
    GPIO4_PA2,
    GPIO4_PA3,
    GPIO4_PA4,
    GPIO4_PA5,
    GPIO4_PA6,
    GPIO4_PA7,

    GPIO4_PB0,
    GPIO4_PB1,
    GPIO4_PB2,
    GPIO4_PB3,
    GPIO4_PB4,
    GPIO4_PB5,
    GPIO4_PB6,
    GPIO4_PB7,

    GPIO4_PC0,
    GPIO4_PC1,
    GPIO4_PC2,
    GPIO4_PC3,
    GPIO4_PC4,
    GPIO4_PC5,
    GPIO4_PC6,
    GPIO4_PC7,

    GPIO4_PD0,
    GPIO4_PD1,
    GPIO4_PD2,
    GPIO4_PD3,
    GPIO4_PD4,
    GPIO4_PD5,
    GPIO4_PD6,
    GPIO4_PD7,

    INVALID_GPIO,
} GpioID;

/**
 * @brief Enumerates GPIO level values.
 */
typedef enum {
    /** Low GPIO level */
    LZGPIO_LEVEL_LOW = 0,
    /** High GPIO level */
    LZGPIO_LEVEL_HIGH,
    /** Keep the last value, no set */
    LZGPIO_LEVEL_KEEP,
} LzGpioValue;

/**
 * @brief Enumerates GPIO directions.
 */
typedef enum {
    /** Input */
    LZGPIO_DIR_IN = 0,
    /** Output */
    LZGPIO_DIR_OUT,
    /** Keep the last direction, no set */
    LZGPIO_DIR_KEEP,
} LzGpioDir;

/**
 * @brief Enumerates GPIO interrupt trigger types.
 */
typedef enum {
    LZGPIO_INT_NONE,
    /** Interrupt at rising edge */
    LZGPIO_INT_EDGE_RISING,
    /** Interrupt at falling edge */
    LZGPIO_INT_EDGE_FALLING,
    /** Interrupt at falling and rising edge */
    LZGPIO_INT_EDGE_BOTH,
    /** Interrupt at high level */
    LZGPIO_INT_LEVEL_HIGH,
    /** Interrupt at low level */
    LZGPIO_INT_LEVEL_LOW = 0x8,
} LzGpioIntType;

/**
 * @brief Indicates the GPIO interrupt callback.
 *
 */
typedef void (*GpioIsrFunc) (void *arg);

/**
 * @brief Initializes a GPIO device.
 *
 * @param id Indicates the GPIO pin number.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the GPIO device is initialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioInit(GpioID id);

/**
 * @brief Deinitializes a GPIO device.
 *
 * @param id Indicates the GPIO pin number.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the GPIO device is deinitialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioDeinit(GpioID id);

/**
 * @brief Sets the direction for a GPIO pin.
 *
 * @param id Indicates the GPIO pin number.
 * @param dir Indicates the GPIO input/output direction.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the direction is set;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioSetDir(GpioID id, LzGpioDir dir);

/**
 * @brief Obtains the direction for a GPIO pin.
 *
 * @param id Indicates the GPIO pin number.
 * @param dir Indicates the pointer to the GPIO input/output direction.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the direction is obtained;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioGetDir(GpioID id, LzGpioDir *dir);

/**
 * @brief Sets the level value for a GPIO pin.
 *
 * @param id Indicates the GPIO pin number.
 * @param val Indicates the level value.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the level value is set;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioSetVal(GpioID id, LzGpioValue val);

/**
 * @brief Obtains the level value of a GPIO pin.
 *
 * @param id Indicates the GPIO pin number.
 * @param val Indicates the pointer to the level value.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the level value is obtained;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioGetVal(GpioID id, LzGpioValue *val);

/**
 * @brief Enables the interrupt feature for a GPIO pin.
 *
 * This function can be used to set the interrupt type, interrupt polarity, and interrupt callback for a GPIO pin.
 *
 * @param id Indicates the GPIO pin number.
 * @param type Indicates the interrupt type.
 * @param func Indicates the interrupt callback function.
 * @param arg Indicates the pointer to the argument used in the interrupt callback function.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the interrupt feature is enabled;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioRegisterIsrFunc(GpioID id, LzGpioIntType type, GpioIsrFunc func, void *arg);

/**
 * @brief Disables the interrupt feature for a GPIO pin.
 *
 * @param id Indicates the GPIO pin number.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the interrupt feature is disabled;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioUnregisterIsrFunc(GpioID id);

/**
 * @brief Enable the interrupt for a GPIO pin.
 *
 * @param id Indicates the GPIO pin number.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the interrupt feature is masked;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioEnableIsr(GpioID id);


/**
 * @brief Disable the interrupt for a GPIO pin.
 *
 * @param id Indicates the GPIO pin number.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the interrupt feature is masked;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzGpioDisableIsr(GpioID id);

#endif
/** @} */
