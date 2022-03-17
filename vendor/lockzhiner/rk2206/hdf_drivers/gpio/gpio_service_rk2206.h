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
