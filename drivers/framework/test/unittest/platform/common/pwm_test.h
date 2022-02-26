/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PWM_TEST_H
#define PWM_TEST_H

#include "hdf_device_desc.h"
#include "hdf_platform.h"
#include "pwm_if.h"

enum PwmTestCmd {
    PWM_SET_PERIOD_TEST = 0,
    PWM_SET_DUTY_TEST,
    PWM_SET_POLARITY_TEST,
    PWM_ENABLE_TEST,
    PWM_DISABLE_TEST,
    PWM_SET_CONFIG_TEST,
    PWM_GET_CONFIG_TEST,
    PWM_RELIABILITY_TEST,
    PWM_TEST_ALL,
};

struct PwmTest {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    int32_t (*TestEntry)(struct PwmTest *test, int32_t cmd);
    uint32_t num;
    struct PwmConfig cfg;
    DevHandle handle;
};

static inline struct PwmTest *GetPwmTest(void)
{
    return (struct PwmTest *)DevSvcManagerClntGetService("PWM_TEST");
}

#endif /* PWM_TEST_H */
