/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_rtc_entry_test.h"
#include "hdf_log.h"
#include "osal_time.h"
#include "rtc_base.h"
#include "rtc_if.h"

#define HDF_LOG_TAG hdf_rtc_entry_test

#define RTC_TEST_TIME           60
#define RTC_TEST_TIME_MAX_YEAR  2222
#define RTC_TEST_TIME_YEAR      2020
#define RTC_TEST_TIME_MONTH     8
#define RTC_TEST_TIME_DAY       8
#define RTC_TEST_TIME_HOUR      8
#define RTC_TEST_TIME_MIN       8
#define RTC_TEST_TIME_SECOND    8
#define RTC_TEST_FREQ           32768
#define RTC_TEST_USER_VALUE     0x8
#define RTC_TEST_USER_MAX_INDEX 8
#define RTC_TEST_WAIT_TIME_S    3
#define RTC_TEST_WR_WAIT_MS     30

DevHandle g_rtcHandle = NULL;

#if defined(LOSCFG_DRIVERS_HDF_PLATFORM_RTC)
static int8_t g_rtcIrqCallback = HDF_FAILURE;

int32_t RtcAlarmACallback(enum RtcAlarmIndex alarmIndex)
{
    if (alarmIndex == RTC_ALARM_INDEX_A) {
        HDF_LOGE("RtcAlarmACallback: alarm a callback success");
        g_rtcIrqCallback = HDF_SUCCESS;
    } else {
        g_rtcIrqCallback = HDF_FAILURE;
    }
    return 0;
}
#endif

static int32_t RtcTestInit(void)
{
    g_rtcHandle = RtcOpen();
    if (g_rtcHandle == NULL) {
        HDF_LOGE("RtcTestInit: g_rtcHandle NULL");
        return -1;
    }
    return 0;
}

static int32_t RtcTestUniInit(void)
{
    if (g_rtcHandle != NULL) {
        RtcClose(g_rtcHandle);
        g_rtcHandle = NULL;
    }

    return 0;
}

static int32_t IsSameRtcTestTime(const struct RtcTime *readTime, const struct RtcTime *writeTime)
{
    if ((readTime->year != writeTime->year) || (readTime->month != writeTime->month) ||
        (readTime->day != writeTime->day) || (readTime->weekday != writeTime->weekday) ||
        (readTime->hour != writeTime->hour) || (readTime->minute != writeTime->minute) ||
        (readTime->second < writeTime->second)) {
        return -1;
    }
    return 0;
}

static int32_t RtcReadWriteTimeTest(struct RtcTime *writeTime)
{
    int32_t ret;
    struct RtcTime readTime = {0};

    if (g_rtcHandle == NULL) {
        HDF_LOGE("RtcReadWriteTimeTest g_rtcHandle null");
        return -1;
    }
    ret = RtcWriteTime(g_rtcHandle, writeTime);
    if (ret != 0) {
        HDF_LOGE("RtcReadWriteTimeTest write failed");
        return -1;
    }

    OsalMSleep(RTC_TEST_WR_WAIT_MS);
    ret = RtcReadTime(g_rtcHandle, &readTime);
    if (ret != 0) {
        HDF_LOGE("RtcReadWriteTimeTest read failed");
        return -1;
    }

    if (IsSameRtcTestTime(&readTime, writeTime) != 0) {
        HDF_LOGE("RtcReadWriteTimeTest: different time");
        return -1;
    }
    return 0;
}

static int32_t RtcReadWriteAlarmTimeTest(struct RtcTime *writeTime)
{
    int32_t ret;
    struct RtcTime readTime = {0};

    if (g_rtcHandle == NULL) {
        return -1;
    }

    ret = RtcWriteAlarm(g_rtcHandle, RTC_ALARM_INDEX_A, writeTime);
    if (ret != 0) {
        HDF_LOGE("RtcReadWriteAlarmTimeTest write failed");
        return -1;
    }

    OsalMSleep(RTC_TEST_WR_WAIT_MS);
    ret = RtcReadAlarm(g_rtcHandle, RTC_ALARM_INDEX_A, &readTime);
    if (ret != 0) {
        HDF_LOGE("RtcReadWriteAlarmTimeTest read failed");
        return -1;
    }

    if (IsSameRtcTestTime(&readTime, writeTime) != 0) {
        return -1;
    }
    return 0;
}

