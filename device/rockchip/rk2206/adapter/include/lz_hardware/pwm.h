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
 * @file pwm.h
 */

#ifndef LZ_HARDWARE_PWM_H
#define LZ_HARDWARE_PWM_H
typedef void (*PwmIsrFunc) (void *arg);

typedef enum _LZ_HARDWARE_PWM_CHN
{
    LZ_HARDWARE_PWM_CHN0 = 0,
    LZ_HARDWARE_PWM_CHN1,
    LZ_HARDWARE_PWM_CHN2,
    LZ_HARDWARE_PWM_CHN3,
    LZ_HARDWARE_PWM_MAX_CHANNEL,
} LZ_HARDWARE_PWM_CHN;

#define LZ_HARDWARE_PWM_POLARITY_INVERTED  (1 << 0)

typedef enum _LZ_HARDWARE_PWM_CMD
{
    LZ_HARDWARE_PWM_CMD_ENABLE = (128 + 0),
    LZ_HARDWARE_PWM_CMD_DISABLE,
    LZ_HARDWARE_PWM_CMD_SET,
    LZ_HARDWARE_PWM_CMD_SET_ONESHOT,
} LZ_HARDWARE_PWM_CMD;

typedef enum _LZ_HARDWARE_PWM_MODE
{
    PWM_ONESHOT = 0,
    PWM_CONTINUOUS,
    PWM_CAPTURE,
} LzPwmMode;

typedef struct _LZ_HARDWARE_PWM_CONFIG
{
    uint8_t channel; /* 0-3 */
    uint32_t period; /* unit:ns 1ns~4.29s:1Ghz~0.23hz */
    uint32_t pulse;  /* unit:ns (pulse<=period) */
    uint8_t mode;    /* LzPwmMode */
    bool polarity;   /* invert or not */
} PwmConfig;

typedef struct _LZ_HARDWARE_PWM_ONESHOT_CONFIG
{
    PwmConfig config;
    uint32_t count;
} LZ_HARDWARE_PWM_ONESHOT_CONFIG;

typedef struct _LZ_HARDWARE_PWM_CAPTURE_CONFIG
{
    uint32_t period;
    bool pol;
    bool active;
} LZ_HARDWARE_PWM_CAPTURE_CONFIG;

typedef struct _LZ_HARDWARE_PWM_ENABLED_CONFIG
{
    uint8_t channel; /* 0-3 */
    LzPwmMode mode;
} LZ_HARDWARE_PWM_ENABLED_CONFIG;
/**
 * @brief Initializes a PWM device.
 *
 * @param port Indicates the port number of the PWM device.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the PWM device is initialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzPwmInit(unsigned int port);

/**
 * @brief Deinitializes a PWM device.
 *
 * @param port Indicates the port number of the PWM device.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the PWM device is deinitialized;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzPwmDeinit(unsigned int port);

/**
 * @brief Starts PWM signal output from a specified port based on the given output frequency and duty cycle.
 *
 *
 *
 * @param port Indicates the port number of the PWM device.
 * @param duty Indicates the duty cycle for PWM signal output. The value ranges from 1 to 99.
 * @param cycle Indicates the cycle for PWM signal output.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the PWM signal output is started;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzPwmStart(unsigned int port, unsigned int duty, unsigned int cycle);

/**
 * @brief Stops PWM signal output from a specified port.
 *
 * @param port Indicates the port number of the PWM device.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the PWM signal output is stopped;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzPwmStop(unsigned int port);

/**
 * @brief Enables the interrupt feature for a PWM device.
 *
 * This function can be used to set the interrupt type, interrupt polarity, and interrupt callback for a PWM device.
 *
 * @param port Indicates the port number of the PWM device.
 * @param func Indicates the interrupt callback function.
 * @param arg Indicates the pointer to the argument used in the interrupt callback function.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the interrupt feature is enabled;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzPwmRegisterIsrFunc(unsigned int port, PwmIsrFunc func, void *arg);

/**
 * @brief Disables the interrupt feature for a PWM device.
 *
 * @param port Indicates the port number of the PWM device.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the interrupt feature is disabled;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzPwmUnregisterIsrFunc(unsigned int port);

/**
 * @brief Enable the interrupt for a PWM device.
 *
 * @param port Indicates the port number of the PWM device.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the interrupt feature is masked;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzPwmEnableIsr(unsigned int port);

/**
 * @brief Disable the interrupt for a PWM device.
 *
 * @param port Indicates the port number of the PWM device.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the interrupt feature is masked;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzPwmDisableIsr(unsigned int port);

/**
 * @brief SET the MODE for a PWM device.
 *
 * @param port Indicates the port number of the PWM device.
 * @param Working mode of the PWM device.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the interrupt feature is masked;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */
unsigned int LzPwmSetMode(unsigned int port, LzPwmMode mode);

#endif
/** @} */
