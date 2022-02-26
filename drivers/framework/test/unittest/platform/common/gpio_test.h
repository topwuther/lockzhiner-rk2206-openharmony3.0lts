/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef GPIO_TEST_H
#define GPIO_TEST_H

#include "hdf_device_desc.h"

enum GpioTestCmd {
    GPIO_TEST_SET_GET_DIR = 0,
    GPIO_TEST_WRITE_READ = 1,
    GPIO_TEST_IRQ_LEVEL = 2,
    GPIO_TEST_IRQ_EDGE = 3,
    GPIO_TEST_IRQ_THREAD = 4,
    GPIO_TEST_RELIABILITY = 5,
    GPIO_TEST_MAX = 6,
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

static inline struct GpioTester *GpioTesterGet(void)
{
    return (struct GpioTester *)DevSvcManagerClntGetService("GPIO_TEST");
}

#endif /* GPIO_TEST_H */