static int32_t RtcReadWriteTime(void)
{
    int32_t ret;
    uint8_t weekday;
    struct RtcTime tm;

    /* 2020-08-08 Saturday 09:08:08 .000 */
    tm.year = RTC_TEST_TIME_YEAR;
    tm.month = RTC_TEST_TIME_MONTH;
    tm.day = RTC_TEST_TIME_DAY;
    tm.hour = RTC_TEST_TIME_HOUR;
    tm.minute = RTC_TEST_TIME_MIN;
    tm.second = RTC_TEST_TIME_SECOND;
    tm.millisecond = 0;
    weekday = RtcGetWeekDay(&tm);
    tm.weekday = weekday;

    ret = RtcReadWriteTimeTest(&tm);
    if (ret != 0) {
        HDF_LOGE("RtcReadWriteTime failed");
        return -1;
    }

    return 0;
}

static int32_t RtcReadWriteMaxTime(void)
{
    int32_t ret;
    uint8_t weekday;
    struct RtcTime tm;

    tm.year = RTC_TEST_TIME_MAX_YEAR;
    tm.month = RTC_MAX_MONTH + 1;
    tm.day = RTC_GREAT_MONTH_DAY + 1;
    tm.hour = RTC_DAY_HOURS;
    tm.minute = RTC_MAX_MINUTE;
    tm.second = RTC_MAX_SECOND;
    tm.millisecond = RTC_MAX_MS;
    weekday = RtcGetWeekDay(&tm);
    tm.weekday = weekday;
    ret = RtcReadWriteTimeTest(&tm);
    if (ret == 0) {
        HDF_LOGE("RtcReadWriteMaxTime failed");
        return -1;
    }

    return 0;
}

static int32_t RtcReadWriteMinTime(void)
{
    int32_t ret;
    uint8_t weekday;
    struct RtcTime tm;

    tm.year = RTC_BEGIN_YEAR - 1;
    tm.month = 0;
    tm.day = 0;
    tm.hour = 0;
    tm.minute = 0;
    tm.second = 0;
    tm.millisecond = 0;
    weekday = RtcGetWeekDay(&tm);
    tm.weekday = weekday;
    ret = RtcReadWriteTimeTest(&tm);
    if (ret == 0) {
        HDF_LOGE("RtcReadWriteMinTime failed");
        return -1;
    }

    return 0;
}

static int32_t RtcReadWriteAlarmTime(void)
{
    int32_t ret;
    uint8_t weekday;
    struct RtcTime tm;

    /* 2020-08-08 Saturday 09:08:08 .000 */
    tm.year = RTC_TEST_TIME_YEAR;
    tm.month = RTC_TEST_TIME_MONTH;
    tm.day = RTC_TEST_TIME_DAY;
    tm.hour = RTC_TEST_TIME_HOUR + RTC_UNIT_DIFF;
    tm.minute = RTC_TEST_TIME_MIN;
    tm.second = RTC_TEST_TIME_SECOND;
    tm.millisecond = 0;
    weekday = RtcGetWeekDay(&tm);
    tm.weekday = weekday;
    ret = RtcReadWriteAlarmTimeTest(&tm);
    if (ret != 0) {
        HDF_LOGE("RtcReadWriteAlarmTime read failed");
        return -1;
    }

    return 0;
}

static int32_t RtcReadWriteMaxAlarmTime(void)
{
    int32_t ret;
    uint8_t weekday;
    struct RtcTime tm;

    tm.year = RTC_TEST_TIME_MAX_YEAR;
    tm.month = RTC_MAX_MONTH + 1;
    tm.day = RTC_GREAT_MONTH_DAY + 1;
    tm.hour = RTC_DAY_HOURS;
    tm.minute = RTC_MAX_MINUTE;
    tm.second = RTC_MAX_SECOND;
    tm.millisecond = RTC_MAX_MS;
    weekday = RtcGetWeekDay(&tm);
    tm.weekday = weekday;
    ret = RtcReadWriteAlarmTimeTest(&tm);
    if (ret == 0) {
        HDF_LOGE("RtcReadWriteMaxAlarmTime read failed");
        return -1;
    }

    return 0;
}

