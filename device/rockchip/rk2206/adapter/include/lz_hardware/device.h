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
 * @file device.h
 *
 */
#ifndef LZ_HARDWARE_DEVICE_H
#define LZ_HARDWARE_DEVICE_H

#include "lz_hardware/pinctrl.h"

typedef struct _DevIo {
    /** Interrupt gpio for this device; INVALID_GPIO if not used */
    Pinctrl isr;
    /** Reset gpio for this device; INVALID_GPIO if not used */
    Pinctrl rst;
    /** Controller gpio for this device; INVALID_GPIO if not used */ 
    Pinctrl ctrl1;
    /** Controller gpio for this device; INVALID_GPIO if not used */ 
    Pinctrl ctrl2;
} DevIo;

typedef struct _SpiBusIo {
    Pinctrl cs;
    Pinctrl clk;
    Pinctrl mosi;
    Pinctrl miso;
    FuncID id;
    FuncMode mode;
} SpiBusIo;

typedef struct _I2cBusIo {
    Pinctrl scl;
    Pinctrl sda;
    FuncID id;
    FuncMode mode;
} I2cBusIo;

typedef struct _UartBusIo {
    Pinctrl tx;
    Pinctrl rx;
    Pinctrl ctsn;
    Pinctrl rtsn;
    FuncID id;
    FuncMode mode;
} UartBusIo;

typedef struct _PwmBusIo {
    Pinctrl pwm;
    FuncID id;
    FuncMode mode;
} PwmBusIo;

unsigned int DevIoInit(DevIo io);
unsigned int SpiIoInit(SpiBusIo io);
unsigned int I2cIoInit(I2cBusIo io);
unsigned int UartIoInit(UartBusIo io);
unsigned int PwmIoInit(PwmBusIo io);

#endif
/** @} */
