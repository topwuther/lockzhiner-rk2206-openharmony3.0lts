/*
 * rtc_adapter.c
 *
 * rtc driver adapter of linux
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/rtc.h>
#include "device_resource_if.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "rtc_core.h"

#define HDF_LOG_TAG RTC_ADAPTER
#define MONTH_DIFF 1
#define YEAR_BASE 1900

static inline void HdfTimeToLinuxTime(const struct RtcTime *hdfTime, struct rtc_time *linuxTime)
{
    linuxTime->tm_sec = hdfTime->second;
    linuxTime->tm_min = hdfTime->minute;
    linuxTime->tm_hour = hdfTime->hour;
    linuxTime->tm_mday = hdfTime->day;
    linuxTime->tm_mon = hdfTime->month - MONTH_DIFF;
    linuxTime->tm_year = hdfTime->year - YEAR_BASE;
    linuxTime->tm_wday = hdfTime->weekday;
    linuxTime->tm_yday = rtc_year_days(linuxTime->tm_mday, linuxTime->tm_mon, linuxTime->tm_year);
}

static inline void LinuxTimeToHdfTime(struct RtcTime *hdfTime, const struct rtc_time *linuxTime)
{
    hdfTime->second = linuxTime->tm_sec;
    hdfTime->minute = linuxTime->tm_min;
    hdfTime->hour = linuxTime->tm_hour;
    hdfTime->day = linuxTime->tm_mday;
    hdfTime->month = linuxTime->tm_mon + MONTH_DIFF;
    hdfTime->year = linuxTime->tm_year + YEAR_BASE;
    hdfTime->weekday = linuxTime->tm_wday;
}

static inline struct rtc_device *HdfGetRtcDevice(void)
{
    struct rtc_device *dev = rtc_class_open(CONFIG_RTC_SYSTOHC_DEVICE);

    if (dev == NULL) {
        HDF_LOGE("%s: failed to get rtc device", __func__);
    }
    return dev;
}

static inline void HdfPutRtcDevice(struct rtc_device *dev)
{
    rtc_class_close(dev);
}

static int32_t HiRtcReadTime(struct RtcHost *host, struct RtcTime *hdfTime)
{
    int32_t ret;
    struct rtc_time linuxTime = {0};
    struct rtc_device *dev = HdfGetRtcDevice();

    (void)host;
    if (dev == NULL) {
        return HDF_FAILURE;
    }
    ret = rtc_read_time(dev, &linuxTime);
    if (ret < 0) {
        HDF_LOGE("%s: rtc_read_time error, ret is %d", __func__, ret);
        return ret;
    }
    HdfPutRtcDevice(dev);
    LinuxTimeToHdfTime(hdfTime, &linuxTime);
    return HDF_SUCCESS;
}

static int32_t HiRtcWriteTime(struct RtcHost *host, const struct RtcTime *hdfTime)
{
    int32_t ret;
    struct rtc_time linuxTime = {0};
    struct rtc_device *dev = HdfGetRtcDevice();

    (void)host;
    if (dev == NULL) {
        return HDF_FAILURE;
    }
    HdfTimeToLinuxTime(hdfTime, &linuxTime);
    ret = rtc_set_time(dev, &linuxTime);
    if (ret < 0) {
        HDF_LOGE("%s: rtc_set_time error, ret is %d", __func__, ret);
        return ret;
    }

    HdfPutRtcDevice(dev);
    return HDF_SUCCESS;
}

static int32_t HiReadAlarm(struct RtcHost *host, enum RtcAlarmIndex alarmIndex, struct RtcTime *hdfTime)
{
    int32_t ret;
    struct rtc_wkalrm alarm = {0};
    struct rtc_device *dev = HdfGetRtcDevice();

    (void)host;
    (void)alarmIndex;
    if (dev == NULL) {
        return HDF_FAILURE;
    }
    ret = rtc_read_alarm(dev, &alarm);
    if (ret < 0) {
        HDF_LOGE("%s: rtc_read_alarm error, ret is %d", __func__, ret);
        return ret;
    }

    LinuxTimeToHdfTime(hdfTime, &(alarm.time));
    HdfPutRtcDevice(dev);
    return HDF_SUCCESS;
}

static int32_t HiWriteAlarm(struct RtcHost *host, enum RtcAlarmIndex alarmIndex, const struct RtcTime *hdfTime)
{
    int32_t ret;
    struct rtc_wkalrm alarm = {0};
    struct rtc_device *dev = HdfGetRtcDevice();

    (void)host;
    (void)alarmIndex;
    if (dev == NULL) {
        return HDF_FAILURE;
    }

    HdfTimeToLinuxTime(hdfTime, &(alarm.time));
    alarm.enabled = 0;
    ret = rtc_set_alarm(dev, &alarm);
    if (ret < 0) {
        HDF_LOGE("%s: rtc_read_alarm error, ret is %d", __func__, ret);
        return ret;
    }

    HdfPutRtcDevice(dev);
    return HDF_SUCCESS;
}

static int32_t HiAlarmInterruptEnable(struct RtcHost *host, enum RtcAlarmIndex alarmIndex, uint8_t enable)
{
    int32_t ret;
    struct rtc_device *dev = HdfGetRtcDevice();

    (void)host;
    (void)alarmIndex;
    if (dev == NULL) {
        return HDF_FAILURE;
    }
    ret = rtc_alarm_irq_enable(dev, enable);
    if (ret < 0) {
        HDF_LOGE("%s: rtc_read_alarm error, ret is %d", __func__, ret);
        return ret;
    }

    HdfPutRtcDevice(dev);
    return HDF_SUCCESS;
}

static struct RtcMethod g_method = {
    .ReadTime = HiRtcReadTime,
    .WriteTime = HiRtcWriteTime,
    .ReadAlarm = HiReadAlarm,
    .WriteAlarm = HiWriteAlarm,
    .RegisterAlarmCallback = NULL,
    .AlarmInterruptEnable = HiAlarmInterruptEnable,
    .GetFreq = NULL,
    .SetFreq = NULL,
    .Reset = NULL,
    .ReadReg = NULL,
    .WriteReg = NULL,
};

static int32_t HiRtcBind(struct HdfDeviceObject *device)
{
    struct RtcHost *host = NULL;

    host = RtcHostCreate(device);
    if (host == NULL) {
        HDF_LOGE("%s: create host fail", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    host->device = device;
    device->service = &host->service;
    return HDF_SUCCESS;
}

static int32_t HiRtcInit(struct HdfDeviceObject *device)
{
    struct RtcHost *host = NULL;

    if (device == NULL) {
        HDF_LOGE("%s: err, device is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    host = RtcHostFromDevice(device);
    host->method = &g_method;
    HDF_LOGI("%s: Hdf dev service:%s init success", __func__, HdfDeviceGetServiceName(device));
    return HDF_SUCCESS;
}

static void HiRtcRelease(struct HdfDeviceObject *device)
{
    struct RtcHost *host = NULL;

    if (device == NULL) {
        return;
    }

    host = RtcHostFromDevice(device);
    RtcHostDestroy(host);
}

struct HdfDriverEntry g_rtcDriverEntry = {
    .moduleVersion = 1,
    .Bind = HiRtcBind,
    .Init = HiRtcInit,
    .Release = HiRtcRelease,
    .moduleName = "HDF_PLATFORM_RTC",
};

HDF_INIT(g_rtcDriverEntry);