static int32_t RtcReadWriteMinAlarmTime(void)
{
    int32_t ret;
    uint8_t weekday;
    struct RtcTime time;

    time.year = RTC_BEGIN_YEAR - 1;
    time.month = 0;
    time.day = 0;
    time.hour = 0;
    time.minute = 0;
    time.second = 0;
    time.millisecond = 0;
    weekday = RtcGetWeekDay(&time);
    time.weekday = weekday;
    ret = RtcReadWriteAlarmTimeTest(&time);
    if (ret == 0) {
        HDF_LOGE("RtcReadWriteMinAlarmTime read failed");
        return -1;
    }

    return 0;
}

#if defined(CONFIG_DRIVERS_HDF_PLATFORM_RTC)
static int32_t RtcAlarmEnable(void)
{
    int32_t ret;
    uint8_t weekday;
    struct RtcTime tm;

    /* 2020-08-08 Saturday 08:09:08 .000 */
    tm.year = RTC_TEST_TIME_YEAR;
    tm.month = RTC_TEST_TIME_MONTH;
    tm.day = RTC_TEST_TIME_DAY;
    tm.hour = RTC_TEST_TIME_HOUR;
    tm.minute = RTC_TEST_TIME_MIN + RTC_UNIT_DIFF;
    tm.second = RTC_TEST_TIME_SECOND;
    tm.millisecond = 0;
    weekday = RtcGetWeekDay(&tm);
    tm.weekday = weekday;
    ret = RtcReadWriteAlarmTimeTest(&tm);
    if (ret != 0) {
        HDF_LOGE("RtcReadWriteAlarmTime read failed");
        return -1;
    }
    ret = RtcAlarmInterruptEnable(g_rtcHandle, RTC_ALARM_INDEX_A, 1);
    if (ret != 0) {
        HDF_LOGE("RtcAlarmInterruptEnable failed");
        return -1;
    }
    return 0;
}
#endif

#if defined(LOSCFG_DRIVERS_HDF_PLATFORM_RTC)
static int32_t RtcAlarmIrqAttachConfig(void)
{
    int32_t ret;
    const uint32_t freq = RTC_TEST_FREQ;

    if (g_rtcHandle == NULL) {
        return -1;
    }

    ret = RtcRegisterAlarmCallback(g_rtcHandle, RTC_ALARM_INDEX_A, RtcAlarmACallback);
    if (ret != 0) {
        HDF_LOGE("RtcRegisterAlarmCallback failed");
        return -1;
    }
    ret = RtcSetFreq(g_rtcHandle, freq);
    if (ret != 0) {
        HDF_LOGE("RtcSetFreq failed");
        return -1;
    }
    ret = RtcAlarmInterruptEnable(g_rtcHandle, RTC_ALARM_INDEX_A, 1);
    if (ret != 0) {
        HDF_LOGE("RtcAlarmInterruptEnable failed");
        return -1;
    }
    return 0;
}

static int32_t RtcAlarmIrq(void)
{
    int32_t ret;
    uint8_t weekday;
    struct RtcTime time;

    if (g_rtcHandle == NULL) {
        return -1;
    }

    /* set time 2020-08-08 Saturday 08:08:08 .000 */
    time.year = RTC_TEST_TIME_YEAR;
    time.month = RTC_TEST_TIME_MONTH;
    time.day = RTC_TEST_TIME_DAY;
    time.hour = RTC_TEST_TIME_HOUR;
    time.minute = RTC_TEST_TIME_MIN;
    time.second = RTC_TEST_TIME_SECOND;
    time.millisecond = 0;
    weekday = RtcGetWeekDay(&time);
    time.weekday = weekday;
    ret = RtcAlarmIrqAttachConfig();
    if (ret != 0) {
        HDF_LOGE("RtcWriteTime failed");
        return -1;
    }
    ret = RtcWriteTime(g_rtcHandle, &time);
    if (ret != 0) {
        HDF_LOGE("RtcWriteTime failed");
        return -1;
    }
    /* set alarm time 2020-08-08 Saturday 08:08:09 .000 */
    time.second = RTC_TEST_TIME_SECOND + 1;
    ret = RtcWriteAlarm(g_rtcHandle, RTC_ALARM_INDEX_A, &time);
    if (ret != 0) {
        HDF_LOGE("RtcWriteAlarm failed");
        return -1;
    }
    OsalSleep(RTC_TEST_WAIT_TIME_S);
    if (g_rtcIrqCallback == HDF_FAILURE) {
        HDF_LOGE("RtcWriteAlarm failed");
        return -1;
    }
    g_rtcIrqCallback = HDF_FAILURE;
    return 0;
}

