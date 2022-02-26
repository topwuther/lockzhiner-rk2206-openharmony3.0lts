/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_RTC_ENTRY_TEST_H
#define HDF_RTC_ENTRY_TEST_H

#include "hdf_main_test.h"

typedef enum {
    RTC_INIT,
    RTC_UNINIT,
    RTC_WR_TIME,
    RTC_WR_MAX_TIME,
    RTC_WR_MIN_TIME,
    RTC_WR_ALARM_TIME,
    RTC_WR_ALARM_MAX_TIME,
    RTC_WR_ALARM_MIN_TIME,
    RTC_ALARM_ENABLE,
    RTC_ALARM_IRQ,
    RTC_REGISTER_CALLBACK,
    RTC_REGISTER_CALLBACK_NULL,
    RTC_WR_FREQ,
    RTC_WR_MAX_FREQ,
    RTC_WR_MIN_FREQ,
    RTC_WR_USER_REG,
    RTC_WR_USER_REG_MAX_INDEX,
    RTC_WR_RELIABILITY,
    RTC_FUNCTION_TEST,
} HdfRtcTestCaseCmd;

int32_t HdfRtcEntry(HdfTestMsg *msg);
#endif /* HDF_RTC_ENTRY_TEST_H */
