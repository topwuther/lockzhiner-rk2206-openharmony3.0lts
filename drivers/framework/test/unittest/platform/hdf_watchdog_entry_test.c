/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_watchdog_entry_test.h"
#include "hdf_log.h"
#include "watchdog_if.h"
#include "watchdog_test.h"

#define HDF_LOG_TAG hdf_watchdog_entry_test

int32_t HdfWatchdogTestEntry(HdfTestMsg *msg)
{
    struct WatchdogTester *tester = NULL;

    if (msg == NULL) {
        return HDF_FAILURE;
    }

    tester = WatchdogTesterGet();
    if (tester == NULL) {
        HDF_LOGE("%s: tester is NULL!\n", __func__);
        return HDF_FAILURE;
    }

    msg->result = tester->doTest(tester, msg->subCmd);

    return HDF_SUCCESS;
}