static int32_t RtcRegisterCallback(void)
{
    int32_t ret;

    if (g_rtcHandle == NULL) {
        return -1;
    }
    ret = RtcRegisterAlarmCallback(g_rtcHandle, RTC_ALARM_INDEX_A, RtcAlarmACallback);
    if (ret != 0) {
        HDF_LOGE("RtcRegisterCallback failed");
        return -1;
    }
    return 0;
}

static int32_t RtcRegisterNullCallback(void)
{
    int32_t ret;

    if (g_rtcHandle == NULL) {
        return -1;
    }

    ret = RtcRegisterAlarmCallback(g_rtcHandle, RTC_ALARM_INDEX_A, NULL);
    if (ret == 0) {
        HDF_LOGE("RtcRegisterCallback failed");
        return -1;
    }
    return 0;
}

static int32_t RtcSetNormalFreq(void)
{
    int32_t ret;

    if (g_rtcHandle == NULL) {
        return -1;
    }

    ret = RtcSetFreq(g_rtcHandle, RTC_TEST_FREQ);
    if (ret != 0) {
        HDF_LOGE("RtcSetNormalFreq failed");
        return -1;
    }
    return 0;
}

static int32_t RtcSetMaxFreq(void)
{
    int32_t ret;

    if (g_rtcHandle == NULL) {
        return -1;
    }

    ret = RtcSetFreq(g_rtcHandle, RTC_TEST_FREQ * RTC_TIME_UNIT);
    if (ret == 0) {
        HDF_LOGE("RtcSetMaxFreq failed");
        return -1;
    }
    return 0;
}

static int32_t RtcSetMinFreq(void)
{
    int32_t ret;

    if (g_rtcHandle == NULL) {
        return -1;
    }

    ret = RtcSetFreq(g_rtcHandle, 0);
    if (ret == 0) {
        HDF_LOGE("RtcSetMinFreq failed");
        return -1;
    }
    ret = RtcSetFreq(g_rtcHandle, RTC_TEST_FREQ);
    return ret;
}

static int32_t RtcReadWriteUserReg(void)
{
    int32_t ret;
    uint8_t value = RTC_TEST_USER_VALUE;

    if (g_rtcHandle == NULL) {
        return -1;
    }

    ret = RtcWriteReg(g_rtcHandle, 0, value);
    if (ret != 0) {
        HDF_LOGE("RtcReadWriteUserReg write failed");
        return -1;
    }
    ret = RtcReadReg(g_rtcHandle, 0, &value);
    if (ret != 0) {
        HDF_LOGE("RtcSetMinFreq read failed");
        return -1;
    }
    if (value != RTC_TEST_USER_VALUE) {
        return -1;
    }
    return 0;
}

static int32_t RtcReadWriteMaxUserIndex(void)
{
    int32_t ret;
    uint8_t value = RTC_TEST_USER_VALUE;

    if (g_rtcHandle == NULL) {
        return -1;
    }

    ret = RtcWriteReg(g_rtcHandle, RTC_TEST_USER_MAX_INDEX, value);
    if (ret == 0) {
        HDF_LOGE("RtcReadWriteUserReg write failed");
        return -1;
    }
    ret = RtcReadReg(g_rtcHandle, RTC_TEST_USER_MAX_INDEX, &value);
    if (ret == 0) {
        HDF_LOGE("RtcSetMinFreq read failed");
        return -1;
    }
    return 0;
}

