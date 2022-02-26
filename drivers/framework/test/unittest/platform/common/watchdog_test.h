/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef WATCHDOG_TEST_H
#define WATCHDOG_TEST_H

#include "hdf_device_desc.h"
#include "hdf_platform.h"

enum WatchdogTestCmd {
    WATCHDOG_TEST_SET_GET_TIMEOUT = 0,
    WATCHDOG_TEST_START_STOP = 1,
    WATCHDOG_TEST_FEED = 2,
    WATCHDOG_TEST_RELIABILITY = 3,
    WATCHDOG_TEST_BARK = 4,
    WATCHDOG_TEST_MAX = 5,
};

struct WatchdogTester {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    int32_t (*doTest)(struct WatchdogTester *tester, int32_t cmd);
    uint16_t total;
    uint16_t fails;
    DevHandle handle;
};

static inline struct WatchdogTester *WatchdogTesterGet(void)
{
    return (struct WatchdogTester *)DevSvcManagerClntGetService("WATCHDOG_TEST");
}

#endif /* WATCHDOG_TEST_H */
