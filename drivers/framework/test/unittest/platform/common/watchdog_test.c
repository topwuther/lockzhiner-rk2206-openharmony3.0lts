/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "watchdog_test.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "osal_time.h"
#include "watchdog_if.h"

#define HDF_LOG_TAG watchdog_test

#define WATCHDOG_TEST_CASE_NUM    5
#define WATCHDOG_TEST_TIMEOUT     2
#define WATCHDOG_TEST_FEED_TIME   6

static int32_t WatchdogTestSetUp(struct WatchdogTester *tester)
{
    if (tester == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (tester->handle == NULL) {
        tester->handle = WatchdogOpen(0);
    }
    if (tester->handle == NULL) {
        return HDF_ERR_DEVICE_BUSY;
    }
    tester->total = WATCHDOG_TEST_CASE_NUM;
    tester->fails = 0;

    return HDF_SUCCESS;
}

static void WatchdogTestTearDown(struct WatchdogTester *tester)
{
    int ret;

    if (tester == NULL || tester->handle == NULL) {
        return;
    }

    ret = WatchdogStop(tester->handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: stop fail! ret:%d", __func__, ret);
        return;
    }

    WatchdogClose(tester->handle);
    tester->handle = NULL;
}

static int32_t TestCaseWatchdogSetGetTimeout(struct WatchdogTester *tester)
{
    int32_t ret;
    uint32_t timeoutGet = 0;
    const uint32_t timeoutSet = WATCHDOG_TEST_TIMEOUT;

    ret = WatchdogSetTimeout(tester->handle, timeoutSet);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: set timeout fail! ret:%d", __func__, ret);
        return ret;
    }
    ret = WatchdogGetTimeout(tester->handle, &timeoutGet);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get timeout fail! ret:%d", __func__, ret);
        return ret;
    }
    if (timeoutSet != timeoutGet) {
        HDF_LOGE("%s: set:%u, but get:%u", __func__, timeoutSet, timeoutGet);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t TestCaseWatchdogStartStop(struct WatchdogTester *tester)
{
    int32_t ret;
    int32_t status;

    ret = WatchdogStart(tester->handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: satrt fail! ret:%d", __func__, ret);
        return ret;
    }
    status = WATCHDOG_STOP;
    ret = WatchdogGetStatus(tester->handle, &status);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get status fail! ret:%d", __func__, ret);
        return ret;
    }
    if (status != WATCHDOG_START) {
        HDF_LOGE("%s: status is:%d after start", __func__, status);
        return HDF_FAILURE;
    }

    ret = WatchdogStop(tester->handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: stop fail! ret:%d", __func__, ret);
        return ret;
    }
    status = WATCHDOG_START;
    ret = WatchdogGetStatus(tester->handle, &status);
    if (status != WATCHDOG_STOP) {
        HDF_LOGE("%s: status is:%d after stop", __func__, status);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t TestCaseWatchdogFeed(struct WatchdogTester *tester)
{
    int32_t ret;
    int32_t i;

    ret = WatchdogStart(tester->handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: satrt fail! ret:%d", __func__, ret);
        return ret;
    }

    for (i = 0; i < WATCHDOG_TEST_FEED_TIME; i++) {
        HDF_LOGE("%s: feeding watchdog %d times... ", __func__, i);
        ret = WatchdogFeed(tester->handle);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: feed dog fail! ret:%d", __func__, ret);
            return ret;
        }
        OsalSleep(1);
    }

    HDF_LOGE("%s: no reset ... feeding test OK!!!", __func__);
    return HDF_SUCCESS;
}

static int32_t TestCaseWatchdogBark(struct WatchdogTester *tester)
{
#ifdef WATCHDOG_TEST_BARK_RESET
    int32_t ret;
    int32_t i;

    ret = WatchdogStart(tester->handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: satrt fail! ret:%d", __func__, ret);
        return ret;
    }

    for (i = 0; i < WATCHDOG_TEST_FEED_TIME; i++) {
        HDF_LOGE("%s: watiting dog buck %d times... ", __func__, i);
        OsalSleep(1);
    }

    HDF_LOGE("%s: dog has't buck!!! ", __func__, i);
    return HDF_FAILURE;
#else
    (void)tester;
    return HDF_SUCCESS;
#endif
}

static int32_t TestCaseWatchdogReliability(struct WatchdogTester *tester)
{
    int32_t status;
    uint32_t timeout;

    HDF_LOGE("%s: test dfr for WatchdogGetStatus ...", __func__);
    /* invalid device handle */
    (void)WatchdogGetStatus(NULL, &status);
    /* invalid status pointer */
    (void)WatchdogGetStatus(tester->handle, NULL);

    HDF_LOGE("%s: test dfr for WatchdogStart&Stop ...", __func__);
    /* invalid device handle */
    (void)WatchdogStart(NULL);
    /* invalid device handle */
    (void)WatchdogStop(NULL);

    HDF_LOGE("%s: test dfr for WatchdogSet&GetTimeout ...", __func__);
    /* invalid device handle */
    (void)WatchdogSetTimeout(NULL, WATCHDOG_TEST_TIMEOUT);
    /* invalid device handle */
    (void)WatchdogGetTimeout(NULL, &timeout);
    /* invalid timeout pointer */
    (void)WatchdogGetTimeout(tester->handle, NULL);

    HDF_LOGE("%s: test dfr for WatchdogFeed ...", __func__);
    /* invalid device handle */
    (void)WatchdogFeed(NULL);

    return HDF_SUCCESS;
}

static int32_t WatchdogTestByCmd(struct WatchdogTester *tester, int32_t cmd)
{
    int32_t i;

    if (cmd == WATCHDOG_TEST_SET_GET_TIMEOUT) {
        return TestCaseWatchdogSetGetTimeout(tester);
    } else if (cmd == WATCHDOG_TEST_START_STOP) {
        return TestCaseWatchdogStartStop(tester);
    } else if (cmd == WATCHDOG_TEST_FEED) {
        return TestCaseWatchdogFeed(tester);
    } else if (cmd == WATCHDOG_TEST_BARK) {
        return TestCaseWatchdogBark(tester);
    } else if (cmd == WATCHDOG_TEST_RELIABILITY) {
        return TestCaseWatchdogReliability(tester);
    }

    for (i = 0; i < WATCHDOG_TEST_MAX; i++) {
        if (WatchdogTestByCmd(tester, i) != HDF_SUCCESS) {
            tester->fails++;
        }
    }
    HDF_LOGE("%s: **********PASS:%u  FAIL:%u**************\n\n",
        __func__, tester->total - tester->fails, tester->fails);
    return (tester->fails > 0) ? HDF_FAILURE : HDF_SUCCESS;
}

static int32_t WatchdogDoTest(struct WatchdogTester *tester, int32_t cmd)
{
    int32_t ret;

    if (tester == NULL) {
        HDF_LOGE("%s: tester is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = WatchdogTestSetUp(tester);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: setup fail!", __func__);
        return ret;
    }

    ret = WatchdogTestByCmd(tester, cmd);

    WatchdogTestTearDown(tester);
    return ret;
}

static int32_t WatchdogTestBind(struct HdfDeviceObject *device)
{
    static struct WatchdogTester tester;

    if (device == NULL) {
        HDF_LOGE("%s: device is null!", __func__);
        return HDF_ERR_IO;
    }

    tester.doTest = WatchdogDoTest;
    device->service = &tester.service;
    return HDF_SUCCESS;
}

static int32_t WatchdogTestInit(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static void WatchdogTestRelease(struct HdfDeviceObject *device)
{
    if (device != NULL) {
        device->service = NULL;
    }
    return;
}

struct HdfDriverEntry g_watchdogTestEntry = {
    .moduleVersion = 1,
    .Bind = WatchdogTestBind,
    .Init = WatchdogTestInit,
    .Release = WatchdogTestRelease,
    .moduleName = "PLATFORM_WATCHDOG_TEST",
};
HDF_INIT(g_watchdogTestEntry);
