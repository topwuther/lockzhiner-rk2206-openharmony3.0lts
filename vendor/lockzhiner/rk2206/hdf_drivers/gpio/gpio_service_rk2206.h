/*
 * Copyright (c) 2022 Lockzhiner Electronic Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HDF_GPIO_DRIVER_RK2206_H
#define HDF_GPIO_DRIVER_RK2206_H

#include "hdf_device_desc.h"
#include "device_resource_if.h"

#define GPIO_TEST_IRQ_TIMEOUT 1000
#define GPIO_TEST_IRQ_DELAY   200

struct GpioServiceRk2206
{
    struct IDeviceIoService service;
    const char *(*getData)(void);
    int32_t (*setData)(const char *data);
};

enum GpioTestCmd {
    GPIO_TEST_SET_GET_DIR = 0,
    GPIO_TEST_WRITE_READ = 1,
    //GPIO_TEST_IRQ_LEVEL = 2,
    //GPIO_TEST_IRQ_EDGE = 3,
    //GPIO_TEST_IRQ_THREAD = 4,
    //GPIO_TEST_RELIABILITY = 5,
    GPIO_TEST_MAX,
};

struct GpioTester {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    int32_t (*doTest)(struct GpioTester *tester, int32_t cmd);
    uint16_t gpio;
    uint16_t gpioIrq;
    uint16_t oldDir;
    uint16_t oldVal;
    uint16_t irqCnt;
    uint16_t total;
    uint16_t fails;
    uint32_t irqTimeout;
};

#endif