static int32_t RtcTestSample(void)
{
    int32_t ret;
    struct RtcTime tm;
    const uint32_t freq = RTC_TEST_FREQ;

    if (g_rtcHandle == NULL) {
        return -1;
    }

    ret = RtcRegisterAlarmCallback(g_rtcHandle, RTC_ALARM_INDEX_A, RtcAlarmACallback);
    if (ret != 0) {
        return -1;
    }
    ret = RtcSetFreq(g_rtcHandle, freq);
    if (ret != 0) {
        return -1;
    }
    ret = RtcAlarmInterruptEnable(g_rtcHandle, RTC_ALARM_INDEX_A, 1);
    if (ret != 0) {
        return -1;
    }

    tm.year = RTC_TEST_TIME_YEAR;
    tm.month = RTC_JANUARY;
    tm.day = RTC_TEST_TIME_DAY;
    tm.hour = 0;
    tm.minute = 0;
    tm.second = 0;
    tm.millisecond = 0;

    ret = RtcWriteTime(g_rtcHandle, &tm);
    if (ret != 0) {
        return -1;
    }

    tm.second = RTC_TEST_TIME_SECOND;
    ret = RtcWriteAlarm(g_rtcHandle, RTC_ALARM_INDEX_A, &tm);
    if (ret != 0) {
        return -1;
    }
    OsalMSleep(RTC_TEST_WR_WAIT_MS);
    ret = RtcReadAlarm(g_rtcHandle, RTC_ALARM_INDEX_A, &tm);
    if (ret != 0) {
        return -1;
    }

    ret = RtcReadTime(g_rtcHandle, &tm);
    if (ret != 0) {
        return -1;
    }

    return 0;
}
#endif

static int32_t RtcReadWriteReliability(void)
{
    int i;
    for (i = 0; i < RTC_TEST_TIME; i++) {
        (void)RtcReadWriteTime();
        (void)RtcReadWriteAlarmTime();
#if defined(LOSCFG_DRIVERS_HDF_PLATFORM_RTC)
        (void)RtcSetNormalFreq();
#endif
    }
    return 0;
}

// add test case entry
HdfTestCaseList g_hdfRtcTestCaseList[] = {
    { RTC_INIT, RtcTestInit },
    { RTC_UNINIT, RtcTestUniInit },
    { RTC_WR_TIME, RtcReadWriteTime },
    { RTC_WR_MAX_TIME, RtcReadWriteMaxTime },
    { RTC_WR_MIN_TIME, RtcReadWriteMinTime },
    { RTC_WR_ALARM_TIME, RtcReadWriteAlarmTime },
    { RTC_WR_ALARM_MAX_TIME, RtcReadWriteMaxAlarmTime },
    { RTC_WR_ALARM_MIN_TIME, RtcReadWriteMinAlarmTime },
#if defined(CONFIG_DRIVERS_HDF_PLATFORM_RTC)
    { RTC_ALARM_ENABLE, RtcAlarmEnable },
#endif
#if defined(LOSCFG_DRIVERS_HDF_PLATFORM_RTC)
    { RTC_ALARM_IRQ, RtcAlarmIrq },
    { RTC_REGISTER_CALLBACK, RtcRegisterCallback },
    { RTC_REGISTER_CALLBACK_NULL, RtcRegisterNullCallback },
    { RTC_WR_FREQ, RtcSetNormalFreq },
    { RTC_WR_MAX_FREQ, RtcSetMaxFreq },
    { RTC_WR_MIN_FREQ, RtcSetMinFreq },
    { RTC_WR_USER_REG, RtcReadWriteUserReg },
    { RTC_WR_USER_REG_MAX_INDEX, RtcReadWriteMaxUserIndex },
    { RTC_FUNCTION_TEST, RtcTestSample },
#endif
    { RTC_WR_RELIABILITY, RtcReadWriteReliability },
};

int32_t HdfRtcEntry(HdfTestMsg *msg)
{
    int i;

    if (msg == NULL) {
        return HDF_FAILURE;
    }

    for (i = 0; i < sizeof(g_hdfRtcTestCaseList) / sizeof(g_hdfRtcTestCaseList[0]); ++i) {
        if (msg->subCmd != g_hdfRtcTestCaseList[i].subCmd) {
            continue;
        }
        if (g_hdfRtcTestCaseList[i].testFunc == NULL) {
            msg->result = HDF_FAILURE;
            continue;
        }
        msg->result = g_hdfRtcTestCaseList[i].testFunc();
        if (msg->result != HDF_SUCCESS) {
            continue;
        }
    }

    return HDF_SUCCESS;
}
